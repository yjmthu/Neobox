#pragma once
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

enum class YJSON_DEBUG{
    HIDE = 0,
    SHOW = 1
};

class YJsonItem
{
protected:
    inline YJsonItem(){};

public:
    YJsonItem(const YJsonItem&);
    YJsonItem(const std::string&);
    YJsonItem(const std::wstring&);
    ~YJsonItem();
    static YJsonItem* newFromFile(const std::wstring&);
    static std::pair<bool, std::string> ep;

    static YJsonItem Array();
    static YJsonItem Object();
    static YJsonItem* newArray();
    static YJsonItem* newObject();

    inline YJSON_TYPE getType() const { return _type; };
    inline YJsonItem* getPrevItem() const { return _prev; };
    inline YJsonItem* getNextItem() const { return _next; };
    inline YJsonItem* getChildItem() const { return _child; };
    inline YJsonItem* getParentItem() const { return _parent; };

    inline const char *getValueString() const { return _valuestring; };
    inline int getValueInt() const { return _valueint; };
    inline double getValueDouble() const { return _valuedouble; };

    int getChildNum() const;
    const YJsonItem* getTopItem() const;

    const char* toString(bool fmt = true);
    bool toFile(const std::wstring file_name, const YJSON_ENCODE& file_encode);

    YJsonItem& operator=(const YJsonItem*);
    YJsonItem& operator=(const YJsonItem&);
    YJsonItem& operator=(YJsonItem&&);
    YJsonItem& operator=(int);
    YJsonItem& operator=(double);
    YJsonItem& operator=(const char*);
    YJsonItem& operator[](int) const;
    YJsonItem& operator[](const char*) const;
    YJsonItem operator+(const YJsonItem*);
    YJsonItem operator+(const YJsonItem&);
    YJsonItem& operator+=(const YJsonItem&);
    YJsonItem& operator+=(const YJsonItem*);
    YJsonItem& operator+=(int);
    YJsonItem& operator+=(double);
    YJsonItem& operator+=(const char*);
    YJsonItem& operator++();
    YJsonItem& operator--();

    YJsonItem* findItem(int index) const;
    YJsonItem* findItem(const char* key) const;
    YJsonItem* findItemByValue(int value) const;
    YJsonItem* findItemByValue(double value) const;
    YJsonItem* findItemByValue(const char* str) const;

    YJsonItem* appendItem(const YJsonItem&, const char* key=nullptr);
    YJsonItem* appendItem(const YJsonItem*, const char* key=nullptr);
    YJsonItem* appendItem(int, const char* key=nullptr);
    YJsonItem* appendItem(double, const char* key=nullptr);
    YJsonItem* appendItem(const char*, const char* key=nullptr);

    bool removeItem(int index);
    bool removeItem(const char* key);
    bool removeItemByValue(int value);
    bool removeItemByValue(double value);
    bool removeItemByValue(std::string value);

    static YJSON_DEBUG DEBUG_OUT_PUT;

private:
	YJsonItem *_next = nullptr,*_prev = nullptr;
    
	YJsonItem *_child = nullptr; YJsonItem *_parent = nullptr;
    char *_valuestring = nullptr;
	int _valueint = 0;
	double _valuedouble = 0;

    char *_keystring = nullptr;
    YJSON_TYPE _type = YJSON_TYPE::YJSON_NULL;

    char *_buffer = nullptr;
    int _depth = 1;

    template<typename Type>
    bool strict_parse(Type);

    template<typename Type>
    Type parse_value(Type);
    void print_value();

    template<typename Type>
    Type parse_number(Type);
    void print_number();

    template<typename Type>
    Type parse_string(Type str);
    void print_string(bool use_keystring = false);

    template<typename Type>
    Type parse_array(Type value);
    void print_array();

    template<typename Type>
    Type parse_object(Type value);
    void print_object();

    void joinKeyValue(const char* valuestring, const bool use_key = true);
    void clearContent();

    void CopyJson(const YJsonItem*, YJsonItem*);
    void TakeJson(YJsonItem*, YJsonItem*, bool cpoy_key = true);
    void UpdateDepth(int depth);

    bool removeItem(YJsonItem*);
    YJsonItem* appendItem(YJSON_TYPE type);

    friend std::ostream& operator<<(std::ostream &out, YJsonItem &c1);
    friend std::ostream& operator<<(std::ostream &out, YJsonItem *c1);
    friend std::ostream& operator<<(std::ostream &out, const YJsonItem &c1);
    friend std::ostream& operator<<(std::ostream &out, const YJsonItem *c1);
    friend std::ostream& operator<<(std::ostream &out, YJsonItem &&c1);
};
