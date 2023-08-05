#include <weather.hpp>
#include <httplib.h>

#include "apikey.cpp"

#include <QtZlib/zlib.h>

using namespace std::literals;

template<class _IBufferType, class _OBufferType>
bool GzipCompress(const _IBufferType& inBuffer, _OBufferType& outBuffer)
{
  std::array<Bytef, 4096> tempBuffer;
  int windowBits = 15;

  if (inBuffer.empty()) {
    return 1;
  }

  z_stream gzipStream {
    .next_in = (Bytef *)inBuffer.data(),
    .avail_in = static_cast<z_uInt>(inBuffer.size()),
  };

  auto error = deflateInit2(&gzipStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
      MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY);
  if (error != Z_OK) return false;
  do {
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
  std::array<Bytef, 4096> tempBuffer;
  z_stream gzipStream {
    .next_in = (Bytef*)inBuffer.data(),
    .avail_in = static_cast<z_uInt>(inBuffer.size()),
  };

  int error = inflateInit2(&gzipStream, MAX_WBITS + 16);
  if (error != Z_OK) {
    mgr->ShowMsg("zlib 初始化失败！");
    return false;
  }
  do {
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

Weather::Weather()
  : QObject()
  , m_WeatherData { std::nullopt }
  , m_CityList { std::nullopt }
{
  //
}

Weather::~Weather()
{
  //
}

void Weather::FetchDays()
{
  std::lock_guard<std::mutex> locker(m_Mutex);
  if (m_Request && !m_Request->IsFinished()) return;

  HttpUrl url(u8"https://devapi.qweather.com/v7/weather/3d?"sv, {
    {u8"location", u8"101010100"},
    {u8"key", u8"" QWEATHER_KEY},
  });

  m_Request = std::make_unique<HttpLib>(url, true, 5);
  
  HttpLib::Callback callback = {
    .m_FinishCallback = [this](auto msg, auto res) {
      if (msg.empty() && res->status / 100 == 2) {
        std::lock_guard<std::mutex> locker(m_Mutex);
        std::u8string unCompressed;
        if (GZipUnCompress(res->body, unCompressed)) {
          m_WeatherData = std::make_optional<YJson>(unCompressed.begin(), unCompressed.end());
          emit Finished(true);
          return;
        }
      }
      emit Finished(false);
    },
  };

  m_Request->GetAsync(std::move(callback));
}

std::optional<YJson> Weather::GetDays()
{
  std::lock_guard<std::mutex> locker(m_Mutex);
  if (m_Request && !m_Request->IsFinished()) return std::nullopt;
  if (!m_WeatherData) return std::nullopt;

  std::optional<YJson> result = std::move(*m_WeatherData);
  m_WeatherData = std::nullopt;

  return result;
}