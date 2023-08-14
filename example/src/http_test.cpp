#include <neobox/httplib.h>
#include <stdexcept>
#include <format>
#include <regex>
#include <fstream>
#include <iostream>
#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, const std::u8string& str) {
  return out.write(reinterpret_cast<const char*>(str.data()), str.length());
}

int main()
{
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif
  std::cout << "============Begin============" << std::endl;
  std::mutex mutex;
  std::condition_variable cv;
  static bool done = false;
#if 0
  HttpUrl url(u8"https://w.wallhaven.cc/full/o5/wallhaven-o59gvl.jpg"sv);
  std::cout << url.host << std::endl;
  std::cout << url.GetObjectString() << std::endl;
  HttpLib clt(url, true);
  std::ofstream file("wallhaven-o59gvl.jpg", std::ios::out | std::ios::binary);
#elif 0
  HttpLib clt("https://www.linux.org/"s, true);
  std::ofstream file(L"wallhaven-o59gvl.html", std::ios::out | std::ios::binary);
#elif 1
  HttpUrl url(u8"https://source.unsplash.com/random/2500x1600"sv);
  HttpLib clt(url, true, 10s);
  clt.SetRedirect(-1);
  std::ofstream file("unsplash-test.png", std::ios::out | std::ios::binary);
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
          std::wcout << L"ALL OK.\n---------------" << res->status << std::endl;
          for (auto [i, j]: res->headers) {
            std::cout << i << ": " << j << std::endl;
          }
          std::wcout << L"---------------\n";
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

