#include <neoupdate.h>

constexpr uint64_t VersionToInt(int a, int b, int c) {
  return (a << 16) | (b << 8) | (c << 0);
}

NeoUpdate::NeoUpdate()
  : m_QtVersion(VersionToInt(5, 15, 5))
  , m_Version(VersionToInt(0, 0, 0))
  , m_DirJson()
  , m_IsX64(sizeof(void*) == 8)
{
  GetDesktop();
  GetPlatform();
}

NeoUpdate::~NeoUpdate()
{
}

void NeoUpdate::GetPlatform() {
#ifdef __linux__
  m_Platform = Platfrom::LINUX;
#elif define (WIN64)
  m_Platform = Platfrom::WINx64;
#elif define (WIN32)
#else
#endif
}

void NeoUpdate::GetDesktop() {
}

