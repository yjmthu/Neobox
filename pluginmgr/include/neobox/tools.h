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
  ~ValueGuard() {
    if (!m_Released) {
      m_Value = m_End;
    }
  }

  void set(const ValueType& value) { m_Value = value; }

  void release() { m_Released = true; }
  void retrieve() { m_Released = false; }
private:
  ValueType& m_Value;
  ValueType m_Begin;
  ValueType m_End;
  bool m_Released = false;
};

typedef ValueGuard<bool> BoolGuard;

#endif // NEOBOX_TOOLS
