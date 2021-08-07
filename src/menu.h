#ifndef MENU_H
#define MENU_H


#define MENU_WIDTH 92
#define MENU_HEIGHT 270

#include <QMenu>
class QAction;
class Form;
class MenuWallpaper;

class Menu : public QMenu
{
	Q_OBJECT

protected:
    void showEvent(QShowEvent *event);

public:
    explicit Menu();
	~Menu();
    void Show(int, int);          // 自动移动menu到合适位置，防止menu出现在屏幕之外。

private:
	void initMenuConnect() const;        //初始化信号和槽的连接
	void initActions();
	void initUi();

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
