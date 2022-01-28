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
    bool systemParametersInfo(const std::string & path);
    bool systemParametersInfo(const std::wstring & path);
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
    bool m_doing = false;

public:
    enum class Type
    {
        Hot, Nature, Anime, Simple, Random, User, Bing, Other, Native, Advance
    };
    std::list<std::string> PicHistory;
    std::list<std::string>::const_iterator CurPic;
    bool update;
    std::string url;
    QUrl bing_api;
    QString bing_folder;
    QString image_path;
    QString image_name;
    class QTimer *timer = nullptr;                       //定时更换壁纸

    Type PaperType = Type::Hot;                                        //当下正在使用的壁纸类型
    bool AutoChange = false;
    unsigned char PageNum = 1;
    unsigned char TimeInterval;
    bool UseDateAsBingName;
    bool AutoSaveBingPicture;
    QString NativeDir;                                                 //当下正在使用的用户本地壁纸文件夹
    QString UserCommand;                //当下正在使用的用户高级命令
    bool FirstChange = true;

    Wallpaper(); ~Wallpaper();
    void push_back();
    void next();
    void prev();
    void apply();
    void dislike();
    bool applyClicked = false;
    inline bool isActive() const { return m_doing; };
    inline void kill() { m_doing = false; };

    static bool isOnline(bool wait=false);
};

#endif // WALLPAPER_H
