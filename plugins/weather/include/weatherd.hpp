#ifndef WEATHERD_HPP
#define WEATHERD_HPP

#include <yjson.h>
#include <weatherdlg.h>

#include <QScrollArea>
#include <QLabel>
#include <QHBoxLayout>
#include <QLayout>
#include <QDateTime>

struct WeatherDDetail {
  QLabel* const m_Date;
  QLabel* const m_Text;
  QLabel* const m_Icon;
  QWidget* const m_TempFrame;
  QFrame* const m_Temp;
  QLabel* const m_TempLabel;
  inline static constexpr auto w = 32;
  inline static QString m_StrWeeksEng = "SunMonTueWedThuFriSat";
  inline static QString m_StrWeeksChi = "日一二三四五六";
  WeatherDDetail(QWidget* parent, int index)
    : m_Date(new QLabel(parent))
    , m_Text(new QLabel(parent))
    , m_Icon(new QLabel(parent))
    , m_TempFrame(new QWidget(parent))
    , m_Temp(new QFrame(m_TempFrame))
    , m_TempLabel(new QLabel(parent))
  {
    SetupObjectName();
    SetupSize();
    parent->setFixedHeight(w);
    auto const layout = new QHBoxLayout(parent);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(16);
    layout->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_Date);
    layout->addWidget(m_Text);
    layout->addWidget(m_Icon);
    layout->addWidget(m_TempFrame);
    layout->addWidget(m_TempLabel);
    m_Temp->setStyleSheet("background-color: qlineargradient("
      "x1:0, y1:0, x2:1, y2:0,"
      "stop:0 #427bff,"
      "stop:1 #f18360"
    ");");
  }
  void SetupObjectName() {
    m_Temp->setObjectName("TempRangeBar");
    m_Icon->setObjectName("IconDay");
  }
  void SetupSize() {
    m_Date->setFixedSize(78, w);
    m_Text->setFixedSize(40, w);
    m_Icon->setFixedSize(w, w);
    m_TempFrame->setFixedSize(180, w);
    m_TempLabel->setFixedSize(50, w);
    m_Temp->setFixedSize(m_TempFrame->width(), 6);
    m_Temp->move(0, 13);
  }
  void ParseJSON(const YJson& data, int min, int max)
  {
    auto tempMax = data[u8"tempMax"].getValueInt();
    auto tempMin = data[u8"tempMin"].getValueInt();
    auto& fxDate = data[u8"fxDate"].getValueString();
    auto& iconDay = data[u8"iconDay"].getValueString();
    auto& textDay = data[u8"textDay"].getValueString();

    auto&& date = QDateTime::fromString(QString::fromUtf8(fxDate.data(), fxDate.size()), "yyyy-MM-dd");

    auto week = date.toString("ddd");
    auto weekIndex = m_StrWeeksEng.indexOf(week) / 3;
    m_Date->setText(date.toString("M月d日 周%1").arg(m_StrWeeksChi.at(weekIndex)));
    m_Text->setText(QString::fromUtf8(textDay.data(), textDay.size()));
    m_Icon->setText(QChar(WeatherDlg::m_FontsMap[iconDay]));
    float range = max - min;
    if (range == 0.0) {
      range = 1;
    }
    auto l = (tempMin - min) / range * m_TempFrame->width();
    auto r = (tempMax - min) / range * m_TempFrame->width();
    m_Temp->move(static_cast<int>(l), 13);
    m_Temp->setFixedWidth(static_cast<int>(r - l));
    m_TempLabel->setText(QStringLiteral("%1°~%2°").arg(tempMin).arg(tempMax));
  }
};

class WeatherD: public QScrollArea
{
public:
  explicit WeatherD(QWidget* parent);
  virtual ~WeatherD();
  void SetJSON(YJson& json);
private:
  std::vector<WeatherDDetail> m_Data;
private:
  void SetupUi();
};

#endif // WEATHERD_HPP