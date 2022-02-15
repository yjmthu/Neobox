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
    minButton->setToolTip(QStringLiteral("最小化"));
    closeButton->setToolTip(QStringLiteral("关闭"));
    minButton->setCursor(Qt::PointingHandCursor);
    closeButton->setCursor(Qt::PointingHandCursor);
    minButton->setStyleSheet(QStringLiteral("QPushButton{background-color:#85c43b;border-radius:7px;}"));
    closeButton->setStyleSheet(QStringLiteral("QPushButton{background-color:#ea6e4d;border-radius:7px;}"));
    // setStyleSheet("QToolTip{border:1px solid rgb(118, 118, 118); background-color: #ffffff; color:#484848; font-size:12px;}");
}

BlankFrom::~BlankFrom()
{
    qDebug("析构BlankForm");
}

#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
void BlankFrom::enterEvent(QEvent * e)
#else
void BlankFrom::enterEvent(QEnterEvent *e)
#endif
{
    closeButton->setStyleSheet(QStringLiteral("QPushButton{border-image: url(:/icons/close.png);border-radius:7px;}"));
    minButton->setStyleSheet(QStringLiteral("QPushButton{border-image: url(:/icons/minimize.png);border-radius:7px;}"));
    e->accept();
}

void BlankFrom::leaveEvent(QEvent *e)
{
    minButton->setStyleSheet(QStringLiteral("QPushButton{background-color:#85c43b;border-radius:7px;}"));
    closeButton->setStyleSheet(QStringLiteral("QPushButton{background-color:#ea6e4d;border-radius:7px;}"));
    e->accept();
}
