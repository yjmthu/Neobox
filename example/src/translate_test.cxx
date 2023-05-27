#include <systemapi.h>
#include <translate.h>
#include <yjson.h>
#include <iostream>

using namespace std::literals;

int main() {
  YJson setting = YJson::O{
    {u8"PairBaidu", YJson::A {0, 0}},
    {u8"PairYoudao", YJson::A {0, 0}},
  };
  SetConsoleOutputCP(65001);
  Translate tran(setting, [](const void* data, size_t size){
    std::cout.write(reinterpret_cast<const char*>(data), size);
    if (!size) std::cout << "\nPOST Failed.\n";
  });
  auto const& res = tran.GetResult("翻译结果"s);
  return 0;
}
