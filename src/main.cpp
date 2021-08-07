#include <QScreen>
#include <QProcess>

#include "funcbox.h"
#include "form.h"

int main(int argc, char* argv[])
{
    VarBox.HMutex = CreateMutex(NULL, FALSE, TEXT("__SpeedBox2__"));
    if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;
    QApplication a(argc, argv);                                            //创建app
    a.setQuitOnLastWindowClosed(false);                                    //防止QFileDialog被当成最主窗口导致程序结束
    QScreen* screen = QGuiApplication::primaryScreen();                    //获取屏幕分辨率
    QRect geo = screen->geometry();
    VarBox.ScreenWidth = geo.width(); VarBox.ScreenHeight = geo.height();
    Form w;                                                               //创建悬浮窗，并且传入屏幕数据和当前路径。
    APPBARDATA abd;
    memset(&abd, 0, sizeof(abd));
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = HWND(w.winId());
    abd.uCallbackMessage = MSG_APPBAR_MSGID;
    SHAppBarMessage(ABM_NEW, &abd);
    w.show();                                                              //显示悬浮窗
    if (a.exec() == RETCODE_RESTART)                                              //如果收到重启常数
    {
        CloseHandle(VarBox.HMutex);
        QProcess::startDetached(a.applicationFilePath());   //重启程序
        return 0;
    }
    CloseHandle(VarBox.HMutex);
    return 0;
}
