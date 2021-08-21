#ifndef YENCODE_H
#define YENCODE_H

#include <deque>
#include <vector>
#include <string>

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
            ++utf8;
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
            utf16.push_back(0xD800 | (word >> 10));
            utf16.push_back(0xDC00 | (word & 0x3FF));
        }
    }
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
    while (*utf16)
    {
        if (*utf16 < 0x80)   // 6位以内，1字节
        {
            utf8.push_back(*utf16);
        }
        else if (*utf16 < 0x800)     // 11位以内，2字节
        {
            utf8.push_back(0xC0 | (*utf16 >> 6));
            utf8.push_back(0x80 | (*utf16 & 0x3F));
        }
        else if (*utf16 >= 0xD800 && *utf16 < 0xDC00)
        {
            uint32_t word = (*utf16 & 0x3FF) << 10;
            if (!*(++utf16)) return false;
            if (*utf16 >= 0xDC00 && *utf16 < 0xE000)
            {
                word |= *utf16 & 0x3FF;
                if (word < 0x200000)   // 21位以内
                {
                    utf8.push_back(0xF0 | (word >> 18));
                }
                else
                {
                    if (word < 0x4000000)    // 26位以内
                    {
                        utf8.push_back(0xF8 | (word >> 24));
                    }
                    else
                    {
                        utf8.push_back(0xF8 | (word >> 30));
                        utf8.push_back(0x80 | ((word >> 24) & 0x3F));

                    }
                    utf8.push_back(0x80 | ((word >> 18) & 0x3F));
                }
                utf8.push_back(0x80 | ((word >> 12) & 0x3F));
                utf8.push_back(0x80 | ((word >> 6) & 0x3F));
                utf8.push_back(0x80 | (word & 0x3F));
            }
            else
                return false;

        }
        else    // 16位以内，3字节
        {
            utf8.push_back(0xE0 | (*utf16 >> 12));
            utf8.push_back(0x80 | (*utf16 >> 6) & 0x3F);
            utf8.push_back(0x80 | (*utf16 & 0x3F));
        }
        ++utf16;
    }
    utf8.push_back(0);
    return true;
}

#endif // YENCODE_H
