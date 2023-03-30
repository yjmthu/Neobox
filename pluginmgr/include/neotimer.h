#ifndef TIMER_H
#define TIMER_H

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>

class NeoTimer {
  typedef std::lock_guard<std::mutex> Locker;
public:
  NeoTimer();
  NeoTimer(const NeoTimer& t);
  ~NeoTimer();
  void StartTimer(uint32_t interval, std::function<void()> task);
  void ResetTime(uint32_t mini, const std::function<void()>& task);
  bool IsActive() const;
  void Expire();

private:

  std::atomic<bool> m_Expired;
  std::atomic<bool> m_ToExpire;
  std::mutex m_Mutex;
  std::condition_variable m_Condition;
};

#endif  // TIMER_H
