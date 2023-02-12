#include <favorie.h>

#include <set>

Favorite::Favorite(YJson& setting):
  WallBase(InitSetting(setting))
  // , m_Data(nullptr)
{
  // InitData();
}

Favorite::~Favorite()
{
  // delete m_Data;
}

YJson& Favorite::InitSetting(YJson& setting)
{
  bool save = false;
  if (!setting.isObject()) {
    setting = YJson::O {
      { u8"Directory", GetStantardDir(u8"收藏壁纸") },
      { u8"MaxCount", 100 },
      { u8"NameFormat", u8"{0}" },
      { u8"Random", true },
      { u8"Version", 0},
    };
    SaveSetting();
    return setting;
  }
  auto& max = setting[u8"Version"];
  if (!max.isNumber()) {
    max = 0;
    setting[u8"MaxCount"] = 100;
    setting[u8"NameFormat"] = u8"{0}";
    setting[u8"Random"] = true;
    save = true;
  }

  if (save) SaveSetting();
  return setting;
}

size_t Favorite::GetFileCount() const
{
  size_t m_iCount = 0;
  auto const curDir = GetImageDir();
  if (!fs::exists(curDir) || !fs::is_directory(curDir))
    return m_iCount;

  for (auto& iter : fs::directory_iterator(curDir)) {
    if (fs::is_directory(iter.status())) {
    } else if (Wallpaper::IsImageFile(iter.path())) {
      ++m_iCount;
    }
  }
  return m_iCount;
}

// void Favorite::InitData()
// {
//   if (fs::exists(m_DataPath)) {
//     m_Data = new YJson(m_DataPath, YJson::UTF8);
//     if (m_Data->isObject()) {
//       return;
//     }
//     delete m_Data;
//     m_Data = nullptr;
//   }
//   m_Data = new YJson(
//     YJson::O{
//       {u8"Unused", YJson::Array}, 
//       {u8"Used", YJson::Array},
//     });
//   m_Data->toFile(m_DataPath);
// }

bool Favorite::GetFileList()
{
  size_t m_Toltal = GetFileCount(), m_Index = 0;
  if (!m_Toltal)
    return false;
  auto const curDir = GetImageDir();
  auto const maxCount = m_Setting[u8"MaxCount"].getValueInt();
  std::vector<size_t> numbers;
  std::mt19937 g(std::random_device{}());
  if (m_Toltal < maxCount) {
    numbers.resize(m_Toltal);
    std::iota(numbers.begin(), numbers.end(), 0);
  } else {
    std::set<size_t> already;
    auto pf = std::uniform_int_distribution<size_t>(0, m_Toltal - 1);
    for (uint32_t i = 0; i < maxCount; ++i) {
      size_t temp = pf(g);
      while (already.find(temp) != already.end())
        temp = pf(g);
      already.insert(temp);
    }
    numbers.assign(already.begin(), already.end());
  }

  for (auto target = numbers.cbegin(); auto& iter : fs::directory_iterator(curDir)) {
    fs::path path = iter.path();
    if (fs::is_directory(iter.status())) {
    } else if (Wallpaper::IsImageFile(path)) {
      if (*target == m_Index) {
        m_FileList.emplace_back(path.u8string());
        ++target;
      }
      ++m_Index;
    }
  }

  if (m_Setting[u8"Random"].isTrue()) {
    std::shuffle(m_FileList.begin(), m_FileList.end(), g);
  }
  return true;
}

ImageInfoEx Favorite::GetNext()
{
  ImageInfoEx ptr(new ImageInfo);

#if 0
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
#endif

  while (!m_FileList.empty() && !fs::exists(m_FileList.back())) {
    m_FileList.pop_back();
  }

  // if (m_FileList.empty() && !GetFileList()) {
  //   ptr->ErrorMsg = u8"Empty folder with no wallpaper in it.";
  //   ptr->ErrorCode = ImageInfo::FileErr;
  //   return ptr;
  // }

  while (!m_FileList.empty()) {
    if (!fs::exists(m_FileList.back()))
      m_FileList.pop_back();
    else
      break;
  }

  if (m_FileList.empty() && !GetFileList()) {
    ptr->ErrorMsg = u8"Empty folder with no wallpaper in it.";
    ptr->ErrorCode = ImageInfo::FileErr;
    return ptr;
  }

  ptr->ImagePath = std::move(m_FileList.back());
  m_FileList.pop_back();
  ptr->ErrorCode = ImageInfo::NoErr;
  return ptr;
}

void Favorite::Dislike(const std::u8string& sImgPath)
{
  const fs::path oldPath = sImgPath;
  const fs::path oldDir = oldPath.parent_path();
  // oldDir.make_preferred();
  const auto curDir = GetImageDir();
  if (!fs::exists(curDir) && !fs::create_directories(curDir)) {
    return;
  }
  if (fs::exists(oldDir) && fs::equivalent(curDir, oldDir)) {
    fs::remove(oldPath);
  } else {
#ifdef _WIN32
    const auto fmtStr = Utf82AnsiString(m_Setting[u8"NameFormat"].getValueString());
#else
    const auto fmtStr = Utf8AsString(m_Setting[u8"NameFormat"].getValueString());
#endif
    const auto newName = std::vformat(fmtStr, std::make_format_args(oldPath.stem().string())) + oldPath.extension().string();
    fs::remove(curDir / newName);
  }
}

void Favorite::UndoDislike(const std::u8string& sImgPath)
{
  
  const fs::path oldPath = sImgPath;
  const auto curDir = GetImageDir();
  if (!fs::exists(curDir) && !fs::create_directories(curDir)) {
    return;
  }
#ifdef _WIN32
  const auto fmtStr = Utf82AnsiString(m_Setting[u8"NameFormat"].getValueString());
#else
  const auto fmtStr = Utf8AsString(m_Setting[u8"NameFormat"].getValueString());
#endif
  const auto newName = std::vformat(fmtStr, std::make_format_args(oldPath.stem().string())) + oldPath.extension().string();
  const auto newPath = curDir / newName;
  if (!fs::exists(newPath) && fs::exists(oldPath)) {
    fs::copy_file(oldPath, newPath);
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

// void Favorite::SetCurDir(const std::u8string& str)
// {
//   m_Setting[u8"Directory"] = str;
//   SaveSetting();
// }
