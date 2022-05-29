#ifndef SPEEDAPP_H
#define SPEEDAPP_H

#include <QApplication>

class VarBox
{
public:
    VarBox();
    ~VarBox();
    class Wallpaper *m_Wallpaper;
    class QSystemTrayIcon *m_Tray;
    class YJson* m_Setting;
    class SpeedMenu* m_Menu;
    class Translater* m_Translater;
    void SaveSetting();
private:
    class SpeedBox* m_SpeedBox;
    friend class SpeedMenu;
    void GetSetting();
    void LoadFonts();
};

extern VarBox* m_VarBox;

#endif // SPEEDAPP_H
