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
  auto initDirPath = ms_HomePicLocation / u8"脚本获取";
  initDirPath.make_preferred();
  auto const initDir = initDirPath.u8string();
  return setting = YJson::O {
    {u8"curcmd", u8"默认脚本"},
    {u8"cmds", YJson::O {
      {u8"默认脚本", YJson::O{
        {u8"command", u8"python.exe \"scripts/getpic.py\""},
        {u8"directory", initDir}
      }}
    }}
  };
}

ImageInfoEx ScriptOutput::GetNext()
{
  ImageInfoEx ptr(new ImageInfo);
  auto const & u8cmd = GetCurInfo()[u8"command"].getValueString();

  if (u8cmd.empty()) {
    ptr->ErrorMsg = u8"Invalid command to get wallpaper path."s;
    ptr->ErrorCode = ImageInfo::CfgErr;
    return ptr;
  }
#ifdef _WIN32
  std::vector<std::wstring> result;
  const std::wstring wcmd = Utf82WideString(u8cmd);
  GetCmdOutput(wcmd.c_str(), result);
  if (result.empty()) {
    ptr->ErrorMsg = u8"Invalid command to get wallpaper path."s;
    ptr->ErrorCode = ImageInfo::CfgErr;
    return ptr;
  }
  auto str = Wide2Utf8String(result.front());
#elif def __linux__
  std::vector<std::u8string> result;
  auto cmd = GetCommandWithArg();
  GetCmdOutput(reinterpret_cast<const char*>(cmd.c_str()), result);
  if (result.empty())
    ptr->ErrorMsg = u8"Run command with empty output."s;
  ptr->ErrorCode = ImageInfo::RunErr;
  return ptr;
  auto& str = result.front();
#endif
  if (str.empty()) {
    ptr->ErrorMsg = u8"Run command with wrong output."s;
    ptr->ErrorCode = ImageInfo::RunErr;
    return ptr;
  }
  if (!fs::exists(str)) {
    auto wErMsg = std::accumulate(result.begin(), result.end(),
      L"程序运行输出不匹配，请确保输出图片路径！\n"s,
      [](const std::wstring& a, const std::wstring& b){
        return a + L'\n' + b;
      });
    ptr->ErrorMsg = Wide2Utf8String(wErMsg);
    ptr->ErrorCode = ImageInfo::RunErr;
    return ptr;
  }
  ptr->ImagePath = std::move(str);
  ptr->ErrorCode = ImageInfo::NoErr;
  return ptr;
}

YJson& ScriptOutput::GetCurInfo() 
{
  return m_Setting[u8"cmsd"][m_Setting[u8"curcmd"].getValueString()];
}

fs::path ScriptOutput::GetImageDir() const
{
  return GetCurInfo()[u8"directory"].getValueString();
}

void ScriptOutput::SetCurDir(const std::u8string &sImgDir)
{  
  GetCurInfo()[u8"directory"] = sImgDir;
  SaveSetting();
}

void ScriptOutput::SetJson(bool update)
{
  SaveSetting();
}
