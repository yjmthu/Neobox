#ifndef TIMER_H
#define TIMER_H

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>

class NeoTimer {
  typedef std::lock_guard<std::mutex> Locker;
public:
  NeoTimer();
  NeoTimer(const NeoTimer& t);
  ~NeoTimer();
  void StartTimer(std::chrono::minutes minutes, std::function<void()> task);
  void StartTimer(std::chrono::seconds seconds, std::function<void()> task);
  void ResetTime(std::chrono::seconds seconds, const std::function<void()>& task);
  void ResetTime(std::chrono::minutes minutes, const std::function<void()>& task);
  bool IsActive() const;
  void Expire();

  template<typename Rep, typename Period>
  void StartOnce(std::chrono::duration<Rep, Period> duration, std::function<void()> task) {
    StartTimer(duration, [this, task]() {
      task();
      Expire();
    });
  }

private:

  std::atomic<bool> m_Expired;
  std::atomic<bool> m_ToExpire;
  std::mutex m_Mutex;
  std::condition_variable m_Condition;
};

#endif  // TIMER_H
