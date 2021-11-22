#include <iostream>
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
bool m_strncmp(const _Ty* ptr1, const char* ptr2, size_t n)
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
    enum _Error_State { NoError, Null, Undefined, Wait, Invalid };
    double _value = std::numeric_limits<double>::infinity();
    double calculate();
    explicit _Base_FormulaPaser(const _Ty* text);
    static int _error_state;
    static double _ans;
    std::vector<_Base_FormulaPaser> _son;
    _String outstr(bool remember);
    void print(bool re);
private:
    explicit _Base_FormulaPaser(const _Ty* begin, const _Ty* end);
    enum class Sign { A, S, D, M, P, N };
    enum class Func { Def, Sin, Cos, Tan, Asin, Acos, Atan, Sinh, Cosh, Tanh, Asinh, Acosh, Atanh, Sqrt, Abs, Exp, Ln, Lg, Rad, Deg, Factorial };
    static constexpr double _PI_ = 3.141592653589793, _E_ = 2.718281828459045;
    Sign _operation = Sign::N;
    int _negative = 1;                   // 标记负号, 负负得正, 每次负号累积乘以-1
    typedef double(*_Func_Type)(double);
    static const _Func_Type Function[21];
    Func _func = Func::Def;
    const _Ty* _parse_si(const _Ty* begin, const _Ty* end);
    const _Ty* _parse_md(const _Ty* begin, const _Ty* end);
    const _Ty* _parse_pw(const _Ty* begin, const _Ty* end);
    void _parse_val(const _Ty* begin, const _Ty* end);
    void _parse_all(const _Ty* begin, const _Ty* end);
    int _count_bracket(const _Ty* begin, const _Ty* end);
public:
    _Base_FormulaPaser() {};
    _Base_FormulaPaser(Sign type, const _Ty*, const _Ty*);
    ~_Base_FormulaPaser() {};
};

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
int _Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::_error_state = _Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::NoError;

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
double _Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::_ans = 0;
typedef double(*_Func_Type)(double);

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
const _Func_Type _Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::Function[21] {
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

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
_Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::_Base_FormulaPaser(const _Ty* text)
{
    const _Ty* end = text;
    if (!*end || !end)
    {
        _error_state = _Error_State::Null;
        return;
    }
    _error_state = _Error_State::NoError;
    while (*++end)
    {
        if (!isascii(*end) || isspace(*end) || !(strchr("+-*/^().", *end) || isalnum(*end)))
        {
            // std::cout << "e1";
            _error_state = _Error_State::Invalid;
            return;
        }
    }
    _parse_all(text, end);
}

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
_Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::_Base_FormulaPaser(Sign type, const _Ty* begin, const _Ty* end) :
    _operation(type)
{
    _parse_all(begin, end);
}

/***
* 以运算符或括号或函数为分割界限, 解析出一个独立且正确的参与运算的成员
* 只能解析当前算式最外围的独立成员例如sin(a)+b*c被解析为sin(a), 返回指向+的指针, 再次解析将返回指向*的指针
* 解析失败返回 nullptr
*/

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
const _Ty* _Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::_parse_si(const _Ty* begin, const _Ty* end)
{
    auto ptr = begin;
    // std::cout << "parse_one is here.\n";
    if (strchr("+-*/^", *ptr))
    {
        _error_state = _Error_State::Invalid;
        return nullptr;
    }
    if (ptr >= end)
    {
        // std::cout << "empty str!\n";
        return nullptr;
    }
    unsigned left = 1;
    if (strchr("+-", *ptr)) ++ptr;
    if (isdigit(*ptr))
    {
        if (*ptr == '0' && *++ptr != '.') return ptr;
        while (isdigit(*ptr)) ++ptr;
        if (*ptr == '.' && isdigit(ptr[1]))
        {
            while (isdigit(*++ptr));
            return ptr;
        }
        // std::cout << "parse one is digit \\" << begin << "\\" << end << "\\" << "\n";
        return ptr;
    }
    while (islower(*ptr)) ++ptr;
    while (++ptr < end && left)
    {
        if (*ptr == '(') ++left;
        else if (*ptr == ')') --left;
    }
    if (left) return nullptr;
    else return ptr;
}

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
const _Ty* _Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::_parse_md(const _Ty* begin, const _Ty* end)
{
    if (begin >= end) return nullptr;
    // std::cout << "parse_md is here. \\" << begin << "\\" << end << "\\\n";
    const _Ty* ptr = _parse_si(begin, end);
    while (ptr && ptr != end && strchr("*/^", *ptr))
    {
        ptr = _parse_si(++ptr, end);
        if (!ptr) return nullptr;
    }
    return ptr;
}

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
const _Ty* _Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::_parse_pw(const _Ty* begin, const _Ty* end)
{
    if (begin >= end) return nullptr;
    // std::cout << "parse_pw is here. \\" << begin << "\\" << end << "\\\n";
    const _Ty* ptr = _parse_si(begin, end);
    if (*ptr != '^') return ptr;
    return _parse_si(++ptr, end);
}

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
int  _Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::_count_bracket(const _Ty* begin, const _Ty* end)
{
    const _Ty* ptr;
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

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
void _Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::_parse_val(const _Ty* left, const _Ty* right)
{
    if (*left == '-')
    {
        _negative = -1;
        ++left;
    }
    else if (*left == '+')
        ++left;
    if (isdigit(*left))
    {
        // std::cout << "this is a digit.  \\" << left << '\\' << right << "\\\n";
        _Istream(_String(left, right)) >> _value;
        // std::cout << "digit value is " << _value << std::endl;
        return;
    }
    if (*left == '(')
    {
        int k = _count_bracket(left, right);
        _parse_all(left + k, right - k);
        return;
    }
    else if (m_strncmp<_Ty>(left, "arcsin(", 7))
    {
        left += 7;
        _func = Func::Asin;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "arccos(", 7))
    {
        left += 7;
        _func = Func::Acos;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "arctan(", 7))
    {
        left += 7;
        _func = Func::Atan;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "arcsinh(", 8))
    {
        left += 8;
        _func = Func::Asinh;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "arccosh(", 8))
    {
        left += 8;
        _func = Func::Acosh;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "arctanh(", 8))
    {
        left += 8;
        _func = Func::Atanh;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "sqrt(", 5))
    {
        left += 5;
        _func = Func::Sqrt;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "sin(", 4))
    {
        left += 4;
        _func = Func::Sin;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "cos(", 4))
    {
        left += 4;
        _func = Func::Cos;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "tan(", 4))
    {
        left += 4;
        _func = Func::Tan;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "sinh(", 5))
    {
        left += 5;
        _func = Func::Sinh;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "cosh(", 5))
    {
        left += 5;
        _func = Func::Cosh;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "tanh(", 5))
    {
        left += 5;
        _func = Func::Tanh;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "ln(", 3))
    {
        left += 3;
        _func = Func::Ln;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "lg(", 3))
    {
        left += 3;
        _func = Func::Lg;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "exp(", 4))
    {
        left += 4;
        _func = Func::Exp;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "rad(", 4))
    {
        left += 4;
        _func = Func::Rad;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "deg(", 4))
    {
        left += 4;
        _func = Func::Deg;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "factorial(", 10))
    {
        left += 10;
        _func = Func::Factorial;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "abs(", 4))
    {
        left += 4;
        _func = Func::Abs;
        --right;
    }
    else if (m_strncmp<_Ty>(left, "pi()", 4))
    {
        _value = _PI_;
        return;
    }
    else if (m_strncmp<_Ty>(left, "e()", 3))
    {
        _value = _E_;
        return;
    }
    else if (m_strncmp<_Ty>(left, "ans()", 5))
    {
        _value = _ans;
        return;
    }
    else if (m_strncmp<_Ty>(left, "rand()", 5))
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
    if (*left == ')')
    {
        _error_state = _Error_State::Invalid;
        return;
    }
    // std::cout << "re parse all left: " << left << " right " << right << "\n";
    _son.emplace_back(Sign::N, left, right);
}

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
void _Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::_parse_all(const _Ty* begin, const _Ty* end)
{
    if (begin >= end)
    {
        // std::cout << "empty str\n";
        return;
    }
    // std::cout << " =================== begin all ==================== \n";
    const _Ty* ptr1 = strchr("+-*/", *begin) ? begin + 1 : begin;
    const _Ty* ptr2 = _parse_si(ptr1, end);

    if (ptr2 && ptr2 != end && !strchr("+-*/^", ptr2[1]))
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
                if (!ptr2)
                {
                    // std::cout << "out md.\n";
                    _error_state = _Error_State::Wait;
                    return;
                }
                // std::cout << "-------------> ptr1: " << ptr1 << '\n';
                // std::cout << "-------------> ptr2: " << ptr2 << '\n';
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
            if (!ptr2)
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
                    ptr2 = _parse_md(++ptr2, end);
                    if (!ptr2)
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
                    ptr2 = _parse_pw(++ptr2, end);
                    if (!ptr2) return;
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
            if (!ptr2)
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
                    _son.back()._son.emplace_back(Sign::P, ++ptr2, end);
                    return;
                }
                if (*begin == '-') _negative *= -1;
                _son.emplace_back(Sign::N, ptr1, ptr2);
                while (ptr2 != end)
                {
                    ptr1 = ptr2;
                    ptr2 = _parse_pw(++ptr2, end);
                    if (!ptr2)
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
                    ptr2 = _parse_md(++ptr2, end);
                    if (!ptr2)
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

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
double _Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::calculate()
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

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
_String _Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::outstr(bool remember)
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

template <typename _Ty, typename _String, typename _Istream, typename _Ostream>
void _Base_FormulaPaser<_Ty, _String, _Istream, _Ostream>::print(bool re)
{
    // std::cout << "value: " << _value << " child: " << _son.size() << " Operator: " << (int)_operation << (re ? " ||| \n" : " ||| ");
    for (size_t i = 0; i < _son.size(); ++i)
    {
        _son[i].print(i == _son.size() - 1);
    }
}

template <typename _Ty>
class FormulaPaser : private std::conditional_t<std::is_same<_Ty, char>::value,
        _Base_FormulaPaser<_Ty, std::string, std::istringstream, std::ostringstream>,
        _Base_FormulaPaser<_Ty,std::wstring, std::wistringstream, std::wostringstream>>
{
private:
    typedef std::conditional_t<std::is_same<_Ty, char>::value, _Base_FormulaPaser<_Ty, std::string, std::istringstream, std::ostringstream>, _Base_FormulaPaser<_Ty, std::wstring, std::wistringstream, std::wostringstream>> _parent;
    typedef std::conditional_t<std::is_same<_Ty, char>::value, std::string, std::wstring> _String;
    typedef std::conditional_t<std::is_same<_Ty, char>::value, std::ostringstream, std::wostringstream> _Ostream;
public:
    using _parent::calculate;
    FormulaPaser(const _Ty* text): _parent(text){};
    using _parent::outstr;
};
