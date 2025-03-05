#include <neobox/neotimer.h>
#include <set>

#ifdef _DEBUG
#include <iostream>
#endif

using namespace std::literals;

class Pool {
  std::mutex m_PoolMutex;
  std::set<NeoTimer*> m_Pool;
  int m_Count = 0;
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
#ifdef _DEBUG
    std::cout << "destroy timer begin..." << std::endl;
#endif
    timer->Expire();
#ifdef _DEBUG
    std::cout << "timer expired-----" << std::endl;
#endif
    timer->Destroy();
#ifdef _DEBUG
    std::cout << "destroy timer end." << std::endl;
#endif
  }
  m_Pool.clear();

#ifdef _DEBUG
  std::cout << "wait for " << m_Count << " tasks to expire..." << std::endl;
#endif
  m_Condition.wait(locker, [this] { return m_Count == 0; });
#ifdef _DEBUG
  std::cout << "all tasks expired." << std::endl;
#endif
}

NeoTimer* Pool::Add() {
  std::unique_lock _(m_PoolMutex);
  auto timer = NeoTimer::New();
  m_Pool.insert(timer);
  ++m_Count;
  return timer;
}

void Pool::Remove(NeoTimer* timer) {
#ifdef _DEBUG
  std::cout << "remove timer from pool" << std::endl;
#endif
  std::unique_lock _(m_PoolMutex);
  auto iter = m_Pool.find(timer);
  if (iter != m_Pool.end()) {
    m_Pool.erase(iter);
    timer->Destroy();
  }
  if (--m_Count == 0) {
    m_Condition.notify_all();
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
  auto const timer = m_Pool.Add();

  Locker locker(timer->m_Mutex);
  timer->m_Expired = false;

  std::thread([timer, duration, task = std::move(task)] {
    Locker locker(timer->m_Mutex);
#ifdef _DEBUG
    std::cout << "start single shot timer " << duration << std::endl;
#endif
    // auto const r = timer->m_Condition.wait_for(locker, duration, [timer] {
    //   return timer->m_ToExpire;
    // });
    locker.unlock();
    for (auto i = 0; i < duration.count(); ++i) {
      std::this_thread::sleep_for(1s);
      std::cout << "timer " << i << std::endl;
      locker.lock();
      if (timer->m_ToExpire) break;
      locker.unlock();
    }
    locker.lock();
    auto r = false;
    if (!timer->m_ToExpire && !r) {
      task();
    }
    timer->m_ToExpire = false;
    timer->m_Expired = true;
    locker.unlock();
#ifdef _DEBUG
    std::cout << "timer expired" << std::endl;
#endif
    timer->m_Condition.notify_all();

    m_Pool.Remove(timer);
  }).detach();
}
