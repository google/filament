cmake_minimum_required(VERSION 3.19)
project(geometry)

set(TARGET geometry)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/geometry/SurfaceOrientation.h
        include/geometry/TangentSpaceMesh.h
        include/geometry/Transcoder.h
)

set(SRCS
        src/SurfaceOrientation.cpp
        src/TangentSpaceMesh.cpp
        src/Transcoder.cpp
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_link_libraries(${TARGET} PUBLIC math meshoptimizer mikktspace utils)

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)

# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (MSVC)
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/fp:fast>)
else()
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:-ffast-math>)
    target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)
endif()

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
install(DIRECTORY ${PUBLIC_HDR_DIR}/geometry DESTINATION include)

# ==================================================================================================
# Tests
# ==================================================================================================
if (NOT ANDROID AND NOT WEBGL AND NOT IOS)
    set(TARGET test_transcoder)
    add_executable(${TARGET} tests/test_transcoder.cpp)
    target_link_libraries(${TARGET} PRIVATE geometry gtest)
    set_target_properties(${TARGET} PROPERTIES FOLDER Tests)

    set(TARGET test_tangent_space_mesh)
    add_executable(${TARGET} tests/test_tangent_space_mesh.cpp)
    target_link_libraries(${TARGET} PRIVATE geometry gtest)
    set_target_properties(${TARGET} PROPERTIES FOLDER Tests)
endif()
