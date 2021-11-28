#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
#include <functional>
#include <random>
#include <limits>
#include <iomanip>
#include <utility>
#include <cmath>
#include <cstring>

#ifdef max
#undef max
#endif // max

#ifdef min
#undef min
#endif // min

/***
* 针对 wchar_t 和 char 做的区分.
* 在本类中, 是替代 wcsncmp 和 strncmp 的更好的选择.
*/

template <typename _Ty>
bool m_strncmp(_Ty ptr1, const char* ptr2, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        if (ptr1[i] != ptr2[i])
            return false;
    }
    return true;
}

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
class _Base_FormulaPaser
{
protected:
    typedef typename  _String::const_iterator citerator;
    typedef typename  _String::iterator iterator;
    enum _Error_State { NoError, Null, Undefined, Wait, Invalid };
    double _value = std::numeric_limits<double>::infinity();
    static int _error_state;
    static double _ans;
    std::vector<_Base_FormulaPaser> _son;

    double calculate()
    {
        if (!_son.size()) return _negative * Function[static_cast<int>(_func)](_value);
        auto iter = _son.begin();
        double q = iter->calculate();
        while (++iter != _son.end())
        {
            switch (iter->_operation)
            {
            case Sign::A:
                q += iter->calculate();
                break;
            case Sign::S:
                q -= iter->calculate();
                break;
            case Sign::M:
                q *= iter->calculate();
                break;
            case Sign::D:
                q /= iter->calculate();
                break;
            case Sign::P:
                q = pow(q, iter->calculate());
            default:
                break;
            }
        }
        return _negative * Function[static_cast<int>(_func)](q);
    }

    explicit _Base_FormulaPaser(_String& text) {
        citerator end = text.cbegin();
        if (text.empty())
        {
            _error_state = _Error_State::Null;
            return;
        }
        _error_state = _Error_State::NoError;
        for (const auto c : text)
        {
            if (!isascii(c) || !(strchr("+-*/^(). ", c) || isalnum(c)))
            {
                _error_state = _Error_State::Invalid;
                return;
            }
        }
        std::stable_sort(text.begin(), text.end(), [](const _Ty& val1, const _Ty& val2)->bool { return val2 == ' ' && val1 != ' '; });
        typename _String::size_type pos = text.find(' ');
        if (pos != _String::npos)
        {
            text.erase(text.begin() + pos, text.end());
        }
        _parse_all(text.cbegin(), text.cend());
    }

    _String outstr(bool remember = false)
    {
        switch (_error_state)
        {
        case _Error_State::Null:
            return { 'N', 'u', 'l', 'l', '\0' };
        case _Error_State::Undefined:
            return { 'U', 'n', 'd', 'e', 'f', 'i', 'n', 'e', 'd', '\0' };
        case _Error_State::Invalid:
            return { 'I', 'n', 'v', 'a', 'l', 'i', 'd', '\0' };
        case _Error_State::Wait:
            return { 'W', 'a', 'i', 't', '\0' };
        case _Error_State::NoError:
            return (_Ostream() << std::setprecision(16) << (remember ? (_ans = calculate()) : calculate())).str();
        default:
            return (_Ostream() << std::setprecision(16) << _value).str();
        }
    }

    void print(bool re)
    {
        std::cout << "value: " << _value << " child: " << _son.size() << " Operator: " << (int)_operation << (re ? " ||| \n" : " ||| ");
        for (size_t i = 0; i < _son.size(); ++i)
        {
            _son[i].print(i == _son.size() - 1);
        }
    }
private:
    // explicit _Base_FormulaPaser(citerator begin, citerator end);
    enum class Sign { A, S, D, M, P, N };
    enum class Func { Def, Sin, Cos, Tan, Asin, Acos, Atan, Sinh, Cosh, Tanh, Asinh, Acosh, Atanh, Sqrt, Abs, Exp, Ln, Lg, Rad, Deg, Fact };
    static constexpr double _PI_ = 3.141592653589793, _E_ = 2.718281828459045;
    Sign _operation = Sign::N;
    int _negative = 1;                   // 标记负号, 负负得正, 每次负号累积乘以-1
    typedef double(*_Func_Type)(double);
    static const _Func_Type Function[21];
    Func _func = Func::Def;

    /***
    * 以运算符或括号或函数为分割界限, 解析出一个独立且正确的参与运算的成员
    * 只能解析当前算式最外围的独立成员例如sin(a)+b*c被解析为sin(a), 返回指向+的指针, 再次解析将返回指向*的指针
    * 解析失败返回 nullptr
    */

    citerator _parse_si(citerator begin, citerator end) {
        citerator ptr = begin;
        // std::cout << "parse_one is here.\n";
        if (ptr >= end)
        {
            // std::cout << "empty str!\n";
            return begin;
        }
        if (strchr("+-*/^", *ptr))
        {
            _error_state = _Error_State::Invalid;
            return begin;
        }
        unsigned left = 1;
        if (strchr("+-", *ptr)) ++ptr;
        if (isdigit(*ptr))
        {
            if (*ptr == '0')
            {
                if (++ptr == end) return end;
                if (*ptr != '.') return ptr;
            }
            while (ptr < end && isdigit(*ptr)) ++ptr;
            if (ptr == end) return end;
            if (*ptr == '.' && ptr < end - 1 && isdigit(ptr[1]))
            {
                while (++ptr < end && isdigit(*ptr));
                return ptr;
            }
            // std::cout << "parse one is digit \\" << begin << "\\" << end << "\\" << "\n";
            return ptr;
        }
        while (ptr < end && islower(*ptr)) ++ptr;
        if (ptr >= end) return begin;
        while (++ptr < end && left)
        {
            if (*ptr == '(') ++left;
            else if (*ptr == ')') --left;
        }
        if (left) return begin;
        else return ptr;
    }

    citerator _parse_md(citerator begin, citerator end)
    {
        if (begin >= end) return begin;
        // std::cout << "parse_md is here. \\" << begin << "\\" << end << "\\\n";
        citerator ptr = _parse_si(begin, end), ptr1;
        while (ptr != begin && ptr != end && strchr("*/^", *ptr))
        {
            ptr1 = ++ptr;
            ptr = _parse_si(ptr1, end);
            if (ptr == ptr1) return begin;
        }
        return ptr;
    }

    citerator _parse_pw(citerator begin, citerator end)
    {
        if (begin >= end) return begin;
        // std::cout << "parse_md is here. \\" << begin << "\\" << end << "\\\n";
        citerator ptr = _parse_si(begin, end), ptr1;
        while (ptr != begin && ptr < end && '^' == *ptr)
        {
            ptr1 = ++ptr;
            ptr = _parse_si(ptr1, end);
            if (ptr == ptr1) return begin;
        }
        return ptr;
    }

    void _parse_val(citerator begin, citerator end)
    {
        if (*begin == '-')
        {
            _negative = -1;
            ++begin;
        }
        else if (*begin == '+')
            ++begin;
        if (isdigit(*begin))
        {
            // std::cout << "this is a digit.  \\" << begin << '\\' << end << "\\\n";
            _Istream(_String(begin, end)) >> _value;
            // std::cout << "digit value is " << _value << std::endl;
            return;
        }
        if (*begin == '(')
        {
            int k = _count_bracket(begin, end);
            _parse_all(begin + k, end - k);
            return;
        }
        else if (m_strncmp<citerator>(begin, "arcsin(", 7))
        {
            begin += 7;
            _func = Func::Asin;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "arccos(", 7))
        {
            begin += 7;
            _func = Func::Acos;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "arctan(", 7))
        {
            begin += 7;
            _func = Func::Atan;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "arcsinh(", 8))
        {
            begin += 8;
            _func = Func::Asinh;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "arccosh(", 8))
        {
            begin += 8;
            _func = Func::Acosh;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "arctanh(", 8))
        {
            begin += 8;
            _func = Func::Atanh;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "sqrt(", 5))
        {
            begin += 5;
            _func = Func::Sqrt;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "sin(", 4))
        {
            begin += 4;
            _func = Func::Sin;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "cos(", 4))
        {
            begin += 4;
            _func = Func::Cos;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "tan(", 4))
        {
            begin += 4;
            _func = Func::Tan;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "sinh(", 5))
        {
            begin += 5;
            _func = Func::Sinh;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "cosh(", 5))
        {
            begin += 5;
            _func = Func::Cosh;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "tanh(", 5))
        {
            begin += 5;
            _func = Func::Tanh;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "ln(", 3))
        {
            begin += 3;
            _func = Func::Ln;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "lg(", 3))
        {
            begin += 3;
            _func = Func::Lg;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "exp(", 4))
        {
            begin += 4;
            _func = Func::Exp;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "rad(", 4))
        {
            begin += 4;
            _func = Func::Rad;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "deg(", 4))
        {
            begin += 4;
            _func = Func::Deg;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "fact(", 5))
        {
            begin += 5;
            _func = Func::Fact;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "abs(", 4))
        {
            begin += 4;
            _func = Func::Abs;
            --end;
        }
        else if (m_strncmp<citerator>(begin, "pi()", 4))
        {
            _value = _PI_;
            return;
        }
        else if (m_strncmp<citerator>(begin, "e()", 3))
        {
            _value = _E_;
            return;
        }
        else if (m_strncmp<citerator>(begin, "ans()", 5))
        {
            _value = _ans;
            return;
        }
        else if (m_strncmp<citerator>(begin, "rand()", 5))
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<double> dis(0, 1);
            _value = dis(gen);
            return;
        }
        else
        {
            _error_state = _Error_State::Undefined;
            return;
        }
        if (*begin == ')')
        {
            _error_state = _Error_State::Invalid;
            return;
        }
        // std::cout << "re parse all begin: " << begin << " end " << end << "\n";
        _son.emplace_back(Sign::N, begin, end);
    }

    void _parse_all(citerator begin, citerator end)
    {
        if (begin >= end)
        {
            // std::cout << "empty str\n";
            return;
        }
        // std::cout << " =================== begin all ==================== \n";
        citerator ptr1 = strchr("+-", *begin) ? begin + 1 : begin, ptr2 = _parse_si(ptr1, end);

        if (ptr2 < end - 1 && ptr2 != ptr1 && !strchr("+-*/^", ptr2[1]))
        {
            // std::cout << "-------------------1--------------------\n";
            switch (*ptr2)
            {
            case '+':
            case '-':
                // std::cout << "case +-: \n";
                _son.emplace_back(Sign::N, ptr1, ptr2);
                if (*begin == '-') _son.back()._negative *= -1;
                while (ptr2 < end)
                {
                    ptr1 = ptr2;
                    ptr2 = _parse_md(++ptr2, end);
                    if (ptr2 == ptr1 + 1)
                    {
                        // std::cout << "out md.\n";
                        _error_state = _Error_State::Wait;
                        return;
                    }
                    if (*ptr1 == '+')
                        _son.emplace_back(Sign::A, ++ptr1, ptr2);
                    else
                        _son.emplace_back(Sign::S, ++ptr1, ptr2);
                }
                // std::cout << "son number is " << _son.size() << '\n';
                break;
            case '*':
            case '/':
                ptr2 = _parse_md(ptr1, end);
                if (ptr2 == ptr1)
                {
                    _error_state = _Error_State::Wait;
                    return;
                }
                if (ptr2 < end && (*ptr2 == '-' || *ptr2 == '+'))
                {
                    _son.emplace_back(Sign::N, ptr1, ptr2);
                    if (*begin == '-') _son.back()._negative = -1;
                    while (ptr2 < end)
                    {
                        ptr1 = ptr2;
                        ptr2 = _parse_md(ptr1 + 1, end);
                        if (ptr2 == ptr1 + 1)
                        {
                            _error_state = _Error_State::Wait;
                            return;
                        }
                        if (*ptr1 == '+')
                            _son.emplace_back(Sign::A, ++ptr1, ptr2);
                        else
                            _son.emplace_back(Sign::S, ++ptr1, ptr2);
                    }
                    return;
                }
                else if (ptr2 == end)
                {
                    ptr2 = _parse_si(ptr1, end);
                    if (*begin == '-') _negative *= -1;
                    _son.emplace_back(Sign::N, ptr1, ptr2);
                    while (ptr2 != end)
                    {
                        ptr1 = ptr2;
                        ptr2 = _parse_pw(ptr1 + 1, end);
                        if (ptr2 == ptr1 + 1) return;
                        if (*ptr1 == '*')
                            _son.emplace_back(Sign::M, ++ptr1, ptr2);
                        else
                            _son.emplace_back(Sign::D, ++ptr1, ptr2);
                    }
                    return;
                }
                return;
            case '^':
                ptr2 = _parse_md(ptr1, end);
                if (ptr2 == ptr1)
                {
                    _error_state = _Error_State::Wait;
                    return;
                }
                if (ptr2 > end)
                {
                    _error_state = _Error_State::Undefined;
                    return;
                }
                if (ptr2 == end)
                {
                    ptr2 = _parse_pw(ptr1, end);
                    if (ptr2 == end)
                    {
                        ptr2 = _parse_si(ptr1, end);
                        _son.emplace_back();
                        if (*begin == '-') _son.back()._negative *= -1;
                        _son.back()._son.emplace_back(Sign::N, ptr1, ptr2);
                        while (ptr2 < end)
                        {
                            ptr1 = ++ptr2;
                            ptr2 = _parse_si(ptr1, end);
                            if (ptr2 == ptr1)
                            {
                                _error_state = _Error_State::Invalid;
                                return;
                            }
                            _son.back()._son.emplace_back(Sign::P, ptr1, ptr2);
                        }
                        return;
                    }
                    if (*begin == '-') _negative *= -1;
                    _son.emplace_back(Sign::N, ptr1, ptr2);
                    while (ptr2 != end)
                    {
                        ptr1 = ptr2;
                        ptr2 = _parse_pw(ptr1 + 1, end);
                        if (ptr2 == ptr1 + 1)
                        {
                            _error_state = _Error_State::Invalid;
                            return;
                        }
                        if (*ptr1 == '*')
                            _son.emplace_back(Sign::M, ++ptr1, ptr2);
                        else
                            _son.emplace_back(Sign::D, ++ptr1, ptr2);
                    }
                    return;
                }
                else if (*ptr2 == '-' || *ptr2 == '+')
                {
                    _son.emplace_back(Sign::N, ptr1, ptr2);
                    if (*begin == '-') _son.back()._negative *= -1;
                    while (ptr2 < end)
                    {
                        ptr1 = ptr2;
                        ptr2 = _parse_md(ptr1 + 1, end);
                        if (ptr2 == ptr1 + 1)
                        {
                            _error_state = _Error_State::Wait;
                            return;
                        }
                        if (*ptr1 == '+')
                            _son.emplace_back(Sign::A, ++ptr1, ptr2);
                        else
                            _son.emplace_back(Sign::S, ++ptr1, ptr2);
                    }
                    return;
                }
                else
                {
                    _error_state = _Error_State::Undefined;
                }
                return;
            default:
                _error_state = _Error_State::Undefined;
                break;
            }
            return;
        }
        else if (ptr2 == end)
        {
            if (*begin == '-') _negative *= -1;
            return _parse_val(ptr1, end);
        }
        _error_state = _Error_State::Invalid;
    }

    int _count_bracket(citerator begin, citerator end)
    {
        citerator ptr;
        int all = 0, offset = 0;
        while (*begin == '(' && begin < end)
        {
            ptr = begin;
            while (ptr < end)
            {
                if (*ptr == '(') ++offset;
                else if (*ptr == ')') --offset;
                ++ptr;
            }
            if (!offset) ++all;
            else break;
            ++begin;
        }
        return all;
    }
public:
    _Base_FormulaPaser() {};
    _Base_FormulaPaser(Sign type, citerator begin, citerator end) : _operation(type) {
        _parse_all(begin, end);
    }
    ~_Base_FormulaPaser() {};
};

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
int _Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::_error_state = _Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::NoError;

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
double _Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::_ans = 0;
typedef double(*_Func_Type)(double);

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
const _Func_Type _Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::Function[21]{
    [](double q)->double {return q; },
    (_Func_Type)sin, (_Func_Type)cos, (_Func_Type)tan,
    (_Func_Type)asin, (_Func_Type)acos, (_Func_Type)atan,
    (_Func_Type)sinh, (_Func_Type)cosh, (_Func_Type)tanh,
    (_Func_Type)asinh, (_Func_Type)acosh, (_Func_Type)atanh,
    (_Func_Type)sqrt, (_Func_Type)abs, (_Func_Type)exp,
    (_Func_Type)log, (_Func_Type)log10,
    [](double deg)->double { return deg * _PI_ / 180; },
    [](double rad)->double { return rad * 180 / _PI_; },
    [](double n)->double {
        long long N = round(n);
        if (N >= std::numeric_limits<int>::max())
            return std::numeric_limits<double>::infinity();
        if (N < 0)
            return std::numeric_limits<double>::quiet_NaN();
        double q = 1;
        for (int i = 1; i <= N; ++i)
            q *= i;
        return q;
    }
};

template <typename _Ty>
class FormulaPaser : private std::conditional_t<std::is_same<_Ty, char>::value,
    _Base_FormulaPaser<_Ty, std::string, std::istringstream, std::ostringstream>,
    _Base_FormulaPaser<_Ty, std::wstring, std::wistringstream, std::wostringstream>>
{
private:
    typedef std::conditional_t<std::is_same<_Ty, char>::value, _Base_FormulaPaser<_Ty, std::string, std::istringstream, std::ostringstream>, _Base_FormulaPaser<_Ty, std::wstring, std::wistringstream, std::wostringstream>> _parent;
    typedef std::conditional_t<std::is_same<_Ty, char>::value, std::string, std::wstring> _String;
    typedef std::conditional_t<std::is_same<_Ty, char>::value, std::ostringstream, std::wostringstream> _Ostream;
public:
    using _parent::calculate;
    using _parent::print;
    FormulaPaser(_String& text) : _parent(text) {};
    FormulaPaser(_String text) : _parent(text) {};
    using _parent::outstr;
};

