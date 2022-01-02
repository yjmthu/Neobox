#ifndef YSTRING_H
#define YSTRIHG_H

#include <vector>
#include <string>
#include <deque>

template <typename ..._Ty> inline void PX_UNUSED(_Ty...) {}

template <typename T=char, typename... Types>
typename std::enable_if<std::is_same<T, wchar_t>::value || std::is_same<T, char>::value, size_t>::type
strlen(const T* head, Types...args)
{
    typedef size_t(*PF1)(const wchar_t*);
    typedef size_t(*PF2)(const char*);
    static const void* const str_cpoy_funcs[] = {(void*)((PF1)(wcslen)), (void*)((PF2)(strlen))};
    size_t len = ((typename std::conditional<std::is_same<T, wchar_t>::value,PF1,PF2>::type)(str_cpoy_funcs[std::is_same<T, char>::value]))(head);
    char array[] = {'\0', (len += strlen(args), '\0')...};
    PX_UNUSED(array);
    return len;
}

template <typename T=char>
typename std::enable_if<std::is_same<T, char>::value||std::is_same<T, wchar_t>::value, size_t>::type
boxlen(const std::deque<T*>& str_box)
{
    size_t len = 0;
    for (auto*x: str_box) len += strlen(x);
    return len;
}

template <typename T=char>
typename std::enable_if<std::is_same<T, wchar_t>::value || std::is_same<T, char>::value,void>::type
StrCopy(T* str_old, const T* str_new_start, const T* str_new_end)
{
    while (str_new_start != str_new_end) *str_old++ = *str_new_start++;
    *str_old = *str_new_end;
    *++str_old = 0;
}

template <typename T=char>
typename std::enable_if<std::is_same<T, wchar_t>::value || std::is_same<T, char>::value,T*>::type
StrRepeat(const T c, size_t count)
{
    if (count < 0)
        return nullptr;
    else if (count == 0)
    {
        T* x = new T[1]; *x = 0;
        return x;
    }
    else
    {
        T* str = new T[count + 1];
        str[count] = 0;
        while (count--) str[count] = c;
        return str;
    }
}

template <typename T=const char*>
typename std::enable_if<std::is_same<T, std::deque<char>::const_iterator>::value
                      ||
                        std::is_same<T, std::vector<char>::const_iterator>::value
                      ||
                        std::is_same<T, std::string::const_iterator>::value
                      ||
                        std::is_same<T, const char*>::value, int>::type
strcmp(T s1, const char* s2)
{
      char c1, c2;
      do {
            c1 = *s1++;
            c2 = *s2++;
            if (c1 == '\0')
                return c1 - c2;
      } while (c1 == c2);
      return c1 - c2;
}

template <typename T=const char*>
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

template <typename T=char>
typename std::enable_if<std::is_same<T, wchar_t>::value || std::is_same<T, char>::value,T*>::type
ByteCpy(T* s1, const T* s2)
{
    if (s1)
    {
        while (*s1++ = *s2++);
        return --s1;
    }
    else
        return s1;
}


template <typename T=char, typename... Types>
typename std::enable_if<std::is_same<T, wchar_t>::value || std::is_same<T, char>::value,T*>::type
ByteCpy(T* head, const T* first, Types...args)
{
    static_assert (std::is_same<T, char>::value || std::is_same<T, wchar_t>::value, "error in type");
    return ByteCpy<T>(ByteCpy<T>(head, first), args...);
}

template <typename T=char, typename... Types>
typename std::enable_if<std::is_same<T, wchar_t>::value || std::is_same<T, char>::value, T>::type
StrAppend(T* head, Types...args)
{
    static_assert (std::is_same<T, char>::value || std::is_same<T, wchar_t>::value, "error in type");
    while (*head)
        ++head;
    --head;
    return ByteCpy<T>(head, args...);
}

template<int A>
char* StrJoin(const char* start, const char* end, const char* dist, const std::deque<char*>& box)
{
    if (!start || !end || !dist || box.empty())
        return nullptr;
    size_t len = strlen(start, end) + boxlen(box) + strlen(dist) * (box.size() - 1);
    char* str = new char[len + 1];
    char* ptr = ByteCpy<char>(str, start);
    for (char*k: box)
    {
        ptr = ByteCpy<char>(ptr, k, dist);
        delete[] k;
    }
    ByteCpy<char>(ptr-=A, end);
    delete [] start;
    return str;
}

template <typename T=char, typename... Types>
typename std::enable_if<std::is_same<T, wchar_t>::value || std::is_same<T, char>::value, T*>::type
StrJoin(const T* head, Types...args)
{
    if (!head)
        return nullptr;
    T *str = new T[strlen(head, args...) + 1];
    ByteCpy<T>(str, head, args...);
    return str;
}

template <typename T=char>
inline typename std::enable_if<std::is_same<T, char>::value||std::is_same<T, wchar_t>::value, bool>::type
CharInRanges(const T tc, const T* str)
{
    return (tc >= *str) && (tc <= str[2]);
}

template <typename T=char, typename...Args>
typename std::enable_if<std::is_same<T, char>::value||std::is_same<T, wchar_t>::value, bool>::type
CharInRanges(const T tc, const T* str, Args...args)
{
    return CharInRanges<T>(tc, str) || CharInRanges<T>(tc, args...);
}

template <typename T=char, typename...Args>
typename std::enable_if<std::is_same<T, char>::value||std::is_same<T, wchar_t>::value, const T*>::type
StrContainCharInRanges(const T* start, int length, Args...args)
{
    if (length <= 0 || start == nullptr)
        return nullptr;
    while (length--)
    {
        if (*start && CharInRanges<T>(*start++, args...))
            continue;
        return nullptr;
    }
    return start;   
}

template <typename T=const char*>
typename std::enable_if<
                    std::is_same<T, std::string::const_iterator>::value
                ||
                    std::is_same<T, std::vector<char>::const_iterator>::value
                ||
                    std::is_same<T, std::deque<char>::const_iterator>::value
                ||
                    std::is_same<T, const char*>::value
                     , T>::type
StrSkip(T content)
{
    while (*content && static_cast<const unsigned char>(*content) <= 32)
        content++;
    return content;
}

#endif // !YSTRING_H
