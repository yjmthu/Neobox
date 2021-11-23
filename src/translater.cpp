#include <thread>
#include <sstream>
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
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <sapi.h> //导入语音头文件

#include "YEncode.h"
#include "YString.h"
#include "YJson.h"
#include "form.h"
#include "blankform.h"
#include "gmpoperatetip.h"
#include "translater.h"

#define M_WIN_HOT_KEY_SHIFT_Z 1001
#define M_WIN_HOT_KEY_SHIFT_A 1002

#define TRAN_HEIGHT 300
#define TRAN_WIDTH 240

bool  MSSpeak(LPCTSTR speakContent) // 文字转为语音
{
    ISpVoice *pVoice = NULL;  //初始化COM接口
    if (FAILED(::CoInitialize(NULL)))
        return false;         // MessageBoxW(NULL, (LPCWSTR)L"COM接口初始化失败！", (LPCWSTR)L"提示", MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2); //获取SpVoice接口
    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
    if (SUCCEEDED(hr))
    {
        pVoice->SetVolume((USHORT)100);    //设置音量，范围是 0 -100
        pVoice->SetRate(0);                //设置速度，范围是 -10 - 10
        pVoice->Speak(speakContent, 0, NULL);
        pVoice->Release();
        pVoice = NULL;
    }
    CoUninitialize();                      //释放com资源
    return true;
}

Translater::Translater() :
    QWidget(nullptr),
    ui(new Ui::Translater),
    _en("en"), _zh("zh"), from(_zh), to(_en),
    mgr(new QNetworkAccessManager)
{
    mgr->setTransferTimeout(1000);
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
    ui->TextFrom->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->TextTo->setContextMenuPolicy(Qt::CustomContextMenu);

    if (*VarBox->FirstUse)
    {
        ui->TextFrom->setPlainText("在这输入你想翻译的文本,一秒钟只能翻译一次。");
        ui->TextTo->setPlainText("Enter the text you want to translate here. You can only translate it once a second.");
        *const_cast<bool*>(VarBox->FirstUse) = false;
    }
    auto btngrp = new QButtonGroup(this);
    btngrp->addButton(ui->pBtnEnToZh, 0);
    btngrp->addButton(ui->pBtnZhToEn, 1);
    btngrp->setExclusive(true);

    setMinimumSize(TRAN_WIDTH, TRAN_HEIGHT);
    setMaximumSize(TRAN_WIDTH, TRAN_HEIGHT);

    connect(this, &Translater::msgBox, [=](const char* str){
        jobTip->showTip(str);
    });
    RegisterHotKey(HWND(winId()), M_WIN_HOT_KEY_SHIFT_Z, MOD_SHIFT, 'Z');
    RegisterHotKey(HWND(winId()), M_WIN_HOT_KEY_SHIFT_A, MOD_SHIFT, 'A');
    initConnects();
}

Translater::~Translater()
{
    qout << "析构Translater开始";
    qout << "取消注册热键";
    UnregisterHotKey(HWND(winId()), M_WIN_HOT_KEY_SHIFT_Z);
    UnregisterHotKey(HWND(winId()), M_WIN_HOT_KEY_SHIFT_A);
    qout << "删除mgr";
    delete mgr;
    qout << "删除ui";
    delete ui;
    qout << "析构Translater结束";
}

void Translater::initConnects()
{
    connect(ui->pBtnPin, &QPushButton::clicked, this, &Translater::setFix);
    connect(ui->pBtnEnToZh, &QPushButton::clicked, this, &Translater::startEnToZh);
    connect(ui->pBtnZhToEn, &QPushButton::clicked, this, [=](bool checked){startEnToZh(!checked);});
    connect(ui->pBtnCopyTranlate, &QPushButton::clicked, this, &Translater::copyTranlate);
    connect(ui->bBtnClean, &QPushButton::clicked, this, [this](){
        ui->TextFrom->clear();ui->TextTo->clear();
    });
    QString menu_style("QMenu{"
                    "border-radius:3px;"
                    "background-color: white;"
                    "color: black;"
                    "border: 1px solid rgb(0,255,255);"
                "}"
                "QMenu::item {"
                    "background-color: transparent;"
                    "padding: 6px 3px;"
                    "border-radius: 3px;"
                "}"
                "QMenu::item:selected {"
                    "background-color: yellow;"
                    "border-radius: 3px;"
                    "padding: 6px 3px;"
                "}");
    connect(ui->TextFrom, &QPlainTextEdit::customContextMenuRequested, ui->TextFrom, [=](const QPoint &pos){
        QMenu* menu = ui->TextFrom->createStandardContextMenu();
        menu->setStyleSheet(menu_style);
        menu->connect(menu->addAction("Read All"), &QAction::triggered, [this](){MSSpeak(ui->TextFrom->toPlainText().toStdWString().c_str());});
        menu->exec((pos + ui->TextFrom->pos()) + this->pos());
        delete menu;
    });
    connect(ui->TextTo, &QPlainTextEdit::customContextMenuRequested, ui->TextFrom, [=](const QPoint &pos){
        QMenu* menu = ui->TextTo->createStandardContextMenu();
        menu->setStyleSheet(menu_style);
        menu->connect(menu->addAction("Read All"), &QAction::triggered, [this](){MSSpeak(ui->TextTo->toPlainText().toStdWString().c_str());});
        menu->exec((pos + ui->TextTo->pos()) + this->pos());
        delete menu;
    });
}

void Translater::showEvent(QShowEvent* event)
{
    int x, y;  RECT rt; ui->pBtnPin->setChecked(!VarBox->AutoHide);
    ui->pBtnPin->setIcon(QIcon(VarBox->AutoHide?":/icons/drip_pin.ico": ":/icons/drip_blue_pin.ico"));
    int w = (GetWindowRect(HWND(winId()), &rt), rt.right - rt.left), h = (rt.bottom - rt.top), sw = GetSystemMetrics(SM_CXSCREEN);
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
        QTimer::singleShot(10000, this, [=](void){ if (isVisible() && VarBox->AutoHide) hide();});
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
                if (from == _en)
                {
                    ui->pBtnZhToEn->click();
                    break;
                }
            }
            else
            {
                if (from == _zh)
                {
                    ui->pBtnEnToZh->click();
                    break;
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
                QString text = ui->TextFrom->toPlainText();
                auto cur = ui->TextFrom->textCursor();
                if (!text.length()) goto label_end;
                else if (cur.position() >= QTextCursor::Start && text.at(cur.position()-1) == ' ')
                {
                    cur.deletePreviousChar();
                    cur.insertText("\n");
                    goto label_end;
                }
                getReply(text.toUtf8());
label_end:
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
        if (ui->pBtnZhToEn->isChecked())
            ui->pBtnEnToZh->click();
        else
            ui->pBtnZhToEn->click();
        event->accept();
	}
}

void Translater::getReply(const QByteArray& q)
{
    qout << "开始执行翻译函数";
    constexpr char salt[] = u8"1435660288";                           //请求参数之一

    auto time_now = GetTickCount();
    if (time_now - last_post_time < 1000) Sleep(last_post_time + 1000 - time_now);

    //std::string &&q = urlEncode(utf8_q, strlen(utf8_q));

    const char* utf8_q = StrJoin<char, const char*>(VarBox->AppId, q, salt, VarBox->PassWord);
    QByteArray sign = QCryptographicHash::hash(utf8_q, QCryptographicHash::Md5).toHex();
    delete [] utf8_q;

    connect(mgr, &QNetworkAccessManager::finished, this, [this](QNetworkReply* rep) {
        if (rep->error() != QNetworkReply::NoError)
        {
            emit msgBox("翻译请求失败,可能是没有网络！");
            return ;
        }
        YJson json_data(rep->readAll());
        YJson* have_item = nullptr;
        if ((have_item = json_data.find("trans_result")) &&
            (have_item = have_item->getChild()))
        {
            YJson * temp = nullptr;
            ui->TextTo->clear();
            do {
                temp = have_item->find("dst");
                ui->TextTo->appendPlainText(temp->getValueString());
            } while (have_item = have_item->getNext());
            QTextCursor cursor = ui->TextFrom->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->TextFrom->setTextCursor(cursor);
            last_post_time = GetTickCount();
        }
        else if (json_data.find("error_code"))
        {
            if (have_item = json_data.find("error_msg"))
            {
                qout << "错误消息：" << have_item->getValueString();
                if (!strcmp(have_item->getValueString(), "Invalid Access Limit"))
                    emit msgBox("翻译失败，使用公共密钥造成请求间隔时间过短!");
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
    });
    QString url = "http://api.fanyi.baidu.com/api/trans/vip/translate?q=%1&from=%2&to=%3&appid=%4&salt=%5&sign=%6";
    mgr->get(QNetworkRequest(QUrl(url.arg(q, from, to, VarBox->AppId, salt, sign))));
}

void Translater::setFix(bool checked)
{
    ui->pBtnPin->setIcon(QIcon(checked?":/icons/drip_blue_pin.ico":":/icons/drip_pin.ico"));
    VarBox->AutoHide = !checked;
    QSettings IniWrite("SpeedBox.ini", QSettings::IniFormat);
    IniWrite.beginGroup("Translate");
    IniWrite.setValue("AutoHide", VarBox->AutoHide);
    IniWrite.endGroup();
    if (VarBox->AutoHide)
        QTimer::singleShot(10000, this, [=](void){ if (isVisible() && VarBox->AutoHide) hide();});
}

void Translater::startEnToZh(bool checked)
{
    ui->pBtnEnToZh->setIcon(QIcon(checked?":/icons/black_zh.ico": ":/icons/empty_zh.ico"));
    ui->pBtnZhToEn->setIcon(QIcon(checked?":/icons/empty_en.ico":":/icons/black_en.ico"));
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
    QString str = ui->TextFrom->toPlainText();
    if (!str.isEmpty()) getReply(str.toUtf8());
}

void Translater::copyTranlate()
{
	QApplication::clipboard()->setText(ui->TextTo->toPlainText());
    emit msgBox("复制成功！");
}
