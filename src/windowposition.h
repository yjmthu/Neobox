#ifndef WINDOWPOSITION_H
#define WINDOWPOSITION_H

#include <QPoint>
#include <QDebug>
#include <fstream>

struct WindowPosition {
    QPoint m_netFormPos;
    QPoint m_noteDoorPos;
    QPoint m_noteBookPos;
    QPoint m_squareClockPos;
    QPoint m_roundClockPos;
    // QPoint m_other1, m_other2, m_other3, m_other4;
    void toFile() {
        std::ofstream file(".window-pos", std::ios::out | std::ios::binary);
        if (file.is_open()) {
            file.write(reinterpret_cast<const char*>(this), sizeof (WindowPosition));
            file.close();
        } else {
            qDebug("写入窗口位置失败。");
        }
    }
    static WindowPosition* fromFile() {
        WindowPosition *ret { new WindowPosition };
        std::ifstream file(".window-pos", std::ios::in | std::ios::binary);
        if (file.is_open()) {
            qDebug("读取窗口位置成功。");
            file.read(reinterpret_cast<char*>(ret), sizeof (WindowPosition));
            file.close();
            return ret;
        } else {
            qDebug("读取窗口位置失败。");
            memset(ret, 0, sizeof (WindowPosition));
            return ret;
        }
    }
};


#endif // WINDOWPOSITION_H
