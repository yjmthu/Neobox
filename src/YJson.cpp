#include <iostream>
#include <ostream>
#include <fstream>
#include "funcbox.h"
#include "YJson.h"
#include "YString.h"

#define cout qDebug()
#define endl "\n"

/*
#define cout std::cout
#define endl std::endl
*/

bool YJsonItem::FMT = true;
YJsonDebug YJsonItem::DEBUG_OUT_PUT = YJsonDebug::YJSON_SHOW;
const char* YJsonItem::ep = nullptr;
FILE_ENCODE YJsonItem::_encode = FILE_ENCODE::CODE_OTHER;

YJsonItem::YJsonItem() {}

YJsonItem::YJsonItem(const YJsonItem* js)
{
    CopyJson(js, nullptr);
}

YJsonItem::YJsonItem(const YJsonItem& js)
{
    CopyJson(&js, nullptr);
}

YJsonItem::YJsonItem(const std::string str, const YJsonParse type)
{
    switch (type)
    {
    case YJsonParse::YJson_File:
    {
        char *json_str = nullptr;
        std::ifstream file(str, std::ios::in | std::ios::binary);
        if (file.is_open())
        {
            file.seekg(0, std::ios::end);
            int size = file.tellg();
            if (size < 20)
            {
                file.close();
                return;
            }
            unsigned char c1 = 0, c2 = 0, c3 = 0; file.seekg(0, std::ios::beg);
            if (!(file.read((char*)&c1, sizeof(char))
                    &&
                  file.read((char*)&c2, sizeof(char))
                    &&
                  file.read((char*)&c3, sizeof(char))
               ))
            {
                file.close();
                return;
            }
            size -= sizeof(char) * 3;
            if (c1 == 0xef && c2 == 0xbb && c3 == 0xbf)
            {
                _encode = FILE_ENCODE::CODE_UTF8;
                json_str = new char[size + 1]; json_str[size] = 0;
                file.read(json_str, size); file.close();
            }
            else if (c1 == 0xff && c2 == 0xfe)
            {
                _encode = FILE_ENCODE::CODE_UTF16;
                if ((size + sizeof (char)) % sizeof (wchar_t))
                {
                    file.close(); return;
                }
                else
                {
                    wchar_t* json_str_temp = new wchar_t[size/sizeof (wchar_t) + 1];
                    char *ptr = (char*)json_str_temp; *ptr = c3;
                    file.read(++ptr, size); int len = WideCharToMultiByte(CP_UTF8, 0, json_str_temp, -1, nullptr, 0, nullptr, nullptr);
                    json_str = new char[len]; WideCharToMultiByte(CP_UTF8, 0, json_str_temp, -1, json_str, len, nullptr, nullptr);
                    delete [] json_str_temp; file.close();
                }
            }
            else
            {
                _encode = FILE_ENCODE::CODE_OTHER;
                file.close(); return;
            }
            if (_encode != FILE_ENCODE::CODE_OTHER)
            {
                const char* temp = StrSkip(json_str);
                switch (*temp)
                {
                case '{':
                    parse_object(temp);
                    break;
                case '[':
                    parse_array(temp);
                    break;
                case '\0':
                    break;
                default:
                    ep = temp;
                    break;
                }
                if (ep) cout << "error:\t" << ep << endl;
                delete[] json_str; ep = 0;
            }
            else
            {
                if (json_str) delete [] json_str;
                cout << "unsuported encode";
            }
        }
        else
        {
            ep = str.c_str(); cout << "error:\t" << ep << endl;
        }
        break;
    }
    case YJsonParse::YJson_String:
    {
        const char* temp = StrSkip(str.c_str());
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
        if (ep) cout << "error:\t" << ep << endl;
        break;
    }
    default:
        break;
    }
}

YJsonItem YJsonItem::newArray()
{
    YJsonItem json_array = YJsonItem();
    json_array._type = YJson::YJSON_ARRAY;
    return json_array;
}

YJsonItem YJsonItem::newObject()
{
    YJsonItem json_array = YJsonItem();
    json_array._type = YJson::YJSON_OBJECT;
    return json_array;
}

//复制json，当parent为NULL，如果自己有key用自己的key
void YJsonItem::CopyJson(const YJsonItem* json, YJsonItem* parent)
{
    if (json->ep) return;
    _type = json->_type;

    if (!(_parent = parent) || parent->_type != YJson::YJSON_OBJECT)
    {
        if (_keystring) delete[] _keystring; _keystring = nullptr;
    }
    else if (!_keystring && json->_keystring)
    {
        _keystring = StrJoin(json->_keystring);
    }

    if (_type == YJson::YJSON_ARRAY || _type == YJson::YJSON_OBJECT)
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
    else if (_type == YJson::YJSON_STRING)
        _valuestring = StrJoin(json->_valuestring);
    else if (_type == YJson::YJSON_NUMBER)
    {
        _valueint = json->_valueint;
        _valuedouble = json->_valuedouble;
    }
}

void YJsonItem::TakeJson(YJsonItem* js, YJsonItem* parent, bool copy_key)
{
    if (parent) _parent = parent;
    _type = js->_type; js->_type = YJson::YJSON_NULL;
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
    ep = js->ep; js->ep = nullptr;
}

YJson YJsonItem::getType() const
{
    return _type;
}

//清除_buffer, _valuestring, _child, 以及ep、valueint、 _valuedouble
void YJsonItem::clearContent()
{
    if (_buffer) (delete[] _buffer, _buffer = nullptr);
    if (_valuestring) (delete _valuestring, _valuestring = nullptr);
    if (_child) (delete _child, _child = nullptr);
    if (ep) ep = nullptr; _valueint = 0; _valuedouble = 0;
}

const char* YJsonItem::getValueSring() const
{
    return _valuestring;
}

int YJsonItem::getValueInt() const
{
    return _valueint;
}

double YJsonItem::getValueDouble() const
{
    return _valuedouble;
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
    if (_type == YJson::YJSON_OBJECT)
    {
        YJsonItem *child = _child;
        if (child)
        {
            do {
                if (StrCompare(child->_keystring, key))
                    return child;
            } while (child = child->_next);
        }
    }
    return nullptr;
}

YJsonItem* YJsonItem::findItem(int key) const
{
    if (_type == YJson::YJSON_ARRAY)
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
    if (_type == YJson::YJSON_ARRAY || _type == YJson::YJSON_OBJECT)
    {
        YJsonItem *child = _child;
        if (child) do 
            if (child->_type==YJson::YJSON_NUMBER && child->_valueint==value && fabs(((double)value)-child->_valuedouble) <= DBL_EPSILON)
                return child;
        while (child = child->_next);
    }
    return nullptr;
}

YJsonItem* YJsonItem::findItemByValue(double value) const
{
    if (_type == YJson::YJSON_ARRAY || _type == YJson::YJSON_OBJECT)
    {
        YJsonItem *child = _child;
        if (child) do
            if (child->_type == YJson::YJSON_NUMBER && fabs(value-child->_valuedouble) <= DBL_EPSILON)
                return child;
        while (child = child->_next);
        
    }
    return nullptr;
}

YJsonItem* YJsonItem::findItemByValue(const char* str) const
{
    if (_type == YJson::YJSON_ARRAY || _type == YJson::YJSON_OBJECT)
    {
        YJsonItem *child = _child;
        if (child) do
            if (child->_type == YJson::YJSON_STRING && StrCompare(child->_valuestring, str))
                return child;
        while (child = child->_next);
        
    }
    return nullptr;
}

YJsonItem* YJsonItem::getPrevItem() const
{
    return _prev;
}

YJsonItem* YJsonItem::getNextItem() const
{
    return _next;
}

YJsonItem* YJsonItem::getChildItem() const
{
    return _child;
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

YJsonItem* YJsonItem::getParentItem() const
{
    return _parent;
}

YJsonItem* YJsonItem::appendItem(YJson type)
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
    if (_type == YJson::YJSON_OBJECT)
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
    if (_type == YJson::YJSON_OBJECT) {if (!key) return nullptr;}
    else {if (key) return nullptr;}
    YJsonItem* child = appendItem(YJson::YJSON_NUMBER);
    child->_valuedouble = child->_valueint = value;
    if (key) child->_keystring = StrJoin(key);
    return child;
}

YJsonItem* YJsonItem::appendItem(double value, const char* key)
{
    if (_type == YJson::YJSON_OBJECT)
        {if (!key) return nullptr;}
    else
        {if (key) return nullptr;}
    YJsonItem* child = appendItem(YJson::YJSON_NUMBER);
    child->_valueint = (int)value; child->_valuedouble = value;
    if (key) child->_keystring = StrJoin(key);
    return child;
}

YJsonItem* YJsonItem::appendItem(const char* str, const char* key)
{
    if (_type == YJson::YJSON_OBJECT) {
        if (!key) return nullptr;
    } else {
        if (key) return nullptr;
    }
    YJsonItem* child = appendItem(YJson::YJSON_STRING);
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

bool YJsonItem::removeItemByValue(char* str)
{
    return removeItem(findItemByValue(str));
}

YJsonItem& YJsonItem::operator=(const YJsonItem* s)
{
    if (s == this)
        return *this;
    else if (s->getTopItem() == getTopItem())
        return (*this = YJsonItem(s));
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
    clearContent(); _type = YJson::YJSON_NUMBER;
    _valuedouble = _valueint = value;
    return *this;
}

YJsonItem& YJsonItem::operator=(double value)
{
    clearContent(); _type = YJson::YJSON_NUMBER;
    _valueint = (int)value; _valuedouble = value;
    return *this;
}

YJsonItem& YJsonItem::operator=(const char* str)
{
    clearContent(); _type = YJson::YJSON_STRING;
    _valuestring = StrJoin(str);
    return *this;
}

YJsonItem YJsonItem::operator+(const YJsonItem*that)
{
    if (ep || !that || that->ep || _type != YJson::YJSON_ARRAY || _type != YJson::YJSON_OBJECT || _type != that->_type)
    {
        YJsonItem s;
        return s;
    }
    else
    {
        YJsonItem js(this); js += that;
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
        return *this += YJsonItem(this);
    }
    if (ep || !js || js->ep || _type != js->_type || (_type != YJson::YJSON_ARRAY && _type != YJson::YJSON_OBJECT))
        return *this;
    YJsonItem *child = _child;
    YJsonItem *childx = js->_child;
    if (child)
    {
        while (child->_next) child = child->_next;
        if (childx)
        {
            do {
                child->_next = new YJsonItem(childx);
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
    if (_type == YJson::YJSON_ARRAY)
    {
        YJsonItem *item = appendItem(YJson::YJSON_STRING);
        item->_valuestring = StrJoin(str);
    }
    return *this;
}

YJsonItem& YJsonItem::operator+=(int value)
{
    if (_type == YJson::YJSON_ARRAY)
    {
        YJsonItem *item = appendItem(YJson::YJSON_NUMBER);
        item->_valuedouble = item->_valueint = value;
    }
    return *this;
}

YJsonItem& YJsonItem::operator+=(double value)
{
    if (_type == YJson::YJSON_ARRAY)
    {
        YJsonItem *item = appendItem(YJson::YJSON_NUMBER);
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
    YJsonItem::FMT = true;
    c1->UpdateDepth(0);
    c1->print_value();
    out << ((const char*)c1->_buffer)+1 << endl;
	return out;
}

std::ostream  &operator<<(std::ostream &out, YJsonItem &c1)
{
    YJsonItem::FMT = false;
    c1.UpdateDepth(0);
    c1.print_value();
    out << (const char*)c1._buffer << endl;
	return out;
}

std::ostream  &operator<<(std::ostream &out, YJsonItem &&c1)
{
    c1.UpdateDepth(0);
    c1.print_value();
    out << (const char*)c1._buffer << endl;
	return out;
}

std::ostream  &operator<<(std::ostream &out, const YJsonItem *c1)
{
    YJsonItem::FMT = true;
    out << YJsonItem(c1);
	return out;
}

std::ostream  &operator<<(std::ostream &out, const YJsonItem &c1)
{
    YJsonItem::FMT = false;
	out << YJsonItem(c1);
	return out;
}

const char* YJsonItem::toString(bool fmt)
{
    FMT = fmt?1:0;
    UpdateDepth(0);
    print_value();
    if (_buffer)
        return _buffer+FMT;
    else if (ep)
        return ep;
    else
        return nullptr;
}

bool YJsonItem::toFile(const std::string name)
{
    FMT = 1; qout << "开始打印";
    UpdateDepth(0); print_value();
    qout << "打印成功";
    if (_buffer)
    {
        switch (_encode) {
        case (FILE_ENCODE::CODE_UTF16):
        {
            cout << "UTF-16" << "保存开始。";
            wchar_t c = 0xfeff, *data = nullptr;
            int len = MultiByteToWideChar(CP_UTF8, 0, _buffer+1, -1, nullptr, 0);
            data = new wchar_t[len];
            MultiByteToWideChar(CP_ACP, 0, _buffer+1, -1, data, len);
            data[len-1] = L'\n';
            std::ofstream outFile(name, std::ios::out | std::ios::binary);
            if (outFile.is_open())
            {
                outFile.write((char*)&c, sizeof(wchar_t));
                outFile.write((char*)(data), len * sizeof(wchar_t));
                outFile.close();
            }
            delete [] data;
            break;
        }
        default:
        {
            cout << "UTF-8" << "保存开始。";
            unsigned char c[3] = {0xef, 0xbb, 0xbf};
            std::ofstream outFile(name, std::ios::out | std::ios::binary);
            if (outFile.is_open())
            {
                outFile.write((char*)c, sizeof(char)*3);
                outFile.write((char*)(_buffer + 1), strlen(_buffer + 1)*sizeof(char));
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
        if (FMT)
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
        if (FMT)
        {
            //qout << "重复字符串开始" << "depth:" << _depth;
            char* x = StrRepeat(' ', _depth*4);
            //qout << "拼接 x" << x << "长度:" << strlen(x) << "valuestring:" << valuestring;
            _buffer = StrJoin("\n", x, valuestring);
            //qout << "重复字符串结束 buffer" << _buffer;
            delete[] x;
        }
        else
            _buffer = StrJoin(valuestring);
    }
}

const char *YJsonItem::parse_value(const char *value)
{
    //value = StrSkip(value);
    if (!value)                        return 0;    /* Fail on null. */
    if (StrCompare(value, "null", 4))    { _type= YJson::YJSON_NULL;  return value+4; }
    if (StrCompare(value, "false", 5))    { _type= YJson::YJSON_FALSE; return value+5; }
    if (StrCompare(value, "true", 4))    { _type= YJson::YJSON_TRUE; _valueint=1;    return value+4; }
    if (*value=='\"')                { return parse_string(value); }
    if (*value=='-' || (*value>='0' && *value<='9'))    { return parse_number(value); }
    if (*value=='[')                { return parse_array(value); }
    if (*value=='{')                { return parse_object(value); }

    ep = value; return 0;                              /* failure. */
}

void YJsonItem::print_value()
{
    switch (_type)
    {
        case YJson::YJSON_NULL:
        {
            joinKeyValue("null");
            break;
        }
        case YJson::YJSON_FALSE:
        {
            joinKeyValue("false");
            break;
        }
        case YJson::YJSON_TRUE:
        {
            joinKeyValue("true");
            break;
        }
        case YJson::YJSON_NUMBER:
        {
            print_number();
            joinKeyValue(_valuestring);
            break;
        }
        case YJson::YJSON_STRING:
        {
            print_string();
            char* x = _buffer; _buffer = nullptr;
            joinKeyValue(x);
            delete[] x;
            break;
        }
        case YJson::YJSON_ARRAY:
        {
            print_array();
            break;
        }
        case YJson::YJSON_OBJECT:
        {
            print_object();
            break;
        }
        default:
            break;
    }
}

const char * YJsonItem::parse_number(const char *num)
{
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
        _type = YJson::YJSON_NUMBER;
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
    _type = YJson::YJSON_NUMBER;
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

wchar_t parse_hex4(const char* str)
{
    wchar_t h = 0;
    if (*str >= '0' && *str <= '9')
        h += (*str) - '0';
    else if (*str >= 'A' && *str <= 'F')
        h += 10 + (*str) - 'A';
    else if (*str >= 'a' && *str <= 'f')
        h += 10 + (*str) - 'a';
    else
        return 0;
    h = h << 4; ++str;
    if (*str >= '0' && *str <= '9')
        h += (*str) - '0';
    else if (*str >= 'A' && *str <= 'F')
        h += 10 + (*str) - 'A';
    else if (*str >= 'a' && *str <= 'f')
        h += 10 + (*str) - 'a';
    else
        return 0;
    h = h << 4; ++str;
    if (*str >= '0' && *str <= '9') 
        h += (*str) - '0';
    else if (*str >= 'A' && *str <= 'F') 
        h += 10 + (*str) - 'A'; 
    else if (*str >= 'a' && *str <= 'f') 
        h += 10 + (*str) - 'a';
    else 
        return 0;
    h = h << 4; ++str;
    if (*str >= '0' && *str <= '9') 
        h += (*str) - '0'; 
    else if (*str >= 'A' && *str <= 'F') 
        h += 10 + (*str) - 'A'; 
    else if (*str >= 'a' && *str <= 'f') 
        h += 10 + (*str) - 'a'; 
    else 
        return 0;
    return h;
}

static const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

const char *YJsonItem::parse_string(const char *str)
{
    //cout << "加载字符串：" << str << endl;
    const char* ptr = str + 1;
    char* ptr2; unsigned uc,uc2;
    size_t len = 0;
    if (*str != '\"') {
        //cout << "不是字符串！" << endl;
        ep = str;
        return nullptr;
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
    _type= YJson::YJSON_STRING;
    while (*str != '\"' && *str) if (*str++ == '\\') ++str;
    //cout << "结尾字符串：" << str << endl;
    return ++str;
}

void YJsonItem::print_string(bool use_keystring)
{
    qout << "打印字符开始";
    if (use_keystring ) { if (!_keystring) return; }
    else { if (!_valuestring) return; }
    if (_buffer) (delete[] _buffer, _buffer = nullptr);

    const char *str = use_keystring ? _keystring:_valuestring;
    //cout << "输出字符串" << str << endl;
    const char* ptr; char* ptr2;
    size_t len = 0, flag = 0; unsigned char token;
    
    for (ptr = str; *ptr; ptr++)
        flag |= ((*ptr > 0 && *ptr < 32) || (*ptr == '\"') || (*ptr == '\\')) ? 1 : 0;
    if (!flag)
    {
        len = ptr - str;
        memset(_buffer = new char[len + 3], 0, sizeof(char) * (len + 3));
        ptr2 = _buffer; *ptr2 = '\"';
        ptr2 = StrCopy(++ptr2, str);
        *ptr2 = '\"'; *++ptr2 = 0; qout << "打印字符结束";
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

const char *YJsonItem::parse_array(const char *value)
{
    YJsonItem *child = nullptr;
    if (*value != '[') { ep = value; return 0; }    /* not an array! */

    _type = YJson::YJSON_ARRAY;
    value=StrSkip(++value);
    if (*value==']')
    {
        _child = nullptr;
        return value+1;    /* empty array. */
    }

    _child = child = new YJsonItem;
    _child->_parent = this;
    value = StrSkip(child->parse_value(StrSkip(value)));    /* skip any spacing, get the value. */
    if (!value) return 0;

    while (*value==',')
    {
        YJsonItem *new_item = new YJsonItem;
        child->_next = new_item;
        new_item->_prev = child;
        child = new_item; child->_parent = this;
        value = StrSkip(child->parse_value(StrSkip(value + 1)));
        if (!*value) return 0;
    }

    if (*value == ']')
        return value + 1;    /* end of array */
    ep = value;              /* not an array! */
    return nullptr;           /* malformed. */
}

void YJsonItem::print_array()
{
    qout << "打印列表开始";
    if (_buffer) (delete[] _buffer, _buffer = nullptr);
    char **entries = nullptr;
    YJsonItem *child = _child;
    int numentries=0, i=0;
    
    /* How many entries in the array? */
    while (child) numentries++, child = child->_next;
    if (!numentries)
    {
        joinKeyValue("[]");
        return;
    }

    entries = new char*[numentries];
    if (!entries)
        return;
    if (child = _child)
        do {
            if (!(child->print_value(), entries[i++] = child->_buffer))
            {
                delete[] entries;
                return;
            }
        } while (child = child->_next);
    char* str_start = (joinKeyValue("["), _buffer),
           * str_end = (_buffer = nullptr, joinKeyValue("]", false), _buffer);
    _buffer = StrJoinX(str_start, (const char*)str_end, ",", (const char**)entries, numentries);
    if (str_start) delete[] str_start;
    if (str_end) delete[] str_end;
    delete[] entries; qout << "打印列表结束";
}

const char* YJsonItem::parse_object(const char *value)
{
    //cout << "加载字典：" << value << endl;
    if (*value!='{')    {ep = value; return 0;}    /* not an object! */
    YJsonItem *child = nullptr;
    _type = YJson::YJSON_OBJECT;
    value = StrSkip(++value);
    if (*value == '}') return value + 1;
    _child = child = new YJsonItem;
    _child->_parent = this;
    value = StrSkip(child->parse_string(StrSkip(value)));
    if (!value || !*value) return nullptr;
    child->_keystring = child->_valuestring;
    child->_valuestring = nullptr;
    //cout << "找到键：" << child->_keystring << endl;

    if (*value != ':')
    {
        ep = value;
        return nullptr;
    }
    value = StrSkip(child->parse_value(StrSkip(value + 1))); 
    if (!value || !*value) return 0;
    
    while (*value==',')
    {
        YJsonItem *new_item = new YJsonItem;
        child->_next = new_item; new_item->_prev = child; child = new_item; child->_parent = this;
        value = StrSkip(child->parse_string(StrSkip(value+1)));
        if (!value) return 0;
        child->_keystring = child->_valuestring; child->_valuestring = nullptr;
        if (*value!=':')
        {
            ep=value;
            return 0;
        }    /* fail! */
        value = StrSkip(child->parse_value(StrSkip(value+1)));    /* skip any spacing, get the value. */
        if (!value) return 0;
    }
    
    if (*value=='}')
        return value+1;
    ep = value;
    return nullptr;
}

void YJsonItem::print_object()
{
    qout << "打印字典开始";
    if (_buffer) (delete[] _buffer, _buffer=nullptr);

    char **entries = nullptr; int i=0, numentries=0;
    YJsonItem *child = _child;
    while (child) numentries++, child=child->_next;
    if (!numentries)
    {
        joinKeyValue("{}");
        return;
    }
    entries = new char*[numentries];
    if (child = _child)
        do {
            if (!(entries[i++] = (child->print_value(), child->_buffer)))
            {
                delete[] entries;
                return ;
            }
        } while (child = child->_next);
    qout << "开始拼接字符串";
    char *str_start = (joinKeyValue("{"), _buffer), *str_end = (_buffer = nullptr, joinKeyValue("}", false), _buffer);
    qout << "初始字符串：" << str_start << "结束字符串" << str_end;
    _buffer = StrJoinX(str_start, (const char*)str_end, ",", (const char**)entries, numentries);
    if (str_start) delete[] str_start;
    if (str_end) delete[] str_end;
    delete[] entries; qout << "打印字典结束";
}
