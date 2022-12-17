#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QKeySequence>

#include <map>

class Shortcut {
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
  // struct KeyId {
  //   int id = -1;
  //   KeyId& operator++() {
  //     return ++id, *this;
  //   }
  // } m_StaticId;

public:
  explicit Shortcut(const class YJson& m_Data);
  ~Shortcut();
  void RegisterAllHotKey();
  void UnregisterAllHotKey();
  const std::u8string& GetPluginName(int id);

private:
  bool RegisterHotKey(QString shortcut, std::u8string plugin);
  bool UnregisterHotKey(QString shortcut);
  bool IsKeyRegistered(QString shortcut);
  bool IsKeyRegistered(const KeyName& keyName);
  static KeyName GetKeyName(const QKeySequence& shortcut);
  static uint32_t GetNativeModifiers(Qt::KeyboardModifiers modifiers);
  static uint32_t GetNativeKeycode(Qt::Key key);
private:
  std::map<KeyName, int> m_HotKeys;
  std::vector<std::u8string> m_Plugins;
  const class YJson& m_Data;
};

#endif
