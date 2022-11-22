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

void NetSpeedHelper::FormatSpeed(uint64_t bytes, bool upload) {
  // https://unicode-table.com/en/2192/
  static constexpr auto units = "BKMGTP";
  const char *u = units;
  uint64_t size = 1;
  bytes &= (1ull << 50) - 1;
  while ((bytes >> 10) >= size) {
    size <<= 10;
    ++u;
  }
  m_SysInfo[upload ? 0 : 1] = std::vformat(m_StrFmt[upload ? 0 : 1], std::make_format_args(static_cast<float>(bytes) / size, *u));
}

NetSpeedHelper::NetSpeedHelper()
    : m_CpuUse(0), m_MemUse(0),
      m_StrFmt() {}

#ifdef _WIN32

NetSpeedHelper::~NetSpeedHelper() {
  HeapFree(GetProcessHeap(), 0, pIfTable);
}

void NetSpeedHelper::SetMemInfo() {
  static MEMORYSTATUS ms;
  GlobalMemoryStatus(&ms);
  m_MemUse = ms.dwMemoryLoad / 100.0f;
  m_SysInfo[2] = std::vformat(m_StrFmt[2], std::make_format_args(ms.dwMemoryLoad));
}

void NetSpeedHelper::SetCpuInfo()
{
  static bool bFirstCall = true;
  static FILETIME preIdleTime { 0 }, preKernelTime { 0 }, preUserTime { 0 };
  static FILETIME idleTime, kernelTime, userTime;

  // https://blog.csdn.net/alwaysrun/article/details/106433080
  const auto Filetime2Int64 = [](const FILETIME &ftime)
  {
      LARGE_INTEGER li;
      li.LowPart = ftime.dwLowDateTime;
      li.HighPart = ftime.dwHighDateTime;
      return li.QuadPart;
  };

  GetSystemTimes(&idleTime, &kernelTime, &userTime);
  if (bFirstCall) {
    preIdleTime = idleTime;
    preKernelTime = kernelTime;
    preUserTime = userTime;
    bFirstCall = false;
  } else {
    const uint64_t free = Filetime2Int64(idleTime) - Filetime2Int64(preIdleTime);
    const uint64_t all = (Filetime2Int64(kernelTime) - Filetime2Int64(preKernelTime))
      + (Filetime2Int64(userTime) - Filetime2Int64(preUserTime));
    if (all == 0) {
      m_CpuUse = 100;
    } else {
      m_CpuUse = all - free;
      m_CpuUse /= all;
    }
  }
  m_SysInfo[3] = std::vformat(m_StrFmt[3], std::make_format_args(
    static_cast<int>(m_CpuUse*100)));
}

void NetSpeedHelper::UpdateAdaptersAddresses()
{
  PIP_ADAPTER_ADDRESSES piaa = nullptr;  // Network Card
  DWORD dwIPSize = 0;
  if (GetAdaptersAddresses(AF_INET, 0, 0, piaa, &dwIPSize) ==
      ERROR_BUFFER_OVERFLOW) {
    HeapFree(GetProcessHeap(), 0, piaa);
    piaa = (PIP_ADAPTER_ADDRESSES)HeapAlloc(GetProcessHeap(),
                                            HEAP_ZERO_MEMORY, dwIPSize);
    GetAdaptersAddresses(AF_INET, 0, 0, piaa, &dwIPSize);
  }
  m_Adapters.clear();
  std::string strAdapterName;
  for (auto paa = piaa; paa; paa = paa->Next) {
    if (paa->IfType == IF_TYPE_SOFTWARE_LOOPBACK ||
        paa->IfType == IF_TYPE_TUNNEL) {
      continue;
    }
    strAdapterName = paa->AdapterName;
    bool enabled = m_AdapterBalckList.find(strAdapterName) == m_AdapterBalckList.end();
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
  static DWORD m_last_in_bytes = 0, m_last_out_bytes = 0;

  DWORD dwMISize = 0;
  if (GetIfTable(pIfTable, &dwMISize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
    dwMISize += sizeof(MIB_IFROW) * 2;
    HeapFree(GetProcessHeap(), 0, pIfTable);
    pIfTable = (MIB_IFTABLE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwMISize);
    GetIfTable(pIfTable, &dwMISize, FALSE);
  }
  DWORD m_in_bytes = 0, m_out_bytes = 0;
  const IpAdapter* ptr;
  auto find_fn = [&ptr](const MIB_IFROW& item)->bool{
      return ptr->enabled && item.dwIndex == ptr->index;
  };
  const auto *table_begin = pIfTable->table, *table_end = table_begin + pIfTable->dwNumEntries;
  for (const auto& adpt: m_Adapters){
    // if (!adpt.enabled) continue;
    ptr = &adpt;               // pass it to find_fn.
    auto pIfRow = std::find_if(table_begin, table_end, find_fn);
    if (pIfRow != table_end) {
      m_in_bytes += pIfRow->dwInOctets;
      m_out_bytes += pIfRow->dwOutOctets;
    }
  }
  
  FormatSpeed(m_out_bytes - m_last_out_bytes, true);
  FormatSpeed(m_in_bytes - m_last_in_bytes, false);
  m_last_out_bytes = m_out_bytes;
  m_last_in_bytes = m_in_bytes;
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

void NetSpeedHelper::InitStrings()
{
  FormatSpeed(0, true);
  FormatSpeed(0, false);
  m_SysInfo[2] = std::vformat(m_StrFmt[2], std::make_format_args(0));
  UpdateAdaptersAddresses();
}

void NetSpeedHelper::GetSysInfo() {
  SetMemInfo();
  SetNetInfo();
  SetCpuInfo();
}
