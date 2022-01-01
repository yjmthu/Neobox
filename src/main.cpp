#include <QScreen>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QTextCodec>

#include "funcbox.h"
#include "form.h"

int main(int argc, char* argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif
    VARBOX::HMutex = CreateMutexW(NULL, FALSE, L"__SpeedBox__");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return 0;
    QApplication a(argc, argv);                                            //创建app
    a.setQuitOnLastWindowClosed(false);                                    //防止QFileDialog被当成最主窗口导致程序结束
    QDir::setCurrent(a.applicationDirPath());
    QFile::remove("Speed_Box_Updater.exe");
    QScreen* screen = QGuiApplication::primaryScreen();                    //获取屏幕分辨率
    QRect geo = screen->geometry();
    VarBox = new VARBOX(geo.width(), geo.height());
    switch (a.exec()) {
    case RETCODE_RESTART:                                   //如果收到重启常数
        CloseHandle(VARBOX::HMutex);
        QProcess::startDetached(a.applicationFilePath(), QStringList());   //重启程序
        break;
    case RETCODE_UPDATE:
        CloseHandle(VARBOX::HMutex);
        QProcess::startDetached(QDir(a.applicationDirPath()).absoluteFilePath("Speed_Box_Updater.exe"), QStringList());   //重启程序
        break;
    case RETCODE_ERROR_EXIT:
        CloseHandle(VARBOX::HMutex);
        QMessageBox::critical(nullptr, "出错", "程序遇到不能处理的错误，必须立即退出！");
    default:
        CloseHandle(VARBOX::HMutex);
        break;
    }
    delete VarBox;
    return 0;
}
