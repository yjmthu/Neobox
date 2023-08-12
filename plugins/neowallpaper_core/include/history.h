#ifndef HISTORY_H
#define HISTORY_H

#include <deque>
#include <filesystem>
#include <optional>
#include <algorithm>

namespace fs = std::filesystem;

typedef std::deque<fs::path> WallpaperHistoryBase;

class WallpaperHistory: public WallpaperHistoryBase
{
private:
  void ReadSettings();
  void WriteSettings();
  using WallpaperHistoryBase::push_back;
  using WallpaperHistoryBase::back;
  using WallpaperHistoryBase::front;
public:

  explicit WallpaperHistory();
  ~WallpaperHistory();

  void UpdateRegString();
  void PushBack(fs::path path) {
    path.make_preferred();
    WallpaperHistoryBase::push_back(std::move(path));
  }

  std::optional<value_type> GetCurrent() const {
    if (empty()) return std::nullopt;
    return back();
  }

  void EraseCurrent() {
    if (!empty()) pop_back();
  }

  std::optional<value_type> GetPrevious() {
    if (size() < 2) return std::nullopt;
    auto iter = std::find_if(crbegin() + 1, crend(), [](const fs::path & path){
      return fs::exists(path);
    });
    std::optional<value_type> res;
    if (iter != crend()) {
      res = *iter;
    }
    erase(iter.base(), cend() - 1);
    return res;
  }

  void ErasePrevious() {
    if (size() < 2) return;
    erase(cend() - 2);
  }
};

#endif // HISTORY