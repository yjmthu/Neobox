#include <cfloat>
#include <cmath>
#include <string>
#include "YString.h"

bool StrIsEmpty(const char* m)
{
    return !(*m);
}

bool StrIsEmpty(const wchar_t* m)
{
    return !(*m);
}

wchar_t* StrRepeat(const wchar_t c, size_t count)
{
    if (count < 0)
        return nullptr;
    else if (count == 0)
    {
        wchar_t* x = new wchar_t[1]; *x = 0;
        return x;
    }
    else
    {
        wchar_t* str = new wchar_t[count + 1];
        str[count] = 0;
        while (count--) str[count] = c;
        return str;
    }
}

char* StrRepeat(const char c, size_t count)
{
    if (count < 0)
        return nullptr;
    else if (count == 0)
    {
        char *x = new char[1]; *x = 0;
        return x;
    }
    else
    {
        char* str = new char[count + 1];
        str[count] = 0;
        while (count--) str[count] = c;
        return str;
    }
}


size_t strlen(const char** box, size_t num)
{
    size_t len = 0;
    while (num-- > 0) len += strlen(*box++);
    return  len;
}

size_t wcslen(const wchar_t** box, size_t num)
{
    size_t len = 0;
    while (num-- > 0) len += wcslen(*box++);
    return  len;
}

char* STRING_APPEND(char* head)
{
    return head;
}

wchar_t* STRING_APPEND(wchar_t* head)
{
    return head;
}

char* STRING_APPEND(char* head, const char* first)
{
    if (first)
    {
        while (*head++ = *first++);
        return --head;
    }
    else
        return head;
}

wchar_t* STRING_APPEND(wchar_t* head, const wchar_t* first)
{
    if (first)
    {
        while (*head++ = *first++);
        return --head;
    }
    else
        return head;
}

wchar_t* StrJoinX(const wchar_t* start, const wchar_t* end, const wchar_t* dist, const wchar_t** box, size_t num)
{
    if (num <= 0 || !start || !end || !dist || !box)
        return nullptr;
    size_t len = wcslen(start, end) + wcslen(box, num) + wcslen(dist) * (num - 1);
    wchar_t* str = new wchar_t[len + 1];
    wchar_t* ptr = STRING_APPEND(str, start);
    while (num--) ptr = STRING_APPEND(ptr, *(box++), dist);
    STRING_APPEND(--ptr, end);
    return str;
}

char* StrJoinX(const char* start, const char* end, const char* dist, const char** box, size_t num)
{
    if (num <= 0 || !start || !end || !dist || !box)
        return nullptr;
    size_t len = strlen(start, end) + strlen(box, num) + strlen(dist) * (num - 1);
    char* str = new char[len + 1];
    char* ptr = STRING_APPEND(str, start);
    while (num--) ptr = STRING_APPEND(ptr, *(box++), dist);
    STRING_APPEND(--ptr, end);
    return str;
}

wchar_t* StrCopy(wchar_t* str_old, const wchar_t* str_new)
{
    return STRING_APPEND(str_old, str_new);
}

char* StrCopy(char* str_old, const char* str_new)
{
    return STRING_APPEND(str_old, str_new);
}

void StrCopy(char* str_old, const char* str_new_start, const char* str_new_end)
{
    while (str_new_start != str_new_end)
    {
        *str_old++ = *str_new_start++;
    }
    *str_old = *str_new_end;
    *++str_old = 0;
}

void StrCopy(wchar_t* str_old, const wchar_t* str_new_start, const wchar_t* str_new_end)
{
    while (str_new_start != str_new_end)
    {
        *str_old++ = *str_new_start++;
    }
    *str_old = *str_new_end;
    *++str_old = 0;
}

bool StrCompare(const char* str_a, const char* str_b, size_t length)
{
    if (length <= 0)
        return true;
    while (length)
    {
        --length;
        if (str_a[length] != str_b[length])
            return false;
    }
    return true;
}

bool StrCompare(const wchar_t* str_a, const wchar_t* str_b, size_t length)
{
    if (length <= 0)
        return true;
    while (length)
    {
        --length;
        if (str_a[length] != str_b[length])
            return false;
    }
    return true;
}

bool StrCompareA(const char* str_a, const char* str_b)
{
    size_t  length_a = strlen(str_a), length_b = strlen(str_b);
    if (length_a != length_b)
        return false;
    while (length_a)
    {
        --length_a;
        if (str_a[length_a] != str_b[length_a])
        {
            if ('a' <= str_a[length_a] && str_a[length_a] <= 'z' && str_a[length_a] - str_b[length_a] == 'a' - 'A')
                continue;
            else if ('A' <= str_a[length_a] && str_a[length_a] <= 'Z' && str_b[length_a] - str_a[length_a] == 'a' - 'A')
                continue;
            else
                return false;
        }
    }
    return true;
}

bool StrCompare(const char* str_a, const char* str_b)
{
    size_t  length_a = strlen(str_a), length_b = strlen(str_b);
    if (length_a != length_b)
        return false;
    while (length_a)
    {
        --length_a;
        if (str_a[length_a] != str_b[length_a])
            return false;
    }
    return true;
}

bool StrCompare(const wchar_t* str_a, const wchar_t* str_b)
{
    size_t  length_a = wcslen(str_a), length_b = wcslen(str_b);
    if (length_a != length_b)
        return false;
    while (length_a)
    {
        --length_a;
        if (str_a[length_a] != str_b[length_a])
            return false;
    }
    return true;
}

char* StrContainStr(char* str_a, char* str_b)
{
    if (str_a == nullptr || str_b == nullptr)
        return nullptr;
    size_t len_a = strlen(str_a), len_b = strlen(str_b);
    if (len_a < len_b)
        return nullptr;
    else
    {
        while (*str_a)
        {
            if (StrCompare(str_a++, str_b, len_b))
                return --str_a + len_b;
        }
        return nullptr;
    }
}

const char* StrSkip(const char* content)
{
    while (content && *content && (unsigned char)*content <= 32) content++; return content;
}
