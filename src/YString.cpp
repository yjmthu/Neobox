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

size_t strlen(const std::deque<char*>& str_box)
{
    size_t len = 0;
    for (char*x: str_box)
    {
        len += strlen(x);
    }
    return len;
}

size_t wcslen(const std::deque<wchar_t*>& str_box)
{
    size_t len = 0;
    for (wchar_t*x: str_box)
    {
        len += wcslen(x);
    }
    return len;
}

wchar_t* StrJoinX(const wchar_t* start, const wchar_t* end, const wchar_t* dist, const std::deque<wchar_t*>& box)
{
    if (!start || !end || !dist || box.empty())
        return nullptr;
    size_t len = wcslen(start, end) + wcslen(box) + wcslen(dist) * (box.size() - 1);
    wchar_t* str = new wchar_t[len + 1];
    wchar_t* ptr = STRING_APPEND(str, start);
    for (wchar_t* k: box)
        ptr = STRING_APPEND(ptr, k, dist);
    STRING_APPEND(--ptr, end);
    return str;
}

char* StrJoinX(const char* start, const char* end, const char* dist, const std::deque<char*>& box)
{
    if (!start || !end || !dist || box.empty())
        return nullptr;
    size_t len = strlen(start, end) + strlen(box) + strlen(dist) * (box.size() - 1);
    char* str = new char[len + 1];
    char* ptr = STRING_APPEND(str, start);
    for (char*k: box)
        ptr = STRING_APPEND(ptr, k, dist);
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
            if (!strncmp<const char*>(str_a++, str_b, len_b))
                return --str_a + len_b;
        }
        return nullptr;
    }
}
