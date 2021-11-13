#ifndef MENUWALLPAPER_H
#define MENUWALLPAPER_H

#include "wallpaper.h"
#include <QObject>

class MenuWallpaper :  public Wallpaper
{
    Q_OBJECT

public:
    void setWallhaven() override;
    void setBing() override;
    void setNative() override;
    void setAdvance() override;
    void setOther() override;
    void startWork() override;

public slots:
    void previousPic();
    void removePic();
};

#endif // MENUWALLPAPER_H
