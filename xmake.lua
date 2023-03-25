set_languages("cxx20")

add_subdirs(
  "thirdlib/YJson",
  "pluginmgr",
  "plugins"
)

target("neobox")
  add_rules("qt.widgetapp")
  add_files("src/*.cpp") 
  add_files("src/mainwindow.ui")
  add_files("src/mainwindow.h")  -- 添加带有 Q_OBJECT 的meta头文件
