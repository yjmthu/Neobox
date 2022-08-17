cmake_minimum_required(VERSION 3.15)

include_directories(include)

add_library(network
   src/httplib.cpp
)

if(WIN32)
  target_link_libraries(network
    Crypt32
    Winhttp
    WinInet
#    libssl
#    libcrypto
  )
elseif(UNIX)
else()
endif()
