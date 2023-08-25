#include <neobox/shortcut.h>
#include <neobox/pluginobject.h>
#include <yjson/yjson.h>
#include <neobox/systemapi.h>
#include <neobox/pluginmgr.h>
#include <../widgets/plugincenter.hpp>

#include <QWidget>
#include <QApplication>

#include <QProcess>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <xcb/xcb.h>
#include <X11/Xlib.h>
#endif

#include <ranges>

using namespace std::literals;

std::vector<char*> LoadArgList(const YJson::ArrayType& array) {
  std::vector<char*> result;
  for (auto const& i: array | std::views::transform([](const YJson& item){ return item.getValueString(); })) {
    auto const str = new char[i.size()+1];
    str[i.size()] = 0;
    std::copy_n(i.data(), i.size(), str);
    result.push_back(str);
  }
  result.push_back(nullptr);
  return result;
}

void FreeArgList(const std::vector<char*>& args) {
  for (auto const str: args) {
    delete [] str;
  }
}

bool Shortcut::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *)
{
  if (PluginCenter::m_Instance) return false;
#ifdef _WIN32
  if(eventType != "windows_generic_MSG" && eventType != "windows_dispatcher_MSG") {
    return false;
  }
  MSG* msg = static_cast<MSG*>(message);

  if (WM_HOTKEY != msg->message) {
    return false;
  }
  /*
   * idHotKey = wParam;
   * Modifiers = (UINT) LOWORD(lParam);
   * uVirtKey = (UINT) HIWORD(lParam);
   */
  auto const hash = msg->wParam;
  auto& keyString = GetCallbackInfo(hash);
#else
  // https://stackoverflow.com/questions/4037230/global-hotkey-with-x11-xlib
  if (eventType != "xcb_generic_event_t") 
    return false;
  auto const ev = reinterpret_cast<xcb_generic_event_t *>(message);
  if (!ev || (ev->response_type & 127) != XCB_KEY_PRESS) {
    return false;
  }
  auto const kev = reinterpret_cast<xcb_key_press_event_t *>(ev);
  // unsigned char keycode = kev->detail;
  KeyName keyName {{{0,0}}};
  // Mod1Mask == Alt, Mod4Mask == Meta, Mod2Mask == Num Lock, Mod3Mask = Scroll Lock
  if (kev->state & XCB_MOD_MASK_1)
    keyName.data.nativeMods |= Mod1Mask;
  // if (kev->state & XCB_MOD_MASK_2)
  //   keyName.nativeMods |= Mod2Mask;
  // if (kev->state & XCB_MOD_MASK_3)
  //   keyName.nativeMods |= Mod3Mask;
  if (kev->state & XCB_MOD_MASK_4)
    keyName.data.nativeMods |= Mod4Mask;
  
  if (kev->state & XCB_MOD_MASK_CONTROL)
    keyName.data.nativeMods |= ControlMask;
  if (kev->state & XCB_MOD_MASK_SHIFT)
    keyName.data.nativeMods |= ShiftMask;
  
  keyName.data.nativeKey = kev->detail;
  auto& keyString = GetCallbackInfo(keyName);
#endif
  auto iter = FindShortcutData(keyString);
  if (!iter) return false;

  if (auto iterData = iter->find(u8"Command"); iterData != iter->endO()) {
    auto& data = iterData->second;
    auto& arguments = data[u8"Arguments"].getArray();

    auto& directory = data[u8"Directory"].getValueString();
    QStringList arglist;
    for (auto& i: arguments) {
      auto& str = i.getValueString();
      arglist.push_back(QString::fromUtf8(str.data(), str.size()));
    }
    QString exe = arglist.front();
    arglist.pop_front();
    QProcess:: startDetached(exe, arglist, QString::fromUtf8(directory.data(), directory.size()));
  } else if (iterData = iter->find(u8"Plugin"); iterData != iter->endO()) {
    auto& data = iterData->second;
    auto& pluginName = data[u8"PluginName"].getValueString();
    auto& function = data[u8"Function"].getValueString();
    if (auto iterPlugin = mgr->m_Plugins.find(pluginName); iterPlugin != mgr->m_Plugins.end()) {
      auto& funcMap = iterPlugin->second.plugin->m_PluginMethod;
      auto iterFunc = funcMap.find(function);
      if (iterFunc != funcMap.end()) {
        iterFunc->second.function(PluginEvent::HotKey, nullptr);
      }
    }
    // SendBroadcast(PluginEvent::HotKey, &name);
  }
  return false;

}

#ifdef __linux__
static Display* GetX11Display() {
  auto x11App = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
  if (!x11App) return nullptr;
  return x11App->display();
}
static wl_display* GetWaylandDisplay() {
  // auto wayland = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
  // if (!wayland) return nullptr;
  // return wayland->display();
  return nullptr;
}
#endif

Shortcut::Shortcut(YJson& data)
  : m_Data(data)
#ifdef __linux__
  , m_Display(GetX11Display())
  , m_WlDisplay(GetWaylandDisplay())
  , m_GrabWindow(m_Display ? DefaultRootWindow(m_Display) : 0)
#endif
{
#ifdef __linux__
  // 巨坑至关重要！
  if (m_Display) {
    XSelectInput(m_Display, m_GrabWindow, KeyPressMask);
  } else if (m_WlDisplay) {
  
  }
#endif
  qApp->installNativeEventFilter(this);
}

Shortcut::~Shortcut() {
}

const std::u8string_view Shortcut::GetCallbackInfo(int id)
{
  if (auto iter = m_HotKeyNames.find(id); iter != m_HotKeyNames.end()) {
    return iter->second;
  }
  return u8""sv;
}

const std::u8string_view Shortcut::GetCallbackInfo(KeyName keyName)
{
  if (auto i = m_HotKeyIds.find(keyName); i != m_HotKeyIds.end()) {
    if (auto j = m_HotKeyNames.find(i->second); j != m_HotKeyNames.end()) {
      return j->second;
    }
  }
  return u8""sv;
}

Shortcut::KeyName Shortcut::GetKeyName(const QKeySequence& shortcut) {
  const uint32_t nativeKey =
      GetNativeKeycode(shortcut.isEmpty() ? Qt::Key(0) : shortcut[0].key());
  const uint32_t nativeMods =
      GetNativeModifiers(shortcut.isEmpty() ? Qt::KeyboardModifiers(0) : shortcut[0].keyboardModifiers());
  return KeyName { {{
    .nativeKey = nativeKey,
    .nativeMods = nativeMods
  }} };
}

bool Shortcut::RegisterPlugin(std::u8string_view pluginName)
{
  // enabled = RegisterHotKey(infomation[u8"KeySequence"].getValueString());
  auto iter = FindPluginData(pluginName);
  if (!iter) return false;

  auto& infomation = *iter;
  auto& enabled = infomation[u8"Enabled"];
  if (!enabled.isTrue()) return true;

  enabled = this->RegisterHotKey(infomation[u8"KeySequence"].getValueString());
  return enabled.isTrue();
}

bool Shortcut::UnregisterPlugin(std::u8string_view pluginName)
{
  auto iter = FindPluginData(pluginName);
  if (!iter) return false;

  auto& infomation = *iter;
  auto& enabled = infomation[u8"Enabled"];
  if (!enabled.isTrue()) return true;
  return this->UnregisterHotKey(infomation[u8"KeySequence"].getValueString());
}

bool Shortcut::RegisterHotKey(const std::u8string& keyString) 
{
  auto shortcuts = PluginObject::Utf82QString(keyString);
  KeyName keyName = GetKeyName(shortcuts);
  if (IsKeyRegistered(keyName))
    return false;
  const auto id = GetHotKeyId();
#ifdef _WIN32
  if (!::RegisterHotKey(NULL, id,
      keyName.data.nativeMods, keyName.data.nativeKey))
    return false;
#else
  if (!m_Display || !m_GrabWindow) return false;
  auto ret = ::XGrabKey(m_Display, keyName.data.nativeKey, keyName.data.nativeMods, m_GrabWindow, False, GrabModeAsync, GrabModeAsync);
  if (ret != 1) return false;
  ret = ::XGrabKey(m_Display, keyName.data.nativeKey, keyName.data.nativeMods | Mod2Mask, m_GrabWindow, False, GrabModeAsync, GrabModeAsync);
  if (ret != 1) {
    ::XUngrabKey(m_Display, keyName.data.nativeKey, keyName.data.nativeMods, m_GrabWindow);
    return false;
  }
#endif
  m_HotKeyIds[keyName] = id;
  m_HotKeyNames[id] = keyString;
  return true;
}

bool Shortcut::UnregisterHotKey(const std::u8string& keyString) {
  const KeyName keyName = GetKeyName(PluginObject::Utf82QString(keyString));

  auto iter = m_HotKeyIds.find(keyName);
  if (iter != m_HotKeyIds.end()) {
#ifdef __linux__
    if (!m_Display) return false;
    auto ret1 = ::XUngrabKey(m_Display, keyName.data.nativeKey, keyName.data.nativeMods, m_GrabWindow);
    auto ret2 = ::XUngrabKey(m_Display, keyName.data.nativeKey, keyName.data.nativeMods | Mod2Mask, m_GrabWindow);
    if (ret1 != 1 || ret2 != 1) return false;
#else
    auto const ret = ::UnregisterHotKey(NULL, iter->second);
    if (ret == 0) return false;
#endif
    m_HotKeyNames.erase(iter->second);
    m_HotKeyIds.erase(iter);
    return true;
  }
  return false;
}


YJson* Shortcut::FindPluginData(std::u8string_view pluginName) {
  for (auto& item: m_Data.getArray()) {
    auto iter = item.find(u8"Plugin");
    if (iter == item.endO()) {
      continue;
    }
    iter = iter->second.find(u8"PluginName");
    if (iter->second.getValueString() != pluginName) {
      continue;
    }
    return &item;
  }
  return nullptr;
}

YJson* Shortcut::FindShortcutData(std::u8string_view keyString)
{
  auto iter = std::find_if(m_Data.beginA(), m_Data.endA(), [&keyString](const YJson& info){
    return info[u8"KeySequence"].getValueString() == keyString;
  });
  if (iter == m_Data.endA()) return nullptr;

  return &*iter;
}

bool Shortcut::IsKeyRegistered(const KeyName& keyName) {
  return m_HotKeyIds.find(keyName) != m_HotKeyIds.end();
}

uint32_t Shortcut::GetNativeModifiers(Qt::KeyboardModifiers modifiers) {
  uint32_t native = 0;
#ifdef _WIN32
  if (modifiers & Qt::ShiftModifier)
    native |= MOD_SHIFT;
  if (modifiers & Qt::ControlModifier)
    native |= MOD_CONTROL;
  if (modifiers & Qt::AltModifier)
    native |= MOD_ALT;
  if (modifiers & Qt::MetaModifier)
    native |= MOD_WIN;
#else
  if (modifiers & Qt::ShiftModifier)
    native |= ShiftMask;
  if (modifiers & Qt::ControlModifier)
    native |= ControlMask;
  if (modifiers & Qt::AltModifier)
    native |= Mod1Mask;
  if (modifiers & Qt::MetaModifier)
    native |= Mod4Mask;
#endif
  return native;
}

int Shortcut::GetHotKeyId() const
{
  int lastId = 0;
  for (const auto& [id, name]: m_HotKeyNames) {
    if (id != ++lastId) return lastId;
  }
  return ++lastId;
}

uint32_t Shortcut::GetNativeKeycode(Qt::Key key) {
#ifdef __linux__
  // https://doc.qt.io/qt-6/extras-changes-qt6.html
  auto const keySequence = QKeySequence(key).toString().toLatin1();
  KeySym keysym = XStringToKeysym(keySequence.data());
  if (keysym == NoSymbol)
      keysym = static_cast<ushort>(key);

  // auto const x11Application = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
  // return XKeysymToKeycode(x11Application->display(), keysym);
  return ::XKeysymToKeycode(m_Display, keysym);
#else
  switch (key) {
    case Qt::Key_Escape:
      return VK_ESCAPE;
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
      return VK_TAB;
    case Qt::Key_Backspace:
      return VK_BACK;
    case Qt::Key_Return:
    case Qt::Key_Enter:
      return VK_RETURN;
    case Qt::Key_Insert:
      return VK_INSERT;
    case Qt::Key_Delete:
      return VK_DELETE;
    case Qt::Key_Pause:
      return VK_PAUSE;
    case Qt::Key_Print:
      return VK_PRINT;
    case Qt::Key_Clear:
      return VK_CLEAR;
    case Qt::Key_Home:
      return VK_HOME;
    case Qt::Key_End:
      return VK_END;
    case Qt::Key_Left:
      return VK_LEFT;
    case Qt::Key_Up:
      return VK_UP;
    case Qt::Key_Right:
      return VK_RIGHT;
    case Qt::Key_Down:
      return VK_DOWN;
    case Qt::Key_PageUp:
      return VK_PRIOR;
    case Qt::Key_PageDown:
      return VK_NEXT;
    case Qt::Key_F1:
      return VK_F1;
    case Qt::Key_F2:
      return VK_F2;
    case Qt::Key_F3:
      return VK_F3;
    case Qt::Key_F4:
      return VK_F4;
    case Qt::Key_F5:
      return VK_F5;
    case Qt::Key_F6:
      return VK_F6;
    case Qt::Key_F7:
      return VK_F7;
    case Qt::Key_F8:
      return VK_F8;
    case Qt::Key_F9:
      return VK_F9;
    case Qt::Key_F10:
      return VK_F10;
    case Qt::Key_F11:
      return VK_F11;
    case Qt::Key_F12:
      return VK_F12;
    case Qt::Key_F13:
      return VK_F13;
    case Qt::Key_F14:
      return VK_F14;
    case Qt::Key_F15:
      return VK_F15;
    case Qt::Key_F16:
      return VK_F16;
    case Qt::Key_F17:
      return VK_F17;
    case Qt::Key_F18:
      return VK_F18;
    case Qt::Key_F19:
      return VK_F19;
    case Qt::Key_F20:
      return VK_F20;
    case Qt::Key_F21:
      return VK_F21;
    case Qt::Key_F22:
      return VK_F22;
    case Qt::Key_F23:
      return VK_F23;
    case Qt::Key_F24:
      return VK_F24;
    case Qt::Key_Space:
      return VK_SPACE;
    case Qt::Key_Asterisk:
      return VK_MULTIPLY;
    case Qt::Key_Plus:
      return VK_ADD;
    case Qt::Key_Comma:
      return VK_SEPARATOR;
    case Qt::Key_Minus:
      return VK_SUBTRACT;
    case Qt::Key_Slash:
      return VK_DIVIDE;
    case Qt::Key_MediaNext:
      return VK_MEDIA_NEXT_TRACK;
    case Qt::Key_MediaPrevious:
      return VK_MEDIA_PREV_TRACK;
    case Qt::Key_MediaPlay:
      return VK_MEDIA_PLAY_PAUSE;
    case Qt::Key_MediaStop:
      return VK_MEDIA_STOP;
      // couldn't find those in VK_*
      // case Qt::Key_MediaLast:
      // case Qt::Key_MediaRecord:
    case Qt::Key_VolumeDown:
      return VK_VOLUME_DOWN;
    case Qt::Key_VolumeUp:
      return VK_VOLUME_UP;
    case Qt::Key_VolumeMute:
      return VK_VOLUME_MUTE;

      // numbers
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
      return key;

      // letters
    case Qt::Key_A:
    case Qt::Key_B:
    case Qt::Key_C:
    case Qt::Key_D:
    case Qt::Key_E:
    case Qt::Key_F:
    case Qt::Key_G:
    case Qt::Key_H:
    case Qt::Key_I:
    case Qt::Key_J:
    case Qt::Key_K:
    case Qt::Key_L:
    case Qt::Key_M:
    case Qt::Key_N:
    case Qt::Key_O:
    case Qt::Key_P:
    case Qt::Key_Q:
    case Qt::Key_R:
    case Qt::Key_S:
    case Qt::Key_T:
    case Qt::Key_U:
    case Qt::Key_V:
    case Qt::Key_W:
    case Qt::Key_X:
    case Qt::Key_Y:
    case Qt::Key_Z:
      return key;

    default:
      return 0;
  }
#endif
}
