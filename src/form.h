#ifndef FORM_H
#define FORM_H

#include "funcbox.h"
#include "translater.h"
#include "dialog.h"
#include "menu.h"


#define FORM_WIDTH  92
#define FORM_HEIGHT 40

class QPropertyAnimation;
namespace Ui {
	class Form;
}

class Form : public QWidget
{
	Q_OBJECT

protected:
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
    bool eventFilter(QObject *target, QEvent *event);       //事件过滤器
    void enterEvent(QEnterEvent* event);
	void leaveEvent(QEvent* event);
    bool nativeEvent(const QByteArray &eventType, void *message, long long *result);

public:
	explicit Form(QWidget* parent = nullptr);
    ~Form();

private:
	Ui::Form* ui;                                //ui指向悬浮窗界面
    friend class Menu;  friend class Translater; //右键菜单可以访问私有数据
	QPoint _startPos;                            //记录鼠标
	QPoint _endPos;                              //鼠标移动向量
	Menu* menu;                                  //右键菜单
    Dialog* dialog;                              //设置对话
	QTimer* monitor_timer;                       //每隔一秒钟刷新一次数据
    Translater* translater;                      //翻译类，自带ui
	bool moved = false;
	void initForm();                             //根据设置文件初始化悬浮窗
	void initConnects();                         //初始化连接
	void get_mem_usage();                        //读取内存占用率
	void get_net_usage();                        //读取网速

	QPropertyAnimation* animation;               //贴边隐藏动画效果
    void tb_hide();                 //隐藏悬浮窗
    void tb_show();                 //显示悬浮窗
	void startAnimation(int width, int height);  // 隐藏/显示动画效果

public slots:
    void msgBox(const char*, const char*);           //弹出消息
    void set_wallpaper_fail(const char*);            //弹出消息和设置对话框

private slots:
	void updateInfo();                           //更新界面数据
	void enableTrans(bool);                      //启用翻译功能
    void savePos();
};

#endif // FORM_H
