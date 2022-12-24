#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

void print(const fs::path& p1, const fs::path& p2)
{
  if (fs::equivalent(p1, p2)) {
    std::cout << p1.string() << " == " << p2.string();
  } else {
    std::cout << p1.string() << " != " << p2.string();
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