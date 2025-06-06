cmake_minimum_required(VERSION 3.16)
project(consolepauser VERSION 1.0 LANGUAGES CXX)

add_executable(consolepauser)

if(WIN32)
    target_sources(consolepauser PUBLIC
        main.windows.cpp
    )

    target_compile_definitions(consolepauser PRIVATE
        _WIN32_WINNT=0x0501
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

if(LINUX)
    target_link_libraries(consolepauser PRIVATE
        rt
    )
endif()

install(TARGETS consolepauser
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
