#ifndef TIMER_H
#define TIMER_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

class NeoTimer {
 public:
  NeoTimer();
  NeoTimer(const NeoTimer& t);
  ~NeoTimer();
  void StartTimer(uint32_t interval, std::function<void()> task);
  void ResetTime(uint32_t mini, const std::function<void()>& task);
  void Expire();

 private:
  std::atomic<bool> expired_;
  std::atomic<bool> try_to_expire_;
  std::mutex mutex_;
  std::condition_variable expired_cond_;
};

#endif  // TIMER_H
