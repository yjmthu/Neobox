#ifndef WALLPAPER_H
#define WALLPAPER_H

class YJson;
class QNetworkAccessManager;
class QNetworkConfigurationManager;
#include <QThread>
#include <QObject>
#include <QString>
#include <QUrl>

class Wallpaper: public QObject
{
    Q_OBJECT

signals:
    void msgBox(const char* str, const char* title);
    void setFailed(const char*);
private:
    void _set_w(YJson*);
    void _set_b(YJson*);
    QNetworkConfigurationManager* netmgr;
    void set_from_Wallhaven();           //从ImgData.db中选取链接下载图片设置壁纸，返回成功状态
    void set_from_Bing();            //从必应下载壁纸，根据传入的布尔值决定是否设置为壁纸
    void set_from_Native();                //从本地文件夹选取图片设为壁纸
    void set_from_Advance();             //根据高级命令设置壁纸
    void set_from_Other();                //从Wallhaven获取120个壁纸链接
    void get_url_from_Wallhaven(YJson*);   //从Wallhaven获取120个壁纸链接
    void get_url_from_Bing();   //从Wallhaven获取120个壁纸链接
    QThread* thrd;
    QNetworkAccessManager* mgr;

public:
    void next(); void prev(); void apply(); void dislike();
    bool applyClicked;
    Wallpaper(); ~Wallpaper();
    bool isActive() { return mgr || thrd; };
    void kill();
    bool set_bing = false; bool update;
    std::string url; QUrl bing_api; QString bing_folder;
    QString image_path; QString image_name;
public slots:
    void timer();
};

#endif // WALLPAPER_H
