#ifndef YJSON_H
#define YJSON_H
#include <string>
#include <iostream>

enum class YJSON{
    ARRAY=5,
    OBJECT=6
};

enum class YJSON_TYPE{
    YJSON_FALSE=0,
    YJSON_TRUE=1,
    YJSON_NULL=2,
    YJSON_NUMBER=3,
    YJSON_STRING=4,
    YJSON_ARRAY=5,
    YJSON_OBJECT=6,
};

enum class YJSON_ENCODE{
    AUTO = -1,
    UTF8 = 0,
    UTF8BOM = 1,
    UTF16 = 2,
    UTF16BOM = 3,
    ANSI = 4,
    OTHER = 4
};

class YJson
{
protected:
    inline explicit YJson() { };

public:
    inline explicit YJson(YJSON type):_type(static_cast<YJSON_TYPE>(type)) { };
    inline explicit YJson(const YJson& js): _type(js._type), _value(js._value) { CopyJson(&js, nullptr); }
    inline explicit YJson(std::ifstream && file) noexcept { loadFile(file); };
    inline explicit YJson(std::ifstream & file) { loadFile(file); };
    inline explicit YJson(const std::wstring& path, YJSON_ENCODE encode) {loadFile(path, encode);};
    explicit YJson(const char* str);
    explicit YJson(const std::string& str);
    explicit YJson(const wchar_t*);
    explicit YJson(const std::wstring&);
    YJson(const std::initializer_list<const char*>& lst);
    ~YJson();
    static std::pair<bool, std::string> ep;

    inline YJSON_TYPE getType() const { return _type; }
    inline YJson* getPrev() const { return _prev; }
    inline YJson* getNext() const { return _next; }
    inline YJson* getChild() const { return _child; }
    inline YJson* getParent() const { return _parent; }

    inline const char *getKeyString() const { if (this) return _key; else return nullptr;}
    inline const char *getValueString() const { if (this) return _value; else return nullptr;}
    inline int getValueInt() const { if (this && _type == YJSON_TYPE::YJSON_NUMBER) return *reinterpret_cast<double*>(_value); else return 0;}
    inline double getValueDouble() const { if (this && _type == YJSON_TYPE::YJSON_NUMBER) return *reinterpret_cast<double*>(_value); else return 0;}
    std::string urlEncode() const;
    std::string urlEncode(const char* url) const;
    std::string urlEncode(const std::string& url) const;

    int getChildNum() const;
    YJson* getTop() const;

    char* toString(bool fmt=false);
    bool toFile(const std::wstring name, const YJSON_ENCODE& encode=YJSON_ENCODE::UTF8BOM, bool fmt=false);
    void loadFile(const std::wstring& path, YJSON_ENCODE encode=YJSON_ENCODE::AUTO);

    YJson& operator=(const YJson&);
    YJson& operator=(YJson&&) noexcept;
    inline YJson& operator[](int i) const { return *find(i); }
    inline YJson& operator[](const char* key) const { return *find(key); }
    inline operator bool() const { const YJson* s = this; return s; };
    inline void setText(const char* val) {delete _child; _child = nullptr; delete [] _value; auto len = strlen(val)+1; _value = new char[len]; std::copy(val, val+len, _value); _type=YJSON_TYPE::YJSON_STRING;};
    inline void setText(const std::string& val) {delete _child; _child = nullptr; delete [] _value; _value = new char[val.length()+1]; std::copy(val.begin(), val.end(), _value); _type=YJSON_TYPE::YJSON_STRING;};
    inline void setValue(double val) {delete _child; _child = nullptr; delete [] _value; _value = new char[sizeof (double)]; std::copy((char*)&val, (char*)&val+sizeof (val), _value); _type=YJSON_TYPE::YJSON_NUMBER;};
    inline void setValue(int val) {setValue(static_cast<double>(val));};
    inline void setNull() {delete _child; _child = nullptr; delete [] _value; _value = nullptr; _type=YJSON_TYPE::YJSON_NULL;}

    bool join(const YJson&);
    static YJson join(const YJson&, const YJson&);

    YJson* find(int index) const;
    YJson* find(const char* key) const;
    inline YJson* findByVal(int value) const {return findByVal(static_cast<double>(value));};
    YJson* findByVal(double value) const;
    YJson* findByVal(const char* str) const;

    YJson* append(const YJson&, const char* key=nullptr);
    YJson* append(YJSON type, const char* key=nullptr);
    inline YJson* append(int value, const char* key=nullptr) { return append(static_cast<double>(value), key);};
    YJson* append(double, const char* key=nullptr);
    YJson* append(const char*, const char* key=nullptr);
    YJson* append(const std::string&, const char* key=nullptr);

    inline bool remove(int index) { return remove(find(index)); }
    inline bool remove(const char* key) { return remove(find(key)); }
    bool remove(YJson *item);
    inline bool removeByVal(int value) { return remove(findByVal(value)); }
    inline bool removeByVal(double value) { return remove(findByVal(value)); }
    inline bool removeByVal(const std::string & str) { return remove(findByVal(str.c_str())); }

    inline bool clear(){
        if (_type != YJSON_TYPE::YJSON_ARRAY && _type != YJSON_TYPE::YJSON_OBJECT) return false;
        else if (_child) { delete _child; _child = nullptr;}
        return true;
    }
    inline bool empty(){
        return !this || !_child;
    }
    class iterator{
    private:
        YJson* _data = nullptr;
    public:
        iterator(YJson* data_): _data(data_){};
        bool operator!=(const iterator& that) {
            return _data != that._data;
        }
        bool operator==(const iterator& that) {
            return _data == that._data;
        }
        iterator& operator++() {
            _data = _data->_next;
            return *this;
        }
        iterator& operator--() {
            _data = _data->_prev;
            return *this;
        }
        YJson& operator*() {
            return *_data;
        }
        iterator(const iterator&) = delete;
        iterator& operator =(const iterator&) = delete;
        ~iterator() = default;
    };
    iterator begin() {
        return _child;
    }
    iterator end() {
        return nullptr;
    }

private:
    YJSON_TYPE _type = YJSON_TYPE::YJSON_NULL;
    YJson *_next = nullptr,*_prev = nullptr, *_child = nullptr, *_parent = nullptr;
    char *_key = nullptr, *_value = nullptr;

    template<typename Type>
    bool strict_parse(Type);

    template<typename Type>
    Type parse_value(Type);
    char* print_value();
    char* print_value(int depth);

    template<typename Type>
    Type parse_number(Type);
    char* print_number();

    template<typename Type>
    Type parse_string(Type str);
    char* print_string(const char* const);

    template<typename Type>
    Type parse_array(Type value);
    char* print_array();
    char* print_array(int depth);

    template<typename Type>
    Type parse_object(Type value);
    char* print_object();
    char* print_object(int depth);

    char* joinKeyValue(const char* valuestring, int depth, bool delvalue);
    char* joinKeyValue(const char* valuestring, bool delvalue);

    void loadFile(std::ifstream &);
    void CopyJson(const YJson*, YJson*);

    YJson* append(YJSON_TYPE type);
};

#endif
