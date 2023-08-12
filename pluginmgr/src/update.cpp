#include <update.hpp>
#include <httplib.h>
#include <config.h>
#include <neotimer.h>
#include <systemapi.h>
#include <neomenu.hpp>
#include <zip.h>

#include <chrono>
#include <utility>
#include <array>
#include <regex>
#include <filesystem>

#include <QMessageBox>
#include <QProcess>
#include <QApplication>
#include <QDir>

using namespace std::literals;
namespace fs = std::filesystem;

PluginUpdate::PluginUpdate(YJson& settings)
  : m_Settings(InitSettings(settings))
  , m_Timer(new NeoTimer)
{
#if 1
  connect(this, &PluginUpdate::AskInstall, this, [this](){
    if (QMessageBox::question(mgr->m_Menu, "提示", "有新版本可用，是否下载更新？") == QMessageBox::No)
      return;
    
    DownloadUpgrade([](PluginUpdate& obj){
      obj.CopyExecutable();
    });
  });
  connect(this, &PluginUpdate::QuitApp, this, [](QString exe, QStringList arg){
    qApp->quit();
#ifdef _WIN32
    // 设置进程优先级-实时，使其抢先于操作系统组件之前运行
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    // 设置线程优先级-实时
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#endif
    QProcess::startDetached(exe, arg);
  });
#endif
  if (m_Settings.GetAutoCheck()) {
    m_Timer->StartTimer(10s, [this]() {
      Callback callback = [this](PluginUpdate& self) {
        if (!self.NeedUpgrade()) return;
        if (!self.m_Settings.GetAutoUpgrade()) {
          emit AskInstall();
        } else {
          // mgr->ShowMsg("开始更新！");
          self.DownloadUpgrade([](PluginUpdate& obj){
            obj.CopyExecutable();
          });
        }
      };
      CheckUpdate(std::move(callback));
      m_Timer->Expire();
    });
  }
}

PluginUpdate::~PluginUpdate()
{
  delete m_Timer;
  m_DataRequest = nullptr;
}


YJson& PluginUpdate::InitSettings(YJson& settings)
{
  if (!settings.isObject()) {

    settings = YJson::O {
      {u8"LastCheck", YJson::String},
      {u8"UpgradeCycle", 0},
      {u8"AutoCheck", true},
      {u8"AutoUpgrade", false},
    };
  }

  return settings;
}

std::array<int, 3> PluginUpdate::ParseVersion(const std::wstring& vStr)
{
  std::array<int, 3> version = {0, 0, 0};
  std::wregex pattern(L"^v(\\d+).(\\d+).(\\d+)$");
  std::wsmatch match;
  if (std::regex_match(vStr, match, pattern)) {
    version = { std::stoi(match[1]), std::stoi(match[2]), std::stoi(match[3]) };
  }
  return version;
}

bool PluginUpdate::NeedUpgrade() const
{
  if (!m_LatestData) return false;

  auto& versionString = (*m_LatestData)[u8"tag_name"].getValueString();
  auto const vNew = PluginUpdate::ParseVersion(Utf82WideString(versionString));
  auto const vOld = PluginUpdate::ParseVersion(L"" NEOBOX_VERSION);

  return vNew != vOld;
}

fs::path PluginUpdate::GetTempFilePath() const
{
  fs::path pluginTemp = mgr->GetJunkDir();
  // fs::path pluginDst = pluginTemp;
  pluginTemp /= m_ZipUrl.substr(m_ZipUrl.rfind(u8'/') + 1);

  return pluginTemp;
}

void PluginUpdate::CopyExecutable() const
{
  const auto pluginTempPath = GetTempFilePath();
  const auto pluginTemp= pluginTempPath.string();
  const auto pluginDst = pluginTempPath.parent_path().string();
  const auto ret = zip_extract(pluginTemp.c_str(), pluginDst.c_str(), nullptr, nullptr);
  if (ret < 0) return;
  fs::remove(pluginTempPath);

  fs::path dataDir = mgr->GetJunkDir() / L"Neobox";
  dataDir.make_preferred();
  fs::path exePath = dataDir / "update.exe";
  if (!fs::exists(dataDir) || !fs::is_directory(dataDir) || !fs::exists(exePath)) {
    return;
  }

  exePath.make_preferred();

  emit QuitApp(QString::fromStdWString(exePath.wstring()), {
    QDir::toNativeSeparators(qApp->applicationDirPath())
  });
}

void PluginUpdate::DownloadUpgrade(Callback cb)
{
  if (!m_LatestData) return;

  for (auto& asset: (*m_LatestData)[u8"assets"].getArray()) {
    auto& url = asset[u8"browser_download_url"].getValueString();
#ifdef _WIN32
    if (!url.ends_with(u8".zip")) {
      continue;
    }
#else
    if (!url.ends_with(u8".tar.gz")) {
      continue;
    }
#endif
    m_ZipUrl = url;
    m_File.open(GetTempFilePath(), std::ios::out | std::ios::binary);
    if (!m_File.is_open()) return;

    std::thread([this, cb](){
      m_DataRequest = std::make_unique<HttpLib>(HttpUrl(m_ZipUrl), true);

      HttpLib::Callback callback = {
        .m_WriteCallback = [this](auto data, auto size) {
          m_File.write(reinterpret_cast<const char*>(data), size);
        },
        .m_FinishCallback = [cb, this](auto msg, auto res){
          m_File.close();
          if (msg.empty() && res->status == 200) {
            cb(*this);
          }
        },
      };
      m_DataRequest->GetAsync(std::move(callback));
    }).detach();
    return;
  }
}

void PluginUpdate::CheckUpdate(Callback cb)
{
  if (m_DataRequest && !m_DataRequest->IsFinished()) return;

  m_DataRequest = std::make_unique<HttpLib>(u8"" NEOBOX_LATEST_URL ""sv, true);

  m_DataRequest->SetHeader(u8"User-Agent", u8"Libcurl in Neobox App/1.0");
  HttpLib::Callback callback = {
    .m_FinishCallback = [this, cb](auto msg, auto res) {
      if (msg.empty() && res->status == 200) {
        m_LatestData = std::make_unique<YJson>(res->body.begin(), res->body.end());
        cb(*this);
      }
    },
  };
  m_DataRequest->GetAsync(std::move(callback));
}
