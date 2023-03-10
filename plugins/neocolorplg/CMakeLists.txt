cmake_minimum_required(VERSION 3.10.1)

# find_package(Leptonica CONFIG REQUIRED)
file(GLOB source_files src/*.cpp include/*.hpp)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(system_shared_files Shell32)
set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui)
set(pliugin_libfiles pluginmgr)

qt_wrap_ui(wrap_files
  src/colordlg.ui
  src/smallform.ui
)

add_library(neocolorplg SHARED 
  ${source_files} 
  ${wrap_files}
)

file(GLOB _resource_files icons/*.png styles/*.qss)
set(resource_files "")
foreach(full_filepath ${_resource_files})
  file(RELATIVE_PATH file_path ${CMAKE_CURRENT_SOURCE_DIR} ${full_filepath})
  message(STATUS "res='${file_path}'")
  list(APPEND resource_files ${file_path})
endforeach()

qt_add_resources(neocolorplg "neocolorplg"
  PREFIX "/"
  FILES ${resource_files}
)


target_link_libraries(neocolorplg PRIVATE
  ${pliugin_libfiles}
  ${qt_shared_files}
  # ${system_shared_files}
)
