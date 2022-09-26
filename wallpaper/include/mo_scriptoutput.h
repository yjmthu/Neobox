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

/* export */ class ScriptOutput : public WallBase {
 public:
  explicit ScriptOutput() : WallBase() { InitBase(); }
  ~ScriptOutput() override {}
  bool LoadSetting() override {
    if (fs::exists(m_SettingPath)) {
      m_pSetting = new YJson(m_SettingPath, YJson::UTF8);
      m_u8strCommand = m_pSetting->find(u8"executeable"s)->second.getValueString();
      auto range = m_pSetting->find(u8"arglist"s)->second.getArray() |
                   std::views::transform(
                       [](const YJson& item) { return item.getValueString(); });
      m_ArgList.assign(range.begin(), range.end());
      if (auto iter = m_pSetting->find(u8"imgdir"); iter != m_pSetting->endO()) {
        m_ImageDir = iter->second.getValueString();
      } else {
        m_ImageDir = ms_HomePicLocation / L"脚本获取";
        m_pSetting->getObject().emplace_back(u8"imgdir"s, m_ImageDir.u8string());
        m_pSetting->toFile(m_SettingPath);
      }
      return true;
    }
    return false;
  }
  bool WriteDefaultSetting() override {
    m_pSetting = new YJson(YJson::Object);
    m_pSetting->append(m_u8strCommand, u8"executeable"s);
    m_ImageDir = ms_HomePicLocation / L"脚本获取";
    auto item = m_pSetting->append(YJson::Array, u8"arglist"s);
    for (auto& i: m_ArgList) {
      item->second.append(i);
    }
    m_pSetting->getObject().emplace_back(u8"imgdir"s, m_ImageDir.u8string());
    m_pSetting->toFile(m_SettingPath);
    return true;
  }
  ImageInfoEx GetNext() override {
    ImageInfoEx ptr(new ImageInfo);
    if (m_u8strCommand.empty()) {
      ptr->ErrorMsg = u8"Invalid command to get wallpaper path."s;
      ptr->ErrorCode = ImageInfo::CfgErr;
      return ptr;
    }
#ifdef _WIN32
    std::vector<std::wstring> result;
    std::wstring wcmd = Utf82WideString(GetCommandWithArg());
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
    ptr->ImagePath = std::move(str);
    ptr->ErrorCode = ImageInfo::NoErr;
    return ptr;
  }

  YJson* GetJson() override {
    return m_pSetting;
  }

  void SetCurDir(const std::u8string &sImgDir) override {
    m_ImageDir = sImgDir;
    m_pSetting->find(u8"imgdir")->second = sImgDir;
    m_pSetting->toFile(m_SettingPath);
  }

  void SetJson(bool update) override {
    m_u8strCommand = m_pSetting->find(u8"executeable"s)->second.getValueString();
    auto range = m_pSetting->find(u8"arglist"s)->second.getArray() |
                 std::views::transform([](const YJson& item) -> std::u8string {
                   return item.getValueString();
                 });
    m_ArgList.assign(range.begin(), range.end());
    m_pSetting->toFile(m_SettingPath);
  }

 private:
  const char m_SettingPath[19]{"ScriptCommand.json"};
  YJson* m_pSetting;
  std::u8string m_u8strCommand = u8"python.exe";
  std::vector<std::u8string> m_ArgList = { u8"scripts/getpic.py" };
  std::u8string GetCommandWithArg() const {
    std::u8string sExeWithArgs;
    auto fnCheckSpaces = [&sExeWithArgs](const auto& str) {
      if (u8"\"\'"sv.find(str.front()) == 
          std::string_view::npos &&
          str.find(u8' ') != std::u8string::npos)
      {
        sExeWithArgs.push_back(u8'"');
        sExeWithArgs.append(str);
        sExeWithArgs.push_back(u8'"');
      } else {
        sExeWithArgs.append(str);
      }
      sExeWithArgs.push_back(u8' ');
    };
    fnCheckSpaces(m_u8strCommand);
    for (auto& arg : m_ArgList) {
      fnCheckSpaces(arg);
    }
    if (sExeWithArgs.ends_with(u8' ')) sExeWithArgs.pop_back();
    return sExeWithArgs;
  }
};
