cmake_minimum_required(VERSION 3.10)

add_definitions(-DCPPHTTPLIB_OPENSSL_SUPPORT)

include_directories(include)

add_library(httplib STATIC
    src/httplib.cc
)

if(WIN32)
    set(SSL_LIBS
        libssl_static
        libcrypto_static
        Wininet
    )
elseif(UNIX)
    set(SSL_LIBS
        ssl
        crypto
    )
endif()

target_link_libraries(httplib ${SSL_LIBS})

