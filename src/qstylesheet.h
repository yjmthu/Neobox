#ifndef QSTYLESHEET_H
#define QSTYLESHEET_H

#include <fstream>
#include <QString>


struct QStyleSheet
{
    uint32_t bk_red, bk_green, bk_blue, bk_alpha;
    uint32_t ft_red, ft_green, ft_blue, ft_alpha;
    uint32_t bd_red, bd_green, bd_blue, bd_alpha;
    uint32_t bd_width, bd_radius, bk_fuzzy, ft_size;
    uint32_t bk_win, bd_have, bk_img, other2;

    enum _Border { None = 0, Left = 1, Top = 2, Right = 4, Bottom = 8, Border = 16 };
    enum _BorderRadius {
        TopLeft = 32, TopRight = 64, BottomLeft = 128, BottomRight = 256, BorderRadius = 512,
        TheAround = BorderRadius | TopLeft | TopRight | BottomLeft | BottomRight,
        TheLeft = BorderRadius | TopLeft | BottomLeft,
        TheRight = BorderRadius | TopRight | BottomRight,
        TheTop = BorderRadius | TopLeft | TopRight,
        TheBottom = BorderRadius | BottomLeft | BottomRight,
        TheTopLeft = BorderRadius | TopLeft, TheTopRight = BorderRadius | TopRight,
        TheBottomLeft = BorderRadius | BottomLeft, TheBottomRight = BorderRadius | BottomRight
    };

    inline QString getBorder() {
        QString str;
        if (!(bd_have & Border)) return QStringLiteral("border:none;\n");
        if (bd_have & Left)
            str += QStringLiteral("border-left-width:%1px;border-left-style:solid;border-left-color:rgba(%2,%3,%4,%5);\n").arg(QString::number(bd_width), QString::number(bd_red), QString::number(bd_green), QString::number(bd_blue), QString::number(bd_alpha));
        else
            str += QStringLiteral("border-left:none;");
        if (bd_have & Top)
            str += QStringLiteral("border-top-width:%1px;border-top-style:solid;border-top-color:rgba(%2,%3,%4,%5);\n").arg(QString::number(bd_width), QString::number(bd_red), QString::number(bd_green), QString::number(bd_blue), QString::number(bd_alpha));
        else
            str += QStringLiteral("border-top:none;");
        if (bd_have & Right)
            str += QStringLiteral("border-right-width:%1px;border-right-style:solid;border-right-color:rgba(%2,%3,%4,%5);\n").arg(QString::number(bd_width), QString::number(bd_red), QString::number(bd_green), QString::number(bd_blue), QString::number(bd_alpha));
        else
            str += QStringLiteral("border-right:none;");
        if (bd_have & Bottom)
            str += QStringLiteral("border-bottom-width:%1px;border-bottom-style:solid;border-bottom-color:rgba(%2,%3,%4,%5);\n").arg(QString::number(bd_width), QString::number(bd_red), QString::number(bd_green), QString::number(bd_blue), QString::number(bd_alpha));
        else
            str += QStringLiteral("border-bottom:none;");
        return str;
    }

    inline QString getBorderRadius() {
        QString str;
        if (!(bd_have & BorderRadius)) return QStringLiteral("border-radius:none;\n");
        str += QStringLiteral(
            "border-top-left-radius:%1px;\n"
            "border-top-right-radius:%2px;\n"
            "border-bottom-left-radius:%3px;\n"
            "border-bottom-right-radius:%4px;\n"
        ).arg((bd_have & TopLeft)? bd_radius: 0).arg((bd_have & TopRight)? bd_radius: 0).arg((bd_have & BottomLeft)? bd_radius: 0).arg((bd_have & BottomRight)? bd_radius: 0);
        return str;
    }

    inline QString getString(const QString& class_name) {
        return QStringLiteral("%1{\n"
                           "background-color:rgba(%2,%3,%4,%5);\n"
                           "color:rgba(%6,%7,%8,%9);\n"
                           "font-size:%10pt;\n"
                           "%11"
                       "}\n").arg(
            class_name,
            QString::number(bk_red), QString::number(bk_green), QString::number(bk_blue), QString::number(bk_alpha),
            QString::number(ft_red), QString::number(ft_green), QString::number(ft_blue), QString::number(ft_alpha),
            QString::number(ft_size), getBorder() + getBorderRadius()
        );
    }

    inline QString getString(bool l)
    {
        return getString(l ? QStringLiteral("QLabel"): QStringLiteral("QFrame"));
    }

    static inline bool toFile(QStyleSheet* sheet) {
        std::ofstream file(".box-style", std::ios::out | std::ios::binary);
        if (file.is_open()) {
            file.write(reinterpret_cast<const char *>(sheet), sizeof (QStyleSheet) * 4);
            file.close();
            return true;
        } else {
            return false;
        }
    }

    static inline bool fromFile(QStyleSheet* & sheet) {
        std::ifstream file(".box-style", std::ios::in | std::ios::binary);
        if (file.is_open()) {
            sheet = new QStyleSheet[4];
            file.read(reinterpret_cast<char *>(sheet), sizeof (QStyleSheet) * 4);
            file.close();
            return true;
        } else {
            sheet = nullptr;
            return false;
        }
    }
};

#endif // QSTYLESHEET_H
