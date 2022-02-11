#include "globalfn.h"

#include <QDir>
#include <QProcess>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

namespace GlobalFn {

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

uint32_t getVersion(const char* A)
{
    uint32_t buffer = _getINT(A);
    (buffer <<= 8) |= _getINT(++A);
    (buffer <<= 8) |= _getINT(++A);
    return buffer;
}

QByteArray runCmd(const QString& program, const QStringList& argument, short line)
{
    QProcess process;
    process.setProgram(program);
    process.setArguments(argument);
    QObject::connect(&process, &QProcess::errorOccurred, [&](){ line=0; });
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

#if defined (Q_OS_WIN32)

std::string ansiToUtf8(const std::string& strAnsi)//传入的strAnsi是GBK编码
{
    //gbk转unicode
    int len = MultiByteToWideChar(CP_ACP, 0, strAnsi.c_str(), -1, NULL, 0);
    wchar_t *strUnicode = new wchar_t[len];
    wmemset(strUnicode, 0, len);
    MultiByteToWideChar(CP_ACP, 0, strAnsi.c_str(), -1, strUnicode, len);

    //unicode转UTF-8
    len = WideCharToMultiByte(CP_UTF8, 0, strUnicode, -1, NULL, 0, NULL, NULL);
    //char * strUtf8 = new char[len];
    std::string strTemp(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, strUnicode, -1, &strTemp.front(), len, NULL, NULL);

    delete[] strUnicode;
    return strTemp;
}

std::string utf8ToAnsi(const std::string& strUtf8)//传入的strUtf8是UTF-8编码
{
    //UTF-8转unicode
    int len = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0);
    wchar_t * strUnicode = new wchar_t[len];//len = 2
    wmemset(strUnicode, 0, len);
    MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, strUnicode, len);

    //unicode转gbk
    len = WideCharToMultiByte(CP_ACP, 0, strUnicode, -1, NULL, 0, NULL, NULL);
    std::string strTemp(len, 0);//此时的strTemp是GBK编码
    WideCharToMultiByte(CP_ACP,0, strUnicode, -1, &strTemp.front(), len, NULL, NULL);

    delete[] strUnicode;
    return strTemp;
}
#endif

void openDirectory(const QString& dir)
{
#ifdef Q_OS_WIN
    auto path = utf8ToAnsi(QDir::toNativeSeparators(dir).toStdString());
    ShellExecuteA(NULL, "open", "explorer.exe", path.c_str(), NULL, SW_SHOWNORMAL);
#elif defined Q_OS_LINUX
    system(("xdg-open \"" + dir + "\"").toStdString().c_str());
#endif
}

FILE* readFile(const std::string& filePath)
{
#if defined (Q_OS_WIN32)
    const std::string ansiString(utf8ToAnsi(filePath));
    return fopen(ansiString.c_str(), "rb");
#elif defined (Q_OS_LINUX)
    return fopen(filePath.c_str(), "rb");
#endif
}

void msgBox(const char *text, const char* title, QMessageBox::StandardButtons s)
{
    QMessageBox message(QMessageBox::Information, title, text, s, NULL);
    QFile qss(QStringLiteral(":/qss/dialog_style.qss"));
    qss.open(QFile::ReadOnly);
    message.setStyleSheet(qss.readAll());
    qss.close();
    message.exec();
}

const char* getFileName(const char* file_path)
{
    const char* ptr = file_path;
    while (*++ptr);
    while (!strchr("\\/", *--ptr));
    return ++ptr;
}
}
