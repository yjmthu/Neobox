#include <fstream>
#include <QDateTime>
#include <QRandomGenerator>
#include <QDir>
#include <QFile>
#include <QNetworkInformation>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

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
    return StrJoin<wchar_t>(++ptr);
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

inline bool setWallpaper(const QString &file_path)             //根据路径设置壁纸
{
    if (QFile::exists(file_path))
    {
        qout << "设置壁纸：" << file_path;
        wchar_t* temp = new wchar_t[MAX_PATH] {0};
        file_path.toWCharArray(temp);
        VarBox->PicHistory.emplace_back(temp);
        VarBox->CurPic = --VarBox->PicHistory.end();
        return VarBox->SystemParametersInfo(temp);
    }
    else
        return false;
}

Wallpaper::Wallpaper():
    thrd(nullptr), mgr(nullptr), applyClicked(false),
    update(false), url(), bing_api(), bing_folder(), image_path(),  image_name()
{
    //connect(this, SIGNAL(finished()), this, SLOT(clean()));
    srand((unsigned)time(0));           // 防止出现重复
}

Wallpaper::~Wallpaper()
{
    delete mgr;
    delete thrd;
}

void Wallpaper::kill()
{
    if (mgr)
    {
        mgr->deleteLater(); mgr = nullptr;
    }
    if (thrd)
    {
        thrd->deleteLater(); thrd = nullptr;
    }
}

void Wallpaper::_set_w(YJson* jsonArray)
{
    YJson * blacklist = jsonArray->getParent()->find("Blacklist");
    std::string pic_url;
    if (jsonArray->getChild())
    {
        int pic_num = jsonArray->getChildNum();
        qout << "Wallhaven 找到随机id";
        if (blacklist->getChild())
        {
            qout << "黑名不为空！";
            for (YJson *item = jsonArray->find(rand() % pic_num);pic_num; item = jsonArray->find(rand() % pic_num))
            {
                if (item->getType() == YJSON_TYPE::YJSON_STRING)
                {
                    qout << "随机id" << item->getValueString();
                    if (blacklist->findByVal(item->getValueString()))
                    {
                        qout << "在黑名单里面";
                        jsonArray->remove(item);
                        --pic_num;
                    }
                    else
                    {
                        qout << "不在黑名单里面";
                        jsonArray->getParent()->find("Used")->append(*item);
                        pic_url = item->getValueString();
                        jsonArray->remove(item);
                        break;
                    }
                }
            }
        }
        else
        {
            qout << "黑名单为空!";
            YJson* item = jsonArray->find(rand() % pic_num);
            pic_url = item->getValueString();
            jsonArray->getParent()->find("Used")->append(*item);
            jsonArray->remove(item);
        }
        if (pic_url.length())
        {
            qout << "壁纸网址：" << pic_url.c_str();
            const QString&& img = image_path + "\\" + pic_url.c_str();
            if (!QFile::exists(img))
            {
                mgr = new QNetworkAccessManager;
                connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep)->void{
                    QFile file(img);
                    if (file.open(QIODevice::WriteOnly))
                    {
                        file.write(rep->readAll());
                        file.close();
                        if (!file.size())
                        {
                            qout << "文件大小为0";
                            QFile::remove(img);
                        }
                        qout << file.size();
                    }
                    mgr->deleteLater();
                    mgr = nullptr;
                    setWallpaper(img);
                });
                mgr->get(QNetworkRequest(QUrl(("https://w.wallhaven.cc/full/"+pic_url.substr(10, 2)+"/"+pic_url).c_str())));
            }
            else
            {
                setWallpaper(img);
            }
            qout << "壁纸设置完毕";
        }
        qout << "保存 json 文件";
        jsonArray->getTop()->toFile(L"ImgData.json", YJSON_ENCODE::UTF8BOM, true);
        qout << "json 文件保存完毕";
    }
    delete jsonArray->getTop();
}

void Wallpaper::_set_b(YJson * file_data)
{
    qout << "详细处理";
    qout << "当前索引: " << file_data->find("current")->getValueInt();
    YJson* temp = file_data->find("images")->find(file_data->find("current")->getValueInt());
    if (!temp)
    {
        qout << "到达末尾";
        temp = file_data->find("images")->getChild();
        file_data->find("current")->setValue(1);
    }
    else
    {
        qout << "加一";
        file_data->find("current")->setValue(file_data->find("current")->getValueInt()+1);
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
    file_data->toFile(L"BingData.json", YJSON_ENCODE::UTF8BOM, true);
    if (!QFile::exists(bing_name))
    {
        mgr = new QNetworkAccessManager;
        connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep)->void{
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
            if (VarBox->PaperType == PAPER_TYPE::Bing) setWallpaper(bing_name);
        });
        connect(mgr->get(QNetworkRequest(QUrl(img_url.c_str()))), &QNetworkReply::errorOccurred, this, [this](){
            mgr->deleteLater();
            mgr = nullptr;
        });
    }
    else
    {
        if (VarBox->PaperType == PAPER_TYPE::Bing) setWallpaper(bing_name);
    }
}

void Wallpaper::set_from_Wallhaven()  // 从数据库中随机抽取一个链接地址进行设置。
{
    qout << "Wallhaven 开始检查json文件";
    QString file_name = "ImgData.json";
    qout << "文件路径" << file_name;
    std::string pic_url;
    YJson* jsonObject = nullptr, *jsonArray = nullptr, * find_item = nullptr;
    if (!QFile::exists(file_name)) goto label_1;
    qout << "读取ImageData.json文件";
    jsonObject = new YJson(file_name.toStdWString(), YJSON_ENCODE::AUTO);
    if (YJson::ep.first){
        qout << "ImageData文件出现错误!";
        goto label_1;
    }
    if (jsonObject->getType() == YJSON_TYPE::YJSON_OBJECT &&
        jsonObject->find("Api") &&
        (find_item = jsonObject->find("PageNum")) &&
        find_item->getType() == YJSON_TYPE::YJSON_NUMBER &&
        find_item->getValueInt() == VarBox->PageNum &&
        (jsonArray = jsonObject->find("ImgUrls")) &&
        jsonArray->getType() == YJSON_TYPE::YJSON_OBJECT)
    {
        if (Wallpaper::update || strcmp(jsonObject->find("Api")->getValueString(), Wallpaper::url.c_str()) || Wallpaper::url != jsonObject->find("Api")->getValueString())
        {
            qout << "找到json文件, 需要更新！";
            Wallpaper::update = false;
            jsonObject->find("Api")->setText(Wallpaper::url.c_str());
            jsonArray->find("Used")->clear();
            jsonArray = jsonArray->find("Unused");
            jsonArray->clear();
            return get_url_from_Wallhaven(jsonArray);
        }
        else
        {
            if (jsonArray->empty())
            {
                //qout << "找到json文件但是没有孩子！";
                jsonArray->append(YJSON::ARRAY, "Used");
                jsonArray->append(YJSON::ARRAY, "Balcklist");
                jsonArray = jsonArray->append(YJSON::ARRAY, "Unused");
                return get_url_from_Wallhaven(jsonArray);
            }
            else if (jsonArray->find("Unused")->empty())
            {
                //qout << "Unused为空.";
                if (jsonArray->find("Used")->empty())
                {
                    //qout << "Used为空.";
                    jsonArray = jsonArray->find("Unused");
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
    jsonObject = new YJson(YJSON::OBJECT);
    jsonObject->append(Wallpaper::url.c_str(), "Api");
    jsonObject->append(VarBox->PageNum, "PageNum");
    jsonArray = jsonObject->append(YJSON::OBJECT, "ImgUrls");
    jsonArray->append(YJSON::ARRAY, "Used");
    jsonArray->append(YJSON::ARRAY, "Blacklist");
    jsonArray = jsonArray->append(YJSON::ARRAY, "Unused");
    //qout << "Wallhaven 尝试从wallhaven下载源码";
    return get_url_from_Wallhaven(jsonArray);
}

void Wallpaper::get_url_from_Wallhaven(YJson* jsonArray)
{
    mgr = new QNetworkAccessManager;
    int *k = new int(5 * (VarBox->PageNum - 1) + 1);
    connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep){
        if (*k <= 5 * VarBox->PageNum)
        {
            YJson *js = new YJson(rep->readAll());
            //qout << "nn";
            YJson* ptr = js->find("data")->getChild();
            if (ptr)
                do {
                    jsonArray->append(ptr->find("path")->getValueString() + 31);
                } while (ptr = ptr->getNext());
            delete js;
            mgr->get(QNetworkRequest(QUrl((url + "&page=" + std::to_string(++*k)).c_str())));
        }
        else
        {
            delete k;
            _set_w(jsonArray);
            mgr->deleteLater();
            mgr = nullptr;
        }
    });
    mgr->get(QNetworkRequest(QUrl((url + "&page=" + std::to_string(*k)).c_str())));
}

void Wallpaper::get_url_from_Bing()
{
    mgr = new QNetworkAccessManager;
    connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep){
        YJson bing_data(rep->readAll());
        bing_data.append(0, "current");
        bing_data.append(QDateTime::currentDateTime().toString("yyyyMMdd").toStdString(), "today");
        mgr->deleteLater();
        mgr = nullptr;
        _set_b(&bing_data);
    });
    connect(mgr->get(QNetworkRequest(bing_api)), &QNetworkReply::errorOccurred, this, [=](QNetworkReply::NetworkError){
        mgr->deleteLater();
        mgr = nullptr;
    });
}

void Wallpaper::set_from_Native()
{
    QDir dir(VarBox->NativeDir);
    if (!dir.exists()) return;
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.wbep";
    dir.setFilter(QDir::Files | QDir::NoSymLinks);                                //设置类型过滤器，只为文件格式
    int dir_count = dir.count();
    if (!dir_count)
    {
        if (!VarBox->InternetGetConnectedState())
            return;
    }
    QString file_name = dir[QRandomGenerator::global()->bounded(dir_count)];  //随机生成文件名称。
    file_name = VarBox->NativeDir + "/" + file_name;

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
                    setWallpaper(file_name);
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
    setWallpaper(file_name);
};


void Wallpaper::set_from_Bing()
{
    YJson *file_data = nullptr;
    if (!QFile::exists("BingData.json"))
        return get_url_from_Bing();
    qout << "必应文件存在";
    file_data = new YJson(L"BingData.json", YJSON_ENCODE::AUTO);
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
        setWallpaper(path);
        mgr->deleteLater();
        mgr = nullptr;
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
        lst.emplace_back(iter1, iter-iter1);
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
                    qout << "置零";
                    program_output[0][i] = 0;
                }
                else
                {
                    qout << "开始设置壁纸";
                    if (strlen(*program_output) && VarBox->PathFileExists(*program_output))
                    {
                        qout << "设置壁纸" ;
                        VarBox->SystemParametersInfo(*program_output);
                        qout << "添加壁纸记录" ;
                        VarBox->PicHistory.emplace_back(*program_output);
                        qout << "当前壁纸后移" ;
                        VarBox->CurPic = --VarBox->PicHistory.end();
                        qout << "删除输出";
                        delete program_output;
                                qout  << "清理现场";
                        thrd->deleteLater();
                        thrd = nullptr;
                        return ;
                    }
                    else
                    {
                        break;
                    }
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

void Wallpaper::timer()
{
    static const char _ = VarBox->AutoSaveBingPicture && (set_from_Bing(), 1);
    PX_UNUSED(_);
    qout << "壁纸类型：" << (int)VarBox->PaperType;
    switch (VarBox->PaperType)
    {
    case PAPER_TYPE::Advance:
        set_from_Advance();
        break;
    case PAPER_TYPE::Native:
        set_from_Native();
        break;
    case PAPER_TYPE::Bing:
        set_from_Bing();
        break;
    case PAPER_TYPE::Other:
        set_from_Other();
        break;
    default:
        set_from_Wallhaven();
    }
}

void Wallpaper::next()
{
    if (thrd || mgr) return emit msgBox("频繁点击是没有效的哦！", "提示");
    qout << "下一张图片";
    if (VarBox->CurPic != VarBox->PicHistory.end() && ++VarBox->CurPic != VarBox->PicHistory.end())
    {
        thrd = QThread::create([this](){
            if (VarBox->PathFileExists(*VarBox->CurPic))
            {
                if (VarBox->GetFileAttributes(*VarBox->CurPic))
                {
                    if (VarBox->InternetGetConnectedState())
                    {
                        if (!VarBox->OneDriveFile(*VarBox->CurPic))
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
                VarBox->SystemParametersInfo(*VarBox->CurPic);
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
    switch (VarBox->PaperType)
    {
    case PAPER_TYPE::Advance:
        set_from_Advance();
        break;
    case PAPER_TYPE::Native:
        set_from_Native();
        break;
    case PAPER_TYPE::Bing:
        set_from_Bing();
        break;
    case PAPER_TYPE::Other:
        set_from_Other();
        break;
    default:
        set_from_Wallhaven();
    }
}

void Wallpaper::prev()
{
    if (thrd) return emit msgBox("和后台壁纸切换冲突，请稍后再试。", "提示");
    thrd = QThread::create([this](){
        for (int i=0; i<100; ++i)
        {
            if (VarBox->CurPic == VarBox->PicHistory.begin())
            {
                emit msgBox("无法找到更早的壁纸历史记录！", "提示");
                return ;
            }
            if (VarBox->PathFileExists(*--VarBox->CurPic))
            {
                if (VarBox->GetFileAttributes(*VarBox->CurPic))
                {
                    if (VarBox->InternetGetConnectedState())
                    {
                        if (!VarBox->OneDriveFile(*VarBox->CurPic))
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
                VarBox->SystemParametersInfo(*VarBox->CurPic);
                return;
            }
            delete [] *VarBox->CurPic;
            VarBox->CurPic = VarBox->PicHistory.erase(VarBox->CurPic);
            if (VarBox->CurPic != VarBox->PicHistory.begin())
                --VarBox->CurPic;
        }
    });
    connect(thrd, &QThread::finished, this, [this](){
        thrd->deleteLater(); thrd = nullptr;
    });
    thrd->start();
}

void Wallpaper::dislike()
{
    if (thrd)
       return emit msgBox("和后台壁纸切换冲突，请稍后再试。", "提示");
    thrd = QThread::create([](){
        qout << "不喜欢该壁纸。";
        const wchar_t* pic_path = *VarBox->CurPic;
        const wchar_t* pic_name = get_file_name(pic_path);
        char id[21] = { 0 };
        check_is_wallhaven(pic_name, id);
        if (*id)
        {
            YJson *json = new YJson(L"ImgData.json", YJSON_ENCODE::AUTO);
            YJson *blacklist = json->find("ImgUrls")->find("Blacklist");
            blacklist->append(id);
            json->toFile(L"ImgData.json", YJSON_ENCODE::UTF8BOM, true);
            delete  json;
        }
        DeleteFileW(pic_path);
        delete [] pic_path;
        delete [] pic_name;
        VarBox->CurPic = VarBox->PicHistory.erase(VarBox->CurPic);
        if (VarBox->CurPic != VarBox->PicHistory.begin())
            --VarBox->CurPic;
    });
    connect(thrd, &QThread::finished, this, [this](){
        thrd->deleteLater(); thrd = nullptr;
        qout << "设置壁纸!" ;
        next();
    });
    thrd->start();
}
