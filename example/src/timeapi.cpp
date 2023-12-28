#include <neobox/httplib.h>
#include <fstream>
#include <array>
#include <windows.h>

namespace chrono = std::chrono;

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

  std::wstring exeFilePath(MAX_PATH + 1, 0);
  GetModuleFileNameW(nullptr, exeFilePath.data(), exeFilePath.size());
  exeFilePath.resize(exeFilePath.find_last_of(L"\\") + 1);
  std::filesystem::path logFilePath = exeFilePath;
  logFilePath /= L"timeapi.log";

  auto tpDay = chrono::time_point_cast<chrono::days>(chrono::system_clock::now());
  const auto totalDays = tpDay.time_since_epoch().count();
  std::ifstream fileIn(logFilePath, std::ios::in);
  if (fileIn.is_open()) {
    uint64_t oldDays;
    fileIn >> oldDays;
    if (totalDays < oldDays || totalDays - oldDays < 6) {
      return 0;
    }
    fileIn.close();
  }

  // std::cout << "============Begin============" << std::endl;
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
      }
      finished = true;
    },
  };
  clt.GetAsync(std::move(callback));
  while (!finished) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  if (current) {
    SetSystemTime(current.get());
    std::ofstream fileOut(logFilePath, std::ios::out);
    if (fileOut.is_open()) {
      fileOut << totalDays << std::endl;
      fileOut.close();
    }
    MessageBoxW(nullptr, L"set current time success", L"Success", MB_OK);

  } else {
    MessageBoxW(nullptr, L"failed to get current time", L"Error", MB_OK);
  }

  std::this_thread::sleep_for(1s);

  return 0;
}
