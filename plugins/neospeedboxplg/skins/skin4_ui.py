# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'skin4.ui'
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
from PySide6.QtWidgets import (QApplication, QLabel, QSizePolicy, QWidget)

class Ui_center4(object):
    def setupUi(self, center4):
        if not center4.objectName():
            center4.setObjectName(u"center4")
        center4.resize(281, 27)
        center4.setCursor(QCursor(Qt.PointingHandCursor))
        center4.setStyleSheet(u"*#center4 {\n"
"	background-color: transparent;\n"
"    border-image: url(\":/icons/duba.png\");\n"
"}\n"
"\n"
"QLabel {\n"
"	color: rgb(255, 255, 255);\n"
"	font: 8pt \"Microsoft YaHei UI\";\n"
"}")
        self.cpuUse = QLabel(center4)
        self.cpuUse.setObjectName(u"cpuUse")
        self.cpuUse.setGeometry(QRect(14, 7, 52, 16))
        self.netUp = QLabel(center4)
        self.netUp.setObjectName(u"netUp")
        self.netUp.setGeometry(QRect(80, 7, 57, 16))
        self.netDown = QLabel(center4)
        self.netDown.setObjectName(u"netDown")
        self.netDown.setGeometry(QRect(160, 7, 57, 16))
        self.memUse = QLabel(center4)
        self.memUse.setObjectName(u"memUse")
        self.memUse.setGeometry(QRect(240, 7, 57, 16))

        self.retranslateUi(center4)

        QMetaObject.connectSlotsByName(center4)
    # setupUi

    def retranslateUi(self, center4):
        center4.setWindowTitle(QCoreApplication.translate("center4", u"Form", None))
    # retranslateUi

