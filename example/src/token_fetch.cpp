#include <iostream>
#include <httplib.h>
#include <yjson.h>

#ifdef _WIN32
#include <Windows.h>
#endif

std::u8string get_access_token(const std::string &AK, const std::string &SK) {
  HttpLib clt(std::format("https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id={}&client_secret={}", AK, SK));
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
    std::cout << "your token is 【" << get_access_token("enwvUXsYtstHiKPDOPShBOuE", argv[1]) << "】.\n";
  } else {
    std::cout << "you should parse the 'Secret Key' to this executable.\n";
  }
  return 0;
}
