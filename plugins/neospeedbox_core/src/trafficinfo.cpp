#include <trafficinfo.h>
#include <format>

std::string TrafficInfo::FormatSpeed(uint32_t bytes, const std::string& fmtStr, const std::array<std::string, 6> & uints)
{
  // https://unicode-table.com/en/2192/
  auto iter = uints.cbegin();
  uint64_t size = 1;
  for (auto const kb = (bytes >> 10); size < kb; ++iter) { size <<= 10; }

  return std::vformat(fmtStr, std::make_format_args(
    static_cast<float>(bytes) / size, *iter
  ));
  // m_SysInfo[upload ? 0 : 1] = std::vformat(m_StrFmt[upload ? 0 : 1], std::make_format_args(static_cast<float>(bytes) / size, *u));
}

