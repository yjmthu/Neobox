#include "funcbox.h"
#include "menuwallpaper.h"

void MenuWallpaper::startWork()                                                      //根据壁纸类型来判断执行哪个函数
{
    if (VarBox->RunApp && VarBox->CurPic != VarBox->PicHistory.end() && ++VarBox->CurPic != VarBox->PicHistory.end())
    {
        if ((VarBox->CurPic)->first)
        {
            if (VARBOX::PathFileExists(static_cast<wchar_t*>(VarBox->CurPic->second)))
            {
                SystemParametersInfoW(SPI_SETDESKWALLPAPER, UINT(0), VarBox->CurPic->second, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
            }
        }
        else
        {
            if (VARBOX::PathFileExists(static_cast<char*>(VarBox->CurPic->second)))
            {
                SystemParametersInfoA(SPI_SETDESKWALLPAPER, UINT(0), VarBox->CurPic->second, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
            }
        }
    }
    else if (VarBox->RunApp)
    {
        qout << "壁纸类型：" << VarBox->StandardNames[static_cast<int>(VarBox->PaperType)];
        switch (VarBox->PaperType)
        {
        case PAPER_TYPE::Advance:
            setAdvance();
            break;
        case PAPER_TYPE::Native:
            setNative();
            break;
        case PAPER_TYPE::Bing:
            setBing();
            break;
        case PAPER_TYPE::Random:
            setRandom();
            break;
        default:
            setWallhaven();
        }
    }
    emit finished();
}

void MenuWallpaper::setWallhaven()
{
    if (VARBOX::isOnline(false))
		set_from_Wallhaven();
	else
        emit msgBox("没有网络！", "提示");
}

void MenuWallpaper::setBing()
{
    if (VARBOX::isOnline(false))
	{
        if (!set_from_Bing(true)) emit msgBox("设置必应壁纸失败！", "警告");
	}
	else
        emit msgBox("没有网络！", "提示");
}

void MenuWallpaper::setNative()
{
	if (!set_from_Native())
        emit setFailed("本地文件夹无效！");
}

void MenuWallpaper::setRandom()
{
    if (VARBOX::isOnline(false))
	{
        if (!set_from_Random()) emit msgBox("设置随机壁纸失败！", "警告");
	}
	else
        emit msgBox("设置壁纸失败，请检查网络连接是否正常！", "警告");
}

void MenuWallpaper::setAdvance()
{
	if (!set_from_Advance())
        emit setFailed("设置壁纸失败，高级命令不正确！");
}
