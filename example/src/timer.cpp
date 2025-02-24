#include <neobox/neotimer.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>

using namespace std::literals;

#if 0
int main() {
  auto timer = NeoTimer::New();
  timer->StartTimer(1s, [timer] {
    std::cout << "Hello, world!" << std::endl;
    timer->Expire();
  });

  std::this_thread::sleep_for(2s);

  timer->StartOnce(3s, [] {
    std::cout << "Goodbye, world!" << std::endl;
  });

  std::this_thread::sleep_for(6s);

  timer->Destroy();
  std::cout << "End." << std::endl;

  return 0;
}

#elif 1

int main() {
  std::mutex mutex;
  std::condition_variable cv;
  bool ready = false;
  std::thread t([&] {
    std::this_thread::sleep_for(1s);
    std::lock_guard<std::mutex> lock(mutex);
    std::cout << "Notifying..." << std::endl;
    ready = true;
    cv.notify_one();
    std::cout << "Notified." << std::endl;
    std::this_thread::sleep_for(1s);
  });

  std::unique_lock<std::mutex> lock(mutex);
  cv.wait_for(lock, 5s, [&] { return ready; });
  std::cout << "Ready." << std::endl;
  t.join();
  return 0;
}

#endif
