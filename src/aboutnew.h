#ifndef ABOUTNEW_H
#define ABOUTNEW_H

#include <QDialog>

class AboutNew : public QDialog
{
    Q_OBJECT
protected:
    void showEvent(QShowEvent *event);
public:
    explicit AboutNew(QWidget *parent = nullptr);
    ~AboutNew();
private:
    class QPlainTextEdit* m_pTextEdit { nullptr };
    class QProgressBar* m_pProgressBar { nullptr };
    class QNetworkAccessManager *m_pNetMgr { nullptr };
    class YJson* m_pJson {nullptr}, *m_pJsWin { nullptr };
    bool DownloadData(const QString& url, const QString& path);
    bool NeedUpdater(YJson* js);
    bool NeedZip();
    bool DownloadJson();
    bool DownloadExe(YJson* js);
    bool DownloadUpdater(YJson* js);
    bool DownloadZip(YJson* js);
private slots:
    void GetUpdate();
signals:

};

#endif // ABOUTNEW_H
