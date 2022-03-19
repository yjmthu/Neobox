#ifndef TRANSLATER_H
#define TRANSLATER_H

#include "ui_translater.h"
#include <chrono>

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
#ifdef Q_OS_WIN
#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
#else
    bool nativeEvent(const QByteArray &eventType, void *message, long long *result);
#endif
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
    void initConnects();
    void initSettings();
    static bool getShiftAState();
    static bool getShiftZState();

private:
    enum LangType { EN, JA, KR, FR, RU, SP, ZH_CN };
    LangType m_dLangFrom  { LangType::EN };
    LangType m_dLangTo    { LangType::EN };
    const char m_aLangType[6][3] {
        /*英*/ "EN", /*日*/"JA", /*韩*/"KR", /*法*/"FR", /*俄*/"RU", /*西*/"SP"
    };
    const char m_sNativeLang[6] { "ZH_CN" };
    Ui::Translater *ui;                                     //界面
    static constexpr size_t m_dLangPos {53};
    char m_dYouDaoApi[65] { "http://fanyi.youdao.com/translate?&doctype=json&type=ZH_CN2EN&i=" };
    decltype (std::chrono::milliseconds().count()) m_dLastPostTime;
    void requestData(const char*, std::string*);
    class QNetworkAccessManager* const m_pMgr         {nullptr};
    class QTimer* const m_pTimer;
    char m_dTimeLeft                                  {  10   };
#if defined (Q_OS_WIN32)
    HWND hCurrentCursor                               {nullptr};
#endif
#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
    class QTextToSpeech *m_pSpeaker                   {nullptr};
    static bool m_bAutoHide;
#endif

private slots:
    void setFix(bool checked);
    void copyTranlate();
    void startEnToZh(bool checked);
    void getReply(const QString& q);

public slots:
    void setShiftA();
    void setShiftZ();
};

#endif // TRANSLATER_H
