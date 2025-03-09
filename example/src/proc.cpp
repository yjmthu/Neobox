#include <neobox/process.h>
#include <neobox/unicode.h>
#include <iostream>
#include <filesystem>

static std::ostream& operator<<(std::ostream& o, std::u8string_view data)
{
  return o.write(reinterpret_cast<const char*>(data.data()), data.size());
}

AsyncVoid Run(std::filesystem::path app, std::u8string args)
{
  NeoProcess process(app, args);
  auto re = co_await process.Run().awaiter();
  if (re && *re == 0) {
    std::cout << "Process finished successfully." << std::endl;

    std::cout << "StdOut: <\n" << process.GetStdOut() << ">." << std::endl;
    std::cout << "StdError: <\n" << process.GetStdErr() << ">." << std::endl;
  } else if (re) {
    std::cout << "Process finished with error code: " << *re << std::endl;
  } else {
    std::cout << "Process failed." << std::endl;
  }
}

int main()
{
  auto const argv = GetUtf8Argv();
  int const argc = argv.size();

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <app> [args]" << std::endl;
    return 1;
  }

  SetLocale("zh_CN.UTF-8");

  std::u8string app;
  std::u8string args;

  if (argc > 1) {
    std::cout << "App: " << argv[1] << std::endl;
    app = argv[1];
  }
  if (argc > 2) {
    std::cout << "Args: " << argv[2] << std::endl;
    args = argv[2];
  }

  auto coro = Run(app, args);
  std::cout << "Running..." << std::endl;
  coro.get();
  std::cout << "Done." << std::endl;

  return 0;
}

