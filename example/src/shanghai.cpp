#include <iostream>

#include <neobox/httplib.h>
#include <windows.h>

// https://worldtimeapi.org/api/timezone/Asia/Shanghai
/*
{
    "abbreviation": "CST",
    "client_ip": "183.172.215.165",
    "datetime": "2023-12-28T00:23:09.321081+08:00",
    "day_of_week": 4,
    "day_of_year": 362,
    "dst": false,
    "dst_from": null,
    "dst_offset": 0,
    "dst_until": null,
    "raw_offset": 28800,
    "timezone": "Asia/Shanghai",
    "unixtime": 1703694189,
    "utc_datetime": "2023-12-27T16:23:09.321081+00:00",
    "utc_offset": "+08:00",
    "week_number": 52
}
*/

int main() {
  SetConsoleOutputCP(CP_UTF8);
  std::cout << "============Begin============" << std::endl;
  static std::atomic_bool finished = false;
  uint64_t unixtime = 0;
  HttpLib clt(HttpUrl(u8"https://worldtimeapi.org/api/timezone/Asia/Shanghai"sv), true);
  HttpLib::Callback callback {
    // .m_WriteCallback = [](auto data, auto size) {},
    .m_FinishCallback = [&unixtime](auto msg, auto res) {
      if (msg.empty() && res->status == 200) {
        YJson json(res->body.begin(), res->body.end());
        std::cout << json << std::endl;
        auto& timestamp = json[u8"unixtime"].getValueDouble();
        unixtime = static_cast<decltype(unixtime)>(timestamp);
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
  
  if (unixtime++) {
    std::cout << "unixtime: " << unixtime << std::endl;
    // change system time to unixtime
    SYSTEMTIME st;
    FILETIME ft;
    LONGLONG ll;
    ll = Int32x32To64(unixtime, 10000000) + 116444736000000000;
    ft.dwLowDateTime = (DWORD)ll;
    ft.dwHighDateTime = ll >> 32;
    FileTimeToSystemTime(&ft, &st);
    // print st
    std::cout << "year: " << st.wYear << std::endl;
    std::cout << "month: " << st.wMonth << std::endl;
    std::cout << "day: " << st.wDay << std::endl;
    std::cout << "hour: " << st.wHour << std::endl;
    std::cout << "minute: " << st.wMinute << std::endl;
    std::cout << "second: " << st.wSecond << std::endl;
    std::cout << "milliseconds: " << st.wMilliseconds << std::endl;
    SetSystemTime(&st);
  } else {
    std::cout << "failed to get unixtime" << std::endl;
  }

  std::cout << "============End============" << std::endl;
  getchar();
  return 0;
}
