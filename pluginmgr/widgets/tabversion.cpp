#include "tabversion.hpp"

#include <yjson.h>
#include <httplib.h>
#include <config.h>

#include <format>
#include <vector>
#include <ranges>

#include <QFile>
#include <QMessageBox>
#include <QTextBrowser>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDesktopServices>

#if 0
namespace NeoGetDateTime {
  template<const char* _Str, int _Index>
  struct Month {
    static constexpr int Value() {
      return (Cmp() ? _Index + 1: Month<_Str, _Index + 1>::Value());
    }
    static constexpr bool Cmp() {
      constexpr int i = _Index * 3;
      return _Str[i] == __DATE__[0] && _Str[i+1] == __DATE__[1] && _Str[i+2] == __DATE__[2];
    }
  };
  template<const char* _Str>
  struct Month<_Str, 11> {
    static constexpr int Value() {
      return 12;
    }
  };

  constexpr int GetValue(int i) { return __DATE__[i] == ' ' ? 0 : __DATE__[i] - '0'; }

  constexpr int YearValue = GetValue(7) * 1000 + GetValue(8) * 100 + GetValue(9) * 10 + GetValue(10);

  constexpr int DateValue = GetValue(4) * 10 + GetValue(5);

  constexpr const char szMonths[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

  constexpr int MonthValue = Month<szMonths, 0>::Value();
}
#endif

TabVersion::TabVersion(QWidget* parent)
  : QWidget(nullptr)
{
  QVBoxLayout* layout1 = new QVBoxLayout(this);
  setContentsMargins(0, 0, 0, 0);
  m_Text = new QTextBrowser(this);
  // m_text->setTextFormat(Qt::RichText);
  // m_text->setTextInteractionFlags(Qt::TextSelectableByMouse);
  m_Text->setOpenExternalLinks(true);
  layout1->addWidget(m_Text);
  m_Text->setReadOnly(true);

  QHBoxLayout* layout2 =new QHBoxLayout;
  m_btnWeb = new QPushButton("开源网站", this);
  m_btnBug = new QPushButton("报告问题", this);
  m_btnChk = new QPushButton("检查更新", this);
  layout2->addWidget(m_btnWeb);
  layout2->addWidget(m_btnBug);
  layout2->addWidget(m_btnChk);
  layout1->addLayout(layout2);
  LoadJson();
  Connect();
}

TabVersion::~TabVersion()
{
  // delete m_VersionInfo;
}


void TabVersion::LoadJson()
{
  const auto name = std::format("<h2>当前版本</h2>Neobox {} {} <br> 发布日期：{}<br>{}",
    NEOBOX_VERSION, NEOBOX_BUILD_TYPE, NEOBOX_BUILD_TIME, NEOBOX_COPYRIGHT);
  m_Text->setText(QString::fromUtf8(name.data(), name.size()));
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
  HttpLib clt(std::string(NEOBOX_LATEST_URL));
  clt.SetHeader("User-Agent", "Libcurl in Neobox App/1.0");
  auto res = clt.Get();
  if (200 != res->status)
  {
    QMessageBox::information(this, "提示", "获取信息失败，请稍后再试！");
    return;
  }

  m_btnChk->setEnabled(false);
  // QString qhtml = m_Text->text();
  const YJson jsAboutNew(res->body.begin(), res->body.end());
  std::u8string buffer(u8"<h2>最新版本</h2><p style='color: #FF00FF;'>");
  buffer.append(jsAboutNew[u8"name"].getValueString());

  const std::u8string& createDate = jsAboutNew[u8"created_at"].getValueString();

  buffer += u8"<br>发布日期：" + createDate.substr(0, 10) + u8"</p><h3>发行说明</h3>";
  auto html_view = jsAboutNew[u8"body"].getValueString() | std::ranges::views::filter([](char c){return c != '\n' && c != '\r';});
  buffer.append(html_view.begin(), html_view.end());

  auto& array = jsAboutNew[u8"assets"].getArray();
  if (!array.empty()) {
    buffer.append(u8"<h3>下载链接：</h3><ol>");
    for (auto& item: array)
    {
      auto& name = item[u8"name"].getValueString();
      auto& url = item[u8"browser_download_url"].getValueString();
      buffer += u8"<li><a href='" + url + u8"'>" + name + u8"</a></li>";
    }
    buffer.append(u8"</ol>");
  }

  m_Text->append(QString::fromUtf8(buffer.data(), buffer.size()));
}

// void TabVersion::showEvent(QShowEvent *event)
// {
//   const QScreen* screen = QGuiApplication::primaryScreen();
//   const QRect screenRect = screen->availableGeometry();
//   const QRect rect(screenRect.width() / 3, screenRect.height() / 3,
//       screenRect.width() / 3, screenRect.height() / 3);
//   setGeometry(rect);
// }
