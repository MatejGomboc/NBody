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
    CL_HPP_MINIMUM_OPENCL_VERSION=120
    CL_HPP_TARGET_OPENCL_VERSION=120
    CL_HPP_CL_1_2_DEFAULT_BUILD
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

add_custom_target(CopyOpenCLKernels 
    SOURCES test_kernel.cl
    WORKING_DIRECTORY ${OPENCL_COPY_DIR}/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_SOURCE_DIR}/test_kernel.cl
        ${OPENCL_COPY_DIR}/
    BYPRODUCTS test_kernel.cl
    VERBATIM
)

add_dependencies(NBody
    OpenCLCopy
    CopyOpenCLKernels
)