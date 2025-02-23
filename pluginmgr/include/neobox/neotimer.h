#ifndef TIMER_H
#define TIMER_H

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <chrono>

class NeoTimer {
  typedef std::recursive_mutex Mutex;
  typedef std::unique_lock<Mutex> Locker;
  typedef std::chrono::milliseconds Ms;
public:
  static NeoTimer* New() {
    return new NeoTimer();
  }
  void Destroy();
  bool StartTimer(Ms time, std::function<void()> task);
  bool StartOnce(Ms duration, std::function<void()> task);
  bool ResetTime(Ms time, const std::function<void()>& task);
  bool IsActive() const;
  void Expire();

private:
  NeoTimer();
  ~NeoTimer();

  bool Prepare();

  bool m_Expired;
  bool m_ToExpire;
  std::thread::id m_ThreadId;
  Ms m_Ms;
  std::function<void()> m_Task;
  Mutex m_Mutex;
  std::condition_variable_any m_Condition;
};

#endif  // TIMER_H
