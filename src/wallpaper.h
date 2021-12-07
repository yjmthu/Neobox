#ifndef WALLPAPER_H
#define WALLPAPER_H

class YJson;
class QNetworkAccessManager;
class QNetworkConfigurationManager;
#include <QThread>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include <random>

class Wallpaper: public QObject
{
    Q_OBJECT

signals:
    void msgBox(const char* str, const char* title);
    void setFailed(const char*);
private:
    std::random_device _rd; std::mt19937 _gen;
    const std::function<bool(void*)> SystemParametersInfo;
    void _set_w(YJson*);
    void _set_b(YJson*);
    bool set_wallpaper(const QString& pic);
    void set_from_Wallhaven();            //从ImgData.json中选取链接下载图片设置壁纸，返回成功状态
    void set_from_Bing();                 //从必应下载壁纸，根据传入的布尔值决定是否设置为壁纸
    void set_from_Native();               //从本地文件夹选取图片设为壁纸
    void set_from_Advance();              //根据高级命令设置壁纸
    void set_from_Other();                //使用其它Api
    void get_url_from_Wallhaven(YJson* urlObject);   //从Wallhaven获取120个壁纸链接
    void get_url_from_Bing();             //从Wallhaven获取120个壁纸链接
    QThread* thrd = nullptr;
    QNetworkAccessManager* mgr = nullptr;

public:
    Wallpaper(); ~Wallpaper();
    void push_back();
    void next();
    void prev();
    void apply();
    void dislike();
    bool applyClicked;
    inline bool isActive() const { return mgr || thrd; };
    void kill();
    std::list<wchar_t*> PicHistory;
    std::list<wchar_t*>::const_iterator CurPic;
    bool update;
    std::string url;
    QUrl bing_api;
    QString bing_folder;
    QString image_path;
    QString image_name;
    QTimer* timer = nullptr;                       //定时更换壁纸
};

#endif // WALLPAPER_H
