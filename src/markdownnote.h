#ifndef MARKDOWNNOTE_H
#define MARKDOWNNOTE_H

#include <QPlainTextEdit>
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
    QWidget *door { nullptr };
    QPlainTextEdit* m_text { nullptr };
    bool m_textChanged { false };
    QPoint m_ptPress;
    void readPosition();
    void readNoteText();
private slots:
    void writeNoteText();
};

#endif // MARKDOWNNOTE_H
