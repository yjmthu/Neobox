#ifndef MARKDOWNNOTE_H
#define MARKDOWNNOTE_H

#include <QWidget>

class MarkdownNote: public QWidget
{
    Q_OBJECT

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
public:
    explicit MarkdownNote(QWidget* parent=nullptr);
    ~MarkdownNote();
    void writePosition();
private:
    QWidget *door {nullptr};
    QPoint m_ptPress;
    void readPosition();
};

#endif // MARKDOWNNOTE_H
