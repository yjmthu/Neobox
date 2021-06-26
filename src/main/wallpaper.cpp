#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QRegExp>

#include "wallpaper.h"
#include "funcbox.h"

bool Wallpaper::a_thresd_isrunning = false;

void chooseUrl(QString& url_1, QString& url_2)  //选则正确的请求链接组合
{
	D("Wallhaven分支选择中.....");
	switch (VarBox.PaperType)
	{
	case Latest:
        url_1 = "https://wallhaven.cc/latest";
		url_2 = "?page=";
		break;
	case Hot:
        url_1 = "https://wallhaven.cc/hot";
		url_2 = "?page=";
		break;
	case Nature:
        url_1 = "https://wallhaven.cc/search?q=id:37&sorting=random&ref=fp";
		url_2 = "&seed=DMPB3x&page=";
		break;
	case Anime:
        url_1 = "https://wallhaven.cc/search?q=id:1&sorting=random&ref=fp";
		url_2 = "&seed=DMPB3x&page=";
		break;
	default:
        url_1 = "https://wallhaven.cc/search?q=id:2278&sorting=random&ref=fp";
		url_2 = "&seed=DMPB3x&page=";
		break;
	}
    D("Wallhaven分支选择完毕。");
}

QString getBingName()                           //生成今日必应壁纸的名称
{
	QDateTime dateTime(QDateTime::currentDateTime());

	QString name = FuncBox::get_pic_path(Bing);
	name += "/";
	name += dateTime.toString("yyyy-MM-dd");
	name += "必应壁纸.jpg";
    D("必应壁纸名称：" + name);
		return name;
}

bool setWallpaper(QString img_name)             //根据路径设置壁纸
{
    D("即将设置壁纸：" + img_name);
		if (QFile::exists(img_name))
		{
            D("壁纸存在，名称是：" + img_name);
#if defined(Q_OS_WIN32)
				img_name = img_name.replace(QRegExp("/"), "\\");  //Windows.h的路径适合使用反斜杠
			std::wstring img_pa = img_name.toStdWString();
			PVOID img_path = (PVOID)(img_pa.c_str());
			SystemParametersInfoW(SPI_SETDESKWALLPAPER, UINT(0), img_path, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
#elif defined(Q_OS_LINUX)
				QStringList lst1, lst2;
			QString script = FuncBox::get_sub_path("/scripts") + "/SetWallPaper.sh";
			lst1.append("+x");
			lst1.append(script);
			lst2.append(img_name);
			FuncBox::runCommand("chmod", lst1, 0);                    //赋予脚本执行权限。
			FuncBox::runCommand(script, lst2, 0);                     //利用脚本更换壁纸，这只可以在 linux系统+kde桌面 上使用。
#endif
            D("设置壁纸成功，路径：" + img_name);
				return true;
		}
		else
		{
            D("壁纸文件" + img_name + "不存在，无法设置！");
				return false;
		}
}

bool download_from_Wallheaven(QString img_url, QString img_name)     //从Wallhaven下载图片，成功后设置壁纸
{
	return (
		(
            FuncBox::downloadImage(img_url + ".jpg", img_name + ".jpg")
			&&
			setWallpaper(img_name + ".jpg")
			)
		||
		(
            FuncBox::downloadImage(img_url + ".png", img_name + ".png")
			&&
			setWallpaper(img_name + ".png")
			)
		);
}

void Wallpaper::startWork()                                                      //根据壁纸类型来判断执行哪个函数
{
	switch (VarBox.PaperType)
	{
	case Advance:
		setAdvance();
		break;
	case Native:
		setNative();
		break;
	case Bing:
		setBing();
		break;
	case Random:
		setRandom();
		break;
	default:
		setWallhaven();
	};
}

bool Wallpaper::get_url_from_Wallhaven(bool netStatus) const            //从Wallhaven下载图片地址到ImgData.db
{
    D("计划从Wallhaven下载图片链接。");
    if (netStatus || FuncBox::isOnline(false))                     //netStatus为false则检测网络。
	{
        QFile img_data(FuncBox::get_dat_path() + "/ImgData.json");
        if (img_data.open(QIODevice::WriteOnly))
        {
            QString html, url_1, url_2, url;
            QRegExp rx("<a class=\"preview\" href=\"https://wallhaven.cc/w/(\\w{6})\"  target=\"_blank\"  >");
            rx.setMinimal(true);
            chooseUrl(url_1, url_2);
            QJsonArray jsonArray;
            D("下载120个链接中...");
            for (short k = 5*(VarBox.PageNum-1)+1; VarBox.RUN_APP&&(k <= 5*VarBox.PageNum); k++)    //获取所给的页面中的所有数据
            {
                int pos = 0;
                if (k == 1)
                    url = url_1;
                else
                    url = url_1 + url_2 + QString::number(k);
                VarBox.RUN_APP && FuncBox::getWebCode(url.toStdWString().c_str(), html);            //及时结束程序
                short stop = 0;
                while (VarBox.RUN_APP && ((pos = rx.indexIn(html, pos)) != -1) && (++stop <= 24))          //遍历匹配结果
                {
                    jsonArray.append(rx.cap(1));
                    pos += rx.matchedLength();
                }
            }
            if (jsonArray.size())
            {
                QJsonDocument jsonDoc;
                jsonDoc.setArray(jsonArray);
                img_data.write(jsonDoc.toJson());
                img_data.close();
                D("120个链接下载成功！");
                return VarBox.RUN_APP && true;
            }
            img_data.close();
        }

	}
    D("未能从Wallhaven下载链接。");
    return false;
}

bool Wallpaper::set_from_Wallhaven() const                              //从数据库中随机抽取一个链接地址进行设置。
{
    D("从数据库选取壁纸信息中。");
    QString file_name = FuncBox::get_dat_path() + "/ImgData.json";
    if (VarBox.RUN_APP && QFile::exists(file_name))
	{
        QFile img_data(file_name);
        if (VarBox.RUN_APP && img_data.open(QIODevice::ReadOnly))
        {
            QJsonParseError *error=new QJsonParseError;
            QJsonDocument jdc = QJsonDocument::fromJson(img_data.readAll(), error);
            if(VarBox.RUN_APP && error->error==QJsonParseError::NoError)
            {
                QJsonArray arry = jdc.array();
                short pic_num = arry.size();
                if (VarBox.RUN_APP && pic_num)
                {
                    srand(QDateTime::currentDateTime().time().second() + QDateTime::currentDateTime().time().msec());
                    QString pic = arry.at(rand() % pic_num).toString();
                    delete error;
                    D("数据库信息选取完毕！");
                    img_data.close();
                    return VarBox.RUN_APP && pic.compare("") && download_from_Wallheaven(
                    "https://w.wallhaven.cc/full/"+ pic.mid(0, 2) +"/wallhaven-" + pic,
                    FuncBox::get_pic_path(VarBox.PaperType) + "/wallhaven-" + pic);
                }
            }
            img_data.close();
            delete error;
        }
	}
        return false;
}

bool Wallpaper::set_from_Native() const
{
	VarBox.RUN_NET_CHECK = false;
    D("从本地选取壁纸进行设置。");
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
            D("即将设置本地壁纸：" + file_name);
				VarBox.RUN_NET_CHECK = true;
			return VarBox.RUN_APP && setWallpaper(file_name);
		}
		else
		{
			VarBox.RUN_NET_CHECK = true;
			return false;
		}
	}
	else
	{
		VarBox.RUN_NET_CHECK = true;
		return false;
	}
};

bool Wallpaper::set_from_Bing(bool setBing) const
{
	QString img_name = getBingName();
	if (VarBox.RUN_APP && !QFile::exists(img_name))
	{
		QString img_url;
        FuncBox::getBingCode(img_url);
        img_url = img_url.split("\" as=\"image\" /><link rel=\"preload\" href=")[0];
        img_url = img_url.split("<head><link id=\"bgLink\" rel=\"preload\" href=\"")[1];
        img_url = "https://cn.bing.com" + img_url;
        D("今日必应壁纸链接：" + img_url);
        return (FuncBox::downloadImage(img_url, img_name) && setBing) && setWallpaper(img_name);
	}
	else
		return VarBox.RUN_APP && setBing && setWallpaper(img_name);
}

bool Wallpaper::set_from_Random() const
{
    D("下载随机壁纸中。");
		QString img_name = FuncBox::get_pic_path(Random);
	img_name += "/" + QDateTime::currentDateTime().toString("yyyy-MM-dd hhmmss") + ".jpg";
	bool success = false;
    success = VarBox.RUN_APP && FuncBox::downloadImage("https://source.unsplash.com/random/2560x1600", img_name) && setWallpaper(img_name);
    if (success)
        D("随机壁纸下载完毕。");
    else
        D("随机壁纸下载失败。");
    return VarBox.RUN_APP && success;
}

bool Wallpaper::set_from_Advance() const
{
    D("高级设置开始。");
    VarBox.RUN_NET_CHECK = false;
	if (VarBox.RUN_APP && VarBox.UserCommand.compare(""))
	{
        D("高级命令是" + VarBox.UserCommand);
			QStringList lst = VarBox.UserCommand.split(" ");
		QString program_file = lst[0];
		lst.removeFirst();
		QString program_output = FuncBox::runCommand(program_file, lst, 1);
		program_output = program_output.trimmed();                      //trimmed()函数去除字符串首尾空格
        D("执行高级命令，命令行最终提取输出：" + program_output);
			if (!QString(program_output[0]).compare("\""))
				program_output = program_output.mid(1, program_output.length() - 2);   //去除首尾的 " 符号
		if (!QString(program_output[0]).compare("'"))
			program_output = program_output.mid(1, program_output.length() - 2);   //去除首尾的 ' 符号
		bool success = VarBox.RUN_APP && setWallpaper(program_output);
		if (success)
            D("高级设置成功");
		else
            D("高级设置失败。");
        VarBox.RUN_NET_CHECK = true;
		return success;
	}
	else
	{
        D("高级命令无效。");
        VarBox.RUN_NET_CHECK = true;
		return false;
	}
}
