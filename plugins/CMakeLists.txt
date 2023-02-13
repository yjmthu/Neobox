cmake_minimum_required(VERSION 3.10.1)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)
qt_standard_project_setup()

set(plugin_list
  neoocrplg
  neospeedboxplg
  neosystemplg
  neotranslateplg
  neowallpaperplg
  # neousbplg
  # neocolorplg
)

function(include_sub_directories_recursively root_dir)
  if (IS_DIRECTORY ${root_dir})   # 当前路径是一个目录吗，是的话就加入到包含目录
    if (${root_dir} MATCHES "include")
      message("include dir: " ${root_dir})
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

# file(GLOB plugin_subdirs */CMakeLists.txt)
set(plugin_subdirs
  neosystem_qt
  neospeedbox_core
  neospeedbox_qt
  neoocr_core
  neoocr_qt
  neowallpaper_core
  neowallpaper_qt
  neotranslate_core
  neotranslate_qt
)

foreach(subdir_name ${plugin_subdirs})
  # get_filename_component(subdir_name ${plugin_subdir_cmake} DIRECTORY)
  # add_subdirectory(${subdir_name})
  add_subdirectory(${subdir_name})
endforeach()

# add_subdirectory(neowallpaper_core)
# add_subdirectory(neowallpaper_qt)

set(install_dir ${PROJECT_SOURCE_DIR}/install)

install(TARGETS ${plugin_list}
  LIBRARY DESTINATION ${install_dir}/lib
  ARCHIVE DESTINATION ${install_dir}/lib
  RUNTIME DESTINATION ${install_dir}/plugins
  PUBLIC_HEADER DESTINATION ${install_dir}include
)
