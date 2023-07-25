#ifndef TABVERSION_HPP
#define TABVERSION_HPP

#include <QWidget>

class YJson;

class TabVersion: public QWidget
{
  Q_OBJECT
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
  const QString m_TextRaw;
private:
  void InitLayout();
  static QString FormatString();
  void Connect();
  void GetUpdate();
  std::array<int, 3> ParseVersion(const std::wstring& vStr);
  bool DownloadNew(std::u8string_view url);
  void DoUpgrade(const YJson& data);
};

#endif // TABVERSION_HPP
