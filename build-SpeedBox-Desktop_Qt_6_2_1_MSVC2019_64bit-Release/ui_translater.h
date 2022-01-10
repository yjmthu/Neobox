/********************************************************************************
** Form generated from reading UI file 'translater.ui'
**
** Created by: Qt User Interface Compiler version 6.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TRANSLATER_H
#define UI_TRANSLATER_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Translater
{
public:
    QVBoxLayout *verticalLayout;
    QFrame *frame;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *Icon;
    QLabel *Title;
    QSpacerItem *horizontalSpacer;
    QPlainTextEdit *TextFrom;
    QPlainTextEdit *TextTo;
    QHBoxLayout *horizontalLayout;
    QPushButton *pBtnCopyTranlate;
    QPushButton *bBtnClean;
    QPushButton *pBtnEnToZh;
    QPushButton *pBtnZhToEn;
    QPushButton *pBtnPin;

    void setupUi(QWidget *Translater)
    {
        if (Translater->objectName().isEmpty())
            Translater->setObjectName(QString::fromUtf8("Translater"));
        Translater->resize(240, 300);
        Translater->setMaximumSize(QSize(16777215, 16777215));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/ya.ico"), QSize(), QIcon::Normal, QIcon::Off);
        Translater->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(Translater);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        frame = new QFrame(Translater);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout_2 = new QVBoxLayout(frame);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        Icon = new QLabel(frame);
        Icon->setObjectName(QString::fromUtf8("Icon"));
        Icon->setMinimumSize(QSize(20, 20));
        Icon->setMaximumSize(QSize(20, 20));
        Icon->setStyleSheet(QString::fromUtf8("QLabel{\n"
"border-image: url(:/icons/ya.ico);\n"
"}"));

        horizontalLayout_2->addWidget(Icon);

        Title = new QLabel(frame);
        Title->setObjectName(QString::fromUtf8("Title"));
        Title->setStyleSheet(QString::fromUtf8("QLabel{\n"
"	color: rgb(0, 0, 0);\n"
"	border: None;\n"
"}"));

        horizontalLayout_2->addWidget(Title);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);


        verticalLayout_2->addLayout(horizontalLayout_2);

        TextFrom = new QPlainTextEdit(frame);
        TextFrom->setObjectName(QString::fromUtf8("TextFrom"));
        TextFrom->setStyleSheet(QString::fromUtf8("QPlainTextEdit {\n"
"    background-color: rgba(8, 8, 8, 150);\n"
"    color: #ffff00;\n"
"        border: 1px solid #ff00ff\n"
"}\n"
""));

        verticalLayout_2->addWidget(TextFrom);

        TextTo = new QPlainTextEdit(frame);
        TextTo->setObjectName(QString::fromUtf8("TextTo"));
        TextTo->setFocusPolicy(Qt::ClickFocus);
        TextTo->setStyleSheet(QString::fromUtf8("QPlainTextEdit {\n"
"    background-color: rgba(8, 8, 8, 150);\n"
"    color: #00ffff;\n"
"    border: 1px solid #ff00ff\n"
"}\n"
""));
        TextTo->setReadOnly(false);

        verticalLayout_2->addWidget(TextTo);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pBtnCopyTranlate = new QPushButton(frame);
        pBtnCopyTranlate->setObjectName(QString::fromUtf8("pBtnCopyTranlate"));
        pBtnCopyTranlate->setMinimumSize(QSize(30, 20));
        pBtnCopyTranlate->setMaximumSize(QSize(30, 20));
        pBtnCopyTranlate->setCursor(QCursor(Qt::PointingHandCursor));
        pBtnCopyTranlate->setFocusPolicy(Qt::ClickFocus);
        pBtnCopyTranlate->setStyleSheet(QString::fromUtf8(""));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/drip_copy.ico"), QSize(), QIcon::Normal, QIcon::Off);
        pBtnCopyTranlate->setIcon(icon1);
        pBtnCopyTranlate->setFlat(true);

        horizontalLayout->addWidget(pBtnCopyTranlate);

        bBtnClean = new QPushButton(frame);
        bBtnClean->setObjectName(QString::fromUtf8("bBtnClean"));
        bBtnClean->setMinimumSize(QSize(30, 20));
        bBtnClean->setMaximumSize(QSize(30, 20));
        bBtnClean->setStyleSheet(QString::fromUtf8(""));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/drip_trash.ico"), QSize(), QIcon::Normal, QIcon::Off);
        bBtnClean->setIcon(icon2);
        bBtnClean->setFlat(true);

        horizontalLayout->addWidget(bBtnClean);

        pBtnEnToZh = new QPushButton(frame);
        pBtnEnToZh->setObjectName(QString::fromUtf8("pBtnEnToZh"));
        pBtnEnToZh->setMinimumSize(QSize(30, 20));
        pBtnEnToZh->setMaximumSize(QSize(30, 20));
        pBtnEnToZh->setStyleSheet(QString::fromUtf8(""));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/empty_zh.ico"), QSize(), QIcon::Normal, QIcon::Off);
        pBtnEnToZh->setIcon(icon3);
        pBtnEnToZh->setCheckable(true);
        pBtnEnToZh->setFlat(true);

        horizontalLayout->addWidget(pBtnEnToZh);

        pBtnZhToEn = new QPushButton(frame);
        pBtnZhToEn->setObjectName(QString::fromUtf8("pBtnZhToEn"));
        pBtnZhToEn->setMinimumSize(QSize(30, 20));
        pBtnZhToEn->setMaximumSize(QSize(30, 20));
        pBtnZhToEn->setStyleSheet(QString::fromUtf8(""));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/black_en.ico"), QSize(), QIcon::Normal, QIcon::Off);
        pBtnZhToEn->setIcon(icon4);
        pBtnZhToEn->setCheckable(true);
        pBtnZhToEn->setChecked(true);
        pBtnZhToEn->setFlat(true);

        horizontalLayout->addWidget(pBtnZhToEn);

        pBtnPin = new QPushButton(frame);
        pBtnPin->setObjectName(QString::fromUtf8("pBtnPin"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(30);
        sizePolicy.setVerticalStretch(20);
        sizePolicy.setHeightForWidth(pBtnPin->sizePolicy().hasHeightForWidth());
        pBtnPin->setSizePolicy(sizePolicy);
        pBtnPin->setMinimumSize(QSize(30, 20));
        pBtnPin->setMaximumSize(QSize(30, 20));
        pBtnPin->setStyleSheet(QString::fromUtf8(""));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/drip_pin.ico"), QSize(), QIcon::Normal, QIcon::Off);
        pBtnPin->setIcon(icon5);
        pBtnPin->setCheckable(true);
        pBtnPin->setFlat(true);

        horizontalLayout->addWidget(pBtnPin);


        verticalLayout_2->addLayout(horizontalLayout);


        verticalLayout->addWidget(frame);


        retranslateUi(Translater);

        QMetaObject::connectSlotsByName(Translater);
    } // setupUi

    void retranslateUi(QWidget *Translater)
    {
        Translater->setWindowTitle(QCoreApplication::translate("Translater", " \346\236\201\347\256\200\347\277\273\350\257\221", nullptr));
        Icon->setText(QString());
        Title->setText(QCoreApplication::translate("Translater", "\346\236\201\347\256\200\347\277\273\350\257\221", nullptr));
#if QT_CONFIG(tooltip)
        TextFrom->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
        TextFrom->setPlainText(QString());
#if QT_CONFIG(tooltip)
        TextTo->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
        TextTo->setPlainText(QString());
#if QT_CONFIG(tooltip)
        pBtnCopyTranlate->setToolTip(QCoreApplication::translate("Translater", "\345\244\215\345\210\266\347\277\273\350\257\221\347\273\223\346\236\234", nullptr));
#endif // QT_CONFIG(tooltip)
        pBtnCopyTranlate->setText(QString());
#if QT_CONFIG(tooltip)
        bBtnClean->setToolTip(QCoreApplication::translate("Translater", "\346\270\205\347\251\272\346\226\207\346\234\254", nullptr));
#endif // QT_CONFIG(tooltip)
        bBtnClean->setText(QString());
#if QT_CONFIG(tooltip)
        pBtnEnToZh->setToolTip(QCoreApplication::translate("Translater", "\347\277\273\350\257\221\344\270\272\344\270\255\346\226\207", nullptr));
#endif // QT_CONFIG(tooltip)
        pBtnEnToZh->setText(QString());
#if QT_CONFIG(tooltip)
        pBtnZhToEn->setToolTip(QCoreApplication::translate("Translater", "\347\277\273\350\257\221\344\270\272\350\213\261\346\226\207", nullptr));
#endif // QT_CONFIG(tooltip)
        pBtnZhToEn->setText(QString());
#if QT_CONFIG(tooltip)
        pBtnPin->setToolTip(QCoreApplication::translate("Translater", "\351\230\262\346\255\242\351\232\220\350\227\217", nullptr));
#endif // QT_CONFIG(tooltip)
        pBtnPin->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class Translater: public Ui_Translater {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TRANSLATER_H
