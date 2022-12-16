#include <neoocrplg.h>
#include <neoocr.h>
#include <pluginmgr.h>
#include <screenfetch.h>
#include <yjson.h>
#include <neoapp.h>
#include <neoapp.h>

#include <QMenu>
#include <QDir>
#include <QAction>
#include <QEventLoop>
#include <QFileDialog>
#include <QPlainTextEdit>
#include <QImage>
#include <QDropEvent>
#include <QMimeData>

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
        for (auto fun: m_Followers) {
          fun->operator()(PluginEvent::U8string, &str);
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
          u8Path.swap(u8NewPath);
          mgr->SaveSettings();
          glb->glbShowMsg("设置数据文件失成功！");
        } else {
          glb->glbShowMsg("设置数据文件失败！");
        }
      }, PluginEvent::Void},
    }
  };

  m_Following.push_back({u8"neospeedboxplg", [this](PluginEvent event, void* data){
    if (event == PluginEvent::Drop) {
      const auto mimeData = reinterpret_cast<QDropEvent*>(data)->mimeData();
      if (!mimeData->hasUrls()) return;
      auto urls = mimeData->urls();
      auto uiUrlsView = urls | std::views::filter([](const QUrl& i) {
      return i.isValid() && i.isLocalFile() && i.fileName().endsWith(".traineddata"); });
      // to do sth
    }
  }});

  m_Following.push_back({u8"neohotkeyplg", [this](PluginEvent event, void* data){
    if (event == PluginEvent::HotKey) {
      // 判断是否为想要的快捷键
      // MSG* msg = reinterpret_cast<MSG*>(data);
      m_PluginMethod[u8"screenfetch"].function(event, data);
    }
  }});
}

QAction* NeoOcrPlg::InitMenuAction()
{
  return this->PluginObject::InitMenuAction();
}

YJson& NeoOcrPlg::InitSettings(YJson& settings)
{
  if (settings.isObject()) return settings;
  return settings = YJson::O {
    { u8"TessdataDir", u8"tessdata" },
    { u8"Languages", YJson::A { u8"chi_sim", u8"eng" } },
  };
  // we may not need to call SaveSettings;
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
