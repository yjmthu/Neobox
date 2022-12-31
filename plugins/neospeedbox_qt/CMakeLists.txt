cmake_minimum_required(VERSION 3.10.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS UiTools)
file(GLOB source_files src/*.cpp)

set(system_shared_files Shell32)
set(qt_shared_files Qt6::Core Qt6::Widgets)
set(pliugin_libfiles neospeedbox_core pluginmgr yjson)

add_library(neospeedboxplg SHARED ${source_files})

file(GLOB _resource_files fonts/*.ttf icons/* dlls/*.dll)

set(resource_files "")
foreach(full_filepath ${_resource_files})
  file(RELATIVE_PATH file_path ${CMAKE_CURRENT_SOURCE_DIR} ${full_filepath})
  message(STATUS "res='${file_path}'")
  list(APPEND resource_files ${file_path})
endforeach()

qt_add_resources(neospeedboxplg "neospeedboxplg"
    PREFIX "/"
    FILES ${resource_files}
)

target_link_libraries(neospeedboxplg PRIVATE
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
