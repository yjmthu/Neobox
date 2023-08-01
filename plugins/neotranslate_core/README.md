# 极简翻译编译说明

需要在include文件夹下创建名为`apikey.h`的文件，并定义如下宏

```cpp
#define ICIBA_KEY "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define BAIDU_ID "xxxxxxxxxxxxxxxxx"
#define BAIDU_KEY "xxxxxxxxxxxxxxxxxxxx"
#define YOUDAO_ID "xxxxxxxxxxxxxxxx"
#define YOUDAO_KEY "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
```

如要正常使用翻译功能，将其中的 `xxxxxx` 替换为你的 **app id** 和 **secret key**。
