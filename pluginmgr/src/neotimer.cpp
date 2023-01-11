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
  if (m_Expired == false) {
    //          std::cout << "timer is currently running, please expire it
    //          first..." << std::endl;
    return;
  }
  m_Expired = false;
  std::thread([this, interval, task]() {
    for (auto temp = interval; !m_ToExpire && temp--; ) {
      for (char i = 0; !m_ToExpire && i != 120; ++i) {
        std::this_thread::sleep_for(500ms);
      }
    }
    if (!m_ToExpire) task();
  label:
    //          std::cout << "stop task..." << std::endl;
    {
      std::lock_guard<std::mutex> locker(m_Mutex);
      m_Expired = true;
      m_ExpiredCondition.notify_one();
    }
  }).detach();
}

void NeoTimer::ResetTime(uint32_t mini, const std::function<void()>& task) {
  Expire();
  StartTimer(mini, task);
}

void NeoTimer::Expire() {
  if (m_Expired) {
    return;
  }

  if (m_ToExpire) {
    //          std::cout << "timer is trying to expire, please wait..." <<
    //          std::endl;
    return;
  }
  m_ToExpire = true;
  {
    std::unique_lock<std::mutex> locker(m_Mutex);
    m_ExpiredCondition.wait(locker, [this] { return m_Expired == true; });
    if (m_Expired == true) {
      //              std::cout << "timer expired!" << std::endl;
      m_ToExpire = false;
    }
  }
}
