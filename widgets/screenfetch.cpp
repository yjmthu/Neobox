#include "screenfetch.h"

#include <iostream>

#include <QMouseEvent>
#include <QPainter>
#include <QGuiApplication>
#include <QScreen>

ScreenFetch::ScreenFetch()
    : QDialog(nullptr)
    , m_IsGetPicture(false)
    , m_IsTrackingMouse(false)
    , m_TextRect( 0, 0, 0, 0 )
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowState(windowState() ^ Qt::WindowFullScreen);
}

ScreenFetch::~ScreenFetch()
{
}

void ScreenFetch::paintEvent(QPaintEvent* event)
{
    QPainter painter;
    painter.begin(this);
    // painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.fillRect(rect(), QColor(0, 0, 0, 150));
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
    painter.eraseRect(m_TextRect);
    painter.end();
    event->accept();
}

void ScreenFetch::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_K:
    case Qt::Key_Up:
        m_TextRect.setTop(m_TextRect.top() - 1);
        break;
    case Qt::Key_J:
    case Qt::Key_Down:
        m_TextRect.setTop(m_TextRect.top() + 1);
        break;
    case Qt::Key_H:
    case Qt::Key_Left:
        m_TextRect.setLeft(m_TextRect.left() - 1);
        break;
    case Qt::Key_L:
    case Qt::Key_Right:
        m_TextRect.setLeft(m_TextRect.left() + 1);
        break;
    case Qt::Key_Escape:
        if (m_IsTrackingMouse) {
            setMouseTracking(false);
        }
        close();
        break;
    default:
        break;
    }
    update();
    event->accept();
}

void ScreenFetch::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_IsTrackingMouse) {
            m_IsTrackingMouse = false;
            auto screen = QGuiApplication::primaryScreen();
            if (m_TextRect.width() && m_TextRect.height()) {
                screen->grabWindow(0, m_TextRect.left(), m_TextRect.top(), m_TextRect.width(), m_TextRect.height()).save("screenfetch.jpg", "jpg");
            } else {
                screen->grabWindow(0).save("screenfetch.jpg", "jpg");
            }
            m_IsGetPicture = true;
            close();
        } else {
            m_IsTrackingMouse = true;
            m_TextRect.setTopLeft(event->pos());
            m_TextRect.setBottomRight(event->pos());
        }
        setMouseTracking(m_IsTrackingMouse);
    }
    event->accept();
}

void ScreenFetch::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::MiddleButton) {
        m_TextRect.moveTo(event->pos() - QPoint(m_TextRect.width(), m_TextRect.height()));
    } else {
        m_TextRect.setBottomRight(event->pos());
    }
    update();
    event->accept();
}

