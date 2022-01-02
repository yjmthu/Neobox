#include <string.h>

#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QStandardPaths>
#include <QMessageBox>
#include <QColorDialog>
#include <QSettings>
#include <QButtonGroup>
#include <QListView>
#include <QFileDialog>
#include <QTimer>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QTextCodec>

#include "YString.h"
#include "YJson.h"
#include "dialog.h"
#include "ui_dialog.h"
#include "form.h"
#include "blankform.h"
#include "gmpoperatetip.h"
#include "downloadprogress.h"
#include "wallpaper.h"
#include "bingsetting.h"
#include "formsetting.h"
#include "desktopmask.h"

#include "explaindialog.h"

constexpr const char *color_theme[8] =
{
    "255,255,255,255",
    "255,255,255,150",
    "155,89,182,150",
    "255,10,10,150",
    "50,200,50,150",
    "135,160,250,150",
    "200,100,100,150",
    "0,0,0,100"
};

constexpr const char *reg_keys[4] = {
    "HKEY_CURRENT_USER\\SOFTWARE\\Classes\\*\\shell\\QCoper",
    "HKEY_CURRENT_USER\\SOFTWARE\\Classes\\Directory\\shell\\QCoper",
    "HKEY_CURRENT_USER\\SOFTWARE\\Classes\\Directory\\Background\\shell\\QCoper",
    "mshta vbscript:clipboarddata.setdata(\"text\",\"%%1\")(close)"
};

Dialog::Dialog():
    SpeedWidget<QWidget>(nullptr),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);                                   // 一定要在下面这句之前，否则会出现问题。
    initChildren();
    initUi();
	initConnects();
    initButtonFilter();
}

Dialog::~Dialog()
{
    qout << "析构dialog开始";
    delete buttonGroup;
    delete ui;
    qout << "析构dialog结束";
}

void Dialog::initUi()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);  //| Qt::WindowStaysOnTopHint
    setAttribute(Qt::WA_DeleteOnClose, true);
    ui->pushButton_12->setEnabled(false);
    QFile qss(":/qss/dialog_style.qss");
    qss.open(QFile::ReadOnly);
    setStyleSheet(QString(qss.readAll()));
    qss.close();
    initSpeedBox(ui->frame, &Dialog::showMinimized, &Dialog::close);
    buttonGroup->addButton(ui->rBtnHot, (int)PAPER_TYPE::Hot);         //将按钮添加到按钮组合，同时设置按钮 id
    buttonGroup->addButton(ui->rBtnNature, (int)PAPER_TYPE::Nature);
    buttonGroup->addButton(ui->rBtnAnime, (int)PAPER_TYPE::Anime);
    buttonGroup->addButton(ui->rBtnSimple, (int)PAPER_TYPE::Simple);
    buttonGroup->addButton(ui->rBtnUser, (int)PAPER_TYPE::User);
    buttonGroup->addButton(ui->rBtnRandom, (int)PAPER_TYPE::Random);
    buttonGroup->addButton(ui->rBtnBing, (int)PAPER_TYPE::Bing);
    buttonGroup->addButton(ui->rBtnOther, (int)PAPER_TYPE::Other);
    buttonGroup->addButton(ui->rBtnNative, (int)PAPER_TYPE::Native);
    buttonGroup->addButton(ui->rBtnAdvance, (int)PAPER_TYPE::Advance);
	buttonGroup->setExclusive(true);                     // 按钮之间相互排斥
    buttonGroup->button(static_cast<int>(VarBox->PaperType))->setChecked(true);
    ui->lineAppData->setText(QDir::currentPath().replace("/", "\\"));

    ui->checkBox_2->setChecked(false);
    ui->LinePath->setText(VarBox->NativeDir);
    ui->sLdPageNum->setValue(VarBox->PageNum);
    ui->sLdTimeInterval->setValue(VarBox->TimeInterval);
    ui->labTimeInterval->setText(QString::number(VarBox->TimeInterval));
    ui->chkEnableChangePaper->setChecked(VarBox->AutoChange);
    ui->usrCmd->setText(VarBox->UserCommand);

    ui->line_APP_ID->setText(VarBox->AppId);
    ui->line_PASS_WORD->setText(VarBox->PassWord);
    ui->cBxAutoStart->setChecked(!QSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat).value("SpeedBox").toString().compare(qApp->applicationFilePath().replace("/", "\\")));
    ui->lineEdit_2->setText(qApp->applicationDirPath().replace("/", "\\"));
    switch (VarBox->PaperType) {
    case PAPER_TYPE::Bing:
        ui->rBtnBingApi->setChecked(true);
        my_on_rBtnBingApi_clicked();
        break;
    case PAPER_TYPE::User:
        ui->rBtnWallhavenApiUser->setChecked(true);
        my_on_rBtnWallhavenApiUser_clicked();
        break;
    case PAPER_TYPE::Other:
        ui->rBtnOtherApi->setChecked(true);
        my_on_rBtnOtherApi_clicked();
        break;
    default:
        ui->rBtnWallhavenApiDefault->setChecked(true);
        my_on_rBtnWallhavenApiDefault_clicked();
    }
    if (QSettings(TASK_DESK_SUB, QSettings::NativeFormat).contains(TASKBAR_ACRYLIC_OPACITY))
        switch (QSettings(TASK_DESK_SUB, QSettings::NativeFormat).value(TASKBAR_ACRYLIC_OPACITY).toInt())
        {
        case 10:                                              // 不透明
            ui->radioButton_11->setChecked(true);
            break;
        case 5:                                               // 半透明
            ui->radioButton_10->setChecked(true);
            break;
        case 0:                                               // 全透明
            ui->radioButton_12->setChecked(true);
            break;
        default:                                              // 默认不透明
            ui->radioButton_11->setChecked(true);
            break;
        }
    else
        ui->radioButton_11->setChecked(true);                 // 默认不透明
    if (VarBox->PathToOpen.compare(qApp->applicationDirPath().replace("/", "\\")))
        ui->radioButton_14->setChecked(true);
    else
        ui->radioButton_13->setChecked(true);
    ui->pushButton_4->setText("确定");
    ui->comboBox_3->setCurrentIndex((int)VarBox->CurTheme);
    ui->lineEdit->setText(VarBox->PathToOpen);
    ui->checkBox_3->setChecked(VarBox->FirstChange);
    ui->BtnChooseFolder->setEnabled(ui->rBtnNative->isChecked());
    ui->checkBox_4->setChecked(VarBox->ControlDesktopIcon);
    ui->checkBox->setChecked(VarBox->enableUSBhelper);
    ui->frame->setStyleSheet(QString("QFrame{background-color:rgba(%1);}QLabel{border-radius: 3px;background-color: transparent;}Line{background-color:black};").arg(color_theme[static_cast<int>(VarBox->CurTheme)]));
    checkSettings();
    move((VarBox->ScreenWidth - width()) / 2, (VarBox->ScreenHeight - height()) / 2);
}

void Dialog::initChildren()
{
	buttonGroup = new QButtonGroup;
}

void Dialog::initConnects()
{
    qout << "对话框链接A";
    connect(ui->pushButton_14, &QPushButton::clicked, this, [](){
        QDesktopServices::openUrl(QUrl("https://yjmthu.github.io/Speed-Box"));
    });
    connect(ui->rBtnNative, &QRadioButton::toggled, this, [this](bool checked){ui->BtnChooseFolder->setEnabled(checked);});
    connect(ui->BtnChooseFolder, &QToolButton::clicked, this, &Dialog::chooseFolder);
    connect(ui->pBtnCancel, &QPushButton::clicked, this, &Dialog::close);
    connect(ui->pBtnOk, &QPushButton::clicked, this, &Dialog::saveWallpaperSettings);
    connect(ui->pBtnOpenAppData, &QPushButton::clicked, VarBox, [](){
        qout << QDir::currentPath();
        VarBox->openDirectory(QDir::currentPath());
    });
    connect(ui->pBtnOpenPicturePath, &QPushButton::clicked, this, &Dialog::openPicturePath);
    connect(ui->linePictuerPath, &QLineEdit::returnPressed, this, &Dialog::linePictuerPathReturn);
    connect(ui->cBxAutoStart, &QPushButton::clicked, this, [this](bool checked){
        QSettings* reg = new QSettings(
            "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
            QSettings::NativeFormat);
        if (checked)
            reg->setValue("SpeedBox", qApp->applicationFilePath().replace("/", "\\"));
        else
            reg->remove("SpeedBox");
        delete reg;
        jobTip->showTip("修改成功！");
    });
    connect(ui->tBtnSetBing, &QToolButton::clicked, this, [this](){
        BingSetting x;
        x.move(frameGeometry().x()+(width()-x.width())/2, frameGeometry().y()+(height()-x.height())/2);
        x.exec();
    });
    connect(ui->pBtnApply, &QPushButton::clicked, this, &Dialog::applyWallpaperSettings);
    connect(ui->sLdPageNum, &QSlider::valueChanged, this, &Dialog::sLdPageNumCurNum);
    connect(ui->sLdTimeInterval, &QSlider::valueChanged, this, [this](int value){ui->labTimeInterval->setText(QString::number(value, 10));});
    connect(ui->rBtnWallhavenApiDefault, &QRadioButton::clicked, this, &Dialog::my_on_rBtnWallhavenApiDefault_clicked);
    connect(ui->rBtnWallhavenApiUser, &QRadioButton::clicked, this, &Dialog::my_on_rBtnWallhavenApiUser_clicked);
    connect(ui->rBtnBingApi, &QRadioButton::clicked, this, &Dialog::my_on_rBtnBingApi_clicked);
    connect(ui->rBtnOtherApi, &QRadioButton::clicked, this, &Dialog::my_on_rBtnOtherApi_clicked);
    connect(ui->cBxApis, &QComboBox::currentTextChanged, this, &Dialog::my_on_cBxApis_currentTextChanged);
    connect(ui->pushButton, &QPushButton::clicked, VarBox, [](){
        VarBox->openDirectory(qApp->applicationDirPath());
    });
    qout << "对话框链接B";
}

void Dialog::initButtonFilter()
{
    QList<QSlider *> sliders = findChildren<QSlider *>();
    QList<QSlider *>::iterator sliderList = sliders.begin();
    for (sliderList = sliders.begin(); sliderList != sliders.end(); sliderList++)
    {
        (*sliderList)->installEventFilter(this);
    }
    QList<QComboBox *> combs = findChildren<QComboBox *>();
    QList<QComboBox *>::iterator combList = combs.begin();
    for (combList = combs.begin(); combList != combs.end(); combList++)
    {
        (*combList)->installEventFilter(this);
        (*combList)->setView(new QListView());
    }
}

bool Dialog::eventFilter(QObject* target, QEvent* event)
{
    if (/*typeid (*target) == typeid (QPushButton) || */typeid (*target) == typeid (QComboBox))
    {
        if ((QEvent::Type)(event->type()) == QEvent::MouseMove)
            return true;
    }
    else if (typeid (*target) == typeid (QSlider))
    {
        QMouseEvent *env = static_cast<QMouseEvent *>(event);
        QSlider *tar = static_cast<QSlider *>(target);
        //static bool clicked = false;
        switch ((QEvent::Type)(env->type()))
        {
        case QEvent::MouseMove:
            tar->setValue(((double)env->pos().x() / (double)tar->width() * (tar->maximum() - tar->minimum())) + tar->minimum());
            return true;
        case QEvent::MouseButtonPress:
            tar->setValue(((double)env->pos().x() / (double)tar->width() * (tar->maximum() - tar->minimum())) + tar->minimum());
            tar->setMouseTracking(true);
            return true;
        case QEvent::MouseButtonRelease:
            tar->setMouseTracking(false);
            return true;
        default:
            break;
        }
    }
    return QWidget::eventFilter(target, event);
}

void Dialog::closeEvent(QCloseEvent *event)
{
    *const_cast<Dialog**>(&(VarBox->dialog)) = nullptr;
    event->accept();
}

void Dialog::chooseFolder()
{
	QDir d;
    if (VarBox->NativeDir.isEmpty() || (!d.exists(VarBox->NativeDir) && !d.mkdir(VarBox->NativeDir)))
	{
        VarBox->NativeDir = QDir::toNativeSeparators(QStandardPaths::writableLocation((QStandardPaths::PicturesLocation)));
        VarBox->sigleSave("Wallpaper", "NativeDir", VarBox->NativeDir);
        ui->LinePath->setText(VarBox->NativeDir);
	}
	QString titile = "请选择一个文件夹";
    QString dir = QFileDialog::getExistingDirectory(NULL, titile, VarBox->NativeDir, QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty())
        ui->LinePath->setText(QDir::toNativeSeparators(dir));
}

void Dialog::sLdPageNumCurNum(int value)
{
	ui->label_2->setText(QString::number(value, 10));
}

void Dialog::saveWallpaperSettings()
{
    QSettings IniWrite("SpeedBox.ini", QSettings::IniFormat);
    IniWrite.setIniCodec(QTextCodec::codecForName("UTF-8"));
    IniWrite.beginGroup("Wallpaper");
    IniWrite.setValue("PaperType", buttonGroup->checkedId());
    IniWrite.setValue("NativeDir", ui->LinePath->text());
    IniWrite.setValue("PageNum", ui->sLdPageNum->value());
    IniWrite.setValue("TimeInerval", ui->sLdTimeInterval->value());
    IniWrite.setValue("AutoChange", ui->chkEnableChangePaper->isChecked());
    IniWrite.setValue("UserCommand", ui->usrCmd->text());
    IniWrite.setValue("FirstChange", ui->checkBox_3->isChecked());
    IniWrite.endGroup();
    jobTip->showTip("保存成功！");
}

void Dialog::applyWallpaperSettings()
{
    VarBox->wallpaper->update = ui->checkBox_2->isChecked();
    VarBox->PaperType = (PAPER_TYPE)buttonGroup->checkedId();
    VarBox->NativeDir = ui->LinePath->text();
    VarBox->PageNum = ui->sLdPageNum->value();
    VarBox->TimeInterval = ui->sLdTimeInterval->value();
    VarBox->AutoChange = ui->chkEnableChangePaper->isChecked();
    VarBox->UserCommand = ui->usrCmd->text();
    VarBox->FirstChange = ui->checkBox_3->isChecked();
    for (int type = 0; type < 2; ++type)
    {
        if (type == 0)
        {
            if (VarBox->wallpaper->isActive())
            {
                if (QMessageBox::question(this, "警告", "您之前的壁纸类型更换正在生效中，是否终止？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                {
                    VarBox->wallpaper->kill();
                    if (QMessageBox::question(this, "提示", "终止成功！是否继续？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                        continue;
                }
                break;
            }
        }
        else if (type == 1)
        {
            if (VarBox->FirstChange)
            {
                qout << "首更1";
                VarBox->wallpaper->applyClicked = true;
                VarBox->wallpaper->push_back();
                qout << "首更";
            }
            if (VarBox->AutoChange)
            {
                VarBox->wallpaper->timer->setInterval(VarBox->TimeInterval * 60000);  // 设置时间间隔,Timer的单位是毫秒
                VarBox->wallpaper->timer->start();
                qout << "开始更换壁纸";
            }
            else
                VarBox->wallpaper->timer->stop();
            if (QMessageBox::question(this, "提示", "应用成功！是否保存设置？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                saveWallpaperSettings();
        }
    }
}

void Dialog::checkSettings()
{
    ui->chkFile->setChecked(QSettings(reg_keys[0], QSettings::NativeFormat).contains("."));
    ui->chkFolder->setChecked(QSettings(reg_keys[1], QSettings::NativeFormat).contains("."));
    ui->chkFolderBack->setChecked(QSettings(reg_keys[2], QSettings::NativeFormat).contains("."));
    ui->chkTimeUnit_sec->setChecked(QSettings(TASK_DESK_SUB, QSettings::NativeFormat).contains(SHOW_SECONDS_IN_SYSTEM_CLOCK));

}

void Dialog::on_pBtnApply_2_clicked()
{
    static QString icon_path = "Copy.ico";
	QFile file(icon_path);
	if (!file.exists() && file.open(QIODevice::WriteOnly))
	{
        QFile qss(":/icons/copy.ico");
		qss.open(QFile::ReadOnly);
		file.write(qss.readAll());
		qss.close();
		file.close();
	}
    const QCheckBox* box[3] = {ui->chkFile, ui->chkFolder, ui->chkFolderBack}; short type = -1;
    while (++type != 3)
    {
        if (box[type]->isChecked())
        {
            qout << "创建键:" << type;
            QSettings settings(reg_keys[type], QSettings::NativeFormat);
            if (!settings.contains("."))
            {
                settings.setValue(".", QString("复制路径"));
                settings.setValue("Icon", QDir().absoluteFilePath(icon_path).replace("/", "\\"));
                settings.beginGroup("command");
                settings.setValue(".", QString(reg_keys[3]).arg(type==2?'V':'1'));
                settings.endGroup();
            }
        }
        else
        {
            qout << "删除键:" << type;
            QSettings settings(reg_keys[type], QSettings::NativeFormat);
            if (settings.contains("."))
            {
//                if (settings.contains("command"))
//                    settings.remove("command");
                settings.clear();
            }
        }
    }
    jobTip->showTip("设置成功！");
}

void Dialog::openPicturePath()
{
	QString str = ui->linePictuerPath->text();
	QDir dir;
    if (dir.exists(str) || (!str.isEmpty() && dir.mkdir(str)))
        VarBox->wallpaper->image_path = str.replace("/", "\\");
	else
	{
        if (dir.exists(VarBox->wallpaper->image_path) || dir.mkdir(VarBox->wallpaper->image_path))
            ui->linePictuerPath->setText(VarBox->wallpaper->image_path.replace("/", "\\"));
		else
            VarBox->wallpaper->image_path = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
	}
    if (dir.exists(VarBox->wallpaper->image_path))
        VarBox->openDirectory(VarBox->wallpaper->image_path);
	else
		QMessageBox::warning(this, "警告", "路径不存在！", QMessageBox::Ok, QMessageBox::Ok);
}

void Dialog::linePictuerPathReturn()
{
	QDir dir;
	if (dir.exists(ui->linePictuerPath->text()))
	{
        VarBox->wallpaper->image_path = ui->linePictuerPath->text();
	}
	else
	{
		QMessageBox::warning(this, "警告", "路径不存在！", QMessageBox::Ok, QMessageBox::Ok);
        ui->linePictuerPath->setText(VarBox->wallpaper->image_path);
	}
}

void Dialog::on_chkTimeUnit_min_clicked()
{
    QSettings settings(TASK_DESK_SUB, QSettings::NativeFormat);
    if (settings.contains(SHOW_SECONDS_IN_SYSTEM_CLOCK))
        settings.remove(SHOW_SECONDS_IN_SYSTEM_CLOCK);
    jobTip->showTip("更改成功！");
}

void Dialog::on_chkTimeUnit_sec_clicked()
{
    QSettings settings(TASK_DESK_SUB, QSettings::NativeFormat);
    if (!settings.contains(SHOW_SECONDS_IN_SYSTEM_CLOCK))
    {
        settings.setValue(SHOW_SECONDS_IN_SYSTEM_CLOCK, 1);
    }
    jobTip->showTip("更改成功！");
}

void Dialog::on_radioButton_11_clicked()
{
    QSettings settings(TASK_DESK_SUB, QSettings::NativeFormat);
    settings.setValue(TASKBAR_ACRYLIC_OPACITY, 10);
    jobTip->showTip("更改成功！");
}

void Dialog::on_radioButton_10_clicked()
{
    QSettings settings(TASK_DESK_SUB, QSettings::NativeFormat);
    settings.setValue(TASKBAR_ACRYLIC_OPACITY, 5);
    jobTip->showTip("更改成功！");
}

void Dialog::on_radioButton_12_clicked()
{
    QSettings settings(TASK_DESK_SUB, QSettings::NativeFormat);
    settings.setValue(TASKBAR_ACRYLIC_OPACITY, 0);
    jobTip->showTip("更改成功！");
}

void Dialog::on_radioButton_13_clicked()
{
    ui->lineEdit->setText(qApp->applicationDirPath().replace("/", "\\"));
    ui->pushButton_4->setText("确定");
}


void Dialog::on_radioButton_14_clicked()
{
    ui->lineEdit->setText("点击右侧按钮选择文件");
    ui->pushButton_4->setText("选择");
}

void Dialog::on_pushButton_4_clicked()
{
    if (ui->pushButton_4->text().compare("确定"))
	{
		QString titile = "请选择一个文件夹";
        QString dir = QFileDialog::getExistingDirectory(ui->frame, titile, QStandardPaths::writableLocation(QStandardPaths::HomeLocation), QFileDialog::ShowDirsOnly);
        ui->pushButton_4->setText("确定");
        if (!dir.isEmpty())
		{
            ui->lineEdit->setText(dir.replace("/", "\\"));
		}
        return;
	}
    VarBox->PathToOpen = ui->lineEdit->text();
    QDir().mkdir(VarBox->PathToOpen);
    VarBox->sigleSave("Dirs", "OpenDir", VarBox->PathToOpen);
    jobTip->showTip("更换路径成功！");
}

void Dialog::on_pBtn_Save_Tran_Info_clicked()
{
    QString str_1 = ui->line_APP_ID->text().trimmed();
    QString str_2 = ui->line_PASS_WORD->text().trimmed();
    if (str_1.isEmpty() || str_2.isEmpty())
        QMessageBox::information(ui->frame, "提示", "APP ID或密钥不能为空！");
    else
    {
        QFile file("AppId.txt");
        if (file.open(QIODevice::WriteOnly))
        {
            if (VarBox->AppId) delete [] VarBox->AppId;
            if (VarBox->PassWord) delete [] VarBox->PassWord;
            VarBox->HaveAppRight = true;
            qout << 1;
            unsigned char c[3] = {0xEF, 0xBB, 0xBF}; size_t len = 0;
            file.write((char*)c, 3 * sizeof (char));
            file.write("APP ID\t", 7 * sizeof (char));
            QByteArray s1 = str_1.toUtf8(); len = s1.length();
            qout << 2;
            VarBox->AppId = new char[len+1]; memcpy_s(VarBox->AppId, len, s1, len);
            VarBox->AppId[len] = 0;
            file.write(VarBox->AppId, len*sizeof (char));
            file.write("\nPASSWORD\t", 10 * sizeof(char));
            QByteArray s2 = str_2.toUtf8(); len = s2.length();
            qout << 3;
            VarBox->PassWord = new char[len+1]; memcpy_s(VarBox->PassWord, len, s2, len);
            VarBox->PassWord[len] = 0;
            file.write(VarBox->PassWord, len*sizeof (char));
            file.write("\n", sizeof (char));
            file.close();
            jobTip->showTip("记录成功！");
            *const_cast<bool*>(VarBox->FirstUse) = true;
        }
        else
        {
            qout << QDir::currentPath(); QDir::current();
            QMessageBox::warning(ui->frame, "错误", "文件写入失败。");
        }
    }
}

inline void del_file(Dialog *di ,QString str)
{
    if (QFile::exists(str))
    {
        if (QMessageBox::question(di, "提示", "本操作不可逆，是否继续？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)==QMessageBox::Yes)
        {
            if (QFile::remove(str))
            {
                di->jobTip->showTip("删除成功!");
            }
            else
                QMessageBox::critical(di, "出错", "无法删除，可能是权限不够！");
        }
    }
    else
    {
        di->jobTip->showTip("文件本就不存在！");
    }
}

void Dialog::on_pushButton_7_clicked()
{
    VarBox->CurTheme = static_cast<COLOR_THEME>(ui->comboBox_3->currentIndex());
    VarBox->enableUSBhelper = ui->checkBox->isChecked();
    QSettings IniWrite("SpeedBox.ini", QSettings::IniFormat);
    IniWrite.setIniCodec(QTextCodec::codecForName("UTF-8"));
    IniWrite.beginGroup("USBhelper");
    IniWrite.setValue("enableUSBhelper",  VarBox->enableUSBhelper);
    jobTip->showTip("应用并保存成功！");
}


void Dialog::on_pushButton_5_clicked()
{
    VarBox->CurTheme = static_cast<COLOR_THEME>(ui->comboBox_3->currentIndex());
    QSettings IniWrite("SpeedBox.ini", QSettings::IniFormat);
    IniWrite.setIniCodec(QTextCodec::codecForName("UTF-8"));
    IniWrite.beginGroup("UI");
    IniWrite.setValue("ColorTheme", (int)VarBox->CurTheme);
    ui->frame->setStyleSheet(QString("QFrame{background-color:rgba(%1);}QLabel{border-radius: 3px;background-color: transparent;}Line{background-color:black};").arg(color_theme[static_cast<int>(VarBox->CurTheme)]));
    jobTip->showTip("设置成功！");
    qDebug() << (int)VarBox->CurTheme;
}

// 展示版本信息
void Dialog::on_pushButton_13_clicked()
{
    jobTip->showTip(QString("版本号：") + VarBox->Version, 1000);
}

// 检查是否有更新 https://gitee.com/yjmthu/Speed-Box/raw/main/Update.json
void Dialog::on_pushButton_10_clicked()
{
    static bool wait = false;
    if (wait)
    {
        QMessageBox::warning(this, "提示", "已经在检查更新中，可能是网速比较慢，请勿重复点击！");
        return;
    }
    else
        wait = true;

    QNetworkAccessManager* mgr = new QNetworkAccessManager;

    connect(mgr, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep){
        if (rep->error() != QNetworkReply::NoError)
        {
            jobTip->showTip("下载失败！");
            mgr->deleteLater();
            return;
        }
        const YJson json(rep->readAll());
        if (json.getType() != YJson::Object)
        {
            jobTip->showTip("Gitee源出现问题！");
            mgr->deleteLater();
            return;
        }
        const char * version = json.find("Latest Version")->getValueString();
        if (!strcmp(VarBox->Version, version))
        {
            jobTip->showTip("当前已经是最新版本！", 1000);
        }
        else
        {
            if (VarBox->versionBefore(VarBox->Version, version))
            {
                jobTip->showTip(QString("\t有新版本已经出现！\n当前版本：%1%2%3").arg(VarBox->Version, "; 最新版本：", version), 2000);
                ui->pushButton_12->setEnabled(true);
            }
            else
            {
                jobTip->showTip(version, 800);
                jobTip->showTip("更新失败，当前版本过高，请手动打开官网更新。", 3000);
            }
        }
        mgr->deleteLater();
    });

    mgr->get(QNetworkRequest(QUrl("https://gitee.com/yjmthu/Speed-Box/raw/main/update/update.json")));
    wait = false;
}

// 下载更新  https://gitee.com/yjmthu/Speed-Box/raw/main/Update.json
void Dialog::on_pushButton_12_clicked()
{
    if (VarBox->InternetGetConnectedState())
    {
        QMessageBox::information(this, "提示", "点击确认开始下载,下载时间一般不会很长,请耐心等待,\n不要关闭设置窗口.");
    }
    else
    {
        QMessageBox::information(this, "提示", "没有网络！");
        return;
    }
    QNetworkAccessManager* mgr0 = new QNetworkAccessManager;
    connect(mgr0, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep){
        YJson json(rep->readAll());
        mgr0->deleteLater();
        if (json.ep.first)
        {
            VarBox->MSG("Json文件出错, 下载失败!");
            return;
        }
        if (json.getType() != YJson::Object)
        {
            jobTip->showTip("Gitee源出现问题！");
            return;
        }
        YJson *urls = json.find("Files"); YJson *child = nullptr;
        //VarBox->MSG(json.toString());
        if (urls && urls->getType() == YJson::Object)
        {
            if (!(child = urls->find("zip")))
            {
                jobTip->showTip("Json 文件不含zip, 下载失败！");
                return;
            }
            const QUrl url1(child->getValueString());
            if (!(child = urls->find("update")))
            {
                jobTip->showTip("Json 文件不含zip, 下载失败!");
                return;
            }
            if (!json["File Name"])
            {
                jobTip->showTip("Json 文件不含File Name, 下载失败！");
                return;
            }
            const QUrl url2(child->getValueString());
            const QString zipfile = json["File Name"].getValueString();
            qout << "文件地址：" << url1 << url2 << zipfile;

            DownloadProgress pro(zipfile, url1, url2,this);

            pro.exec();
            qout << "下载第一个文件";
        }
        else
        {
            jobTip->showTip("未知原因，下载失败！");
        }

    });
    mgr0->get(QNetworkRequest(QUrl("https://gitee.com/yjmthu/Speed-Box/raw/main/update/update.json")));
}

void Dialog::on_toolButton_2_clicked()
{
    QString titile = "请选择一个文件夹";
    QString dir = QFileDialog::getExistingDirectory(NULL, titile, VarBox->wallpaper->image_path.length()? VarBox->wallpaper->image_path: qApp->applicationDirPath(), QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty())
    {
        VarBox->wallpaper->image_path = QDir::toNativeSeparators(dir);
        ui->linePictuerPath->setText(VarBox->wallpaper->image_path);
        std::string ph = "WallpaperApi.json";
        YJson json(ph, YJson::AUTO);
        if (ui->rBtnWallhavenApiDefault->isChecked())
        {
            json["Default"]["ApiData"][ui->cBxApis->currentIndex()]["Folder"].setText(VarBox->wallpaper->image_path.toUtf8());
        }
        else if (ui->rBtnWallhavenApiUser->isChecked())
        {
            json["User"]["ApiData"][(const char*)ui->cBxApis->currentText().toUtf8()]["Folder"].setText(VarBox->wallpaper->image_path.toUtf8());
        }
        else if (ui->rBtnBingApi->isChecked())
        {
            json["BingApi"]["Folder"].setText(VarBox->wallpaper->image_path.toUtf8());
        }
        else
        {
            json["OtherApi"]["ApiData"][(const char*)ui->cBxApis->currentText().toUtf8()]["Folder"].setText(VarBox->wallpaper->image_path.toUtf8());
        }
        json.toFile(ph, YJson::UTF8BOM, true);
        jobTip->showTip("修改成功！");
    }
    else
    {
        jobTip->showTip("更改失败!");
    }
}


void Dialog::on_pushButton_6_clicked()
{
    FormSetting d;
    d.move(frameGeometry().x()+(width()-d.width())/2, frameGeometry().y()+(height()-d.height())/2);
    d.exec();
}


void Dialog::on_checkBox_4_clicked(bool checked)
{
    QSettings IniWrite("SpeedBox.ini", QSettings::IniFormat);
    IniWrite.setIniCodec(QTextCodec::codecForName("UTF-8"));
    IniWrite.beginGroup("UI");
    IniWrite.setValue("ControlDesktopIcon", checked);
    if (checked)
    {
        VarBox->ControlDesktopIcon = new DesktopMask;
    }
    else
    {
        delete VarBox->ControlDesktopIcon;
        VarBox->ControlDesktopIcon = nullptr;
    }
    jobTip->showTip("设置并保存成功！");
}

void Dialog::my_on_rBtnWallhavenApiDefault_clicked()
{
    disconnect(ui->cBxApis, &QComboBox::currentTextChanged, this, &Dialog::my_on_cBxApis_currentTextChanged);
    ui->cBxApis->clear();
    ui->cBxApis->addItems({"最热", "自然", "动漫", "极简"});
    YJson json("WallpaperApi.json", YJson::AUTO);
    ui->linePictuerPath->setText(json["Default"]["ApiData"][ui->cBxApis->currentIndex()]["Folder"].getValueString());
    connect(ui->cBxApis, &QComboBox::currentTextChanged, this, &Dialog::my_on_cBxApis_currentTextChanged);
}

void Dialog::my_on_rBtnWallhavenApiUser_clicked()
{
    disconnect(ui->cBxApis, &QComboBox::currentTextChanged, this, &Dialog::my_on_cBxApis_currentTextChanged);
    ui->cBxApis->clear();
    YJson json("WallpaperApi.json", YJson::AUTO);
    YJson& temp = json["User"];
    for (auto& c: temp["ApiData"])
        ui->cBxApis->addItem(c.getKeyString());
    ui->cBxApis->setCurrentText(temp["Curruent"].getValueString());
    ui->linePictuerPath->setText(temp["ApiData"][(const char*)ui->cBxApis->currentText().toUtf8()]["Folder"].getValueString());
    connect(ui->cBxApis, &QComboBox::currentTextChanged, this, &Dialog::my_on_cBxApis_currentTextChanged);
}

void Dialog::my_on_rBtnBingApi_clicked()
{
    disconnect(ui->cBxApis, &QComboBox::currentTextChanged, this, &Dialog::my_on_cBxApis_currentTextChanged);
    ui->cBxApis->clear();
    ui->cBxApis->addItem("Bing");
    YJson json("WallpaperApi.json", YJson::AUTO);
    ui->linePictuerPath->setText(json["BingApi"]["Folder"].getValueString());
    connect(ui->cBxApis, &QComboBox::currentTextChanged, this, &Dialog::my_on_cBxApis_currentTextChanged);
}

void Dialog::my_on_rBtnOtherApi_clicked()
{
    disconnect(ui->cBxApis, &QComboBox::currentTextChanged, this, &Dialog::my_on_cBxApis_currentTextChanged);
    ui->cBxApis->clear();
    YJson json("WallpaperApi.json", YJson::AUTO);
    YJson& temp = json["OtherApi"];
    for (auto& c: temp["ApiData"])
        ui->cBxApis->addItem(c.getKeyString());
    ui->cBxApis->setCurrentText(temp["Curruent"].getValueString());
    ui->linePictuerPath->setText(temp["ApiData"][ui->cBxApis->currentText().toStdString().c_str()]["Folder"].getValueString());
    connect(ui->cBxApis, &QComboBox::currentTextChanged, this, &Dialog::my_on_cBxApis_currentTextChanged);
}

void Dialog::my_on_cBxApis_currentTextChanged(const QString &arg1)
{
    constexpr char ph[] = "WallpaperApi.json";
    YJson json(ph, YJson::AUTO);
    if (ui->rBtnWallhavenApiDefault->isChecked())
    {
        ui->linePictuerPath->setText(json["Default"]["ApiData"][ui->cBxApis->currentIndex()]["Folder"].getValueString());
        return;
    }
    if (ui->rBtnWallhavenApiUser->isChecked())
    {
        json["User"]["Curruent"].setText(ui->cBxApis->currentText().toUtf8());
        if (VarBox->PaperType == PAPER_TYPE::User)
        {
            VarBox->wallpaper->image_path = json["User"]["ApiData"][(const char*)arg1.toUtf8()]["Folder"].getValueString();
            ui->linePictuerPath->setText(VarBox->wallpaper->image_path);
        }
        else
        {
            ui->linePictuerPath->setText(json["User"]["ApiData"][(const char*)arg1.toUtf8()]["Folder"].getValueString());
        }
        json.toFile(ph, YJson::UTF8BOM, true);
    }
    else if (ui->rBtnBingApi->isChecked())
    {
        if (VarBox->PaperType == PAPER_TYPE::Bing)
        {
            VarBox->wallpaper->image_path = json["BingApi"]["Folder"].getValueString();
            ui->linePictuerPath->setText(VarBox->wallpaper->image_path);
        }
        else
        {
            ui->linePictuerPath->setText(json["BingApi"]["Folder"].getValueString());
        }
    }
    else
    {
        if (VarBox->PaperType == PAPER_TYPE::Other)
        {
            json["OtherApi"]["Curruent"].setText(arg1.toUtf8());
            VarBox->wallpaper->image_path = json["OtherApi"]["ApiData"][(const char*)arg1.toUtf8()]["Folder"].getValueString();
            VarBox->wallpaper->url = json["OtherApi"]["ApiData"][(const char*)arg1.toUtf8()]["Url"].getValueString();
            ui->linePictuerPath->setText(VarBox->wallpaper->image_path);
        }
        else
        {
            json["OtherApi"]["Curruent"].setText(arg1.toUtf8());
            ui->linePictuerPath->setText(json["OtherApi"]["ApiData"][(const char*)arg1.toUtf8()]["Folder"].getValueString());
        }
        json.toFile(ph, YJson::UTF8BOM, true);
    }
    jobTip->showTip("应用成功!");
}

void Dialog::on_pushButton_3_clicked()
{
    ExplainDialog dialog(this);
    dialog.exec();
}

