#include <history.h>
#include <fstream>
#include <systemapi.h>
#ifdef _WIN32
#include <Windows.h>
#endif

WallpaperHistory::WallpaperHistory()
  : WallpaperHistoryBase()
{
  ReadSettings();
}

WallpaperHistory::~WallpaperHistory()
{
  WriteSettings();
}


#ifdef _WIN32
void WallpaperHistory::UpdateRegString()
{
  fs::path curWallpaper = RegReadString(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"WallPaper");
  if (!curWallpaper.empty() && fs::exists(curWallpaper)) {
    auto curImage = GetCurrent();
    if (!curImage || *curImage != curWallpaper) {
      PushBack(std::move(curWallpaper));
    }
  }
}
#else
void WallpaperHistory::UpdateRegString()
{
  //
}
#endif


void WallpaperHistory::ReadSettings()
{
  std::ifstream file("wallpaperData/History.txt", std::ios::in);
  if (file.is_open()) {
    std::string temp;
    if (std::getline(file, temp)) {
      while (std::getline(file, temp)) {
        emplace_front(temp);
        front().make_preferred();
      }
    }
    file.close();
  }
}

void WallpaperHistory::WriteSettings() {
  int m_CountLimit = 100;
  std::ofstream file("wallpaperData/History.txt", std::ios::out);
  if (!file.is_open())
    return;

  for (auto i = rbegin(); i != rend(); ++i) {
    file << i->string() << std::endl;
    if (!--m_CountLimit)
      break;
  }
  file.close();
}
