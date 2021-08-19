#ifndef DIALOG_H
#define DIALOG_H

#include <QWidget>
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

class Dialog : public QWidget                            //壁纸设置界面
{
	Q_OBJECT

signals:
    void finished(bool, const char* str = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    bool eventFilter(QObject *target, QEvent *event);       //事件过滤器
    void showEvent(QShowEvent *event);

public:
    explicit Dialog();
	~Dialog();

private:
    friend void del_file(Dialog *di ,QString str);
    Ui::Dialog* ui;
    DialogWallpaper* wallpaper;                          //壁纸处理类
	QTimer* change_paper_timer;                          //定时更换壁纸
    Tray* tray;
	QButtonGroup* buttonGroup;                           //单选按钮组合，互斥，添加按钮id
    GMPOperateTip *jobTip;
    bool mouse_moved = false;
    QPoint _startPos;                                    //记录鼠标
    QPoint _endPos;                                      //鼠标移动向量
    short last_checked_button = 0;                       //上次选中的按钮的 id
	void checkSettings();                                //读取注册表
    void setTheme();
	void initChildren();
	void initUi();
	void initConnects();
	void initBehaviors();
    void initButtonFilter();
	void changeType(BOOL);

private slots:
	void on_SliderTimeInterval_valueChanged(int value);  //滑块滑动时，后面的标签上的数字同时改变。
	void on_rBtnNative_toggled(bool checked);            //当“本地”单选按钮被选中时，下面的选择文件夹按钮可用，反之不可用。
	void on_BtnChooseFolder_clicked();                   //当选择文件夹按钮被点击时，弹出选择文件夹的对话框。
	void on_pBtnCancel_clicked();                        //取消按钮被点击时，不保存数据，直接退出设置对话框。
	void on_pBtnOk_clicked();                            //确定被按钮点击时，保存设置数据，退出对话框。
	void on_pBtnApply_clicked();                         //应用按钮点击时，保存设置数据，应用设置，最后退出对话框。
	void on_pBtnApply_2_clicked();
	void on_radioButton_7_clicked();
	void on_horizontalSlider_3_valueChanged(int value);
	void on_chkTimeUnit_min_clicked();
	void on_chkTimeUnit_sec_clicked();
	void on_radioButton_3_clicked();
	void on_radioButton_4_clicked();
	void on_radioButton_5_clicked();
	void on_radioButton_6_clicked();
	void on_horizontalSlider_2_valueChanged(int value);
	void on_radioButton_11_clicked();
	void on_comboBox_currentIndexChanged(int index);
    void on_SliderPageNum_valueChanged(int value);
    void on_horizontalSlider_valueChanged(int value);
	void on_radioButton_10_clicked();
	void on_radioButton_12_clicked();
	void on_radioButton_8_clicked();
    void on_pBtn_Save_Tran_Info_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_7_clicked();
    void on_radioButton_9_clicked();
	void on_radioButton_2_clicked();
	void on_pushButton_3_clicked();
    void on_pBtnOpenAppData_clicked();
	void on_pBtnOpenPicturePath_clicked();
    void on_pushButton_11_clicked();
    void on_linePictuerPath_returnPressed();
	void on_pushButton_clicked();
	void on_radioButton_clicked();
	void on_pBtnChange_clicked();
    void on_lineNewName_returnPressed();
    void on_pushButton_14_clicked();
    void on_pushButton_4_clicked();
    void on_toolButton_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_9_clicked();
    void on_radioButton_13_clicked();
    void on_radioButton_14_clicked();
    void on_pushButton_13_clicked();
    void on_pushButton_10_clicked();
    void on_pushButton_12_clicked();
    void on_checkBox_clicked(bool checked);
    void on_toolButton_2_clicked();
};

#endif // DIALOG_H
