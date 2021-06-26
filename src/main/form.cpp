#include <QRect>
#include <QMessageBox>

#include "form.h"
#include "ui_form.h"

typedef struct _TRAFFIC
{
    DWORD in_bytes;
    DWORD out_bytes;
    DWORD in_byte;
    DWORD out_byte;
    PWCHAR FriendlyName;
    PCHAR AdapterName;
    WCHAR IP4[16];
} TRAFFIC;

pfnGetAdaptersAddresses GetAdaptersAddressesT;
pfnGetIfTable GetIfTableT;

PIP_ADAPTER_ADDRESSES piaa;//网卡结构
HMODULE hIphlpapi = NULL;
TRAFFIC *traffic;   //每个网卡速度
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
	return  QString("  %1  %2 %3").arg(units[up_down], QString::number(DW, 'f', 1), units[the_unit]);
}


Form::Form(QWidget* parent) :
	QWidget(parent),
	ui(new Ui::Form)
{
	FuncBox::write_log("开始构造悬浮窗。");
    ui->setupUi(this);                                                     //创建界面
    ui->LabMemory->installEventFilter(this);
	initForm();                                                            //初始化大小、位置、翻译功能
	initConnects();
//#if defined(Q_OS_WIN32)
//	DWORD uRetCode = GetIfTable(m_pTable, &m_dwAdapters, TRUE);            //检索 MIB-II 界面表，返回成功状态
//	if (uRetCode == ERROR_INSUFFICIENT_BUFFER)                             //m_dwAdapters参数指向的缓冲区不够大。所需的大小以m_dwAdapters参数指向的DWORD变量返回。
//		m_pTable = (PMIB_IFTABLE)new BYTE[65535];                          //假设端口数不超过65535个
//#endif
	monitor_timer->start(1000);                                            //开始检测网速和内存
    D("悬浮窗构造完毕。");
}

Form::~Form()
{
    D("开始析构悬浮窗。");
		A(if (VarBox.ENABLE_TRANSLATER) delete translater;)
		A(delete monitor_timer;)
		A(delete menu;)
		A(delete dialog;)
		A(delete ui;)
#if defined(Q_OS_WIN32)
		A(delete animation;)
//		A(delete[] m_pTable;)
        A(HeapFree(GetProcessHeap(), 0, piaa);)
        A(if (hIphlpapi) FreeLibrary(hIphlpapi);)
#endif
        D("析构悬浮窗成功。");
}

void Form::initForm()
{
    D("初始化悬浮窗配置。");
    D("设置字体中。");
    int font_use = QFontDatabase::addApplicationFont(":/fonts/fonts/qitijian.otf");
	QStringList fontFamilies = QFontDatabase::applicationFontFamilies(font_use);
	QFont font;
	font.setFamily(fontFamilies.at(0));
	font.setPointSize(17);
	font.setBold(true);
	ui->LabMemory->setFont(font);
	font_use = QFontDatabase::addApplicationFont(":/fonts/fonts/netspeed.ttf");
	fontFamilies = QFontDatabase::applicationFontFamilies(font_use);
	font.setFamily(fontFamilies.at(0));
	font.setPointSize(8);
	font.setBold(true);
	ui->Labdown->setFont(font);
	ui->Labup->setFont(font);
    D("设置字体成功。");
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    //setStyleSheet("background:transparent");
	setMinimumSize(FORM_WIDTH, FORM_HEIGHT);
	setMaximumSize(FORM_WIDTH, FORM_HEIGHT);
    D("初始化文件中。");
		FuncBox::build_init_files();                  //初始化文件，读取悬浮窗位置以及其它设置。
    D("初始化文件成功。");
		int x, y;                                     //存储悬浮窗位置坐标。
    QSettings IniRead(FuncBox::get_ini_path(), QSettings::IniFormat);
    IniRead.beginGroup("UI");
    x = IniRead.value("x").toInt() - ADAPT_WIDTH;
    y = IniRead.value("y").toInt() - ADAPT_HEIGHT;
    IniRead.endGroup();
	move(x, y);
	if (VarBox.ENABLE_TRANSLATER)
		enableTrans(true);
	else
		translater = nullptr;                     //防止野指针
	ui->LabMemory->setMaximumWidth(30);
    D("悬浮窗配置初始化成功。");
}

void Form::initConnects()
{
	dialog = new Dialog(this);                                             //dialog要比menu先定义，它是menu的依赖。
	menu = new Menu(this);
	monitor_timer = new QTimer;
#if defined(Q_OS_WIN32)
	animation = new QPropertyAnimation(this, "geometry");                  //用于贴边隐藏的动画
#endif
    //connect(dialog, SIGNAL(enableForm(bool)), this, SLOT(setEnabled(bool); setEnabled(true);));
	connect(monitor_timer, SIGNAL(timeout()), this, SLOT(updateInfo()));   //每秒钟刷新一次界面
}

void Form::msgBox(QString str)
{
    D("错误提示：" + str);
    QMessageBox::information(nullptr, "提示", str, QMessageBox::Ok, QMessageBox::Ok);
}


void Form::set_wallpaper_fail(QString str)
{
    D("错误内容：" + str);
    msgBox(str);
	dialog->showSelf();
}

bool Form::nativeEvent(const QByteArray &, void *message, long *)
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
		QPoint menu_point = event->globalPos();
		menu->new_show(menu_point.x(), menu_point.y());
	}
	event->accept();
}

void Form::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)  // 鼠标左键释放
	{
		QPoint pt = event->globalPos();
		savePos(pt.x(), pt.y());
		setMouseTracking(false);           // 停止跟踪鼠标。
		_endPos = event->pos() - _startPos;
		if (VarBox.ENABLE_TRANSLATER && !mouse_moved)
        {
            if (translater->isHidden())
                translater->showself();

            else
                translater->hide();
        }
		mouse_moved = false;
	}
	event->accept();
}

void Form::mouseDoubleClickEvent(QMouseEvent* event)
{
	event->accept();
    D("重启程序");
    VarBox.RUN_APP = false;
	VarBox.RUN_NET_CHECK = false;
	qApp->exit(RETCODE_RESTART);
}

void Form::mouseMoveEvent(QMouseEvent* event)
{
    if (VarBox.ENABLE_TRANSLATER && !translater->isHidden())
    {
        translater->hide();
    }
	_endPos = event->pos() - _startPos;  //计算位置变化情况。
	move(pos() + _endPos);               //当前位置加上位置变化情况，从而实现悬浮窗和鼠标同时移动。
	mouse_moved = true;
	event->accept();
}

#if defined(Q_OS_WIN32)
void Form::enterEvent(QEvent* event)     //鼠标进入事件
{
	tb_show(event);
}

void Form::leaveEvent(QEvent* event)
{
	tb_hide(event);
}

void Form::tb_hide(QEvent* event)
{
	QPoint pos = frameGeometry().topLeft();
	if (pos.x() + FORM_WIDTH >= VarBox.SCREEN_WIDTH)  // 右侧隐藏
	{
		startAnimation(VarBox.SCREEN_WIDTH - 2, pos.y());
		event->accept();
		moved = true;
	}
	else if (pos.x() <= 2)                    // 左侧隐藏
	{
		startAnimation(2 - FORM_WIDTH, pos.y());
		event->accept();
		moved = true;
	}
	else if (pos.y() <= 2)                    // 顶层隐藏
	{
		startAnimation(pos.x(), 2 - FORM_HEIGHT);
		event->accept();
		moved = true;
	}
}

void Form::tb_show(QEvent* event)
{
	QPoint pos = frameGeometry().topLeft();
	if (moved) {
		if (pos.x() + FORM_WIDTH >= VarBox.SCREEN_WIDTH)   // 右侧显示
		{
			startAnimation(VarBox.SCREEN_WIDTH - FORM_WIDTH + 2, pos.y());
			event->accept();
			moved = false;
		}
		else if (pos.x() <= 0)                    // 左侧显示
		{
			startAnimation(0, pos.y());
			event->accept();
			moved = false;
		}
		else if (pos.y() <= 0)                    // 顶层显示
		{
			startAnimation(pos.x(), 0);
			event->accept();
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
#endif


void Form::savePos(int x, int y)
{
    QSettings IniWrite(FuncBox::get_ini_path(), QSettings::IniFormat);
    IniWrite.beginGroup("UI");
    IniWrite.setValue("x", x);
    IniWrite.setValue("y", y);
    IniWrite.endGroup();
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
		if (VarBox.HAVE_APP_RIGHT)
		{
			VarBox.ENABLE_TRANSLATER = true;
			translater = new Translater(this);
			connect(translater, SIGNAL(msgBox(QString)), this, SLOT(msgBox(QString)));
		}
		else
		{
			QMessageBox::warning(nullptr, "错误", "未找到APP ID和密钥！请打开设置并确认是否填写正确。");
			VarBox.ENABLE_TRANSLATER = false;
		}
	}
	else
	{
		VarBox.ENABLE_TRANSLATER = false;
		disconnect(this, SIGNAL(stop_hide(bool)), translater, SLOT(stop_timer(bool)));
		delete translater;
		translater = nullptr;
	}

    QSettings IniWrite(FuncBox::get_ini_path(), QSettings::IniFormat);
    IniWrite.setValue("EnableTranslater", VarBox.ENABLE_TRANSLATER);
}

#if defined(Q_OS_WIN32)
void Form::get_mem_usage()
{
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);
	ui->LabMemory->setText(QString::number(ms.dwMemoryLoad));
	//ui->LabMemory->setText("100");
}

//void Form::get_net_usage()
//{
//	static DWORD   dwLastIn = 0;                        //上一秒钟的接收字节数
//	static DWORD   dwLastOut = 0;                       //上一秒钟的发送字节数

//	DWORD   dwBandIn = 0;                               //下载速度
//	DWORD   dwBandOut = 0;                              //上传速度
//	GetIfTable(m_pTable, &m_dwAdapters, TRUE);          //获取网速
//	DWORD   dwInOctets = 0;                             //下载流量
//	DWORD   dwOutOctets = 0;                            //上传流量

//	//将所有端口的流量进行统计
//	for (UINT i = 0; i < m_pTable->dwNumEntries; i++)
//	{
//		MIB_IFROW   Row = m_pTable->table[i];
//		dwInOctets += Row.dwInOctets;                   //获取现在下载的字节数
//		dwOutOctets += Row.dwOutOctets;                 //获取下载上传的字节数
//	}

//	dwBandIn = dwInOctets - dwLastIn;                   //1秒钟内下载的字节数
//	dwBandOut = dwOutOctets - dwLastOut;                //1秒钟内上传的字节数
//	if (dwLastIn <= 0)
//		dwBandIn = 0;
//	else
//		dwBandIn /= 8;                                  //b转换成B

//	if (dwLastOut <= 0)
//		dwBandOut = 0;
//	else
//		dwBandOut /= 8;                                 //b转换成B

//	dwLastIn = dwInOctets;                              //本次下载的字节数赋值给上次
//	dwLastOut = dwOutOctets;                            //本次上传的字节数赋值给上次

////    checkfullWindows();                                 //检测是否有全屏程序
//    if (isVisible())
//	{
//		if (isHidden()) show();                         //如果已经隐藏则显示
//		ui->Labup->setText(formatSpped(dwBandOut, false));        //将上传速度显示出来
//		ui->Labdown->setText(formatSpped(dwBandIn, true));    //将下载速度显示出来
//	}
//}

#elif defined(Q_OS_LINUX)
void Form::get_mem_usage()
{
	QStringList arguments;
	arguments.append("-m");
	QString str = FuncBox::runCommand("free", arguments, 2);
	str.replace("\n", "");
	str.replace(QRegExp("( ){1,}"), " ");  // 将连续空格替换为单个空格 用于分割
	QStringList lst = str.split(" ");
	ui->LabMemory->setText(QString::number(int((lst[2].toFloat() / lst[1].toFloat()) * 100)));
}

void Form::get_net_usage()
{
	static unsigned long long recv_bytes, send_bytes;
	static bool initNet = false;
	QProcess process;
	QStringList arguments;
	unsigned long int recv = 0, send = 0;
	arguments.append("/proc/net/dev");
	process.start("cat", arguments); //读取文件/proc/net/dev获取流量
	process.waitForFinished();
	process.readLine();
	process.readLine();
	while (!process.atEnd())  //这样可以获取所有网卡的数据。
	{
		QString str = process.readLine();
		str.replace("\n", "");
		str.replace(QRegExp("( ){1,}"), " ");
		QStringList lst = str.split(" ");
		recv += lst[2].toLongLong();
		send += lst[10].toLongLong();
	}
	if (initNet) {
		QString upload = QString(" ↑ ");
		QString download = QString(" ↓ ");
		upload += formatSpped(send - send_bytes).c_str();
		download += formatSpped(recv - recv_bytes).c_str();
		ui->Labup->setText(upload);
		ui->Labdown->setText(download);
	}
	else initNet = true;
	recv_bytes = recv;
	send_bytes = send;
}
#endif

void Form::get_net_usage()
{
    static int iGetAddressTime = 10;//10秒一次获取网卡信息
    static DWORD dwIPSize = 0;
    static DWORD dwMISize = 0;
    static int nTraffic = 0;
    static DWORD m_last_in_bytes = 0;//总上一秒下载速度
    static DWORD m_last_out_bytes = 0;//总上一秒上传速度
    static DWORD s_in_byte = 0;//总下载速度
    static DWORD s_out_byte = 0;//总上传速度
    if (hIphlpapi == NULL)
    {
        hIphlpapi = LoadLibrary(L"iphlpapi.dll");
        if (hIphlpapi)
        {
            GetAdaptersAddressesT = (pfnGetAdaptersAddresses)GetProcAddress(hIphlpapi, "GetAdaptersAddresses");
            GetIfTableT = (pfnGetIfTable)GetProcAddress(hIphlpapi, "GetIfTable");
        }
    }
    if (hIphlpapi)
    {
        PIP_ADAPTER_ADDRESSES paa;
        if (iGetAddressTime == 10)
        {
            //				DWORD odwIPSize = dwIPSize;
            dwIPSize = 0;
            if (GetAdaptersAddressesT(AF_INET, 0, 0, piaa, &dwIPSize) == ERROR_BUFFER_OVERFLOW)
            {
                //					if (dwIPSize != odwIPSize)
                {

                    HeapFree(GetProcessHeap(), 0, piaa);
                    int n = 0;
                    piaa = (PIP_ADAPTER_ADDRESSES)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwIPSize);
                    if (GetAdaptersAddressesT(AF_INET, 0, 0, piaa, &dwIPSize) == ERROR_SUCCESS)
                    {
                        paa = &piaa[0];
                        while (paa)
                        {
                            if (paa->IfType != IF_TYPE_SOFTWARE_LOOPBACK && paa->IfType != IF_TYPE_TUNNEL)
                            {
                                ++n;
                            }
                            paa = paa->Next;
                        }
                        if (n != nTraffic)
                        {
                            HeapFree(GetProcessHeap(), 0, traffic);
                            nTraffic = n;
                            traffic = (TRAFFIC*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, nTraffic * sizeof(TRAFFIC));
                        }
                    }
                }
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
        DWORD m_in_bytes = 0;
        DWORD m_out_bytes = 0;
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
                        traffic[l].in_byte = (mi->table[i].dwInOctets - traffic[l].in_bytes) * 8;
                        traffic[l].out_byte = (mi->table[i].dwOutOctets - traffic[l].out_bytes) * 8;
                        traffic[l].in_bytes = mi->table[i].dwInOctets;
                        traffic[l].out_bytes = mi->table[i].dwOutOctets;

                        PIP_ADAPTER_UNICAST_ADDRESS pUnicast = paa->FirstUnicastAddress;
                        while (pUnicast)
                        {
                            if (AF_INET == pUnicast->Address.lpSockaddr->sa_family)// IPV4 地址，使用 IPV4 转换
                            {
                                void* pAddr = &((sockaddr_in*)pUnicast->Address.lpSockaddr)->sin_addr;
                                byte* bp = (byte*)pAddr;
                                wsprintf(traffic[l].IP4, L"%d.%d.%d.%d", bp[0], bp[1], bp[2], bp[3]);
                                break;
                            }
                            pUnicast = pUnicast->Next;
                        }
                        traffic[l].FriendlyName = paa->FriendlyName;
                        traffic[l].AdapterName = paa->AdapterName;
                        if (lstrlen(paa->FriendlyName) > 19)
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
        if (m_last_in_bytes != 0)
        {
            s_in_byte = m_in_bytes - m_last_in_bytes;
            s_out_byte = m_out_bytes - m_last_out_bytes;
        }
        m_last_out_bytes = m_out_bytes;
        m_last_in_bytes = m_in_bytes;

        if (isVisible())
        {
            if (isHidden()) show();                                    //如果已经隐藏则显示
            ui->Labup->setText(formatSpped(s_out_byte, false));        //将上传速度显示出来
            ui->Labdown->setText(formatSpped(s_in_byte, true));        //将下载速度显示出来
        }
    }
}
