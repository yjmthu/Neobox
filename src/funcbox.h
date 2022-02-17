#ifndef FUNCBOX_H
#define FUNCBOX_H

#ifndef qout
#define qout qDebug()
#endif

#include <QString>
#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>

#ifdef Q_OS_WIN
#include <Windows.h>
#elif defined Q_OS_LINUX
#include <Windows.h>
#endif

#include "systemfunctions.h"
#include "globalfn.h"

class VARBOX: public QObject
{
    Q_OBJECT

public:
    static constexpr int RETCODE_ERROR_EXIT { 1071 };       //异常退出常数
    static constexpr int RETCODE_UPDATE     { 1072 };       //更新常数，更新软件
    static constexpr int RETCODE_RESTART    { 1073 };       //重启常数，双击时界面会返回这个常数实现整个程序重新启动。
    static constexpr int MSG_APPBAR_MSGID   { 2731 };

    constexpr static uint32_t m_dVersion  { GlobalFn::getVersion(22, 2, 18) };
    const bool m_bFirstUse                { false };
    const SystemVersion m_dSystemVersion;
    const int m_dScreenWidth, m_dScreenHeight;                  //屏幕宽高

    QString m_pathToOpen;                                       // 右键要打开的文件夹
    bool m_bEnableTranslater      { false };
    bool m_bEnableUSBhelper       { true  };
    bool m_bMarkdownNote          { false };
    bool m_bSquareClock           { false };
    bool m_bRoundClock            { false };
    bool m_bTuoPanIcon            { false };

    class WindowPosition* const m_pWindowPosition  { nullptr };
    class Form* const m_pForm                      { nullptr };
    class Dialog* const m_pDialog                  { nullptr };
    class MarkdownNote* m_pNote                    { nullptr };
    class SquareClock* m_sClock                    { nullptr };
    class RoundClock* m_rClock                     { nullptr };
    class Wallpaper* m_pWallpaper                  { nullptr };                  //壁纸处理类
    class QSystemTrayIcon* m_pSystemTrayIcon       { nullptr };

    explicit VARBOX(int, int);
    ~VARBOX();
public slots:
    void createTrayIcon(bool create);
    void createMarkdown(bool create);
    void createSquareClock(bool create);
    void createRoundClock(bool create);
private:
     void initProJob();
     void initFile();
     void initChildren();
     void initBehavior();
};

extern VARBOX* VarBox;
#endif // FUNCBOX_H
