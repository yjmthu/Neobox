#include <itemcolor.hpp>

#include <QGraphicsDropShadowEffect>
#include <QVBoxLayout>

ItemColor::ItemColor(const QColor& color, QWidget* parent)
  : QWidget(parent)
  , m_CenterWidget(new QWidget(this))
{
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(20, 10, 20, 10);
  // m_CenterWidget->setFixedSize(36, 36);
  layout->addWidget(m_CenterWidget);
  setStyleSheet("border-radius: 0px;");
  SetColor(color);

  auto const effect = new QGraphicsDropShadowEffect(this);
  effect->setOffset(2, 2);
  effect->setColor(QColor(40, 40, 40));
  effect->setBlurRadius(5);
  m_CenterWidget->setGraphicsEffect(effect);
}

ItemColor::~ItemColor()
{
}

void ItemColor::SetColor(const QColor& color)
{
  m_Color = color;
  m_CenterWidget->setStyleSheet(QStringLiteral("border-radius: 7px;background-color: %1;").arg(color.name(QColor::HexRgb)));
}
