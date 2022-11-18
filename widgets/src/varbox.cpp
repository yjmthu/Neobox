#include <speedbox.h>
#include <varbox.h>
#include <msgdlg.h>
#include <yjson.h>

#include <QApplication>
#include <QDir>
#include <QFontDatabase>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTimer>
#include <QSharedMemory>
#include <QProcess>

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

extern std::unique_ptr<YJson> GetMenuJson();

QSharedMemory* VarBox::m_SharedMemory = nullptr;

void VarBox::WriteSharedFlag(int flag)
{
    m_SharedMemory->lock();
    *reinterpret_cast<int *>(m_SharedMemory->data()) = flag;
    m_SharedMemory->unlock();
}

int VarBox::ReadSharedFlag()
{
    m_SharedMemory->lock();
    const auto state = *reinterpret_cast<const int*>(m_SharedMemory->constData());
    m_SharedMemory->unlock();
    return state;
}

VarBox::VarBox():
  m_Skins({
    {u8"经典火绒", u8":/styles/Huorong.ui"},
    {u8"电脑管家", u8":/styles/Guanjia.ui"},
    {u8"数字卫士", u8":/styles/360.ui"},
    {u8"独霸一方", u8":/styles/duba.ui"}
  }),
  m_MsgDlg(new MsgDlg)
{
  MakeDirs();
  CopyFiles();
  LoadFonts();
  InitSettings();
  LoadSkins();
}

VarBox::~VarBox() {
  delete GetSpeedBox();
  delete m_MsgDlg;
  delete m_Settings;
}

void VarBox::CompareJson(YJson& jsDefault, YJson& jsUser)
{
  if (!jsDefault.isSameType(&jsUser))
    return;
  if (!jsUser.isObject()) {
    YJson::swap(jsDefault, jsUser);
    return;
  }
  for (auto& [key, val]: jsUser.getObject()) {
    auto iter = jsDefault.find(key);
    if (iter != jsDefault.endO()) {
      CompareJson(iter->second, val);
    } else {
      YJson::swap(jsDefault[key], val);
    }
  }
}

void VarBox::InitSettings()
{
  QFile fJson(QStringLiteral(":/jsons/setting.json"));
  fJson.open(QIODevice::ReadOnly);
  QByteArray array = fJson.readAll();
  fJson.close();

  m_Settings = new YJson(array.begin(), array.end());

  static const char szFileName[] = "Settings.json";
  if (QFile::exists(szFileName)) {
    YJson jsUser(szFileName, YJson::UTF8);
    CompareJson(*m_Settings, jsUser);
  }
  m_Settings->toFile(szFileName);
}

YJson& VarBox::GetSettings(const char8_t* key) {
  return key ? GetInstance()->m_Settings->find(key)->second : *GetInstance()->m_Settings;
}

SpeedBox* VarBox::GetSpeedBox() {
  static SpeedBox* box = new SpeedBox;
  return box;
}

VarBox* VarBox::GetInstance()
{
  static std::unique_ptr<VarBox> self(new VarBox);
  return self.get();
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

std::unique_ptr<YJson> VarBox::LoadJsons() {
  auto data = GetMenuJson();
  auto& appSettings = (*data)[u8"设置中心"][u8"children"][u8"软件设置"][u8"children"];
  auto* item = &appSettings[u8"皮肤选择"][u8"children"].getObject();
  const auto& u8SkinName = m_Settings->find(u8"FormGlobal")->second[u8"CurSkin"].getValueString();
  for (int index = 0; const auto& [i, j]: m_Skins) {
    if (!QFile::exists(QString::fromUtf8(j.data(), j.size())))
      continue;
    if (++index == 5) {
      item->emplace_back(u8"Separator",
        YJson { YJson::O {
          {u8"type", u8"Separator"},
      }});
    }
    item->emplace_back(i,
      YJson { YJson::O {
        {u8"type", u8"Checkable"},
        {u8"checked", i == u8SkinName},
        {u8"string", i},
        {u8"function", u8"AppSelectSkin"},
        {u8"tip", j},
    }});
  }
  item = &appSettings[u8"皮肤删除"][u8"children"].getObject();
  for (size_t index = 4; index < m_Skins.size(); ++index)
  {
    item->emplace_back(m_Skins[index].name,
      YJson {
        YJson::O {
          {u8"type", u8"StringItem"},
          {u8"function", u8"AppRemoveSkin"},
          {u8"string", m_Skins[index].name},
          {u8"tip", m_Skins[index].path},
        }
      }
    );
  }
  return data;
}

void VarBox::MakeDirs() {
  QDir dir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
  constexpr char relPath[] = ".config/Neobox";
  if (dir.exists(relPath) || dir.mkpath(relPath)) {
    dir.cd(relPath);
    QDir::setCurrent(dir.absolutePath());
  } else {
    VarBox::WriteSharedFlag(1);
    QProcess::startDetached(QApplication::applicationFilePath(), QStringList {});
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

void VarBox::LoadSkins()
{
  // There should be 'm_Settings', not the 'GetInstance()'.
  auto& obj = m_Settings->find(u8"FormGlobal")->second.find(u8"UserSkins")->second.getObject();
  for (const auto& [key, value]: obj) {
    m_Skins.emplace_back(key, value.getValueString());
  }
}

void VarBox::ShowMsg(const QString &text)
{
  GetInstance()->m_MsgDlg->ShowMessage(text);
}

