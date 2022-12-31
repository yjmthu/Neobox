#ifndef HEIGHTCTRL_H
#define HEIGHTCTRL_H

#include <QFrame>

#include <string>

#include <translate.h>

class HeightCtrl: public QFrame 
{
protected:
  void wheelEvent(QWheelEvent *event) override;
public:
  explicit HeightCtrl(class NeoTranslateDlg* parent, class YJson& setting);
  ~HeightCtrl() {}
public:
  void UpdateUi() {
    SetTextHeight(m_Source == Translate::Baidu ? m_Baidu : m_Youdao);
    SetStyleSheet(m_Source == Translate::Baidu ? m_Baidu : m_Youdao);
  }
private:
  void SetStyleSheet(double value);
  void SetTextHeight(double value);
private:
  double& m_Baidu;
  double& m_Youdao;
  bool& m_Changed;
  Translate::Source& m_Source;
  class QPlainTextEdit& m_TextFrom;
  class QTextEdit& m_TextTo;
protected:
  static constexpr int m_MaxRatio = 340;
  static constexpr int m_MinRatio = 20;
  static constexpr int m_DefaultRatio = 180;
};

#endif // HEIGHTCTRL_H
