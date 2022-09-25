#include <systemapi.h>
#include <translate.h>
#include <yjson.h>
#include <iostream>

int main() {
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
