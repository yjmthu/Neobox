#ifndef MARKDOWNNOTE_H
#define MARKDOWNNOTE_H

#include <QWidget>

class MarkdownNote: public QWidget
{
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
public:
    explicit MarkdownNote(QWidget* parent=nullptr);
    ~MarkdownNote();
private:
    class DoorButton *door {nullptr};
    QPoint m_ptPress;
};

#endif // MARKDOWNNOTE_H
