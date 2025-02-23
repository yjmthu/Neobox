#include <neobox/neotimer.h>
#include <utility>

using namespace std::literals;

NeoTimer::NeoTimer() : m_Expired(true), m_ToExpire(false) {}

NeoTimer::~NeoTimer() {
  Expire();
  Locker _(m_Mutex);
}

void NeoTimer::Destroy() {
  if (m_ThreadId == std::this_thread::get_id()) {
    throw std::logic_error("Can not destroy timer in its own thread.");
  }
  delete this;
}

bool NeoTimer::StartTimer(Ms time, std::function<void()> task) {
  // 检查是否已经开启线程
  {
    Locker locker(m_Mutex);
    // if (!m_Expired || m_ToExpire) return false;
    if (!m_Expired) {
      if (m_ThreadId == std::this_thread::get_id()) {
        return false;
      }
      if (!m_ToExpire) {
        m_ToExpire = true;
      }
      m_Condition.wait(locker, [this] { return m_Expired; });
    }
    m_Expired = false;
  }

  m_Ms = time;
  m_Task = std::move(task);

  // 开启线程
  std::thread([this]() {
    Locker locker(m_Mutex);
    m_ThreadId = std::this_thread::get_id();

    while (!m_ToExpire) {
      auto const r = m_Condition.wait_for(locker, m_Ms, [this] {
        return m_ToExpire;
      });
      if (m_ToExpire || r) break;
      m_Task();
    }
    // 线程退出
    m_Expired = true;
    m_ToExpire = false;

    locker.unlock();
    m_Condition.notify_all();
  }).detach();

  return true;
}


bool NeoTimer::IsActive() const {
  return m_Expired;
}

bool NeoTimer::ResetTime(Ms time, const std::function<void()>& task) {
  Locker locker(m_Mutex);

  if (!m_Expired && std::this_thread::get_id() == m_ThreadId) {
    m_ToExpire = false;
    m_Expired = false;
    m_Ms = time;
    m_Task = task;
  } else {
    return StartTimer(time, task);
  }

  return true;
}

void NeoTimer::Expire() {
  Locker locker(m_Mutex);

  // 检查是否未开启线程
  if (m_Expired || m_ToExpire) return;

  m_ToExpire = true;

  if (std::this_thread::get_id() == m_ThreadId) return;

  locker.unlock();
  m_Condition.notify_all();

  locker.lock();
  // 释放锁，等待定时器被释放
  m_Condition.wait(locker, [this] { return m_Expired; });
}
