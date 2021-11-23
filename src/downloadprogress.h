#ifndef DOWNLOADPROGRESS_H
#define DOWNLOADPROGRESS_H

#include <QDialog>
#include <QUrl>

namespace Ui {
class DownloadProgress;
}
class QNetworkAccessManager;

class DownloadProgress : public QDialog
{
    Q_OBJECT
signals:
    void message(const char* msg, int timeout);
protected:
    void closeEvent(QCloseEvent *event);
public:
    explicit DownloadProgress(const QString& zipfile, const QUrl& url1, const QUrl& url2,QWidget *parent = nullptr);
    ~DownloadProgress();
    const QString zipfile;

private:
    Ui::DownloadProgress *ui = nullptr;
    const QUrl url1, url2;
    QNetworkAccessManager *mgr1 = nullptr, *mgr2 = nullptr;
    bool succeed = true;
public slots:
    void setZip(qint64, qint64);
    void setExe(qint64, qint64);
};

#endif // DOWNLOADPROGRESS_H
