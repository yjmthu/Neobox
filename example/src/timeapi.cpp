#include <neobox/httplib.h>
#include <iostream>
#include <array>
#include <windows.h>

// https://www.timeapi.io/api/Time/current/zone?timeZone=UTC

/*
{
    "year": 2023,
    "month": 12,
    "day": 28,
    "hour": 11,
    "minute": 22,
    "seconds": 50,
    "milliSeconds": 360,
    "dateTime": "2023-12-28T11:22:50.360864",
    "date": "12/28/2023",
    "time": "11:22",
    "timeZone": "Asia/Shanghai",
    "dayOfWeek": "Thursday",
    "dstActive": false
}
*/

const std::array strWeeks = {
  u8"Sunday", u8"Monday", u8"Tuesday", u8"Wednesday", u8"Thursday", u8"Friday", u8"Saturday"
};

int main() {
  SetConsoleOutputCP(CP_UTF8);

  std::cout << "============Begin============" << std::endl;
  std::unique_ptr<SYSTEMTIME> current;
  static std::atomic_bool finished = false;
  HttpLib clt(HttpUrl(u8"https://www.timeapi.io/api/Time/current/zone?timeZone=UTC"sv), true);
  HttpLib::Callback callback {
    // .m_WriteCallback = [](auto data, auto size) {},
    .m_FinishCallback = [&current](auto msg, auto res) {
      if (msg.empty() && res->status == 200) {
        YJson json(res->body.begin(), res->body.end());
        current.reset(new SYSTEMTIME {
          static_cast<unsigned short>(json[u8"year"].getValueInt()),
          static_cast<unsigned short>(json[u8"month"].getValueInt()),
          static_cast<unsigned short>(std::find(strWeeks.begin(), strWeeks.end(), json[u8"dayOfWeek"].getValueString()) - strWeeks.begin()), // wday
          static_cast<unsigned short>(json[u8"day"].getValueInt()),
          static_cast<unsigned short>(json[u8"hour"].getValueInt()),
          static_cast<unsigned short>(json[u8"minute"].getValueInt()),
          static_cast<unsigned short>(json[u8"seconds"].getValueInt()),
          static_cast<unsigned short>(json[u8"milliSeconds"].getValueInt()),
        });
      } else {
        std::cout << "failed: " << std::endl;
      }
      finished = true;
    },
  };
  clt.GetAsync(std::move(callback));
  while (!finished) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  if (current) {
    std::cout << "year: " << current->wYear << std::endl;
    std::cout << "month: " << current->wMonth << std::endl;
    std::cout << "day: " << current->wDay << std::endl;
    std::cout << "hour: " << current->wHour << std::endl;
    std::cout << "minute: " << current->wMinute << std::endl;
    std::cout << "second: " << current->wSecond << std::endl;
    std::cout << "milliseconds: " << current->wMilliseconds << std::endl;
    SetSystemTime(current.get());
  } else {
    std::cout << "failed to get current time" << std::endl;
  }

  std::cout << "============End============" << std::endl;

  getchar();

  return 0;
}
