#include< iostream>
#include <coroutine>

struct task {
  struct promise_type;
};

struct task::promise_type {
  auto get_return_object() -> task { return {}; }

  auto initial_suspend() -> std::suspend_always { return {}; }

  // finalsuspend必须定义为 noexcept
  auto final_suspend() noexcept -> std::suspend_always { return {}; }

  auto unhandled_exception() -> void {}
};

auto call() -> task {
  std::cout << "call" << std::endl;
  co_await std::suspend_always{};
};

auto main() -> int {
  auto co = call();

  return 0;
}