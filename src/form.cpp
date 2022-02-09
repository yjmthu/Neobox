#include <fstream>

#include <QRect>
#include <QMessageBox>
#include <QTimer>
#include <QSettings>
#include <QPropertyAnimation>
#include <QTextCodec>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "translater.h"
#include "dialog.h"
#include "menu.h"
#include "netspeedhelper.h"
#include "YString.h"
#include "form.h"
#include "wallpaper.h"
#include "usbdrivehelper.h"
#include "systemfunctions.h"
#include "qstylesheet.h"
#include "YJson.h"

#ifdef Q_OS_WIN32
#include <Windows.h>
#include <dbt.h>
#elif defined Q_OS_LINUX
#include <QScreen>
#include <X11/Xlib.h>
#include <QX11Info>
#endif


Form::Form(QWidget* parent) :
    QWidget(parent),
    netHelper(new NetSpeedHelper)
{
    *const_cast<Form**>(&(VarBox->form)) = this;
    qout << "悬浮窗UI";
    initSettings();
    setupUi();                         //创建界面
    qout << "悬浮窗连接";
	initConnects();
}

Form::~Form()
{
    qout << "析构Form开始";
    for (auto i: m_usbHelpers)
        delete i;
    delete translater;
    delete animation;
    delete netHelper;
    qout << "析构Form结束";
}

void Form::saveBoxPos()
{
    const QPoint pos = this->pos();
    std::ofstream file(".speed-box", std::ios::out | std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(&pos), sizeof (QPoint));
        file.close();
    }
}

void Form::keepInScreen()
{
    QRect rt = geometry();
    int x = rt.left(), y = rt.top();
    if (x + 1 > VarBox->ScreenWidth)
        x = VarBox->ScreenWidth - 2;
    else if (rt.right() < 1)
        x = 2 - width();
    if (y + height() > VarBox->ScreenHeight)
        y = VarBox->ScreenHeight - height();
    else if (rt.bottom() < 1)
        y = 2 - height();
    move(x, y);
    saveBoxPos();
}

void Form::setupUi()
{
    QSize size(FORM_WIDTH, FORM_HEIGHT);
    setMinimumSize(size);
    setMaximumSize(size);
    setCursor(QCursor(Qt::PointingHandCursor));
    QHBoxLayout *horizontalLayout = new QHBoxLayout(this);
    frame = new QFrame(this), labUp = new QLabel(frame), labDown = new QLabel(frame), labMemory = new QLabel(frame);
    QHBoxLayout *hboxlayout = new QHBoxLayout(frame);
    QVBoxLayout *vboxlayout = new QVBoxLayout();
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    vboxlayout->setContentsMargins(0, 0, 0, 0);
    vboxlayout->addWidget(labUp);
    vboxlayout->addWidget(labDown);
    hboxlayout->setContentsMargins(0, 0, 0, 0);
    hboxlayout->setSpacing(0);
    hboxlayout->addWidget(labMemory);
    hboxlayout->addLayout(vboxlayout);
    horizontalLayout->addWidget(frame);
    labMemory->setAlignment(Qt::AlignCenter);
    labMemory->setText("0");
    labUp->setText("↑ 0.0 B");
    labDown->setText("↓ 0.0 B");
    labMemory->setMaximumWidth(30);

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    QFont font;
    bool changed = false;
    YJson* js = nullptr;
    if (!QFile::exists("BoxFont.json"))
    {
        QFile qfile(":/json/BoxFont.json");
        if (qfile.open(QFile::ReadOnly))
        {
            js = new YJson(static_cast<const char*>(qfile.readAll()) + 3); // 偏移3个代表utf-8 bom的字节
            changed = true;
            js->append((*js)["default"], "user");
        } else {
            return qApp->exit(RETCODE_ERROR_EXIT);
        }
    } else {
        js = new YJson("BoxFont.json", YJson::UTF8BOM);
    }
    YJson *ptr = js->find("user");
    YJson *js_ui[4] { ptr->find("frame"), ptr->find("labUp"), ptr->find("labDown"), ptr->find("labMemory")};
    QWidget* _ui[4] { frame, labUp, labDown, labMemory};
    for (int i = 0; i < 4; i++) {
        YJson* familyinfo = js_ui[i]->find("family");
        if (familyinfo->getType() == YJson::Null || !strcmp(familyinfo->getValueString(), "none") || !strcmp(familyinfo->getValueString(), "null") )
            continue;
        font.setFamily(familyinfo->getValueString());
        font.setBold(js_ui[i]->find("bold")->getType() == YJson::True);
        font.setItalic(js_ui[i]->find("italic")->getType() == YJson::True);
        _ui[i]->setFont(font);
    }
    frame->setToolTip(js->find("tip")->getValueString());
    if (changed) js->toFile("BoxFont.json", YJson::UTF8BOM, true);
    delete js;

    std::ifstream file(".speed-box", std::ios::in | std::ios::binary);
    if (file.is_open()) {
        QPoint pt;
        file.read(reinterpret_cast<char*>(&pt), sizeof (QPoint));
        file.close();
        move(pt);
    } else {
        move(100, 100);
    }

    if (VarBox->EnableTranslater) {
        enableTranslater(true);
    } else {
        translater = nullptr;                     // 防止野指针
    }
    loadStyle();
}


void Form::loadStyle()
{
    QStyleSheet *sheet;
    std::ifstream file_in(".boxstyle", std::ios::in | std::ios::binary);
    if (file_in.is_open()) {
        sheet = new QStyleSheet[4];
        file_in.read(reinterpret_cast<char *>(sheet), sizeof (QStyleSheet) * 4);
        file_in.close();
        /*  qout << sheet[0].getString("QFrame");
            qout << sheet[1].getString("QLabel");
            qout << sheet[2].getString("QLabel");
            qout << sheet[3].getString("QLabel");  */
    } else {
        sheet = new QStyleSheet[4]
        { /*{bk r    g    b   a    ft   r    g    b    a   bd  r  g  b  a   b   w  r fuz si win                            0  0  0 }*/
            { 255, 255, 255, 80, /**/   0,   0,   0,   0, /**/ 0, 0, 0, 0, /**/ 0, 3, 0,  1, 80,      QStyleSheet::TheAround, 0, 0 },
            {   0,   0,   0,  0, /**/   0, 255, 255, 255, /**/ 0, 0, 0, 0, /**/ 0, 3, 0, 17,  0,        QStyleSheet::TheLeft, 0, 0 },
            {   0,   0,   0,  0, /**/ 250, 170,  35, 255, /**/ 0, 0, 0, 0, /**/ 0, 3, 0,  8,  0,    QStyleSheet::TheTopRight, 0, 0 },
            {   0,   0,   0,  0, /**/ 140, 240,  30, 255, /**/ 0, 0, 0, 0, /**/ 0, 3, 0,  8,  0, QStyleSheet::TheBottomRight, 0, 0 }
        };
        QStyleSheet::toFile(sheet);
    }
    frame->setStyleSheet(sheet[0].getString(false));
    labMemory->setStyleSheet(sheet[1].getString(true));
    labUp->setStyleSheet(sheet[2].getString(true));
    labDown->setStyleSheet(sheet[3].getString(true));

    SystemFunctions::SetWindowCompositionAttribute(HWND(winId()), ACCENT_STATE::ACCENT_ENABLE_BLURBEHIND, (sheet->bk_win << 24) & RGB(sheet->bk_red, sheet->bk_green, sheet->bk_blue));
    delete [] sheet;
}

void Form::initSettings()
{
    QSettings IniRead(QStringLiteral("SpeedBox.ini"), QSettings::IniFormat);
    IniRead.setIniCodec(QTextCodec::codecForName("UTF-8"));
    IniRead.beginGroup(QStringLiteral("UI"));
    tieBianHide = IniRead.value(QStringLiteral("TieBianHide"), true).toBool();
    IniRead.endGroup();
}

void Form::initConnects()
{
    qout << "FORM链接A";
	animation = new QPropertyAnimation(this, "geometry");                  //用于贴边隐藏的动画
    connect(netHelper, &NetSpeedHelper::netInfo, this, [this](QString up, QString dw){
        labUp->setText(up);          //将上传速度显示出来
        labDown->setText(dw);        //将下载速度显示出来
    });
    connect(netHelper, &NetSpeedHelper::memInfo, labMemory, &QLabel::setText);
    connect(animation, &QPropertyAnimation::finished, this, &Form::saveBoxPos);
    connect(VarBox->wallpaper, &Wallpaper::setFailed, this, &Form::set_wallpaper_fail);
    qout << "FORM链接B";
}


void Form::set_wallpaper_fail(const char* str)
{
    if (VarBox->dialog) {
        if (VarBox->dialog->isVisible())
            VarBox->dialog->setWindowState(Qt::WindowActive | Qt::WindowNoState);    // 让窗口从最小化恢复正常并激活窗口
            //d->activateWindow();
            //d->raise();
        else
            VarBox->dialog->show();
    } else {
        *const_cast<Dialog**>(&(VarBox->dialog)) = new Dialog;
        VarBox->dialog->show();
    }
    VarBox->MSG(str, "出错");
}

#ifdef Q_OS_WIN32

char FirstDriveFromMask (ULONG unitmask)
{
    char i;

    for (i = 0; i < 26; ++i) {
        if (unitmask & 0x1)
            break;
        unitmask = unitmask >> 1;
    }
    return (i + 'A');
}

#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
    bool Form::nativeEvent(const QByteArray &, void *message, long *)
#else
    bool Form::nativeEvent(const QByteArray &, void *message, long long *)
#endif
{
    MSG *msg = static_cast<MSG*>(message);

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
//            qout << "设备插入";
            if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME && VarBox->enableUSBhelper)
            {
                PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                m_usbHelpers.push_back(new USBdriveHelper(FirstDriveFromMask(lpdbv->dbcv_unitmask), m_usbHelpers.size(), &m_usbHelpers, nullptr));
                connect(m_usbHelpers.back(), &USBdriveHelper::appQuit, this, [=](unsigned index){
                    auto iter = m_usbHelpers.begin() + index;
                    delete *iter;
                    iter = m_usbHelpers.erase(iter);
                    for (; iter < m_usbHelpers.end(); ++iter)
                    {
                        (*iter)->m_index -= 1;
                    }

                });
                m_usbHelpers.back()->show();
            }
            break;
        case DBT_DEVICEREMOVECOMPLETE:      //设备删除
            if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME  && !m_usbHelpers.empty()) {
                DEV_BROADCAST_VOLUME *db_volume = (DEV_BROADCAST_VOLUME *) lpdb;
                char drive = FirstDriveFromMask(db_volume->dbcv_unitmask);
                if (db_volume->dbcv_flags & DBTF_MEDIA) {
                    qDebug("Drive %c: Media has been removed.", drive);
                } else if (db_volume->dbcv_flags & DBTF_NET) {
                    qDebug("Drive %c: Network share has been removed.", drive);
                } else {
                    qDebug("Drive %c: Device has been removed.", drive);
                }
                for (auto iter = m_usbHelpers.begin(); iter < m_usbHelpers.end(); ++iter)
                {
                    if ((*iter)->diskId.front() == drive) {
                        for (auto i=iter+1; i < m_usbHelpers.end(); ++i)
                        {
                            (*i)->m_index -= 1;
                        }
                        delete *iter;
                        m_usbHelpers.erase(iter);
                        break;
                    }
                }
            }
            break;
        }
    }
    return false;
}
#endif

void Form::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)                   // 鼠标左键点击悬浮窗
	{
		setMouseTracking(true);                              // 开始跟踪鼠标位置，从而使悬浮窗跟着鼠标一起移动。
        m_ptPress = event->pos();                            // 记录开始位置
	}
	else if (event->button() == Qt::RightButton)             // 鼠标右键点击悬浮窗
    {

#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
        QPoint pt = event->globalPos();
#else
        QPointF pt = event->globalPosition();
#endif
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
        saveBoxPos();
		setMouseTracking(false);           // 停止跟踪鼠标。
	}
	event->accept();
}

void Form::mouseDoubleClickEvent(QMouseEvent*)
{
    if (VarBox->EnableTranslater)
    {
        if (!translater->isVisible()) {
            translater->show();
            translater->activateWindow();
        } else {
            translater->hide();
        }
    }
}

void Form::mouseMoveEvent(QMouseEvent* event)
{
#ifdef Q_OS_WIN32
    move(event->globalPos() - m_ptPress);               //当前位置加上位置变化情况，从而实现悬浮窗和鼠标同时移动。
#elif defined Q_OS_LINUX
    QPoint _curPos = event->globalPos() * 2;
    XEvent xe;
    memset(&xe, 0, sizeof(XEvent));

    Display *display = QX11Info::display();
    xe.xclient.type = ClientMessage;
    xe.xclient.message_type = XInternAtom(display, "_NET_WM_MOVERESIZE", False);
    xe.xclient.display = display;
    //wid 是当前程序的 window id，可以通过 QWidget->wId()获得，QWidget 必须实例化
    xe.xclient.window = (XID)(winId());
    xe.xclient.format = 32;
    xe.xclient.data.l[0] = _curPos.x();
    xe.xclient.data.l[1] = _curPos.y();
    xe.xclient.data.l[2] = 8;
    xe.xclient.data.l[3] = Button1;
    xe.xclient.data.l[4] = 1;

    XUngrabPointer(display, CurrentTime);
    XSendEvent(display,
               QX11Info::appRootWindow(QX11Info::appScreen()),
               False,
               SubstructureNotifyMask | SubstructureRedirectMask,
               &xe);
    XFlush(display);
#endif
	event->accept();
}

#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
    void Form::enterEvent(QEvent* event)
#else
    void Form::enterEvent(QEnterEvent* event)
#endif
{
    if (tieBianHide)
    {
        const QPoint pos = this->pos();
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
    }
    if (event) event->accept();
}

void Form::leaveEvent(QEvent* event)
{
    if (tieBianHide)
    {
        QPoint pos = this->pos();
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
    if (checked) {
        VarBox->EnableTranslater = true;
        translater = new Translater;
    } else {
        VarBox->EnableTranslater = false;
        delete translater;
        translater = nullptr;
    }
    VARBOX::saveOneSet<bool>(QStringLiteral("Translate"), QStringLiteral("EnableTranslater"), VarBox->EnableTranslater);
}
