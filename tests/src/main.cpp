#include <sysapi.h>
#include <windows.h>
#include <iostream>

int main() {
  std::cout << Utf82AnsiString(
      u8"/home/yjmthu/Pictures/桌面壁纸/wallhaven-m96o1m.jpg\n");
  return 0;
}
