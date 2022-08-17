#include <fstream>
#include <httplib.h>

int main()
{
    std::ofstream file("C:/Users/hacker/Documents/GitHub/Neobox/build/tests/https.txt",
                       std::ios::out | std::ios::binary);
    httplib::HttpGet get;
#if 0
    auto res = get.Get("https://wallhaven.cc");
#else
    auto res = get.Get("https://global.bing.com/HPImageArchive.aspx?format=js&idx=0&n=8&mkt=zh-CN");
#endif
    if (res)
    {
        file << res->body << std::endl;
    }
    else
    {
        file << "No response.\n";
    }
    file.close();
    return 0;
}
