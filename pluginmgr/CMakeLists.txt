cmake_minimum_required(VERSION 3.10.1)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)
qt_standard_project_setup()

file(GLOB source_files src/*.cpp widgets/*.cpp)
file(GLOB _resource_files fonts/*.ttf  icons/*  jsons/*.json scripts/* styles/*)

set(system_shared_files Shell32)
set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui)
set(pliugin_libfiles neocore yjson)

set(resource_files "")
foreach(full_filepath ${_resource_files})
  file(RELATIVE_PATH file_path ${CMAKE_CURRENT_SOURCE_DIR} ${full_filepath})
  message(STATUS "res='${file_path}'")
  list(APPEND resource_files ${file_path})
endforeach()

add_library(pluginmgr SHARED ${source_files})

target_link_libraries(pluginmgr PUBLIC
  ${pliugin_libfiles}
  ${qt_shared_files}
  ${system_shared_files}
)

qt_add_resources(pluginmgr "neobox"
  PREFIX "/"
  FILES ${resource_files}
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  # message(STATUS "============deBUG=================")
  # target_link_libraries(pluginmgr PRIVATE neospeedboxplg)
else()
endif()

install(TARGETS pluginmgr
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  PUBLIC_HEADER DESTINATION include
)
