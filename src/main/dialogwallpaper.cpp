#include "wallpaper.h"
#include "funcbox.h"
#include "dialogwallpaper.h"

DialogWallpaper::DialogWallpaper()
{
    D("对话壁纸构造成功！");
}

DialogWallpaper::~DialogWallpaper()
{
    D("设置壁纸析构成功！");
}

void DialogWallpaper::run()
{
	if (!a_thresd_isrunning && VarBox.AutoChange)
	{
		a_thresd_isrunning = true;
		startWork();
		a_thresd_isrunning = false;
	}
}

void DialogWallpaper::setWallhaven()  // 从数据库中随机抽取一个链接地址进行设置。
{
    (FuncBox::isOnline(true) && get_url_from_Wallhaven(true)) && set_from_Wallhaven();
	set_from_Bing(false);
}

void DialogWallpaper::setBing()
{
	FuncBox::isOnline(true) && set_from_Bing(true);
}

void DialogWallpaper::setRandom()
{
	FuncBox::isOnline(true) && set_from_Random() && set_from_Bing(false);
}

void DialogWallpaper::setNative()
{
	if (!set_from_Native())
		emit setFailed("请更换本地文件夹、改变壁纸类型或取消自动更换壁纸！");
	FuncBox::isOnline(true) && set_from_Bing(false);
}

void DialogWallpaper::setAdvance()
{
	if (!set_from_Advance())
		emit setFailed("设置壁纸失败，高级命令不正确！");
	FuncBox::isOnline(true) && set_from_Bing(false);
}
