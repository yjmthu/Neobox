#include <httplib.h>
#include <stdexcept>
#include <format>
#include <regex>
#include <fstream>
#include <iostream>
#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>

#include <windows.h>

using namespace std::literals;

int main()
{
  std::cout << "============Begin============" << std::endl;
  std::mutex mutex;
  std::condition_variable cv;
  static bool done = false;
#if 1
  HttpLib clt("https://w.wallhaven.cc/full/o5/wallhaven-o59gvl.jpg"s, true);
  std::ofstream file(L"wallhaven-o59gvl.jpg", std::ios::out, std::ios::binary);
#else
  HttpLib clt("https://wallhaven.cc/w/o59gvl"s, true);
  std::ofstream file(L"wallhaven-o59gvl.html", std::ios::out, std::ios::binary);
#endif
  if (file.is_open()) {
    clt.GetAsync(
      [&file, &mutex, &cv](const void* data, size_t size){
        std::lock_guard locker(mutex);
        if (size != 0) {
          std::cout << "--OK--" << size << "--" << std::endl;
          file.write(reinterpret_cast<const char*>(data), size);
        } else {
          done = true;
          cv.notify_one();
        }
      },
      [&clt, &mutex, &cv](std::wstring error, auto res){
        if (error.empty()) {
          std::wcout << L"ALL OK." << std::endl;
          for (auto [i, j]: res->headers) {
            std::cout << i << ": " << j << std::endl;
          }
        } else {
          std::wcerr << error << std::endl;
          std::lock_guard locker(mutex);
          done = true;
          cv.notify_one();
        }
      }
    );
    std::unique_lock locker(mutex);
    cv.wait(locker, []{ return done; });
    file.close();
  }
  std::cout << "============End============" << std::endl;
  // std::getchar();
  return 0;
}

