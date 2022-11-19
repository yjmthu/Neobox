#ifndef VERSION_H
#define VERSION_H

#include <QDialog>

class VersionDlg: public QDialog
{
protected:
  void showEvent(QShowEvent *event) override;
public:
  explicit VersionDlg();
  ~VersionDlg();
private:
  // const class YJson* m_VersionInfo;
  class QPushButton* m_btnWeb;
  class QPushButton* m_btnChk;
  class QPushButton* m_btnCls;
  class QLabel* m_text;
  void LoadJson();
  void Connect();
  void GetUpdate();
};

#endif // VERSION_H
