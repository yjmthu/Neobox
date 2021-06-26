#include <QFileDialog>
#include <QRegExpValidator>
#include <QMessageBox>
#include <QStandardPaths>
#include <QColor>
#include <QColorDialog>
#include <QGraphicsDropShadowEffect>
#include <QListView>

#include "dialog.h"
#include "ui_dialog.h"
#include "form.h"
#include "dialogwallpaper.h"

const char *color_theme[4] =
{
    "255,255,255,255",
    "255,255,255,150",
    "155,89,182,150",
    "255,0,0,200"
};

BlankFrom::BlankFrom(Dialog* di, QWidget* parent):
    QWidget(parent),
    dialog(di)
{
    setMaximumSize(100, 40);
    setMinimumSize(100, 40);
    closeButton = new QPushButton(this);
    minButton = new QPushButton(this);
    connect(closeButton, &QPushButton::clicked, dialog, &QWidget::close);
    connect(minButton, &QPushButton::clicked, dialog, &QWidget::showMinimized);
    minButton->setGeometry(width()-75,12,14,14);
    closeButton->setGeometry(width()-40,12,14,14);
    minButton->setToolTip(tr("最小化"));
    closeButton->setToolTip(tr("关闭"));
    minButton->setCursor(Qt::PointingHandCursor);
    closeButton->setCursor(Qt::PointingHandCursor);
    minButton->setStyleSheet("QPushButton{background-color:#85c43b;border-radius:7px;}");
    closeButton->setStyleSheet("QPushButton{background-color:#ea6e4d;border-radius:7px;}");
}

void BlankFrom::enterEvent(QEvent *)
{
    //closeButton->setStyleSheet("QPushButton{background-color:#ea6e4d;border:none;background-position:center;background-repeat:no-repeat;width: 14px;height: 14px;border-radius:7px;background-image: url(:/icons/icons/Close.png)}");
    closeButton->setStyleSheet("QPushButton{border-image: url(:/icons/icons/Close_2.png);border-radius:7px;}");
    //minButton->setStyleSheet("QPushButton{background-color:#85c43b;border:none;background-position:center;background-repeat:no-repeat;width: 14px;height: 14px;border-radius:7px;background-image: url(:/icons/icons/Minimize.png);}");
    minButton->setStyleSheet("QPushButton{border-image: url(:/icons/icons/Minimize_2.png);border-radius:7px;}");
}

void BlankFrom::leaveEvent(QEvent *)
{
    minButton->setStyleSheet("QPushButton{background-color:#85c43b;border-radius:7px;}");
    closeButton->setStyleSheet("QPushButton{background-color:#ea6e4d;border-radius:7px;}");
//    closeButton->setStyleSheet("QPushButton{border-image: url(:/icons/icons/Close_1.png);border-radius:7px;}");
//    minButton->setStyleSheet("QPushButton{border-color:blue;border-image: url(:/icons/icons/Minimize_1.png);border-radius:7px;}");
}

GMPOperateTip::GMPOperateTip(QWidget* parent):
    QWidget(parent)
{
    tip = new QLabel(this);
    centerPos = parent->rect().center();

    m_pAnimation = new QPropertyAnimation(this);
    m_pAnimation->setTargetObject(this);

    m_pOpacity = new QGraphicsOpacityEffect(this);
    m_pOpacity->setOpacity(1);
    setGraphicsEffect(m_pOpacity);
    m_pAnimation->setTargetObject(m_pOpacity);

    m_pAnimation->setPropertyName("opacity");
    m_pAnimation->setStartValue(1);
    m_pAnimation->setEndValue(0);

    m_pAnimation->setDuration(150);
    connect(m_pAnimation, &QPropertyAnimation::finished, this, [this](){setVisible(false);m_pOpacity->setOpacity(1);});
    setStyleSheet("QWidget{background-color: transparent;}QLabel{background-color: #4cd05c;border-radius: 3px; padding: 6px;}");
    setVisible(false);
}

GMPOperateTip::~GMPOperateTip()
{
    delete m_pOpacity;
    delete m_pAnimation;
    delete  tip;
}


void GMPOperateTip::showTip(QString str = "写了些啥")
{
    tip->setText(str);
    int w = str.length()*20 +12;
    int h = 20 + 12;
    setGeometry(centerPos.x()-w/2, centerPos.y()-h/2, w, h);
    setVisible(true);
    QTimer::singleShot(500, m_pAnimation, SLOT(start()));
}

QString getStyleSheet()
{
	QColor col(BueaTray::TraySave.dAlphaColor[BueaTray::TraySave.SET_FULL] & 0xffffff);
	return QString("background-color: rgb(%1, %2, %3)").arg(QString::number(col.blue()), QString::number(col.green()), QString::number(col.red()));
}


Dialog::Dialog(Form* v, QWidget* parent) :
	QWidget(parent),
	ui(new Ui::Dialog),
	form(v)
{
    D("开始构造壁纸设置对话框。");
		ui->setupUi(this);                                   // 一定要在下面这句之前，否则会出现问题。
	initChildren();
	initUi();
	initConnects();
	initBehaviors();
    initButtonFilter();
    D("壁纸设置对话框构造完毕。");
}

Dialog::~Dialog()
{
    D("设置对话框删除中。");
		A(wallpaper->wait();)
        A(delete jobTip;)
		A(delete change_paper_timer;)
		A(delete wallpaper;)
		A(delete buttonGroup;)
		A(delete ui;)
        D("设置对话框删除完毕");
}

void Dialog::initUi()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);  //| Qt::WindowStaysOnTopHint
    setAttribute(Qt::WA_TranslucentBackground);
    QFile qss(":/qss/qss/dialog_tad_style.qss");
    qss.open(QFile::ReadOnly);
    setStyleSheet(qss.readAll());
    qss.close();
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(this);
    effect->setOffset(0, 0);          //具体阴影
    effect->setBlurRadius(15);        //阴影半径
    effect->setColor(Qt::black);      //阴影颜色
    ui->frame->setGraphicsEffect(effect);
    BlankFrom *blank = new BlankFrom(this, this);
    blank->move(width()-100, 0);
    buttonGroup->addButton(ui->rBtnNew, (int)Latest);                //将按钮添加到按钮组合，同时设置按钮 id
	buttonGroup->addButton(ui->rBtnHot, (int)Hot);
	buttonGroup->addButton(ui->rBtnNature, (int)Nature);
	buttonGroup->addButton(ui->rBtnAnime, (int)Anime);
	buttonGroup->addButton(ui->rBtnSimple, (int)Simple);
	buttonGroup->addButton(ui->rBtnRandom, (int)Random);
	buttonGroup->addButton(ui->rBtnBing, (int)Bing);
	buttonGroup->addButton(ui->rBtnNative, (int)Native);
	buttonGroup->addButton(ui->rBtnAdvance, (int)Advance);
	buttonGroup->setExclusive(true);                     // 按钮之间相互排斥
	//ui->lineEditPageNum->setValidator(new QRegExpValidator(QRegExp("[1-9][0-9]")));
	ui->lineAppData->setText(FuncBox::get_dat_path().replace("/", "\\"));
	ui->lineAppData->setReadOnly(true);
    setTheme();
}

void Dialog::initChildren()
{
	buttonGroup = new QButtonGroup;
	wallpaper = new DialogWallpaper;                     // 创建壁纸更换类
	change_paper_timer = new QTimer;                     // 定时器，定时更换壁纸
    jobTip = new GMPOperateTip(this);
}

void Dialog::initConnects()
{
	connect(wallpaper, SIGNAL(setFailed(QString)), form, SLOT(set_wallpaper_fail(QString)));
	connect(change_paper_timer, SIGNAL(timeout()), wallpaper, SLOT(start()));
	connect(wallpaper, SIGNAL(msgBox(QString)), form, SLOT(msgBox(QString)));
	for (int c = 0; c <= 9; c++)
		if ((c != (int)Native) && (c != 7)) connect(buttonGroup->button(c), &QPushButton::clicked, this,
			[=]() {
				last_checked_button = c;
                D(QString("你临时选中了") + VarBox.PaperTypes[c] + "！");
			});
}

void Dialog::initBehaviors()
{
	Readcfg();
	change_paper_timer->setInterval(TimeInerval * 60000);
	wallpaper->start();                                  // Timer默认第一次不启动，这里要加上。
	if (VarBox.AutoChange)
	{
        D(QString("已经自动开始更换壁纸，壁纸类型：") + VarBox.PaperTypes[VarBox.PaperType]); // 检测更换壁纸是否开始，同时打印壁纸类型
        change_paper_timer->start();
	}
}

void Dialog::initButtonFilter()
{
    QList<QPushButton *> pBtns = findChildren<QPushButton *>();
    QList<QPushButton *>::iterator pBtnList = pBtns.begin();
    for (pBtnList = pBtns.begin(); pBtnList != pBtns.end(); pBtnList++)
    {
        (*pBtnList)->installEventFilter(this);
    }
    QList<QSlider *> sliders = findChildren<QSlider *>();
    QList<QSlider *>::iterator sliderList = sliders.begin();
    for (sliderList = sliders.begin(); sliderList != sliders.end(); sliderList++)
    {
        (*sliderList)->installEventFilter(this);
    }
    QList<QComboBox *> combs = findChildren<QComboBox *>();
    QList<QComboBox *>::iterator combList = combs.begin();
    for (combList = combs.begin(); combList != combs.end(); combList++)
    {
        (*combList)->installEventFilter(this);
        (*combList)->setView(new QListView());
    }
}

bool Dialog::eventFilter(QObject* target, QEvent* event)
{
    if (typeid (*target) == typeid (QPushButton) || typeid (*target) == typeid (QComboBox))
    {
        if ((QEvent::Type)(event->type()) == QEvent::MouseMove)
            return true;
    }
    else if (typeid (*target) == typeid (QSlider))
    {
        QMouseEvent *env = static_cast<QMouseEvent *>(event);
        QSlider *tar = static_cast<QSlider *>(target);
        //static bool clicked = false;
        switch ((QEvent::Type)(env->type()))
        {
        case QEvent::MouseMove:
            tar->setValue(((double)env->pos().x() / (double)tar->width() * (tar->maximum() - tar->minimum())) + tar->minimum());
            return true;
        case QEvent::MouseButtonPress:
            tar->setValue(((double)env->pos().x() / (double)tar->width() * (tar->maximum() - tar->minimum())) + tar->minimum());
            tar->setMouseTracking(true);
            return true;
        case QEvent::MouseButtonRelease:
            tar->setMouseTracking(false);
            return true;
        default:
            break;
        }
    }
    return QWidget::eventFilter(target, event);
}

void Dialog::mousePressEvent(QMouseEvent* event)
{
    qDebug() << event->pos();
    if (event->button() == Qt::LeftButton)                   // 鼠标左键点击悬浮窗
    {
        setMouseTracking(true);                              // 开始跟踪鼠标位置，从而使悬浮窗跟着鼠标一起移动。
        _startPos = event->pos();                            // 记录开始位置
    }
    event->accept();
}

void Dialog::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)  // 鼠标左键释放
    {
        setMouseTracking(false);           // 停止跟踪鼠标。
        _endPos = event->pos() - _startPos;
        mouse_moved = false;
    }
    event->accept();
}

void Dialog::mouseMoveEvent(QMouseEvent* event)
{
    _endPos = event->pos() - _startPos;  //计算位置变化情况。
    move(pos() + _endPos);               //当前位置加上位置变化情况，从而实现悬浮窗和鼠标同时移动。
    mouse_moved = true;
    event->accept();
}

/**
 * @brief Dialog::showSelf
 *
 * @消除之前所作的未保存的改动，将静态变量中的数据加载到界面上，并显示设置界面。
 *
 */

void Dialog::showSelf()
{
    if (isVisible())
         setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);   //只能是 Qt::Widget 不能是 Qt::Dialog，否则再次点击不会在顶层显示。
    else{
    last_checked_button = VarBox.PaperType;
	ui->BtnChooseFolder->setEnabled(last_checked_button == 7);
	buttonGroup->button(last_checked_button)->setChecked(true);
	ui->LinePath->setText(VarBox.NativeDir.replace("/", "\\"));
	ui->SliderPageNum->setValue(VarBox.PageNum);
	ui->SliderTimeInterval->setValue(TimeInerval);
	ui->labTimeInterval->setText(QString::number(TimeInerval));
	ui->chkEnableChangePaper->setChecked(VarBox.AutoChange);
	ui->usrCmd->setText(VarBox.UserCommand);
	ui->linePictuerPath->setText(VarBox.FAMILY_PATH.replace("/", "\\"));
	ui->radioButton_2->setChecked(bueaty.TraySave.SET_FULL);
    ui->horizontalSlider->setValue(bueaty.TraySave.dAlphaColor[bueaty.TraySave.SET_FULL] >> 24);
	ui->horizontalSlider_2->setValue(bueaty.TraySave.bAlpha[bueaty.TraySave.SET_FULL]);
	ui->horizontalSlider_3->setValue(bueaty.TraySave.refresh_time);
	ui->pushButton_3->setStyleSheet(getStyleSheet());
	ui->lineNewName->setText(VarBox.FAMILY_NAMES[ui->comboBox->currentIndex()]);
    ui->line_APP_ID->setText(VarBox.APP_ID);
    ui->line_PASS_WORD->setText(VarBox.PASS_WORD);
	switch (bueaty.TraySave.aMode[bueaty.TraySave.SET_FULL])
	{
	case (ACCENT_DISABLED):
		ui->radioButton_3->setChecked(true);
		break;
	case (ACCENT_ENABLE_TRANSPARENTGRADIENT):
		ui->radioButton_4->setChecked(true);
		break;
	case (ACCENT_ENABLE_BLURBEHIND):
		ui->radioButton_5->setChecked(true);
		break;
	case (ACCENT_ENABLE_ACRYLICBLURBEHIND):
		ui->radioButton_6->setChecked(true);
		break;
	default:
		break;
	}
	int Tr;
	bueaty.getValue(L"TaskbarAcrylicOpacity", Tr);
	switch (Tr)
	{
	case 10:
		ui->radioButton_11->setChecked(true);
		break;
	case 5:
		ui->radioButton_10->setChecked(true);
		break;
	case 0:
		ui->radioButton_12->setChecked(true);
		break;
	default:
		break;
	}
	switch (bueaty.iPos)
	{
	case TASK_LEFT:
		ui->radioButton_7->setChecked(true);
		break;
	case TASK_CENTER:
		ui->radioButton_8->setChecked(true);
		break;
	case TASK_RIGHT:
		ui->radioButton_9->setChecked(true);
		break;
	default:
		break;
	}
	VarBox.PATH_TO_OPEN.replace("\\", "/");
    VarBox.FAMILY_PATH.replace("\\", "/");
	if (!VarBox.PATH_TO_OPEN.compare(qApp->applicationDirPath()))
		ui->comboBox_2->setCurrentIndex(0);
    else if (!VarBox.PATH_TO_OPEN.compare(VarBox.FAMILY_PATH))
		ui->comboBox_2->setCurrentIndex(1);
    else if (!VarBox.PATH_TO_OPEN.compare(VarBox.FAMILY_PATH + "/" + VarBox.FAMILY_NAMES[7]))
		ui->comboBox_2->setCurrentIndex(2);
	else if (!VarBox.PATH_TO_OPEN.compare(FuncBox::get_dat_path()))
		ui->comboBox_2->setCurrentIndex(3);
	else
		ui->comboBox_2->setCurrentIndex(4);
    ui->comboBox_3->setCurrentIndex(VarBox.cur_theme);
	ui->label_path_to_open->setText(VarBox.PATH_TO_OPEN.replace("/", "\\"));
	ui->label_path_to_open->setToolTip(VarBox.PATH_TO_OPEN);
    checkSettings();}
    //moveDialog();
    show();
}

/**
 * @brief Dialog::Readcfg
 *
 * @将文件中的数据加载到静态变量中。
 *
 */

void Dialog::Readcfg()
{
    D("开始读取配置文件。");
    QSettings IniRead(FuncBox::get_ini_path(), QSettings::IniFormat);
    IniRead.beginGroup("Wallpaper");
    VarBox.PaperType = (PAPER_TYPE)IniRead.value("PaperType").toInt();
	buttonGroup->button(VarBox.PaperType)->setChecked(true);
    VarBox.NativeDir = IniRead.value("NativeDir").toString();
	ui->LinePath->setText(VarBox.NativeDir.replace("/", "\\"));
    VarBox.PageNum = IniRead.value("PageNum").toInt();
	ui->SliderPageNum->setValue(VarBox.PageNum);
    TimeInerval = IniRead.value("TimeInerval").toInt();
    ui->SliderTimeInterval->setValue(TimeInerval);
	ui->labTimeInterval->setText(QString::number(TimeInerval));
    VarBox.AutoChange = IniRead.value("AutoChange").toBool();
	ui->chkEnableChangePaper->setChecked(VarBox.AutoChange);
    VarBox.UserCommand = IniRead.value("UserCommand").toString();
    IniRead.endGroup();
    D("配置文件读取完毕");
}

/**
 * @brief Dialog::Writecfg
 *
 * @将界面中的数据保存到文件之中。
 *
 */

void Dialog::Writecfg() const
{
    D("开始保存配置文件。");
    QSettings IniWrite(FuncBox::get_ini_path(), QSettings::IniFormat);
    IniWrite.beginGroup("Wallpaper");
    IniWrite.setValue("PaperType", buttonGroup->checkedId());
    IniWrite.setValue("NativeDir", ui->LinePath->text().replace("\\", "/"));
    IniWrite.setValue("PageNum", ui->SliderPageNum->value());
    IniWrite.setValue("TimeInerval", ui->SliderTimeInterval->value());
    IniWrite.setValue("AutoChange", ui->chkEnableChangePaper->isChecked());
    IniWrite.setValue("UserCommand", ui->usrCmd->text());
    IniWrite.endGroup();
    D("配置文件保存完毕。");
}

/**
 * @brief Dialog::moveDialog
 *
 * @将对话框移动到屏幕中央。
 *
 */

void Dialog::moveDialog()
{
	move((VarBox.SCREEN_WIDTH - width()) / 2, (VarBox.SCREEN_HEIGHT - height()) / 2);
}

/**
 * @brief Dialog::on_rBtnNative_toggled
 *
 * @当“本地”按钮被选中时，选择文件夹按钮可用。
 *
 * @param {checked} 按钮选中状态
 *
 */

void Dialog::on_rBtnNative_toggled(bool checked)
{
	ui->BtnChooseFolder->setEnabled(checked);
}

/**
 * @brief Dialog::on_BtnChooseFolder_clicked
 *
 * @选择文件按钮被点击时，弹出选择文件对话框。
 *
 */

void Dialog::on_BtnChooseFolder_clicked()
{
	QDir d;
	if (!VarBox.NativeDir.compare("") || (!d.exists(VarBox.NativeDir) && !d.mkdir(VarBox.NativeDir)))
	{
		VarBox.NativeDir = QStandardPaths::writableLocation((QStandardPaths::MusicLocation));
		QSettings set(FuncBox::get_ini_path());
        set.beginGroup("Wallpaper");
		set.setValue("NativeDir", VarBox.NativeDir);
        set.endGroup();
		ui->LinePath->setText(VarBox.NativeDir);
	}
	QString titile = "请选择一个文件夹";
	QString dir = QFileDialog::getExistingDirectory(NULL, titile, VarBox.NativeDir, QFileDialog::ShowDirsOnly);
	if (dir.compare(""))
		ui->LinePath->setText(dir);
	else
	{
		buttonGroup->button(last_checked_button)->setChecked(true);
		ui->BtnChooseFolder->setEnabled(last_checked_button == 7);
	}
}

void Dialog::on_SliderTimeInterval_valueChanged(int value)
{
	ui->labTimeInterval->setText(QString::number(value, 10));  // 十进制整数转化为字符串
}

void Dialog::on_SliderPageNum_valueChanged(int value)
{
	ui->label_2->setText(QString::number(value, 10));
}

/**
 * @brief Dialog::on_pBtnCancel_clicked
 *
 * @取消按钮被点击后，关闭对话框，主界恢复可点击状态。
 *
 */

void Dialog::on_pBtnCancel_clicked()
{
	close();
}

/**
 * @brief Dialog::on_pBtnOk_clicked
 *
 * @“确认”按钮被点击后，将界面的配置写入文件，但是不修改静态变量。
 *
 */

void Dialog::on_pBtnOk_clicked()
{
    D(QString("从设置保存更换壁纸，壁纸类型：") + VarBox.PaperTypes[VarBox.PaperType]);
	Writecfg();
	close();
}

/**
 * @brief Dialog::on_pBtnApply_clicked
 *
 * @“应用”按钮被点击后，将界面上的配置导入到文件中，将文件中的数据加载到静态变量。
 *
 */

void Dialog::on_pBtnApply_clicked()
{
	if (wallpaper->isRunning())
	{
		if (QMessageBox::question(this, "警告", "您之前的壁纸类型更换正在生效中，是否终止？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
		{
			setEnabled(false);
			VarBox.RUN_NET_CHECK = false;
			VarBox.RUN_APP = false;
			wallpaper->wait();
			VarBox.RUN_APP = true;
			VarBox.RUN_NET_CHECK = true;
			setEnabled(true);
			if (QMessageBox::question(this, "提示", "终止成功！是否继续？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
				goto label;
		}
		return;
	}
label:
	{
		if (Wallpaper::a_thresd_isrunning)
			QMessageBox::information(this, "提示", "请先等待后台下载结束再重试！");
		else
		{
			Writecfg();                                              // 写入界面上面的配置到congig.ini文件
			Readcfg();
            D(QString("从设置启用更换壁纸，壁纸类型：") + VarBox.PaperTypes[VarBox.PaperType]);
			if (VarBox.AutoChange)
			{
				wallpaper->start();
				change_paper_timer->setInterval(TimeInerval * 60000);  // 设置时间间隔,Timer的单位是毫秒
				change_paper_timer->start();
			}
			else {
				change_paper_timer->stop();
				wallpaper->start();
			}
			close();
		}
	}
}

void Dialog::checkSettings()
{
	Menu_status[0] = '0' + QSettings("HKEY_CLASSES_ROOT\\*\\shell\\QCoper", QSettings::NativeFormat).contains(".");
	ui->chkFile->setChecked(Menu_status[0] == '1');
	Menu_status[1] = '0' + QSettings("HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\QCoper", QSettings::NativeFormat).contains(".");
	ui->chkFolder->setChecked(Menu_status[1] == '1');
	Menu_status[2] = '0' + QSettings("HKEY_CLASSES_ROOT\\Directory\\shell\\QCoper", QSettings::NativeFormat).contains(".");
	ui->chkFolderBack->setChecked(Menu_status[2] == '1');
	ui->chkTimeUnit_sec->setChecked(bueaty.checkKey(SHOW_SECONDS_IN_SYSTEM_CLOCK));
}

bool IsProcessRunAsAdmin()
{
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup;
	BOOL b = AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&AdministratorsGroup);
	if (b)
	{
		CheckTokenMembership(NULL, AdministratorsGroup, &b);
		FreeSid(AdministratorsGroup);
	}
	return b == TRUE;
}

bool GetAdmin(QString app_path, QString args)
{
	HINSTANCE res = ShellExecute(
		NULL,
		L"runas",
		app_path.toStdWString().c_str(),
		args.toStdWString().c_str(),
		NULL,
		SW_SHOWDEFAULT);
	return ((long long)res > 32);
}

void Dialog::on_pBtnApply_2_clicked()
{
	static QString icon_path = FuncBox::get_dat_path() + "/Copy.ico";
	QFile file(icon_path);
	if (!file.exists() && file.open(QIODevice::WriteOnly))
	{
		QFile qss(":/icons/icons/QCopy.ico");
		qss.open(QFile::ReadOnly);
		file.write(qss.readAll());
		qss.close();
		file.close();
	}
	QString exe_file = FuncBox::get_son_dir("bin").replace("/", "\\") + "\\plreg.exe";
    if (!QFile::exists(exe_file))
    {
        QFile f(exe_file);
        if (!f.open(QIODevice::WriteOnly))
        {
            QMessageBox::critical(this, "错误", "不能创建工具！", QMessageBox::Cancel, QMessageBox::Cancel);
            return;
        }
        QFile qss(":/scripts/scripts/plreg.exe");
        qss.open(QFile::ReadOnly);
        f.write(qss.readAll());
        qss.close();
        f.close();
    }
	char cm[3] = { ui->chkFile->isChecked() ? '1' : '0', ui->chkFolder->isChecked() ? '1' : '0', ui->chkFolderBack->isChecked() ? '1' : '0' };
	if (IsProcessRunAsAdmin())
	{
		if (QFile::exists(exe_file))
		{
			FuncBox::runCommand(exe_file, QStringList(QString(cm)));
			goto label1;
		}
		else
		{
            QMessageBox::critical(this, "错误", "工具丢失。", QMessageBox::Cancel, QMessageBox::Cancel);
			goto label2;
		}
	}
	else
	{
		if (QMessageBox::question(this, "警告", "没有管理员权限，是否提供权限以继续？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
		{

			if (GetAdmin(exe_file, QString(cm)))
				goto label1;
			else
			{
                QMessageBox::critical(this, "错误", "未成功获取权限。", QMessageBox::Cancel, QMessageBox::Cancel);
				goto label2;
			}
		}
		else
			return;
	}
label1:
	{
        jobTip->showTip("执行成功！");
		Menu_status[0] = ui->chkFile->isChecked() ? '1' : '0';
		Menu_status[1] = ui->chkFolder->isChecked() ? '1' : '0';
		Menu_status[2] = ui->chkFolderBack->isChecked() ? '1' : '0';
		return;
	}
label2:
	{
		ui->chkFile->setChecked(Menu_status[0] == '1');
		ui->chkFolder->setChecked(Menu_status[1] == '1');
		ui->chkFolderBack->setChecked(Menu_status[2] == '1');
		return;
	}
}

void Dialog::on_pBtnOpenAppData_clicked()
{
	FuncBox::runCommand("explorer", QStringList(FuncBox::get_dat_path().replace("/", "\\")));
}

void Dialog::on_pBtnOpenPicturePath_clicked()
{
	//do_the_Button_clicked(, 0);
	//QLineEdit *li = ui->linePictuerPath;
	QString str = ui->linePictuerPath->text();
	QDir dir;
	if (dir.exists(str) || (str.compare("") && dir.mkdir(str)))
		VarBox.FAMILY_PATH = str.replace("\\", "/");
	else
	{
		if (dir.exists(VarBox.FAMILY_PATH) || dir.mkdir(VarBox.FAMILY_PATH))
			ui->linePictuerPath->setText(VarBox.FAMILY_PATH.replace("/", "\\"));
		else
			VarBox.FAMILY_PATH = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
	}
	FuncBox::get_wal_path();
	if (dir.exists(VarBox.FAMILY_PATH))
		FuncBox::runCommand("explorer", QStringList(VarBox.FAMILY_PATH.replace("/", "\\")));
	else
		QMessageBox::warning(this, "警告", "路径不存在！", QMessageBox::Ok, QMessageBox::Ok);
}


void Dialog::on_linePictuerPath_returnPressed()
{
	QDir dir;
	if (dir.exists(ui->linePictuerPath->text()))
	{
		VarBox.FAMILY_PATH = ui->linePictuerPath->text().replace("\\", "/");
		FuncBox::get_wal_path();
	}
	else
	{
		QMessageBox::warning(this, "警告", "路径不存在！", QMessageBox::Ok, QMessageBox::Ok);
		ui->linePictuerPath->setText(VarBox.FAMILY_PATH.replace("/", "\\"));
	}
}

void Dialog::on_horizontalSlider_valueChanged(int value)
{
	DWORD bAlphaB = (DWORD)value << 24;
	ui->label_4->setText(QString::number(value, 10));
	bueaty.TraySave.dAlphaColor[bueaty.TraySave.SET_FULL] = bAlphaB + (bueaty.TraySave.dAlphaColor[bueaty.TraySave.SET_FULL] & 0xffffff);
}

void Dialog::on_horizontalSlider_2_valueChanged(int value)
{
	ui->label_6->setText(QString::number(value, 10));
	bueaty.TraySave.bAlpha[bueaty.TraySave.SET_FULL] = value;
}

void Dialog::on_horizontalSlider_3_valueChanged(int value)
{
	ui->label_17->setText(QString::number(value, 10));
	bueaty.beautifyTask->setInterval(value);
	bueaty.TraySave.refresh_time = value;
}

void Dialog::on_chkTimeUnit_min_clicked()
{
	bueaty.delKey(SHOW_SECONDS_IN_SYSTEM_CLOCK);
}

void Dialog::on_chkTimeUnit_sec_clicked()
{
	bueaty.setKey(SHOW_SECONDS_IN_SYSTEM_CLOCK, TRUE);
}

void Dialog::on_radioButton_3_clicked()
{
	bueaty.TraySave.aMode[bueaty.TraySave.SET_FULL] = ACCENT_DISABLED;
}

void Dialog::on_radioButton_4_clicked()
{
    if ((bueaty.TraySave.aMode[0] == ACCENT_DISABLED)&&(bueaty.TraySave.aMode[1] == ACCENT_DISABLED))
    {
        bueaty.beautifyTask->start();
    }
	bueaty.TraySave.aMode[bueaty.TraySave.SET_FULL] = ACCENT_ENABLE_TRANSPARENTGRADIENT;
}


void Dialog::on_radioButton_5_clicked()
{
    if ((bueaty.TraySave.aMode[0] == ACCENT_DISABLED)&&(bueaty.TraySave.aMode[1] == ACCENT_DISABLED))
    {
        bueaty.beautifyTask->start();
    }
	bueaty.TraySave.aMode[bueaty.TraySave.SET_FULL] = ACCENT_ENABLE_BLURBEHIND;
}

void Dialog::on_radioButton_6_clicked()
{
    if ((bueaty.TraySave.aMode[0] == ACCENT_DISABLED)&&(bueaty.TraySave.aMode[1] == ACCENT_DISABLED))
    {
        bueaty.beautifyTask->start();
    }
	bueaty.TraySave.aMode[bueaty.TraySave.SET_FULL] = ACCENT_ENABLE_ACRYLICBLURBEHIND;
}

void Dialog::on_pushButton_clicked()
{
	bueaty.savestyle();
    jobTip->showTip("保存完毕！");
}

void Dialog::on_radioButton_11_clicked()
{
	bueaty.setKey(L"TaskbarAcrylicOpacity", 10);
}

void Dialog::on_radioButton_10_clicked()
{
	bueaty.setKey(L"TaskbarAcrylicOpacity", 5);
}

void Dialog::on_radioButton_12_clicked()
{
	bueaty.setKey(L"TaskbarAcrylicOpacity", 0);
}

void Dialog::on_radioButton_7_clicked()
{
	bueaty.iPos = TASK_LEFT;
}

void Dialog::on_radioButton_8_clicked()
{
	if (bueaty.iPos == TASK_LEFT)
		bueaty.centerTask->start();
	bueaty.iPos = TASK_CENTER;
}

void Dialog::on_radioButton_9_clicked()
{
	if (bueaty.iPos == TASK_LEFT)
		bueaty.centerTask->start();
	bueaty.iPos = TASK_RIGHT;
}

void Dialog::changeType(BOOL ok)
{
	bueaty.TraySave.SET_FULL = ok;
    ui->horizontalSlider->setValue(bueaty.TraySave.dAlphaColor[ok] >> 24);
	ui->horizontalSlider_2->setValue(bueaty.TraySave.bAlpha[ok]);
	ui->pushButton_3->setStyleSheet(getStyleSheet());
	switch (bueaty.TraySave.aMode[ok])
	{
	case (ACCENT_DISABLED):
		ui->radioButton_3->setChecked(true);
		break;
	case (ACCENT_ENABLE_TRANSPARENTGRADIENT):
		ui->radioButton_4->setChecked(true);
		break;
	case (ACCENT_ENABLE_BLURBEHIND):
		ui->radioButton_5->setChecked(true);
		break;
	case (ACCENT_ENABLE_ACRYLICBLURBEHIND):
		ui->radioButton_6->setChecked(true);
		break;
	default:
		break;
	}
}

void Dialog::on_radioButton_clicked()
{
	changeType(false);
}

void Dialog::on_radioButton_2_clicked()
{
	changeType(true);
}

void Dialog::on_pushButton_3_clicked()
{

	QColor color = QColor(bueaty.TraySave.dAlphaColor[bueaty.TraySave.SET_FULL] & 0xffffff);
	color = QColor(color.blue(), color.green(), color.red());
    color = QColorDialog::getColor(color, ui->frame, "颜色");
	if (color.isValid())
	{
		short r = color.red(), g = color.green(), b = color.blue();
		QString str = QString("background-color: rgb(%1, %2, %3)").arg(QString::number(r), QString::number(g), QString::number(b));
		ui->pushButton_3->setStyleSheet(str);
        bueaty.TraySave.dAlphaColor[bueaty.TraySave.SET_FULL] = (ui->horizontalSlider->value() << 24) + RGB(r, g, b);
	}
}

void Dialog::on_comboBox_currentIndexChanged(int index)
{
	ui->lineNewName->setText(VarBox.FAMILY_NAMES[index]);
}


void Dialog::on_pBtnChange_clicked()
{
	QString str = ui->lineNewName->text();
	short i = ui->comboBox->currentIndex();
	if (VarBox.FAMILY_NAMES[i].compare(str))
	{
		if (str.isEmpty())
			QMessageBox::warning(this, "警告", "名称不能为空！", QMessageBox::Ok, QMessageBox::Ok);
		else
		{
			QString patrn = "[`~!@#$%^&-+=\\?:\"|,/;'\\[\\]·~！@#￥%……&*（）+=\\{\\}\\|《》？：“”【】、；‘'，。\\、\\-]";
			QRegExp rg(patrn);
			if (rg.indexIn(str) != -1)
				QMessageBox::warning(this, "警告", "名称含有非法字符！", QMessageBox::Ok, QMessageBox::Ok);
			else
			{
				QString new_name = (i == 7) ? VarBox.FAMILY_PATH + "/" + str : FuncBox::get_wal_path() + "/" + str;
				QDir dir;
				if (dir.exists(new_name))
				{
					if (QMessageBox::question(this, "冲突", "目标路径下有一个同名文件夹，您想直接更换文件夹而不迁移图片吗？",
						QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
					{
						VarBox.FAMILY_NAMES[i] = str;
						QSettings IniWrite(FuncBox::get_ini_path(), QSettings::IniFormat);
                        IniWrite.beginGroup("Dirs");
                        IniWrite.setValue(VarBox.PaperTypes[i], str);
                        IniWrite.endGroup();
                        jobTip->showTip("更改名称成功！");
					}
					else
					{
                        jobTip->showTip("放心，我什么也没做！");
                        ui->lineNewName->setText(VarBox.FAMILY_NAMES[i]);
					}
				}
				else
				{
					QString old_name;
					if (i == 7)
						old_name = VarBox.FAMILY_PATH + "/" + VarBox.FAMILY_NAMES[7];
					else
						old_name = FuncBox::get_wal_path() + "/" + VarBox.FAMILY_NAMES[i];
					if (dir.exists(old_name))
						dir.rename(old_name, new_name);
					else
						dir.mkdir(new_name);
					VarBox.FAMILY_NAMES[i] = str;
					QSettings IniWrite(FuncBox::get_ini_path(), QSettings::IniFormat);
                    IniWrite.beginGroup("Dirs");
                    IniWrite.setValue(VarBox.PaperTypes[i], str);
                    IniWrite.endGroup();
                    jobTip->showTip("更换名称成功！");
				}
			}
		}
	}
    else
    {
        jobTip->showTip("似乎什么也没做。");
    }
}

void Dialog::on_lineNewName_returnPressed()
{
	on_pBtnChange_clicked();

}

void Dialog::on_comboBox_2_currentIndexChanged(int index)
{
	switch (index)
	{
	case 0:
		ui->label_path_to_open->setText(qApp->applicationDirPath().replace("/", "\\"));
		ui->pushButton_4->setText("确定");
		break;
	case 1:
		ui->label_path_to_open->setText(VarBox.FAMILY_PATH.replace("/", "\\"));
		ui->pushButton_4->setText("确定");
		break;
	case 2:
		ui->label_path_to_open->setText((VarBox.FAMILY_PATH.replace("/", "\\") + "\\" + VarBox.FAMILY_NAMES[7]));
		ui->pushButton_4->setText("确定");
		break;
	case 3:
		ui->label_path_to_open->setText(FuncBox::get_dat_path().replace("/", "\\"));
		ui->pushButton_4->setText("确定");
		break;
	case 4:
		ui->label_path_to_open->setText("点击按钮选择文件");
		ui->pushButton_4->setText("选择文件夹");
		break;
	default:
		break;
	}
}


void Dialog::on_pushButton_4_clicked()
{
	if (ui->comboBox_2->currentIndex() == 4)
	{
		QString titile = "请选择一个文件夹";
        QString dir = QFileDialog::getExistingDirectory(ui->frame, titile, QStandardPaths::writableLocation(QStandardPaths::HomeLocation), QFileDialog::ShowDirsOnly);
		if (dir.compare(""))
		{
			ui->label_path_to_open->setText(dir.replace("/", "\\"));
			VarBox.PATH_TO_OPEN = dir.replace("\\", "/");
			FuncBox::save_the_open_path();
            jobTip->showTip("更换路径成功！");
			ui->pushButton_4->setText("确定");
		}
	}
	else
	{
		VarBox.PATH_TO_OPEN = ui->label_path_to_open->text().replace("\\", "/");
		FuncBox::save_the_open_path();
        jobTip->showTip("更换路径成功！");
	}

}

void Dialog::on_pBtn_Save_Tran_Info_clicked()
{
    QString str_1 = ui->line_APP_ID->text().trimmed();
    QString str_2 = ui->line_PASS_WORD->text().trimmed();
    if (str_1.isEmpty() || str_2.isEmpty())
        QMessageBox::information(ui->frame, "提示", "APP ID或密钥不能为空！");
    else
    {
        VarBox.APP_ID = str_1;
        VarBox.PASS_WORD = str_2;
        QFile file(FuncBox::get_son_dir("/scripts") + "/appid.txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream txtOutput(&file);
#if (QT_VERSION != QT_VERSION_CHECK(5,12,5))
        txtOutput << VarBox.APP_ID << Qt::endl;
        txtOutput << VarBox.PASS_WORD << Qt::endl;
#elif (QT_VERSION == QT_VERSION_CHECK(5,12,5))
        txtOutput << VarBox.APP_ID << endl;
        txtOutput << VarBox.PASS_WORD << endl;
#endif
            VarBox.HAVE_APP_RIGHT = true;
            jobTip->showTip("记录成功！");
        }
        else
        {
            QMessageBox::warning(ui->frame, "错误", "文件写入失败。");
        }
    }
}

void Dialog::on_pBtnCancel_2_clicked()
{
    close();
}

void del_file(Dialog *di ,QString str)
{
    if (QFile::exists(str))
    {
        if (QMessageBox::question(di, "提示", "本操作不可逆，是否继续？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)==QMessageBox::Yes)
        {
            if (QFile::remove(str))
            {
                di->jobTip->showTip("删除成功!");
            }
            else
                QMessageBox::critical(di, "出错", "无法删除，可能是权限不够！");
        }
    }
    else
    {
        di->jobTip->showTip("文件本就不存在！");
    }
}

void Dialog::on_pushButton_7_clicked()
{
    QString str = FuncBox::get_ini_path();
    if (QFile::exists(str))
    {
        if (QMessageBox::question(this, "提示", "将要清除所有设置并退出软件，是否继续？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)==QMessageBox::Yes)
        {
            if (QFile::remove(str))
            {
                jobTip->showTip("删除成功!");
                setEnabled(false);
                form->setEnabled(false);
                VarBox.RUN_APP = false;
                VarBox.RUN_NET_CHECK = false;
                if (QMessageBox::question(this, "提示", "您想退出后重启吗？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)==QMessageBox::Yes)
                    qApp->exit(RETCODE_RESTART);
                else
                    qApp->quit();
            }
            else
                QMessageBox::critical(this, "出错", "无法删除，可能是权限不够！");
        }
    }
    else
    {
        jobTip->showTip("文件本就不存在！");
    }
}


void Dialog::on_pushButton_6_clicked()
{
    del_file(this, FuncBox::get_dat_path()+"/ImgData.json");
}



void Dialog::on_pushButton_8_clicked()
{
    del_file(this, FuncBox::get_son_dir("/scripts")+"/appid.txt");
}


void Dialog::on_pushButton_2_clicked()
{
    close();
}

void Dialog::setTheme()
{
//    QPalette palette(ui->tabWidget->palette());
//    palette.setColor(QPalette::Background,QColor(color_theme[VarBox.cur_theme][0],color_theme[VarBox.cur_theme][1],
//                         color_theme[VarBox.cur_theme][2],color_theme[VarBox.cur_theme][3]));
    ui->frame->setStyleSheet(QString("QFrame{background-color:rgba(%1);}QLabel{border-radius: 3px;background-color: transparent;}Line{background-color:black};").arg(color_theme[VarBox.cur_theme]));
    jobTip->showTip("设置成功！");
//    ui->tabWidget->setAutoFillBackground(true);
//    ui->tabWidget->setPalette(palette);
}

void Dialog::on_pushButton_5_clicked()
{
    VarBox.cur_theme = (COLOR_THEME)ui->comboBox_3->currentIndex();
    QSettings IniWrite(FuncBox::get_ini_path(), QSettings::IniFormat);
    IniWrite.beginGroup("UI");
    IniWrite.setValue("ColorTheme", (int)VarBox.cur_theme);
    setTheme();
    qDebug() << (int)VarBox.cur_theme;
}

HINSTANCE pShellExecute(_In_opt_ HWND hwnd, _In_opt_ LPCWSTR lpOperation, _In_ LPCWSTR lpFile, _In_opt_ LPCWSTR lpParameters, _In_opt_ LPCWSTR lpDirectory, _In_ INT nShowCmd)
{
    HINSTANCE hInstance = NULL;
    typedef HINSTANCE(WINAPI* pfnShellExecute)(_In_opt_ HWND hwnd, _In_opt_ LPCWSTR lpOperation, _In_ LPCWSTR lpFile, _In_opt_ LPCWSTR lpParameters, _In_opt_ LPCWSTR lpDirectory, _In_ INT nShowCmd);
    HMODULE hShell32 = LoadLibrary(L"shell32.dll");
    if (hShell32)
    {
        pfnShellExecute pShellExecuteW = (pfnShellExecute)GetProcAddress(hShell32, "ShellExecuteW");
        if (pShellExecuteW)
            hInstance = pShellExecuteW(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
        FreeLibrary(hShell32);
    }
    return hInstance;
}

void Dialog::on_pushButton_9_clicked()
{
    pShellExecute(NULL, L"open", L"https://github.com/yjmthu", NULL, NULL, SW_SHOW);
}


void Dialog::on_pushButton_10_clicked()
{
    pShellExecute(NULL, L"open", L"https://www.52pojie.cn/home.php?mod=space&uid=1595632", NULL, NULL, SW_SHOW);
}

