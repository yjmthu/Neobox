#include "portal.h"

#include <Windows.h>
#include <filesystem>
#include <yjson/yjson.h>

int main() {
  SetConsoleOutputCP(CP_UTF8);

  std::filesystem::path path = __FILE__;
  YJson json(path.parent_path() / u8"pwd.json", YJson::UTF8);
  Portal p(json[0].getValueString(), json[1].getValueString());

  std::cout << "Press any key to login\n";
  std::cin.get();
  p.login(Portal::Type::Account);
  // std::this_thread::sleep_for(5s);
  std::cout << "Press any key to logout\n";
  std::cin.get();
  p.logout();
  return 0;
}
