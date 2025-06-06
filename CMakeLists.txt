cmake_minimum_required(VERSION 3.16)
project(Red_Panda_CPP VERSION 1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Network PrintSupport Svg Widgets Xml)
find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS Gui)


add_subdirectory(RedPandaIDE)
add_subdirectory(tools/consolepauser)
add_subdirectory(libs/redpanda_qt_utils)
add_subdirectory(libs/qsynedit)
add_subdirectory(libs/lua)
