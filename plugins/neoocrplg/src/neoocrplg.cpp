#include <neoocrplg.h>
#include <neoocr.h>
#include <pluginmgr.h>
#include <neomenu.hpp>
#include <screenfetch.h>
#include <yjson.h>
#include <menubase.hpp>

#include <QDir>
#include <QLabel>
#include <QAction>
#include <QEventLoop>
#include <QFileDialog>
#include <QPlainTextEdit>
#include <QApplication>
#include <QClipboard>
#include <QImage>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QPushButton>
#include <QCheckBox>
#include <QDesktopServices>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QActionGroup>
#include <QRadioButton>
#include <QButtonGroup>

#include <filesystem>
#include <ranges>

#define CLASS_NAME NeoOcrPlg
#include <pluginexport.cpp>

namespace fs = std::filesystem;
using namespace std::literals;
/*
 * NeoOcrPlg Class
 *
 */

NeoOcrPlg::NeoOcrPlg(YJson& settings):
  PluginObject(InitSettings(settings), u8"neoocrplg", u8"文字识别"),
  m_Ocr(new NeoOcr(settings, std::bind(&PluginMgr::SaveSettings, mgr)))
{
  QDir dir;
  auto lst = {"tessdata"};
  for (auto i : lst) {
    if (!dir.exists(i))
      dir.mkdir(i);
  }
  InitFunctionMap();
}

NeoOcrPlg::~NeoOcrPlg()
{
  delete m_MainMenuAction;
  delete m_Ocr;
}

void NeoOcrPlg::InitFunctionMap() {
  m_PluginMethod = {
    {u8"screenfetch",
      {u8"截取屏幕", u8"截取屏幕区域，识别其中文字。", [this](PluginEvent, void*) {
        static bool busy = false;
        if (busy) return; else busy = true;
        QImage image;
        ScreenFetch* box = new ScreenFetch(image);
        QEventLoop loop;
        QObject::connect(box, &ScreenFetch::destroyed, &loop, &QEventLoop::quit);
        box->showFullScreen();
        loop.exec();
        busy = false;
        if (!box->HaveCatchImage())
          return;
        std::u8string str = m_Ocr->GetText(std::move(image));
        if (m_Settings[u8"WriteClipboard"].isTrue()) {
          QApplication::clipboard()->setText(QString::fromUtf8(str.data(), str.size()));
          mgr->ShowMsg("复制数据成功");
        }
        if (m_Settings[u8"ShowWindow"].isTrue()) {
          SendBroadcast(PluginEvent::U8string, &str);
        }
      }, PluginEvent::Void},
    },
    {u8"setDataDir",
      {u8"设置路径", u8"设置训练数据（语言包）的存储位置", [this](PluginEvent, void*) {
        std::u8string& u8Path =
            m_Settings[u8"TessdataDir"].getValueString();
        auto u8PathNew = mgr->m_Menu->GetExistingDirectory("请选择Tessdata数据文件存放位置", u8Path);
        if (!u8PathNew) {
          mgr->ShowMsg("取消设置成功！");
          return;
        } else {
          m_Ocr->SetDataDir(*u8PathNew);
          u8Path.swap(*u8PathNew);
          mgr->SaveSettings();
          mgr->ShowMsg("设置数据文件失成功！");
        }
      }, PluginEvent::Void},
    },
    {u8"chooseDataFiles",
      {u8"选择语言", u8"可批量选择训练数据文件", [this](PluginEvent, void*){
        ChooseLanguages();
      },
      PluginEvent::Void}
    },
    {u8"enableWriteClipboard",
      {u8"复制文字", u8"复制文字到剪切板", [this](PluginEvent event, void* data) {
        if (event == PluginEvent::Bool) {
          m_Settings[u8"WriteClipboard"] =  *reinterpret_cast<bool*>(data);
          mgr->SaveSettings();
          mgr->ShowMsg("设置成功");
        } else if (event == PluginEvent::BoolGet) {
          *reinterpret_cast<bool*>(data) = m_Settings[u8"WriteClipboard"].isTrue();
        }
      }, PluginEvent::Bool}
    },
    {u8"enableShowWindow",
      {u8"显示窗口", u8"唤起极简翻译窗口", [this](PluginEvent event, void* data) {
        if (event == PluginEvent::Bool) {
          m_Settings[u8"ShowWindow"] =  *reinterpret_cast<bool*>(data);
          mgr->SaveSettings();
          mgr->ShowMsg("设置成功");
        } else if (event == PluginEvent::BoolGet) {
          *reinterpret_cast<bool*>(data) = m_Settings[u8"ShowWindow"].isTrue();
        }
      }, PluginEvent::Bool}
    },
  };

  m_Following.push_back({u8"neospeedboxplg", [this](PluginEvent event, void* data){
    if (event == PluginEvent::Drop) {
      const auto mimeData = reinterpret_cast<QDropEvent*>(data)->mimeData();
      if (!mimeData->hasUrls()) return;
      const auto urls = mimeData->urls();
      auto uiUrlsView = urls | std::views::filter([](const QUrl& i) {
          return i.isValid() && i.isLocalFile() && i.fileName().endsWith(".traineddata");
        }) | std::views::transform([](const QUrl& url){
          return fs::path(PluginObject::QString2Utf8(url.toLocalFile()));
        }) | std::views::filter([](const fs::path& url) {
          return fs::exists(url);
        });
    std::vector<fs::path> vec(uiUrlsView.begin(), uiUrlsView.end());
      auto const folder = fs::path(m_Settings[u8"TessdataDir"].getValueString());
      for (const auto& file: vec) {
        fs::copy(file, folder / file.filename());
      }
      if (!vec.empty())
        mgr->ShowMsg("复制数据文件成功。");
    }
  }});
}

void NeoOcrPlg::AddServerMenu() {
  auto const typeAction = m_MainMenu->addAction("谁在识别");
  auto const menu = new MenuBase(m_MainMenu);
  typeAction->setMenu(menu);
  auto const group = new QActionGroup(menu);
  group->setExclusive(true);
  static const QStringList servers = { "Windows", "Tesseract" };
  const std::array description = {
    QStringLiteral("Windows原生的Ocr引擎"),
    QStringLiteral("非常优秀的开源Ocr引擎，识别结果更加准确"),
  };

  for (auto iter =description.cbegin(); const auto& name: servers) {
    auto const action = menu->addAction(name);
    action->setCheckable(true);
    action->setToolTip(*iter++);
    group->addAction(action);
  }

  // servers.at(m_Settings[u8"OcrServer"].getValueInt())
  group->actions().at(m_Settings[u8"OcrServer"].getValueInt())->setChecked(true);

  QObject::connect(group, &QActionGroup::triggered, menu, [this](QAction* action){
    // 目前必须是.getValueDouble()
    m_Settings[u8"OcrServer"].getValueDouble() = static_cast<int>(servers.indexOf(action->text()));
    mgr->ShowMsg("保存成功");
    mgr->SaveSettings();
  });
}

QAction* NeoOcrPlg::InitMenuAction()
{
  AddServerMenu();
  this->PluginObject::InitMenuAction();
  m_MainMenuAction = new QAction("文字识别");
  QObject::connect(m_MainMenuAction, &QAction::triggered, m_MainMenu, std::bind(m_PluginMethod[u8"screenfetch"].function, PluginEvent::Void, nullptr));
  return m_MainMenuAction;
}

YJson& NeoOcrPlg::InitSettings(YJson& settings)
{
  if (!settings.isObject()) {
    return settings = YJson::O {
      { u8"TessdataDir", u8"tessdata" },
      { u8"Languages", YJson::A { u8"chi_sim", u8"eng" } },
      { u8"WriteClipboard", false },
      { u8"ShowWindow", true },
      { u8"Version", 0},
    };
  }
  auto& version = settings[u8"Version"];
  if (!version.isNumber()) {
    version = 0;
    settings[u8"WriteClipboard"] = false;
    settings[u8"ShowWindow"] = true;
  }
  if (version.getValueInt() == 0) {
    version = 1;
    settings[u8"OcrServer"] = 0;
    settings[u8"WinLan"] = u8"user-Profile";
  }
  return settings;
  // we may not need to call SaveSettings;
}

void NeoOcrPlg::ChooseLanguages()
{
  auto const dialog = new QDialog;
  dialog->setWindowTitle("语种选择");
  dialog->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);

  auto const vlayout = new QVBoxLayout(dialog);
  AddWindowsSection(dialog, vlayout);
  AddTesseractSection(dialog, vlayout);
  dialog->exec();
}

void NeoOcrPlg::AddWindowsSection(QWidget* parent, QVBoxLayout* layout) {
  layout->addWidget(new QLabel("<h3>Windows Ocr目前支持下列语言</h3>", parent));

  QHBoxLayout* hlayout = nullptr;
  auto const group = new QButtonGroup(parent);
  group->setExclusive(true);

  auto& u8CurLan = m_Settings[u8"WinLan"].getValueString();
  auto qLanTag = QString::fromUtf8(u8CurLan.data(), u8CurLan.size());

  // WinLan
  for (int count = 0; const auto& [tag, name]: m_Ocr->GetLanguages()) {
    if (count == 0) {
      hlayout = new QHBoxLayout;
      layout->addLayout(hlayout);
    }

    auto qTag = QString::fromStdWString(tag);
    auto const chk = new QRadioButton(QString::fromStdWString(name), parent);
    chk->setProperty("lanTag", qTag);
    chk->setChecked(qLanTag == qTag);
    group->addButton(chk);
    hlayout->addWidget(chk);

    if (++count == 7) {
      count = 0;
    }
  }
  QObject::connect(group, &QButtonGroup::buttonClicked, parent, [&u8CurLan](QAbstractButton* box){
    u8CurLan = QString2Utf8(box->property("lanTag").toString());
    mgr->ShowMsg("保存成功");
    mgr->SaveSettings();
  });
  layout->addWidget(new QLabel("如果想要支持更多语言，请在 Windows 设置中安装。", parent));
}

void NeoOcrPlg::AddTesseractSection(QWidget* parent, QVBoxLayout* layout) {
  QHBoxLayout* hlayout = nullptr;
  auto label = new QLabel("<h3>Tesseract Ocr目前支持下列语言</h3><p>如果为空不要慌，将下载好的语言文件(*.traineddata)拖拽到网速悬浮窗后，在这里就可以看到了~</p>"
      "<p>"
      "<a href=\"https://tesseract-ocr.github.io/tessdoc/Data-Files.html\">官网</a>"
      "提供了三种下载选择："
      "<a href=\"https://github.com/tesseract-ocr/tessdata_fast\">tessdata_fast</a>，"
      "<a href=\"https://github.com/tesseract-ocr/tessdata_best\">tessdata_best</a>，"
      "<a href=\"https://github.com/tesseract-ocr/tessdata\">tessdata</a>，"
      "点击进入相应的github页面即可下载。</p>", parent);
  label->setOpenExternalLinks(true);
  layout->addWidget(label);
  auto const chkboxs = new QButtonGroup(parent);
  chkboxs->setExclusive(false);

  std::set<QString> curDatas;
  for (auto& item: m_Settings[u8"Languages"].getArray()) {
    curDatas.insert(Utf82QString(item.getValueString()));
  }

  const QDir dir(Utf82QString(m_Settings[u8"TessdataDir"].getValueString()));
  QStringList files = dir.entryList(QStringList { QStringLiteral("*.traineddata") }, QDir::Files | QDir::Readable, QDir::Name);

  for (size_t i=0; const auto& file: files) {
    auto const name = file.left(file.indexOf('.'));
    auto const box = new QCheckBox(name, parent);
    chkboxs->addButton(box);
    box->setChecked(curDatas.find(name) != curDatas.end());
    if (i == 0) {
      hlayout = new QHBoxLayout;
      layout->addLayout(hlayout);
      ++i;
    } else if (i == 6) {
      i = 0;
    } else {
      ++i;
    }
    hlayout->addWidget(box);
  }
  auto const btnOk = new QPushButton("确认", parent);
  QObject::connect(btnOk, &QPushButton::clicked, parent, [this, parent, chkboxs](){
    auto& object = m_Settings[u8"Languages"].getArray();
    object.clear();
    for (auto box: chkboxs->buttons() | std::views::filter(std::bind(&QAbstractButton::isChecked, std::placeholders::_1))) {
      object.push_back(QString2Utf8(box->text()));
    }
    m_Ocr->InitLanguagesList();
    mgr->SaveSettings();
    mgr->ShowMsg("保存成功！");
    parent->close();
  });
  layout->addWidget(btnOk);
}
