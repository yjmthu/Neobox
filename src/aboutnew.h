#ifndef ABOUTNEW_H
#define ABOUTNEW_H

#include <QWidget>

class AboutNew : public QWidget
{
    Q_OBJECT
public:
    explicit AboutNew(QWidget *parent = nullptr);
private:
    class YJson* DownloadJson();
    bool DownloadZip();

signals:

};

#endif // ABOUTNEW_H
