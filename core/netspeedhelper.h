#ifndef NETSPEEDHELPER_H
#define NETSPEEDHELPER_H

#include <QObject>
#include <string>
#ifdef _WIN32
// #include <Windows.h>
#include <winsock2.h>
#include <iphlpapi.h> // Include for PIP_ADAPTER_ADDRESSES
#endif

class NetSpeedHelper: public QObject
{
    Q_OBJECT
public:
    explicit NetSpeedHelper(QObject *parent);
    ~NetSpeedHelper();
    void GetSysInfo();
    std::u8string m_SysInfo[3] = { u8"0", u8"↑ 0.0B", u8"↓ 0.0B"}; 
    void StartTimer();
    void StopTimer();

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
