#ifndef YENCODE_H
#define YENCODE_H

#include <funcbox.h>
#include <deque>
#include <vector>
#include <string>

constexpr const uint16_t utf16FirstWcharMark[3] = { 0xD800, 0xDC00, 0xE000 };
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
    while (*utf8)
    {
        if (static_cast<unsigned char>(*utf8) < 0x80)
        {
            utf16.push_back(*utf8);
            utf8 += 1;
        }
        else if (static_cast<unsigned char>(*utf8) < 0xE0)
        {
            if (!utf8[1]) return false;
            utf16.push_back(((utf8[0] & 0x1F) << 6) | (utf8[1] & 0x3F));
            utf8 += 2;
        }
        else if (static_cast<unsigned char>(*utf8) < 0xF0)
        {
            if (!(utf8[1] && utf8[2])) return false;
            utf16.push_back(((utf8[0] & 0xF) << 12) | ((utf8[1] & 0x3F) << 6) | (utf8[2] & 0x3F));
            utf8 += 3;
        }
        else
        {
            uint32_t word = 0;
            if (static_cast<unsigned char>(*utf8) < 0xF8)
            {
                if (!(utf8[1] && utf8[2] && utf8[3])) return false;
                word = ((utf8[0] & 0x7) << 18);
                word &= ((utf8[1] & 0x3F) << 12) | ((utf8[2] & 0x3F) << 6) | (utf8[3] & 0x3F);
                utf8 += 4;
            }
            else if (static_cast<unsigned char>(*utf8) < 0xFC)
            {
                if (!(utf8[1] && utf8[2] && utf8[3] && utf8[4])) return false;
                word = ((utf8[0] & 0x3) << 24) | ((utf8[1] & 0x3F) << 18);
                word &= ((utf8[2] & 0x3F) << 12) | ((utf8[3] & 0x3F) << 6) | (utf8[4] & 0x3F);
                utf8 += 5;
            }
            else
            {
                if (!(utf8[1] && utf8[2] && utf8[3] && utf8[4] && utf8[5])) return false;
                word = ((utf8[0] & 0x1) << 30) | ((utf8[1] & 0x3F) << 24) | ((utf8[2] & 0x3F) << 18);
                word &= ((utf8[3] & 0x3F) << 12) | ((utf8[4] & 0x3F) << 6) | (utf8[5] & 0x3F);
                utf8 += 6;
            }
            utf16.push_back(utf16FirstWcharMark[0] | (word >> 10));
            utf16.push_back(utf16FirstWcharMark[1] | (word & 0x3FF));
        }
    }
    utf16.push_back(0);
    return true;
}

constexpr const unsigned char utf8FirstCharMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

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
    while (*utf16)
    {
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
        ++utf16;
    }
    if (utf8.back()) utf8.push_back(0);
    return true;
}

#endif // YENCODE_H
