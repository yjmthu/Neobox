#include "apiclass.hpp"

#include "core/sysapi.h"

namespace WallClass {

class ScriptOutput: public WallBase {
public:
    explicit ScriptOutput(): WallBase() {
        InitBase();
    }
    virtual ~ScriptOutput(){ }
    virtual bool LoadSetting() {
        if (std::filesystem::exists(m_SettingPath)) {
            m_Setting = new YJson(m_SettingPath, YJson::UTF8);
            m_Command = m_Setting->find(u8"executeable")->second.getValueString();
            for (auto& i: m_Setting->find(u8"arglist")->second.getArray()) {
                m_ArgList.emplace_back(i.getValueString());
            }
            return true;
        }
        return false;
    }
    virtual bool WriteDefaultSetting() {
        m_Setting = new YJson(YJson::Object);
        m_Setting->append(m_Command, u8"executeable");
        m_Setting->append(YJson::Array, u8"arglist");
        m_Setting->toFile(m_SettingPath);
        return true;
    }
    virtual ImageInfoEx GetNext() {
        ImageInfoEx ptr(new std::vector<std::u8string>);
        if (m_Command.empty()) return ptr;
        std::vector<std::u8string> result;
        std::u8string cmd = GetCommandWithArg();
        GetCmdOutput<char8_t>((const char*)cmd.c_str(), result, 1);
        auto& str = result.front();
        while (!str.empty() && '\n' == str.back()) {
            str.pop_back();
        }
        // std::cout << "[ " << str << " ]" << std::endl;
        if (str.empty()) return ptr;
        ptr->push_back(str);
        return ptr;
    }
    virtual void Dislike(const std::string& img) {}
    virtual void SetCurDir(const std::string& str) {}
    virtual const void* GetDataByName(const char* key) const {
        if (!strcmp(key, "m_Setting")) {
            return &m_Setting;
        } else if (!strcmp(key, "m_Command")) {
            return &m_Command;
        } else if (!strcmp(key, "m_ArgList")) {
            return &m_ArgList;
        } else {
            return nullptr;
        }
    }
private:
    const char m_SettingPath[19] { "ScriptCommand.json" };
    YJson* m_Setting;
    std::u8string m_Command;
    std::vector<std::u8string> m_ArgList;
    std::u8string GetCommandWithArg() const {
        // if (m_Command.empty()) return "";
        std::u8string cmd;
        auto check = [&cmd](const std::u8string& str){
            if (!strchr("\"\'", str.front()) && str.find(' ') != std::u8string::npos) {
                cmd.append(u8"\"" + str + u8"\"");
            } else {
                cmd.append(str);
            }
            cmd.push_back(' ');
        };
        check(m_Command);
        for (auto& i: m_ArgList) {
            check(i);
        }
        if (cmd.back() == ' ') cmd.pop_back();
        return cmd;
    }
};

}
