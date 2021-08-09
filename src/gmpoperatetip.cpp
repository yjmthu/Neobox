#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QTimer>

#include "gmpoperatetip.h"

GMPOperateTip::GMPOperateTip(QWidget* parent):
    QWidget(parent)
{
    tip = new QLabel(this);
    centerPos = parent->rect().center();

    m_pAnimation = new QPropertyAnimation(this);
    m_pAnimation->setTargetObject(this);

    m_pOpacity = new QGraphicsOpacityEffect(this);
    m_pOpacity->setOpacity(1);
    setGraphicsEffect(m_pOpacity);
    m_pAnimation->setTargetObject(m_pOpacity);

    m_pAnimation->setPropertyName("opacity");
    m_pAnimation->setStartValue(1);
    m_pAnimation->setEndValue(0);

    m_pAnimation->setDuration(150);
    connect(m_pAnimation, &QPropertyAnimation::finished, this, [=](){setVisible(false); m_pOpacity->setOpacity(1);});
    setStyleSheet("QWidget{background-color: transparent;}QLabel{background-color: #4cd05c;border-radius: 3px; padding: 6px;}");
    setVisible(false);
}

GMPOperateTip::~GMPOperateTip()
{
    delete m_pOpacity;
    delete m_pAnimation;
    delete  tip;
}


void GMPOperateTip::showTip(QString str = "写了些啥", int time)
{
    tip->setText(str);
    tip->adjustSize();
    setGeometry(centerPos.x()-tip->width()/2, centerPos.y()-tip->height()/2, tip->width(), tip->height());
    setVisible(true);
    QTimer::singleShot((time?time:500), m_pAnimation, SLOT(start()));
}
