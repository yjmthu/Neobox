#include <winsock2.h>
#include <iphlpapi.h>

#include "netspeedhelper.h"
#include <systemapi.h>
#include <yjson.h>


#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <regex>
#include <string>
#include <map>

NetSpeedHelper::NetSpeedHelper(const YJson& blacklist)
  : m_IfTableBuffer(new unsigned char[sizeof(MIB_IFTABLE)])
  , m_IfTableBufferSize(sizeof(MIB_IFTABLE))
  , m_PreIdleTime(0)
  , m_PreKernelTime(0)
  , m_PreUserTime(0)
{
  for (const auto& item: blacklist.getArray()) {
    m_AdapterBalckList.emplace(item.getValueString());
  }
  UpdateAdaptersAddresses();
  SetNetInfo();
  m_TrafficInfo.bytesUp = m_TrafficInfo.bytesDown = 0;
  SetCpuInfo();
  m_TrafficInfo.cpuUsage = 0;
}

#ifdef _WIN32

NetSpeedHelper::~NetSpeedHelper() {
  delete [] m_IfTableBuffer;
}

void NetSpeedHelper::SetMemInfo() {
  static MEMORYSTATUS ms;
  GlobalMemoryStatus(&ms);
  m_TrafficInfo.memUsage = ms.dwMemoryLoad / 100.0f;
  // m_SysInfo[2] = std::vformat(m_StrFmt[2], std::make_format_args(ms.dwMemoryLoad));
}

uint64_t NetSpeedHelper::Filetime2Int64(const void* ftime)
{
  // https://blog.csdn.net/alwaysrun/article/details/106433080

  static LARGE_INTEGER li;
  auto pointer = reinterpret_cast<const FILETIME*>(ftime);
  li.LowPart = pointer->dwLowDateTime;
  li.HighPart = pointer->dwHighDateTime;
  return li.QuadPart;
}

void NetSpeedHelper::SetCpuInfo()
{
  static FILETIME idleTime, kernelTime, userTime;
  GetSystemTimes(&idleTime, &kernelTime, &userTime);

  static uint64_t curIdleTime, curKernelTime, curUserTime;
  curIdleTime = Filetime2Int64(&idleTime);
  curKernelTime = Filetime2Int64(&kernelTime);
  curUserTime = Filetime2Int64(&userTime);

  const uint64_t all = (curKernelTime - m_PreKernelTime) + (curUserTime - m_PreUserTime);
  if (all == 0) {
    m_TrafficInfo.cpuUsage = 100;
  } else {
    m_TrafficInfo.cpuUsage = all - curIdleTime + m_PreIdleTime;
    m_TrafficInfo.cpuUsage /= all;
  }

  m_PreIdleTime = curIdleTime;
  m_PreKernelTime = curKernelTime;
  m_PreUserTime = curUserTime;
}

void NetSpeedHelper::UpdateAdaptersAddresses()
{
  DWORD dwIPSize = 0;
  // ULONG ulIPSize = 0;
  // PIP_ADAPTER_INFO pAdapterInfo = nullptr;
  // GetAdaptersInfo(pAdapterInfo, &ulIPSize);
  // if (GetAdaptersInfo(pAdapterInfo, &ulIPSize) != ERROR_BUFFER_OVERFLOW) return;
  if (GetAdaptersAddresses(AF_INET, 0, 0, nullptr, &dwIPSize) != ERROR_BUFFER_OVERFLOW) return;
  
  auto const pIpAdpAddress = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(new unsigned char[dwIPSize]);
  // auto const pIpAdpAddress = reinterpret_cast<PIP_ADAPTER_INFO>(new unsigned char[ulIPSize]);
  
  // 检索的地址的地址系列: IPv4
  GetAdaptersAddresses(AF_INET, 0, 0, pIpAdpAddress, &dwIPSize);
  // if (GetAdaptersInfo(pAdapterInfo, &ulIPSize) != NO_ERROR) return;

  m_Adapters.clear();
  std::u8string strAdapterName;
  for (auto pAdapter = pIpAdpAddress; pAdapter; pAdapter = pAdapter->Next) {
    if (pAdapter->IfType == MIB_IF_TYPE_LOOPBACK ||
        pAdapter->IfType == MIB_IF_TYPE_OTHER) {
      continue;
    }
    strAdapterName = reinterpret_cast<const char8_t*>(pAdapter->AdapterName);
    bool const enabled = m_AdapterBalckList.find(strAdapterName) == m_AdapterBalckList.end();
    m_Adapters.push_back(IpAdapter {
      std::move(strAdapterName),
      pAdapter->FriendlyName,
      enabled,
      pAdapter->IfIndex
    });
  }

  delete[] reinterpret_cast<const unsigned char*>(pIpAdpAddress);
}

void NetSpeedHelper::SetNetInfo() {
  static const IpAdapter* ptr;
  
  // static auto ifTableBuffer = std::make_unique_for_overwrite<unsigned char[]>(sizeof(MIB_IFTABLE));
  auto pIfTable = reinterpret_cast<MIB_IFTABLE*>(m_IfTableBuffer);

  // static DWORD dwMISize = sizeof(MIB_IFTABLE);
  if (GetIfTable(pIfTable, &m_IfTableBufferSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
    delete[] m_IfTableBuffer;
    m_IfTableBuffer = new unsigned char[m_IfTableBufferSize];
    pIfTable = reinterpret_cast<MIB_IFTABLE*>(m_IfTableBuffer);
    GetIfTable(pIfTable, &m_IfTableBufferSize, FALSE);
  }

  auto const tableBegin = pIfTable->table;
  auto const tableEnd = tableBegin + pIfTable->dwNumEntries;
  const auto find_fn = std::bind(
      std::find_if<const MIB_IFROW*, bool(*)(const MIB_IFROW& item)>,
      tableBegin, tableEnd,
      [](const MIB_IFROW& item)->bool { return item.dwIndex == ptr->index;}
  );

  DWORD inBytes = 0, outBytes = 0;
  for (const auto& adpt: m_Adapters) {
    if (!adpt.enabled) continue;
    ptr = &adpt;               // pass it to find_fn.
    auto pIfRow = find_fn();
    if (pIfRow != tableEnd) {
      inBytes += pIfRow->dwInOctets;
      outBytes += pIfRow->dwOutOctets;
    }
  }
  
  m_TrafficInfo.bytesUp = outBytes - m_TrafficInfo.bytesTotalUp;
  m_TrafficInfo.bytesDown = inBytes - m_TrafficInfo.bytesTotalDown;
  m_TrafficInfo.bytesTotalUp = outBytes;
  m_TrafficInfo.bytesTotalDown = inBytes;
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
