cmake_minimum_required(VERSION 3.10.1)

file(GLOB source_files src/*.cpp src/*.ui)

# set(system_shared_files Shell32)
set(qt_shared_files Qt6::Core Qt6::Widgets Qt6::Gui)
set(pliugin_libfiles neobox_core yjson)

add_library(neohotkeyplg SHARED ${source_files})

target_link_libraries(neohotkeyplg PRIVATE
  ${pliugin_libfiles}
  ${qt_shared_files}
  ${system_shared_files}
)
