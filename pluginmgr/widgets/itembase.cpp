#include <neobox/downloadingdlg.hpp>
#include "itembase.hpp"
#include "plugincenter.hpp"

#include <neobox/httplib.h>
#include <neobox/pluginmgr.h>

#include <format>
#include <filesystem>

#ifdef _WIN32
#include <zip.h>
#else
#include <QProcess>
#endif

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialog>

namespace fs = std::filesystem;
using namespace std::literals;

ItemBase::ItemBase(std::u8string_view pluginName, const YJson& data, QWidget* parent)
  : QWidget(parent)
  , m_PluginName(pluginName)
  , m_PluginAuthor(data[u8"Author"].getValueString())
  , m_PluginFriendlyName(data[u8"FriendlyName"].getValueString())
  , m_PluginDescription(data[u8"Description"].getValueString())
  , m_MainLayout(new QHBoxLayout)
{
  GetVersion(m_PluginOldVersion, data[u8"Version"]);;
  m_PluginNewVersion = m_PluginOldVersion;
  SetupUi();
}


void ItemBase::GetVersion(Version& version, const YJson& array)
{
  for (auto iter = array.beginA(); auto& ver: version) {
    ver = static_cast<uint8_t>(iter->getValueInt());
    ++iter;
  }
}

void ItemBase::SetupUi()
{
  // m_MainLayout->setContentsMargins(0, 0, 0, 0);
  QVBoxLayout* layout = new QVBoxLayout(this);
  m_PluginNameLabel = new QLabel(
        QStringLiteral("<h3>%1</h3>").arg( QString::fromUtf8(m_PluginFriendlyName.data(), m_PluginFriendlyName.size())), this);
  m_MainLayout->addWidget(m_PluginNameLabel);

  QHBoxLayout* subLayout = new QHBoxLayout;
  m_PluginAuthorLabel = new QLabel("作者：" + QString::fromUtf8(m_PluginAuthor.data(), m_PluginAuthor.size()), this);
  subLayout->addWidget(m_PluginAuthorLabel);

  subLayout->addSpacing(20);

  m_DescriptionLabel = new QLabel("简介：" + QString::fromUtf8(m_PluginDescription.data(), m_PluginDescription.size()), this);
  subLayout->addWidget(m_DescriptionLabel);
  subLayout->addStretch();

  layout->addLayout(m_MainLayout);
  layout->addLayout(subLayout);
}

void ItemBase::UpdateUi()
{
  m_PluginNameLabel->setText(QStringLiteral("<h3>%1</h3>").arg(QString::fromUtf8(m_PluginFriendlyName.data(), m_PluginFriendlyName.size())));
  m_PluginAuthorLabel->setText("作者：" + QString::fromUtf8(m_PluginAuthor.data(), m_PluginAuthor.size()));
  m_DescriptionLabel->setText("简介：" + QString::fromUtf8(m_PluginDescription.data(), m_PluginDescription.size()));
}

void ItemBase::SetVersionLabel(std::wstring_view preText, Version& version, QLabel* label)
{
  const std::wstring text = std::format(L"{}：{}.{}.{}", preText, version[0], version[1], version[2]);
  label->setText(QString::fromStdWString(text));
}

/*
 *
  "neospeedboxplg": {
    "Enabled": true,
    "FriendlyName": "网速悬浮",
    "Description": "实时显示网速和内存占用",
    "Author": "yjmthu",
    "Version": [0, 0, 1]
  },
 *
 */

// int on_extract_entry(const char *filename, void *arg) {
//     // static int i = 0;
//     // int n = *(int *)arg;
//     // printf("Extracted: %s (%d of %d)\n", filename, ++i, n);
//     return 0;
// }

bool ItemBase::PluginDownload()
{
  if (!HttpLib::IsOnline()) {
    mgr->ShowMsgbox(L"失败", L"请检查网络连接！");
    return false;
  }
  bool result = false;
  DownloadingDlg dialog(this);
  connect(this, &ItemBase::DownloadFinished, &dialog, &QDialog::close);
  connect(this, &ItemBase::Downloading, &dialog, &DownloadingDlg::SetPercent);

#ifdef _WIN32
  auto const plugin = m_PluginName + u8".zip";
#else
  auto const plugin = m_PluginName + u8".tar.gz";
#endif
  const auto pluginTemp = mgr->GetJunkDir() / plugin;
  const auto pluginDst = mgr->GetPluginDir() / m_PluginName;
  HttpLib clt(HttpUrl(PluginCenter::m_RawUrl + plugin), true, 10s);

  std::ofstream file(pluginTemp, std::ios::out | std::ios::binary);
  if (!file.is_open()) return false;

  HttpLib::Callback callback = {
    .onProcess = nullptr,
    .onFinish = [&](auto msg, auto res) {
      file.close();
      if (!msg.empty() || res->status != 200) {
        mgr->ShowMsgbox(L"失败", L"下载清单失败！");
        // result = false;
      } else {
#ifdef _WIN32
        auto temp = pluginTemp.string();
        auto dst = pluginDst.string();
        result = zip_extract(temp.c_str(), dst.c_str(), nullptr, nullptr) >= 0;
        if (!result) {
          mgr->ShowMsgbox(L"失败", L"无法解压文件");
        }
#else
        std::error_code error;
        if (!fs::exists(pluginDst) && !fs::create_directories(pluginDst, error)) {
          mgr->ShowMsgbox(L"失败", std::format(L"创建文件夹失败，错误码：{}。", error.value()));
          return;
        }
        QProcess process;
        const auto ret = process.execute("tar", QStringList {
          QStringLiteral("-xzf"),
          QString::fromStdWString(pluginTemp.wstring()),
          QStringLiteral("-C"),
          QString::fromStdWString(pluginDst.wstring())
        });
        result = result == 0;
        if (!result) {
          mgr->ShowMsgbox(L"失败", std::format(L"解压文件出错，tar返回值：{}。", ret));
        }
#endif
      }

      fs::remove(pluginTemp);
      emit DownloadFinished();
    },
    .onWrite = [&file](auto data, auto size) {
      file.write(reinterpret_cast<const char*>(data), size);
    },
  };
  connect(&dialog, &DownloadingDlg::Terminate, std::bind(&HttpLib::ExitAsync, &clt));

  clt.GetAsync(std::move(callback));
  dialog.exec();
  return result;
}

void ItemBase::PluginInstall()
{
  bool result = PluginCenter::m_Instance->UpdatePluginData();

  if (result) {
    result = PluginDownload();
  }

  if (result) {
    result = mgr->InstallPlugin(m_PluginName, PluginCenter::m_Instance->m_PluginData->find(u8"Plugins")->second.find(m_PluginName)->second);
  }

  mgr->ShowMsg(result ? "安装成功！" : "安装失败！");
  DoFinished(FinishedType::Install, result);
}

void ItemBase::PluginUninstall()
{
  bool result = mgr->UnInstallPlugin(m_PluginName);

  if (result) {
    std::error_code error;
    fs::remove_all(mgr->GetPluginDir() / m_PluginName, error);
    result = error.value() == 0;
  }

  mgr->ShowMsg(result ? "卸载成功！" : "卸载失败！");

  DoFinished(FinishedType::Uninstall, result);
}

void ItemBase::PluginUpgrade()
{
  // bool result = m_PluginOldVersion < m_PluginNewVersion;
  bool result = PluginCenter::m_Instance->UpdatePluginData();
  YJson* pluginsInfo = nullptr;
  if (result) {
    pluginsInfo = &PluginCenter::m_Instance->m_PluginData->find(u8"Plugins")->second;
    auto iter = pluginsInfo->find(m_PluginName);
    result = iter != pluginsInfo->endO();
  }
  
  auto const pluginEnabled = mgr->IsPluginEnabled(m_PluginName);
  result = result && mgr->UpdatePlugin(m_PluginName, nullptr);

  if (result) {
    std::error_code error;
    fs::remove_all(mgr->GetPluginDir() / m_PluginName, error);
    result = error.value() == 0;
  }

  if (result) {
    auto& pluginData = pluginsInfo->find(m_PluginName)->second;
    pluginData[u8"Enabled"] = pluginEnabled;
    result = PluginDownload() && mgr->UpdatePlugin(m_PluginName, &pluginData);
    if (result) {
      GetVersion(m_PluginNewVersion, pluginData[u8"Version"]);
      m_PluginFriendlyName = pluginData[u8"FriendlyName"].getValueString();
      m_PluginDescription = pluginData[u8"Description"].getValueString();
      UpdateUi();
    }
  }

  mgr->ShowMsg(result ? "升级成功！" : "升级失败！");
  DoFinished(FinishedType::Upgrade, result);
}

