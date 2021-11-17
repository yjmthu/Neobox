#ifndef TRANSLATER_H
#define TRANSLATER_H

#include "ui_translater.h"

class Form;

namespace std {
class thread;
}

namespace Ui {
class Translater;
}

class Translater : public QWidget
{
    Q_OBJECT

signals:
    void msgBox(const char*);
    void finished(bool);

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
    uint64_t last_post_time;
    void requestData(const char*, std::string*);
    std::thread *thrd = nullptr;
    void initConnects();

private slots:
    void setFix(bool checked);
    void copyTranlate();
    void startEnToZh(bool checked);
    void getReply(const QByteArray&);
};

#endif // TRANSLATER_H
