#include <scriptoutput.h>

#include <wallpaper.h>
#include <wallbase.h>
#include <systemapi.h>

#include <ranges>
#include <utility>
#include <numeric>
#include <functional>
#include <filesystem>

// export module wallpaper5;

namespace fs = std::filesystem;
using namespace std::literals;

ScriptOutput::ScriptOutput(YJson& setting):
  WallBase(InitSetting(setting))
{
}

ScriptOutput::~ScriptOutput()
{
  //
}

YJson& ScriptOutput::InitSetting(YJson& setting)
{
  if (setting.isObject())
    return setting;
  setting = YJson::O {
    {u8"curcmd", u8"默认脚本"},
    {u8"cmds", YJson::O {
      {u8"默认脚本", YJson::O{
        {u8"command", u8"python.exe \"scripts/getpic.py\""},
        {u8"directory", GetStantardDir(u8"脚本获取")}
      }}
    }}
  };
  SaveSetting();
  return setting;
}

void ScriptOutput::GetNext(std::function<void(ImageInfoEx)> callback)
{
  ImageInfoEx ptr(new ImageInfo);
  m_DataMutex.lock();
  auto const & u8cmd = GetCurInfo()[u8"command"].getValueString();
  m_DataMutex.unlock();

  if (u8cmd.empty()) {
    ptr->ErrorMsg = u8"Invalid command to get wallpaper path."s;
    ptr->ErrorCode = ImageInfo::CfgErr;
    callback(ptr);
    return;
  }
#ifdef _WIN32
  std::vector<std::wstring> result;
  const auto wcmd = Utf82WideString(u8cmd);

  GetCmdOutput(wcmd.c_str(), result);

  if (result.empty()) {
    ptr->ErrorMsg = u8"Invalid command to get wallpaper path."s;
    ptr->ErrorCode = ImageInfo::CfgErr;
    callback(ptr);
    return;
  }
  auto str = Wide2Utf8String(result.front());
#elif defined (__linux__)
  std::vector<std::u8string> result;
  GetCmdOutput(reinterpret_cast<const char*>(u8cmd.c_str()), result);
  if (result.empty())
    ptr->ErrorMsg = u8"Run command with empty output."s;
  ptr->ErrorCode = ImageInfo::RunErr;
  return ptr;
  auto& str = result.front();
#endif
  if (str.empty()) {
    ptr->ErrorMsg = u8"Run command with wrong output."s;
    ptr->ErrorCode = ImageInfo::RunErr;
    callback(ptr);
    return;
  }
  if (!fs::exists(str)) {
#ifdef _WIN32
    auto wErMsg = std::accumulate(result.begin(), result.end(),
      L"程序运行输出不匹配，请确保输出图片路径！\n"s,
      [](const std::wstring& a, const std::wstring& b) {
        return a + L'\n' + b;
      });
    ptr->ErrorMsg = Wide2Utf8String(wErMsg);
#else
    auto wErMsg = std::accumulate(result.begin(), result.end(),
      u8"程序运行输出不匹配，请确保输出图片路径！\n"s,
      [](const std::u8string& a, const std::u8string& b) {
        return a + u8'\n' + b;
      });
    ptr->ErrorMsg = std::move(wErMsg);
#endif
    ptr->ErrorCode = ImageInfo::RunErr;
    callback(ptr);
    return;
  }
  ptr->ImagePath = std::move(str);
  ptr->ErrorCode = ImageInfo::NoErr;
  callback(ptr);
  return;
}

YJson& ScriptOutput::GetCurInfo() 
{
  return m_Setting[u8"cmds"][m_Setting[u8"curcmd"].getValueString()];
}
