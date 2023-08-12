#include <favorie.h>
#include <download.h>

#include <set>

Favorite::Favorite(YJson& setting):
  WallBase(InitSetting(setting))
{
}

Favorite::~Favorite()
{
}

YJson& Favorite::InitSetting(YJson& setting)
{
  if (!setting.isObject()) {
    setting = YJson::O {
      { u8"Directory", GetStantardDir(m_Name) },
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
  }

  return setting;
}

size_t Favorite::GetFileCount() const
{
  size_t m_iCount = 0;
  auto const curDir = GetImageDir();
  if (!fs::exists(curDir) || !fs::is_directory(curDir))
    return m_iCount;

  for (auto& iter : fs::directory_iterator(curDir)) {
    auto& path = iter.path();
    if (fs::is_directory(iter.status())) {
    } else if (DownloadJob::IsImageFile(path.u8string())) {
      ++m_iCount;
    }
  }
  return m_iCount;
}

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
    auto& path = iter.path();
    if (fs::is_directory(iter.status())) {
    } else if (DownloadJob::IsImageFile(path.u8string())) {
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

void Favorite::GetNext(Callback callback)
{
  Locker locker(m_DataMutex);

  ImageInfoEx ptr(new ImageInfo);

  while (!m_FileList.empty() && !fs::exists(m_FileList.back())) {
    m_FileList.pop_back();
  }

  while (!m_FileList.empty()) {
    if (!fs::exists(m_FileList.back()))
      m_FileList.pop_back();
    else
      break;
  }

  if (m_FileList.empty() && !GetFileList()) {
    ptr->ErrorMsg = u8"Empty folder with no wallpaper in it.";
    ptr->ErrorCode = ImageInfo::FileErr;
  } else {
    ptr->ImagePath = std::move(m_FileList.back());
    m_FileList.pop_back();
    ptr->ErrorCode = ImageInfo::NoErr;
  }
  callback(ptr);
}

void Favorite::Dislike(std::u8string_view sImgPath)
{
  Locker locker(m_DataMutex);

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

void Favorite::UndoDislike(std::u8string_view sImgPath)
{  
  Locker locker(m_DataMutex);

  const fs::path oldPath = sImgPath;
  const auto curDir = GetImageDir();
  std::error_code error;
  if (!fs::exists(curDir) && !fs::create_directories(curDir, error)) {
    mgr->ShowMsgbox(L"出错", std::format(L"创建文件夹失败！\n文件夹名称：{}。\n错误码：{}。",
      curDir.wstring(), error.value()));
    return;
  }
  const auto fmtStr = Utf82WideString(m_Setting[u8"NameFormat"].getValueString());
  const auto newName = std::vformat(fmtStr,
    std::make_wformat_args(oldPath.stem().wstring())) + oldPath.extension().wstring();
  const auto newPath = curDir / newName;
  if (!fs::exists(newPath) && fs::exists(oldPath)) {
    fs::copy_file(oldPath, newPath);
  }
}

fs::path Favorite::GetImageDir() const
{
  return m_Setting[u8"Directory"].getValueString();
}
