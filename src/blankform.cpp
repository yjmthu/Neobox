#include <QPushButton>
#include "dialog.h"
#include "blankform.h"


BlankFrom::BlankFrom(QWidget* p):
    QWidget(p)
{
    setMaximumSize(100, 40);
    setMinimumSize(100, 40);
    closeButton = new QPushButton(this);
    minButton = new QPushButton(this);
    minButton->setGeometry(width()-75,12,14,14);
    closeButton->setGeometry(width()-40,12,14,14);
    minButton->setToolTip(tr("最小化"));
    closeButton->setToolTip(tr("关闭"));
    minButton->setCursor(Qt::PointingHandCursor);
    closeButton->setCursor(Qt::PointingHandCursor);
    minButton->setStyleSheet("QPushButton{background-color:#85c43b;border-radius:7px;}");
    closeButton->setStyleSheet("QPushButton{background-color:#ea6e4d;border-radius:7px;}");
    //setStyleSheet("QToolTip{border:1px solid rgb(118, 118, 118); background-color: #ffffff; color:#484848; font-size:12px;}");
}

BlankFrom::~BlankFrom()
{
    qDebug("析构BlankForm");
}

void BlankFrom::enterEvent(QEnterEvent *)
{
    closeButton->setStyleSheet("QPushButton{border-image: url(:/icons/close.png);border-radius:7px;}");
    minButton->setStyleSheet("QPushButton{border-image: url(:/icons/minimize.png);border-radius:7px;}");
}

void BlankFrom::leaveEvent(QEvent *)
{
    minButton->setStyleSheet("QPushButton{background-color:#85c43b;border-radius:7px;}");
    closeButton->setStyleSheet("QPushButton{background-color:#ea6e4d;border-radius:7px;}");
}
