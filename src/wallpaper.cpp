#include <fstream>
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

#include "funcbox.h"
#include "wallpaper.h"
#include "YEncode.h"
#include "YString.h"
#include "YJson.h"

const wchar_t* get_file_name(const wchar_t* file_path)
{
    const wchar_t* ptr = file_path;
    while (*++ptr);
    while (*--ptr != '\\');
    return ++ptr;
}

void check_is_wallhaven(const wchar_t* pic, char* id)
{
    if (wcslen(pic) != 20)
        return ;
    if (!wcsncmp(pic, L"wallhaven-", 10) && StrContainCharInRanges<wchar_t>(pic+10, 6, L"a-z", L"0-9") &&
            (!wcscmp(pic+16, L".png") || !wcscmp(pic+16, L".jpg")))
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
        qout << "设置壁纸：" << file_path;
        wchar_t* temp = new wchar_t[MAX_PATH] {0};
        file_path.toWCharArray(temp);
        PicHistory.emplace_back(temp);
        CurPic = --PicHistory.end();
        return SystemParametersInfo(temp);
    }
    else
        return false;
}

Wallpaper::Wallpaper():
    _rd(), _gen(_rd()),
    SystemParametersInfo(
        std::bind(
            SystemParametersInfoW,
            SPI_SETDESKWALLPAPER,
            UINT(0),
            std::placeholders::_1,
            SPIF_SENDCHANGE | SPIF_UPDATEINIFILE
            )
        ),
    thrd(nullptr), mgr(nullptr), applyClicked(false),
    update(false), url(), bing_api(), bing_folder(), image_path(),  image_name(), timer(new QTimer)
{
    connect(this, &Wallpaper::msgBox, VarBox, std::bind(VARBOX::MSG, std::placeholders::_1, std::placeholders::_2, QMessageBox::Ok));
    QSettings set("HKEY_CURRENT_USER\\Control Panel\\Desktop", QSettings::NativeFormat);
    if (set.contains("WallPaper"))
    {
        QString temp_paper = set.value("WallPaper").toString();
        if (!temp_paper.isEmpty())
        {
            qout << "首张壁纸长度: " << temp_paper.length();
            auto s = reinterpret_cast<const wchar_t*>(temp_paper.utf16());
            auto l = wcslen(s) + 1;
            PicHistory.emplace_back(new wchar_t[l]);
            std::copy(s, s+l, PicHistory.back());
            std::wcout << L"注册表壁纸记录: " << PicHistory.back();
        }
    }
    CurPic = PicHistory.begin();
    timer->setInterval(VarBox->TimeInterval * 60000);
    if (VarBox->FirstChange){
        qout << "自动换";
        if (!VarBox->InternetGetConnectedState())
        {
            thrd = QThread::create([=](){
                for (int i = 0; i < 300; ++i)
                {
                    Sleep(200);
                    if (VarBox->InternetGetConnectedState())
                    {
                        push_back();
                        break;
                    }
                }
            });
            connect(thrd, &QThread::finished, this, [this](){
                thrd->deleteLater();
                thrd = nullptr;
            });
            thrd->start();
        }
        else
            push_back();
       static const char _ = VarBox->AutoSaveBingPicture && (set_from_Bing(), 1);
       (void)_;
    }
    else qout << "不自动换";
    if (VarBox->AutoChange) timer->start();
    connect(timer, &QTimer::timeout, this, &Wallpaper::next);
}

Wallpaper::~Wallpaper()
{
    delete mgr;
    if (thrd)
    {
        thrd->deleteLater();
        qout << "等待线程析构中";
        thrd->wait();
    }
    delete timer;
    for (auto c: PicHistory)
        delete [] c;
}

void Wallpaper::_set_w(YJson* jsonArray)
{
    qout << "智能设置壁纸开始.";

    if (jsonArray->getChild())
    {
        int pic_num = jsonArray->getChildNum();
        qout << "Wallhaven 找到随机id";
        std::uniform_int_distribution<int> dis(0, pic_num);
        YJson* item = jsonArray->find(dis(_gen));
        std::string pic_url = item->getValueString();
        jsonArray->getParent()->find("Used")->append(*item);
        jsonArray->remove(item);
        if (pic_url.length())
        {
            qout << "壁纸网址：" << pic_url.c_str();
            const QString&& img = image_path + "\\" + pic_url.c_str();
            if (!QFile::exists(img))
            {
                qout << "本地找不到文件, 直接开始下载";
                mgr = new QNetworkAccessManager;
                mgr->setTransferTimeout(30000);
                connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep)->void{
                    if (rep->error() != QNetworkReply::NoError)
                    {
                        mgr->deleteLater();
                        mgr = nullptr;
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
                    mgr = nullptr;
                    set_wallpaper(img);
                });
                mgr->get(QNetworkRequest(QUrl(("https://w.wallhaven.cc/full/"+pic_url.substr(10, 2)+"/"+pic_url).c_str())));
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
    if (VarBox->UseDateAsBingName)
    {
        qout << "使用日期名称";
        bing_name = temp->find("enddate")->getValueString();
        bing_name.insert(4, '-');
        bing_name.insert(7, '-');
        bing_name = bing_folder + "\\" + bing_name + "必应壁纸.jpg";
    }
    else
    {
        qout << "使用CopyRight名称";
        bing_name = temp->find("copyright")->getValueString();
        bing_name = bing_name.mid(0, bing_name.indexOf(" (© "));
        bing_name = bing_folder + "\\" + bing_name + ".jpg";
    }
    qout << "当前索引: " << file_data->find("current")->getValueInt();
    file_data->toFile("BingData.json", YJson::UTF8BOM, true);
    if (!QFile::exists(bing_name))
    {
        qout << "下载图片";
        mgr = new QNetworkAccessManager;
        qout << "检查一下";
        connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep)->void{
            if (rep->error() != QNetworkReply::NoError)
            {
                mgr->deleteLater();
                mgr = nullptr;
                return;
            }
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
            mgr = nullptr;
            if (VarBox->PaperType == PAPER_TYPE::Bing) set_wallpaper(bing_name);
        });
        mgr->setTransferTimeout(30000);
        connect(mgr->get(QNetworkRequest(QUrl(img_url.c_str()))), &QNetworkReply::errorOccurred, this, [this](){
            mgr->deleteLater();
            mgr = nullptr;
        });
    }
    else
    {
        if (VarBox->PaperType == PAPER_TYPE::Bing) set_wallpaper(bing_name);
    }
}

void Wallpaper::push_back()
{
    YJson& json = *new YJson("WallpaperApi.json", YJson::UTF8);
    const char* curApi = nullptr;
    QDir dir;
    int index = (int)VarBox->PaperType;
    qout << "种类: " << index;
    bing_api = json["BingApi"]["Parameter"].urlEncode(json["MainApis"]["BingApi"].getValueString()).c_str();
    bing_folder = json["BingApi"]["Folder"].getValueString();
    qout << "必应Api" << bing_api;
    switch (VarBox->PaperType)
    {
    case PAPER_TYPE::Bing:
        delete &json;
        set_from_Bing();
        return;
    case PAPER_TYPE::Other:
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
    case PAPER_TYPE::Advance:
        delete &json;
        set_from_Advance();
        return;
    case PAPER_TYPE::Native:
        delete &json;
        set_from_Native();
        return;
    case PAPER_TYPE::User:
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
        find_item->getValueInt() == VarBox->PageNum &&
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
    jsonObject->append(Wallpaper::url.c_str(), "Api");
    jsonObject->append(VarBox->PageNum, "PageNum");
    jsonArray = jsonObject->append(YJson::Object, "ImgUrls");
    jsonArray->append(YJson::Array, "Used");
    jsonArray->append(YJson::Array, "Blacklist");
    jsonArray->append(YJson::Array, "Unused");
    //qout << "Wallhaven 尝试从wallhaven下载源码";
    return get_url_from_Wallhaven(jsonArray);
}

void Wallpaper::get_url_from_Wallhaven(YJson* jsonArray)
{
    qout << "开始从wallhaven获取链接.";
    YJson* urllist =  jsonArray->find("Unused"), *blacklist = jsonArray->find("Blacklist");
    mgr = new QNetworkAccessManager;
    int *k = new int(5 * (VarBox->PageNum - 1) + 1);
    connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep){
        if (rep->error() != QNetworkReply::NoError)
        {
            delete k;
            delete jsonArray->getTop();
            mgr->deleteLater();
            mgr = nullptr;
            return;
        }
        qout << "一轮壁纸链接请求结束";
        if (*k <= 5 * VarBox->PageNum)
        {
            YJson *js = new YJson(rep->readAll());
            if (js->find("error"))  //{"error":"Not Found"}
            {
                delete js;
                delete k;
                mgr->deleteLater();
                mgr = nullptr;
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
                } while (ptr = ptr->getNext());
                qout << "发送请求";
                mgr->get(QNetworkRequest(QUrl((url + "&page=" + std::to_string(++*k)).c_str())));
            }
            else
            {
                delete k;
                mgr->deleteLater();
                mgr = nullptr;
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
            delete k;
            if (urllist->getChild())
            {
                mgr->deleteLater();
                mgr = nullptr;
                _set_w(urllist);
            }
            else
            {
                delete jsonArray->getTop();
                mgr->deleteLater();
                mgr = nullptr;
                emit msgBox("该页面范围下没有壁纸, 请更换壁纸类型或者页面位置!", "出错");
            }
        }
    });
    mgr->setTransferTimeout(30000);
    mgr->get(QNetworkRequest(QUrl((url + "&page=" + std::to_string(*k)).c_str())));
}

void Wallpaper::get_url_from_Bing()
{
    qout << "获取必应链接";
    mgr = new QNetworkAccessManager;
    connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep){
        if (rep->error() != QNetworkReply::NoError)
        {
            mgr->deleteLater();
            mgr = nullptr;
            return;
        }
        qout << "必应请求完成";
        YJson bing_data(rep->readAll());
        qout << rep->readAll();
        bing_data.append(0, "current");
        bing_data.append(QDateTime::currentDateTime().toString("yyyyMMdd").toStdString(), "today");
        mgr->deleteLater();
        mgr = nullptr;
        _set_b(&bing_data);
    });
    mgr->setTransferTimeout(30000);
    mgr->get(QNetworkRequest(bing_api));
}

void Wallpaper::set_from_Native()
{
    QDir dir(VarBox->NativeDir);
    if (!dir.exists()) return;
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.wbep";
    dir.setFilter(QDir::Files | QDir::NoSymLinks);                                //设置类型过滤器，只为文件格式
    int dir_count = dir.count();
    if (!dir_count) return;
    std::uniform_int_distribution<int> dis(0, dir_count);
    QString file_name = dir[dis(_gen)];       //随机生成文件名称。
    file_name = VarBox->NativeDir + "\\" + file_name;

    if (VarBox->GetFileAttributes(reinterpret_cast<const wchar_t*>(file_name.utf16())))
    {
        if (VarBox->InternetGetConnectedState())
        {
            if (thrd) return emit msgBox("当前正忙, 请稍后再试.", "提示");
            thrd = QThread::create([=](){
                if (!VarBox->OneDriveFile(file_name.toStdWString().c_str()))
                {
                    emit setFailed("本地文件无效，请更换本地文件夹、改变壁纸类型或取消自动更换壁纸！");
                    return;
                }
                else
                    set_wallpaper(file_name);
            });
            connect(thrd, &QThread::finished, this, [this](){
                thrd->deleteLater();
                thrd = nullptr;
            });
            thrd->start();
            return;
        }
        else
        {
            emit msgBox("没有网络！", "提示");
            return;
        }
    }
    set_wallpaper(file_name);
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
    return void();
}

void Wallpaper::set_from_Other()
{
    mgr = new QNetworkAccessManager;
    connect(mgr, &QNetworkAccessManager::finished, this, [this](QNetworkReply* rep){
        if (rep->error() != QNetworkReply::NoError)
        {
            mgr->deleteLater();
            mgr = nullptr;
            return;
        }
        QString path =  image_path + QDateTime::currentDateTime().toString("\\" + image_name);
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
        mgr = nullptr;
    });
    mgr->setTransferTimeout(30000);
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
#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
        lst.push_back(QString(iter1, iter-iter1));
#else
        lst.emplace_back(iter1, iter-iter1);
#endif
    } while (iter++ != str.constEnd());
    return lst;
}

void Wallpaper::set_from_Advance()
{
    if (VarBox->UserCommand.isEmpty()) return;
    qout << "高级命令开始" << VarBox->UserCommand;
    wchar_t** program_output = new wchar_t*(nullptr);
    thrd = QThread::create([=](){
        QStringList&& lst = _parse_arguments(VarBox->UserCommand);
        QString program_file = lst[0]; lst.removeFirst();
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
        *program_output = VarBox->runCmd(program_file, lst, 1);
        qout << "运行输出" << std::wstring(*program_output);
    });
    connect(thrd, &QThread::finished, this, [=](){
        qout << "线程结束!" ;
        if (*program_output)
        {
            for (auto i = wcslen(*program_output)-1; i > 0; --i)
            {
                if (wcschr(L"\"\\\b\f\n\r\t\'", program_output[0][i]))
                {
                    program_output[0][i] = 0;
                }
                else
                {
                    qout << "开始设置壁纸";
                    if (strlen(*program_output) && VarBox->PathFileExists(*program_output))
                    {
                        qout << "设置壁纸" ;
                        SystemParametersInfo(*program_output);
                        qout << "添加壁纸记录" ;
                        PicHistory.emplace_back(*program_output);
                        qout << "当前壁纸后移" ;
                        CurPic = --PicHistory.end();
                        qout << "删除输出";
                        delete program_output;
                                qout  << "清理现场";
                        thrd->deleteLater();
                        thrd = nullptr;
                        return ;
                    }
                    break;
                }
            }
        }
        qout << "清除输出";
        delete [] *program_output;
        qout << "清除一级指针内存";
        delete program_output;
        thrd->deleteLater();
        thrd = nullptr;
    });
    thrd->start();
}

void Wallpaper::next()
{
    if (thrd || mgr) return emit msgBox("频繁点击是没有效的哦！", "提示");
    qout << "下一张图片";
    if (CurPic != PicHistory.end() && ++CurPic != PicHistory.end())
    {
        thrd = QThread::create([this](){
            if (VarBox->PathFileExists(*CurPic))
            {
                if (VarBox->GetFileAttributes(*CurPic))
                {
                    if (VarBox->InternetGetConnectedState())
                    {
                        if (!VarBox->OneDriveFile(*CurPic))
                        {
                            return;
                        }
                    }
                    else
                    {
                        emit msgBox("没有网络！", "提示");
                        return;
                    }
                }
                SystemParametersInfo(*CurPic);
            }
        });
        connect(thrd, &QThread::finished, this, [this](){
            thrd->deleteLater();
            thrd = nullptr;
        });
        thrd->start();
        return;
    }
    qout << "壁纸类型：" << static_cast<int>(VarBox->PaperType);
    push_back();
}

void Wallpaper::prev()
{
    if (thrd) return emit msgBox("和后台壁纸切换冲突，请稍后再试。", "提示");
    thrd = QThread::create([this](){
        for (int i=0; i<100; ++i)
        {
            if (CurPic == PicHistory.begin())
            {
                emit msgBox("无法找到更早的壁纸历史记录！", "提示");
                return ;
            }
            if (VarBox->PathFileExists(*--CurPic))
            {
                if (VarBox->GetFileAttributes(*CurPic))
                {
                    if (VarBox->InternetGetConnectedState())
                    {
                        if (!VarBox->OneDriveFile(*CurPic))
                        {
                            return;
                        }
                    }
                    else
                    {
                        emit msgBox("没有网络！", "提示");
                        return;
                    }
                }
                SystemParametersInfo(*CurPic);
                return;
            }
            delete [] *CurPic;
            CurPic = PicHistory.erase(CurPic);
            if (CurPic != PicHistory.begin())
                --CurPic;
        }
    });
    connect(thrd, &QThread::finished, this, [this](){
        thrd->deleteLater(); thrd = nullptr;
    });
    thrd->start();
}

void Wallpaper::dislike()
{
    qout << "不喜欢该壁纸。";
    const wchar_t* pic_path = *CurPic;
    const wchar_t* pic_name = get_file_name(pic_path);
    char id[21] = { 0 };
    check_is_wallhaven(pic_name, id);
    if (*id)
    {
        YJson *json = new YJson("ImgData.json", YJson::AUTO);
        YJson *blacklist = json->find("ImgUrls")->find("Blacklist");
        if (!blacklist->findByVal(id))
            blacklist->append(id);
        YJson *black_id;
        if (black_id = json->find("ImgUrls")->find("Used")->findByVal(id))
            YJson::remove(black_id);
        if (black_id = json->find("ImgUrls")->find("Unused")->findByVal(id))
            YJson::remove(black_id);
        json->toFile("ImgData.json", YJson::UTF8BOM, true);
        delete  json;
    }
    if (!DeleteFileW(pic_path))
        emit msgBox("删除文件失败!", "出错");
    delete [] pic_path;
    CurPic = PicHistory.erase(CurPic);
    if (CurPic != PicHistory.end())
    {
        if (thrd) return emit msgBox("和后台壁纸切换冲突，请稍后再试。", "提示");
        if (!VarBox->PathFileExists(*CurPic))
        {
            delete [] *CurPic;
            CurPic = PicHistory.erase(CurPic);
            return push_back();
        }
        thrd = QThread::create([this](){
            if (VarBox->GetFileAttributes(*CurPic))
            {
                if (VarBox->InternetGetConnectedState())
                {
                    if (!VarBox->OneDriveFile(*CurPic))
                    {
                        return;
                    }
                }
                else
                {
                    emit msgBox("没有网络！", "提示");
                    return;
                }
            }
            SystemParametersInfo(*CurPic);
        });
        connect(thrd, &QThread::finished, this, [this](){
            thrd->deleteLater();
            thrd = nullptr;
        });
        thrd->start();
        return;
    }
    push_back();
}

void Wallpaper::kill()
{
    if (mgr) {
        mgr->deleteLater();
        mgr = nullptr;
    }
    if (thrd) {
        thrd->deleteLater();
        thrd = nullptr;
    }
}
