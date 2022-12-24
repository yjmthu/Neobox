#include <httplib.h>
#include <stdexcept>
#include <format>
#include <regex>
#include <fstream>
#include <iostream>
#include <string>

#include <windows.h>

using namespace std::literals;

class DateTime {
public:
  uint8_t yea_hi;
  uint8_t yea_lo;
  uint8_t mon;
  uint8_t mday;
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint8_t wday;
private:
  bool is_leap() const noexcept {
    return yea_lo % 4 == 0 && (yea_lo != 0 || yea_hi % 4 == 0);
  }
  uint8_t month_day_count() const;
public:
  void update_days();
  unsigned short year() const { return (int)yea_hi * 100 + yea_lo; }
};

uint8_t DateTime::month_day_count() const {
  switch (mon) {
    case 1: case 3: case 5: case 7: case 8: case 10: case 12:
      return 31;
    case 2:
      return is_leap() ? 29 : 28;
    default:
      return 30;
  }
  return 31;
}

void DateTime::update_days() {
  const uint8_t mon_max = month_day_count();
  if (mday > mon_max) mday = mon_max;
  int16_t week, year = (int16_t)yea_hi * 100 + yea_lo, month = mon;
  if (month <= 2)
    --year, month += 12;
  week = yea_lo + yea_lo / 4 + yea_hi / 4 - 2 * yea_hi + 26 * (month + 1) / 10 + mday - 1;
  week %= 7;
  this->wday = week < 0 ? week + 7: week;
}


// Date:  Sat, 24 Dec 2022 17:19:03 GMT
static DateTime GetDateTime(std::string dateStr)
{
  static const auto strMonths = "JanFebMarAprMayJunJulAugSepOctNovDec"s;
  std::cout << "inital string is {" << dateStr << "} size <" << dateStr.size() << ">" << std::endl;
  std::regex pattern(".*?(\\d{2}) (\\w{3}) (\\d{4}) (\\d{2})\\:(\\d{2})\\:(\\d{2}) GMT");
  std::smatch result;
  if (!std::regex_match(dateStr, result, pattern)) {
    throw std::runtime_error("can't match string!");
  }
  auto year = std::stoul(result.str(3));
  DateTime datetime = DateTime {
    static_cast<uint8_t>(year / 100),
    static_cast<uint8_t>(year % 100),
    static_cast<uint8_t>(strMonths.find(result.str(2)) / 3 + 1),
    static_cast<uint8_t>(std::stoul(result.str(1))),
    static_cast<uint8_t>(std::stoul(result[4])),
    static_cast<uint8_t>(std::stoul(result[5])),
    static_cast<uint8_t>(std::stoul(result[6])),
    uint8_t(0)
  };
  datetime.update_days();
  return datetime;
}

void SetDateTime(const DateTime& datetime)
{
  SYSTEMTIME time {
    datetime.year(),
    datetime.mon,
    datetime.wday,
    datetime.mday,
    datetime.hour,
    datetime.min,
    datetime.sec,
    0
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
        datetime.year(), datetime.mon, datetime.mday, datetime.hour, datetime.min, datetime.sec, datetime.wday);
    SetDateTime(datetime);
    std::cout << "all ok.\n";
  } catch (std::runtime_error err) {
    std::cerr << err.what() << std::endl;
  }
  std::cout << "============End============" << std::endl;
  return 0;
}

