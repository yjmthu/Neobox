#ifndef NETSPEEDHELPER_H
#define NETSPEEDHELPER_H

#include <string>
#include <array>
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
  void GetSysInfo();
  // std::tuple<int, uint64_t, uint64_t> m_SysInfo;
  std::array<std::string, 4> m_StrFmt;
  std::array<std::string, 4> m_SysInfo;
  float m_CpuUse, m_MemUse;
  void ClearData();


 private:
  uint64_t m_RecvBytes, m_SendBytes;
#ifdef _WIN32
  PIP_ADAPTER_ADDRESSES piaa = nullptr;  // Network Card
  MIB_IFTABLE* mi = nullptr;             // Network Speed
#endif
  class QTimer* m_Timer;
  void SetMemInfo();
  void SetNetInfo();
  void FormatSpeed(uint64_t bytes, bool upload);
};

#endif  // NETSPEEDHELPER_H
