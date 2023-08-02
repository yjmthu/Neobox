# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'skin2.ui'
##
## Created by: Qt User Interface Compiler version 6.5.1
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QBrush, QColor, QConicalGradient, QCursor,
    QFont, QFontDatabase, QGradient, QIcon,
    QImage, QKeySequence, QLinearGradient, QPainter,
    QPalette, QPixmap, QRadialGradient, QTransform)
from PySide6.QtWidgets import (QApplication, QFrame, QHBoxLayout, QLabel,
    QSizePolicy, QSpacerItem, QVBoxLayout, QWidget)

class Ui_center2(object):
    def setupUi(self, center2):
        if not center2.objectName():
            center2.setObjectName(u"center2")
        center2.resize(114, 54)
        center2.setMinimumSize(QSize(114, 54))
        center2.setMaximumSize(QSize(114, 54))
        center2.setCursor(QCursor(Qt.PointingHandCursor))
        center2.setStyleSheet(u"QWidget#center2 {\n"
"background-color: rgba(255, 255, 255, 0);\n"
"}\n"
"QWidget:hover#center2{\n"
"border-image: url(:/icons/guanjia_shadow_hoverd.png);\n"
"}\n"
"QWidget#center2 {\n"
"border-image: url(:/icons/guanjia_shadow_normal.png);\n"
"}")
        self.frame = QFrame(center2)
        self.frame.setObjectName(u"frame")
        self.frame.setGeometry(QRect(2, 2, 50, 50))
        self.frame.setMinimumSize(QSize(50, 50))
        self.frame.setMaximumSize(QSize(50, 50))
        self.frame.setStyleSheet(u"background-color: #595c6b;\n"
"border-radius: 25px;")
        self.frame.setFrameShape(QFrame.StyledPanel)
        self.frame.setFrameShadow(QFrame.Raised)
        self.frame_2 = QFrame(center2)
        self.frame_2.setObjectName(u"frame_2")
        self.frame_2.setGeometry(QRect(5, 5, 44, 44))
        self.frame_2.setMinimumSize(QSize(44, 44))
        self.frame_2.setMaximumSize(QSize(44, 44))
        self.frame_2.setStyleSheet(u"background-color: qconicalgradient(cx:0.5, cy:0.5, angle:90, stop:0 red, stop:0.5 yellow stop:1 #00ffff);\n"
"border-radius: 21px;")
        self.frame_2.setFrameShape(QFrame.StyledPanel)
        self.frame_2.setFrameShadow(QFrame.Raised)
        self.memColorFrame = QFrame(center2)
        self.memColorFrame.setObjectName(u"memColorFrame")
        self.memColorFrame.setGeometry(QRect(3, 3, 48, 48))
        self.memColorFrame.setMinimumSize(QSize(48, 48))
        self.memColorFrame.setMaximumSize(QSize(48, 48))
        self.memColorFrame.setFrameShape(QFrame.StyledPanel)
        self.memColorFrame.setFrameShadow(QFrame.Raised)
        self.memUse = QLabel(center2)
        self.memUse.setObjectName(u"memUse")
        self.memUse.setGeometry(QRect(7, 7, 40, 40))
        self.memUse.setMinimumSize(QSize(40, 40))
        self.memUse.setMaximumSize(QSize(40, 40))
        self.memUse.setStyleSheet(u"background-color: #595c6b;\n"
"border-radius: 19px;\n"
"font: 16pt \"\u534e\u6587\u7ec6\u9ed1\";\n"
"color: rgb(223, 223, 223);")
        self.memUse.setAlignment(Qt.AlignCenter)
        self.frame_4 = QFrame(center2)
        self.frame_4.setObjectName(u"frame_4")
        self.frame_4.setGeometry(QRect(32, 8, 80, 38))
        self.frame_4.setMinimumSize(QSize(80, 38))
        self.frame_4.setMaximumSize(QSize(80, 3))
        self.frame_4.setStyleSheet(u"background-color: #343740;\n"
"border-radius: 18px;\n"
"")
        self.frame_4.setFrameShape(QFrame.StyledPanel)
        self.frame_4.setFrameShadow(QFrame.Raised)
        self.horizontalLayout = QHBoxLayout(self.frame_4)
        self.horizontalLayout.setSpacing(0)
        self.horizontalLayout.setObjectName(u"horizontalLayout")
        self.horizontalLayout.setContentsMargins(21, 1, 15, 1)
        self.verticalLayout = QVBoxLayout()
        self.verticalLayout.setSpacing(0)
        self.verticalLayout.setObjectName(u"verticalLayout")
        self.label = QLabel(self.frame_4)
        self.label.setObjectName(u"label")
        self.label.setStyleSheet(u"font: 700 7pt \"Microsoft YaHei UI\";\n"
"color: #549eca;")
        self.label.setAlignment(Qt.AlignCenter)

        self.verticalLayout.addWidget(self.label)

        self.label_2 = QLabel(self.frame_4)
        self.label_2.setObjectName(u"label_2")
        self.label_2.setMaximumSize(QSize(16777215, 16777215))
        self.label_2.setStyleSheet(u"font: 7pt \"Microsoft YaHei UI\";\n"
"color: #2fb634;")
        self.label_2.setAlignment(Qt.AlignCenter)

        self.verticalLayout.addWidget(self.label_2)


        self.horizontalLayout.addLayout(self.verticalLayout)

        self.horizontalSpacer = QSpacerItem(40, 20, QSizePolicy.Expanding, QSizePolicy.Minimum)

        self.horizontalLayout.addItem(self.horizontalSpacer)

        self.verticalLayout_2 = QVBoxLayout()
        self.verticalLayout_2.setSpacing(0)
        self.verticalLayout_2.setObjectName(u"verticalLayout_2")
        self.netUp = QLabel(self.frame_4)
        self.netUp.setObjectName(u"netUp")
        self.netUp.setStyleSheet(u"font: 7pt \"Microsoft YaHei UI\";\n"
"color: rgb(223, 223, 223);")
        self.netUp.setAlignment(Qt.AlignRight|Qt.AlignTrailing|Qt.AlignVCenter)

        self.verticalLayout_2.addWidget(self.netUp)

        self.netDown = QLabel(self.frame_4)
        self.netDown.setObjectName(u"netDown")
        self.netDown.setStyleSheet(u"font: 7pt \"Microsoft YaHei UI\";\n"
"color: rgb(223, 223, 223);")
        self.netDown.setAlignment(Qt.AlignRight|Qt.AlignTrailing|Qt.AlignVCenter)

        self.verticalLayout_2.addWidget(self.netDown)


        self.horizontalLayout.addLayout(self.verticalLayout_2)

        self.label_3 = QLabel(center2)
        self.label_3.setObjectName(u"label_3")
        self.label_3.setGeometry(QRect(37, 20, 16, 16))
        self.label_3.setStyleSheet(u"color: rgb(255, 255, 255);\n"
"font: 6pt \"Microsoft YaHei UI\";")
        self.frame_4.raise_()
        self.frame.raise_()
        self.frame_2.raise_()
        self.memColorFrame.raise_()
        self.memUse.raise_()
        self.label_3.raise_()

        self.retranslateUi(center2)

        QMetaObject.connectSlotsByName(center2)
    # setupUi

    def retranslateUi(self, center2):
        center2.setWindowTitle(QCoreApplication.translate("center2", u"Form", None))
#if QT_CONFIG(tooltip)
        center2.setToolTip(QCoreApplication.translate("center2", u"\u884c\u52d5\u662f\u6cbb\u7652\u6050\u61fc\u7684\u826f\u85e5\uff0c\u800c\u7336\u8c6b\u3001\u62d6\u5ef6\u5c07\u4e0d\u65b7\u6ecb\u990a\u6050\u61fc", None))
#endif // QT_CONFIG(tooltip)
        self.label.setText(QCoreApplication.translate("center2", u"\u2191", None))
        self.label_2.setText(QCoreApplication.translate("center2", u"\u2193", None))
        self.label_3.setText(QCoreApplication.translate("center2", u"%", None))
    # retranslateUi

