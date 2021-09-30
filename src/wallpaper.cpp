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

void chooseUrl(const char* &url_1, const char* &url_2)  //选则正确的请求链接组合
{
    switch (VarBox->PaperType)
	{
	case PAPER_TYPE::Latest:
        url_1 = StrJoin<char>("https://wallhaven.cc/latest");
        url_2 = StrJoin<char>("?page=");
		break;
	case PAPER_TYPE::Hot:
        url_1 = StrJoin<char>("https://wallhaven.cc/hot");
        url_2 = StrJoin<char>("?page=");
		break;
	case PAPER_TYPE::Nature:
        url_1 = StrJoin<char>("https://wallhaven.cc/search?q=id:37&sorting=random&ref=fp");
        url_2 = StrJoin<char>("&seed=DMPB3x&page=");
		break;
	case PAPER_TYPE::Anime:
        url_1 = StrJoin<char>("https://wallhaven.cc/search?q=id:1&sorting=random&ref=fp");
        url_2 = StrJoin<char>("&seed=DMPB3x&page=");
		break;
	default:
        url_1 = StrJoin<char>("https://wallhaven.cc/search?q=id:2278&sorting=random&ref=fp");
        url_2 = StrJoin<char>("&seed=DMPB3x&page=");
		break;
	}
}

inline bool setWallpaper(const QString &img_name)             //根据路径设置壁纸
{
    if (QFile::exists(img_name))
    {
        //qout << "设置壁纸：" << img_name;
        wchar_t* temp = StrJoin<wchar_t>(img_name.toStdWString().c_str());
        VarBox->PicHistory.push_back(std::pair<bool, wchar_t*>(true, temp));
        VarBox->CurPic = --VarBox->PicHistory.end();
        return SystemParametersInfoW(SPI_SETDESKWALLPAPER, UINT(0), (PVOID)temp, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
    }
    else
        return false;
}

bool download_from_Wallheaven(const char* img_url, QString img_name)     //从Wallhaven下载图片，成功后设置壁纸
{
	return (
		(
            VARBOX::downloadImage(StrJoin<char>(img_url, ".jpg"), img_name+".jpg")
			&&
            setWallpaper(img_name + ".jpg")
			)
		||
		(
            VARBOX::downloadImage(StrJoin<char>(img_url, ".png"), img_name + ".png")
			&&
            setWallpaper(img_name + ".png")
			)
		);
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
    // qout << "Wallhaven 开始检查json文件";
    QString file_name = VarBox->get_dat_path() + "\\ImgData.json";
    // qout << "文件路径" << file_name;
    bool func_ok = false;
    YJson* jsonObject = nullptr, *jsonArray = nullptr, * find_item = nullptr; bool need_save = false;
    if (Wallpaper::update)
    {
        Wallpaper::update = false;
        goto label_1;
    }
    if (!QFile::exists(file_name)) goto label_1;
    jsonObject = new YJson(file_name.toStdWString(), YJSON_ENCODE::UTF8);
    if (YJson::ep.first) goto label_1;
    if (jsonObject->getType() == YJSON_TYPE::YJSON_OBJECT &&
        (find_item = jsonObject->find("PaperType")) &&
        find_item->getType() == YJSON_TYPE::YJSON_STRING &&
        !strcmp(find_item->getValueString(), VarBox->StandardNames[(int)VarBox->PaperType][0]) &&
        (find_item = jsonObject->find("PageNum")) &&
        find_item->getType() == YJSON_TYPE::YJSON_NUMBER &&
        find_item->getValueInt() == VarBox->PageNum &&
        (jsonArray = jsonObject->find("ImgUrls")) &&
        jsonArray->getType() == YJSON_TYPE::YJSON_ARRAY)
    {
        if (!jsonArray->getChild())
        {
            //qout << "找到json文件但是没有孩子！";
            need_save = get_url_from_Wallhaven(*jsonArray);
            if (!need_save) return false;
        }
        //qout << "找到Json文件和孩子!";
        goto label_2;
    }
    //qout << "json文件不正确！";
    delete jsonObject; jsonObject = nullptr;
label_1:
    //qout << "Wallhaven 创建新的Json对象";
    jsonObject = new YJson(YJSON::OBJECT);
    jsonObject->append(VarBox->StandardNames[static_cast<int>(VarBox->PaperType)][0], "PaperType");
    jsonObject->append(VarBox->PageNum, "PageNum");
    jsonArray = jsonObject->append(YJSON::ARRAY, "ImgUrls");
    //qout << "Wallhaven 尝试从wallhaven下载源码";
    need_save = get_url_from_Wallhaven(*jsonArray);
    if (!need_save)
    {
        delete jsonObject; jsonObject = nullptr;
        return false;
    }
    //qout << "Wallhaven 源码下载完毕";
label_2:
    //qout << "Wallhaven 开始设置";
    YJson& img_data = *jsonArray;
    int pic_num = img_data.getChildNum();
    if (pic_num)
    {
        //qout << "Wallhaven 找到随机id";
        const char* pic = nullptr;
        QString temp = VARBOX::get_dat_path() + "\\Blacklist.json";
        if (QFile::exists(temp))
        {
            //qout << "黑名单文件存在！";
            YJson blacklist(temp.toStdWString(), YJSON_ENCODE::UTF8BOM);
            if (blacklist.getType() == YJSON_TYPE::YJSON_ARRAY)
                for (unsigned char x = 0; x < 0xff && pic_num; ++x)
                {
                    srand((unsigned)time(0)+x);           // 防止出现重复
                    YJson &item = img_data[rand() % pic_num];
                    if (item.getType() == YJSON_TYPE::YJSON_STRING)
                    {
                        pic = item.getValueString();
                        //qout << "随机id" << pic;
                        if (blacklist.findByVal(pic))
                        {
                            //qout << "在黑名单里面";
                            img_data.removeByVal(pic);
                            --pic_num;
                            need_save = true;
                            pic = nullptr;
                        }
                        else
                        {
                            //qout << "不在黑名单里面";
                            break;
                        }
                    }
                }
        }
        else
        {
            srand((unsigned)time(0));
            YJson& item = img_data[rand() % pic_num];
            pic = item.getValueString();
        }
        if (pic && strlen(pic) == 6)
        {
            //qout << "Wallhaven 开始下载壁纸";
            srand((unsigned)time(0));
            char pic_mid[3] = { 0 }; StrCopy<char>(pic_mid, pic, pic + 1);
            char* pic_url = StrJoin<char>("https://w.wallhaven.cc/full/", pic_mid, "/wallhaven-", pic);
            //qout << "壁纸网址：" << pic_url;
            QString pic_path = VarBox->get_pic_path((short)VarBox->PaperType) + "\\wallhaven-" + pic;
            //qout << "壁纸存储位置：" << pic_path;
            func_ok = download_from_Wallheaven(pic_url, pic_path);
            //qout << "壁纸设置完毕";
            delete[] pic_url;
        }
        if (need_save)
        {
            //qout << "保存 json 文件";
            jsonObject->toFile(file_name.toStdWString(), YJSON_ENCODE::UTF8, true);
            //qout << "json 文件保存完毕";
        }
    }
    else
    {
        delete jsonObject;
        jsonObject = nullptr;
        goto label_1;
    }
    delete jsonObject; jsonObject = nullptr;
	return func_ok;
}

bool Wallpaper::get_url_from_Wallhaven(YJson& jsonArray) const            //从Wallhaven下载图片地址到ImgData.db
{
    std::string html; const char *url_1, *url_2, *url;
	chooseUrl(url_1, url_2); bool func_ok = true;
    const char str_a[] = "<a class=\"preview\" href=\"https://wallhaven.cc/w/";
    const char str_b[] = "\"  target=\"_blank\"  ></a><div class=\"thumb-info\">";
    for (short k = 5 * (VarBox->PageNum - 1) + 1; VarBox->RunApp && (k <= 5 * VarBox->PageNum); k++)    //获取所给的页面中的所有数据
	{
		if (k == 1)
            url = StrJoin<char>(url_1);
		else
		{
            url = StrJoin<char>(url_1, url_2, std::to_string(k).c_str());
		}
        func_ok &= VARBOX::getWebCode(url, html, true);
        const char* pos = html.c_str(); short stop = 0;
		while (*pos && (++stop <= 24))          //遍历匹配结果
		{
            char math_pos[7] = { 0 };
			while (*pos)
			{
                if (!strncmp<const char*>(pos, str_a, 48) && StrContainCharInRanges<char>(pos + 48, 6, "a-z", "0-9") && !strncmp<const char*>(pos + 54, str_b, 49))
				{
                    StrCopy<char>(math_pos, pos + 48, pos + 53);
                    jsonArray.append(math_pos); pos += 103;
				}
				else
					++pos;
			}
		}
	}
    delete [] url_1; delete [] url_2;
	return func_ok;
}

void Wallpaper::set_from_Native(bool net)
{
    QDir dir(VarBox->NativeDir);
    if (dir.exists())
    {
        QStringList filters;
        filters << QString("*.png");
        filters << QString("*.jpg");
        filters << QString("*.jpeg");
        filters << QString("*.bmp");
        filters << QString("*.wbep");
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
            //qout << "必应文件存在";
            file_data = new YJson(file_name.toStdWString(), YJSON_ENCODE::UTF16BOM);
            if (strcmp(file_data->find("today")->getValueString(), today.c_str()))
            {
                //qout << "必应文件过期，将开始下载最新数据";
                delete file_data;
                bing = 0;
                file_data = nullptr;
                need_save = true;
                PX_UNUSED(need_save);
                goto label_1;
            }
            //qout << "必应文件为最新";
            temp = file_data->find("images")->find(-bing);
            //qout << "查找到images";
            goto label_2;
        }
label_1:
        {
            //qout << "必应文件不存在或过期";
            std::string img_html;
            VARBOX::getWebCode("https://cn.bing.com/HPImageArchive.aspx?format=js&idx=0&n=8&mkt=zh-CN", img_html, false);
            YJson bing_data(img_html); YJson *find;
            if (bing_data.getType() != YJSON_TYPE::YJSON_OBJECT || !(find = bing_data.find("images")) || !(find = find->getChild()))
                return false;
            file_data = new YJson(YJSON::OBJECT);
            file_data->append(today.c_str(), "today");
            YJson* imgs = file_data->append(YJSON::ARRAY, "images");
            constexpr const char pattern_str[] = u8" (© "; const char* ptr_1, *ptr_2; char *ptr_3;
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
            const char * const img_url = StrJoin<char>("https://cn.bing.com", temp->find("url")->getValueString());
            QString img_name = VarBox->get_pic_path((short)PAPER_TYPE::Bing) + "\\";
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

bool Wallpaper::set_from_Random() const
{
    QString img_name = VarBox->get_pic_path((short)PAPER_TYPE::Random);
    img_name += "\\" + QDateTime::currentDateTime().toString("yyyy-MM-dd hhmmss") + ".jpg";
    return VARBOX::downloadImage("https://source.unsplash.com/random/2560x1600", img_name, false) && setWallpaper(img_name);
}

bool Wallpaper::set_from_Advance() const
{
    if (VarBox->UserCommand.isEmpty())
    {
        return false;
    }
    else
    {
        QStringList lst = VarBox->UserCommand.split(" ");
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
                    program_output[i]=0;
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
