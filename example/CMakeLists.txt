cmake_minimum_required(VERSION 3.10.1)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "None")
  message("---- skip build example ----")
  return()
endif()

file(GLOB sources src/*.cpp)

foreach(item ${sources})
  string(REGEX MATCH "src/.*" target_name ${item})
  string(REGEX REPLACE "src/" "test_" target_name ${target_name})
  string(REGEX REPLACE "/|\\." "_" target_name ${target_name})
  add_executable(${target_name} ${item})
  target_link_libraries(${target_name} pluginmgr)
endforeach()

if(UNIX)
  add_executable(x11_test src/x11_test.cc)
  target_link_libraries(x11_test PUBLIC X11)
endif()

if (MSVC)
  set(ADMIN_CONSOLE_APPS test_time_fetch_cpp test_shanghai_cpp)
  set(ADMIN_WINDOWS_APPS test_timeapi_cpp)
  set_target_properties(${ADMIN_CONSOLE_APPS} PROPERTIES LINK_FLAGS
    "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:CONSOLE"
  )
  set_target_properties(${ADMIN_WINDOWS_APPS} PROPERTIES LINK_FLAGS
    "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:WINDOWS"
  )
  install(TARGETS ${ADMIN_CONSOLE_APPS} ${ADMIN_WINDOWS_APPS}
    RUNTIME DESTINATION bin
  )
endif()