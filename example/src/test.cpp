#include <systemapi.h>
#include <iostream>
#include <string>

int main() {
  SetConsoleOutputCP(65001);
  std::vector<std::wstring> data;
  GetCmdOutput(TEXT("python.exe scripts/getpic.py"), data);
  for (auto i : data) {
    auto str = Wide2Utf8String(i);
    std::cout.write((const char*)str.data(), str.size());
    std::cout.put('\n');
  }
  return 0;
}

/*
output is:

81 65 66 67 68 32 83 65 70 68 72 32 70 70 70 32 48 48 10
72 69 76 76 79 32 87 79 82 76 68 10
\r: 13 \n: 10

*/
