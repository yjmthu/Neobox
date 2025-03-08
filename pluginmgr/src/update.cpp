#include <neobox/update.hpp>
#include <neobox/httplib.h>
#include <config.h>
#include <neobox/neotimer.h>
#include <neobox/systemapi.h>
#include <neobox/neomenu.hpp>
#include <neobox/neosystemtray.hpp>

#ifdef _WIN32
#include <zip.h>
#include <wintoastlib.h>
#endif

// #include <chrono>
#include <utility>
#include <array>
#include <regex>
#include <filesystem>

#include <QMessageBox>
#include <QProcess>
#include <QApplication>
#include <QDir>

using namespace std::literals;
using namespace WinToastLib;
namespace fs = std::filesystem;

class CutomHanderForUpdate : public IWinToastHandler
{
public:
  explicit CutomHanderForUpdate(PluginUpdate& obj)
    : m_Obj(obj)
  {};
  virtual ~CutomHanderForUpdate() = default;
private:
  PluginUpdate& m_Obj;
public:
  void toastActivated() const override
  {
    // std::wcout << L"The user clicked in this toast" << std::endl;
  }

  void toastActivated(int actionIndex) const override
  {
    // std::wcout << L"The user clicked on action #" << actionIndex << std::endl;
    if (actionIndex == 0) {
      m_Obj.DownloadUpgrade().get();
    }
  }

  void toastDismissed(WinToastDismissalReason state) const override
  {
    // std::wcout << L"The toast has been dismissed" << std::endl;
    // std::wcout << L"Reason: ";
    switch (state) {
    case UserCanceled:
      break;
    case ApplicationHidden:
      // std::wcout << L"ApplicationHidden" << std::endl;
      break;
    case TimedOut:
      // std::wcout << L"TimedOut" << std::endl;
      break;
    default:
      // std::wcout << L"ApplicationNotInForeground" << std::endl;
      break;
    }
  }

  void toastFailed() const override
  {
    // std::wcout << L"Error showing current toast" << std::endl;
  }
};

PluginUpdate::PluginUpdate(YJson& settings)
  : m_Settings(InitSettings(settings))
#ifdef _WIN32
#endif
{
#if 1
  connect(this, &PluginUpdate::AskInstall, this, [this](){
    WinToastTemplate templ = WinToastTemplate(WinToastTemplate::ImageAndText01);
    templ.setTextField(L"Neobox有新版本，是否更新？", WinToastTemplate::FirstLine);

    std::vector<std::wstring> actions {
      L"立即更新", L"下次提醒"
    };
    for (auto const &action : actions) {
      templ.addAction(action);
    }

    auto handler(new CutomHanderForUpdate(*this));
    WinToast::instance()->showToast(templ, handler);
  });
  connect(this, &PluginUpdate::QuitApp, this, [](QString exe, QStringList arg){
    mgr->Quit();
#ifdef _WIN32
    // 设置进程优先级-实时，使其抢先于操作系统组件之前运行
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    // 设置线程优先级-实时
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#endif
    QProcess::startDetached(exe, arg);
  });
#endif

#ifdef _WIN32
  WinToast::instance()->setAppName(L"Neobox");
  WinToast::instance()->setAppUserModelId(WinToast::configureAUMI(L"yjmthu", L"neobox", L"neoboxupdate", L"20231226"));
  if (!WinToast::instance()->initialize()) {
    mgr->ShowMsg(u8"Error, your system in not compatible!");
  }
#endif
  if (m_Settings.GetAutoCheck()) {
#ifdef _DEBUG
    std::cout << "Auto check neobox update." << std::endl;
#endif
    NeoTimer::SingleShot(30s, [this] { this->StartAutoCheck().get(); });
  }
}

PluginUpdate::~PluginUpdate()
{
  WinToast::instance()->clear();
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
#ifdef _WIN32
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
#else
#endif
}

AsyncVoid PluginUpdate::DownloadUpgrade()
{
  if (!m_LatestData) co_return;

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
  if (!m_File.is_open()) co_return;

    m_DataRequest = std::make_unique<HttpLib>(HttpUrl(m_ZipUrl), true);

    HttpLib::Callback callback = {
      .onWrite = [this](auto data, auto size) {
        m_File.write(reinterpret_cast<const char*>(data), size);
      },
    };
    auto res = co_await m_DataRequest->GetAsync(std::move(callback));
    m_File.close();

    if (res->status != 200) {
      fs::remove(GetTempFilePath());
      co_return;
    }
    CopyExecutable();
  }
}

AsyncBool PluginUpdate::CheckUpdate()
{
  if (IsBusy()) co_return false;

  m_DataRequest = std::make_unique<HttpLib>(u8"" NEOBOX_LATEST_URL ""sv, true);

  m_DataRequest->SetHeader(u8"User-Agent", u8"Libcurl in Neobox App/1.0");
  auto res = co_await m_DataRequest->GetAsync();

  if (res->status != 200) {
    co_return false;
  }

  m_LatestData = std::make_unique<YJson>(res->body.begin(), res->body.end());

  co_return true;
}


AsyncVoid PluginUpdate::StartAutoCheck() {
  auto result = co_await CheckUpdate().awaiter();

  if (!result || !*result || !m_LatestData) {
#ifdef _DEBUG
    std::cout << "fetch update detail failed." << std::endl;
#endif
    co_return;
  }

  if (!NeedUpgrade()) {
#ifdef _DEBUG
    std::cout << "No need to upgrade." << std::endl;
#endif
    co_return;
  }
  if (m_Settings.GetAutoUpgrade()) {
#ifdef _DEBUG
    std::cout << "Auto upgrade neobox." << std::endl;
#endif
    co_await DownloadUpgrade().awaiter();
  } else {
    emit AskInstall();
  }
}

bool PluginUpdate::IsBusy() const { return m_DataRequest && !m_DataRequest->IsFinished(); }
