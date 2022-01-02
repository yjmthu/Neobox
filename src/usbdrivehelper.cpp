#include "funcbox.h"
#include "usbdrivehelper.h"
#include "ui_usbdrivehelper.h"

#include <QThread>

#include <QHBoxLayout>
#include <stdio.h>
#include <math.h>
#include <QDesktopServices>

#include "YString.h"

inline QString bytes_to_string(DWORD64 size)
{
    DWORD64 r = 1;
    if (size < (r <<= 10))
        return QString("%1 B").arg(size);
    if (size < (r <<= 10))
        return QString("%1 KB").arg(QString::number((double)size / (r>>10), 'f', 1));
    if (size < (r <<= 10))
        return QString("%1 MB").arg(QString::number((double)size / (r>>10), 'f', 1));
    if (size < (r <<= 10))
        return QString("%1 GB").arg(QString::number((double)size / (r>>10), 'f', 1));
    if (size < (r <<= 10))
        return QString("%1 TB").arg(QString::number((double)size / (r>>10), 'f', 1));
    if (size < (r <<= 10))
        return QString("%1 PB").arg(QString::number((double)size / (r>>10), 'f', 1));
    return "Error";
}


#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
void USBdriveHelper::enterEvent(QEvent *event)
#else
void USBdriveHelper::enterEvent(QEnterEvent *event)
#endif
{
    widget->show();
    event->accept();
}

void USBdriveHelper::leaveEvent(QEvent *event)
{
    widget->hide();
    event->accept();
}

void USBdriveHelper::showEvent(QShowEvent *event)
{
    const QRect rect = geometry();
    setGeometry(VarBox->ScreenWidth - rect.width() - 20, 20, rect.width(), rect.height());
    widget->setGeometry(ui->frame->geometry());
    ui->frame->setGeometry(ui->frame->geometry());
    ui->progressBar->setMinimumWidth(rect.width());
    event->accept();
}

void USBdriveHelper::closeEvent(QCloseEvent *event)
{
    qout << "退出U盘管理";
    event->accept();
    deleteLater();
    emit appQuit();
}


USBdriveHelper::USBdriveHelper(char U, QWidget *parent) :
    SpeedWidget<QDialog>(parent),
    ui(new Ui::USBdriveHelper)
{
    // setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    ui->setupUi(this);
    initSpeedBox(this, &USBdriveHelper::showMinimized, &USBdriveHelper::close, false);
    ui->label_6->setText(QChar(U));

    //得出磁盘的可用空间
    DWORD dwTotalClusters;   //总的簇
    DWORD dwFreeClusters;    //可用的簇
    DWORD dwSectPerClust;    //每个簇有多少个扇区
    DWORD dwBytesPerSect;    //每个扇区有多少个字节
    pans.push_back(new char[4] {char(U), ':','\\', '\0'});
    BOOL bResult = GetDiskFreeSpaceA(pans.back(), &dwSectPerClust, &dwBytesPerSect, &dwFreeClusters, &dwTotalClusters);
    if (bResult)
    {
        DWORD64 total = dwTotalClusters * (DWORD64)dwSectPerClust * (DWORD64)dwBytesPerSect;
        DWORD64 used = dwFreeClusters * (DWORD64)dwSectPerClust * (DWORD64)dwBytesPerSect;
        int rate = 100 - used * 100 / total;
        ui->progressBar->setValue(rate);
        ui->label_7->setText(bytes_to_string(total));
        ui->label_8->setText(bytes_to_string(used));
    }
    widget = new QWidget(this);
    QHBoxLayout *horizontalLayout = new QHBoxLayout(widget);
    QPushButton *btn1(new QPushButton), *btn2(new QPushButton);
    btn1->setText("打开");
    btn2->setText("弹出");
    horizontalLayout->addWidget(btn1);
    horizontalLayout->addWidget(btn2);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalLayout->setSpacing(0);
    btn1->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    btn2->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    connect(btn1, &QPushButton::clicked, VarBox, [this](){
        VarBox->openDirectory(pans.front());
    });
    connect(btn2, &QPushButton::clicked, this, [this]()->bool{

        QString device_path = pans.back();
        QString error_string;
        const char* temp = "\\\\.\\";
        char device_path1[10] = { 0 };
        memcpy(device_path1, temp, strlen(temp));
        QByteArray dp = device_path.toLocal8Bit();
        device_path1[4] = dp.at(0);
        device_path1[5] = dp.at(1);
        HANDLE handleDevice = CreateFileA(device_path1, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        bool is_handle_invalid = (handleDevice == INVALID_HANDLE_VALUE);
        if (is_handle_invalid)
        {
            error_string = "Device is not connection to system!";
            qDebug() << GetLastError();
            return false;
        }
        // Do this in a loop until a timeout period has expired
        const int try_lock_volume_count = 3;
        int try_count = 0;
        for (; try_count < try_lock_volume_count; ++try_count)
        {
            DWORD dwBytesReturned;
            if (!DeviceIoControl(handleDevice, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &dwBytesReturned, NULL))
            {
                qDebug() << "Device is using....." << try_count;
                break;
            }
            QThread::sleep(1);
        }
        if (try_count == try_lock_volume_count)
        {
            error_string = "Device is using, try again later";
            CloseHandle(handleDevice);
            return false;
        }
        DWORD  dwBytesReturned = 0;
        PREVENT_MEDIA_REMOVAL PMRBuffer;
        PMRBuffer.PreventMediaRemoval = FALSE;
        if (!DeviceIoControl(handleDevice, IOCTL_STORAGE_MEDIA_REMOVAL, &PMRBuffer, sizeof(PREVENT_MEDIA_REMOVAL), NULL, 0, &dwBytesReturned, NULL))
        {
            error_string = QStringLiteral("Unmount failed! error code:%1").arg(GetLastError());
            qDebug() << "DeviceIoControl IOCTL_STORAGE_MEDIA_REMOVAL failed:" << GetLastError();
            CloseHandle(handleDevice);
            return false;
        }
        long   bResult = 0;
        DWORD retu = 0;
        bResult = DeviceIoControl(handleDevice, IOCTL_STORAGE_EJECT_MEDIA, NULL, 0, NULL, 0, &retu, NULL);
        if (!bResult)
        {
            error_string = QStringLiteral("Disconnect IGU failed! error code:%1").arg(GetLastError());
            CloseHandle(handleDevice);
            qDebug() << "Disconnect IGU IoControl failed error:" << GetLastError();
            return false;
        }
        CloseHandle(handleDevice);
        return true;
    });
    widget->setStyleSheet("QWidget{background-color:rgba(90, 90, 90, 190);}");
    btn1->setStyleSheet("QPushButton{color:yellow;background-color:rgba(70,70,70,90);}"
                        "QPushButton:hover{background-color:rgba(50, 50, 50, 90);}");
    btn2->setStyleSheet("QPushButton{color:green;background-color:rgba(70,70,70,90);}"
                        "QPushButton:hover{background-color:rgba(50, 50, 50, 90);}");
    widget->hide();
}

USBdriveHelper::~USBdriveHelper()
{
    for (const auto &i : pans)
        delete [] i;
    qout << "析构UI";
    delete ui;
}
