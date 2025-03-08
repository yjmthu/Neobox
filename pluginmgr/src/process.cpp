#include <neobox/process.h>
#include <neobox/systemapi.h>

namespace fs = std::filesystem;

struct WinProcess {
  HANDLE hProcess;
  HANDLE hWaitHandle;
  HANDLE hPipeReadOutput;
  HANDLE hPipeWriteOutput;
  HANDLE hPipeReadError;
  HANDLE hPipeWriteError;
};

NeoProcess::NeoProcess(const std::u8string& command)
  : m_AppPath()
  , m_Args(command)
  , m_WorkDir(fs::current_path())
  , m_Handle(WinProcess {})
{
}

NeoProcess::NeoProcess(const Path& app, const std::u8string& args)
  : m_AppPath(app)
  , m_Args(args)
  , m_WorkDir(fs::current_path())
  , m_Handle(WinProcess {})
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

void NeoProcess::Stop(bool force)
{
  if (!m_IsRunning) {
    return;
  }

  auto& handle = std::any_cast<WinProcess&>(m_Handle);

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

// #define BUFFER_SIZE 4096

// typedef struct {
//     OVERLAPPED overlapped;
//     CHAR buffer[BUFFER_SIZE];
//     DWORD bytesRead;
// } IO_CONTEXT;

static std::optional<std::wstring> GetEnvBlock(const std::optional<std::vector<std::u8string>>& envs) {
  if (!envs) return std::nullopt;

  if (envs->empty()) return std::wstring { L'\0', L'\0' };

  std::optional envBlock = std::wstring {};

  for (const auto& env : *envs) {
    envBlock->append(Utf82WideString(env));
    envBlock->push_back(L'\0');
  }
  envBlock->push_back(L'\0');

  return envBlock;
}

AsyncAwaiter<int> NeoProcess::Run()
{
  if (m_IsRunning) {
    return { nullptr };
  }

  return { this };
}

bool NeoProcess::StartProcess() {
  struct Guard {
    ~Guard() {
      if (!m_Success) m_Process.CleanUp();
    }
    NeoProcess& m_Process;
    bool m_Success = false;
  } guard(*this);

  auto& handle = std::any_cast<WinProcess&>(m_Handle);

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

  auto cmdLine = Utf82WideString(m_Args);
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

  // HANDLE hCompletionPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0);

  // CreateIoCompletionPort(hPipeReadOutput, hCompletionPort, (ULONG_PTR)hPipeReadOutput, 0);

  // BOOL bResult = ReadFile(
  //   hPipeReadOutput,
  //   ioContext.buffer,
  //   BUFFER_SIZE - 1,
  //   &ioContext.bytesRead,
  //   &ioContext.overlapped
  // );

  // if (!bResult && GetLastError() != ERROR_IO_PENDING) {
  //   // 错误处理
  //   return false;
  // }

  m_IsRunning = true;
  guard.m_Success = true;
  return true;
}

// void Read(HANDLE hCompletionPort, HANDLE readPipe) {
//   DWORD dwBytesTransferred;
//   ULONG_PTR completionKey;
//   LPOVERLAPPED pOverlapped;

//   while (GetQueuedCompletionStatus(hCompletionPort, &dwBytesTransferred,
//                                    &completionKey, &pOverlapped, INFINITE)) {
//     IO_CONTEXT *pIoContext =
//         CONTAINING_RECORD(pOverlapped, IO_CONTEXT, overlapped);
//     pIoContext->buffer[dwBytesTransferred] = '\0';

//     // 处理数据（如发送到UI线程）
//     // PostMessage(hWnd, WM_USER_OUTPUT, (WPARAM)pIoContext, 0);

//     // 继续发起下一个异步读取
//     ReadFile(readPipe, pIoContext->buffer, BUFFER_SIZE - 1, NULL,
//              &pIoContext->overlapped);
//   }
// }

static void ReadOutput(HANDLE hPipeReadOutput, std::string& output) {
  DWORD dwRead;
  CHAR chBuf[4096];
  BOOL bSuccess = FALSE;

  for (;;) {
    bSuccess = ReadFile(hPipeReadOutput, chBuf, 4096, &dwRead, NULL);
    if (!bSuccess || dwRead == 0) {
      break;
    }
    chBuf[dwRead] = '\0';
    output += chBuf;
  }
}

void NeoProcess::CleanUp()
{
  auto& handle = std::any_cast<WinProcess&>(m_Handle);

  if (handle.hWaitHandle) {
    UnregisterWait(handle.hWaitHandle);
    handle.hWaitHandle = NULL;
  }

  CloseHandle(handle.hPipeReadOutput);
  handle.hPipeReadOutput = NULL;
  CloseHandle(handle.hPipeWriteOutput);
  handle.hPipeWriteOutput = NULL;
  CloseHandle(handle.hProcess);
  handle.hProcess = NULL;
}

void NeoProcess::ReadOutput()
{
  auto& handle = std::any_cast<WinProcess&>(m_Handle);
  ::ReadOutput(handle.hPipeReadOutput, m_StdOut);
}

void NeoProcess::ReadError()
{
  auto& handle = std::any_cast<WinProcess&>(m_Handle);
  ::ReadOutput(handle.hPipeReadError, m_StdErr);
}
