#ifndef MENUWALLPAPER_H
#define MENUWALLPAPER_H

#include "wallpaper.h"

class MenuWallpaper : public Wallpaper
{
    Q_OBJECT

public:
    explicit MenuWallpaper();
    ~MenuWallpaper();
    void run() override;
    void setWallhaven() override;
    void setBing() override;
    void setNative() override;
    void setRandom() override;
    void setAdvance() override;
};

#endif // MENUWALLPAPER_H
