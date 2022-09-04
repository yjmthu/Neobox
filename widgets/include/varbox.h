#ifndef VARBOX_H
#define VARBOX_H

class VarBox {
 public:
  explicit VarBox();
  ~VarBox();
  static class YJson& GetSettings(const char8_t* key);
  static class SpeedBox* GetSpeedBox();
  static void WriteSettings();

 private:
  void LoadFonts() const;
  void LoadJsons();
  void MakeDirs();
  void CopyFiles() const;
};

#endif
