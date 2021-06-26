#ifndef MENU_H
#define MENU_H

#include <QMenu>


//#define MENU_WIDTH 108

#if defined(Q_OS_WIN32)
//#define MENU_HEIGHT 306
#define MENU_WIDTH 92
#define MENU_HEIGHT 270
#elif defined(Q_OS_LINUX)
#define MENU_WIDTH 84
#define MENU_HEIGHT 225  //252
#endif

class Form;
class MenuWallpaper;

class Menu : public QMenu
{
	Q_OBJECT

public:
	explicit Menu(Form* form);
	~Menu();
	void new_show(int, int);

private:
	void initMenuConnect() const;        //初始化信号和槽的连接
	void initActions();
	void initUi();
	void moveMenuPos(int, int);          // 自动移动menu到合适位置，防止menu出现在屏幕之外。

	/*九个不同功能的Action*/
	QAction* autoStartAct;               //开机启动，可选Action
	QAction* translateAct;               //是否启用翻译，可选Action
	QAction* nextPaperAct;               //换一张壁纸
	QAction* settingDialogAct;           //打开设置对话框
	QAction* noSleepAct;                 //防止电脑休眠，可选Action
	QAction* openFolderAct;              //打开exe文件所在文件夹
	QAction* restartAct;                 //重启计算机
	QAction* shutdownAct;                //关机
	QAction* quitAct;                    //关闭程序

	Form* form;                          //主界面指针
	MenuWallpaper* wallpaper;            //用于壁纸更换
	QTimer* MouseMoveTimer;              //用于定时移动鼠标，防止息屏

	void setChecks();                    //读取自启状况，再决定是否选中AutoStartAct

private slots:
	void OpenFolder() const;             //打开程序所在文件夹
	void RestartComputer() const;        //重启电脑
	void ShutdownComputer() const;       //关闭电脑
	void setAutoStart(bool) const;       //设置是否自启
	void auto_move_cursor(bool);         //打开或关闭移动鼠标的计时器
	void auto_move_cursor() const;       //和计时器相连的槽函数，用于移动鼠标
};

#endif // MENU_H
