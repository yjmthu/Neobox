cmake_minimum_required(VERSION 3.10.1)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

function(include_sub_directories_recursively root_dir)
  if (IS_DIRECTORY ${root_dir})   # 当前路径是一个目录吗，是的话就加入到包含目录
    if (${root_dir} MATCHES "include")
      # message("include dir: " ${root_dir})
      include_directories(${root_dir})
    endif()
  endif()

  file(GLOB ALL_SUB RELATIVE ${root_dir} ${root_dir}/*) # 获得当前目录下的所有文件，让如ALL_SUB列表中
  foreach(sub ${ALL_SUB})
    if (IS_DIRECTORY ${root_dir}/${sub})
      include_sub_directories_recursively(${root_dir}/${sub}) # 对子目录递归调用，包含
    endif()
  endforeach()
endfunction()

include_sub_directories_recursively(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DMYSHAREDLIB_LIBRARY)

set(install_dir ${PROJECT_SOURCE_DIR}/install)

function(install_plugin plugin_name)
  install(TARGETS ${plugin_name}
    LIBRARY DESTINATION ${install_dir}/lib
    ARCHIVE DESTINATION ${install_dir}/lib
    RUNTIME DESTINATION ${install_dir}/plugins
    PUBLIC_HEADER DESTINATION ${install_dir}include
  )
endfunction()

file(GLOB child_dirs ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(subdirectory ${child_dirs})
  if(IS_DIRECTORY ${subdirectory})
    get_filename_component(plugin_name ${subdirectory} NAME)
    add_subdirectory(${plugin_name})
  endif()
endforeach()

