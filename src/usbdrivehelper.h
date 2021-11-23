#ifndef USBDRIVEHELPER_H
#define USBDRIVEHELPER_H

#include <QDialog>
#include <Windows.h>

#include <vector>
#include "speedwidget.h"

namespace Ui {
class USBdriveHelper;
}

class USBdriveHelper : public SpeedWidget<QDialog>
{
    Q_OBJECT
protected:
    void enterEvent(QEnterEvent* event);
    void leaveEvent(QEvent* event);
    void showEvent(QShowEvent* event);
    void closeEvent(QCloseEvent *event);

public:
    explicit USBdriveHelper(char U, QWidget *parent = nullptr);
    ~USBdriveHelper();

private:
    Ui::USBdriveHelper *ui;
    std::vector<TCHAR*> pans;
    QWidget* widget;
};

#endif // USBDRIVEHELPER_H
