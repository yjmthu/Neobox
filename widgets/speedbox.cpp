#include "speedbox.h"
#include "speedapp.h"
#include "speedmenu.h"

#include "core/netspeedhelper.h"
#include "core/appcode.hpp"

#include "wallpaper/wallpaper.h"

#include <yjson.h>

#include <QMouseEvent>
#include <QApplication>
#include <QPainter>
#include <QMimeData>
#include <QMessageBox>
#include <QScreen>
#include <QPropertyAnimation>
#include <QGraphicsBlurEffect>
#include <QTimer>

void SpeedBox::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_LastPos = event->pos();
        setMouseTracking(true);
    } else if (event->button() == Qt::RightButton) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        m_Menu->popup(event->globalPos());
#else
        m_Menu->popup(event->globalPosition().toPoint());
#endif
    } else if (event->button() == Qt::MiddleButton) {
        qApp->exit(static_cast<int>(ExitCode::RETCODE_RESTART));
    } else {
        // TO DO
    }
}

void SpeedBox::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setMouseTracking(false);
        WritePosition();
    }
}

void SpeedBox::mouseMoveEvent(QMouseEvent *event)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    move(event->globalPos() - m_LastPos);
#else
    move(event->globalPosition().toPoint() - m_LastPos);
#endif
}

void SpeedBox::paintEvent(QPaintEvent *)
{
    QPainter painter;
    painter.begin(this);
    painter.setBrush(QBrush(m_BackCol));
    painter.setPen(Qt::transparent);
    painter.drawRoundedRect(QRect(0, 0, width(), height()), 3, 3); // round rect
    painter.setFont(std::get<1>(m_Style[0]));
    painter.setPen(std::get<0>(m_Style[0]));
    painter.drawText(std::get<2>(m_Style[0]), QString::fromStdString(m_NetSpeedHelper->m_SysInfo[0]));
    painter.setPen(std::get<0>(m_Style[1]));
    painter.setFont(std::get<1>(m_Style[1]));
    painter.drawText(std::get<2>(m_Style[1]), QString::fromStdString(m_NetSpeedHelper->m_SysInfo[1]));
    painter.setPen(std::get<0>(m_Style[2]));
    painter.setFont(std::get<1>(m_Style[2]));
    painter.drawText(std::get<2>(m_Style[2]), QString::fromStdString(m_NetSpeedHelper->m_SysInfo[2]));
    painter.end();
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
void SpeedBox::enterEvent(QEvent* event)
#else
void SpeedBox::enterEvent(QEnterEvent* event)
#endif
{
    const QRect& rect = geometry();
    m_Animation->setStartValue(geometry());
    if (m_Width != width()) {
        m_Animation->setDuration(200);
        if (rect.left() == 0) {
            m_Animation->setEndValue(
            QRect(0, rect.top(), m_Width, m_Height));
        } else if (rect.right() + 1 >= m_ScreenWidth) {
            m_Animation->setEndValue(
            QRect(m_ScreenWidth-m_Width, rect.top(), m_Width, m_Height));
        } else {
            goto label;
        }
    } else  if (m_Height != height()) {
        m_Animation->setDuration(100);
        if (rect.top() == 0) {
            m_Animation->setEndValue(
            QRect(rect.left(), 0, m_Width, m_Height));
        } else if (rect.bottom() + 1 >= m_ScreenHeight) {
            m_Animation->setEndValue(
            QRect(rect.left(), m_ScreenHeight-m_Height, m_Width, m_Height));
        } else {
            goto label;
        }
    } else {
        goto label;
    }
    m_Animation->start();
label:
    if (event) event->accept();
}

void SpeedBox::leaveEvent(QEvent* event)
{
    AutoHide();
    if (event) event->accept();
}

void SpeedBox::AutoHide()
{
    constexpr int liuchu = 2;
    const QRect& rect = geometry();
    m_Animation->setStartValue(geometry());
    if (rect.left() == 0) {
        m_Animation->setDuration(200);
        m_Animation->setEndValue(
        QRect(0, rect.top(), liuchu, m_Height));
    } else if (rect.right() + 1 >= m_ScreenWidth) {
        m_Animation->setDuration(200);
        m_Animation->setEndValue(
        QRect(m_ScreenWidth-liuchu, rect.top(), liuchu, m_Height));
    } else if (rect.top() == 0) {
        m_Animation->setDuration(100);
        m_Animation->setEndValue(
        QRect(rect.left(), 0, m_Width, liuchu));
    } else if (rect.bottom() + 1 >= m_ScreenHeight) {
        m_Animation->setDuration(100);
        m_Animation->setEndValue(
        QRect(rect.left(), m_ScreenHeight-liuchu, m_Width, liuchu));
    } else {
        return;
    }
    // animation->setLoopCount(-1);
    m_Animation->start();
}

void SpeedBox::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void SpeedBox::dropEvent(QDropEvent *event)
{
    std::string temp = event->mimeData()->urls().first().toString().toStdString();
    temp.erase(0, 7);
    m_VarBox->m_Wallpaper->SetDropFile(temp);
}

SpeedBox::SpeedBox(int type, QWidget *parent)
    : QWidget(parent)
    , m_Animation(new QPropertyAnimation(this))
    , m_Width(100)
    , m_Height(42)
    , m_ScreenHeight(0)
    , m_ScreenWidth(0)
    , m_ExecType(type)
    , m_NetSpeedHelper(new NetSpeedHelper(this))
    , m_Menu(new SpeedMenu(this))
{
    QRect geo = QGuiApplication::primaryScreen()->geometry();
    *const_cast<int*>(&m_ScreenWidth) = geo.width();
    *const_cast<int*>(&m_ScreenHeight) = geo.height();
    m_Animation->setTargetObject(this);
    m_Animation->setPropertyName("geometry");
//     connect(m_Animation, &QPropertyAnimation::finished,
//     this, &SpeedBox::WritePosition);
//     QGraphicsBlurEffect *blureffect = new QGraphicsBlurEffect(this);
//     blureffect->setBlurRadius(5);	   //数值越大，越模糊
//     setGraphicsEffect(blureffect);      //设置模糊特效

    SetupUi();
}

SpeedBox::~SpeedBox()
{
}

void SpeedBox::SetupUi()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setMaximumSize(m_Width, m_Height);
    setToolTip(u8"行動是治癒恐懼的良藥，而猶豫、拖延將不斷滋養恐懼");
    setCursor(Qt::CursorShape::PointingHandCursor);
    setAcceptDrops(true);
    ReadPosition();
    GetBackGroundColor();
    connect(m_Menu, &SpeedMenu::ChangeBoxColor, this, &SpeedBox::SetBackGroundColor);
    connect(m_Menu, &SpeedMenu::ChangeBoxAlpha, this, &SpeedBox::SetBackGroundAlpha);
    GetStyle();
}

void SpeedBox::GetStyle()
{
    const char* li[] = {"MemUseage", "NetUpSpeed", "NetDownSpeed"};
    auto ui = *m_VarBox->m_Setting->find("FormUi");
    for (size_t i=0; i<3; ++i) {
        auto& ptr = ui[li[i]];
        std::get<0>(m_Style[i]).setNamedColor(ptr["color"].getValueString());
        std::get<1>(m_Style[i]).setFamily(ptr["font-family"].getValueString());
        std::get<1>(m_Style[i]).setPointSize(ptr["font-size"].getValueInt());
        std::get<1>(m_Style[i]).setItalic(ptr["italic"].isTrue());
        std::get<1>(m_Style[i]).setBold(ptr["bold"].isTrue());
        std::get<2>(m_Style[i]).setX(ptr["pos"][0].getValueInt());
        std::get<2>(m_Style[i]).setY(ptr["pos"][1].getValueInt());
    }
}

void SpeedBox::ReadPosition()
{
    int pt[2] { 0, 0 };
    FILE* fp = fopen("boxpos", "rb");
    if (!fp) {
        setGeometry(30, 30, m_Width, m_Height);
        return;
    }
    fread(pt, sizeof(int), 2, fp);
    fclose(fp);
    setGeometry(pt[0], pt[1], m_Width, m_Height);
    AutoHide();
}

void SpeedBox::WritePosition()
{
    int pt[2] { x(), y() };
    FILE* fp = fopen("boxpos", "wb");
    if (!fp) return;
    fwrite(pt, sizeof(int), 2, fp);
    fclose(fp);
}

void SpeedBox::GetBackGroundColor()
{
    auto ptr = m_VarBox->m_Setting->find("FormUi")->find("BkColor")->getChild();
    m_BackCol.setRed(ptr->getValueInt());
    ptr = ptr->getNext();
    m_BackCol.setGreen(ptr->getValueInt());
    ptr = ptr->getNext();
    m_BackCol.setBlue(ptr->getValueInt());
    ptr = ptr->getNext();
    m_BackCol.setAlpha(ptr->getValueInt());
}

void SpeedBox::OnTimer()
{
    m_NetSpeedHelper->GetSysInfo();
    update();
}

void SpeedBox::SetBackGroundColor(QColor col)
{
    auto ptr = m_VarBox->m_Setting->find("FormUi")->find("BkColor")->getChild();
    ptr->setValue(col.red());
    m_BackCol.setRed(col.red());
    ptr = ptr->getNext();
    m_BackCol.setGreen(col.green());
    ptr = ptr->getNext();
    m_BackCol.setBlue(col.blue());
    m_VarBox->m_Setting->toFile(m_VarBox->m_szSettingFile, YJson::UTF8, true);
}

void SpeedBox::SetBackGroundAlpha(int alpha)
{
    m_BackCol.setAlpha(alpha);
    m_VarBox->m_Setting->find("FormUi")->find("BkColor")->find(-1)->setValue(alpha);
    m_VarBox->m_Setting->toFile(m_VarBox->m_szSettingFile, YJson::UTF8, true);
}
