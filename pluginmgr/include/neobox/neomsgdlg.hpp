#ifndef NEOMSGDLG_HPP
#define NEOMSGDLG_HPP

#include <QWidget>
#include <queue>

class NeoMsgDlg: QWidget
{
  Q_OBJECT
  
public:
  explicit NeoMsgDlg(QWidget* parent);
  ~NeoMsgDlg();
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
private:
  void AdjustSize();
  void HandleShowMsg();
  std::queue<QString> m_Data;
};

#endif // NEOMSGDLG_HPP
