#include "speedmenu.h"
#include "speedapp.h"

#include "core/appcode.hpp"
#include "core/sysapi.h"
#include "wallpaper/wallpaper.h"
#include "translate/text.h"
#include "speedbox.h"
#include "screenfetch.h"

#include "3rdlib/3rd_qxtglobalshortcut/qxtglobalshortcut.h"

#include <yjson.h>

#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QFileDialog>
#include <QActionGroup>
#include <QTimer>
#include <QMessageBox>
#include <QInputDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QLabel>
#include <QColorDialog>
#include <QSettings>
#include <QShowEvent>

#include <thread>
#include <unistd.h>

void SpeedMenu::showEvent(QShowEvent *event)
{
    QString m_AutoStartFile = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.config/autostart/Neobox.desktop";
    if (!QFile::exists(m_AutoStartFile)) {
        m_AutoStartApp->setChecked(false);
    } else {
        QSettings m_Setting(m_AutoStartFile, QSettings::IniFormat);
        m_Setting.beginGroup("Desktop Entry");
        QString str = m_Setting.value("Exec", "null").toString();
        m_AutoStartApp->setChecked(str.indexOf(qApp->applicationFilePath()) != -1);
        m_Setting.endGroup();
    }
    if (event)
        event->accept();
}

SpeedMenu::SpeedMenu(QWidget *parent)
    : QMenu(parent)
    , m_Actions(new QAction[9])
{
    SetupUi();
    SetupConntects();
}

SpeedMenu::~SpeedMenu()
{
    delete [] m_Actions;
}

void SpeedMenu::SetupUi()
{
    constexpr char lst[11][13] = {
        "软件设置", "文字识别", "上一张图", "下一张图", "不看此图",
        "打开目录", "快速关机", "快捷重启", "本次退出"
    };
    for (int i = 0; i < 9; ++i) {
        m_Actions[i].setText(lst[i]);
        addAction(m_Actions+i);
    }
    SetupSettingMenu();
}

void SpeedMenu::ScreenShot()
{
    ScreenFetch getscreen;
    getscreen.exec();
    void* image = getscreen.m_Picture;
    if (image)
        (new TextDlg(image))->show();
}

void SpeedMenu::SetupConntects()
{
    connect(m_Actions+1, &QAction::triggered, this, &SpeedMenu::ScreenShot);

    QxtGlobalShortcut* shotCut = new QxtGlobalShortcut(this);
    if (shotCut->setShortcut(QKeySequence("Shift+A"))) {
        connect(shotCut, &QxtGlobalShortcut::activated, this, &SpeedMenu::ScreenShot);
    } else {
        std::cout << "Can't regist Shift+A\n";
    }

    connect(m_Actions+2, &QAction::triggered, qApp,
            std::bind(&Wallpaper::SetSlot, m_VarBox->m_Wallpaper, -1));
    connect(m_Actions+3, &QAction::triggered, qApp,
            std::bind(&Wallpaper::SetSlot, m_VarBox->m_Wallpaper, 1));
    connect(m_Actions+4, &QAction::triggered, qApp,
            std::bind(&Wallpaper::SetSlot, m_VarBox->m_Wallpaper, 0));
    connect(m_Actions+5, &QAction::triggered, qApp,
            std::bind(&QDesktopServices::openUrl, QUrl::fromLocalFile(qApp->applicationDirPath())));
    connect(m_Actions+6, &QAction::triggered, this, [](){
            QTimer::singleShot(500, std::bind(::system, "shutdown -h now"));
        });
    connect(m_Actions+7, &QAction::triggered, this, [](){
            QTimer::singleShot(500, std::bind(::system, "shutdown -r now"));
        });
    connect(m_Actions+8, &QAction::triggered, qApp, &QApplication::quit);
}

void SpeedMenu::SetupSettingMenu()
{
    using namespace std::literals;
    QMenu *m_pSettingMenu = new QMenu(this);
    auto ptr0 = m_pSettingMenu->addAction("软件设置");
    auto ptr1 = m_pSettingMenu->addAction("壁纸设置");
    auto ptr2 = m_pSettingMenu->addAction("工具设置");
    m_pSettingMenu->addSeparator();
    connect(m_pSettingMenu->addAction("打开配置"), &QAction::triggered,
        qApp, std::bind(&QDesktopServices::openUrl, QUrl::fromLocalFile(QDir::currentPath())));
    QMenu *m_pWallpaperMenu = new QMenu(m_pSettingMenu);
    QMenu *m_pAppSettingMenu = new QMenu(m_pSettingMenu);
    auto ptr3 = m_pWallpaperMenu->addAction("类型选择");
    auto ptr4 = m_pWallpaperMenu->addAction("首次更换");
    auto ptr5 = m_pWallpaperMenu->addAction("自动更换");
    auto ptr6 = m_pWallpaperMenu->addAction("存储路径");
    auto ptr7 = m_pWallpaperMenu->addAction("时间间隔");
    m_AdditionalAction = m_pWallpaperMenu->addAction("附加设置");
    ptr0->setMenu(m_pAppSettingMenu);
    ptr1->setMenu(m_pWallpaperMenu);
    m_Actions->setMenu(m_pSettingMenu);
    ptr4->setCheckable(true);
    ptr5->setCheckable(true);
    auto& m_Setting = m_VarBox->m_Setting->find(u8"Wallpaper")->second;
    ptr4->setChecked(m_Setting.find(u8"FirstChange")->second.isTrue());
    ptr5->setChecked(m_Setting.find(u8"AutoChange")->second.isTrue());
    connect(ptr4, &QAction::triggered, 
        m_VarBox->m_Wallpaper, &Wallpaper::SetFirstChange);
    connect(ptr5, &QAction::triggered, 
        m_VarBox->m_Wallpaper, &Wallpaper::SetAutoChange);
    connect(ptr6, &QAction::triggered, this, [](){
        auto i = QFileDialog::getExistingDirectory(nullptr, "选择文件夹",
            QString::fromStdU16String(m_VarBox->m_Wallpaper->GetImageDir().u16string()), QFileDialog::ShowDirsOnly);
        if (!i.isEmpty()) m_VarBox->m_Wallpaper->SetCurDir(i.toStdString());
    });
    connect(ptr7, &QAction::triggered, this, [](){
        int val = QInputDialog::getInt(nullptr, 
            "输入时间", "时间间隔（分钟）：",
            m_VarBox->m_Wallpaper->GetTimeInterval(),
            5, std::numeric_limits<int>::max());
        m_VarBox->m_Wallpaper->SetTimeInterval(val);
    });
    SetupImageType(m_pWallpaperMenu, ptr3);
    SetAdditionalMenu();
    
    m_AutoStartApp = m_pAppSettingMenu->addAction("开机自启");
    m_AutoStartApp->setCheckable(true);
    connect(m_AutoStartApp, &QAction::triggered, this, [](bool checked) {
        QString configLocation = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config";
        QString desktopFileName = configLocation + "/autostart/Neobox.desktop";
        QString iconFileName = configLocation + "/Neobox/Neobox.ico";
        if (checked) {
            if (!QFile::exists(iconFileName)) {
                QFile::copy(":/icons/speedbox.ico", iconFileName);
                QFile::setPermissions(iconFileName, QFileDevice::ReadUser);
            }
            std::ofstream file(desktopFileName.toLocal8Bit(), std::ios::binary | std::ios::out);
            file << "[Desktop Entry]"sv << std::endl
                 << "Name=Neobox" << std::endl
                 << "Comment=A powerful wallpaper tool." << std::endl
                 << "Icon=" << iconFileName.toStdString() << std::endl
                 << "Exec=sh -c \"(sleep 3 && exec \'" << qApp->applicationFilePath().toStdString() << "\' -b)\"" << std::endl
                 << "Categories=System" << std::endl
                 << "Terminal=false" << std::endl
                 << "Type=Application" << std::endl;
            file.close();
        } else {
            QFile::remove(desktopFileName);
        }
    });
    auto ptr = m_pAppSettingMenu->addAction("显示界面");
    ptr->setCheckable(true);
    ptr->setChecked(m_VarBox->m_Setting->find(u8"FormGlobal")->second[u8"ShowForm"].second.isTrue());
    connect(ptr, &QAction::triggered, [](bool checked){
        m_VarBox->m_Setting->find(u8"FormGlobal")->second[u8"ShowForm"].second = checked;
        if (checked) {
            m_VarBox->m_SpeedBox->show();
        } else {
            m_VarBox->m_SpeedBox->hide();
        }
        m_VarBox->SaveSetting();
    });
    connect(m_pAppSettingMenu->addAction("重启软件"), &QAction::triggered, this, [](){
        qApp->exit(static_cast<int>(ExitCode::RETCODE_RESTART));
    });
    ptr = m_pAppSettingMenu->addAction("背景颜色");
    connect(ptr, &QAction::triggered, this, [this](){
        QColor col = QColorDialog::getColor(m_VarBox->m_SpeedBox->m_BackCol, nullptr, QStringLiteral("选择颜色"));
        if (col.isValid()) {
            emit ChangeBoxColor(col);
        }
    });
    ptr = m_pAppSettingMenu->addAction("背景透明");
    connect(ptr, &QAction::triggered, this, [this](){
        int val = QInputDialog::getInt(nullptr, QStringLiteral("请输入不透明度"),
            QStringLiteral("整数(1~255)"), m_VarBox->m_SpeedBox->m_BackCol.alpha(),
            0, 255);
        emit ChangeBoxAlpha(val);
    });
}

void SpeedMenu::SetupImageType(QMenu* parent, QAction* ac)
{
    size_t index = 0;
    static size_t type = m_VarBox->m_Setting->find(u8"Wallpaper")->second.find(u8"ImageType")->second.getValueInt();
    QMenu* mu = new QMenu(parent);

    auto acGroup = new QActionGroup(this);
    acGroup->setExclusive(true);
    for (const auto& i: { "壁纸天堂", "必应壁纸", "直链网址", "本地壁纸", "脚本输出" }) {
        auto ptr = mu->addAction(i);
        ptr->setCheckable(true);
        acGroup->addAction(ptr);
        ptr->setChecked(index == type);
        connect(ptr, &QAction::triggered,
            this, [index, this]() {
                if (m_VarBox->m_Wallpaper->IsWorking() && QMessageBox::question(
                        nullptr, QStringLiteral("提示"), 
                        QStringLiteral("当前正忙，是否强制继续？"))
                         != QMessageBox::Yes)
                {
                    return ;
                }
                m_VarBox->m_Wallpaper->SetImageType(index);
                SetAdditionalMenu();
            });
        ++index;
    }
    ac->setMenu(mu);
}

void SpeedMenu::SetAdditionalMenu()
{
    static QMenu* m_Menus = nullptr;
    QMenu* m_tempMenu = new QMenu(this);
    m_AdditionalAction->setMenu(m_tempMenu);
    if (m_Menus) {
        delete m_Menus;
        m_Menus = m_tempMenu;
    }
    switch (m_VarBox->m_Wallpaper->GetImageType()) {
    case 0:
    {
        QActionGroup* group = new QActionGroup(m_tempMenu);
        YJson *m_Setting = *reinterpret_cast<YJson*const*>(m_VarBox->m_Wallpaper->GetDataByName("m_Setting"));
        const auto& curType = m_Setting->find(u8"WallhavenCurrent")->second.getValueString();
        m_tempMenu->addAction("壁纸类型");
        m_tempMenu->addSeparator();
        auto& _jjk = m_Setting->find(u8"WallhavenApi")->second.getObject();
        for (auto i = _jjk.begin(); i != _jjk.end(); ++i) {
            const std::u8string_view temp = i->first;
            auto ac = m_tempMenu->addAction(
                QString::fromUtf8(reinterpret_cast<const char*>(i->first.data()), i->first.size()));
            ac->setCheckable(true);
            ac->setChecked(i->first == curType);
            group->addAction(ac);
            QMenu* mn = new QMenu(m_tempMenu);
            ac->setMenu(mn);
            connect(mn->addAction("启用此项"), &QAction::triggered, this,
                [temp, m_Setting, ac](){
                    if (ac->isChecked()) {
                        QMessageBox::critical(nullptr, QStringLiteral("警告"), QStringLiteral("当前正在使用该类型！"));
                        return ;
                    }
                    ac->setChecked(true);
                    int val = QInputDialog::getInt(nullptr, 
                        QStringLiteral("输入页面"), QStringLiteral("页面位置（1~100）："),
                        m_VarBox->m_Wallpaper->GetInt(),
                        1, 100);
                    m_Setting->find(u8"WallhavenCurrent")->second.setText(temp);
                    m_Setting->find(u8"PageNumber")->second.setValue(val);
                    m_VarBox->m_Wallpaper->SetSlot(3);
                });
            if (i->second.find(u8"Parameter")->second.isObject()) {
            connect(mn->addAction("参数设置"), &QAction::triggered, 
                this, [i, this, ac, m_Setting](){
                    QDialog dlg;
                    QVBoxLayout *vlayout = new QVBoxLayout(&dlg);
                    QHBoxLayout hlayout1, hlayout2;
                    QLineEdit *lineEdit = new QLineEdit(&dlg);
                    auto label = new QLabel(&dlg);
                    label->setText(QStringLiteral("类型名称"));
                    lineEdit->setText(QString::fromUtf8(reinterpret_cast<const char *>(i->first.data()), i->first.size()));
                    hlayout1.addWidget(label);
                    hlayout1.addWidget(lineEdit);
                    dlg.setLayout(vlayout);
                    int row = i->second.find(u8"Parameter")->second.sizeO(), index = 0;
                    QTableWidget m_TableWidget(row,  2);
                    vlayout->addLayout(&hlayout1);
                    vlayout->addWidget(&m_TableWidget);
                    vlayout->addLayout(&hlayout2);
                    dlg.setWindowTitle(QStringLiteral("参数设置"));
                    m_TableWidget.setHorizontalHeaderLabels(QStringList{ "key", "value"});
                    QPushButton* bt1 = new QPushButton(&dlg);
                    bt1->setText(QStringLiteral("添加"));
                    QPushButton* bt2 = new QPushButton(&dlg);
                    bt2->setText(QStringLiteral("删除"));
                    QPushButton* bt3 = new QPushButton(&dlg);
                    bt3->setText(QStringLiteral("确认"));
                    hlayout2.addWidget(bt1);
                    hlayout2.addWidget(bt2);
                    hlayout2.addWidget(bt3);
                    connect(bt1, &QPushButton::clicked, this, [&m_TableWidget](){
                        m_TableWidget.setRowCount(m_TableWidget.rowCount() + 1);
                    });
                    connect(bt2, &QPushButton::clicked, this, [&m_TableWidget](){
                        m_TableWidget.removeRow(m_TableWidget.currentRow());
                    });
                    connect(bt3, &QPushButton::clicked, this, [&m_TableWidget, this, &dlg, &i, ac, lineEdit, m_Setting](){
                        int row = m_TableWidget.rowCount();
                        YJson js(YJson::Object);
                        for (int i=0; i<row; ++i) {
                            std::u8string&& key = GetU8String(m_TableWidget.item(i, 0)->text().toUtf8());
                            std::u8string&& val = GetU8String(m_TableWidget.item(i, 1)->text().toUtf8());
                            if (key.empty() || val.empty()) {
                                continue;
                            } else {
                                js.append(val, key);
                            }
                        }
                        if (!js.emptyO()) {
                            auto array = lineEdit->text().toUtf8();
                            std::u8string newKey(array.begin(), array.end());
                            if (newKey.empty()) newKey = u8"新类型";
                            YJson::swap(i->second.find(u8"Parameter")->second, js);
                            if (newKey != i->first) {
                                i->first = newKey;
                                m_Setting->find(u8"WallhavenCurrent")->second.setText(newKey);
                            }
                            if (m_VarBox->m_Wallpaper->IsWorking()) return;
                            m_VarBox->m_Wallpaper->SetSlot(ac->isChecked()? 3:2);
                            SetupSettingMenu();
                        }
                        dlg.close();
                    });
                    for (auto& j: i->second.find(u8"Parameter")->second.getObject()) {
                        m_TableWidget.setItem(index, 0,  new QTableWidgetItem(
                            QString::fromUtf8(reinterpret_cast<const char*>(j.first.data(), j.first.size()))));
                        if (j.second.isNumber()) {
                            m_TableWidget.setItem(index++, 1, new QTableWidgetItem(QString::number(j.second.getValueInt())));
                        } else {
                            std::u8string_view str = j.second.getValueString();
                            m_TableWidget.setItem(index++, 1, new QTableWidgetItem(
                                QString::fromUtf8(reinterpret_cast<const char*>(str.data()), str.size())));
                        }
                    }
                    dlg.exec();
            });}
            if (!ac->isChecked()) {
                connect(mn->addAction("删除此项"), &QAction::triggered, 
                    this, [m_Setting, i, this](){
                        m_Setting->find(u8"WallhavenApi")->second.remove(i);
                        m_VarBox->m_Wallpaper->SetSlot(2);
                        SetupSettingMenu();
                    });
            }
        }
        connect(m_tempMenu->addAction("添加更多"),
            &QAction::triggered, this, [this, m_Setting](){
            QString m_TypeName = QInputDialog::getText(nullptr, QStringLiteral("获取名称"), QStringLiteral("请输入名称："));
            if (m_TypeName.isEmpty()) return ;
            auto m_TypeDir = QFileDialog::getExistingDirectory(nullptr, "选择文件夹",
                QString::fromStdU16String(m_VarBox->m_Wallpaper->GetImageDir().u16string()), QFileDialog::ShowDirsOnly).toStdString();
            if (m_TypeDir.empty()) return;
            QDialog dlg(nullptr);
            QVBoxLayout *vlayout = new QVBoxLayout(&dlg);
            QHBoxLayout hlayout;
            dlg.setLayout(vlayout);
            QTableWidget m_TableWidget(0,  2);
            vlayout->addWidget(&m_TableWidget);
            dlg.setWindowTitle(QStringLiteral("参数设置"));
            m_TableWidget.setHorizontalHeaderLabels(QStringList{ "key", "value"});
            // m_TableWidget.setSelectionBehavior ( QAbstractItemView::SelectRows);
            QPushButton* bt1 = new QPushButton(&dlg);
            bt1->setText(QStringLiteral("添加"));
            QPushButton* bt2 = new QPushButton(&dlg);
            bt2->setText(QStringLiteral("删除"));
            QPushButton* bt3 = new QPushButton(&dlg);
            bt3->setText(QStringLiteral("确认"));
            connect(bt1, &QPushButton::clicked, this, [&m_TableWidget](){
                m_TableWidget.setRowCount(m_TableWidget.rowCount() + 1);
            });
            connect(bt2, &QPushButton::clicked, this, [&m_TableWidget](){
                m_TableWidget.removeRow(m_TableWidget.currentRow());
            });
            connect(bt3, &QPushButton::clicked, this, [&m_TableWidget, m_Setting, this, &m_TypeDir, &m_TypeName, &dlg](){
                int row = m_TableWidget.rowCount();
                YJson js(YJson::Object);
                js.append(m_TypeDir, u8"Directory");
                auto& ptr = js.append(YJson::Object, u8"Parameter")->second;
                for (int i=0; i<row; ++i) {
                    std::u8string&& key = GetU8String(m_TableWidget.item(i, 0)->text().toUtf8());
                    std::u8string&& val = GetU8String(m_TableWidget.item(i, 1)->text().toUtf8());
                    if (key.empty() || val.empty()) {
                        continue;
                    } else {
                        ptr.append(val, key);
                    }
                }
                if (!ptr.emptyO()) {
                    YJson::swap(m_Setting->find(u8"WallhavenApi")->second.append(YJson::Object, GetU8String(m_TypeName.toUtf8()))->second, js);
                    m_VarBox->m_Wallpaper->SetSlot(3);
                    SetupSettingMenu();
                }
                dlg.close();
            });
            hlayout.addWidget(bt1);
            hlayout.addWidget(bt2);
            hlayout.addWidget(bt3);
            vlayout->addLayout(&hlayout);
            dlg.exec();
        });
        m_tempMenu->addSeparator();
        connect(m_tempMenu->addAction("当前壁纸"), &QAction::triggered, this, [](){
            const std::u8string& curImage = m_VarBox->m_Wallpaper->GetCurIamge().u8string();
            auto beg = curImage.find_last_of('/') + 1;
            std::u8string imgData = curImage.substr(beg, curImage.find_last_of(u8'.') - beg);
            auto index = imgData.find(u8"wallhaven-");
            if (imgData.size() != 16 || index == std::u8string::npos) {
                goto label;
            } else {
                imgData = imgData.substr(index + 10);
                if (imgData.length() != 6 || std::find_if(imgData.begin(), imgData.end(), [](char c)->bool{ return !std::isdigit(c) && !std::islower(c); }) != imgData.end()) {
                    goto label;
                } else {
                    std::u8string url(u8"https://wallhaven.cc/w/");
                    url += imgData;
                    QDesktopServices::openUrl(QString::fromUtf8(reinterpret_cast<const char*>(url.data()), url.size()));
                    return;
                }
            }
label:
            QMessageBox::critical(nullptr, "错误", "当前壁纸并不是Wallhaven壁纸！");
        });
        break;
    }
    case 1:
        //
        //break;
    case 2:
    case 3:
    case 4:
    default:
        break;
    }
}
