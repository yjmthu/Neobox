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
  std::atomic<bool> m_Expired;
  std::atomic<bool> m_ToExpire;
  std::mutex m_Mutex;
  std::condition_variable m_ExpiredCondition;
};

#endif  // TIMER_H
