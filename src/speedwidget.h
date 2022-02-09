#ifndef SPEEDWIDGET_H
#define SPEEDWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QFrame>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>

#include "blankform.h"
#include "gmpoperatetip.h"

template <class Parent = QWidget>
class SpeedWidget : public Parent
{
protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    template<typename A, typename B, typename C>
    void initSpeedBox(A frame, B left, C right, bool d=true)
    {
        if (d)
        {
            QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(this);
            effect->setOffset(0, 0);          //具体阴影
            effect->setBlurRadius(15);        //阴影半径
            effect->setColor(Qt::black);      //阴影颜色
            frame->setGraphicsEffect(effect);
            jobTip = new GMPOperateTip(this);
        }
        blank = new BlankFrom(static_cast<QWidget*>(this));
        QObject::connect(blank->closeButton, &QPushButton::clicked, this, right);
        QObject::connect(blank->minButton, &QPushButton::clicked, this, left);
        blank->move(static_cast<Parent*>(this)->width()-100, 0);
    }
    GMPOperateTip* jobTip = nullptr;
    BlankFrom *blank = nullptr;

public:
    SpeedWidget(QWidget *parent = nullptr);
    ~SpeedWidget();

private:
    QPoint m_ptPress;
};

template <class Parent>
SpeedWidget<Parent>::SpeedWidget(QWidget *parent) : Parent(parent)
{
    static_cast<Parent*>(this)->setAttribute(Qt::WA_TranslucentBackground);
}

template <class Parent>
SpeedWidget<Parent>::~SpeedWidget()
{
    delete jobTip;
}

template <class Parent>
void SpeedWidget<Parent>::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)                   // 鼠标左键点击悬浮窗
    {
        static_cast<Parent*>(this)->setMouseTracking(true);                              // 开始跟踪鼠标位置，从而使悬浮窗跟着鼠标一起移动。
        m_ptPress = event->pos();                            // 记录开始位置
    }
    event->accept();
}

template <class Parent>
void SpeedWidget<Parent>::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)  // 鼠标左键释放
    {
        static_cast<Parent*>(this)->setMouseTracking(false);           // 停止跟踪鼠标。
    }
    event->accept();
}

template <class Parent>
void SpeedWidget<Parent>::mouseMoveEvent(QMouseEvent* event)
{
    static_cast<Parent*>(this)->move(event->globalPos() - m_ptPress);               //当前位置加上位置变化情况，从而实现悬浮窗和鼠标同时移动。
    event->accept();
}


#endif // SPEEDWIDGET_H
