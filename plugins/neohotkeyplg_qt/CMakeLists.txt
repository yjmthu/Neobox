cmake_minimum_required(VERSION 3.10.1)

# set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# include_directories(${CMAKE_BINARY_DIR})

file(GLOB
  source_files src/*.cpp
  include/shortcutdlg.h
  include/hotkeyitemwidget.h
)

qt_wrap_ui(WRAP_FILES src/shortcutdlg.ui) # 根据UI文件生成头文件

# set(system_shared_files Shell32)
set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui)
set(pliugin_libfiles neobox_core yjson)

add_library(neohotkeyplg SHARED
  ${source_files}
  ${WRAP_FILES}
)

target_link_libraries(neohotkeyplg PRIVATE
  ${pliugin_libfiles}
  ${qt_shared_files}
  ${system_shared_files}
)
