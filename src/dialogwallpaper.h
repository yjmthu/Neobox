#ifndef DIALOGWALLPAPER_H
#define DIALOGWALLPAPER_H

#include "wallpaper.h"

class DialogWallpaper : public Wallpaper
{
    Q_OBJECT

public:
    void setWallhaven() override;
    void setBing() override;
    void setNative() override;
    void setAdvance() override;
    void setOther() override;
    void startWork() override;
    ~DialogWallpaper();
    void iniStart();
};

#endif // DIALOGWALLPAPER_H
