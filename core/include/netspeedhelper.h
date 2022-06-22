#ifndef NETSPEEDHELPER_H
#define NETSPEEDHELPER_H

#include <string>
#include <tuple>
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
  std::tuple<int, uint64_t, uint64_t> m_SysInfo; //[3]{u8"0", u8"\u2191 0.0 B", u8"\u2193 0.0 B"};
  void ClearData();

 private:
  uint64_t m_RecvBytes, m_SendBytes;
#ifdef _WIN32
  PIP_ADAPTER_ADDRESSES piaa = nullptr;  // Network Card
  MIB_IFTABLE *mi = nullptr;             // Network Speed
#endif
  class QTimer *m_Timer;
  void SetMemInfo();
  void SetNetInfo();
};

#endif  // NETSPEEDHELPER_H
