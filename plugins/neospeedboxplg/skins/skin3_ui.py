# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'skin3.ui'
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
from PySide6.QtWidgets import (QApplication, QFrame, QGridLayout, QLabel,
    QSizePolicy, QWidget)

class Ui_center3(object):
    def setupUi(self, center3):
        if not center3.objectName():
            center3.setObjectName(u"center3")
        center3.resize(120, 50)
        center3.setMinimumSize(QSize(120, 50))
        center3.setMaximumSize(QSize(120, 50))
        center3.setCursor(QCursor(Qt.PointingHandCursor))
        center3.setStyleSheet(u"QWidget#center3{\n"
"background-color: rgba(255, 255, 255, 0);\n"
"}")
        self.memUse = QLabel(center3)
        self.memUse.setObjectName(u"memUse")
        self.memUse.setGeometry(QRect(0, 0, 50, 50))
        self.memUse.setMinimumSize(QSize(50, 50))
        self.memUse.setMaximumSize(QSize(50, 50))
        self.memUse.setStyleSheet(u"background-color: transparent;\n"
"font: 11pt \"Microsoft YaHei UI\";\n"
"color: rgb(255, 255, 255);\n"
"border-radius: 25px;")
        self.memUse.setAlignment(Qt.AlignCenter)
        self.frame = QFrame(center3)
        self.frame.setObjectName(u"frame")
        self.frame.setGeometry(QRect(2, 7, 118, 36))
        self.frame.setMinimumSize(QSize(118, 36))
        self.frame.setMaximumSize(QSize(118, 36))
        self.frame.setStyleSheet(u"QFrame#frame{\n"
"background-color: #FCFCFC;\n"
"border: 2px solid gray;\n"
"border-radius: 18px;\n"
"}")
        self.frame.setFrameShape(QFrame.StyledPanel)
        self.frame.setFrameShadow(QFrame.Raised)
        self.gridLayout = QGridLayout(self.frame)
        self.gridLayout.setObjectName(u"gridLayout")
        self.gridLayout.setContentsMargins(44, 0, 13, 0)
        self.label_2 = QLabel(self.frame)
        self.label_2.setObjectName(u"label_2")
        self.label_2.setMaximumSize(QSize(10, 16777215))
        self.label_2.setStyleSheet(u"color: rgb(152, 79, 78);\n"
"font: 7pt \"\u9ed1\u4f53\";")

        self.gridLayout.addWidget(self.label_2, 0, 0, 1, 1)

        self.netUp = QLabel(self.frame)
        self.netUp.setObjectName(u"netUp")
        self.netUp.setStyleSheet(u"font: 7pt \"Microsoft YaHei UI\";\n"
"color: #E09A13;\n"
"font-weight: bold;")

        self.gridLayout.addWidget(self.netUp, 0, 1, 1, 1)

        self.label = QLabel(self.frame)
        self.label.setObjectName(u"label")
        self.label.setMaximumSize(QSize(10, 16777215))
        self.label.setStyleSheet(u"color: rgb(45, 130, 85);\n"
"font: 7pt \"\u9ed1\u4f53\";")

        self.gridLayout.addWidget(self.label, 1, 0, 1, 1)

        self.netDown = QLabel(self.frame)
        self.netDown.setObjectName(u"netDown")
        self.netDown.setStyleSheet(u"font: 7pt \"Microsoft YaHei UI\";\n"
"color: #7CE00E;\n"
"font-weight: bold;")

        self.gridLayout.addWidget(self.netDown, 1, 1, 1, 1)

        self.memColorFrame = QFrame(center3)
        self.memColorFrame.setObjectName(u"memColorFrame")
        self.memColorFrame.setGeometry(QRect(5, 5, 40, 40))
        self.memColorFrame.setFrameShape(QFrame.StyledPanel)
        self.memColorFrame.setFrameShadow(QFrame.Raised)
        self.frame_2 = QFrame(center3)
        self.frame_2.setObjectName(u"frame_2")
        self.frame_2.setGeometry(QRect(1, 1, 48, 48))
        self.frame_2.setStyleSheet(u"background-color: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0 rgba(0, 255, 0, 255), stop:0.87 rgba(0, 100, 0, 255), stop:0.90 #eeeeee, stop:0.99 #ffffff, stop:1 #ffffff);\n"
"border-radius: 24px;")
        self.frame_2.setFrameShape(QFrame.StyledPanel)
        self.frame_2.setFrameShadow(QFrame.Raised)
        self.frame.raise_()
        self.frame_2.raise_()
        self.memColorFrame.raise_()
        self.memUse.raise_()

        self.retranslateUi(center3)

        QMetaObject.connectSlotsByName(center3)
    # setupUi

    def retranslateUi(self, center3):
        center3.setWindowTitle(QCoreApplication.translate("center3", u"Form", None))
#if QT_CONFIG(tooltip)
        center3.setToolTip(QCoreApplication.translate("center3", u"\u884c\u52d5\u662f\u6cbb\u7652\u6050\u61fc\u7684\u826f\u85e5\uff0c\u800c\u7336\u8c6b\u3001\u62d6\u5ef6\u5c07\u4e0d\u65b7\u6ecb\u990a\u6050\u61fc", None))
#endif // QT_CONFIG(tooltip)
        self.label_2.setText(QCoreApplication.translate("center3", u"\u2191", None))
        self.label.setText(QCoreApplication.translate("center3", u"\u2193", None))
    # retranslateUi

