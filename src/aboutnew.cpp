#include "aboutnew.h"

#include <QTextEdit>
#include <QVBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>

#include "yjson.h"

AboutNew::AboutNew(QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout* lay {new QVBoxLayout(this)};
    QTextEdit* textEdit {new QTextEdit(this)};
    lay->addWidget(textEdit);
    textEdit->setText("检查更新中...");
}

YJson *AboutNew::DownloadJson()
{
    static YJson *js;
    js = nullptr;
    QNetworkAccessManager* mgr { new QNetworkAccessManager };
    QEventLoop* loop { new QEventLoop };
    connect(mgr, &QNetworkAccessManager::finished, this, [loop](QNetworkReply* reply) {
        if (reply->error() == QNetworkReply::NoError) {
            try {
                js = new YJson(reply->readAll());
            } catch (...) {
                js = nullptr;
            }
        } else {
            js = nullptr;
        }
        loop->quit();
    });

    mgr->get(QNetworkRequest(QUrl("")));
    loop->exec();
    delete loop;
    delete mgr;
    return js;
}

bool AboutNew::DownloadZip()
{
    return false;
}
