#include <filesystem>
#include <thread>
#include <chrono>
#include <format>
#include <systemapi.h>

#include <Windows.h>

using namespace std::literals;
namespace fs = std::filesystem;

int main(int argc, char*argv[]) {
  if (argc != 2) {
    MessageBoxW(nullptr, L"参数个数错误！", L"出错", MB_OK | MB_ICONERROR);
    return 0;
  }
  std::this_thread::sleep_for(1s);
  fs::path desDir = argv[1];
  desDir.make_preferred();

  try {
    fs::path resDir = fs::absolute(argv[0]).parent_path();
    fs::current_path(desDir.parent_path());

    if (fs::exists(desDir)) {
      fs::remove_all(desDir);
    }

    fs::rename(resDir, desDir);

    fs::path exeFilePath = desDir / "neobox.exe";
    auto exeFile = exeFilePath.wstring();
    exeFile.push_back(L'\0');
    ShellExecuteW(nullptr, L"open", exeFile.data(), L"new", nullptr, SW_SHOWNORMAL);
  } catch (fs::filesystem_error err) {
    auto message = std::format(
      L"文件操作出错！\n错误描述：{}\n错误信息：{}\n错误码：{}。",
      Ansi2WideString(err.what()),
      Ansi2WideString(err.code().message()),
      err.code().value()
    );
    MessageBoxW(nullptr, message.c_str(), L"出错", MB_OK | MB_ICONERROR);
  }
  return 0;
}
