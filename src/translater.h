#ifndef TRANSLATER_H
#define TRANSLATER_H

#include "ui_translater.h"

class Form;

namespace Ui {
class Translater;
}

class Translater : public QWidget
{
    Q_OBJECT

signals:
    void msgBox(const char*);
    void receivedData(bool);
    void enableself(bool);

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long long *result);
    bool eventFilter(QObject *target, QEvent *event);       //事件过滤器
    void keyPressEvent(QKeyEvent *event);                   //键盘点击时间
    void showEvent(QShowEvent* event);

public:
    explicit Translater();
    ~Translater();

private:
    Ui::Translater *ui;                                     //界面
    const char _en[3], _zh[3];                              //翻译方向，默认中译英
    const char* from, * to;
    void requestData(const char*, std::string*);
    bool unfinished = false;

private slots:
    void on_isFix_clicked(bool checked);
    void on_pBtnCopyTranlate_clicked();
    void on_ENTOZH_clicked(bool checked);
    void on_ZHTOEN_clicked(bool checked);
    void getReply(const QByteArray&);
};

#endif // TRANSLATER_H
