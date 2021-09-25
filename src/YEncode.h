#ifndef YENCODE_H
#define YENCODE_H

#include <iostream>
#include <deque>
#include <vector>
#include <string>

constexpr uint16_t utf16FirstWcharMark[3] = { 0xD800, 0xDC00, 0xE000 };
constexpr unsigned char utf8FirstCharMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
constexpr uint32_t get_0x1s(const int i)
{
    return (1 << (8-i)) - 1;
}

template <class C1, class C2>
bool utf8_to_utf16(C1 utf16, C2 utf8)
{
    static_assert( (
        (            std::is_same<C1, std::deque<wchar_t>&>::value
                 ||
                     std::is_same<C1, std::vector<wchar_t>&>::value
                 ||
                     std::is_same<C1, std::wstring&>::value)
                 &&
        (            std::is_same<C2, const char*>::value
                 ||
                     std::is_same<C2, std::string::const_iterator>::value
                 ||
                     std::is_same<C2, std::vector<char>::const_iterator>::value
                 ||
                     std::is_same<C2, std::deque<char>::const_iterator>::value
        )
                   )
            , "error in type");
    std::cout << "Qt: 为何禁用cout1。\n";
    uint32_t unicode;
    uint8_t len;// bool double_wchar = false;
    do {
        unicode = static_cast<unsigned char>(*utf8);
        if (unicode < 0x80) len = 1;
        else if (unicode >> 6 == 0x2) return false;
        else if (unicode < utf8FirstCharMark[3]) len = 2;
        else if (unicode < utf8FirstCharMark[4]) len = 3;
        else if (unicode < utf8FirstCharMark[5]) len = 4;
        else if (unicode < utf8FirstCharMark[6]) len = 5;
        else len = 6;
        unicode &= get_0x1s(len);
        switch (len) {
        case 6: unicode <<= 6; unicode |= *(++utf8) & 0x3F;
        case 5: unicode <<= 6; unicode |= *(++utf8) & 0x3F;
        case 4: unicode <<= 6; unicode |= *(++utf8) & 0x3F;
        case 3: unicode <<= 6; unicode |= *(++utf8) & 0x3F;
        case 2: unicode <<= 6; unicode |= *(++utf8) & 0x3F;
        default:
            if (len <= 3)
            {
                utf16.push_back(unicode);
                break;
            }
            utf16.push_back(utf16FirstWcharMark[0] | (unicode >> 10));
            utf16.push_back(utf16FirstWcharMark[1] | (unicode & 0x3FF));
            break;
        }
    } while (*++utf8);
    utf16.push_back(0);
    return true;
}

template <typename  C1, typename C2>
bool utf16_to_utf8(C1 utf8, C2 utf16)
{
    static_assert(  (
            (         std::is_same<C1, std::deque<char>&>::value
                  ||
                      std::is_same<C1, std::vector<char>&>::value
                  ||
                      std::is_same<C1, std::string&>::value
            )
                  &&
            (         std::is_same<C2, const wchar_t*>::value
                  ||
                      std::is_same<C2, std::wstring::const_iterator>::value
                  ||
                      std::is_same<C2, std::vector<wchar_t>::const_iterator>::value
                  ||
                      std::is_same<C2, std::deque<wchar_t>::const_iterator>::value
            )
                    )
            , "error in type");
    uint32_t unicode;
    uint8_t len;
    do {
        unicode = *utf16;
        if (unicode < 0x80) len = 1;           // 6位以内，1字节
        else if (unicode < 0x800) len = 2;     // 11位以内，2字节
        else if (unicode >= utf16FirstWcharMark[0] && unicode < utf16FirstWcharMark[1])   // 双宽字节，对应4~6字节
        {
            if (*++utf16 < utf16FirstWcharMark[1] || *utf16 >= utf16FirstWcharMark[2]) return false;
            unicode = ((unicode & 0x3FF) << 10) | (*utf16 & 0x3FF);
            if (unicode < 0x200000) len = 4; else if (unicode < 0x4000000) len = 5; else len =6;
        }
        else if (unicode >= utf16FirstWcharMark[1] && unicode < utf16FirstWcharMark[2])   // 双宽字节第二位，无效
            return false;
        else len = 3;                          // 16位以内，3字节
        utf8.resize(utf8.size() + len);
        auto ptr = utf8.end();
        switch (len) {
        case 6: *--ptr = 0x80 | (unicode & 0x3F); unicode >>= 6;
        case 5: *--ptr = 0x80 | (unicode & 0x3F); unicode >>= 6;
        case 4: *--ptr = 0x80 | (unicode & 0x3F); unicode >>= 6;
        case 3: *--ptr = 0x80 | (unicode & 0x3F); unicode >>= 6;
        case 2: *--ptr = 0x80 | (unicode & 0x3F); unicode >>= 6;
        default: *--ptr = utf8FirstCharMark[len] | unicode;
        }
    } while (*++utf16);
    utf8.push_back(0);
    return true;
}

inline unsigned char ToHex(unsigned char x)
{
    return  x > 9 ? x + 55 : x + 48;
}

template <typename T1>
typename std::enable_if<std::is_same<T1, const char*>::value || std::is_same<T1, std::string::const_iterator>::value, std::string>::type
urlEncode(T1 str, size_t length)
{
    std::string strTemp;
    for (size_t i = 0; i < length; i++)
    {
        if (isalnum((unsigned char)str[i]) || strchr("-_.~", str[i]))
            strTemp.push_back(str[i]);
        else if (str[i] == ' ')
            strTemp.push_back('+');
        else
        {
            strTemp.push_back('%');
            strTemp.push_back(ToHex((unsigned char)str[i] >> 4));
            strTemp.push_back(ToHex((unsigned char)str[i] % 16));
        }
    }
    return strTemp;
}


#endif // YENCODE_H
