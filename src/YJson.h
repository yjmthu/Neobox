#pragma once
#include <string>

typedef enum class _YJSON{
    YJSON_FALSE=0,
    YJSON_TRUE=1,
    YJSON_NULL=2,
    YJSON_NUMBER=3,
    YJSON_STRING=4,
    YJSON_ARRAY=5,
    YJSON_OBJECT=6,
} YJson;

typedef enum class _YJSON_PARSE{
    YJson_File = 0,
    YJson_String = 1
} YJsonParse;

typedef enum class _FILE_ENCODE{
    CODE_UTF8 = 0,
    CODE_UTF16 = 1,
    CODE_GBK = 2,
    CODE_OTHER = 3
} FILE_ENCODE;

typedef enum class _YJSON_DEBUG{
    YJSON_HIDE = 0,
    YJSON_SHOW = 1
} YJsonDebug;

class YJsonItem
{
protected:
    YJsonItem();
public:
    YJsonItem(const YJsonItem&);
    YJsonItem(const YJsonItem*);
    YJsonItem(const std::string, const YJsonParse=YJsonParse::YJson_String);
    ~YJsonItem();
    static bool FMT; static const char* ep;

    static YJsonItem newArray();
    static YJsonItem newObject();

    YJson getType() const;
    YJsonItem* getPrevItem() const;
    YJsonItem* getNextItem() const;
    YJsonItem* getChildItem() const;
    const YJsonItem* getTopItem() const;
    YJsonItem* getParentItem() const;

    const char *getValueSring() const;
	int getValueInt() const;
	double getValueDouble() const;
    int getChildNum() const;

    const char* toString(bool fmt = true);
    bool toFile(const std::string file_name);

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

    YJsonItem* appendItem(const YJsonItem&, const char* key = nullptr);
    YJsonItem* appendItem(const YJsonItem*, const char* key = nullptr);
    YJsonItem* appendItem(int, const char* key=nullptr);
    YJsonItem* appendItem(double, const char* key=nullptr);
    YJsonItem* appendItem(const char*, const char* key=nullptr);

    bool removeItem(int index);
    bool removeItem(const char* key);
    bool removeItemByValue(int value);
    bool removeItemByValue(double value);
    bool removeItemByValue(std::string value);

    static YJsonDebug DEBUG_OUT_PUT;

private:
	YJsonItem *_next = nullptr,*_prev = nullptr;
    
	YJsonItem *_child = nullptr; YJsonItem *_parent = nullptr;
    char *_valuestring = nullptr;
	int _valueint = 0;
	double _valuedouble = 0;

    char *_keystring = nullptr;
    YJson _type = YJson::YJSON_NULL;

    char *_buffer = nullptr;
    int _depth = 1; static FILE_ENCODE _encode;

    const char *parse_value(const char *value);
    void print_value();
    const char *parse_number(const char *num);
    void print_number();
    const char *parse_string(const char *str);
    void print_string(bool use_keystring = false);
    const char *parse_array(const char *value);
    void print_array();
    const char *parse_object(const char *value);
    void print_object();

    void joinKeyValue(const char* valuestring, const bool use_key = true);
    void clearContent();
    void CopyJson(const YJsonItem*, YJsonItem*);
    void TakeJson(YJsonItem*, YJsonItem*, bool cpoy_key = true);
    void UpdateDepth(int depth);

    bool removeItem(YJsonItem*);
    YJsonItem* appendItem(YJson type);

    friend std::ostream& operator<<(std::ostream &out, YJsonItem &c1);
    friend std::ostream& operator<<(std::ostream &out, YJsonItem *c1);
    friend std::ostream& operator<<(std::ostream &out, const YJsonItem &c1);
    friend std::ostream& operator<<(std::ostream &out, const YJsonItem *c1);
    friend std::ostream& operator<<(std::ostream &out, YJsonItem &&c1);
};
