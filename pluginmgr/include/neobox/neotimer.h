#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <chrono>

class NeoTimer {
  typedef std::mutex Mutex;
  typedef std::unique_lock<Mutex> Locker;
  typedef std::chrono::milliseconds Ms;
  typedef std::function<void()> Task;
public:
  static NeoTimer* New();
  void Destroy();
  void StartTimer(Ms time, Task task);
  void StartOnce(Ms duration, Task task);
  bool IsActive() const;
  void Expire();
  
  static void SingleShot(Ms duration, Task task);

private:
  NeoTimer();
  ~NeoTimer();

  void StartTask(Task task);

  bool m_Expired;
  bool m_ToExpire;
  uint32_t m_Count = 0;
  Mutex mutable m_Mutex;
  std::condition_variable m_Condition;
};

struct TimerGuard {
  TimerGuard();
  ~TimerGuard();
};
#endif  // TIMER_H
