#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QKeySequence>
#include <QObject>

#include <vector>

class Shortcut : public QObject {
 protected:
  union KeyName {
    struct {
      uint32_t nativeKey;
      uint32_t nativeMods;
    };
    uint64_t big;
  };

 public:
  explicit Shortcut(QObject* parent);
  ~Shortcut();
  bool RegistHotKey(QKeySequence shortcut, std::function<void()>);
  inline bool RegistHotKey(QString shortcut, std::function<void()> func) {
    return RegistHotKey(QKeySequence(shortcut), func);
  }
  bool UnregistHotKey(QKeySequence shortcut);
  inline bool UnregistHotKey(QString shortcut) {
    return UnregistHotKey(QKeySequence(shortcut));
  }
  void CallFunction(int id);
  inline bool IsKeyRegisted(QString shortcut) {
    return IsKeyRegisted(GetKeyName(QKeySequence(shortcut)));
  }

 private:
  std::vector<std::tuple<int, const KeyName, std::function<void()>>> m_HotKeys;
  bool IsKeyRegisted(const KeyName& keyName);
  static KeyName GetKeyName(const QKeySequence& shortcut);
  static uint32_t GetNativeModifiers(Qt::KeyboardModifiers modifiers);
  static uint32_t GetNativeKeycode(Qt::Key key);
};

#endif
