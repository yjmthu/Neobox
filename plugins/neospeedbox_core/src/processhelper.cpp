#include <processhelper.h>

#ifdef _WIN32
#include <tlhelp32.h>
#include <psapi.h>
#else
#include <unistd.h>
#include <fstream>
namespace fs = std::filesystem;
#endif

// extern uint64_t Filetime2Int64(const FILETIME& ftime);
// extern const DWORD g_ProcessorCount;

// https://blog.csdn.net/qq_41883523/article/details/109706056

/*
bool ProcessInfo::GetCpuUsage()
{
  //上一次的时间  
  static int64_t lastTime = 0;
  static int64_t lastSystemTime = 0;

  FILETIME creation_time;
  FILETIME exit_time;
  FILETIME kernel_time;
  FILETIME user_time;

  // GetSystemTimeAsFileTime(&now);
  if (!GetProcessTimes(processHandle, &creation_time, &exit_time,
    &kernel_time, &user_time))
  {
    // We don't assert here because in some cases (such as in the Task Manager)  
    // we may call this function on a process that has just exited but we have  
    // not yet received the notification.  
    return false;
  }
  cpuTime = (Filetime2Int64(kernel_time) + Filetime2Int64(user_time)) / g_ProcessorCount;
  return true;
}
*/

#ifdef _WIN32
bool ProcessInfo::GetMemoryUsage()
{
  static PROCESS_MEMORY_COUNTERS pmc;
  if (GetProcessMemoryInfo(processHandle, &pmc, sizeof(pmc))) {
    workingSetSize = pmc.WorkingSetSize;
    // pagefileUsage = pmc.PagefileUsage;
    return true;
  }
  return false;
}
#else
std::string ProcessHelper::GetCmdLine(const std::filesystem::path& dir) {
  std::ifstream file(dir / "cmdline");
  std::string line;
  std::getline(file, line);
  file.close();
  return line;
}

std::string ProcessHelper::GetExeName(const std::filesystem::path& dir) {
  std::ifstream file(dir / "comm");
  std::string name;
  std::getline(file, name);
  file.close();
  return name;
}

size_t ProcessHelper::GetMemUsage(const std::filesystem::path& dir) {
  std::ifstream file(dir / "statm");
  size_t value;
  file >> value >> value;
  file.close();
  return value * m_PageSize;
}
#endif

#ifdef _WIN32
const size_t ProcessHelper::m_PageSize = []() {
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  return info.dwPageSize;
}();
#else
const size_t ProcessHelper::m_PageSize = getpagesize();
#endif

ProcessHelper::ProcessHelper()
	: m_ProcessInfo([](const ProcessInfo* a, const ProcessInfo* b)->bool{
		return a->workingSetSize > b->workingSetSize;
	})
{
	//
}

ProcessHelper::~ProcessHelper()
{
	ClearInfo();
}

void ProcessHelper::ClearInfo()
{
	for (auto i: m_ProcessInfo) {
		delete i;
	}
	m_ProcessInfo.clear();
}

bool ProcessHelper::HasProcess(ProcessInfo::Pid id) const {
  for (auto i: m_ProcessInfo) {
    if (i->processID == id) {
      return true;
    }
  }
  return false;
}

bool ProcessHelper::GetProcessInfo()
{
	ClearInfo();

#ifdef _WIN32
  // std::map<DWORD, ProcessInfo> mapPIdInfo{};
  PROCESSENTRY32 pe32;
  pe32.dwSize = sizeof(pe32);

  HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcessSnap == INVALID_HANDLE_VALUE) {
    return false;
  }

  for (auto bResult = Process32First(hProcessSnap, &pe32); bResult; bResult = Process32Next(hProcessSnap, &pe32))
  {
    // if (HasProcess(pe32.th32ProcessID)) {
    //   continue;
    // }
    auto processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
    if (!processHandle) continue;
    DWORD size = MAX_PATH;
    std::wstring sExeName(MAX_PATH, L'\0');
    if (FALSE == QueryFullProcessImageName(processHandle, 0, sExeName.data(), &size)) {
      continue;
    }
    sExeName.erase(sExeName.find(L'\0'));
    
    //GetModuleFileNameExW(processHaldle,nullptr, szProcessName, MAX_PATH); //win7下出错
		auto const info = new ProcessInfo {
			pe32.szExeFile,
      std::move(sExeName),
			processHandle,
			pe32.th32ProcessID,
			0
		};
		if (info->GetMemoryUsage()) {
			m_ProcessInfo.insert(info);
		} else {
			delete info;
		}
		CloseHandle(processHandle);
	}
  CloseHandle(hProcessSnap);
#else
  for (auto& dirEntry: fs::directory_iterator("/proc")) {
    if (!dirEntry.is_directory()) continue;
    fs::path path = dirEntry.path();
    std::string const name = path.stem().string();
    if (!name.empty() && std::isdigit(name.front())) {
      pid_t pid = std::stoi(path.stem().string());
      auto const info = new ProcessInfo {
        GetExeName(path),
        GetCmdLine(path),
        pid,
        GetMemUsage(path)
      };
      m_ProcessInfo.insert(info);
    }
  }
#endif
  return true;
}
