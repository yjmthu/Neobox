#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QColorDialog>

#include "YEncode.h"
#include "YString.h"
#include "YJson.h"
#include "funcbox.h"
#include "formsetting.h"
#include "ui_formsetting.h"
#include "blankform.h"
#include "form.h"
#include "ui_form.h"

constexpr char style_style[] = "QFrame{border-radius:%1px;background-color:rgba(%2,%3,%4,%5);}";

YJsonItem* load_style(const std::string style)
{
    std::string::const_iterator ptr = StrSkip(style.cbegin());
    if (!strncmp(ptr, "QFrame", 6))
    {
        ptr = StrSkip(ptr += 6);
        if (*ptr != '{') return nullptr;
    }
    else if (*ptr != '{')
    {
        return nullptr;
    }
    if (*++ptr == '}') return nullptr;
    YJsonItem* js = YJsonItem::newObject();
    std::string::const_iterator ptr1;
    while (true)
    {
        ptr = StrSkip(ptr);
        if (!strncmp(ptr, "border-radius", 13))
        {
            int radius = 0;
            ptr = StrSkip(ptr += 13);
            if (*ptr != ':') break;
            ptr1 = ptr = StrSkip(++ptr);
            while ('0' <= *ptr1 && *ptr1 <= '9') (radius *= 10) += *ptr1++ - '0';
            js->appendItem(radius, "border-radius");
            ptr = ptr1;
            while (*ptr && *ptr != ';') ++ptr;
            if (*ptr) ++ptr;
            else break;
        }
        else if (!strncmp(ptr, "background-color", 16))
        {
            int color;
            ptr = StrSkip(ptr += 16);
            if (*ptr != ':') break;
            ptr = StrSkip(++ptr);
            if (strncmp(ptr, "rgba(", 5)) break ;
            ptr1 = ptr + 5;
            YJsonItem* temp = js->appendItem(YJsonItem::Array(), "background-color");
            while (true) {
                color = 0;
                while ('0' <= *ptr1 && *ptr1 <= '9') (color *= 10) += *ptr1++- '0';
                temp->appendItem(color);
                ptr1 = StrSkip(ptr1);
                if (!*ptr1) break;
                if (*ptr1 == ',') ptr1 = StrSkip(++ptr1);
                if (!*ptr1)
                    break;
                else if (*ptr1 == ')')
                {
                    ptr = ptr1 + 1;
                    break;
                }
            }
            while (*ptr && *ptr != ';') ++ptr;
            if (*ptr) ++ptr;
            else break;
        }
        else
            break;
    }
    return js;
}

FormSetting::FormSetting(QDialog*) :
    SpeedWidget<QDialog>(),
    ui(new Ui::FormSetting)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    QFile qss(":/qss/bing_style.qss");
    qss.open(QFile::ReadOnly);
    setStyleSheet(QString(qss.readAll()));
    qss.close();
    ui->setupUi(this);
    initSpeedBox(ui->frame, &FormSetting::showMinimized, &FormSetting::close);
    ui->horizontalSlider->installEventFilter(this);
    old_style = load_style(VarBox->form->ui->frame->styleSheet().toStdString());
    int r = 8, g = 8, b=8, a = 190, p;
    YJsonItem *temp = old_style->findItem("background-color");
    r = temp->findItem(0)->getValueInt();
    g = temp->findItem(1)->getValueInt();
    b = temp->findItem(2)->getValueInt();
    a = temp->findItem(3)->getValueInt();
    p = old_style->findItem("border-radius")->getValueInt();
    ui->pushButton->setStyleSheet(QString("QPushButton{background-color: rgb(%1, %2, %3);}").arg(r).arg(g).arg(b));
    new_color = QColor(r, g, b);
    ui->horizontalSlider->setValue(a);
    if (p == 0)
    {
        ui->radioButton->setChecked(true);
    }
    else if (p == 3)
    {
        ui->radioButton_2->setChecked(true);
    }
    else
    {
        ui->radioButton_3->setChecked(true);
    }
}

FormSetting::~FormSetting()
{
    delete old_style;
    delete ui;
}

bool FormSetting::eventFilter(QObject* target, QEvent* event)
{
    if (typeid (*target) == typeid (QSlider))
    {
        QMouseEvent *env = static_cast<QMouseEvent *>(event);
        QSlider *tar = static_cast<QSlider *>(target);
        //static bool clicked = false;
        switch ((QEvent::Type)(env->type()))
        {
        case QEvent::MouseMove:
            tar->setValue(((double)env->pos().x() / (double)tar->width() * (tar->maximum() - tar->minimum())) + tar->minimum());
            return true;
        case QEvent::MouseButtonPress:
            tar->setValue(((double)env->pos().x() / (double)tar->width() * (tar->maximum() - tar->minimum())) + tar->minimum());
            tar->setMouseTracking(true);
            return true;
        case QEvent::MouseButtonRelease:
            tar->setMouseTracking(false);
            return true;
        default:
            break;
        }
    }
    return QWidget::eventFilter(target, event);
}


void FormSetting::closeEvent(QCloseEvent *event)
{
    int r = 8, g = 8, b=8, a = 190, p;
    YJsonItem *temp = old_style->findItem("background-color");
    r = temp->findItem(0)->getValueInt();
    g = temp->findItem(1)->getValueInt();
    b = temp->findItem(2)->getValueInt();
    a = temp->findItem(3)->getValueInt();
    p = old_style->findItem("border-radius")->getValueInt();
    VarBox->form->ui->frame->setStyleSheet((QString(style_style).arg(p).arg(r).arg(g).arg(b).arg(a)));
    event->accept();
}

void FormSetting::on_pushButton_clicked()
{
    YJsonItem& temp = (*old_style)["background-color"];
    int r = 8, g = 8, b=8;
    if ((temp && temp.getType() == YJSON_TYPE::YJSON_ARRAY))
    {
        r = temp[0].getValueInt();
        g = temp[1].getValueInt();
        b = temp[2].getValueInt();
    }
    QColor color = QColorDialog::getColor(QColor(r, g, b), ui->frame, "颜色");
    if (color.isValid())
    {
        new_color = color;
        ui->pushButton->setStyleSheet(QString("QPushButton{background-color: rgb(%1, %2, %3);}").arg(new_color.red()).arg(new_color.green()).arg(new_color.blue()));
        ui->pushButton->showNormal();
    }
}


void FormSetting::on_horizontalSlider_valueChanged(int value)
{
    ui->label_8->setText(QString::number(value));
}

void FormSetting::on_pushButton_2_clicked()
{
    //预览颜色
    int r = new_color.red(), g = new_color.green(), b=new_color.blue(), a=ui->horizontalSlider->value(), p=3;
    YJsonItem *js = load_style(VarBox->form->ui->frame->styleSheet().toStdString());
    p = js->findItem("border-radius")->getValueInt();
    delete js;
    VarBox->form->ui->frame->setStyleSheet(QString(style_style).arg(p).arg(r).arg(g).arg(b).arg(a));
}

void FormSetting::on_pushButton_7_clicked()
{
    //恢复颜色
    int r = 8, g = 8, b=8, a = 190, p=3;
    YJsonItem &temp = (*old_style)["background-color"];
    if (temp && temp.getType() == YJSON_TYPE::YJSON_ARRAY)
    {
        r = temp[0].getValueInt();
        g = temp[1].getValueInt();
        b = temp[2].getValueInt();
        a = temp[3].getValueInt();
    }
    ui->pushButton->setStyleSheet(QString("QPushButton{background-color: rgb(%1, %2, %3);}").arg(r).arg(g).arg(b));
    ui->horizontalSlider->setValue(a);
    YJsonItem *js = load_style(VarBox->form->ui->frame->styleSheet().toStdString());
    p = js->findItem("border-radius")->getValueInt();
    delete js;
    VarBox->form->ui->frame->setStyleSheet((QString(style_style).arg(p).arg(r).arg(g).arg(b).arg(a)));
}



void FormSetting::on_pushButton_5_clicked()
{
    //预览默认
    constexpr int r = 8, g = 8, b=8, a = 190, p=3;
    new_color = QColor(r,g,b);
    ui->pushButton->setStyleSheet(QString("background-color: rgb(%1, %2, %3)").arg(r).arg(g).arg(b));
    ui->horizontalSlider->setValue(a);
    ui->radioButton_2->setChecked(true);
    VarBox->form->ui->frame->setStyleSheet((QString(style_style).arg(p).arg(r).arg(g).arg(b).arg(a)));
}


void FormSetting::on_pushButton_3_clicked()
{
    //预览圆角
    int r = 8, g = 8, b=8, a = 190, p;
    YJsonItem *js = load_style(VarBox->form->ui->frame->styleSheet().toStdString());
    YJsonItem &temp = *(js->findItem("background-color"));
    if (temp && temp.getType() == YJSON_TYPE::YJSON_ARRAY)
    {
        r = temp[0].getValueInt();
        g = temp[1].getValueInt();
        b = temp[2].getValueInt();
        a = temp[3].getValueInt();
    }
    delete js;
    if (ui->radioButton->isChecked()) p = 0;
    else if (ui->radioButton_2->isChecked()) p = 3;
    else p = 10;
    VarBox->form->ui->frame->setStyleSheet(QString(style_style).arg(p).arg(r).arg(g).arg(b).arg(a));
}

void FormSetting::on_pushButton_6_clicked()
{
    //保存设置
    delete old_style;
    int r = new_color.red(), g = new_color.green(), b=new_color.blue(), a=ui->horizontalSlider->value(), p;
    if (ui->radioButton->isChecked()) p = 0;
    else if (ui->radioButton_2->isChecked()) p = 3;
    else p = 10;
    QString style = QString(style_style).arg(p).arg(r).arg(g).arg(b).arg(a);
    old_style = load_style(style.toStdString());
    VarBox->form->ui->frame->setStyleSheet(style);
    QString file = VARBOX::get_dat_path() + "\\" + "FormStyle.json";
    if (QFile::exists(file))
    {
        YJsonItem &&s = YJsonItem::Object();
        YJsonItem *js = s.findItem("form-frame");
        if (js)
        {
            *js = *old_style;
        }
        else
        {
            s.appendItem(*old_style, "form-frame");
        }
        s.toFile(file.toStdWString(), YJSON_ENCODE::UTF8);
    }
    else
    {
        YJsonItem &&s = YJsonItem::Object();
        s.appendItem(*old_style, "form-frame");
        s.toFile(file.toStdWString(), YJSON_ENCODE::UTF8);
    }
    jobTip->showTip("保存成功！");
}

void FormSetting::load_style_from_file()
{
    int r = 8, g = 8, b=8, a = 190, p=3;
    QString file = VARBOX::get_dat_path() + "\\" + "FormStyle.json";
    if (QFile::exists(file))
    {
        YJsonItem *s = YJsonItem::newFromFile(file.toStdWString());
        YJsonItem *js = s->findItem("form-frame");
        if (js)
        {

            YJsonItem *temp = js->findItem("background-color");
            r = temp->findItem(0)->getValueInt();
            g = temp->findItem(1)->getValueInt();
            b = temp->findItem(2)->getValueInt();
            a = temp->findItem(3)->getValueInt();
            p = js->findItem("border-radius")->getValueInt();
        }
        delete s;
    }
    VarBox->form->ui->frame->setStyleSheet(QString(style_style).arg(p).arg(r).arg(g).arg(b).arg(a));
}

