#ifndef BINGSETTING_H
#define BINGSETTING_H

#include <QDialog>
#include "speedwidget.h"

namespace Ui {
    class BingSetting;
}

class BingSetting : public SpeedWidget<QDialog>
{
    Q_OBJECT
protected:
    void closeEvent(QCloseEvent*);

public:
    BingSetting();
    ~BingSetting();

private:
    Ui::BingSetting *ui;

private slots:
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
};

#endif // BINGSETTING_H
