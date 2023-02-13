#ifndef PROCESSHELPER_H
#define PROCESSHELPER_H

#include <string>
#include <set>
#ifdef _WIN32
#include <wtypes.h>
#else
#include <filesystem>
#include <iostream>
#endif

#define PROCESS_SIMPLE_INFO

struct ProcessInfo {
#ifdef _WIN32
  typedef std::wstring String;
  typedef DWORD Pid;
  typedef SIZE_T Size;
#else
  typedef std::string String;
  typedef pid_t Pid;
  typedef size_t Size;
#endif
  // variables
  String exeFile;
  String commandLine;
#ifdef WIN32
  HANDLE processHandle;
#endif
  Pid processID;
  Size workingSetSize;
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
#ifdef _WIN32
  bool GetMemoryUsage();
#endif
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
  bool HasProcess(ProcessInfo::Pid id) const;
  void ClearInfo();
  static const size_t m_PageSize;
#ifdef __linux__
  static std::string GetCmdLine(const std::filesystem::path& dir);
  static std::string GetExeName(const std::filesystem::path& dir);
  static size_t GetMemUsage(const std::filesystem::path& dir);
#endif
};

#endif // PROCESSHELPER_H
