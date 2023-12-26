#include <neobox/neotimer.h>
#include <utility>

using namespace std::literals;

NeoTimer::NeoTimer() : m_Expired(true), m_ToExpire(false) {}

NeoTimer::NeoTimer(const NeoTimer& t) {
  m_Expired = t.m_Expired.load();
  m_ToExpire = t.m_ToExpire.load();
}
NeoTimer::~NeoTimer() {
  Expire();
  while (m_ToExpire);
  Locker _(m_Mutex);
  //      std::cout << "timer destructed!" << std::endl;
}

void NeoTimer::StartTimer(std::chrono::seconds seconds, std::function<void()> task) {
  // 检查是否已经开启线程
  {
    Locker locker(m_Mutex);
    if (!m_Expired) return;
    m_Expired = false;
  }

  // 开启线程
  auto count = seconds.count();
  std::thread([this, count, task]() {
    while (true) {
      for (auto temp = count; !m_ToExpire && temp--; ) {
        std::this_thread::sleep_for(1s);
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


void NeoTimer::StartTimer(std::chrono::minutes minutes, std::function<void()> task)
{
  auto seconds = static_cast<std::chrono::seconds>(minutes);
  StartTimer(seconds, std::move(task));
}

bool NeoTimer::IsActive() const {
  return m_Expired.load();
}

void NeoTimer::ResetTime(std::chrono::minutes minutes, const std::function<void()>& task) {
  Expire();
  StartTimer(minutes, task);
}

void NeoTimer::ResetTime(std::chrono::seconds seconds, const std::function<void()>& task) {
  Expire();
  StartTimer(seconds, task);
}

void NeoTimer::Expire() {
  // 检查是否未开启线程
  if (m_Expired) return;

  Locker locker(m_Mutex);

  if (m_ToExpire) return;
  m_ToExpire = true;

  // 这样在task中也能成功调用Expire，但是需要等待结束
  std::thread([this]() {
    std::unique_lock<std::mutex> locker(m_Mutex);
    // 释放锁，等待定时器被释放
    m_Condition.wait(locker, [this](){
      return m_Expired == true;
    });
    m_ToExpire = false;
  }).detach();
}
