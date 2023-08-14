#ifndef NEOCONFIG_H
#define NEOCONFIG_H

#include <mutex>
#include <yjson/yjson.h>
#include <functional>
#include <neobox/pluginmgr.h>

#define CfgBool(VarName)                                       \
public:                                                        \
  bool Get##VarName() const {                                  \
    Locker locker(m_Mutex);                                    \
    return m_##VarName == YJson::True;                         \
  }                                                            \
  void Set##VarName(bool on, bool save=true) {                 \
    Locker locker(m_Mutex);                                    \
    m_##VarName = on ? YJson::True : YJson::False;             \
    if (save) m_Callback();                                    \
  }                                                            \
private:                                                       \
  YJson::Type& m_##VarName = m_Settings[u8"" #VarName].getType();

#define CfgInt(VarName)                                        \
public:                                                        \
  int Get##VarName() const {                                   \
    Locker locker(m_Mutex);                                    \
    return static_cast<int>(m_##VarName);                      \
  }                                                            \
  void Set##VarName(int val, bool save=true) {                 \
    Locker locker(m_Mutex);                                    \
    m_##VarName = val;                                         \
    if (save) m_Callback();                                    \
  }                                                            \
private:                                                       \
  double& m_##VarName=m_Settings[u8"" #VarName].getValueDouble();

#define CfgDouble(VarName)                                     \
public:                                                        \
  double Get##VarName() const {                                \
    Locker locker(m_Mutex);                                    \
    return m_##VarName;                                        \
  }                                                            \
  void Set##VarName(double val, bool save=true) {              \
    Locker locker(m_Mutex);                                    \
    m_##VarName = val;                                         \
    if (save) m_Callback();                                    \
  }                                                            \
private:                                                       \
  double& m_##VarName=m_Settings[u8"" #VarName].getValueDouble();

#define CfgString(VarName)                                     \
public:                                                        \
  std::u8string Get##VarName() const {                         \
    Locker locker(m_Mutex);                                    \
    return m_##VarName;                                        \
  }                                                            \
  void Set##VarName(std::u8string val, bool save=true) {       \
    Locker locker(m_Mutex);                                    \
    m_##VarName.swap(val);                                     \
    if (save) m_Callback();                                    \
  }                                                            \
private:                                                       \
  std::u8string& m_##VarName=m_Settings[u8"" #VarName].getValueString();


#define CfgArray(VarName)                                      \
public:                                                        \
  YJson::ArrayType Get##VarName() const {                      \
    Locker locker(m_Mutex);                                    \
    return m_##VarName;                                        \
  }                                                            \
  void Append##VarName(YJson value, bool save=true) {          \
    Locker locker(m_Mutex);                                    \
    m_##VarName.emplace_back(std::move(value));                \
    if (save) m_Callback();                                    \
  }                                                            \
  void Set##VarName(YJson::ArrayType val, bool save=true) {    \
    Locker locker(m_Mutex);                                    \
    m_##VarName.swap(val);                                     \
    if (save) m_Callback();                                    \
  }                                                            \
private:                                                       \
  YJson::ArrayType& m_##VarName=m_Settings[u8"" #VarName].getArray();


#define CfgObject(VarName)                                      \
public:                                                         \
  YJson::ObjectType Get##VarName() const {                      \
    Locker locker(m_Mutex);                                     \
    return m_##VarName;                                         \
  }                                                             \
  void Append##VarName(std::u8string key, YJson value, bool save=true) {\
    Locker locker(m_Mutex);                                     \
    m_##VarName.emplace_back(std::move(key), std::move(value)); \
    if (save) m_Callback();                                     \
  }                                                             \
  void Set##VarName(YJson::ObjectType val, bool save=true) {    \
    Locker locker(m_Mutex);                                     \
    m_##VarName.swap(val);                                      \
    if (save) m_Callback();                                    \
  }                                                             \
private:                                                        \
  YJson::ObjectType& m_##VarName=m_Settings[u8"" #VarName].getObject();


#define CfgYJson(VarName)                                      \
public:                                                        \
  YJson Get##VarName() const {                                 \
    Locker locker(m_Mutex);                                    \
    return m_##VarName;                                        \
  }                                                            \
  void Set##VarName(YJson val, bool save=true) {               \
    Locker locker(m_Mutex);                                    \
    m_##VarName = std::move(val);                              \
    if (save) m_Callback();                                    \
  }                                                            \
private:                                                       \
  YJson& m_##VarName=m_Settings[u8"" #VarName];

#define ConfigConsruct(ClassName)                              \
public:                                                        \
  explicit ClassName(YJson& settings)                          \
    : NeoConfig(settings) {}

class NeoConfig {
protected:
  typedef void (*Callback) ();
  typedef std::mutex Mutex;
  typedef std::lock_guard<Mutex> Locker;
  typedef std::unique_lock<Mutex> LockerEx;
protected:
  std::mutex& m_Mutex = mgr->m_Mutex;
  const Callback m_Callback = [](){ mgr->SaveSettings(); };
  YJson& m_Settings;
public:
  explicit NeoConfig(YJson& data)
    : m_Settings(data)
  {}
  void SaveData() const {
    m_Mutex.lock();
    m_Callback();
    m_Mutex.unlock();
  }
};

#endif // NEOCONFIG_H
