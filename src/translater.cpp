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

#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
#include <QTextCodec>
#include <QTextToSpeech> //导入语音头文件

#include <3rd_qxtglobalshortcut/qxtglobalshortcut.h>
#else
#include <sapi.h> //导入语音头文件
#endif

#include "YEncode.h"
#include "YString.h"
#include "YJson.h"
#include "form.h"
#include "blankform.h"
#include "gmpoperatetip.h"
#include "translater.h"

#define TRAN_HEIGHT 300
#define TRAN_WIDTH 240

#if (QT_VERSION_CHECK(6,0,0) <= QT_VERSION)
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
#endif

Translater::Translater() :
    QWidget(nullptr),
    ui(new Ui::Translater), type(Type::ZH_CN2EN),
    shortcut_show(new QxtGlobalShortcut(QKeySequence("Shift+Z"))),
    shortcut_hide(new QxtGlobalShortcut(QKeySequence("Shift+A"))),
    mgr(new QNetworkAccessManager), timer(new QTimer)
    #if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
        , speaker(new QTextToSpeech)
    #endif
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
    connect(blank->closeButton, &QPushButton::clicked, VarBox, [](){VarBox->form->enableTranslater(false);});
    connect(blank->minButton, &QPushButton::clicked, this, &Translater::hide);
    blank->closeButton->setToolTip("退出"); blank->minButton->setToolTip("隐藏");
    blank->move(width()-100, 0);
	ui->TextFrom->installEventFilter(this);
    ui->TextFrom->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->TextTo->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(timer, &QTimer::timeout, this, [=](){
        if (isVisible() && --time_left <= 0 && VarBox->AutoHide)
            hide();
    });

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

    connect(this, &Translater::msgBox, this, [=](const char* str){
        jobTip->showTip(str);
    });
    connect(shortcut_show, &QxtGlobalShortcut::activated, this, [=](){
        qout << "按下显示热键";
        hCurrentCursor = GetForegroundWindow();
        if (!isVisible()) show();
        activateWindow();  //SwitchToThisWindow(HWND(winId()), TRUE);
        QClipboard* cl = QApplication::clipboard();              //读取剪切板
        QString content = cl->text();
        if (content.isEmpty()) return ;
        ui->TextFrom->setPlainText(content);
        qout << "剪贴板内容" << content;
        ushort uNum = content.at(0).unicode();
        if(uNum >= 0x4E00 && uNum <= 0x9FA5)
        {
            if (type == Type::EN2ZH_CN)
            {
                ui->pBtnZhToEn->click();
                return ;
            }
        }
        else
        {
            if (type == Type::ZH_CN2EN)
            {
                ui->pBtnEnToZh->click();
                return ;
            }
        }
        getReply(content.toUtf8());
    });
    connect(shortcut_hide, &QxtGlobalShortcut::activated, this, [=](){
        qout << "按下隐藏热键";
        if (isVisible())
        {
            if (hCurrentCursor && IsWindow(hCurrentCursor) && IsWindowVisible(hCurrentCursor))
                SetForegroundWindow(hCurrentCursor);
            hide();
        }
    });
    initConnects();
}

Translater::~Translater()
{
#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
    delete speaker;
#endif
    qout << "析构Translater开始";
    qout << "删除mgr";
    delete timer;
    delete mgr;
    qout << "删除ui";
    delete shortcut_hide;
    delete shortcut_show;
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
        time_left = 10;
        ui->TextFrom->clear();ui->TextTo->clear();
        ui->TextFrom->setFocus();
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
        menu->connect(menu->addAction("Read All"), &QAction::triggered, this, [this](){

#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
        speaker->say(ui->TextFrom->toPlainText());
#else
        MSSpeak(ui->TextFrom->toPlainText().toStdWString().c_str());
#endif
        });
        menu->exec((pos + ui->TextFrom->pos()) + this->pos());
        delete menu;
    });
    connect(ui->TextTo, &QPlainTextEdit::customContextMenuRequested, ui->TextFrom, [=](const QPoint &pos){
        QMenu* menu = ui->TextTo->createStandardContextMenu();
        menu->setStyleSheet(menu_style);
        menu->connect(menu->addAction("Read All"), &QAction::triggered, this, [this](){
#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
        speaker->say(ui->TextTo->toPlainText());
#else
        MSSpeak(ui->TextTo->toPlainText().toStdWString().c_str());
#endif
        });
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
    if (VarBox->AutoHide) timer->start(1000);
    event->accept();
}

#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
bool Translater::nativeEvent(const QByteArray &, void *message, long *)
#else
bool Translater::nativeEvent(const QByteArray &, void *message, long long *)
#endif
{
    MSG* msg = static_cast<MSG*>(message);
    switch (msg->message) {
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

void Translater::mouseReleaseEvent(QMouseEvent *event)
{
    time_left = 10;
    event->accept();
}

void Translater::mousePressEvent(QMouseEvent *event)
{
    time_left = 10;
    event->accept();
}

void Translater::hideEvent(QHideEvent *event)
{
    timer->stop();
    event->accept();
}

void Translater::closeEvent(QCloseEvent *event)
{
    qout << "关闭translater窗口";
    hide();
    event->accept();
}

bool Translater::eventFilter(QObject* target, QEvent* event)
{
	if (target == ui->TextFrom)
	{
        time_left = 10;
        if (event->type() == QEvent::KeyPress)               //
		{
			QKeyEvent* k = static_cast<QKeyEvent*>(event);

			if (k->key() == Qt::Key_Return)       //回车键
			{
                QString text = ui->TextFrom->toPlainText();
                auto cur = ui->TextFrom->textCursor();
                if (!text.length())
                    goto label_end;
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
    else if (target == ui->TextTo)
    {
        time_left = 10;
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
    constexpr char youdao_api[] = "http://fanyi.youdao.com/translate?&doctype=json&type=%1&i=%2";
    qout << "开始执行翻译函数";

    auto time_now = GetTickCount();
    if (time_now - last_post_time < 1000) Sleep(last_post_time + 1000 - time_now);

    connect(mgr, &QNetworkAccessManager::finished, this, [this](QNetworkReply* rep) {
        if (rep->error() != QNetworkReply::NoError)
        {
            emit msgBox("翻译请求失败,可能是没有网络！");
            return ;
        }
        YJson json_data(rep->readAll());
        YJson* have_item = nullptr;
        if ((have_item = json_data.find("translateResult")) &&
            (have_item = have_item->getChild()))
        {
            YJson * temp = nullptr;
            ui->TextTo->clear();
            do {
                temp = have_item->getChild()->find("tgt");
                ui->TextTo->appendPlainText(temp->getValueString());
            } while (have_item = have_item->getNext());
            QTextCursor cursor = ui->TextFrom->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->TextFrom->setTextCursor(cursor);
            last_post_time = GetTickCount();
        }//"errorCode":0
        else if (json_data.find("errorCode"))
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
    mgr->get(QNetworkRequest(QUrl(QString(youdao_api).arg(types[type], (const char*)q))));
}

void Translater::setFix(bool checked)
{
    ui->pBtnPin->setIcon(QIcon(checked?":/icons/drip_blue_pin.ico":":/icons/drip_pin.ico"));
    VarBox->AutoHide = !checked;
    QSettings IniWrite("SpeedBox.ini", QSettings::IniFormat);
    IniWrite.setIniCodec(QTextCodec::codecForName("UTF-8"));
    IniWrite.beginGroup("Translate");
    IniWrite.setValue("AutoHide", VarBox->AutoHide);
    IniWrite.endGroup();
    if (VarBox->AutoHide)
        timer->start(1000);
}

void Translater::startEnToZh(bool checked)
{
    time_left = 10;
    ui->pBtnEnToZh->setIcon(QIcon(checked?":/icons/black_zh.ico": ":/icons/empty_zh.ico"));
    ui->pBtnZhToEn->setIcon(QIcon(checked?":/icons/empty_en.ico":":/icons/black_en.ico"));
    type = checked ? Type::EN2ZH_CN : Type::ZH_CN2EN;
    QString str = ui->TextFrom->toPlainText();
    if (!str.isEmpty()) getReply(str.toUtf8());
}

void Translater::copyTranlate()
{
    time_left = 10;
	QApplication::clipboard()->setText(ui->TextTo->toPlainText());
    emit msgBox("复制成功！");
}
