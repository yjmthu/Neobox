#include "aboutnew.h"

#include <QTextEdit>
#include <QVBoxLayout>

AboutNew::AboutNew(QWidget *parent)
    : QWidget{parent}
{
    QVBoxLayout* lay {new QVBoxLayout(this)};
    QTextEdit* textEdit {new QTextEdit(this)};
    lay->addWidget(textEdit);
    textEdit->setText("检查更新中...");
}
