cmake_minimum_required(VERSION 3.16)
project(redpanda-win-git-askpass VERSION 1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)

qt_add_executable(redpanda-win-git-askpass WIN32
    main.cpp
)
target_compile_definitions(redpanda-win-git-askpass PRIVATE
    -UNICODE
)

target_link_libraries(redpanda-win-git-askpass PRIVATE
    Qt::Core
    Qt::Gui
)

if(MSVC)
    target_link_libraries(redpanda-win-git-askpass PRIVATE
        advapi32.lib
        comdlg32.lib
        gdi32.lib
        shell32.lib
        user32.lib
        ws2_32.lib
    )
endif()

install(TARGETS redpanda-win-git-askpass
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
