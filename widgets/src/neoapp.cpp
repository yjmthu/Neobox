#include <neoapp.h>
#include <neomenu.h>
#include <neomsgdlg.h>
#include <neosystemtray.h>
#include <yjson.h>

#include <QSharedMemory>
#include <QMessageBox>

static NeoSystemTray* glbTray = nullptr;
static NeoMenu* glbMenu = nullptr;
static QSharedMemory* glbSharedMemory = nullptr;
static NeoMsgDlg* glbMsgDlg = nullptr;

QMenu* glbGetMenu() {
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


void glbShowMsgbox(const std::u8string& title,
                 const std::u8string& text,
                 int type) {
  QMetaObject::invokeMethod(glbMenu, [=](){
    QMessageBox::information(glbMenu,
                             QString::fromUtf8(title.data(), title.size()),
                             QString::fromUtf8(text.data(), text.size()));
  });
}

void glbShowMsg(class QString text)
{
  glbMsgDlg->ShowMessage(text);
}

void glbWriteSharedFlag(int flag) {
  glbSharedMemory->lock();
  *reinterpret_cast<int *>(glbSharedMemory->data()) = flag;
  glbSharedMemory->unlock();
}

int glbReadSharedFlag() {
  glbSharedMemory->lock();
  const auto state = *reinterpret_cast<const int*>(glbSharedMemory->constData());
  glbSharedMemory->unlock();
  return state;
}

bool glbCreateSharedMemory() {
  static QSharedMemory qSharedMemory;
  glbSharedMemory = &qSharedMemory;
  qSharedMemory.setKey(QStringLiteral("__Neobox__"));
  if(qSharedMemory.attach()) {
    /*
    * 0: already have an instance;
    * 1: previous app want to restart;
    * 2: app should go to left top.
    * 4: app should quit
    */
    if (glbReadSharedFlag() == 0) {
      glbWriteSharedFlag(2);
      glbSharedMemory->detach();
      return false;
    }
  } else if (!qSharedMemory.create(sizeof(int))) {
    return false;
  }
  glbWriteSharedFlag(0);
  return true;
}

void glbDetachSharedMemory()
{
  glbSharedMemory->detach();
}

void InitApp()
{
  glbTray = new NeoSystemTray;
  glbMenu = new NeoMenu;
  glbMsgDlg = new NeoMsgDlg(glbMenu);
  glbMenu->InitPluginMgr();
  glbTray->setContextMenu(glbMenu);
  glbTray->show();
}
