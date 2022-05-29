#ifndef TEXT_H
#define TEXT_H

#include <QDialog>

class TextDlg: public QDialog
{
public:
    explicit TextDlg(void* image);
private:
    class QPlainTextEdit* m_TextEdit;
    void ParsePicture(void* image);
};

#endif // TEXT_H
