#ifndef PORTAL_H
#define PORTAL_H

#include <string>
#include <functional>
#include <mutex>
#include <neobox/httplib.h>

struct Portal {
  std::u8string timestamp, token;
  using Mutex = std::mutex;
  // Mutex mtx;

  std::u8string const mainHost = u8"tsinghua.edu.cn";
  std::u8string const subHost = u8"auth4." + mainHost;

  Portal(std::u8string username, std::u8string password)
    : client(HttpUrl(u8"login." + mainHost, u8"/"), true)
  {
    userInfo.username = username;
    userInfo.password = password;
    init().get();
  }

  ~Portal() {
    // std::lock_guard<Mutex> lock(mtx);
  }

  enum class Type {
    Account
  } type;

  struct UserInfo {
    std::u8string acID;
    std::u8string username;
    std::u8string password;
    std::u8string domain;
    std::u8string ip;
    bool isLogin = false;
  } userInfo;

  typedef std::function<void(std::u8string)> Callback;

  static std::u8string getTimestamp() {
    using namespace std::chrono;
    auto timeMs = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    auto count =  std::to_string(timeMs.count());
    return std::u8string(count.begin(), count.end());
  }
  
  HttpLib client;

  HttpAction<void> init();
  HttpAction<void> login(Type type);
  HttpAction<void> logout();

  HttpAwaiter getInfo();
  HttpAwaiter sendAuth(std::u8string_view token);
  HttpAwaiter getToken(std::u8string_view ip);
  std::optional<YJson> parseJson(HttpResponse* res);
  std::u8string_view parseToken(YJson& json);
  // std::u8string_view parseInfo(YJson& json);

  static std::u8string base64(std::u8string data, const std::u8string& alpha);
  static std::u8string encodeUserInfo(const class YJson& info, std::u8string_view token);
  static std::u8string hmd5(std::u8string_view str, std::u8string_view key);
};


#endif  // PORTAL_H
