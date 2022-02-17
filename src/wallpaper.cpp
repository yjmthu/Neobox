#include <cstdio>
#include <QDateTime>
#include <QSettings>
#include <QDir>
#include <QFile>
#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
#include <QNetworkConfigurationManager>
#else
#include <QNetworkInformation>
#endif
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QEventLoop>
#include <QStandardPaths>
#if defined (Q_OS_LINUX)
#include <unistd.h>
#elif defined (Q_OS_WINDOWS)
#include <io.h>
#endif

#include "funcbox.h"
#include "wallpaper.h"
#include "yencode.h"
#include "ystring.h"
#include "yjson.h"
#include "globalfn.h"

void check_is_wallhaven(const char* pic, char* id)
{
    if (strlen(pic) != 20)
        return ;
    if (!strncmp(pic, "wallhaven-", 10) && YString::StrContainCharInRanges<char>(pic+10, 6, "a-z", "0-9") &&
            (!strcmp(pic+16, ".png") || !strcmp(pic+16, ".jpg")))
    {
        for (int i=0; i < 20; ++i)
        {
            id[i] = static_cast<char>(pic[i]);
        }
    }
}

bool Wallpaper::set_wallpaper(const QString &file_path)             //根据路径设置壁纸
{
    if (QFile::exists(file_path))
    {
#if defined (Q_OS_WIN32)
        qout << "设置壁纸：" << file_path;
        m_picture_history.emplace_back(file_path.toStdString());
        std::wstring temp_utf16;
        YEncode::utf8_to_utf16LE<std::wstring&>(temp_utf16, (--(m_curpic = m_picture_history.end()))->c_str());
        return systemParametersInfo(temp_utf16);
#elif defined (Q_OS_LINUX)
        m_picture_history.push_back(file_path.toStdString());
        m_curpic = --m_picture_history.end();
        return systemParametersInfo(file_path.toStdString());
#endif
    }
    else
        return false;
}

Wallpaper::Wallpaper():
    _rd(), _gen(_rd()),
    m_doing(false), m_update_wallhaven_api(false), timer(new QTimer)
{
    loadWallpaperSettings();

    connect(this, &Wallpaper::msgBox, VarBox, std::bind(GlobalFn::msgBox, std::placeholders::_1, std::placeholders::_2, QMessageBox::Ok));
#if defined (Q_OS_WIN32)
    QSettings set("HKEY_CURRENT_USER\\Control Panel\\Desktop", QSettings::NativeFormat);
    if (set.contains("WallPaper"))
    {
        const QString& temp_paper = set.value("WallPaper").toString();
        if (!temp_paper.isEmpty()) {
            m_picture_history.emplace_back(temp_paper.toStdString());
        }
    }
#endif
    m_curpic = m_picture_history.begin();

    timer->setInterval(m_timeInterval * 60000);
    if (m_firstChange) {
        qout << "自动换已经开启，正在下载第一张壁纸。";
        if (isOnline(false)) {
            qout << "网络正常！";
            push_back();
        } else {
            qout << "网络异常，正在检测网络连接。";
            m_doing = true;
            auto thrd = QThread::create([=](){
                if (isOnline(true)) {
                    push_back();
                } else {
                    qout << "网络异常，放弃更换第一张壁纸。";
                }
            });
            connect(thrd, &QThread::finished, this, [=](){
                thrd->deleteLater();
                m_doing = false;
            });
            thrd->start();
        }
        if (m_autoSaveBingPicture && m_paperType != Type::Bing)
        {
            qout << "下载必应壁纸。";
            loadApiFile();
            set_from_Bing();
        }
        qout << "自动换结束。";
    } else {
        qout << "未开启自动更换壁纸。";
    }
    connect(timer, &QTimer::timeout, this, &Wallpaper::next);
    if (m_autoChange)
        timer->start();
    qout << "wallpaper初始化完毕";
}

Wallpaper::~Wallpaper()
{
    delete timer;
}

bool Wallpaper::systemParametersInfo(const std::string &path)
{
    std::wstring str;
    YEncode::utf8_to_utf16LE<std::wstring&>(str, path);
#if defined (Q_OS_WIN32)
        return ::SystemParametersInfoW(
            SPI_SETDESKWALLPAPER,
            UINT(0),
            const_cast<wchar_t *>(str.c_str()),
            SPIF_SENDCHANGE | SPIF_UPDATEINIFILE
        );
#elif defined (Q_OS_LINUX)
        return true;
#endif
}

bool Wallpaper::systemParametersInfo(const std::wstring &path)
{
#if defined (Q_OS_WIN32)
        return ::SystemParametersInfoW(
            SPI_SETDESKWALLPAPER,
            UINT(0),
            const_cast<wchar_t *>(path.c_str()),
            SPIF_SENDCHANGE | SPIF_UPDATEINIFILE
        );
#elif defined (Q_OS_LINUX)
        constexpr char script[] = "./Scripts/SetWallPaper.sh";
        QStringList lst1, lst2;
        lst1.append("+x");
        lst1.append("./Scripts/SetWallPaper.sh");
        lst2.append(path.c_str());
        VarBox->runCmd("chmod", lst1, 0);                    //赋予脚本执行权限。
        VarBox->runCmd(script, lst2, 0);                     //利用脚本更换壁纸，这只可以在 linux系统+kde桌面 上使用。
        return true;
#endif
}

void Wallpaper::_set_w(YJson* jsonArray)
{
    if (jsonArray->getChild())
    {
        int pic_num = jsonArray->getChildNum();
        std::uniform_int_distribution<int> dis(0, pic_num-1);
        YJson* item = jsonArray->find(dis(_gen));
        std::string pic_url = item->getValueString();
        jsonArray->getParent()->find("Used")->append(*item);
        jsonArray->remove(item);
        if (pic_url.length())
        {
            download_image(
                QString::fromStdString("https://w.wallhaven.cc/full/"+pic_url.substr(10, 2)+"/"+pic_url),
                QDir::toNativeSeparators(m_wallhaven_folder + "/" + QString::fromStdString(pic_url)),
                true
            );
        }
        jsonArray->getTop()->toFile("ImgData.json", YJson::UTF8BOM, true);
    } else {
        emit msgBox("当前页面没有找到图片, 请切换较小的页面或者更换壁纸类型!", "提示");
    }
    delete jsonArray->getTop();
}

void Wallpaper::_set_b(YJson * file_data)
{
    int curindex = file_data->find("current")->getValueInt();
    YJson* temp = file_data->find("images");
    qout << (bool)*temp;
    temp = temp->find(curindex);
    if (!temp)
    {
        temp = file_data->find("images")->getChild();
        file_data->find("current")->setValue(1);
    } else {
        file_data->find("current")->setValue(curindex+1);
    }
    std::string img_url("https://cn.bing.com");
    img_url += temp->find("url")->getValueString();
    QString bing_name;
    if (m_useDateAsBingName)
    {
        qout << "使用日期名称";
        bing_name = temp->find("enddate")->getValueString();
        bing_name.insert(4, '-');
        bing_name.insert(7, '-');
        bing_name = QDir::toNativeSeparators(m_bing_folder + "/" + bing_name + "必应壁纸.jpg");
    } else {
        qout << "使用CopyRight名称";
        bing_name = temp->find("copyright")->getValueString();
        bing_name = bing_name.mid(0, bing_name.indexOf(" (© "));
        bing_name = QDir::toNativeSeparators(m_bing_folder + "/" + bing_name + ".jpg");
    }
    file_data->toFile("BingData.json", YJson::UTF8BOM, true);
    download_image(QString::fromStdString(img_url), bing_name, m_paperType == Type::Bing);
}

void Wallpaper::push_back()
{
    qout << "push_back(), 壁纸类型：" << static_cast<int>(m_paperType);
    QDir dir;
    try {
        loadApiFile();
    } catch (std::string& errorStr) {
        qout << "错误原因:" << errorStr.c_str();
        return;
    }

    switch (m_paperType)
    {
    case Type::Bing:
        set_from_Bing();
        break;
    case Type::Other:
        if (dir.exists(m_other_folder) || dir.mkdir(m_other_folder))
            set_from_Other();
        else
            emit msgBox("壁纸存放文件夹不存在, 请手动创建!", "出错");
        break;
    case Type::Advance:
        set_from_Advance();
        break;
    case Type::Native:
        set_from_Native();
        break;
    case Type::User:
    default:
        if (dir.exists(m_wallhaven_folder) || dir.mkdir(m_wallhaven_folder))
            set_from_Wallhaven();
        else
            return emit msgBox("壁纸存放文件夹不存在, 请手动创建!", "出错");
        break;
    }
}

void Wallpaper::set_from_Wallhaven()  // 从数据库中随机抽取一个链接地址进行设置。
{
    qout << "Wallhaven 开始检查json文件";
    constexpr char file_name[] = "ImgData.json";
    std::string pic_url;
    YJson* jsonObject = nullptr, *jsonArray = nullptr, * find_item = nullptr;
    if (!QFile::exists(file_name))
        goto label_1;
    try {
        jsonObject = new YJson(file_name, YJson::AUTO);
    } catch (std::string& errorStr) {
        goto label_1;
    }
    if (jsonObject->getType() == YJson::Object &&
        jsonObject->find("Api") &&
        (find_item = jsonObject->find("PageNum")) &&
        find_item->getType() == YJson::Number &&
        find_item->getValueInt() == m_pageNum &&
        (jsonArray = jsonObject->find("ImgUrls")) &&
        jsonArray->getType() == YJson::Object)
    {
        if (m_update_wallhaven_api || m_wallhaven_api != jsonObject->find("Api")->getValueString())
        {
            qout << "找到json文件, 需要更新！";
            m_update_wallhaven_api = false;
            jsonObject->find("Api")->setText(m_wallhaven_api);
            jsonArray->find("Used")->clear();
            jsonArray->find("Unused")->clear();
            return get_url_from_Wallhaven(jsonArray);
        } else {
            if (jsonArray->empty())
            {
                qout << "找到json文件但是没有孩子！";
                jsonArray->append(YJson::Array, "Used");
                jsonArray->append(YJson::Array, "Unused");
                jsonArray->append(YJson::Array, "Balcklist");
                return get_url_from_Wallhaven(jsonArray);
            }
            else if (jsonArray->find("Unused")->empty())
            {
                qout << "Unused为空.";
                if (jsonArray->find("Used")->empty())
                {
                    qout << "Used为空.";
                    return get_url_from_Wallhaven(jsonArray);
                } else {
                    qout << "Unused正常.";
                    jsonArray = jsonArray->find("Used");
                }
            } else {
                jsonArray = jsonArray->find("Unused");
            }
            qout << "执行_set_w";
            return _set_w(jsonArray);
        }
    }
    qout << "json文件格式不正确！\n" << "Wallhaven 创建新的Json对象";
label_1:
    delete jsonObject;
    jsonObject = new YJson(YJson::Object);
    jsonObject->append(Wallpaper::m_wallhaven_api, "Api");
    jsonObject->append(m_pageNum, "PageNum");
    jsonArray = jsonObject->append(YJson::Object, "ImgUrls");
    jsonArray->append(YJson::Array, "Used");
    jsonArray->append(YJson::Array, "Blacklist");
    jsonArray->append(YJson::Array, "Unused");
    return get_url_from_Wallhaven(jsonArray);
}

void Wallpaper::get_url_from_Wallhaven(YJson* jsonArray)
{
    static int k;
    qout << "开始从wallhaven获取链接.";
    YJson* urllist =  jsonArray->find("Unused"), *blacklist = jsonArray->find("Blacklist");
    m_doing = true;
    auto mgr = new QNetworkAccessManager;
    k = int(5 * (m_pageNum - 1) + 1);
    connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep){
        if (rep->error() != QNetworkReply::NoError)
        {
            rep->deleteLater();
            delete jsonArray->getTop();
            mgr->deleteLater();
            m_doing = false;
            return;
        }
        const QByteArray& data = rep->readAll();
        rep->deleteLater();
        qout << "一轮壁纸链接请求结束";
        if (k <= 5 * m_pageNum)
        {
            YJson *js = new YJson(data);
            if (js->find("error"))  //{"error":"Not Found"}
            {
                delete js;
                mgr->deleteLater();
                m_doing = false;
                emit msgBox("该页面范围下没有壁纸, 请更换壁纸类型或者页面位置!", "出错");
                return ;
            }
            YJson* ptr = js->find("data")->getChild();
            const char* wn = nullptr;
            if (ptr)
            {
                do {
                    wn = ptr->find("path")->getValueString() + 31;
                    //qout << "后缀名:" << wn;
                    if (blacklist->findByVal(wn))
                    {
                        //qout << "在黑名单内";
                        continue;
                    }
                    urllist->append(wn);
                } while ((ptr = ptr->getNext()));
                QString str = QString::fromStdString(m_wallhaven_api + "&page=" + std::to_string(++k));
                mgr->get(QNetworkRequest(QUrl(str)));
            } else {
                mgr->deleteLater();
                m_doing = false;
                if (urllist->getChild())
                    _set_w(urllist);
                else
                {
                    delete jsonArray->getTop();
                    emit msgBox("该页面范围下没有壁纸, 请更换壁纸类型或者页面位置!", "出错");
                }
            }
            delete js;
        }
        else {
            if (urllist->getChild())
            {
                mgr->deleteLater();
                m_doing = false;
                _set_w(urllist);
            }
            else
            {
                delete jsonArray->getTop();
                mgr->deleteLater();
                m_doing = false;
                emit msgBox("该页面范围下没有壁纸, 请更换壁纸类型或者页面位置!", "出错");
            }
        }
    });
    mgr->get(QNetworkRequest(QUrl(QString::fromStdString(m_wallhaven_api + "&page=" + std::to_string(k)))));
}

void Wallpaper::get_url_from_Bing()
{
    qout << "获取必应链接";
    m_doing = true;
    auto mgr = new QNetworkAccessManager;
    QEventLoop* loop = new QEventLoop;
    connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep){
        if (rep->error() == QNetworkReply::NoError)
        {
            QByteArray && by = rep->readAll();
            rep->deleteLater();
            try {
                YJson bing_data(by);
                bing_data.append(0, "current");
                bing_data.append(QDateTime::currentDateTime().toString("yyyyMMdd").toStdString(), "today");
                mgr->deleteLater();
                m_doing = false;
                _set_b(&bing_data);
            } catch (std::string& errorStr) {
                qout << errorStr.c_str();
            }
        }
        rep->deleteLater();
        mgr->deleteLater();
        m_doing = false;
        loop->quit();
        loop->deleteLater();
    });
    mgr->get(QNetworkRequest(m_bing_api));
    loop->exec();
}

void Wallpaper::loadWallpaperSettings()
{
    qout << "读取壁纸信息";
    m_nativeDir = { QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)) };
    QSettings *IniRead = new QSettings("SpeedBox.ini", QSettings::IniFormat);
    IniRead->beginGroup("Wallpaper");
    m_nativeDir = IniRead->value("NativeDir", m_nativeDir).toString();
    unsigned safeEnum = IniRead->value("PaperType", static_cast<int>(m_paperType)).toInt();
    if (safeEnum > 9) safeEnum = 0;
    m_paperType = static_cast<Type>(safeEnum);
    m_timeInterval = IniRead->value("TimeInerval", m_timeInterval).toInt();
    m_pageNum = IniRead->value("PageNum", m_pageNum).toInt();
    m_autoChange = IniRead->value("AutoChange", m_autoChange).toBool();
#if defined (Q_OS_WIN32)
    m_userCommand = IniRead->value("UserCommand", QStringLiteral("python.exe -u \"X:\\xxxxx.py\"")).toString();
#elif defined (Q_OS_LINUX)
    m_userCommand = IniRead->value("UserCommand", "python.exe -u \"~/xxxxx.py\"").toString();
#endif
    m_useDateAsBingName = IniRead->value("UseDateAsBingName", m_useDateAsBingName).toBool();
    m_autoSaveBingPicture = IniRead->value("AutoSaveBingPicture", m_autoSaveBingPicture).toBool();
    m_firstChange = IniRead->value("FirstChange", m_firstChange).toBool();
    IniRead->endGroup();
    delete IniRead;
    qout << "读取壁纸信息完毕";
}

template <class _Ty>
void Wallpaper::download_image(const _Ty &url, const QString &path, bool set)
{
    qout << "壁纸网址：" << url << "  位置：" << path;
    if (!QFile::exists(path))
    {
        qout << "本地找不到文件, 直接开始下载";
        m_doing = true;
        auto mgr = new QNetworkAccessManager;
        QNetworkRequest netReq = QNetworkRequest(QUrl(url));
        netReq.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
        connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep)->void{
            if (rep->error() != QNetworkReply::NoError)
            {
                rep->deleteLater();
                qout << "下载图片出错！";
                mgr->deleteLater();
                m_doing = false;
                return;
            }
            const QByteArray& data = rep->readAll();
            rep->deleteLater();
            QFile file(path);
            qout << "下载图片成功!";
            if (data.size() && file.open(QIODevice::WriteOnly))
            {
                file.write(data);
                file.close();
                qout << file.size();
            }
            mgr->deleteLater();
            m_doing = false;
            if (set)
                set_wallpaper(path);
        });
        mgr->get(netReq);
    } else if (set) {
        qout << "本地图片文件存在";
        set_wallpaper(path);
    }
}

void Wallpaper::set_from_Native()
{
    QDir dir(m_nativeDir);
    if (!dir.exists()) return;
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.wbep";
    dir.setFilter(QDir::Files | QDir::NoSymLinks);                                //设置类型过滤器，只为文件格式
    int dir_count = dir.count();
    if (!dir_count) return;
    std::uniform_int_distribution<int> dis(0, dir_count - 1);
    QString file_name = dir[dis(_gen)];       //随机生成文件名称。
    file_name = QDir::toNativeSeparators(m_nativeDir + "/" + file_name);

    if (m_doing)
        return emit msgBox("当前正忙, 请稍后再试.", "提示");
    m_doing = true;
    auto thrd = QThread::create([=](){
        QFile f(file_name);
        char buffer;
        if (f.open(QIODevice::ReadOnly) && f.size() && f.read(&buffer, 1))
        {
            set_wallpaper(file_name);
        } else {
            emit setFailed("本地文件无效，请更换本地文件夹、改变壁纸类型或取消自动更换壁纸！");
        }
        return;
    });
    connect(thrd, &QThread::finished, this, [=](){
        thrd->deleteLater();
        m_doing = false;
    });
    thrd->start();
};


void Wallpaper::set_from_Bing()
{
    YJson *file_data = nullptr;
    if (!QFile::exists("BingData.json"))
        return get_url_from_Bing();
    qout << "必应文件存在";
    file_data = new YJson("BingData.json", YJson::AUTO);
    qout << "加载文件完成";
    if (QDateTime::currentDateTime().toString("yyyyMMdd") != file_data->find("today")->getValueString())
    {
        qout << "必应文件过期，将开始下载最新数据";
        delete file_data;
        return get_url_from_Bing();
    } else {
        qout << "必应文件为最新";
        _set_b(file_data);
        delete file_data;
    }
}

void Wallpaper::set_from_Other()
{
    download_image<QString>(
        QString::fromStdString(m_other_api),
        QDir::toNativeSeparators(m_other_folder + QDateTime::currentDateTime().toString("/" + m_other_name)),
        true
    );
}

QStringList _parse_arguments(const QString& str)
{
    QStringList lst;
    QChar d(' ');
    QString::const_iterator iter1 = str.constBegin(), iter=iter1;
    do {
        iter1 = std::find_if(iter, str.constEnd(), [](const QChar& c)->bool{ return c != QChar(' ');});
        if (iter == str.constEnd()) break;
        if (*iter1 == QChar('\"')) {
            d = '\"';
            ++iter1;
        } else {
            d = ' ';
        }
        iter = std::find(iter1, str.constEnd(), d);
        lst.push_back(QString(iter1, iter-iter1));
    } while (iter++ != str.constEnd());
    return lst;
}

void Wallpaper::set_from_Advance()
{
    if (m_userCommand.isEmpty()) return;
    qout << "高级命令开始" << m_userCommand;
    static std::string program_output;
    m_doing = true;
    auto thrd = QThread::create([=](){
        QStringList&& lst = _parse_arguments(m_userCommand);
        const QString program_file = lst[0];
        lst.removeFirst();
        if (applyClicked)
        {
            applyClicked = false;
            lst << "0";
        } else {
            lst << "1";
        }
        qout << "程序: " <<  program_file << "; 参数: " << lst;
        program_output = static_cast<const char*>(GlobalFn::runCmd(program_file, lst, 1));
        qout << "运行输出ansi: " << program_output.c_str();
    });
    connect(thrd, &QThread::finished, this, [=](){
        qout << "线程结束!" ;
        if (program_output.size())
        {
            for (auto i = program_output.size()-1; i > 0; --i)
            {
                if (strchr("\"\\\b\f\n\r\t\'", program_output[i]))
                {
                    program_output[i] = 0;
                }
                else
                {
                    qout << "开始设置壁纸";
                    FILE* f = fopen(program_output.c_str(), "rb");
                    if (program_output.size() && f)
                    {
                        fclose(f);
                        systemParametersInfo(program_output);
#ifdef Q_OS_WIN32
                        m_picture_history.emplace_back(GlobalFn::ansiToUtf8(program_output));
#elif defined (Q_OS_LINUX)
                        m_picture_history.emplace_back(program_output);
#endif
                        m_curpic = --m_picture_history.end();
                        program_output.clear();
                        thrd->deleteLater();
                        m_doing = false;
                        return ;
                    }
                    break;
                }
            }
        }
        thrd->deleteLater();
        m_doing = false;
    });
    thrd->start();
}

void Wallpaper::next()
{
    if (m_doing) return emit msgBox("频繁点击是没有效的哦！", "提示");
    qout << "下一张图片";
    if (++m_curpic < m_picture_history.end())
    {
        FILE* file = GlobalFn::readFile(*m_curpic);
        if (!file) return;
        m_doing = true;
        auto thrd = QThread::create([=](){
            char buffer;
            if (file && fread(&buffer, sizeof(char), 1, file) != 0) {
                systemParametersInfo(*m_curpic);
            } else {
                emit msgBox("没有网络！", "提示");
            }
            return;
        });
        connect(thrd, &QThread::finished, this, [=]() {
            fclose(file);
            thrd->deleteLater();
            m_doing = false;
        });
        thrd->start();
        return;
    } else {
        push_back();
    }
}

void Wallpaper::prev()
{
    if (m_doing) return emit msgBox("和后台壁纸切换冲突，请稍后再试。", "提示");
    m_doing = true;
    auto thrd = QThread::create([this](){
        for (int i=0; i<100; ++i) {
            if (m_curpic == m_picture_history.begin()) {
                emit msgBox("无法找到更早的壁纸历史记录！", "提示");
                return ;
            }
            FILE* file = GlobalFn::readFile(*--m_curpic);
            if (file) {
                char buffer;
                if (!fread(&buffer, sizeof(char), 1, file))
                {
                    fclose(file);
                    return ;
                }
                fclose(file);
                systemParametersInfo(*m_curpic);
                return;
            }
            m_curpic = m_picture_history.erase(m_curpic);
            if (m_curpic != m_picture_history.begin())
                --m_curpic;
        }
    });
    connect(thrd, &QThread::finished, this, [=](){
        thrd->deleteLater();
        m_doing = false;
    });
    thrd->start();
}

void Wallpaper::dislike()
{
    qout << "不喜欢该壁纸。";
    const char* pic_path = m_curpic->c_str();
    const char* pic_name = GlobalFn::getFileName(pic_path);
    char id[21] = { 0 };
    check_is_wallhaven(pic_name, id);
    if (*id)
    {
        YJson *json = new YJson("ImgData.json", YJson::AUTO);
        YJson *blacklist = json->find("ImgUrls")->find("Blacklist");
        if (!blacklist->findByVal(id))
            blacklist->append(id);
        YJson *black_id;
        if (black_id = json->find("ImgUrls")->find("Used")->findByVal(id), black_id)
            YJson::remove(black_id);
        if (black_id = json->find("ImgUrls")->find("Unused")->findByVal(id), black_id)
            YJson::remove(black_id);
        json->toFile("ImgData.json", YJson::UTF8BOM, true);
        delete  json;
    }
    if (!QFile::remove(pic_path))
        emit msgBox("删除文件失败!", "出错");
    m_curpic = m_picture_history.erase(m_curpic);
    if (m_curpic != m_picture_history.end())
    {
        if (m_doing)
            return emit msgBox("和后台壁纸切换冲突，请稍后再试。", "提示");
        FILE * file = GlobalFn::readFile(*m_curpic);
        if (!file) {
            m_curpic = m_picture_history.erase(m_curpic);
            return push_back();
        }
        m_doing = true;
        auto thrd = QThread::create([=](){
            char buffer;
            if (!fread(&buffer, sizeof(char), 1, file))
            {
                return ;
            }
            systemParametersInfo(*m_curpic);
        });
        connect(thrd, &QThread::finished, this, [=](){
            fclose(file);
            thrd->deleteLater();
            m_doing = false;
        });
        thrd->start();
        return;
    }
    push_back();
}

void Wallpaper::loadApiFile()
{
    YJson json("WallpaperApi.json", YJson::UTF8);
    const char* curApi = nullptr;
    int index = static_cast<int>(m_paperType);
    m_bing_api = QString::fromStdString(json["BingApi"]["Parameter"].urlEncode(json["MainApis"]["BingApi"].getValueString()));
    m_bing_folder = json["BingApi"]["Folder"].getValueString();
    switch (m_paperType)
    {
    case Type::Other:
        curApi = json["OtherApi"]["Curruent"].getValueString();
        m_other_api = json["OtherApi"]["ApiData"][curApi]["Url"].getValueString();
        m_other_folder = json["OtherApi"]["ApiData"][curApi]["Folder"].getValueString();
        m_other_name = json["OtherApi"]["ApiData"][curApi]["Name"].getValueString();
        break;
    case Type::Bing:
    case Type::Advance:
    case Type::Native:
        break;
    case Type::User:
        curApi = json["User"]["Curruent"].getValueString();
        m_wallhaven_api = json["User"]["ApiData"][curApi]["Parameter"].urlEncode(json["MainApis"]["WallhavenApi"].getValueString());
        m_wallhaven_folder = json["User"]["ApiData"][curApi]["Folder"].getValueString();
        break;
    default:
        m_wallhaven_api = json["Default"]["ApiData"][index]["Parameter"].urlEncode(json["MainApis"]["WallhavenApi"].getValueString());
        m_wallhaven_folder = json["Default"]["ApiData"][index]["Folder"].getValueString();
    }
}

bool Wallpaper::isOnline(bool wait)
{
    qout << "检查网络链接";
    QNetworkAccessManager *mgr = new QNetworkAccessManager;
    static bool success; success = false;
    QEventLoop* loop = new QEventLoop;
    QNetworkRequest request(QUrl("https://www.baidu.com"));
//    request.setRawHeader(                 //设置请求头
//        "User-Agent",
//        "Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko"
//    );
    connect(mgr, &QNetworkAccessManager::finished, loop, [loop](QNetworkReply *rep){
        success = rep->error() == QNetworkReply::NoError;
        rep->deleteLater();
        qout << "网络链接情况：" << success;
        loop->quit();
    });

    for (int c=1, n = (wait?30:1); c <= n; c++)
    {
        qout << "发送检测请求";
        mgr->get(request);
        loop->exec();
        if (success) {
            break;
        } else if (wait) {
            QThread::sleep(3);
        }
    }
    return success;
}
