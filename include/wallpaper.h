#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <QThread>

class Translater;

class Wallpaper: public QThread
{
    Q_OBJECT

signals:
    void msgBox(QString);                      //传递错误信号给From

protected:
    bool set_from_Wallhaven() const;           //从ImgData.db中选取链接下载图片设置壁纸，返回成功状态
    bool set_from_Bing(bool) const;            //从必应下载壁纸，根据传入的布尔值决定是否设置为壁纸
    bool set_from_Native() const;              //从本地文件夹选取图片设为壁纸
    bool set_from_Random() const;              //随机下载图片设置壁纸
    bool set_from_Advance() const;             //根据高级命令设置壁纸
    bool get_url_from_Wallhaven(bool) const;   //从Wallhaven获取120个壁纸链接
    void startWork();                          //根据壁纸类型决定执行public里面哪个set函数

public:
    static bool a_thresd_isrunning;
    virtual void setWallhaven() = 0;           //从Wallhave下载并设置壁纸
    virtual void setBing() = 0;            //从必应下载并设置壁纸
    virtual void setNative() = 0;              //从本地更换壁纸
    virtual void setRandom() = 0;              //随机更换壁纸
    virtual void setAdvance() = 0;             //高级命令更换壁纸
};

#endif // WALLPAPER_H
