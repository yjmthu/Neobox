#include <neobox/neotimer.h>
#include <chrono>
#include <iostream>

using namespace std::literals;

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
