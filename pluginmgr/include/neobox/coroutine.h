#ifndef COROUTINE_H
#define COROUTINE_H

#include <coroutine>
#include <functional>
#include <optional>
#include <mutex>
#include <condition_variable>

class AsyncActionBase {
  typedef std::function<void()> ExceptionCallback;
protected:
  class Awaiter {
  protected:
    AsyncActionBase& m_Action;
  public:
    Awaiter(AsyncActionBase& action) : m_Action(action) {}
    bool await_ready() const { return m_Action.finished(); }
    void await_suspend(std::coroutine_handle<> handle) {
      m_Action.m_AwaiterHandle = handle;
    }
    void await_resume() {}
  };
public:
  AsyncActionBase()
    : m_Finished(false)
    , m_AwaiterHandle {}
    , m_ExceptionCallback { }
  {}
  AsyncActionBase(AsyncActionBase&& other) = delete;
  AsyncActionBase(AsyncActionBase& other) = delete;
  AsyncActionBase(AsyncActionBase const& other) = delete;
#ifdef _DEBUG
  ~AsyncActionBase() {
    // std::cout << "~AsyncActionBase" << std::endl;
  }
#else
  ~AsyncActionBase() = default;
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

class AsyncPromise {
public:
  auto initial_suspend() -> std::suspend_never { return {}; }
  auto final_suspend() noexcept -> std::suspend_never { return {}; }
};

template<typename ReturnType>
class AsyncAction : public AsyncActionBase {
public:
  typedef std::function<void(std::optional<ReturnType>&)> Callback;
  class promise_type : public AsyncPromise {
  public:
    auto get_return_object() {
      return AsyncAction(this);
    }
    auto return_value(ReturnType value) -> void {
      m_Action->return_value(std::move(value));
    }
    void unhandled_exception() {
      if (m_Action) m_Action->handle_exception();
    }
    
    void set_action(AsyncAction* action) {
      m_Action = action;
    }

  private:
    AsyncAction* m_Action = nullptr;
  };

  AsyncAction(promise_type* promise)
    : AsyncActionBase()
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
      AwaiterValue(AsyncAction& action)
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
class AsyncAction<void> : public AsyncActionBase {
public:
  typedef std::function<void()> Callback;
  class promise_type : public AsyncPromise {
  public:
    auto get_return_object() {
      return AsyncAction(this);
    }
    auto return_void() -> void {
      m_Action->return_void();
    }
    auto unhandled_exception() -> void {
      if (m_Action) m_Action->handle_exception();
    }

    void set_action(AsyncAction* action) {
      m_Action = action;
    }
  private:
    AsyncAction* m_Action = nullptr;
  };

  AsyncAction(promise_type* promise)
    : AsyncActionBase()
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

template<typename  ReturnType>
class AsyncAwaiterObject {
public:
  virtual void DoSuspend(std::coroutine_handle<> handle) {
    m_Handle = handle;
  };
  virtual ~AsyncAwaiterObject() = default;
  virtual ReturnType* GetResult() = 0;
protected:
  std::coroutine_handle<> m_Handle;
};


template<typename ReturnType>
class AsyncAwaiter {
  AsyncAwaiterObject<ReturnType>* const m_Object;
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

  AsyncAwaiter(AsyncAwaiterObject<ReturnType>* object) : m_Object(object) {}
  AsyncAwaiter(AsyncAwaiter&& other) : m_Object(other.m_Object) {
    // m_Finished = other.m_Finished;
    other.m_Finished = true;
  }
  ~AsyncAwaiter() {
    if (!m_Finished && m_Object) {
      m_Object->DoSuspend(nullptr);
    }
  }
};

typedef AsyncAction<void> AsyncVoid;
typedef AsyncAction<bool> AsyncBool;
typedef AsyncAction<int> AsyncInt;
typedef AsyncAction<std::string> AsyncString;
typedef AsyncAction<std::wstring> AsyncWString;
typedef AsyncAction<std::u8string> AsyncU8String;

#endif
