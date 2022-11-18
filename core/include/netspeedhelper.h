#ifndef NETSPEEDHELPER_H
#define NETSPEEDHELPER_H

#include <set>
#include <string>
#include <array>
#include <vector>
#ifdef _WIN32
#include <Winsock2.h>
// #include <Windows.h>
#include <Iphlpapi.h>
#endif

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
  std::array<std::string, 4> m_StrFmt;      // UTF8
  std::array<std::string, 4> m_SysInfo;     // UTF8
  float m_CpuUse, m_MemUse;
  void ClearData();
  void InitStrings();

 private:
  // uint64_t m_RecvBytes, m_SendBytes;
#ifdef _WIN32
  MIB_IFTABLE* pIfTable = nullptr;             // Network Speed
#endif
  class QTimer* m_Timer;
  void SetMemInfo();
  void SetNetInfo();
  void SetCpuInfo();
  void FormatSpeed(uint64_t bytes, bool upload);
};

#endif  // NETSPEEDHELPER_H
