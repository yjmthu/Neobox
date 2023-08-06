cmake_minimum_required(VERSION 3.10.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# if(UNIX AND NOT MACOS)
#   find_package(KF5WindowSystem REQUIRED)
# endif()

# find_package(Qt6 REQUIRED COMPONENTS UiTools)
file(GLOB source_files src/*.cpp)

if(WIN32)
  set(system_shared_files Shell32)
elseif(UNIX)
  # Qt6 和 KF5冲突
  # set(system_shared_files KF5::WindowSystem)
  set(system_shared_files "")
endif()
set(qt_shared_files Qt6::Core Qt6::Widgets)
set(pliugin_libfiles neospeedbox_core pluginmgr)

add_library(neospeedboxplg SHARED ${source_files})

set(resource_files
  fonts/*.ttf
  icons/*.png
  # dynamic/*.dll
  # dynamic/*.so
)
load_resources(neospeedboxplg resource_files)

set(speedbox_libs
  ${pliugin_libfiles}
  ${system_shared_files}
)

# ===== 编译皮肤 ======

set(SPEEDBOX_SKIN_FLAGS "-DSPEEDBOX_SKIN_STATIC" )

# 找到所有CPP文件
file(GLOB all_cpp_files skins/*.cpp)
foreach(full_cpp_filepath ${all_cpp_files})
  # 正则替换掉cpp后缀
  string(REGEX REPLACE "\\.cpp$" ".ui" full_ui_filepath ${full_cpp_filepath})
  # 判断对应的UI文件是否存在
  if (EXISTS ${full_ui_filepath})
    qt_wrap_ui(wrap_file ${full_ui_filepath}) # 根据UI文件生成头文件
  endif()
  # 获取不带扩展的文件名称
  get_filename_component(file_name ${full_cpp_filepath} NAME_WE)
  # 将CPP文件和对应的皮肤编译动态库
  add_library(${file_name} STATIC
    skins/${file_name}.cpp
    ${wrap_file}
  )
  target_link_libraries(${file_name} ${qt_shared_files})
  set_target_properties(${file_name} PROPERTIES COMPILE_FLAGS ${SPEEDBOX_SKIN_FLAGS})
  # 添加到主程序库列表
  list(APPEND speedbox_libs ${file_name})
  # # 链接动态库
  # target_link_libraries(${file_name}
  #   neospeedboxplg
  #   ${pliugin_libfiles}
  #   ${qt_shared_files}
  # )
  # # 安装目录
  # install(TARGETS ${file_name}
  #   LIBRARY DESTINATION skins
  #   ARCHIVE DESTINATION skins
  #   RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/dynamic
  #   PUBLIC_HEADER DESTINATION skins
  # )
  # file(COPY ${CMAKE_CURRENT_BINARY_DIR}/${file_name}.dll
  #   DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/dlls/${file_name}.dll FOLLOW_SYMLINK_CHAIN)
endforeach()

target_link_libraries(neospeedboxplg PUBLIC
  ${speedbox_libs}
)

install_plugin(neospeedboxplg)