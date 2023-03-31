#ifndef ITEMBASE_HPP
#define ITEMBASE_HPP

#include <QWidget>
#include <array>
#include <yjson.h>

class QLabel;

class ItemBase: public QWidget
{
  Q_OBJECT

signals:
  void DownloadFinished();
  void Downloading(int count, int size);
protected:
  enum class FinishedType { Download, Upgrade, Install, Uninstall };
  typedef std::array<uint8_t, 3> Version;
  static void GetVersion(Version& version, const YJson& array);
  static void SetVersionLabel(std::wstring_view preText,
      Version& version, class QLabel*);
  bool PluginDownload();
  virtual void DoFinished(FinishedType type, bool ok) = 0;
public slots:
  void PluginUninstall();
  void PluginInstall();
  void PluginUpgrade();
public:
  explicit ItemBase(std::u8string_view pluginName, const YJson& data, QWidget* parent);
  virtual ~ItemBase() {}
public:
  bool CanUpgrade() const { return m_PluginOldVersion < m_PluginNewVersion; }
public:
  // data member
  const std::u8string m_PluginName;
protected:
  // data member
  std::u8string m_PluginAuthor;
  std::u8string m_PluginFriendlyName;
  std::u8string m_PluginDescription;
  Version m_PluginOldVersion;
  Version m_PluginNewVersion;
  // ui member
  class QHBoxLayout* m_MainLayout;
private:
  QLabel* m_PluginNameLabel;
  QLabel* m_PluginAuthorLabel;
  QLabel* m_DescriptionLabel;
private:
  void SetupUi();
  void UpdateUi();
};

#endif // ITEMBASE_HPP
