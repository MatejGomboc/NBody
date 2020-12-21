cmake_minimum_required(VERSION 3.16)

project(NBody VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets OpenGLWidgets REQUIRED)

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
    Qt6::Widgets
    ${CMAKE_SOURCE_DIR}/CL/OpenCL.lib
    Qt6::OpenGLWidgets
    Opengl32.lib
)
