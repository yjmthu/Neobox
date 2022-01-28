#include "netspeedhelper.h"

#include <QProcess>
#include <QTimer>
#include <funcbox.h>


inline QString formatSpped(long long dw, bool up_down)
{
    static const char* units[] = { u8"↑", u8"↓", "", "K", "M" };
    long double DW = dw;
    ushort the_unit = 2;
    if (DW >= 1024)
    {
        DW /= 1024;
        the_unit++;
        if (DW >= 1024)
        {
            DW /= 1024;
            the_unit++;
        }
    }
    return  QString("%1 %2 %3B").arg(units[up_down], QString::number(DW, 'f', 1), units[the_unit]);
}

#if defined (Q_OS_WIN32)

NetSpeedHelper::NetSpeedHelper(QObject *parent) :
    QObject(parent), timer(new QTimer), hIphlpapi(LoadLibraryA("iphlpapi.dll"))
{
    connect(timer, &QTimer::timeout, this, [this](){
        get_mem_usage();
        get_net_usage();
    });
    if (hIphlpapi)           //获取两个函数指针
    {
        GetAdaptersAddresses = (pfnGetAdaptersAddresses)GetProcAddress(hIphlpapi, "GetAdaptersAddresses");
        GetIfTable = (pfnGetIfTable)GetProcAddress(hIphlpapi, "GetIfTable");
    }
    timer->start(1000);                                    //开始检测网速和内存
}

NetSpeedHelper::~NetSpeedHelper()
{
    delete timer;
    FreeLibrary(hIphlpapi);
    HeapFree(GetProcessHeap(), 0, piaa);
    HeapFree(GetProcessHeap(), 0, mi);
}

void NetSpeedHelper::get_mem_usage()
{
    MEMORYSTATUS ms;
    GlobalMemoryStatus(&ms);
    emit memInfo(QString::number(ms.dwMemoryLoad));
}

void NetSpeedHelper::get_net_usage()
{
    static unsigned char iGetAddressTime = 10;  //10秒一次获取网卡信息
    static DWORD m_last_in_bytes = 0 /* 总上一秒下载字节 */,  m_last_out_bytes = 0 /* 总上一秒上传字节 */;

    if (iGetAddressTime == 10)        // 10秒更新
    {
        DWORD dwIPSize = 0;
        if (GetAdaptersAddresses(AF_INET, 0, 0, piaa, &dwIPSize) == ERROR_BUFFER_OVERFLOW)
        {
            HeapFree(GetProcessHeap(), 0, piaa);
            piaa = (PIP_ADAPTER_ADDRESSES)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwIPSize);
            GetAdaptersAddresses(AF_INET, 0, 0, piaa, &dwIPSize);
        }
        iGetAddressTime = 0;
    }
    else
        iGetAddressTime++;

    DWORD dwMISize = 0;
    if (GetIfTable(mi, &dwMISize, FALSE) == ERROR_INSUFFICIENT_BUFFER)
    {
        dwMISize += sizeof(MIB_IFROW) * 2;
        HeapFree(GetProcessHeap(), 0, mi);
        mi = (MIB_IFTABLE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwMISize);
        GetIfTable(mi, &dwMISize, FALSE);
    }
    DWORD m_in_bytes = 0, m_out_bytes = 0; PIP_ADAPTER_ADDRESSES paa = nullptr;
    for (DWORD i = 0; i < mi->dwNumEntries; i++)
    {
        paa = &piaa[0];
        while (paa)
        {
            if (paa->IfType != IF_TYPE_SOFTWARE_LOOPBACK && paa->IfType != IF_TYPE_TUNNEL)
            {
                if (paa->IfIndex == mi->table[i].dwIndex)
                {
                    m_in_bytes += mi->table[i].dwInOctets;
                    m_out_bytes += mi->table[i].dwOutOctets;
                }
            }
            paa = paa->Next;
        }
    }

    if (m_last_in_bytes)
    {
        emit netInfo(
            formatSpped(m_out_bytes - m_last_out_bytes, false),
            formatSpped(m_in_bytes - m_last_in_bytes, true)
        );
    }
    m_last_out_bytes = m_out_bytes;
    m_last_in_bytes = m_in_bytes;
}

#elif defined(Q_OS_LINUX)
NetSpeedHelper::NetSpeedHelper(QObject *parent) :
    QObject(parent), timer(new QTimer)
{
    connect(timer, &QTimer::timeout, this, [this](){
        get_mem_usage();
        get_net_usage();
    });
    timer->start(1000);                                    //开始检测网速和内存
}

NetSpeedHelper::~NetSpeedHelper()
{
    delete timer;
}
void NetSpeedHelper::get_mem_usage()
{
    QStringList arguments;
    arguments.append("-m");
    QString str = VarBox->runCmd("free", arguments, 2);
    str.replace("\n","");
    str.replace(QRegExp("( ){1,}")," ");  // 将连续空格替换为单个空格 用于分割
    QStringList lst = str.split(" ");
    emit memInfo(QString::number(int((lst[2].toFloat()/lst[1].toFloat())*100)));
}

void NetSpeedHelper::get_net_usage()
{
    static unsigned long long recv_bytes = 0, send_bytes = 0;
    QProcess process;
    QStringList arguments;
    unsigned long int recv=0, send=0;
    arguments.append("/proc/net/dev");
    process.start("cat", arguments); //读取文件/proc/net/dev获取流量
    process.waitForFinished();
    process.readLine();
    process.readLine();
    while(!process.atEnd())  //这样可以获取所有网卡的数据。
    {
        QString str = process.readLine();
        str.replace("\n","");
        str.replace(QRegExp("( ){1,}")," ");
        QStringList lst = str.split(" ");
        recv += lst[2].toLongLong();
        send += lst[10].toLongLong();
    }
    if (recv_bytes)
    {
        emit netInfo(
            formatSpped(send - send_bytes, false),
            formatSpped(recv - recv_bytes, true)
        );
    }
    recv_bytes = recv;
    send_bytes = send;
}
#endif
