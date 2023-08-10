#include <weather.hpp>
#include <httplib.h>

#ifdef WEATHER_WRONG_USER_KEY
#define QWEATHER_ID  "XXXXXXXXXXXXXXXXXX"
#define QWEATHER_KEY "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#else
#include "apikey.cpp"
#endif

#ifdef _WIN32
#include <QtZlib/zlib.h>
#else
#include <zlib.h>
#endif

using namespace std::literals;

template<class _IBufferType, class _OBufferType>
bool GzipCompress(const _IBufferType& inBuffer, _OBufferType& outBuffer)
{
  std::vector<Bytef> tempBuffer(1 << 15);
  int windowBits = 15;

  if (inBuffer.empty()) {
    return 1;
  }

  z_stream gzipStream {
    .next_in = (Bytef *)inBuffer.data(),
    .avail_in = static_cast<uInt>(inBuffer.size()),
  };

  auto error = deflateInit2(&gzipStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
      MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY);
  if (error != Z_OK) return false;
  do {
    std::fill(tempBuffer.begin(), tempBuffer.end(), 0);
    gzipStream.next_out = tempBuffer.data();
    gzipStream.avail_out = tempBuffer.size();
    error = deflate(&gzipStream, Z_FINISH);
    outBuffer.append(tempBuffer.begin(), tempBuffer.end() - gzipStream.avail_out);
  } while (error == Z_OK);
  deflateEnd(&gzipStream);
  return error == Z_STREAM_END;
}

template<class _IBufferType, class _OBufferType>
bool GZipUnCompress(const _IBufferType& inBuffer, _OBufferType& outBuffer)
{
  std::vector<Bytef> tempBuffer(1 << 15);
  z_stream gzipStream {
    .next_in = (Bytef*)inBuffer.data(),
    .avail_in = static_cast<uInt>(inBuffer.size()),
  };

  int error = inflateInit2(&gzipStream, MAX_WBITS + 16);
  if (error != Z_OK) {
    mgr->ShowMsg("zlib 初始化失败！");
    return false;
  }
  do {
    std::fill(tempBuffer.begin(), tempBuffer.end(), 0);
    gzipStream.avail_out = tempBuffer.size();
    gzipStream.next_out = tempBuffer.data();
    error = inflate(&gzipStream, Z_FINISH);
    outBuffer.append(tempBuffer.begin(), tempBuffer.end() - gzipStream.avail_out);
  } while (error == Z_OK);
  inflateEnd(&gzipStream);
  if (error != Z_STREAM_END) {
    mgr->ShowMsg("zilb 解压过程失败！\n" + QString::number(error));
  }
  return error == Z_STREAM_END;
}

Weather::Weather(const WeatherCfg& config)
  : QObject()
  , m_JSON { std::nullopt }
  , m_Config(config)
  , m_Hosts({
    u8"devapi.qweather.com"s,
    u8"geoapi.qweather.com"s
  })
  , m_Paths({
    u8"/v2/city/lookup?"s,
    u8"/v7/weather/24h?"s,
    u8"/v7/weather/7d?"s,
    u8"/v7/weather/now?"s,
  })
{
  // qRegisterMetaType<Weather::GetTypes>("Weather::GetTypes");
}

Weather::~Weather()
{
  std::lock_guard<std::mutex> locker(m_Mutex);
  m_Request = nullptr;
  m_JSON = std::nullopt;
}

HttpUrl Weather::GetUrl(GetTypes type, std::optional<std::u8string_view> data) const
{
  using enum GetTypes;
  std::u8string host;
  auto path = m_Paths[static_cast<int>(type)];
  std::u8string city;
  switch (type) {
  case Cities:
    host = m_Hosts[1];
    city = *data;
    break;
  default:
    if (m_Config.GetIsPaidUser()) {
      host = m_Hosts[0].substr(3);
    } else {
      host = m_Hosts[0];
    }
    city = m_Config.GetCity();
    break;
  }
  auto apikey = m_Config.GetApiKey();
  if (apikey.empty()) apikey = u8"" QWEATHER_KEY ""sv;

  HttpUrl url(host, path, {
    {u8"location", city},
    {u8"key", apikey},
  }, u8"https", 443);

  if (type == Cities) {
    url.parameters[u8"range"] = u8"cn";
  }

  return url;
}

void Weather::Fetch(GetTypes type, std::optional<std::u8string_view> data)
{
  std::lock_guard<std::mutex> locker(m_Mutex);
  if (m_Request && !m_Request->IsFinished()) {
    mgr->ShowMsg("上一次请求还未结束！");
    return;
  }

  auto&& url = GetUrl(type, data);
  m_Request = std::make_unique<HttpLib>(url, true, 2s);
  
  HttpLib::Callback callback = {
    .m_FinishCallback = [this, type](auto msg, auto res) {
      if (msg.empty() && res->status / 100 == 2) {
        std::lock_guard<std::mutex> locker(m_Mutex);
        std::u8string unCompressed;
        if (GZipUnCompress(res->body, unCompressed)) {
          m_JSON = std::make_optional<YJson>(unCompressed.begin(), unCompressed.end());
          auto iter = m_JSON->find(u8"code");
          if (iter != m_JSON->endO() && iter->second.isString()) {
            if (iter->second.getValueString() == u8"200") {
              emit Finished(static_cast<int>(type), true);
              return;
            } else {
              auto msg = u8"密钥暂时失效！错误码：" + iter->second.getValueString();
              mgr->ShowMsg(QString::fromUtf8(msg.data(), msg.size()));
            }
          } else {
            mgr->ShowMsg(u8"无效的响应体！");
          }
          m_JSON = std::nullopt;
        }
      } else {
#ifdef _DEBUG
        mgr->ShowMsgbox(L"请求出错", std::format(L"错误信息：{}\n错误码：{}", msg, res->status));
#else
        mgr->ShowMsg(u8"HTTP请求出错");
#endif
      }
      emit Finished(static_cast<int>(type), false);
    },
  };

  m_Request->GetAsync(std::move(callback));
}

std::optional<YJson> Weather::Get()
{
  std::lock_guard<std::mutex> locker(m_Mutex);
  if (m_Request && !m_Request->IsFinished()) return std::nullopt;
  if (!m_JSON) return std::nullopt;

  std::optional<YJson> result = std::move(*m_JSON);
  m_JSON = std::nullopt;

  return result;
}