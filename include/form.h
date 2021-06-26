#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QCoreApplication>
#include <QPropertyAnimation>
#include <QTimer>

#include "funcbox.h"
#include "translater.h"
#include "dialog.h"
#include "menu.h"

//#define FORM_WIDTH 140         //适用于1680x1900的屏幕
//#define FORM_HEIGHT 60         //适用于1680x1900的屏幕

#define FORM_WIDTH 100        //适用于2560x1600的屏幕
#define FORM_HEIGHT 40        //适用于2560x1600的屏幕

namespace Ui {
	class Form;
}

typedef ULONG(WINAPI* pfnGetAdaptersAddresses)(_In_ ULONG Family, _In_ ULONG Flags, _Reserved_ PVOID Reserved, _Out_writes_bytes_opt_(*SizePointer) PIP_ADAPTER_ADDRESSES AdapterAddresses, _Inout_ PULONG SizePointer);
typedef DWORD(WINAPI* pfnGetIfTable)(_Out_writes_bytes_opt_(*pdwSize) PMIB_IFTABLE pIfTable, _Inout_ PULONG pdwSize, _In_ BOOL bOrder);

class Form : public QWidget
{
	Q_OBJECT

protected:
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void mouseDoubleClickEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
    bool eventFilter(QObject *target, QEvent *event);       //事件过滤器
#if defined(Q_OS_WIN32)
	void enterEvent(QEvent* event);
	void leaveEvent(QEvent* event);
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
#endif

public:
	explicit Form(QWidget* parent = nullptr);
	~Form();

private:
	bool mouse_moved = false;                    //
	Ui::Form* ui;                                //ui指向悬浮窗界面
	friend class Menu;                           //右键菜单可以访问私有数据
	QPoint _startPos;                            //记录鼠标
	QPoint _endPos;                              //鼠标移动向量
	Menu* menu;                                  //右键菜单
	Dialog* dialog;                              //设置对话
	QTimer* monitor_timer;                       //每隔一秒钟刷新一次数据
	Translater* translater;                      //翻译类，自带ui
	bool moved = false;
	void initForm();                             //根据设置文件初始化悬浮窗
	void initConnects();                         //初始化连接
	void savePos(int x, int y);                  //保存悬浮窗位置
	void get_mem_usage();                        //读取内存占用率
	void get_net_usage();                        //读取网速
//    void get_net_speed();
#if defined(Q_OS_WIN32)
	QPropertyAnimation* animation;               //贴边隐藏动画效果
	void tb_hide(QEvent* event);                 //隐藏悬浮窗
	void tb_show(QEvent* event);                 //显示悬浮窗
	void startAnimation(int width, int height);  // 隐藏/显示动画效果

//	PMIB_IFTABLE m_pTable = NULL;                //指向缓冲区，存储MIB_IFTABLE结构（含有网速信息）
//    MIB_IFTABLE *mi;
//    PIP_ADAPTER_INFO ipinfo;
//	DWORD m_dwAdapters = 0;                      //缓冲区中字节的大小。
//	DWORD dwMISize = 0;
#endif

public slots:
	void msgBox(QString);                        //弹出消息
	void set_wallpaper_fail(QString);            //弹出消息和设置对话框

private slots:
	void updateInfo();                           //更新界面数据
	void enableTrans(bool);                      //启用翻译功能
};

#endif // FORM_H

#pragma once
