#ifndef WEATHERH_HPP
#define WEATHERH_HPP

#include <QWidget>
#include <yjson.h>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>

#include <weatherdlg.h>

struct WeatherHDetail {
  QLabel* const tempText;
  QProgressBar* const tempSlider;
  QLabel* const icon;
  QLabel* const windIcon;
  QLabel* const fxTime;
  std::u8string text;
  std::u8string wind360;
  WeatherHDetail(QWidget* parent, QVBoxLayout* layout, int index)
    : tempText(new QLabel(u8"0℃", parent))
    , tempSlider(new QProgressBar(parent))
    , icon(new QLabel(parent))
    , windIcon(new QLabel(parent))
    , fxTime(new QLabel(QStringLiteral("%1:00").arg(index), parent))
  {
    constexpr auto w = 32;
    icon->setObjectName("IconHour");
    icon->setFixedSize(w, w);
    icon->setText(QChar(WeatherDlg::m_FontsMap[u8"999"]));
    icon->setAlignment(Qt::AlignCenter);
    windIcon->setFixedSize(w, 24);
    windIcon->setText("W");
    windIcon->setAlignment(Qt::AlignCenter);
    tempSlider->setOrientation(Qt::Orientation::Vertical);
    tempSlider->setRange(-50, 50);
    tempSlider->setValue(0);
    tempSlider->setFixedSize(6, 50);
    tempSlider->setTextVisible(false);
    tempText->setFixedWidth(w);
    tempText->setAlignment(Qt::AlignCenter);
    fxTime->setFixedWidth(w);
    fxTime->setAlignment(Qt::AlignCenter);

    layout->addWidget(tempText);
    layout->addWidget(tempSlider);
    // layout->addWidget(tempSlider);
    auto const l = new QHBoxLayout;
    l->setContentsMargins(13, 0, 13, 0);
    l->addWidget(tempSlider);
    layout->addLayout(l);
    layout->addWidget(icon);
    layout->addWidget(windIcon);
    layout->addWidget(fxTime);
  }
  void ParseJSON(const YJson& data) {
    auto& fxTime = data[u8"fxTime"].getValueString();
    auto& temp = data[u8"temp"].getValueString();
    auto& icon = data[u8"icon"].getValueString();
    auto& text = data[u8"text"].getValueString();
    auto& wind360 = data[u8"wind360"].getValueString();

    auto pos = fxTime.data() + 11;
    auto time = QString::fromUtf8(pos, 5);
    this->fxTime->setText(time);
    auto qTemp = QString::fromUtf8(temp.data(), temp.size());
    auto iTemp = qTemp.toInt();
    tempText->setText(QStringLiteral("%1℃").arg(qTemp));
    tempSlider->setValue(iTemp);
    this->icon->setText(QChar(WeatherDlg::m_FontsMap[icon]));
    this->text = text;
  }
};

class WeatherH: public QWidget
{
  Q_OBJECT

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;
public:
  explicit WeatherH(QWidget* parent);
  virtual ~WeatherH();
  void SetJSON(const YJson& json);
  class QScrollArea* const m_ScrollArea;
  QWidget* const m_ScrollWidget;
private:
  std::vector<WeatherHDetail> m_Data;
private:
  void SetupUi();
};

#endif