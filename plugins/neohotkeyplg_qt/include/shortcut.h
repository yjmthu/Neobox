#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QKeySequence>

#include <map>

class Shortcut {
public:
  enum CallbackType { None, Plugin, Command };
private:

  struct KeyName {
    union {
      struct {
        uint32_t nativeKey;
        uint32_t nativeMods;
      };
      uint64_t big;
    };
    bool operator<(const KeyName& other) const {
      return big < other.big;
    }
  };
  struct CallbackInfo {
    std::u8string name;
    CallbackType type;
  };

public:
  explicit Shortcut(const class YJson& m_Data);
  ~Shortcut();
  void RegisterAllHotKey();
  void UnregisterAllHotKey();
  const CallbackInfo& GetCallbackInfo(int id);

private:
  bool RegisterHotKey(const YJson& plugin);
  bool UnregisterHotKey(QString shortcut);
  bool IsKeyRegistered(QString shortcut);
  bool IsKeyRegistered(const KeyName& keyName);
  static KeyName GetKeyName(const QKeySequence& shortcut);
  static uint32_t GetNativeModifiers(Qt::KeyboardModifiers modifiers);
  static uint32_t GetNativeKeycode(Qt::Key key);
private:
  std::map<KeyName, int> m_HotKeys;
  std::vector<CallbackInfo> m_Plugins;
  const class YJson& m_Data;
};

#endif
