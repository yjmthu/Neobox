#ifndef NETSPEEDHELPER_H
#define NETSPEEDHELPER_H

#include <QObject>
#ifdef Q_OS_WIN
#include <Winsock2.h>
#include <Iphlpapi.h>
#include <Windows.h>
#elif def Q_OS_LINUX
#include <winddi.h>
#endif

class QTimer;

class NetSpeedHelper : public QObject
{
    Q_OBJECT
public:
    explicit NetSpeedHelper(QObject *parent = nullptr);
    ~NetSpeedHelper();

private:
    QTimer *timer { nullptr };
    void get_mem_usage();                        //读取内存占用率
    void get_net_usage();                        //读取网速
#ifdef Q_OS_WIN
    HMODULE hIphlpapi = NULL;
    typedef ULONG (WINAPI* pfnGetAdaptersAddresses) (ULONG Family, ULONG Flags, PVOID Reserved, PIP_ADAPTER_ADDRESSES AdapterAddresses, PULONG SizePointer);
    typedef DWORD (WINAPI *pfnGetIfTable) (PMIB_IFTABLE pIfTable, PULONG pdwSize, BOOL bOrder);
    pfnGetIfTable GetIfTable = nullptr;
    pfnGetAdaptersAddresses GetAdaptersAddresses = nullptr;
    PIP_ADAPTER_ADDRESSES piaa = nullptr;   //网卡结构
    MIB_IFTABLE *mi = nullptr;              //网速结构
#endif
signals:
    void netInfo(QString net_up, QString net_dw);
    void memInfo(QString mem);
};

#endif // NETSPEEDHELPER_H
