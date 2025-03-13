#ifndef NEOBOX_TOOLS
#define NEOBOX_TOOLS

template<typename ValueType>
class ValueGuard {
public:
  ValueGuard(ValueType& value, ValueType begin, ValueType end)
    : m_Value(value)
    , m_Begin(begin)
    , m_End(end)
  {
    m_Value = m_Begin;
  }

  ValueGuard(const ValueGuard&) = delete;
  ValueGuard& operator=(const ValueGuard&) = delete;

  ValueGuard(ValueGuard&& other) noexcept
    : m_Value(other.m_Value)
    , m_Begin(other.m_Begin)
    , m_End(other.m_End)
    , m_Released(other.m_Released)
  {
    other.m_Released = true;
  }

  ~ValueGuard() {
    if (!m_Released) {
      m_Value = m_End;
    }
  }

  void Backward() { m_Value = m_Begin; }
  void Forward() { m_Value = m_Begin; }
  void Set(const ValueType& value) { m_Value = value; }
  void Release() { m_Released = true; }
  void Retrieve() { m_Released = false; }
private:
  ValueType& m_Value;
  ValueType m_Begin;
  ValueType m_End;
  bool m_Released = false;
};

typedef ValueGuard<bool> BoolGuard;

#endif // NEOBOX_TOOLS
