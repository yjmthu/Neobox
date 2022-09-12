#ifndef TRANSLATEDLG_H
#define TRANSLATEDLG_H

#include <QWidget>

class TranslateDlg : public QWidget {
 protected:
  void showEvent(QShowEvent *) override;
  bool eventFilter(QObject *, QEvent *) override;
 public:
  explicit TranslateDlg(QWidget* parent);
  ~TranslateDlg();
  void Show(QRect rect, const QString& text="");

 private:
  void GetResultData();
  class QPlainTextEdit *m_TextFrom, *m_TextTo;
  class QComboBox *m_BoxFrom, *m_BoxTo;
  class Translate* m_Translate;
  void AddCombbox(class QHBoxLayout* layout);
  QRect m_FormRect;
};

#endif
