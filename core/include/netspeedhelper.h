#ifndef NETSPEEDHELPER_H
#define NETSPEEDHELPER_H

#include <string>
#ifdef _WIN32
#include <Winsock2.h>
#include <Iphlpapi.h>
#include <Windows.h>
#endif

class NetSpeedHelper
{
public:
    ~NetSpeedHelper();
    void GetSysInfo();
    std::u8string m_SysInfo[3] { u8"0", u8"\u2191 0.0 B", u8"\u2193 0.0 B" };
    void ClearData();

private:
    uint64_t m_RecvBytes, m_SendBytes;
#ifdef _WIN32
    PIP_ADAPTER_ADDRESSES piaa = nullptr;   // Network Card
    MIB_IFTABLE *mi = nullptr;              // Network Speed
#endif
    class QTimer *m_Timer;
    void SetMemInfo();
    void SetNetInfo();
};

#endif // NETSPEEDHELPER_H
