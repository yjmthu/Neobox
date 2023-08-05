#include <iostream>
#include <httplib.h>
#include <yjson.h>
#include <systemapi.h>

#ifdef _WIN32
#include <Windows.h>
#endif

std::u8string get_access_token(const std::u8string &AK, const std::u8string &SK) {
  HttpLib clt(
    HttpUrl(u8"https://aip.baidubce.com/oauth/2.0/token?", {
    {u8"grant_type", u8"client_credentials"},
    {u8"client_id", AK},
    {u8"client_secret", SK}
  }));
  auto res = clt.Get();
  if (res->status != 200)
    return std::u8string(res->body.begin(), res->body.end());
  YJson json(res->body.begin(), res->body.end());
  return json[u8"access_token"].getValueString();
}

std::ostream& operator<<(std::ostream& o, const std::u8string& data) {
  return o.write(reinterpret_cast<const char*>(data.data()), data.size());
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
  SetConsoleOutputCP(65001);
#endif
  if (argc == 2) {
    std::u8string key = Ansi2Utf8String(argv[1]);
    std::cout << "your token is 【" << get_access_token(u8"enwvUXsYtstHiKPDOPShBOuE", key) << "】.\n";
  } else {
    std::cout << "you should parse the 'Secret Key' to this executable.\n";
  }
  return 0;
}
