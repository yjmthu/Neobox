#include "aboutnew.h"

#include <QApplication>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QProgressBar>
#include <QTimer>
#include <QDir>

#include "funcbox.h"
#include "yjson.h"
#include "globalfn.h"

inline QString getIntToVersion(uint32_t ver)
{
    return QStringLiteral("%1.%2.%3").arg(ver >> 16).arg((ver >> 8) & 0XFF).arg(ver & 0XFF);
}

constexpr bool Is64BitOS()
{
    return sizeof(void*) == 8;
}

void AboutNew::showEvent(QShowEvent *event)
{
    move((VarBox->m_dScreenWidth-width())/2, (VarBox->m_dScreenHeight-height())/2);
    QTimer::singleShot(50, this, &AboutNew::GetUpdate);
    event->accept();
}

AboutNew::AboutNew(QWidget *parent)
    : QDialog{parent}, m_pTextEdit {new QPlainTextEdit(this)}, m_pProgressBar { new QProgressBar(this) },
      m_pNetMgr(new QNetworkAccessManager(this))
{
    setWindowTitle(QStringLiteral("SpeedBox更新"));
    QVBoxLayout* lay {new QVBoxLayout(this)};
    lay->addWidget(m_pTextEdit);
    lay->addWidget(m_pProgressBar);
    m_pTextEdit->setReadOnly(true);
    m_pTextEdit->setPlainText(QStringLiteral("检查更新中..."));
    m_pProgressBar->setMaximum(100);
    m_pProgressBar->setMinimum(0);
}

AboutNew::~AboutNew()
{
    delete m_pJson;
}

void AboutNew::GetUpdate()
{
    if (!DownloadJson()) {
        m_pTextEdit->appendPlainText(QStringLiteral("检查更新失败！"));
    } else try {
#ifdef Q_OS_WIN32
#if __SIZEOF_POINTER__ == 4
        m_pJsWin = m_pJson->find("Windows")->find("x86");
#elif __SIZEOF_POINTER__ == 8
        m_pJsWin = m_pJson->find("Windows")->find("x64");
#endif
        YJson *m_pJsBoxVersion = m_pJsWin->find("SpeedBox");

        uint32_t m_nNewBoxVersion = GlobalFn::getVersion(m_pJsBoxVersion->find(0)->getValueInt(),
                    m_pJsBoxVersion->find(1)->getValueInt(),m_pJsBoxVersion->find(2)->getValueInt());
        if (m_nNewBoxVersion > VARBOX::m_dVersion)
        {
            m_pTextEdit->appendPlainText(QStringLiteral("当前版本:%1; 最新版本为%2.").arg(getIntToVersion(VARBOX::m_dVersion), getIntToVersion(m_nNewBoxVersion)));
            if (QMessageBox::question(this, QStringLiteral("提示"), QStringLiteral("是否要下载最新版本？")) == QMessageBox::StandardButton::Yes)
            {
                YJson *m_pJsFile = m_pJsWin->find("Files");
                if (NeedUpdater(m_pJsFile)) {
                    m_pTextEdit->appendPlainText(QStringLiteral("正在下载更新程序..."));
                    if (DownloadUpdater(m_pJsFile)) {
                        m_pTextEdit->appendPlainText(QStringLiteral("更新程序下载完成。"));
                    } else {
                        m_pTextEdit->appendPlainText(QStringLiteral("更新程序下载失败。"));
                        throw 0;
                    }
                }
                if (NeedZip()) {
                    m_pTextEdit->appendPlainText(QStringLiteral("正在下载压缩包..."));
                    if (DownloadZip(m_pJsFile)) {
                        m_pTextEdit->appendPlainText(QStringLiteral("压缩包下载完成。"));
                    } else {
                        m_pTextEdit->appendPlainText(QStringLiteral("压缩包下载失败。"));
                        throw 0;
                    }
                } else {
                    m_pTextEdit->appendPlainText(QStringLiteral("正在下载主程序..."));
                    if (DownloadExe(m_pJsFile)) {
                        m_pTextEdit->appendPlainText(QStringLiteral("主程序下载完成。"));
                    } else {
                        m_pTextEdit->appendPlainText(QStringLiteral("主程序下载失败。"));
                        throw 0;
                    }
                }
                if (QMessageBox::question(this, QStringLiteral("提示"), QStringLiteral("下载完成，是否现在运行更新程序？")) == QMessageBox::Yes)
                {
                    qApp->exit(VARBOX::RETCODE_UPDATE);
                } else {
                    m_pTextEdit->appendPlainText(QStringLiteral("成功取消下载更新。"));
                }
            } else {
                m_pTextEdit->appendPlainText(QStringLiteral("成功取消下载更新。"));
            }
        } else {
            m_pTextEdit->appendPlainText(QStringLiteral("当版本%1已经是最新！").arg(getIntToVersion(VARBOX::m_dVersion)));
        }
#elif defined (Q_OS_LINUX)
#endif
    } catch (...) {
        m_pTextEdit->appendPlainText(QStringLiteral("出现未知错误，无法正常更新。"));
    }
}

bool AboutNew::DownloadData(const QString &url, const QString &path, bool redirected)
{
    QEventLoop loop(this);
    QNetworkRequest netReq = QNetworkRequest(QUrl(url));
    if (redirected)
        netReq.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    QNetworkReply *m_pReply = m_pNetMgr->get(netReq);
    connect(m_pReply, &QNetworkReply::downloadProgress, this, [this](qint64 a, qint64 b){m_pProgressBar->setValue(100*a/b);});
    connect(m_pReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (m_pReply->error() == QNetworkReply::NoError) {
        if (QFile::exists(path))
            QFile::remove(path);
        QFile file(path);
        if (file.open(QIODevice::WriteOnly))
        {
            const QByteArray& bytes = m_pReply->readAll();
            m_pTextEdit->appendPlainText(QStringLiteral("下载完成，文件大小：%1。").arg(GlobalFn::bytes_to_string(bytes.size())));
            file.write(bytes);
            file.close();
        }
        m_pReply->deleteLater();
        return QFile::exists(path);
    } else {
        m_pTextEdit->appendPlainText(QStringLiteral("下载出错。"));
        m_pReply->deleteLater();
        return false;
    }
}

bool AboutNew::NeedUpdater(YJson *js)
{
    if (QFile::exists("./update.exe")) {
        return js->find("Updater")->getChild()->getType();
    } else {
        return true;
    }
}

bool AboutNew::NeedZip()
{
    YJson *m_pJsQtVersion = m_pJsWin->find("Qt");
    uint32_t m_nNewQtVersion = GlobalFn::getVersion(m_pJsQtVersion->find(0)->getValueInt(),
                m_pJsQtVersion->find(1)->getValueInt(),m_pJsQtVersion->find(2)->getValueInt());
    if (QT_VERSION != m_nNewQtVersion) {
#if __SIZEOF_POINTER__ == 4
        QFile file(QStringLiteral(":/json/Directory_x86.json"));
#elif __SIZEOF_POINTER__ == 8
        QFile file(QStringLiteral(":/json/Directory_x64.json"));
#endif
        file.open(QFile::ReadOnly);
        YJson m_jsOld(file.readAll());
        file.close();
        YJson json(YJson::Object);
        json.append(QDir::toNativeSeparators(qApp->applicationDirPath()).toStdString(), "path");
        json.append("zip", "type");
        json.append(m_jsOld, "binary");
        json.append(25, "count");
        json.toFile("./profile.json", YJson::UTF8);
        return true;
    }
#if __SIZEOF_POINTER__ == 4
    QFile file(QStringLiteral(":/json/Directory_x86.json"));
#elif __SIZEOF_POINTER__ == 8
    QFile file(QStringLiteral(":/json/Directory_x64.json"));
#endif
    file.open(QFile::ReadOnly);
    YJson m_jsOld(file.readAll());
    file.close();

    YJson json(YJson::Object);
    json.append(QDir::toNativeSeparators(qApp->applicationDirPath()).toStdString(), "path");
    if (m_pJsWin->find("Struct")->isSameTo(m_jsOld)) {
        json.append("exe", "type");
        json.toFile("./profile.json", YJson::UTF8);
        return false;
    } else {
        json.append("zip", "type");
        json.append(m_jsOld, "binary");
        json.append(25, "count");
        json.toFile("./profile.json", YJson::UTF8);
        return true;
    }
}

bool AboutNew::DownloadJson()
{
    QEventLoop loop(this);
#if 1
    auto m_reply = m_pNetMgr->get(QNetworkRequest(QUrl("https://gitee.com/yjmthu/Speed-Box/raw/main/update/newinfo.json")));
#else
    auto m_reply = m_pNetMgr->get(QNetworkRequest(QUrl("https://raw.githubusercontent.com/yjmthu/Speed-Box/main/update/newinfo.json")));
#endif
    connect(m_reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    if (m_reply->error() == QNetworkReply::NoError) {
        try {
            m_pJson = new YJson(m_reply->readAll());
        } catch (...) {
            m_pJson = nullptr;
        }
    } else {
        m_pJson = nullptr;
    }
    m_reply->deleteLater();
    return m_pJson;
}

bool AboutNew::DownloadExe(YJson* js)
{
    QString url = js->find("Exe")->find(1)->getValueString();
    m_pTextEdit->appendPlainText("下载地址："+url);
    return DownloadData(url, "./SpeedBox.exe", true);
}

bool AboutNew::DownloadUpdater(YJson* js)
{
    QString url = js->find("Updater")->find(1)->getValueString();
    m_pTextEdit->appendPlainText("下载地址："+url);
    return DownloadData(url, "./update.exe", false);
}

bool AboutNew::DownloadZip(YJson* js)
{
    QString url = js->find("Zip")->find(1)->getValueString();
    m_pTextEdit->appendPlainText("下载地址："+url);
    return DownloadData(url, "./SpeedBox.zip", true);
}
