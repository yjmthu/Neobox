#ifndef DIALOG_H
#define DIALOG_H

#include <speedwidget.h>
#include "tray.h"

class DialogWallpaper;
class Form;
class QLineEdit;
class QPushButton;
class QButtonGroup;
class GMPOperateTip;

namespace Ui {
	class Dialog;
}

class Dialog : public SpeedWidget<QWidget>                            //壁纸设置界面
{
	Q_OBJECT

signals:
    void finished(bool, const char* str = nullptr);

protected:
    bool eventFilter(QObject *target, QEvent *event);       //事件过滤器
    void closeEvent(QCloseEvent*event);

public:
    explicit Dialog();
	~Dialog();

private:
    friend void del_file(Dialog *di ,QString str);
    Ui::Dialog* ui;
	QButtonGroup* buttonGroup;                           //单选按钮组合，互斥，添加按钮id
    short last_checked_button = 0;                       //上次选中的按钮的 id
	void checkSettings();                                //读取注册表
	void initChildren();
	void initUi();
	void initConnects();
    void initButtonFilter();
	void changeType(BOOL);

private slots:
    void rBtnNativeLinkWithToolBtn(bool checked);        //当“本地”单选按钮被选中时，下面的选择文件夹按钮可用，反之不可用。
    void chooseFolder();                                 //当选择文件夹按钮被点击时，弹出选择文件夹的对话框。
    void saveWallpaperSettings();                            //确定被按钮点击时，保存设置数据，退出对话框。
    void applyWallpaperSettings();                         //应用按钮点击时，保存设置数据，应用设置，最后退出对话框。
	void on_pBtnApply_2_clicked();
	void on_radioButton_7_clicked();
	void on_chkTimeUnit_min_clicked();
	void on_chkTimeUnit_sec_clicked();
	void on_radioButton_3_clicked();
	void on_radioButton_4_clicked();
	void on_radioButton_5_clicked();
	void on_radioButton_6_clicked();
	void on_radioButton_11_clicked();
    void cBxstandardNameCurIndex(int index);
    void sLdPageNumCurNum(int value);
    void sLdUpdateTimeCurNum(int value);
    void sLdTaskAlphCurNum(int value);
    void sLdIconAlphCurNum(int value);
	void on_radioButton_10_clicked();
	void on_radioButton_12_clicked();
	void on_radioButton_8_clicked();
    void on_pBtn_Save_Tran_Info_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_7_clicked();
    void on_radioButton_9_clicked();
	void on_pushButton_3_clicked();
    void openPicturePath();
    void on_pushButton_11_clicked();
    void linePictuerPathReturn();
    void setCustomName();
    void on_pushButton_14_clicked();
    void on_pushButton_4_clicked();
    void on_checkBox_4_clicked(bool checked);
    void on_radioButton_13_clicked();
    void on_radioButton_14_clicked();
    void on_pushButton_13_clicked();
    void on_pushButton_10_clicked();
    void on_pushButton_12_clicked();
    void on_toolButton_2_clicked();
    void on_pushButton_6_clicked();
};

#endif // DIALOG_H
