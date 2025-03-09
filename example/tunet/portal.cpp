#include <portal.h>

#include <yjson/yjson.h>
#include <neobox/httplib.h>
#include <QByteArray>
#include <QCryptographicHash>

#include <chrono>
#include <regex>

using namespace std::literals;

static std::ostream& operator<<(std::ostream& os, const std::u8string& res) {
  os.write(reinterpret_cast<const char*>(res.data()), res.size());
  return os;
}

static std::ostream& operator<<(std::ostream& os, std::u8string_view res) {
  os.write(reinterpret_cast<const char*>(res.data()), res.size());
  return os;
}

template <typename Char>
static std::u8string operator+(std::u8string a, std::basic_string_view<Char> b) {
  return a.append(b.begin(), b.end());
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

struct UnicodeSearch {
  std::match_results<std::string_view::const_iterator> match;

  bool match_view(std::string_view str, std::string re) {
    return std::regex_search(str.begin(), str.end(), match, std::regex(re));
  }

  auto view(size_t n) {
    auto& res = match[n];
    return std::pair { res.first, res.second };
  }
};

std::optional<YJson> Portal::parseJson(HttpResponse* res) {
  if (res->status != 200) {
    return std::nullopt;
  }
  auto& data = res->body;

  auto i = data.find(u8'{'), j = data.rfind(u8'}');
  if (i == data.npos || j == data.npos) {
    return std::nullopt;
  }
  return YJson(data.begin() + i, data.begin() + j + 1);
}

AsyncVoid Portal::init() {
  
  UnicodeSearch search;
  
  auto res = co_await client.GetAsync();

  if (res->status != 200) {
    std::cerr << "Can not go to <" << client.GetUrl() << ">\n";
    co_return;
  }

  // std::cout << res->body << std::endl;
  if (search.match_view(res->body, R"((URL|href)="?(https://.+?index_\d+\.html))")) {
    auto url = search.view(0);
    std::cout << "Find URL: <" << std::string(url.first, url.second) << ">\n";
    auto [first, second] = search.view(2);
    client.SetUrl(std::u8string(first, second));
    res = co_await client.GetAsync();
    if (res->status != 200) {
      std::cerr << "Can not go to <" << client.GetUrl() << ">\n";
      co_return;
    }
  }

  // std::cout << res->body << std::endl;
  if (search.match_view(res->body, R"(url=(.*ac_id=(\d+)[^"]*))")) {
    auto [m, n] = search.view(1);
    auto url = u8"https://" + subHost + std::u8string(m, n);
    auto [i, j] = search.view(2);
    userInfo.acID.assign(i, j);
    std::cout << "Find ac_id: " << userInfo.acID << std::endl
              << "URL: " << url << std::endl;
    client.SetUrl(url);
    res = co_await client.GetAsync();
    if (res->status != 200) {
      std::cerr << "Can not go to <" << client.GetUrl() << ">\n";
      co_return;
    }
  } else {
    std::cerr << "Can not find ac_id.\n" << res->body << std::endl;
    co_return;
  }

  // std::cout << res->body << std::endl;
  if (search.match_view(res->body, R"(CONFIG = (\{[^}]+\}))")) {
    auto [i, j] = search.view(1);
    auto config = std::string(i, j);
    if (search.match_view(config, R"(ip\s*:\s*"([^"]+)\")")) {
      auto [m, n] = search.view(1);
      userInfo.ip.assign(m, n);
    } else {
      std::cerr << "Can not find ip.\n";
    }
  } else {
    std::cerr << "Can not find CONFIG.\n";
  }
}

HttpAwaiter Portal::getInfo() {
  HttpUrl url(subHost, ApiList::info, {
    { u8"callback", u8"_" },
    { u8"ip", userInfo.ip },
    { u8"_", getTimestamp() }
  }, u8"https", 443);

  client.SetUrl(std::move(url));

  HttpLib::Callback cb {
    .onFinish = [this](auto msg, auto res) {
      if (msg.empty() && res->status == 200) {
        auto i = res->body.find(u8'{'), j = res->body.rfind(u8'}');
        if (i == res->body.npos || j == res->body.npos) {
          std::cerr << "Can not find json content.\n";
          return;
        }
        YJson json(res->body.begin() + i, res->body.begin() + j + 1);
        auto& err = json[u8"error"];
        userInfo.isLogin = err.isString() && err.getValueString() == u8"ok";
        std::cout << json << std::endl;
      } else {
        userInfo.isLogin = false;
        std::cerr << "Connect Error: " << res->status << std::endl;
      }
    },
  };

  return client.GetAsync(std::move(cb));
}

AsyncVoid Portal::login(Type type) {
  if (userInfo.ip.empty()) {
    std::cerr << "Network not found.\n";
    co_return;
  }
  std::cout << "ip: " << std::string(userInfo.ip.begin(), userInfo.ip.end()) << std::endl;
  auto info = co_await getInfo();
  if (userInfo.isLogin) {
    std::cout << "Already login\n";
    co_return;
  }
  userInfo.isLogin = true;

  auto res = co_await getToken(userInfo.ip);
  auto json = parseJson(res);

  if (!json) {
    std::cerr << "Can not get token.\n";
    co_return;
  }

  res = co_await sendAuth(parseToken(*json));
  json = parseJson(res);
  
  if (!json) {
    std::cerr << "Can not get token.\n";
    co_return;
  }

  std::cout << *json << std::endl;
}

AsyncVoid Portal::logout() {

  if (!userInfo.isLogin) {
    std::cout << "Already logout\n";
    co_return;
  }

  userInfo.isLogin = false;

  if (userInfo.ip.empty()) {
    std::cerr << "Network not found.\n";
    co_return;
  }

  // current time in date
  auto time = std::chrono::system_clock::now();
  auto date = std::chrono::duration_cast<std::chrono::days>(time.time_since_epoch());
  auto s = std::to_string(date.count() / 1000);
  auto stamp = std::u8string(s.begin(), s.end());
  auto const unbind = u8"1"s, str = stamp + userInfo.username + userInfo.ip + unbind + stamp;
  auto sha1 = QCryptographicHash::hash(QByteArray(reinterpret_cast<const char*>(str.data()), str.size()), QCryptographicHash::Sha1).toHex();
  std::u8string sign(sha1.begin(), sha1.end());

  HttpUrl url(subHost, ApiList::loginDM, {
    { u8"callback", u8"_" },
    { u8"ip", userInfo.ip },
    { u8"username", userInfo.username },
    { u8"time", stamp },
    { u8"unbind", unbind },
    { u8"sign", sign },
    { u8"_", getTimestamp() }
  }, u8"https", 443);

  client.SetUrl(std::move(url));

  auto res = co_await client.GetAsync();
  
  std::cout << res->body << std::endl;
}

HttpAwaiter Portal::sendAuth(std::u8string_view token) {
  std::cout << "Token: " << std::string(token.begin(), token.end()) << std::endl;
  std::u8string const n = u8"200", type = u8"1";
  YJson info = YJson::O {
    { u8"username", userInfo.username },
    { u8"password", userInfo.password },
    { u8"ip", userInfo.ip },
    { u8"acid", userInfo.acID },
    { u8"enc_ver", u8"srun_bx1" },
  };

  const auto i = encodeUserInfo(info, token);
  std::cout << "Info: " << info << "\nEncode: " << (const char*)i.c_str() << std:: endl; 
  
  std::u8string str(token);
  str += userInfo.username;
  std::u8string hmd5 = this->hmd5(userInfo.password, token);
  str += token;
  str += hmd5;
  str += token;
  str += userInfo.acID;
  str += token;
  str += userInfo.ip;
  str += token;
  str += n;
  str += token;
  str += type;
  str += token;
  str += i;

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

  client.SetUrl(std::move(url));

  return client.GetAsync();
}

HttpAwaiter Portal::getToken(std::u8string_view ip) {
  // if (!this->timestamp.empty()) {
  //   callback(this->token);
  //   return;
  // }
  auto timestamp = getTimestamp();
  HttpUrl url(subHost, ApiList::token, {
    { u8"username"s, userInfo.username },
    { u8"ip"s, std::u8string(ip) },
    { u8"_", timestamp },
    { u8"callback", u8"_" }
  }, u8"https", 443);

  client.SetUrl(std::move(url));
  return client.GetAsync();
}

std::u8string_view Portal::parseToken(YJson& json) {
  return json[u8"challenge"].getValueString();
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

std::u8string Portal::base64(std::u8string data, const std::u8string& alpha) {
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

std::u8string Portal::encodeUserInfo(const YJson& info, std::u8string_view token) {
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
  auto const data = encode(info.toString(), std::u8string(token));
  auto const base64 = Portal::base64(data, alpha);
  auto result = u8"{SRBX1}"s;
  result.resize(result.size() + base64.size());
  std::copy(base64.begin(), base64.end(), result.begin() + 7);
  return result;

}

