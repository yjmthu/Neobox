#ifndef WALLHAVENEX_H
#define WALLHAVENEX_H

#include <functional>
#include <wallbaseex.h>

namespace fs = std::filesystem;

class WallhavenExMenu: public WallBaseEx
{
public:
  explicit WallhavenExMenu(YJson data, MenuBase* parent,
    Callback callback,
    std::function<std::optional<fs::path>()> getCurImg);
  virtual ~WallhavenExMenu();
private:
  static std::string GetImageName(const std::filesystem::path& path);
  void LoadWallpaperTypes();
  void LoadSubSettingMenu(QAction* action);
  void LoadMoreActions();
  void AddNewType();
  void EditNewType(std::u8string typeName);
  void EditCurType(YJson& curJson);
  const std::function<std::optional<std::filesystem::path>()> GetCurImage;
  class QAction* const m_Separator;
  class QActionGroup* m_ActionGroup;
};

#endif // WALLHAVENEX_H
