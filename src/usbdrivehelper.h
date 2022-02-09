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
    void appQuit(unsigned index);
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
    USBdriveHelper(char U, unsigned index, std::vector<USBdriveHelper*>* vectorPtr, QWidget *parent = nullptr);
    ~USBdriveHelper();
    unsigned m_index;
    std::vector<USBdriveHelper*>* m_vectorPtr { nullptr };
    std::string diskId;

private:
    Ui::USBdriveHelper *ui;
    QWidget* widget;
};

#endif // USBDRIVEHELPER_H
