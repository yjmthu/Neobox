#include <sysapi.h>
#include <iostream>
#include <string>

int main() {
  std::vector<std::basic_string<TCHAR>> data;
  GetCmdOutput(TEXT(".\\build-dbg\\tests\\mains.exe"), data);
  std::cout << data.size() << std::endl;
  for (auto i : data) {
#ifdef UNICODE
    std::wcout << i << std::endl;
#else
    std::cout << i << std::endl;
#endif
  }
  return 0;
}

/*
output is:

81 65 66 67 68 32 83 65 70 68 72 32 70 70 70 32 48 48 10
72 69 76 76 79 32 87 79 82 76 68 10
\r: 13 \n: 10

*/
