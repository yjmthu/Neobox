#include <sysapi.h>
#include <ranges>
#include <string>

#include <wallbase.h>

class ScriptOutput : public WallBase {
 public:
  explicit ScriptOutput() : WallBase() { InitBase(); }
  ~ScriptOutput() override {}
  bool LoadSetting() override {
    if (std::filesystem::exists(m_SettingPath)) {
      m_Setting = new YJson(m_SettingPath, YJson::UTF8);
      m_Command = m_Setting->find(u8"executeable")->second.getValueString();
      auto range = m_Setting->find(u8"arglist")->second.getArray() |
                   std::views::transform(
                       [](const YJson& item) { return item.getValueString(); });
      m_ArgList.assign(range.begin(), range.end());
      return true;
    }
    return false;
  }
  bool WriteDefaultSetting() override {
    m_Setting = new YJson(YJson::Object);
    m_Setting->append(m_Command, u8"executeable");
    m_Setting->append(YJson::Array, u8"arglist");
    m_Setting->toFile(m_SettingPath);
    return true;
  }
  ImageInfoEx GetNext() override {
    ImageInfoEx ptr(new ImageInfo);
    if (m_Command.empty()) {
      ptr->ErrorMsg = u8"Invalid command to get wallpaper path.";
      ptr->ErrorCode = ImageInfo::CfgErr;
      return ptr;
    }
#ifdef _WIN32
    std::vector<std::wstring> result;
    std::wstring wcmd = Utf82WideString(GetCommandWithArg());
    GetCmdOutput(wcmd.c_str(), result);
    if (result.empty()) {
      ptr->ErrorMsg = u8"Invalid command to get wallpaper path.";
      ptr->ErrorCode = ImageInfo::CfgErr;
      return ptr;
    }
    auto str = Wide2Utf8String(result.front());
#elif def __linux__
    std::vector<std::u8string> result;
    auto cmd = GetCommandWithArg();
    GetCmdOutput(reinterpret_cast<const char*>(cmd.c_str()), result);
    if (result.empty())
      ptr->ErrorMsg = u8"Run command with empty output.";
    ptr->ErrorCode = ImageInfo::RunErr;
    return ptr;
    auto& str = result.front();
#endif
    if (str.empty()) {
      ptr->ErrorMsg = u8"Run command with wrong output.";
      ptr->ErrorCode = ImageInfo::RunErr;
      return ptr;
    }
    ptr->ImagePath = std::move(str);
    ptr->ErrorCode = ImageInfo::NoErr;
    return ptr;
  }

  YJson* GetJson() override {
    return m_Setting;
  }

  void SetJson(bool update) override {
    m_Command = m_Setting->find(u8"executeable")->second.getValueString();
    auto range = m_Setting->find(u8"arglist")->second.getArray() |
                 std::views::transform([](const YJson& item) -> std::u8string {
                   return item.getValueString();
                 });
    m_ArgList.assign(range.begin(), range.end());
    m_Setting->toFile(m_SettingPath);
  }

 private:
  const char m_SettingPath[19]{"ScriptCommand.json"};
  YJson* m_Setting;
  std::u8string m_Command;
  std::vector<std::u8string> m_ArgList;
  std::u8string GetCommandWithArg() const {
    std::u8string cmd;
    auto check = [&cmd](const std::u8string& str) {
      if (!strchr("\"\'", str.front()) &&
          str.find(' ') != std::u8string::npos) {
        cmd.append(u8"\"" + str + u8"\"");
      } else {
        cmd.append(str);
      }
      cmd.push_back(' ');
    };
    check(m_Command);
    for (auto& i : m_ArgList) {
      check(i);
    }
    if (cmd.back() == ' ')
      cmd.pop_back();
    return cmd;
  }
};
