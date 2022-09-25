#ifndef FAVORITE_H
#define FAVORITE_H

#include <wallbase.h>
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class Favorite : public WallBase {
 private:
  const char m_DataPath[14]{"Favorite.json"};
  YJson* m_Data;

 public:
  bool LoadSetting() override {
    if (!fs::exists(m_DataPath))
      return false;
    try {
      m_Data = new YJson(m_DataPath, YJson::UTF8);
      if (auto iter = m_Data->find(u8"Dir"); iter != m_Data->endO()) {
        m_ImageDir = iter->second.getValueString();
      } else {
        m_ImageDir = m_HomePicLocation / u8"收藏壁纸";
        m_Data->getObject().emplace_back(u8"Dir", m_ImageDir.u8string());
        m_Data->toFile(m_DataPath);
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
    using namespace std::literals;
    delete m_Data;
    m_ImageDir = m_HomePicLocation / u8"收藏壁纸"s;
    if (!fs::exists(m_ImageDir)) {
      fs::create_directories(m_ImageDir);
    }
    m_Data = new YJson(
      YJson::O{
        {u8"Unused"sv, YJson::Array}, 
        {u8"Used"sv, YJson::Array},
        {u8"Dir", m_ImageDir.u8string()}
      });
    m_Data->toFile(m_DataPath);
    return true;
  }
  ImageInfoEx GetNext() override {
    ImageInfoEx ptr(new ImageInfo);

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
      pImgPath = m_ImageDir / pImgPath.filename();
      if (!fs::exists(pImgPath)) {
        fs::copy(ptr->ImagePath, pImgPath);
      }
    }
    return ptr;
  }

  void Dislike(const std::u8string& sImgPath) override {
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
      pImgPath = m_ImageDir / pImgPath.filename();
      if (fs::exists(pImgPath)) {
        fs::remove(pImgPath);
      }
    }
    m_Data->toFile(m_DataPath);
  }

  void UndoDislike(const std::u8string& sImgPath) override {
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
      pImgPath = m_ImageDir / pImgPath.filename();
      if (!fs::exists(pImgPath)) {
        fs::copy(sImgPath, pImgPath);
      }
    }
  }

  YJson* GetJson() override { return &m_Data->find(u8"Unused")->second; }

  void SetJson(bool update) override { m_Data->toFile(m_DataPath); }

  void SetCurDir(const std::u8string& str) override {
    m_Data->find(u8"Dir")->second = str;
    m_ImageDir = str;
    m_Data->toFile(m_DataPath);
  }

  explicit Favorite() : WallBase(), m_Data(nullptr) { InitBase(); }
  ~Favorite() { delete m_Data; }
};

#endif
