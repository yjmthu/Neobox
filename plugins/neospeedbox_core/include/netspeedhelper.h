#ifndef NETSPEEDHELPER_H
#define NETSPEEDHELPER_H

#include <set>
#include <vector>

#include <trafficinfo.h>

class NetSpeedHelper {
public:
  explicit NetSpeedHelper(const class YJson& balcklist);
#ifdef WIN32
  ~NetSpeedHelper();
#endif

  struct IpAdapter {
    std::u8string adapterName;  // ASCII
#ifdef _WIN32
    std::wstring friendlyName;
    unsigned long index;
#endif
    bool enabled;
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
#endif
  uint64_t m_PreIdleTime { 0 }, m_PreAllTime { 0 };

private:
  void SetMemInfo();
  void SetNetInfo();
  void SetCpuInfo();
  void FormatSpeed(uint64_t bytes, bool upload);
};

#endif  // NETSPEEDHELPER_H
