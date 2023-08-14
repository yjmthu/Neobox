#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QKeySequence>
#include <QAbstractNativeEventFilter>
#include <map>

class Shortcut: public QAbstractNativeEventFilter
{
private:

  struct KeyName {
    union {
      struct KeyData {
        uint32_t nativeKey;
        uint32_t nativeMods;
      } data;
      uint64_t big;
    };
    bool operator<(const KeyName& other) const {
      return big < other.big;
    }
    bool operator==(const KeyName& other) const {
      return big == other.big;
    }
  };
protected:
  bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override;

public:
  explicit Shortcut(class YJson& m_Data);
  ~Shortcut();
  bool RegisterPlugin(std::u8string_view pluginName);
  bool UnregisterPlugin(std::u8string_view pluginName);
  bool RegisterHotKey(const std::u8string& keyString);
  bool UnregisterHotKey(const std::u8string& keyString);
  YJson* FindPluginData(std::u8string_view pluginName);
  YJson* FindShortcutData(std::u8string_view keyString);
  const std::u8string_view GetCallbackInfo(int id);
  const std::u8string_view GetCallbackInfo(KeyName keyName);

private:
  bool IsKeyRegistered(QString shortcut);
  bool IsKeyRegistered(const KeyName& keyName);
  KeyName GetKeyName(const QKeySequence& shortcut);
  static uint32_t GetNativeModifiers(Qt::KeyboardModifiers modifiers);
  uint32_t GetNativeKeycode(Qt::Key key);
  int GetHotKeyId() const;
private:
  class YJson& m_Data;
#ifdef __linux__
  struct _XDisplay* const m_Display;
  struct wl_display* const m_WlDisplay;
  const unsigned long m_GrabWindow;
#endif
  std::map<KeyName, int> m_HotKeyIds;
  std::map<int, std::u8string> m_HotKeyNames;
};

#endif
