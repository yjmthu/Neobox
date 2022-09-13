#include <translate.h>
#include <iostream>
#include <sysapi.h>
#include <yjson.h>

int main()
{
  Translate tran;
  SetConsoleOutputCP(65001);
  auto res = tran.GetResult(u8"翻译结果");
  if (!res.empty()) {
    std::cout << res;
  } else {
    std::cout << res << "\nPOST Failed.\n";
  }
  return 0;
}
