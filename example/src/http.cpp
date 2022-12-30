#include <httplib.h>
#include <stdexcept>
#include <format>
#include <regex>
#include <fstream>
#include <iostream>
#include <string>
#include <memory>

#include <windows.h>
#include <shlobj_core.h>

using namespace std::literals;

static const auto strMonths = "JanFebMarAprMayJunJulAugSepOctNovDec"s;
static const auto strWeeks = "SunMonTueWedThuFriSat"s;

// Date:  Sat, 24 Dec 2022 17:19:03 GMT
static auto GetDateTime(std::string dateStr)
{
  std::cout << "inital string is {" << dateStr << "} size <" << dateStr.size() << ">" << std::endl;
  std::regex pattern(" (\\w{3}), {1,2}(\\d{1,2}) (\\w{3}) (\\d{4}) (\\d{2})\\:(\\d{2})\\:(\\d{2}) GMT");
  std::smatch result;
  if (!std::regex_match(dateStr, result, pattern)) {
    throw std::runtime_error("can't match string!");
  }
  auto const datetime = new SYSTEMTIME {
    static_cast<unsigned short>(std::stoul(result.str(4))),  // year
    static_cast<unsigned short>(strMonths.find(result.str(3)) / 3 + 1),  // month
    static_cast<unsigned short>(strWeeks.find(result.str(1)) / 3), // wday
    static_cast<unsigned short>(std::stoul(result.str(2))),  // mday
    static_cast<unsigned short>(std::stoul(result[5])),  // hour
    static_cast<unsigned short>(std::stoul(result[6])),  // minute
    static_cast<unsigned short>(std::stoul(result[7])),  // second
    999,
  };
  return std::unique_ptr<SYSTEMTIME>(datetime);
}

void SetDateTime(SYSTEMTIME& datetime)
{
  if (!IsUserAnAdmin())
    throw std::runtime_error("Failed. user is not an admin.");

  if (!SetSystemTime(&datetime)) {
    std::cout << "failed: " << GetLastError() << std::endl;
  }
}


int main()
{
  std::cout << "============Begin============" << std::endl;
  HttpLib clt("https://beijing-time.org/"s);
  auto res = clt.Get();
  try {
    auto ptr = GetDateTime(res->headers["Date"]);
    auto& datetime = *ptr;
    std::cout << std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02} {}\n",
        datetime.wYear, datetime.wMonth, datetime.wDay,
        datetime.wHour, datetime.wMinute, datetime.wSecond,
        strWeeks.substr(datetime.wDayOfWeek * 3, 3));
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

