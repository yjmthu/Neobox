#include <iostream>
#include <chrono>
#include <Windows.h>
#include <yjson/yjson.h>
#include <neobox/httplib.h>
#include <QByteArray>
#include <QCryptographicHash>
#include <mutex>
#include <regex>

#include <filesystem>

using namespace std::literals;

std::ostream& operator<<(std::ostream& os, std::u8string_view res) {
  os.write(reinterpret_cast<const char*>(res.data()), res.size());
  return os;
}

namespace ApiList {
  auto info = u8"/cgi-bin/rad_user_info"s;
  auto auth = u8"/cgi-bin/srun_portal"s;
  auto loginDM = u8"/cgi-bin/rad_user_dm"s;
  auto authWechat = u8"/v1/srun_wechat_code"s;
  auto authSMSPhone = u8"/cgi-bin/srunmobile_portal"s;
  auto authSMSAccount = u8"/v1/srun_portal_sms"s;
  auto authEvent = u8"/cgi-bin/srun_events_auth"s;
  auto token = u8"/cgi-bin/get_challenge"s;
  auto vcodePhone = u8"/cgi-bin/srunmobile_vcode"s;
  auto vcodeAccount = u8"/v1/srun_portal_sms_code"s;
  auto vcodeEvent = u8"/cgi-bin/srun_mobile_events_code"s;
  auto sign = u8"/v1/srun_portal_sign"s;
  auto notice = u8"/v2/srun_portal_message"s;
  auto log = u8"/v1/srun_portal_log"s;
  auto ssoWechat = u8"/v1/srun_wechat_barcode"s;
  auto sso = u8"/v1/srun_portal_sso"s;
  auto protocol = u8"/v1/srun_portal_agree_new"s;
  auto agreeProtocol = u8"/v1/srun_portal_agree_bind"s;
  auto userAgreed = u8"/v1/srun_portal_agrees"s;
  auto authWework = u8"/v1/srun_portal_wework"s;
  auto getPassVcode = u8"/v1/srun_portal_password_code"s;
  auto changeByPass = u8"/v1/srun_portal_password_reset"s;
  auto changeByVcode = u8"/v1/srun_portal_password_forget"s;
  auto ciscoCheck = u8"/v1/precheck_account"s;
  auto getOnlineDevice = u8"/v1/srun_portal_online"s;
  auto acDetect = u8"/v1/srun_portal_detect"s;
  auto huaweiAuth = u8"/v1/huaweiNce"s;
  auto diallog = u8"/v1/srun_portal_diallog"s;
  auto bgSwitch = u8"/v2/srun_portal_bg_switch"s;
};

struct Portal {
  std::u8string timestamp, token;
  std::mutex mtx;

  Portal(std::u8string username, std::u8string password) {
    userInfo.username = username;
    userInfo.password = password;
  }

  ~Portal() {
    std::lock_guard<std::mutex> lock(mtx);
  }

  std::u8string const mainHost = u8"tsinghua.edu.cn";

  enum class Type {
    Account
  } type;

  struct UserInfo {
    std::u8string acID = u8"21";
    std::u8string username;
    std::u8string password;
    std::u8string domain;
    std::u8string ip;
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

  bool init(std::u8string subHost);

  void login(Type type);

  std::u8string encodeUserInfo(YJson info, std::u8string token);

  void sendAuth(std::u8string subHost, bool defaultStack);

  void getToken(std::u8string subHost, std::u8string ip, Callback callback);

  static std::u8string hmd5(std::u8string_view str, std::u8string_view key);
};

bool Portal::init(std::u8string subHost) {
  HttpUrl url(subHost, u8"/", {}, u8"https", 443);
  HttpLib clt(url);
  // clt.SetRedirect(3);
  auto res = clt.Get();
  if (res->status != 302) {
    return false;
  }
  // std::cout << res->headers.at(u8"Location") << std::endl;
  auto location = res->headers[u8"Location"];
  if (location.empty()) {
    return false;
  }
  std::cout << "Redirect to: " << url.GetFullUrl() << std::endl;
  clt.SetUrl(location);

  res = clt.Get();
  if (res->status != 200) {
    return false;
  }
  // std::cout << "Init success\n" << res->body << std::endl;
  {
    std::regex re(R"(url=(/srun_portal_pc\?ac_id=\d+[^"]*))");
    std::cmatch match;
    if (!std::regex_search(reinterpret_cast<const char*>(res->body.data()), match, re)) {
      return false;
    }
    // std::cout << "Match: " << match[2] << std::endl;
    std::string path = match[1];
    clt.SetUrl(url.scheme + u8"://" + subHost + std::u8string(path.begin(), path.end()));
    
    res = clt.Get();
    if (res->status != 200) {
      return false;
    }
  }

  {
    std::regex re(R"(CONFIG = (\{[^}]+\}))");
    std::cmatch match;
    if (!std::regex_search(reinterpret_cast<const char*>(res->body.data()), match, re)) {
      return false;
    }
    // std::cout << "Match: " << match[1] << std::endl;
    // std::cout << "Init success\n" << res->body << std::endl;
  }
  
  {
    std::regex re(R"(ip\s*:\s*"([^"]+)\")");
    std::cmatch match;
    if (!std::regex_search(reinterpret_cast<const char*>(res->body.data()), match, re)) {
      return false;
    }

    // std::cout << "Match: " << match[1] << std::endl;
    std::string ip = match[1];
    userInfo.ip = std::u8string(ip.begin(), ip.end());
    return true;
  }
  return false;
}

void Portal::login(Type type) {
  auto subHost = u8"auth4." + mainHost;
  if (!init(subHost)) {
    std::cerr << "Init failed\n";
    return;
  }
  if (userInfo.ip.empty()) {
    std::cerr << "Network not found.\n";
    return;
  }
  std::cout << "ip: " << std::string(userInfo.ip.begin(), userInfo.ip.end()) << std::endl;
  sendAuth(subHost, false);
}

void Portal::sendAuth(std::u8string subHost, bool defaultStack) {
  std::cout << "Get Token\n";
  getToken(subHost, userInfo.ip, [this, subHost](auto token) {
    std::cout << "Token: " << std::string(token.begin(), token.end()) << std::endl;
    // std::u8string const acID = u8"222";
    // std::u8string const acID = u8"21";
    std::u8string n = u8"200", type = u8"1";
    YJson info = YJson::O {
      { u8"username", userInfo.username },
      { u8"password", userInfo.password },
      { u8"ip", userInfo.ip },
      { u8"acid", userInfo.acID },
      { u8"enc_ver", u8"srun_bx1" },
    };

    const auto i = encodeUserInfo(info, token);
    std::cout << "Encode: " << (const char*)i.c_str() << std:: endl; 
    
    std::u8string str = token + userInfo.username;
    std::u8string hmd5 = this->hmd5(userInfo.password, token);
    str += token + hmd5;
    str += token + userInfo.acID;
    str += token + userInfo.ip;
    str += token + n;
    str += token + type;
    str += token + i;

    // sha1 of str
    auto sha1 = QCryptographicHash::hash(QByteArray(reinterpret_cast<const char*>(str.data()), str.size()), QCryptographicHash::Sha1).toHex();
    std::u8string chksum(sha1.begin(), sha1.end());
    // std::cout << (const char*)str.c_str() << "\n" << sha1.toStdString() << std::endl;

    HttpUrl url(subHost, ApiList::auth, {
      { u8"callback", u8"_" },
      { u8"action", u8"login" },
      { u8"username", userInfo.username },
      { u8"password", u8"{MD5}" + hmd5 },
      { u8"os", u8"Windows 10" },
      { u8"name", u8"Windows" },
      { u8"double_stack", u8"0" },
      { u8"chksum", chksum },
      { u8"info", i },
      { u8"ac_id", userInfo.acID },
      { u8"ip", userInfo.ip },
      { u8"n", n },
      { u8"type", type },
      { u8"_", getTimestamp() }
    }, u8"https", 443);

    mtx.lock();
    std::thread([url, this](){
      // auto full = url.GetFullUrl();
      // std::cout << (const char*)full.c_str() << std::endl;

      HttpLib clt(url, true);
      std::atomic_bool finished = false;
      HttpLib::Callback cb {
        .m_FinishCallback = [&finished](auto msg, auto res) {
          if (msg.empty() && res->status == 200) {
            // std::cout << (const char*)res->body.c_str() << std::endl;
            auto start = res->body.find(u8'{'), end = res->body.rfind(u8'}');
            YJson json(res->body.begin() + start, res->body.begin() + end + 1);
            std::cout << json << std::endl;
          } else {
            std::cerr << "Connect Error: " << res->status << std::endl;
          }
          std::cout << "Finished\n";
          finished = true;
        },
        .m_ProcessCallback = [](auto current, auto total) {
          std::cout << current << "/" << total << std::endl;
        },
      };

      clt.GetAsync(std::move(cb));
      while (!finished) {
        std::this_thread::sleep_for(10ms);
        std::cout << "wait...\n";
      }
      mtx.unlock();
    }).detach();
  });
}

void Portal::getToken(std::u8string subHost, std::u8string ip, Callback callback) {
  if (!this->timestamp.empty()) {
    callback(this->token);
    return;
  }
  auto timestamp = getTimestamp();
  HttpUrl url(subHost, ApiList::token, {
    { u8"username"s, userInfo.username },
    { u8"ip"s, ip },
    { u8"_", timestamp },
    { u8"callback", u8"_" }
  }, u8"https", 443);
  std:std::u8string full = url.GetFullUrl();
  std::cout << std::string(full.begin(), full.end()) << std::endl;

  HttpLib clt(url, true);
  std::atomic_bool finished = false;
  HttpLib::Callback cb{
      .m_FinishCallback =
          [callback, &finished](auto msg, auto res) {
            if (msg.empty() && res->status == 200) {
              auto begin = res->body.find(u8'{');
              auto end = res->body.rfind(u8'}');
              YJson json(res->body.begin() + begin,
                          res->body.begin() + end + 1);
              // std::cout << json;
              auto const token = json[u8"challenge"].getValueString();
              callback(token);
            } else {
              std::cerr << "Error: " << res->status << std::endl;
            }
            std::cout << "Finished\n";
            finished = true;
          },
      .m_ProcessCallback =
          [](auto current, auto total) {
            std::cout << current << "/" << total << std::endl;
          },
  };
  clt.GetAsync(std::move(cb));

  while (!finished) {
    std::this_thread::sleep_for(10ms);
    std::cout << "wait...\n";
  }
}

std::u8string Portal::hmd5(std::u8string_view key, std::u8string_view str) {
  constexpr size_t pageSize = 64;
  QByteArray strArray(reinterpret_cast<const char*>(str.data()), str.size());
  QByteArray u(pageSize, 0x36), c(pageSize, 0x5c);

  if (strArray.length() > pageSize) {
    // str -> md(str)
    strArray = QCryptographicHash::hash(strArray, QCryptographicHash::Md5);
  }

  strArray.resize(pageSize, u8'\0');

  for (size_t i = 0; i < pageSize; i++) {
    u[i] ^= strArray[i];
    c[i] ^= strArray[i];
  }

  u.append(reinterpret_cast<const char*>(key.data()), key.size());
  c.append(QCryptographicHash::hash(u, QCryptographicHash::Md5));
  auto hmd5 = QCryptographicHash::hash(c, QCryptographicHash::Md5).toHex();

  // std::cout << c.toHex().toStdString() << "/" << c.size() << std::endl;

  return std::u8string(hmd5.begin(), hmd5.end());
}

std::u8string Portal::encodeUserInfo(YJson info, std::u8string token) {
  auto s = [](std::u8string a, bool b) {
    auto car = a.length(), c = car;
    const auto remainder = c & 0b11;
    if (remainder) {
      c += 4 - remainder;
      a.resize(c, 0);
    }

    std::vector<uint32_t> v(c >> 2, 0);

    for (size_t i = 0; i < car; i += 4) {
      v[i >> 2] = a[i] | a[i + 1] << 8 | a[i + 2] << 16 | a[i + 3] << 24;
    }

    if (b) v.push_back(car);
    return v;
  };

  auto l = [](std::vector<uint32_t> a, bool b) {
    auto d = a.size();
    auto c = (d - 1) << 2;

    if (b) {
      auto m = a[d - 1];
      if (m < c - 3 || m > c) return u8""s;
      c = m;
    }

    std::u8string str(a.size() << 2, 0);

    for (size_t i = 0; i < d; i++) {
      auto k = i << 2;
      str[k] = a[i] & 0xff;
      str[k + 1] = a[i] >> 8 & 0xff;
      str[k + 2] = a[i] >> 16 & 0xff;
      str[k + 3] = a[i] >> 24 & 0xff;
    }

    return b ? str.substr(0, c) : str;
  };

  auto encode = [s, l](std::u8string str, std::u8string key) {

    if (str.empty()) return decltype(l({}, 0)) {};
    auto v = s(str, true);
    auto k = s(key, false);

    if (k.size() < 4) k.resize(4, 0);
    char32_t n = v.size() - 1,
        z = v[n],
        y = v[0],
        c = 0x86014019 | 0x183639A0,
        m = 0,
        e = 0,
        p = 0,
        q = floor(6 + 52 / (n + 1)),
        d = 0;

    while (0 < q--) {
      d = d + c & (0x8CE0D9BF | 0x731F2640);
      e = d >> 2 & 3;

      for (p = 0; p < n; p++) {
        y = v[p + 1];
        m = z >> 5 ^ y << 2;
        m += y >> 3 ^ z << 4 ^ (d ^ y);
        m += k[p & 3 ^ e] ^ z;
        z = v[p] = v[p] + m & (0xEFB8D130 | 0x10472ECF);
      }

      y = v[0];
      m = z >> 5 ^ y << 2;
      m += y >> 3 ^ z << 4 ^ (d ^ y);
      m += k[p & 3 ^ e] ^ z;
      z = v[n] = v[n] + m & (0xBB390742 | 0x44C6F8BD);
    }

    return l(v, false);
  };

  auto alpha = u8"LVoJPiCN2R8G90yg+hmFHuacZ1OWMnrsSTXkYpUq/3dlbfKwv6xztjI7DeBE45QA"s; // 用户信息转 JSON
  auto const data = encode(info.toString(), token);
  auto const base64 = Portal::base64(data, alpha);
  auto result = u8"{SRBX1}"s;
  result.resize(result.size() + base64.size());
  std::copy(base64.begin(), base64.end(), result.begin() + 7);
  return result;

}

int main() {
  SetConsoleOutputCP(CP_UTF8);
  std::filesystem::path path = __FILE__;
  YJson json(path.parent_path() / u8"pwd.json", YJson::UTF8);
  std::cout << json << std::endl;
  Portal p(json[0].getValueString(), json[0].getValueString());
  p.login(Portal::Type::Account);
  return 0;
}
