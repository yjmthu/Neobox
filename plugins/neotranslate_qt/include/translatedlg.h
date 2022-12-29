#ifndef TRANSLATEDLG_H
#define TRANSLATEDLG_H

#include <QWidget>

class NeoTranslateDlg : public QWidget {
 protected:
  void showEvent(QShowEvent*) override;
  void hideEvent(QHideEvent *event) override;
  bool eventFilter(QObject*, QEvent*) override;

 public:
  explicit NeoTranslateDlg(class YJson& setings);
  ~NeoTranslateDlg();
  void ToggleVisibility();

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
private:
  void GetResultData();
  class QWidget* ReferenceObject() const;
  void AddCombbox(class QHBoxLayout* layout);
  void ChangeLanguageSource(bool checked);
  void ChangeLanguageFrom(int index);
  void ChangeLanguageTo(int index);
};

#endif // TRANSLATEDLG_H
