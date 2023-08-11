cmake_minimum_required(VERSION 3.10.1)

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/apikey.cpp")
  add_definitions(-DWEATHER_WRONG_USER_KEY)
endif()

file(GLOB source_files src/*.cpp include/*hpp)

set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui)
set(pliugin_libfiles pluginmgr)

set(plugin_name weather)
add_library(${plugin_name} SHARED ${source_files})

target_include_directories(${plugin_name} PRIVATE include)

set(resource_files fonts/*.ttf fonts/*.json styles/*.qss icons/*.png)
load_resources(${plugin_name} resource_files)

target_link_libraries(${plugin_name} PRIVATE
  ${pliugin_libfiles}
  ${qt_shared_files}
)

install_plugin(${plugin_name})
