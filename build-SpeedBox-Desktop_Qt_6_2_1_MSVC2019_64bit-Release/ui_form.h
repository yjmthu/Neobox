/********************************************************************************
** Form generated from reading UI file 'form.ui'
**
** Created by: Qt User Interface Compiler version 6.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FORM_H
#define UI_FORM_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Form
{
public:
    QVBoxLayout *verticalLayout_2;
    QFrame *frame;
    QHBoxLayout *horizontalLayout;
    QLabel *LabMemory;
    QVBoxLayout *verticalLayout;
    QLabel *Labup;
    QLabel *Labdown;

    void setupUi(QWidget *Form)
    {
        if (Form->objectName().isEmpty())
            Form->setObjectName(QString::fromUtf8("Form"));
        Form->resize(92, 40);
        Form->setMinimumSize(QSize(0, 0));
        Form->setMaximumSize(QSize(92, 40));
        Form->setCursor(QCursor(Qt::PointingHandCursor));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/icons/exe.ico"), QSize(), QIcon::Normal, QIcon::Off);
        Form->setWindowIcon(icon);
        verticalLayout_2 = new QVBoxLayout(Form);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        frame = new QFrame(Form);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setMinimumSize(QSize(72, 40));
        frame->setMaximumSize(QSize(92, 40));
        frame->setStyleSheet(QString::fromUtf8("QFrame\n"
"{\n"
"	border-radius: 3px;\n"
"	background-color: rgba(8, 8, 8, 190);\n"
"}"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        horizontalLayout = new QHBoxLayout(frame);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        LabMemory = new QLabel(frame);
        LabMemory->setObjectName(QString::fromUtf8("LabMemory"));
        LabMemory->setMaximumSize(QSize(45, 30));
        QFont font;
        font.setFamilies({QString::fromUtf8("\351\273\221\344\275\223")});
        font.setPointSize(18);
        LabMemory->setFont(font);
        LabMemory->setStyleSheet(QString::fromUtf8("QLabel{\n"
"	background-color: rgba(0, 0, 0, 0);\n"
"	color: rgb(0, 255, 255);\n"
"	margin-bottom: 4px;\n"
"}"));
        LabMemory->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(LabMemory);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        Labup = new QLabel(frame);
        Labup->setObjectName(QString::fromUtf8("Labup"));
        Labup->setMinimumSize(QSize(0, 0));
        Labup->setMaximumSize(QSize(16777215, 16777215));
        Labup->setCursor(QCursor(Qt::PointingHandCursor));
        Labup->setLayoutDirection(Qt::LeftToRight);
        Labup->setStyleSheet(QString::fromUtf8("QLabel\n"
"{\n"
"background-color: rgba(0, 0, 0, 0);\n"
"color: rgb(250,170,35);\n"
"border-top-right-radius: 5px;\n"
"}"));

        verticalLayout->addWidget(Labup);

        Labdown = new QLabel(frame);
        Labdown->setObjectName(QString::fromUtf8("Labdown"));
        Labdown->setMinimumSize(QSize(0, 0));
        Labdown->setMaximumSize(QSize(16777215, 16777215));
        Labdown->setCursor(QCursor(Qt::PointingHandCursor));
        Labdown->setStyleSheet(QString::fromUtf8("QLabel\n"
"{\n"
"	background-color: rgba(0, 0, 0, 0);\n"
"	color: rgb(140, 240, 30);\n"
"}"));

        verticalLayout->addWidget(Labdown);


        horizontalLayout->addLayout(verticalLayout);


        verticalLayout_2->addWidget(frame);


        retranslateUi(Form);

        QMetaObject::connectSlotsByName(Form);
    } // setupUi

    void retranslateUi(QWidget *Form)
    {
        Form->setWindowTitle(QCoreApplication::translate("Form", "Speed Box", nullptr));
#if QT_CONFIG(tooltip)
        Form->setToolTip(QCoreApplication::translate("Form", "\350\241\214\345\213\225\346\230\257\346\262\273\347\231\222\346\201\220\346\207\274\347\232\204\350\211\257\350\227\245\357\274\214\350\200\214\347\214\266\350\261\253\343\200\201\346\213\226\345\273\266\345\260\207\344\270\215\346\226\267\346\273\213\351\244\212\346\201\220\346\207\274", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(tooltip)
        frame->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
        LabMemory->setText(QCoreApplication::translate("Form", "0", nullptr));
        Labup->setText(QCoreApplication::translate("Form", "\342\206\221  0.0 B", nullptr));
        Labdown->setText(QCoreApplication::translate("Form", "\342\206\223  0.0 B", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Form: public Ui_Form {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FORM_H
