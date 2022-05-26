#include "netspeedhelper.h"
#include "sysapi.h"

#include "widgets/speedbox.h"

#include <QTimer>
#include <QDebug>

#include <cstdint>
#include <array>
#include <algorithm>
#include <cstring>
#include <string>

#ifdef _WIN32
#include <iphlpapi.h>
#endif

template <typename _Ty=int>
_Ty StringToInt(std::string::const_iterator first, std::string::const_iterator last)
{
    _Ty res = 0;
    if (first == last) return res;
    bool sign = true;
    if (*first == '-') {
        sign = false;
        ++first;
    } else if (*first == '+') {
        ++first;
    }
    while (first != last) {
        res *= 10;
        res += *first - '0';
        ++first;
    }
    return sign?res:-res;
}

void formatSpped(std::string& str, uint64_t dw, bool up_down)
{
    static const char* units[] = {
        u8"↑", u8"↓",
        u8"", u8"K", u8"M", u8"G", u8"T", u8"P", u8"N"
    };
    static char temp[20];
    long double DW = dw;
    size_t the_unit = 2;
    while (DW >= 1000) {
        DW /= 1024;
        ++the_unit;
    }
    if (the_unit > 8) the_unit = 8;
    sprintf(temp, "%s %0.1Lf %sB", units[up_down], DW, units[the_unit]);
    str = temp;
}

#ifdef _WIN32

NetSpeedHelper::NetSpeedHelper(wxWindow* box):
    m_Timer(new wxTimer(box, wxID_ANY))
{
    box->Connect(wxEVT_TIMER, wxTimerEventHandler(SpeedBox::OnTimer), box);
    m_Timer->Start(1000);
}

NetSpeedHelper::~NetSpeedHelper()
{
    m_Timer->Stop();
    HeapFree(GetProcessHeap(), 0, piaa);
    HeapFree(GetProcessHeap(), 0, mi);
}

void NetSpeedHelper::SetMemInfo()
{
    MEMORYSTATUS ms;
    GlobalMemoryStatus(&ms);
    m_SysInfo[0] = wxString::Format("%lu", ms.dwMemoryLoad);
}

void NetSpeedHelper::SetNetInfo()
{
    static unsigned char iGetAddressTime = 10;  // 10秒一次获取网卡信息
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
        ++iGetAddressTime;

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
        while (paa) {
            if (paa->IfType != IF_TYPE_SOFTWARE_LOOPBACK && paa->IfType != IF_TYPE_TUNNEL) {
                if (paa->IfIndex == mi->table[i].dwIndex) {
                    m_in_bytes += mi->table[i].dwInOctets;
                    m_out_bytes += mi->table[i].dwOutOctets;
                }
            }
            paa = paa->Next;
        }
    }

    if (m_last_in_bytes) {
        formatSpped(m_SysInfo[1], m_out_bytes - m_last_out_bytes, false);
        formatSpped(m_SysInfo[2], m_in_bytes - m_last_in_bytes, true);
    }
    m_last_out_bytes = m_out_bytes;
    m_last_in_bytes = m_in_bytes;
}

#elif defined(__linux__)
NetSpeedHelper::NetSpeedHelper(QObject *parent) :
    QObject(parent), m_Timer(new QTimer(this))
{
    connect(m_Timer, &QTimer::timeout, dynamic_cast<SpeedBox*>(parent), &SpeedBox::OnTimer);
    m_Timer->start(1000);                                    //开始检测网速和内存
}

NetSpeedHelper::~NetSpeedHelper()
{
    // delete m_Timer;
}

void NetSpeedHelper::StartTimer()
{
    m_RecvBytes = 0;
    m_SendBytes = 0;
    m_Timer->start();
}

void NetSpeedHelper::StopTimer()
{
    m_Timer->stop();
}

void NetSpeedHelper::SetMemInfo()
{

    std::list<std::string> result;
    GetCmdOutput("free -m", result);
    result.pop_front();

    std::array<double, 2> ary{ 0, 0};
    for (auto& str: result)
    {
        auto left = std::find(str.begin(), str.end(), ':');
        left = std::find_if(left, str.end(), &isdigit);
        std::string::iterator right;
        for (size_t i=0; i<2; ++i) {
            right = std::find_if(left, str.end(), [](char c)->bool{return strchr(" \n\t\r", c);});
            ary[i] += StringToInt<unsigned long long>(left, right);
            left = std::find_if(right, str.end(), &isdigit);
        }
    }

    m_SysInfo[0] = std::to_string(static_cast<unsigned short>(ary[1]/ary[0]*100));
}

void NetSpeedHelper::SetNetInfo()
{
    uint64_t recv=0, send=0;
    std::list<std::string> result;
    GetCmdOutput("cat /proc/net/dev", result);
    result.pop_front();
    result.pop_front();
    for (auto& str: result)          // 这样可以获取所有网卡的数据。
    {
        auto left = std::find(str.begin(), str.end(), ':');
        left = std::find_if(left, str.end(), &isdigit);
        std::string::iterator right;
        for (size_t i=0; i < 9; ++i) {
            right = std::find_if(left, str.end(), [](char c)->bool{return strchr(" \n\t\r", c);});
            if (i == 0) {
                recv += StringToInt<uint64_t>(left, right);
            } else if (i == 8) {
                send += StringToInt<uint64_t>(left, right);
            }
            left = std::find_if(right, str.end(), &isdigit);
        }
    }
    if (m_RecvBytes) {
        formatSpped(m_SysInfo[1], send - m_SendBytes, false);
        formatSpped(m_SysInfo[2], recv - m_RecvBytes, true);
    }
    // std::cout << recv << " " << send << std::endl;
    m_RecvBytes = recv;
    m_SendBytes = send;
}
#endif

void NetSpeedHelper::GetSysInfo()
{
    SetMemInfo();
    SetNetInfo();
}
