#ifndef WALLPAPER_H
#define WALLPAPER_H

class YJson;
#include <thread>
#include <QObject>
#include <QString>

class Wallpaper: public QObject
{
    Q_OBJECT

signals:
    void finished(); void msgBox(const char*, const char*); void setFailed(const char*);
public slots:
    void start(); void clean();
protected:
    bool set_from_Wallhaven() const;           //从ImgData.db中选取链接下载图片设置壁纸，返回成功状态
    bool set_from_Bing(bool) const;            //从必应下载壁纸，根据传入的布尔值决定是否设置为壁纸
    void set_from_Native(bool);                //从本地文件夹选取图片设为壁纸
    bool set_from_Advance() const;             //根据高级命令设置壁纸
    bool set_from_Other() const;                //从Wallhaven获取120个壁纸链接
    bool get_url_from_Wallhaven(YJson&) const;   //从Wallhaven获取120个壁纸链接
    static std::thread* thrd;

public:
    virtual void setWallhaven() = 0;
    virtual void setBing() = 0;                //从必应下载并设置壁纸
    virtual void setOther() = 0;             //高级命令更换壁纸
    virtual void setNative() = 0;              //从本地更换壁纸
    virtual void setAdvance() = 0;             //高级命令更换壁纸
    virtual void startWork() = 0;              //根据壁纸类型决定执行public里面哪个set函数
    static bool initSet; static bool canCreat() { return !thrd; };
    Wallpaper(); ~Wallpaper(); bool isActive() { return thrd; };
    static char bing; static bool update;
    static std::string url; static std::string bing_api; static QString bing_folder;
    static QString image_path;
};

#endif // WALLPAPER_H
