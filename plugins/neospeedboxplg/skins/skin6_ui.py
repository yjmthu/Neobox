# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'skin6.ui'
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
    QVBoxLayout, QWidget)

class Ui_center6(object):
    def setupUi(self, center6):
        if not center6.objectName():
            center6.setObjectName(u"center6")
        center6.resize(120, 52)
        center6.setMinimumSize(QSize(120, 52))
        center6.setMaximumSize(QSize(120, 52))
        center6.setCursor(QCursor(Qt.PointingHandCursor))
        center6.setStyleSheet(u"QWidget#center6 {\n"
"background-color: rgba(255, 255, 255, 0);\n"
"}")
        self.memUse = QLabel(center6)
        self.memUse.setObjectName(u"memUse")
        self.memUse.setGeometry(QRect(0, 0, 52, 52))
        self.memUse.setMinimumSize(QSize(52, 52))
        self.memUse.setMaximumSize(QSize(52, 52))
        self.memUse.setStyleSheet(u"background-color: transparent;\n"
"font: 700 13pt \"Magneto\";\n"
"color: rgb(135, 243, 255);\n"
"border-color: rgb(148, 148, 148);")
        self.memUse.setAlignment(Qt.AlignCenter)
        self.frame = QFrame(center6)
        self.frame.setObjectName(u"frame")
        self.frame.setGeometry(QRect(52, 5, 68, 42))
        self.frame.setMinimumSize(QSize(68, 42))
        self.frame.setMaximumSize(QSize(68, 42))
        self.frame.setStyleSheet(u"QFrame#frame{\n"
"	background-color: rgba(215, 255, 130, 150);\n"
"	border-top-right-radius: 4px;\n"
"    border-bottom-right-radius: 4px;\n"
"}")
        self.frame.setFrameShape(QFrame.StyledPanel)
        self.frame.setFrameShadow(QFrame.Raised)
        self.verticalLayout = QVBoxLayout(self.frame)
        self.verticalLayout.setSpacing(0)
        self.verticalLayout.setObjectName(u"verticalLayout")
        self.verticalLayout.setContentsMargins(2, 2, 0, 2)
        self.netUp = QLabel(self.frame)
        self.netUp.setObjectName(u"netUp")
        self.netUp.setStyleSheet(u"font: 8pt \"Microsoft YaHei UI\";\n"
"color: rgb(252, 255, 96);\n"
"color: rgb(255, 121, 44);\n"
"background-color: transparent;\n"
"")

        self.verticalLayout.addWidget(self.netUp)

        self.netDown = QLabel(self.frame)
        self.netDown.setObjectName(u"netDown")
        self.netDown.setStyleSheet(u"font: 8pt \"Microsoft YaHei UI\";\n"
"color: rgb(48, 255, 124);\n"
"background-color: transparent;")

        self.verticalLayout.addWidget(self.netDown)

        self.memColorFrame = QFrame(center6)
        self.memColorFrame.setObjectName(u"memColorFrame")
        self.memColorFrame.setGeometry(QRect(4, 4, 44, 44))
        self.memColorFrame.setFrameShape(QFrame.StyledPanel)
        self.memColorFrame.setFrameShadow(QFrame.Raised)
        self.borderColor = QFrame(center6)
        self.borderColor.setObjectName(u"borderColor")
        self.borderColor.setGeometry(QRect(0, 0, 52, 52))
        self.borderColor.setMaximumSize(QSize(52, 52))
        self.borderColor.setStyleSheet(u"background-color: rgba(32, 255, 188, 100);\n"
"border-radius: 5px;")
        self.borderColor.setFrameShape(QFrame.StyledPanel)
        self.borderColor.setFrameShadow(QFrame.Raised)
        self.frame.raise_()
        self.borderColor.raise_()
        self.memColorFrame.raise_()
        self.memUse.raise_()

        self.retranslateUi(center6)

        QMetaObject.connectSlotsByName(center6)
    # setupUi

    def retranslateUi(self, center6):
        center6.setWindowTitle(QCoreApplication.translate("center6", u"Form", None))
#if QT_CONFIG(tooltip)
        center6.setToolTip(QCoreApplication.translate("center6", u"\u884c\u52d5\u662f\u6cbb\u7652\u6050\u61fc\u7684\u826f\u85e5\uff0c\u800c\u7336\u8c6b\u3001\u62d6\u5ef6\u5c07\u4e0d\u65b7\u6ecb\u990a\u6050\u61fc", None))
#endif // QT_CONFIG(tooltip)
    # retranslateUi

