#ifndef UPDATE_H
#define UPDATE_H

#include <memory>
#include <functional>
#include <yjson.h>
#include <neoconfig.h>

#include <QObject>

struct UpgradeConfig: NeoConfig
{
  ConfigConsruct(UpgradeConfig)
  CfgString(LastCheck)
  CfgInt(UpgradeCycle)
  CfgBool(AutoCheck)
  CfgBool(AutoUpgrade)
};

class PluginUpdate: public QObject
{
  Q_OBJECT

public:
  typedef std::function<void(PluginUpdate&)> Callback;
private:
  static YJson& InitSettings(YJson& settings);
public:
  explicit PluginUpdate(YJson& settings);
  ~PluginUpdate();
  void CheckUpdate(Callback callback);
  bool NeedUpgrade() const;
  void DownloadUpgrade(Callback callback);
  void CopyExecutable() const;
  static std::array<int, 3> ParseVersion(const std::wstring& vStr);
  std::filesystem::path GetTempFilePath() const;
  UpgradeConfig m_Settings;
private:
  class NeoTimer* m_Timer;
  std::u8string m_ZipUrl;
  std::unique_ptr<YJson> m_LatestData;
  std::unique_ptr<class HttpLib> m_DataRequest;
  std::ofstream m_File;
signals:
  void AskInstall();
  void QuitApp(QString exe, QStringList arg) const;
};

#endif // UPDATE_H
