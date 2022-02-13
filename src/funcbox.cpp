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
#include "roundclock.h"
#include "windowposition.h"
#include "globalfn.h"


constexpr char VARBOX::m_dVersion[];
VARBOX* VarBox = nullptr;


VARBOX::VARBOX(int w, int h):
    QObject(nullptr),
    m_dSystemVersion(SystemFunctions::getWindowsVersion()),
    m_dScreenWidth(w), m_dScreenHeight(h),
    m_pWindowPosition(WindowPosition::fromFile())
{
    qout << "VarBox构造函数开始。";
    initProJob();
    initFile();
    initChildren();
    initBehavior();
    qout << "VarBox构造函数结束。";
}

VARBOX::~VARBOX()
{
    qout << "结构体析构中~";
    delete m_pWindowPosition;
    delete m_pSystemTrayIcon;
    delete m_sClock;
    delete m_rClock;
    delete m_pNote;
    delete m_pForm;
    delete m_pWallpaper;
    delete m_pDialog;
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
    QDir dir;
    const QByteArray& x { GlobalFn::readOneSet<QByteArray>(QStringLiteral("SpeedBox"), QStringLiteral("Version"), "21.8.0").toByteArray() };
    qout << "文件存在，版本信息为" << x;
    if (GlobalFn::getVersion(x) < GlobalFn::getVersion("21.8.1"))
    {
        qout << "ini文件过期，将其删除。";
        QFile::remove(file);
        GlobalFn::saveOneSet<QString>(QStringLiteral("SpeedBox"), QStringLiteral("Version"), m_dVersion);
    } else {
        qout << "开始读取设置。";
        QSettings *IniRead = new QSettings(file, QSettings::IniFormat);
        IniRead->setIniCodec(QTextCodec::codecForName("UTF-8"));
        IniRead->beginGroup(QStringLiteral("SpeedBox"));
        IniRead->setValue(QStringLiteral("Version"), m_dVersion);
        IniRead->endGroup();

        IniRead->beginGroup(QStringLiteral("Translate"));
        m_bAutoHide = IniRead->value(QStringLiteral("AutoHide"), m_bAutoHide).toBool();
        m_bEnableTranslater = IniRead->value(QStringLiteral("EnableTranslater"), m_bEnableTranslater).toBool();
        IniRead->endGroup();
        qout << "读取翻译信息完毕";
        IniRead->beginGroup(QStringLiteral("Dirs"));
        m_pathToOpen = IniRead->value(QStringLiteral("OpenDir"), QDir::toNativeSeparators(qApp->applicationDirPath())).toString();
        IniRead->endGroup();
        qout << "读取路径信息完毕";
        IniRead->beginGroup(QStringLiteral("UI"));
        unsigned safeEnum = IniRead->value(QStringLiteral("ColorTheme"), static_cast<int>(Dialog::curTheme)).toInt();
        if (safeEnum > 7) safeEnum = 0;
        Dialog::curTheme = static_cast<Dialog::Theme>(safeEnum);
        m_bTuoPanIcon = IniRead->value(QStringLiteral("TuoPanIcon"), m_bTuoPanIcon).toBool();
        IniRead->endGroup();
        IniRead->beginGroup(QStringLiteral("Apps"));
        m_bMarkdownNote = IniRead->value(QStringLiteral("MarkdownNote"), m_bMarkdownNote).toBool();
        m_bSquareClock = IniRead->value(QStringLiteral("SquareClock"), m_bSquareClock).toBool();
        m_bRoundClock = IniRead->value(QStringLiteral("RoundClock"), m_bRoundClock).toBool();
        m_bEnableUSBhelper = IniRead->value(QStringLiteral("UsbHelper"), m_bEnableUSBhelper).toBool();
        IniRead->endGroup();
        delete IniRead;
        qout << "读取设置完毕。";
    }
    if (!dir.exists(m_pathToOpen) && !dir.mkdir(m_pathToOpen))
    {
        m_pathToOpen = QDir::toNativeSeparators(qApp->applicationDirPath());
        GlobalFn::saveOneSet(QStringLiteral("Dirs"), QStringLiteral("OpenDir"), m_pathToOpen);
    }
    const std::string apifile("WallpaperApi.json");
    if (!QFile::exists(QString::fromStdString(apifile)))
    {
        qout << "文件不存在!";
        QString picfolder { QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)) + "/桌面壁纸" };
        dir.mkdir(picfolder);
        dir.cd(picfolder);
        QFile::copy(QStringLiteral(":/json/WallpaperApi.json"), QString::fromStdString(apifile+".temp"));
        const QStringList lst {"最热壁纸", "风景壁纸", "动漫壁纸", "极简壁纸", "随机壁纸", "鬼刀壁纸", "必应壁纸"};
        for (const auto&c: lst)
            dir.mkdir(c);
        QStringList::ConstIterator iter = lst.begin();
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
        QFile::remove(QString::fromStdString(apifile+".temp"));
    }
}

void VARBOX::initChildren()
{
    qout << "开始创建基本成员";
    m_pWallpaper = new Wallpaper;                           // 创建壁纸更换对象
    Form* form = new Form;
#ifdef Q_OS_WIN
    static APPBARDATA abd { 0,0,0,0,{0,0,0,0},0 };
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = HWND(form->winId());
    abd.uCallbackMessage = MSG_APPBAR_MSGID;
    SHAppBarMessage(ABM_NEW, &abd);
#endif
    QObject::connect(QGuiApplication::primaryScreen(), &QScreen::geometryChanged, form, [this, form](const QRect&rect){
        this->m_pForm->move(this->m_pForm->pos().x() * rect.width() / VarBox->m_dScreenWidth, this->m_pForm->pos().y() * rect.height() / VarBox->m_dScreenHeight);
        *const_cast<int *>(&m_dScreenWidth) = rect.width();
        *const_cast<int *>(&m_dScreenHeight) = rect.height();
        form->keepInScreen();
    });
    createMarkdown(m_bMarkdownNote);
    createTrayIcon(m_bTuoPanIcon);
    createSquareClock(m_bSquareClock);
    createRoundClock(m_bRoundClock);
}

void VARBOX::initBehavior()
{
    m_pForm->show();                                                              //显示悬浮窗
    m_pForm->keepInScreen();
}

void VARBOX::createTrayIcon(bool create)
{
    if ((m_bTuoPanIcon = create))
    {
        m_pSystemTrayIcon = new QSystemTrayIcon;
        m_pSystemTrayIcon->setIcon(QIcon(QStringLiteral(":/icons/speedbox.ico")));
        QMenu *m_trayMenu = new QMenu;
        QAction *initAct = m_trayMenu->addAction(QStringLiteral("复原"));
        QAction *quitAct = m_trayMenu->addAction(QStringLiteral("退出"));
        connect(quitAct, &QAction::triggered, qApp, &QApplication::quit);
        connect(initAct, &QAction::triggered, m_pForm, [=](){m_pForm->move(100, 100);m_pForm->saveBoxPos();});
        m_pSystemTrayIcon->setContextMenu(m_trayMenu);
        m_pSystemTrayIcon->show();
    } else {
        delete m_pSystemTrayIcon;
        m_pSystemTrayIcon = nullptr;
    }
    GlobalFn::saveOneSet<bool>(QStringLiteral("UI"), QStringLiteral("TuoPanIcon"), create);
}

void VARBOX::createMarkdown(bool create)
{
    if ((m_bMarkdownNote = create)) {
        m_pNote = new MarkdownNote;
    } else {
        delete m_pNote;
        m_pNote = nullptr;
    }
    GlobalFn::saveOneSet<bool>(QStringLiteral("Apps"), QStringLiteral("MarkdownNote"), create);
}

void VARBOX::createSquareClock(bool create)
{
    if ((m_bSquareClock = create)) {
        m_sClock = new SquareClock;
        m_sClock->show();
    } else {
        delete m_sClock;
        m_sClock = nullptr;
    }
    GlobalFn::saveOneSet<bool>(QStringLiteral("Apps"), QStringLiteral("SquareClock"), create);
}

void VARBOX::createRoundClock(bool create)
{
    if ((m_bRoundClock = create)) {
        m_rClock = new RoundClock;
        m_rClock->show();
    } else {
        delete m_rClock;
        m_rClock = nullptr;
    }
    GlobalFn::saveOneSet<bool>(QStringLiteral("Apps"), QStringLiteral("RoundClock"), create);
}

void VARBOX::initProJob()
{
    VarBox = this;
#if defined (Q_OS_WIN32)
    switch (m_dSystemVersion) {
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

