#ifndef TRANSLATER_H
#define TRANSLATER_H

#include <QWidget>
#include <QTimer>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QShowEvent>

#include "ui_translater.h"
#include "qxtglobalshortcut.h"

//#define TRAN_HEIGHT 390
//#define TRAN_WIDTH 360
#define TRAN_HEIGHT 260
#define TRAN_WIDTH 240

#if defined(Q_OS_WIN32)
#define ADAPT_HEIGHT 30
#define ADAPT_WIDTH 60
#elif defined(Q_OS_LINUX)
#define ADAPT_HEIGHT 30
#define ADAPT_WIDTH 60
#endif

class Form;

namespace Ui {
class Translater;
}

class Translater : public QWidget
{
    Q_OBJECT

signals:
    void msgBox(QString);

protected:
    bool eventFilter(QObject *target, QEvent *event);       //事件过滤器
    void keyPressEvent(QKeyEvent *event);                   //键盘点击时间

public:
    explicit Translater(Form *);
    ~Translater();

private:
    Ui::Translater *ui;                                     //界面
    QButtonGroup *group;                                    //中英按钮，互斥
    Form *form;                                             //指向悬浮窗界面
    QString from = "en", to = "zh";                         //翻译方向，默认中译英
    const QString salt="1435660288";                        //请求参数之一
    void replyFinished(QString*);                           //请求结束
    void check_is_auto_hide() const;                        //读取设置文件，决定是否自动隐藏
    QxtGlobalShortcut *shortcut_show;                       //捕获显示界面快捷键
    QxtGlobalShortcut *shortcut_hide;                       //捕获隐藏界面快捷键

public slots:
    void showself();                                        //显示界面

private slots:
    void on_isFix_clicked(bool checked);
    void on_pBtnCopyTranlate_clicked();
    void on_ENTOZH_clicked(bool checked);
    void on_ZHTOEN_clicked(bool checked);
    void getReply(QString);
};

#endif // TRANSLATER_H
