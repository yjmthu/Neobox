#ifndef TRANSLATER_H
#define TRANSLATER_H

#include "ui_translater.h"

class Form;
class QNetworkAccessManager;

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
#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
#else
    bool nativeEvent(const QByteArray &eventType, void *message, long long *result);
#endif
    void mouseReleaseEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void hideEvent(QHideEvent *event);
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *target, QEvent *event);       //事件过滤器
    void keyPressEvent(QKeyEvent *event);                   //键盘点击时间
    void showEvent(QShowEvent* event);

public:
    explicit Translater();
    ~Translater();

private:
    enum Type
    {
        ZH_CN2EN, ZH_CN2JA, ZH_CN2KR, ZH_CN2FR,
        ZH_CN2RU, ZH_CN2SP, EN2ZH_CN, JA2ZH_CN,
        KR2ZH_CN, FR2ZH_CN, RU2ZH_CN, SP2ZH_CN,
    };
    const char types[12][9] =
    {
        "ZH_CN2EN", //中文　»　英语
        "ZH_CN2JA", //中文　»　日语
        "ZH_CN2KR", //中文　»　韩语
        "ZH_CN2FR", //中文　»　法语
        "ZH_CN2RU", //中文　»　俄语
        "ZH_CN2SP", //中文　»　西语
        "EN2ZH_CN", //英语　»　中文
        "JA2ZH_CN", //日语　»　中文
        "KR2ZH_CN", //韩语　»　中文
        "FR2ZH_CN", //法语　»　中文
        "RU2ZH_CN", //俄语　»　中文
        "SP2ZH_CN" //西语　»　中文
    };
    Ui::Translater *ui;                                     //界面
    Type type;
    class QxtGlobalShortcut * const shortcut_show, * const shortcut_hide;
    // = new QxtGlobalShortcut(QKeySequence("Ctrl+Shift+F12"), w)
    uint64_t last_post_time;
    void requestData(const char*, std::string*);
    QNetworkAccessManager * const mgr;
    class QTimer * const timer;
    void initConnects();
    char time_left = 10;
    HWND hCurrentCursor = NULL;
#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
    class QTextToSpeech *speaker;
#endif

private slots:
    void setFix(bool checked);
    void copyTranlate();
    void startEnToZh(bool checked);
    void getReply(const QByteArray&);
};

#endif // TRANSLATER_H
