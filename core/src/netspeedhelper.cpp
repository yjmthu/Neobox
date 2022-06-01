#include "netspeedhelper.h"
#include "sysapi.h"

#include <cstdint>
#include <array>
#include <algorithm>
#include <string>
#include <sstream>
#include <list>

using namespace std::literals;

#ifdef _WIN32
#include <iphlpapi.h>
#endif

void formatSpped(std::u8string& str, uint64_t dw, bool up_down)
{
    // https://unicode-table.com/en/2192/
    static char units[] = "\221\223BKMGTPN\342\206\222 %0.1Lf B";
    static char8_t temp[20];
    double DW = static_cast<double>(dw);
    size_t the_unit = 2;
    while (DW >= 1000) {
        DW /= 1024;
        ++the_unit;
    }
    if (the_unit > 8) the_unit = 8;
    units[11] = units[up_down];
    units[20] = units[the_unit];
    sprintf(reinterpret_cast<char*>(temp), units+9, DW);
    str = temp;
}

#ifdef _WIN32

NetSpeedHelper::~NetSpeedHelper()
{
    HeapFree(GetProcessHeap(), 0, piaa);
    HeapFree(GetProcessHeap(), 0, mi);
}

void NetSpeedHelper::SetMemInfo()
{
    static char m_szMemStr[4];
    static MEMORYSTATUS ms;
    GlobalMemoryStatus(&ms);
    sprintf(m_szMemStr, "%d", static_cast<uint16_t>(ms.dwMemoryLoad));
    m_SysInfo[0] = reinterpret_cast<char8_t*>(m_szMemStr);
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

NetSpeedHelper::~NetSpeedHelper()
{
    // delete m_Timer;
}

void NetSpeedHelper::SetMemInfo()
{
    static char8_t m_szMemStr[4];
    std::list<std::string> result;
    GetCmdOutput<char>("free -m", result);
    result.pop_front();
    uint64_t m_NumberBuffer;

    std::array<double, 2> ary{ 0, 0};
    for (auto& str: result)
    {
        std::istringstream ss(str);
        ss >> str;
        for (size_t i=0; i<2; ++i) {
            ss >> m_NumberBuffer;
            ary[i] += m_NumberBuffer;
        }
    }
    sprintf(reinterpret_cast<char*>(m_szMemStr), "%d", static_cast<uint16_t>(ary[1]/ary[0]*100));
    m_SysInfo[0] = m_szMemStr;
}

void NetSpeedHelper::SetNetInfo()
{
    uint64_t recv=0, send=0, buffer;
    std::list<std::string> result;
    GetCmdOutput<char>("cat /proc/net/dev", result);
    result.pop_front();
    result.pop_front();
    for (auto& str: result) {
        std::istringstream ss(str);
        ss >> str;
        for (size_t i=0; i < 9; ++i) {
            ss >> buffer;
            if (i == 0) recv += buffer;
            else if (i == 8) send += buffer;
        }
    }
    if (m_RecvBytes) {
        formatSpped(m_SysInfo[1], send - m_SendBytes, false);
        formatSpped(m_SysInfo[2], recv - m_RecvBytes, true);
    }
    m_RecvBytes = recv;
    m_SendBytes = send;
}

#endif

void NetSpeedHelper::ClearData()
{
    m_RecvBytes = 0;
    m_SendBytes = 0;
}

void NetSpeedHelper::GetSysInfo()
{
    SetMemInfo();
    SetNetInfo();
}
