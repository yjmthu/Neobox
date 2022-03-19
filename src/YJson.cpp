//#include <iostream>
//#include <ostream>
#include <fstream>
#include <deque>
#include <limits>
#include <iomanip>
#include <utility>
#include <cmath>
#include <algorithm>

#include "yjson.h"
#include "ystring.h"
#include "yencode.h"

#ifdef max
#undef max
#endif // max

#ifdef min
#undef min
#endif // min

typedef unsigned char byte;

inline const char* goEnd(const char* begin)
{
    while (*begin) ++begin;
    return begin;
}

const unsigned char YJson::utf8bom[] = {0xEF, 0xBB, 0xBF};
const unsigned char YJson::utf16le[] = {0xFF, 0xFE};

YJson::YJson(const char* str) { strict_parse<const char*>(str, goEnd(str)); }

YJson::YJson(const std::string& str) { strict_parse<std::string::const_iterator>(str.cbegin(), str.cend()); }

YJson::YJson(const std::wstring& str)
{
    std::deque<char> dstr;
    YEncode::utf16LE_to_utf8<std::deque<char>&, std::wstring::const_iterator>(dstr, str.cbegin());
    strict_parse(dstr.cbegin(), dstr.cend());
}

YJson::YJson(const wchar_t* str)
{
    std::deque<char> dstr;
    YEncode::utf16LE_to_utf8<std::deque<char>&, const wchar_t*>(dstr, str);
    strict_parse(dstr.cbegin(), dstr.cend());
}

YJson::YJson(const std::initializer_list<const char*>& lst)
{
    YJson* iter, **child = &_child;
    for (const auto& c: lst)
    {
        iter = new YJson();
        auto len = strlen(c) + 1;
        iter->_value = new char[len];
        std::copy(c, c+len, iter->_value);
        iter->_type = YJson::String;
        child = &(*child = iter)->_next;
    }
}

template<typename T>
void YJson::strict_parse(T temp, T end)
{
    temp = YString::StrSkip(temp);
    switch (*temp)
    {
        case '{':
            // qout << "object";
            parse_object(temp, end);
            break;
        case '[':
            // qout << "array";
            parse_array(temp, end);
            break;
        default:
            break;
    }
}

void YJson::loadFile(std::ifstream& file)
{
    // qout << "从文件加载";
    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        long long size = file.tellg();
        if (size < 6)
        {
            file.close();
            return ;
        }
        unsigned char c[3] = {0}; file.seekg(0, std::ios::beg);
        if (!(file.read(reinterpret_cast<char*>(c), sizeof(char) *3)))
        {
            file.close();
            throw std::string("文件不可读");
        }
        size -= sizeof(char) * 3;
        if (std::equal(c, c+3, utf8bom))
        {
            // qout << "utf8编码格式";
            std::vector<char> json_vector;
            json_vector.resize(size + 1);
            json_vector[size] = 0;
            file.read(&json_vector.front(), size);
            file.close();
            strict_parse(json_vector.cbegin(), json_vector.cend());
        }
        else if (std::equal(c, c+2, utf16le))
        {
            // qout << "utf16编码格式";
            if ((size + sizeof (char)) % sizeof (wchar_t))
            {
                throw std::string("文件编码出错");
            } else {
                std::deque<char> json_str;
                wchar_t* json_wstr = new wchar_t[size/sizeof(wchar_t) + 1];
                char *ptr = reinterpret_cast<char*>(json_wstr); *ptr = c[2];
                file.read(++ptr, size);
                YEncode::utf16LE_to_utf8<std::deque<char>&, const wchar_t*>(json_str, json_wstr);
                delete [] json_wstr; file.close();
                strict_parse(json_str.cbegin(), json_str.cend());
            }
        } else {
            file.close();
            throw std::string("文件编码出错");
        }
    } else {
        throw std::string("文件未打开");
    }
    //qout << "文件初始化结束";
}

//复制json，当parent为NULL，如果自己有key用自己的key
void YJson::CopyJson(const YJson* json, YJson* parent)
{
    if (!(_parent = parent) || parent->_type != YJson::Object)
    {
        delete[] _key; _key = nullptr;
    }
    else if (!_key && json->_key)
    {
        _key = YString::StrJoin<char>(json->_key);
    }

    if (_type == YJson::Array || _type == YJson::Object)
    {
        YJson *child = nullptr, *childx = nullptr;
        if ((child = json->_child))
        {
            childx = _child = new YJson(static_cast<YJson>(_type));
            _child->_type = child->_type;
            _child->CopyJson(child, this);
            while ((child = child->_next))
            {
                childx->_next = new YJson;
                childx->_next->_type = child->_type;
                childx->_next->CopyJson(child, this);
                childx->_next->_prev = childx; childx = childx->_next;
            }
        }
    }
    else if (_type == YJson::String)
        _value = YString::StrJoin<char>(reinterpret_cast<char*>(json->_value));
    else if (_type == YJson::Number)
    {
        _value = new char[sizeof (double)];
        memcpy(_value, json->_value, sizeof (double));
    }
}

bool YJson::isSameTo(const YJson *other)
{
    if (this == other)
        return true;
    if (this->_type != other->_type)
        return false;
    if (_key) {
        if (!other->_key)
            return false;
        else if (strcmp(_key, other->_key))
            return false;
    } else if (other->_key)
        return false;
    switch (_type) {
    case YJson::Array:
    case YJson::Object:
    {
        YJson* child1 = _child, *child2 = other->_child;
        if (child1) {
            if (!child2)
                return false;
            else {
                if (child1->_next) {
                    if (!child2->_next)
                        return false;
                    else
                        return child1->isSameTo(child2) && child1->_next->isSameTo(child2->_next);
                } else if (child2->_next)
                    return false;
                else
                    return child1->isSameTo(child2);
            }
        } else if (child2)
            return false;
        else
            return true;
    }
    case YJson::Number:
        return !memcmp(_value, other->_value, sizeof (double));
    case YJson::String:
        return !strcmp(_value, other->_value);
    case YJson::Null:
    case YJson::False:
    case YJson::True:
    default:
        return true;
    }
}

int YJson::getChildNum() const
{
    int j = 0; YJson* child = _child;
    if (_child) {
        ++j;
        while ((child = child->_next))
            ++j;
    }
    return j;
}

std::string YJson::urlEncode() const
{
    std::string param;
    char* value = nullptr;
    if (_type == YJson::Object && _child)
    {
        YJson* ptr = _child;
        do {
            param += ptr->_key;
            param.push_back('=');
            switch (ptr->_type) {
            case YJson::Number:
                value = ptr->print_number();
                param += value;
                delete [] value;
                break;
            case YJson::String:
                param.append(ptr->_value);
                break;
            case YJson::True:
                param.append("true");
                break;
            case YJson::False:
                param.append("false");
                break;
            case YJson::Null:
            default:
                param.append("null");
            }
            if ((ptr = ptr->_next))
                param.push_back('&');
            else
                break;
        } while (true);
    }
    for (auto iter = std::find(param.begin(), param.end(), ' '); iter!=param.end(); iter = std::find(param.begin(), param.end(), ' '))
        param.replace(iter, iter+1, "%20");
    return param;//::urlEncode(param.cbegin(), param.length());
}

std::string YJson::urlEncode(const char *url) const
{
    std::string param(url);
    char* value = nullptr;
    if (_type == YJson::Object && _child)
    {
        YJson* ptr = _child;
        do {
            param += ptr->_key;
            param.push_back('=');
            switch (ptr->_type) {
            case YJson::Number:
                value = ptr->print_number();
                param += value;
                delete [] value;
                break;
            case YJson::String:
                param.append(ptr->_value);
                break;
            case YJson::True:
                param.append("true");
                break;
            case YJson::False:
                param.append("false");
                break;
            case YJson::Null:
            default:
                param.append("null");
            }
            if ((ptr = ptr->_next))
                param.push_back('&');
            else
                break;
        } while (true);
    }
    for (auto iter = std::find(param.begin(), param.end(), ' '); iter!=param.end(); iter = std::find(param.begin(), param.end(), ' '))
        param.replace(iter, iter+1, "%20");
    return param;//::urlEncode(param.cbegin(), param.length());
}

std::string YJson::urlEncode(const std::string& url) const
{
    std::string param(url);
    char* value = nullptr;
    if (_type == YJson::Object && _child)
    {
        YJson* ptr = _child;
        do {
            param += ptr->_key;
            param.push_back('=');
            switch (ptr->_type) {
            case YJson::Number:
                value = ptr->print_number();
                param += value;
                delete [] value;
                break;
            case YJson::String:
                param.append(ptr->_value);
                break;
            default:
                param.append("null");
            }
            if ((ptr = ptr->_next))
                param.push_back('&');
            else
                break;
        } while (true);
    }
    for (auto iter = std::find(param.begin(), param.end(), ' '); iter!=param.end(); iter = std::find(param.begin(), param.end(), ' '))
        param.replace(iter, iter+1, "%20");
    return param;//::urlEncode(param.cbegin(), param.length());
}

YJson::~YJson()
{
    delete [] _value;
    delete[] _key;
    delete _child;
    if (_prev) _prev->_next = nullptr;
    delete _next;
}

YJson* YJson::find(const char* key) const
{
    if (_type == YJson::Object)
    {
        YJson *child = _child;
        if (child && child->_key)
        {
            do {
                if (!YString::strcmp<const char*>(child->_key, key))
                    return child;
            } while ((child = child->_next));
        }
    }
    return nullptr;
}

YJson *YJson::find(const std::string &key) const
{
    if (_type == YJson::Object)
    {
        YJson *child = _child;
        if (child && child->_key)
        {
            do {
                if (child->_key == key)
                    return child;
            } while ((child = child->_next));
        }
    }
    return nullptr;
}

YJson* YJson::find(int key) const
{
    if (_type == YJson::Array || _type == YJson::Object)
    {
        YJson *child = _child;
        if (child)
        {
            if (key >= 0)
            {
                do {
                    if (!(key--)) return child;
                } while ((child = child->_next));
            }
            else
            {
                int i = 0;
                while (child->_next) ++i, child = child->_next;
                if (-key <= ++i)
                {
                    while (++key) child = child->_prev;
                    return child;
                }
                
            }
        }
    }
    return nullptr;
}

YJson* YJson::findByVal(double value) const
{
    if (_type == YJson::Array || _type == YJson::Object)
    {
        YJson *child = _child;
        if (child) do
            if (child->_type == YJson::Number && fabs(value-*reinterpret_cast<double*>(child->_value)) <= std::numeric_limits<double>::epsilon())
                return child;
        while ((child = child->_next));
        
    }
    return nullptr;
}

YJson* YJson::findByVal(const char* str) const
{
    if (_type == YJson::Array || _type == YJson::Object)
    {
        YJson *child = _child;
        if (child) do
            if (child->_type == YJson::String && !YString::strcmp<const char*>(reinterpret_cast<char*>(child->_value), str))
                return child;
        while ((child = child->_next));
        
    }
    return nullptr;
}

YJson* YJson::getTop() const
{
    const YJson* top = this;
    while (true)
    {
        if (top->_parent) top = top->_parent;
        else return const_cast<YJson*>(top);
    }
}

YJson* YJson::append(const YJson& js, const char* key)
{
    if (_type == YJson::Object)
        {if (!key && !js._key) return nullptr;}
    else
        {if (key || js._key) return nullptr;}

    YJson* child = append(js._type); *child = js;
    if (key)
    {
        delete[] child->_key;
        child->_key = YString::StrJoin<char>(key);
    }
    return child;
}

YJson* YJson::append(YJson::Type type, const char* key)
{
    YJson* child = _child;
    if (child)
    {
        while (child->_next) child = child->_next;
        child->_next = new YJson;
        child->_next->_prev = child; child = child->_next;
    }
    else
    {
        _child = child = new YJson; _child->_parent = this;
    }
    child->_parent = this;
    child->_type = type;
    switch (type) {
    case YJson::String:
        child->_value = new char[1] {0};
        break;
    case YJson::Number:
        child->_value = new char[sizeof (double)] {0};
    default:
        break;
    }
    if (key) child->_key = YString::StrJoin<char>(key);
    return child;
}

YJson* YJson::append(double value, const char* key)
{
    if (_type == YJson::Object)
        {if (!key) return nullptr;}
    else
        {if (key) return nullptr;}
    YJson* child = append(YJson::Number);
    child->_value = new char[sizeof (double)];
    memcpy(child->_value, &value, sizeof (double));
    if (key) child->_key = YString::StrJoin<char>(key);
    return child;
}

YJson* YJson::append(const char* str, const char* key)
{
    if (_type == YJson::Object) {
        if (!key) return nullptr;
    }
    else if (key || _type != YJson::Array) return nullptr;
    YJson* child = append(YJson::String);
    child->_value = YString::StrJoin<char>(str); if (key) child->_key = YString::StrJoin<char>(key);
    return child;
}

YJson *YJson::append(const std::string &str, const char *key)
{
    if (_type == YJson::Object) {
        if (!key) return nullptr;
    }
    else if (key || _type != YJson::Array) return nullptr;
    YJson* child = append(YJson::String);
    child->_value = new char[str.length()+1] { 0 };
    std::copy(str.begin(), str.end(), child->_value);
    child->_value[str.length()] = 0;
    if (key) child->_key = YString::StrJoin<char>(key);
    return child;
}

bool YJson::remove(YJson* item)
{
    if (item)
    {
        if (item->_prev)
        {
            if ((item->_prev->_next = item->_next))
                item->_next->_prev = item->_prev;
        }
        else
        {
            if (item->_parent && (item->_parent->_child = item->_next))
            {
                item->_parent->_child->_prev = nullptr;
            }
        }
        item->_prev = nullptr; item->_next = nullptr;
        delete item;
        return true;
    }
    else
        return false;
}

bool YJson::isUtf8BomFile(const std::string &path)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (file.is_open()) {
        unsigned char bom[3];
        if (file.read(reinterpret_cast<char*>(bom), 3)){
            if (std::equal(bom, bom + 3, utf8bom)) {
                file.close();
                return true;
            }
        } else {
            file.close();
        }
    }
    return false;
}

YJson& YJson::operator=(const YJson& s)
{
    if (&s == this)
        return *this;
    else if (s.getTop() == this->getTop())
        return (*this = YJson(s));
    delete [] _value;
    _value = nullptr;
    delete _child, _child = nullptr;
    _type = s._type;
    CopyJson(&s, _parent);
    return *this;
}

bool YJson::isSameTo(const YJson &other, bool cmpKey)
{
    if (this == &other)
        return true;
    if (this->_type != other._type)
        return false;
    if (cmpKey) {
        if (_key) {
            if (!other._key)
                return false;
            else if (strcmp(_key, other._key))
                return false;
        } else if (other._key)
            return false;
    }
    switch (_type) {
    case YJson::Array:
    case YJson::Object:
    {
        YJson* child1 = _child, *child2 = other._child;
        if (child1) {
            if (!child2)
                return false;
            else {
                if (child1->_next) {
                    if (!child2->_next)
                        return false;
                    else
                        return child1->isSameTo(child2) && child1->_next->isSameTo(child2->_next);
                } else if (child2->_next)
                    return false;
                else
                    return child1->isSameTo(child2);
            }
        } else if (child2)
            return false;
        else
            return true;
    }
    case YJson::Number:
        return !memcmp(_value, other._value, sizeof (double));
    case YJson::String:
        return !strcmp(_value, other._value);
    case YJson::Null:
    case YJson::False:
    case YJson::True:
    default:
        return true;
    }
}

YJson& YJson::operator=(YJson&& s) noexcept
{
    std::swap(_type, s._type);
    std::swap(_child, s._child);
    std::swap(_value, s._value);
    return *this;
}

bool YJson::join(const YJson & js)
{
    if (&js == this)
        return join(YJson(*this));
    if (_type != js._type || (_type != YJson::Array && _type != YJson::Object))
        return false;
    YJson *child = _child;
    YJson *childx = js._child;
    if (child)
    {
        while (child->_next) child = child->_next;
        if (childx)
        {
            do {
                child->_next = new YJson(*childx);
                child->_next->_key = YString::StrJoin<char>(childx->_key);
                child->_next->_prev = child;
                child = child->_next;
            } while ((childx = childx->_next));
        }
    }
    else if (childx)
    {
        *this = js;
    }
    return true;
}

YJson YJson::join(const YJson & j1, const YJson & j2)
{
    if ((j1._type != YJson::Array && j1._type != YJson::Object) || j2._type != j1._type)
    {
        return YJson();
    }
    else
    {
        YJson js(j1); js.join(j2);
        return YJson(js);
    }
}

char* YJson::toString(bool fmt)
{
    return fmt? print_value(0):print_value();
}

bool YJson::toFile(const std::string name, const YJson::Encode& file_encode, bool fmt)
{
    //qout << "开始打印" << name;
    char* buffer = fmt ? print_value(0): print_value();
    //qout << "打印成功" << buffer;
    if (buffer)
    {
        switch (file_encode) {
        case (YJson::UTF16LE):
        {
            //std::cout << "UTF-16" << u8"保存开始。";
            std::wstring data;
            data.push_back(*reinterpret_cast<const wchar_t*>(utf16le));
            YEncode::utf8_to_utf16LE<std::wstring&>(data, buffer);
            data.back() = L'\n';
            std::ofstream outFile(name, std::ios::out | std::ios::binary);
            if (outFile.is_open())
            {
                outFile.write(reinterpret_cast<const char*>(data.data()), data.length() * sizeof(wchar_t));
                outFile.close();
            }
            break;
        }
        case UTF8BOM:
        {
            //cout << "UTF-8" << "保存开始。";
            std::ofstream outFile(name, std::ios::out | std::ios::binary);
            if (outFile.is_open())
            {
                outFile.write(reinterpret_cast<const char*>(utf8bom), 3);
                outFile.write((const char*)(buffer), strlen(buffer));
                outFile.write("\n", sizeof(char));
                outFile.close();
            }
            break;
        }
        default:
        {
            //cout << "UTF-8" << "保存开始。";
            std::ofstream outFile(name, std::ios::out | std::ios::binary);
            if (outFile.is_open())
            {
                outFile.write((const char*)(buffer), strlen(buffer));
                outFile.write("\n", sizeof(char));
                outFile.close();
            }
            break;
        }
        }
        delete [] buffer;
    }
    return false;
}

void YJson::loadFile(const std::string &path, YJson::Encode encode)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    std::string json_vector;
    if (!file.is_open())
    {
        throw std::string("文件不可读");
        return;
    }
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    if (size < 6)
    {
        file.close();
        return ;
    }
    file.seekg(0, std::ios::beg);
    switch (encode){
    case YJson::UTF8BOM:
    {
        // qout << "UTF8BOM";
        file.seekg(3, std::ios::beg);
        json_vector.resize(size - 2);
        file.read(reinterpret_cast<char*>(&json_vector[0]), size - 3);
        json_vector.back() = '\0';
        file.close();
        strict_parse(json_vector.cbegin(), json_vector.cend());
        break;
    }
    case YJson::UTF8:
    {
        // qout << "UTF8";
        unsigned char bom[3] {0};
        if (!(file.read(reinterpret_cast<char*>(bom), 3)))
        {
            file.close();
            throw std::string("文件不可读");
        }
        if (std::equal(bom, bom + 3, utf8bom))
        {
            json_vector.resize(size - 2);
            file.read(reinterpret_cast<char*>(&json_vector[0]), size - 3);
        }
        else
        {
            json_vector.resize(size + 1);
            std::copy(bom, bom + 3, json_vector.begin());
            file.read(reinterpret_cast<char*>(&json_vector[3]), size - 3);
        }
        json_vector.back() = '\0';
        file.close();
        strict_parse(json_vector.cbegin(), json_vector.cend());
        break;
    }
    case YJson::UTF16LEBOM:
    {
        // qout << "UTF16BOM";
        file.seekg(2, std::ios::beg);
        std::wstring json_wstr(size / sizeof(wchar_t), 0);
        file.read(reinterpret_cast<char*>(&json_wstr[0]), size - 2);
        YEncode::utf16LE_to_utf8<std::string&, std::wstring::const_iterator>(json_vector, json_wstr.cbegin());
        file.close();
        strict_parse(json_vector.cbegin(), json_vector.cend());
        break;
    }
    case YJson::UTF16LE:
    {
        unsigned char bom[2] {0};
        if (!(file.read(reinterpret_cast<char*>(bom), 2)))
        {
            file.close();
            throw std::string("文件不可读");
        }
        std::wstring json_wstr;
        if (std::equal(bom, bom+2, utf16le))
        {
            json_wstr.resize(size / sizeof(wchar_t));
            file.read(reinterpret_cast<char*>(&(json_wstr.front())), size - 2);
        }
        else
        {
            json_wstr.resize(size / sizeof(wchar_t) + 1);
            char *ptr = reinterpret_cast<char*>(&(json_wstr.front()));
            std::copy(bom, bom+2, ptr);
            file.read(ptr+2, size - 2);
        }
        json_wstr.back() = '\0';
        YEncode::utf16LE_to_utf8<std::string&, std::wstring::const_iterator>(json_vector, json_wstr.cbegin());
        file.close();
        strict_parse(json_vector.cbegin(), json_vector.cend());
        break;
    }
    default:
        loadFile(file);
    }

}

char* YJson::joinKeyValue(const char* valuestring, int depth, bool delvalue)
{
    char* buffer = nullptr;
    char* x = YString::StrRepeat(' ', depth*4);
    if (_key)
    {
        char *key = print_string(_key);
        buffer = YString::StrJoin<char>(x, key, ": ", valuestring);
        delete[] key;
    }
    else
    {
        buffer = YString::StrJoin<char>(x, valuestring);
    }
    delete[] x;
    if (delvalue) delete[] valuestring;
    return buffer;
}

char* YJson::joinKeyValue(const char* valuestring,  bool delvalue)
{
    char* buffer = nullptr;
    if (_key)
    {
        char *key = print_string(_key);
        buffer = YString::StrJoin<char>(key, ":", valuestring);
        delete[] key;
    }
    else
    {
        buffer = YString::StrJoin<char>(valuestring);
    }
    if (delvalue) delete[] valuestring;
    return buffer;
}

template <typename T>
T YJson::parse_value(T value, T end)
{
    //qout << "加载数据";
    if (end <= value) return end;
    //qout << "剩余字符串开头：" << *value;
    if (!YString::strncmp(value, "null", 4))     { _type= YJson::Null;  return value+4; }
    if (!YString::strncmp(value, "false", 5))    { _type= YJson::False; return value+5; }
    if (!YString::strncmp(value, "true", 4))     { _type= YJson::True;  return value+4; }
    if (*value=='\"')                   { return parse_string(value, end); }
    if (*value=='-' || (*value>='0' && *value<='9'))    { return parse_number(value, end); }
    if (*value=='[')                    { return parse_array(value, end); }
    if (*value=='{')                    { return parse_object(value, end); }

    //qout << "加载数据出错";
    return end;                              /* failure. */
}

char* YJson::print_value()
{
    switch (_type)
    {
    case YJson::Null:
        return  joinKeyValue("null", false);
    case YJson::False:
        return joinKeyValue("false", false);
    case YJson::True:
        return joinKeyValue("true", false);
    case YJson::Number:
        return joinKeyValue(print_number(), true);
    case YJson::String:
        return joinKeyValue(print_string(_value), true);;
    case YJson::Array:
        return print_array();
    case YJson::Object:
        return print_object();
    default:
        return nullptr;
    }
}

char* YJson::print_value(int depth)
{
    switch (_type)
    {
    case YJson::Null:
        return  joinKeyValue("null", depth, false);
    case YJson::False:
        return joinKeyValue("false", depth, false);
    case YJson::True:
        return joinKeyValue("true", depth, false);
    case YJson::Number:
        return joinKeyValue(print_number(), depth, true);
    case YJson::String:
        return joinKeyValue(print_string(_value), depth, true);
    case YJson::Array:
        return print_array(depth);
    case YJson::Object:
        return print_object(depth);
    default:
        return nullptr;
    }
}

template<typename T>
T YJson::parse_number(T num, T)
{
    _type = YJson::Number;
    _value = new char[sizeof (double)] {0};
    double *value_ptr = reinterpret_cast<double*>(_value);
    short sign = 1;
    int scale = 0;
    int signsubscale = 1, subscale = 0;
    if (*num=='-') {
        sign = -1;
        ++num;
    }
    if (*num=='0')
    {
        return ++num;
    }
    if ('1' <= *num && *num <= '9')
        do *value_ptr += (*value_ptr *= 10.0, *num++ - '0');
        while (isdigit(*num));
    if ('.' == *num && isdigit(num[1]))
    {
        ++num;
        do  *value_ptr += (*value_ptr *= 10.0, *num++ - '0'), scale--;
        while (isdigit(*num));
    }
    if ('e' == *num || 'E' == *num)
    {
        if (*++num == '-')
        {
            signsubscale = -1;
            ++num;
        }
        else if (*num == '+')
        {
            ++num;
        }
        while (isdigit(*num))
        {
            subscale *= 10;
            subscale += *num++ - '0';
        }
    }
    *value_ptr *= sign * pow(10, scale + signsubscale * subscale);
    return num;
}

char* YJson::print_number()
{
    char* buffer = nullptr;
    double valuedouble = *reinterpret_cast<double*>(_value);
    //qout << "原来的值" << valuedouble;
    if (valuedouble == 0)
    {
        //qout << "恰好为0";
        buffer = YString::StrJoin<char>("0");
    }
    else if (fabs(round(valuedouble) - valuedouble) <= std::numeric_limits<double>::epsilon() && valuedouble <= (double)std::numeric_limits<int>::max() && valuedouble >= (double)std::numeric_limits<int>::min())
    {
        //qout << "近似" << valuedouble;
        char temp[21] = { 0 }; sprintf(temp,"%.0lf",valuedouble);
        buffer = YString::StrJoin<char>(temp);
    }
    else
    {
        char temp[64] = {0};
        if (fabs(floor(valuedouble)-valuedouble)<=std::numeric_limits<double>::epsilon() && fabs(valuedouble)<1.0e60)
            sprintf(temp,"%.0f",valuedouble);
        else if (fabs(valuedouble)<1.0e-6 || fabs(valuedouble)>1.0e9)
            sprintf(temp,"%e",valuedouble);
        else
            sprintf(temp,"%f",valuedouble);
        buffer = YString::StrJoin<char>(temp);
    }
    //qout << "Buffer" << buffer;
    return buffer;
}

template<typename T>
inline bool _parse_hex4(T str, uint16_t& h)
{
    if (*str >= '0' && *str <= '9')
        h += (*str) - '0';
    else if (*str >= 'A' && *str <= 'F')
        h += 10 + (*str) - 'A';
    else if (*str >= 'a' && *str <= 'f')
        h += 10 + (*str) - 'a';
    else
        return true;
    return false;
}

template<typename T>
uint16_t parse_hex4(T str)
{
    uint16_t h = 0;
    if (_parse_hex4(str, h) || _parse_hex4(++str, h = h<<4) || _parse_hex4(++str, h = h<<4) || _parse_hex4(++str, h = h<<4))
        return 0;
    return h;
}

template<typename T>
T YJson::parse_string(T str, T end)
{
    //qout << "加载字符串";
    T ptr = str + 1;
    char* ptr2; uint32_t uc, uc2;
    size_t len = 0;
    if (*str != '\"') {
        //qout << "不是字符串！";
        throw std::string("不是字符串");
    }
    while (*ptr!='\"' && ptr != end && ++len) if (*ptr++ == '\\') ++ptr;    /* Skip escaped quotes. */
    _value = new char[len + 1];
    ptr = str + 1;
    ptr2 = _value;
    while (*ptr != '\"' && *ptr)
    {
        if (*ptr != '\\') *ptr2++ = *ptr++;
        else
        {
            ptr++;
            switch (*ptr)
            {
            case 'b': *ptr2++ = '\b';    break;
            case 'f': *ptr2++ = '\f';    break;
            case 'n': *ptr2++ = '\n';    break;
            case 'r': *ptr2++ = '\r';    break;
            case 't': *ptr2++ = '\t';    break;
            case 'u': uc=parse_hex4(ptr+1);ptr+=4;                                                   /* get the unicode char. */

                if ((uc>=YEncode::utf16FirstWcharMark[1] && uc<YEncode::utf16FirstWcharMark[2]) || uc==0)    break;    /* check for invalid.    */

                if (uc>=YEncode::utf16FirstWcharMark[0] && uc<YEncode::utf16FirstWcharMark[1])                         /* UTF16 surrogate pairs.    */
                {
                    if (ptr[1]!='\\' || ptr[2]!='u')    break;                                       /* missing second-half of surrogate.    */
                    uc2=parse_hex4(ptr+3);ptr+=6;
                    if (uc2<YEncode::utf16FirstWcharMark[1] || uc2>=YEncode::utf16FirstWcharMark[2])        break;     /* invalid second-half of surrogate.    */
                    uc=0x10000 + (((uc&0x3FF)<<10) | (uc2&0x3FF));
                }

                len=4;if (uc<0x80) len=1;else if (uc<0x800) len=2;else if (uc<0x10000) len=3; ptr2+=len;

                switch (len) {
                    case 4: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
                    case 3: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
                    case 2: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
                    case 1: *--ptr2 =(uc | YEncode::utf8FirstCharMark[len]);
                }
                ptr2+=len;
                break;
            default:  *ptr2++ = *ptr; break;
            }
            ptr++;
        }
    }
    *ptr2 = 0; ++str;
    _type= YJson::String;
    while (*str != '\"' && *str) if (*str++ == '\\') ++str;
    //qout << "所得字符串：" << _value;
    return ++str;
}

char* YJson::print_string(const char* const str)
{
    char* buffer = nullptr;
    //cout << "输出字符串" << str << endl;
    const char* ptr; char* ptr2;
    size_t len = 0, flag = 0; unsigned char token;
    
    for (ptr = str; *ptr; ptr++)
        flag |= ((*ptr > 0 && *ptr < 32) || (*ptr == '\"') || (*ptr == '\\')) ? 1 : 0;
    if (!flag)
    {
        len = ptr - str;
        buffer = new char[len + 3];
        ptr2 = buffer; *ptr2++ = '\"';
        memcpy(ptr2, str, len);
        *(ptr2 += len) = '\"'; *++ptr2 = 0;
        return buffer;
    }
    if (!str)
    {
        buffer = YString::StrJoin<char>("\"\"");
        return buffer;
    }
    ptr = str;
    while ((token = (unsigned char)*ptr) && ++len)
    {
        if (strchr("\"\\\b\f\n\r\t", token))
            len++;
        else if (token < 32)
            len += 5;
        ptr++;
    }
    
    memset(buffer = new char[len + 3], 0, (len + 3) * sizeof(char));

    ptr2 = buffer; ptr = str;
    *ptr2++ = '\"';
    while (*ptr)
    {
        if ((unsigned char)*ptr > 31 && *ptr != '\"' && *ptr != '\\') *ptr2++ = *ptr++;
        else
        {
            *ptr2++='\\';
            switch (token = (unsigned char)*ptr++)
            {
            case '\\':    *ptr2++ = '\\';   break;
            case '\"':    *ptr2++ = '\"';   break;
            case '\b':    *ptr2++ = 'b';    break;
            case '\f':    *ptr2++ = 'f';    break;
            case '\n':    *ptr2++ = 'n';    break;
            case '\r':    *ptr2++ = 'r';    break;
            case '\t':    *ptr2++ = 't';    break;
            default: sprintf(ptr2,"u%04x",token);ptr2+=5;    break;
            }
        }
    }
    *ptr2 = '\"'; *++ptr2 = 0;
    return buffer;
}

template<typename T>
T YJson::parse_array(T value, T end)
{
    //qout << "加载列表";
    YJson *child = nullptr;
    _type = YJson::Array;
    value=YString::StrSkip(++value);
    if (*value==']')
    {
        return value+1;    /* empty array. */
    }
    _child = child = new YJson;
    _child->_parent = this;
    value = YString::StrSkip(child->parse_value(YString::StrSkip(value), end));    /* skip any spacing, get the value. */
    if (value >= end) return end;

    while (*value==',')
    {
        if (*YString::StrSkip(value+1)==']')
        {
            return YString::StrSkip(value+1);
        }
        YJson *new_ = new YJson;
        child->_next = new_;
        new_->_prev = child;
        child = new_; child->_parent = this;
        value = YString::StrSkip(child->parse_value(YString::StrSkip(value + 1), end));
        if (value >= end) return end;
    }

    if (*value++ == ']')
        return value;
    throw std::string("未匹配到列表结尾！");
}

char* YJson::print_array()
{
    //qout << "打印列表开始";
    std::deque<char*> entries;
    YJson *child = _child;
    if (!child)
    {
        return joinKeyValue("[]", false);
    }
    do {
        entries.push_back(child->print_value());
    } while ((child = child->_next));
    return YString::StrJoin<1>(joinKeyValue("[", false), "]", ",", entries);
}

char* YJson::print_array(int depth)
{
    //qout << "打印列表开始";
    std::deque<char*> entries;
    YJson *child = _child;
    char*buffer = nullptr;
    if (!child)
        return joinKeyValue("[]", depth, false);
    do {
        entries.push_back(child->print_value(depth+1));
    } while ((child = child->_next));

    char* str_start = joinKeyValue("[\n", depth, false);
    ++(depth *= 4);
    char *str_end = new char[2+depth] {0};
    *str_end = '\n'; str_end[depth] = ']';
    while (--depth) str_end[depth] = ' ';
    //qout << "打印列表结束";
    buffer = YString::StrJoin<2>(str_start, str_end, ",\n", entries);
    delete [] str_end;
    return buffer;
}

template<typename T>
T YJson::parse_object(T value, T end)
{
    //qout << "加载字典：";
    YJson *child = nullptr;
    _type = YJson::Object;
    value = YString::StrSkip(++value);
    if (*value == '}') return value + 1;
    _child = child = new YJson;
    _child->_parent = this;
    value = YString::StrSkip(child->parse_string(YString::StrSkip(value), end));
    if (value >= end) return end;
    child->_key = child->_value;
    child->_value = nullptr;
    //qout << "中间断开";
    if (*value != ':')
    {
        throw std::string("加载字典时，没有找到冒号");
    }
    value = YString::StrSkip(child->parse_value(YString::StrSkip(value + 1), end));
    while (*value==',')
    {
        if (*YString::StrSkip(value+1)=='}')
        {
            return YString::StrSkip(value+1);
        }
        YJson *new_ = new YJson;
        child->_next = new_; new_->_prev = child; child = new_; child->_parent = this;
        value = YString::StrSkip(child->parse_string(YString::StrSkip(value+1), end));
        child->_key = child->_value; child->_value = nullptr;
        if (*value!=':')
        {
            throw std::string("加载字典时，没有找到冒号");
        }
        value = YString::StrSkip(child->parse_value(YString::StrSkip(value+1), end));    /* skip any spacing, get the value. */
        if (!(*value)) return end;
    }
    
    if (*value=='}')
    {
        //qout << "加载字典结束";
        return value+1;
    }
    throw std::string("加载字典时，没有找到结尾大括号");
    return end;
}

char* YJson::print_object()
{
    //qout << "打印字典开始";

    std::deque<char*> entries;
    YJson *child = _child;
    char* buffer = nullptr;
    if (!child) return joinKeyValue("{}", false);;
    do {
        if (!(buffer = child->print_value()))
            return nullptr;
        entries.push_back(buffer);
    } while ((child = child->_next));
    return YString::StrJoin<1>(joinKeyValue("{", false), "}", ",", entries);
}

char* YJson::print_object(int depth)
{
    //qout << "打印字典开始";
    std::deque<char*> entries;
    YJson *child = _child;
    char* buffer = nullptr;
    if (!child) return joinKeyValue("{}", depth, false);;
    do {
        if (!(buffer = child->print_value(depth+1)))
            return nullptr;
        entries.push_back(buffer);
    } while ((child = child->_next));
    //qout << "开始拼接字符串";
    char *str_start = joinKeyValue("{\n", depth, false);
    ++(depth *= 4);
    char *str_end = new char[2+depth] {0};
    *str_end = '\n'; str_end[depth] = '}';
    while (--depth) str_end[depth] = ' ';
    buffer = YString::StrJoin<2>(str_start, str_end, ",\n", entries);
    delete [] str_end;
    return buffer;
}
