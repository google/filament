cmake_minimum_required(VERSION 3.19)
project(glslkomi)

set(TARGET         glslkomi)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(HDRS
        include/glslkomi/Komi.h
        include/glslkomi/Debug.h)

set(PRIVATE_HDRS)

set(SRCS
        src/Komi.cpp
        src/Debug.cpp)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})
include_directories(${CMAKE_BINARY_DIR})

# glslkomi
add_library(${TARGET} STATIC ${HDRS} ${PRIVATE_HDRS} ${SRCS})
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)
target_link_libraries(${TARGET} backend_headers math utils)

# We are being naughty and accessing private headers here
# For spirv-tools, we're just following glslang's example
target_include_directories(${TARGET} PRIVATE ${spirv-tools_SOURCE_DIR}/include)

# glslang libraries have circular dependencies. To make sure the proper object are part of the link
# we need to force archive re-scan on new symbol dependencies via start/end-group.
# Read more about this here https://eli.thegreenplace.net/2013/07/09/library-order-in-static-linking
if (APPLE OR MSVC)
    target_link_libraries(${TARGET} glslang SPIRV SPIRV-Tools-opt spirv-cross-glsl)
else()
    target_link_libraries(${TARGET}
            -Wl,--start-group glslang SPIRV SPIRV-Tools-opt spirv-cross-glsl -Wl,--end-group)
endif()

# ==================================================================================================
# Compiler flags
# ==================================================================================================
# this must match options enabled in glslang's CMakeLists.txt
target_compile_options(${TARGET} PRIVATE -DAMD_EXTENSIONS -DNV_EXTENSIONS )

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0 /Zc:__cplusplus")
endif()

if (FILAMENT_ENABLE_MATDBG)
    add_definitions(-DFILAMENT_ENABLE_MATDBG)
endif()

# ==================================================================================================
# Installation
# ==================================================================================================

# Glslkomi has dependencies on a bunch of SPIRV-related libraries. To make things simpler, we bundle
# them together into a single shared library and copy this into the installation folder. This
# requires us to explicitly list the dependencies below, as CMake doesn't have a way to recursively
# query dependencies.
set(GLSLKOMI_DEPS
        OGLCompiler
        OSDependent
        SPIRV
        SPIRV-Tools
        SPIRV-Tools-opt
        glslkomi
        glslang
        spirv-cross-core
        spirv-cross-glsl
        spirv-cross-msl
        )

set(GLSLKOMI_COMBINED_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libglslkomi_combined.a")
combine_static_libs(glslkomi "${GLSLKOMI_COMBINED_OUTPUT}" "${GLSLKOMI_DEPS}")

set(GLSLKOMI_LIB_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}glslkomi${CMAKE_STATIC_LIBRARY_SUFFIX})
install(FILES "${GLSLKOMI_COMBINED_OUTPUT}" DESTINATION lib/${DIST_DIR} RENAME ${GLSLKOMI_LIB_NAME})
install(DIRECTORY ${PUBLIC_HDR_DIR}/glslkomi DESTINATION include)

# ==================================================================================================
# Tests
# ==================================================================================================
project(test_glslkomi)
set(TARGET test_glslkomi)
set(SRCS
        tests/test_glslkomi.cpp)

add_executable(${TARGET} ${SRCS})

target_include_directories(${TARGET} PRIVATE src)

target_link_libraries(${TARGET} glslkomi gtest)

set_target_properties(${TARGET} PROPERTIES FOLDER Tests)
