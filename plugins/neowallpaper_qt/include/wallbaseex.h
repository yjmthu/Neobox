#ifndef WALLBASEEX_H
#define WALLBASEEX_H

#include <menubase.hpp>
#include <yjson.h>

class WallBaseEx: public MenuBase
{
protected:
  typedef std::function<void(const YJson&)> Callback;
  YJson m_Data;
  // bool finished = false;
  void SaveSettings() const;
public:
  explicit WallBaseEx(Callback callback, YJson json, MenuBase* parent);
  virtual ~WallBaseEx();
private:
  const Callback m_Callback;
};

#endif // WALLBASEEX_H