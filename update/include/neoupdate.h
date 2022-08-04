#ifndef NEOUPDATE_H
#define NEOUPDATE_H

#include <stdint.h>
#include <string>

class NeoUpdate {
private:
  enum class Platfrom { WIN, LINUX, MACOS } m_Platform;
  enum class Desktop { WIN, KDE, DDE, GNOME, XFCE, OTHER } m_Desktop;
  const uint64_t m_Version;
  const uint64_t m_QtVersion;
  const std::u8string m_DirJson;
  const bool m_IsX64;

  void GetPlatform();
  void GetDesktop();
public:
  NeoUpdate();
  ~NeoUpdate();
};

#endif // NEOUPDATE_H

