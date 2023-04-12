#include <wallbaseex.h>

WallBaseEx::WallBaseEx(Callback callback, YJson json, MenuBase* parent)
  : MenuBase(parent)
  , m_Callback(std::move(callback))
  , m_Data(std::move(json))
{}

void WallBaseEx::SaveSettings() const
{
  m_Callback(m_Data);
}

WallBaseEx::~WallBaseEx() {
  // if (finished) {
  //   m_Callback(std::move(m_Data));
  // }
}
