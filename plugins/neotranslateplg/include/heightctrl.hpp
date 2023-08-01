#ifndef HEIGHTCTRL_HPP
#define HEIGHTCTRL_HPP

#include <QFrame>

#include <string>

#include <translate.h>
#include <translatecfg.h>

class HeightCtrl: public QFrame 
{
  Q_OBJECT

protected:
  void wheelEvent(QWheelEvent *event) override;
public:
  explicit HeightCtrl(class NeoTranslateDlg* parent, YJson setting);
  ~HeightCtrl() {}
public:
  void UpdateUi() {
    SetTextHeight(m_Heights[m_Source]);
    SetStyleSheet(m_Heights[m_Source]);
  }
public:
  std::array<int, Translate::None> m_Heights;
  bool m_Changed;
private:
  void SetStyleSheet(double value);
  void SetTextHeight(double value);
private:
  Translate::Source& m_Source;
  class QPlainTextEdit& m_TextFrom;
  class QTextBrowser& m_TextTo;
protected:
  static constexpr int m_MaxRatio = 200;
  static constexpr int m_MinRatio = 20;
  static constexpr int m_DefaultRatio = 180;
};

#endif // HEIGHTCTRL_HPP
