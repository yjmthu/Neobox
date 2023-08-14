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
project(${target_name} VERSION 2.4.3 LANGUAGES CXX C)

# if (NOT CMAKE_BUILD_TYPE)
#   message("编译类型未定义target_name默认设置为Debug")
#   set(CMAKE_BUILD_TYPE "Debug")
# endif()

message("编译类型：${CMAKE_BUILD_TYPE}")
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_definitions(-D_DEBUG)
  message("定义Debug宏, 类型${CMAKE_BUILD_TYPE}。")
else()
  add_definitions(-D_RELEASE)
  message("定义Rlease宏, 类型${CMAKE_BUILD_TYPE}。")
  set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 
  set(CMAKE_INSTALL_RPATH "\${ORIGIN}/../lib") #指定运行时动态库的加载路径,ORIGIN指运行文件所在目录
endif()

if (WIN32)
  add_definitions(-DUNICODE -D_UNICODE)
  if(MSVC)
    add_compile_options("/utf-8")
  endif()
elseif(UNIX)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT ANDROID)
    add_compile_options(-stdlib=libc++)
    # Presumably need the above for linking too, maybe other options missing as well
    add_link_options(-stdlib=libc++)   # New command on CMake master, not in 3.12 release
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexperimental-library")  # std::format
  endif()
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

target_link_libraries(${target_name} PUBLIC
  pluginmgr
  Qt6::Widgets
)

if(MSVC)
  target_compile_options(${target_name} PRIVATE /W4 /WX)
else()
  target_compile_options(${target_name} PRIVATE -Wall -Wextra -pedantic) # -Werror 
endif()

install(TARGETS ${target_name})
