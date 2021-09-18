#ifndef YJSON_H
#define YJSON_H
#include <string>

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

class YJsonItem
{
protected:
    inline YJsonItem(){};

public:
    inline YJsonItem(const YJsonItem& js) { CopyJson(&js, nullptr); }
    explicit YJsonItem(const char* str);
    explicit YJsonItem(const std::string& str);
    explicit YJsonItem(const wchar_t*);
    explicit YJsonItem(const std::wstring&);
    ~YJsonItem();
    static YJsonItem* newFromFile(const std::wstring&);
    static std::pair<bool, std::string> ep;

    static YJsonItem Array();
    static YJsonItem Object();
    static YJsonItem* newArray();
    static YJsonItem* newObject();

    inline YJSON_TYPE getType() const { return _type; }
    inline YJsonItem* getPrevItem() const { return _prev; }
    inline YJsonItem* getNextItem() const { return _next; }
    inline YJsonItem* getChildItem() const { return _child; }
    inline YJsonItem* getParentItem() const { return _parent; }

    inline const char *getValueString() const { if (this) return _valuestring; else return nullptr;}
    inline int getValueInt() const { if (this) return _valueint; else return 0;}
    inline double getValueDouble() const { if (this) return _valuedouble; else return 0;}

    int getChildNum() const;
    const YJsonItem* getTopItem() const;

    char* toString(bool fmt=true);
    bool toFile(const std::wstring name, const YJSON_ENCODE& encode, bool fmt=false);

    YJsonItem& operator=(const YJsonItem&);
    YJsonItem& operator=(YJsonItem&&);
    inline YJsonItem& operator[](int i) const { return *findItem(i); }
    inline YJsonItem& operator[](const char* key) const { return *findItem(key); }
    inline operator bool(){YJsonItem* s = this; return s;};

    bool joinItem(const YJsonItem&);
    static YJsonItem joinItem(const YJsonItem&, const YJsonItem&);

    YJsonItem* findItem(int index) const;
    YJsonItem* findItem(const char* key) const;
    YJsonItem* findItemByValue(int value) const;
    YJsonItem* findItemByValue(double value) const;
    YJsonItem* findItemByValue(const char* str) const;

    YJsonItem* appendItem(const YJsonItem&, const char* key=nullptr);
    YJsonItem* appendItem(int, const char* key=nullptr);
    YJsonItem* appendItem(double, const char* key=nullptr);
    YJsonItem* appendItem(const char*, const char* key=nullptr);

    inline bool removeItem(int index) { return removeItem(findItem(index)); }
    inline bool removeItem(const char* key) { return removeItem(findItem(key)); }
    inline bool removeItemByValue(int value) { return removeItem(findItemByValue(value)); }
    inline bool removeItemByValue(double value) { return removeItem(findItemByValue(value)); }
    inline bool removeItemByValue(const std::string & str) { return removeItem(findItemByValue(str.c_str())); }

    inline bool clear(){
        if (_type != YJSON_TYPE::YJSON_ARRAY && _type != YJSON_TYPE::YJSON_OBJECT) return false;
        else if (_child) { delete _child; _child = nullptr;}
        return true;
    }

private:
	YJsonItem *_next = nullptr,*_prev = nullptr;
    
	YJsonItem *_child = nullptr; YJsonItem *_parent = nullptr;
    char *_valuestring = nullptr;
	int _valueint = 0;
	double _valuedouble = 0;

    char *_keystring = nullptr;
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

    void CopyJson(const YJsonItem*, YJsonItem*);

    bool removeItem(YJsonItem*);
    YJsonItem* appendItem(YJSON_TYPE type);
};

#endif
