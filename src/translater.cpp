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
    connect(blank->closeButton, &QPushButton::clicked, [](){VarBox->form->enableTranslater(false);});
    connect(blank->minButton, &QPushButton::clicked, this, &Translater::hide);
    blank->closeButton->setToolTip("退出"); blank->minButton->setToolTip("隐藏");
    blank->move(width()-100, 0);
	ui->TextFrom->installEventFilter(this);

    if (*VarBox->FirstUse)
    {
        ui->TextFrom->setPlainText("一秒钟只能翻译一次。");
        ui->TextTo->setPlainText("You can only translate once a second.");
        *const_cast<bool*>(VarBox->FirstUse) = false;
    }

    setMinimumSize(TRAN_WIDTH, TRAN_HEIGHT);
    setMaximumSize(TRAN_WIDTH, TRAN_HEIGHT);

    connect(this, &Translater::msgBox, [=](const char* str){
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
    qout << "析构Translater开始";
    UnregisterHotKey(HWND(winId()), M_WIN_HOT_KEY_SHIFT_Z);
    UnregisterHotKey(HWND(winId()), M_WIN_HOT_KEY_SHIFT_A);
    delete ui;
    qout << "析构Translater结束";
}

void Translater::showEvent(QShowEvent* event)
{
    int x, y;  RECT rt; ui->isFix->setChecked(!VarBox->AutoHide);
    static const int w = (GetWindowRect(HWND(winId()), &rt), rt.right - rt.left), h = (rt.bottom - rt.top), sw = GetSystemMetrics(SM_CXSCREEN);
    GetWindowRect(HWND(VarBox->form->winId()), &rt);
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
    if (VarBox->AutoHide)
        QTimer::singleShot(10000, this, [=](void){ if (isVisible() && VarBox->AutoHide) close();});
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
    std::string q;
    const char* utf8_q = text;
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

    utf8_q = StrJoin(VarBox->AppId, (const char*)text, salt, VarBox->PassWord);
    QByteArray sign = QCryptographicHash::hash(utf8_q, QCryptographicHash::Md5).toHex();
    ///char sign[33]; strcpy_s(sign, 33, QCryptographicHash::hash(utf8_q, QCryptographicHash::Md5).toHex());
    delete [] utf8_q;
    qout << "mysign" << sign.data();

    std::string reply_data; std::thread thrd; QEventLoop loop;

    connect(this, &Translater::finished, &loop, [&](bool success){
        qout << "处理请求结果";
        if (thrd.joinable()) thrd.join(); loop.quit();
        if (!success)
        {
            emit msgBox("翻译请求失败,可能是没有网络！");
            return ;
        }
        qout << "翻译请求结果" << reply_data.c_str();
        YJsonItem json_data(reply_data); YJsonItem* have_item = nullptr;
        if ((have_item = json_data.findItem("trans_result")) &&
            (have_item = have_item->getChildItem()) && (have_item = have_item->findItem("dst")))
        {
            qout << "没有错误";
            if (have_item->getType() == YJSON_TYPE::YJSON_STRING)
            {
                ui->TextTo->setPlainText(have_item->getValueString());
                QTextCursor cursor = ui->TextFrom->textCursor();
                cursor.movePosition(QTextCursor::End);
                ui->TextFrom->setTextCursor(cursor);
            }
            else
                emit msgBox("未知错误。");
        }
        else
        {
            if (json_data.findItem("error_code"))
            {
                if (have_item = json_data.findItem("error_msg"))
                {
                    qout << "错误消息：" << have_item->getValueString();
                    if (!strcmp(have_item->getValueString(), "Invalid Access Limit"))
                        emit msgBox("翻译失败，请求间隔时间过短!");
                    else if (!strcmp(have_item->getValueString(), "UNAUTHORIZED USER"))
                        emit msgBox("翻译失败，APPID不存在！");
                    else if (!strcmp(have_item->getValueString(), "Invalid Sign"))
                        emit msgBox("翻译失败，密钥错误！");
                    else
                        emit msgBox("翻译失败， 其它错误。");
                }
                else
                    emit msgBox("翻译失败，未知原因。");
            }
            else
                emit msgBox("翻译失败，未知错误");
        }
    });
    qout << "开始执行线程";
    thrd = std::thread([&](){
        if (VARBOX::getTransCode(StrJoin(API, u8"?q=", q.c_str(), u8"&from=", from, u8"&to=", to, u8"&appid=", VarBox->AppId, u8"&salt=", salt, u8"&sign=", sign.data()), reply_data))
            emit finished(true);
        else
            emit finished(false);
        qout << "线程执行完毕";
    });
    loop.exec();    //阻塞函数等待翻译结果，但是不阻塞ui。ui仍然可以编辑改动，不会卡死。
    qout << "翻译函数执行完毕";
    unfinished = false;
}

void Translater::on_isFix_clicked(bool checked)
{
    VarBox->AutoHide = !checked;
    QSettings IniWrite(VarBox->get_ini_path(), QSettings::IniFormat);
    IniWrite.beginGroup("Translate");
    IniWrite.setValue("AutoHide", VarBox->AutoHide);
    IniWrite.endGroup();
    if (VarBox->AutoHide)
        QTimer::singleShot(10000, this, [=](void){ if (isVisible() && VarBox->AutoHide) close();});
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
