#include <regex>
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc != 2) return 0;
  auto str = std::string(argv[1]);
  std::regex pattern("wallhaven-([a-z0-9]{6})");
  std::cout << "string: " << argv[1] << "\npattern: wallhaven-([a-z0-9]{6})" << std::endl;
  std::smatch result;
  if (std::regex_search(str, result, pattern)) {
    std::cout << "result: <" << result.str() << "> ; <" << result[1] << ">.\n";
  } else {
    std::cout << "bad search.\n";
  }
  return 0;
}
