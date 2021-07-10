cmake_minimum_required(VERSION 3.19.2)

project(OpenCL)

add_library(OpenCL SHARED IMPORTED GLOBAL)

set_target_properties(OpenCL PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_SOURCE_DIR}/include/")

set_target_properties(OpenCL PROPERTIES IMPORTED_IMPLIB_DEBUG "${PROJECT_SOURCE_DIR}/lib/Windows/Debug/OpenCL.lib")
set_target_properties(OpenCL PROPERTIES IMPORTED_LOCATION_DEBUG "${PROJECT_SOURCE_DIR}/lib/Windows/Debug/OpenCL.dll")
set_target_properties(OpenCL PROPERTIES IMPORTED_IMPLIB_RELEASE "${PROJECT_SOURCE_DIR}/lib/Windows/Release/OpenCL.lib")
set_target_properties(OpenCL PROPERTIES IMPORTED_LOCATION_RELEASE "${PROJECT_SOURCE_DIR}/lib/Windows/Release/OpenCL.dll")
set_target_properties(OpenCL PROPERTIES IMPORTED_IMPLIB_MINSIZEREL "${PROJECT_SOURCE_DIR}/lib/Windows/MinSizeRel/OpenCL.lib")
set_target_properties(OpenCL PROPERTIES IMPORTED_LOCATION_MINSIZEREL "${PROJECT_SOURCE_DIR}/lib/Windows/MinSizeRel/OpenCL.dll")
set_target_properties(OpenCL PROPERTIES IMPORTED_IMPLIB_RELWITHDEBINFO "${PROJECT_SOURCE_DIR}/lib/Windows/RelWithDebInfo/OpenCL.lib")
set_target_properties(OpenCL PROPERTIES IMPORTED_LOCATION_RELWITHDEBINFO "${PROJECT_SOURCE_DIR}/lib/Windows/RelWithDebInfo/OpenCL.dll")

add_custom_target(OpenCLMkdir
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OPENCL_COPY_DIR}/
    VERBATIM
)

add_custom_target(OpenCLCopy
    WORKING_DIRECTORY ${OPENCL_COPY_DIR}/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/lib/Windows/$<CONFIG>/OpenCL.dll"
        "$<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/lib/Windows/$<CONFIG>/OpenCL.pdb>"
        "$<$<CONFIG:RelWithDebInfo>:${PROJECT_SOURCE_DIR}/lib/Windows/$<CONFIG>/OpenCL.pdb>"
        ${OPENCL_COPY_DIR}/
    BYPRODUCTS OpenCL.dll OpenCL.pdb
    VERBATIM
)

add_dependencies(OpenCLCopy
    OpenCLMkdir
)
