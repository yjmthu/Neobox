#include <fstream>
#include <QDateTime>
#include <QRandomGenerator>
#include <QDir>
#include <QFile>

#include "funcbox.h"
#include "wallpaper.h"
#include "YEncode.h"
#include "YString.h"
#include "YJson.h"


std::thread* Wallpaper::thrd = nullptr;
bool Wallpaper::initSet = false;
char Wallpaper::bing = 0;
bool Wallpaper::update = false;
std::string Wallpaper::url;//nullptr;
QString Wallpaper::image_path;
QString Wallpaper::bing_folder;
std::string Wallpaper::bing_api;

inline bool setWallpaper(const QString &img_name)             //根据路径设置壁纸
{
    if (QFile::exists(img_name))
    {
        qout << "设置壁纸：" << img_name;
        wchar_t* temp = StrJoin<wchar_t>(img_name.toStdWString().c_str());
        VarBox->PicHistory.push_back(std::pair<bool, wchar_t*>(true, temp));
        VarBox->CurPic = --VarBox->PicHistory.end();
        return SystemParametersInfoW(SPI_SETDESKWALLPAPER, UINT(0), (PVOID)temp, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
    }
    else
        return false;
}

Wallpaper::Wallpaper()
{
    connect(this, SIGNAL(finished()), this, SLOT(clean()));
}

Wallpaper::~Wallpaper()
{
    if (thrd)
    {
        thrd->join(); delete thrd; thrd = nullptr;
    }
}

void Wallpaper::start()
{
    if (!thrd)
    {
		thrd = new std::thread(&Wallpaper::startWork, this);
    }
	else
        emit msgBox("和后台壁纸切换冲突，请稍后再试。", "提示");
}

void Wallpaper::clean()
{
    qout << "清理线程";
    if (thrd) thrd->join(); delete thrd; thrd = nullptr;
}

bool Wallpaper::set_from_Wallhaven() const  // 从数据库中随机抽取一个链接地址进行设置。
{
    qout << "Wallhaven 开始检查json文件";
    QString file_name = VarBox->get_dat_path() + "\\ImgData.json";
    qout << "文件路径" << file_name;
    bool func_ok = false;
    std::string pic_url;
    YJson* jsonObject = nullptr, *jsonArray = nullptr, * find_item = nullptr, *blacklist=nullptr;
    if (!QFile::exists(file_name)) goto label_1;
    jsonObject = new YJson(file_name.toStdWString(), YJSON_ENCODE::UTF8);
    if (YJson::ep.first){
        qout << "出现错误";
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
            blacklist = jsonArray->find("Blacklist");
            jsonArray->find("Used")->clear();
            jsonArray = jsonArray->find("Unused");
            jsonArray->clear();
            if (!get_url_from_Wallhaven(*jsonArray)) return false;
        }
        else
        {
            blacklist = jsonArray->find("Blacklist");
            if (jsonArray->empty())
            {
                qout << "找到json文件但是没有孩子！";
                jsonArray->append(YJSON::ARRAY, "Used");
                blacklist = jsonArray->append(YJSON::ARRAY, "Balcklist");
                jsonArray = jsonArray->append(YJSON::ARRAY, "Unused");
                if (!get_url_from_Wallhaven(*jsonArray)) return false;
            }
            else if (jsonArray->find("Unused")->empty())
            {
                qout << "Unused为空.";
                if (jsonArray->find("Used")->empty())
                {
                    qout << "Used为空.";
                    jsonArray = jsonArray->find("Unused");
                    if (!get_url_from_Wallhaven(*jsonArray)) return false;
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
        }
        qout << "找到Json文件和孩子!";
        goto label_2;
    }
    qout << "json文件格式不正确！";
label_1:
    qout << "Wallhaven 创建新的Json对象";
    delete jsonObject;
    jsonObject = new YJson(YJSON::OBJECT);
    jsonObject->append(Wallpaper::url.c_str(), "Api");
    jsonObject->append(VarBox->PageNum, "PageNum");
    jsonArray = jsonObject->append(YJSON::OBJECT, "ImgUrls");
    jsonArray->append(YJSON::ARRAY, "Used");
    blacklist = jsonArray->append(YJSON::ARRAY, "Blacklist");
    jsonArray = jsonArray->append(YJSON::ARRAY, "Unused");
    qout << "Wallhaven 尝试从wallhaven下载源码";
    if (!get_url_from_Wallhaven(*jsonArray))
    {
        qout << "源码下载失败!";
        delete jsonObject;
        return false;
    }
    qout << "Wallhaven 源码下载完毕";
label_2:
    qout << "Wallhaven 开始设置";
    if (jsonArray->getChild())
    {
        int pic_num = jsonArray->getChildNum();
        qout << "Wallhaven 找到随机id";
        srand((unsigned)time(0));           // 防止出现重复
        char pic[7] {0};
        if (blacklist->getChild())
        {
            qout << "黑名不为空！";
            for (YJson *item = jsonArray->find(rand() % pic_num);pic_num; item = jsonArray->find(rand() % pic_num))
            {
                if (item->getType() == YJSON_TYPE::YJSON_STRING)
                {
                    std::copy(item->getValueString() + 41, item->getValueString() + 47, pic);
                    qout << "随机id" << pic;
                    if (blacklist->findByVal(pic))
                    {
                        qout << "在黑名单里面";
                        jsonArray->remove(item);
                        --pic_num;
                        *pic = 0;
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
            *pic = true;
        }
        if (*pic)
        {
            qout << "壁纸网址：" << pic_url.c_str();
            const QString&& img_name = Wallpaper::image_path + "\\wallhaven-" + pic_url.substr(41).c_str();
            func_ok = VARBOX::downloadImage(pic_url, img_name) && setWallpaper(img_name);
            qout << "壁纸设置完毕" << func_ok;
        }
        qout << "保存 json 文件";
        jsonObject->toFile(file_name.toStdWString(), YJSON_ENCODE::UTF8, true);
        qout << "json 文件保存完毕";
    }
    else
    {
        jsonArray = jsonArray->getParent()->find("Unused");
        if (!get_url_from_Wallhaven(*jsonArray)) return false;
        goto label_2;
    }
    delete jsonObject;
	return func_ok;
}

bool Wallpaper::get_url_from_Wallhaven(YJson& jsonArray) const            //从Wallhaven下载图片地址到ImgData.db
{
    QByteArray json_byte;
    qout << url.c_str();
    for (short k = 5 * (VarBox->PageNum - 1) + 1; VarBox->RunApp && (k <= 5 * VarBox->PageNum); k++)    //获取所给的页面中的所有数据
        if (VARBOX::getWebCode(url + "&page=" + std::to_string(k), json_byte))
        {
            qout << "good" << json_byte.toStdString().c_str();
            YJson *js = new YJson(json_byte);
            qout << "nn";
            YJson* ptr = js->find("data")->getChild();
            if (ptr)
                do {
                    jsonArray.append(ptr->find("path")->getValueString());
                } while (ptr = ptr->getNext());
            delete js;
            qout << "mm";
        }
        else
            return false;
    qout << "bad";
    return true;
}

void Wallpaper::set_from_Native(bool net)
{
    QDir dir(VarBox->NativeDir);
    if (dir.exists())
    {
        QStringList filters;
        filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.wbep";
        dir.setFilter(QDir::Files | QDir::NoSymLinks);                                //设置类型过滤器，只为文件格式
        int dir_count = dir.count();
        if (!dir_count)
        {
            if (!VARBOX::isOnline(false))
            {
                if (VARBOX::isOnline(true))
                    goto label_normal;
            }
            else
                goto label_quit;
        }
label_normal:
        QString file_name = dir[QRandomGenerator::global()->bounded(dir_count)];  //随机生成文件名称。
        file_name = VarBox->NativeDir + "/" + file_name;
        if ((GetFileAttributesW(file_name.toStdWString().c_str()) & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS))
        {
            if (VARBOX::isOnline(net))
            {
                if (!VARBOX::OneDriveFile(file_name.toStdWString().c_str()))
                {
                    qout << "文件属性：" <<GetFileAttributesW(file_name.toStdWString().c_str());
                    emit setFailed("本地文件无效，请更换本地文件夹、改变壁纸类型或取消自动更换壁纸！");
                    return;
                }
            }
            else
            {
                emit msgBox("没有网络！", "提示");
                return;
            }
        }
        VarBox->RunApp && setWallpaper(file_name);
        return;
    }
label_quit: emit setFailed("请更换本地文件夹、改变壁纸类型或取消自动更换壁纸！");
};


bool Wallpaper::set_from_Bing(bool setBing) const
{
    if (!VarBox->AutoRotationBingPicture || bing < -7 || bing > 0) bing = 0;
    //qout << "必应值: " << (int)bing;

    std::string today = QDateTime::currentDateTime().toString("yyyy-MM-dd").toStdString();
    QString file_name = VARBOX::get_dat_path() + "\\BingData.json";
    bool need_save = false;
    YJson *file_data = nullptr, *temp = nullptr;
    if (!QFile::exists(file_name)) goto label_1;
label_0:
        {
            qout << "必应文件存在";
            file_data = new YJson(file_name.toStdWString(), YJSON_ENCODE::UTF16BOM);
            if (strcmp(file_data->find("today")->getValueString(), today.c_str()))
            {
                qout << "必应文件过期，将开始下载最新数据";
                delete file_data;
                bing = 0;
                file_data = nullptr;
                need_save = true;
                PX_UNUSED(need_save);
                goto label_1;
            }
            qout << "必应文件为最新";
            temp = file_data->find("images")->find(-bing);
            qout << "查找到images";
            goto label_2;
        }
label_1:
        {
            qout << "必应文件不存在或过期";
            QByteArray img_html;

            VARBOX::getWebCode(bing_api, img_html);
            YJson bing_data(img_html); YJson *find;
            if (bing_data.getType() != YJSON_TYPE::YJSON_OBJECT || !(find = bing_data.find("images")) || !(find = find->getChild()))
                return false;
            file_data = new YJson(YJSON::OBJECT);
            file_data->append(today.c_str(), "today");
            YJson* imgs = file_data->append(YJSON::ARRAY, "images");
            constexpr char pattern_str[] = u8" (© "; const char* ptr_1, *ptr_2; char *ptr_3;
            do {
                temp = imgs->append(YJSON::OBJECT);
                ptr_1 = ptr_2 = find->find("copyright")->getValueString();
                while (*ptr_2 && strncmp<const char*>(ptr_2, pattern_str, 5))++ptr_2;
                ptr_3 = new char[ptr_2-ptr_1+1] {0};
                StrCopy<char>(ptr_3, ptr_1, --ptr_2);
                temp->append(ptr_3, "copyright");
                temp->append(find->find("url")->getValueString(), "url");
            } while (find = find->getNext());
            temp = imgs->find(-bing);
            need_save = true;
            goto label_2;
        }
label_2:
        {
            std::string img_url("https://cn.bing.com");
            img_url += temp->find("url")->getValueString();
            QString img_name = bing_folder + "\\";
            if (VarBox->UseDateAsBingName)
            {
                if (bing)
                {
                    img_name += QDateTime::currentDateTime().addDays(bing).toString("yyyy-MM-dd");
                }
                else
                {
                    img_name += today.c_str();
                }
                img_name += "必应壁纸.jpg";
            }
            else
            {
                //qout << "使用CopyRight名称";
                img_name += temp->find("copyright")->getValueString();
                img_name += ".jpg";
            }

            if (need_save)
            {
                qout << "需要保存必应数据";
                file_data->toFile(file_name.toStdWString(), YJSON_ENCODE::UTF16, true);
            }
            delete file_data;
            if (VarBox->CurPic != VarBox->PicHistory.end())
            {
                QString cur_name;
                if (VarBox->CurPic->first)
                {
                    cur_name = QString::fromWCharArray(VarBox->CurPic->second);
                }
                else
                {
                    cur_name = reinterpret_cast<char*>(VarBox->CurPic->second);
                }
                if (!cur_name.compare(img_name))
                {
                    --bing;
                    goto label_0;
                }
            }

            if (QFile::exists(img_name))
            {
                //qout << "必应壁纸存在！";
                --bing;
                return (setBing && setWallpaper(img_name)) || true;
            }

            if ((VARBOX::downloadImage(img_url, img_name) && setBing) && setWallpaper(img_name))
            {
                //qout << "必应壁纸下载成功";
                --bing;
                return true;
            }
            //qout << "必应壁纸下载失败";
            goto label_3;
        }
label_3:
    return false;
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

bool Wallpaper::set_from_Advance() const
{
    if (VarBox->UserCommand.isEmpty())
    {
        return false;
    }
    else
    {
        QStringList&& lst = _parse_arguments(VarBox->UserCommand);
        QString program_file = lst[0]; lst.removeFirst();
        if (initSet)
        {
            initSet = false;
            lst << "0";
        }
        else
        {
            lst << "1";
        }
        char* program_output = VARBOX::runCommand(program_file, lst, 1);
        if (program_output)
        {
            for (size_t i = strlen(program_output)-1; i > 0; --i)
            {
                if (strchr("\"\\\b\f\n\r\t\'", uchar(program_output[i])))
                {
                    program_output[i] = 0;
                }
                else
                {
                    if (strlen(program_output) && VARBOX::PathFileExists(program_output))
                    {
                        SystemParametersInfoA(SPI_SETDESKWALLPAPER, UINT(0), (PVOID)program_output, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
                        //这里当然不需要 delete [] program_output;
                        VarBox->PicHistory.push_back(std::pair<bool, wchar_t*>(false, reinterpret_cast<wchar_t*>(program_output)));
                        VarBox->CurPic = --VarBox->PicHistory.end();
                        return true;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            delete [] program_output;
        }
        return false;
    }
}
