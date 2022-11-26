#include <wallpaper.h>
#include <wallbase.h>
#include <systemapi.h>

#include <utility>
#include <numeric>
#include <functional>
#include <filesystem>

// export module wallpaper3;

namespace fs = std::filesystem;
using namespace std::literals;

/* export */ class Favorite : public WallBase {
 private:
  const char m_szDataPath[14]{"Favorite.json"};
  YJson* m_pData;

 public:
  bool LoadSetting() override {
    if (!fs::exists(m_szDataPath))
      return false;
    try {
      m_pData = new YJson(m_szDataPath, YJson::UTF8);
      if (auto iter = m_pData->find(u8"Dir"); iter != m_pData->endO()) {
        m_ImageDir = iter->second.getValueString();
      } else {
        m_ImageDir = ms_HomePicLocation / u8"收藏壁纸";
        m_pData->getObject().emplace_back(u8"Dir", m_ImageDir.u8string());
        m_pData->toFile(m_szDataPath);
      }
      if (!fs::exists(m_ImageDir)) {
        // Make sure the directory exists.
        fs::create_directories(m_ImageDir);
      }
      return true;
    } catch (...) {
      return false;
    }
  }

  bool WriteDefaultSetting() override {
    delete m_pData;
    m_ImageDir = ms_HomePicLocation / u8"收藏壁纸"s;
    if (!fs::exists(m_ImageDir)) {
      fs::create_directories(m_ImageDir);
    }
    m_pData = new YJson(
      YJson::O{
        {u8"Unused"sv, YJson::Array}, 
        {u8"Used"sv, YJson::Array},
        {u8"Dir", m_ImageDir.u8string()}
      });
    m_pData->toFile(m_szDataPath);
    return true;
  }
  ImageInfoEx GetNext() override {
    ImageInfoEx ptr(new ImageInfo);

    auto& arrayA = m_pData->find(u8"Unused")->second.getArray();
    auto& arrayB = m_pData->find(u8"Used")->second.getArray();
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
    m_pData->toFile(m_szDataPath);
    ptr->ErrorCode = ImageInfo::NoErr;

    fs::path pImgPath = ptr->ImagePath;
    if (fs::exists(pImgPath) && pImgPath.has_filename()) {
      pImgPath = m_ImageDir / pImgPath.filename();
      if (!fs::exists(pImgPath)) {
        fs::copy(ptr->ImagePath, pImgPath);
      }
    }
    return ptr;
  }

  void Dislike(const std::u8string& sImgPath) override {
    auto ptr = &m_pData->find(u8"Used")->second;
    auto iter = ptr->findByValA(sImgPath);
    if (iter != ptr->endA())
      ptr->remove(iter);
    ptr = &m_pData->find(u8"Unused")->second;
    iter = ptr->findByValA(sImgPath);
    if (iter != ptr->endA())
      ptr->remove(iter);
    fs::path pImgPath = sImgPath;
    if (pImgPath.has_filename()) {
      pImgPath = m_ImageDir / pImgPath.filename();
      if (fs::exists(pImgPath)) {
        fs::remove(pImgPath);
      }
    }
    m_pData->toFile(m_szDataPath);
  }

  void UndoDislike(const std::u8string& sImgPath) override {
    auto& used = m_pData->find(u8"Used")->second;
    auto& unused = m_pData->find(u8"Unused")->second;

    if (used.findByValA(sImgPath) == used.endA() &&
        unused.findByValA(sImgPath) == unused.endA())
    {
      used.append(sImgPath);
      m_pData->toFile(m_szDataPath);
    }

    fs::path pImgPath = sImgPath;
    if (fs::exists(pImgPath) && pImgPath.has_filename()) {
      pImgPath = m_ImageDir / pImgPath.filename();
      if (!fs::exists(pImgPath)) {
        fs::copy(sImgPath, pImgPath);
      }
    }
  }

  YJson* GetJson() override { return &m_pData->find(u8"Unused")->second; }

  void SetJson(bool update) override { m_pData->toFile(m_szDataPath); }

  void SetCurDir(const std::u8string& str) override {
    m_pData->find(u8"Dir")->second = str;
    m_ImageDir = str;
    m_pData->toFile(m_szDataPath);
  }

  explicit Favorite() : WallBase(false), m_pData(nullptr) { InitBase(); }
  ~Favorite() { delete m_pData; }
};
