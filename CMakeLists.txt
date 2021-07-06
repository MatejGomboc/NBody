cmake_minimum_required(VERSION 3.19.2)

project(NBody LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(NBody 
    main.cpp
)

target_compile_definitions(NBody PRIVATE
    CL_HPP_MINIMUM_OPENCL_VERSION=110
    CL_HPP_TARGET_OPENCL_VERSION=110
)

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set(OPENCL_COPY_DIR "${PROJECT_BINARY_DIR}/$<CONFIG>")
else()
    set(OPENCL_COPY_DIR ${PROJECT_BINARY_DIR})
endif()

add_subdirectory(OpenCL)

target_link_libraries(NBody PRIVATE
    OpenCL
)

add_dependencies(NBody
    OpenCLCopy
)
