#include <httplib.h>
#include <fstream>
#include <string>

using namespace std::literals;

int main() {
  std::ofstream file( "https.txt", std::ios::out | std::ios::binary);
  HttpLib lib("https://api.github.com/repos/yjmthu/Neobox/releases/latest"s);
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
  return 0;
}
