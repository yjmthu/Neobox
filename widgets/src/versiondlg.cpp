#include <versiondlg.h>
#include <yjson.h>
#include <httplib.h>
#include <varbox.h>
#include <config.h>

#include <format>
#include <vector>
#include <ranges>

#include <QFile>
#include <QMessageBox>
#include <QGuiApplication>
#include <QScreen>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDesktopServices>

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

  constexpr int YearValue = (__DATE__[7] * 1000 + __DATE__[8] * 100 + __DATE__[9] * 10 + __DATE__[10]) - '0' * 1111;

  constexpr int DateValue = (__DATE__[4] * 10 + __DATE__[5]) - '0' * 11;

  constexpr const char szMonths[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

  constexpr int MonthValue = Month<szMonths, 0>::Value();
}

VersionDlg::VersionDlg():
  QDialog(nullptr)
{
  setWindowTitle("关于 Neobox");
  QVBoxLayout* layout1 = new QVBoxLayout(this);
  setContentsMargins(9, 9, 9, 9);
  m_text = new QLabel(this);
  m_text->setTextFormat(Qt::RichText);
  // m_text->setTextInteractionFlags(Qt::TextSelectableByMouse);
  m_text->setOpenExternalLinks(true);
  layout1->addWidget(m_text);
  QHBoxLayout* layout2 =new QHBoxLayout;
  m_btnWeb = new QPushButton("开源网站", this);
  m_btnChk = new QPushButton("检查更新", this);
  m_btnCls = new QPushButton("退出", this);
  layout2->addWidget(m_btnWeb);
  layout2->addWidget(m_btnChk);
  layout2->addWidget(m_btnCls);
  layout1->addLayout(layout2);
  LoadJson();
  Connect();
}

VersionDlg::~VersionDlg()
{
  // delete m_VersionInfo;
}


void VersionDlg::LoadJson()
{
  const auto name = std::format("<h2>当前版本</h2>Neobox {}.{} {} <br> 发布日期：{:04d}-{:02d}-{:02d}<br>",
    NEOBOX_VERSION_MAJOR, NEOBOX_VERSION_MINOR, NEOBOX_BETA,
    NeoGetDateTime::YearValue, NeoGetDateTime::MonthValue, NeoGetDateTime::DateValue);
  m_text->setText(QString::fromUtf8(name.data(), name.size()) + NEOBOX_COPYRIGHT);
}

void VersionDlg::Connect()
{
  connect(m_btnCls, &QPushButton::clicked, this, &QWidget::close);
  connect(m_btnWeb, &QPushButton::clicked, this,
    std::bind(QDesktopServices::openUrl, QUrl(NEOBOX_WEBSITE)));
  connect(m_btnChk, &QPushButton::clicked, this, &VersionDlg::GetUpdate);
}

void VersionDlg::GetUpdate()
{
  if (!HttpLib::IsOnline()) {
    QMessageBox::information(this, "提示", "当前没有网络，请稍后再试！");
    return;
  }
  HttpLib clt(std::string(NEOBOX_LATEST));
  clt.SetHeader("User-Agent", "Libcurl in Neobox App/1.0");
  auto res = clt.Get();
  if (200 != res->status)
  {
    QMessageBox::information(this, "提示", "获取信息失败，请稍后再试！");
    return;
  }

  m_btnChk->setEnabled(false);
  QString qhtml = m_text->text();
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

  qhtml += QString::fromUtf8(buffer.data(), buffer.size());
  m_text->setText(qhtml);
}

void VersionDlg::showEvent(QShowEvent *event)
{
  const QScreen* screen = QGuiApplication::primaryScreen();
  const QRect screenRect = screen->availableGeometry();
  const QRect rect(screenRect.width() / 3, screenRect.height() / 3,
      screenRect.width() / 3, screenRect.height() / 3);
  setGeometry(rect);
}
