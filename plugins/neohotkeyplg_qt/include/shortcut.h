#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QKeySequence>

#include <map>

class Shortcut {
public:
  struct KeyName {
    union {
      struct {
        uint32_t nativeKey;
        uint32_t nativeMods;
      };
      uint64_t big;
    };
    // KeyName(uint32_t nativeKey, uint32_t nativeMods):
    // nativeKey {nativeKey}, nativeMods {nativeMods} {
    // }
    bool operator<(const KeyName& other) const {
      return big < other.big;
    }
  };
  struct KeyId {
    int id = -1;
    KeyId& operator++() {
      return ++id, *this;
    }
  };

 public:
  explicit Shortcut();
  ~Shortcut();
  inline bool RegisterHotKey(QString shortcut);
  bool UnregisterHotKey(QString shortcut);
  inline bool IsKeyRegistered(QString shortcut);

 private:
  std::map<KeyName, KeyId> m_HotKeys;
  bool IsKeyRegistered(const KeyName& keyName);
  static KeyName GetKeyName(const QKeySequence& shortcut);
  static uint32_t GetNativeModifiers(Qt::KeyboardModifiers modifiers);
  static uint32_t GetNativeKeycode(Qt::Key key);
};

#endif
