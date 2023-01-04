#ifndef NETSPEEDHELPER_H
#define NETSPEEDHELPER_H

#include <set>
#include <vector>
#include <memory>
#ifdef _WIN32
#include <Winsock2.h>
// #include <Windows.h>
#include <Iphlpapi.h>
#endif

#include <trafficinfo.h>

class NetSpeedHelper {
 public:
  explicit NetSpeedHelper(const class YJson& balcklist);
#ifdef WIN32
  ~NetSpeedHelper();
#endif

  struct IpAdapter {
    std::u8string adapterName;  // ASCII
    std::wstring friendlyName;
    bool enabled;
    IF_INDEX index;
  };
  
  void UpdateAdaptersAddresses();
  void GetSysInfo();
  std::set<std::u8string_view> m_AdapterBalckList; // ASCII
  std::vector<IpAdapter> m_Adapters;
  TrafficInfo m_TrafficInfo;

 private:
#ifdef _WIN32
  unsigned char* m_IfTableBuffer;
  DWORD m_IfTableBufferSize;
  static LONGLONG Filetime2Int64(const FILETIME &ftime);
  // MIB_IFTABLE* m_IfTable = nullptr;             // Network Speed
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
