#include <neobox/process.h>
#include <neobox/systemapi.h>
#include <iostream>
#include <filesystem>

#include <Windows.h>

AsyncVoid Run(std::filesystem::path app, std::u8string args)
{
  NeoProcess process(app, args);
  auto re = co_await process.Run();
  if (re && *re == 0) {
    std::cout << "Process finished successfully." << std::endl;

    std::cout << "StdOut: <" << process.GetStdOut() << ">." << std::endl;
  } else {
    std::cout << "Process finished with error code: " << re << std::endl;
  }
}

int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <app> [args]" << std::endl;
    return 1;
  }

  SetConsoleOutputCP(CP_UTF8);

  std::string app;
  std::string args;

  if (argc > 1) {
    std::cout << "App: " << argv[1] << std::endl;
    app = argv[1];
  }
  if (argc > 2) {
    std::cout << "Args: " << argv[2] << std::endl;
    args = argv[2];
  }

  Run(app, Ansi2Utf8String(args)).get();

  return 0;
}

