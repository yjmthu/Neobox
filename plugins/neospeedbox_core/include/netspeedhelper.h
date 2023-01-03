#ifndef NETSPEEDHELPER_H
#define NETSPEEDHELPER_H

#include <set>
#include <vector>
#ifdef _WIN32
#include <Winsock2.h>
// #include <Windows.h>
#include <Iphlpapi.h>
#endif

#include <trafficinfo.h>

class NetSpeedHelper {
 public:
  explicit NetSpeedHelper();
#ifdef WIN32
  ~NetSpeedHelper();
#endif

  struct IpAdapter {
    std::string adapterName;  // ANSI
    std::wstring friendlyName;
    bool enabled;
    IF_INDEX index;
  };
  
  void UpdateAdaptersAddresses();
  void GetSysInfo();
  std::set<std::string> m_AdapterBalckList; // ANSI
  std::vector<IpAdapter> m_Adapters;
  TrafficInfo m_TrafficInfo;

 private:
#ifdef _WIN32
  static LONGLONG Filetime2Int64(const FILETIME &ftime);
  MIB_IFTABLE* m_IfTable = nullptr;             // Network Speed
  DWORD m_LastInBytes = 0, m_LastOutBytes = 0;
  FILETIME m_PreIdleTime { 0 }, m_PreKernelTime { 0 }, m_PreUserTime { 0 };
#endif
  // class QTimer* m_Timer;
  void SetMemInfo();
  void SetNetInfo();
  void SetCpuInfo();
  void FormatSpeed(uint64_t bytes, bool upload);
};

#endif  // NETSPEEDHELPER_H
