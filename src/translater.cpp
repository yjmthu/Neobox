#include <thread>
#include <windows.h>
#include <winuser.h>
#include <QApplication>
#include <QClipboard>
#include <QButtonGroup>
#include <QMessageBox>
#include <QSettings>
#include <QMouseEvent>
#include <QTimer>
#include <QFile>
#include <QCryptographicHash>

#include "YString.h"
#include "YJson.h"
#include "form.h"
#include "blankform.h"
#include "gmpoperatetip.h"
#include "translater.h"

//#define appid "20210503000812254"
//#define password QString("Q_2PPxmCr66r6B2hi0ts")
#define M_WIN_HOT_KEY_SHIFT_Z 1001
#define M_WIN_HOT_KEY_SHIFT_A 1002

#define TRAN_HEIGHT 290
#define TRAN_WIDTH 240

Translater::Translater() :
    QWidget(nullptr),
    ui(new Ui::Translater),
    _en("en"), _zh("zh"), from(_zh), to(_en)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    QFile qss(":/qss/translater_style.qss");
    qss.open(QFile::ReadOnly);
    setStyleSheet(QString(qss.readAll()));
    qss.close();

	ui->setupUi(this);
    BlankFrom *blank = new BlankFrom(this); GMPOperateTip* jobTip = new GMPOperateTip(this);
    connect(blank->closeButton, &QPushButton::clicked, this, [=](){close(); emit enableself(false);});
    connect(blank->minButton, &QPushButton::clicked, this, &Translater::hide);
    blank->closeButton->setToolTip("退出"); blank->minButton->setToolTip("隐藏");
    blank->move(width()-100, 0);
	ui->TextFrom->installEventFilter(this);
    setMinimumSize(TRAN_WIDTH, TRAN_HEIGHT);
    setMaximumSize(TRAN_WIDTH, TRAN_HEIGHT);

    connect(this, &Translater::msgBox, this, [=](const char* str){
        jobTip->showTip(str);
    });

    if (RegisterHotKey(HWND(winId()), M_WIN_HOT_KEY_SHIFT_Z, MOD_SHIFT, 'Z'))
    {
        qDebug() << "注册热键 SHIFT + Z 成功.";
    }
    else
    {
        qout << "注册热键 SHIFT + Z 失败.";
    }
    if (RegisterHotKey(HWND(winId()), M_WIN_HOT_KEY_SHIFT_A, MOD_SHIFT, 'A'))
    {
        qDebug() << "注册热键  SHIFT + A 成功.";
    }
    else
    {
        qout << "注册热键 SHIFT + A 失败.";
    }
}

Translater::~Translater()
{
    UnregisterHotKey(HWND(winId()), M_WIN_HOT_KEY_SHIFT_Z);
    UnregisterHotKey(HWND(winId()), M_WIN_HOT_KEY_SHIFT_A);
    delete ui;
}

void Translater::showEvent(QShowEvent* event)
{
    int x, y;  RECT rt; ui->isFix->setChecked(!VarBox.AutoHide);
    static const int w = (GetWindowRect(HWND(winId()), &rt), rt.right - rt.left), h = (rt.bottom - rt.top), sw = GetSystemMetrics(SM_CXSCREEN);
    GetWindowRect(HWND(((Form*)VarBox.form)->winId()), &rt);
    if (rt.top > h)
        y = rt.top - h;
    else
        y = rt.bottom;
    if (rt.left + rt.right + w > sw * 2)
        x = sw - w;
    else if (rt.right + rt.left < w)
        x = 0;
    else
        x = (rt.left + rt.right - w) / 2;
    SetWindowPos(HWND(winId()), HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);

    QTextCursor cursor = ui->TextFrom->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->TextFrom->setTextCursor(cursor);
    ui->TextFrom->setFocus();
    if (VarBox.AutoHide)
        QTimer::singleShot(10000, this, [=](void){ if (isVisible() && VarBox.AutoHide) close();});
    event->accept();
}

bool Translater::nativeEvent(const QByteArray &eventType, void *message, long long *result)
{
    static HWND hCurrentCursor = NULL;
    Q_UNUSED(eventType);
    Q_UNUSED(result);
    MSG* msg = static_cast<MSG*>(message);
    switch (msg->message) {
    case WM_HOTKEY:
    {
        switch (msg->wParam) {
        case M_WIN_HOT_KEY_SHIFT_Z:
        {
            qout << "按下显示热键";
            hCurrentCursor = GetForegroundWindow();
            if (!isVisible()) show();
            activateWindow();  //SwitchToThisWindow(HWND(winId()), TRUE);
            QClipboard* cl = QApplication::clipboard();              //读取剪切板
            QString content = cl->text();
            if (content.isEmpty()) break;
            ui->TextFrom->setPlainText(content);
            qout << "剪贴板内容" << content;
            ushort uNum = content.at(0).unicode();
            if(uNum >= 0x4E00 && uNum <= 0x9FA5)
            {
                if (from != _zh)
                {
                    from = _zh;
                    to = _en;
                    ui->ZHTOEN->setChecked(true);
                }
            }
            else
            {
                if (from != _en)
                {
                    from = _en;
                    to = _zh;
                    ui->ENTOZH->setChecked(true);
                }
            }
            getReply(content.toUtf8());
            break;
        }
        case M_WIN_HOT_KEY_SHIFT_A:
        {
            qout << "按下隐藏热键";
            if (isVisible())
            {
                if (hCurrentCursor && IsWindow(hCurrentCursor) && IsWindowVisible(hCurrentCursor))
                    SetForegroundWindow(hCurrentCursor);
                hide();
            }
            break;
        }
        default:
            qout << "被注入了其他热键.";
            break;
        }
        return true;
    }
    case WM_KEYDOWN:
    {
        if (msg->wParam == VK_ESCAPE)
        {
            if (isVisible() && hCurrentCursor && IsWindow(hCurrentCursor) && IsWindowVisible(hCurrentCursor))
                SetForegroundWindow(hCurrentCursor);
            hide();
            return true;
        }
        break;
    }
    case WM_CLOSE:
    {
        qout << "关闭translater窗口";
        hide();
        return true;
    }
    case WM_SHOWWINDOW:
    {
        if (msg->wParam == SW_HIDE)
        {
            hCurrentCursor = NULL;
        }
        break;
    }
    default:
        break;
    }
    return false;
}

bool Translater::eventFilter(QObject* target, QEvent* event)
{
	if (target == ui->TextFrom)
	{
        if (event->type() == QEvent::KeyPress)               //
		{
			QKeyEvent* k = static_cast<QKeyEvent*>(event);

			if (k->key() == Qt::Key_Return)       //回车键
			{
                if (unfinished)
                {
                    event->accept();
                    return true;
                }
                unfinished = true;
                getReply(ui->TextFrom->toPlainText().toUtf8());
                event->accept();
				return true;
			}
			else if (k->key() == Qt::Key_Delete)  //删除键
			{
				ui->TextFrom->clear();
				ui->TextTo->clear();
                event->accept();
				return true;
			}
		}
	}
//    else if (target == ui->ENTOZH || target == ui->ZHTOEN)
//    {
//        if (event->type() == QEvent::MouseButtonDblClick)
//        {
//            if (static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
//            {
//                event->accept();
//                return true;
//            }
//        }
//    }
	return QWidget::eventFilter(target, event);
}

void Translater::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete)                                  //按下delete键清空要翻译的文本
	{
		ui->TextFrom->clear();
		ui->TextTo->clear();
		ui->TextFrom->setFocus();
        event->accept();
	}
	else if ((event->key() == Qt::Key_Alt) || (event->key() == Qt::Key_AltGr))  //按下Alt键切换中英文转换
	{
        if (unfinished)
        {
            event->accept();
            return;
        }
        unfinished = true;
        if (ui->ZHTOEN->isChecked())
        {
            ui->ENTOZH->setChecked(true);
            from = _en;
            to = _zh;
            getReply(ui->TextFrom->toPlainText().toUtf8());
        }
        else
        {
            ui->ZHTOEN->setChecked(true);
            from = _zh;
            to = _en;
            getReply(ui->TextFrom->toPlainText().toUtf8());
        }
        event->accept();
	}
	else
	{
	}
}

void Translater::requestData(const char* url, std::string* data)
{
    emit receivedData(FuncBox::getTransCode(url, data));
}

void Translater::getReply(const QByteArray& text)
{
    qout << "开始执行翻译函数";
    static const char API[] = u8"http://api.fanyi.baidu.com/api/trans/vip/translate";
    static const char salt[] = u8"1435660288";                           //请求参数之一
    unfinished = true;
    if (!text.length())
    {
        emit msgBox("内容为空！");
        unfinished = false;
        return;
    }
    std::string q; const char* utf8_q = text.data(), *sign = StrJoin(VarBox.AppId, utf8_q, salt, VarBox.PassWord);
    delete [] sign; sign = StrJoin(QCryptographicHash::hash(sign, QCryptographicHash::Md5).toHex());   //千万不要画蛇添足再delete[] sign, 也尽量不要在中途使用sign;
    while (*utf8_q)
    {
        if (isalnum((unsigned char)*utf8_q))
        {
            char tempbuff[2] = { 0 };
            sprintf_s(tempbuff, u8"%c", (unsigned char)*utf8_q);
            q.append(tempbuff);
        }
        else if (isspace((unsigned char)*utf8_q))
        {
            q.append("+");
        }
        else
        {
            char tempbuff[4];
            sprintf_s(tempbuff, u8"%%%X%X", ((unsigned char)*utf8_q) >> 4, ((unsigned char)*utf8_q) & 0xf);
            q.append(tempbuff);
        }
        ++utf8_q;
    }
    std::string reply_data; std::thread thrd; QEventLoop loop;
    connect(this, &Translater::receivedData, &loop, [&reply_data, &loop, &thrd, this](bool success){
        if (success)
        {
            qout << "翻译请求结果" << reply_data.c_str();
            YJsonItem json_data(reply_data); YJsonItem* have_item = NULL;
            if ((have_item = json_data.findItem(TEXT("trans_result"))) &&
                (have_item = have_item->getChildItem()) && (have_item = have_item->findItem(TEXT("dst"))))
            {
                if (have_item->getType() == YJson::YJSON_STRING)
                {
                    ui->TextTo->setPlainText(have_item->getValueSring());
                    QTextCursor cursor = ui->TextFrom->textCursor();
                    cursor.movePosition(QTextCursor::End);
                    ui->TextFrom->setTextCursor(cursor);
                }
                else
                    emit msgBox("未知错误。");
            }
            else
            {
                emit msgBox("翻译失败，可能是翻译速度过快\n翻译内容错误或者密钥信息错误。");
            }
        }
        else
        {
            emit msgBox("翻译请求失败,可能是密钥信息填写错误或者没有网络！");
        }
        if (thrd.joinable()) thrd.join(); loop.quit();
    });
    /* std::thrd不支持传引用，只支持传值 */
    //qout << "mysign" << sign;
    thrd = std::thread(&Translater::requestData, this, StrJoin(API, u8"?q=", q.c_str(), u8"&from=", from, "&to=", to, "&appid=", VarBox.AppId, "&salt=", salt, "&sign=", sign), &reply_data);
    delete [] sign; loop.exec();    //阻塞函数等待翻译结果，但是不阻塞ui。ui仍然可以编辑改动，不会卡死。
    qout << "翻译函数执行完毕"; unfinished = false;
}

void Translater::on_isFix_clicked(bool checked)
{
    VarBox.AutoHide = !checked;
    QSettings IniWrite(FuncBox::get_ini_path(), QSettings::IniFormat);
    IniWrite.beginGroup("Translate");
    IniWrite.setValue("AutoHide", VarBox.AutoHide);
    IniWrite.endGroup();
    if (VarBox.AutoHide)
        QTimer::singleShot(10000, this, [=](void){ if (isVisible() && VarBox.AutoHide) close();});
}

void Translater::on_ENTOZH_clicked(bool checked)
{
    if (unfinished) return;
    unfinished = true;
	if (checked)
	{
        from = _en;
        to = _zh;
	}
	else
	{
        from = _zh;
        to = _en;
	}
    getReply(ui->TextFrom->toPlainText().toUtf8());
}

void Translater::on_ZHTOEN_clicked(bool checked)
{
    if (unfinished) return;
    unfinished = true;
	if (checked)
	{
        from = _zh;
        to = _en;
	}
	else
	{
        from = _en;
        to = _zh;
	}
    getReply(ui->TextFrom->toPlainText().toUtf8());
}

void Translater::on_pBtnCopyTranlate_clicked()
{
	QApplication::clipboard()->setText(ui->TextTo->toPlainText());
    emit msgBox("复制成功！");
}
