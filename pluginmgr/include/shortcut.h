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
protected:
  bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override;

public:
  explicit Shortcut(class YJson& m_Data);
  ~Shortcut();
  bool RegisterHotKey(const std::u8string& keyString);
  bool UnregisterHotKey(const std::u8string& keyString);
  const std::u8string_view GetCallbackInfo(int id);

private:
  bool IsKeyRegistered(QString shortcut);
  bool IsKeyRegistered(const KeyName& keyName);
  static KeyName GetKeyName(const QKeySequence& shortcut);
  static uint32_t GetNativeModifiers(Qt::KeyboardModifiers modifiers);
  static uint32_t GetNativeKeycode(Qt::Key key);
  int GetHotKeyId() const;
private:
  class YJson& m_Data;
  std::map<KeyName, int> m_HotKeyIds;
  std::map<int, std::u8string> m_HotKeyNames;
};

#endif
