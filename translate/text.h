#ifndef TEXT_H
#define TEXT_H

#include <QDialog>

class TextDlg: public QDialog
{
public:
    explicit TextDlg();
private:
    class QPlainTextEdit* m_TextEdit;
    void ParsePicture();
};

#endif // TEXT_H
