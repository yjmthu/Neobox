#ifndef NEOCONFIG_H
#define NEOCONFIG_H

#include <mutex>
#include <string>
#include <yjson.h>
#include <functional>

#define CfgBool(VarName)                                       \
public:                                                        \
  bool Get##VarName() {                                        \
    Locker locker(m_Mutex);                                    \
    return m_##VarName == YJson::True;                         \
  }                                                            \
  void Set##VarName(bool on) {                                 \
    Locker locker(m_Mutex);                                    \
    m_##VarName = on ? YJson::True : YJson::False;             \
    m_Callback();                                              \
  }                                                            \
private:                                                       \
  YJson::Type& m_##VarName = m_Settings[u8"" #VarName].getType();

#define CfgInt(VarName)                                        \
public:                                                        \
  int Get##VarName() {                                         \
    Locker locker(m_Mutex);                                    \
    return static_cast<int>(m_##VarName);                      \
  }                                                            \
  void Set##VarName(int val) {                                 \
    Locker locker(m_Mutex);                                    \
    m_##VarName = val;                                         \
    m_Callback();                                              \
  }                                                            \
private:                                                       \
  double& m_##VarName=m_Settings[u8"" #VarName].getValueDouble();

#define CfgDouble(VarName)                                     \
public:                                                        \
  double Get##VarName() {                                      \
    Locker locker(m_Mutex);                                    \
    return m_##VarName;                                        \
  }                                                            \
  void Set##VarName(double val) {                              \
    Locker locker(m_Mutex);                                    \
    m_##VarName = val;                                         \
    m_Callback();                                              \
  }                                                            \
private:                                                       \
  double& m_##VarName=m_Settings[u8"" #VarName].getValueDouble();

#define CfgString(VarName)                                     \
public:                                                        \
  std::u8string Get##VarName() {                               \
    Locker locker(m_Mutex);                                    \
    return m_##VarName;                                        \
  }                                                            \
  void Set##VarName(std::u8string val) {                       \
    Locker locker(m_Mutex);                                    \
    m_##VarName.swap(val);                                     \
    m_Callback();                                              \
  }                                                            \
private:                                                       \
  std::u8string& m_##VarName=m_Settings[u8"" #VarName].getValueString();

#define ConfigConsruct(ClassName)                              \
public:                                                        \
  explicit ClassName(YJson& settings, Callback callback)      \
    : NeoConfig(settings, callback) {}

class NeoConfig {
protected:
  typedef std::mutex Mutex;
  typedef std::lock_guard<Mutex> Locker;
  typedef std::function<void()> Callback;
  std::mutex m_Mutex;
  YJson& m_Settings;
  const Callback m_Callback;
public:
  explicit NeoConfig(YJson& data, Callback callback)
    : m_Settings(data)
    , m_Callback(std::move(callback))
  {}
};

#endif // NEOCONFIG_H