#ifndef TABVERSION_HPP
#define TABVERSION_HPP

#include <QWidget>

class TabVersion: public QWidget
{
// protected:
//   void showEvent(QShowEvent *event) override;
public:
  explicit TabVersion(class PluginCenter* parent);
  ~TabVersion();
private:
  // const class YJson* m_VersionInfo;
  class QHBoxLayout* m_MainLayout;
  class QPushButton* m_btnWeb;
  class QPushButton* m_btnBug;
  class QPushButton* m_btnChk;
  class QTextBrowser* m_Text;
private:
  void InitLayout();
  void LoadJson();
  void Connect();
  void GetUpdate();
};

#endif // TABVERSION_HPP
