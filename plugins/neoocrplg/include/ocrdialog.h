#ifndef OCRDIALOG_H
#define OCRDIALOG_H

#include <widgetbase.hpp>
#include <neoocr.h>

class OcrDialog: public WidgetBase {
public:
  explicit OcrDialog(NeoOcr& ocr, OcrDialog*& self);
  virtual ~OcrDialog();
private:
  void InitBaseLayout();
  void SetImage(QString path);
  QString OpenImage();
private:
  NeoOcr& m_OcrEngine;
  OcrDialog* & m_Self;
  class QLabel* m_ImageLabel;
  QImage* m_Image;
};

#endif // OCRDIALOG_H