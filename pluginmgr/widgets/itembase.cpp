#include "downloadingdlg.hpp"
#include "itembase.hpp"
#include "plugincenter.hpp"

#include <httplib.h>
#include <pluginmgr.h>

#include <format>
#include <filesystem>
#include <thread>

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

int ItemBase::GetFileCount(const YJson& manifest)
{
  int result = 0;
  for (auto& [dir, info]: manifest.getObject()) {
    result += static_cast<int>(info[u8"files"].sizeA());
  }
  return result;
}

bool ItemBase::PluginDownload()
{
  if (!HttpLib::IsOnline()) {
    mgr->ShowMsgbox(u8"失败", u8"请检查网络连接！");
    return false;
  }
  bool result = false, exit = false;
  const auto dialog = new DownloadingDlg(this);
  dialog->setAttribute(Qt::WA_DeleteOnClose, true);
  dialog->m_PreventClose = true;
  connect(this, &ItemBase::DownloadFinished, dialog, &QDialog::close);
  connect(this, &ItemBase::Downloading, dialog, &DownloadingDlg::SetPercent);

  std::thread thread([&]() {
    HttpLib clt(PluginCenter::m_RawUrl + u8"plugins/" + m_PluginName + u8"/manifest.json");
    auto res = clt.Get();
    dialog->m_PreventClose = false;

    if (res->status != 200) {
      mgr->ShowMsgbox(u8"失败", u8"下载清单失败！");
      emit DownloadFinished();
      return;
    }

    const YJson data(res->body.begin(), res->body.end());
    const int size = GetFileCount(data);
    int count = 0;
    for (auto& [dir, info]: data.getObject()) {
      if (exit) return;

      if (!fs::exists(dir)) {
        fs::create_directories(dir);
      }
      fs::path path = dir;
      for (auto& subdir: info[u8"dirs"].getArray()) {
        auto subpath = path / subdir.getValueString();
        if (!fs::exists(subpath)) {
          fs::create_directory(subpath);
        }
      }
      for (auto& file: info[u8"files"].getArray()) {
        if (exit) return;

        auto const subpath = path / file.getValueString();
        clt.SetUrl<char8_t>(
            PluginCenter::m_RawUrl + dir + u8"/" +
            file.getValueString());
        res = clt.Get(subpath);
        if (res->status != 200) {
          mgr->ShowMsgbox(u8"失败", u8"下载插件失败！");
          emit DownloadFinished();
          return;
        }
        emit Downloading(++count, size);
      }
    }
    result = true;
    emit DownloadFinished();
  });
  dialog->exec();
  exit = true;
  thread.join();
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
    fs::remove_all(u8"plugins/" + m_PluginName, error);
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
    fs::remove_all(u8"plugins/" + m_PluginName, error);
    result = error.value() == 0;
  }

  if (result) {
    auto& data = pluginsInfo->find(m_PluginName)->second;
    data[u8"Enabled"] = pluginEnabled;
    result = PluginDownload() && mgr->UpdatePlugin(m_PluginName, &data);
    if (result) {
      GetVersion(m_PluginNewVersion, data[u8"Version"]);
      m_PluginFriendlyName = data[u8"FriendlyName"].getValueString();
      m_PluginDescription = data[u8"Description"].getValueString();
      UpdateUi();
    }
  }

  mgr->ShowMsg(result ? "升级成功！" : "升级失败！");
  DoFinished(FinishedType::Upgrade, result);
}

