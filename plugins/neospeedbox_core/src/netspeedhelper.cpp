#include "netspeedhelper.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <list>
#include <regex>
#include <string>
#include <map>


NetSpeedHelper::NetSpeedHelper() {
  GetSystemTimes(&m_PreIdleTime, &m_PreKernelTime, &m_PreUserTime);
}

#ifdef _WIN32

NetSpeedHelper::~NetSpeedHelper() {
  HeapFree(GetProcessHeap(), 0, m_IfTable);
}

void NetSpeedHelper::SetMemInfo() {
  static MEMORYSTATUS ms;
  GlobalMemoryStatus(&ms);
  m_TrafficInfo.memUsage = ms.dwMemoryLoad / 100.0f;
  // m_SysInfo[2] = std::vformat(m_StrFmt[2], std::make_format_args(ms.dwMemoryLoad));
}

LONGLONG NetSpeedHelper::Filetime2Int64(const FILETIME &ftime)
{
  // https://blog.csdn.net/alwaysrun/article/details/106433080

  static LARGE_INTEGER li;
  li.LowPart = ftime.dwLowDateTime;
  li.HighPart = ftime.dwHighDateTime;
  return li.QuadPart;
}

void NetSpeedHelper::SetCpuInfo()
{
  static FILETIME idleTime, kernelTime, userTime;

  GetSystemTimes(&idleTime, &kernelTime, &userTime);
  const uint64_t free = Filetime2Int64(idleTime) - Filetime2Int64(m_PreIdleTime);
  const uint64_t all = (Filetime2Int64(kernelTime) - Filetime2Int64(m_PreKernelTime))
    + (Filetime2Int64(userTime) - Filetime2Int64(m_PreUserTime));
  if (all == 0) {
    m_TrafficInfo.cpuUsage = 100;
  } else {
    m_TrafficInfo.cpuUsage = all - free;
    m_TrafficInfo.cpuUsage /= all;
  }
  // m_SysInfo[3] = std::vformat(m_StrFmt[3], std::make_format_args(
    // static_cast<int>(m_CpuUse * 100)));
}

void NetSpeedHelper::UpdateAdaptersAddresses()
{
  
  DWORD dwIPSize = 0;
  if (GetAdaptersAddresses(AF_INET, 0, 0, nullptr, &dwIPSize) != ERROR_BUFFER_OVERFLOW) return;
  
  auto const piaa = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(
    HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwIPSize));
  
  // 检索的地址的地址系列: IPv4
  GetAdaptersAddresses(AF_INET, 0, 0, piaa, &dwIPSize);
  m_Adapters.clear();
  std::string strAdapterName;
  for (auto paa = piaa; paa; paa = paa->Next) {
    if (paa->IfType == IF_TYPE_SOFTWARE_LOOPBACK ||
        paa->IfType == IF_TYPE_TUNNEL) {
      continue;
    }
    strAdapterName = paa->AdapterName;
    bool const enabled = m_AdapterBalckList.find(strAdapterName) == m_AdapterBalckList.end();
    m_Adapters.push_back(IpAdapter {
      std::move(strAdapterName),
      paa->FriendlyName,
      enabled,
      paa->IfIndex
    });
  }

  HeapFree(GetProcessHeap(), 0, piaa);
}

void NetSpeedHelper::SetNetInfo() {

  DWORD dwMISize = 0;
  if (GetIfTable(m_IfTable, &dwMISize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
    dwMISize += sizeof(MIB_IFROW) * 2;
    HeapFree(GetProcessHeap(), 0, m_IfTable);
    m_IfTable = (MIB_IFTABLE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwMISize);
    GetIfTable(m_IfTable, &dwMISize, FALSE);
  }
  DWORD inBytes = 0, outBytes = 0;
  const IpAdapter* ptr;
  auto find_fn = [&ptr](const MIB_IFROW& item)->bool{
      return ptr->enabled && item.dwIndex == ptr->index;
  };

  for (const auto *table_begin = m_IfTable->table, *table_end = table_begin + m_IfTable->dwNumEntries;
    const auto& adpt: m_Adapters) {
    // if (!adpt.enabled) continue;
    ptr = &adpt;               // pass it to find_fn.
    auto pIfRow = std::find_if(table_begin, table_end, find_fn);
    if (pIfRow != table_end) {
      inBytes += pIfRow->dwInOctets;
      outBytes += pIfRow->dwOutOctets;
    }
  }
  
  m_TrafficInfo.bytesUp = outBytes - m_LastOutBytes;
  m_TrafficInfo.bytesDown = inBytes - m_LastInBytes;
  m_LastOutBytes = outBytes;
  m_LastInBytes = inBytes;
}

#elif defined(__linux__)

void NetSpeedHelper::SetMemInfo() {
  using namespace std::literals;
  uint8_t result = 0;
  uint64_t buffer;
  std::string str;
  std::ifstream fs("/proc/meminfo" s);

  std::array<double, 2> ary{0, 0};
  while ((result != 2) && !fs.eof()) {
    fs >> str >> buffer;
    if (str == "MemTotal:" sv) {
      ary[0] += buffer;
      ++result;
    } else if (str == "MemAvailable:" sv) {
      ary[1] += buffer;
      ++result;
    }
    fs >> str;
  }
  fs.close();
  m_MemUse = ary[1] * 100 / array[0];
  std::get<0>(m_SysInfo) = 100 - static_cast<uint64_t>(ary[1] / ary[0] * 100);
}

void NetSpeedHelper::SetNetInfo() {
  uint64_t recv = 0, send = 0, buffer;
  std::ifstream fs("/proc/net/dev" s);
  std::string str;
  std::getline(fs, str);
  std::getline(fs, str);
  while (std::getline(fs, str)) {
    fs >> str;
    for (size_t i = 0; i < 9; ++i) {
      fs >> buffer;
      if (i == 0)
        recv += buffer;
      else if (i == 8)
        send += buffer;
    }
  }
  fs.close();
  if (m_RecvBytes) {
    std::get<1>(m_SysInfo) = send - m_SendBytes;
    std::get<2>(m_SysInfo) = recv - m_RecvBytes;
  }
  m_RecvBytes = recv;
  m_SendBytes = send;
}

#endif

void NetSpeedHelper::GetSysInfo() {
  SetMemInfo();
  SetNetInfo();
  SetCpuInfo();
}
