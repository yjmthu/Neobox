#include <QScreen>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QTextCodec>
#include <QSharedMemory>
#include <QApplication>
#include <QStandardPaths>

#include "funcbox.h"
#include "form.h"

int main(int argc, char* argv[])
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#endif
    QSharedMemory shared_memory;
    shared_memory.setKey(QStringLiteral("__SpeedBox__"));
    if(shared_memory.attach() || !shared_memory.create(1))                                                 //防止多次打开
        return 0;
    QApplication a(argc, argv);                                            //创建app
    a.setQuitOnLastWindowClosed(false);                                    //防止QFileDialog被当成最主窗口导致程序结束
    QDir::setCurrent(a.applicationDirPath());
    QFile::remove(QStringLiteral("Speed_Box_Updater.exe"));
#if defined (Q_OS_WIN32)
    QDir dir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
#elif defined(Q_OS_LINUX)
    QString data_path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +  "/.config/SpeedBox";
#endif
    dir.mkpath(QStringLiteral("./AppData/Local/SpeedBox"));
    dir.cd(QStringLiteral("./AppData/Local/SpeedBox"));
    QDir::setCurrent(dir.absolutePath());
    QScreen* screen = QGuiApplication::primaryScreen();                    //获取屏幕分辨率
    QRect geo = screen->geometry();
    VarBox = new VARBOX(geo.width(), geo.height());
    int exit_code = a.exec();
    shared_memory.detach();
    switch (exit_code) {
    case VARBOX::RETCODE_RESTART:                                   //如果收到重启常数
        QProcess::startDetached(a.applicationFilePath(), QStringList());   //重启程序
        break;
    case VARBOX::RETCODE_UPDATE:
        QProcess::startDetached(QDir(a.applicationDirPath()).absoluteFilePath("Speed_Box_Updater.exe"), QStringList());   //重启程序
        break;
    case VARBOX::RETCODE_ERROR_EXIT:
        QMessageBox::critical(nullptr, "出错", "程序遇到不能处理的错误，必须立即退出！");
    default:
        break;
    }
    delete VarBox;
    return 0;
}
