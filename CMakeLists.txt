cmake_minimum_required(VERSION 3.5)

project(NBody LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

add_executable(NBody
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    openglscenewidget.cpp
    openglscenewidget.h
    openglsceneresources.qrc
    nbodysim2d.h
    nbodysim2d.cpp
    nbodysim2dresources.qrc
)

target_link_libraries(NBody PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    ${CMAKE_SOURCE_DIR}/CL/OpenCL.lib
    Opengl32.lib
)
