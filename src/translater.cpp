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

#if defined (Q_OS_WIN32)
#include <QThread>
#include <windows.h>
#include <winuser.h>
#elif defined (Q_OS_LINUX)
#include <sys/time.h>
#include <unistd.h>
#endif

#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
#include <QTextCodec>
#include <QTextToSpeech> //导入语音头文件
#include <menu.h>

#include <3rd_qxtglobalshortcut/qxtglobalshortcut.h>
#else
#include <sapi.h> //导入语音头文件
#endif

#include <thread>
#include <sstream>
#include <algorithm>

#include "yencode.h"
#include "ystring.h"
#include "yjson.h"
#include "form.h"
#include "blankform.h"
#include "gmpoperatetip.h"
#include "translater.h"
#include "globalfn.h"

constexpr size_t Translater::m_dLangPos;
bool Translater::m_bAutoHide  { false };


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
    ui(new Ui::Translater),
    m_pMgr(new QNetworkAccessManager), m_pTimer(new QTimer)
    #if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
        , m_pSpeaker(new QTextToSpeech)
    #endif
{
    initSettings();
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    QFile qss(QStringLiteral(":/qss/translater_style.qss"));
    qss.open(QFile::ReadOnly);
    setStyleSheet(QString(qss.readAll()));
    qss.close();

	ui->setupUi(this);
    BlankFrom *blank = new BlankFrom(this); GMPOperateTip* jobTip = new GMPOperateTip(this);
    connect(blank->closeButton, &QPushButton::clicked, VarBox, [](){VarBox->m_pForm->enableTranslater(false);});
    connect(blank->minButton, &QPushButton::clicked, this, &Translater::hide);
    blank->closeButton->setToolTip(QStringLiteral("退出"));
    blank->minButton->setToolTip(QStringLiteral("隐藏"));
    blank->move(width()-100, 0);
	ui->TextFrom->installEventFilter(this);
    ui->TextFrom->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->TextTo->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(m_pTimer, &QTimer::timeout, this, [=](){
        if (isVisible() && --m_dTimeLeft <= 0 && m_bAutoHide)
            hide();
    });

    if (VarBox->m_bFirstUse)
    {
        ui->TextFrom->setPlainText(QStringLiteral("在这输入你想翻译的文本,一秒钟只能翻译一次。"));
        ui->TextTo->setPlainText(QStringLiteral("Enter the text you want to translate here. You can only translate it once a second."));
        *const_cast<bool*>(&VarBox->m_bFirstUse) = false;
    }
    auto btngrp = new QButtonGroup(this);
    btngrp->addButton(ui->pBtnEnToZh, 0);
    btngrp->addButton(ui->pBtnZhToEn, 1);
    btngrp->setExclusive(true);


    constexpr int TRAN_HEIGHT = 300;
    constexpr int TRAN_WIDTH = 240;

    const QSize size(TRAN_WIDTH, TRAN_HEIGHT);

    setMinimumSize(size);
    setMaximumSize(size);

    connect(this, &Translater::msgBox, this, [=](const char* str){ jobTip->showTip(str); });

    initConnects();
}

Translater::~Translater()
{
#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
    delete m_pSpeaker;
#endif
    qout << "析构Translater开始";
    delete m_pTimer;
    delete m_pMgr;
    delete m_pShortcutHide;
    delete m_pShortcutShow;
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
        m_dTimeLeft = 10;
        ui->TextFrom->clear();ui->TextTo->clear();
        ui->TextFrom->setFocus();
    });
    QString menu_style(QStringLiteral("QMenu{"
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
        "}"));
    connect(ui->TextFrom, &QPlainTextEdit::customContextMenuRequested, ui->TextFrom, [=](const QPoint &pos){
        QMenu* menu = ui->TextFrom->createStandardContextMenu();
        menu->setStyleSheet(menu_style);
        menu->connect(menu->addAction(QStringLiteral("Read All")), &QAction::triggered, this, [this](){

#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
        m_pSpeaker->say(ui->TextFrom->toPlainText());
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
        m_pSpeaker->say(ui->TextTo->toPlainText());
#else
        MSSpeak(ui->TextTo->toPlainText().toStdWString().c_str());
#endif
        });
        menu->exec((pos + ui->TextTo->pos()) + this->pos());
        delete menu;
    });
}

void Translater::initSettings()
{
    QSettings IniRead(QStringLiteral("SpeedBox.ini"), QSettings::IniFormat);
    IniRead.setIniCodec(QTextCodec::codecForName("UTF-8"));
    IniRead.beginGroup(QStringLiteral("Translate"));
    m_bAutoHide = IniRead.value(QStringLiteral("AutoHide"), m_bAutoHide).toBool();
    if (IniRead.value(QStringLiteral("HideShiftA"), true).toBool())
    {
        m_pShortcutHide = new QxtGlobalShortcut(QKeySequence("Shift+A"));
        setShiftA();
    }

    if (IniRead.value(QStringLiteral("ShowShiftZ"), true).toBool()) {
        m_pShortcutShow = new QxtGlobalShortcut(QKeySequence("Shift+Z"));
        setShiftZ();
    }
    IniRead.endGroup();
}

bool Translater::getShiftAState()
{
    return GlobalFn::readOneSet<bool>(QStringLiteral("Translate"), QStringLiteral("HideShiftA"), true).toBool();
}

bool Translater::getShiftZState()
{
    return GlobalFn::readOneSet<bool>(QStringLiteral("Translate"), QStringLiteral("ShowShiftZ"), true).toBool();
}

void Translater::showEvent(QShowEvent* event)
{
    int x, y;
    ui->pBtnPin->setChecked(!m_bAutoHide);
    ui->pBtnPin->setIcon(QIcon(m_bAutoHide? ":/icons/drip_pin.ico": ":/icons/drip_blue_pin.ico"));
    QRect rt = VarBox->m_pForm->geometry();
    if (rt.top() > height())
        y = rt.top() - height();
    else
        y = rt.bottom();
    if (rt.left() + rt.right() + width() > VarBox->m_dScreenWidth * 2)
        x = VarBox->m_dScreenWidth - width();
    else if (rt.right() + rt.left() < width())
        x = 0;
    else
        x = (rt.left() + rt.right() - width()) / 2;
    move(x, y);

    QTextCursor cursor = ui->TextFrom->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->TextFrom->setTextCursor(cursor);
    ui->TextFrom->setFocus();
    if (m_bAutoHide)
        m_pTimer->start(1000);
    event->accept();
}

#ifdef Q_OS_WIN
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
#endif

void Translater::mouseReleaseEvent(QMouseEvent *event)
{
    m_dTimeLeft = 10;
    event->accept();
}

void Translater::mousePressEvent(QMouseEvent *event)
{
    m_dTimeLeft = 10;
    event->accept();
}

void Translater::hideEvent(QHideEvent *event)
{
    m_pTimer->stop();
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
        m_dTimeLeft = 10;
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
                getReply(text);
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
        m_dTimeLeft = 10;
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

void Translater::getReply(const QString& q)
{
    std::string url(m_dYouDaoApi);

#if defined (Q_OS_WIN32)
    unsigned long long time_now = GetTickCount();  // haomiao
#elif defined (Q_OS_LINUX)
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long long time_now = tv.tv_sec * 1000000 + tv.tv_usec;  //weimiao
#endif
    if (time_now - m_dLastPostTime < 1000)
        QThread::msleep(m_dLastPostTime + 1000 - time_now);
    url += YEncode::urlEncode<std::string, std::string>(q.toStdString());
    QNetworkReply *m_pReply = m_pMgr->get(QNetworkRequest(QUrl(QString::fromStdString(url))));
    connect(m_pReply, &QNetworkReply::finished, this, [=]() {
        const QByteArray& m_qReplyData = m_pReply->readAll();
        m_pReply->deleteLater();
        try {
            YJson m_vJsonData(m_qReplyData);
            YJson* m_pHaveItem, *m_pJsTemp;
            if ((m_pHaveItem = m_vJsonData.find("translateResult")) &&
                (m_pHaveItem = m_pHaveItem->getChild()))
            {
                ui->TextTo->clear();
                do {
                    m_pJsTemp = m_pHaveItem->getChild()->find("tgt");
                    ui->TextTo->appendPlainText(m_pJsTemp->getValueString());
                } while ((m_pHaveItem = m_pHaveItem->getNext()));
                QTextCursor cursor = ui->TextFrom->textCursor();
                cursor.movePosition(QTextCursor::End);
                ui->TextFrom->setTextCursor(cursor);

                auto time_now = std::chrono::system_clock::now();
                m_dLastPostTime = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch()).count();
            } else {
                emit msgBox("null");
            }
        } catch (std::string& errorStr) {
            emit msgBox("Json文件出错");
        }
    });
    connect(m_pReply, &QNetworkReply::errorOccurred, this, [=](QNetworkReply::NetworkError){
        emit msgBox("发送get请求出错, 可能是没有网络！");
        m_pReply->deleteLater();
    });
    connect(m_pReply, &QNetworkReply::sslErrors, this, [=](const QList<QSslError> &){
        emit msgBox("sslErrors");
        m_pReply->deleteLater();
    });
}

void Translater::setShiftA()
{

    connect(m_pShortcutHide, &QxtGlobalShortcut::activated, this, [=](){
        qout << "按下隐藏热键";
        if (isVisible())
        {
#ifdef Q_OS_WIN
            if (hCurrentCursor && IsWindow(hCurrentCursor) && IsWindowVisible(hCurrentCursor))
                SetForegroundWindow(hCurrentCursor);
#endif
            hide();
        }
    });
}

void Translater::setShiftZ()
{
    connect(m_pShortcutShow, &QxtGlobalShortcut::activated, this, [=](){
        qout << "按下显示热键";
#ifdef Q_OS_WIN
        hCurrentCursor = GetForegroundWindow();
#endif
        if (!isVisible())
            show();
        activateWindow();        //SwitchToThisWindow(HWND(winId()), TRUE);
        QClipboard* cl = QApplication::clipboard();              //读取剪切板
        const QString& content = cl->text();
        if (content.isEmpty())
            return ;
        ui->TextFrom->setPlainText(content);
        ushort uNum = content.at(0).unicode();
        if(uNum >= 0x4E00 && uNum <= 0x9FA5)
        {
            if (!strncmp(m_dYouDaoApi+m_dLangPos, m_aLangType[m_dLangTo], 2))
            {
                ui->pBtnZhToEn->click();
                return ;
            }
        } else {
            if (!strncmp(m_dYouDaoApi+m_dLangPos+6, m_aLangType[m_dLangTo], 2))
            {
                ui->pBtnEnToZh->click();
                return ;
            }
        }
        getReply(content);
    });
}

void Translater::setFix(bool checked)
{
    ui->pBtnPin->setIcon(QIcon(checked?QStringLiteral(":/icons/drip_blue_pin.ico"):QStringLiteral(":/icons/drip_pin.ico")));
    m_bAutoHide = !checked;
    GlobalFn::saveOneSet<bool>(QStringLiteral("Translate"), QStringLiteral("AutoHide"), m_bAutoHide);
    if (m_bAutoHide)
        m_pTimer->start(1000);
}

void Translater::startEnToZh(bool checked)
{
    m_dTimeLeft = 10;
    if (checked)
    {
        ui->pBtnEnToZh->setIcon(QIcon(":/icons/black_zh.ico"));
        ui->pBtnZhToEn->setIcon(QIcon(":/icons/empty_en.ico"));
        strncpy(m_dYouDaoApi+m_dLangPos, m_aLangType[m_dLangFrom], 2);
        m_dYouDaoApi[m_dLangPos+2] = '2';
        strncpy(m_dYouDaoApi+m_dLangPos+3, m_sNativeLang, 5);
    } else {
        ui->pBtnEnToZh->setIcon(QIcon(":/icons/empty_zh.ico"));
        ui->pBtnZhToEn->setIcon(QIcon(":/icons/black_en.ico"));
        strncpy(m_dYouDaoApi+m_dLangPos, m_sNativeLang, 5);
        m_dYouDaoApi[m_dLangPos+5] = '2';
        strncpy(m_dYouDaoApi+m_dLangPos+6, m_aLangType[m_dLangTo], 2);
    }
    QString str = ui->TextFrom->toPlainText();
    if (!str.isEmpty()) getReply(str);
}

void Translater::copyTranlate()
{
    m_dTimeLeft = 10;
	QApplication::clipboard()->setText(ui->TextTo->toPlainText());
    emit msgBox("复制成功！");
}
