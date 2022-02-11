#ifndef ROUNDCLOCK_H
#define ROUNDCLOCK_H

#include <QWidget>

class RoundClock : public QWidget
{
    Q_OBJECT
protected:
    void paintEvent(QPaintEvent *event);
public:
    explicit RoundClock(QWidget *parent = nullptr);
private:
    QRectF textRectF(double radius, int pointSize, double angle);

signals:

};

#endif // ROUNDCLOCK_H
