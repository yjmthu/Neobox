#include <neobox/neotimer.h>
#include <set>

#ifdef _DEBUG
#include <iostream>
#endif

using namespace std::literals;

class Pool {
  std::mutex m_PoolMutex;
  std::set<NeoTimer*> m_Pool;
  std::condition_variable m_Condition;
public:
  explicit Pool();
  ~Pool();
  NeoTimer* Add();
  void Remove(NeoTimer*);
};

Pool::Pool() {}
Pool::~Pool() {
  std::unique_lock locker(m_PoolMutex);
  for (auto timer : m_Pool) {
    timer->Expire();
  }
  m_Condition.wait(locker, [this] { return m_Pool.empty(); });
#ifdef _DEBUG
  std::cout << "all single shot have been killed." << std::endl;
#endif
}

NeoTimer* Pool::Add() {
  std::unique_lock _(m_PoolMutex);
  auto timer = NeoTimer::New();
  m_Pool.insert(timer);
  return timer;
}

void Pool::Remove(NeoTimer* timer) {
  std::unique_lock _(m_PoolMutex);
  m_Pool.erase(timer);
  if (m_Pool.empty()) {
    m_Condition.notify_all();
  }
}

static Pool* st_Pool = nullptr;

TimerGuard::TimerGuard() {
  if (!st_Pool) st_Pool = new Pool();
}

TimerGuard::~TimerGuard() {
  delete st_Pool;
  st_Pool = nullptr;
}

NeoTimer::NeoTimer() : m_Expired(true), m_ToExpire(false) {}

NeoTimer::~NeoTimer() {
  Expire();
  Locker _(m_Mutex);
  if (m_Count) {
    m_Condition.wait(_, [this] { return m_Count == 0; });
  }
}

NeoTimer* NeoTimer::New() {
  auto timer = new NeoTimer();
  return timer;
}

void NeoTimer::Destroy() {
  delete this;
}

void NeoTimer::StartTimer(Ms duration, std::function<void()> task) {
  this->Expire();

  Locker locker(m_Mutex);
  m_Expired = false;

  // 开启线程
  std::thread([this, duration, task]() {
    Locker locker(m_Mutex);

    while (!m_ToExpire) {
      auto const r = m_Condition.wait_for(locker, duration, [this] {
        return m_ToExpire;
      });
      if (m_ToExpire || r) break;
      StartTask(std::move(task));
    }
    // 线程退出
    m_ToExpire = false;
    m_Expired = true;

    locker.unlock();
    m_Condition.notify_all();
  }).detach();
}

void NeoTimer::StartOnce(Ms duration, std::function<void()> task) {
  this->Expire();

  Locker locker(m_Mutex);
  m_Expired = false;

  std::thread([this, duration, task = std::move(task)] {
    Locker locker(m_Mutex);

    auto const r = m_Condition.wait_for(locker, duration, [this] {
      return m_ToExpire;
    });
    if (!m_ToExpire && !r) {
      StartTask(std::move(task));
    }

    m_ToExpire = false;
    m_Expired = true;

    locker.unlock();
    m_Condition.notify_all();
  }).detach();
}

bool NeoTimer::IsActive() const {
  Locker locker(m_Mutex);
  return m_Expired;
}

void NeoTimer::Expire() {
  Locker locker(m_Mutex);

  // 检查是否未开启线程
  if (m_Expired) return;

  if (!m_ToExpire) {
    m_ToExpire = true;

    locker.unlock();
    m_Condition.notify_all();
    locker.lock();
  }

  // 释放锁，等待定时器被释放
  m_Condition.wait(locker, [this] { return m_Expired; });
}

void NeoTimer::StartTask(Task task) {
  m_Count++;
  std::thread([this, task = std::move(task)] {
    task();
    m_Mutex.lock();
    --m_Count;
    m_Mutex.unlock();
    if (m_Count == 0) {
      m_Condition.notify_all();
    }
  }).detach();
}

void NeoTimer::SingleShot(Ms duration, Task task) {
  auto const timer = st_Pool->Add();

  Locker locker(timer->m_Mutex);
  timer->m_Expired = false;

  std::thread([timer, duration, task = std::move(task)] {
    Locker locker(timer->m_Mutex);
    auto const r = timer->m_Condition.wait_for(locker, duration, [timer] {
      return timer->m_ToExpire;
    });
    if (!timer->m_ToExpire && !r) {
      task();
    }
    timer->m_ToExpire = false;
    timer->m_Expired = true;
    locker.unlock();
    timer->m_Condition.notify_all();

    st_Pool->Remove(timer);
  }).detach();
}
