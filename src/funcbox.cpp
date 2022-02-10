#include <fstream>
#include <cstdlib>
#include <QApplication>
#include <QStandardPaths>
#include <QSettings>
#include <QScreen>
#include <QProcess>
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QTextCodec>
#include <QDesktopServices>

#if defined (Q_OS_WIN32)
#include <oaidl.h>
#include <WinInet.h>
#include <QFontDatabase>
#include <QSystemTrayIcon>
#include <QMenu>
#include "systemfunctions.h"
#elif defined (Q_OS_LINUX)
//#include <sys/time.h>
#include <dialog.h>
#include <unistd.h>
#endif

#include "funcbox.h"
#include "YString.h"
#include "YJson.h"
#include "form.h"
#include "wallpaper.h"
#include "dialog.h"
#include "markdownnote.h"
#include "squareclock.h"
#include "windowposition.h"


//wchar_t* GetCorrectUnicode(const QByteArray &ba)
//{
//    QTextCodec::ConverterState state;
//    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
//    codec->toUnicode(ba.constData(), ba.size(), &state);
//    QString str = (state.invalidChars > 0) ? QTextCodec::codecForName("GBK")->toUnicode(ba): ba;
//    qout << "转换后的字符串: " << str;
//    auto s = reinterpret_cast<const wchar_t*>(str.utf16());
//    auto l = wcslen(s)+1;
//    auto t = new wchar_t[l];
//    std::copy(s, s+l+1, t);
//    return t;
//}

VARBOX* VarBox = nullptr;


VARBOX::VARBOX(int w, int h):
    QObject(nullptr),
    m_systemVersion(SystemFunctions::getWindowsVersion()),
    m_windowPosition(WindowPosition::fromFile()),
    ScreenWidth(w), ScreenHeight(h)
{
    qout << "VarBox构造函数开始。";
    initProJob();
    initFile();
    initChildren();
    initBehaviors();
    qout << "VarBox构造函数结束。";
}

VARBOX::~VARBOX()
{
    qout << "结构体析构中~";
    delete m_windowPosition;
    delete systemTrayIcon;
    delete m_sClock;
    delete m_note;
    delete form;
    delete wallpaper;
    delete dialog;
    qout << "结构体析构成功。";
}

void VARBOX::initFile()
{
    if (!QDir().exists(QStringLiteral("./Scripts")))
        QDir().mkdir(QStringLiteral("./Scripts"));
    if (!QFile::exists(QStringLiteral("./Scripts/SetWallPaper.sh")))
        QFile::copy(QStringLiteral("://scripts/SetWallpaper.sh"), QStringLiteral("./Scripts/SetWallPaper.sh"));
    const QString file = QStringLiteral("SpeedBox.ini");
    qout << "配置文件目录" << file;
    QString picfolder { QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)) };
    QDir dir;
    if (!QFile::exists(file)) {
        qout << "SpeedBox.ini 文件不存在";
        goto label_1;
    } else {
        QSettings set(file, QSettings::IniFormat);
        set.setIniCodec(QTextCodec::codecForName("UTF-8"));
        set.beginGroup(QStringLiteral("SpeedBox"));
        const QByteArray& x = set.value(QStringLiteral("Version")).toByteArray();
        set.endGroup();
        qout << "文件存在，版本信息为" << x;
        if (getVersion(x) < getVersion("21.8.1"))
        {
            qout << "ini文件过期，将其删除。";
            QFile::remove(file);
            goto label_1;
        } else {
            qout << "版本支持，继续读取";
            goto label_2;
        }
    }
label_1:
        {
            qout << "创建新的Ini文件。";
            QSettings *IniWrite = new QSettings(QStringLiteral("SpeedBox.ini"), QSettings::IniFormat);
            IniWrite->setIniCodec(QTextCodec::codecForName("UTF-8"));
            IniWrite->beginGroup(QStringLiteral("SpeedBox"));
            IniWrite->setValue(QStringLiteral("Version"), Version);
            IniWrite->endGroup();
            IniWrite->beginGroup(QStringLiteral("Wallpaper"));
            IniWrite->setValue(QStringLiteral("PaperType"), 0);
            IniWrite->setValue(QStringLiteral("TimeInerval"), 15);
            IniWrite->setValue(QStringLiteral("PageNum"), 1);
            IniWrite->setValue(QStringLiteral("setNative"), false);
            IniWrite->setValue(QStringLiteral("AutoChange"), false);
            IniWrite->setValue(QStringLiteral("NativeDir"), picfolder);
#if defined (Q_OS_WIN32)
            IniWrite->setValue(QStringLiteral("UserCommand"), QStringLiteral("python.exe -u \"X:\\xxxxx.py\""));
#elif defined (Q_OS_LINUX)
            IniWrite->setValue("UserCommand", "python.exe -u \"~/xxxxx.py\"");
#endif
            IniWrite->setValue(QStringLiteral("UseDateAsBingName"), true);
            IniWrite->endGroup();

            IniWrite->beginGroup(QStringLiteral("Translate"));
            IniWrite->setValue(QStringLiteral("EnableTranslater"), false);
            IniWrite->setValue(QStringLiteral("AutoHide"), true);
            IniWrite->endGroup();

            IniWrite->beginGroup(QStringLiteral("UI"));
            IniWrite->setValue(QStringLiteral("ColorTheme"), static_cast<int>(Dialog::curTheme));
            IniWrite->setValue(QStringLiteral("TuoPanIcon"), false);
            IniWrite->setValue(QStringLiteral("TieBianHide"), true);
            IniWrite->setValue(QStringLiteral("ShowToolTip"), true);
            IniWrite->endGroup();

            IniWrite->beginGroup(QStringLiteral("Dirs"));
            IniWrite->setValue(QStringLiteral("OpenDir"), QDir::toNativeSeparators(qApp->applicationDirPath()));
            IniWrite->endGroup();

            IniWrite->beginGroup(QStringLiteral("Apps"));
            IniWrite->setValue(QStringLiteral("MarkdownNote"), false);
            IniWrite->setValue(QStringLiteral("DesktopClock"), false);
            IniWrite->endGroup();
            delete IniWrite;
            qout << "创建新的ini文件完毕。";
            goto label_3;
        }
label_2:
        {
            qout << "开始读取设置。";
            QSettings *IniRead = new QSettings(file, QSettings::IniFormat);
            IniRead->setIniCodec(QTextCodec::codecForName("UTF-8"));
            IniRead->beginGroup(QStringLiteral("SpeedBox"));
            IniRead->setValue(QStringLiteral("Version"), Version);
            IniRead->endGroup();

            IniRead->beginGroup(QStringLiteral("Translate"));
            m_autoHide = IniRead->value(QStringLiteral("AutoHide")).toBool();
            m_enableTranslater = IniRead->value(QStringLiteral("EnableTranslater")).toBool();
            IniRead->endGroup();
            qout << "读取翻译信息完毕";
            IniRead->beginGroup(QStringLiteral("Dirs"));
            PathToOpen = IniRead->value(QStringLiteral("OpenDir")).toString();
            IniRead->endGroup();
            qout << "读取路径信息完毕";
            IniRead->beginGroup(QStringLiteral("UI"));
            unsigned safeEnum = IniRead->value(QStringLiteral("ColorTheme")).toInt();
            if (safeEnum > 7) safeEnum = 0;
            Dialog::curTheme = static_cast<Dialog::Theme>(safeEnum);
            m_TuoPanIcon = IniRead->value(QStringLiteral("TuoPanIcon"), false).toBool();
            IniRead->endGroup();
            IniRead->beginGroup("Apps");
            m_MarkdownNote = IniRead->value(QStringLiteral("MarkdownNote"), m_MarkdownNote).toBool();
            m_SquareClock = IniRead->value(QStringLiteral("SquareClock"), m_SquareClock).toBool();
            m_RoundClock = IniRead->value(QStringLiteral("RoundClock"), m_RoundClock).toBool();
            m_enableUSBhelper = IniRead->value(QStringLiteral("UsbHelper"), m_enableUSBhelper).toBool();
            IniRead->endGroup();
            delete IniRead;
            qout << "读取设置完毕。";
        }
label_3:
        {
            if (!dir.exists(PathToOpen) && !dir.mkdir(PathToOpen))
            {
                PathToOpen = QDir::toNativeSeparators(qApp->applicationDirPath());
                saveOneSet(QStringLiteral("Dirs"), QStringLiteral("OpenDir"), PathToOpen);
            }
        }
    const std::string apifile = "WallpaperApi.json";
    if (!QFile::exists(apifile.c_str()))
    {
        qout << "文件不存在!";
        picfolder += "/桌面壁纸";
        dir.mkdir(picfolder);
        dir.cd(picfolder);
        QFile::copy(QStringLiteral(":/json/WallpaperApi.json"), (apifile+".temp").c_str());
        const std::vector<QString> lst {"最热壁纸", "风景壁纸", "动漫壁纸", "极简壁纸", "随机壁纸", "鬼刀壁纸", "必应壁纸"};
        for (const auto&c: lst)
            dir.mkdir(c);
        std::vector<QString>::const_iterator iter = lst.begin();
        YJson json(apifile+".temp", YJson::UTF8);
        for (auto&c: json["Default"]["ApiData"])
            c["Folder"].setText(QDir::toNativeSeparators((picfolder+"/"+*iter++)).toStdString());
        for (auto&c: json["User"]["ApiData"])
            c["Folder"].setText(QDir::toNativeSeparators(picfolder+"/"+*iter).toUtf8());
        json["BingApi"]["Folder"].setText(QDir::toNativeSeparators(picfolder+"/"+*++iter).toStdString());
        for (auto&c: json["OtherApi"]["ApiData"])
        {
            c["Folder"].setText(QDir::toNativeSeparators(picfolder+"/"+c.getKeyString()).toStdString());
            dir.mkdir(c["Folder"].getValueString());
        }
        json.toFile(apifile, YJson::UTF8, true);
        QFile::remove((apifile+".temp").c_str());
    }
}

void VARBOX::initChildren()
{
    qout << "开始创建基本成员";
    wallpaper = new Wallpaper;                           // 创建壁纸更换对象
    Form* form = new Form;
#ifdef Q_OS_WIN
    static APPBARDATA abd { 0,0,0,0,{0,0,0,0},0 };
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = HWND(form->winId());
    abd.uCallbackMessage = MSG_APPBAR_MSGID;
    SHAppBarMessage(ABM_NEW, &abd);
#endif
    QObject::connect(QGuiApplication::primaryScreen(), &QScreen::geometryChanged, form, [this, form](const QRect&rect){
        this->form->move(this->form->pos().x() * rect.width() / VarBox->ScreenWidth, this->form->pos().y() * rect.height() / VarBox->ScreenHeight);
        *const_cast<int *>(&ScreenWidth) = rect.width();
        *const_cast<int *>(&ScreenHeight) = rect.height();
        form->keepInScreen();
    });
    createMarkdown(m_MarkdownNote);
    createTrayIcon(m_TuoPanIcon);
    createSquareClock(m_SquareClock);
    createRoundClock(m_RoundClock);
}

void VARBOX::initBehaviors()
{
    form->show();                                                              //显示悬浮窗
    form->keepInScreen();
}

void VARBOX::createTrayIcon(bool create)
{
    if ((m_TuoPanIcon = create))
    {
        systemTrayIcon = new QSystemTrayIcon;
        systemTrayIcon->setIcon(QIcon(QStringLiteral(":/icons/speedbox.ico")));
        QMenu *m_trayMenu = new QMenu;
        QAction *initAct = m_trayMenu->addAction(QStringLiteral("复原"));
        QAction *quitAct = m_trayMenu->addAction(QStringLiteral("退出"));
        connect(quitAct, &QAction::triggered, qApp, &QApplication::quit);
        connect(initAct, &QAction::triggered, form, [=](){form->move(100, 100);form->saveBoxPos();});
        systemTrayIcon->setContextMenu(m_trayMenu);
        systemTrayIcon->show();
    } else {
        delete systemTrayIcon;
        systemTrayIcon = nullptr;
    }
    saveOneSet<bool>(QStringLiteral("UI"), QStringLiteral("TuoPanIcon"), create);
}

void VARBOX::createMarkdown(bool create)
{
    if ((m_MarkdownNote = create)) {
        m_note = new MarkdownNote;
    } else {
        delete m_note;
        m_note = nullptr;
    }
    saveOneSet<bool>(QStringLiteral("Apps"), QStringLiteral("MarkdownNote"), create);
}

void VARBOX::createSquareClock(bool create)
{
    if ((m_SquareClock = create)) {
        m_sClock = new SquareClock;
        m_sClock->show();
    } else {
        delete m_sClock;
        m_sClock = nullptr;
    }
    saveOneSet<bool>(QStringLiteral("Apps"), QStringLiteral("SquareClock"), create);
}

void VARBOX::createRoundClock(bool create)
{
//    if ((m_RoundClock = create)) {
//        m_rClock = new DesktopClock;
//        m_rClock->show();
//    } else {
//        delete m_rClock;
//        m_rClock = nullptr;
//    }
//    saveOneSet<bool>(QStringLiteral("Apps"), QStringLiteral("RoundClock"), create);
}

void VARBOX::initProJob()
{
    VarBox = this;
#if defined (Q_OS_WIN32)
    switch (m_systemVersion) {
    case SystemVersion::Windows10:
        break;
    case SystemVersion::Windows8_1:
    case SystemVersion::Windows8:
    case SystemVersion::Windows7:
        break;
    default:
        break;
    }
#endif
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Nickainley-Normal-small.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Carattere-Regular-small.ttf"));
}

QByteArray VARBOX::runCmd(const QString& program, const QStringList& argument, short line)
{
    QProcess process;
    process.setProgram(program);
    process.setArguments(argument);
    connect(&process, &QProcess::errorOccurred, [&](){ qout << "运行出错"; line=0; });
    process.start();
    process.waitForStarted(); //等待程序启动
    process.waitForFinished(15000);
    if (line) {
        for (int c = 1; c < line; c++)
        {
            process.readLine();
        }
        return process.readLine();
    }
    else return nullptr;
}

extern std::string Utf8ToAnsi(const std::string& strUtf8);
void VARBOX::openDirectory(const QString& dir)
{
#ifdef Q_OS_WIN
    auto path = Utf8ToAnsi(QDir::toNativeSeparators(dir).toStdString());
    ShellExecuteA(NULL, "open", "explorer.exe", path.c_str(), NULL, SW_SHOWNORMAL);
#elif defined Q_OS_LINUX
    system(("xdg-open \"" + dir + "\"").toStdString().c_str());
#endif
}


char _getINT(const char* &X)
{
    unsigned char x = 0;
    while (true)
    {
        x += *X++ - '0';
        if (*X == '.' || *X == '\0')
            return x;
        x *= 10;
    }
    return x;
}
uint32_t VARBOX::getVersion(const char* A)
{
    uint32_t buffer = _getINT(A);
    (buffer <<= 8) |= _getINT(++A);
    (buffer <<= 8) |= _getINT(++A);
    return buffer;
}

void VARBOX::MSG(const char *text, const char* title, QMessageBox::StandardButtons s)
{
    QMessageBox message(QMessageBox::Information, title, text, s, NULL);
    QFile qss(QStringLiteral(":/qss/dialog_style.qss"));
    qss.open(QFile::ReadOnly);
    message.setStyleSheet(qss.readAll());
    qss.close();
    message.exec();
}

