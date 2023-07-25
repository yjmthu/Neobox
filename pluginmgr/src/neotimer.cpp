#include <neotimer.h>
#include <stdint.h>

using namespace std::literals;

NeoTimer::NeoTimer() : m_Expired(true), m_ToExpire(false) {}

NeoTimer::NeoTimer(const NeoTimer& t) {
  m_Expired = t.m_Expired.load();
  m_ToExpire = t.m_ToExpire.load();
}
NeoTimer::~NeoTimer() {
  Expire();
  //      std::cout << "timer destructed!" << std::endl;
}

void NeoTimer::StartTimer(uint32_t interval, std::function<void()> task) {
  // 检查是否已经开启线程
  {
    Locker locker(m_Mutex);
    if (!m_Expired) return;
    m_Expired = false;
  }

  // 开启线程
  std::thread([this, interval, task]() {
    while (true) {
      for (auto temp = interval; !m_ToExpire && temp--; ) {
        for (char i = 0; !m_ToExpire && i != 120; ++i) {
          std::this_thread::sleep_for(500ms);
        }
      }
      if (m_ToExpire) {
        // 线程退出
        Locker locker(m_Mutex);
        m_Expired = true;
        m_Condition.notify_one();
        break;
      } else {
        task();
      }
    }
  }).detach();
}

bool NeoTimer::IsActive() const {
  return m_Expired.load();
}

void NeoTimer::ResetTime(uint32_t mini, const std::function<void()>& task) {
  Expire();
  StartTimer(mini, task);
}

void NeoTimer::Expire() {
  // 检查是否未开启线程
  if (m_Expired) return;

  {
    Locker locker(m_Mutex);

    if (m_ToExpire) return;
    m_ToExpire = true;
  }

  {
    std::unique_lock<std::mutex> locker(m_Mutex);
    // 释放锁，等待定时器被释放
    m_Condition.wait(locker, [this](){
      return m_Expired == true;
    });
    m_ToExpire = false;
  }
}
