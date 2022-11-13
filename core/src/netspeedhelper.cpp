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
  static constexpr auto units = "BKMGTPN";
  const char *first = units, *last = first + 6;
  double dw = static_cast<double>(bytes);
  while (dw >= 1024) {
    dw /= 1024;
    ++first;
  }
  if (first >= last) {
    dw = 0;
    first = last;
  }
  m_SysInfo[upload ? 0 : 1] = std::vformat(m_StrFmt[upload ? 0 : 1], std::make_format_args(dw, *first));
}

NetSpeedHelper::NetSpeedHelper()
    : m_CpuUse(0), m_MemUse(0), m_RecvBytes(0), m_SendBytes(0) {}

#ifdef _WIN32

NetSpeedHelper::~NetSpeedHelper() {
  HeapFree(GetProcessHeap(), 0, piaa);
  HeapFree(GetProcessHeap(), 0, mi);
}

void NetSpeedHelper::SetMemInfo() {
  static MEMORYSTATUS ms;
  GlobalMemoryStatus(&ms);
  m_MemUse = ms.dwMemoryLoad / 100.0;
  m_SysInfo[2] = std::vformat(m_StrFmt[2], std::make_format_args(ms.dwMemoryLoad));
}

void NetSpeedHelper::SetNetInfo() {
  static unsigned char iGetAddressTime = 10;
  static DWORD m_last_in_bytes = 0, m_last_out_bytes = 0;

  if (iGetAddressTime == 10) {
    DWORD dwIPSize = 0;
    if (GetAdaptersAddresses(AF_INET, 0, 0, piaa, &dwIPSize) ==
        ERROR_BUFFER_OVERFLOW) {
      HeapFree(GetProcessHeap(), 0, piaa);
      piaa = (PIP_ADAPTER_ADDRESSES)HeapAlloc(GetProcessHeap(),
                                              HEAP_ZERO_MEMORY, dwIPSize);
      GetAdaptersAddresses(AF_INET, 0, 0, piaa, &dwIPSize);
    }
    iGetAddressTime = 0;
  } else
    ++iGetAddressTime;

  DWORD dwMISize = 0;
  if (GetIfTable(mi, &dwMISize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
    dwMISize += sizeof(MIB_IFROW) * 2;
    HeapFree(GetProcessHeap(), 0, mi);
    mi = (MIB_IFTABLE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwMISize);
    GetIfTable(mi, &dwMISize, FALSE);
  }
  DWORD m_in_bytes = 0, m_out_bytes = 0;
  PIP_ADAPTER_ADDRESSES paa = piaa;
  auto find_fn = [&paa](const MIB_IFROW& item)->bool{
      return item.dwIndex == paa->IfIndex;
  };
  const auto *table_begin = mi->table, *table_end = table_begin + mi->dwNumEntries;
  while (paa) {
    if (paa->IfType == IF_TYPE_SOFTWARE_LOOPBACK ||
        paa->IfType == IF_TYPE_TUNNEL) {
      paa = paa->Next;
      continue;
    }
    auto iter = std::find_if(table_begin, table_end, find_fn);
    if (iter != table_end) {
      m_in_bytes += iter->dwInOctets;
      m_out_bytes += iter->dwOutOctets;
    }
    paa = paa->Next;
  }

  if (m_last_in_bytes) {
    FormatSpeed(m_out_bytes - m_last_out_bytes, true);
    FormatSpeed(m_in_bytes - m_last_in_bytes, false);
  }
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

void NetSpeedHelper::ClearData() {
  m_RecvBytes = 0;
  m_SendBytes = 0;
}

void NetSpeedHelper::GetSysInfo() {
  SetMemInfo();
  SetNetInfo();
}
