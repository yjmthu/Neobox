#ifndef TABVERSION_HPP
#define TABVERSION_HPP

#include <QWidget>

class TabVersion: public QWidget
{
// protected:
//   void showEvent(QShowEvent *event) override;
public:
  explicit TabVersion(QWidget* parent);
  ~TabVersion();
private:
  // const class YJson* m_VersionInfo;
  class QPushButton* m_btnWeb;
  class QPushButton* m_btnBug;
  class QPushButton* m_btnChk;
  class QTextBrowser* m_Text;
  void LoadJson();
  void Connect();
  void GetUpdate();
};

#endif // TABVERSION_HPP
