#ifndef DIALOG_H
#define DIALOG_H

#include <speedwidget.h>

namespace Ui {
	class Dialog;
}

class Dialog : public SpeedWidget<QWidget>                            //壁纸设置界面
{
	Q_OBJECT

protected:
    bool eventFilter(QObject *target, QEvent *event);       //事件过滤器
    void closeEvent(QCloseEvent*event);

public:
    enum class Theme { White, Gray, Purple, Red, Green, Blue, Brown, Black };
    explicit Dialog();
	~Dialog();
    static Theme curTheme;

private:
    friend void del_file(Dialog *di ,QString str);
    Ui::Dialog* ui = nullptr;
    class QButtonGroup* buttonGroup { nullptr };                 //单选按钮组合，互斥，添加按钮id
    class Wallpaper *wallpaper { nullptr };
    class QStyleSheet *sheet { nullptr };
    class YJson* formFontJson { nullptr };
    std::array<QWidget*, 4> formPart;
	void checkSettings();                                //读取注册表
	void initChildren();
	void initUi();
    void loadFormStyle();
	void initConnects();
    void initButtonFilter();
    void setFrameStyle(int index);

private slots:
    void chooseFolder();                                 //当选择文件夹按钮被点击时，弹出选择文件夹的对话框。
    void saveWallpaperSettings();                            //确定被按钮点击时，保存设置数据，退出对话框。
    void applyWallpaperSettings();                         //应用按钮点击时，保存设置数据，应用设置，最后退出对话框。
    void setFormStyleSheet();
	void on_pBtnApply_2_clicked();
    void sLdPageNumCurNum(int value);
    void on_pushButton_5_clicked();
    void openPicturePath();
    void linePictuerPathReturn();
    void on_pushButton_4_clicked();
    void on_radioButton_13_clicked();
    void on_radioButton_14_clicked();
    void on_pushButton_13_clicked();
    void on_pushButton_10_clicked();
    void on_pushButton_12_clicked();
    void on_toolButton_2_clicked();
    void my_on_rBtnWallhavenApiUser_clicked();
    void my_on_rBtnBingApi_clicked();
    void my_on_rBtnOtherApi_clicked();
    void my_on_cBxApis_currentTextChanged(const QString &arg1);
    void my_on_rBtnWallhavenApiDefault_clicked();
};

#endif // DIALOG_H
