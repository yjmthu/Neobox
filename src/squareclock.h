#ifndef SQUARECLOCK_H
#define SQUARECLOCK_H

#include <QWidget>

class SquareClock : public QWidget
{
    Q_OBJECT
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
public:
    explicit SquareClock(QWidget *parent = nullptr);
private:
    void setupUi();
    QPoint m_ptPress;
signals:

};

#endif // SQUARECLOCK_H
