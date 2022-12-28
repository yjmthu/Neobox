#include <httplib.h>
#include <stdexcept>
#include <format>
#include <regex>
#include <fstream>
#include <iostream>
#include <string>

#include <windows.h>
#include <shlobj_core.h>

using namespace std::literals;

class DateTime {
public:
  uint16_t year;
  uint8_t mon;
  uint8_t mday;
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint8_t wday;
};

// Date:  Sat, 24 Dec 2022 17:19:03 GMT
static DateTime GetDateTime(std::string dateStr)
{
  static const auto strMonths = "JanFebMarAprMayJunJulAugSepOctNovDec"s;
  static const auto strWeeks = "Sun Mon TuesWed ThurFri Sat "s;
  std::cout << "inital string is {" << dateStr << "} size <" << dateStr.size() << ">" << std::endl;
  std::regex pattern(" ?(\\w{3,4}), {1,2}(\\d{1,2}) (\\w{3}) (\\d{4}) (\\d{2})\\:(\\d{2})\\:(\\d{2}) GMT");
  std::smatch result;
  if (!std::regex_match(dateStr, result, pattern)) {
    throw std::runtime_error("can't match string!");
  }
  DateTime datetime = DateTime {
    static_cast<uint16_t>(std::stoul(result.str(4))),
    static_cast<uint8_t>(strMonths.find(result.str(3)) / 3 + 1),
    static_cast<uint8_t>(std::stoul(result.str(2))),
    static_cast<uint8_t>(std::stoul(result[5])),
    static_cast<uint8_t>(std::stoul(result[6])),
    static_cast<uint8_t>(std::stoul(result[7])),
    static_cast<uint8_t>(strWeeks.find(result.str(1)) / 4)
  };
  return datetime;
}

void SetDateTime(const DateTime& datetime)
{
  if (!IsUserAnAdmin())
    throw std::runtime_error("Failed. user is not an admin.");
  SYSTEMTIME time {
    datetime.year,
    datetime.mon,
    datetime.wday,
    datetime.mday,
    datetime.hour,
    datetime.min,
    datetime.sec,
    999
  };

  if (!SetSystemTime(&time)) {
    std::cout << "failed: " << GetLastError() << std::endl;
  }
}


int main()
{
  std::cout << "============Begin============" << std::endl;
  HttpLib clt("https://beijing-time.org/"s);
  auto res = clt.Get();
  try {
    auto datetime = GetDateTime(res->headers["Date"]);
    std::cout << std::format("year: {} month: {} day: {} hour: {} minute: {} second: {} weekday: {}\n",
        datetime.year, datetime.mon, datetime.mday, datetime.hour, datetime.min, datetime.sec, datetime.wday);
    SetDateTime(datetime);
    std::cout << "all ok.\n";
  } catch (std::runtime_error err) {
    std::cerr << err.what() << std::endl;
  } catch (...) {
    std::cerr << "other errors occured." << std::endl;
  }
  std::cout << "============End============" << std::endl;
  // std::getchar();
  return 0;
}

