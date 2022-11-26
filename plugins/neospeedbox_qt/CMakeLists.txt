cmake_minimum_required(VERSION 3.10.1)

find_package(Qt6 REQUIRED COMPONENTS UiTools)
file(GLOB source_files src/*.cpp)

set(system_shared_files Shell32)
set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui Qt6::UiTools)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  message(STATUS "============deBUG=================")
  set(pliugin_libfiles neospeedbox_core yjson)
  add_library(neospeedboxplg STATIC ${source_files})
else()
  add_library(neospeedboxplg SHARED ${source_files})
  set(pliugin_libfiles neospeedbox_core neobox_core yjson)
endif()

file(GLOB _resource_files fonts/*.ttf icons/* skins/*.ui)

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


