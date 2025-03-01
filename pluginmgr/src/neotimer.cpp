#include <neobox/neotimer.h>

#ifdef _DEBUG
#include <iostream>
#endif

using namespace std::literals;

struct Pool {
  std::mutex m_PoolMutex;
  uint32_t m_PoolCount;
  std::condition_variable m_PoolCondition;

  explicit Pool();
  ~Pool();
  void Add();
  void Remove();
};

Pool::Pool() : m_PoolCount(0) {}
Pool::~Pool() {
  std::unique_lock _(m_PoolMutex);
  if (m_PoolCount) {
#ifdef _DEBUG
    std::cout << "Pool is not empty, waiting..." << std::endl;
#endif
    m_PoolCondition.wait(_, [this] { return m_PoolCount == 0; });
#ifdef _DEBUG
    std::cout << "Pool is empty now, exit." << std::endl;
#endif
  }
}
void Pool::Add() {
  std::unique_lock _(m_PoolMutex);
  m_PoolCount++;
}

void Pool::Remove() {
  std::unique_lock _(m_PoolMutex);
  m_PoolCount--;
  if (m_PoolCount == 0) {
    m_PoolCondition.notify_all();
  }
}

static Pool m_Pool = Pool();

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
  m_Pool.Add();
  return timer;
}

void NeoTimer::Destroy() {
  delete this;
  m_Pool.Remove();
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
  auto const timer = NeoTimer::New();
  timer->StartTimer(duration, [timer, task = std::move(task)] {
    task();
    timer->Destroy();
  });
}
