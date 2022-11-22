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

  static bool CreateSharedMemory();
  static void WriteSharedFlag(int flag);
  static int ReadSharedFlag();
  static void DetachSharedMemory();

 private:
  void LoadFonts() const;
  void LoadSkins();
  void MakeDirs();
  void CopyFiles() const;
  void InitSettings();
  void CompareJson(class YJson& jsDefault, class YJson& jsUser);
  static std::unique_ptr<YJson> GetMenuJson();

  static class QSharedMemory* m_SharedMemory;
  class MsgDlg* m_MsgDlg;
  class YJson* m_Settings;
};

#endif
