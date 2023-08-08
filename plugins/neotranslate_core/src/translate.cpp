#include <systemapi.h>
#include <array>
#include <translate.h>
#include <xstring>
#include <yjson.h>
#include <httplib.h>
#include <sha256.h>
#include <md5.h>
#include <translatecfg.h>

#include <chrono>
#include <format>
#include <random>

#ifdef TRANSLATE_WRONG_USER_KEY
#define ICIBA_KEY "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define BAIDU_ID "xxxxxxxxxxxxxxxxx"
#define BAIDU_KEY "xxxxxxxxxxxxxxxxxxxx"
#define YOUDAO_ID "xxxxxxxxxxxxxxxx"
#define YOUDAO_KEY "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#else
#include "apikey.cpp"
#endif

using namespace std::literals;
namespace chrono = std::chrono;

static auto& operator<<(std::ostream& stream, const std::u8string& other) {
  return stream.write(reinterpret_cast<const char*>(other.data()), other.size());
}

inline std::u8string GetTimeStamp() {
  using namespace std::chrono;
  const auto ms =
      duration_cast<seconds>(system_clock::now().time_since_epoch());
  const auto msCount = std::to_string(ms.count());
  return std::u8string(msCount.begin(), msCount.end());
}

std::u8string Truncate(const Utf8Array& q) {
  std::vector<char const*> string;
  // char8_t const* start = q.begin, *stop = start + q.end;
  for (auto ptr = q.begin; ptr != q.end; ++ptr) {
    if ((*ptr >> 6) != 0b10) string.push_back(
      reinterpret_cast<const char*>(ptr)
    );
  }
  if (string.size() > 20) {
    std::u8string result;
    result.reserve(20);
    std::format_to(std::back_inserter(result), "{}{}{}",
      std::string_view(reinterpret_cast<const char*>(q.begin), string[10]),
      string.size(),
      std::string_view(string[string.size() - 10], reinterpret_cast<const char*>(q.end)));
    return result;
  }
  return std::u8string(q.begin, q.end);
}

inline std::u8string Sha256(const std::u8string& str) {
		SHA256 sha;
		sha.update(str);
		uint8_t * digest = sha.digest();
		auto result =  SHA256::toString<char8_t>(digest);
		delete[] digest;
  return result;
}

inline std::u8string Md5(const std::u8string& str)
{
  return MD5(str).toStr<char8_t>();
}

inline std::u8string Uuid1() {
  std::u8string result;
  result.reserve(36);   // uuid length
  union {
    struct {
      uint32_t time_low;
      uint16_t time_mid;
      uint16_t time_hi_and_version;
      uint8_t clk_seq_hi_res;
      uint8_t clk_seq_low;
      uint8_t node[6];
    };
    uint16_t __rnd[8];
  } uuid;

  std::independent_bits_engine<std::default_random_engine, 16, uint16_t> bytes(
      std::default_random_engine{}());
  std::generate_n(uuid.__rnd, 8, std::ref(bytes));

  // Refer Section 4.2 of RFC-4122
  // https://tools.ietf.org/html/rfc4122#section-4.2
  uuid.clk_seq_hi_res = (uint8_t)((uuid.clk_seq_hi_res & 0x3F) | 0x80);
  uuid.time_hi_and_version =
      (uint16_t)((uuid.time_hi_and_version & 0x0FFF) | 0x4000);

  std::format_to(std::back_inserter(result), "{:08x}-{:04x}-{:04x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
      uuid.time_low, uuid.time_mid, uuid.time_hi_and_version, uuid.clk_seq_hi_res,
      uuid.clk_seq_low, uuid.node[0], uuid.node[1], uuid.node[2],
      uuid.node[3], uuid.node[4], uuid.node[5]);
  return result;
}


std::map<std::u8string, std::u8string> Translate::m_LangNameMap
{
  {u8"auto",   u8"自动检测"},
  {u8"zh",     u8"中文简体"},
  {u8"zh-CNS", u8"中文简体"},
  {u8"cht",    u8"中文繁体"},
  {u8"en",     u8"英语"},
  {u8"ja",     u8"日语"},
  {u8"jp",     u8"日语"},
  {u8"fra",    u8"法语"},
  {u8"fr",     u8"法语"},
  {u8"ko",     u8"韩语"},
  {u8"kor",    u8"韩语"},
  {u8"ru",     u8"俄语"},
};

const Translate::LanMaps Translate::m_LanguageCanFromTo
{
  Translate::LanMap {  // baidu
    {u8"auto", {u8"zh", u8"cht", u8"en", u8"jp", u8"fra", u8"ru", u8"kor"}},
    {u8"zh",   {u8"cht", u8"en", u8"jp", u8"fra", u8"ru", u8"kor"}},
    {u8"cht",  {u8"zh", u8"en", u8"jp", u8"fra", u8"ru", u8"kor"}},
    {u8"en",   {u8"zh", u8"cht", u8"jp", u8"fra", u8"ru", u8"kor"}},
    {u8"jp",   {u8"zh", u8"cht", u8"en", u8"fra", u8"ru", u8"kor"}},
    {u8"fra",  {u8"zh", u8"cht", u8"en", u8"jp", u8"ru", u8"kor"}},
    {u8"ru",   {u8"zh", u8"cht", u8"en", u8"jp", u8"fra", u8"kor"}},
    {u8"kor",   {u8"zh", u8"cht", u8"en", u8"jp", u8"fra", u8"ru"}},
   },
  Translate::LanMap {  // youdao
    {u8"auto",   {u8"auto", u8"zh-CNS", u8"en", u8"ja", u8"ko", u8"fr", u8"ru"}},
    {u8"zh-CNS", {u8"auto", u8"en", u8"ja", u8"ko", u8"fr", u8"ru"}},
    {u8"en",     {u8"auto", u8"zh-CNS", u8"ja"}},
    {u8"ja",     {u8"auto", u8"zh-CNS", u8"en"}},
    {u8"ko",     {u8"auto", u8"zh-CNS"}},
    {u8"fr",     {u8"auto", u8"zh-CNS"}},
    {u8"ru",     {u8"auto", u8"zh-CNS"}},
  },
  Translate::LanMap {
    {u8"auto", {u8"auto"}}
  },
  Translate::LanMap {
    {u8"auto", {u8"auto"}}
  },
  Translate::LanMap {
    {u8"en", {u8"en"}}
  },
};

LanPair::LanPair(const YJson::ArrayType& array)
  : f(array.front().getValueDouble())
  , t(array.back().getValueDouble())
{
}

LanPair::LanPair(std::array<int, 2> array)
  : f(0)
  , t(0)
{
}

Translate::Translate(TranslateCfg& setting, Translate::Callback&& callback)
    : m_Callback(std::move(callback))
    , m_AllLanPair({
        setting.GetPairBaidu(),
        setting.GetPairYoudao(),
        std::array<int, 2> {0, 0},
        std::array<int, 2> {0, 0},
        std::array<int, 2> {0, 0},
      })
    , m_LanPair(nullptr)
{
  SetSource(Baidu);
}

Translate::~Translate() {}

void Translate::SetSource(Source dict) {
  m_Source = dict;
  m_LanPair = &m_AllLanPair[dict];
}

std::optional<std::pair<int, int>> Translate::ReverseLanguage()
{
  auto const& dict = m_LanguageCanFromTo[m_Source];
  auto const& from = dict[m_LanPair->f].first;
  auto const& to = dict[m_LanPair->f].second[m_LanPair->t];
  auto iterFrom = std::find_if(dict.cbegin(), dict.cend(), [&to](decltype(dict.front())& item){
    return item.first == to;
  });
  if (iterFrom == dict.cend()) return std::nullopt;
  auto iterTo = std::find_if(iterFrom->second.cbegin(), iterFrom->second.cend(), [&from](const std::u8string& item){
    return item == from;
  });
  if (iterTo == iterFrom->second.cend()) return std::nullopt;
  return std::pair {
    static_cast<int>(iterFrom - dict.cbegin()),
    static_cast<int>(iterTo - iterFrom->second.cbegin())
  };
}

inline static std::u8string GetSalt() {
  auto const now = chrono::system_clock::now();
  auto const count = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
  std::string data = std::format("{}", count);
  return std::u8string(data.begin(), data.end());
}

void Translate::GetResultBaidu(const Utf8Array& text) {
  // http://api.fanyi.baidu.com/product/113
  static HttpUrl url(u8"https://fanyi-api.baidu.com/api/trans/vip/translate?"sv, {
    {
      {u8"from", u8"auto"},
      {u8"to", u8"zh"},
      {u8"q", std::u8string {}},
      {u8"appid", u8"" BAIDU_ID},
      {u8"salt", std::u8string {}},
      {u8"sign", std::u8string {}},
    }
  });
  static auto const badNet = u8"<h1>没网络了！</h1>"s;

  static auto& from = url.parameters[u8"from"];
  from = m_LanguageCanFromTo[Baidu][m_LanPair->f].first;
  static auto& to = url.parameters[u8"to"];
  to = m_LanguageCanFromTo[Baidu][m_LanPair->f].second[m_LanPair->t];

  static auto& salt = url.parameters[u8"salt"];
  salt = GetSalt();

  static auto& q = url.parameters[u8"q"];
  q.assign(text.begin, text.end);
 
  static auto& sign = url.parameters[u8"sign"];
  sign = Md5(u8"" BAIDU_ID ""s + q + salt + u8"" BAIDU_KEY);

  m_Request = std::make_unique<HttpLib>(url, true);
  
  HttpLib::Callback callback = {
    .m_FinishCallback = [this](auto message, auto res) {
      if (message.empty() && (res->status / 100 == 2)) {
        YJson jsData(res->body.begin(), res->body.end());
        if (auto iter = jsData.find(u8"error_msg"); iter != jsData.endO()) {
          // Access token expired
          m_Callback(res->body.data(), res->body.size());
          return;
          // return iter->second.getValueString();
        }
        const auto& obTransResult =
            jsData[u8"trans_result"].getArray();

        std::u8string content;
        for (auto& item : obTransResult) {
          content.append(item[u8"dst"].getValueString());
          content.push_back('\n');
        }
        m_Callback(content.data(), content.size());
        return;
      }
      m_Callback(badNet.data(), badNet.size());
    }
  };
  m_Request->GetAsync(std::move(callback));
}

void Translate::GetResultYoudao(const Utf8Array& text) {
  static HttpUrl url(u8"http://openapi.youdao.com/api/?"sv, {
    {u8"from", u8"auto"},
    {u8"to", u8"auto"},
    {u8"appKey", u8"" YOUDAO_ID},
    {u8"signType", u8"v3"},
    {u8"curtime", std::u8string {}},
    {u8"q", std::u8string {}},
    {u8"salt", std::u8string {}},
    {u8"sign", std::u8string {}}
  });

  static auto& from = url.parameters[u8"from"];
  from = m_LanguageCanFromTo[Youdao][m_LanPair->f].first;
  static auto& to = url.parameters[u8"to"];
  to = m_LanguageCanFromTo[Youdao][m_LanPair->f].second[m_LanPair->t];

  static auto& q = url.parameters[u8"q"];
  q.assign(text.begin, text.end);
  static auto& curtime = url.parameters[u8"curtime"];
  curtime = GetTimeStamp();
  static auto& salt = url.parameters[u8"salt"];
  salt = Uuid1();  //"d818bc30-99df-11ec-9e18-1cbfc0a98096";
  static auto& sign = url.parameters[u8"sign"];
  sign = Sha256(u8"" YOUDAO_ID + Truncate(text) + salt + curtime + u8"" YOUDAO_KEY);

  // 这样同时也取消了前一个请求
  m_Request = std::make_unique<HttpLib>(url, true);
  m_Request->SetHeader(u8"Content-Type", u8"application/x-www-form-urlencoded");

  HttpLib::Callback callback = {
    .m_FinishCallback = [this](auto message, auto res) {
      if (message.empty() && (res->status / 100 == 2)) {
        FormatYoudaoResult(YJson(res->body.begin(), res->body.end()));
      }
    }
  };

  m_Request->GetAsync(std::move(callback));
}

void Translate::FormatYoudaoResult(const YJson& data) {
  auto const& err = data[u8"errorCode"].getValueString();
  if (err != u8"0") {
    m_Callback(err.data(), err.size());
    return;
  }
  std::ostringstream html;

  // 查询内容
  html << "<h3>" << data[u8"query"].getValueString();

  // basic 基本词典，查词时才有
  if (auto basic = data[u8"basic"]; basic.isObject()) {

    // 获取读音
    auto const& l = data[u8"l"].getValueString();

    static const std::u8string lst[][2] {
      {u8"英"s, u8"uk-phonetic"s},
      {u8"美"s, u8"us-phonetic"s},
      {u8"拼音"s, u8"phonetic"s},
      {u8"读音"s, u8"phonetic"s},
    };
    size_t left = 0, right = 0;
    if (l.starts_with(u8"en2")) {
      left = 0;
      right = 2;
    } else if (l.starts_with(u8"zh")) {
      left = 2;
      right = 3;
    } else {
      left = 3;
      right = 4;
    }

    for (int i = left; i != right; ++i) {
      auto const& phonetic = basic[lst[i][1]];
      if (!phonetic.isString()) continue;
      html << "  " << lst[i][0] << "[<span style='color:#44EEEE;'>" << phonetic.getValueString() << "</span>]";
    }

    html << "</h3><hr/>";

    // 获取
    if (auto wfs = basic[u8"wfs"]; wfs.isArray() && !wfs.emptyA()) {
      html << "<h5>形式变化</h5><ul>";
      for (auto& wf : wfs.getArray()) {
        auto temp = wf.beginO();
        if (temp == wf.endO() || temp->first != u8"wf")
          break;
        html << "<li style='color:#FF77CC;'>"
          << temp->second.find(u8"name")->second.getValueString()
          << ": "
          << temp->second.find(u8"value")->second.getValueString()
          << "</li>";
      }
      html << "</ul><hr/>";
    }

    // 	基本释义
    if (const auto& explains = basic[u8"explains"]; explains.isArray() && !explains.emptyA()) {
      html << "<h5>基本释义</h5><ul>";
      for (const auto& i : explains.getArray()) {
        html << "<li>" << i.getValueString() << "</li>";
      }
      html << "</ul><hr/>";
    }
  } else {
    html << "</h3><hr/>";
  }

  // 网络释义
  if (auto web = data[u8"web"]; web.isArray() && !web.emptyA()) {
    html << "<h5>网络释义</h5><ol>";
    for (const auto& i : web.getArray()) {
      html << "<li><span style='color: #FF00FF;'>"
        << i[u8"key"].getValueString()
        << "</span> &lt;";
      const auto& value = i[u8"value"].getArray();
      for (auto& j : value) {
        html << j.getValueString() << "; ";
      }
      html << "&gt;</li>";
    }
    html << "</ol><hr/>";
  }

  // 翻译结果
  if (auto const& translation = data[u8"translation"]; translation.isArray() && !translation.emptyA()) {
    html << "<h5>翻译结果</h5><ul>";
    for (auto const& i : translation.getArray()) {
      html << "<li>" << i.getValueString() << "</li>";
    }
    html << "</ul><hr/>";
  }

  // html << "<br>" << l;
#ifdef _WIN32
  auto const view = html.rdbuf()->view();
#else
  auto const view = html.str();
#endif
  m_Callback(view.data(), view.size());
}

void Translate::GetResultBingSimple(const Utf8Array& text) {

  static HttpUrl url(u8"https://cn.bing.com/dict/SerpHoverTrans?"sv, {
    {u8"q", std::u8string {}},
  });
  url.parameters[u8"q"].assign(text.begin, text.end);

  m_Request = std::make_unique<HttpLib>(url, true);

  HttpLib::Callback callback = {
    .m_FinishCallback = [this](auto message, auto res) {
      if (message.empty() && (res->status / 100 == 2)) {
        m_Callback(res->body.data(), res->body.size());
      } else {
        std::wstring msg = std::format(L"error: {}\ncode:{}", message, res->status);
        auto u8msg = Wide2Utf8String(msg);
        m_Callback(u8msg.data(), u8msg.size());
      }
    }
  };

  m_Request->GetAsync(std::move(callback));
}

void Translate::GetResultIciba(const Utf8Array& text)
{
  HttpUrl url(u8"http://dict-co.iciba.com/api/dictionary.php?"sv, {
    {u8"type", u8"json"},
    {u8"key", u8"" ICIBA_KEY},
    {u8"w", std::u8string(text.begin, text.end)},
  });
  m_Request = std::make_unique<HttpLib>(url, true);

  HttpLib::Callback callback = {
    .m_FinishCallback = [this](auto message, auto res) {
      if (message.empty() && (res->status / 100 == 2)) {
        // m_Callback(res->body.data(), res->body.size());
        // return;
        try {
          YJson root(res->body.begin(), res->body.end());
          FormatIcibaResult(root);
        } catch (std::runtime_error error) {
          m_Callback(res->body.data(), res->body.size());
        }
      } else {
        std::wstring msg = std::format(L"error: {}\ncode:{}", message, res->status);
        auto u8msg = Wide2Utf8String(msg);
        m_Callback(u8msg.data(), u8msg.size());
      }
    }
  };

  m_Request->GetAsync(std::move(callback));
}

void Translate::FormatIcibaResult(const YJson& data)
{
  typedef std::map<std::u8string, std::string> Map;
  static const Map exchangeMap {
    {u8"word_pl"s, "复数"s},
    {u8"word_past"s, "过去时"s},
    {u8"word_done"s, "完成时"s},
    {u8"word_ing"s, "进行时"s},
    {u8"word_third"s, "三单"s},
    {u8"word_er"s, "比较级"s},
    {u8"word_est"s, "最高级"s},
  };

  static const Map symbolsMap {
    {u8"ph_en"s, "英式音标"s},
    {u8"ph_am"s, "美式音标"s},
    {u8"ph_other"s, "其它"s},
    {u8"word_symbol"s, "拼音"s},
  };
  static const Map mp3Map {
    {u8"ph_en_mp3"s, "英式发音"s},
    {u8"ph_am_mp3"s, "英式发音"s},
    {u8"ph_tts_mp3"s, "TTS发音"s},
    {u8"symbol_mp3"s, "发音"s},
  };
  auto& wordName = data[u8"word_name"];

  std::ostringstream html;

  if (wordName.isString()) {
    html << "<h3>" << wordName << "</h3><hr>";
  } else {
    html << "<p style=\"color: red;\">只能查询单词。</p>";
    goto callback;
  }

  if (auto& excahnge = data[u8"exchange"]; excahnge.isObject()) {
    html << "<h5>形式转换</h5><ul>";
    for (auto& [key, value]: excahnge.getObject()) {
      if (!value.isArray()) continue;
      auto iter = exchangeMap.find(key);
      if (iter == exchangeMap.end()) continue;
      html << "<li>" << iter->second << ": ";
      for (auto& val: value.getArray()) {
        html << val.getValueString() << "; ";
      }
      html << "</li>";
    }
    html << "</ul><hr>";
  }

  if (auto& symbols = data[u8"symbols"]; symbols.isArray() && !symbols.emptyA()) {
    auto& symble = symbols.frontA();
    html << "<h5>读音</h5><ul>";
    for (auto& [key, value]: symble.getObject()) {
      if (!value.isString() || value.getValueString().empty()) continue;
      auto iter = symbolsMap.find(key);
      if (iter == symbolsMap.end()) continue;
      html << "<li>" << iter->second << ": /" << value.getValueString() << "/</li>";
    }
    html << "</ul><hr>";

    auto& parts = symble[u8"parts"];

    if (parts.isArray() && !parts.emptyA()) {
      html << "<h5>基本释义</h5><ol>";
      for (auto& item: parts.getArray()) {
        auto& part = item[u8"part"];
        auto& means = item[u8"means"];

        if (part.isString()) {
          html << "<li><span style=\"color: purple;\">" << part.getValueString() << "</span>: ";
          for (auto& m : means.getArray()) {
            html << m.getValueString() << "; ";
          }
          html << "</li>";
        } else {
          for (auto& m : means.getArray()) {
            auto& mean = m.find(u8"word_mean");
            if (mean == m.endO() || mean->second.getValueString().empty()) continue;
            html << "<li>" << mean->second.getValueString() << ";</li>";
          }
        }
        
      }
      html << "</ol><hr>";
    }
  }

callback:
#ifdef _WIN32
  auto const view = html.rdbuf()->view();
#else
  auto const view = html.str();
#endif
  m_Callback(view.data(), view.size());
}

void Translate::GetResultDictionary(const Utf8Array& text)
{
  auto invalid = u8"只能翻译英文单词！"s;
  auto iter = std::find_if(text.begin, text.end, [](char8_t c){
    return !std::isalnum(c);
  });
  if (iter != text.end) {
    m_Callback(invalid.data(), invalid.size());
    return;
  }

  auto url = u8"https://api.dictionaryapi.dev/api/v2/entries/en/"s;
  url.append(text.begin, text.end);

  m_Request = std::make_unique<HttpLib>(HttpUrl(url), true);

  HttpLib::Callback callback = {
    .m_FinishCallback = [this](auto message, auto res) {
      if (message.empty() && (res->status / 100 == 2)) {
#if 0
        m_Callback(res->body.data(), res->body.size());
        return;
#else
        YJson root(res->body.begin(), res->body.end());
        FormatDictionaryResult(root);
#endif
      } else {
        std::wstring msg = std::format(L"error: {}\ncode:{}", message, res->status);
        auto u8msg = Wide2Utf8String(msg);
        m_Callback(u8msg.data(), u8msg.size());
      }
    }
  };

  m_Request->GetAsync(std::move(callback));
}

void Translate::FormatDictionaryResult(const class YJson& data)
{
  std::ostringstream html;

  for (auto& wordData: data.getArray()) {
    auto& word = wordData[u8"word"];
    html << "<h3>" << word.getValueString() << "</h3><hr>";
    auto& phonetics = wordData[u8"phonetics"];

    if (phonetics.isArray() && !phonetics.emptyA()) {
      html << "<h4>phonetics</h4><ul><li>";
      for (auto& item: phonetics.getArray()) {
        auto text = item.find(u8"text");
        if (text != item.endO() && text->second.isString()) {
          html << text->second.getValueString() << "; ";
        }
      }
      html << "</li></ul><hr>";
    }

    auto& meanings = wordData[u8"meanings"];
    if (meanings.isArray() && !meanings.emptyA()) {
      html << "<h4>meaning</h4><ul>";
      for (auto& item: meanings.getArray()) {
        html << "<li>[" << item[u8"partOfSpeech"].getValueString() << "]";
        auto& definitions = item[u8"definitions"];
        if (definitions.isArray() && !definitions.emptyA()) {
          html << "<ol>";
          for (auto& mean: definitions.getArray()) {
            auto& definition = mean[u8"definition"];
            auto& example = mean[u8"example"];
            html << "<li>" << definition.getValueString();
            if (example.isString()) {
              html << "<br><span style=\"color: green;\">e.g.</span> "
                << example.getValueString();
            }
            html << "</li>";
          }
          html << "</ol>";
        }
        html << "</li>";
      }
      html << "</ul><hr>";
    }

    auto& sourceUrls = wordData[u8"sourceUrls"];
    if (sourceUrls.isArray() && !sourceUrls.emptyA()) {
      html << "<h4>sourceUrls</h4><ol>";
      for (auto& url: sourceUrls.getArray()) {
        html << "<li><a href=\"" << url.getValueString() << "\">"
          << url.getValueString() << "</a></li>";
      }
      html << "</ol><hr>";
    }
  }
#ifdef _WIN32
  auto const view = html.rdbuf()->view();
#else
  auto const view = html.str();
#endif
  m_Callback(view.data(), view.size());
}
