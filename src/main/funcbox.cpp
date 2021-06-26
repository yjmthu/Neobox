#include "funcbox.h"
#include <fstream>
#include <QStandardPaths>

//#pragma comment(lib, "WinHttp.lib")
//#pragma comment(lib,"wininet.lib")

const char* VAR_BOX::PaperTypes[10] = { "Latest", "Hot", "Nature", "Anime", "Simple", "Random", "Bing", "Wallpapers", "Native", "Advance" };
QString VAR_BOX::FAMILY_NAMES[10] = { "Latest", "Hot", "Nature", "Anime", "Simple", "Random", "Bing", "Wallpapers", "Native", "Advance" };
QString VAR_BOX::FAMILY_PATH = NULL;
PAPER_TYPE VAR_BOX::PaperType = Latest;
COLOR_THEME VAR_BOX::cur_theme = ClassicWhite;
//bool VAR_BOX::FULL_SCRENN = false;
bool VAR_BOX::RUN_APP = true;
bool VAR_BOX::RUN_NET_CHECK = true;

bool VAR_BOX::AutoChange = false;
short VAR_BOX::PageNum = 1;
QString VAR_BOX::NativeDir = NULL;
QString VAR_BOX::UserCommand = NULL;
QString VAR_BOX::PATH_TO_OPEN = NULL;

int VAR_BOX::SCREEN_WIDTH = 0;
int VAR_BOX::SCREEN_HEIGHT = 0;

bool VAR_BOX::HAVE_APP_RIGHT = false;
bool VAR_BOX::ENABLE_TRANSLATER = false;
QString VAR_BOX::APP_ID = NULL;
QString VAR_BOX::PASS_WORD = NULL;

namespace FuncBox {

    bool getWebCode(const WCHAR* url, QString& src)
    {
        bool OK = false;
        HINTERNET hSession = InternetOpen(L"Firefox", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (hSession != NULL)
        {
            HINTERNET hURL = InternetOpenUrl(hSession, url, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
            if (hURL != NULL)
            {
                char Temp[1024] = {0};
                ULONG Number = 1;
                while (Number > 0)
                {
                    InternetReadFile(hURL, Temp, 1024 - 1, &Number);
                    src += Temp;
                }
                OK = !src.isEmpty();
                InternetCloseHandle(hURL);
                hURL = NULL;
            }

            InternetCloseHandle(hSession);
            hSession = NULL;
        }
        return OK;
    }

    bool getBingCode(QString &code)
    {
        bool OK = false;
        HINTERNET hSession = InternetOpen(L"Chromium", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (hSession)
        {
            HINTERNET hURL = InternetOpenUrl(hSession, L"https://cn.bing.com/", NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
            if (hURL)
            {
                char Temp[512] = {0};
                ULONG Number = 1;
                InternetReadFile(hURL, Temp, 511, &Number);
                code.append(Temp);
                OK = code.compare("");
                InternetCloseHandle(hURL);
                hURL = NULL;
            }
            InternetCloseHandle(hSession);
            hSession = NULL;
        }
        return OK;
    }

    bool getTransCode(const WCHAR* url,QString &outcome)
    {
        bool OK = false;
        HINTERNET hSession = InternetOpen(L"Microsoft Edge", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (hSession)
        {
            HINTERNET hURL = InternetOpenUrl(hSession, url, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
            if (hURL)
            {
                CHAR buffer[1025];
                DWORD dwRecv = 0;
                do {
                    ZeroMemory(buffer, 1025);
                    InternetReadFile(hURL, buffer, 1024, &dwRecv);
                    buffer[dwRecv] = '\0';
                    outcome += buffer;
                    qDebug() << "读入的字节数量：" << dwRecv;
                } while(dwRecv);
                OK = outcome.length();
                //qDebug() << outcome;
//                int a = outcome.indexOf("{"), b = outcome.lastIndexOf("}");
//                if (OK = (a!=b))
//                    outcome = outcome.mid(a,b+1);
//                else
//                    outcome.clear();
            }

            InternetCloseHandle(hSession);
            hSession = NULL;
        }
        return OK;
    }

    bool downloadImage(QString url, QString path)
    {
        if (QFile::exists(path))
            return 1;
        bool OK = false;
        HANDLE hFile = INVALID_HANDLE_VALUE;
        DWORD dwRecv = 0, dwSend = 0;
        int allWrite = 0;
        char szDownLoad[1024*40] = "0";
        D("开始下载：" + url + "到" + path);
        HINTERNET hSession = InternetOpen(L"Chromium", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (hSession)
        {
            HINTERNET hOpenUrl = InternetOpenUrl(hSession, url.toStdWString().c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
            if (hOpenUrl)
            {
                hFile = CreateFile(path.replace("/", "\\").toStdWString().c_str(), GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
                if (hFile != INVALID_HANDLE_VALUE)
                {
                    while (true)
                    {
                        Sleep(10);
                        InternetReadFile(hOpenUrl, szDownLoad, 1024*40, &dwRecv);
                        if (!dwRecv)
                            break;
                        WriteFile(hFile, szDownLoad, dwRecv, &dwSend, NULL);
                        allWrite += dwSend;
                        if (!dwSend)
                            break;
                    }
                    CloseHandle(hFile);
                    OK = (allWrite != 0) && (allWrite != 235);
                    if (!OK && QFile::exists(path))
                        QFile::remove(path);
                }
            }
        }
        return OK;
    }

    bool check_or_make_dir(QString str)
	{
		QDir dir;
		return dir.exists(str) || dir.mkdir(str);
	}

    QString get_son_dir(QString str)
	{
		QDir dir;
		dir.exists(str) || dir.mkdir(str);
		return str;
	}

    QString get_son_dir(const char t[])
	{
		return get_son_dir(qApp->applicationDirPath() + "/" + t);
	}

    void save_the_open_path()
	{
		QSettings IniWrite(get_ini_path(), QSettings::IniFormat);
        IniWrite.beginGroup("Dirs");
		IniWrite.setValue("OpenDir", VarBox.PATH_TO_OPEN);
        IniWrite.endGroup();
	}

	QString get_wal_path()
	{
		QDir dir;
		if (VarBox.FAMILY_PATH.isEmpty() || (!dir.exists(VarBox.FAMILY_PATH) && !dir.mkdir(VarBox.FAMILY_PATH)))
		{
			VarBox.FAMILY_PATH = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
		}
		QString paper_sub_dir = VarBox.FAMILY_PATH + "/" + VarBox.FAMILY_NAMES[7];
		if (VarBox.FAMILY_NAMES[7].isEmpty() || (!dir.exists(paper_sub_dir) && !dir.mkdir(paper_sub_dir)))
		{
			VarBox.FAMILY_NAMES[7] = VarBox.PaperTypes[7];
			paper_sub_dir = VarBox.FAMILY_PATH + "/" + VarBox.FAMILY_NAMES[7];
		}
		QSettings IniWrite(get_ini_path(), QSettings::IniFormat);
        IniWrite.beginGroup("Dirs");
		IniWrite.setValue("FamilyPath", VarBox.FAMILY_PATH);
        IniWrite.setValue(VarBox.PaperTypes[7], VarBox.FAMILY_NAMES[7]);
        IniWrite.endGroup();
		return paper_sub_dir;
	}

	QString get_pic_path(short i)
	{
		QDir dir;
		QString str = get_wal_path() + "/" + VarBox.FAMILY_NAMES[i];
		if (VarBox.FAMILY_NAMES[i].isEmpty() || (!dir.exists(str) && !dir.mkdir(str)))
		{
			VarBox.FAMILY_NAMES[i] = VarBox.PaperTypes[i];
			str = get_son_dir(VarBox.FAMILY_PATH + "/" + VarBox.FAMILY_NAMES[7] + "/" + VarBox.FAMILY_NAMES[i]);
		}

		QSettings IniWrite(get_ini_path(), QSettings::IniFormat);
        IniWrite.beginGroup("Dirs");
        IniWrite.setValue(VarBox.PaperTypes[i], VarBox.FAMILY_NAMES[i]);
        IniWrite.endGroup();

		return str;
	}

    QString get_ini_path()
	{
		return get_son_dir(get_dat_path() + "/ini") + "/config.ini";
	}

    QString get_dat_path()
	{
		return get_son_dir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/AppData/Local/SpeedBox");
	}

	bool check_app_right()
	{
		QString file_name = get_son_dir("/scripts") + "/appid.txt";
		if (QFile::exists(file_name))
		{
			QFile file(file_name);
			if (file.open(QIODevice::ReadOnly | QIODevice::Text))
			{
				QByteArray line = file.readLine();
				VarBox.APP_ID = QString(line).trimmed();
				if (!file.atEnd())
				{
					line = file.readLine();
					VarBox.PASS_WORD = QString(line).trimmed();
					file.close();
					qDebug() << "翻译信息" << VarBox.APP_ID << VarBox.PASS_WORD;
					return true;
				}
				file.close();
			}
		}
		return false;
	}

	bool build_init_files()
    {
        //QString abc;
        //getTransCode(L"http://api.fanyi.baidu.com/api/trans/vip/translate?q=English&from=zh&to=en&appid=20210503000812254&salt=1435660288&sign=a1b039c74a3e7b44ad981a1dae445f06", abc);
        QString curPath = get_son_dir("scripts");
		QString filePath = qApp->applicationFilePath();
		QFile fApp(filePath);
		fApp.link(curPath + "/SpeedBox.lnk");
        D("生成脚本文件中。");
			QFile f(curPath + "/SetWallPaper.sh");
		if (!f.exists())
		{
			if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
			{
                D("创建shell脚本文件失败。");
					return false;
			}
			QFile qss(":/scripts/scripts/SetWallPaper.sh");
			qss.open(QFile::ReadOnly);
			f.write(QString(qss.readAll()).replace("\n", "").toUtf8());
			qss.close();
			f.close();
		}
		QFile g(curPath + "/SpeedBox.desktop");
		if (!g.exists())
		{
			if (!g.open(QIODevice::WriteOnly | QIODevice::Text))
			{
                D("创建快捷方式失败。");
					return false;
			}
			QFile qss(":/scripts/scripts/SpeedBox.desktop");
			qss.open(QFile::ReadOnly);
			g.write(QString(qss.readAll()).replace("speedbox_path", filePath).replace("\n", "").toUtf8());
			qss.close();
			g.close();
		}
        D("脚本生成完毕。");

			QString fileName = get_ini_path();

		if (!QFile::exists(fileName))  //判断文件是否存在
		{
			VarBox.FAMILY_PATH = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
			VarBox.PATH_TO_OPEN = qApp->applicationDirPath();
			QSettings* IniWrite = new QSettings(fileName, QSettings::IniFormat);

            IniWrite->beginGroup("Wallpaper");
			IniWrite->setValue("PaperType", (int)VarBox.PaperType);
			IniWrite->setValue("TimeInerval", 10);
			IniWrite->setValue("PageNum", QString::number(1));
			IniWrite->setValue("setNative", false);
			IniWrite->setValue("AutoChange", VarBox.AutoChange);
			IniWrite->setValue("AutoHide", true);
			IniWrite->setValue("UserCommand", VarBox.UserCommand);
            IniWrite->setValue("NativeDir", VarBox.FAMILY_PATH);
            IniWrite->endGroup();

            IniWrite->beginGroup("Translate");
            IniWrite->setValue("EnableTranslater", VarBox.ENABLE_TRANSLATER);
            IniWrite->endGroup();

            IniWrite->beginGroup("UI");
            IniWrite->setValue("ColorTheme", VarBox.cur_theme);
            IniWrite->setValue("x", 100);
            IniWrite->setValue("y", 100);
            IniWrite->endGroup();

            IniWrite->beginGroup("Dirs");
            IniWrite->setValue("FamilyPath", VarBox.FAMILY_PATH);
            IniWrite->setValue("OpenDir", VarBox.PATH_TO_OPEN);
			for (short i = 0; i < 10; i++)
                IniWrite->setValue(VarBox.PaperTypes[i], VarBox.PaperTypes[i]);
            IniWrite->endGroup();
			delete IniWrite;
		}
		QSettings* IniRead = new QSettings(fileName, QSettings::IniFormat);

        IniRead->beginGroup("Translate");
		VarBox.ENABLE_TRANSLATER = IniRead->value("EnableTranslater").toBool();
        IniRead->endGroup();

        IniRead->beginGroup("Dirs");
        VarBox.PATH_TO_OPEN = IniRead->value("OpenDir").toString();
        VarBox.FAMILY_PATH = IniRead->value("FamilyPath").toString();
		for (short i = 0; i < 10; i++)
            VarBox.FAMILY_NAMES[i] = IniRead->value(VarBox.PaperTypes[i]).toString();
        IniRead->endGroup();

        IniRead->beginGroup("UI");
        VarBox.cur_theme = (COLOR_THEME)(IniRead->value("ColorTheme").toInt());
        IniRead->endGroup();

		delete IniRead;
		for (short i = 0; i < 7; i++)
			get_pic_path(i);
		if (!check_or_make_dir(VarBox.PATH_TO_OPEN))
		{
			VarBox.PATH_TO_OPEN = qApp->applicationDirPath();
			save_the_open_path();
		}
		VarBox.HAVE_APP_RIGHT = check_app_right();
		VarBox.ENABLE_TRANSLATER = VarBox.HAVE_APP_RIGHT && VarBox.ENABLE_TRANSLATER;
		return true;
	}

	void write_log(QString str, bool clean)
	{
//        qDebug() << str;
//		static int num = 0;
//		std::ofstream os("checkbug.log", clean ? (std::ios_base::out) : (std::ios_base::out | std::ios_base::app));
//		os << ((num ? ("step-" + QString("%1").arg(num, 4, 10, QLatin1Char('0')) + ": ") : QString("Date Time: ")) + str).toStdString() << std::endl;
//		num += 1;
//		os.close();
        qDebug() << str;
	}

	QString runCommand(QString program, QStringList argument, short line)
	{
		QProcess process;
		process.setProgram(program);
		process.setArguments(argument);
		process.start();
		process.waitForStarted(); //等待程序启动
		process.waitForFinished(5000);
		if (line) {
			for (int c = 1; c < line; c++)
				process.readLine();
			return process.readLine();
		}
		else return QString();
	}

//	template<typename T>bool getReplyData_Temp(QNetworkAccessManager* manager, QUrl url, T content)
//	{
//		bool success = false;
//		QEventLoop loop;
//		QNetworkRequest request;
//		request.setUrl(QUrl(url));
//		request.setRawHeader(                 //设置请求头
//			"User-Agent",
//			"Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko"
//		);
//		QNetworkReply* reply;
//		reply = manager->get(request);
//		QObject::connect(reply, &QNetworkReply::finished, &loop, [&loop, &success]() {success = true; loop.quit(); });
//        //QObject::connect(reply, &QNetworkReply::error, &loop, [&loop, &success]() {success = false; loop.quit(); });
//		loop.exec();
//		*content = reply->readAll();
//		return (success && (content->size()));  //如果请求失败或者请求结果的大小为0则返回 false.
//	}

//	bool getReplyData(QNetworkAccessManager* manager, QUrl url, QString* content)
//	{
//		return getReplyData_Temp(manager, url, content);
//	}
//	bool getReplyData(QNetworkAccessManager* manager, QUrl url, QByteArray* content)
//	{
//		return getReplyData_Temp(manager, url, content);
//	}

    bool isOnline(bool wait)
	{
        D("检测网络连接中。");
        DWORD flag;
		for (int c = 1; VarBox.RUN_NET_CHECK && VarBox.RUN_APP && (c <= (wait ? 30 : 1)); c++)
		{
            if (VarBox.RUN_NET_CHECK && VarBox.RUN_APP && InternetGetConnectedState(&flag, 0))
			{
                D("网络连接正常。");
					return true;
			}
			else if (VarBox.RUN_NET_CHECK && VarBox.RUN_APP)
				for (short j = 1; VarBox.RUN_NET_CHECK && VarBox.RUN_APP && (j <= 6); j++)
#if defined (Q_OS_WIN32)
					Sleep(500);
#elif defined (Q_OS_LINUX)
					sleep(0.5);
#endif
            D("再次检测网络连接。");
		}
        D("没有网络连接。");
			return false;
	}

}
