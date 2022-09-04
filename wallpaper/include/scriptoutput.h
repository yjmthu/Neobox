#include <sysapi.h>
#include <string>

#include "wallbase.h"

class ScriptOutput : public WallBase {
 public:
  explicit ScriptOutput(const std::filesystem::path& picHome)
      : WallBase(picHome) {
    InitBase();
  }
  ~ScriptOutput() override {}
  bool LoadSetting() override {
    if (std::filesystem::exists(m_SettingPath)) {
      m_Setting = new YJson(m_SettingPath, YJson::UTF8);
      m_Command = m_Setting->find(u8"executeable")->second.getValueString();
      for (auto& i : m_Setting->find(u8"arglist")->second.getArray()) {
        m_ArgList.emplace_back(i.getValueString());
      }
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
    ImageInfoEx ptr(new std::vector<std::u8string>);
    if (m_Command.empty())
      return ptr;
    std::vector<std::u8string> result;
#ifdef _WIN32
#ifdef UNICODE
    std::wstring wcmd = Utf82WideString(GetCommandWithArg());
    std::vector<std::wstring> wresult;
    GetCmdOutput(wcmd.c_str(), wresult);
    for (auto& i : wresult) {
      result.emplace_back(Wide2Utf8String(i));
    }
#else
    std::string ccmd = Utf82AnsiString(GetCommandWithArg());
    std::vector<std::string> cresult;
    GetCmdOutput(ccmd.c_str(), cresult);
    for (auto& i : wresult) {
      result.emplace_back(Ansi2Utf8String(i));
    }
#endif
#elif def __linux__
    auto cmd = GetCommandWithArg();
    GetCmdOutput(reinterpret_cast<const char*>(cmd.c_str()), result);
#endif
    auto& str = result.front();
    if (str.empty())
      return ptr;
    ptr->push_back(str);
    return ptr;
  }

  YJson* GetJson() override {
    return m_Setting;
  }

  void SetJson() override {
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

