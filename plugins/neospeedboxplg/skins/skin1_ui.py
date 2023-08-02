# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'skin1.ui'
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
from PySide6.QtWidgets import (QApplication, QHBoxLayout, QLabel, QSizePolicy,
    QVBoxLayout, QWidget)

class Ui_center1(object):
    def setupUi(self, center1):
        if not center1.objectName():
            center1.setObjectName(u"center1")
        center1.resize(100, 44)
        center1.setCursor(QCursor(Qt.PointingHandCursor))
        center1.setStyleSheet(u"QWidget#center1 {\n"
"  background-color: #33333377;\n"
"  border-radius: 2px;\n"
"}")
        self.horizontalLayout = QHBoxLayout(center1)
        self.horizontalLayout.setSpacing(0)
        self.horizontalLayout.setObjectName(u"horizontalLayout")
        self.horizontalLayout.setContentsMargins(2, 0, 0, 0)
        self.memUse = QLabel(center1)
        self.memUse.setObjectName(u"memUse")
        self.memUse.setMinimumSize(QSize(28, 0))
        self.memUse.setMaximumSize(QSize(28, 16777215))
        self.memUse.setCursor(QCursor(Qt.PointingHandCursor))
        self.memUse.setStyleSheet(u"QLabel#memUse {\n"
"  width: 24px;\n"
"  color: #00FFFF;\n"
"  font-size: 16pt;\n"
"  padding-left: 1px;\n"
"}")
        self.memUse.setAlignment(Qt.AlignCenter)

        self.horizontalLayout.addWidget(self.memUse)

        self.verticalLayout = QVBoxLayout()
        self.verticalLayout.setSpacing(0)
        self.verticalLayout.setObjectName(u"verticalLayout")
        self.verticalLayout.setContentsMargins(-1, 3, -1, 3)
        self.netUp = QLabel(center1)
        self.netUp.setObjectName(u"netUp")
        self.netUp.setMinimumSize(QSize(65, 0))
        self.netUp.setCursor(QCursor(Qt.PointingHandCursor))
        self.netUp.setStyleSheet(u"QLabel#netUp {\n"
"  color: #FAAA23;\n"
"  margin-left: 1px;\n"
"}")

        self.verticalLayout.addWidget(self.netUp)

        self.netDown = QLabel(center1)
        self.netDown.setObjectName(u"netDown")
        self.netDown.setMinimumSize(QSize(65, 0))
        self.netDown.setCursor(QCursor(Qt.PointingHandCursor))
        self.netDown.setStyleSheet(u"QLabel#netDown {\n"
"  color: #8CF01E;\n"
"  margin-left: 1px;\n"
"}")

        self.verticalLayout.addWidget(self.netDown)


        self.horizontalLayout.addLayout(self.verticalLayout)


        self.retranslateUi(center1)

        QMetaObject.connectSlotsByName(center1)
    # setupUi

    def retranslateUi(self, center1):
        center1.setWindowTitle(QCoreApplication.translate("center1", u"Form", None))
#if QT_CONFIG(tooltip)
        center1.setToolTip(QCoreApplication.translate("center1", u"\u884c\u52d5\u662f\u6cbb\u7652\u6050\u61fc\u7684\u826f\u85e5\uff0c\u800c\u7336\u8c6b\u3001\u62d6\u5ef6\u5c07\u4e0d\u65b7\u6ecb\u990a\u6050\u61fc", None))
#endif // QT_CONFIG(tooltip)
    # retranslateUi

