#ifndef TRANSLATEDLG_H
#define TRANSLATEDLG_H

#include <QWidget>

class TranslateDlg : public QWidget {
 protected:
  void showEvent(QShowEvent*) override;
  bool eventFilter(QObject*, QEvent*) override;

 public:
  explicit TranslateDlg(QWidget* parent);
  ~TranslateDlg();
  void Show(QRect rect, const QString& text = "");

 private:
  void GetResultData();
  class QPlainTextEdit *m_TextFrom, *m_TextTo;
  class QComboBox *m_BoxFrom, *m_BoxTo;
  class Translate* m_Translate;
  class QPushButton *m_BtnCopyFrom, *m_BtnCopyTo;
  QPushButton* m_BtnTransMode;
  void AddCombbox(class QHBoxLayout* layout);
  void ChangeLanguage(int from = 0);
  QRect m_FormRect;
};

#endif
