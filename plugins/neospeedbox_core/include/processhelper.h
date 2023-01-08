#ifndef PROCESSHELPER_H
#define PROCESSHELPER_H

#include <wtypes.h>
#include <string>
#include <set>

#define PROCESS_SIMPLE_INFO

struct ProcessInfo {
  // variables
  std::wstring exeFile;
  std::wstring commandLine;
  HANDLE processHandle;
  DWORD processID;
  SIZE_T workingSetSize;
  // SIZE_T pagefileUsage;
#ifndef PROCESS_SIMPLE_INFO
  DWORD cntThreads;
  // DWORD64 lastCpuTime = 0;
  DWORD64 cpuTime;
  ULONGLONG diskReadBytes;
  ULONGLONG diskWriteBytes;
  LONG netReceivedBytes;
  LONG netSentBytes;
#endif

  // functions
  // explicit ProcessInfo(PROCESSENTRY32& pe32);
  bool GetMemoryUsage();
};

class ProcessHelper {
public:
  typedef bool(*Cmp)(const ProcessInfo*, const ProcessInfo*);
public:
  std::set<ProcessInfo*, Cmp> m_ProcessInfo;
  bool GetProcessInfo();
  explicit ProcessHelper();
  ~ProcessHelper();
private:
  bool HasProcess(DWORD id) const;
  void ClearInfo();
};

#endif // PROCESSHELPER_H
