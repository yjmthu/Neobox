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

#include "dialog.h"
#include "ui_dialog.h"
#include "form.h"
#include "blankform.h"
#include "gmpoperatetip.h"
#include "dialogwallpaper.h"
#include "YString.h"
#include "YJson.h"

const char *color_theme[4] =
{
    "255,255,255,255",
    "255,255,255,150",
    "155,89,182,150",
    "255,0,0,200"
};

const char *reg_keys[4] = {
    "HKEY_CURRENT_USER\\SOFTWARE\\Classes\\*\\shell\\QCoper",
    "HKEY_CURRENT_USER\\SOFTWARE\\Classes\\Directory\\shell\\QCoper",
    "HKEY_CURRENT_USER\\SOFTWARE\\Classes\\Directory\\Background\\shell\\QCoper",
    "mshta vbscript:clipboarddata.setdata(\"text\",\"%%1\")(close)"
};

QString getStyleSheet()
{
    QColor col(VarBox.dAlphaColor[VarBox.SET_FULL] & 0xffffff);
    return QString("QPushButton{background-color: rgb(%1, %2, %3)}").arg(QString::number(col.blue()), QString::number(col.green()), QString::number(col.red()));
}


Dialog::Dialog():
    QWidget(nullptr),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);                                   // 一定要在下面这句之前，否则会出现问题。
    initChildren();
    initUi();
	initConnects();
	initBehaviors();
    initButtonFilter();
}

Dialog::~Dialog()
{
    delete tray;
    delete jobTip;
    delete change_paper_timer;
    delete wallpaper;
    delete buttonGroup;
    delete ui;
}

void Dialog::initUi()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);  //| Qt::WindowStaysOnTopHint
    setAttribute(Qt::WA_TranslucentBackground);
    QFile qss(":/qss/dialog_style.qss");
    qss.open(QFile::ReadOnly);
    setStyleSheet(QString(qss.readAll()));
    qss.close();
    QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(this);
    effect->setOffset(0, 0);          //具体阴影
    effect->setBlurRadius(15);        //阴影半径
    effect->setColor(Qt::black);      //阴影颜色
    ui->frame->setGraphicsEffect(effect);
    BlankFrom *blank = new BlankFrom(this);
    connect(blank->closeButton, &QPushButton::clicked, this, &Dialog::close);
    connect(blank->minButton, &QPushButton::clicked, this, &Dialog::showMinimized);
    blank->move(width()-100, 0);
    buttonGroup->addButton(ui->rBtnNew, (int)PAPER_TYPE::Latest);                //将按钮添加到按钮组合，同时设置按钮 id
    buttonGroup->addButton(ui->rBtnHot, (int)PAPER_TYPE::Hot);
    buttonGroup->addButton(ui->rBtnNature, (int)PAPER_TYPE::Nature);
    buttonGroup->addButton(ui->rBtnAnime, (int)PAPER_TYPE::Anime);
    buttonGroup->addButton(ui->rBtnSimple, (int)PAPER_TYPE::Simple);
    buttonGroup->addButton(ui->rBtnRandom, (int)PAPER_TYPE::Random);
    buttonGroup->addButton(ui->rBtnBing, (int)PAPER_TYPE::Bing);
    buttonGroup->addButton(ui->rBtnNative, (int)PAPER_TYPE::Native);
    buttonGroup->addButton(ui->rBtnAdvance, (int)PAPER_TYPE::Advance);
	buttonGroup->setExclusive(true);                     // 按钮之间相互排斥
    ui->lineAppData->setText(FuncBox::get_dat_path());
	ui->lineAppData->setReadOnly(true);
    move((VarBox.ScreenWidth - width()) / 2, (VarBox.ScreenHeight - height()) / 2);
    setTheme();
}

void Dialog::initChildren()
{
	buttonGroup = new QButtonGroup;
    wallpaper = new DialogWallpaper;                     // 创建壁纸更换类
	change_paper_timer = new QTimer;                     // 定时器，定时更换壁纸
    jobTip = new GMPOperateTip(this);
    tray = new Tray;
}

void Dialog::initConnects()
{
    connect(wallpaper, SIGNAL(setFailed(const char*)), (Form*)VarBox.form, SLOT(set_wallpaper_fail(const char*)));
    connect(change_paper_timer, SIGNAL(timeout()), wallpaper, SLOT(start()));
    connect(wallpaper, &DialogWallpaper::msgBox, (Form*)VarBox.form, &Form::msgBox);
	for (int c = 0; c <= 9; c++)
        if ((c != (int)PAPER_TYPE::Native) && (c != 7)) connect(buttonGroup->button(c), &QPushButton::clicked, this, [=]() {last_checked_button = c;});
}

void Dialog::initBehaviors()
{
    change_paper_timer->setInterval(VarBox.TimeInterval * 60000);
    wallpaper->start();                                  // Timer默认第一次不启动，这里要加上。
	if (VarBox.AutoChange)
	{
        change_paper_timer->start();
	}
}

void Dialog::initButtonFilter()
{
    QList<QPushButton *> pBtns = findChildren<QPushButton *>();
    QList<QPushButton *>::iterator pBtnList = pBtns.begin();
    for (pBtnList = pBtns.begin(); pBtnList != pBtns.end(); pBtnList++)
    {
        (*pBtnList)->installEventFilter(this);
    }
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

void Dialog::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)                   // 鼠标左键点击悬浮窗
    {
        setMouseTracking(true);                              // 开始跟踪鼠标位置，从而使悬浮窗跟着鼠标一起移动。
        _startPos = event->pos();                            // 记录开始位置
    }
    event->accept();
}

void Dialog::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)  // 鼠标左键释放
    {
        setMouseTracking(false);           // 停止跟踪鼠标。
        _endPos = event->pos() - _startPos;
        mouse_moved = false;
    }
    event->accept();
}

void Dialog::mouseMoveEvent(QMouseEvent* event)
{
    _endPos = event->pos() - _startPos;  //计算位置变化情况。
    move(pos() + _endPos);               //当前位置加上位置变化情况，从而实现悬浮窗和鼠标同时移动。
    mouse_moved = true;
    event->accept();
}

void Dialog::showEvent(QShowEvent *event)
{
    qout << "showEvent被调用！";
    last_checked_button = (int)VarBox.PaperType;
    ui->BtnChooseFolder->setEnabled(last_checked_button == 7);
    buttonGroup->button(last_checked_button)->setChecked(true);
    ui->LinePath->setText(VarBox.NativeDir);
    ui->SliderPageNum->setValue(VarBox.PageNum);
    ui->SliderTimeInterval->setValue(VarBox.TimeInterval);
    ui->labTimeInterval->setText(QString::number(VarBox.TimeInterval));
    ui->chkEnableChangePaper->setChecked(VarBox.AutoChange);
    ui->usrCmd->setText(VarBox.UserCommand);
    ui->linePictuerPath->setText(VarBox.FamilyPath);
    ui->radioButton_2->setChecked(VarBox.SET_FULL);
    ui->horizontalSlider->setValue(VarBox.dAlphaColor[VarBox.SET_FULL] >> 24);
    ui->horizontalSlider_2->setValue(VarBox.bAlpha[VarBox.SET_FULL]);
    ui->horizontalSlider_3->setValue(VarBox.RefreshTime);
    ui->pushButton_3->setStyleSheet(getStyleSheet());
    ui->lineNewName->setText(VarBox.FamilyNames[ui->comboBox->currentIndex()]);
    ui->line_APP_ID->setText(VarBox.AppId);
    ui->line_PASS_WORD->setText(VarBox.PassWord);
    switch (VarBox.aMode[VarBox.SET_FULL])
    {
    case (ACCENT_STATE::ACCENT_DISABLED):
        ui->radioButton_3->setChecked(true);
        break;
    case (ACCENT_STATE::ACCENT_ENABLE_TRANSPARENTGRADIENT):
        ui->radioButton_4->setChecked(true);
        break;
    case (ACCENT_STATE::ACCENT_ENABLE_BLURBEHIND):
        ui->radioButton_5->setChecked(true);
        break;
    case (ACCENT_STATE::ACCENT_ENABLE_ACRYLICBLURBEHIND):
        ui->radioButton_6->setChecked(true);
        break;
    default:
        break;
    }
    switch (QSettings(TASK_DESK_SUB, QSettings::NativeFormat).value(TASKBAR_ACRYLIC_OPACITY).toInt())
    {
    case 10:
        ui->radioButton_11->setChecked(true);
        break;
    case 5:
        ui->radioButton_10->setChecked(true);
        break;
    case 0:
        ui->radioButton_12->setChecked(true);
        break;
    default:
        break;
    }
    switch (VarBox.iPos)
    {
    case TaskBarCenterState::TASK_LEFT:
        ui->radioButton_7->setChecked(true);
        break;
    case TaskBarCenterState::TASK_CENTER:
        ui->radioButton_8->setChecked(true);
        break;
    case TaskBarCenterState::TASK_RIGHT:
        ui->radioButton_9->setChecked(true);
        break;
    default:
        break;
    }
    if (VarBox.PathToOpen.compare(qApp->applicationDirPath().replace("/", "\\")))
        ui->radioButton_14->setChecked(true);
    else
        ui->radioButton_13->setChecked(true);
    ui->pushButton_4->setText("确定");
    ui->comboBox_3->setCurrentIndex((int)VarBox.CurTheme);
    ui->label_path_to_open->setText(VarBox.PathToOpen);
    ui->label_path_to_open->setToolTip(VarBox.PathToOpen);
    checkSettings(); event->accept();
}

/**
 * @brief Dialog::on_rBtnNative_toggled
 *
 * @当“本地”按钮被选中时，选择文件夹按钮可用。
 *
 * @param {checked} 按钮选中状态
 *
 */

void Dialog::on_rBtnNative_toggled(bool checked)
{
	ui->BtnChooseFolder->setEnabled(checked);
}

void Dialog::on_BtnChooseFolder_clicked()
{
	QDir d;
    if (VarBox.NativeDir.isEmpty() || (!d.exists(VarBox.NativeDir) && !d.mkdir(VarBox.NativeDir)))
	{
        VarBox.NativeDir = QDir::toNativeSeparators(QStandardPaths::writableLocation((QStandardPaths::PicturesLocation)));
        FuncBox::sigleSave("Wallpaper", "NativeDir", VarBox.NativeDir);
		ui->LinePath->setText(VarBox.NativeDir);
	}
	QString titile = "请选择一个文件夹";
	QString dir = QFileDialog::getExistingDirectory(NULL, titile, VarBox.NativeDir, QFileDialog::ShowDirsOnly);
    if (dir.isEmpty())
	{
		buttonGroup->button(last_checked_button)->setChecked(true);
		ui->BtnChooseFolder->setEnabled(last_checked_button == 7);
	}
    else
        ui->LinePath->setText(QDir::toNativeSeparators(dir));
}

void Dialog::on_SliderTimeInterval_valueChanged(int value)
{
	ui->labTimeInterval->setText(QString::number(value, 10));  // 十进制整数转化为字符串
}

void Dialog::on_SliderPageNum_valueChanged(int value)
{
	ui->label_2->setText(QString::number(value, 10));
}

void Dialog::on_pBtnCancel_clicked()
{
	close();
}

void Dialog::on_pBtnOk_clicked()
{
    QSettings IniWrite(FuncBox::get_ini_path(), QSettings::IniFormat);
    IniWrite.beginGroup("Wallpaper");
    IniWrite.setValue("PaperType", buttonGroup->checkedId());
    IniWrite.setValue("NativeDir", ui->LinePath->text());
    IniWrite.setValue("PageNum", ui->SliderPageNum->value());
    IniWrite.setValue("TimeInerval", ui->SliderTimeInterval->value());
    IniWrite.setValue("AutoChange", ui->chkEnableChangePaper->isChecked());
    IniWrite.setValue("UserCommand", ui->usrCmd->text());
    IniWrite.endGroup();
    jobTip->showTip("保存成功！");
}

/**
 * @brief Dialog::on_pBtnApply_clicked
 *
 * @“应用”按钮被点击后，将界面上的配置导入到文件中，将文件中的数据加载到静态变量。
 *
 */

void Dialog::on_pBtnApply_clicked()
{
    VarBox.PaperType = (PAPER_TYPE)buttonGroup->checkedId();
    VarBox.NativeDir = ui->LinePath->text();
    VarBox.PageNum = ui->SliderPageNum->value();
    VarBox.TimeInterval = ui->SliderTimeInterval->value();
    VarBox.AutoChange = ui->chkEnableChangePaper->isChecked();
    VarBox.UserCommand = ui->usrCmd->text();
    for (int type = 0; type < 2; ++type)
    {
        if (type == 0)
        {
            if (wallpaper->isActive())
            {
                if (QMessageBox::question(this, "警告", "您之前的壁纸类型更换正在生效中，是否终止？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                {
                    setEnabled(false);
                    VarBox.RunApp = false;
                    wallpaper->clean();
                    VarBox.RunApp = true;
                    setEnabled(true);
                    if (QMessageBox::question(this, "提示", "终止成功！是否继续？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                        continue;
                }
                break;
            }
            else
            {
                if (Wallpaper::canCreat())
                    continue;
                QMessageBox::information(this, "提示", "右键更换壁纸正在运行，请稍后再试。", QMessageBox::Ok);
                break;
            }
        }
        else if (type == 1)
        {
            if (VarBox.AutoChange)
            {
                Wallpaper::initSet = true;
                wallpaper->start();
                change_paper_timer->setInterval(VarBox.TimeInterval * 60000);  // 设置时间间隔,Timer的单位是毫秒
                if (!change_paper_timer->isActive()) change_paper_timer->start();
                qout << "开始更换壁纸";
            }
            else
                if (change_paper_timer->isActive()) change_paper_timer->stop();
            if (QMessageBox::question(this, "提示", "应用成功！是否保存设置？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                on_pBtnOk_clicked();
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
    static QString icon_path = FuncBox::get_dat_path() + "\\Copy.ico";
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
                settings.setValue("Icon", icon_path);
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

void Dialog::on_pBtnOpenAppData_clicked()
{
    FuncBox::runCommand("explorer", QStringList(FuncBox::get_dat_path()));
}

void Dialog::on_pBtnOpenPicturePath_clicked()
{
	QString str = ui->linePictuerPath->text();
	QDir dir;
    if (dir.exists(str) || (!str.isEmpty() && dir.mkdir(str)))
        VarBox.FamilyPath = str.replace("/", "\\");
	else
	{
        if (dir.exists(VarBox.FamilyPath) || dir.mkdir(VarBox.FamilyPath))
            ui->linePictuerPath->setText(VarBox.FamilyPath.replace("/", "\\"));
		else
            VarBox.FamilyPath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
	}
	FuncBox::get_wal_path();
    if (dir.exists(VarBox.FamilyPath))
        FuncBox::runCommand("explorer", QStringList(VarBox.FamilyPath));
	else
		QMessageBox::warning(this, "警告", "路径不存在！", QMessageBox::Ok, QMessageBox::Ok);
}


void Dialog::on_linePictuerPath_returnPressed()
{
	QDir dir;
	if (dir.exists(ui->linePictuerPath->text()))
	{
        VarBox.FamilyPath = ui->linePictuerPath->text();
		FuncBox::get_wal_path();
	}
	else
	{
		QMessageBox::warning(this, "警告", "路径不存在！", QMessageBox::Ok, QMessageBox::Ok);
        ui->linePictuerPath->setText(VarBox.FamilyPath);
	}
}

void Dialog::on_horizontalSlider_valueChanged(int value)
{
    DWORD bAlphaB = (DWORD)value << 24;
    ui->label_4->setText(QString::number(value, 10));
    VarBox.dAlphaColor[VarBox.SET_FULL] = bAlphaB + (VarBox.dAlphaColor[VarBox.SET_FULL] & 0xffffff);
    if (!tray->beautifyTask->isActive())
        tray->beautifyTask->start();
}

void Dialog::on_horizontalSlider_2_valueChanged(int value)
{
    ui->label_6->setText(QString::number(value, 10));
    VarBox.bAlpha[VarBox.SET_FULL] = value;
    if (!tray->beautifyTask->isActive())
        tray->beautifyTask->start();
}

void Dialog::on_horizontalSlider_3_valueChanged(int value)
{
    ui->label_17->setText(QString::number(value, 10));
    tray->beautifyTask->setInterval(value);
    VarBox.RefreshTime = value;
    if (!tray->beautifyTask->isActive())
        tray->beautifyTask->start();
}

void Dialog::on_chkTimeUnit_min_clicked()
{
    QSettings settings(TASK_DESK_SUB, QSettings::NativeFormat);
    if (settings.contains(SHOW_SECONDS_IN_SYSTEM_CLOCK))
        settings.remove(SHOW_SECONDS_IN_SYSTEM_CLOCK);
}

void Dialog::on_chkTimeUnit_sec_clicked()
{
    QSettings settings(TASK_DESK_SUB, QSettings::NativeFormat);
    if (!settings.contains(SHOW_SECONDS_IN_SYSTEM_CLOCK))
    {
        settings.setValue(SHOW_SECONDS_IN_SYSTEM_CLOCK, 1);
    }
}

void Dialog::on_radioButton_3_clicked()
{
    VarBox.aMode[VarBox.SET_FULL] = ACCENT_STATE::ACCENT_DISABLED;
}

void Dialog::on_radioButton_4_clicked()
{
    if ((VarBox.aMode[0] == ACCENT_STATE::ACCENT_DISABLED)&&(VarBox.aMode[1] == ACCENT_STATE::ACCENT_DISABLED))
    {
        tray->beautifyTask->start(VarBox.RefreshTime);
    }
    VarBox.aMode[VarBox.SET_FULL] = ACCENT_STATE::ACCENT_ENABLE_TRANSPARENTGRADIENT;
}


void Dialog::on_radioButton_5_clicked()
{
    if ((VarBox.aMode[0] == ACCENT_STATE::ACCENT_DISABLED)&&(VarBox.aMode[1] == ACCENT_STATE::ACCENT_DISABLED))
    {
        tray->beautifyTask->start(VarBox.RefreshTime);
    }
    VarBox.aMode[VarBox.SET_FULL] = ACCENT_STATE::ACCENT_ENABLE_BLURBEHIND;
}

void Dialog::on_radioButton_6_clicked()
{
    if ((VarBox.aMode[0] == ACCENT_STATE::ACCENT_DISABLED)&&(VarBox.aMode[1] == ACCENT_STATE::ACCENT_DISABLED))
    {
        tray->beautifyTask->start(VarBox.RefreshTime);
    }
    VarBox.aMode[VarBox.SET_FULL] = ACCENT_STATE::ACCENT_ENABLE_ACRYLICBLURBEHIND;
}

void Dialog::on_pushButton_clicked()
{
    FuncBox::saveTrayStyle();
    jobTip->showTip("保存完毕！");
}

void Dialog::on_radioButton_11_clicked()
{
    QSettings settings(TASK_DESK_SUB, QSettings::NativeFormat);
    settings.setValue(TASKBAR_ACRYLIC_OPACITY, 10);
}

void Dialog::on_radioButton_10_clicked()
{
    QSettings settings(TASK_DESK_SUB, QSettings::NativeFormat);
    settings.setValue(TASKBAR_ACRYLIC_OPACITY, 5);
}

void Dialog::on_radioButton_12_clicked()
{
    QSettings settings(TASK_DESK_SUB, QSettings::NativeFormat);
    settings.setValue(TASKBAR_ACRYLIC_OPACITY, 0);
}

void Dialog::on_radioButton_7_clicked()
{
    VarBox.iPos = TaskBarCenterState::TASK_LEFT;
}

void Dialog::on_radioButton_8_clicked()
{
    if (VarBox.iPos == TaskBarCenterState::TASK_LEFT)
        tray->centerTask->start();
    qout << "居中点击";
    VarBox.iPos = TaskBarCenterState::TASK_CENTER;
}

void Dialog::on_radioButton_9_clicked()
{
    if (VarBox.iPos == TaskBarCenterState::TASK_LEFT)
        tray->centerTask->start();
    VarBox.iPos = TaskBarCenterState::TASK_RIGHT;
}

void Dialog::changeType(BOOL ok)
{
    VarBox.SET_FULL = ok;
    ui->horizontalSlider->setValue(VarBox.dAlphaColor[ok] >> 24);
    ui->horizontalSlider_2->setValue(VarBox.bAlpha[ok]);
	ui->pushButton_3->setStyleSheet(getStyleSheet());
    switch (VarBox.aMode[ok])
	{
    case (ACCENT_STATE::ACCENT_DISABLED):
		ui->radioButton_3->setChecked(true);
		break;
    case (ACCENT_STATE::ACCENT_ENABLE_TRANSPARENTGRADIENT):
		ui->radioButton_4->setChecked(true);
		break;
    case (ACCENT_STATE::ACCENT_ENABLE_BLURBEHIND):
		ui->radioButton_5->setChecked(true);
		break;
    case (ACCENT_STATE::ACCENT_ENABLE_ACRYLICBLURBEHIND):
		ui->radioButton_6->setChecked(true);
		break;
	default:
		break;
	}
}

void Dialog::on_radioButton_clicked()
{
	changeType(false);
}

void Dialog::on_radioButton_2_clicked()
{
	changeType(true);
}

void Dialog::on_pushButton_3_clicked()
{

    QColor color = QColor(VarBox.dAlphaColor[VarBox.SET_FULL] & 0xffffff);
	color = QColor(color.blue(), color.green(), color.red());
    color = QColorDialog::getColor(color, ui->frame, "颜色");
	if (color.isValid())
	{
		short r = color.red(), g = color.green(), b = color.blue();
		QString str = QString("background-color: rgb(%1, %2, %3)").arg(QString::number(r), QString::number(g), QString::number(b));
		ui->pushButton_3->setStyleSheet(str);
        VarBox.dAlphaColor[VarBox.SET_FULL] = (ui->horizontalSlider->value() << 24) + RGB(r, g, b);
	}
}

void Dialog::on_comboBox_currentIndexChanged(int index)
{
    ui->lineNewName->setText(VarBox.FamilyNames[index]);
}


void Dialog::on_pBtnChange_clicked()
{
	QString str = ui->lineNewName->text();
	short i = ui->comboBox->currentIndex();
    if (VarBox.FamilyNames[i].compare(str))
	{
		if (str.isEmpty())
        {
			QMessageBox::warning(this, "警告", "名称不能为空！", QMessageBox::Ok, QMessageBox::Ok);
            ui->lineNewName->setText(VarBox.FamilyNames[i]);
        }
		else
		{
            const wchar_t patrn[] = L"[`~!@#$%^&-+=\\?:\"|,/;'\\[\\]·~！@#￥%……&*（）+=\\{\\}\\|《》？：“”【】、；‘'，。\\、\\-]";
            for (wchar_t c: str.toStdWString())
            {
                if (wcschr(patrn, c))
                {
                    QMessageBox::warning(this, "警告", "名称含有非法字符！", QMessageBox::Ok, QMessageBox::Ok);
                    return;
                }
            }
            QString new_name = (i == 7) ? VarBox.FamilyPath + "\\" + str : FuncBox::get_wal_path() + "\\" + str;
            QDir dir;
            if (dir.exists(new_name))
            {
                if (QMessageBox::question(this, "冲突", "目标路径下有一个同名文件夹，您想直接更换文件夹而不迁移图片吗？",
                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                {
                    FuncBox::sigleSave("Dirs", VarBox.PaperTypes[i][0], VarBox.FamilyNames[i] = str);
                    jobTip->showTip("更改名称成功！");
                }
                else
                {
                    jobTip->showTip("放心，我什么也没做！");
                    ui->lineNewName->setText(VarBox.FamilyNames[i]);
                }
            }
            else
            {
                QString old_name;
                if (i == 7)
                    old_name = VarBox.FamilyPath + "\\" + VarBox.FamilyNames[7];
                else
                    old_name = FuncBox::get_wal_path() + "\\" + VarBox.FamilyNames[i];
                if (dir.exists(old_name))
                    dir.rename(old_name, new_name);
                else
                    dir.mkdir(new_name);
                VarBox.FamilyNames[i] = str;
                FuncBox::sigleSave("Dirs", VarBox.PaperTypes[i][0], str);
                jobTip->showTip("更换名称成功！");
            }
		}
	}
    else
    {
        jobTip->showTip("似乎什么也没做。");
    }
}

void Dialog::on_lineNewName_returnPressed()
{
	on_pBtnChange_clicked();

}

void Dialog::on_radioButton_13_clicked()
{
    ui->label_path_to_open->setText(qApp->applicationDirPath().replace("/", "\\"));
    ui->label_path_to_open->setToolTip(ui->label_path_to_open->text());
    ui->pushButton_4->setText("确定");
}


void Dialog::on_radioButton_14_clicked()
{
    ui->label_path_to_open->setText("点击右侧按钮选择文件");
    ui->label_path_to_open->setToolTip("点击右侧按钮选择文件");
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
            ui->label_path_to_open->setText(dir.replace("/", "\\"));
            ui->label_path_to_open->setToolTip(ui->label_path_to_open->text());
		}
        return;
	}
    VarBox.PathToOpen = ui->label_path_to_open->text();
    FuncBox::get_son_dir(VarBox.PathToOpen);
    FuncBox::sigleSave("Dirs", "OpenDir", VarBox.PathToOpen);
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
        QFile file(FuncBox::get_dat_path() + "\\AppId.txt");
        if (file.open(QIODevice::WriteOnly))
        {
            if (VarBox.AppId) delete [] VarBox.AppId;
            if (VarBox.PassWord) delete [] VarBox.PassWord;
            VarBox.HaveAppRight = true;
            qout << 1;
            unsigned char c[3] = {0xEF, 0xBB, 0xBF}; size_t len = 0;
            file.write((char*)c, 3 * sizeof (char));
            file.write("APP ID\t", 7 * sizeof (char));
            QByteArray s1 = str_1.toUtf8(); len = s1.length();
            qout << 2;
            VarBox.AppId = new char[len+1]; memcpy_s(VarBox.AppId, len, s1, len);
            VarBox.AppId[len] = 0;
            file.write(VarBox.AppId, len*sizeof (char));
            file.write("\nPASSWORD\t", 10 * sizeof(char));
            QByteArray s2 = str_2.toUtf8(); len = s2.length();
            qout << 3;
            VarBox.PassWord = new char[len+1]; memcpy_s(VarBox.PassWord, len, s2, len);
            VarBox.PassWord[len] = 0;
            file.write(VarBox.PassWord, len*sizeof (char));
            file.write("\n", sizeof (char));
            file.close();
            jobTip->showTip("记录成功！");
        }
        else
        {
            qout << FuncBox::get_dat_path();
            QMessageBox::warning(ui->frame, "错误", "文件写入失败。");
        }
    }
}

void del_file(Dialog *di ,QString str)
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
    QString str = FuncBox::get_ini_path();
    if (QFile::exists(str))
    {
        if (QMessageBox::question(this, "提示", "将要清除所有设置并退出软件，是否继续？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)==QMessageBox::Yes)
        {
            if (QFile::remove(str))
            {
                jobTip->showTip("删除成功!");
                setEnabled(false);
                ((Form*)VarBox.form)->setEnabled(false);
                VarBox.RunApp = false;
                if (QMessageBox::question(this, "提示", "您想退出后重启吗？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)==QMessageBox::Yes)
                    qApp->exit(RETCODE_RESTART);
                else
                    qApp->quit();
            }
            else
                QMessageBox::critical(this, "出错", "无法删除，可能是权限不够！");
        }
    }
    else
    {
        jobTip->showTip("文件本就不存在！");
    }
}


void Dialog::on_pushButton_6_clicked()
{
    del_file(this, FuncBox::get_dat_path() + "\\ImgData.json");
}



void Dialog::on_pushButton_8_clicked()
{
    del_file(this, FuncBox::get_dat_path() + "\\AppId.txt");
}

void Dialog::on_pushButton_2_clicked()
{
    close();
}

void Dialog::setTheme()
{
    ui->frame->setStyleSheet(QString("QFrame{background-color:rgba(%1);}QLabel{border-radius: 3px;background-color: transparent;}Line{background-color:black};").arg(color_theme[(int)VarBox.CurTheme]));
    jobTip->showTip("设置成功！");
}

void Dialog::on_pushButton_5_clicked()
{
    VarBox.CurTheme = (COLOR_THEME)ui->comboBox_3->currentIndex();
    QSettings IniWrite(FuncBox::get_ini_path(), QSettings::IniFormat);
    IniWrite.beginGroup("UI");
    IniWrite.setValue("ColorTheme", (int)VarBox.CurTheme);
    setTheme();
    qDebug() << (int)VarBox.CurTheme;
}


void Dialog::on_pushButton_9_clicked()
{
    ShellExecute(NULL, "open", "https://github.com/yjmthu/Speed-Box", NULL, NULL, SW_SHOW);
}

void Dialog::on_pushButton_11_clicked()
{
    QString str = ui->linePictuerPath->text();
    QDir dir;
    if (dir.exists(str) || (!str.isEmpty() && dir.mkdir(str)))
        VarBox.FamilyPath = str.replace("/", "\\");
    else
    {
        if (dir.exists(VarBox.FamilyPath) || dir.mkdir(VarBox.FamilyPath))
            ui->linePictuerPath->setText(VarBox.FamilyPath.replace("/", "\\"));
        else
            VarBox.FamilyPath = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    }
    FuncBox::get_wal_path();
    jobTip->showTip("修改成功！");
}

// 展示版本信息
void Dialog::on_pushButton_13_clicked()
{
    jobTip->showTip(QString("版本号：") + VarBox.Version, 1000);
}

// 检查是否有更新 https://gitee.com/yjmthu/Speed-Box/raw/main/Update.json
void Dialog::on_pushButton_10_clicked()
{
    static bool wait = false;
    if (wait) return; else wait = true;
    std::string str;
    if (FuncBox::getWebCode("https://gitee.com/yjmthu/Speed-Box/raw/main/Update.json", str, false) && !str.empty())
    {
        const YJsonItem json(str);
        if (json.getType() == YJson::YJSON_OBJECT)
        {
            const char* newVersion = json.findItem("Latest Version")->getValueSring();
            qout << newVersion;
            if (StrCompare(VarBox.Version, newVersion))
            {
                jobTip->showTip("当前已经是最新版本, 不过你仍可尝试下载更新！", 1000);
                ui->pushButton_12->setEnabled(true);
            }
            else
            {
                if (FuncBox::versionBefore(VarBox.Version, newVersion))
                {
                    jobTip->showTip(((QString("\t有新版本已经出现！\n当前版本：")+=VarBox.Version)+="; 最新版本：")+=newVersion, 2000);
                    ui->pushButton_12->setEnabled(true);
                }
                else
                {
                    jobTip->showTip(newVersion);
                    jobTip->showTip("检查更新失败，请手动打开浏览器下载访问Gitee更新。", 2000);
                }
            }
        }
    }
    wait = false;
}

// 下载更新  https://gitee.com/yjmthu/Speed-Box/raw/main/Update.json
void Dialog::on_pushButton_12_clicked()
{
    static bool wait = false;
    if (wait) return; else wait = true;
    std::string str; jobTip->showTip("请耐心等待几秒...");
    if (FuncBox::getWebCode("https://gitee.com/yjmthu/Speed-Box/raw/main/Update.json", str, false) && !str.empty())
    {
        YJsonItem json(str);
        if (json.getType() == YJson::YJSON_OBJECT)
        {
            YJsonItem *qtVersion = json.findItem("Qt Version");
            if (qtVersion->getType() != YJson::YJSON_STRING || !StrCompare(qtVersion->getValueSring(), "6.1.2"))
            {
                jobTip->showTip("下载更新失败，请手动打开浏览器到Gitee下载！", 2000);
                return;
            }
            YJsonItem *urls = json.findItem("Files"); YJsonItem *child = nullptr;
            if (urls && urls->getType() == YJson::YJSON_OBJECT)
            {
                if (!(child = urls->findItem("main")))
                {
                    jobTip->showTip("下载失败！");
                    return;
                }
                const char* url1 = child->getValueSring();
                if (!(child = urls->findItem("update")))
                {
                    jobTip->showTip("下载失败！");
                    return;
                }
                const char* url2 = child->getValueSring();
                qout << "文件地址：" << url1 << url2;
                QString temp_str = FuncBox::get_dat_path()+"\\SpeedBox.exe";
                if (QFile::exists(temp_str)) QFile::remove(temp_str);
                if (FuncBox::downloadImage(url1, temp_str, false)
                        &&
                    FuncBox::downloadImage(url2, qApp->applicationDirPath().replace("/", "\\")+"\\update.exe", false))
                {
                    if (QMessageBox::information(this, "更新已经下载完成，重启后完成更新！", "提示", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                    {
                        VarBox.RunApp = false;
                        qApp->exit(RETCODE_UPDATE);
                    }
                    else
                        jobTip->showTip("成功取消更新。", 700);
                }
                else
                    jobTip->showTip("下载失败！");
            }
            else
            {
                jobTip->showTip("未知原因，下载失败！");
            }
        }
        else
        {
            jobTip->showTip("下载失败。");
        }
    }
    wait = false;
}


void Dialog::on_pushButton_14_clicked()
{
    ShellExecute(NULL, "open", "https://gitee.com/yjmthu/Speed-Box", NULL, NULL, SW_SHOW);
}

