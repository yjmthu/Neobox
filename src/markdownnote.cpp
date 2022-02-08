#include "markdownnote.h"

#include <fstream>

#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QHBoxLayout>
#include <QDebug>

class DoorButton: public QWidget
{
protected:
    void mousePressEvent(QMouseEvent *event) {
        if (event->button() == Qt::LeftButton) {
            setMouseTracking(true);
            m_ptPress = event->pos();
        }
        event->accept();
    }
    void mouseReleaseEvent(QMouseEvent *event) {
        if (event->button() == Qt::LeftButton) {
            setMouseTracking(false);
            if (!m_moved) m_book->setVisible(!m_book->isVisible());
            else m_moved = false;
        }
        m_book->writePosition();
        event->accept();
    }
    void mouseMoveEvent(QMouseEvent *event) {
        m_moved = true;
        move(event->globalPos() - m_ptPress);
        event->accept();
    }
    void enterEvent(QEvent *event) {
        m_labImage->setStyleSheet(
            QStringLiteral("QLabel{background-color:rgba(100,100,100,100);border-image:url(:/icons/checklist.ico);}")
        );
        event->accept();
    }
    void leaveEvent(QEvent *event) {
        m_labImage->setStyleSheet(
            QStringLiteral("QLabel{background-color:transparent;border-image:url(:/icons/checklist.ico);}")
        );
        event->accept();
    }
public:
    DoorButton(MarkdownNote* book): QWidget(nullptr), m_book(book) {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint | Qt::Tool);
        setAttribute(Qt::WA_TranslucentBackground);
        setMinimumSize(48, 48);
        setMaximumSize(48, 48);
        m_labImage = new QLabel(this);
        m_labImage->setGeometry(0, 0, width(), height());
        m_labImage->setStyleSheet(
            QStringLiteral("QLabel{background-color:rgba(100,100,100,100);border-image:url(:/icons/checklist.ico);}")
        );
    }
    virtual ~DoorButton() {
        //
    }
private:
    MarkdownNote *m_book;
    QPoint m_ptPress;
    QLabel *m_labImage;
    bool m_moved {false};
};

void MarkdownNote::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setMouseTracking(true);
        m_ptPress = event->pos();
    }
    event->accept();
}

void MarkdownNote::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setMouseTracking(false);
    }
    writePosition();
    event->accept();
}

void MarkdownNote::mouseMoveEvent(QMouseEvent *event)
{
    move(event->globalPos() - m_ptPress);
    event->accept();
}

MarkdownNote::MarkdownNote(QWidget* parent):
    QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    QFrame *frame = new QFrame(this);
    QHBoxLayout *hlayout = new QHBoxLayout(this);
    setLayout(hlayout);
    hlayout->addWidget(frame);
    frame->setStyleSheet(QStringLiteral("QFrame{border-radius:6px;background-color:rgba(100,100,100,100);}"));
    door = new DoorButton(this);
    setMinimumSize(300, 300);
    setMaximumSize(300, 300);
    readPosition();
    door->show();
}

MarkdownNote::~MarkdownNote()
{
    delete door;
}

void MarkdownNote::readPosition()
{
    QPoint pts[2];
    std::ifstream file(".markdown-note", std::ios::in | std::ios::binary);
    if (file.is_open())
    {
        file.read(reinterpret_cast<char *>(pts), sizeof (QPoint) * 2);
        door->move(pts[0]);
        this->move(pts[1]);
        file.close();
    } else {
        door->move(100, 100);
    }
}

void MarkdownNote::writePosition()
{
    QPoint pts[2] { door->pos(), this->pos() };
    std::ofstream file(".markdown-note", std::ios::out | std::ios::binary);
    if (file.is_open())
    {
        file.write(reinterpret_cast<const char *>(pts), sizeof (QPoint) * 2);
        file.close();
    } else {
        // 失败
    }
}
