#ifndef MENU_H
#define MENU_H

#include <QMenu>
class QAction;
class Form;
class Wallpaper;

class Menu : public QMenu
{
	Q_OBJECT

protected:
    void showEvent(QShowEvent *event);

public:
    explicit Menu(QWidget* parent);
	~Menu();
    void Show(int, int);          // 自动移动menu到合适位置，防止menu出现在屏幕之外。

private:
    void initMenuConnect();        //初始化信号和槽的连接
	void initActions();
	void initUi();

	/*九个不同功能的Action*/
    QAction* calculateAct;               //科学计算
    QAction* translateAct;               //是否启用翻译，可选Action
    QAction* prevPaperAct;               //上一张壁纸
    QAction* nextPaperAct;               //下一张壁纸
    QAction* removePicAct;               //不喜欢这张图片
    QAction* settingDialogAct;           //打开设置对话框
    QAction* noSleepAct;                 //防止电脑休眠，可选Action
    QAction* openFolderAct;              //打开exe文件所在文件夹
    QAction* restartAct;                 //重启
    QAction* shutdownAct;                //关机
    QAction* quitAct;                    //关闭程序

private slots:
	void OpenFolder() const;             //打开程序所在文件夹
	void ShutdownComputer() const;       //关闭电脑
};

#endif // MENU_H
