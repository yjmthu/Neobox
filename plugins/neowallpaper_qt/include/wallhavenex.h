#ifndef WALLHAVENEX_H
#define WALLHAVENEX_H

#include <functional>
#include <menubase.hpp>

namespace std {
  namespace filesystem {
    class path;
  }
}

class WallhavenExMenu: public MenuBase
{
public:
  explicit WallhavenExMenu(class YJson& data, MenuBase* parent, std::function<void(bool)> callback, std::function<const std::filesystem::path&()> getCurImg);
  virtual ~WallhavenExMenu();
private:
  static std::string GetImageName(const std::filesystem::path& path);
  void LoadWallpaperTypes();
  void LoadSubSettingMenu(QAction* action);
  void LoadMoreActions();
  void AddNewType();
  void EditNewType(const std::u8string& typeName);
  void EditCurType(const std::u8string& typeName);
  const std::function<void(bool)> m_CallBack;
  const std::function<const std::filesystem::path &()> GetCurImage;
  YJson& m_Data;
  class QAction* const m_Separator;
  class QActionGroup* m_ActionGroup;
};

#endif // WALLHAVENEX_H
