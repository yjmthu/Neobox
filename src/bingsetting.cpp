#include <QSettings>
#include <QGraphicsDropShadowEffect>
#include <QFile>

#include "funcbox.h"
#include "form.h"
#include "bingsetting.h"
#include "blankform.h"
#include "gmpoperatetip.h"
#include "ui_bing_setting.h"

BingSetting::BingSetting():
    QDialog(),
    ui(new Ui::BingSetting)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    QFile qss(":/qss/bing_style.qss");
    qss.open(QFile::ReadOnly);
    setStyleSheet(QString(qss.readAll()));
    qss.close();
    ui->setupUi(this);
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(this);
    effect->setOffset(0, 0);          //具体阴影
    effect->setBlurRadius(15);        //阴影半径
    effect->setColor(Qt::black);      //阴影颜色
    ui->frame->setGraphicsEffect(effect);
    BlankFrom *blank = new BlankFrom(this);
    connect(blank->closeButton, &QPushButton::clicked, this, &BingSetting::close);
    connect(blank->minButton, &QPushButton::clicked, this, &BingSetting::showMinimized);
    blank->move(width()-100, 3);
    jobTip = new GMPOperateTip(this);

    if (VarBox->UseDateAsBingName)
        ui->radioButton->setChecked(true);
    else
        ui->radioButton_2->setChecked(true);
    ui->checkBox->setChecked(VarBox->AutoSaveBingPicture);
    ui->checkBox_2->setChecked(VarBox->AutoRotationBingPicture);
}

BingSetting::~BingSetting()
{
    delete  jobTip;
    delete ui;
}

void BingSetting::on_pushButton_2_clicked()
{
    QSettings set(VarBox->get_ini_path(), QSettings::IniFormat);
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
    VarBox->AutoRotationBingPicture = ui->checkBox_2->isChecked();
    jobTip->showTip("应用成功，可能需要点击保存！");
}

void BingSetting::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)                   // 鼠标左键点击悬浮窗
    {
        setMouseTracking(true);                              // 开始跟踪鼠标位置，从而使悬浮窗跟着鼠标一起移动。
        _startPos = event->pos();                            // 记录开始位置
    }
    event->accept();
}

void BingSetting::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)  // 鼠标左键释放
    {
        setMouseTracking(false);           // 停止跟踪鼠标。
        _endPos = event->pos() - _startPos;
        mouse_moved = false;
    }
    event->accept();
}

void BingSetting::mouseMoveEvent(QMouseEvent* event)
{
    _endPos = event->pos() - _startPos;  //计算位置变化情况。
    move(pos() + _endPos);               //当前位置加上位置变化情况，从而实现悬浮窗和鼠标同时移动。
    mouse_moved = true;
    event->accept();
}

