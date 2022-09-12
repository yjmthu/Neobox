#ifndef FAVORITE_H
#define FAVORITE_H

#include <wallbase.h>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <string>

class Favorite : public WallBase {
 private:
  const char m_DataPath[14]{"Favorite.json"};
  YJson* m_Data;

 public:
  bool LoadSetting() override {
    if (!std::filesystem::exists(m_DataPath))
      return false;
    try {
      m_Data = new YJson(m_DataPath, YJson::UTF8);
      return true;
    } catch (...) {
      return false;
    }
  }

  bool WriteDefaultSetting() override {
    using namespace std::literals;
    delete m_Data;
    m_Data = new YJson(
        YJson::O{{u8"Unused"sv, YJson::Array}, {u8"Used"sv, YJson::Array}});
    m_Data->toFile(m_DataPath);
    return true;
  }
  ImageInfoEx GetNext() override {
    ImageInfoEx ptr(new std::vector<std::u8string>);

    auto& arrayA = m_Data->find(u8"Unused")->second.getArray();
    auto& arrayB = m_Data->find(u8"Used")->second.getArray();
    if (arrayA.empty() && arrayB.empty()) {
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

    ptr->emplace_back(std::move(arrayA.back().getValueString()));
    arrayA.pop_back();
    arrayB.push_back(ptr->back());
    m_Data->toFile(m_DataPath);
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
    m_Data->toFile(m_DataPath);
  }

  void UndoDislike(const std::u8string& sImgPath) override {
    auto& used = m_Data->find(u8"Used")->second;
    auto& unused = m_Data->find(u8"Unused")->second;
    if (used.findByValA(sImgPath) != used.endA() ||
        unused.findByValA(sImgPath) != unused.endA())
      return;
    used.append(sImgPath);
    m_Data->toFile(m_DataPath);
  }

  YJson* GetJson() override { return &m_Data->find(u8"Unused")->second; }

  void SetJson(bool update) override { m_Data->toFile(m_DataPath); }

  void SetCurDir(const std::u8string& str) override {}

  explicit Favorite(const std::filesystem::path& picHome)
      : WallBase(picHome), m_Data(nullptr) {
    InitBase();
  }
  ~Favorite() { delete m_Data; }
};

#endif
