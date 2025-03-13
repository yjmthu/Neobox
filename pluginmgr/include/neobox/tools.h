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
    m_Value = begin;
  }
  ~ValueGuard() {
    m_Value = end;
  }
private:
  ValueType& m_Value;
  ValueType m_Begin;
  ValueType m_End;
};

typedef ValueGuard<bool> BoolGuard;

#endif // NEOBOX_TOOLS
