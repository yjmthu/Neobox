#ifndef BLANKFORM_H
#define BLANKFORM_H

#include <QWidget>
class QPushButton;

// 无边框窗口右上角的关闭和最小化按钮
class BlankFrom: public QWidget
{
    Q_OBJECT
protected:
#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
    void enterEvent(QEvent *event);           // 鼠标进入后改变图标
#else
    void enterEvent(QEnterEvent *event);      // 鼠标进入后改变图标
#endif
    void leaveEvent(QEvent *event);           // 鼠标移出后改变图标
public:
    explicit BlankFrom(QWidget* p);           // 必须指定父窗口
    ~BlankFrom();
    QPushButton *closeButton {nullptr};       // 关闭按钮
    QPushButton *minButton   {nullptr};       // 最小化按钮
};


#endif // BLANKFORM_H
