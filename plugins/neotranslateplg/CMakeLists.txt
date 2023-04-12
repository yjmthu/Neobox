cmake_minimum_required(VERSION 3.10.1)

file(GLOB source_files src/*.cpp include/*.hpp)

# set(system_shared_files Shell32)
set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui)
set(pliugin_libfiles translate_core pluginmgr)

add_library(neotranslateplg SHARED
  ${source_files}
)

set(resource_files icons/*.png)
load_resources(neotranslateplg ${resource_files})

target_link_libraries(neotranslateplg PUBLIC
  ${pliugin_libfiles}
  ${qt_shared_files}
  # ${system_shared_files}
)
