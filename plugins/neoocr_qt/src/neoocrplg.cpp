#include <neoocrplg.h>
#include <neoocr.h>
#include <pluginmgr.h>
#include <screenfetch.h>
#include <yjson.h>
#include <glbobject.h>

#include <QMenu>
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

#include <leptonica/allheaders.h>
#include <filesystem>
#include <ranges>

#define CLASS_NAME NeoOcrPlg
#include <pluginexport.cpp>

namespace fs = std::filesystem;
Pix* QImage2Pix(const QImage& qImage);

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
        auto str = m_Ocr->GetText(QImage2Pix(image));
        if (m_Settings[u8"WriteClipboard"].isTrue()) {
          QApplication::clipboard()->setText(Utf82QString(str));
          glb->glbShowMsg("复制数据成功");
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
        const QString folder =
            QFileDialog::getExistingDirectory(
                glb->glbGetMenu(), "请选择Tessdata数据文件存放位置",
                QString::fromUtf8(u8Path.data(), u8Path.size()));
        if (folder.isEmpty() || folder.isNull()) {
          glb->glbShowMsg("取消设置成功！");
          return;
        }
        fs::path pNewPath = PluginObject::QString2Utf8(folder);
        pNewPath.make_preferred();
        std::u8string u8NewPath = pNewPath.u8string();
        if (!u8NewPath.empty() && u8NewPath != u8Path) {
          m_Ocr->SetDataDir(u8NewPath);
          u8Path.swap(u8NewPath);
          mgr->SaveSettings();
          glb->glbShowMsg("设置数据文件失成功！");
        } else {
          glb->glbShowMsg("设置数据文件失败！");
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
          glb->glbShowMsg("设置成功");
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
          glb->glbShowMsg("设置成功");
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
        glb->glbShowMsg("复制数据文件成功。");
    }
  }});

  m_Following.push_back({u8"neohotkeyplg", [this](PluginEvent event, void* data){
    if (event == PluginEvent::HotKey) {
      // 判断是否为想要的快捷键
      if (*reinterpret_cast<std::u8string*>(data) == u8"neoocrplg") {
        // MSG* msg = reinterpret_cast<MSG*>(data);
        m_PluginMethod[u8"screenfetch"].function(event, data);
      }
    }
  }});
}

QAction* NeoOcrPlg::InitMenuAction()
{
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
  return settings;
  // we may not need to call SaveSettings;
}

void NeoOcrPlg::ChooseLanguages()
{
  const auto& path = m_Settings[u8"TessdataDir"].getValueString();
  QDir dir(Utf82QString(path));
  QStringList files = dir.entryList(QStringList { QStringLiteral("*.traineddata") }, QDir::Files | QDir::Readable, QDir::Name);

  std::set<QString> curDatas;
  for (auto& item: m_Settings[u8"Languages"].getArray()) {
    curDatas.insert(Utf82QString(item.getValueString()));
  }

  auto const dialog = new QDialog;
  dialog->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);

  auto const vlayout = new QVBoxLayout(dialog);
  QHBoxLayout* hlayout = nullptr;
  auto label = new QLabel("<p>如果为空不要慌，将下载好的语言文件(*.traineddata)拖拽到网速悬浮窗后，在这里就可以看到了~</p>"
      "<p>官网提供了三种下载选择："
      "<a href=\"https://github.com/tesseract-ocr/tessdata_fast\">tessdata_fast</a>，"
      "<a href=\"https://github.com/tesseract-ocr/tessdata_best\">tessdata_best</a>，"
      "<a href=\"https://github.com/tesseract-ocr/tessdata\">tessdata</a>，"
      "点击进入相应的github页面即可下载。</p>", dialog);
  vlayout->addWidget(label);
  std::vector<QCheckBox*> chkboxs;

  for (size_t i=0; auto file: files) {
    auto const name = file.left(file.indexOf('.'));
    auto const box = new QCheckBox(name, dialog);
    chkboxs.push_back(box);
    box->setChecked(curDatas.find(name) != curDatas.end());
    if (i == 0) {
      hlayout = new QHBoxLayout;
      vlayout->addLayout(hlayout);
      ++i;
    } else if (i == 6) {
      i = 0;
    } else {
      ++i;
    }
    hlayout->addWidget(box);
  }

  hlayout = new QHBoxLayout;
  vlayout->addLayout(hlayout);
  auto const btnDownload = new QPushButton("官方网站", dialog);
  auto const btnNo = new QPushButton("取消", dialog);
  auto const btnOk = new QPushButton("确认", dialog);
  QObject::connect(btnDownload, &QPushButton::clicked, dialog, std::bind(
        &QDesktopServices::openUrl, QUrl("https://tesseract-ocr.github.io/tessdoc/Data-Files.html")));
  QObject::connect(btnOk, &QPushButton::clicked, dialog, [&](){
    auto& object = m_Settings[u8"Languages"];
    object.clearA();
    for (auto box: chkboxs) {
      if (!box->isChecked()) continue;
      object.append(QString2Utf8(box->text()));
    }
    m_Ocr->InitLanguagesList();
    mgr->SaveSettings();
    glb->glbShowMsg("保存成功！");
    dialog->close();
  });
  QObject::connect(btnNo, &QPushButton::clicked, dialog, &QDialog::close);
  hlayout->addWidget(btnDownload);
  hlayout->addWidget(btnNo);
  hlayout->addWidget(btnOk);
  dialog->exec();
}

static inline bool IsBigDuan() {
  const uint16_t s = 1;
  return *reinterpret_cast<const uint8_t*>(&s);
}

Pix* QImage2Pix(const QImage& qImage) {
  static const bool bIsBigDuan = IsBigDuan();
  if (qImage.isNull())
    return nullptr;
  const int width = qImage.width(), height = qImage.height();
  const int depth = qImage.depth(), bytePerLine = qImage.bytesPerLine();
  PIX* pix = pixCreate(width, height, depth);

  if (qImage.colorCount()) {
    PIXCMAP* map = pixcmapCreate(8);
    if (bIsBigDuan) {  // b g r a
      for (const auto& i : qImage.colorTable()) {
        auto cols = reinterpret_cast<const uchar*>(&i);
        pixcmapAddColor(map, cols[2], cols[1], cols[0]);
      }
    } else {  // a r g b
      for (const auto& i : qImage.colorTable()) {
        auto cols = reinterpret_cast<const uchar*>(&i);
        pixcmapAddColor(map, cols[1], cols[2], cols[3]);
      }
    }
    pixSetColormap(pix, map);
  }

  auto start = pixGetData(pix);
  auto wpld = pixGetWpl(pix);

  switch (qImage.format()) {
    case QImage::Format_Mono:
    case QImage::Format_Indexed8:
    case QImage::Format_RGB888:
      for (int i = 0; i < height; ++i) {
        std::copy_n(qImage.scanLine(i), bytePerLine,
                    reinterpret_cast<uchar*>(start + wpld * i));
      }
      break;
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
      for (int i = 0; i < height; ++i) {
        auto lines = qImage.scanLine(i);
        l_uint32* lined = start + wpld * i;
        if (bIsBigDuan) {
          for (int j = 0; j < width; ++j, lines += 4) {
            l_uint32 pixel;
            composeRGBPixel(lines[2], lines[1], lines[0], &pixel);
            lined[j] = pixel;
          }
        } else {
          for (int j = 0; j < width; ++j, lines += 4) {
            l_uint32 pixel;
            composeRGBPixel(lines[1], lines[2], lines[3], &pixel);
            lined[j] = pixel;
          }
        }
      }
      break;
    default:
      break;
  }
  return pix;
}
