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
    QAction* const actions;
};

#endif // MENU_H
