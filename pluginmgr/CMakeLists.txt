cmake_minimum_required(VERSION 3.10.1)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

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
load_resources(pluginmgr resource_files)

target_link_libraries(pluginmgr PUBLIC
  ${pliugin_libfiles}
  ${qt_shared_files}
  ${system_shared_files}
)

install(TARGETS pluginmgr
  LIBRARY DESTINATION bin
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  PUBLIC_HEADER DESTINATION include
)

if (MSVC)
  add_executable(update WIN32 src/update.cc)
  add_executable(installer WIN32 src/install.cc)
  add_executable(uninstaller WIN32 src/uninstall.cc)
  set(app_helper update installer uninstaller)
  foreach(app ${app_helper})
    target_link_libraries(${app} PRIVATE pluginmgr)
  endforeach()

  if (MSVC)
    install(TARGETS ${app_helper}
      RUNTIME DESTINATION bin
    )
    set_target_properties(${app_helper}
      PROPERTIES LINK_FLAGS
      "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:WINDOWS"
    )
  endif()
endif()
