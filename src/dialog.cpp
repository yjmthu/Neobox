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
    ui(new Ui::Dialog),
    wallpaper(VarBox->wallpaper)
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
    typedef Wallpaper::Type Type;
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);  //| Qt::WindowStaysOnTopHint
    setAttribute(Qt::WA_DeleteOnClose, true);
    ui->pushButton_12->setEnabled(false);
    QFile qss(":/qss/dialog_style.qss");
    qss.open(QFile::ReadOnly);
    setStyleSheet(QString(qss.readAll()));
    qss.close();
    initSpeedBox(ui->frame, &Dialog::showMinimized, &Dialog::close);
    buttonGroup->addButton(ui->rBtnHot, (int)Type::Hot);         //将按钮添加到按钮组合，同时设置按钮 id
    buttonGroup->addButton(ui->rBtnNature, (int)Type::Nature);
    buttonGroup->addButton(ui->rBtnAnime, (int)Type::Anime);
    buttonGroup->addButton(ui->rBtnSimple, (int)Type::Simple);
    buttonGroup->addButton(ui->rBtnUser, (int)Type::User);
    buttonGroup->addButton(ui->rBtnRandom, (int)Type::Random);
    buttonGroup->addButton(ui->rBtnBing, (int)Type::Bing);
    buttonGroup->addButton(ui->rBtnOther, (int)Type::Other);
    buttonGroup->addButton(ui->rBtnNative, (int)Type::Native);
    buttonGroup->addButton(ui->rBtnAdvance, (int)Type::Advance);
	buttonGroup->setExclusive(true);                     // 按钮之间相互排斥
    buttonGroup->button(static_cast<int>(wallpaper->PaperType))->setChecked(true);
    ui->lineAppData->setText(QDir::currentPath().replace("/", "\\"));

    ui->checkBox_2->setChecked(false);
    ui->LinePath->setText(wallpaper->NativeDir);
    ui->sLdPageNum->setValue(wallpaper->PageNum);
    ui->sLdTimeInterval->setValue(wallpaper->TimeInterval);
    ui->labTimeInterval->setText(QString::number(wallpaper->TimeInterval));
    ui->chkEnableChangePaper->setChecked(wallpaper->AutoChange);
    ui->usrCmd->setText(wallpaper->UserCommand);

    ui->cBxAutoStart->setChecked(!QSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat).value("SpeedBox").toString().compare(qApp->applicationFilePath().replace("/", "\\")));
    ui->lineEdit_2->setText(qApp->applicationDirPath().replace("/", "\\"));
    switch (wallpaper->PaperType) {
    case Type::Bing:
        ui->rBtnBingApi->setChecked(true);
        my_on_rBtnBingApi_clicked();
        break;
    case Type::User:
        ui->rBtnWallhavenApiUser->setChecked(true);
        my_on_rBtnWallhavenApiUser_clicked();
        break;
    case Type::Other:
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
    ui->checkBox_3->setChecked(wallpaper->FirstChange);
    ui->BtnChooseFolder->setEnabled(ui->rBtnNative->isChecked());
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
    if (wallpaper->NativeDir.isEmpty() || (!d.exists(wallpaper->NativeDir) && !d.mkdir(wallpaper->NativeDir)))
	{
        wallpaper->NativeDir = QDir::toNativeSeparators(QStandardPaths::writableLocation((QStandardPaths::PicturesLocation)));
        VarBox->sigleSave("Wallpaper", "NativeDir", wallpaper->NativeDir);
        ui->LinePath->setText(wallpaper->NativeDir);
	}
	QString titile = "请选择一个文件夹";
    QString dir = QFileDialog::getExistingDirectory(NULL, titile, wallpaper->NativeDir, QFileDialog::ShowDirsOnly);
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
    wallpaper->update = ui->checkBox_2->isChecked();
    wallpaper->PaperType = static_cast<Wallpaper::Type>(buttonGroup->checkedId());
    wallpaper->NativeDir = ui->LinePath->text();
    wallpaper->PageNum = ui->sLdPageNum->value();
    wallpaper->TimeInterval = ui->sLdTimeInterval->value();
    wallpaper->AutoChange = ui->chkEnableChangePaper->isChecked();
    wallpaper->UserCommand = ui->usrCmd->text();
    wallpaper->FirstChange = ui->checkBox_3->isChecked();
    for (int type = 0; type < 2; ++type)
    {
        if (type == 0)
        {
            if (wallpaper->isActive())
            {
                if (QMessageBox::question(this, "警告", "您之前的壁纸类型更换正在生效中，是否终止？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                {
                    wallpaper->kill();
                    if (QMessageBox::question(this, "提示", "终止成功！是否继续？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                        continue;
                }
                break;
            }
        }
        else if (type == 1)
        {
            if (wallpaper->FirstChange)
            {
                qout << "首更1";
                wallpaper->applyClicked = true;
                wallpaper->push_back();
                qout << "首更";
            }
            if (wallpaper->AutoChange)
            {
                wallpaper->timer->setInterval(wallpaper->TimeInterval * 60000);  // 设置时间间隔,Timer的单位是毫秒
                wallpaper->timer->start();
                qout << "开始更换壁纸";
            }
            else
                wallpaper->timer->stop();
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
        wallpaper->image_path = str.replace("/", "\\");
	else
	{
        if (dir.exists(wallpaper->image_path) || dir.mkdir(wallpaper->image_path))
            ui->linePictuerPath->setText(wallpaper->image_path.replace("/", "\\"));
		else
            wallpaper->image_path = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
	}
    if (dir.exists(wallpaper->image_path))
        VarBox->openDirectory(wallpaper->image_path);
	else
		QMessageBox::warning(this, "警告", "路径不存在！", QMessageBox::Ok, QMessageBox::Ok);
}

void Dialog::linePictuerPathReturn()
{
	QDir dir;
	if (dir.exists(ui->linePictuerPath->text()))
	{
        wallpaper->image_path = ui->linePictuerPath->text();
	}
	else
	{
		QMessageBox::warning(this, "警告", "路径不存在！", QMessageBox::Ok, QMessageBox::Ok);
        ui->linePictuerPath->setText(wallpaper->image_path);
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
            if (VarBox->getVersion(VarBox->Version) < VarBox->getVersion(version))
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
    QString dir = QFileDialog::getExistingDirectory(NULL, titile, wallpaper->image_path.length()? wallpaper->image_path: qApp->applicationDirPath(), QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty())
    {
        wallpaper->image_path = QDir::toNativeSeparators(dir);
        ui->linePictuerPath->setText(wallpaper->image_path);
        std::string ph = "WallpaperApi.json";
        YJson json(ph, YJson::AUTO);
        if (ui->rBtnWallhavenApiDefault->isChecked())
        {
            json["Default"]["ApiData"][ui->cBxApis->currentIndex()]["Folder"].setText(wallpaper->image_path.toUtf8());
        }
        else if (ui->rBtnWallhavenApiUser->isChecked())
        {
            json["User"]["ApiData"][(const char*)ui->cBxApis->currentText().toUtf8()]["Folder"].setText(wallpaper->image_path.toUtf8());
        }
        else if (ui->rBtnBingApi->isChecked())
        {
            json["BingApi"]["Folder"].setText(wallpaper->image_path.toUtf8());
        }
        else
        {
            json["OtherApi"]["ApiData"][(const char*)ui->cBxApis->currentText().toUtf8()]["Folder"].setText(wallpaper->image_path.toUtf8());
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
    typedef Wallpaper::Type Type ;
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
        if (wallpaper->PaperType == Type::User)
        {
            wallpaper->image_path = json["User"]["ApiData"][(const char*)arg1.toUtf8()]["Folder"].getValueString();
            ui->linePictuerPath->setText(wallpaper->image_path);
        }
        else
        {
            ui->linePictuerPath->setText(json["User"]["ApiData"][(const char*)arg1.toUtf8()]["Folder"].getValueString());
        }
        json.toFile(ph, YJson::UTF8BOM, true);
    }
    else if (ui->rBtnBingApi->isChecked())
    {
        if (wallpaper->PaperType == Type::Bing)
        {
            wallpaper->image_path = json["BingApi"]["Folder"].getValueString();
            ui->linePictuerPath->setText(wallpaper->image_path);
        }
        else
        {
            ui->linePictuerPath->setText(json["BingApi"]["Folder"].getValueString());
        }
    }
    else
    {
        if (wallpaper->PaperType == Type::Other)
        {
            json["OtherApi"]["Curruent"].setText(arg1.toUtf8());
            wallpaper->image_path = json["OtherApi"]["ApiData"][(const char*)arg1.toUtf8()]["Folder"].getValueString();
            wallpaper->url = json["OtherApi"]["ApiData"][(const char*)arg1.toUtf8()]["Url"].getValueString();
            ui->linePictuerPath->setText(wallpaper->image_path);
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

