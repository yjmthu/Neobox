#include <native.h>
#include <httplib.h>
#include <wallpaper.h>
#include <systemapi.h>

#include <set>
#include <utility>
#include <numeric>
#include <functional>
#include <filesystem>

namespace fs = std::filesystem;
using namespace std::literals;

Native::Native(YJson& setting):
  WallBase(InitSetting(setting))
{
}

Native::~Native()
{
}

YJson& Native::InitSetting(YJson& setting)
{
  if (setting.isObject())
    return setting;
  
  setting = YJson::O {
    {u8"curdir", u8"默认位置"},
    {u8"max", 100},
    {u8"dirs", YJson::O {
      {u8"默认位置", YJson::O {
        {u8"imgdir"sv, ms_HomePicLocation.u8string() },
        {u8"random"sv, true},
        {u8"recursion"sv, false},
      }}
    }}
  };
  SaveSetting();
  return setting;
}

YJson& Native::GetCurInfo()
{
  return m_Setting[u8"dirs"][m_Setting[u8"curdir"].getValueString()];
}

fs::path Native::GetImageDir() const
{
  return GetCurInfo()[u8"imgdir"].getValueString();
}

size_t Native::GetFileCount() {
  size_t m_iCount = 0;
  auto const curDir = GetImageDir();
  if (!fs::exists(curDir) || !fs::is_directory(curDir))
    return m_iCount;

  const bool bRecursion = GetCurInfo()[u8"recursion"].isTrue();
  std::queue<fs::path> qDirsToWalk;
  qDirsToWalk.push(curDir);

  while (!qDirsToWalk.empty()) {
    for (auto& iter : fs::directory_iterator(qDirsToWalk.front())) {
      if (fs::is_directory(iter.status())) {
        if (bRecursion) {
          qDirsToWalk.push(iter.path());
        }
      } else if (Wallpaper::IsImageFile(iter.path())) {
        ++m_iCount;
      }
    }
    qDirsToWalk.pop();
  }
  return m_iCount;
}

bool Native::GetFileList() {
  size_t m_Toltal = GetFileCount(), m_Index = 0;
  if (!m_Toltal)
    return false;
  auto const curDir = GetImageDir();
  auto const maxCount = m_Setting[u8"max"].getValueInt();
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
    // std::sort(numbers.begin(), numbers.end());
  }

  const bool bRecursion = GetCurInfo()[u8"recursion"].isTrue();
  auto target = numbers.cbegin();

  std::queue<fs::path> qDirsToWalk;
  qDirsToWalk.push(curDir);

  while (!qDirsToWalk.empty()) {
    for (auto& iter : fs::directory_iterator(qDirsToWalk.front())) {
      fs::path path = iter.path();
      if (fs::is_directory(iter.status())) {
        if (bRecursion) {
          qDirsToWalk.push(path);
        }
      } else if (Wallpaper::IsImageFile(path)) {
        if (*target == m_Index) {
          m_FileList.emplace_back(path.u8string());
          ++target;
        }
        ++m_Index;
      }
    }
    qDirsToWalk.pop();
  }

  if (GetCurInfo()[u8"random"].isTrue()) {
    std::shuffle(m_FileList.begin(), m_FileList.end(), g);
  }
  return true;
}

ImageInfoEx Native::GetNext()
{
  ImageInfoEx ptr(new ImageInfo);

  while (!m_FileList.empty() && !fs::exists(m_FileList.back())) {
    m_FileList.pop_back();
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

// void Native::SetCurDir(const std::u8string& str)
// {
//   GetCurInfo()[u8"imgdir"] = str;
//   m_FileList.clear();
//   SaveSetting();
// }

void Native::SetJson(bool update) {
  m_FileList.clear();
  SaveSetting();
}
