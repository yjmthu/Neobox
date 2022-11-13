add_requires("leptonica", "tesseract")

target("Neobox")
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
