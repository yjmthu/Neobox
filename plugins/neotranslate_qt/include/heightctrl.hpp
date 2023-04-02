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
  explicit HeightCtrl(class NeoTranslateDlg* parent, std::list<YJson> setting);
  ~HeightCtrl() {}
public:
  void UpdateUi() {
    SetTextHeight(m_Source == Translate::Baidu ? m_Baidu : m_Youdao);
    SetStyleSheet(m_Source == Translate::Baidu ? m_Baidu : m_Youdao);
  }
public:
  int m_Baidu;
  int m_Youdao;
private:
  void SetStyleSheet(double value);
  void SetTextHeight(double value);
private:
  bool m_Changed;
  Translate::Source& m_Source;
  class QPlainTextEdit& m_TextFrom;
  class QTextEdit& m_TextTo;
protected:
  static constexpr int m_MaxRatio = 340;
  static constexpr int m_MinRatio = 20;
  static constexpr int m_DefaultRatio = 180;
};

#endif // HEIGHTCTRL_HPP
