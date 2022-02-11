#ifndef FORM_H
#define FORM_H

#include "funcbox.h"

constexpr int FORM_WIDTH  = 92;
constexpr int FORM_HEIGHT = 40;

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
#ifdef Q_OS_WIN32
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
#endif
    void enterEvent(QEvent* event);
#else
#ifdef Q_OS_WIN32
    bool nativeEvent(const QByteArray &eventType, void *message, long long *result);
#endif
    void enterEvent(QEnterEvent* event);
#endif
      void leaveEvent(QEvent* event);
signals:
    void appQuit();
public:
	explicit Form(QWidget* parent = nullptr);
    ~Form();
    void keepInScreen();
    bool m_tieBianHide { true }, m_showToolTip { true };
    std::vector<class USBdriveHelper*> m_usbHelpers;

private:
    friend class Menu;                           //右键菜单可以访问私有数据
    friend void VARBOX::initChildren();
    friend class Dialog;
    class QFrame* frame { nullptr };
    class QStyleSheet *m_sheet { nullptr };
    QLabel *labUp, *labDown, *labMemory;
    void setupUi();
    void loadStyle();
    class NetSpeedHelper* netHelper;                   //设置对话
    QPoint m_ptPress;
    class Translater* translater { nullptr };            //翻译类，自带ui
	bool moved = false;
    void initSettings();
	void initConnects();                         //初始化连接

    class QPropertyAnimation* animation = nullptr;     //贴边隐藏动画效果
	void startAnimation(int width, int height);  // 隐藏/显示动画效果

public slots:
    void saveBoxPos();
    void set_wallpaper_fail(const char*);        //弹出消息和设置对话框
    void enableTranslater(bool);                 //启用翻译功能
};

#endif // FORM_H
