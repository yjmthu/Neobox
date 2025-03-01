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

class HttpActionBase {
  typedef std::function<void()> ExceptionCallback;
protected:
  class Awaiter {
  protected:
    HttpActionBase& m_Action;
  public:
    Awaiter(HttpActionBase& action) : m_Action(action) {}
    bool await_ready() const { return m_Action.finished(); }
    void await_suspend(std::coroutine_handle<> handle) {
      m_Action.m_AwaiterHandle = handle;
    }
    void await_resume() {}
  };
public:
  HttpActionBase()
    : m_Finished(false)
    , m_AwaiterHandle {}
    , m_ExceptionCallback { }
  {}
  HttpActionBase(HttpActionBase&& other) = delete;
  HttpActionBase(HttpActionBase& other) = delete;
  HttpActionBase(HttpActionBase const& other) = delete;
#ifdef _DEBUG
  ~HttpActionBase() {
    std::cout << "~HttpActionBase" << std::endl;
  }
#else
  ~HttpActionBase() = default;
#endif

  void handle_exception() {
    if (m_ExceptionCallback) m_ExceptionCallback();
  }
protected:
  std::mutex mutable m_Mutex {};
  std::condition_variable m_CV {};
  bool m_Finished = false;
  std::coroutine_handle<> m_AwaiterHandle;
  ExceptionCallback m_ExceptionCallback;

  void notify_return() {
    m_Mutex.lock();
    m_Finished = true;
    m_Mutex.unlock();

    if (m_AwaiterHandle) {
      m_AwaiterHandle.resume();
    } else {
      m_CV.notify_all();
    }
  }

  void wait_return() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_CV.wait(lock, [this] { return m_Finished; });
  }

  bool finished() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Finished;
  }
};

class HttpPromise {
public:
  auto initial_suspend() -> std::suspend_never { return {}; }
  auto final_suspend() noexcept -> std::suspend_never { return {}; }
};

template<typename ReturnType>
class HttpAction : public HttpActionBase {
public:
  typedef std::function<void(std::optional<ReturnType>&)> Callback;
  class promise_type : public HttpPromise {
  public:
    auto get_return_object() {
      return HttpAction(this);
    }
    auto return_value(ReturnType value) -> void {
      m_Action->return_value(std::move(value));
    }
    void unhandled_exception() {
      if (m_Action) m_Action->handle_exception();
    }
    
    void set_action(HttpAction* action) {
      m_Action = action;
    }

  private:
    HttpAction* m_Action = nullptr;
  };

  HttpAction(promise_type* promise)
    : HttpActionBase()
    , m_Callback {}
    , m_Value {}
  {
    promise->set_action(this);
  }

  std::optional<ReturnType> get() {
    wait_return();
    return std::move(m_Value);
  }

  auto& then(Callback callback) {
    m_Callback = std::move(callback);
    return *this;
  }

  auto& cat(std::function<void()> callback) {
    m_ExceptionCallback = std::move(callback);
    return *this;
  }

  auto awaiter() {
    class AwaiterValue: public Awaiter {
    public:
      AwaiterValue(HttpAction& action)
        : Awaiter(action)
        , m_Value(action.m_Value) {}
      std::optional<ReturnType> await_resume() {
        return std::move(m_Value);
      }
    private:
      std::optional<ReturnType>& m_Value;
    };
    return AwaiterValue(*this);
  }
private:
  void return_value(ReturnType value) {
    m_Value = std::move(value);
    if (m_Callback) m_Callback(m_Value);
    this->notify_return();
  }
  Callback m_Callback;
  std::optional<ReturnType> m_Value;
};

template<>
class HttpAction<void> : public HttpActionBase {
public:
  typedef std::function<void()> Callback;
  class promise_type : public HttpPromise {
  public:
    auto get_return_object() {
      return HttpAction<void>(this);
    }
    auto return_void() -> void {
      m_Action->return_void();
    }
    auto unhandled_exception() -> void {
      if (m_Action) m_Action->handle_exception();
    }

    void set_action(HttpAction* action) {
      m_Action = action;
    }
  private:
    HttpAction* m_Action = nullptr;
  };

  HttpAction<void>(promise_type* promise)
    : HttpActionBase()
    , m_Callback {}
  {
    promise->set_action(this);
  }

  void get() { wait_return(); }

  auto& then(Callback callback) {
    m_Callback = callback;
    return *this;
  }

  auto& cat(std::function<void()> callback) {
    m_ExceptionCallback = callback;
    return *this;
  }

  auto awaiter() {
    return Awaiter(*this);
  }

  void return_void() {
    if (m_Callback) m_Callback();
    this->notify_return();
  }
private:
  Callback m_Callback;
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

template<typename  ReturnType>
class HttpAwaiterObject {
public:
  virtual void DoSuspend(std::coroutine_handle<>) = 0;
  virtual ~HttpAwaiterObject() = default;
  virtual ReturnType* GetResult() = 0;
};

template<typename ReturnType=HttpResponse>
class HttpAwaiter {
  HttpAwaiterObject<ReturnType>* const m_Object;
  bool m_Finished = false;
public:
  bool await_ready() const { return !m_Object; }
  void await_suspend(std::coroutine_handle<> handle) {
    if (m_Object) {
      m_Object->DoSuspend(handle);
    } else {
      handle.resume();
    }
    m_Finished = true;
  }

  auto await_resume() {
    return m_Object ? m_Object->GetResult() : nullptr;
  }

  HttpAwaiter(HttpAwaiterObject<ReturnType>* object) : m_Object(object) {}
  HttpAwaiter(HttpAwaiter&& other) : m_Object(other.m_Object) {
    // m_Finished = other.m_Finished;
    other.m_Finished = true;
  }
  ~HttpAwaiter() {
    if (!m_Finished && m_Object) {
      m_Object->DoSuspend(nullptr);
    }
  }
};

class HttpLib: public HttpAwaiterObject<HttpResponse> {
public:
  typedef std::recursive_mutex Mutex;
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
  HttpAwaiter<HttpResponse> GetAsync(std::optional<Callback> callback = std::nullopt);
  void ExitAsync();
  bool IsFinished() const { return m_Finished; }
  static bool IsOnline();
public:
  static std::optional<HttpProxy> m_Proxy;
private:
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
  // void StartAsync(std::coroutine_handle<> handle);
  void DoSuspend(std::coroutine_handle<> handle) override;
  Response* GetResult() override { return &m_Response; }
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
