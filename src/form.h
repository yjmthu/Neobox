#ifndef FORM_H
#define FORM_H

#include "funcbox.h"
#include "translater.h"
#include "dialog.h"
#include "menu.h"


#define FORM_WIDTH  92
#define FORM_HEIGHT 40

class QPropertyAnimation;
class DesktopMask;
class USBdriveHelper;
class NetSpeedHelper;
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
#if (QT_VERSION_CHECK(6,0,0) > QT_VERSION)
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
    void enterEvent(QEvent* event);
#else
    bool nativeEvent(const QByteArray &eventType, void *message, long long *result);
    void enterEvent(QEnterEvent* event);
#endif
      void leaveEvent(QEvent* event);
signals:
    void appQuit();
public:
	explicit Form(QWidget* parent = nullptr);
    ~Form();
    void keepInScreen();

private:
    friend class Menu;                           //右键菜单可以访问私有数据
    friend class FormSetting;
    friend void VARBOX::initChildren();
	Ui::Form* ui;                                //ui指向悬浮窗界面
    NetSpeedHelper* netHelper;                   //设置对话
	QPoint _startPos;                            //记录鼠标
    QPoint _endPos;                              //鼠标移动向
    Translater* translater = nullptr;            //翻译类，自带ui
	bool moved = false;
	void initForm();                             //根据设置文件初始化悬浮窗
	void initConnects();                         //初始化连接
    QTimer* MouseMoveTimer = nullptr;            //用于定时移动鼠标，防止息屏

    QPropertyAnimation* animation = nullptr;     //贴边隐藏动画效果
	void startAnimation(int width, int height);  // 隐藏/显示动画效果

public slots:
    void set_wallpaper_fail(const char*);        //弹出消息和设置对话框
    void enableTranslater(bool);                 //启用翻译功能
};

#endif // FORM_H
