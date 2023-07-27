cmake_minimum_required(VERSION 3.21.2)

# 写在所有project之前，对所有project生效
if(UNIX)
  set(CMAKE_C_COMPILER /usr/bin/clang)
  set(CMAKE_CXX_COMPILER /usr/bin/clang++)
endif()

# C++20标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 导出动态库
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# 导出ninja编译命令
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# 安装目录
set(CMAKE_INSTALL_PREFIX "install")

string(TIMESTAMP COMPILE_TIME %Y-%m-%d)
set(NEOBOX_BUILD_TIME ${COMPILE_TIME})

# list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.4.2/msvc2019_64")

project(neobox VERSION 2.3.10 LANGUAGES CXX C)

if (NOT CMAKE_BUILD_TYPE)
  message("编译类型未定义，默认设置为Debug")
  set(CMAKE_BUILD_TYPE "Debug")
endif()

message("编译类型：${CMAKE_BUILD_TYPE}")
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_definitions(-D_DEBUG)
  message("定义Debug宏, 类型${CMAKE_BUILD_TYPE}。")
else()
  add_definitions(-D_RELEASE)
  message("定义Rlease宏, 类型${CMAKE_BUILD_TYPE}。")
endif()

if(MSVC)
  add_definitions(-DUNICODE -D_UNICODE)
  add_compile_options("/utf-8")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT ANDROID)
  add_compile_options(-stdlib=libc++)
  # Presumably need the above for linking too, maybe other options missing as well
  add_link_options(-stdlib=libc++)   # New command on CMake master, not in 3.12 release
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexperimental-library")  # std::format
endif()

function(load_resources plugin_name files_list)
  file(GLOB _resource_files ${files_list})
  set(resource_files)
  message("- <${plugin_name}> ${files_list}")
  foreach(full_filepath ${_resource_files})
    file(RELATIVE_PATH file_path ${CMAKE_CURRENT_SOURCE_DIR} ${full_filepath})
    # message(STATUS "res: '${file_path}'")
    list(APPEND resource_files ${file_path})
  endforeach()

  qt_add_resources(${plugin_name} "${plugin_name}"
    PREFIX "/"
    FILES ${resource_files}
  )
  message("- <${plugin_name}>")
endfunction()

link_directories(build/pluginmgr)
include_directories(pluginmgr/include)
include_directories(thirdlib/YJson/include)
include_directories(thirdlib/zip/src)

add_subdirectory(thirdlib/YJson yjson)
add_subdirectory(thirdlib/zip zip)
add_subdirectory(pluginmgr)
add_subdirectory(plugins)
add_subdirectory(example)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
add_executable(neobox WIN32 main.cpp logo.rc)

target_link_libraries(neobox PUBLIC
  pluginmgr
  Qt6::Widgets
)

install(TARGETS neobox
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  PUBLIC_HEADER DESTINATION include
)
