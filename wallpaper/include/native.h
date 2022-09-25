#include <functional>
#include <numeric>
#include <set>

#include <wallbase.h>
#include <wallpaper.h>

namespace fs = std::filesystem;

static size_t GetFileCount(fs::path path) {
  size_t n = 0;
  for (auto& iter : fs::directory_iterator(path)) {
    if (fs::is_directory(iter.status())) {
      n += GetFileCount(iter.path());
    } else if (Wallpaper::IsImageFile(iter.path())) {
      ++n;
    }
  }
  return n;
}

class Native : public WallBase {
 private:
  size_t GetFileCount() {
    size_t m_iCount = 0;
    if (!fs::exists(m_ImageDir) || !fs::is_directory(m_ImageDir))
      return m_iCount;

    const bool bRecursion = m_Setting->find(u8"recursion")->second.isTrue();
    for (auto& iter : fs::directory_iterator(m_ImageDir)) {
      if (fs::is_directory(iter.status())) {
        if (bRecursion) {
          m_iCount += ::GetFileCount(iter.path());
        }
      } else if (Wallpaper::IsImageFile(iter.path())) {
        ++m_iCount;
      }
    }
    return m_iCount;
  }

  bool GetFileList() {
    size_t m_Toltal = GetFileCount(), m_Index = 0;
    if (!m_Toltal)
      return false;
    std::vector<size_t> numbers;
    if (m_Toltal < m_MaxCount) {
      numbers.resize(m_Toltal);
      std::iota(numbers.begin(), numbers.end(), 0);
    } else {
      std::set<size_t> already;
      std::mt19937 g(std::random_device{}());
      auto pf = std::uniform_int_distribution<size_t>(0, m_Toltal - 1);
      for (int i = 0; i < m_MaxCount; ++i) {
        auto temp = pf(g);
        while (already.find(temp) != already.end())
          temp = pf(g);
        already.insert(temp);
        numbers.push_back(temp);
      }
      std::sort(numbers.begin(), numbers.end());
    }

    const bool bRecursion = m_Setting->find(u8"recursion")->second.isTrue();
    auto target = numbers.cbegin();
    std::function<void(fs::path)> _GetFileList;
    _GetFileList = [&](fs::path path) {
      for (auto& iter : fs::directory_iterator(path)) {
        fs::path path = iter.path();
        if (fs::is_directory(iter.status())) {
          _GetFileList(path);
        } else if (Wallpaper::IsImageFile(path)) {
          if (*target == m_Index) {
            m_FileList.emplace_back(path.u8string());
            ++target;
          }
          ++m_Index;
        }
      }
    };
    for (auto& iter : fs::directory_iterator(m_ImageDir)) {
      fs::path path = iter.path();
      if (fs::is_directory(iter.status())) {
        if (bRecursion) {
          _GetFileList(path);
        }
      } else if (Wallpaper::IsImageFile(path)) {
        if (*target == m_Index) {
          m_FileList.emplace_back(path.u8string());
          ++target;
        }
        ++m_Index;
      }
    }

    std::mt19937 g(std::random_device{}());
    if (m_Setting->find(u8"random")->second.isTrue()) {
      std::shuffle(m_FileList.begin(), m_FileList.end(), g);
    }
    return true;
  }

 public:
  explicit Native() : WallBase() { InitBase(); }
  ~Native() override { delete m_Setting; }
  ImageInfoEx GetNext() override {
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
  bool LoadSetting() override {
    if (fs::exists(m_SettingPath)) {
      m_Setting = new YJson(m_SettingPath, YJson::UTF8);
      m_ImageDir =
          m_Setting->find(u8"imgdirs")->second.beginA()->getValueString();
      return true;
    }
    return false;
  }
  bool WriteDefaultSetting() override {
    using namespace std::literals;
    m_ImageDir = m_HomePicLocation;
    m_Setting = new YJson(YJson::O{{u8"imgdirs"sv, {m_ImageDir}},
                                   {u8"random"sv, true},
                                   {u8"recursion"sv, false}});
    m_Setting->toFile(m_SettingPath);
    return true;
  }
  void SetCurDir(const std::u8string& str) override {
    m_ImageDir = str;
    auto& li = m_Setting->find(u8"imgdirs")->second;
    li.beginA()->setText(str);
    m_Setting->toFile(m_SettingPath);
    m_FileList.clear();
  }

  YJson* GetJson() override { return m_Setting; }

  void SetJson(bool update) override {
    m_FileList.clear();
    m_Setting->toFile(m_SettingPath);
  }

 private:
  const char m_SettingPath[12]{"Native.json"};
  const uint32_t m_MaxCount = 100;
  YJson* m_Setting;
  std::vector<std::u8string> m_FileList;
};
