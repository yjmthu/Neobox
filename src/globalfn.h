#ifndef GLOBALFN_H
#define GLOBALFN_H

#include <QMessageBox>
#include <QSettings>
#include <QTextCodec>

#include <string>

namespace GlobalFn {
template<class _Ty>
static void saveOneSet(const QString& group, const QString& key, _Ty val) {
    QSettings IniRead(QStringLiteral("SpeedBox.ini"), QSettings::IniFormat);
    IniRead.setIniCodec(QTextCodec::codecForName("UTF-8"));
    IniRead.beginGroup(group);
    IniRead.setValue(key, val);
    IniRead.endGroup();
}
template<class _Ty>
static QVariant readOneSet(const QString& group, const QString& key, _Ty val) {
    QSettings IniRead(QStringLiteral("SpeedBox.ini"), QSettings::IniFormat);
    IniRead.setIniCodec(QTextCodec::codecForName("UTF-8"));
    IniRead.beginGroup(group);
    QVariant var(IniRead.value(key, val));
    IniRead.endGroup();
    return var;
}
#if defined (Q_OS_WIN32)
std::string ansiToUtf8(const std::string& strAnsi);
std::string utf8ToAnsi(const std::string& strUtf8);
#endif
const char* getFileName(const char* file_path);
uint32_t getVersion(const char* A);
FILE* readFile(const std::string& filePath);
void msgBox(const char* text, const char* title="提示", QMessageBox::StandardButtons buttons=QMessageBox::Ok);
QByteArray runCmd(const QString & program, const QStringList& argument, short line);
void openDirectory(const QString& dir);
}

#endif // GLOBALFN_H
