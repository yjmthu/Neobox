#ifndef YJSON_H
#define YJSON_H
#include <string>

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
    UTF8 = 0,
    UTF16 = 1,
    OTHER = 2
};

class YJson
{
protected:
    inline explicit YJson() { };

public:
    inline explicit YJson(YJSON type):_type(static_cast<YJSON_TYPE>(type)) { };
    inline explicit YJson(const YJson& js) { CopyJson(&js, nullptr); }
    inline explicit YJson(std::ifstream && file) { LoadFile(file); };
    inline explicit YJson(std::ifstream & file) { LoadFile(file); };
    explicit YJson(const char* str);
    explicit YJson(const std::string& str);
    explicit YJson(const wchar_t*);
    explicit YJson(const std::wstring&);
    ~YJson();
    static std::pair<bool, std::string> ep;

    inline YJSON_TYPE getType() const { return _type; }
    inline YJson* getPrev() const { return _prev; }
    inline YJson* getNext() const { return _next; }
    inline YJson* getChild() const { return _child; }
    inline YJson* getParent() const { return _parent; }

    inline const char *getValueString() const { if (this) return _value; else return nullptr;}
    inline int getValueInt() const { if (this && _type == YJSON_TYPE::YJSON_NUMBER) return *reinterpret_cast<double*>(_value); else return 0;}
    inline double getValueDouble() const { if (this && _type == YJSON_TYPE::YJSON_NUMBER) return *reinterpret_cast<double*>(_value); else return 0;}

    int getChildNum() const;
    const YJson* getTop() const;

    char* toString(bool fmt=true);
    bool toFile(const std::wstring name, const YJSON_ENCODE& encode, bool fmt=false);

    YJson& operator=(const YJson&);
    YJson& operator=(YJson&&);
    inline YJson& operator[](int i) const { return *find(i); }
    inline YJson& operator[](const char* key) const { return *find(key); }
    inline operator bool(){YJson* s = this; return s;};

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

    inline bool remove(int index) { return remove(find(index)); }
    inline bool remove(const char* key) { return remove(find(key)); }
    inline bool removeByVal(int value) { return remove(findByVal(value)); }
    inline bool removeByVal(double value) { return remove(findByVal(value)); }
    inline bool removeByVal(const std::string & str) { return remove(findByVal(str.c_str())); }

    inline bool clear(){
        if (_type != YJSON_TYPE::YJSON_ARRAY && _type != YJSON_TYPE::YJSON_OBJECT) return false;
        else if (_child) { delete _child; _child = nullptr;}
        return true;
    }

private:
    YJson *_next = nullptr,*_prev = nullptr, *_child = nullptr, *_parent = nullptr;
    char *_key = nullptr, *_value = nullptr;
    YJSON_TYPE _type = YJSON_TYPE::YJSON_NULL;

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
    void clearContent();

    void LoadFile(std::ifstream &);
    void CopyJson(const YJson*, YJson*);

    bool remove(YJson*);
    YJson* append(YJSON_TYPE type);
};

#endif
