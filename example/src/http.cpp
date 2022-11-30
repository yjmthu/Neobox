#include <httplib.h>
#include <fstream>
#include <iostream>
#include <string>

using namespace std::literals;

int main() {
  std::ofstream file( "https.txt", std::ios::out | std::ios::binary);
  // HttpLib lib("https://api.github.com/repos/yjmthu/Neobox/releases/latest"s);
  HttpLib lib("https://api.ixiaowai.cn/api/api.php"s);
#if 0
    auto res = get.Get("https://wallhaven.cc");
#else
  lib.SetHeader("User-Agent", "Dark Secret Ninja/1.0");
  auto res = lib.Get();
#endif
  if (res) {
    file.write(res->body.data(), res->body.size());
    file.put('\n');
  } else {
    file << std::to_string(res->status) << " No response.\n";
  }
  file.close();
  std::cout << "hreaders: <\n";
  for (auto& [x, y]: res->headers) {
    std::cout << x << std::endl;
  }
  std::cout << ">\n";
  return 0;
}
