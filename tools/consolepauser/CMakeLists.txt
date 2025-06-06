cmake_minimum_required(VERSION 3.16)
project(consolepauser VERSION 1.0 LANGUAGES CXX)

add_executable(consolepauser)

if(WIN32)
    target_sources(consolepauser PUBLIC
        main.windows.cpp
    )

    target_link_libraries(consolepauser PRIVATE
        psapi
    )
endif()

if(UNIX)
    target_sources(consolepauser PUBLIC
        main.unix.cpp
    )
endif()

install(TARGETS consolepauser
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
