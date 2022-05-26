#ifndef NETSPEEDHELPER_H
#define NETSPEEDHELPER_H

#include <QObject>
#include <string>

class NetSpeedHelper: public QObject
{
    Q_OBJECT
public:
    explicit NetSpeedHelper(QObject *parent);
    ~NetSpeedHelper();
    void GetSysInfo();
    std::string m_SysInfo[3] = {u8"0", u8"↑ 0.0B", u8"↓ 0.0B"}; 
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
