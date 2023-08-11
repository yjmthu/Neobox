#ifndef TRAFFICINFO_H
#define TRAFFICINFO_H

#include <stdint.h>
#include <array>
#include <string>

class TrafficInfo {
public:
  typedef std::wstring FormatString;
  typedef std::array<FormatString, 6> SpeedUnits;

public:
  uint64_t storageSizeAll = 0;
  uint64_t storageSizeFree = 0;
  uint32_t bytesUp = 0;
  uint32_t bytesDown = 0;
  uint32_t bytesTotalUp = 0;
  uint32_t bytesTotalDown = 0;
  float memUsage = 0;
  float cpuUsage = 0;
  float batteryUsage = 0;
  float storageUsage = 0;
  float cpuTemperature = 0;
  float mainboardTemperature = 0;
  float gpuTemperature = 0;

public:
  static FormatString FormatSpeed(uint32_t bytes,
      const FormatString& fmtStr,
      const SpeedUnits& uints);
};

#endif // TRAFFICINFO_H
