#ifndef USBDRIVEHELPER_H
#define USBDRIVEHELPER_H

#include <QDialog>

#include <vector>
#include "speedwidget.h"

namespace Ui {
class USBdriveHelper;
}

class USBdriveHelper : public SpeedWidget<QDialog>
{
    Q_OBJECT
signals:
    void appQuit();
protected:
#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
          void enterEvent(QEvent* event);
#else
          void enterEvent(QEnterEvent* event);
#endif
    void leaveEvent(QEvent* event);
    void showEvent(QShowEvent* event);
    void closeEvent(QCloseEvent *event);

public:
    explicit USBdriveHelper(char U, QWidget *parent = nullptr);
    ~USBdriveHelper();

private:
    Ui::USBdriveHelper *ui;
    std::vector<char*> pans;
    QWidget* widget;
};

#endif // USBDRIVEHELPER_H
