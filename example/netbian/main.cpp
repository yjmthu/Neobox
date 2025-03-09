#include <neobox/httplib.h>
#include <neobox/unicode.h>

#include <iostream>
#include <fstream>
#include <regex>
#include <random>
#include <filesystem>

#ifdef NO_NETBIAN_COOKIE
#define COOKIE u8"your cookie"s
#else
#include "cookie.cpp"
#endif
#define USERAGENT u8"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/93.0.4577.63 Safari/537.36 Edg/93.0.961.47"s

#ifdef _WIN32
#include <windows.h>
#undef max
#else
#include <iconv.h>
#endif

using namespace std::literals;
namespace fs = std::filesystem;
const std::u8string domain = u8"pic.netbian.com";
const std::u8string host = u8"https://" + domain;

#ifdef _WIN32
std::u8string Gbk2Utf8(std::string_view gbk) {
  // gbk -> unicode windows
  int unicodeLen = MultiByteToWideChar(CP_ACP, 0, gbk.data(), (int)gbk.size(), nullptr, 0);
  std::wstring unicode(unicodeLen, 0);
  MultiByteToWideChar(CP_ACP, 0, gbk.data(), (int)gbk.size(), unicode.data(), unicodeLen);

  // unicode windows -> utf-8
  int utf8Len = WideCharToMultiByte(CP_UTF8, 0, unicode.data(), (int)unicode.size(), nullptr, 0, nullptr, nullptr);
  std::u8string utf8(utf8Len, 0);
  WideCharToMultiByte(CP_UTF8, 0, unicode.data(), (int)unicode.size(), (char*)utf8.data(), utf8Len, nullptr, nullptr);
  return utf8;
}
#else
std::u8string Gbk2Utf8(std::string_view gbk) {
  iconv_t cd = iconv_open("UTF-8", "GBK");
  if (cd == (iconv_t)-1) {
    return {};
  }

  std::string in(gbk);
  std::string out(2 * in.size(), 0);
  char* inbuf = const_cast<char*>(in.data());
  char* outbuf = out.data();
  size_t inbytesleft = in.size();
  size_t outbytesleft = out.size();
  iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
  iconv_close(cd);

  return std::u8string(out.begin(), out.end());
}
#endif

std::ostream& operator<<(std::ostream& os, const std::u8string& res) {
  os.write(reinterpret_cast<const char*>(res.data()), res.size());
  return os;
}

struct IndexPage {
  std::u8string title;
  std::u8string text;
  std::u8string href;
};

struct PictureDetail {
  int dataId;
  std::u8string name;
  std::u8string referer;
};


class TargetPasser {
public:
  std::string left;
  std::string right;
  std::string target;
  std::regex pattern;
private:
  const HttpUrl url;
  std::sregex_iterator wordsBegin, wordsEnd;
  bool GetTargetStringView(std::string_view str) {
    auto const left = str.find(this->left) + this->left.size();
    if (left == str.npos) {
      std::cerr << "Error: can not found left string." << std::endl;
      return false; 
    }
    auto const right = str.find(this->right, left);
    if (right == str.npos) {
      std::cerr << "Error: can not found right string." << std::endl;
      return false;
    }
    target = std::string_view(str.begin() + left, str.begin() + right);
    return true;
  }
public:
  explicit TargetPasser(HttpUrl url): url(std::move(url)) {}
  auto begin() const { return wordsBegin; }
  auto end() const { return wordsEnd; }
  bool perform() {
    HttpLib clt(url);
    clt.SetHeader(u8"User-Agent", USERAGENT);
    auto res = clt.Get();
    if (res->status / 100 != 2) {
      std::cerr << "Error: " << res->status << std::endl;
      return false;
    }
    std::string_view body(reinterpret_cast<const char*>(res->body.data()), res->body.size());
    if (!GetTargetStringView(body)) {
      std::cerr << "Error: can not found target string." << std::endl;
      return false;
    }
    wordsBegin = std::sregex_iterator(target.begin(), target.end(), pattern);
    wordsEnd = decltype(wordsBegin)();
    return true;
  }
};

std::vector<IndexPage> GetIndexPages() {
  TargetPasser passer((HttpUrl(host)));
  passer.left = "<li> <a href=\"javascript:;\" class=\"nav-link\">分类</a>"s;
  passer.right = "</li>"s;
  // <a href="/4Kxinnian/" title="4K新年图片">4K新年</a>
  passer.pattern = "<a href=\"(.+?)\" title=\"(.+?)\">(.+?)</a>";

  if (!passer.perform()) {
    return {};
  }

  std::vector<IndexPage> pages;
  for (auto& match: passer) {
    auto const& href = match[1].str();
    auto const& title = match[2].str();
    auto const& text = match[3].str();
    pages.push_back(IndexPage {
      .title = std::u8string(title.begin(), title.end()),
      .text = std::u8string(text.begin(), text.end()),
      .href = std::u8string(href.begin(), href.end()),
    });
  }
  return pages;
}

int GetPageCount(const IndexPage& indexPage) {
  TargetPasser passer(HttpUrl(domain, indexPage.href, {}, u8"https", 443));
  passer.left = "<div class=\"page\">"s;
  passer.right = "</div>"s;
  // <a href="/4kfengjing/index_204.html">204</a>
  passer.pattern = "<a href=\".+?\">(\\d+?)</a>"s;
  
  if (!passer.perform()) {
    return 0;
  }

  int pageCount = 1;
  for (auto& match: passer) {
    pageCount = std::max(pageCount, std::stoi(match[1].str()));
  }
  return pageCount;
}

std::vector<PictureDetail> GetPictureList(const IndexPage& pageInfo, int pageIndex) {
  std::string suffix = "index";
  if (pageIndex > 1) {
    suffix += "_" + std::to_string(pageIndex);
  }
  suffix += ".html";
  auto url = host + pageInfo.href + std::u8string(suffix.begin(), suffix.end());

  TargetPasser passer((HttpUrl(url)));
  passer.left = "<ul class=\"clearfix\">"s;
  passer.right = "</ul>"s;
  passer.pattern = "<li><a href=\"(.+?)\" target=\"_blank\"><img src=\".+?\" alt=\"(.+?)\" /><b>.+?</b></a></li>"s;
  if (!passer.perform()) {
    return {};
  }

  std::cout << "size: " << passer.target.size() << std::endl;

  std::vector<PictureDetail> pictures;
  for (auto& match: passer) {
    const auto href = match[1].str();
    auto referer = host + std::u8string(href.begin(), href.end());
    auto dataId = std::stoi(href.substr(8, href.size() - 13));
    auto name = match[2].str();
    pictures.push_back(PictureDetail{
      .dataId = dataId,
      .name = std::u8string(name.begin(), name.end()),
      .referer = referer
    });
  }
  return pictures;
}

std::u8string GetPictureUrl(const PictureDetail& detail) {
  static auto engine = std::default_random_engine();
  static std::uniform_real_distribution<double> distribution(0.0, 1.0);

  auto dataId = AnsiAsUtf8(std::to_string(detail.dataId));
  auto random = AnsiAsUtf8(std::format("{}", distribution(engine)));
  HttpUrl url(domain, u8"/e/extend/downpic.php"s, {
    { u8"id"s, dataId },
    { u8"t"s, random },
  }, u8"https", 443);

  // std::cout << "url: " << url.GetUrl() << std::endl;

  HttpLib clt(std::move(url));
  clt.SetHeader(u8"User-Agent", USERAGENT);
  clt.SetHeader(u8"Cookie", COOKIE);
  auto res = clt.Get();
  if (res->status / 100 != 2) {
    std::cerr << "Error: " << res->status << std::endl;
    return {};
  }
  auto jsonGBK = std::string_view(reinterpret_cast<const char*>(res->body.data()), res->body.size());
  auto jsonU8 = Gbk2Utf8(jsonGBK);
  YJson json(jsonU8.begin(), jsonU8.end());
  
  auto const code = json[u8"msg"].getValueInt();
  switch (code) {
    case 4:
      break;
    case 3:
      if (!json[u8"pic"].getValueString().empty()) {
        std::cout << "下载过于频繁，3秒后下载。" << std::endl;
      } else {
        std::cout << json[u8"info"] << std::endl;
        exit(1);
      }
      std::this_thread::sleep_for(3s);
      break;
    default:
      std::cerr << "Error: " << json << std::endl;
      return {};
  }
  return host + json[u8"pic"].getValueString();
}

AsyncInt DownloadPicture(const fs::path folder, const PictureDetail& detail) {
  auto filePath = folder / (detail.name + u8".jpg"s);
  if (fs::exists(filePath)) {
    std::cout << "File exists: " << filePath.u8string() << std::endl;
    co_return 1;
  }
  HttpUrl url { GetPictureUrl(detail) };
  std::cout << "url: " << url.GetUrl() << std::endl;
  HttpLib clt(std::move(url), true);
  clt.SetHeader(u8"Referer", detail.referer);
  clt.SetHeader(u8"User-Agent", USERAGENT);
  clt.SetHeader(u8"Cookie", COOKIE);

  std::ofstream fileOut(filePath, std::ios::binary);
  if (!fileOut.is_open()) {
    std::cerr << "Error: can not open file: " << filePath.u8string() << std::endl;
    co_return -1;
  }

  const float barWidth = 70.0;

  HttpLib::Callback callback {
    .onProcess = [barWidth](auto current, auto total) {
      // std::cout << "Progress: " << current << "/" << total << std::endl;
      auto percent = float(current) / total;
      int pos = barWidth * percent;
      std::cout << "[";
      std::cout << std::setfill('=') << std::setw(pos + 1) << '>';
      if (pos == barWidth) std::cout << "\b=";
      std::cout << std::setfill(' ') << std::setw(barWidth - pos + 1) << ']';
      std::cout << std::setw(4) << int(percent * 100.0) << " %\r";
      std::cout.flush();
    },
    .onFinish = [](auto msg, auto res) {
      std::cout << std::endl;
      if (msg.empty() && res->status == 200) {
        std::cout << "Download success" << std::endl;
      } else {
        std::cerr << "Error: " << msg << std::endl;
      }
    },
    .onWrite = [&fileOut](auto data, auto size) {
      fileOut.write(reinterpret_cast<const char*>(data), size);
    },
  };

  auto res = co_await clt.GetAsync(std::move(callback));
  fileOut.close();

  if (res->status != 200) {
    fs::remove(filePath);
    std::cerr << "Error: " << res->status << std::endl;
    co_return -1;
  }

  co_return 0;
}

void SleepRandom() {
  static auto engine = std::default_random_engine();
  static std::uniform_int_distribution<int> distribution(0, 500);
  const auto sleepTime = 3s + 2ms * distribution(engine);
  std::cout << "Begin sleep: " << sleepTime.count() << "ms. " << std::flush;
  std::this_thread::sleep_for(sleepTime);
  std::cout << "End sleep." << std::endl;
}


int main(int argc, char** argv) {
  ::SetLocale();

  auto indexPages = GetIndexPages();
  if (indexPages.empty()) {
    std::cerr << "Error: empty index pages" << std::endl;
    return 1;
  }
  // uer choose index page
  for (auto i = 0; i < indexPages.size(); ++i) {
    std::cout << i << ": " << indexPages[i].text << ' ';
  }
  std::cout << "\nChoose: ";
  int choose = 0;
  std::cin >> choose;
  if (choose < 0 || choose >= indexPages.size()) {
    std::cerr << "Error: invalid choose" << std::endl;
    return 1;
  }
  auto const& indexPage = indexPages[choose];
  auto const pageCount = GetPageCount(indexPage);
  std::cout << "Page name: " << indexPage.text
    << " Page count: " << pageCount << std::endl;
  
  // choose page range
  std::cout << "Choose page range: ";
  int begin = 0, end = 0;
  std::cin >> begin >> end;
  if (begin < 1 || end > pageCount || begin > end) {
    std::cerr << "Error: invalid page range" << std::endl;
    return 1;
  }

  fs::path folder = argc > 1 ? argv[1] : ".";
  if (!fs::exists(folder)) {
    folder = ".";
  }
  folder /= indexPage.text;
  if (!fs::exists(folder)) {
    fs::create_directory(folder);
  }
  std::cout << "Folder: " << fs::absolute(folder).u8string() << std::endl;

  for (auto i = begin; i <= end; ++i) {
    auto pictures = GetPictureList(indexPage, i);
    if (pictures.empty()) {
      std::cerr << "Error: empty picture list" << std::endl;
      return 1;
    }
    // Output in this format: "**************** page index: 1 ****************"
    std::cout << std::setfill('*') << std::setw(16) << ' '
      << "Page " << std::setfill('0') << std::setw(3) << i << ' '
      << std::setfill('*') << std::setw(16) << ' ' << std::endl;
    SleepRandom();
    for (auto const& picture : pictures) {
      std::cout << picture.dataId  << ": " << picture.name << std::endl;
      auto res = DownloadPicture(folder, picture).get();
      if (res == 1) continue;
      // get random integer from 0 to 500
      SleepRandom();
    }
  }
  return 0;
}