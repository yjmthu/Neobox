#include "timer.h"

#include <stdint.h>

Timer::Timer() : expired_(true), try_to_expire_(false) {}

Timer::Timer(const Timer& t) {
  expired_ = t.expired_.load();
  try_to_expire_ = t.try_to_expire_.load();
}
Timer::~Timer() {
  Expire();
  //      std::cout << "timer destructed!" << std::endl;
}

void Timer::StartTimer(uint32_t interval, std::function<void()> task) {
  if (expired_ == false) {
    //          std::cout << "timer is currently running, please expire it
    //          first..." << std::endl;
    return;
  }
  expired_ = false;
  std::thread([this, interval, task]() {
    auto temp = interval;
    while (temp--) {
      for (char i = 0; i < 60; ++i) {
        if (try_to_expire_)
          goto label;
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    }
    task();
  label:
    //          std::cout << "stop task..." << std::endl;
    {
      std::lock_guard<std::mutex> locker(mutex_);
      expired_ = true;
      expired_cond_.notify_one();
    }
  }).detach();
}

void Timer::ResetTime(uint32_t mini, const std::function<void()>& task) {
  Expire();
  StartTimer(mini, task);
}

void Timer::Expire() {
  if (expired_) {
    return;
  }

  if (try_to_expire_) {
    //          std::cout << "timer is trying to expire, please wait..." <<
    //          std::endl;
    return;
  }
  try_to_expire_ = true;
  {
    std::unique_lock<std::mutex> locker(mutex_);
    expired_cond_.wait(locker, [this] { return expired_ == true; });
    if (expired_ == true) {
      //              std::cout << "timer expired!" << std::endl;
      try_to_expire_ = false;
    }
  }
}
