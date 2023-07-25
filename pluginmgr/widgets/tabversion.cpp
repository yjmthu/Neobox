#include "tabversion.hpp"
#include "plugincenter.hpp"
#include "downloadingdlg.hpp"

#include <systemapi.h>
#include <yjson.h>
#include <httplib.h>
#include <config.h>
#include <zip.h>

#include <format>
#include <vector>
#include <ranges>
#include <regex>

#include <QFile>
#include <QMessageBox>
#include <QTextBrowser>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QApplication>
#include <QProcess>

using namespace std::literals;

namespace fs = std::filesystem;

TabVersion::TabVersion(PluginCenter* parent)
  : QWidget(parent)
  , m_MainLayout(new QHBoxLayout(this))
{
  InitLayout();
  LoadJson();
  Connect();
}

TabVersion::~TabVersion()
{
  // delete m_VersionInfo;
}

void TabVersion::InitLayout()
{
  m_MainLayout->setContentsMargins(0, 0, 0, 0);
  auto background = new QWidget(this);
  m_MainLayout->addWidget(background);
  background->setObjectName("whiteBackground");

  QVBoxLayout* layout1 = new QVBoxLayout(background);
  setContentsMargins(0, 0, 0, 0);
  m_Text = new QTextBrowser(this);
  // m_text->setTextFormat(Qt::RichText);
  // m_text->setTextInteractionFlags(Qt::TextSelectableByMouse);
  m_Text->setOpenExternalLinks(true);
  layout1->addWidget(m_Text);
  m_Text->setReadOnly(true);

  qobject_cast<PluginCenter*>(parent())->AddScrollBar(m_Text->verticalScrollBar());

  QHBoxLayout* layout2 =new QHBoxLayout;
  m_btnWeb = new QPushButton("开源网站", this);
  m_btnBug = new QPushButton("报告问题", this);
  m_btnChk = new QPushButton("检查更新", this);
  layout2->addWidget(m_btnWeb);
  layout2->addWidget(m_btnBug);
  layout2->addWidget(m_btnChk);
  layout1->addLayout(layout2);
}

void TabVersion::LoadJson()
{
  const auto name = 
    "<h2>当前版本</h2>Neobox " NEOBOX_VERSION " " NEOBOX_BUILD_TYPE
    "<br>发布日期：" NEOBOX_BUILD_TIME
    "<br>" NEOBOX_COPYRIGHT ""s;
  m_TextRaw = QString::fromUtf8(name.data(), name.size());
  m_Text->setText(m_TextRaw);
}

void TabVersion::Connect()
{
  connect(m_btnWeb, &QPushButton::clicked, this,
    std::bind(QDesktopServices::openUrl, QUrl(NEOBOX_WEBSITE_URL)));
  connect(m_btnBug, &QPushButton::clicked, this,
    std::bind(QDesktopServices::openUrl, QUrl(NEOBOX_ISSUE_URL)));
  connect(m_btnChk, &QPushButton::clicked, this, &TabVersion::GetUpdate);
}

void TabVersion::GetUpdate()
{
  if (!HttpLib::IsOnline()) {
    QMessageBox::information(this, "提示", "当前没有网络，请稍后再试！");
    return;
  }
  const auto url = u8"" NEOBOX_LATEST_URL ""sv;
  auto res = PluginCenter::m_Instance->DownloadFile(url);
  if (!res) {
    QMessageBox::information(this, "提示", "下载失败，请稍后再试！");
    return;
  }
  // QString qhtml = m_Text->text();
  const YJson jsAboutNew(res->begin(), res->end());
  std::u8string buffer(u8"<h2>最新版本</h2><p style='color: #FF00FF;'>");
  buffer.append(jsAboutNew[u8"name"].getValueString());

  const std::u8string& createDate = jsAboutNew[u8"created_at"].getValueString();

  buffer += u8"<br>发布日期：" + createDate.substr(0, 10) + u8"</p><h3>发行说明</h3>";
  auto html_view = jsAboutNew[u8"body"].getValueString() | std::ranges::views::filter([](char c){return c != '\n' && c != '\r';});
  buffer.append(html_view.begin(), html_view.end());

  auto& array = jsAboutNew[u8"assets"].getArray();
  if (!array.empty()) {
    buffer.append(u8"<h3>下载链接：</h3><ol>");
    for (auto& item: array) {
      auto& name = item[u8"name"].getValueString();
      auto& url = item[u8"browser_download_url"].getValueString();
      buffer += u8"<li><a href='" + url + u8"'>" + name + u8"</a></li>";
    }
    buffer.append(u8"</ol>");
  }

  m_Text->setText(m_TextRaw);
  m_Text->append(QString::fromUtf8(buffer.data(), buffer.size()));

  DoUpgrade(jsAboutNew);
}

// void TabVersion::showEvent(QShowEvent *event)
// {
//   const QScreen* screen = QGuiApplication::primaryScreen();
//   const QRect screenRect = screen->availableGeometry();
//   const QRect rect(screenRect.width() / 3, screenRect.height() / 3,
//       screenRect.width() / 3, screenRect.height() / 3);
//   setGeometry(rect);
// }

std::array<int, 3> TabVersion::ParseVersion(const std::wstring& vStr) {
  std::array<int, 3> version = {0, 0, 0};
  std::wregex pattern(L"^v(\\d+).(\\d+).(\\d+)$");
  std::wsmatch match;
  if (std::regex_match(vStr, match, pattern)) {
    version = { std::stoi(match[1]), std::stoi(match[2]), std::stoi(match[3]) };
  }
  return version;
}

bool TabVersion::DownloadNew(std::u8string_view url) {
  if (!HttpLib::IsOnline()) {
    mgr->ShowMsgbox(u8"失败", u8"请检查网络连接！");
    return false;
  }
  bool result = false;
  const auto dialog = new DownloadingDlg(this);
  dialog->setAttribute(Qt::WA_DeleteOnClose, true);

  fs::path pluginTemp = L"junk";
  fs::create_directory(pluginTemp);
  fs::path pluginDst = pluginTemp;
  pluginTemp /= L"UpdatePack.zip";

  std::ofstream file(pluginTemp, std::ios::binary | std::ios::out);
  if (!file.is_open()) {
    mgr->ShowMsgbox(u8"出错", u8"无法写入文件！");
    return false;
  }

  HttpLib clt(url, true);
  HttpLib::Callback callback = {
    .m_WriteCallback = [&file](auto data, auto size){
      file.write(reinterpret_cast<const char*>(data), size);
    },
    .m_FinishCallback = [dialog, &result, &file, &pluginTemp, &pluginDst](std::wstring msg, const HttpLib::Response* res) {
      file.close();
      if (msg.empty() && res->status == 200) {
        result = true;
        auto temp = pluginTemp.string();
        auto dst = pluginDst.string();
        const auto ret = zip_extract(temp.c_str(), dst.c_str(), nullptr, nullptr);
        if (ret < 0) {
          mgr->ShowMsg(QString("无法解压文件！错误码：%d。").arg(ret));
        }
      } else {
        mgr->ShowMsg("下载压缩包失败！");
      }
      fs::remove(pluginTemp);
      dialog->emitFinished();
    },
    .m_ProcessCallback = [dialog](auto recieve, auto total) {
      dialog->emitProcess(recieve, total);
    },
  };

  clt.GetAsync(callback);

  connect(dialog, &DownloadingDlg::Terminate, std::bind(&HttpLib::ExitAsync, &clt));

  dialog->exec();

  if (file.is_open()) {
    file.close();
    fs::remove(pluginTemp);
  }
  return result;

}

void TabVersion::DoUpgrade(const YJson& data)
{
  auto const vNew = ParseVersion(Utf82WideString(data[u8"tag_name"].getValueString()));
  auto const vOld = ParseVersion(L"" NEOBOX_VERSION);
  if (vNew > vOld) {
    const auto res = QMessageBox::question(this, "提示", "有新版本可用！请确保能流畅访问GitHub，是否立即下载安装？");
    if (res == QMessageBox::Yes) {
      for (auto& asset: data[u8"assets"].getArray()) {
        auto& url = asset[u8"browser_download_url"].getValueString();
        if (!url.ends_with(u8".zip")) {
          continue;
        }
        if (!DownloadNew(url)) {
          mgr->ShowMsg("下载失败！");
          return;
        }
        fs::path dataDir = fs::absolute(L"junk") / L"Neobox";
        if (!fs::exists(dataDir) || !fs::is_directory(dataDir)) {
          mgr->ShowMsg("安装包数据出错！");
          return;
        }
        fs::path curDir = QApplication::applicationDirPath().toStdU16String();
        curDir.make_preferred();
        fs::path appPath = curDir / "neobox.exe";

        std::ofstream file("update.bat", std::ios::out);
        file << "timeout /t 2" << " && "
          << "rd /s /q " << curDir << " && "
          << "move /y " << dataDir << " " << curDir << " && "
          << appPath << std::endl;
        file.close();

        QApplication::quit();
        QProcess::startDetached("cmd.exe", QStringList { "/c", "update.bat" });
        return;
      }
      mgr->ShowMsg("未找到下载链接，请手动下载！");
    }
  } else {
    mgr->ShowMsg("当前已是最新版本！");
  }
}
