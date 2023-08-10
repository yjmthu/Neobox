#include <systemapi.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <iostream>

int main() {
#ifdef _WIN32
  SetConsoleOutputCP(65001);
#endif
  std::cout << "/home/yjmthu/Pictures/桌面壁纸/wallhaven-m96o1m.jpg\n";
  return 0;
}
