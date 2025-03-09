#include <neobox/process.h>
#include <neobox/unicode.h>

namespace fs = std::filesystem;

#ifdef _DEBUG
#include <iostream>
#endif

#ifdef _WIN32
#include <Windows.h>

struct WinProcess {
  HANDLE hProcess;
  HANDLE hWaitHandle;
  HANDLE hPipeReadOutput;
  HANDLE hPipeWriteOutput;
  HANDLE hPipeReadError;
  HANDLE hPipeWriteError;
};

typedef WinProcess ProcessHandle;
#else
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

struct UnixProcess {
  pid_t pid;
  int pipeStdout[2];
  int pipeStderr[2];
};

static std::map<int, NeoProcess*> st_Processes;
static std::mutex st_ProcessesMutex;
static std::condition_variable st_ProcessesCond;

static void InsertProcess(pid_t pid, NeoProcess* process) {
  std::lock_guard lock(st_ProcessesMutex);
  st_Processes[pid] = process;
}

static void RemoveProcess(pid_t pid) {
  std::lock_guard lock(st_ProcessesMutex);
  st_Processes.erase(pid);
}

static NeoProcess* FindProcess(pid_t pid) {
  std::lock_guard lock(st_ProcessesMutex);
  if (auto iter = st_Processes.find(pid); iter != st_Processes.end()) {
    return iter->second;
  }
  return nullptr;
}

typedef UnixProcess ProcessHandle;
#endif

NeoProcess::NeoProcess(const std::u8string& command)
  : m_AppPath()
  , m_Args(command)
  , m_WorkDir(fs::current_path())
  , m_Handle(ProcessHandle {})
{
}

NeoProcess::NeoProcess(const Path& app, const std::u8string& args)
  : m_AppPath(app)
  , m_Args(args)
  , m_WorkDir(fs::current_path())
  , m_Handle(ProcessHandle {})
{
}

NeoProcess::~NeoProcess()
{
  Stop(true);
}

void NeoProcess::SetAppPath(const Path& app)
{
  m_AppPath = app;
}

void NeoProcess::SetWorkDir(const std::u8string& dir)
{
  m_WorkDir = dir;
}

void NeoProcess::SetArgs(const std::u8string& args)
{
  m_Args = args;
}

void NeoProcess::SetEnvs(std::vector<std::u8string> envs)
{
  m_Envs = std::move(envs);
}

#ifdef _WIN32
void NeoProcess::Stop(bool force)
{
  if (!m_IsRunning) {
    return;
  }

  auto& handle = std::any_cast<ProcessHandle&>(m_Handle);

  if (force) {
    TerminateProcess(handle.hProcess, 0);
  } else {
    CloseHandle(handle.hProcess);
    handle.hProcess = NULL;
  }

  CleanUp();

  m_IsRunning = false;
  m_ExitCode = -1;
}
#else
void NeoProcess::Stop(bool force)
{
  if (!m_IsRunning) {
    return;
  }

  auto& handle = std::any_cast<ProcessHandle&>(m_Handle);

  if (force) {
    kill(handle.pid, SIGKILL);
  } else {
    kill(handle.pid, SIGTERM);
  }

  CleanUp();

  m_IsRunning = false;
  m_ExitCode = -1;
}
#endif

bool NeoProcess::ParseCommand(const std::u8string& command)
{
  if (command.empty()) {
    return false;
  }

  std::u8string app;
  std::u8string args;

  char8_t quote = 0;
  bool inQuote = false;
  bool inArgs = false;

  for (auto const c : command) {
    if (c == u8'"' || c == u8'\'') {
      if (inQuote) {
        if (quote == c) {
          inQuote = false;
          continue;
        }
      } else {
        inQuote = true;
        quote = c;
        continue;
      }
    } else if (c == u8' ' || c == u8'\t') {
      if (!inQuote) {
        inArgs = true;
        continue;
      }
    }
    if (inArgs) {
      args.push_back(c);
    } else {
      app.push_back(c);
    }
  }

  if (app.empty()) {
    return false;
  }

  m_AppPath = app;
  m_Args = args;

  return true;
}

void NeoProcess::ParseArgs(const std::u8string& command, std::vector<std::string>& args)
{
  std::string arg;

  char8_t quote = 0;
  bool inQuote = false;

  for (auto const c : command) {
    if (c == u8'"' || c == u8'\'') {
      if (inQuote) {
        if (quote == c) {
          inQuote = false;
          continue;
        }
      } else {
        inQuote = true;
        quote = c;
        continue;
      }
    } else if (c == u8' ' || c == u8'\t') {
      if (!inQuote) {
        if (!arg.empty()) {
          args.push_back(arg);
          arg.clear();
        }
        continue;
      }
    }
    arg.push_back(c);
  }

  if (!arg.empty()) {
    args.push_back(arg);
  }
}

void NeoProcess::DoSuspend(std::coroutine_handle<> handle)
{
  Base::DoSuspend(handle);

  if (!m_IsRunning) {
    if (!StartProcess()) {
      m_ExitCode = -1;
      Base::m_Handle.resume();
    }
  }
}

#ifdef _WIN32
static std::optional<std::wstring> GetEnvBlock(const std::optional<std::vector<std::u8string>>& envs) {
  if (!envs) return std::nullopt;

  if (envs->empty()) return std::wstring { L'\0', L'\0' };

  std::optional envBlock = std::wstring {};

  for (const auto& env : *envs) {
    envBlock->append(Utf82Wide(env));
    envBlock->push_back(L'\0');
  }
  envBlock->push_back(L'\0');

  return envBlock;
}
#else
static std::optional<std::vector<char*>> GetEnvBlock(std::optional<std::vector<std::u8string>>& envs) {
  if (!envs) return std::nullopt;

  if (envs->empty()) return std::vector<char*> { nullptr };

  std::optional envBlock = std::vector<char*> {};

  for (auto& env : *envs) {
    if (!env.ends_with(u8'\0')) {
      env.push_back(u8'\0');
    }
    envBlock->push_back(reinterpret_cast<char*>(env.data()));
  }
  envBlock->push_back(nullptr);

  return envBlock;
}
#endif

AsyncInt NeoProcess::Run()
{
  if (m_IsRunning) {
    throw std::runtime_error("Process is already running.");
  }

  auto res = co_await AsyncAwaiter<int> { this };

  if (res) {
    co_return *res;
  }

  throw std::runtime_error("Process failed to start.");
}

#ifdef _WIN32
bool NeoProcess::StartProcess() {
  struct Guard {
    ~Guard() {
      if (!m_Success) m_Process.CleanUp();
    }
    NeoProcess& m_Process;
    bool m_Success = false;
  } guard(*this);

  auto& handle = std::any_cast<ProcessHandle&>(m_Handle);

  // IO_CONTEXT ioContext {};
  // ioContext.overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

  SECURITY_ATTRIBUTES saOutput = {
    .nLength = sizeof(SECURITY_ATTRIBUTES),
    .lpSecurityDescriptor = NULL,
    .bInheritHandle = TRUE
  };

  SECURITY_ATTRIBUTES saError = {
    .nLength = sizeof(SECURITY_ATTRIBUTES),
    .lpSecurityDescriptor = NULL,
    .bInheritHandle = TRUE
  };

  // 创建输出管道
  if (!CreatePipe(&handle.hPipeReadOutput, &handle.hPipeWriteOutput, &saOutput, 0)) {
#ifdef _DEBUG
    std::cout << "CreatePipe失败！错误码：" << GetLastError() << std::endl;
#endif
    return false;
  }

  SetHandleInformation(handle.hPipeReadOutput, HANDLE_FLAG_INHERIT, 0);

  // 创建错误管道
  if (!CreatePipe(&handle.hPipeReadError, &handle.hPipeWriteError, &saError, 0)) {
#ifdef _DEBUG
    std::cout << "CreatePipe失败！错误码：" << GetLastError() << std::endl;
#endif
    return false;
  }

  SetHandleInformation(handle.hPipeReadError, HANDLE_FLAG_INHERIT, 0);

  // 创建进程启动信息
  STARTUPINFO si {};
  si.cb = sizeof(STARTUPINFO);
  si.hStdOutput = handle.hPipeWriteOutput;
  si.hStdError = handle.hPipeWriteError;
  si.wShowWindow = SW_HIDE;
  si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

  PROCESS_INFORMATION pi {};
  auto&& app = m_AppPath.make_preferred().wstring();
  WCHAR* szApp = nullptr;
  if (!app.empty()) {
    app.push_back(L'\0');
    szApp = app.data();
  }

  auto cmdLine = Utf82Wide(m_Args);
  WCHAR* szCmdLine = nullptr;
  if (cmdLine.empty()) {
    if (!szApp) {
      return false;
    }
  } else {
    cmdLine.push_back(L'\0');
    szCmdLine = cmdLine.data();
  }

  auto&& envBlock = GetEnvBlock(m_Envs);
  auto szEnvBlock = envBlock ? envBlock->data() : nullptr;

  auto&& workDir = m_WorkDir.make_preferred().wstring();
  WCHAR* szWorkDir = nullptr;
  if (!workDir.empty()) {
    workDir.push_back(L'\0');
    szWorkDir = workDir.data();
  }

  // 创建子进程
  if (!CreateProcessW(szApp, // 应用程序路径
      szCmdLine,             // 命令行
      NULL,                  // 进程安全描述符
      NULL,                  // 线程安全描述符
      TRUE,                  // 子进程继承句柄
      0,                     // 创建标志
      szEnvBlock,            // 环境变量
      szWorkDir,             // 工作目录
      &si, &pi)) {
#ifdef _DEBUG
    std::cout << "CreateProcess失败！错误码：" << GetLastError() << std::endl;
#endif
    return false;
  }

  handle.hProcess = pi.hProcess;

  // 立即关闭不需要的线程句柄
  CloseHandle(pi.hThread);
  CloseHandle(handle.hPipeWriteOutput);
  handle.hPipeWriteOutput = NULL;
  CloseHandle(handle.hPipeWriteError);
  handle.hPipeWriteError = NULL;


// 进程结束回调函数
  VOID /*CALLBACK*/ (*callback)(PVOID lpParam, BOOLEAN timerOrWaitFired);
  
  callback = [](PVOID lpParam, BOOLEAN timerOrWaitFired) {
    auto self = static_cast<NeoProcess*>(lpParam);
    auto handle = std::any_cast<WinProcess&>(self->m_Handle);

    DWORD dwExitCode;

    // 获取进程退出码
    if (GetExitCodeProcess(handle.hProcess, &dwExitCode)) {
#ifdef _DEBUG
      std::cout << "子进程已退出，退出码：" << dwExitCode << std::endl;
#endif
    } else {
#ifdef _DEBUG
      std::cout << "获取退出码失败！错误码：" << GetLastError() << std::endl;
#endif
    }

    self->m_ExitCode = dwExitCode;

    if (!timerOrWaitFired) {
      self->ReadOutput();
      self->ReadError();
    }

    // 关闭进程句柄
    self->CleanUp();
    self->m_IsRunning = false;

    self->Base::m_Handle.resume();
  };

  // 注册异步等待
  if (!RegisterWaitForSingleObject(&handle.hWaitHandle, pi.hProcess, callback,
        this, // 将进程句柄作为参数传递给回调
        INFINITE, WT_EXECUTEONLYONCE)) {
#ifdef _DEBUG
    std::cout << "RegisterWaitForSingleObject失败！错误码：" << GetLastError() << std::endl;
#endif
    return false;
  }

  m_IsRunning = true;
  guard.m_Success = true;
  return true;
}
#else
bool NeoProcess::StartProcess() {
  struct Guard {
    ~Guard() {
      if (!m_Success) m_Process.CleanUp();
    }
    NeoProcess& m_Process;
    bool m_Success = false;
  } guard(*this);

  auto& handle = std::any_cast<ProcessHandle&>(m_Handle);

  void (*callback)(int);


  callback = [](int sig) {
    (void)sig;
#ifdef _DEBUG
    std::cout << "子进程已退出，退出码：" << sig << std::endl;
#endif
    pid_t id;
    int status;
    while((id = waitpid(-1, &status, WNOHANG)) > 0) {   
    printf("wait child success:%d\n", id);
    auto const self = ::FindProcess(id);
    if (nullptr == self) continue;
    ::RemoveProcess(id);

    self->m_ExitCode = WEXITSTATUS(status);

    if (WIFEXITED(status)) {
      printf("child exit normally\n");
    } else {
      printf("child exit abnormally\n");
    }

    self->ReadOutput();
    self->ReadError();

    self->CleanUp();
    self->m_IsRunning = false;

    self->Base::m_Handle.resume();
    }
  };

  ::signal(SIGCHLD, callback);

  if (pipe(handle.pipeStdout) == -1) {
    return false;
  }

  if (pipe(handle.pipeStderr) == -1) {
    return false;
  }

  handle.pid = fork();
  if (handle.pid == -1) {
    return false;
  }

  if (handle.pid == 0) { // 子进程
    close(handle.pipeStdout[0]);
    dup2(handle.pipeStdout[1], STDOUT_FILENO);
    close(handle.pipeStderr[0]);
    dup2(handle.pipeStderr[1], STDERR_FILENO);

    auto app = fs::absolute(m_AppPath).make_preferred().string();
    std::vector<std::string> argv = { m_AppPath.filename().string() };
    ParseArgs(m_Args, argv);
    std::vector<char*> cargv;
    for (auto& arg : argv) {
      arg.push_back('\0');
      cargv.push_back(reinterpret_cast<char*>(arg.data()));
    }
    cargv.push_back(nullptr);

    if (!m_WorkDir.empty()) {
      fs::current_path(m_WorkDir);
    }

    auto envBlock = GetEnvBlock(m_Envs);
    if (envBlock) {
      execve(app.c_str(), cargv.data(), envBlock->data());
    } else {
      execv(app.c_str(), cargv.data());
    }

    exit(EXIT_FAILURE);
  } else { // 父进程
    ::InsertProcess(handle.pid, this);
    // close(handle.pipFd[1]);
    close(handle.pipeStdout[1]);
    close(handle.pipeStderr[1]);
  }

  m_IsRunning = true;
  guard.m_Success = true;
  return true;
}
#endif

#ifdef _WIN32
static void ReadOutput(HANDLE hPipeReadOutput, std::u8string& output) {
  DWORD dwRead;
  CHAR chBuf[4096];
  BOOL bSuccess = FALSE;

  for (;;) {
    bSuccess = ReadFile(hPipeReadOutput, chBuf, 4096, &dwRead, NULL);
    if (!bSuccess || dwRead == 0) {
      break;
    }
    chBuf[dwRead] = '\0';
    output.append(chBuf, chBuf + dwRead);
  }
}
#else
static void ReadOutput(int fd, std::string& output) {
  char buf[4096];
  ssize_t n;
  while ((n = read(fd, buf, sizeof(buf))) > 0) {
    output.append(buf, n);
  }
}
#endif

#ifdef _WIN32
void NeoProcess::CleanUp()
{
  auto& handle = std::any_cast<ProcessHandle&>(m_Handle);

  if (handle.hWaitHandle) {
    UnregisterWait(handle.hWaitHandle);
    handle.hWaitHandle = NULL;
  }

  CloseHandle(handle.hPipeReadOutput);
  handle.hPipeReadOutput = NULL;
  CloseHandle(handle.hPipeWriteOutput);
  handle.hPipeWriteOutput = NULL;

  CloseHandle(handle.hPipeReadError);
  handle.hPipeReadError = NULL;
  CloseHandle(handle.hPipeWriteError);
  handle.hPipeWriteError = NULL;

  CloseHandle(handle.hProcess);
  handle.hProcess = NULL;
}
#else
void NeoProcess::CleanUp()
{
  auto& handle = std::any_cast<ProcessHandle&>(m_Handle);

  close(handle.pipeStdout[0]);
  close(handle.pipeStdout[1]);
  close(handle.pipeStderr[0]);
  close(handle.pipeStderr[1]);
}
#endif

extern std::u8string Ansi2Utf8(std::string_view ansi);

void NeoProcess::ReadOutput()
{
  auto& handle = std::any_cast<ProcessHandle&>(m_Handle);
#ifdef _WIN32
  ::ReadOutput(handle.hPipeReadOutput, m_StdOut);
#else
  ::ReadOutput(handle.pipeStdout[0], m_StdOut);
#endif
}

void NeoProcess::ReadError()
{
  auto& handle = std::any_cast<ProcessHandle&>(m_Handle);
#ifdef _WIN32
  ::ReadOutput(handle.hPipeReadError, m_StdErr);
#else
  ::ReadOutput(handle.pipeStderr[0], m_StdErr);
#endif
}
