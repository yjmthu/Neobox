#ifndef HTTPPROXY_H
#define HTTPPROXY_H

#include <mutex>
#include <string>

class YJson;

class HttpProxy 
{
private:
  typedef std::lock_guard<std::mutex> Locker;
public:
  typedef std::u8string String;
  explicit HttpProxy(YJson& settings);

  static std::u8string GetSystemProxy();
  static bool IsSystemProxy();
  bool IsUserEmpty();
private:
  YJson& m_Settings;
public:
  String GetProxy();
  void SetProxy(String);
  String GetUsername();
  void SetUsername(String);
  String GetPassword();
  void SetPassword(String);
  int GetType();
  void SetType(int);
  void UpdateSystemProxy();
private:
  std::mutex m_Mutex;
  String& m_Proxy;
  String& m_Username;
  String& m_Password;
  double& m_Type;
  static YJson& InitSettings(YJson&);
};

#endif