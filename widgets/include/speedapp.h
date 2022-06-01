#ifndef SPEEDAPP_H
#define SPEEDAPP_H

#include <QApplication>
#include <QObject>

class VarBox: public QObject
{
    Q_OBJECT
public:
    VarBox();
    ~VarBox();
    class Wallpaper *m_Wallpaper;
    class QSystemTrayIcon *m_Tray;
    class SpeedMenu* m_Menu;
    class Translater* m_Translater;
private:
    class SpeedBox* m_SpeedBox;
    class QTimer* m_Timer;
    friend class SpeedMenu;
    void GetSetting();
    void LoadFonts();
};

extern VarBox* m_VarBox;

#endif // SPEEDAPP_H
