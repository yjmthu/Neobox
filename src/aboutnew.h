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
    class YJson* m_pJson {nullptr};
    void GetUpdate();
    bool DownloadJson();
    bool DownloadExe();
    bool DownloadUpdater();
    bool DownloadZip();

signals:

};

#endif // ABOUTNEW_H
