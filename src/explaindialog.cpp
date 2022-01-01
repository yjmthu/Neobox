#include "explaindialog.h"
#include "ui_explaindialog.h"

ExplainDialog::ExplainDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExplainDialog)
{
    ui->setupUi(this);
}

ExplainDialog::~ExplainDialog()
{
    delete ui;
}
