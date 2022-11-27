#include <neoapp.h>
#include <neomenu.h>
#include <neomsgdlg.h>
#include <neosystemtray.h>
#include <yjson.h>

#include <QSharedMemory>
#include <QMessageBox>
#include <QApplication>

GlbObject *glb;

QMenu* GlbObject::glbGetMenu() {
  return glbMenu;
}

void CompareJson(YJson& jsDefault, YJson& jsUser)
{
  if (!jsDefault.isSameType(&jsUser))
    return;
  if (!jsUser.isObject()) {
    YJson::swap(jsDefault, jsUser);
    return;
  }
  for (auto& [key, val]: jsUser.getObject()) {
    auto iter = jsDefault.find(key);
    if (iter != jsDefault.endO()) {
      CompareJson(iter->second, val);
    } else {
      YJson::swap(jsDefault[key], val);
    }
  }
}


void GlbObject::glbShowMsgbox(const std::u8string& title,
                 const std::u8string& text,
                 int type) {
  QMetaObject::invokeMethod(glbMenu, [=](){
    QMessageBox::information(glbMenu,
                             QString::fromUtf8(title.data(), title.size()),
                             QString::fromUtf8(text.data(), text.size()));
  });
}

void GlbObject::glbShowMsg(class QString text)
{
  glbMsgDlg->ShowMessage(text);
}

#if 0
void GlbObject::glbWriteSharedFlag(int flag) {
  QSharedMemory mem;
  mem.setKey(QStringLiteral("__Neobox__"));
  mem.lock();
  *reinterpret_cast<int *>(mem.data()) = flag;
  mem.unlock();
}

int GlbObject::glbReadSharedFlag() {
  QSharedMemory mem;
  mem.setKey(QStringLiteral("__Neobox__"));
  mem.lock();
  const auto state = *reinterpret_cast<const int*>(mem.constData());
  mem.unlock();
  return state;
}

bool GlbObject::glbCreateSharedMemory() {
  QSharedMemory mem;
  mem.setKey(QStringLiteral("__Neobox__"));
  if(mem.attach()) {
    /*
    * 0: already have an instance;
    * 1: previous app want to restart;
    * 2: app should go to left top.
    * 4: app should quit
    */
    if (glbReadSharedFlag() == 0) {
      glbWriteSharedFlag(2);
      mem.detach();
      return false;
    }
  } else if (!mem.create(sizeof(int))) {
    return false;
  }
  glbWriteSharedFlag(0);
  return true;
}

void GlbObject::glbDetachSharedMemory()
{
  QSharedMemory mem;
  mem.setKey(QStringLiteral("__Neobox__"));
  mem.detach();
}
#endif

GlbObject::GlbObject()
{
  glb = this;
  // if (!glbCreateSharedMemory())
  //   return;
  QApplication::setQuitOnLastWindowClosed(false);
  // glbDetachSharedMemory();
  glbTray = new NeoSystemTray;
  glbMenu = new NeoMenu(this);
  glbMsgDlg = new NeoMsgDlg(glbMenu);
  glbMenu->InitPluginMgr();
  glbTray->setContextMenu(glbMenu);
  glbTray->show();
  QApplication::exec();
}

GlbObject::~GlbObject()
{
}
