#include "funcbox.h"
#include "menuwallpaper.h"

MenuWallpaper::MenuWallpaper()
{
    D("菜单壁纸构造成功。");
}

MenuWallpaper::~MenuWallpaper()
{
    D("菜单壁纸析构成功！");
}

void MenuWallpaper::run()
{
	if (!a_thresd_isrunning)
	{
		a_thresd_isrunning = true;
        D("换一张图已被点击！");
        D(QString("壁纸类型是：") + VarBox.PaperTypes[VarBox.PaperType]);
        startWork();
		a_thresd_isrunning = false;
	}
	else
	{
		emit msgBox("和后台壁纸切换冲突，请稍后再试。");
	}
}

void MenuWallpaper::setWallhaven()  //从数据库中随机抽取一个链接地址进行设置。
{
	if (VarBox.RUN_APP && !set_from_Wallhaven())
	{
		if (get_url_from_Wallhaven(false))
			set_from_Wallhaven();
		else
			emit msgBox("设置壁纸失败，请检查网络连接是否正常！");
	}
}

void MenuWallpaper::setBing()
{
	if (VarBox.RUN_APP && !set_from_Bing(true))
		emit msgBox("设置必应壁纸失败！");
}

void MenuWallpaper::setNative()
{
	if (VarBox.RUN_APP && !set_from_Native())
		emit msgBox("本地文件夹无效！");
}

void MenuWallpaper::setRandom()
{
	if (VarBox.RUN_APP && !set_from_Random())
		emit msgBox("设置壁纸失败，请检查网络连接是否正常！");
}

void MenuWallpaper::setAdvance()
{
	if (VarBox.RUN_APP && !set_from_Advance())
		emit msgBox("设置壁纸失败，高级命令不正确！");
}
