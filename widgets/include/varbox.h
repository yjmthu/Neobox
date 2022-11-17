#ifndef VARBOX_H
#define VARBOX_H

#include <string>
#include <vector>
#include <memory>

class VarBox {
 public:
  explicit VarBox();
  ~VarBox();
  static class YJson& GetSettings(const char8_t* key);
  static class SpeedBox* GetSpeedBox();
  static void WriteSettings();
  static VarBox* GetInstance();
  static void ShowMsg(const class QString& text);

  struct Skin { std::u8string name; std::u8string path; };
  std::vector<Skin> m_Skins;

  std::unique_ptr<YJson> LoadJsons();

 private:
  void LoadFonts() const;
  void LoadSkins();
  void MakeDirs();
  void CopyFiles() const;

  class MsgDlg* m_MsgDlg;
};

#endif
