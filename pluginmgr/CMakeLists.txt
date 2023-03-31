cmake_minimum_required(VERSION 3.10.1)

# if (UNIX)
#   find_package(CURL CONFIG REQUIRED)
# endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)
qt_standard_project_setup()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

configure_file(
  src/config.h.in config.h
)

qt_wrap_ui(wrap_files
  widgets/tabnetproxy.ui
  widgets/tabhotkey.ui
) # 根据UI文件生成头文件

file(GLOB source_files include/*.hpp src/*.cpp widgets/*.cpp widgets/*.hpp)

set(system_shared_files)
set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui)
set(pliugin_libfiles yjson zip)

if(WIN32)
  list(APPEND system_shared_files Iphlpapi Winhttp Shell32)
else()
  list(APPEND system_shared_files curl)
endif()

add_library(pluginmgr SHARED
  ${source_files}
  ${wrap_files}
)

set(resource_files
  fonts/*.ttf
  icons/*.png
  icons/*.ico
  jsons/*.json
  scripts/*.desktop
  styles/*.qss
)
load_resources(pluginmgr
  "${resource_files}"
)

target_link_libraries(pluginmgr PUBLIC
  ${pliugin_libfiles}
  ${qt_shared_files}
  ${system_shared_files}
)

install(TARGETS pluginmgr
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  PUBLIC_HEADER DESTINATION include
)
