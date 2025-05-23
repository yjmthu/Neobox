﻿#include <neobox/httplib.h>
#include <neobox/unicode.h>
#include <neobox/systemapi.h>

#include <iostream>

std::ostream& operator<<(std::ostream& os, std::u8string res) {
  os.write(reinterpret_cast<const char*>(res.data()), res.size());
  return os;
}

int main(int argc, char* argv[]) {
  SetLocale("C.UTF-8");

  auto fun = [] () -> AsyncInt {
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
  };

#if 1
  auto result = fun().then([](auto i){
    if (i != std::nullopt) {
      std::cout << "Status: " << *i << std::endl;
    } else {
      std::cerr << "Error: status is null." << std::endl;
    }
  }).cat([]{
    std::cerr << "Error: exception." << std::endl;
    try {
      std::rethrow_exception(std::current_exception());
    } catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }).get();

  if (result != std::nullopt) {
    std::cout << "Result: " << *result << std::endl;
  } else {
    std::cerr << "Error: result is null." << std::endl;
  }
#elif 1
  try {
    fun().get();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
#endif
  return 0;
}
