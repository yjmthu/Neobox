#include "roundclock.h"

#include <QPainter>
#include <QTime>
#include <QTimer>

#include <cmath>

void RoundClock::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // 时针、分针、秒针位置 - 多边形
    static const QPoint hourHand[4] = {
        QPoint(3, 5),
        QPoint(0, 13),
        QPoint(-3, 5),
        QPoint(0, -40)
    };
    static const QPoint minuteHand[4] = {
        QPoint(3, 5),
        QPoint(0, 16),
        QPoint(-3, 5),
        QPoint(0, -70)
    };

    static const QPoint secondHand[4] = {
        QPoint(3, 5),
        QPoint(0, 18),
        QPoint(-3, 5),
        QPoint(0, -90)
    };

    // 时针、分针、秒针颜色
    const QColor hourColor(200, 100, 0, 200);
    const QColor minuteColor(0, 127, 127, 150);
    const QColor secondColor(0, 160, 230, 150);

    int side = std::min(width(), height());
    const QTime& time = QTime::currentTime();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    // 平移坐标系原点至中心点
    painter.translate(width() / 2, height() / 2);
    // 缩放
    painter.scale(side / 200.0, side / 200.0);

    // 绘制时针
    painter.setPen(Qt::NoPen);
    painter.setBrush(hourColor);

    painter.save();
    // 每圈360° = 12h 即：旋转角度 = 小时数 * 30°
    painter.rotate(30.0 * ((time.hour() + time.minute() / 60.0)));
    painter.drawConvexPolygon(hourHand, 4);
    painter.restore();

    painter.setPen(hourColor);

    // 绘制小时线 （360度 / 12 = 30度）
    for (int i = 0; i < 12; ++i) {
        painter.drawLine(88, 0, 96, 0);
        painter.rotate(30.0);
    }

    int radius = 100;
    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);
    int pointSize = font.pointSize();

    // 绘制小时文本
    int nHour = 0;
    for (int i = 0; i < 12; ++i) {
        nHour = i + 3;
        if (nHour > 12)
            nHour -= 12;
        painter.drawText(textRectF(radius*0.8, pointSize, i * 30), Qt::AlignCenter, QString::number(nHour));
    }

    // 绘制分针
    painter.setPen(Qt::NoPen);
    painter.setBrush(minuteColor);

    painter.save();
    // 每圈360° = 60m 即：旋转角度 = 分钟数 * 6°
    painter.rotate(6.0 * (time.minute() + time.second() / 60.0));
    painter.drawConvexPolygon(minuteHand, 4);
    painter.restore();

    painter.setPen(minuteColor);

    // 绘制分钟线 （360度 / 60 = 6度）
    for (int j = 0; j < 60; ++j) {
        if ((j % 5) != 0)
            painter.drawLine(92, 0, 96, 0);
        painter.rotate(6.0);
    }

    // 绘制秒针
    painter.setPen(Qt::NoPen);
    painter.setBrush(secondColor);

    painter.save();
    // 每圈360° = 60s 即：旋转角度 = 秒数 * 6°
    painter.rotate(6.0 * time.second());
    painter.drawConvexPolygon(secondHand, 4);
    painter.restore();
}

RoundClock::RoundClock(QWidget *parent)
    : QWidget{parent}
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnBottomHint);
    setAttribute(Qt::WA_TranslucentBackground);
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this](){this->update();});
    timer->start(1000);
}

QRectF RoundClock::textRectF(double radius, int pointSize, double angle)
{
    // constexpr double M_PI = 3.141592653589793;
    QRectF rectF;
    rectF.setX(radius*cos(angle*M_PI/180.0) - pointSize*2);
    rectF.setY(radius*sin(angle*M_PI/180.0) - pointSize/2.0);
    rectF.setWidth(pointSize*4);
    rectF.setHeight(pointSize);
    return rectF;
}
