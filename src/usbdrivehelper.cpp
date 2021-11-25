#include "funcbox.h"
#include "usbdrivehelper.h"
#include "ui_usbdrivehelper.h"

#include <Dbt.h>
#include <winioctl.h>
#include "setupapi.h"
#include "cfgmgr32.h"
#pragma comment(lib,"setupapi.lib")
#include <QHBoxLayout>
#include <stdio.h>
#include <math.h>

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

DEVINST GetDrivesDevInstByDiskNumber(long DiskNumber)
{
    GUID* guid = (GUID*)(void*)&GUID_DEVINTERFACE_DISK;

    // Get device interface info set handle for all devices attached to system
    HDEVINFO hDevInfo = SetupDiGetClassDevs(guid, NULL, NULL,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    // Retrieve a context structure for a device interface of a device
    // information set.
    DWORD dwIndex = 0;
    SP_DEVICE_INTERFACE_DATA devInterfaceData = {0};
    devInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    BOOL bRet = FALSE;


    PSP_DEVICE_INTERFACE_DETAIL_DATA pspdidd;
    SP_DEVICE_INTERFACE_DATA spdid;
    SP_DEVINFO_DATA spdd;
    DWORD dwSize;

    spdid.cbSize = sizeof(spdid);

    while ( true )
    {
        bRet = SetupDiEnumDeviceInterfaces(hDevInfo, NULL, guid, dwIndex,
            &devInterfaceData);
        if (!bRet)
        {
            break;
        }

        SetupDiEnumInterfaceDevice(hDevInfo, NULL, guid, dwIndex, &spdid);

        dwSize = 0;
        SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid, NULL, 0, &dwSize,NULL);

        if ( dwSize )
        {
            pspdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, dwSize);
            if ( pspdidd == NULL )
            {
                continue; // autsch
            }
            pspdidd->cbSize = sizeof(*pspdidd);
            ZeroMemory((PVOID)&spdd, sizeof(spdd));
            spdd.cbSize = sizeof(spdd);


            long res = SetupDiGetDeviceInterfaceDetail(hDevInfo, &spdid,pspdidd, dwSize, &dwSize, &spdd);
            if ( res )
            {
                HANDLE hDrive = CreateFile(pspdidd->DevicePath, 0,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
                if ( hDrive != INVALID_HANDLE_VALUE )
                {
                    STORAGE_DEVICE_NUMBER sdn;
                    DWORD dwBytesReturned = 0;
                    res = DeviceIoControl(hDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER,NULL, 0, &sdn, sizeof(sdn), &dwBytesReturned, NULL);
                    if ( res )
                    {
                        if ( DiskNumber == (long)sdn.DeviceNumber )
                        {
                            CloseHandle(hDrive);
                            SetupDiDestroyDeviceInfoList(hDevInfo);
                            return spdd.DevInst;
                        }
                    }
                    CloseHandle(hDrive);
                }
            }
            HeapFree(GetProcessHeap(), 0, pspdidd);
        }
        dwIndex++;
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);

    return 0;
}

static int EjectUSBDisk(TCHAR discId)
{
    DWORD accessMode = GENERIC_WRITE | GENERIC_READ;
    DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    HANDLE hDevice;
    //long bResult = 0;
    //DWORD retu = 0;
    //DWORD dwError;
    DWORD dwBytesReturned;
    //int nTryCount;
    TCHAR szDriv[10];

    if(discId == NULL)
    {
        return 0;
    }

    wsprintf(szDriv, TEXT("\\\\.\\%c:"), discId);
    hDevice = CreateFile(szDriv, accessMode, shareMode, NULL, OPEN_EXISTING, 0, NULL);
    if(hDevice == INVALID_HANDLE_VALUE)
    {
        //printf("uninstallusb createfile failed error:%d\n",GetLastError());
        return -1;
    }

    //使用CM_Request_Device_Eject弹出USB设备
    STORAGE_DEVICE_NUMBER sdn;
    long DiskNumber = -1;
    long res = DeviceIoControl(hDevice,IOCTL_STORAGE_GET_DEVICE_NUMBER,NULL,0,&sdn,sizeof(sdn),&dwBytesReturned,NULL);
    if(!res)
    {
        //printf("DeviceIoControl IOCTL_STORAGE_GET_DEVICE_NUMBER failed:%d\n",GetLastError());
        CloseHandle(hDevice);
        return -1;
    }

    CloseHandle(hDevice);
    DiskNumber = sdn.DeviceNumber;
    if(DiskNumber == -1)
    {
        //printf("DiskNumber == -1\n");
        return -1;
    }

    DEVINST DevInst = GetDrivesDevInstByDiskNumber(DiskNumber);
    if(DevInst == 0)
    {
        //printf("GetDrivesDevInstDiskNumber failed\n");
        return -1;
    }

    ULONG Status = 0;
    ULONG ProblemNumber = 0;
    PNP_VETO_TYPE VetoType = PNP_VetoTypeUnknown;
    TCHAR VetoName[MAX_PATH];
    bool bSuccess = false;

    res = CM_Get_Parent(&DevInst, DevInst, 0);         //disk's parent, e.g. the USB bridge, the SATA controller....
    PX_UNUSED(res);
    res = CM_Get_DevNode_Status(&Status, &ProblemNumber, DevInst, 0);
    PX_UNUSED(res);
    bool IsRemovable = ((Status & DN_REMOVABLE) != 0);

    //printf("isremovable:%d\n",IsRemovable);
    long i;
    // try 3 times
    for(i = 0;i < 3;i++)
    {
        VetoName[0] = '\0';
        if(IsRemovable)
        {
            res = CM_Request_Device_Eject(DevInst,&VetoType,VetoName,MAX_PATH,0);
        }
        else
        {
            res = CM_Query_And_Remove_SubTree(DevInst,&VetoType,VetoName,MAX_PATH,0);
        }
        bSuccess = (res == CR_SUCCESS && VetoName[0] == '\0');
        if(bSuccess)
        {
            break;
        }
        else
        {
            Sleep(200);
        }
    }
    if(bSuccess){
        //printf("Success\n\n");
    }else{
        //printf("failed\n");

    }
    return 0;
}

void USBdriveHelper::enterEvent(QEnterEvent *event)
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
    pans.push_back(new TCHAR[4] {TCHAR(U), ':','\\', '\0'});
    BOOL bResult = GetDiskFreeSpace(pans.back(), &dwSectPerClust, &dwBytesPerSect, &dwFreeClusters, &dwTotalClusters);
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
    connect(btn1, &QPushButton::clicked, this, [this](){
        VARBOX::runCmd("explorer", QStringList(QString::fromWCharArray(pans.front())));
    });
    connect(btn2, &QPushButton::clicked, this, [this](){
        EjectUSBDisk(pans.front()[0]);
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
