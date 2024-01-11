#include <neobox/httplib.h>
#include <fstream>
#include <timeapi.hpp>
#include <windows.h>

#include <regex>

namespace chrono = std::chrono;

// #define USE_TIME_API
#define USE_TAOBAO
// #define USE_WORLD_TIME_API
// #define USE_BEIJING_TIME

#ifdef USE_TIME_API
#define TIME_API_URL u8"https://www.timeapi.io/api/Time/current/zone?timeZone=UTC"sv

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

SYSTEMTIME TimeApiCb(const HttpLib::Response& res) {
  YJson json(res.body.begin(), res.body.end());
  return SYSTEMTIME {
    static_cast<unsigned short>(json[u8"year"].getValueInt()),
    static_cast<unsigned short>(json[u8"month"].getValueInt()),
    static_cast<unsigned short>(GetWeekNumber(json[u8"dayOfWeek"].getValueString())), // wday
    static_cast<unsigned short>(json[u8"day"].getValueInt()),
    static_cast<unsigned short>(json[u8"hour"].getValueInt()),
    static_cast<unsigned short>(json[u8"minute"].getValueInt()),
    static_cast<unsigned short>(json[u8"seconds"].getValueInt()),
    static_cast<unsigned short>(json[u8"milliSeconds"].getValueInt()),
  };
}
#define API_CALLBACK TimeApiCb
#define API_URL TIME_API_URL
#endif

#ifdef USE_TAOBAO
#define TAOBAO_URL u8"http://api.m.taobao.com/rest/api3.do?api=mtop.common.getTimestamp"sv

/*
{
  "api": "mtop.common.getTimestamp",
  "v": "*",
  "ret": [ "SUCCESS::接口调用成功" ],
  "data": { "t": "1703767130951" }
}
*/

SYSTEMTIME* TaobaoCb(const HttpLib::Response& res) {
  YJson json(res.body.begin(), res.body.end());
  auto& timestamp = json[u8"data"][u8"t"].getValueString();
  auto timestampView = std::string(timestamp.begin(), timestamp.end());
  const chrono::milliseconds ms(std::stoull(timestampView));
  chrono::system_clock::time_point tp(ms - 8h);

  // print yyyy-MM-dd HH:mm:ss
  std::time_t tt = chrono::system_clock::to_time_t(tp);
  std::tm t;
#ifdef _WIN32
  localtime_s(&t, &tt);
#else
  localtime_r(&tt, &t);
#endif
  return new SYSTEMTIME {
    .wYear = static_cast<unsigned short>(t.tm_year + 1900),
    .wMonth = static_cast<unsigned short>(t.tm_mon + 1),
    .wDayOfWeek = static_cast<unsigned short>(t.tm_wday),
    .wDay = static_cast<unsigned short>(t.tm_mday),
    .wHour = static_cast<unsigned short>(t.tm_hour),
    .wMinute = static_cast<unsigned short>(t.tm_min),
    .wSecond = static_cast<unsigned short>(t.tm_sec),
    .wMilliseconds = static_cast<unsigned short>(ms.count() % 1000),
  };
}

#define API_CALLBACK TaobaoCb
#define API_URL TAOBAO_URL
#endif

#ifdef USE_WORLD_TIME_API
#define WORLD_TIME_API_URL u8"https://worldtimeapi.org/api/timezone/Asia/Shanghai"sv

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

SYSTEMTIME WorldTimeApiCb(const HttpLib::Response& res) {
  YJson json(res.body.begin(), res.body.end());
  auto& timestamp = json[u8"unixtime"].getValueDouble();
  uint64_t unixtime = static_cast<decltype(unixtime)>(timestamp);
  SYSTEMTIME st;
  FILETIME ft;
  LONGLONG ll;
  ll = Int32x32To64(unixtime, 10000000) + 116444736000000000;
  ft.dwLowDateTime = (DWORD)ll;
  ft.dwHighDateTime = ll >> 32;
  FileTimeToSystemTime(&ft, &st);
  return st;
}

#define API_CALLBACK WorldTimeApiCb
#define API_URL WORLD_TIME_API_URL
#endif

#ifdef USE_BEIJING_TIME
#define BEIJING_TIME_URL u8"https://beijing-time.org/"sv

SYSTEMTIME* GetBeijingTimeCb(const HttpLib::Response& res) {
  const auto& headers = res.headers;
  auto date = headers.find(u8"Date");
  if (date == headers.end()) {
    throw std::runtime_error("can't find 'Date' in header.");
  }
  std::smatch result;
  std::regex pattern("(\\w{3}), {1,2}(\\d{1,2}) (\\w{3}) (\\d{4}) (\\d{2})\\:(\\d{2})\\:(\\d{2}) GMT");
  auto dateString = std::string(date->second.begin(), date->second.end());
  if (!std::regex_match(dateString, result, pattern)) {
    throw std::runtime_error("can't match string!");
  }
  return new SYSTEMTIME {
    static_cast<unsigned short>(std::stoul(result.str(4))),  // year
    static_cast<unsigned short>(GetMonthNumber3(result.str(3))),  // month
    static_cast<unsigned short>(GetWeekNumber3(result.str(1))), // wday
    static_cast<unsigned short>(std::stoul(result.str(2))),  // mday
    static_cast<unsigned short>(std::stoul(result[5])),  // hour
    static_cast<unsigned short>(std::stoul(result[6])),  // minute
    static_cast<unsigned short>(std::stoul(result[7])),  // second
    999,
  };
}

#define API_CALLBACK GetBeijingTimeCb
#define API_URL BEIJING_TIME_URL
#endif

void PrintSystemTime(const SYSTEMTIME& st) {
  std::cout << std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02} {}\n",
      st.wYear, st.wMonth, st.wDay,
      st.wHour, st.wMinute, st.wSecond,
      GetWeekDay3<std::string>(st.wDayOfWeek));
}

std::filesystem::path GetLogFilePath() {
  std::wstring exeFilePath(MAX_PATH + 1, 0);
  GetModuleFileNameW(nullptr, exeFilePath.data(), exeFilePath.size());
  exeFilePath.resize(exeFilePath.find_last_of(L"\\") + 1);
  std::filesystem::path logFilePath = exeFilePath;
  logFilePath /= L"timeapi.log";
  return logFilePath;
}

inline uint64_t GetTotalDays() {
  auto tpDay = chrono::time_point_cast<chrono::days>(chrono::system_clock::now());
  return tpDay.time_since_epoch().count();
}

bool CheckLogDate(const std::filesystem::path& filePath, uint64_t totalDays, int delta) {
  std::ifstream fileIn(filePath, std::ios::in);
  if (fileIn.is_open()) {
    uint64_t oldDays;
    fileIn >> oldDays;
    if (totalDays < oldDays || totalDays - oldDays < delta) {
      return false;
    }
    fileIn.close();
  }
  return true;
}

void UpdateLogDate(const std::filesystem::path& filePath, uint64_t totalDays) {
  std::ofstream fileOut(filePath, std::ios::out);
  if (fileOut.is_open()) {
    fileOut << totalDays << std::endl;
    fileOut.close();
  }
}

bool GetNetworkTime(HttpUrl url, std::function<SYSTEMTIME* (const HttpLib::Response&)> callback) {
  static std::atomic_bool finished = false;
  bool success = false;
  HttpLib clt(std::move(url), true);
  HttpLib::Callback cb {
    .m_FinishCallback = [&callback, &success](auto msg, auto res) {
      if (msg.empty() && res->status == 200) {
        auto const current = callback(*res);
        if (current) {
          success = true;
          SetSystemTime(current);
          delete current;
        }
      }
      finished = true;
    },
  };
  clt.GetAsync(std::move(cb));
  while (!finished) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return success;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
  SetConsoleOutputCP(CP_UTF8);

  if (lpCmdLine) {
    // check if lpCmdLine is interger
    const std::string delayStr = lpCmdLine;
    if (delayStr.find_first_not_of("0123456789") != std::string::npos) {
      // std::cout << "invalid delay: " << lpCmdLine << std::endl;
    } else {
      // std::cout << "delay: " << delayStr << std::endl;
      int delay = std::stoi(delayStr);
      std::this_thread::sleep_for(std::chrono::seconds(delay));
    }
  }

  const auto logFilePath { GetLogFilePath() };
  const auto totalDays { GetTotalDays() };
  if (!CheckLogDate(logFilePath, totalDays, 3)) return 0;

  // std::cout << "============Begin============" << std::endl;
  HttpUrl url(API_URL);

  if (GetNetworkTime(std::move(url), API_CALLBACK)) {
    UpdateLogDate(logFilePath, totalDays);
    MessageBoxW(nullptr, L"set current time success", L"Success", MB_OK);
  } else {
    MessageBoxW(nullptr, L"failed to get current time", L"Error", MB_OK);
  }

  std::this_thread::sleep_for(1s);

  return 0;
}
