#ifndef QSTYLESHEET_H
#define QSTYLESHEET_H

#include <QString>

struct QStyleSheet
{
    int bk_red, bk_green, bk_blue, bk_alpha;
    int ft_red, ft_green, ft_blue, ft_alpha;
    int bd_red, bd_green, bd_blue, bd_alpha;
    int bd_width, bd_radius, bk_fuzzy, ft_size;
    int bk_win, other1, other2, other3;

    inline QString getString(QString class_name) {
        return QString("%1{\n"
                           "background-color:rgba(%2,%3,%4,%5);\n"
                           "color:rgba(%6,%7,%8,%9);\n"
                           "border%10;\n"                          // border: 1px solid #ff00ff
                           "border-radius:%11;\n"
                           "font-size:%12pt\n"
                       "}\n").arg(
            class_name,
            QString::number(bk_red), QString::number(bk_green), QString::number(bk_blue), QString::number(bk_alpha),
            QString::number(ft_red), QString::number(ft_green), QString::number(ft_blue), QString::number(ft_alpha),
            (bd_alpha ? QString("-width:%1px;\nborder-style:solid;\nborder-color:rgba(%2,%3,%4,%5)").arg(
                QString::number(bd_width), QString::number(bd_red), QString::number(bd_green), QString::number(bd_blue), QString::number(bd_alpha)
                ):QString(":none")
            ), QString("%1px").arg(bd_radius),
            QString::number(ft_size)
        );
    }
};

#endif // QSTYLESHEET_H
