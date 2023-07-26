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
  QString OpenImage();
private:
  NeoOcr& m_OcrEngine;
  OcrDialog* & m_Self;
  class OcrImageQFrame* m_ImageLabel;
};

#endif // OCRDIALOG_H