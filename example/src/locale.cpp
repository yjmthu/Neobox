#include <iostream>
#include <filesystem>

int main(int argc, char* argv[]) {
  if (setlocale(LC_ALL, "zh_CN.UTF-8") == nullptr) {
    std::cerr << "Error: setlocale failed." << std::endl;
    return 1;
  }

  std::cout << "你好，世界！" << std::endl;

  std::filesystem::path p = "测试中文.txt";
  std::cout << p.string() << std::endl;
  return 0;
}
