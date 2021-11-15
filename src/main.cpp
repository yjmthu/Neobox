#include <QScreen>
#include <QProcess>
#include <QFile>
#include <QDir>

#include "funcbox.h"
#include "form.h"

int main(int argc, char* argv[])
{
    VARBOX::HMutex = CreateMutexW(NULL, FALSE, L"__SpeedBox__");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return 0;
    QApplication a(argc, argv);                                            //创建app
    a.setQuitOnLastWindowClosed(false);                                    //防止QFileDialog被当成最主窗口导致程序结束
    if (QFile::exists(a.applicationDirPath() + "/update.exe"))
        QFile::remove(a.applicationDirPath() + "/update.exe");
    QScreen* screen = QGuiApplication::primaryScreen();                    //获取屏幕分辨率
    QRect geo = screen->geometry();
    VarBox = new VARBOX(geo.width(), geo.height());
    switch (a.exec()) {
    case RETCODE_RESTART:                                   //如果收到重启常数
        CloseHandle(VARBOX::HMutex);
        QProcess::startDetached(a.applicationFilePath());   //重启程序
        break;
    case RETCODE_UPDATE:
        CloseHandle(VARBOX::HMutex);
        QProcess::startDetached(a.applicationDirPath().replace("/", "\\") + "\\update.exe");   //重启程序
        break;
    case RETCODE_ERROR_EXIT:
        CloseHandle(VARBOX::HMutex);
        MessageBoxW(NULL, L"运行程序遇到不能处理的错误，必须立即退出！", L"出错", 0);
    default:
        CloseHandle(VARBOX::HMutex);
        break;
    }
    delete VarBox;
    return 0;
}
