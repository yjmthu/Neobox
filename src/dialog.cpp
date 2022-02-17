#include <string.h>
#include <fstream>

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
#include <QFontDatabase>
#include <QComboBox>
#include <QSystemTrayIcon>

#include <3rd_qxtglobalshortcut/qxtglobalshortcut.h>

#include "ystring.h"
#include "yjson.h"
#include "dialog.h"
#include "ui_dialog.h"
#include "form.h"
#include "blankform.h"
#include "gmpoperatetip.h"
#include "downloadprogress.h"
#include "wallpaper.h"
#include "bingsetting.h"
#include "qstylesheet.h"
#include "globalfn.h"
#include "translater.h"
#include "aboutnew.h"

const QStringList Dialog::reg_keys {
    QStringLiteral("HKEY_CURRENT_USER\\SOFTWARE\\Classes\\*\\shell\\QCoper"),
    QStringLiteral("HKEY_CURRENT_USER\\SOFTWARE\\Classes\\Directory\\shell\\QCoper"),
    QStringLiteral("HKEY_CURRENT_USER\\SOFTWARE\\Classes\\Directory\\Background\\shell\\QCoper"),
    QStringLiteral("mshta vbscript:clipboarddata.setdata(\"text\",\"%%1\")(close)")
};

Dialog::Theme Dialog::curTheme { Dialog::Theme::White };

Dialog::Dialog():
    SpeedWidget<QWidget>(nullptr),
    ui(new Ui::Dialog),
    wallpaper(VarBox->m_pWallpaper),
    m_sheet(VarBox->m_pForm->m_sheet),
    formFontJson(new YJson("BoxFont.json", YJson::UTF8BOM)),
    formPart({VarBox->m_pForm->frame, VarBox->m_pForm->labMemory, VarBox->m_pForm->labUp, VarBox->m_pForm->labDown})
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
    delete formFontJson;
    delete buttonGroup;
    delete ui;
    qout << "析构dialog结束";
}

void Dialog::initUi()
{
    typedef Wallpaper::Type Type;
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);  //| Qt::WindowStaysOnTopHint
    setAttribute(Qt::WA_DeleteOnClose, true);

    QFile qss(":/qss/dialog_style.qss");
    qss.open(QFile::ReadOnly);
    setStyleSheet(QString(qss.readAll()).arg("宋体"));
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
    buttonGroup->button(static_cast<int>(wallpaper->m_paperType))->setChecked(true);
    ui->lineAppData->setText(QDir::toNativeSeparators(QDir::currentPath()));

    ui->checkBox_2->setChecked(false);
    ui->LinePath->setText(wallpaper->m_nativeDir);
    ui->sLdPageNum->setValue(wallpaper->m_pageNum);
    ui->sLdTimeInterval->setValue(wallpaper->m_timeInterval);
    ui->labTimeInterval->setText(QString::number(wallpaper->m_timeInterval));
    ui->chkEnableChangePaper->setChecked(wallpaper->m_autoChange);
    ui->usrCmd->setText(wallpaper->m_userCommand);
#ifdef Q_OS_WIN32
    ui->cBxAutoStart->setChecked(!QSettings(QStringLiteral("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), QSettings::NativeFormat).value("SpeedBox").toString().compare(QDir::toNativeSeparators(qApp->applicationFilePath())));
#elif defined (Q_OS_LINUX)
    ui->cBxAutoStart->setChecked(QFile::exists(QDir::homePath()+"/.config/autostart/SpeedBox.desktop"));
#endif
    ui->lineEdit_2->setText(QDir::toNativeSeparators(qApp->applicationDirPath()));
    switch (wallpaper->m_paperType) {
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
#if defined (Q_OS_WIN32)
    if (VarBox->m_pathToOpen.compare(QDir::toNativeSeparators(qApp->applicationDirPath())))
        ui->radioButton_14->setChecked(true);
    else
        ui->radioButton_13->setChecked(true);
#elif defined (Q_OS_LINUX)
    if (VarBox->PathToOpen.compare(qApp->applicationDirPath()))
        ui->radioButton_14->setChecked(true);
    else
        ui->radioButton_13->setChecked(true);
#endif
    ui->cBxTuoPanIcon->setChecked(VarBox->m_bTuoPanIcon);
    ui->cBxTieBianHide->setChecked(VarBox->m_pForm->m_tieBianHide);
    ui->cBxFormToolTip->setChecked(VarBox->m_pForm->m_showToolTip);
    ui->pushButton_4->setText(QStringLiteral("确定"));
    ui->comboBox_3->setCurrentIndex((int)curTheme);
    ui->lineEdit->setText(VarBox->m_pathToOpen);
    ui->checkBox_3->setChecked(wallpaper->m_firstChange);
    ui->BtnChooseFolder->setEnabled(ui->rBtnNative->isChecked());
    ui->cBxEnableUSBhelper->setChecked(VarBox->m_bEnableUSBhelper);
    ui->cBxEnableMarkdown->setChecked(VarBox->m_bMarkdownNote);
    ui->cBxEnableSquareClock->setChecked(VarBox->m_bSquareClock);
    ui->cBxEnableRoundClock->setChecked(VarBox->m_bRoundClock);
    ui->cBxFanyierShiftA->setChecked(Translater::getShiftAState());
    ui->cBxFanyierShiftZ->setChecked(Translater::getShiftZState());
    setFrameStyle(static_cast<int>(curTheme));
    checkSettings();
    loadFormStyle();
    move((VarBox->m_dScreenWidth - width()) / 2, (VarBox->m_dScreenHeight - height()) / 2);
}

void Dialog::loadFormStyle()
{
    QString str(QStringLiteral("QPushButton{background-color:rgb(%1,%2,%3);border:1px solid black;border-radius:4px;}").arg(QString::number(m_sheet->bk_red), QString::number(m_sheet->bk_green), QString::number(m_sheet->bk_blue)));
    ui->pBtnFormColor->setStyleSheet(str);
    ui->sLdTranparent->setValue(m_sheet->bk_alpha);
    ui->sLdBorderRadius->setValue(m_sheet->bd_radius);
    ui->sLdFuzzy->setValue(m_sheet->bk_fuzzy);
    ui->cBxEnableBorderRadius->setChecked(m_sheet->bd_have & QStyleSheet::Border);
    ui->cBxLeftBorderRadius->setChecked(m_sheet->bd_have & QStyleSheet::Left);
    ui->cBxRightBorderRadius->setChecked(m_sheet->bd_have & QStyleSheet::Right);
    ui->cBxTopBorderRadius->setChecked(m_sheet->bd_have & QStyleSheet::Top);
    ui->cBxBottomBorderRadius->setChecked(m_sheet->bd_have & QStyleSheet::Bottom);
    YJson* temp = formFontJson->find("image");
    ui->lineMultyPath->setText(temp->getType() == YJson::Null ? "null" : temp->getValueString());
}

void Dialog::initChildren()
{
	buttonGroup = new QButtonGroup;
}

void Dialog::initConnects()
{
    qout << "对话框链接A";
    connect(ui->pBtnAppWebSite, &QPushButton::clicked, this, std::bind(&QDesktopServices::openUrl, QUrl(QStringLiteral("https://yjmthu.github.io/Speed-Box"))));
    connect(ui->rBtnNative, &QRadioButton::toggled, this, [this](bool checked){ui->BtnChooseFolder->setEnabled(checked);});
    connect(ui->BtnChooseFolder, &QToolButton::clicked, this, &Dialog::chooseFolder);
    connect(ui->pBtnCancel, &QPushButton::clicked, this, &Dialog::close);
    connect(ui->pBtnOk, &QPushButton::clicked, this, &Dialog::saveWallpaperSettings);
    connect(ui->pBtnOpenAppData, &QPushButton::clicked, VarBox, std::bind(GlobalFn::openDirectory, QDir::currentPath()));
    connect(ui->pBtnOpenPicturePath, &QPushButton::clicked, this, &Dialog::openPicturePath);
    connect(ui->linePictuerPath, &QLineEdit::returnPressed, this, &Dialog::linePictuerPathReturn);
    connect(ui->cBxFanyierShiftA, &QCheckBox::clicked, this, [=](bool checked){
        auto tlr = VarBox->m_pForm->translater;
        if (tlr) {
            if (checked) {
                tlr->m_pShortcutHide = new QxtGlobalShortcut(QKeySequence("Shift+A"));
                tlr->setShiftA();
            } else {
                delete tlr->m_pShortcutHide;
                tlr->m_pShortcutHide = nullptr;
            }
        }
        GlobalFn::saveOneSet<bool>("Translate", "HideShiftA", checked);
        jobTip->showTip("修改成功！");
    });
    connect(ui->cBxFanyierShiftZ, &QCheckBox::clicked, this, [=](bool checked){
        auto tlr = VarBox->m_pForm->translater;
        if (tlr) {
            if (checked) {
                tlr->m_pShortcutShow = new QxtGlobalShortcut(QKeySequence("Shift+Z"));
                tlr->setShiftZ();
            } else {
                delete tlr->m_pShortcutShow;
                tlr->m_pShortcutShow = nullptr;
            }
        }
        GlobalFn::saveOneSet<bool>("Translate", "ShowShiftZ", checked);
        jobTip->showTip("修改成功！");
    });
    connect(ui->cBxAutoStart, &QPushButton::clicked, this, [this](bool checked){
#ifdef Q_OS_WIN32
        QSettings* reg = new QSettings(
            QStringLiteral("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"),
            QSettings::NativeFormat);
        if (checked)
            reg->setValue("SpeedBox", QDir::toNativeSeparators(qApp->applicationFilePath()));
        else
            reg->remove("SpeedBox");
        delete reg;
        jobTip->showTip("修改成功！");
#elif defined Q_OS_LINUX
        QString linkpath = QDir::homePath()+"/.config/autostart/SpeedBox.desktop";
        QFile file(":/scripts/SpeedBox.desktop");
        if (checked && !QFile::exists(linkpath))
        {
            if (!file.open(QFile::ReadOnly))
            {
                jobTip->showTip("修改失败！");
                return ;
            }
            QString str = file.readAll();
            file.close();
            QFile newFile(linkpath);
            if (!newFile.open(QFile::WriteOnly))
            {
                jobTip->showTip("修改失败！");
                return ;
            }
            newFile.write(str.arg(qApp->applicationFilePath(), "/home/yjmthu/OneDrive/PersonalFiles/UserFiles/GitHub/Speed-Box/src/icons/speedbox.ico", VarBox->Version).toUtf8());
            newFile.close();
            //赋予快捷方式可执行权限。
            QStringList lst;
            lst.append("+x");
            lst.append(linkpath);
            VarBox->runCmd("chmod", lst, 0);
            jobTip->showTip("修改成功！");
        }
        else
        {
            if (QFile::remove(linkpath))
                jobTip->showTip("修改成功！");
            else
                jobTip->showTip("修改失败！");
        }
#endif
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
    connect(ui->pushButton, &QPushButton::clicked, VarBox, std::bind(GlobalFn::openDirectory, qApp->applicationDirPath()));
    connect(ui->cBxEnableUSBhelper, &QCheckBox::clicked, this, [this](bool checked){
        curTheme = static_cast<Theme>(ui->comboBox_3->currentIndex());
        VarBox->m_bEnableUSBhelper = checked;
        GlobalFn::saveOneSet<bool>(QStringLiteral("Apps"), QStringLiteral("UsbHelper"), VarBox->m_bEnableUSBhelper);
        jobTip->showTip("应用并保存成功！");
    });
    connect(ui->sLdTranparent, &QSlider::valueChanged, this, [=](int value){
        int index = ui->cBxFormPart->currentIndex();
        switch (ui->cBxSetBorder->checkState()) {
        case Qt::Unchecked:
            m_sheet[index].bk_alpha = value;
            break;
        case Qt::PartiallyChecked:
            m_sheet[index].ft_alpha = value;
            break;
        case Qt::Checked:
            m_sheet[index].bd_alpha = value;
            break;
        default:
            break;
        }
        formPart[index]->setStyleSheet(m_sheet[index].getString(index));
    });
    connect(ui->cBxEnableMarkdown, &QCheckBox::clicked, VarBox, &VARBOX::createMarkdown);
    connect(ui->cBxEnableSquareClock, &QCheckBox::clicked, VarBox, &VARBOX::createSquareClock);
    connect(ui->cBxEnableRoundClock, &QCheckBox::clicked, VarBox, &VARBOX::createRoundClock);
    connect(ui->pBtnFormColor, &QPushButton::clicked, this, [=](){
        int index = ui->cBxFormPart->currentIndex();
        QString str("QPushButton{background-color:rgb(%1,%2,%3);border:1px solid black;border-radius:4px;}");
        QColor color = QColorDialog::getColor(ui->cBxSetBorder->isChecked()?QColor(m_sheet[index].bd_red, m_sheet[index].bd_green, m_sheet[index].bd_blue):QColor(m_sheet[index].bk_red, m_sheet[index].bk_green, m_sheet[index].bk_blue));
        if (color.isValid()) {
            switch (ui->cBxSetBorder->checkState()) {
            case Qt::Unchecked:
                m_sheet[index].bk_red = color.red();
                m_sheet[index].bk_green = color.green();
                m_sheet[index].bk_blue = color.blue();
                break;
            case Qt::PartiallyChecked:
                m_sheet[index].ft_red = color.red();
                m_sheet[index].ft_green = color.green();
                m_sheet[index].ft_blue = color.blue();
                break;
            case Qt::Checked:
                m_sheet[index].bd_red = color.red();
                m_sheet[index].bd_green = color.green();
                m_sheet[index].bd_blue = color.blue();
                break;
            default:
                break;
            }
            ui->pBtnFormColor->setStyleSheet(str.arg(QString::number(color.red()), QString::number(color.green()), QString::number(color.blue())));
            formPart[index]->setStyleSheet(m_sheet[index].getString(index));
        }
    });
    connect(ui->cBxSetBorder, &QCheckBox::stateChanged, this, [=](int state){
        int index = ui->cBxFormPart->currentIndex();
        YJson * temp;
        QString str("QPushButton{background-color:rgb(%1,%2,%3);border:1px solid black;border-radius:4px;}");
        switch (state) {
        case Qt::Unchecked: // 背景
            temp = formFontJson->find("image");
            ui->lineMultyPath->setText(temp->getType() == YJson::Null ? "null" : temp->getValueString());
            ui->cBxSetBorder->setText(QStringLiteral("背景"));
            ui->pBtnFormColor->setStyleSheet(str.arg(QString::number(m_sheet[index].bk_red), QString::number(m_sheet[index].bk_green), QString::number(m_sheet[index].bk_blue)));
            ui->sLdTranparent->setValue(m_sheet[index].bk_alpha);
            ui->sLdBorderRadius->setValue(m_sheet[index].bk_win);
            ui->label_5->setText(QStringLiteral("模糊"));
            ui->label_6->setText(QStringLiteral("背景"));
            ui->label_12->setText(QStringLiteral("玻璃"));
            ui->sLdFuzzy->setMaximum(99);
            ui->sLdFuzzy->setValue(m_sheet[index].bk_fuzzy);
            my_on_border_radius(ui->rBtnBorder->isChecked());
            break;
        case Qt::PartiallyChecked:  //
            temp = formFontJson->find("user")->find((*formFontJson)["index"][index].getValueString())->find("family");
            ui->lineMultyPath->setText(temp->getType() == YJson::Null ? "null" : temp->getValueString());
            ui->cBxSetBorder->setText(QStringLiteral("字体"));
            ui->pBtnFormColor->setStyleSheet(str.arg(QString::number(m_sheet[index].ft_red), QString::number(m_sheet[index].ft_green), QString::number(m_sheet[index].ft_blue)));
            ui->sLdTranparent->setValue(m_sheet[index].ft_alpha);
            ui->sLdBorderRadius->setValue(m_sheet[index].bd_radius);
            ui->label_5->setText(QStringLiteral("大小"));
            ui->label_6->setText(QStringLiteral("字体"));
            ui->label_12->setText(QStringLiteral("圆角"));
            ui->sLdFuzzy->setMaximum(30);
            ui->sLdFuzzy->setValue(m_sheet[index].ft_size);
            my_on_border_radius(ui->rBtnBorder->isChecked());
            break;
        case Qt::Checked:
            temp = formFontJson->find("image");
            ui->lineMultyPath->setText(temp->getType() == YJson::Null ? "null" : temp->getValueString());
            ui->cBxSetBorder->setText(QStringLiteral("边线"));
            ui->pBtnFormColor->setStyleSheet(str.arg(QString::number(m_sheet[index].bd_red), QString::number(m_sheet[index].bd_green), QString::number(m_sheet[index].bd_blue)));
            ui->sLdTranparent->setValue(m_sheet[index].bd_alpha);
            ui->sLdBorderRadius->setValue(m_sheet[index].bd_radius);
            ui->label_5->setText(QStringLiteral("粗细"));
            ui->label_6->setText(QStringLiteral("背景"));
            ui->label_12->setText(QStringLiteral("圆角"));
            ui->sLdFuzzy->setMaximum(10);
            ui->sLdFuzzy->setValue(m_sheet[index].bd_width);
            my_on_border_radius(ui->rBtnBorder->isChecked());
            break;
        default:
            break;
        }
    });
    connect(ui->sLdBorderRadius, &QSlider::valueChanged, this, [=](int value){
        int index = ui->cBxFormPart->currentIndex();
        m_sheet[index].bd_radius = value;
        formPart[index]->setStyleSheet(m_sheet[index].getString(index));
    });
    connect(ui->sLdFuzzy, &QSlider::valueChanged, this, [=](int value){
        int index = ui->cBxFormPart->currentIndex();
        switch (ui->cBxSetBorder->checkState()) {
        case Qt::Unchecked:
            m_sheet[index].bk_fuzzy = value;
            formPart[index]->setStyleSheet(m_sheet[index].getString(index));
            break;
        case Qt::PartiallyChecked:
            m_sheet[index].ft_size = value;
            formPart[index]->setStyleSheet(m_sheet[index].getString(index));
            break;
        case Qt::Checked:
            m_sheet[index].bd_width = value;
            formPart[index]->setStyleSheet(m_sheet[index].getString(index));
            break;
        default:
            break;
        }
    });
    connect(ui->pBtnSaveFormStyle, &QPushButton::clicked, this, [=](){
        if (QStyleSheet::toFile(m_sheet)) {
            jobTip->showTip(QStringLiteral("保存成功!"));
        } else {
            jobTip->showTip(QStringLiteral("保持失败！"));
        }
    });
    // Qt5才有 QOverload
    connect(ui->cBxFormPart, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
        YJson* temp;
        QString str("QPushButton{background-color:rgb(%1,%2,%3);border:1px solid black;border-radius:4px;}");
        switch (ui->cBxSetBorder->checkState()) {
        case Qt::Unchecked:
            temp = formFontJson->find("image");
            ui->lineMultyPath->setText(temp->getType() == YJson::Null ? "null" : temp->getValueString());
            ui->pBtnFormColor->setStyleSheet(str.arg(QString::number(m_sheet[index].bk_red), QString::number(m_sheet[index].bk_green), QString::number(m_sheet[index].bk_blue)));
            ui->sLdTranparent->setValue(m_sheet[index].bk_alpha);
            ui->sLdBorderRadius->setValue(m_sheet[index].bk_win);
            ui->sLdFuzzy->setValue(m_sheet[index].bk_fuzzy);
            my_on_border_radius(ui->rBtnBorder->isChecked());
            break;
        case Qt::PartiallyChecked:
            temp = formFontJson->find("user")->find((*formFontJson)["index"][index].getValueString())->find("family");
            ui->lineMultyPath->setText(temp->getType() == YJson::Null ? "null" : temp->getValueString());
            ui->pBtnFormColor->setStyleSheet(str.arg(QString::number(m_sheet[index].ft_red), QString::number(m_sheet[index].ft_green), QString::number(m_sheet[index].ft_blue)));
            ui->sLdTranparent->setValue(m_sheet[index].ft_alpha);
            ui->sLdBorderRadius->setValue(m_sheet[index].bd_radius);
            ui->sLdFuzzy->setValue(m_sheet[index].ft_size);
            my_on_border_radius(ui->rBtnBorder->isChecked());
            break;
        case Qt::Checked:
            temp = formFontJson->find("image");
            ui->lineMultyPath->setText(temp->getType() == YJson::Null ? "null" : temp->getValueString());
            ui->pBtnFormColor->setStyleSheet(str.arg(QString::number(m_sheet[index].bd_red), QString::number(m_sheet[index].bd_green), QString::number(m_sheet[index].bd_blue)));
            ui->sLdTranparent->setValue(m_sheet[index].bd_alpha);
            ui->sLdBorderRadius->setValue(m_sheet[index].bd_radius);
            ui->sLdFuzzy->setValue(m_sheet[index].bd_width);
            my_on_border_radius(ui->rBtnBorder->isChecked());
            break;
        default:
            break;
        }
    });
    connect(ui->cBxTuoPanIcon, &QCheckBox::clicked, VarBox, &VARBOX::createTrayIcon);
    connect(ui->cBxTieBianHide, &QCheckBox::clicked, VarBox, [](bool checked){
        VarBox->m_pForm->m_tieBianHide = checked;
        GlobalFn::saveOneSet<bool>(QStringLiteral("UI"), QStringLiteral("TieBianHide"), checked);
    });
    connect(ui->cBxFormToolTip, &QCheckBox::clicked, VarBox, [](bool checked){
        VarBox->m_pForm->m_showToolTip = checked;
        GlobalFn::saveOneSet<bool>(QStringLiteral("UI"), QStringLiteral("ShowToolTip"), checked);
    });
    connect(ui->cBxLeftBorderRadius, &QCheckBox::clicked, VarBox, [=](bool checked){
        int index = ui->cBxFormPart->currentIndex();
        if (ui->rBtnBorder->isChecked()) {
            if (checked)
                m_sheet[index].bd_have |= QStyleSheet::Left;
            else
                m_sheet[index].bd_have &= ~QStyleSheet::Left;
        } else {
            if (checked)
                m_sheet[index].bd_have |= QStyleSheet::TopLeft;
            else
                m_sheet[index].bd_have &= ~QStyleSheet::TopLeft;
        }
        formPart[index]->setStyleSheet(m_sheet[index].getString(index));
    });
    connect(ui->cBxRightBorderRadius, &QCheckBox::clicked, VarBox, [=](bool checked){
        int index = ui->cBxFormPart->currentIndex();
        if (ui->rBtnBorder->isChecked()) {

            if (checked)
                m_sheet[index].bd_have |= QStyleSheet::Right;
            else
                m_sheet[index].bd_have &= ~QStyleSheet::Right;
        } else {
            if (checked)
                m_sheet[index].bd_have |= QStyleSheet::BottomLeft;
            else
                m_sheet[index].bd_have &= ~QStyleSheet::BottomLeft;
        }
        formPart[index]->setStyleSheet(m_sheet[index].getString(index));
    });
    connect(ui->cBxTopBorderRadius, &QCheckBox::clicked, VarBox, [=](bool checked){
        int index = ui->cBxFormPart->currentIndex();
        if (ui->rBtnBorder->isChecked()) {
            if (checked)
                m_sheet[index].bd_have |= QStyleSheet::Top;
            else
                m_sheet[index].bd_have &= ~QStyleSheet::Top;
        } else {
            if (checked)
                m_sheet[index].bd_have |= QStyleSheet::TopRight;
            else
                m_sheet[index].bd_have &= ~QStyleSheet::TopRight;
        }
        formPart[index]->setStyleSheet(m_sheet[index].getString(index));
    });
    connect(ui->cBxBottomBorderRadius, &QCheckBox::clicked, VarBox, [=](bool checked){
        int index = ui->cBxFormPart->currentIndex();
        if (ui->rBtnBorder->isChecked()) {
            if (checked)
                m_sheet[index].bd_have |= QStyleSheet::Bottom;
            else
                m_sheet[index].bd_have &= ~QStyleSheet::Bottom;
        } else {
            if (checked)
                m_sheet[index].bd_have |= QStyleSheet::BottomRight;
            else
                m_sheet[index].bd_have &= ~QStyleSheet::BottomRight;
        }
        formPart[index]->setStyleSheet(m_sheet[index].getString(index));
    });
    connect(ui->rBtnBorder, &QRadioButton::toggled, this, &Dialog::my_on_border_radius);
    connect(ui->cBxEnableBorderRadius, &QCheckBox::clicked, VarBox, [=](bool checked){
        int index = ui->cBxFormPart->currentIndex();
        if (checked) {
            if (ui->rBtnBorder->isChecked())
                m_sheet[index].bd_have |= QStyleSheet::Border;
            else
                m_sheet[index].bd_have |= QStyleSheet::BorderRadius;
        } else {
            if (ui->rBtnBorder->isChecked())
                m_sheet[index].bd_have &= ~QStyleSheet::Border;
            else
                m_sheet[index].bd_have &= ~QStyleSheet::BorderRadius;
        }
        formPart[index]->setStyleSheet(m_sheet[index].getString(index));
        qout << formPart[index]->styleSheet();
    });
    connect(ui->pBtnCheckUpdate, &QPushButton::clicked, this, [](){
        AboutNew chekNew;
        chekNew.exec();
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

void Dialog::setFrameStyle(int index)
{
    QString frameSheet("QFrame{background-color:rgba(%1,%2,%3,%4);}QLabel{border-radius:3px;background-color:transparent;}Line{background-color:black};");
    const int m_themes[8][4]
    {
        { 255, 255, 255, 255 },
        { 255, 255, 255, 150 },
        { 155,  89, 182, 150 },
        { 255,  10,  10, 150 },
        {  50, 200,  50, 150 },
        { 135, 160, 250, 150 },
        { 200, 100, 100, 150 },
        {   0,   0,   0, 100 }
    };
    for (int i=0; i<4; i++)
    {
        frameSheet = frameSheet.arg(QString::number(m_themes[index][i]));
    }
    ui->frame->setStyleSheet(frameSheet);
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
    *const_cast<Dialog**>(&(VarBox->m_pDialog)) = nullptr;
    event->accept();
}

void Dialog::chooseFolder()
{
	QDir d;
    if (wallpaper->m_nativeDir.isEmpty() || (!d.exists(wallpaper->m_nativeDir) && !d.mkdir(wallpaper->m_nativeDir)))
	{
        wallpaper->m_nativeDir = QDir::toNativeSeparators(QStandardPaths::writableLocation((QStandardPaths::PicturesLocation)));
        GlobalFn::saveOneSet<QString>("Wallpaper", "NativeDir", wallpaper->m_nativeDir);
        ui->LinePath->setText(wallpaper->m_nativeDir);
	}
    QString titile = QStringLiteral("请选择一个文件夹");
    QString dir = QFileDialog::getExistingDirectory(NULL, titile, wallpaper->m_nativeDir, QFileDialog::ShowDirsOnly);
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
    wallpaper->m_update_wallhaven_api = ui->checkBox_2->isChecked();
    wallpaper->m_paperType = static_cast<Wallpaper::Type>(buttonGroup->checkedId());
    wallpaper->m_nativeDir = ui->LinePath->text();
    wallpaper->m_pageNum = ui->sLdPageNum->value();
    wallpaper->m_timeInterval = ui->sLdTimeInterval->value();
    wallpaper->m_autoChange = ui->chkEnableChangePaper->isChecked();
    wallpaper->m_userCommand = ui->usrCmd->text();
    wallpaper->m_firstChange = ui->checkBox_3->isChecked();
    for (int type = 0; type < 2; ++type)
    {
        if (type == 0) {
            if (wallpaper->isActive()) {
                if (QMessageBox::question(this, "警告", "您之前的壁纸类型更换正在生效中，是否终止？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                {
                    wallpaper->kill();
                    if (QMessageBox::question(this, "提示", "终止成功！是否继续？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                        continue;
                }
                break;
            }
        } else if (type == 1) {
            if (wallpaper->m_firstChange) {
                qout << "首更1";
                wallpaper->applyClicked = true;
                wallpaper->push_back();
            }
            if (wallpaper->m_autoChange)
            {
                wallpaper->timer->setInterval(wallpaper->m_timeInterval * 60000);  // 设置时间间隔,Timer的单位是毫秒
                wallpaper->timer->start();
                qout << "开始更换壁纸";
            } else
                wallpaper->timer->stop();
            if (QMessageBox::question(this, "提示", "应用成功！是否保存设置？", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
                saveWallpaperSettings();
        }
    }
}

void Dialog::setFormStyleSheet()
{
    int index = ui->cBxFormPart->currentIndex();
    QStyleSheet& i = m_sheet[index];
    if (ui->cBxSetBorder->isChecked()) {
        i.bd_alpha = ui->sLdTranparent->value();
        i.bd_radius = ui->sLdBorderRadius->value();
        i.bd_width = ui->sLdFuzzy->value();
    } else {
        i.bk_alpha = ui->sLdTranparent->value();
        i.bd_radius = ui->sLdBorderRadius->value();
        i.bd_width = ui->sLdFuzzy->value();
    }


    formPart[index]->setStyleSheet(m_sheet[index].getString(index));
}

void Dialog::checkSettings()
{
    QCheckBox *boxs[3] {ui->chkFile, ui->chkFolder, ui->chkFolderBack};
    for (int i=0; i<3; i++)
    {
        boxs[i]->setChecked(QSettings(reg_keys[i], QSettings::NativeFormat).contains("."));
    }
}

void Dialog::on_pBtnApply_2_clicked()
{
    static QString icon_path = "Copy.ico";
	QFile file(icon_path);
	if (!file.exists() && file.open(QIODevice::WriteOnly))
	{
        QFile qss(QStringLiteral(":/icons/copy.ico"));
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
                settings.setValue("Icon", QDir::toNativeSeparators(QDir().absoluteFilePath(icon_path)));
                settings.beginGroup("command");
                settings.setValue(".", reg_keys[3].arg(type==2?'V':'1'));
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
    jobTip->showTip(QStringLiteral("设置成功！"));
}

void Dialog::openPicturePath()
{
    QString& img_folder = (ui->rBtnBingApi->isChecked())? wallpaper->m_bing_folder: (ui->rBtnOtherApi->isChecked()? wallpaper->m_other_folder: wallpaper->m_wallhaven_folder);
    QString str = ui->linePictuerPath->text();
	QDir dir;
    if (dir.exists(str) || (!str.isEmpty() && dir.mkdir(str)))
        img_folder = QDir::toNativeSeparators(str);
	else
	{
        if (dir.exists(img_folder) || dir.mkdir(img_folder))
            ui->linePictuerPath->setText(QDir::toNativeSeparators(img_folder));
		else
            img_folder = QDir::toNativeSeparators(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
	}
    if (dir.exists(img_folder))
        GlobalFn::openDirectory(img_folder);
	else
		QMessageBox::warning(this, "警告", "路径不存在！", QMessageBox::Ok, QMessageBox::Ok);
}

void Dialog::linePictuerPathReturn()
{
    QString& img_folder = (ui->rBtnBingApi->isChecked())? wallpaper->m_bing_folder: (ui->rBtnOtherApi->isChecked()? wallpaper->m_other_folder: wallpaper->m_wallhaven_folder);
    QDir dir;
	if (dir.exists(ui->linePictuerPath->text()))
	{
        img_folder = ui->linePictuerPath->text();
    } else {
		QMessageBox::warning(this, "警告", "路径不存在！", QMessageBox::Ok, QMessageBox::Ok);
        ui->linePictuerPath->setText(img_folder);
	}
}


void Dialog::on_radioButton_13_clicked()
{
    ui->lineEdit->setText(QDir::toNativeSeparators(qApp->applicationDirPath()));
    ui->pushButton_4->setText("确定");
}


void Dialog::on_radioButton_14_clicked()
{
    ui->lineEdit->setText(QStringLiteral("点击右侧按钮选择文件"));
    ui->pushButton_4->setText(QStringLiteral("选择"));
}

void Dialog::on_pushButton_4_clicked()
{
    if (ui->pushButton_4->text().compare(QStringLiteral("确定")))
	{
        QString titile = QStringLiteral("请选择一个文件夹");
        QString dir = QFileDialog::getExistingDirectory(ui->frame, titile, QStandardPaths::writableLocation(QStandardPaths::HomeLocation), QFileDialog::ShowDirsOnly);
        ui->pushButton_4->setText("确定");
        if (!dir.isEmpty())
		{
            ui->lineEdit->setText(QDir::toNativeSeparators(dir));
		}
        return;
	}
    VarBox->m_pathToOpen = ui->lineEdit->text();
    QDir().mkdir(VarBox->m_pathToOpen);
    GlobalFn::saveOneSet<QString>(QStringLiteral("Dirs"), QStringLiteral("OpenDir"), VarBox->m_pathToOpen);
    jobTip->showTip(QStringLiteral("更换路径成功！"));
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

void Dialog::on_pushButton_5_clicked()
{
    curTheme = static_cast<Theme>(ui->comboBox_3->currentIndex());
    QSettings IniWrite("SpeedBox.ini", QSettings::IniFormat);
    IniWrite.setIniCodec(QTextCodec::codecForName("UTF-8"));
    IniWrite.beginGroup("UI");
    IniWrite.setValue("ColorTheme", (int)curTheme);
    setFrameStyle(static_cast<int>(curTheme));
    jobTip->showTip(QStringLiteral("设置成功！"));
}

#if 0

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
        const QByteArray& data = rep->readAll();
        if (rep->error() != QNetworkReply::NoError)
        {
            rep->deleteLater();
            jobTip->showTip(QStringLiteral("下载失败！"));
            mgr->deleteLater();
            return;
        }
        rep->deleteLater();
        const YJson json(data);
        if (json.getType() != YJson::Object)
        {
            jobTip->showTip(QStringLiteral("Gitee源出现问题！"));
            mgr->deleteLater();
            return;
        }
        const char * version = json.find("Latest Version")->getValueString();
        if (!strcmp(VarBox->m_dVersion, version))
        {
            jobTip->showTip("当前已经是最新版本！", 1000);
        }
        else
        {
            if (GlobalFn::getVersion(VarBox->m_dVersion) < GlobalFn::getVersion(version))
            {
                jobTip->showTip(QString("\t有新版本已经出现！\n当前版本：%1%2%3").arg(VarBox->m_dVersion, "; 最新版本：", version), 2000);
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
    if (Wallpaper::isOnline()) {
        QMessageBox::information(this, "提示", "点击确认开始下载,下载时间一般不会很长,请耐心等待,\n不要关闭设置窗口.");
    } else {
        QMessageBox::information(this, "提示", "没有网络！");
        return;
    }
    QNetworkAccessManager* mgr0 = new QNetworkAccessManager;
    connect(mgr0, &QNetworkAccessManager::finished, this, [=](QNetworkReply* rep){
        const QByteArray& data { rep->readAll() };
        rep->deleteLater();
        mgr0->deleteLater();
        try {
            YJson json(data);
            if (json.getType() != YJson::Object)
            {
                jobTip->showTip(QStringLiteral("Gitee源出现问题！"));
                return;
            }
            YJson *urls = json.find("Files"); YJson *child = nullptr;
            if (urls && urls->getType() == YJson::Object)
            {
                if (!(child = urls->find("zip")))
                {
                    jobTip->showTip(QStringLiteral("Json 文件不含zip, 下载失败！"));
                    return;
                }
                const QUrl url1(child->getValueString());
                if (!(child = urls->find("update")))
                {
                    jobTip->showTip(QStringLiteral("Json 文件不含zip, 下载失败!"));
                    return;
                }
                if (!json["File Name"])
                {
                    jobTip->showTip("Json 文件不含File Name, 下载失败！");
                    return;
                }
                const QUrl url2(child->getValueString());
                const QString zipfile = json["File Name"].getValueString();

                DownloadProgress pro(zipfile, url1, url2,this);
                pro.exec();
                qout << "下载第一个文件";
            } else {
                jobTip->showTip("未知原因，下载失败！");
            }
        } catch (std::string& errorStr) {
            qout << errorStr.c_str();
            GlobalFn::msgBox("Json文件出错, 下载失败!");
        }

    });
    mgr0->get(QNetworkRequest(QUrl("https://gitee.com/yjmthu/Speed-Box/raw/main/update/update.json")));
}

#endif

void Dialog::on_toolButton_2_clicked()
{
    QString& img_folder = (ui->rBtnBingApi->isChecked())? wallpaper->m_bing_folder: (ui->rBtnOtherApi->isChecked()? wallpaper->m_other_folder: wallpaper->m_wallhaven_folder);
    QString titile = "请选择一个文件夹";
    QString dir = QFileDialog::getExistingDirectory(NULL, titile, img_folder.length()? img_folder: qApp->applicationDirPath(), QFileDialog::ShowDirsOnly);
    if (!dir.isEmpty()) {
        img_folder = QDir::toNativeSeparators(dir);
        ui->linePictuerPath->setText(img_folder);
        std::string ph = "WallpaperApi.json";
        YJson json(ph, YJson::AUTO);
        if (ui->rBtnWallhavenApiDefault->isChecked()) {
            json["Default"]["ApiData"][ui->cBxApis->currentIndex()]["Folder"].setText(img_folder.toStdString());
        } else if (ui->rBtnWallhavenApiUser->isChecked()) {
            json["User"]["ApiData"][(const char*)ui->cBxApis->currentText().toUtf8()]["Folder"].setText(img_folder.toUtf8());
        } else if (ui->rBtnBingApi->isChecked()) {
            json["BingApi"]["Folder"].setText(img_folder.toStdString());
        } else {
            json["OtherApi"]["ApiData"][(const char*)ui->cBxApis->currentText().toUtf8()]["Folder"].setText(img_folder.toUtf8());
        }
        json.toFile(ph, YJson::UTF8BOM, true);
        jobTip->showTip("修改成功！");
    } else {
        jobTip->showTip("更改失败！");
    }
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

void Dialog::my_on_border_radius(bool checked)
{
    const auto x = m_sheet[ui->cBxFormPart->currentIndex()].bd_have;
    if (checked) {
        ui->cBxEnableBorderRadius->setChecked(x & QStyleSheet::Border);
        ui->cBxLeftBorderRadius->setChecked(x & QStyleSheet::Left);
        ui->cBxRightBorderRadius->setChecked(x & QStyleSheet::Right);
        ui->cBxTopBorderRadius->setChecked(x & QStyleSheet::Top);
        ui->cBxBottomBorderRadius->setChecked(x & QStyleSheet::Bottom);
        ui->cBxLeftBorderRadius->setText(QStringLiteral("左"));
        ui->cBxRightBorderRadius->setText(QStringLiteral("右"));
        ui->cBxTopBorderRadius->setText(QStringLiteral("上"));
        ui->cBxBottomBorderRadius->setText(QStringLiteral("下"));
    } else {
        ui->cBxEnableBorderRadius->setChecked(x & QStyleSheet::BorderRadius);
        ui->cBxLeftBorderRadius->setChecked(x & QStyleSheet::TopLeft);
        ui->cBxRightBorderRadius->setChecked(x & QStyleSheet::BottomLeft);
        ui->cBxTopBorderRadius->setChecked(x & QStyleSheet::TopRight);
        ui->cBxBottomBorderRadius->setChecked(x & QStyleSheet::BottomRight);
        ui->cBxLeftBorderRadius->setText(QStringLiteral("左上"));
        ui->cBxRightBorderRadius->setText(QStringLiteral("左下"));
        ui->cBxTopBorderRadius->setText(QStringLiteral("右上"));
        ui->cBxBottomBorderRadius->setText(QStringLiteral("右下"));
    }
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
    ui->cBxApis->addItem(QStringLiteral("Bing"));
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
    ui->linePictuerPath->setText(temp["ApiData"][ui->cBxApis->currentText().toStdString()]["Folder"].getValueString());
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
        if (wallpaper->m_paperType == Type::User)
        {
            wallpaper->m_wallhaven_folder = json["User"]["ApiData"][(const char*)arg1.toUtf8()]["Folder"].getValueString();
            ui->linePictuerPath->setText(wallpaper->m_wallhaven_folder);
        }
        else
        {
            ui->linePictuerPath->setText(json["User"]["ApiData"][(const char*)arg1.toUtf8()]["Folder"].getValueString());
        }
        json.toFile(ph, YJson::UTF8BOM, true);
    }
    else if (ui->rBtnBingApi->isChecked())
    {
        if (wallpaper->m_paperType == Type::Bing)
        {
            wallpaper->m_bing_folder = json["BingApi"]["Folder"].getValueString();
            ui->linePictuerPath->setText(wallpaper->m_bing_folder);
        }
        else
        {
            ui->linePictuerPath->setText(json["BingApi"]["Folder"].getValueString());
        }
    }
    else
    {
        if (wallpaper->m_paperType == Type::Other)
        {
            json["OtherApi"]["Curruent"].setText(arg1.toUtf8());
            wallpaper->m_other_folder = json["OtherApi"]["ApiData"][(const char*)arg1.toUtf8()]["Folder"].getValueString();
            wallpaper->m_other_api = json["OtherApi"]["ApiData"][(const char*)arg1.toUtf8()]["Url"].getValueString();
            ui->linePictuerPath->setText(wallpaper->m_other_folder);
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


