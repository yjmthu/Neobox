#include <varbox.h>
#include <yjson.h>

#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QMessageBox>
#include <QStandardPaths>

#include <memory>

void ShowMessage(const std::u8string& title,
                 const std::u8string& text,
                 int type = 0) {
  QMessageBox::information(nullptr,
                           QString::fromUtf8(title.data(), title.size()),
                           QString::fromUtf8(text.data(), text.size()));
}

VarBox::VarBox() {
  MakeDirs();
  LoadFonts();
  LoadJsons();
}

VarBox::~VarBox() {}

YJson& VarBox::GetSettings(const char8_t* key) {
  const char szFileName[] = "Settings.json";
  static std::unique_ptr<YJson> m_Settings;
  if (!m_Settings) {
    if (QFile::exists(szFileName)) {
      m_Settings = std::make_unique<YJson>(szFileName, YJson::UTF8);
      m_Settings->toFile(szFileName);
    } else {
      QFile fJson(QStringLiteral(":/jsons/setting.json"));
      fJson.open(QIODevice::ReadOnly);
      QByteArray array = fJson.readAll();
      m_Settings = std::make_unique<YJson>(array.begin(), array.end());
      fJson.close();
    }
  }
  return key ? m_Settings->find(key)->second : *m_Settings;
}

void VarBox::WriteSettings() {
  VarBox::GetSettings(nullptr).toFile("Settings.json");
}

void VarBox::LoadFonts() const {
  QFontDatabase::addApplicationFont(
      QStringLiteral(":/fonts/Nickainley-Normal-small.ttf"));
  QFontDatabase::addApplicationFont(
      QStringLiteral(":/fonts/Carattere-Regular-small.ttf"));
}

void VarBox::LoadJsons() {}

void VarBox::MakeDirs() {
  QDir dir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
  constexpr char relPath[] = ".config/Neobox";
  if (dir.exists(relPath) || dir.mkpath(relPath)) {
    dir.cd(relPath);
    QDir::setCurrent(dir.absolutePath());
  } else {
    qApp->quit();
    return;
  }
  auto lst = {"junck", "qmls"};
  for (auto i : lst) {
    if (!dir.exists(i))
      dir.mkdir(i);
  }
}
