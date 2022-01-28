#include "downloadprogress.h"
#include "ui_downloadprogress.h"
#include "funcbox.h"

#include <QFile>
#include <QCloseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDir>

void DownloadProgress::closeEvent(QCloseEvent *event)
{
    if (!succeed)
    {
        if (QMessageBox::information(this, "提示", "正在下载中, 确认退出?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
            event->accept();
        else
            event->ignore();
    }
}

DownloadProgress::DownloadProgress(const QString& zipfile, const QUrl& url1, const QUrl& url2, QWidget *parent) :
    QDialog(parent),
    zipfile(std::move(zipfile)), ui(new Ui::DownloadProgress),
    url1(std::move(url1)), url2(std::move(url2)),
    mgr1(new QNetworkAccessManager), mgr2(new QNetworkAccessManager)
{
    ui->setupUi(this);
    connect(mgr2, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep2){
        succeed = true;
        QString temp_str_2 = QDir::toNativeSeparators(qApp->applicationDirPath()+"/Speed_Box_Updater.exe");
        if (QFile::exists(temp_str_2)) QFile::remove(temp_str_2);
        QFile file(temp_str_2);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(rep2->readAll());
            file.close();
            if (!file.size())
            {
                qout << "文件大小为0";
                QFile::remove(temp_str_2);
            } else {
                if (QMessageBox::information(this, "提示", "更新已经下载完成，点击确认重启软件后完成更新！", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                {
                    qApp->exit(RETCODE_UPDATE);
                } else {
                    emit message("成功取消更新。", 700);
                }
            }
        }
        close();
    });
    connect(mgr1, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep1){
        succeed = true;
        if (rep1->error() != QNetworkReply::NoError)
        {
            close();
            emit message("下载zip文件出错!", 2000);
            return ;
        }
        if (QFile::exists(zipfile)) QFile::remove(zipfile);
        QFile file(zipfile);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(rep1->readAll());
            file.close();
            if (!file.size())
            {
                qout << "文件大小为0";
                QFile::remove(zipfile);
                QMessageBox::information(this, "提示", "更新已经下载完成，点击确认重启软件后完成更新！", QMessageBox::Yes);
                close();
            } else {
                succeed = false;
                connect(mgr2->get(QNetworkRequest(url2)), &QNetworkReply::downloadProgress, this, &DownloadProgress::setExe);
                qout << "下载第二个文件";
            }
            qout << file.size();
        }
    });
    succeed = false;
    connect(mgr1->get(QNetworkRequest(url1)), &QNetworkReply::downloadProgress, this, &DownloadProgress::setZip);
}

DownloadProgress::~DownloadProgress()
{
    delete ui;
    delete mgr2;
    delete mgr1;
}

void DownloadProgress::setZip(qint64 a, qint64 b)
{
    ui->progressBar->setValue(100 * a / b);
}

void DownloadProgress::setExe(qint64 a, qint64 b)
{
    ui->progressBar_2->setValue(100 * a / b);
}
