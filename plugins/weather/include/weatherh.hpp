#ifndef WEATHERH_HPP
#define WEATHERH_HPP

#include <QScrollArea>
#include <yjson.h>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>

#include <weatherdlg.h>

class WindIcon: public QFrame {
public:
  explicit WindIcon(QWidget* parent, int w, int h)
    : QFrame(parent)
    , x(w/2)
    , y(h/2)
    , w(w)
    , h(h)
  {
    setFixedSize(32, 24);
  }
  virtual ~WindIcon() {}
  void UpdateAngle(int angle) {
    m_Angle = angle;
    update();
  }
private:
  int m_Angle = 0;
  const int x, y, w, h;
  inline static constexpr auto m_PixWidth = 16;
protected:
  void paintEvent(QPaintEvent *) override {
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    QPixmap map(":/icons/Wind360.png");
    painter.translate(x, y);
    painter.rotate(m_Angle + 180);
    painter.drawPixmap(-m_PixWidth / 2, -m_PixWidth / 2, m_PixWidth, m_PixWidth, map);
  }
};

struct WeatherHDetail {
  inline static constexpr auto w = 32;
  QLabel* const tempText;
  QProgressBar* const tempSlider;
  QLabel* const icon;
  WindIcon* const windIcon;
  QLabel* const fxTime;
  std::u8string text;
  std::u8string wind360;
  WeatherHDetail(QWidget* parent, QVBoxLayout* layout, int index)
    : tempText(new QLabel(u8"0℃", parent))
    , tempSlider(new QProgressBar(parent))
    , icon(new QLabel(parent))
    , windIcon(new WindIcon(parent, w, 24))
    , fxTime(new QLabel(QStringLiteral("%1:00").arg(index), parent))
  {
    icon->setObjectName("IconHour");
    icon->setFixedSize(w, w);
    icon->setText(QChar(WeatherDlg::m_FontsMap[u8"999"]));
    icon->setAlignment(Qt::AlignCenter);
    windIcon->setFixedSize(w, 24);
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
  void ParseJSON(const YJson& data, int min, int max) {
    auto& fxTime = data[u8"fxTime"].getValueString();
    auto temp = data[u8"temp"].getValueInt();
    auto& icon = data[u8"icon"].getValueString();
    auto& text = data[u8"text"].getValueString();
    auto& wind360 = data[u8"wind360"].getValueString();

    auto pos = fxTime.data() + 11;
    auto time = QString::fromUtf8(pos, 5);
    this->fxTime->setText(time);
    tempText->setText(QStringLiteral("%1℃").arg(temp));
    tempSlider->setRange(min, max);
    tempSlider->setValue(temp);
    this->icon->setText(QChar(WeatherDlg::m_FontsMap[icon]));
    this->text = text;

    windIcon->UpdateAngle(std::stoi(std::string(wind360.begin(), wind360.end())));
  }
};

class WeatherH: public QScrollArea
{
  Q_OBJECT

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;
public:
  explicit WeatherH(QWidget* parent);
  virtual ~WeatherH();
  void SetJSON(const YJson& json);
private:
  std::vector<WeatherHDetail> m_Data;
private:
  void SetupUi();
};

#endif