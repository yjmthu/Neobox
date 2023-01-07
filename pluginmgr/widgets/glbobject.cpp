#include <glbobject.h>
#include <neomenu.h>
#include <neomsgdlg.h>
#include <neosystemtray.h>
#include <yjson.h>

#include <QProcess>
#include <QSharedMemory>
#include <QMessageBox>
#include <QApplication>

extern GlbObject *glb;

QMenu* GlbObject::glbGetMenu() {
  return m_Menu;
}

NeoSystemTray* GlbObject::glbGetSystemTray()
{
  return m_Tray;
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
  QMetaObject::invokeMethod(m_Menu, [=](){
    QMessageBox::information(m_Menu,
                             QString::fromUtf8(title.data(), title.size()),
                             QString::fromUtf8(text.data(), text.size()));
  });
}

void GlbObject::glbShowMsg(class QString text)
{
  m_MsgDlg->ShowMessage(text);
}

void GlbObject::glbWriteSharedFlag(int flag) {
  //m_SharedMemory->setKey(QStringLiteral("__Neobox__"));
  m_SharedMemory->lock();
  *reinterpret_cast<int *>(m_SharedMemory->data()) = flag;
  m_SharedMemory->unlock();
}

int GlbObject::glbReadSharedFlag() {
  //m_SharedMemory->setKey(QStringLiteral("__Neobox__"));
  m_SharedMemory->lock();
  const auto state = *reinterpret_cast<const int*>(m_SharedMemory->constData());
  m_SharedMemory->unlock();
  return state;
}

bool GlbObject::glbCreateSharedMemory() {
  m_SharedMemory->setKey(QStringLiteral("__Neobox__"));
  if(m_SharedMemory->attach()) {
    auto const code = glbReadSharedFlag();
    switch (code) {
    case 0:   //  already have an instance;
      glbWriteSharedFlag(2);
      m_SharedMemory->detach();
      return false;
    case 1:   // previous app want to restart;
      break;
    case 2:   // app should go to left top.
      glbWriteSharedFlag(0);
      m_SharedMemory->detach();
      return false;
    case 3:   // app should quit
    default:
      break;
    }
  } else if (!m_SharedMemory->create(sizeof(int))) {
    throw std::runtime_error("Already have an instance.");
  }
  glbWriteSharedFlag(0);
  return true;
}

void GlbObject::glbDetachSharedMemory()
{
  // m_SharedMemory->setKey(QStringLiteral("__Neobox__"));
  m_SharedMemory->detach();
}

GlbObject::GlbObject()
  : m_SharedMemory(new QSharedMemory)
  , m_Tray(nullptr)
  , m_Menu(nullptr)
  , m_MsgDlg(nullptr)
{
  glb = this;
  if (!glbCreateSharedMemory()) {
    throw std::runtime_error("Can not create QSharedMemory.");
  }
  QApplication::setQuitOnLastWindowClosed(false);
  m_Tray = new NeoSystemTray;
  m_Menu = new NeoMenu(this);
  m_MsgDlg = new NeoMsgDlg(m_Menu);
  m_Menu->InitPluginMgr();
  m_Tray->setContextMenu(m_Menu);
  m_Tray->show();
}

GlbObject::~GlbObject()
{
  glb = nullptr;
  glbDetachSharedMemory();   // 在构造函数抛出异常后析构函数将不再被调用
  delete m_Menu;
  delete m_Tray;
}

int GlbObject::Exec()
{
  return QApplication::exec();
}

void GlbObject::Quit()
{
  QApplication::quit();
}

void GlbObject::Restart()
{
  glbWriteSharedFlag(1);
  QProcess::startDetached(
    QApplication::applicationFilePath(), QStringList {}
  );
  QApplication::quit();
}
