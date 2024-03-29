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

file(GLOB source_files include/*/*.hpp src/*.cpp widgets/*.cpp widgets/*.hpp)

set(system_shared_files)
set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui)
set(pliugin_libfiles yjson)

if(WIN32)
  list(APPEND system_shared_files Iphlpapi Winhttp Shell32)
else()
  list(APPEND system_shared_files curl)
endif()

add_subdirectory(../thirdlib/YJson yjson)
if(WIN32)
  add_subdirectory(../thirdlib/zip zip)
  add_subdirectory(../thirdlib/WinToast WinToast)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

set(third_include ../thirdlib/YJson/include)
if(WIN32)
  list(APPEND third_include ../thirdlib/zip/src)
  list(APPEND third_include ../thirdlib/WinToast/include)
  list(APPEND pliugin_libfiles zip WinToast)
endif()

set(target_name pluginmgr)
add_library(${target_name} SHARED
  ${source_files}
  ${wrap_files}
)

target_include_directories(${target_name} PUBLIC
  include
  ${third_include}
)

if(MSVC)
  target_compile_options(${target_name} PRIVATE /W4 /WX)
else()
  target_compile_options(${target_name} PRIVATE -Wall -Wextra -pedantic) # -Werror
endif()

set(resource_files
  fonts/*.ttf
  icons/*.png
  icons/*.ico
  jsons/*.json
  scripts/*.desktop
  styles/*.qss
)
load_resources(${target_name} resource_files)

target_link_libraries(${target_name} PUBLIC
  ${pliugin_libfiles}
  ${qt_shared_files}
  ${system_shared_files}
)

install(TARGETS ${target_name}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/neobox DESTINATION "include")

if (MSVC)
  add_executable(update WIN32 src/update.cc)
  add_executable(installer WIN32 src/install.cc)
  add_executable(uninstaller WIN32 src/uninstall.cc)
  set(app_helper update installer uninstaller)
  foreach(app ${app_helper})
    target_link_libraries(${app} PRIVATE ${target_name})
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
