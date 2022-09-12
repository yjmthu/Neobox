#include <translate.h>
#include <iostream>
#include <sysapi.h>
#include <yjson.h>

int main()
{
  Translate tran;
  tran.SetToLanguage(Translate::Lan::ZH_CN);
  SetConsoleOutputCP(65001);
  auto res = tran.GetResult(u8"Hello");
  if (!res.empty()) {
    std::cout << res;
  } else {
    std::cout << res << "\nPOST Failed.\n";
  }
  return 0;
}
