/********************************************************************************
** Form generated from reading UI file 'formsetting.ui'
**
** Created by: Qt User Interface Compiler version 6.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FORMSETTING_H
#define UI_FORMSETTING_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FormSetting
{
public:
    QVBoxLayout *verticalLayout_2;
    QFrame *frame;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_6;
    QLabel *label_7;
    QSpacerItem *horizontalSpacer_7;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QPushButton *pushButton;
    QLabel *label_2;
    QSlider *horizontalSlider;
    QLabel *label_8;
    QPushButton *pushButton_2;
    QPushButton *pushButton_7;
    QGroupBox *groupBox_3;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_5;
    QRadioButton *radioButton;
    QRadioButton *radioButton_2;
    QRadioButton *radioButton_3;
    QPushButton *pushButton_3;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_2;
    QRadioButton *radioButton_4;
    QRadioButton *radioButton_5;
    QRadioButton *radioButton_6;
    QRadioButton *radioButton_7;
    QPushButton *pushButton_4;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer_4;
    QPushButton *pushButton_5;
    QSpacerItem *horizontalSpacer_5;
    QPushButton *pushButton_6;
    QSpacerItem *horizontalSpacer_6;

    void setupUi(QWidget *FormSetting)
    {
        if (FormSetting->objectName().isEmpty())
            FormSetting->setObjectName(QString::fromUtf8("FormSetting"));
        FormSetting->resize(438, 283);
        verticalLayout_2 = new QVBoxLayout(FormSetting);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(5, 5, 5, 5);
        frame = new QFrame(FormSetting);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout = new QVBoxLayout(frame);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(-1, 5, -1, -1);
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_6 = new QLabel(frame);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label_6->sizePolicy().hasHeightForWidth());
        label_6->setSizePolicy(sizePolicy);
        label_6->setMinimumSize(QSize(20, 20));
        label_6->setMaximumSize(QSize(20, 20));
        label_6->setStyleSheet(QString::fromUtf8("QLabel{\n"
"border-image: url(:/icons/ya.ico);\n"
"}"));

        horizontalLayout_5->addWidget(label_6);

        label_7 = new QLabel(frame);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        sizePolicy.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy);

        horizontalLayout_5->addWidget(label_7);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_7);


        verticalLayout->addLayout(horizontalLayout_5);

        groupBox = new QGroupBox(frame);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy1);
        groupBox->setMinimumSize(QSize(0, 65));
        horizontalLayout = new QHBoxLayout(groupBox);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(label);

        pushButton = new QPushButton(groupBox);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setMinimumSize(QSize(24, 24));
        pushButton->setMaximumSize(QSize(24, 24));
        pushButton->setStyleSheet(QString::fromUtf8("background-color: rgb(8, 8, 8);\n"
"border: 1px solid black;"));

        horizontalLayout->addWidget(pushButton);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(label_2);

        horizontalSlider = new QSlider(groupBox);
        horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
        horizontalSlider->setMaximum(255);
        horizontalSlider->setSingleStep(1);
        horizontalSlider->setValue(190);
        horizontalSlider->setOrientation(Qt::Horizontal);

        horizontalLayout->addWidget(horizontalSlider);

        label_8 = new QLabel(groupBox);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        horizontalLayout->addWidget(label_8);

        pushButton_2 = new QPushButton(groupBox);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));

        horizontalLayout->addWidget(pushButton_2);

        pushButton_7 = new QPushButton(groupBox);
        pushButton_7->setObjectName(QString::fromUtf8("pushButton_7"));

        horizontalLayout->addWidget(pushButton_7);


        verticalLayout->addWidget(groupBox);

        groupBox_3 = new QGroupBox(frame);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        sizePolicy1.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
        groupBox_3->setSizePolicy(sizePolicy1);
        groupBox_3->setMinimumSize(QSize(0, 65));
        horizontalLayout_3 = new QHBoxLayout(groupBox_3);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_5 = new QLabel(groupBox_3);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_3->addWidget(label_5);

        radioButton = new QRadioButton(groupBox_3);
        radioButton->setObjectName(QString::fromUtf8("radioButton"));

        horizontalLayout_3->addWidget(radioButton);

        radioButton_2 = new QRadioButton(groupBox_3);
        radioButton_2->setObjectName(QString::fromUtf8("radioButton_2"));
        radioButton_2->setChecked(true);

        horizontalLayout_3->addWidget(radioButton_2);

        radioButton_3 = new QRadioButton(groupBox_3);
        radioButton_3->setObjectName(QString::fromUtf8("radioButton_3"));
        radioButton_3->setChecked(false);

        horizontalLayout_3->addWidget(radioButton_3);

        pushButton_3 = new QPushButton(groupBox_3);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));

        horizontalLayout_3->addWidget(pushButton_3);


        verticalLayout->addWidget(groupBox_3);

        groupBox_2 = new QGroupBox(frame);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        sizePolicy1.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
        groupBox_2->setSizePolicy(sizePolicy1);
        horizontalLayout_2 = new QHBoxLayout(groupBox_2);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        radioButton_4 = new QRadioButton(groupBox_2);
        radioButton_4->setObjectName(QString::fromUtf8("radioButton_4"));
        radioButton_4->setChecked(true);

        horizontalLayout_2->addWidget(radioButton_4);

        radioButton_5 = new QRadioButton(groupBox_2);
        radioButton_5->setObjectName(QString::fromUtf8("radioButton_5"));

        horizontalLayout_2->addWidget(radioButton_5);

        radioButton_6 = new QRadioButton(groupBox_2);
        radioButton_6->setObjectName(QString::fromUtf8("radioButton_6"));

        horizontalLayout_2->addWidget(radioButton_6);

        radioButton_7 = new QRadioButton(groupBox_2);
        radioButton_7->setObjectName(QString::fromUtf8("radioButton_7"));

        horizontalLayout_2->addWidget(radioButton_7);

        pushButton_4 = new QPushButton(groupBox_2);
        pushButton_4->setObjectName(QString::fromUtf8("pushButton_4"));

        horizontalLayout_2->addWidget(pushButton_4);


        verticalLayout->addWidget(groupBox_2);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_4);

        pushButton_5 = new QPushButton(frame);
        pushButton_5->setObjectName(QString::fromUtf8("pushButton_5"));

        horizontalLayout_4->addWidget(pushButton_5);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_5);

        pushButton_6 = new QPushButton(frame);
        pushButton_6->setObjectName(QString::fromUtf8("pushButton_6"));

        horizontalLayout_4->addWidget(pushButton_6);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_6);


        verticalLayout->addLayout(horizontalLayout_4);


        verticalLayout_2->addWidget(frame);


        retranslateUi(FormSetting);

        QMetaObject::connectSlotsByName(FormSetting);
    } // setupUi

    void retranslateUi(QWidget *FormSetting)
    {
        FormSetting->setWindowTitle(QCoreApplication::translate("FormSetting", "Dialog", nullptr));
        label_6->setText(QString());
        label_7->setText(QCoreApplication::translate("FormSetting", "\346\202\254\346\265\256\347\252\227\345\244\226\350\247\202", nullptr));
        groupBox->setTitle(QCoreApplication::translate("FormSetting", "\351\242\234\350\211\262\345\222\214\351\200\217\346\230\216\345\272\246\350\256\276\347\275\256", nullptr));
        label->setText(QCoreApplication::translate("FormSetting", "\351\242\234\350\211\262", nullptr));
#if QT_CONFIG(tooltip)
        pushButton->setToolTip(QCoreApplication::translate("FormSetting", "\351\200\211\346\213\251\351\242\234\350\211\262", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton->setText(QString());
        label_2->setText(QCoreApplication::translate("FormSetting", "\344\270\215\351\200\217\346\230\216\345\272\246", nullptr));
        label_8->setText(QCoreApplication::translate("FormSetting", "190", nullptr));
#if QT_CONFIG(tooltip)
        pushButton_2->setToolTip(QCoreApplication::translate("FormSetting", "\351\242\204\350\247\210\351\200\211\346\213\251\347\232\204\351\242\234\350\211\262\345\222\214\344\270\215\351\200\217\346\230\216\345\272\246", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton_2->setText(QCoreApplication::translate("FormSetting", "\351\242\204\350\247\210", nullptr));
#if QT_CONFIG(tooltip)
        pushButton_7->setToolTip(QCoreApplication::translate("FormSetting", "\346\201\242\345\244\215\344\271\213\345\211\215\347\232\204\351\242\234\350\211\262\345\222\214\351\200\217\346\230\216\345\272\246", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton_7->setText(QCoreApplication::translate("FormSetting", "\346\201\242\345\244\215", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("FormSetting", "\345\234\206\350\247\222\350\256\276\347\275\256", nullptr));
        label_5->setText(QCoreApplication::translate("FormSetting", "\345\234\206\350\247\222\345\215\212\345\276\204", nullptr));
        radioButton->setText(QCoreApplication::translate("FormSetting", "\346\227\240", nullptr));
        radioButton_2->setText(QCoreApplication::translate("FormSetting", "\345\260\217", nullptr));
        radioButton_3->setText(QCoreApplication::translate("FormSetting", "\345\244\247", nullptr));
#if QT_CONFIG(tooltip)
        pushButton_3->setToolTip(QCoreApplication::translate("FormSetting", "\351\242\204\350\247\210\351\200\211\344\270\255\347\232\204\345\234\206\350\247\222", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton_3->setText(QCoreApplication::translate("FormSetting", "\351\242\204\350\247\210", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("FormSetting", "WIN10\351\243\216\346\240\274", nullptr));
        radioButton_4->setText(QCoreApplication::translate("FormSetting", "\351\273\230\350\256\244", nullptr));
        radioButton_5->setText(QCoreApplication::translate("FormSetting", "\351\200\217\346\230\216", nullptr));
        radioButton_6->setText(QCoreApplication::translate("FormSetting", "\347\216\273\347\222\203", nullptr));
        radioButton_7->setText(QCoreApplication::translate("FormSetting", "\344\272\232\345\205\213\345\212\233", nullptr));
        pushButton_4->setText(QCoreApplication::translate("FormSetting", "\351\242\204\350\247\210", nullptr));
#if QT_CONFIG(tooltip)
        pushButton_5->setToolTip(QCoreApplication::translate("FormSetting", "\351\242\204\350\247\210\351\273\230\350\256\244\347\232\204\346\240\267\345\274\217\350\256\276\347\275\256", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton_5->setText(QCoreApplication::translate("FormSetting", "\351\242\204\350\247\210\351\273\230\350\256\244", nullptr));
#if QT_CONFIG(tooltip)
        pushButton_6->setToolTip(QCoreApplication::translate("FormSetting", "*\344\277\235\345\255\230\345\271\266\345\272\224\347\224\250\346\255\244\351\241\265\344\270\212\351\235\242\347\232\204\346\225\260\346\215\256", nullptr));
#endif // QT_CONFIG(tooltip)
        pushButton_6->setText(QCoreApplication::translate("FormSetting", "\344\277\235\345\255\230\346\233\264\346\224\271", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FormSetting: public Ui_FormSetting {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FORMSETTING_H
