#ifndef PORTAL_H
#define PORTAL_H

#include <string>
#include <functional>
#include <iostream>
#include <mutex>

struct Portal {
  std::u8string timestamp, token;
  using Mutex = std::mutex;
  Mutex mtx;

  std::u8string const mainHost = u8"tsinghua.edu.cn";
  std::u8string const subHost = u8"auth4." + mainHost;


  Portal(std::u8string username, std::u8string password) {
    userInfo.username = username;
    userInfo.password = password;
    if (!init()) {
      std::cerr << "Init failed\n";
    }
  }

  ~Portal() {
    std::lock_guard<Mutex> lock(mtx);
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

  static std::u8string base64(std::u8string data, const std::u8string& alpha) {
    uint8_t cursor = 0;
    std::u8string result;
    result.reserve(data.size() * 4 / 3 + 2);
    for (auto iter = data.begin(); iter != data.end(); ++iter) {
      // number before cursor is rubish
      uint8_t const c = *iter;
      if (cursor <= 2) {
        uint8_t code = c << cursor;
        code >>= 2;
        result.push_back(alpha[code]);
        cursor += 6;
      }
      if (cursor == 8) {
        cursor = 0;
        continue;
      }

      uint8_t code = c << cursor;
      code >>= 2;
      if (iter + 1 != data.end()) {
        cursor -= 2;
        code |= iter[1] >> (8 - cursor);
      }
      result.push_back(alpha[code]);
    }

    result.push_back(u8'=');

    return result;
  }

  bool init();

  void getInfo();

  void login(Type type);

  void logout();

  void sendAuth(bool defaultStack);

  void getToken(std::u8string ip, Callback callback);

  static std::u8string encodeUserInfo(const class YJson& info, std::u8string token);
  static std::u8string hmd5(std::u8string_view str, std::u8string_view key);
};


#endif  // PORTAL_H
