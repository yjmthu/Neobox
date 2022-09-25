#ifndef NEOMENU_H
#define NEOMENU_H

#include <QMenu>

#include <functional>

class NeoMenu : public QMenu {
 protected:
  using FuncChecked = std::function<void(bool)>;
  using FuncCheck = std::function<bool()>;
  using FuncItemChecked = std::function<void(int)>;
  using FuncItemCheck = std::function<int()>;

 private:
  class Shortcut* m_Shortcut;
  std::map<std::u8string, std::function<void(void)>> m_FuncNormalMap;
  std::map<std::u8string, std::pair<FuncChecked, FuncCheck>> m_FuncCheckMap;
  std::map<std::u8string, std::pair<FuncItemChecked, FuncItemCheck>>
      m_FuncItemCheckMap;
  std::map<std::u8string, QMenu*> m_ExMenus;

  std::map<std::u8string, QActionGroup*> m_ExclusiveGroups;
  std::map<std::u8string, QAction*> m_CheckableActions;

  friend class SpeedBox;
  class TranslateDlg* m_TranslateDlg;

  void InitFunctionMap();
  void GetMenuContent(QMenu* parent, class YJson const& data);
  bool ChooseFolder(QString title, QString& folder);
  void LoadWallpaperExmenu();
  void WallhavenParams(QAction* action);
  void ScriptApiParams(QAction* action);

 public:
  explicit NeoMenu(QWidget* parent = nullptr);
  ~NeoMenu();
  class Wallpaper* const m_Wallpaper;
  inline void SetFormColorEffect() {
    const auto& [fSetEffetc, fGetEffect] =
        m_FuncItemCheckMap[u8"AppFormEffect"];
    fSetEffetc(fGetEffect());
  }
};

#endif
