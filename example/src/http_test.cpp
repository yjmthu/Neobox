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
  std::ofstream file(L"wallhaven-o59gvl.jpg", std::ios::out | std::ios::binary);
#else
  HttpLib clt("https://www.linux.org/"s, true);
  std::ofstream file(L"wallhaven-o59gvl.html", std::ios::out, std::ios::binary);
#endif
  if (file.is_open()) {
    auto quitLoop = [&mutex, &cv](){
      std::lock_guard locker(mutex);
      done = true;
      cv.notify_one();
    };
    HttpLib::Callback callback = {
      .m_WriteCallback = [&file](const void* data, size_t size){
        if (size) {
          file.write(reinterpret_cast<const char*>(data), size);
        }
      },
      .m_FinishCallback = [&clt, &quitLoop](std::wstring error, auto res){
        if (error.empty()) {
          std::wcout << L"ALL OK." << std::endl;
          for (auto [i, j]: res->headers) {
            std::cout << i << ": " << j << std::endl;
          }
        } else {
          std::wcerr << error << std::endl;
        }
        quitLoop();
      },
      .m_ProcessCallback = [](size_t recieve, size_t total){
        std::cout << recieve << '/' << total << std::endl;
      }
    };
    clt.GetAsync(callback);
    std::unique_lock locker(mutex);
    cv.wait(locker, []{ return done; });
    file.close();
  }
  std::cout << "============End============" << std::endl;
  // std::getchar();
  return 0;
}

