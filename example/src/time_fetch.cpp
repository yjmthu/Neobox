#include <neobox/httplib.h>
#include <stdexcept>
#include <format>
#include <regex>
#include <fstream>
#include <iostream>
#include <string>
#include <memory>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#include <shlobj_core.h>

using namespace std::literals;

static const auto strMonths = "JanFebMarAprMayJunJulAugSepOctNovDec"s;
static const auto strWeeks = "SunMonTueWedThuFriSat"s;


std::ostream& operator<<(std::ostream& o, const std::u8string& data) {
  return o.write(reinterpret_cast<const char*>(data.data()), data.size());
};

// Date:  Sat, 24 Dec 2022 17:19:03 GMT
static auto GetDateTime(std::u8string dateU8Str)
{
  std::string dateStr(dateU8Str.begin(), dateU8Str.end());
  std::cout << "inital string is {" << dateStr << "} size <" << dateStr.size() << ">" << std::endl;
  std::regex pattern("(\\w{3}), {1,2}(\\d{1,2}) (\\w{3}) (\\d{4}) (\\d{2})\\:(\\d{2})\\:(\\d{2}) GMT");
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


void ParseDateTime(const HttpLib::Response& res) {
  try {
    HttpLib::Headers map = res.headers;
    auto date = res.headers.find(u8"Date");
    // for (auto& [i, j]: map) {
    //   std::cout << i << ": " << j << std::endl;
    // }
    if (date == res.headers.end()) {
      throw std::runtime_error("can't find 'Date' in header.");
    }
    auto ptr = GetDateTime(date->second);
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
}

// int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
int main()
{
  std::cout << "============Begin============" << std::endl;
  static std::atomic_bool finished = false;
  HttpLib clt(HttpUrl(u8"https://beijing-time.org/"sv), true);
  HttpLib::Callback callback {
    .m_WriteCallback = [](auto data, auto size) {},
    .m_FinishCallback = [](auto msg, auto res){
      if (msg.empty() && res->status == 200) {
        ParseDateTime(*res);
      }
      finished = true;
    },
  };
  clt.GetAsync(std::move(callback));
  while (!finished) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  
  std::cout << "============End============" << std::endl;
  getchar();
  return 0;
}
#else
int main() {}
#endif
