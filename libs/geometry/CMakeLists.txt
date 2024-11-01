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
        src/MikktspaceImpl.cpp
        src/SurfaceOrientation.cpp
        src/TangentSpaceMesh.cpp
        src/Transcoder.cpp
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

set(GEOMETRY_DEPS
    meshoptimizer
    mikktspace
)

target_link_libraries(${TARGET} PUBLIC math utils)
target_link_libraries(${TARGET} PRIVATE ${GEOMETRY_DEPS})

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)

# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (MSVC)
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/fp:fast>)
else()
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:-ffast-math -fno-finite-math-only>)
    target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)
endif()

# ==================================================================================================
# Installation
# ==================================================================================================

# No need to install since we're combining this lib and the dependent libs into a combined lib
# install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})

install(DIRECTORY ${PUBLIC_HDR_DIR}/geometry DESTINATION include)

set(COMBINED_DEPS
    ${TARGET}
    ${GEOMETRY_DEPS}
)

# Combine the deps into a single static lib so that client only have to link this lib and not have
# to link its dependencies.
set(GEOMETRY_COMBINED_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libgeometry_combined.a")
combine_static_libs(${TARGET} "${GEOMETRY_COMBINED_OUTPUT}" "${COMBINED_DEPS}")

set(GEOMETRY_LIB_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}geometry${CMAKE_STATIC_LIBRARY_SUFFIX})
install(FILES "${GEOMETRY_COMBINED_OUTPUT}" DESTINATION lib/${DIST_DIR} RENAME ${GEOMETRY_LIB_NAME})

# ==================================================================================================
# Tests
# ==================================================================================================
if(FILAMENT_TESTS AND NOT ANDROID AND NOT WEBGL AND NOT IOS)
    set(TARGET test_transcoder)
    add_executable(${TARGET} tests/test_transcoder.cpp)
    target_link_libraries(${TARGET} PRIVATE geometry gtest)
    set_target_properties(${TARGET} PROPERTIES FOLDER Tests)

    set(TARGET test_tangent_space_mesh)
    add_executable(${TARGET} tests/test_tangent_space_mesh.cpp)
    target_link_libraries(${TARGET} PRIVATE geometry gtest)
    set_target_properties(${TARGET} PROPERTIES FOLDER Tests)
endif()
