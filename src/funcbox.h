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

constexpr int RETCODE_ERROR_EXIT =           1071;       //异常退出常数
constexpr int RETCODE_UPDATE     =           1072;       //更新常数，更新软件
constexpr int RETCODE_RESTART    =           1073;       //重启常数，双击时界面会返回这个常数实现整个程序重新启动。
constexpr int MSG_APPBAR_MSGID   =           2731;



class VARBOX: public QObject
{
    Q_OBJECT

public:
    const char* const Version = "22.1.11";
    const bool FirstUse[1] = { false };

    class WindowPosition* const m_windowPosition { nullptr };

    bool EnableTranslater { false }, AutoHide { false };
    bool enableUSBhelper { true };     // enableUSBhelper
    QString PathToOpen;                // 右键要打开的文件夹
    bool m_MarkdownNote { false }, m_SquareClock { false }, m_RoundClock { false }, m_TuoPanIcon { false };

    const int ScreenWidth, ScreenHeight;                  //屏幕宽高

    class Form* const form { nullptr }; class Dialog* const dialog { nullptr };
    class MarkdownNote* m_note { nullptr }; class SquareClock* m_sClock { nullptr };
    void * m_rClock { nullptr };

    std::function<bool(const wchar_t*)> PathFileExists { nullptr };
    explicit VARBOX(int, int);
    ~VARBOX();
    class Wallpaper* wallpaper { nullptr };                          //壁纸处理类
    class QSystemTrayIcon *systemTrayIcon { nullptr };

    static QByteArray runCmd(const QString & program, const QStringList& argument, short line);
    template<class _Ty>
    static void saveOneSet(const QString& group, const QString& key, _Ty val) {
        class QSettings IniRead(QStringLiteral("SpeedBox.ini"), QSettings::IniFormat);
        IniRead.setIniCodec(QTextCodec::codecForName("UTF-8"));
        IniRead.beginGroup(group);
        IniRead.setValue(key, val);
        IniRead.endGroup();
    }
    uint32_t getVersion(const char* A);
public slots:
    static void MSG(const char* text, const char* title="提示", QMessageBox::StandardButtons buttons=QMessageBox::Ok);
    static void openDirectory(const QString& dir);
    void createTrayIcon(bool create);
    void createMarkdown(bool create);
    void createSquareClock(bool create);
    void createRoundClock(bool create);
private:
     friend class Form;
     void initProJob();
     void initFile();
     void initChildren();
     void initBehaviors();
};

extern VARBOX* VarBox;
#endif // FUNCBOX_H
