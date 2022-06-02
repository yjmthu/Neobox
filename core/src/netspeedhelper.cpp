#include "netspeedhelper.h"
#include "sysapi.h"

#include <cstdint>
#include <array>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <regex>
#include <list>
#include <iostream>

using namespace std::literals;

#ifdef _WIN32
#include <iphlpapi.h>
#endif

constexpr std::string __get__(bool i) {
    return i ? "\342\206\223"s: "\342\206\221"s;
} 

template <bool A>
void formatSpped(std::u8string& str, double dw)
{
    // https://unicode-table.com/en/2192/
    using namespace std::literals;
    constexpr std::string_view prex = A? "\342\206\223 "sv: "\342\206\223 "sv;
    constexpr std::string_view units = "BKMGTPN"sv;
    std::string_view::const_iterator iter = units.cbegin();
    while (dw >= 1000) {
        dw /= 1024;
        ++iter;
    }
    if (iter == units.end()) iter = units.end() - 1;
    std::ostringstream ss;
    ss.precision(1);
    ss << prex << std::fixed << dw << ' ' << *iter;
    std::string&& temp = ss.str();
    str.assign(temp.begin(), temp.end());
}

NetSpeedHelper::NetSpeedHelper()
    : m_RecvBytes(0)
    , m_SendBytes(0)
{
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
    static unsigned char iGetAddressTime = 10;
    static DWORD m_last_in_bytes = 0,  m_last_out_bytes = 0;

    if (iGetAddressTime == 10) {
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
        formatSpped<false>(m_SysInfo[1], m_out_bytes - m_last_out_bytes);
        formatSpped<true>(m_SysInfo[2], m_in_bytes - m_last_in_bytes);
    }
    m_last_out_bytes = m_out_bytes;
    m_last_in_bytes = m_in_bytes;
}

#elif defined(__linux__)

void NetSpeedHelper::SetMemInfo()
{
    using namespace std::literals;
    uint8_t result = 0;
    uint64_t buffer;
    std::string str;
    std::ifstream fs("/proc/meminfo"s);

    std::array<double, 2> ary{ 0, 0};
    while ((result != 2) && !fs.eof()) {
        fs >> str >> buffer;
        if (str == "MemTotal:"sv) {
            ary[0] += buffer;
            ++result;
        } else if (str == "MemAvailable:"sv) {
            ary[1] += buffer;
            ++result;
        }
        fs >> str;
    }
    fs.close();
    std::string&& temp = std::to_string(100 - static_cast<uint8_t>(ary[1]/ary[0]*100));
    m_SysInfo[0].assign(temp.begin(), temp.end());
}

void NetSpeedHelper::SetNetInfo()
{
    uint64_t recv=0, send=0, buffer;
    std::ifstream fs("/proc/net/dev"s);
    std::string str;
    std::getline(fs, str);
    std::getline(fs, str);
    while (std::getline(fs, str)) {
        fs >> str;
        for (size_t i=0; i < 9; ++i) {
            fs >> buffer;
            if (i == 0) recv += buffer;
            else if (i == 8) send += buffer;
        }
    }
    fs.close();
    if (m_RecvBytes) {
        formatSpped<false>(m_SysInfo[1], send - m_SendBytes);
        formatSpped<true>(m_SysInfo[2], recv - m_RecvBytes);
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
