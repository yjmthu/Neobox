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
        if (Wallpaper::PathFileExists(m_SettingPath)) {
            m_Setting = new YJson(m_SettingPath, YJson::UTF8);
            m_Command = m_Setting->find("executeable")->getValueString();
            for (auto i: *m_Setting->find("arglist")) {
                m_ArgList.emplace_back(i.getValueString());
            }
            return true;
        }
        return false;
    }
    virtual bool WriteDefaultSetting() {
        // m_Command = "";
        m_Setting = new YJson(YJson::Object);
        m_Setting->append(m_Command, "executeable");
        m_Setting->append(YJson::Array, "arglist");
        m_Setting->toFile(m_SettingPath, YJson::UTF8, true);
        return true;
    }
    virtual ImageInfo GetNext() {
        ImageInfo ptr(new std::vector<std::string>);
        if (m_Command.empty()) return ptr;
        std::vector<std::string> result;
        std::string cmd = GetCommandWithArg();
        // std::cout << cmd << std::endl;
        GetCmdOutput(cmd.c_str(), result, 1);
        auto& str = result.front();
        while (!str.empty() && '\n' != str.back()) {
            str.pop_back();
        }
        if (str.empty()) return ptr;
        auto pos = str.find_last_of(FILE_SEP_PATH);
        ptr->push_back(str.substr(0, pos));
        ptr->push_back(str.substr(pos+1));
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
    std::string m_Command;
    std::vector<std::string> m_ArgList;
    std::string GetCommandWithArg() const {
        // if (m_Command.empty()) return "";
        std::string cmd;
        auto check = [&cmd](const std::string& str){
            if (!strchr("\"\'", str.front()) && str.find(' ') != std::string::npos) {
                cmd.append("\"" + str + "\"");
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
