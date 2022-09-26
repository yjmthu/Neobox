#include <speedbox.h>
#include <varbox.h>
#include <yjson.h>

#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTimer>

#include <memory>
#include <ranges>

void ShowMessage(const std::u8string& title,
                 const std::u8string& text,
                 int type = 0) {
  QMetaObject::invokeMethod(VarBox::GetSpeedBox(), [=](){
    QMessageBox::information(VarBox::GetSpeedBox(),
                             QString::fromUtf8(title.data(), title.size()),
                             QString::fromUtf8(text.data(), text.size()));
  });
}

VarBox::VarBox() {
  MakeDirs();
  CopyFiles();
  LoadFonts();
  LoadJsons();
}

VarBox::~VarBox() {}

YJson& VarBox::GetSettings(const char8_t* key) {
  static const char szFileName[] = "Settings.json";
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

SpeedBox* VarBox::GetSpeedBox() {
  static SpeedBox* box = new SpeedBox;
  return box;
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
  auto lst = {"junk", "qmls", "resource", "tessdata"};
  for (auto i : lst) {
    if (!dir.exists(i))
      dir.mkdir(i);
  }
}

void VarBox::CopyFiles() const {
  namespace fs = std::filesystem;
  QFile jsFile(QStringLiteral(":/jsons/resources.json"));
  if (!jsFile.open(QIODevice::ReadOnly)) {
    QMessageBox::critical(nullptr, "错误", "不能读取资源文件");
    qApp->quit();
    return;
  }

  QByteArray data = jsFile.readAll();
  YJson jsResource(data.begin(), data.end());
  jsFile.close();

  for (const auto& [i, j] : jsResource.getObject()) {
    if (!fs::exists(i))
      fs::create_directory(i);
    auto&& qFiles = j.getArray() | std::views::transform([](const YJson& k) {
                      auto str = k.getValueString();
                      return QString::fromUtf8(str.data(), str.size());
                    });
    for (auto&& k : qFiles) {
      if (!QFile::exists(k)) {
        QFile::copy(":/" + k, k);
        QFile::setPermissions(k, QFile::ReadUser | QFile::WriteUser);
      }
    }
  }
}
