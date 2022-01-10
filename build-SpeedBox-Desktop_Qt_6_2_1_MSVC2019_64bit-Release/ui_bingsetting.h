/********************************************************************************
** Form generated from reading UI file 'bingsetting.ui'
**
** Created by: Qt User Interface Compiler version 6.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BINGSETTING_H
#define UI_BINGSETTING_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_BingSetting
{
public:
    QVBoxLayout *verticalLayout_2;
    QFrame *frame;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label;
    QLabel *label_2;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *verticalSpacer_3;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout;
    QRadioButton *radioButton;
    QRadioButton *radioButton_2;
    QSpacerItem *verticalSpacer_2;
    QHBoxLayout *horizontalLayout_5;
    QCheckBox *checkBox;
    QCheckBox *checkBox_2;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *pushButton;
    QSpacerItem *horizontalSpacer_5;
    QPushButton *pushButton_2;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *pushButton_3;
    QSpacerItem *horizontalSpacer_4;

    void setupUi(QWidget *BingSetting)
    {
        if (BingSetting->objectName().isEmpty())
            BingSetting->setObjectName(QString::fromUtf8("BingSetting"));
        BingSetting->resize(308, 183);
        verticalLayout_2 = new QVBoxLayout(BingSetting);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(5, 5, 5, 5);
        frame = new QFrame(BingSetting);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout = new QVBoxLayout(frame);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(9, 5, -1, -1);
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label = new QLabel(frame);
        label->setObjectName(QString::fromUtf8("label"));
        label->setMinimumSize(QSize(20, 20));
        label->setMaximumSize(QSize(20, 20));
        label->setStyleSheet(QString::fromUtf8("QLabel{\n"
"border-image: url(:/icons/ya.ico);\n"
"}"));

        horizontalLayout_3->addWidget(label);

        label_2 = new QLabel(frame);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_3->addWidget(label_2);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout_3);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_3);

        groupBox = new QGroupBox(frame);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setMinimumSize(QSize(0, 55));
        horizontalLayout = new QHBoxLayout(groupBox);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(-1, 12, -1, 5);
        radioButton = new QRadioButton(groupBox);
        radioButton->setObjectName(QString::fromUtf8("radioButton"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(radioButton->sizePolicy().hasHeightForWidth());
        radioButton->setSizePolicy(sizePolicy);
        radioButton->setChecked(true);

        horizontalLayout->addWidget(radioButton);

        radioButton_2 = new QRadioButton(groupBox);
        radioButton_2->setObjectName(QString::fromUtf8("radioButton_2"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(radioButton_2->sizePolicy().hasHeightForWidth());
        radioButton_2->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(radioButton_2);


        verticalLayout->addWidget(groupBox);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer_2);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        checkBox = new QCheckBox(frame);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));
        checkBox->setChecked(true);

        horizontalLayout_5->addWidget(checkBox);

        checkBox_2 = new QCheckBox(frame);
        checkBox_2->setObjectName(QString::fromUtf8("checkBox_2"));
        checkBox_2->setChecked(true);

        horizontalLayout_5->addWidget(checkBox_2);


        verticalLayout->addLayout(horizontalLayout_5);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);

        pushButton = new QPushButton(frame);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout_2->addWidget(pushButton);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_5);

        pushButton_2 = new QPushButton(frame);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));

        horizontalLayout_2->addWidget(pushButton_2);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_3);

        pushButton_3 = new QPushButton(frame);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));

        horizontalLayout_2->addWidget(pushButton_3);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_4);


        verticalLayout->addLayout(horizontalLayout_2);


        verticalLayout_2->addWidget(frame);


        retranslateUi(BingSetting);
        QObject::connect(pushButton, &QPushButton::clicked, BingSetting, qOverload<>(&QWidget::close));

        QMetaObject::connectSlotsByName(BingSetting);
    } // setupUi

    void retranslateUi(QWidget *BingSetting)
    {
        BingSetting->setWindowTitle(QCoreApplication::translate("BingSetting", "\345\277\205\345\272\224\345\243\201\347\272\270\350\256\276\347\275\256", nullptr));
        label->setText(QString());
        label_2->setText(QCoreApplication::translate("BingSetting", "\345\277\205\345\272\224\345\243\201\347\272\270", nullptr));
        groupBox->setTitle(QCoreApplication::translate("BingSetting", "\345\277\205\345\272\224\345\243\201\347\272\270\345\220\215\347\247\260", nullptr));
#if QT_CONFIG(tooltip)
        radioButton->setToolTip(QCoreApplication::translate("BingSetting", "\344\276\213\345\246\202\357\274\2322021-04-12\345\277\205\345\272\224\345\243\201\347\272\270.jpg", nullptr));
#endif // QT_CONFIG(tooltip)
        radioButton->setText(QCoreApplication::translate("BingSetting", "\344\273\245\346\227\245\346\234\237\345\221\275\345\220\215", nullptr));
#if QT_CONFIG(tooltip)
        radioButton_2->setToolTip(QCoreApplication::translate("BingSetting", "\344\276\213\345\246\202\357\274\232\347\272\246\345\205\213\351\203\241\350\260\267\345\234\260\345\233\275\345\256\266\345\205\254\345\233\255\351\207\214\347\232\204\345\261\261\344\270\230\357\274\214\350\213\261\346\240\274\345\205\260.jpg", nullptr));
#endif // QT_CONFIG(tooltip)
        radioButton_2->setText(QCoreApplication::translate("BingSetting", "\344\273\245\345\233\276\347\211\207\344\277\241\346\201\257\345\221\275\345\220\215", nullptr));
#if QT_CONFIG(tooltip)
        checkBox->setToolTip(QCoreApplication::translate("BingSetting", "\350\275\257\344\273\266\345\220\257\345\212\250\346\227\266\350\207\252\345\212\250\344\270\213\350\275\275\344\277\235\345\255\230\345\275\223\345\244\251\345\243\201\347\272\270", nullptr));
#endif // QT_CONFIG(tooltip)
        checkBox->setText(QCoreApplication::translate("BingSetting", "\345\220\257\345\212\250\346\227\266\344\270\213\350\275\275", nullptr));
#if QT_CONFIG(tooltip)
        checkBox_2->setToolTip(QCoreApplication::translate("BingSetting", "\346\214\211\347\205\247\350\256\276\347\275\256\347\232\204\346\227\266\351\227\264\351\227\264\351\232\224\350\275\256\346\215\242\346\234\200\350\277\221\345\205\253\345\244\251\347\232\204\345\277\205\345\272\224\345\243\201\347\272\270", nullptr));
#endif // QT_CONFIG(tooltip)
        checkBox_2->setText(QCoreApplication::translate("BingSetting", "\350\207\252\345\212\250\350\275\256\346\215\242", nullptr));
        pushButton->setText(QCoreApplication::translate("BingSetting", "\345\217\226\346\266\210", nullptr));
#if QT_CONFIG(tooltip)
        pushButton_2->setToolTip(QCoreApplication::translate("BingSetting", "*\344\277\235\345\255\230", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton_2->setText(QCoreApplication::translate("BingSetting", "\344\277\235\345\255\230", nullptr));
#if QT_CONFIG(tooltip)
        pushButton_3->setToolTip(QCoreApplication::translate("BingSetting", "\345\272\224\347\224\250\344\275\206\344\270\215\344\277\235\345\255\230", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton_3->setText(QCoreApplication::translate("BingSetting", "\345\272\224\347\224\250", nullptr));
    } // retranslateUi

};

namespace Ui {
    class BingSetting: public Ui_BingSetting {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BINGSETTING_H
