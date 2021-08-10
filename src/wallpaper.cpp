#include <QDateTime>
#include <QRandomGenerator>
#include <QDir>
#include <QFile>

#include "funcbox.h"
#include "wallpaper.h"
#include "YString.h"
#include "YJson.h"


std::thread* Wallpaper::thrd = nullptr;
bool Wallpaper::initSet = false;

void chooseUrl(const char* &url_1, const char* &url_2)  //选则正确的请求链接组合
{
	switch (VarBox.PaperType)
	{
	case PAPER_TYPE::Latest:
        url_1 = StrJoin("https://wallhaven.cc/latest");
        url_2 = StrJoin("?page=");
		break;
	case PAPER_TYPE::Hot:
        url_1 = StrJoin("https://wallhaven.cc/hot");
        url_2 = StrJoin("?page=");
		break;
	case PAPER_TYPE::Nature:
        url_1 = StrJoin("https://wallhaven.cc/search?q=id:37&sorting=random&ref=fp");
        url_2 = StrJoin("&seed=DMPB3x&page=");
		break;
	case PAPER_TYPE::Anime:
        url_1 = StrJoin("https://wallhaven.cc/search?q=id:1&sorting=random&ref=fp");
        url_2 = StrJoin("&seed=DMPB3x&page=");
		break;
	default:
        url_1 = StrJoin("https://wallhaven.cc/search?q=id:2278&sorting=random&ref=fp");
        url_2 = StrJoin("&seed=DMPB3x&page=");
		break;
	}
}

bool setWallpaper(const QString &img_name)             //根据路径设置壁纸
{
    if (QFile::exists(img_name)){ qout << "设置壁纸：" << img_name;
        return SystemParametersInfoW(SPI_SETDESKWALLPAPER, UINT(0), (PVOID)img_name.toStdWString().c_str(), SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
}	else
        return false;
}

bool download_from_Wallheaven(const char* img_url, QString img_name)     //从Wallhaven下载图片，成功后设置壁纸
{
	return (
		(
            FuncBox::downloadImage(StrJoin(img_url, ".jpg"), img_name+".jpg")
			&&
            setWallpaper(img_name + ".jpg")
			)
		||
		(
            FuncBox::downloadImage(StrJoin(img_url, ".png"), img_name + ".png")
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
    cleanThread();
}

void Wallpaper::start()
{
	if (!thrd)
    {
		thrd = new std::thread(&Wallpaper::startWork, this);
        _is_working = true;
    }
	else
        emit msgBox("和后台壁纸切换冲突，请稍后再试。", "提示");
}

void Wallpaper::clean()
{
    cleanThread();
    _is_working = false;
}

void Wallpaper::cleanThread()
{
    if (thrd)
    {
        thrd->join(); delete thrd; thrd = nullptr;
    }
}

bool Wallpaper::isActive()
{
    return _is_working;
}

bool Wallpaper::canCreat()
{
    return !thrd;
}

bool Wallpaper::set_from_Wallhaven() const  // 从数据库中随机抽取一个链接地址进行设置。
{
    qout << "Wallhaven 开始检查json文件";
    QString file_name = FuncBox::get_dat_path() + "\\ImgData.json";
    qout << "文件路径" << file_name;
    bool func_ok = false;
	YJsonItem* jsonObject = nullptr, * jsonArray = nullptr, * find_item = nullptr; int type = 0; bool need_save = false;
	do {
		if (type == 0)
		{
            if (!QFile::exists(file_name))
			{
                qout << "没有找到json文件";
				type = 1; continue;
			}
			else
			{
                jsonObject = new YJsonItem(file_name.toStdString(), YJsonParse::YJson_File);
				if (!YJsonItem::ep && jsonObject->getType() == YJson::YJSON_OBJECT &&
                    (find_item = jsonObject->findItem("PaperType")) &&
					find_item->getType() == YJson::YJSON_STRING &&
                    StrCompare(find_item->getValueSring(), VarBox.PaperTypes[(int)VarBox.PaperType][0]) &&
                    (find_item = jsonObject->findItem("PageNum")) &&
					find_item->getType() == YJson::YJSON_NUMBER &&
					find_item->getValueInt() == VarBox.PageNum &&
                    (jsonArray = jsonObject->findItem("ImgUrls")) &&
					jsonArray->getType() == YJson::YJSON_ARRAY)
				{
					if (!jsonArray->getChildItem())
					{
                        qout << "找到json文件但是没有孩子！";
						need_save = get_url_from_Wallhaven(*jsonArray);
					}
                    qout << "找到Json文件和孩子!";
					type = 2; continue;
				}
				else
				{
                    qout << "json文件不正确！";
					delete jsonObject; jsonObject = nullptr;
					type = 1; continue;
				}
			}
		}
		else if (type == 1)
		{
            qout << "Wallhaven 创建新的Json对象";
            jsonObject = new YJsonItem("{}");
            jsonObject->appendItem(VarBox.PaperTypes[(int)VarBox.PaperType][0], "PaperType");
            jsonObject->appendItem(VarBox.PageNum, "PageNum");
            jsonArray = jsonObject->appendItem(YJsonItem::newArray(), "ImgUrls");
            qout << "Wallhaven 尝试从wallhaven下载源码";
			need_save = get_url_from_Wallhaven(*jsonArray);
            qout << "Wallhaven 源码下载完毕";
			type = 2; continue;
		}
		else if (type == 2)
		{
            qout << "Wallhaven 开始设置";
			YJsonItem& img_data = *jsonArray;
			int pic_num = img_data.getChildNum();
			if (pic_num)
			{
                qout << "Wallhaven 找到随机id";
				srand((unsigned)time(NULL));
				YJsonItem& item = img_data[rand() % pic_num];
				if (item.getType() == YJson::YJSON_STRING)
				{
                    const char* pic = item.getValueSring();
                    if (pic && strlen(pic) == 6)
					{
                        qout << "Wallhaven 开始下载壁纸";
						srand((unsigned)time(0));
                        char pic_mid[3] = { 0 }; StrCopy(pic_mid, pic, pic + 1);
                        char* pic_url = StrJoin("https://w.wallhaven.cc/full/", pic_mid, "/wallhaven-", pic);
                        qout << "壁纸网址：" << pic_url;
                        QString pic_path = FuncBox::get_pic_path((short)VarBox.PaperType) + "\\wallhaven-" + pic;
                        qout << "壁纸存储位置：" << pic_path;
                        func_ok = download_from_Wallheaven(pic_url, pic_path);
                        qout << "壁纸设置完毕";
                        delete[] pic_url;
					}
				}
                if (need_save)
                {
                    qout << "保存 json 文件";
                    jsonObject->toFile(file_name.toStdString());
                    qout << "json 文件保存完毕";
                }
			}
			delete jsonObject; jsonObject = nullptr; break;
		}
		else
		{
            qout << "Wallhaven 杀掉漏网json";
			if (jsonObject) delete jsonObject; break;
		}
	} while (true);
	return func_ok;
}

bool Wallpaper::get_url_from_Wallhaven(YJsonItem& jsonArray) const            //从Wallhaven下载图片地址到ImgData.db
{
    std::string html; const char *url_1, *url_2, *url;
	chooseUrl(url_1, url_2); bool func_ok = true;
    for (short k = 5 * (VarBox.PageNum - 1) + 1; VarBox.RunApp && (k <= 5 * VarBox.PageNum); k++)    //获取所给的页面中的所有数据
	{
		if (k == 1)
            url = StrJoin(url_1);
		else
		{
            url = StrJoin(url_1, url_2, std::to_string(k).c_str());
		}
		func_ok &= FuncBox::getWebCode(url, html);
        const char* pos = html.c_str(); short stop = 0;
        const char str_a[] = "<a class=\"preview\" href=\"https://wallhaven.cc/w/";
        const char str_b[] = "\"  target=\"_blank\"  ></a><div class=\"thumb-info\">";
		while (*pos && (++stop <= 24))          //遍历匹配结果
		{
            char math_pos[7] = { 0 };
			while (*pos)
			{
                if (StrCompare(pos, str_a, 48) && StrContainRange(pos + 48, 6, "a-z", "0-9") && StrCompare(pos + 54, str_b, 49))
				{
					StrCopy(math_pos, pos + 48, pos + 53);
					jsonArray += math_pos; pos += 103;
				}
				else
					++pos;
			}
		}
	}
    delete [] url_1; delete [] url_2;
	return func_ok;
}

bool Wallpaper::set_from_Native() const
{
    QDir dir(VarBox.NativeDir);
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
        if (dir_count)
        {
            QString file_name = dir[QRandomGenerator::global()->bounded(dir_count)];  //随机生成文件名称。
            file_name = VarBox.NativeDir + "/" + file_name;
            return VarBox.RunApp && setWallpaper(file_name);
        }
    }
    return false;
};

bool Wallpaper::set_from_Bing(bool setBing) const
{
    QDateTime dateTime(QDateTime::currentDateTime());

    QString img_name = FuncBox::get_pic_path((short)PAPER_TYPE::Bing);
    img_name += "/";
    img_name += dateTime.toString("yyyy-MM-dd");
    img_name += "必应壁纸.jpg";
    qout << "必应壁纸名称：" << img_name;

    if (QFile::exists(img_name))
        return (setBing && setWallpaper(img_name)) || true;
	else
	{
        std::string img_html;
        FuncBox::getBingCode(img_html); qout << img_html.c_str();
        const char* pos_a = img_html.c_str(); const char* pos_b;
		while (true)
		{
			if (*pos_a)
			{
                if (StrCompare(pos_a, "<link rel=\"preload\" href=\"", 26))
				{
                    pos_a += 26;
					break;
				}
				else
					pos_a++;
			}
			else
				return false;
		}
		pos_b = pos_a;
        qout << "起始点" << pos_b;
		while (true)
		{
			if (*pos_b)
			{
                if (StrCompare(pos_b, "\"", 1))
				{
					--pos_b;
					break;
				}
				else
                    ++pos_b;
			}
			else
				return false;
		}
        char* img_id = new char[pos_b - pos_a + 2]; StrCopy(img_id, pos_a, pos_b);
        char* img_url = StrJoin("https://cn.bing.com", img_id); delete[] img_id;
        qout << "必应壁纸链接：" << img_url;
        return (FuncBox::downloadImage(img_url, img_name) && setBing) && setWallpaper(img_name);
	}	
}

bool Wallpaper::set_from_Random() const
{
    QString img_name = FuncBox::get_pic_path((short)PAPER_TYPE::Random);
    img_name += "\\" + QDateTime::currentDateTime().toString("yyyy-MM-dd hhmmss") + ".jpg";
    return FuncBox::downloadImage("https://source.unsplash.com/random/2560x1600", img_name, false) && setWallpaper(img_name);
}

typedef BOOL(WINAPI* pfnPathFileExists)(LPCSTR pszPath);
BOOL PathFileExists(LPCSTR pszPath)
{
    pfnPathFileExists pPathFileExists = NULL;
    BOOL ret = FALSE;
    HMODULE hUser = GetModuleHandle(TEXT("Shlwapi.dll"));
    if (hUser)
        pPathFileExists = (pfnPathFileExists)GetProcAddress(hUser, "PathFileExistsA");
    if (pPathFileExists)
    {
        qout << "找到函数！";
        ret = pPathFileExists(pszPath);
    }
    else
    {
        qout << "找不到函数！";
    }
    return ret;
}

bool Wallpaper::set_from_Advance() const
{
    if (VarBox.UserCommand.isEmpty())
    {
        return false;
    }
    else
    {
        QStringList lst = VarBox.UserCommand.split(" ");
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
        char* program_output = FuncBox::runCommand(program_file, lst, 1);
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
                    if (strlen(program_output) && PathFileExists(program_output))
                    {
                        SystemParametersInfoA(SPI_SETDESKWALLPAPER, UINT(0), (PVOID)program_output, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
                        delete [] program_output;
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
