#ifndef FORMSETTING_H
#define FORMSETTING_H

#include "speedwidget.h"

class GMPOperateTip;
class QColorDialog;
class YJson;

namespace Ui {
class FormSetting;
}

enum class ACCENT_STATE;

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
    void pushButton_clicked();
    void horizontalSlider_valueChanged(int value);
    void on_pushButton_2_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_4_clicked();

private:
    Ui::FormSetting *ui;
    QColor new_color;
    YJson *old_style = nullptr;
    static ACCENT_STATE win_style, temp_win_style;
    void initConnects();
};

#endif // FORMSETTING_H
