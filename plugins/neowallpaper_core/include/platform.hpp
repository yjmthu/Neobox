#pragma once

#include <string>
#include <filesystem>
#include <format>
#include <systemapi.h>
#include <list>

#ifdef _DEBUG
#include <iostream>
#endif

#ifdef __linux__
#include <unistd.h>
#endif

namespace WallpaperPlatform {

namespace fs = std::filesystem;
using namespace std::literals;

enum class Desktop { WIN, KDE, DDE, GNOME, XFCE, UNKNOWN };

inline Desktop GetDesktop() {
#ifdef _WIN32
  return Desktop::WIN;
#elif defined(__linux__)
  auto const ndeEnv = std::getenv("XDG_CURRENT_DESKTOP");
  if (!ndeEnv) {
    return Desktop::UNKNOWN;
  }
  std::string nde = ndeEnv;
  constexpr auto npos = std::string::npos;
  if (nde.find("KDE") != npos) {
    return Desktop::KDE;
  } else if (nde.find("GNOME") != npos) {
    return Desktop::DDE;
  } else if (nde.find("DDE") != npos) {
    return Desktop::DDE;
  } else if (nde.find("XFCE") != npos) {
    return Desktop::XFCE;
  } else {
    return Desktop::UNKNOWN;
  }
#else
  return Desktop::UNKNOWN;
#endif
}

inline std::optional<fs::path> GetWallpaper() {
#ifdef __linux__
  switch (GetDesktop()) {
  case Desktop::KDE: {
    char argStr[] = "qdbus org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.evaluateScript '"
      "var allDesktops = desktops();"
      "print(allDesktops);"
      "for (i=0; i < allDesktops.length; i++){"
        "d = allDesktops[i];"
        "d.wallpaperPlugin = \"org.kde.image\";"
        "d.currentConfigGroup = Array(\"Wallpaper\", \"org.kde.image\", \"General\");"
        "print(d.readConfig(\"Image\"));"
      "}"
    "'";
    std::list<std::string> result;
    GetCmdOutput(argStr, result);
    while (!result.empty() && result.front().empty()) {
      result.pop_front();
    }
    if (!result.empty()) {
      std::string_view data = result.front();
      auto pos = data.find("file://");
      if (pos != data.npos) {
        return data.substr(pos + 7);
      }
    }
    break;
  }
  case Desktop::GNOME:{
    std::string_view argStr = "gsettings get org.gnome.desktop.background picture-uri"sv;
    std::list<std::string> result;
    GetCmdOutput(argStr.data(), result);
    break;
  }
  case Desktop::DDE: {
    std::string_view argStr = "dbus-send --session --print-reply --dest=com.deepin.daemon.Appearance /com/deepin/daemon/Appearance com.deepin.daemon.Appearance.GetMonitorBackground"sv;
    break;
  }
  case Desktop::XFCE: {
    std::string_view argStr = "xfconf-query -c xfce4-desktop -p /backdrop/screen0/monitor0/workspace0/last-image"sv;
    std::list<std::string> result;
    GetCmdOutput(argStr.data(), result);
    break;
  }
  default:
    break;
  }
  return std::nullopt;
#elif defined(_WIN32)
  auto curWallpaper = RegReadString(HKEY_CURRENT_USER, L"Control Panel\\Desktop", L"WallPaper");
  if (curWallpaper.empty() || !fs::exists(curWallpaper)) {
    return std::nullopt;
  }
  return curWallpaper;
#else
  return std::nullopt;
#endif
}

inline bool SetWallpaper(fs::path imagePath) {
#ifdef __linux__
  static auto const m_DesktopType = GetDesktop();
#endif
  if (!fs::exists(imagePath) || !fs::is_regular_file(imagePath)) {
    return false;
  }
#if defined(_WIN32)
  std::thread([imagePath]() mutable {
    // use preferred separator to prevent win32 api crash.
    imagePath.make_preferred();
    std::wstring str = imagePath.wstring();
    ::SystemParametersInfoW(
      SPI_SETDESKWALLPAPER, UINT(0),
      const_cast<WCHAR*>(str.c_str()),
      SPIF_SENDCHANGE | SPIF_UPDATEINIFILE);
  }).detach();
  return true;
#elif defined(__linux__)
  std::string argStr;
  switch (m_DesktopType) {
    // https://github.com/bharadwaj-raju/libdesktop/issues/1
    case Desktop::KDE:
      argStr = std::format(
        "var allDesktops = desktops();"
        "print(allDesktops);"
        "for (i=0; i < allDesktops.length; i++){{"
          "d = allDesktops[i];"
          "d.wallpaperPlugin = \"org.kde.image\";"
          "d.currentConfigGroup = Array(\"Wallpaper\", \"org.kde.image\", \"General\");"
          "d.writeConfig(\"Image\", \"file://{}\")"
        "}}", imagePath.string());
      if (fork() == 0) {
        execlp(
          "qdbus", "qdbus",
          "org.kde.plasmashell", "/PlasmaShell",
          "org.kde.PlasmaShell.evaluateScript",
          argStr.c_str(), nullptr
        );
      }
      break;
    case Desktop::GNOME:
      argStr = std::format("\"file:{}\"", imagePath.string());
      if (fork() == 0) {
        execlp(
          "gsettings", "gsettings",
          "set", "org.gnome.desktop.background", "picture-uri",
          argStr.c_str(), nullptr
        );
      }
      break;
      // cmdStr = "gsettings set org.gnome.desktop.background picture-uri \"file:" + imagePath.string();
    case Desktop::DDE:
    /*
      Old deepin:
      std::string m_sCmd ("gsettings set
      com.deepin.wrap.gnome.desktop.background picture-uri \"");
    */
      // xrandr|grep 'connected primary'|awk '{print $1}' ======> eDP
      argStr = std::format("string:\"file://{}\"", imagePath.string());
      if (fork() == 0) {
        execlp(
          "dbus-send", "dbus-send",
          "--dest=com.deepin.daemon.Appearance", "/com/deepin/daemon/Appearance",
          "--print-reply", "com.deepin.daemon.Appearance.SetMonitorBackground",
          "string:\"eDP\"", argStr.c_str(), nullptr
        );
      }
      break;
    case Desktop::XFCE:
      argStr = std::format("\"{}\"", imagePath.string());
      if (fork() == 0) {
        execlp(
          "xfconf-query",
          "xfconf-query",
          "-c", "xfce4-desktop",
          "-p", "/backdrop/screen0/monitor0/workspace0/last-image",
          "-s", argStr.c_str(), nullptr
        );
      }
      break;
    default:
#ifdef _DEBUG
      std::cerr << "不支持的桌面类型；\n";
#endif
      return false;
  }
  return true;
#endif
}

}
