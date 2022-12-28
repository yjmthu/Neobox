#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

void print(const fs::path& p1, const fs::path& p2)
{
  std::error_code ec;
  if (fs::equivalent(p1, p2, ec)) {   // 两个路径必须同时存在
    std::cout << p1.string() << " equivalent " << p2.string() << std::endl;
  } else {
    std::cout << p1.string() << " not equivalent " << p2.string() << std::endl;
  }

  std::cout << "error code: " << ec.value() << "\n message: " << ec.message() << std::endl;

  if (p1 == p2) {                     // 不要求路径存在
    std::cout << p1.string() << " == " << p2.string() << std::endl;
  } else {
    std::cout << p1.string() << " != " << p2.string() << std::endl;
  }
}

int main()
{
  fs::path p1 = "123\\456";
  fs::path p2 = "123/456";
  fs::path p3 = "123/4567";
  print(p1, p1);
  print(p1, p2);
  print(p1, p3);
  print(p2, p3);
  return 0;
}