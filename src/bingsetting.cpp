#include <QSettings>
#include <QGraphicsDropShadowEffect>
#include <QFile>
#include <QTextCodec>

#include "funcbox.h"
#include "form.h"
#include "bingsetting.h"
#include "blankform.h"
#include "gmpoperatetip.h"
#include "ui_bingsetting.h"
#include "wallpaper.h"

BingSetting::BingSetting():
    SpeedWidget<QDialog>(),
    ui(new Ui::BingSetting)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    QFile qss(QStringLiteral(":/qss/bing_style.qss"));
    qss.open(QFile::ReadOnly);
    setStyleSheet(QString(qss.readAll()));
    qss.close();
    ui->setupUi(this);
    initSpeedBox(ui->frame, &BingSetting::showMinimized, &BingSetting::close);

    if (VarBox->wallpaper->m_useDateAsBingName)
        ui->radioButton->setChecked(true);
    else
        ui->radioButton_2->setChecked(true);
    ui->checkBox->setChecked(VarBox->wallpaper->m_autoSaveBingPicture);
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
    QSettings set(QStringLiteral("SpeedBox.ini"), QSettings::IniFormat);
    set.setIniCodec(QTextCodec::codecForName("UTF-8"));
    set.beginGroup(QStringLiteral("Wallpaper"));
    set.setValue(QStringLiteral("UseDateAsBingName"), ui->radioButton->isChecked());
    set.setValue(QStringLiteral("AutoSaveBingPicture"), ui->checkBox->isChecked());
    set.setValue(QStringLiteral("AutoRotationBingPicture"), ui->checkBox_2->isChecked());
    set.endGroup();
    jobTip->showTip(QStringLiteral("保存成功！"));
}


void BingSetting::on_pushButton_3_clicked()
{
    VarBox->wallpaper->m_useDateAsBingName = ui->radioButton->isChecked();
    VarBox->wallpaper->m_autoSaveBingPicture = ui->checkBox->isChecked();
    jobTip->showTip(QStringLiteral("应用成功，可能需要点击保存！"));
}
