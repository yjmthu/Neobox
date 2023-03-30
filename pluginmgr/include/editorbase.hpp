#ifndef EDITORBASE_HPP
#define EDITORBASE_HPP

#include <yjson.h>
#include <functional>

class EditorBase {
public:
  typedef std::function<void(bool, const YJson&)> Callback;
  explicit EditorBase(YJson data, Callback calllback)
    : m_Data(std::move(data))
    , m_CallBack(calllback)
  { }
  ~EditorBase() {
    m_CallBack(m_DataChanged, m_Data);
  }
protected:
  YJson m_Data;
  bool m_DataChanged = false;
private:
  const Callback m_CallBack;
};

#endif