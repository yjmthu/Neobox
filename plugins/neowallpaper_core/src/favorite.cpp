#include <favorie.h>

Favorite::Favorite(YJson& setting):
  WallBase(InitSetting(setting)),
  m_Data(nullptr)
{
  CheckData();
}

Favorite::~Favorite()
{
  delete m_Data;
}

YJson& Favorite::InitSetting(YJson& setting)
{
  if (setting.isObject())
    return setting;
  setting = YJson::O {
    { u8"Directory", GetStantardDir(u8"收藏壁纸") },
  };
  SaveSetting();
  return setting;
}

bool Favorite::CheckData()
{
  if (m_Data) return true;
  m_Data = new YJson(
    YJson::O{
      {u8"Unused", YJson::Array}, 
      {u8"Used", YJson::Array},
    });
  m_Data->toFile(m_DataPath);
  return true;
}

ImageInfoEx Favorite::GetNext()
{
  ImageInfoEx ptr(new ImageInfo);

  if (!CheckData()) {
    ptr->ErrorCode = ImageInfo::Errors::DataErr;
    ptr->ErrorMsg = u8"favorite data file can not be created!";
    return ptr;
  }

  auto& arrayA = m_Data->find(u8"Unused")->second.getArray();
  auto& arrayB = m_Data->find(u8"Used")->second.getArray();
  if (arrayA.empty() && arrayB.empty()) {
    ptr->ErrorMsg = u8"Empty data content.";
    ptr->ErrorCode = ImageInfo::DataErr;
    return ptr;
  }
  if (arrayA.empty()) {
    std::vector<std::u8string> temp;
    for (auto& i : arrayB) {
      temp.emplace_back(std::move(i.getValueString()));
    }
    std::mt19937 g(std::random_device{}());
    std::shuffle(temp.begin(), temp.end(), g);
    arrayB.clear();
    for (auto& i : temp) {
      arrayA.emplace_back(std::move(i));
    }
  }

  ptr->ImagePath = std::move(arrayA.back().getValueString());
  arrayA.pop_back();
  arrayB.push_back(ptr->ImagePath);
  m_Data->toFile(m_DataPath);
  ptr->ErrorCode = ImageInfo::NoErr;

  fs::path pImgPath = ptr->ImagePath;
  if (fs::exists(pImgPath) && pImgPath.has_filename()) {
    pImgPath = GetImageDir() / pImgPath.filename();
    if (!fs::exists(pImgPath)) {
      fs::copy(ptr->ImagePath, pImgPath);
    }
  }
  return ptr;
}

void Favorite::Dislike(const std::u8string& sImgPath)
{
  auto ptr = &m_Data->find(u8"Used")->second;
  auto iter = ptr->findByValA(sImgPath);
  if (iter != ptr->endA())
    ptr->remove(iter);
  ptr = &m_Data->find(u8"Unused")->second;
  iter = ptr->findByValA(sImgPath);
  if (iter != ptr->endA())
    ptr->remove(iter);
  fs::path pImgPath = sImgPath;
  if (pImgPath.has_filename()) {
    pImgPath = GetImageDir() / pImgPath.filename();
    if (fs::exists(pImgPath)) {
      fs::remove(pImgPath);
    }
  }
  m_Data->toFile(m_DataPath);
}

void Favorite::UndoDislike(const std::u8string& sImgPath)
{
  auto& used = m_Data->find(u8"Used")->second;
  auto& unused = m_Data->find(u8"Unused")->second;

  if (used.findByValA(sImgPath) == used.endA() &&
      unused.findByValA(sImgPath) == unused.endA())
  {
    used.append(sImgPath);
    m_Data->toFile(m_DataPath);
  }

  fs::path pImgPath = sImgPath;
  if (fs::exists(pImgPath) && pImgPath.has_filename()) {
    pImgPath = GetImageDir() / pImgPath.filename();
    if (!fs::exists(pImgPath)) {
      fs::copy(sImgPath, pImgPath);
    }
  }
}

fs::path Favorite::GetImageDir() const
{
  return m_Setting[u8"Directory"].getValueString();
}

void Favorite::SetJson(bool)
{
  SaveSetting();
}

void Favorite::SetCurDir(const std::u8string& str)
{
  m_Setting[u8"Directory"] = str;
  SaveSetting();
}
