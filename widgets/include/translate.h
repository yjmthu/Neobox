#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <QDialog>

class Translate: public QDialog
{
 public:
  explicit Translate(QWidget* parent=nullptr);
  ~Translate();
  void Show(const QString& text);
 private:
  void GetResultData();
  class QPlainTextEdit* m_TextFrom, * m_TextTo;
};

#endif
