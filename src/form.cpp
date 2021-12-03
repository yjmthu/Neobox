#include <winsock2.h>

#include <QRect>
#include <QMessageBox>
#include <QTimer>
#include <QSettings>
#include <QPropertyAnimation>

#include <dbt.h>

#include "YString.h"
#include "form.h"
#include "ui_form.h"
#include "wallpaper.h"
#include "formsetting.h"
#include "desktopmask.h"
#include "wallpaper.h"
#include "usbdrivehelper.h"

PIP_ADAPTER_ADDRESSES piaa;//网卡结构
MIB_IFTABLE *mi;    //网速结构

inline QString formatSpped(long long dw, bool up_down)
{
    static const char* units[] = { u8"↑", u8"↓", "", "K", "M" };
	long double DW = dw;
	ushort the_unit = 2;
	if (DW >= 1024)
	{
		DW /= 1024;
		the_unit++;
		if (DW >= 1024)
		{
			DW /= 1024;
			the_unit++;
		}
	}
    return  QString("%1 %2 %3B").arg(units[up_down], QString::number(DW, 'f', 1), units[the_unit]);
}

inline void savePos()
{
    RECT rt; GetWindowRect(HWND(VarBox->form->winId()), &rt);
    QSettings IniWrite("SpeedBox.ini", QSettings::IniFormat);
    IniWrite.beginGroup("UI");
    IniWrite.setValue("x", (int)rt.left); IniWrite.setValue("y", (int)rt.top);
    IniWrite.endGroup();
}

Form::Form(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    qout << "悬浮窗指针";
    *const_cast<Form**>(&(VarBox->form)) = this;
    qout << "悬浮窗UI";
    ui->setupUi(this);                                                     //创建界面
    qout << "悬浮窗数据";
	initForm();                                                            //初始化大小、位置、翻译功能
    qout << "悬浮窗连接";
	initConnects();
    qout << "悬浮窗定时器";
	monitor_timer->start(1000);                                            //开始检测网速和内存
}

Form::~Form()
{
    qout << "析构Form开始";
    delete MouseMoveTimer;
    delete translater;
    qout << "析构定时器";
    delete monitor_timer;
    delete ui;
    delete animation;
    HeapFree(GetProcessHeap(), 0, piaa);
    HeapFree(GetProcessHeap(), 0, mi);
    qout << "析构Form结束";
}

void Form::keepInScreen()
{
    RECT rt;
    GetWindowRect(HWND(winId()), &rt);
    const auto w = (rt.right - rt.left), h = (rt.bottom - rt.top);
    const auto sw = GetSystemMetrics(SM_CXSCREEN), sh = GetSystemMetrics(SM_CYSCREEN);
    //qout << "当前宽度: " << rt.left << rt.right << rt.top << rt.bottom << sw << sh;
    auto x = rt.left, y = rt.top;
    if (x + 1 > sw)
        x = sw - 2;
    else if (rt.right < 1)
        x = 2 - w;
    if (y + 100 > sh)
        y = sh - 100;
    else if (rt.bottom < 1)
        y = 2 - h;
    SetWindowPos(HWND(winId()), HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
    savePos();
}

void Form::initForm()
{
    qout << "初始化悬浮窗界面";
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    QFont font;
    QFontDatabase::addApplicationFont(":/fonts/Nickainley-Normal-small.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Carattere-Regular-small.ttf");
    font.setFamily("Nickainley Normal");
    font.setPointSize(8);
    font.setBold(true);
    ui->Labdown->setFont(font);
    ui->Labup->setFont(font);
    font.setFamily("Carattere");
    font.setPointSize(17);
    font.setBold(true);
    ui->LabMemory->setFont(font);
    //setStyleSheet("background:transparent");
	setMinimumSize(FORM_WIDTH, FORM_HEIGHT);
	setMaximumSize(FORM_WIDTH, FORM_HEIGHT);
    FormSetting::load_style_from_file();

    QSettings IniRead("SpeedBox.ini", QSettings::IniFormat);
    IniRead.beginGroup("UI");
    SetWindowPos(HWND(winId()), HWND_TOPMOST, IniRead.value("x").toInt(), IniRead.value("y").toInt(), 0, 0, SWP_NOSIZE);
    IniRead.endGroup();

    if (VarBox->EnableTranslater)
    {
        enableTranslater(true);
    }
    else
    {
        translater = nullptr;                     //防止野指针
    }
	ui->LabMemory->setMaximumWidth(30);
    qout << "初始化悬浮窗界面完毕";
}

void Form::initConnects()
{
	monitor_timer = new QTimer;
	animation = new QPropertyAnimation(this, "geometry");                  //用于贴边隐藏的动画
    connect(monitor_timer, &QTimer::timeout, [this](){
        get_mem_usage();
        get_net_usage();
    });   //每秒钟刷新一次界面
    connect(animation, &QPropertyAnimation::finished, &savePos);
    connect(VarBox->wallpaper, &Wallpaper::setFailed, this, &Form::set_wallpaper_fail);
}


void Form::set_wallpaper_fail(const char* str)
{
    if (VarBox->dialog)
    {
        if (VarBox->dialog->isVisible())
            VarBox->dialog->setWindowState(Qt::WindowActive | Qt::WindowNoState);    // 让窗口从最小化恢复正常并激活窗口
            //d->activateWindow();
            //d->raise();
        else
            VarBox->dialog->show();
    }
    else
    {
        *const_cast<Dialog**>(&(VarBox->dialog)) = new Dialog;
        VarBox->dialog->show();
    }
    VarBox->MSG(str, "出错");
}

char FirstDriveFromMask (ULONG unitmask)
{
    char i;

    for (i = 0; i < 26; ++i)
    {
        if (unitmask & 0x1)
            break;
        unitmask = unitmask >> 1;
    }
    return (i + 'A');
}

bool Form::nativeEvent(const QByteArray &, void *message, long long *)
{
    MSG *msg = static_cast<MSG*>(message);
    static USBdriveHelper* helper = nullptr;

    if (MSG_APPBAR_MSGID == msg->message)
    {
        switch ((UINT)msg->wParam)
        {
        case ABN_FULLSCREENAPP:
            if (TRUE == (BOOL)(msg->lParam))
                hide();
            else
                show();
            return true;
        default:
            break;
        }
    }
    else if (WM_DEVICECHANGE == msg->message)
    {
        PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)msg->lParam;
        switch (msg->wParam)
        {
        case DBT_DEVICEARRIVAL:        //插入
            qout << "设备插入";
            if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME && !helper && VarBox->enableUSBhelper)
            {
                PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                helper = new USBdriveHelper(FirstDriveFromMask(lpdbv->dbcv_unitmask));
                connect(this, &Form::appQuit, this, [](){ delete helper; helper = nullptr; });
                connect(helper, &USBdriveHelper::appQuit, this, [&](){ helper = nullptr;});
                helper->show();
            }
            break;
        case DBT_DEVICEREMOVECOMPLETE: //设备删除
            qout << "设备删除";
            delete helper;
            helper = nullptr;
            break;
        }
    }
    return false;
}

void Form::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)                   // 鼠标左键点击悬浮窗
	{
		setMouseTracking(true);                              // 开始跟踪鼠标位置，从而使悬浮窗跟着鼠标一起移动。
		_startPos = event->pos();                            // 记录开始位置
	}
	else if (event->button() == Qt::RightButton)             // 鼠标右键点击悬浮窗
    {
        QPointF pt = event->globalPosition();
        (new Menu(this))->Show(pt.x(), pt.y());
	}
    else if (event->button() == Qt::MiddleButton)
    {
        qApp->exit(RETCODE_RESTART);
    }
	event->accept();
}

void Form::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)  // 鼠标左键释放
	{
        savePos();
		setMouseTracking(false);           // 停止跟踪鼠标。
		_endPos = event->pos() - _startPos;
	}
	event->accept();
}

void Form::mouseDoubleClickEvent(QMouseEvent*)
{
    if (VarBox->EnableTranslater)
    {
        if (!translater->isVisible())
            translater->show();
        else
            translater->hide();
    }
}

void Form::mouseMoveEvent(QMouseEvent* event)
{
    if (VarBox->EnableTranslater && !translater->isHidden())
    {
        translater->hide();
    }
	_endPos = event->pos() - _startPos;  //计算位置变化情况。
	move(pos() + _endPos);               //当前位置加上位置变化情况，从而实现悬浮窗和鼠标同时移动。
	event->accept();
}

void Form::enterEvent(QEnterEvent* event)     //鼠标进入事件
{
    QPoint pos = frameGeometry().topLeft();
    if (moved) {
        if (pos.x() + FORM_WIDTH >= VarBox->ScreenWidth)   // 右侧显示
        {
            startAnimation(VarBox->ScreenWidth - FORM_WIDTH + 2, pos.y());
            moved = false;
        }
        else if (pos.x() <= 0)                    // 左侧显示
        {
            startAnimation(0, pos.y());
            moved = false;
        }
        else if (pos.y() <= 0)                    // 顶层显示
        {
            startAnimation(pos.x(), 0);
            moved = false;
        }
    }
    if (event) event->accept();
}

void Form::leaveEvent(QEvent* event)
{
    QPoint pos = frameGeometry().topLeft();
    if (pos.x() + FORM_WIDTH >= VarBox->ScreenWidth)  // 右侧隐藏
    {
        startAnimation(VarBox->ScreenWidth - 2, pos.y());
        moved = true;
    }
    else if (pos.x() <= 2)                    // 左侧隐藏
    {
        startAnimation(2 - FORM_WIDTH, pos.y());
        moved = true;
    }
    else if (pos.y() <= 2)                    // 顶层隐藏
    {
        startAnimation(pos.x(), 2 - FORM_HEIGHT);
        moved = true;
    }
    if (event) event->accept();
}

void Form::startAnimation(int width, int height)
{
	QRect startpos = geometry();
	animation->setStartValue(startpos);
	QRect newpos = QRect(width, height, startpos.width(), startpos.height());
	animation->setEndValue(newpos);
	animation->setDuration(FORM_WIDTH);  //把宽度作为动画时间
	animation->start();
}

void Form::enableTranslater(bool checked)
{
    if (checked)
    {
        if (VarBox->HaveAppRight)
        {
            VarBox->EnableTranslater = true;
            translater = new Translater;
        }
        else
        {
            VarBox->MSG("未找到APP ID和密钥！请打开设置并确认是否填写正确。", "错误");
            VarBox->EnableTranslater = false;
        }
    }
    else
    {
        VarBox->EnableTranslater = false;
        delete translater;
        translater = nullptr;
    }

    QSettings IniWrite("SpeedBox.ini", QSettings::IniFormat);
    IniWrite.beginGroup("Translate");
    IniWrite.setValue("EnableTranslater", VarBox->EnableTranslater);
    IniWrite.endGroup();
}

void Form::get_mem_usage()
{
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);
	ui->LabMemory->setText(QString::number(ms.dwMemoryLoad));
	//ui->LabMemory->setText("100");
}

void Form::get_net_usage()
{
    static short iGetAddressTime = 10;  //10秒一次获取网卡信息
    static DWORD m_last_in_bytes = 0 /* 总上一秒下载字节 */,  m_last_out_bytes = 0 /* 总上一秒上传字节 */;
    static const char s = (leaveEvent(nullptr), 0);
    PX_UNUSED(s);

    if (iGetAddressTime == 10)        // 10秒更新
    {
        DWORD dwIPSize = 0;
        if (VarBox->GetAdaptersAddresses(AF_INET, 0, 0, piaa, &dwIPSize) == ERROR_BUFFER_OVERFLOW)
        {
            HeapFree(GetProcessHeap(), 0, piaa);
            piaa = (PIP_ADAPTER_ADDRESSES)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwIPSize);
            VarBox->GetAdaptersAddresses(AF_INET, 0, 0, piaa, &dwIPSize);
        }
        iGetAddressTime = 0;
    }
    else
        iGetAddressTime++;

    DWORD dwMISize = 0;
    if (VarBox->GetIfTable(mi, &dwMISize, FALSE) == ERROR_INSUFFICIENT_BUFFER)
    {
        dwMISize += sizeof(MIB_IFROW) * 2;
        HeapFree(GetProcessHeap(), 0, mi);
        mi = (MIB_IFTABLE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwMISize);
        VarBox->GetIfTable(mi, &dwMISize, FALSE);
    }
    DWORD m_in_bytes = 0, m_out_bytes = 0; //PIP_ADAPTER_ADDRESSES paa = nullptr;
    for (DWORD i = 0; i < mi->dwNumEntries; i++)
    {
        auto paa = &piaa[0];
        while (paa)
        {
            if (paa->IfType != IF_TYPE_SOFTWARE_LOOPBACK && paa->IfType != IF_TYPE_TUNNEL)
            {
                if (paa->IfIndex == mi->table[i].dwIndex)
                {
                    m_in_bytes += mi->table[i].dwInOctets;
                    m_out_bytes += mi->table[i].dwOutOctets;
                }
            }
            paa = paa->Next;
        }
    }

    if ( m_last_in_bytes != 0 && isVisible())
    {
        ui->Labup->setText(formatSpped(m_out_bytes - m_last_out_bytes, false));        //将上传速度显示出来
        ui->Labdown->setText(formatSpped(m_in_bytes - m_last_in_bytes, true));        //将下载速度显示出来
    }
    m_last_out_bytes = m_out_bytes;
    m_last_in_bytes = m_in_bytes;
}
