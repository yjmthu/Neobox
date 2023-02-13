#ifdef __linux__
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <netdb.h>
#else
#include <iphlpapi.h>
#include <winsock2.h>
#endif

#include "netspeedhelper.h"
#include <systemapi.h>
#include <yjson.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <fstream>
#include <functional>
// #include <iostream>
#include <list>
#include <map>
#include <regex>
#include <numeric>
#include <string>

#ifdef _WIN32
const DWORD g_ProcessorCount = []() {
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  return info.dwNumberOfProcessors;
}();
#else
// https://www.linuxhowtos.org/manpages/5/proc.htm
#endif

NetSpeedHelper::NetSpeedHelper(const YJson &blacklist)
#ifdef _WIN32
    : m_IfTableBuffer(new unsigned char[sizeof(MIB_IFTABLE)]),
      m_IfTableBufferSize(sizeof(MIB_IFTABLE)), m_PreIdleTime(0),
      m_PreKernelTime(0), m_PreUserTime(0)
#endif
{
  for (const auto &item : blacklist.getArray()) {
    m_AdapterBalckList.emplace(item.getValueString());
  }
#ifdef _WIN32
  UpdateAdaptersAddresses();
#endif
  SetNetInfo();
  m_TrafficInfo.bytesUp = m_TrafficInfo.bytesDown = 0;
  SetCpuInfo();
  m_TrafficInfo.cpuUsage = 0;
}

#ifdef _WIN32

NetSpeedHelper::~NetSpeedHelper() { delete[] m_IfTableBuffer; }

void NetSpeedHelper::SetMemInfo() {
  static MEMORYSTATUS ms;
  GlobalMemoryStatus(&ms);
  m_TrafficInfo.memUsage = ms.dwMemoryLoad / 100.0f;
  // m_SysInfo[2] = std::vformat(m_StrFmt[2],
  // std::make_format_args(ms.dwMemoryLoad));
}

uint64_t Filetime2Int64(const FILETIME &ftime) {
  // https://blog.csdn.net/alwaysrun/article/details/106433080

  static LARGE_INTEGER li;
  li.LowPart = ftime.dwLowDateTime;
  li.HighPart = ftime.dwHighDateTime;
  return li.QuadPart;
}

void NetSpeedHelper::SetCpuInfo() {
  static FILETIME idleTime, kernelTime, userTime;
  GetSystemTimes(&idleTime, &kernelTime, &userTime);

  static uint64_t curIdleTime, curKernelTime, curUserTime;
  curIdleTime = Filetime2Int64(idleTime);
  curKernelTime = Filetime2Int64(kernelTime);
  curUserTime = Filetime2Int64(userTime);

  const uint64_t all =
      (curKernelTime - m_PreKernelTime) + (curUserTime - m_PreUserTime);
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

void NetSpeedHelper::UpdateAdaptersAddresses() {
  DWORD dwIPSize = 0;
  // ULONG ulIPSize = 0;
  // PIP_ADAPTER_INFO pAdapterInfo = nullptr;
  // GetAdaptersInfo(pAdapterInfo, &ulIPSize);
  // if (GetAdaptersInfo(pAdapterInfo, &ulIPSize) != ERROR_BUFFER_OVERFLOW)
  // return;
  if (GetAdaptersAddresses(AF_INET, 0, 0, nullptr, &dwIPSize) !=
      ERROR_BUFFER_OVERFLOW)
    return;

  auto const pIpAdpAddress =
      reinterpret_cast<PIP_ADAPTER_ADDRESSES>(new unsigned char[dwIPSize]);
  // auto const pIpAdpAddress = reinterpret_cast<PIP_ADAPTER_INFO>(new unsigned
  // char[ulIPSize]);

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
    strAdapterName = reinterpret_cast<const char8_t *>(pAdapter->AdapterName);
    bool const enabled =
        m_AdapterBalckList.find(strAdapterName) == m_AdapterBalckList.end();
    m_Adapters.push_back(IpAdapter{std::move(strAdapterName),
                                   pAdapter->FriendlyName, enabled,
                                   pAdapter->IfIndex});
  }

  delete[] reinterpret_cast<const unsigned char *>(pIpAdpAddress);
}

void NetSpeedHelper::SetNetInfo() {
  static const IpAdapter *ptr;

  // static auto ifTableBuffer = std::make_unique_for_overwrite<unsigned
  // char[]>(sizeof(MIB_IFTABLE));
  auto pIfTable = reinterpret_cast<MIB_IFTABLE *>(m_IfTableBuffer);

  // static DWORD dwMISize = sizeof(MIB_IFTABLE);
  if (GetIfTable(pIfTable, &m_IfTableBufferSize, FALSE) ==
      ERROR_INSUFFICIENT_BUFFER) {
    delete[] m_IfTableBuffer;
    m_IfTableBuffer = new unsigned char[m_IfTableBufferSize];
    pIfTable = reinterpret_cast<MIB_IFTABLE *>(m_IfTableBuffer);
    GetIfTable(pIfTable, &m_IfTableBufferSize, FALSE);
  }

  auto const tableBegin = pIfTable->table;
  auto const tableEnd = tableBegin + pIfTable->dwNumEntries;
  const auto find_fn = std::bind(
      std::find_if<const MIB_IFROW *, bool (*)(const MIB_IFROW &item)>,
      tableBegin, tableEnd,
      [](const MIB_IFROW &item) -> bool { return item.dwIndex == ptr->index; });

  DWORD inBytes = 0, outBytes = 0;
  for (const auto &adpt : m_Adapters) {
    if (!adpt.enabled)
      continue;
    ptr = &adpt; // pass it to find_fn.
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
// https://github.com/doleron/cpp-linux-system-stats

void NetSpeedHelper::SetMemInfo() {
  uint8_t result = 0;
  uint64_t buffer;
  std::string str;
  std::ifstream fs("/proc/meminfo", std::ios::in);

  float ary[2] { 0, 0 };
  while ((result != 2) && !fs.eof()) {
    fs >> str >> buffer;
    if (str == "MemTotal:") {
      ary[0] = buffer;
      ++result;
    } else if (str == "MemAvailable:") {
      ary[1] = buffer;
      ++result;
    }
    fs >> str;
  }
  fs.close();
  m_TrafficInfo.memUsage = 1.0f - ary[1] / ary[0];
  // std::get<0>(m_SysInfo) = 100 - static_cast<uint64_t>(ary[1] / ary[0] * 100);
}

union CpuTimes {
  struct {
    uint64_t userTime;
    uint64_t niceTime;
    uint64_t systemTime;
    uint64_t idleTime;
    uint64_t iowaitTime;
    uint64_t irqTime;
    uint64_t softirqTime;
    uint64_t stealTime;
    uint64_t guestTime;
    uint64_t guestNiceTime;
  };
  const uint64_t array[10];
};

void NetSpeedHelper::SetCpuInfo() {
  std::string name;
  CpuTimes curTimes;

  std::ifstream fs("/proc/stat", std::ios::in);
  fs >> name >> curTimes.userTime >> curTimes.niceTime >> curTimes.systemTime;
  fs >> curTimes.idleTime >> curTimes.iowaitTime >> curTimes.irqTime >> curTimes.softirqTime;
  fs >> curTimes.stealTime >> curTimes.guestTime >> curTimes.guestNiceTime;
  fs.close();

  uint64_t idleTime = std::accumulate<const uint64_t*>(
    &curTimes.idleTime, curTimes.array + 10, 0ULL);
  uint64_t allTime = std::accumulate<const uint64_t*>(
    curTimes.array, &curTimes.idleTime, idleTime);

  if (allTime == m_PreAllTime) {
    m_TrafficInfo.cpuUsage = 0;
  } else {
    m_TrafficInfo.cpuUsage = idleTime - m_PreIdleTime;
    m_TrafficInfo.cpuUsage = 1.0f - m_TrafficInfo.cpuUsage / (allTime - m_PreAllTime);
  }

  m_PreIdleTime = idleTime;
  m_PreAllTime = allTime;
}

void NetSpeedHelper::SetNetInfo() {
  // std::ifstream("/proc/net/dev");

  // https://stackoverflow.com/questions/37099016/how-to-get-network-device-stats
  struct ifaddrs *ifaddr, *ifa;
  // int family, s, n;
  // char host[NI_MAXHOST];

  if (getifaddrs(&ifaddr) == -1) {
    std::cerr << "getifaddrs\n";
    return;
  }

  uint32_t inBytes = 0, outBytes = 0;

  /* Walk through linked list, maintaining head pointer so we
     can free list later */

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == nullptr)
      continue;

    if (ifa->ifa_addr->sa_family == AF_PACKET && ifa->ifa_data) {
      std::u8string_view name = reinterpret_cast<const char8_t*>(ifa->ifa_name);
      if (m_AdapterBalckList.find(name) != m_AdapterBalckList.end()) {
        continue;
      }
      auto const stats = reinterpret_cast<rtnl_link_stats *>(ifa->ifa_data);
      inBytes += stats->rx_bytes;
      outBytes += stats->tx_bytes;
    }
  }

  freeifaddrs(ifaddr);

  m_TrafficInfo.bytesUp = outBytes - m_TrafficInfo.bytesTotalUp;
  m_TrafficInfo.bytesDown = inBytes - m_TrafficInfo.bytesTotalDown;
  m_TrafficInfo.bytesTotalUp = outBytes;
  m_TrafficInfo.bytesTotalDown = inBytes;
}

#endif

void NetSpeedHelper::GetSysInfo() {
  SetMemInfo();
  SetNetInfo();
  SetCpuInfo();
}
