cmake_minimum_required(VERSION 3.21.2)

# C++20标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 导出动态库
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# 导出ninja编译命令
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

string(TIMESTAMP COMPILE_TIME %Y-%m-%d)
set(NEOBOX_BUILD_TIME ${COMPILE_TIME})

set(target_name neobox)
project(${target_name} VERSION 2.4.7 LANGUAGES CXX C)
# cmake_policy(SET CMP0095 OLD)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_definitions(-D_DEBUG)
  message("====== defined _DEBUG ======")
else()
  add_definitions(-D_RELEASE)
  message("====== defined _RLEASE ======")
endif()

if (WIN32)
  add_definitions(-DUNICODE -D_UNICODE)
elseif(UNIX)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
  set(CMAKE_INSTALL_RPATH "\${ORIGIN}/../lib")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

if(MSVC)
  add_compile_options("/utf-8")
endif()


function(load_resources plugin_name files_list)
  message("- <${plugin_name}> ${${files_list}}")

  file(GLOB _resource_files ${${files_list}})
  set(resource_files)
  foreach(full_filepath ${_resource_files})
    file(RELATIVE_PATH file_path ${CMAKE_CURRENT_SOURCE_DIR} ${full_filepath})
    message(STATUS "res: '${file_path}'")
    list(APPEND resource_files ${file_path})
  endforeach()

  qt_add_resources(${plugin_name} "${plugin_name}"
    PREFIX "/"
    FILES ${resource_files}
  )
  message("- <${plugin_name}>")
endfunction()

link_directories(build/pluginmgr)

add_subdirectory(pluginmgr)
add_subdirectory(example)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
add_executable(${target_name} WIN32 main.cpp logo.rc)
if(UNIX)
  if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "None")
    message("set runtime lib path.")
    set_target_properties(${target_name} PROPERTIES
      INSTALL_RPATH "\${ORIGIN};/usr/lib;\${ORIGIN}/../lib")
  endif()
endif()


target_link_libraries(${target_name} PUBLIC
  pluginmgr
  Qt6::Widgets
)

if(MSVC)
  target_compile_options(${target_name} PRIVATE /W4 /WX)
else()
  target_compile_options(${target_name} PRIVATE -Wall -Wextra -pedantic) # -Werror 
endif()

if(WIN32)
  set_target_properties(${target_name} PROPERTIES
    LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
  )
endif()

install(TARGETS ${target_name})
