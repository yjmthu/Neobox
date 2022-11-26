cmake_minimum_required(VERSION 3.10.1)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)
qt_standard_project_setup()

file(GLOB plugin_subdirs */CMakeLists.txt)

foreach(plugin_subdir_cmake ${plugin_subdirs})
  get_filename_component(subdir_name ${plugin_subdir_cmake} DIRECTORY)
  add_subdirectory(${subdir_name})
endforeach()

# add_subdirectory(neosystem_core)
# add_subdirectory(neosystem_qt)