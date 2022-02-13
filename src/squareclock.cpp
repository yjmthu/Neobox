#include "squareclock.h"

#include <QFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <functional>
#include <algorithm>

#include "funcbox.h"
#include "windowposition.h"

void SquareClock::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setMouseTracking(true);
        m_ptPress = event->pos();
    }
    event->accept();
}

void SquareClock::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setMouseTracking(false);
    }
    VarBox->m_pWindowPosition->m_squareClockPos = this->pos();
    VarBox->m_pWindowPosition->toFile();
    event->accept();
}

void SquareClock::mouseMoveEvent(QMouseEvent *event)
{
    move(event->globalPos() - m_ptPress);
    event->accept();
}

void SquareClock::enterEvent(QEvent *event)
{
    event->accept();
}

void SquareClock::leaveEvent(QEvent *event)
{
    event->accept();
}

SquareClock::SquareClock(QWidget *parent)
    : QWidget { parent }
{
    setupUi();
}

void SquareClock::setupUi()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    QFrame *frame { new QFrame(this) };
    QVBoxLayout *backLayout { new QVBoxLayout(this) };
    backLayout->addWidget(frame);
    QHBoxLayout *layout { new QHBoxLayout(frame) };
    QLabel *labelTime { new QLabel(frame) };
    layout->addWidget(labelTime);
    frame->setStyleSheet(QStringLiteral("QFrame{"
                                        "background-color:white;"
                                        "border-radius:5px;"
                                        "}"
                                        "QLabel{"
                                        // "font-family:楷体;"
                                        "font-size:20pt;"
                                        "color:black;"
                                        "}"));
    QTimer *timer { new QTimer(this) };
    timer->setInterval(500);
    std::function<void(void)> pFuncChangeText([=](){ labelTime->setText(QTime::currentTime().toString("hh:mm:ss")); });
    pFuncChangeText();
    connect(timer, &QTimer::timeout, this, pFuncChangeText);
    timer->start();
    if (VarBox->m_pWindowPosition->m_squareClockPos != QPoint(0, 0)) {
        move(VarBox->m_pWindowPosition->m_squareClockPos);
    }
}
