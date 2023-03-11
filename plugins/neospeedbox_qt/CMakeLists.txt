cmake_minimum_required(VERSION 3.10.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# if(UNIX AND NOT MACOS)
#   find_package(KF5WindowSystem REQUIRED)
# endif()

# find_package(Qt6 REQUIRED COMPONENTS UiTools)
file(GLOB source_files src/*.cpp)

if(WIN32)
  set(system_shared_files Shell32)
elseif(UNIX)
  # Qt6 和 KF5冲突
  # set(system_shared_files KF5::WindowSystem)
  set(system_shared_files "")
endif()
set(qt_shared_files Qt6::Core Qt6::Widgets)
set(pliugin_libfiles neospeedbox_core pluginmgr yjson)

add_library(neospeedboxplg SHARED ${source_files})

set(resource_files fonts/*.ttf icons/* dlls/*.dll sos/*.so)
# add_subdirectory(..)
load_resources(neospeedboxplg ${resource_files})

target_link_libraries(neospeedboxplg PUBLIC
  ${pliugin_libfiles}
  ${qt_shared_files}
  ${system_shared_files}
)

file(GLOB _ui_files skins/*.ui)
foreach(full_filepath ${_ui_files})
  qt_wrap_ui(wrap_file ${full_filepath}) # 根据UI文件生成头文件
  get_filename_component(file_path ${full_filepath} NAME_WE)
  add_library(${file_path} SHARED
    skins/${file_path}.cpp
    ${wrap_file}
  )
  target_link_libraries(
    ${file_path}
    neospeedboxplg
    ${pliugin_libfiles}
    ${qt_shared_files}
  )
  install(TARGETS ${file_path}
    LIBRARY DESTINATION skins
    ARCHIVE DESTINATION skins
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/dlls
    PUBLIC_HEADER DESTINATION skins
  )
  # file(COPY ${CMAKE_CURRENT_BINARY_DIR}/${file_path}.dll
  #   DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/dlls/${file_path}.dll FOLLOW_SYMLINK_CHAIN)
endforeach()
