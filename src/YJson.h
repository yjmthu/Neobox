#ifndef YJSON_H
#define YJSON_H
#include <cstring>
#include <string>
#include <iostream>

class YJson
{
protected:
    inline explicit YJson() { }

public:
    enum Type { False, True, Null, Number, String, Array, Object };
    enum Encode { AUTO, ANSI, UTF8, UTF8BOM, UTF16LE, UTF16LEBOM, UTF16BE, UTF16BEBOM, OTHER };

    inline explicit YJson(Type type):_type(static_cast<Type>(type)) { };
    inline YJson(const YJson& js): _type(js._type), _value(js._value) { CopyJson(&js, nullptr); }
    inline explicit YJson(std::ifstream && file) noexcept { loadFile(file); };
    inline explicit YJson(std::ifstream & file) { loadFile(file); };
    inline explicit YJson(const std::string& path, Encode encode) { loadFile(path, encode); };
    inline explicit YJson(const char* path, Encode encode) { loadFile(std::string(path), encode); };
    explicit YJson(const char* str);
    explicit YJson(const std::string& str);
    explicit YJson(const wchar_t*);
    explicit YJson(const std::wstring&);
    YJson(const std::initializer_list<const char*>& lst);
    ~YJson();

    inline YJson::Type getType() const { return _type; }
    inline YJson* getPrev() const { return _prev; }
    inline YJson* getNext() const { return _next; }
    inline YJson* getChild() const { return _child; }
    inline YJson* getParent() const { return _parent; }

    inline const char *getKeyString() const {return _key;}
    inline const char *getValueString() const {return _value;}
    inline int getValueInt() const { if (_type == YJson::Number) return *reinterpret_cast<double*>(_value); else return 0;}
    inline double getValueDouble() const { if (_type == YJson::Number) return *reinterpret_cast<double*>(_value); else return 0;}
    std::string urlEncode() const;
    std::string urlEncode(const char* url) const;
    std::string urlEncode(const std::string& url) const;

    int getChildNum() const;
    YJson* getTop() const;

    char* toString(bool fmt=false);
    bool toFile(const std::string name, const YJson::Encode& encode=YJson::UTF8BOM, bool fmt=false);
    void loadFile(const std::string& path, YJson::Encode encode=YJson::Encode::AUTO);

    YJson& operator=(const YJson&);
    YJson& operator=(YJson&&) noexcept;
    inline YJson& operator[](int i) const { return *find(i); }
    inline YJson& operator[](const char* key) const { return *find(key); }
    inline YJson& operator[](const std::string& key) const { return *find(key); }
    inline operator bool() const { const YJson* s = this; return s; };
    inline void setText(const char* val) {delete _child; _child = nullptr; delete [] _value; auto len = strlen(val)+1; _value = new char[len]; std::copy(val, val+len, _value); _type=YJson::String;};
    inline void setText(const std::string& val) {delete _child; _child = nullptr; delete [] _value; _value = new char[val.length()+1]; std::copy(val.begin(), val.end(), _value); _value[val.length()] = 0; _type=YJson::String;};
    inline void setValue(double val) {delete _child; _child = nullptr; delete [] _value; _value = new char[sizeof (double)]; std::copy((char*)&val, (char*)&val+sizeof (val), _value); _type=YJson::Number;};
    inline void setValue(int val) {setValue(static_cast<double>(val));};
    inline void setNull() {delete _child; _child = nullptr; delete [] _value; _value = nullptr; _type=YJson::Null;}

    bool join(const YJson&);
    static YJson join(const YJson&, const YJson&);

    YJson* find(int index) const;
    YJson* find(const char* key) const;
    YJson* find(const std::string& key) const;
    inline YJson* findByVal(int value) const {return findByVal(static_cast<double>(value));};
    YJson* findByVal(double value) const;
    YJson* findByVal(const char* str) const;

    YJson* append(const YJson&, const char* key=nullptr);
    YJson* append(YJson::Type type, const char* key=nullptr);
    inline YJson* append(int value, const char* key=nullptr) { return append(static_cast<double>(value), key);};
    YJson* append(double, const char* key=nullptr);
    YJson* append(const char*, const char* key=nullptr);
    YJson* append(const std::string&, const char* key=nullptr);

    inline bool remove(int index) { return remove(find(index)); }
    inline bool remove(const char* key) { return remove(find(key)); }
    static bool remove(YJson *item);
    inline bool removeByVal(int value) { return remove(findByVal(value)); }
    inline bool removeByVal(double value) { return remove(findByVal(value)); }
    inline bool removeByVal(const std::string & str) { return remove(findByVal(str.c_str())); }

    inline bool clear()
    { if (_type != YJson::Array && _type != YJson::Object) return false; else if (_child) { delete _child; _child = nullptr;} return true; }
    inline bool empty(){ return !_child; }
    class iterator {
    private:
        YJson* _data = nullptr;
    public:
        iterator(YJson* data_): _data(data_){};
        bool operator!=(const iterator& that) { return _data != that._data; }
        bool operator==(const iterator& that) { return _data == that._data; }
        iterator& operator++() { _data = _data->_next; return *this; }
        iterator& operator--() { _data = _data->_prev; return *this; }
        YJson& operator*() { return *_data; }
    };
    iterator begin() { return _child; }
    iterator end() { return nullptr; }

private:
    static const unsigned char utf8bom[];
    static const unsigned char utf16le[];
    YJson::Type _type = YJson::Null;
    YJson *_next = nullptr,*_prev = nullptr, *_child = nullptr, *_parent = nullptr;
    char *_key = nullptr, *_value = nullptr;

    template<typename Type>
    void strict_parse(Type value, Type end);

    template<typename Type>
    Type parse_value(Type, Type end);
    char* print_value();
    char* print_value(int depth);

    template<typename Type>
    Type parse_number(Type, Type end);
    char* print_number();

    template<typename Type>
    Type parse_string(Type str, Type end);
    char* print_string(const char* const);

    template<typename Type>
    Type parse_array(Type value, Type end);
    char* print_array();
    char* print_array(int depth);

    template<typename Type>
    Type parse_object(Type value, Type end);
    char* print_object();
    char* print_object(int depth);

    char* joinKeyValue(const char* valuestring, int depth, bool delvalue);
    char* joinKeyValue(const char* valuestring, bool delvalue);

    void loadFile(std::ifstream &);
    void CopyJson(const YJson*, YJson*);
};

#endif
