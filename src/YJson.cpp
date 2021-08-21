#include <iostream>
#include <ostream>
#include <fstream>
#include <deque>
#include "funcbox.h"
#include "YJson.h"
#include "YString.h"
#include "YEncode.h"

#define TYPE_CHAECK()     static_assert(std::is_same<T, std::string::const_iterator>::value ||\
                              std::is_same<T, std::vector<char>::const_iterator>::value ||\
                              std::is_same<T, std::deque<char>::const_iterator>::value, "error in type")

YJSON_DEBUG YJsonItem::DEBUG_OUT_PUT = YJSON_DEBUG::SHOW;
std::pair<bool, std::string> YJsonItem::ep = std::pair<bool, std::string>(false, std::string());

YJsonItem::YJsonItem(const YJsonItem& js)
{
    CopyJson(&js, nullptr);
}

YJsonItem::YJsonItem(const std::string& str)
{
    qout << str.c_str();
    strict_parse(str.cbegin());
}

YJsonItem::YJsonItem(const std::wstring& str)
{
    std::string dstr;
    utf16_to_utf8<std::string&, std::wstring::const_iterator>(dstr, str.cbegin());
    strict_parse(dstr.cbegin());
}

template<typename T>
bool YJsonItem::strict_parse(T temp)
{
    TYPE_CHAECK();
    temp = StrSkip(temp);
    switch (*temp)
    {
        case '{':
            parse_object(temp);
            break;
        case '[':
            parse_array(temp);
            break;
        default:
            break;
    }
    if (ep.first)
        std::cout << "error:\t" << ep.second << std::endl;
    return ep.first;
}

YJsonItem* YJsonItem::newFromFile(const std::wstring& str)
{
    qout << "从文件加载";
    YJsonItem* json = nullptr;
    YJSON_ENCODE file_encode = YJSON_ENCODE::OTHER;
    std::ifstream file(str, std::ios::in | std::ios::binary);
    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        int size = file.tellg();
        if (size < 20)
        {
            file.close();
            return nullptr;
        }
        unsigned char c[3] = {0}; file.seekg(0, std::ios::beg);
        if (!(file.read(reinterpret_cast<char*>(c), sizeof(char) *3)))
        {
            file.close();
            return nullptr;
        }
        size -= sizeof(char) * 3;
        if (c[0] == 0xef && c[1] == 0xbb && c[2] == 0xbf)
        {
            qout << "utf8编码格式";
            std::vector<char> json_vector;
            file_encode = YJSON_ENCODE::UTF8;
            json_vector.resize(size + 1);
            json_vector[size] = 0;
            file.read(reinterpret_cast<char*>(&json_vector[0]), size);
            file.close();
            json = new YJsonItem();
            json->strict_parse(json_vector.cbegin());
        }
        else if (c[0] == 0xff && c[1] == 0xfe)
        {
            qout << "utf16编马哥";
            file_encode = YJSON_ENCODE::UTF16;
            if ((size + sizeof (char)) % sizeof (wchar_t))
            {
                file.close(); return nullptr;
            }
            else
            {
                std::string json_str;
                wchar_t* json_str_temp = new wchar_t[size/sizeof(wchar_t) + 1];
                char *ptr = reinterpret_cast<char*>(json_str_temp); *ptr = c[2];
                file.read(++ptr, size);
                utf16_to_utf8<std::string&, const wchar_t*>(json_str, json_str_temp);
                delete [] json_str_temp; file.close();
                json = new YJsonItem();
                json->strict_parse(json_str.cbegin());
            }
        }
        else
        {
            file_encode = YJSON_ENCODE::OTHER;
            file.close();
        }
        if (file_encode == YJSON_ENCODE::OTHER)
        {
            qout << "unsuported encode";
            ep.first = true;
            ep.second = "文件编码格式不支持。";
        }
    }
    else
    {
        ep.first = true; ep.second = "文件错误";
    }
    qout << "文件初始化结束";
    return json;
}

YJsonItem YJsonItem::Array()
{
    YJsonItem json_array = YJsonItem();
    json_array._type = YJSON_TYPE::YJSON_ARRAY;
    return json_array;
}

YJsonItem YJsonItem::Object()
{
    YJsonItem json_array = YJsonItem();
    json_array._type = YJSON_TYPE::YJSON_OBJECT;
    return json_array;
}

YJsonItem* YJsonItem::newArray()
{
    YJsonItem* json_array = new YJsonItem;
    json_array->_type = YJSON_TYPE::YJSON_ARRAY;
    return json_array;
}

YJsonItem* YJsonItem::newObject()
{
    YJsonItem* json_array = new YJsonItem;
    json_array->_type = YJSON_TYPE::YJSON_OBJECT;
    return json_array;
}

//复制json，当parent为NULL，如果自己有key用自己的key
void YJsonItem::CopyJson(const YJsonItem* json, YJsonItem* parent)
{
    if (json->ep.first) return;
    _type = json->_type;

    if (!(_parent = parent) || parent->_type != YJSON_TYPE::YJSON_OBJECT)
    {
        if (_keystring) delete[] _keystring; _keystring = nullptr;
    }
    else if (!_keystring && json->_keystring)
    {
        _keystring = StrJoin(json->_keystring);
    }

    if (_type == YJSON_TYPE::YJSON_ARRAY || _type == YJSON_TYPE::YJSON_OBJECT)
    {
        YJsonItem *child = nullptr, *childx = nullptr;
        if (child = json->_child)
        {
            childx = _child = new YJsonItem; _child->CopyJson(child, this);
            while (child = child->_next)
            {
                childx->_next = new YJsonItem; childx->_next->CopyJson(child, this);
                childx->_next->_prev = childx; childx = childx->_next;
            }
        }
    }
    else if (_type == YJSON_TYPE::YJSON_STRING)
        _valuestring = StrJoin(json->_valuestring);
    else if (_type == YJSON_TYPE::YJSON_NUMBER)
    {
        _valueint = json->_valueint;
        _valuedouble = json->_valuedouble;
    }
}

void YJsonItem::TakeJson(YJsonItem* js, YJsonItem* parent, bool copy_key)
{
    if (parent) _parent = parent;
    _type = js->_type; js->_type = YJSON_TYPE::YJSON_NULL;
    _child = js->_child; js->_child = nullptr;
    if (_child)
    {
        YJsonItem* child = _child; child->_parent = this;
        while (child = child->_next) child->_parent = this;
    }
    if (copy_key)
    {
        _keystring = js->_keystring; js->_keystring = nullptr;
    }
	_valuestring = js->_valuestring; js->_valuestring = nullptr;
	_valueint = js->_valueint; js->_valueint = 0;
	_valuedouble = js->_valuedouble; js->_valuedouble = 0;
    _buffer = js->_buffer; js->_buffer = nullptr;
    ep = js->ep; js->ep.first = false;
}

//清除_buffer, _valuestring, _child, 以及ep、valueint、 _valuedouble
void YJsonItem::clearContent()
{
    if (_buffer) (delete[] _buffer, _buffer = nullptr);
    if (_valuestring) (delete _valuestring, _valuestring = nullptr);
    if (_child) (delete _child, _child = nullptr);
    if (ep.first) ep.first = false; _valueint = 0; _valuedouble = 0;
}

int YJsonItem::getChildNum() const
{
    int j = 0; YJsonItem* child = _child;
    if (_child) { ++j; while (child = child->_next) ++j; }
    return j;
}

YJsonItem::~YJsonItem()
{
    if (_buffer) delete[] _buffer;
    if (_keystring) delete[] _keystring;
    if (_valuestring) delete[] _valuestring;
    if (_child) delete _child;
    if (_prev) _prev->_next = nullptr;
    if (_next)
    {
        YJsonItem * temp_next  = _next;
        _next = nullptr;
        temp_next->_prev = nullptr;
        delete temp_next;
    }
}

YJsonItem* YJsonItem::findItem(const char* key) const
{
    if (_type == YJSON_TYPE::YJSON_OBJECT)
    {
        YJsonItem *child = _child;
        if (child && child->_keystring)
        {
            do {
                qout << "比较键值：" << child->_keystring << key;
                if (!strcmp<const char*>(child->_keystring, key))
                    return child;
            } while (child = child->_next);
        }
    }
    return nullptr;
}

YJsonItem* YJsonItem::findItem(int key) const
{
    if (_type == YJSON_TYPE::YJSON_ARRAY)
    {
        YJsonItem *child = _child;
        if (child)
        {
            if (key >= 0)
            {
                do {
                    if (!(key--)) return child;
                } while (child = child->_next);
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

YJsonItem* YJsonItem::findItemByValue(int value) const
{
    if (_type == YJSON_TYPE::YJSON_ARRAY || _type == YJSON_TYPE::YJSON_OBJECT)
    {
        YJsonItem *child = _child;
        if (child) do 
            if (child->_type==YJSON_TYPE::YJSON_NUMBER && child->_valueint==value && fabs(((double)value)-child->_valuedouble) <= DBL_EPSILON)
                return child;
        while (child = child->_next);
    }
    return nullptr;
}

YJsonItem* YJsonItem::findItemByValue(double value) const
{
    if (_type == YJSON_TYPE::YJSON_ARRAY || _type == YJSON_TYPE::YJSON_OBJECT)
    {
        YJsonItem *child = _child;
        if (child) do
            if (child->_type == YJSON_TYPE::YJSON_NUMBER && fabs(value-child->_valuedouble) <= DBL_EPSILON)
                return child;
        while (child = child->_next);
        
    }
    return nullptr;
}

YJsonItem* YJsonItem::findItemByValue(const char* str) const
{
    if (_type == YJSON_TYPE::YJSON_ARRAY || _type == YJSON_TYPE::YJSON_OBJECT)
    {
        YJsonItem *child = _child;
        if (child) do
            if (child->_type == YJSON_TYPE::YJSON_STRING && !strcmp<const char*>(child->_valuestring, str))
                return child;
        while (child = child->_next);
        
    }
    return nullptr;
}

const YJsonItem* YJsonItem::getTopItem() const
{
    const YJsonItem* top = this;
    while (true)
    {
        if (top->_parent) top = top->_parent;
        else return top;
    }
}

YJsonItem* YJsonItem::appendItem(YJSON_TYPE type)
{
    YJsonItem* child = _child;
    if (child)
    {
        while (child->_next) child = child->_next;
        child->_next = new YJsonItem;
        child->_next->_prev = child; child = child->_next;
    }
    else
    {
        _child = child = new YJsonItem; _child->_parent = this;
    }
    child->_parent = this;
    child->_type = type;
    return child;
}

YJsonItem* YJsonItem::appendItem(const YJsonItem& js, const char* key)
{
    return appendItem(&js, key);
}

YJsonItem* YJsonItem::appendItem(const YJsonItem* js, const char* key)
{
    if (_type == YJSON_TYPE::YJSON_OBJECT)
        {if (!key && !js->_keystring) return nullptr;}
    else
        {if (key || js->_keystring) return nullptr;}
    YJsonItem* child = appendItem(js->_type); *child = js;
    if (key)
    {
        if (child->_keystring) delete[] child->_keystring;
        child->_keystring = StrJoin(key);
    }
    return child;
}

YJsonItem* YJsonItem::appendItem(int value, const char* key)
{
    if (_type == YJSON_TYPE::YJSON_OBJECT) {if (!key) return nullptr;}
    else {if (key) return nullptr;}
    YJsonItem* child = appendItem(YJSON_TYPE::YJSON_NUMBER);
    child->_valuedouble = child->_valueint = value;
    if (key) child->_keystring = StrJoin(key);
    return child;
}

YJsonItem* YJsonItem::appendItem(double value, const char* key)
{
    if (_type == YJSON_TYPE::YJSON_OBJECT)
        {if (!key) return nullptr;}
    else
        {if (key) return nullptr;}
    YJsonItem* child = appendItem(YJSON_TYPE::YJSON_NUMBER);
    child->_valueint = (int)value; child->_valuedouble = value;
    if (key) child->_keystring = StrJoin(key);
    return child;
}

YJsonItem* YJsonItem::appendItem(const char* str, const char* key)
{
    if (_type == YJSON_TYPE::YJSON_OBJECT) {
        if (!key) return nullptr;
    } else {
        if (key) return nullptr;
    }
    YJsonItem* child = appendItem(YJSON_TYPE::YJSON_STRING);
    child->_valuestring = StrJoin(str); if (key) child->_keystring = StrJoin(key);
    return child;
}

bool YJsonItem::removeItem(YJsonItem* item)
{
    if (item)
    {
        if (item->_prev)
        {
            if (item->_prev->_next = item->_next) item->_next->_prev = item->_prev;
        }
        else
        {
            if (_child = item->_next)
            {
                _child->_parent = this;
                _child->_prev = nullptr;
            }
        }
        item->_prev = nullptr; item->_next = nullptr; delete item;
        return true;
    }
    else
        return false;
}

bool YJsonItem::removeItem(int index)
{
    return removeItem(findItem(index));
}

bool YJsonItem::removeItem(const char* key)
{
    return removeItem(findItem(key));
}

bool YJsonItem::removeItemByValue(int value)
{
    return removeItem(findItemByValue(value));
}

bool YJsonItem::removeItemByValue(double value)
{
    return removeItem(findItemByValue(value));
}

bool YJsonItem::removeItemByValue(std::string str)
{
    return removeItem(findItemByValue(str.c_str()));
}

YJsonItem& YJsonItem::operator=(const YJsonItem* s)
{
    if (s == this)
        return *this;
    else if (s->getTopItem() == getTopItem())
        return (*this = YJsonItem(*s));
    clearContent(); CopyJson(s, _parent);
    return *this;
}

YJsonItem& YJsonItem::operator=(const YJsonItem& s)
{
    if (&s == this)
        return *this;
    else if (s.getTopItem() == this->getTopItem())
        return (*this = YJsonItem(s));
    clearContent(); CopyJson(&s, _parent);
    return *this;
}

YJsonItem& YJsonItem::operator=(YJsonItem&& s)
{
    if (&s == this) return *this;
    clearContent(); TakeJson(&s, _parent, false); s._next = nullptr;
    return *this;
}

YJsonItem& YJsonItem::operator=(int value)
{
    clearContent(); _type = YJSON_TYPE::YJSON_NUMBER;
    _valuedouble = _valueint = value;
    return *this;
}

YJsonItem& YJsonItem::operator=(double value)
{
    clearContent(); _type = YJSON_TYPE::YJSON_NUMBER;
    _valueint = (int)value; _valuedouble = value;
    return *this;
}

YJsonItem& YJsonItem::operator=(const char* str)
{
    clearContent(); _type = YJSON_TYPE::YJSON_STRING;
    _valuestring = StrJoin(str);
    return *this;
}

YJsonItem YJsonItem::operator+(const YJsonItem*that)
{
    if (ep.first || !that || that->ep.first || _type != YJSON_TYPE::YJSON_ARRAY || _type != YJSON_TYPE::YJSON_OBJECT || _type != that->_type)
    {
        YJsonItem s;
        return s;
    }
    else
    {
        YJsonItem js(*this); js += that;
        return js;
    }
}

YJsonItem YJsonItem::operator+(const YJsonItem& that)
{
    return *this + &that;
}

YJsonItem& YJsonItem::operator+=(const YJsonItem* js)
{
    if (js == this)
    {
        return *this += YJsonItem(*this);
    }
    if (ep.first || !js || js->ep.first || _type != js->_type || (_type != YJSON_TYPE::YJSON_ARRAY && _type != YJSON_TYPE::YJSON_OBJECT))
        return *this;
    YJsonItem *child = _child;
    YJsonItem *childx = js->_child;
    if (child)
    {
        while (child->_next) child = child->_next;
        if (childx)
        {
            do {
                child->_next = new YJsonItem(*childx);
                child->_next->_keystring = StrJoin(childx->_keystring);
                child->_next->_prev = child;
                child = child->_next;
            } while (childx = childx->_next);
        }
    }
    else if (childx)
    {
        *this = js;
    }
    return *this;
}

YJsonItem& YJsonItem::operator+=(const YJsonItem& js)
{
    return (*this += &js);
}

YJsonItem& YJsonItem::operator+=(const char* str)
{
    if (_type == YJSON_TYPE::YJSON_ARRAY)
    {
        YJsonItem *item = appendItem(YJSON_TYPE::YJSON_STRING);
        item->_valuestring = StrJoin(str);
    }
    return *this;
}

YJsonItem& YJsonItem::operator+=(int value)
{
    if (_type == YJSON_TYPE::YJSON_ARRAY)
    {
        YJsonItem *item = appendItem(YJSON_TYPE::YJSON_NUMBER);
        item->_valuedouble = item->_valueint = value;
    }
    return *this;
}

YJsonItem& YJsonItem::operator+=(double value)
{
    if (_type == YJSON_TYPE::YJSON_ARRAY)
    {
        YJsonItem *item = appendItem(YJSON_TYPE::YJSON_NUMBER);
        item->_valueint = (int)value; item->_valuedouble = value;
    }
    return *this;
}

YJsonItem& YJsonItem::operator[](int i) const
{
    return *findItem(i);
}

YJsonItem& YJsonItem::operator[](const char* i) const
{
    return *findItem(i);
}

YJsonItem& YJsonItem::operator++()
{
    if (_next)
    {
        if (_prev)
        {
            _prev->_next = new YJsonItem;
            _prev->_next->TakeJson(this, _parent); TakeJson(_next, nullptr);

            _prev->_next->_next = this; _prev->_next->_prev = _prev; _prev = _prev->_next;
            if (_next->_next)
            {
                _next->_next->_prev = this;
                _parent = _next->_next; _next->_prev = nullptr; _next->_next = nullptr; delete _next;
                _next = _parent; _parent = _prev->_parent;
            }
            else
            {
                _next->_prev = nullptr; _next->_next = nullptr; delete _next; _next = nullptr;
            }
        }
        else
        {
            _prev = new YJsonItem;
            _prev->TakeJson(this, _parent); TakeJson(_next, nullptr);

            _prev->_next = this;
            if (_next->_next)
            {
                _next->_next->_prev = this;
                _parent = _next->_next; _next->_prev = nullptr; _next->_next = nullptr; delete _next;
                _next = _parent; _parent = _prev->_parent;
            }
            else
            {
                _next->_prev = nullptr; _next->_next = nullptr; delete _next; _next = nullptr;
            }
        }
    } 
    return *this;
}

YJsonItem& YJsonItem::operator--()
{
    if (_prev)
    {
        if (_next)
        {
            _next->_prev = new YJsonItem;
            _next->_prev->TakeJson(this, _parent); TakeJson(_prev, nullptr);
            _next->_prev->_prev = this; _next->_prev->_next = _next; _next = _next->_prev;
            if (_prev->_prev)
            {
                _prev->_prev->_next = this;
                _parent = _prev->_prev; _prev->_prev = nullptr; _prev->_next = nullptr; delete _prev;
                _prev = _parent; _parent = _next->_parent;
            }
            else
            {
                _prev->_prev = nullptr; _prev->_next = nullptr; delete _prev; _prev = nullptr; _parent->_child = this;
            }
        }
        else
        {
            _next = new YJsonItem;
            _next->TakeJson(this, _parent); TakeJson(_prev, nullptr);
            _next->_prev = this;
            if (_prev->_prev)
            {
                _prev->_prev->_next = this;
                _parent = _prev->_prev; _prev->_prev = nullptr; _prev->_next = nullptr; delete _prev;
                _prev = _parent; _parent = _next->_parent;
            }
            else
            {
                _prev->_prev = nullptr; _prev->_next = nullptr; delete _prev; _prev = nullptr; _parent->_child = this;
            }
        }
    } 
    return *this;
}

std::ostream  &operator<<(std::ostream &out, YJsonItem *c1)
{
    c1->UpdateDepth(0);
    c1->print_value();
    out << ((const char*)c1->_buffer)+1 << std::endl;
	return out;
}

std::ostream  &operator<<(std::ostream &out, YJsonItem &c1)
{
    c1.UpdateDepth(0);
    c1.print_value();
    out << (const char*)c1._buffer << std::endl;
	return out;
}

std::ostream  &operator<<(std::ostream &out, YJsonItem &&c1)
{
    c1.UpdateDepth(0);
    c1.print_value();
    out << (const char*)c1._buffer << std::endl;
	return out;
}

std::ostream  &operator<<(std::ostream &out, const YJsonItem *c1)
{
    out << YJsonItem(*c1);
	return out;
}

std::ostream  &operator<<(std::ostream &out, const YJsonItem &c1)
{
	out << YJsonItem(c1);
	return out;
}

const char* YJsonItem::toString(bool fmt)
{
    UpdateDepth(0);
    print_value();
    if (_buffer)
        return _buffer+fmt;
    else if (ep.first)
        return nullptr;
    else
        return nullptr;
}

bool YJsonItem::toFile(const std::wstring name, const YJSON_ENCODE& file_encode)
{
    qout << "开始打印";
    UpdateDepth(0); print_value();
    qout << "打印成功";
    if (_buffer)
    {
        switch (file_encode) {
        case (YJSON_ENCODE::UTF16):
        {
            std::cout << "UTF-16" << "保存开始。";
            std::wstring data;
            data.push_back(0xfeff);
            utf8_to_utf16<std::wstring&, const char*>(data, _buffer+1);
            data.back() = L'\n';
            std::ofstream outFile(name, std::ios::out | std::ios::binary);
            if (outFile.is_open())
            {
                outFile.write(reinterpret_cast<const char*>(data.data()), data.length()*sizeof(wchar_t));
                outFile.close();
            }
            break;
        }
        default:
        {
            //cout << "UTF-8" << "保存开始。";
            const unsigned char c[3] = {0xef, 0xbb, 0xbf};
            std::ofstream outFile(name, std::ios::out | std::ios::binary);
            if (outFile.is_open())
            {
                outFile.write(reinterpret_cast<const char*>(c), sizeof(char)*3);
                outFile.write((const char*)(_buffer + 1), strlen(_buffer + 1)*sizeof(char));
                outFile.write("\n", sizeof(char));
                outFile.close();
            }
            break;
        }
        }
    }
    return false;
}

void YJsonItem::UpdateDepth(int depth)
{
    _depth = depth;
    if (_child)
        _child->UpdateDepth(depth + 1);
    if (_next)
        _next->UpdateDepth(depth);
}

void YJsonItem::joinKeyValue(const char* valuestring, const bool use_key)
{
    if (use_key && _keystring && (print_string(true), _buffer))
    {
        char *key = _buffer; _buffer = nullptr;
        if (true)
        {
            char* x = StrRepeat(' ', _depth*4);
            _buffer = StrJoin("\n", x, key, ": ", valuestring);
            delete[] x;
        }
        else
            _buffer = StrJoin(key, ":", valuestring);
        delete[] key;
    }
    else
    {
        if (_buffer) delete[] _buffer;
        if (true)
        {
            char* x = StrRepeat(' ', _depth*4);
            _buffer = StrJoin("\n", x, valuestring);
            delete[] x;
        }
        else
            _buffer = StrJoin(valuestring);
    }
}

template <typename T>
T YJsonItem::parse_value(T value)
{
    TYPE_CHAECK();
    qout << "加载数据";
    //value = StrSkip(value);
    if (!*value || ep.first || T() == value) return T();
    qout << "剩余字符串开头：" << *value;
    if (!strncmp(value, "null", 4))    { _type= YJSON_TYPE::YJSON_NULL;  return value+4; }
    if (!strncmp(value, "false", 5))    { _type= YJSON_TYPE::YJSON_FALSE; return value+5; }
    if (!strncmp(value, "true", 4))    { _type= YJSON_TYPE::YJSON_TRUE; _valueint=1;    return value+4; }
    if (*value=='\"')                { return parse_string(value); }
    if (*value=='-' || (*value>='0' && *value<='9'))    { return parse_number(value); }
    if (*value=='[')                { return parse_array(value); }
    if (*value=='{')                { return parse_object(value); }

    qout << "加载数据出错";
    return T();                              /* failure. */
}

void YJsonItem::print_value()
{
    switch (_type)
    {
        case YJSON_TYPE::YJSON_NULL:
        {
            joinKeyValue("null");
            break;
        }
        case YJSON_TYPE::YJSON_FALSE:
        {
            joinKeyValue("false");
            break;
        }
        case YJSON_TYPE::YJSON_TRUE:
        {
            joinKeyValue("true");
            break;
        }
        case YJSON_TYPE::YJSON_NUMBER:
        {
            print_number();
            joinKeyValue(_valuestring);
            break;
        }
        case YJSON_TYPE::YJSON_STRING:
        {
            print_string();
            char* x = _buffer; _buffer = nullptr;
            joinKeyValue(x);
            delete[] x;
            break;
        }
        case YJSON_TYPE::YJSON_ARRAY:
        {
            print_array();
            break;
        }
        case YJSON_TYPE::YJSON_OBJECT:
        {
            print_object();
            break;
        }
        default:
            break;
    }
}

template<typename T>
T YJsonItem::parse_number(T num)
{
    TYPE_CHAECK();
    _valuedouble = 0;
    short sign = 1;
    int scale = 0;
    int signsubscale = 1, subscale = 0;
    if (*num=='-') {
        sign = -1;
        ++num;
    }
    if (*num=='0')
    {
        _valuedouble = _valueint = 0;
        _type = YJSON_TYPE::YJSON_NUMBER;
        return ++num;
    }
    if ('1' <= *num && *num <= '9')
        do _valuedouble += (_valuedouble*=10.0, *num++ - '0');
        while ('0' <= *num && *num <= '9');
    if ('.' == *num && '0' <= num[1] && num[1] <= '9')
    {
        ++num;
        do  _valuedouble += (_valuedouble*=10.0, *num++ - '0'), scale--;
        while ('0' <= *num && *num <= '9');
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
        while ('0' <= *num && *num <= '9')
        {
            subscale *= 10;
            subscale += *num++ - '0';
        }
    }
    _valueint = (int)(_valuedouble *= sign * pow(10, scale + signsubscale * subscale));
    _type = YJSON_TYPE::YJSON_NUMBER;
    return num;
}

void YJsonItem::print_number()
{
    if (_valuestring) delete[] _valuestring;
    if (_valuedouble == 0)
        _valuestring = StrJoin("0");
    else if (fabs(((double)_valueint) - _valuedouble) <= DBL_EPSILON && _valuedouble <= INT_MAX && _valuedouble >= (double)INT_MIN)
    {
        char temp[21] = { 0 }; sprintf(temp,"%d",_valueint);
        _valuestring = StrJoin(temp);
    }
    else
    {
        char temp[64] = {0};
        if (fabs(floor(_valuedouble)-_valuedouble)<=DBL_EPSILON && fabs(_valuedouble)<1.0e60)
            sprintf(temp,"%.0f",_valuedouble);
        else if (fabs(_valuedouble)<1.0e-6 || fabs(_valuedouble)>1.0e9)
            sprintf(temp,"%e",_valuedouble);
        else
            sprintf(temp,"%f",_valuedouble);
        _valuestring = StrJoin(temp);
    }
}

template<typename T>
inline bool _parse_hex4(T str, wchar_t& h)
{
    TYPE_CHAECK();
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
wchar_t parse_hex4(T str)
{
    TYPE_CHAECK();
    wchar_t h = 0;
    if (_parse_hex4(str, h) || _parse_hex4(++str, h = h<<4) || _parse_hex4(++str, h = h<<4) || _parse_hex4(++str, h = h<<4))
        return 0;
    return h;
}

constexpr const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

template<typename T>
T YJsonItem::parse_string(T str)
{
    TYPE_CHAECK();
    //cout << "加载字符串：" << str << endl;
    T ptr = str + 1;
    char* ptr2; unsigned uc, uc2;
    size_t len = 0;
    if (*str != '\"') {
        //cout << "不是字符串！" << endl;
        ep.first = true;
        return T();
    }
    while (*ptr!='\"' && *ptr && ++len) if (*ptr++ == '\\') ptr++;    /* Skip escaped quotes. */
    _valuestring = new char[len + 1];
    ptr = str + 1;
    ptr2 = _valuestring;
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
            case 'u': uc=parse_hex4(ptr+1);ptr+=4;    /* get the unicode char. */

                if ((uc>=0xDC00 && uc<=0xDFFF) || uc==0)    break;    /* check for invalid.    */

                if (uc>=0xD800 && uc<=0xDBFF)    /* UTF16 surrogate pairs.    */
                {
                    if (ptr[1]!='\\' || ptr[2]!='u')    break;    /* missing second-half of surrogate.    */
                    uc2=parse_hex4(ptr+3);ptr+=6;
                    if (uc2<0xDC00 || uc2>0xDFFF)        break;    /* invalid second-half of surrogate.    */
                    uc=0x10000 + (((uc&0x3FF)<<10) | (uc2&0x3FF));
                }

                len=4;if (uc<0x80) len=1;else if (uc<0x800) len=2;else if (uc<0x10000) len=3; ptr2+=len;

                switch (len) {
                    case 4: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
                    case 3: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
                    case 2: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
                    case 1: *--ptr2 =(uc | firstByteMark[len]);
                }
                ptr2+=len;
                break;
            default:  *ptr2++ = *ptr; break;
            }
            ptr++;
        }
    }
    *ptr2 = 0; ++str;
    _type= YJSON_TYPE::YJSON_STRING;
    while (*str != '\"' && *str) if (*str++ == '\\') ++str;
    qout << "所得字符串：" << _valuestring;
    return ++str;
}

void YJsonItem::print_string(bool use_keystring)
{
    //qout << "打印字符开始";
    if (use_keystring ) { if (!_keystring) return; }
    else { if (!_valuestring) return; }
    if (_buffer) (delete[] _buffer, _buffer = nullptr);

    const char* const str = use_keystring ? _keystring:_valuestring;
    //cout << "输出字符串" << str << endl;
    const char* ptr; char* ptr2;
    size_t len = 0, flag = 0; unsigned char token;
    
    for (ptr = str; *ptr; ptr++)
        flag |= ((*ptr > 0 && *ptr < 32) || (*ptr == '\"') || (*ptr == '\\')) ? 1 : 0;
    if (!flag)
    {
        len = ptr - str;
        qout << len << str;
        _buffer = new char[len + 3];
        ptr2 = _buffer; *ptr2++ = '\"';
        memcpy(ptr2, str, len);
        *(ptr2 += len) = '\"'; *++ptr2 = 0; qout << "=====打印字符结束=====";
        return;
    }
    if (!str)
    {
        _buffer = StrJoin("\"\"");
        return;
    }
    ptr = str;
    while ((token = (unsigned char)*ptr) && ++len)
    {
        if (strchr("\"\\\b\f\n\r\t", token)) len++;
        else if (token < 32) len += 5; ptr++;
    }
    
    memset(_buffer = new char[len + 3], 0, (len + 3) * sizeof(char));

    ptr2 = _buffer; ptr = str;
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
    *ptr2 = '\"'; *++ptr2 = 0; qout << "打印字符结束";
}

template<typename T>
T YJsonItem::parse_array(T value)
{
    TYPE_CHAECK();
    qout << "加载列表";
    YJsonItem *child = nullptr;
    if (*value != '[') { ep.first = true; return T(); }    /* not an array! */

    _type = YJSON_TYPE::YJSON_ARRAY;
    value=StrSkip(++value);
    if (*value==']')
    {
        _child = nullptr;
        return value+1;    /* empty array. */
    }

    _child = child = new YJsonItem;
    _child->_parent = this;
    value = StrSkip(child->parse_value(StrSkip(value)));    /* skip any spacing, get the value. */
    if (!*value) return T();

    while (*value==',')
    {
        YJsonItem *new_item = new YJsonItem;
        child->_next = new_item;
        new_item->_prev = child;
        child = new_item; child->_parent = this;
        value = StrSkip(child->parse_value(StrSkip(value + 1)));
        if (!*value) return T();
    }

    if (*value == ']')
    {
        qout << "加载列表结束";
        return value + 1;
    }
    ep.first = true;
    return T();           /* malformed. */
}

void YJsonItem::print_array()
{
    qout << "打印列表开始";
    if (_buffer) (delete[] _buffer, _buffer = nullptr);
    std::deque<char*> entries;
    YJsonItem *child = _child;

    if (!child)
    {
        joinKeyValue("[]");
        return;
    }
    do {
        child->print_value();
        if (!child->_buffer)
            return;
        entries.push_back(child->_buffer);
    } while (child = child->_next);
    char* str_start = (joinKeyValue("["), _buffer),
           * str_end = (_buffer = nullptr, joinKeyValue("]", false), _buffer);
    _buffer = StrJoinX(str_start, (const char*)str_end, ",", entries);
    if (str_start) delete[] str_start;
    if (str_end) delete[] str_end;
    qout << "打印列表结束";
}

template<typename T>
T YJsonItem::parse_object(T value)
{
    TYPE_CHAECK();
    qout << "加载字典：";
    if (value == T() || *value != '{')    {ep.first = true; return T();}    /* not an object! */
    YJsonItem *child = nullptr;
    _type = YJSON_TYPE::YJSON_OBJECT;
    value = StrSkip(++value);
    if (*value == '}') return value + 1;
    _child = child = new YJsonItem;
    _child->_parent = this;
    value = StrSkip(child->parse_string(StrSkip(value)));
    if (!*value) return T();
    child->_keystring = child->_valuestring;
    child->_valuestring = nullptr;

    if (*value != ':')
    {
        ep.first = true;
        ep.second = "错误：加载字典时，没有找到冒号。";
        return T();
    }
    value = StrSkip(child->parse_value(StrSkip(value + 1)));
    while (*value==',')
    {
        YJsonItem *new_item = new YJsonItem;
        child->_next = new_item; new_item->_prev = child; child = new_item; child->_parent = this;
        value = StrSkip(child->parse_string(StrSkip(value+1)));
        child->_keystring = child->_valuestring; child->_valuestring = nullptr;
        if (*value!=':')
        {
            ep.first = true;
            ep.second = "错误：加载字典时，没有找到冒号。";
            return T();
        }
        value = StrSkip(child->parse_value(StrSkip(value+1)));    /* skip any spacing, get the value. */
        if (!*value) return T();
    }
    
    if (*value=='}')
    {
        qout << "加载字典结束";
        return value+1;
    }
    ep.first = true;
    ep.second = "错误：加载字典时，没有找结尾大括号。";
    return T();
}

void YJsonItem::print_object()
{
    //qout << "打印字典开始";
    if (_buffer) (delete[] _buffer, _buffer=nullptr);

    std::deque<char*> entries;
    YJsonItem *child = _child;
    if (!child)
    {
        joinKeyValue("{}");
        return;
    }
    do {
        child->print_value();
        if (!child->_buffer)
            return;
        entries.push_back(child->_buffer);
    } while (child = child->_next);
    qout << "开始拼接字符串";
    char *str_start = (joinKeyValue("{"), _buffer), *str_end = (_buffer = nullptr, joinKeyValue("}", false), _buffer);
    qout << "初始字符串：" << str_start << "结束字符串" << str_end;

    _buffer = StrJoinX(str_start, (const char*)str_end, ",", entries);
    if (str_start) delete[] str_start;
    if (str_end) delete[] str_end;
    qout << "打印字典结束";
}
