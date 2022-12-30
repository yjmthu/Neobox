#ifndef TRANSLATEDLG_H
#define TRANSLATEDLG_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QTextEdit>

#include <translate.h>
#include <pluginobject.h>

class NeoTranslateDlg : public QWidget {
 protected:
  void showEvent(QShowEvent*) override;
  void hideEvent(QHideEvent *event) override;
  bool eventFilter(QObject*, QEvent*) override;

 public:
  explicit NeoTranslateDlg(class YJson& setings);
  ~NeoTranslateDlg();
  void ToggleVisibility();
  template<typename _Utf8Array>
  void GetResultData(const _Utf8Array& text);

 private:
  class YJson& m_Settings;
  class QPlainTextEdit *m_TextFrom;
  class QTextEdit *m_TextTo;
  class QComboBox *m_BoxFrom, *m_BoxTo;
  class Translate* m_Translate;
  class QPushButton *m_BtnCopyFrom, *m_BtnCopyTo;
  class QPushButton* m_BtnTransMode;
  QPoint m_LastPostion;
  QSize m_LastSize;
  bool m_LanPairChanged = false;
  bool m_TextFromChanged = false;
private:
  class QWidget* ReferenceObject() const;
  void AddCombbox(class QHBoxLayout* layout);
  void ChangeLanguageSource(bool checked);
  void ChangeLanguageFrom(int index);
  void ChangeLanguageTo(int index);
};

template<typename _Utf8Array>
void NeoTranslateDlg::GetResultData(const _Utf8Array& text) {
  // m_Translate->m_LanPair = { m_BoxFrom->currentIndex(), m_BoxTo->currentIndex() };
  std::u8string s;
  if (!m_TextFromChanged) return;
  if (text.size() == 0) {
    m_TextTo->clear();
    m_TextFromChanged = false;
    return;
  }
  auto const& result = m_Translate->GetResult(text);
  m_TextTo->clear();
  if (m_Translate->GetSource() == Translate::Baidu)
    m_TextTo->setPlainText(PluginObject::Utf82QString(result));
  else
    m_TextTo->setHtml(PluginObject::Utf82QString(result));
  m_TextFromChanged = false;
}

#endif // TRANSLATEDLG_H
