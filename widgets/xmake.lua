add_requires("leptonica", "tesseract")

set_configvar("NEOBOX_BETA", "Release")
set_configvar("NEOBOX_LATEST", "https://api.github.com/repos/yjmthu/Neobox/releases/latest")
set_configvar("NEOBOX_WEBSITE", "https://github.com/yjmthu/Neobox")
set_configvar("NEOBOX_COPYRIGHT", "Copyright © 2022 yjmthu. All rights reserved.")

target("Neobox")
  set_default(true)
  set_configdir("include")
  add_configfiles("src/config.h.in")
  -- add_rules("mode.release", "mode.debug")
  if is_plat("windows") then
    add_cxflags("/Zc:__cplusplus")
  elseif is_os("linux") then
    -- find_package(KF5WindowSystem REQUIRED)
    -- KF5::WindowSystem
    add_syslinks("pthread")
  end
  add_rules("qt.widgetapp")
  set_kind("binary")
  add_files("src/*.cpp", "resources.qrc", "logo.rc")
  add_frameworks("QtCore", "QtWidgets", "QtGui", "QtSvg", "Qtuitools")
  add_packages("leptonica", "tesseract")
  add_deps("core", "wallpaper", "yjson", "translate")
target_end()
