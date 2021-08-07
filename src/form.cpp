#include <winsock2.h>
#include <iphlpapi.h>

#include <QRect>
#include <QMessageBox>
#include <QTimer>
#include <QSettings>
#include <QPropertyAnimation>

#include "YString.h"
#include "form.h"
#include "ui_form.h"
#include "wallpaper.h"

typedef ULONG(WINAPI* pfnGetAdaptersAddresses)(_In_ ULONG Family, _In_ ULONG Flags, _Reserved_ PVOID Reserved, _Out_writes_bytes_opt_(*SizePointer) PIP_ADAPTER_ADDRESSES AdapterAddresses, _Inout_ PULONG SizePointer);
typedef DWORD(WINAPI* pfnGetIfTable)(_Out_writes_bytes_opt_(*pdwSize) PMIB_IFTABLE pIfTable, _Inout_ PULONG pdwSize, _In_ BOOL bOrder);

PIP_ADAPTER_ADDRESSES piaa;//网卡结构
MIB_IFTABLE *mi;    //网速结构

QString formatSpped(long long dw, bool up_down)
{
    static const char* units[] = { "\342\206\221", "\342\206\223", "B", "KB", "MB" };
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
    return  QString("%1  %2 %3").arg(units[up_down], QString::number(DW, 'f', 1), units[the_unit]);
}


Form::Form(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::Form)
{
    VarBox.form = this;
    ui->setupUi(this);                                                     //创建界面
    ui->LabMemory->installEventFilter(this);
	initForm();                                                            //初始化大小、位置、翻译功能
	initConnects();
	monitor_timer->start(1000);                                            //开始检测网速和内存
}

Form::~Form()
{
    if (VarBox.EnableTranslater) delete translater;
    delete monitor_timer;
    delete menu;
    delete dialog;
    delete ui;
    delete animation;
    if (VarBox.AppId) delete [] VarBox.AppId;
    if (VarBox.PassWord) delete [] VarBox.PassWord;
    HeapFree(GetProcessHeap(), 0, piaa);
    if (VarBox.hIphlpapi) FreeLibrary(VarBox.hIphlpapi);
    if (VarBox.hOleacc) FreeLibrary(VarBox.hOleacc);
}

void Form::initForm()
{
    qout << "初始化悬浮窗界面";
    int font_use = QFontDatabase::addApplicationFont(":/fonts/qitijian.otf");
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(font_use);
    QFont font;
    font.setFamily(fontFamilies.at(0));
    font.setPointSize(18);
    font.setBold(true);
    ui->LabMemory->setFont(font);
    font_use = QFontDatabase::addApplicationFont(":/fonts/netspeed.ttf");
	fontFamilies = QFontDatabase::applicationFontFamilies(font_use);
	font.setFamily(fontFamilies.at(0));
	font.setPointSize(8);
	font.setBold(true);
	ui->Labdown->setFont(font);
	ui->Labup->setFont(font);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    //setStyleSheet("background:transparent");
	setMinimumSize(FORM_WIDTH, FORM_HEIGHT);
	setMaximumSize(FORM_WIDTH, FORM_HEIGHT);
    qout << "初始化文件开始~";
    FuncBox::build_init_files();                  //初始化文件，读取悬浮窗位置以及其它设置。
    qout << "初始化文件结束";
    int x, y;                                     //存储悬浮窗位置坐标。
    QSettings IniRead(FuncBox::get_ini_path(), QSettings::IniFormat);
    IniRead.beginGroup("UI");
    x = IniRead.value("x").toInt(); y = IniRead.value("y").toInt();
    IniRead.endGroup();
    SetWindowPos(HWND(winId()), HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
    if (VarBox.EnableTranslater)
        enableTrans(true);
    else
        translater = nullptr;                     //防止野指针
	ui->LabMemory->setMaximumWidth(30);
    qout << "初始化悬浮窗界面完毕";
}

void Form::initConnects()
{
    dialog = new Dialog;                                             //dialog要比menu先定义，它是menu的依赖。
    menu = new Menu;
	monitor_timer = new QTimer;
	animation = new QPropertyAnimation(this, "geometry");                  //用于贴边隐藏的动画
	connect(monitor_timer, SIGNAL(timeout()), this, SLOT(updateInfo()));   //每秒钟刷新一次界面
    connect(animation, SIGNAL(finished()), this, SLOT(savePos()));
}

void Form::savePos()
{
    RECT rt; GetWindowRect(HWND(winId()), &rt);
    QSettings IniWrite(FuncBox::get_ini_path(), QSettings::IniFormat);
    IniWrite.beginGroup("UI");
    IniWrite.setValue("x", (int)rt.left); IniWrite.setValue("y", (int)rt.top);
    IniWrite.endGroup();
}

void Form::msgBox(const char* content, const char* title)
{
    QMessageBox::information(nullptr, title, content, QMessageBox::Ok, QMessageBox::Ok);
}


void Form::set_wallpaper_fail(const char* str)
{
    msgBox(str, "出错");
    dialog->show();
}

bool Form::nativeEvent(const QByteArray &, void *message, long long *)
{
    MSG *msg = static_cast<MSG*>(message);

    if (MSG_APPBAR_MSGID == msg->message)
    {
        switch ((UINT)msg->wParam)
        {
        case ABN_FULLSCREENAPP:
            if (TRUE == (BOOL)(msg->lParam))
            {
                hide();
            }
            else
                show();
            return true;
        default:
            break;
        }
    }
    return false;
}

bool Form::eventFilter(QObject* target, QEvent* event)
{
    if (target == ui->LabMemory)
    {
        switch ((QEvent::Type)(event->type()))
        {
        case QEvent::MouseButtonPress:
            mousePressEvent(static_cast<QMouseEvent*>(event));
            return true;
        case QEvent::MouseButtonRelease:
            mouseReleaseEvent(static_cast<QMouseEvent*>(event));
            return true;
        default:
            break;
        }
    }
    return QWidget::eventFilter(target, event);
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
        menu->Show(pt.x(), pt.y());
	}
    else if (event->button() == Qt::MiddleButton)
    {
        VarBox.RunApp = false;
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
    if (VarBox.EnableTranslater)
    {
        if (!translater->isVisible())
            translater->show();
        else
            translater->hide();
    }
}

void Form::mouseMoveEvent(QMouseEvent* event)
{
    if (VarBox.EnableTranslater && !translater->isHidden())
    {
        translater->close();
    }
	_endPos = event->pos() - _startPos;  //计算位置变化情况。
	move(pos() + _endPos);               //当前位置加上位置变化情况，从而实现悬浮窗和鼠标同时移动。
	event->accept();
}

void Form::enterEvent(QEnterEvent* event)     //鼠标进入事件
{
    tb_show(); event->accept();
}

void Form::leaveEvent(QEvent* event)
{
    tb_hide(); event->accept();
}

void Form::tb_hide()
{
	QPoint pos = frameGeometry().topLeft();
    if (pos.x() + FORM_WIDTH >= VarBox.ScreenWidth)  // 右侧隐藏
	{
        startAnimation(VarBox.ScreenWidth - 2, pos.y());
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
}

void Form::tb_show()
{
	QPoint pos = frameGeometry().topLeft();
	if (moved) {
        if (pos.x() + FORM_WIDTH >= VarBox.ScreenWidth)   // 右侧显示
		{
            startAnimation(VarBox.ScreenWidth - FORM_WIDTH + 2, pos.y());
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

void Form::updateInfo()
{
	get_mem_usage();
	get_net_usage();
}

void Form::enableTrans(bool checked)
{
    if (checked)
    {
        if (VarBox.HaveAppRight)
        {
            VarBox.EnableTranslater = true;
            translater = new Translater;
            connect(translater, &Translater::enableself, this, &Form::enableTrans);
        }
        else
        {
            QMessageBox::warning(nullptr, "错误", "未找到APP ID和密钥！请打开设置并确认是否填写正确。");
            VarBox.EnableTranslater = false;
        }
    }
    else
    {
        VarBox.EnableTranslater = false;
        delete translater;
        translater = nullptr;
    }

    QSettings IniWrite(FuncBox::get_ini_path(), QSettings::IniFormat);
    IniWrite.beginGroup("Translate");
    IniWrite.setValue("EnableTranslater", VarBox.EnableTranslater);
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
    static int iGetAddressTime = 10;//10秒一次获取网卡信息
    static DWORD dwIPSize = 0, dwMISize = 0;
    static DWORD m_last_in_bytes = 0;//总上一秒下载速度
    static DWORD m_last_out_bytes = 0;//总上一秒上传速度
    static pfnGetIfTable GetIfTableT = nullptr; static pfnGetAdaptersAddresses GetAdaptersAddressesT = nullptr;
    if (VarBox.hIphlpapi == NULL)
    {
        VarBox.hIphlpapi = LoadLibrary("iphlpapi.dll");
        if (VarBox.hIphlpapi)
        {
            GetAdaptersAddressesT = (pfnGetAdaptersAddresses)GetProcAddress(VarBox.hIphlpapi, "GetAdaptersAddresses");
            GetIfTableT = (pfnGetIfTable)GetProcAddress(VarBox.hIphlpapi, "GetIfTable");
        }
    }

    if (!(VarBox.hIphlpapi && GetIfTableT && GetAdaptersAddressesT)) return;

    if (iGetAddressTime == 10)        // 10秒更新
    {
        dwIPSize = 0;
        if (GetAdaptersAddressesT(AF_INET, 0, 0, piaa, &dwIPSize) == ERROR_BUFFER_OVERFLOW)
        {
            HeapFree(GetProcessHeap(), 0, piaa);
            piaa = (PIP_ADAPTER_ADDRESSES)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwIPSize);
            GetAdaptersAddressesT(AF_INET, 0, 0, piaa, &dwIPSize);
        }
        iGetAddressTime = 0;
    }
    else
        iGetAddressTime++;

    if (GetIfTableT(mi, &dwMISize, FALSE) == ERROR_INSUFFICIENT_BUFFER)
    {
        dwMISize += sizeof(MIB_IFROW) * 2;
        HeapFree(GetProcessHeap(), 0, mi);
        mi = (MIB_IFTABLE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwMISize);
        GetIfTableT(mi, &dwMISize, FALSE);
    }
    DWORD m_in_bytes = 0, m_out_bytes = 0; PIP_ADAPTER_ADDRESSES paa;
    for (DWORD i = 0; i < mi->dwNumEntries; i++)
    {
        int l = 0;
        paa = &piaa[0];
        while (paa)
        {
            if (paa->IfType != IF_TYPE_SOFTWARE_LOOPBACK && paa->IfType != IF_TYPE_TUNNEL)
            {
                if (paa->IfIndex == mi->table[i].dwIndex)
                {
                    if (wcslen(paa->FriendlyName) > 19)
                    {
                        paa->FriendlyName[16] = L'.';
                        paa->FriendlyName[17] = L'.';
                        paa->FriendlyName[18] = L'.';
                        paa->FriendlyName[19] = L'\0';
                    }
                    m_in_bytes += mi->table[i].dwInOctets;
                    m_out_bytes += mi->table[i].dwOutOctets;
                }
                ++l;
            }
            paa = paa->Next;
        }
    }

    if ( m_last_in_bytes != 0 && isVisible())
    {
        if (isHidden()) show();                                    //如果已经隐藏则显示
        ui->Labup->setText(formatSpped(m_in_bytes - m_last_in_bytes, false));        //将上传速度显示出来
        ui->Labdown->setText(formatSpped(m_out_bytes - m_last_out_bytes, true));        //将下载速度显示出来
    }
    m_last_out_bytes = m_out_bytes;
    m_last_in_bytes = m_in_bytes;
}
