#include <string>
#include <vector>

template<typename _Char, typename _Ty>
void GetCmdOutput(const char *cmd, _Ty& result, int rows=-1)
{
    _Char m_buffer[1024];    
    FILE *ptr;
    result.emplace_back();
    if ((ptr = popen(cmd, "r")))   
    {
        while (fgets((char*)m_buffer, 1024, ptr)) {
            if (!result.back().empty() && result.back().back() == '\n') {
                if (!--rows) return;
                result.emplace_back(m_buffer);
            } else {
                result.back().append(m_buffer);
            }
        }
        pclose(ptr);
    } else {   
        // std::cout << "cmd error" << std::endl;
    }
}

inline std::string GetStdString(const std::u8string& str) {
    return std::string(str.begin(), str.end());
}

template <typename _Ty>
inline std::u8string GetU8String(const _Ty& str) {
    return std::u8string(str.begin(), str.end());
}
