#include "markdownnote.h"

#include <fstream>

#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QHBoxLayout>
#include <QDebug>
#include <QTimer>

#include "funcbox.h"
#include "windowposition.h"

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
            if (!m_moved)
                m_book->setVisible(!m_book->isVisible());
            else
                m_moved = false;
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
            QStringLiteral("QLabel{"
               "background-color:rgba(100,100,100,100);"
               "border-image:url(:/icons/checklist.ico);"
               "border-radius:3px;"
            "}")
        );
        event->accept();
    }
    void leaveEvent(QEvent *event) {
        m_labImage->setStyleSheet(
            QStringLiteral("QLabel{"
               "background-color:transparent;"
               "border-image:url(:/icons/checklist.ico);"
               "border-radius:3px;"
            "}")
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
            QStringLiteral("QLabel{"
               "background-color:transparent;"
               "border-image:url(:/icons/checklist.ico);"
               "border-radius:3px;"
            "}")
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
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    QFrame *frame = new QFrame(this);
    QHBoxLayout *mlayout = new QHBoxLayout(this);
    setLayout(mlayout);
    mlayout->addWidget(frame);
    QHBoxLayout *hlayout = new QHBoxLayout(frame);
    m_text = new QPlainTextEdit(frame);
    hlayout->addWidget(m_text);
    frame->setLayout(hlayout);
    frame->setStyleSheet(QStringLiteral("QFrame{border-radius:6px;background-color:rgba(100,100,100,150);}"));
    m_text->setStyleSheet(QStringLiteral("QFrame{border-radius:6px;background-color:rgba(50,50,50,100);}"));
    door = new DoorButton(this);
    setMinimumSize(300, 300);
    setMaximumSize(300, 300);
    readPosition();
    door->show();
    readNoteText();
    QTimer *timer = new QTimer(this);
    timer->setInterval(3000);
    connect(timer, &QTimer::timeout, this, &MarkdownNote::writeNoteText);
    connect(m_text, &QPlainTextEdit::textChanged, this, [this](){ m_textChanged = true; });
    timer->start();
}

MarkdownNote::~MarkdownNote()
{
    writeNoteText();
    delete door;
}

void MarkdownNote::readPosition()
{
    if (VarBox->m_windowPosition->m_noteBookPos != QPoint(0, 0)) {
        this->move(VarBox->m_windowPosition->m_noteBookPos);
    }
    if (VarBox->m_windowPosition->m_noteDoorPos != QPoint(0, 0)) {
        door->move(VarBox->m_windowPosition->m_noteDoorPos);
    }
}

void MarkdownNote::readNoteText()
{
    QFile file(QStringLiteral("MarkNote.md"));
    if (file.open(QIODevice::ReadOnly))
    {
        m_text->setPlainText(file.readAll());
        file.close();
    }
}

void MarkdownNote::writeNoteText()
{
    if (!m_textChanged) return;
    QFile file(QStringLiteral("MarkNote.md"));
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(m_text->toPlainText().toUtf8());
        file.close();
    }
    m_textChanged = false;
}

void MarkdownNote::writePosition()
{
    VarBox->m_windowPosition->m_noteBookPos = this->pos();
    VarBox->m_windowPosition->m_noteDoorPos = door->pos();
    VarBox->m_windowPosition->toFile();
}
