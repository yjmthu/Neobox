#pragma   once
#ifndef YSTRING_H
#define YSTRIHG_H

#include <QDebug>
#include <string>
#include <deque>

bool StrIsEmpty(const char* m);
bool StrIsEmpty(const wchar_t* m);
wchar_t* StrRepeat(const wchar_t c, size_t count);
char* StrRepeat(const char c, size_t count);
char* StrJoinX(const char* start, const char* end, const char* dist, const std::deque<char*>& box);
void StrCopy(char* str_old, const char* str_new_start, const char* str_new_end);
void StrCopy(wchar_t* str_old, const wchar_t* str_new_start, const wchar_t* str_new_end);
char* StrContainStr(char* str_a, char* str_b);

char* STRING_APPEND(char* head);
wchar_t* STRING_APPEND(wchar_t* head);
char* STRING_APPEND(char* head, const char* first);
wchar_t* STRING_APPEND(wchar_t* head, const wchar_t* first);

template <typename T>
int strcmp(T s1, const char* s2)
{
    static_assert ((    std::is_same<T, std::deque<char>::const_iterator>::value
                      ||
                        std::is_same<T, std::vector<char>::const_iterator>::value
                      ||
                        std::is_same<T, std::string::const_iterator>::value
                      ||
                        std::is_same<T, const char*>::value
                   ), "error in type");
      char c1, c2;
      do {
            c1 = *s1++;
            c2 = *s2++;
            if (c1 == '\0')
                return c1 - c2;
      } while (c1 == c2);
      return c1 - c2;
}

template <typename T>
int strncmp(T s1, const char* s2, size_t n)
{
    static_assert ( std::is_same<T, std::deque<char>::const_iterator>::value
                  ||
                    std::is_same<T, std::vector<char>::const_iterator>::value
                  ||
                    std::is_same<T, std::string::const_iterator>::value
                  ||
                    std::is_same<T, const char*>::value
                   , "error in type");
    char c1 = '\0';
    char c2 = '\0';
    if (n >= 4)
    {
        size_t n4 = n >> 2;
        do
        {
            c1 = *s1++;
            c2 = *s2++;
            if (c1 == '\0' || c1 != c2)
                return c1 - c2;
            c1 = *s1++;
            c2 = *s2++;
            if (c1 == '\0' || c1 != c2)
                return c1 - c2;
            c1 = *s1++;
            c2 = *s2++;
            if (c1 == '\0' || c1 != c2)
                return c1 - c2;
            c1 = *s1++;
            c2 = *s2++;
            if (c1 == '\0' || c1 != c2)
                return c1 - c2;
        } while (--n4 > 0);
        n &= 3;
    }

    while (n > 0)
    {
        c1 = *s1++;
        c2 = *s2++;
        if (c1 == '\0' || c1 != c2)
            return c1 - c2;
        n--;
    }
    return c1 - c2;
}

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
    wchar_t *str = new wchar_t[wcslen(head, args...) + 1];
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

inline bool StrCheckRange(const char tc, const char* str)
{
    return (tc >= *str) && (tc <= str[2]);
}

inline bool StrCheckRange(const wchar_t tc, const wchar_t* str)
{
    return (tc >= *str) && (tc <= str[2]);
}

template <typename...Args>
bool StrCheckRange(const char tc, const char* str, Args...args)
{
    return StrCheckRange(tc, str) || StrCheckRange(tc, args...);
}

template <typename...Args>
bool StrCheckRange(const wchar_t tc, const wchar_t* str, Args...args)
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

template <typename...Args>
const wchar_t* StrContainRange(const wchar_t* start, int length, Args...args)
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

template <typename T>
T StrSkip(T content)
{
    static_assert(   std::is_same<T, std::string::const_iterator>::value
                 ||
                     std::is_same<T, std::vector<char>::const_iterator>::value
                 ||
                     std::is_same<T, std::deque<char>::const_iterator>::value
                 ||
                     std::is_same<T, const char*>::value
            , "error in type");
    while (*content && static_cast<const unsigned char>(*content) <= 32) content++; return content;
}

#endif // !YSTRING_H
