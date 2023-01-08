#ifndef NETSPEEDHELPER_H
#define NETSPEEDHELPER_H

#include <set>
#include <vector>
#include <memory>

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
    unsigned long index;
  };
  
  void UpdateAdaptersAddresses();
  void GetSysInfo();

public:
  TrafficInfo m_TrafficInfo;
  std::set<std::u8string_view> m_AdapterBalckList; // ASCII
  std::vector<IpAdapter> m_Adapters;

private:
#ifdef _WIN32
  unsigned char* m_IfTableBuffer;
  unsigned long m_IfTableBufferSize;
  // MIB_IFTABLE* m_IfTable = nullptr;             // Network Speed
  uint64_t m_PreIdleTime { 0 }, m_PreKernelTime { 0 }, m_PreUserTime { 0 };
#endif

private:
  void SetMemInfo();
  void SetNetInfo();
  void SetCpuInfo();
  void FormatSpeed(uint64_t bytes, bool upload);
};

#endif  // NETSPEEDHELPER_H
