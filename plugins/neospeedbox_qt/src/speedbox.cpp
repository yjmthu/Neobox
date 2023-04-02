#include <QApplication>
#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QTimer>
#include <QVBoxLayout>
#include <QSharedMemory>
#include <QProcess>

#include <neomenu.hpp>
#include <speedbox.h>
#include <yjson.h>
#include <pluginmgr.h>
#include <appcode.hpp>
#include <pluginobject.h>
#include <neospeedboxplg.h>
#include <skinobject.h>
#include <netspeedhelper.h>
#include <processform.h>

#include <array>
#include <filesystem>
#include <ranges>

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

SpeedBox::SpeedBox(PluginObject* plugin, SpeedBoxCfg& settings, MenuBase* netcardMenu)
    : WidgetBase(nullptr)
    , m_PluginObject(plugin)
    , m_Settings(settings)
    , m_NetSpeedHelper(*dynamic_cast<NeoSpeedboxPlg*>(plugin)->m_NetSpeedHelper)
    , m_ProcessForm(m_Settings.GetProgressMonitor() ?new ProcessForm(this) : nullptr)
    , m_NetCardMenu(*netcardMenu)
    , m_CentralWidget(nullptr)
    , m_SkinDll(nullptr)
    , m_Timer(new QTimer(this))
    , m_Animation(new QPropertyAnimation(this, "geometry"))
{
  SetWindowMode();
#ifdef _WIN32
  SetHideFullScreen();
#endif
  LoadCurrentSkin();
  InitNetCard();
}

SpeedBox::~SpeedBox() {
  delete m_ProcessForm;
  delete m_CentralWidget;
#ifdef _WIN32
  FreeLibrary(m_SkinDll);
  SHAppBarMessage(ABM_REMOVE, reinterpret_cast<APPBARDATA*>(m_AppBarData));
  delete reinterpret_cast<APPBARDATA*>(m_AppBarData);
#else
  dlclose(m_SkinDll);
#endif
  
  // m_Timer->stop();
  delete m_Timer;
}

void SpeedBox::InitShow(const PluginObject::FollowerFunction& callback) {
  show();

  m_Animation->setDuration(100);
  m_Animation->setTargetObject(this);
  connect(m_Animation, &QPropertyAnimation::finished, this, [this]() {
    m_Settings.SetPosition(YJson::A{x(), y()});
  });

  UpdateNetCardMenu();

  bool buffer = m_Settings.GetColorEffect();
  callback(PluginEvent::Bool, &buffer);
}

void SpeedBox::SetWindowMode() {
  setWindowTitle("Neobox");
  setWindowIcon(QIcon(QStringLiteral(":/icons/neobox.ico")));
  setAttribute(Qt::WA_TransparentForMouseEvents, m_Settings.GetMousePenetrate());
  setAttribute(Qt::WA_TranslucentBackground, true);
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
  setAcceptDrops(true);

  auto position = m_Settings.GetPosition();
  move(position.front().getValueInt(), position.back().getValueInt());
}

bool SpeedBox::UpdateSkin()
{
  delete m_CentralWidget;
#ifdef _WIN32
    FreeLibrary(m_SkinDll);
#else
    dlclose(m_SkinDll);
#endif
  m_CentralWidget = nullptr;
  m_SkinDll = nullptr;
  return LoadCurrentSkin();
}

bool SpeedBox::LoadDll(fs::path dllPath)
{
  if (!fs::exists(dllPath)) return false;

  SkinObject* (*newSkin)(QWidget*, const TrafficInfo&);
  // bool (*skinVersion)(const std::string&);
  dllPath.make_preferred();
#ifdef _WIN32
  auto wPath = dllPath.make_preferred().wstring();
  wPath.push_back(L'\0');
  m_SkinDll = LoadLibraryW(wPath.data());
#else
  auto cPath = dllPath.make_preferred().string();
  cPath.push_back('\0');
  m_SkinDll = dlopen(cPath.data(), RTLD_LAZY);
#endif
  if (!m_SkinDll) return false;

  newSkin = reinterpret_cast<decltype(newSkin)>
#ifdef _WIN32
  (GetProcAddress(m_SkinDll, "newSkin"));
#else
  (dlsym(m_SkinDll, "newSkin"));
#endif
  if (!newSkin) {
#ifdef _WIN32
    FreeLibrary(m_SkinDll);
#else
    dlclose(m_SkinDll);
#endif
    m_SkinDll = nullptr;
    return false;
  }

  m_CentralWidget = newSkin(this, m_NetSpeedHelper.m_TrafficInfo);

  return true;
}

bool SpeedBox::LoadCurrentSkin() {

  auto skinName = m_Settings.GetCurSkin();
  const auto skinFileName = YJson(m_Settings.GetUserSkins())[skinName].getValueString();

#ifdef _WIN32
  auto skinPath = u8"skins/" + skinFileName + u8".dll";
  auto qResSkinPath = ":/dlls/" + QString::fromUtf8(skinFileName.data(), skinFileName.size()) + ".dll";
#else
  auto skinPath = u8"skins/lib" + skinFileName + u8".so";
  auto qResSkinPath = ":/sos/lib" + QString::fromUtf8(skinFileName.data(), skinFileName.size()) + ".so";
#endif
  auto qSkinPath = QString::fromUtf8(skinPath.data(), skinPath.size());

  if (!fs::exists("skins"))
    fs::create_directory("skins");
  
  
  if (QFile::exists(qResSkinPath)) {
    if (QFile::exists(qSkinPath)) {
      QFile fileConst(qResSkinPath);
      fileConst.open(QIODevice::ReadOnly);
      auto const resData = fileConst.readAll();
      fileConst.close();

      QFile fileVar(qSkinPath);
      fileVar.open(QIODevice::ReadOnly);
      auto const varData = fileVar.readAll();
      fileVar.close();

      if (resData != varData) {
        QFile::remove(qSkinPath);
        QFile::copy(qResSkinPath, qSkinPath);
        QFile::setPermissions(qSkinPath, QFile::ReadUser | QFile::WriteUser);
      }
    } else {
      QFile::copy(qResSkinPath, qSkinPath);
      QFile::setPermissions(qSkinPath, QFile::ReadUser | QFile::WriteUser);
    }
  } else if (!QFile::exists(qSkinPath)) {
    mgr->ShowMsg("找不到皮肤资源");
  }

  return LoadDll(skinPath);
}

#ifdef _WIN32
void SpeedBox::SetHideFullScreen() {
  m_AppBarData = new APPBARDATA {
    sizeof(APPBARDATA),
    reinterpret_cast<HWND>(winId()), 
    static_cast<UINT>(MsgCode::MSG_APPBAR_MSGID),
    0, 0, 0
  };
  SHAppBarMessage(ABM_NEW, reinterpret_cast<APPBARDATA*>(m_AppBarData));
}
#endif

void SpeedBox::SetProgressMonitor(bool on)
{
  if (on) {
    if (!m_ProcessForm)
      m_ProcessForm = new ProcessForm(this);
  } else {
    delete m_ProcessForm;
    m_ProcessForm = nullptr;
  }
}

void SpeedBox::wheelEvent(QWheelEvent *event)
{
  auto const numDegrees = event->angleDelta();

  if (!numDegrees.isNull() && numDegrees.y()) {
    if (numDegrees.y() > 0) {
      if (m_ProcessForm && !m_ProcessForm->isVisible()) {
        m_ProcessForm->show();
      }
    } else {
      if (m_ProcessForm && m_ProcessForm->isVisible()) {
        m_ProcessForm->hide();
      }
    }
  }

  event->accept();
}

void SpeedBox::mouseMoveEvent(QMouseEvent* event) {
  if (event->buttons() == Qt::LeftButton) {
    if (m_ProcessForm && m_ProcessForm->isVisible())
      m_ProcessForm->hide();
    m_PluginObject->SendBroadcast(PluginEvent::MouseMove, event);
  }
  this->WidgetBase::mouseMoveEvent(event);
}

void SpeedBox::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::RightButton) {
    mgr->m_Menu->popup(pos() + event->pos());
  } else if (event->button() == Qt::MiddleButton) {
    mgr->Restart();
  }
  this->WidgetBase::mousePressEvent(event);
}

void SpeedBox::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    m_Settings.SetPosition(YJson::A{x(), y()});
  }
  this->WidgetBase::mouseReleaseEvent(event);
}

void SpeedBox::mouseDoubleClickEvent(QMouseEvent* event) {
  m_PluginObject->SendBroadcast(PluginEvent::MouseDoubleClick, event);
  event->accept();
}

void SpeedBox::dragEnterEvent(QDragEnterEvent* event) {
  auto mimeData = event->mimeData();
  if (mimeData->hasUrls() || mimeData->hasText())
    event->acceptProposedAction();
  else
    event->ignore();
}

void SpeedBox::dropEvent(QDropEvent* event) {
  auto mimeData = event->mimeData();

  m_PluginObject->SendBroadcast(PluginEvent::Drop, event);
  event->accept();
}

#ifdef _WIN32
bool SpeedBox::nativeEvent(const QByteArray& eventType,
                           void* message,
                           qintptr* result) {
  MSG* msg = static_cast<MSG*>(message);

  if (MsgCode::MSG_APPBAR_MSGID == static_cast<MsgCode>(msg->message)) {
    switch ((UINT)msg->wParam) {
      case ABN_FULLSCREENAPP:
        if (msg->lParam)
          hide();
        else
          show();
        return true;
      default:
        break;
    }
  }
  return false;
}
#endif

static constexpr int delta = 1;

void SpeedBox::enterEvent(QEnterEvent* event) {
  auto rtScreen = QGuiApplication::primaryScreen()->geometry();
  auto rtForm = this->frameGeometry();
  m_Animation->setStartValue(rtForm);
  switch (m_HideSide) {
    case HideSide::Left:
      rtForm.moveTo(-delta, rtForm.y());
      break;
    case HideSide::Right:
      rtForm.moveTo(rtScreen.right() - rtForm.width() + delta, rtForm.y());
      break;
    case HideSide::Top:
      rtForm.moveTo(rtForm.x(), -delta);
      break;
    case HideSide::Bottom:
      rtForm.moveTo(rtForm.x(), rtScreen.bottom() - rtForm.height() + delta);
      break;
    default:
      event->ignore();
      return;
  }
  event->accept();
  m_Animation->setEndValue(rtForm);
  m_Animation->start();
}

void SpeedBox::leaveEvent(QEvent* event) {
  auto rtScreen = QGuiApplication::primaryScreen()->geometry();
  auto rtForm = this->frameGeometry();
  m_Animation->setStartValue(rtForm);
  auto const hideBits = m_Settings.GetHideAside();
  if ((HideSide::Right & hideBits) && rtForm.right() + delta >= rtScreen.right()) {
    m_HideSide = HideSide::Right;
    rtForm.moveTo(rtScreen.right() - delta, rtForm.y());
  } else if ((HideSide::Left & hideBits)  && rtForm.left() - delta <= rtScreen.left()) {
    m_HideSide = HideSide::Left;
    rtForm.moveTo(delta - rtForm.width(), rtForm.y());
  } else if ((HideSide::Top & hideBits) && rtForm.top() - delta <= rtScreen.top()) {
    m_HideSide = HideSide::Top;
    rtForm.moveTo(rtForm.x(), delta - rtForm.height());
  } else if ((HideSide::Bottom & hideBits) && rtForm.bottom() + delta >= rtScreen.bottom()) {
    m_HideSide = HideSide::Bottom;
    rtForm.moveTo(rtForm.x(), rtScreen.bottom() - delta);
  } else {
    m_HideSide = HideSide::None;
    event->ignore();
    return;
  }
  m_Animation->setEndValue(rtForm);
  m_Animation->start();
  event->accept();
}

void SpeedBox::InitMove()
{
  move(100, 100);
  m_HideSide = HideSide::None;
  m_Settings.SetPosition(YJson::A{100, 100});
  if (!isVisible())
    show();
  mgr->ShowMsg("移动成功！");
}


void SpeedBox::InitNetCard()
{

  connect(m_Timer, &QTimer::timeout, this, [this]() {
#ifdef _WIN32
    static int count = 10;
    if (--count == 0) {
      m_NetSpeedHelper.UpdateAdaptersAddresses();
      UpdateNetCardMenu();
      count = 10;
    }
#endif
    if (!m_CentralWidget) return;
    m_NetSpeedHelper.GetSysInfo();
    m_CentralWidget->UpdateText();
    if (m_ProcessForm) {
      m_ProcessForm->UpdateList();
    }
  });
  m_Timer->start(1000);
}

void SpeedBox::UpdateNetCardMenu()
{
  for (auto i: m_NetCardMenu.actions()) {
    delete i;
  }
  m_NetCardMenu.clear();
  for (const auto& i: m_NetSpeedHelper.m_Adapters) {
#ifdef _WIN32
    const auto fName = QString::fromWCharArray(i.friendlyName.data(), i.friendlyName.size());
    auto const action = m_NetCardMenu.addAction(fName);
#else
    auto const action = m_NetCardMenu.addAction(QString::fromUtf8(i.adapterName.data(), i.adapterName.size()));
#endif
    action->setCheckable(true);
    action->setChecked(i.enabled);
#ifdef _WIN32
    action->setToolTip(QString::fromUtf8(i.adapterName.data(), i.adapterName.size()));
#endif
    connect(action, &QAction::triggered, this, std::bind(
      &SpeedBox::UpdateNetCard, this, action, std::placeholders::_1
    ));
  }
}

void SpeedBox::UpdateNetCard(QAction* action, bool checked)
{
  const auto data = action->toolTip().toUtf8();
  const std::u8string_view guid(reinterpret_cast<const char8_t*>(data.data()), data.size());

  if (checked) {
    // 因为 m_AdapterBalckList 使用的是 u8string_view，所以顺序很重要。
    m_NetSpeedHelper.m_AdapterBalckList.erase(guid);
    auto blklst = m_Settings.GetNetCardDisabled();
    blklst.removeByValA(guid);
    m_Settings.SetNetCardDisabled(std::move(blklst));
    mgr->ShowMsg("添加网卡成功！"); // removed from blacklist
  } else {
    auto blklst = m_Settings.GetNetCardDisabled();
    auto iter = blklst.append(guid);
    m_NetSpeedHelper.m_AdapterBalckList.emplace(iter->getValueString());
    m_Settings.SetNetCardDisabled(std::move(blklst));
    mgr->ShowMsg("删除网卡成功！");
  }
  m_NetSpeedHelper.UpdateAdaptersAddresses();
}
