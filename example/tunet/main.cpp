#include <iostream>
#include <chrono>
#include <yjson/yjson.h>
#include <neobox/httplib.h>
#include "localhost.hpp"
#include <QByteArray>

using namespace std::literals;

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
  enum class Type {
    Account
  } type;

  struct UserInfo {
    std::u8string username;
    std::u8string password;
    std::u8string domain;
    std::u8string ip;
  } userInfo;

  typedef std::function<void(std::u8string)> Callback;

  void login(Type type);

  std::u8string encode(YJson info, std::u8string token);

  void sendAuth(std::u8string host, bool defaultStack);

  void getToken(std::u8string host, std::u8string ip, Callback callback);
};

void Portal::login(Type type) {
  std::u8string host = u8"tsinghua.edu.cn";
  userInfo.ip = GetLocalIPAddressWithDNSSuffix(host);
  std::cout << std::string(userInfo.ip.begin(), userInfo.ip.end()) << std::endl;

  getToken(u8"auth4." + host, userInfo.ip, [](auto token) {
    std::cout << std::string(token.begin(), token.end()) << std::endl;
  });
}

void Portal::sendAuth(std::u8string host, bool defaultStack) {
  auto const ip = defaultStack ? userInfo.ip : u8""s;
}

void Portal::getToken(std::u8string host, std::u8string ip, Callback callback) {
  using namespace std::chrono;
  auto timeMs = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
  auto timestamp = std::to_string(timeMs.count());
  HttpUrl url(host, ApiList::token, {
    { u8"username"s, userInfo.username},
    { u8"ip"s, ip },
    { u8"_", std::u8string(timestamp.begin(), timestamp.end())},
    { u8"callback", u8"_"}
  }, u8"https", 443);
  std:std::u8string full = url.GetFullUrl();
  std::cout << std::string(full.begin(), full.end()) << std::endl;
  HttpLib clt(url, true);
  std::atomic_bool finished = false;
  HttpLib::Callback cb {
    .m_FinishCallback = [callback, &finished](auto msg, auto res) {
      if (msg.empty() && res->status == 200) {
        auto begin = res->body.find(u8'{');
        auto end = res->body.rfind(u8'}');
        YJson json(res->body.begin() + begin, res->body.begin() + end + 1);
        // std::cout << json;
        auto const token = json[u8"challenge"].getValueString();
        callback(token);
      } else {
        std::cerr << "Error: " << res->status << std::endl;
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
}

std::u8string Portal::encode(YJson info, std::u8string token) {
  // base64.setAlpha('LVoJPiCN2R8G90yg+hmFHuacZ1OWMnrsSTXkYpUq/3dlbfKwv6xztjI7DeBE45QA'); // 用户信息转 JSON

  auto s = [](std::u8string a, bool b) {
    auto const cc = a.length();
    auto c = cc;
    const auto remainder = c & 0b11;
    if (remainder) {
      c += 4 - remainder;
      a.resize(c, 0);
    }

    std::vector<char32_t> v(c >> 2, 0);

    for (size_t i = 0; i < cc; i += 4) {
      v[i >> 2] = a[i] | a[i + 1] << 8 | a[i + 2] << 16 | a[i + 3] << 24;
    }

    if (b) v.push_back(cc);
    return v;
  };

  auto l = [](std::vector<char32_t> a, bool b) {
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

  auto const code = encode(info.toString(), token);
  auto const base64 = QByteArray(reinterpret_cast<const char*>(code.data()), code.size()).toBase64();
  auto result = u8"{SRBX1}"s;
  result.resize(result.size() + base64.size());
  std::copy(base64.begin(), base64.end(), result.begin() + 7);
  return result;

}

int main() {
  Portal p;
  p.login(Portal::Type::Account);
  return 0;
}
