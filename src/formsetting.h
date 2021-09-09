#ifndef FORMSETTING_H
#define FORMSETTING_H

#include "speedwidget.h"

class GMPOperateTip;
class QColorDialog;
class YJsonItem;

namespace Ui {
class FormSetting;
}

class FormSetting : public SpeedWidget<QDialog>
{
    Q_OBJECT
protected:
    void closeEvent(QCloseEvent*);
    bool eventFilter(QObject* target, QEvent* event);

public:
    explicit FormSetting(QDialog *parent = nullptr);
    ~FormSetting();
    static void load_style_from_file();

private slots:
    void on_pushButton_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_pushButton_2_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_6_clicked();

private:
    Ui::FormSetting *ui;
    QColor new_color;
    YJsonItem *old_style = nullptr;
};

#endif // FORMSETTING_H
