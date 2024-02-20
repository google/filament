cmake_minimum_required(VERSION 3.19)
project(astrict)

set(TARGET         astrict)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(HDRS
        include/astrict/CommonTypes.h
        include/astrict/DebugCommon.h
        include/astrict/DebugGlsl.h
        include/astrict/FromGlsl.h
        include/astrict/GlslTypes.h
        include/astrict/ToGlsl.h)

set(PRIVATE_HDRS)

set(SRCS
        src/DebugCommon.cpp
        src/DebugGlsl.cpp
        src/FromGlsl.cpp
        src/ToGlsl.cpp)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})
include_directories(${CMAKE_BINARY_DIR})

# astrict
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

# Astrict has dependencies on a bunch of SPIRV-related libraries. To make things simpler, we bundle
# them together into a single shared library and copy this into the installation folder. This
# requires us to explicitly list the dependencies below, as CMake doesn't have a way to recursively
# query dependencies.
set(ASTRICT_DEPS
        OGLCompiler
        OSDependent
        SPIRV
        SPIRV-Tools
        SPIRV-Tools-opt
        astrict
        glslang
        spirv-cross-core
        spirv-cross-glsl
        spirv-cross-msl
        )

set(ASTRICT_COMBINED_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libastrict_combined.a")
combine_static_libs(astrict "${ASTRICT_COMBINED_OUTPUT}" "${ASTRICT_DEPS}")

set(ASTRICT_LIB_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}astrict${CMAKE_STATIC_LIBRARY_SUFFIX})
install(FILES "${ASTRICT_COMBINED_OUTPUT}" DESTINATION lib/${DIST_DIR} RENAME ${ASTRICT_LIB_NAME})
install(DIRECTORY ${PUBLIC_HDR_DIR}/astrict DESTINATION include)

# ==================================================================================================
# Tests
# ==================================================================================================
project(test_astrict)
set(TARGET test_astrict)
set(SRCS
        tests/test_FromGlsl.cpp)

add_executable(${TARGET} ${SRCS})

target_include_directories(${TARGET} PRIVATE src)

target_link_libraries(${TARGET} astrict gtest)

set_target_properties(${TARGET} PROPERTIES FOLDER Tests)
