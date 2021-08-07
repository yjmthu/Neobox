#pragma   once
#ifndef YSTRING_H
#define YSTRIHG_H

#include <QDebug>
#include <string>

bool StrIsEmpty(const char* m);
bool StrIsEmpty(const wchar_t* m);
size_t strlen(const char** box, size_t num);
size_t wcslen(const wchar_t** box, size_t num);
int IntLength(long num);
wchar_t* StrRepeat(const wchar_t c, size_t count);
char* StrRepeat(const char c, size_t count);
char* StrFromInt(long num);
char* StrFromDouble(double num);
bool StrCompare(const char* str_a, const char* str_b, size_t length);
bool StrCompareA(const char* str_a, const char* str_b);
bool StrCompare(const char* str_a, const char* str_b);
wchar_t* StrJoinX(const wchar_t* start, const wchar_t* end, const wchar_t* dist, const wchar_t** box, size_t num);
char* StrJoinX(const char* start, const char* end, const char* dist, const char** box, size_t num);
wchar_t* StrCopy(wchar_t* str_old, const wchar_t* str_new);
char* StrCopy(char* str_old, const char* str_new);
void StrCopy(char* str_old, const char* str_new_start, const char* str_new_end);
void StrCopy(wchar_t* str_old, const wchar_t* str_new_start, const wchar_t* str_new_end);
char* StrContainStr(char* str_a, char* str_b);
const char* StrSkip(const char* content);
const char* StrGoEnd(const char* content);

char* STRING_APPEND(char* head);
wchar_t* STRING_APPEND(wchar_t* head);
char* STRING_APPEND(char* head, const char* first);
wchar_t* STRING_APPEND(wchar_t* head, const wchar_t* first);


template <typename... Types>
size_t strlen(const char* head, Types...args)
{
    return strlen(head) + strlen(args...);
}

template <typename... Types>
size_t wcslen(const wchar_t* head, Types...args)
{
    return wcslen(head) + wcslen(args...);
}

template <typename... Types>
wchar_t* STRING_APPEND(wchar_t* head, const wchar_t* first, Types...args)
{
    return STRING_APPEND(STRING_APPEND(head, first), args...);
}

template <typename... Types>
char* STRING_APPEND(char* head, const char* first, Types...args)
{
    return STRING_APPEND(STRING_APPEND(head, first), args...);
}

template <typename... Types>
wchar_t* StrAppend(wchar_t* head, Types...args)
{
    while (*head) ++head; --head;
    return STRING_APPEND(head, args...);
}

template <typename... Types>
char* StrAppend(char* head, Types...args)
{
    while (*head) ++head; --head;
    return STRING_APPEND(head, args...);
}

template <typename... Types>
wchar_t* StrJoin(const wchar_t* head, Types...args)
{
    if (!head)
        return nullptr;
    wchar_t *str = new wchar_t[StrLength(head, args...) + 1];
    STRING_APPEND(str, head, args...);
    return str;
}

template <typename... Types>
char* StrJoin(const char* head, Types...args)
{
    if (!head)
        return nullptr;
    char *str = new char[strlen(head, args...) + 1];
    STRING_APPEND(str, head, args...);
    return str;
}

template <typename T>
inline bool StrCheckRange(char tc, T str)
{
    return ((int)tc >= (int)*str) && ((int)tc <= (int)str[2]);
}

template <typename T, typename...Args>
bool StrCheckRange(char tc, T str, Args...args)
{
    return StrCheckRange(tc, str) || StrCheckRange(tc, args...);
}

template <typename...Args>
const char* StrContainRange(const char* start, int length, Args...args)
{
    if (length <= 0 || start == nullptr)
        return nullptr;
    while (length--)
    {
        if (*start && StrCheckRange(*start++, args...))
            continue;
        return nullptr;
    }
    return start;   
}

#endif // !YSTRING_H
