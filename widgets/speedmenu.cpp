#include "speedmenu.h"
#include "speedapp.h"

#include "wallpaper/wallpaper.h"
#include "widgets/speedbox.h"

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

#include <thread>
#include <unistd.h>

void SpeedMenu::showEvent(QShowEvent *event)
{
    m_AutoStartApp->setChecked(
        QFile::exists(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.config/autostart/Neobox.desktop"));
}

SpeedMenu::SpeedMenu(QWidget *parent)
    : QMenu(parent)
    , m_Actions(new QAction[11])
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
        u8"软件设置", u8"划词翻译", u8"上一张图", u8"下一张图", u8"不看此图",
        u8"打开目录", u8"快速关机", u8"快捷重启", u8"本次退出", u8"防止息屏"
    };
    for (int i = 0; i < 10; ++i) {
        m_Actions[i].setText(lst[i]);
        addAction(m_Actions+i);
    }
    m_Actions[1].setCheckable(true);
    m_Actions[9].setCheckable(true);
    SetupSettingMenu();
}

void SpeedMenu::SetupConntects()
{
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
    auto m_Setting = m_VarBox->m_Setting->find("Wallpaper");
    ptr4->setChecked(m_Setting->find("FirstChange")->isTrue());
    ptr5->setChecked(m_Setting->find("AutoChange")->isTrue());
    connect(ptr4, &QAction::triggered, 
        m_VarBox->m_Wallpaper, &Wallpaper::SetFirstChange);
    connect(ptr5, &QAction::triggered, 
        m_VarBox->m_Wallpaper, &Wallpaper::SetAutoChange);
    connect(ptr6, &QAction::triggered, this, [](){
        auto i = QFileDialog::getExistingDirectory(nullptr, u8"选择文件夹",
            QString::fromStdString(m_VarBox->m_Wallpaper->GetImageDir()), QFileDialog::ShowDirsOnly);
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
        QString fileName = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.config/autostart/Neobox.desktop";
        if (checked) {
            QFile file(fileName);
            file.open(QIODevice::WriteOnly);
            QTextStream stream(&file);
            stream << "[Desktop Entry]" << Qt::endl
                   << "Name=Neobox" << Qt::endl
                   << "Icon=" << "/home/yjmthu/Documents/GitHub/Neobox/icons/speedbox.ico" << Qt::endl
                   << "Exec=\""<< qApp->applicationFilePath() << "\" -b" << Qt::endl
                   << "Terminal=false" << Qt::endl
                   << "Type=Application" << Qt::endl;
            file.close();
        } else {
            QFile::remove(fileName);
        }
    });
    auto ptr = m_pAppSettingMenu->addAction("背景颜色");
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
    static size_t type = m_VarBox->m_Setting->find("Wallpaper")->find("ImageType")->getValueInt();
    QMenu* mu = new QMenu(parent);

    auto acGroup = new QActionGroup(this);
    acGroup->setExclusive(true);
    for (auto i: { "壁纸天堂", "必应壁纸", "直链网址", "本地壁纸", "脚本输出" }) {
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
        const char* curType = m_Setting->find("WallhavenCurrent")->getValueString();
        m_tempMenu->addAction("壁纸类型");
        m_tempMenu->addSeparator();
        for (YJson** i = &m_Setting->find("WallhavenApi")->getChild(); *i; i = &(*i)->getNext()) {
            const std::string temp = (*i)->getKeyString();
            auto ac = m_tempMenu->addAction((*i)->getKeyString());
            ac->setCheckable(true);
            ac->setChecked(!strcmp((*i)->getKeyString(), curType));
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
                    m_Setting->find("WallhavenCurrent")->setText(temp);
                    m_Setting->find("PageNumber")->setValue(val);
                    m_VarBox->m_Wallpaper->SetSlot(2);
                });
            connect(mn->addAction("参数设置"), &QAction::triggered, 
                this, [i, this, ac, m_Setting](){
                    QDialog dlg;
                    QVBoxLayout *vlayout = new QVBoxLayout(&dlg);
                    QHBoxLayout hlayout1, hlayout2;
                    QLineEdit *lineEdit = new QLineEdit(&dlg);
                    auto label = new QLabel(&dlg);
                    label->setText(QStringLiteral("类型名称"));
                    lineEdit->setText((*i)->getKeyString());
                    hlayout1.addWidget(label);
                    hlayout1.addWidget(lineEdit);
                    dlg.setLayout(vlayout);
                    int row = (*i)->find("Parameter")->size(), index = 0;
                    // std::cout << "row num is " << row << std::endl;
                    QTableWidget m_TableWidget(row,  2);
                    vlayout->addLayout(&hlayout1);
                    vlayout->addWidget(&m_TableWidget);
                    vlayout->addLayout(&hlayout2);
                    dlg.setWindowTitle(QStringLiteral("参数设置"));
                    m_TableWidget.setHorizontalHeaderLabels(QStringList{ "key", "value"});
                    // m_TableWidget.setEditTriggers(QAbstractItemView::NoEditTriggers);
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
                            std::string key = m_TableWidget.item(i, 0)->text().toStdString();
                            std::string val = m_TableWidget.item(i, 1)->text().toStdString();
                            if (key.empty() || val.empty()) {
                                continue;
                            } else {
                                js.append(val, key.c_str());
                            }
                        }
                        if (!js.empty()) {
                            auto newKey = lineEdit->text().toStdString();
                            if (newKey.empty()) newKey = u8"新类型";
                            YJson::swap(*(*i)->find("Parameter"), js);
                            if (newKey != (*i)->getKeyString()) {
                                (*i)->setKeyString(newKey);
                                m_Setting->find("WallhavenCurrent")->setText(newKey);
                            }
                            if (m_VarBox->m_Wallpaper->IsWorking()) return;
                            m_VarBox->m_Wallpaper->SetSlot(ac->isChecked()? 3:2);
                            SetupSettingMenu();
                        }
                        dlg.close();
                    });
                    for (YJson *j = (*i)->find("Parameter")->getChild(); j; j = j->getNext()) {
                        m_TableWidget.setItem(index, 0, new QTableWidgetItem(j->getKeyString()));
                        if (j->isNumber()) {
                            m_TableWidget.setItem(index++, 1, new QTableWidgetItem(QString::number(j->getValueInt())));
                        } else {
                            m_TableWidget.setItem(index++, 1, new QTableWidgetItem(j->getValueString()));
                        }
                    }
                    dlg.exec();
                });
            if (!ac->isChecked()) {
                connect(mn->addAction("删除此项"), &QAction::triggered, 
                    this, [m_Setting, i, this](){
                        m_Setting->find("WallhavenApi")->remove(*i);
                        m_VarBox->m_Wallpaper->SetSlot(2);
                        SetupSettingMenu();
                    });
            }
        }
        m_tempMenu->addSeparator();
        connect(m_tempMenu->addAction("添加更多"),
            &QAction::triggered, this, [this, m_Setting](){
            std::string m_TypeName = QInputDialog::getText(nullptr, QStringLiteral("获取名称"), QStringLiteral("请输入名称：")).toStdString();
            if (m_TypeName.empty()) return ;
            auto m_TypeDir = QFileDialog::getExistingDirectory(nullptr, u8"选择文件夹",
                QString::fromStdString(m_VarBox->m_Wallpaper->GetImageDir()), QFileDialog::ShowDirsOnly).toStdString();
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
                js.append(m_TypeDir, "Directory");
                auto ptr = js.append(YJson::Object, "Parameter");
                for (int i=0; i<row; ++i) {
                    std::string key = m_TableWidget.item(i, 0)->text().toStdString();
                    std::string val = m_TableWidget.item(i, 1)->text().toStdString();
                    if (key.empty() || val.empty()) {
                        continue;
                    } else {
                        ptr->append(val, key.c_str());
                    }
                }
                if (!ptr->empty()) {
                    YJson::swap(*m_Setting->find("WallhavenApi")->append(YJson::Object, m_TypeName.c_str()), js);
                    m_VarBox->m_Wallpaper->SetSlot(2);
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
        // m_tempMenu->addSeparator();
        break;
    }
    case 1:
    case 2:
    case 3:
    case 4:
    default:
        break;
    }
}