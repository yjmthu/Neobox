#include "aboutnew.h"

#include <QApplication>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QProgressBar>

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
    GetUpdate();
    event->accept();
}

AboutNew::AboutNew(QWidget *parent)
    : QDialog{parent}, m_pTextEdit {new QPlainTextEdit(this)}, m_pProgressBar { new QProgressBar(this) },
      m_pNetMgr(new QNetworkAccessManager(this))
{
    setWindowTitle(QStringLiteral("SpeedBox更新"));
    setAttribute(Qt::WA_DeleteOnClose, true);
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
        YJson *m_pJsWin = m_pJson->find("Windows")->find(Is64BitOS()? "x64": "x86");
        YJson *m_pJsBoxVersion = m_pJsWin->find("SpeedBox");
        YJson *m_pJsQtVersion = m_pJsWin->find("Qt");
        uint32_t m_nNewBoxVersion = GlobalFn::getVersion(m_pJsBoxVersion->find(0)->getValueInt(),
                    m_pJsBoxVersion->find(1)->getValueInt(),m_pJsBoxVersion->find(2)->getValueInt());
        uint32_t m_nNewQtVersion = GlobalFn::getVersion(m_pJsQtVersion->find(0)->getValueInt(),
                    m_pJsQtVersion->find(1)->getValueInt(),m_pJsQtVersion->find(2)->getValueInt());
        if (m_nNewBoxVersion > VARBOX::m_dVersion)
        {
            m_pTextEdit->appendPlainText(QStringLiteral("当前版本:%1; 最新版本为%2.").arg(getIntToVersion(VARBOX::m_dVersion), getIntToVersion(m_nNewBoxVersion)));
            if (QMessageBox::question(this, QStringLiteral("提示"), QStringLiteral("是否要下载最新版本？")) == QMessageBox::StandardButton::Yes)
            {
                YJson *m_pJsFile = m_pJsWin->find("Files");
                if (QT_VERSION != getIntToVersion(m_nNewQtVersion)) {
                    YJson *m_pJsUpdater = m_pJsFile->find("Updater");
                    if (m_pJsUpdater->getChild()->getType() == YJson::False && QFile::exists(qApp->applicationDirPath()+"/update.exe"))
                    {
                        DownloadUpdater();
                    } else {
                        DownloadZip();
                    }
                } else {
                    YJson *m_pJsUpdater = m_pJsFile->find("Updater");
                    if (m_pJsUpdater->getChild()->getType() == YJson::False && QFile::exists(qApp->applicationDirPath()+"/update.exe"))
                    {
                        DownloadUpdater();
                    } else {
                        DownloadExe();
                    }
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
        //
    }
}

bool AboutNew::DownloadData(const QString &url, const QString &path)
{
    QEventLoop *loop = new QEventLoop(this);
    QNetworkReply *m_pReply = m_pNetMgr->get(QNetworkRequest(QUrl(url)));
    connect(m_pReply, &QIODevice::readyRead, this, [=]() {
        if (QFile::exists(path)) QFile::remove(path);
        QFile file(path);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(m_pReply->readAll());
            file.close();
        }
        loop->quit();
    });
    connect(m_pReply, &QNetworkReply::errorOccurred, loop, &QEventLoop::quit);
    connect(m_pReply, &QNetworkReply::sslErrors, loop, &QEventLoop::quit);
    connect(m_pReply, &QNetworkReply::downloadProgress, this, [this](qint64 a, qint64 b){m_pProgressBar->setValue(100*a/b);});
    loop->exec();
    m_pReply->deleteLater();
    loop->deleteLater();
    return QFile::exists(path);
}

bool AboutNew::NeedZip(YJson *js)
{

}

bool AboutNew::DownloadJson()
{
    QNetworkAccessManager* mgr { new QNetworkAccessManager };
    QEventLoop* loop { new QEventLoop };
    connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* reply) {
        if (reply->error() == QNetworkReply::NoError) {
            try {
                m_pJson = new YJson(reply->readAll());
            } catch (...) {
                m_pJson = nullptr;
            }
        } else {
            m_pJson = nullptr;
        }
        reply->deleteLater();
        loop->quit();
    });

    mgr->get(QNetworkRequest(QUrl("https://raw.githubusercontent.com/yjmthu/Speed-Box/main/update/newinfo.json")));
    loop->exec();
    loop->deleteLater();
    mgr->deleteLater();
    return m_pJson;
}

bool AboutNew::DownloadExe()
{
    m_pTextEdit->appendPlainText(QStringLiteral("成功取消下载更新。"));
    return false;
}

bool AboutNew::DownloadUpdater()
{
    return false;
}

bool AboutNew::DownloadZip()
{
    return false;
}
