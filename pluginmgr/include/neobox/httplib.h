#ifndef HTTPLIB_H
#define HTTPLIB_H

#include <coroutine>
#include <filesystem>
#include <string>
#include <map>
#include <neobox/httpproxy.h>
#include <atomic>
#include <chrono>

class HttpUrl {
  friend class HttpLib;
  typedef std::u8string_view::const_iterator Iterator;
  typedef std::u8string String;
  typedef std::u8string_view StringView;
  typedef String::value_type Char;
  typedef std::map<String, String> Params;
public:
  HttpUrl(StringView url);
  HttpUrl(StringView host,
    StringView path,
    Params params={},
    StringView scheme=u8"http",
    uint16_t port=80
  );
  HttpUrl(StringView url, Params params);
  HttpUrl(HttpUrl&& url) noexcept;
  HttpUrl(const HttpUrl& url);
  String GetUrl(bool showPort=false) const;
  void SetUrl(StringView url);
  String GetObjectString() const;
  bool IsHttps() const { return scheme.back() == u8's'; }
  HttpUrl& operator=(HttpUrl&& url) noexcept {
    scheme = std::move(url.scheme);
    host = std::move(url.host);
    path = std::move(url.path);
    port = url.port;
    parameters = std::move(url.parameters);
    return *this;
  }
public:
  String scheme = u8"http";
  String host;
  String path = u8"/";
  uint16_t port = 80;
  Params parameters;
private:
  void ParseScheme(Iterator& first, Iterator last);
  void ParseHost(Iterator& first, Iterator last);
  void ParsePort(Iterator& first, Iterator last);
  void ParseParams(Iterator& first, Iterator last);
  void ParsePath(Iterator& first, Iterator last);
  inline static Char ToHex(Char c) {
    constexpr Char A = u8'A' - 10;
    return c > 9 ? c + A : c + u8'0';
  }
  inline static Char FromHex(Char c) {
    constexpr Char A = u8'A' - 10;
    constexpr Char a = u8'a' - 10;
    
    if (std::isupper(c))
      return c - A;
    if (std::islower(c))
      return c - a;
    if (std::isdigit(c))
      return c - '0';

    return c;
  }
public:
  static void UrlEncode(StringView text, String& out);
  static void UrlDecode(StringView text, String& out);
};

struct HttpAsyncAction {
  struct promise_type {
    auto get_return_object() -> HttpAsyncAction {
      return HttpAsyncAction{Handle::from_promise(*this)};
    }
    auto initial_suspend() -> std::suspend_never { return {}; }
    auto final_suspend() noexcept -> std::suspend_never { return {}; }
    auto unhandled_exception() -> void {}

    void return_void() {
      m_Mutex.lock();
      m_Finished = true;
      m_Mutex.unlock();

      m_CV.notify_all();
    }

    void wait_return() {
      std::unique_lock<std::mutex> lock(m_Mutex);
      m_CV.wait(lock, [this] { return m_Finished; });
    }
  private:
    std::mutex m_Mutex;
    std::condition_variable m_CV;
    bool m_Finished = false;
  };
  using Handle = std::coroutine_handle<promise_type>;

  Handle m_Handle;
  HttpAsyncAction(Handle handle) : m_Handle(handle) {}

  HttpAsyncAction(HttpAsyncAction&& other) noexcept
    : m_Handle(other.m_Handle)
  {
    other.m_Handle = nullptr;
  }

  ~HttpAsyncAction() {
    // if (m_Handle) {
    //   m_Handle.destroy();
    // }
  }

  void get() { m_Handle.promise().wait_return(); }
};

struct HttpResponse {
  typedef std::map<std::u8string, std::u8string> Headers;

  std::u8string version;
  long status = -1;
  std::u8string reason;
  Headers headers;
  std::u8string body;
  std::u8string location; // Redirect location
};

class HttpAwaiter {
  HttpLib* const m_Lib;
  bool m_Finished = false;
public:
  bool await_ready() const { return false; }
  void await_suspend(HttpAsyncAction::Handle handle);
  HttpResponse* await_resume();

  HttpAwaiter(HttpLib* lib) : m_Lib(lib) {}
  HttpAwaiter(HttpAwaiter&& other) : m_Lib(other.m_Lib) {
    // m_Finished = other.m_Finished;
    other.m_Finished = true;
  }
  ~HttpAwaiter();
};

class HttpLib {
public:
  typedef std::mutex Mutex;
private:
  typedef std::lock_guard<Mutex> Locker;
  typedef std::unique_lock<Mutex> LockerEx;
public:
  typedef HttpResponse Response;
  typedef Response::Headers Headers;
  typedef uint64_t HttpId;
  typedef size_t( CallbackFunction )(void*, size_t, size_t, void*);

  struct PostData { void* data; size_t size; } m_PostData;

  typedef std::function<void(const void*, size_t)> WriteCallback;
  typedef std::function<void(size_t, size_t)> ProcessCallback;
  typedef std::function<void(std::wstring, const Response*)> FinishCallback;
  struct Callback {
    std::optional<WriteCallback> m_WriteCallback;
    std::optional<FinishCallback> m_FinishCallback;
    std::optional<ProcessCallback> m_ProcessCallback;
  };

  explicit HttpLib(HttpUrl url, bool async = false, std::chrono::seconds timeout=30s)
    : m_Url(std::move(url))
    , m_hSession(nullptr)
    , m_TimeOut(timeout)
    , m_ProxySet(false)
    , m_AsyncSet(async)
  {
    IntoPool();
    HttpPrepare();
    HttpInitialize();
  }
  ~HttpLib();
public:
  void SetUrl(HttpUrl::StringView url) {
    m_Url.SetUrl(url);
    HttpUninitialize();
    HttpInitialize();
  }
  void SetUrl(HttpUrl url) {
    m_Url = std::move(url);
    HttpUninitialize();
    HttpInitialize();
  }
  void SetHeader(std::u8string key, std::u8string value) {
    m_Headers[key] = value;
  }
  void ClearHeader() {
    m_Headers.clear();
  }
  std::u8string GetUrl() const {
    return m_Url.GetUrl();
  }
  void SetRedirect(long redirect);
  void SetPostData(void* data, size_t size);
  Response* Get();
  Response* Get(const std::filesystem::path& path);
  Response* Get(CallbackFunction* callback, void* userData);
  void SetTimeOut(std::chrono::seconds timeOut);
  HttpAwaiter GetAsync(std::optional<Callback> callback = std::nullopt);
  void ExitAsync();
  bool IsFinished() const { return m_Finished; }
  static bool IsOnline();
public:
  static std::optional<HttpProxy> m_Proxy;
private:
  friend HttpAwaiter;
  Headers m_Headers;
  Response m_Response;
  HttpUrl m_Url;
  void* m_hSession = nullptr;
#ifdef _WIN32
  void* m_hConnect = nullptr;
  void* m_hRequest = nullptr;
#endif
  std::chrono::seconds m_TimeOut { 30s };
  int m_RedirectDepth = 0;
  bool m_ProxySet;
  bool m_AsyncSet;
  std::atomic_bool m_Finished;
  size_t m_RecieveSize = 0;
  size_t m_ConnectLength = 0;
private:
  void StartAsync(HttpAsyncAction::Handle handle);
  void IntoPool();
  void HttpInitialize();
  void HttpUninitialize();
  void HttpPrepare();
  void HttpPerform();
  void ResetData();
  bool SendHeaders();
  void SetProxyBefore();
  bool SetProxyAfter();
  void SetAsyncCallback();
  bool SendRequest();
  bool RecvResponse();
private:
  bool ReadStatusCode();
  void ParseHeaders(const std::u8string&);
  bool ReadHeaders();
#ifdef _WIN32
  bool ReadBody();
#endif
  void EmitProcess();
  void EmitFinish(std::wstring message=L"");
private:
  static CallbackFunction WriteFile;
  static CallbackFunction WriteString;
#ifdef __linux__
  static CallbackFunction WriteHeader;
  static CallbackFunction WriteFunction;
#endif
#ifdef _WIN32
  static void RequestStatusCallback(void* hInternet, unsigned long long dwContext, unsigned long dwInternetStatus, void* lpvStatusInformation, unsigned long dwInternetInformationLength);
#endif
  std::function<bool(const void*, size_t)> m_WriteCallback;
  CallbackFunction* m_Callback;
  Callback m_AsyncCallback;
  void* m_DataBuffer = nullptr;
  HttpId m_AsyncId;
};

#endif
