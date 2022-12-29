#include <systemapi.h>
#include <translate.h>
#include <yjson.h>
#include <iostream>

int main() {
  YJson setting = YJson::O{
    {u8"PairBaidu", YJson::A {0, 0}},
    {u8"PairYoudao", YJson::A {0, 0}},
  };
  Translate tran(setting);
  SetConsoleOutputCP(65001);
  auto res = tran.GetResult(u8"翻译结果");
  if (!res.empty()) {
    std::cout << res;
  } else {
    std::cout << res << "\nPOST Failed.\n";
  }
  return 0;
}
