#include <speedapp.h>
#include <speedbox.h>
#include <speedmenu.h>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QQmlApplicationEngine>
#include <QSharedMemory>
#include <QStandardPaths>
#include <appcode.hpp>
#include <iostream>

void ShowMessage(const std::u8string &title, const std::u8string &text, int type)
{
    switch (type)
    {
    case 0:
        QMessageBox::information(
            nullptr, QString::fromUtf8(reinterpret_cast<const char *>(title.data()), static_cast<int>(title.size())),
            QString::fromUtf8(reinterpret_cast<const char *>(text.data()), static_cast<int>(text.size())));
        break;
    case 1:
        QMessageBox::warning(
            nullptr, QString::fromUtf8(reinterpret_cast<const char *>(title.data()), static_cast<int>(title.size())),
            QString::fromUtf8(reinterpret_cast<const char *>(text.data()), static_cast<int>(text.size())));
        break;
    case 2:
        QMessageBox::critical(
            nullptr, QString::fromUtf8(reinterpret_cast<const char *>(title.data()), static_cast<int>(title.size())),
            QString::fromUtf8(reinterpret_cast<const char *>(text.data()), static_cast<int>(text.size())));
        break;
    default:
        break;
    }
}

inline void DoExit(ExitCode m_ExitCode)
{
    switch (m_ExitCode)
    {
    case ExitCode::RETCODE_NORMAL:
        break;
    case ExitCode::RETCODE_RESTART:
        QProcess::startDetached(qApp->applicationFilePath(), QStringList({"-r"})); // 重启程序
        break;
    default:
        break;
    }
}

int main(int argc, char *argv[])
{
    QSharedMemory m_SharedMemory;
    m_SharedMemory.setKey(QStringLiteral("__NeoboxMutex__"));
    if (m_SharedMemory.attach() || !m_SharedMemory.create(1)) // 防止多次打开
        return 0;
        // 控制图片缩放质量
#if (QT_VERSION_MAJOR < 6)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false); // 防止QFileDialog被当成最主窗口导致程序结束
    VarBox box;
    QQmlApplicationEngine engine;
    qmlRegisterType<SpeedMenu>("Neobox", 1, 0, "SpeedMenu");
    qmlRegisterType<SpeedBox>("Neobox", 1, 0, "SpeedBox");
    engine.load(QUrl(QStringLiteral("qrc:/qmls/FloatingWindow.qml")));
    int ret = a.exec();
    m_SharedMemory.detach();
    DoExit(static_cast<ExitCode>(ret));
    return 0;
}
