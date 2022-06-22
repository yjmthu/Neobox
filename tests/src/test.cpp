#include <sysapi.h>

#include <iostream>

int main() {
  std::vector<std::string> data;
#if __linux__
  GetCmdOutput<char>("./tests/main", data);
#else
  GetCmdOutput<char>("./build/tests/Debug/test_main.exe", data);
#endif
  for (auto& i : data) {
    for (auto j : i) {
      std::cout << int(j) << ' ';
    }
    std::cout << std::endl;
  }
  std::cout << "\\r: " << int('\r') << " \\n: " << int('\n') << std::endl;
  return 0;
}

/*
output is:

81 65 66 67 68 32 83 65 70 68 72 32 70 70 70 32 48 48 10
72 69 76 76 79 32 87 79 82 76 68 10
\r: 13 \n: 10

*/
