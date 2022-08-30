#include <fstream>
#include <httplib.h>
#include <xstring>

int main()
{
    std::ofstream file("C:/Users/hacker/Documents/GitHub/Neobox/build/tests/https.txt",
                       std::ios::out | std::ios::binary);
    std::u8string body;
#if 0
    auto res = get.Get("https://wallhaven.cc");
#else
    auto res = HttpLib::Get("https://global.bing.com/HPImageArchive.aspx?format=js&idx=0&n=8&mkt=zh-CN", body);
#endif
    if (res)
    {
        file.write(reinterpret_cast<const char*>(body.data()), body.size());
        file.put('\n');
    }
    else
    {
        file << "No response.\n";
    }
    file.close();
    return 0;
}
