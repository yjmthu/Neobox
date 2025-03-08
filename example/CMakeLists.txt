cmake_minimum_required(VERSION 3.10.1)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "None")
  message("---- skip build example ----")
  return()
endif()

include_directories(include)

file(GLOB sources src/*.cpp)

foreach(item ${sources})
  string(REGEX MATCH "src/.*" target_name ${item})
  string(REGEX REPLACE "src/" "test_" target_name ${target_name})
  string(REGEX REPLACE "/|\\.cpp$" "" target_name ${target_name})
  add_executable(${target_name} ${item})
  target_link_libraries(${target_name} pluginmgr)
endforeach()

# check if exsists "netbian/cookie.cpp"

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/netbian/cookie.cpp")
  add_definitions(-DNO_NETBIAN_COOKIE)
endif()

add_executable(netbian netbian/main.cpp)
target_link_libraries(netbian pluginmgr)

install(TARGETS netbian
  RUNTIME DESTINATION bin
)

if (WIN32)
  file(GLOB time_sources time/*.cpp)
  set(time_apps)

  foreach(absFilePath ${time_sources})
    # file(BASENAME target_name ${absFilePath})
    get_filename_component(target_name ${absFilePath} NAME_WE)
    list(APPEND time_apps ${target_name})
    add_executable(${target_name} ${absFilePath})
    target_link_libraries(${target_name} pluginmgr)
  endforeach()
endif()

if(UNIX)
  add_executable(x11_test src/x11_test.cc)
  target_link_libraries(x11_test PUBLIC X11)
elseif(WIN32)
  add_executable(tunet tunet/main.cpp tunet/portal.cpp)
  target_include_directories(tunet PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tunet)
  target_link_libraries(tunet pluginmgr)
  install(TARGETS tunet test_proc test_print
    RUNTIME DESTINATION bin
  )

endif()

if (MSVC)
  foreach(item ${time_apps})
    # check if item is "timeapi"
    if (NOT "${item}" STREQUAL "timeapi")
      set_target_properties(${item} PROPERTIES LINK_FLAGS
        "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:CONSOLE"
      )
    else()
      set_target_properties(${item} PROPERTIES LINK_FLAGS
        "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:WINDOWS"
      )
    endif()
  endforeach()
  
  install(TARGETS ${time_apps}
    RUNTIME DESTINATION bin
  )
endif()