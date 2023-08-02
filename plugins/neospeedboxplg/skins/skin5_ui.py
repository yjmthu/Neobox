# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'skin5.ui'
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
from PySide6.QtWidgets import (QApplication, QFrame, QLabel, QSizePolicy,
    QWidget)

class Ui_center5(object):
    def setupUi(self, center5):
        if not center5.objectName():
            center5.setObjectName(u"center5")
        center5.resize(150, 50)
        center5.setCursor(QCursor(Qt.PointingHandCursor))
        center5.setStyleSheet(u"QWidget#center5 {\n"
"	background-color: transparent;\n"
"    border-image: url(\":/icons/archlinux-light.png\");\n"
"}\n"
"QWidget:hover#center5 {\n"
"	background-color: transparent;\n"
"    border-image: url(\":/icons/archlinux-dark.png\");\n"
"}")
        self.memUse = QLabel(center5)
        self.memUse.setObjectName(u"memUse")
        self.memUse.setGeometry(QRect(3, 4, 42, 42))
        self.memUse.setStyleSheet(u"font: 16pt \"\u534e\u6587\u7425\u73c0\";\n"
"color: rgb(30, 30, 30);")
        self.memUse.setAlignment(Qt.AlignCenter)
        self.netUp = QLabel(center5)
        self.netUp.setObjectName(u"netUp")
        self.netUp.setGeometry(QRect(105, 15, 60, 20))
        self.netUp.setStyleSheet(u"font: 7pt \"Microsoft YaHei\";\n"
"color: rgb(0, 0, 0);")
        self.netDown = QLabel(center5)
        self.netDown.setObjectName(u"netDown")
        self.netDown.setGeometry(QRect(62, 15, 60, 20))
        self.netDown.setStyleSheet(u"font: 7pt \"Microsoft YaHei\";\n"
"color: rgb(0, 0, 0);")
        self.percent = QLabel(center5)
        self.percent.setObjectName(u"percent")
        self.percent.setGeometry(QRect(35, 17, 20, 20))
        self.percent.setStyleSheet(u"font: 7pt \"\u534e\u6587\u7ec6\u9ed1\";\n"
"color: rgb(0, 0, 0);")
        self.frame = QFrame(center5)
        self.frame.setObjectName(u"frame")
        self.frame.setGeometry(QRect(4, 4, 42, 42))
        self.frame.setStyleSheet(u"background-color: rgb(95, 141, 211);\n"
"border-radius: 21px;")
        self.frame.setFrameShape(QFrame.StyledPanel)
        self.frame.setFrameShadow(QFrame.Raised)
        self.frame_2 = QFrame(center5)
        self.frame_2.setObjectName(u"frame_2")
        self.frame_2.setGeometry(QRect(40, 11, 106, 28))
        self.frame_2.setStyleSheet(u"background-color: rgb(170, 204, 255);\n"
"border-top-right-radius:    14px;\n"
"border-bottom-right-radius: 14px;")
        self.frame_2.setFrameShape(QFrame.StyledPanel)
        self.frame_2.setFrameShadow(QFrame.Raised)
        self.widget = QWidget(center5)
        self.widget.setObjectName(u"widget")
        self.widget.setGeometry(QRect(0, 0, 150, 50))
        self.widget.setStyleSheet(u"border-image: url(:/icons/archlinux.png)")
        self.frame_2.raise_()
        self.frame.raise_()
        self.widget.raise_()
        self.memUse.raise_()
        self.netUp.raise_()
        self.netDown.raise_()
        self.percent.raise_()

        self.retranslateUi(center5)

        QMetaObject.connectSlotsByName(center5)
    # setupUi

    def retranslateUi(self, center5):
        center5.setWindowTitle(QCoreApplication.translate("center5", u"Form", None))
#if QT_CONFIG(tooltip)
        center5.setToolTip(QCoreApplication.translate("center5", u"\u884c\u52d5\u662f\u6cbb\u7652\u6050\u61fc\u7684\u826f\u85e5\uff0c\u800c\u7336\u8c6b\u3001\u62d6\u5ef6\u5c07\u4e0d\u65b7\u6ecb\u990a\u6050\u61fc", None))
#endif // QT_CONFIG(tooltip)
        self.memUse.setText(QCoreApplication.translate("center5", u"00", None))
        self.netUp.setText(QCoreApplication.translate("center5", u"0 B", None))
        self.netDown.setText(QCoreApplication.translate("center5", u"0 B", None))
        self.percent.setText(QCoreApplication.translate("center5", u"%", None))
    # retranslateUi

