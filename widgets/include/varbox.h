#ifndef VARBOX_H
#define VARBOX_H

class VarBox {
 public:
  explicit VarBox();
  ~VarBox();
  static class YJson& GetSettings(const char8_t* key);
  static void WriteSettings();

 private:
  void LoadFonts() const;
  void LoadJsons();
  void MakeDirs();
};

#endif
