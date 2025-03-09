#include <neobox/httplib.h>
#include <neobox/unicode.h>

std::ostream& operator<<(std::ostream& out, const std::u8string& str) {
  return out.write(reinterpret_cast<const char*>(str.data()), str.length());
}

int main() {
  SetLocale();

  HttpUrl url1(u8"http://www.baidu.com/?q=123&p=345", {
    {u8"sdd", u8"华中科技大学"}
  });
  std::cout << url1.GetUrl() << std::endl;

  HttpUrl url2(u8"https://www.baidu.com/?p=%E5%8D%8E%E4%B8%AD%E7%A7%91%E6%8A%80%E5%A4%A7%E5%AD%A6", {
    {u8"q", u8"首都师范大学"},
  });
  std::cout << url2.GetUrl() << std::endl;

  std::cout << "----------------\n";
  for (auto& [i, j]: url2.parameters) {
    std::cout << i << ": " << j << std::endl;
  }
  std::cout << "----------------\n";
  // 
  HttpUrl url3(u8"https://devapi.qweather.com/v7/weather/3d?", {
    {u8"key", u8"首都师范大学"},
  });

  std::cout << url3.scheme + u8"://" + url3.host << std::endl;
  std::cout << url3.GetObjectString() << std::endl;
  std::cout << url3.GetUrl() << std::endl;
}
