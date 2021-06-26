#ifndef DIALOGWALLPAPER_H
#define DIALOGWALLPAPER_H

#include "wallpaper.h"

class DialogWallpaper : public Wallpaper
{
    Q_OBJECT

signals:
    void setFailed(QString);

public:
    explicit DialogWallpaper();
    ~DialogWallpaper();
    void run() override;
    void setWallhaven() override;
    void setBing() override;
    void setNative() override;
    void setRandom() override;
    void setAdvance() override;
};

#endif // DIALOGWALLPAPER_H
