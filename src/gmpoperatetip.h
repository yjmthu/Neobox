#ifndef GMPOPERATETIP_H
#define GMPOPERATETIP_H

#include <QWidget>
class QGraphicsOpacityEffect;
class QLabel;
class GMPOperateTip;
class QPropertyAnimation;

class GMPOperateTip: public QWidget
{
    Q_OBJECT
public:
    explicit GMPOperateTip(QWidget* parent = nullptr);
    ~GMPOperateTip();
    void showTip(QString);
private:
    QPoint centerPos;
    QGraphicsOpacityEffect *m_pOpacity;
    QPropertyAnimation *m_pAnimation;
    QLabel *tip;
};
#endif // GMPOPERATETIP_H
