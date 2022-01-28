#include <QFile>
#if defined (Q_OS_WIN32)
#include <windows.h>
#endif
#include "funcbox.h"
#include "form.h"
#include "calculator.h"
#include "ui_calculator.h"
#include "FormulaPaser.h"

Calculator::Calculator(QWidget *parent) :
    SpeedWidget<QDialog>(parent),
    ui(new Ui::Calculator)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    QFile qss(QStringLiteral(":/qss/calculater.qss"));
    qss.open(QFile::ReadOnly);
    setStyleSheet(QString(qss.readAll()));
    qss.close();
    ui->setupUi(this);
    initSpeedBox(ui->frame, &Calculator::showMinimized, &Calculator::close, false);
    blank->minButton->setGeometry(ui->frame->width()-65,12,14,14);
    blank->closeButton->setGeometry(ui->frame->width()-30,12,14,14);
    ui->plainTextEdit->installEventFilter(this);
}

Calculator::~Calculator()
{
    delete ui;
}

void Calculator::showEvent(QShowEvent* event)
{   
#ifdef Q_OS_WIN
    int x, y;
    RECT rt;
    int w = (GetWindowRect(HWND(winId()), &rt), rt.right - rt.left), h = (rt.bottom - rt.top), sw = GetSystemMetrics(SM_CXSCREEN);
    GetWindowRect(HWND(VarBox->form->winId()), &rt);
    if (rt.top > h)
        y = rt.top - h;
    else
        y = rt.bottom;
    if (rt.left + rt.right + w > sw * 2)
        x = sw - w;
    else if (rt.right + rt.left < w)
        x = 0;
    else
        x = (rt.left + rt.right - w) / 2;
    SetWindowPos(HWND(winId()), HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
#elif defined (Q_OS_LINUX)
    int x, y;
    QRect rt=VarBox->form->geometry();
    if (rt.top() > height())
        y = rt.top() - height();
    else
        y = rt.bottom();
    if (rt.left() + rt.right() + width() > VarBox->ScreenWidth * 2)
        x = VarBox->ScreenWidth - width();
    else if (rt.right() + rt.left() < width())
        x = 0;
    else
        x = (rt.left() + rt.right() - width()) / 2;
    move(x, y);
#endif

    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->plainTextEdit->setTextCursor(cursor);
    ui->plainTextEdit->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    ui->plainTextEdit->setFocus();
    event->accept();
}

bool Calculator::eventFilter(QObject* target, QEvent* event)
{
    if (target == ui->plainTextEdit)
    {
        if (event->type() == QEvent::KeyPress)               //
        {
            QKeyEvent* k = static_cast<QKeyEvent*>(event);

            if (k->key() == Qt::Key_Return || k->key() == Qt::Key_Equal)       //回车键
            {
                FormulaPaser<char> paser(ui->plainTextEdit->toPlainText().toStdString());
                ui->lineEdit->setText(paser.outstr(true).c_str());
                ui->plainTextEdit->setFocus();
                event->accept();
                return true;
            }
            else if (k->key() == Qt::Key_Delete)  //删除键
            {
                ui->plainTextEdit->clear();
                event->accept();
                return true;
            }
        }
    }
    return QWidget::eventFilter(target, event);
}

void Calculator::on_plainTextEdit_textChanged()
{
    FormulaPaser<char> paser(ui->plainTextEdit->toPlainText().toStdString());
    ui->lineEdit->setText(paser.outstr(false).c_str());
    ui->plainTextEdit->setFocus();
}


void Calculator::on_pushButton_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("deg("));
    ui->plainTextEdit->setTextCursor(cursor);
}


void Calculator::on_pushButton_2_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("rad("));
}

void Calculator::on_pushButton_3_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
    {
        cursor.setPosition(cursor.selectionStart());
    }
    else if (!cursor.atStart())
    {
        cursor.setPosition(cursor.position()-1);
    }
    ui->plainTextEdit->setTextCursor(cursor);
    ui->plainTextEdit->setFocus();
}

void Calculator::on_pushButton_4_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
    {
        cursor.setPosition(cursor.selectionEnd());
    }
    else if (!cursor.atEnd())
    {
        cursor.setPosition(cursor.position()+1);
    }
    ui->plainTextEdit->setTextCursor(cursor);
    ui->plainTextEdit->setFocus();
}

void Calculator::on_pushButton_5_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("("));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_6_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral(")"));
    ui->plainTextEdit->setTextCursor(cursor);
}


void Calculator::on_pushButton_7_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("tanh("));
    ui->plainTextEdit->setTextCursor(cursor);
}


void Calculator::on_pushButton_8_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("cosh("));
    ui->plainTextEdit->setTextCursor(cursor);
}


void Calculator::on_pushButton_9_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("sinh("));
    ui->plainTextEdit->setTextCursor(cursor);
}


void Calculator::on_pushButton_10_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("tan("));
    ui->plainTextEdit->setTextCursor(cursor);
}


void Calculator::on_pushButton_11_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("cos("));
    ui->plainTextEdit->setTextCursor(cursor);
}


void Calculator::on_pushButton_12_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("sin("));
    ui->plainTextEdit->setTextCursor(cursor);
}


void Calculator::on_pushButton_13_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("lg("));
    ui->plainTextEdit->setTextCursor(cursor);
}


void Calculator::on_pushButton_14_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("arc"));
    ui->plainTextEdit->setTextCursor(cursor);
}


void Calculator::on_pushButton_15_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("abs("));
    ui->plainTextEdit->setTextCursor(cursor);
}


void Calculator::on_pushButton_16_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("sqrt("));
    ui->plainTextEdit->setTextCursor(cursor);
}


void Calculator::on_pushButton_17_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("^"));
    ui->plainTextEdit->setTextCursor(cursor);
}



void Calculator::on_pushButton_18_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("fact("));
    ui->plainTextEdit->setTextCursor(cursor);
}


void Calculator::on_pushButton_19_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("ln("));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_20_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("7"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_21_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("8"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_22_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("9"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_23_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    else if (!cursor.atStart())
        cursor.deletePreviousChar();
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_24_clicked()
{
    ui->plainTextEdit->clear();
}

void Calculator::on_pushButton_25_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("e()"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_26_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("4"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_27_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("5"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_28_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("6"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_29_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("*"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_30_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("/"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_31_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("pi()"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_32_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("1"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_33_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("2"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_34_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("3"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_35_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("+"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_36_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("-"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_37_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("rand()"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_38_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("0"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_39_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("."));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_40_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("exp("));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_41_clicked()
{
    QTextCursor&& cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection())
        cursor.removeSelectedText();
    cursor.insertText(QStringLiteral("ans()"));
    ui->plainTextEdit->setTextCursor(cursor);
}

void Calculator::on_pushButton_42_clicked()
{
    FormulaPaser<char> paser(ui->plainTextEdit->toPlainText().toStdString());
    ui->lineEdit->setText(paser.outstr(true).c_str());
    ui->plainTextEdit->setFocus();
}
