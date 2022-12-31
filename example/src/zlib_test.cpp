// #include <zlib.h>
#include <iostream>
#include <string>
#include <format>
#include <filesystem>

namespace fs = std::filesystem;

int main()
{
  std::string res, des;
  std::cout << "please input the '.zip' path:\n";
  std::getline(std::cin, res);
  std::cout << "please input the output folder path:\n";
  std::getline(std::cin, des);
  // std::cout << std::format("res's size is {}, dst's size is {}.\n", res.size(), des.size());
  if (!fs::exists(res)) {
    std::cout << "input file not exsists.";
    return 0;
  }
  // auto zFile =
  return 0;
}

