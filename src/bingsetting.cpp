#include <QSettings>
#include <QGraphicsDropShadowEffect>
#include <QFile>

#include "funcbox.h"
#include "form.h"
#include "bingsetting.h"
#include "blankform.h"
#include "gmpoperatetip.h"
#include "ui_bingsetting.h"

BingSetting::BingSetting():
    SpeedWidget<QDialog>(),
    ui(new Ui::BingSetting)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    QFile qss(":/qss/bing_style.qss");
    qss.open(QFile::ReadOnly);
    setStyleSheet(QString(qss.readAll()));
    qss.close();
    ui->setupUi(this);
    initSpeedBox(ui->frame, &BingSetting::showMinimized, &BingSetting::close);

    if (VarBox->UseDateAsBingName)
        ui->radioButton->setChecked(true);
    else
        ui->radioButton_2->setChecked(true);
    ui->checkBox->setChecked(VarBox->AutoSaveBingPicture);
    ui->checkBox_2->setChecked(true);
    ui->checkBox_2->setEnabled(false);
}

BingSetting::~BingSetting()
{
    delete ui;
}

void BingSetting::closeEvent(QCloseEvent *e)
{
    e->accept();
}

void BingSetting::on_pushButton_2_clicked()
{
    QSettings set("SpeedBox.ini", QSettings::IniFormat);
    set.beginGroup("Wallpaper");
    set.setValue("UseDateAsBingName", ui->radioButton->isChecked());
    set.setValue("AutoSaveBingPicture", ui->checkBox->isChecked());
    set.setValue("AutoRotationBingPicture", ui->checkBox_2->isChecked());
    set.endGroup();
    jobTip->showTip("保存成功！");
}


void BingSetting::on_pushButton_3_clicked()
{
    VarBox->UseDateAsBingName = ui->radioButton->isChecked();
    VarBox->AutoSaveBingPicture = ui->checkBox->isChecked();
    jobTip->showTip("应用成功，可能需要点击保存！");
}
