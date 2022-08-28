cmake_minimum_required(VERSION 3.15)

include_directories(include)

add_library(network
   src/httplib.cpp
)

# set(Boost_USE_STATIC_LIBS ON)
# set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_INCLUDE_DIR ${_VCPKG_INSTALLED_DIR}/x64-windows/include)
# set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_package(Boost COMPONENTS system REQUIRED)
if(MSVC)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
endif ()

if(WIN32)
  target_link_libraries(network
    Crypt32
    Winhttp
    WinInet
  )
elseif(UNIX)
else()
endif()
