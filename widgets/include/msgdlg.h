#ifndef MSGDLG_H
#define MSGDLG_H

#include <QWidget>

class MsgDlg: QWidget
{
  friend class VarBox;
  explicit MsgDlg();
  ~MsgDlg();
  void ShowMessage(const QString& text);

  void InitWindowStyle();
  void InitContent();
  void InitAnimation();
  class QGraphicsOpacityEffect *m_pOpacity;
  class QPropertyAnimation *m_pAnimation;
  class QFrame *m_pFrame;
  class QLabel *m_pLabel;
protected:
  void showEvent(QShowEvent *event) override;
};

#endif
