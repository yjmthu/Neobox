#include <cstdio>
#include <fstream>
#include <type_traits>
#include <QDateTime>
#include <QSettings>
#include <QDir>
#include <QFile>
#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
#include <QNetworkConfigurationManager>
#else
#include <QNetworkInformation>
#endif
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QEventLoop>
#if defined (Q_OS_LINUX)
#include <unistd.h>
#elif defined (Q_OS_WINDOWS)
#include <io.h>
#endif

#include "funcbox.h"
#include "wallpaper.h"
#include "YEncode.h"
#include "YString.h"
#include "YJson.h"

#if defined (Q_OS_WIN32)

std::string AnsiToUtf8(const std::string& strAnsi)//传入的strAnsi是GBK编码
{
    //gbk转unicode
    int len = MultiByteToWideChar(CP_ACP, 0, strAnsi.c_str(), -1, NULL, 0);
    wchar_t *strUnicode = new wchar_t[len];
    wmemset(strUnicode, 0, len);
    MultiByteToWideChar(CP_ACP, 0, strAnsi.c_str(), -1, strUnicode, len);

    //unicode转UTF-8
    len = WideCharToMultiByte(CP_UTF8, 0, strUnicode, -1, NULL, 0, NULL, NULL);
    char * strUtf8 = new char[len];
    WideCharToMultiByte(CP_UTF8, 0, strUnicode, -1, strUtf8, len, NULL, NULL);

    std::string strTemp(strUtf8);    //此时的strTemp是UTF-8编码
    delete[] strUnicode;
    delete[] strUtf8;
    return strTemp;
}

std::string Utf8ToAnsi(const std::string& strUtf8)//传入的strUtf8是UTF-8编码
{
    //UTF-8转unicode
    int len = MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, NULL, 0);
    wchar_t * strUnicode = new wchar_t[len];//len = 2
    wmemset(strUnicode, 0, len);
    MultiByteToWideChar(CP_UTF8, 0, strUtf8.c_str(), -1, strUnicode, len);

    //unicode转gbk
    len = WideCharToMultiByte(CP_ACP, 0, strUnicode, -1, NULL, 0, NULL, NULL);
    char *strAnsi = new char[len]; //len=3 本来为2，但是char*后面自动加上了\0
    memset(strAnsi, 0, len);
    WideCharToMultiByte(CP_ACP,0, strUnicode, -1, strAnsi, len, NULL, NULL);

    std::string strTemp(strAnsi);//此时的strTemp是GBK编码
    delete[] strUnicode;
    delete[] strAnsi;
    return strTemp;
}
#endif

FILE* readFile(const std::string& filePath)
{
#if defined (Q_OS_WIN32)
    const std::string ansiString(Utf8ToAnsi(filePath));
    return fopen(ansiString.c_str(), "rb");
#elif defined (Q_OS_LINUX)
    return fopen(filePath.c_str(), "rb");
#endif
}

const char* get_file_name(const char* file_path)
{
    const char* ptr = file_path;
    while (*++ptr);
    while (!strchr("\\/", *--ptr));
    return ++ptr;
}

void check_is_wallhaven(const char* pic, char* id)
{
    if (strlen(pic) != 20)
        return ;
    if (!strncmp(pic, "wallhaven-", 10) && StrContainCharInRanges<char>(pic+10, 6, "a-z", "0-9") &&
            (!strcmp(pic+16, ".png") || !strcmp(pic+16, ".jpg")))
    {
        for (int i=0; i < 20; ++i)
        {
            id[i] = static_cast<char>(pic[i]);
        }
    }
}

bool Wallpaper::set_wallpaper(const QString &file_path)             //根据路径设置壁纸
{
    if (QFile::exists(file_path))
    {
#if defined (Q_OS_WIN32)
        qout << "设置壁纸：" << file_path;
        const char *temp = file_path.toUtf8();
        PicHistory.emplace_back(temp);
        CurPic = --PicHistory.end();
        std::wstring temp_utf16;
        utf8_to_utf16LE<std::wstring&, const char*>(temp_utf16, temp);
        return systemParametersInfo(temp_utf16);
#elif defined (Q_OS_LINUX)
        PicHistory.push_back(file_path.toStdString());
        CurPic = --PicHistory.end();
        return systemParametersInfo(file_path.toStdString());
#endif
    }
    else
        return false;
}

Wallpaper::Wallpaper():
    _rd(), _gen(_rd()),
    m_doing(false), update(false), url(), bing_api(), bing_folder(), image_path(),  image_name(), timer(new QTimer)
{
    QSettings *IniRead = new QSettings("SpeedBox.ini", QSettings::IniFormat);
    IniRead->beginGroup("Wallpaper");
    NativeDir = IniRead->value("NativeDir").toString();
    unsigned safeEnum = IniRead->value("PaperType").toInt();
    if (safeEnum > 9) safeEnum = 0;
    PaperType = static_cast<Type>(safeEnum);
    TimeInterval = IniRead->value("TimeInerval").toInt();
    PageNum = IniRead->value("PageNum").toInt();
    UserCommand = IniRead->value("UserCommand").toString();
    AutoChange = IniRead->value("AutoChange").toBool();
    if (IniRead->contains("AutoRotationBingPicture")) {
        UseDateAsBingName = IniRead->value("UseDateAsBingName").toBool();
        AutoSaveBingPicture = IniRead->value("AutoSaveBingPicture").toBool();
    } else {
        IniRead->setValue("AutoSaveBingPicture", AutoSaveBingPicture);
        IniRead->setValue("UseDateAsBingName", UseDateAsBingName);
    }
    if (IniRead->contains("FirstChange")) {
        FirstChange = IniRead->value("FirstChange").toBool();
    } else {
        IniRead->setValue("FirstChange", FirstChange);
    }
    IniRead->endGroup();
    delete IniRead;
    qout << "读取壁纸信息完毕";

    connect(this, &Wallpaper::msgBox, VarBox, std::bind(VARBOX::MSG, std::placeholders::_1, std::placeholders::_2, QMessageBox::Ok));
#if defined (Q_OS_WIN32)
    QSettings set("HKEY_CURRENT_USER\\Control Panel\\Desktop", QSettings::NativeFormat);
    if (set.contains("WallPaper"))
    {
        QString temp_paper = set.value("WallPaper").toString();
        if (!temp_paper.isEmpty())
        {
            qout << "首张壁纸路径长度: " << temp_paper.length();
            PicHistory.emplace_back(temp_paper.toUtf8());
        }
    }
#endif
    CurPic = PicHistory.end();

    timer->setInterval(TimeInterval * 60000);
    if (FirstChange)
    {
        qout << "自动换";
        if (!isOnline(false))
        {
            qout << "往后追加壁纸0。。。。。";
            m_doing = true;
            auto thrd = QThread::create([=](){
                if (isOnline(true))
                {
                    qout << "往后追加壁纸1。。。。。";
                    push_back();
                }
            });
            connect(thrd, &QThread::finished, this, [=](){
                thrd->deleteLater();
                m_doing = false;
            });
            thrd->start();
        }
        else
        {
            qout << "往后追加壁纸2。。。。。";
            push_back();
        }
        if (AutoSaveBingPicture && PaperType != Type::Bing)
        {
            set_from_Bing();
        }
    }
    else qout << "不自动换";
    if (AutoChange)
        timer->start();
    connect(timer, &QTimer::timeout, this, &Wallpaper::next);
}

Wallpaper::~Wallpaper()
{
    delete timer;
}

bool Wallpaper::systemParametersInfo(const std::string &path)
{
    std::wstring str;
    utf8_to_utf16LE<std::wstring&, std::string::const_iterator>(str, path.begin());
#if defined (Q_OS_WIN32)
        return ::SystemParametersInfoW(
            SPI_SETDESKWALLPAPER,
            UINT(0),
            const_cast<wchar_t *>(str.c_str()),
            SPIF_SENDCHANGE | SPIF_UPDATEINIFILE
        );
#elif defined (Q_OS_LINUX)
        return true;
#endif
}

bool Wallpaper::systemParametersInfo(const std::wstring &path)
{
#if defined (Q_OS_WIN32)
        return ::SystemParametersInfoW(
            SPI_SETDESKWALLPAPER,
            UINT(0),
            const_cast<wchar_t *>(path.c_str()),
            SPIF_SENDCHANGE | SPIF_UPDATEINIFILE
        );
#elif defined (Q_OS_LINUX)
        constexpr char script[] = "./Scripts/SetWallPaper.sh";
        QStringList lst1, lst2;
        lst1.append("+x");
        lst1.append("./Scripts/SetWallPaper.sh");
        lst2.append(path.c_str());
        VarBox->runCmd("chmod", lst1, 0);                    //赋予脚本执行权限。
        VarBox->runCmd(script, lst2, 0);                     //利用脚本更换壁纸，这只可以在 linux系统+kde桌面 上使用。
        return true;
#endif
}

void Wallpaper::_set_w(YJson* jsonArray)
{
    qout << "智能设置壁纸开始.";

    if (jsonArray->getChild())
    {
        int pic_num = jsonArray->getChildNum();
        qout << "Wallhaven 找到随机id";
        std::uniform_int_distribution<int> dis(0, pic_num-1);
        YJson* item = jsonArray->find(dis(_gen));
        std::string pic_url = item->getValueString();
        jsonArray->getParent()->find("Used")->append(*item);
        jsonArray->remove(item);
        if (pic_url.length())
        {
            qout << "壁纸网址：" << pic_url.c_str();
            const QString img = QDir::toNativeSeparators(image_path + "/" + QString::fromStdString(pic_url));
            if (!QFile::exists(img))
            {
                qout << "本地找不到文件, 直接开始下载";
                m_doing = true;
                auto mgr = new QNetworkAccessManager;
                connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep)->void{
                    if (rep->error() != QNetworkReply::NoError)
                    {
                        qout << "下载出错！";
                        mgr->deleteLater();
                        m_doing = false;
                        return;
                    }
                    QFile file(img);
                    qout << "下载图片成功!";
                    if (file.open(QIODevice::WriteOnly))
                    {
                        file.write(rep->readAll());
                        file.close();
                        if (!file.size()) QFile::remove(img);
                        qout << file.size();
                    }
                    mgr->deleteLater();
                    m_doing = false;
                    set_wallpaper(img);
                });
                mgr->get(QNetworkRequest(QString::fromStdString("https://w.wallhaven.cc/full/"+pic_url.substr(10, 2)+"/"+pic_url)));
            }
            else
            {
                qout << "本地文件存在";
                set_wallpaper(img);
            }
            qout << "壁纸设置完毕";
        }
        qout << "保存 json 文件";
        jsonArray->getTop()->toFile("ImgData.json", YJson::UTF8BOM, true);
        qout << "json 文件保存完毕";
    }
    else
    {
        emit msgBox("当前页面没有找到图片, 请切换较小的页面或者更换壁纸类型!", "提示");
    }
    delete jsonArray->getTop();
}

void Wallpaper::_set_b(YJson * file_data)
{
    qout << "详细处理";
    int curindex = file_data->find("current")->getValueInt();
    qout << "当前索引: " << curindex;
    YJson* temp = file_data->find("images");
    qout << (bool)*temp;
    temp = temp->find(curindex);
    qout << "加载必应json文件成功!";
    if (!temp)
    {
        qout << "到达末尾";
        temp = file_data->find("images")->getChild();
        file_data->find("current")->setValue(1);
    }
    else
    {
        qout << "加一";
        file_data->find("current")->setValue(curindex+1);
        qout << "当前索引: " << file_data->find("current")->getValueInt();
    }
    qout << "查找到images";
    std::string img_url("https://cn.bing.com");
    img_url += temp->find("url")->getValueString();
    QString bing_name;
    if (UseDateAsBingName)
    {
        qout << "使用日期名称";
        bing_name = temp->find("enddate")->getValueString();
        bing_name.insert(4, '-');
        bing_name.insert(7, '-');
        bing_name = QDir::toNativeSeparators(bing_folder + "/" + bing_name + "必应壁纸.jpg");
    }
    else
    {
        qout << "使用CopyRight名称";
        bing_name = temp->find("copyright")->getValueString();
        bing_name = bing_name.mid(0, bing_name.indexOf(" (© "));
        bing_name = QDir::toNativeSeparators(bing_folder + "/" + bing_name + ".jpg");
    }
    qout << "当前索引: " << file_data->find("current")->getValueInt();
    file_data->toFile("BingData.json", YJson::UTF8BOM, true);
    if (!QFile::exists(bing_name))
    {
        qout << "下载图片";
        m_doing = true;
        auto mgr = new QNetworkAccessManager;
        qout << "检查一下";
        connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep) {
            if (rep->error() != QNetworkReply::NoError)
            {
                mgr->deleteLater();
                m_doing = false;
                return;
            }
            qout << "没有错误!";
            QFile file(bing_name);
            if (file.open(QIODevice::WriteOnly))
            {
                file.write(rep->readAll());
                file.close();
                if (!file.size())
                {
                    qout << "文件大小为0";
                    QFile::remove(bing_name);
                }
                qout << file.size();
            }
            mgr->deleteLater();
            m_doing = false;
            if (PaperType == Type::Bing)
                set_wallpaper(bing_name);
        });
    }
    else
    {
        if (PaperType == Type::Bing) set_wallpaper(bing_name);
    }
}

void Wallpaper::push_back()
{
    YJson& json = *new YJson("WallpaperApi.json", YJson::UTF8);
    const char* curApi = nullptr;
    QDir dir;
    int index = static_cast<int>(PaperType);
    qout << "种类: " << index;
    bing_api = QString::fromStdString(json["BingApi"]["Parameter"].urlEncode(json["MainApis"]["BingApi"].getValueString()));
    bing_folder = json["BingApi"]["Folder"].getValueString();
    qout << "必应Api" << bing_api;
    switch (PaperType)
    {
    case Type::Bing:
        delete &json;
        set_from_Bing();
        return;
    case Type::Other:
        curApi = json["OtherApi"]["Curruent"].getValueString();
        url = json["OtherApi"]["ApiData"][curApi]["Url"].getValueString();
        image_path = json["OtherApi"]["ApiData"][curApi]["Folder"].getValueString();
        image_name = json["OtherApi"]["ApiData"][curApi]["Name"].getValueString();
        delete &json;
        if (dir.exists(image_path) || dir.mkdir(image_path))
            set_from_Other();
        else
            emit msgBox("壁纸存放文件夹不存在, 请手动创建!", "出错");
        return;
    case Type::Advance:
        delete &json;
        set_from_Advance();
        return;
    case Type::Native:
        delete &json;
        set_from_Native();
        return;
    case Type::User:
        curApi = json["User"]["Curruent"].getValueString();
        url = json["User"]["ApiData"][curApi]["Parameter"].urlEncode(json["MainApis"]["WallhavenApi"].getValueString());
        image_path = json["User"]["ApiData"][curApi]["Folder"].getValueString();
        break;
    default:
        url = json["Default"]["ApiData"][index]["Parameter"].urlEncode(json["MainApis"]["WallhavenApi"].getValueString());
        image_path = json["Default"]["ApiData"][index]["Folder"].getValueString();
    }
    delete &json;
    if (dir.exists(image_path) || dir.mkdir(image_path))
        set_from_Wallhaven();
    else
        return emit msgBox("壁纸存放文件夹不存在, 请手动创建!", "出错");
    qout << "Api选择" << url.c_str() << image_path;
}

void Wallpaper::set_from_Wallhaven()  // 从数据库中随机抽取一个链接地址进行设置。
{
    qout << "Wallhaven 开始检查json文件";
    constexpr char file_name[] = "ImgData.json";
    qout << "文件路径" << file_name;
    std::string pic_url;
    YJson* jsonObject = nullptr, *jsonArray = nullptr, * find_item = nullptr;
    if (!QFile::exists(file_name)) goto label_1;
    qout << "读取ImageData.json文件";
    jsonObject = new YJson(file_name, YJson::AUTO);
    if (YJson::ep.first){
        qout << "ImageData文件出现错误!";
        goto label_1;
    }
    if (jsonObject->getType() == YJson::Object &&
        jsonObject->find("Api") &&
        (find_item = jsonObject->find("PageNum")) &&
        find_item->getType() == YJson::Number &&
        find_item->getValueInt() == PageNum &&
        (jsonArray = jsonObject->find("ImgUrls")) &&
        jsonArray->getType() == YJson::Object)
    {
        if (update || url != jsonObject->find("Api")->getValueString())
        {
            qout << "找到json文件, 需要更新！";
            update = false;
            jsonObject->find("Api")->setText(url);
            jsonArray->find("Used")->clear();
            jsonArray->find("Unused")->clear();
            return get_url_from_Wallhaven(jsonArray);
        }
        else
        {
            if (jsonArray->empty())
            {
                qout << "找到json文件但是没有孩子！";
                jsonArray->append(YJson::Array, "Used");
                jsonArray->append(YJson::Array, "Unused");
                jsonArray->append(YJson::Array, "Balcklist");
                return get_url_from_Wallhaven(jsonArray);
            }
            else if (jsonArray->find("Unused")->empty())
            {
                //qout << "Unused为空.";
                if (jsonArray->find("Used")->empty())
                {
                    //qout << "Used为空.";
                    return get_url_from_Wallhaven(jsonArray);
                }
                else
                {
                    qout << "Unused正常.";
                    jsonArray = jsonArray->find("Used");
                }
            }
            else
            {
                jsonArray = jsonArray->find("Unused");
            }
            return _set_w(jsonArray);
        }
        //qout << "找到Json文件和孩子!";
    }
    qout << "json文件格式不正确！";
    qout << "Wallhaven 创建新的Json对象";
label_1:
    delete jsonObject;
    jsonObject = new YJson(YJson::Object);
    jsonObject->append(Wallpaper::url, "Api");
    jsonObject->append(PageNum, "PageNum");
    jsonArray = jsonObject->append(YJson::Object, "ImgUrls");
    jsonArray->append(YJson::Array, "Used");
    jsonArray->append(YJson::Array, "Blacklist");
    jsonArray->append(YJson::Array, "Unused");
    //qout << "Wallhaven 尝试从wallhaven下载源码";
    return get_url_from_Wallhaven(jsonArray);
}

void Wallpaper::get_url_from_Wallhaven(YJson* jsonArray)
{
    static int k;
    qout << "开始从wallhaven获取链接.";
    YJson* urllist =  jsonArray->find("Unused"), *blacklist = jsonArray->find("Blacklist");
    m_doing = true;
    auto mgr = new QNetworkAccessManager;
    k = int(5 * (PageNum - 1) + 1);
    connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep){
        if (rep->error() != QNetworkReply::NoError)
        {
            delete jsonArray->getTop();
            mgr->deleteLater();
            m_doing = false;
            return;
        }
        qout << "一轮壁纸链接请求结束";
        if (k <= 5 * PageNum)
        {
            YJson *js = new YJson(rep->readAll());
            if (js->find("error"))  //{"error":"Not Found"}
            {
                delete js;
                mgr->deleteLater();
                m_doing = false;
                emit msgBox("该页面范围下没有壁纸, 请更换壁纸类型或者页面位置!", "出错");
                return ;
            }
            YJson* ptr = js->find("data")->getChild();
            const char* wn = nullptr;
            if (ptr)
            {
                do {
                    wn = ptr->find("path")->getValueString() + 31;
                    //qout << "后缀名:" << wn;
                    if (blacklist->findByVal(wn))
                    {
                        //qout << "在黑名单内";
                        continue;
                    }
                    urllist->append(wn);
                } while ((ptr = ptr->getNext()));
                QString str = QString::fromStdString(url + "&page=" + std::to_string(++k));
                qout << "post request" << str << bool(mgr);
                mgr->get(QNetworkRequest(QUrl(str)));
            }
            else
            {
                mgr->deleteLater();
                m_doing = false;
                if (urllist->getChild())
                    _set_w(urllist);
                else
                {
                    delete jsonArray->getTop();
                    emit msgBox("该页面范围下没有壁纸, 请更换壁纸类型或者页面位置!", "出错");
                }
            }
            delete js;
        }
        else {
            if (urllist->getChild())
            {
                mgr->deleteLater();
                m_doing = false;
                _set_w(urllist);
            }
            else
            {
                delete jsonArray->getTop();
                mgr->deleteLater();
                m_doing = false;
                emit msgBox("该页面范围下没有壁纸, 请更换壁纸类型或者页面位置!", "出错");
            }
        }
    });
    mgr->get(QNetworkRequest(QUrl((url + "&page=" + std::to_string(k)).c_str())));
}

void Wallpaper::get_url_from_Bing()
{
    qout << "获取必应链接";
    m_doing = true;
    auto mgr = new QNetworkAccessManager;
    QEventLoop* loop = new QEventLoop;
    connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep){
        if (rep->error() != QNetworkReply::NoError)
        {
            mgr->deleteLater();
            m_doing = false;
            return;
        }
        qout << "必应请求完成";
        YJson bing_data(rep->readAll());
        qout << rep->readAll();
        bing_data.append(0, "current");
        bing_data.append(QDateTime::currentDateTime().toString("yyyyMMdd").toStdString(), "today");
        mgr->deleteLater();
        m_doing = false;
        _set_b(&bing_data);
        loop->quit();
        loop->deleteLater();
    });
    mgr->get(QNetworkRequest(bing_api));
    loop->exec();
}

void Wallpaper::set_from_Native()
{
    QDir dir(NativeDir);
    if (!dir.exists()) return;
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.wbep";
    dir.setFilter(QDir::Files | QDir::NoSymLinks);                                //设置类型过滤器，只为文件格式
    int dir_count = dir.count();
    if (!dir_count) return;
    std::uniform_int_distribution<int> dis(0, dir_count - 1);
    QString file_name = dir[dis(_gen)];       //随机生成文件名称。
    file_name = QDir::toNativeSeparators(NativeDir + "/" + file_name);

    if (m_doing)
        return emit msgBox("当前正忙, 请稍后再试.", "提示");
    m_doing = true;
    auto thrd = QThread::create([=](){
        QFile f(file_name);
        char buffer;
        if (f.open(QIODevice::ReadOnly) && f.size() && f.read(&buffer, 1))
        {
            set_wallpaper(file_name);
        }
        else
        {
            emit setFailed("本地文件无效，请更换本地文件夹、改变壁纸类型或取消自动更换壁纸！");
        }
        return;
    });
    connect(thrd, &QThread::finished, this, [=](){
        thrd->deleteLater();
        m_doing = false;
    });
    thrd->start();

};


void Wallpaper::set_from_Bing()
{
    YJson *file_data = nullptr;
    if (!QFile::exists("BingData.json"))
        return get_url_from_Bing();
    qout << "必应文件存在";
    file_data = new YJson("BingData.json", YJson::AUTO);
    qout << "加载文件完成";
    if (QDateTime::currentDateTime().toString("yyyyMMdd") != file_data->find("today")->getValueString())
    {
        qout << QDateTime::currentDateTime().toString("yyyyMMdd").length() << strlen(file_data->find("today")->getValueString());

        qout << "必应文件过期，将开始下载最新数据";
        delete file_data;
        return get_url_from_Bing();
    }
    qout << "必应文件为最新";
    _set_b(file_data);
    delete file_data;
}

void Wallpaper::set_from_Other()
{
    m_doing = true;
    auto mgr = new QNetworkAccessManager;
    connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep){
        if (rep->error() != QNetworkReply::NoError)
        {
            mgr->deleteLater();
            m_doing = false;
            return;
        }
        QString path =  QDir::toNativeSeparators(image_path + QDateTime::currentDateTime().toString("/" + image_name));
        qout << "其它壁纸: " << path << url.c_str();
        QFile file(path);
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(rep->readAll());
            file.close();
            if (!file.size())
            {
                qout << "文件大小为0";
                QFile::remove(path);
                return;
            }
            qout << file.size();
        }
        set_wallpaper(path);
        mgr->deleteLater();
        m_doing = false;
    });
    mgr->get(QNetworkRequest(QUrl(url.c_str())));
}

QStringList _parse_arguments(const QString& str)
{
    QStringList lst;
    QChar d(' ');
    QString::const_iterator iter1 = str.constBegin(), iter=iter1;
    do {
        iter1 = std::find_if(iter, str.constEnd(), [](const QChar& c)->bool{ return c != QChar(' ');});
        if (iter == str.constEnd()) break;
        if (*iter1 == QChar('\"'))
        {
            d = '\"';
            ++iter1;
        }
        else
        {
            d = ' ';
        }
        iter = std::find(iter1, str.constEnd(), d);
        lst.push_back(QString(iter1, iter-iter1));
    } while (iter++ != str.constEnd());
    return lst;
}

void Wallpaper::set_from_Advance()
{
    if (UserCommand.isEmpty()) return;
    qout << "高级命令开始" << UserCommand;
    static std::string program_output;
    m_doing = true;
    auto thrd = QThread::create([=](){
        QStringList&& lst = _parse_arguments(UserCommand);
        QString program_file = lst[0];
        lst.removeFirst();
        if (applyClicked)
        {
            applyClicked = false;
            lst << "0";
        }
        else
        {
            lst << "1";
        }
        qout << "程序: " <<  program_file << "; 参数: " << lst;
        program_output = static_cast<const char*>(VarBox->runCmd(program_file, lst, 1));
        qout << "运行输出ansi: " << program_output.c_str();
    });
    connect(thrd, &QThread::finished, this, [=](){
        qout << "线程结束!" ;
        if (program_output.size())
        {
            for (auto i = program_output.size()-1; i > 0; --i)
            {
                if (strchr("\"\\\b\f\n\r\t\'", program_output[i]))
                {
                    program_output[i] = 0;
                }
                else
                {
                    qout << "开始设置壁纸";
                    FILE* f = fopen(program_output.c_str(), "rb");
                    if (program_output.size() && f)
                    {
                        fclose(f);
                        qout << "设置壁纸" ;
                        systemParametersInfo(program_output);
                        qout << "添加壁纸记录" ;
#ifdef Q_OS_WIN32
                        PicHistory.emplace_back(AnsiToUtf8(program_output));
#elif defined (Q_OS_LINUX)
                        PicHistory.emplace_back(program_output);
#endif
                        qout << "当前壁纸后移" ;
                        CurPic = --PicHistory.end();
                        qout << "删除输出";
                        program_output.clear();
                        qout  << "清理现场";
                        thrd->deleteLater();
                        m_doing = false;
                        return ;
                    }
                    break;
                }
            }
        }
        thrd->deleteLater();
        m_doing = false;
    });
    thrd->start();
}

void Wallpaper::next()
{
    if (m_doing) return emit msgBox("频繁点击是没有效的哦！", "提示");
    qout << "下一张图片";
    if (CurPic != PicHistory.end() && ++CurPic != PicHistory.end())
    {
        FILE* file = readFile(*CurPic);
        if (!file) return;
        m_doing = true;
        auto thrd = QThread::create([=](){
            char buffer;
            if (file && fread(&buffer, sizeof(char), 1, file) != 0) {
                systemParametersInfo(CurPic->c_str());
            } else {
                emit msgBox("没有网络！", "提示");
            }
            return;
        });
        connect(thrd, &QThread::finished, this, [=]() {
            fclose(file);
            thrd->deleteLater();
            m_doing = false;
        });
        thrd->start();
        return;
    }
    qout << "壁纸类型：" << static_cast<int>(PaperType);
    push_back();
}

void Wallpaper::prev()
{
    if (m_doing) return emit msgBox("和后台壁纸切换冲突，请稍后再试。", "提示");
    m_doing = true;
    auto thrd = QThread::create([this](){
        for (int i=0; i<100; ++i)
        {
            if (CurPic == PicHistory.begin())
            {
                emit msgBox("无法找到更早的壁纸历史记录！", "提示");
                return ;
            }
            FILE* file = readFile(*--CurPic);
            if (file)
            {
                char buffer;
                if (!fread(&buffer, sizeof(char), 1, file))
                {
                    fclose(file);
                    return ;
                }
                fclose(file);
                systemParametersInfo(CurPic->c_str());
                return;
            }
            CurPic = PicHistory.erase(CurPic);
            if (CurPic != PicHistory.begin())
                --CurPic;
        }
    });
    connect(thrd, &QThread::finished, this, [=](){
        thrd->deleteLater();
        m_doing = false;
    });
    thrd->start();
}

void Wallpaper::dislike()
{
    qout << "不喜欢该壁纸。";
    const char* pic_path = CurPic->c_str();
    const char* pic_name = get_file_name(pic_path);
    char id[21] = { 0 };
    check_is_wallhaven(pic_name, id);
    if (*id)
    {
        YJson *json = new YJson("ImgData.json", YJson::AUTO);
        YJson *blacklist = json->find("ImgUrls")->find("Blacklist");
        if (!blacklist->findByVal(id))
            blacklist->append(id);
        YJson *black_id;
        if (black_id = json->find("ImgUrls")->find("Used")->findByVal(id), black_id)
            YJson::remove(black_id);
        if (black_id = json->find("ImgUrls")->find("Unused")->findByVal(id), black_id)
            YJson::remove(black_id);
        json->toFile("ImgData.json", YJson::UTF8BOM, true);
        delete  json;
    }
    if (!remove(pic_path))
        emit msgBox("删除文件失败!", "出错");
    CurPic = PicHistory.erase(CurPic);
    if (CurPic != PicHistory.end())
    {
        if (m_doing)
            return emit msgBox("和后台壁纸切换冲突，请稍后再试。", "提示");
        FILE * file = fopen(CurPic->c_str(), "rb");
        if (!file) {
            CurPic = PicHistory.erase(CurPic);
            return push_back();
        }
        m_doing = true;
        auto thrd = QThread::create([=](){
            char buffer;
            if (!fread(&buffer, sizeof(char), 1, file))
            {
                return ;
            }
            systemParametersInfo(CurPic->c_str());
        });
        connect(thrd, &QThread::finished, this, [=](){
            fclose(file);
            thrd->deleteLater();
            m_doing = false;
        });
        thrd->start();
        return;
    }
    push_back();
}

bool Wallpaper::isOnline(bool wait)
{
    qout << "检查网络链接";
    QNetworkAccessManager *mgr = new QNetworkAccessManager;
    bool success = false;
    QEventLoop loop;
    QNetworkRequest request(QUrl("https://www.baidu.com"));
    request.setRawHeader(                 //设置请求头
                "User-Agent",
                "Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko"
    );
    connect(mgr, &QNetworkAccessManager::finished, &loop, [&loop, &success](QNetworkReply *re){
        success = re->error() == QNetworkReply::NoError;
        qout << "网络错误情况：" << success;
        loop.quit();
    });

    for (int c=1; wait && (c<=(wait?30:1)); c++)
    {
        qout << "发送检测请求";
        mgr->get(request);
        loop.exec();
        if (success) {
            delete mgr;
            return true;
        } else if (wait)
            QThread::sleep(3);
    }
    delete mgr;
    return false;
}
