#include <fstream>
#include <QFile>

#include "YString.h"
#include "YJson.h"
#include "funcbox.h"
#include "menuwallpaper.h"

const wchar_t* get_file_name(const wchar_t* file_path)
{
    const wchar_t* ptr = file_path;
    while (*++ptr);
    while (*--ptr != '\\');
    return StrJoin<wchar_t>(++ptr);
}

void check_is_wallhaven(const wchar_t* pic, char* id)
{
    qout << 99;
    if (wcslen(pic) != 20)
        return ;
    if (!wcsncmp(pic, L"wallhaven-", 10) && StrContainCharInRanges<wchar_t>(pic+10, 6, L"a-z", L"0-9") &&
            (!wcscmp(pic+16, L".png") || !wcscmp(pic+16, L".jpg")))
    {
        for (int i=0; i <= 5; ++i)
        {
            id[i] = static_cast<char>(pic[10+i]);
        }
    }
}


void MenuWallpaper::startWork()                                                      //根据壁纸类型来判断执行哪个函数
{
    if (VarBox->RunApp && VarBox->CurPic != VarBox->PicHistory.end() && ++VarBox->CurPic != VarBox->PicHistory.end())
    {
        if ((VarBox->CurPic)->first)
        {
            if (VARBOX::PathFileExists(VarBox->CurPic->second))
            {
                if ((GetFileAttributesW(VarBox->CurPic->second) & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS))
                {
                    if (VARBOX::isOnline(false))
                    {
                        if (!VARBOX::OneDriveFile(VarBox->CurPic->second))
                        {
                            emit finished();
                            return;
                        }
                    }
                    else
                    {
                        emit msgBox("没有网络！", "提示");
                        emit finished();
                        return;
                    }
                }
                SystemParametersInfoW(SPI_SETDESKWALLPAPER, UINT(0), VarBox->CurPic->second, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
            }
        }
        else
        {
            if (VARBOX::PathFileExists(reinterpret_cast<char*>(VarBox->CurPic->second)))
            {
                if ((GetFileAttributesA(reinterpret_cast<char*>(VarBox->CurPic->second)) & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS))
                {
                    if (VARBOX::isOnline(false))
                    {
                        if (!VARBOX::OneDriveFile(reinterpret_cast<char*>(VarBox->CurPic->second)))
                        {
                            emit finished();
                            return;
                        }
                    }
                    else
                    {
                        emit msgBox("没有网络！", "提示");
                        emit finished();
                        return;
                    }
                }
                SystemParametersInfoA(SPI_SETDESKWALLPAPER, UINT(0), VarBox->CurPic->second, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
            }
        }
    }
    else if (VarBox->RunApp)
    {
        qout << "壁纸类型：" << VarBox->StandardNames[static_cast<int>(VarBox->PaperType)][1];
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
    set_from_Native(false);
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


void MenuWallpaper::previousPic()
{
    if (!thrd)
    {
        thrd = new std::thread([this](){
            for (int i=0;i<100;++i)
            {
                if (VarBox->CurPic == VarBox->PicHistory.begin())
                {
                    emit msgBox("无法找到更早的壁纸历史记录！", "提示");
                    emit finished();
                    return ;
                }
                if ((--VarBox->CurPic)->first)
                {
                    if (VARBOX::PathFileExists(VarBox->CurPic->second))
                    {
                        if ((GetFileAttributesW(VarBox->CurPic->second) & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS))
                        {
                            if (VARBOX::isOnline(false))
                            {
                                if (!VARBOX::OneDriveFile(VarBox->CurPic->second))
                                {
                                    emit finished();
                                    return;
                                }
                            }
                            else
                            {
                                emit msgBox("没有网络！", "提示");
                                emit finished();
                                return;
                            }
                        }
                        SystemParametersInfoW(SPI_SETDESKWALLPAPER, UINT(0), VarBox->CurPic->second, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
                        emit finished();
                        return;
                    }
                    delete [] VarBox->CurPic->second;
                }
                else
                {
                    if (VARBOX::PathFileExists(reinterpret_cast<char*>(VarBox->CurPic->second)))
                    {
                        if ((GetFileAttributesA(reinterpret_cast<char*>(VarBox->CurPic->second)) & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS))
                        {
                            if (VARBOX::isOnline(false))
                            {
                                if (!VARBOX::OneDriveFile(reinterpret_cast<char*>(VarBox->CurPic->second)))
                                {
                                    emit finished();
                                    return;
                                }
                            }
                            else
                            {
                                emit msgBox("没有网络！", "提示");
                                emit finished();
                                return;
                            }
                        }
                        SystemParametersInfoA(SPI_SETDESKWALLPAPER, UINT(0), VarBox->CurPic->second, SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
                        emit finished();
                        return;
                    }
                    delete [] reinterpret_cast<char*>(VarBox->CurPic->second);
                }
                VarBox->CurPic = VarBox->PicHistory.erase(VarBox->CurPic);
                if (VarBox->CurPic != VarBox->PicHistory.begin())
                    --VarBox->CurPic;
            }
            emit finished();
        });
    }
    else
        emit msgBox("和后台壁纸切换冲突，请稍后再试。", "提示");
}

void MenuWallpaper::removePic()
{
    if (!thrd)
    {
        thrd = new std::thread([this](){
            qout << "不喜欢该壁纸。";
            if (VarBox->PaperType == PAPER_TYPE::Bing && !VarBox->AutoRotationBingPicture)
            {
                emit msgBox("壁纸切换列表中没有替代方案，如果不喜欢此张壁纸，\n请切换壁纸类型或者启用必应壁纸轮换。", "提示");
                emit finished();
                return;
            }
            if (VarBox->CurPic->first)
            {
                const wchar_t* pic_path = VarBox->CurPic->second;
                const wchar_t* pic_name = get_file_name(pic_path);
                char id[7] = {0};
                check_is_wallhaven(pic_name, id);
                if (*id)
                {
                    QString str = VarBox->get_dat_path() + "\\Blacklist.json";
                    YJson *blackList = nullptr;
                    if (QFile::exists(str))
                    {
                        blackList = new YJson(str.toStdWString(), YJSON_ENCODE::UTF8BOM);
                        qout << 123;
                        if (blackList->getType() != YJSON_TYPE::YJSON_ARRAY)
                        {
                            qout << "不是Arry";
                            delete blackList;
                            blackList = new YJson(YJSON::ARRAY);
                        }
                    }
                    else
                        blackList = new YJson(YJSON::ARRAY);
                    blackList->append(id);
                    blackList->toFile(str.toStdWString(), YJSON_ENCODE::UTF8, true);
                    delete  blackList;
                }
                DeleteFileW(pic_path);
                delete [] pic_path;
                delete [] pic_name;
            }
            else
            {
                const char* pic_path = reinterpret_cast<const char*>(VarBox->CurPic->second);
                DeleteFileA(pic_path);
                delete [] pic_path;
            }
            VarBox->CurPic = VarBox->PicHistory.erase(VarBox->CurPic);
            if (VarBox->CurPic != VarBox->PicHistory.begin())
                --VarBox->CurPic;
            startWork();
            return;
        });
    }
    else
        emit msgBox("和后台壁纸切换冲突，请稍后再试。", "提示");
}
