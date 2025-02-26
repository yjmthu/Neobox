#include <neobox/httplib.h>
#include <iostream>

std::ostream& operator<<(std::ostream& os, std::u8string_view res) {
  os.write(reinterpret_cast<const char*>(res.data()), res.size());
  return os;
}

HttpAction<int> GetBaidu() {
  std::cout << "Begin." << std::endl;
  HttpLib clt(HttpUrl(u8"https://www.baidu.com"), true);
  auto res = co_await clt.GetAsync();
  if (res->status == 200) {
    std::cout << res->body << std::endl;
  } else {
    std::cerr << "Error: " << res->status << std::endl;
  }
  throw std::runtime_error("An error occurred in coroutine");

  co_return res->status;
}

HttpAction<void> PrintBaidu() {
  auto& result = GetBaidu().cat([]{
    std::cerr << "Error: exception." << std::endl;
    try {
      std::rethrow_exception(std::current_exception());
    } catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  });
  auto i = co_await result.awaiter();
  if (i == std::nullopt) {
    std::cerr << "Error: status is null." << std::endl;
  } else {
    std::cout << "Status: " << *i << std::endl;
  }
}

int main(int argc, char* argv[]) {
  PrintBaidu().then([]{
    std::cout << "Finished." << std::endl;
  }).cat([]{
    std::cerr << "Error: exception." << std::endl;
    try {
      std::rethrow_exception(std::current_exception());
    } catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }).get();
  return 0;
}
