#include "wallpaper.h"
#include "funcbox.h"
#include "dialogwallpaper.h"
#include "YString.h"
#include <iostream>

DialogWallpaper::~DialogWallpaper()
{
    qout << "析构函数居然被调用了";
}

void DialogWallpaper::iniStart()
{
    if (VarBox->FirstChange)
    {
        thrd = new std::thread(&Wallpaper::startWork, this);
    }
}

void DialogWallpaper::startWork()                                                      //根据壁纸类型来判断执行哪个函数
{
    static const char _ = VARBOX::isOnline(true) && VarBox->AutoSaveBingPicture && set_from_Bing(false) && (++bing);
    PX_UNUSED(_);
    if (VarBox->RunApp)
    {
        //qout << "壁纸类型：" << VarBox->StandardNames[(int)VarBox->PaperType][1];
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

void DialogWallpaper::setWallhaven()
{
    VARBOX::isOnline(true) && set_from_Wallhaven();
}

void DialogWallpaper::setBing()
{
    VARBOX::isOnline(true) && set_from_Bing(true);
}

void DialogWallpaper::setRandom()
{
    VARBOX::isOnline(true) && set_from_Random();
}

void DialogWallpaper::setNative()
{
    set_from_Native(true);
}

void DialogWallpaper::setAdvance()
{
	if (!set_from_Advance())
        emit msgBox("设置壁纸失败，高级命令不正确！", "出错！");
}
