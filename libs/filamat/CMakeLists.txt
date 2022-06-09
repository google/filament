cmake_minimum_required(VERSION 3.19)
project(filamat)

set(TARGET         filamat)
set(PUBLIC_HDR_DIR include)

# filamat is split into two targets: filamat, and filamat_lite.
# filamat_lite forgoes SPIRV-V / MSL output, static analysis, and optimization in favor of a smaller
# binary size.
# Both libraries use the same public header files.

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(HDRS
        include/filamat/Enums.h
        include/filamat/IncludeCallback.h
        include/filamat/MaterialBuilder.h
        include/filamat/Package.h)

set(COMMON_PRIVATE_HDRS
        src/eiff/Chunk.h
        src/eiff/ChunkContainer.h
        src/eiff/DictionaryTextChunk.h
        src/eiff/Flattener.h
        src/eiff/LineDictionary.h
        src/eiff/MaterialTextChunk.h
        src/eiff/MaterialInterfaceBlockChunk.h
        src/eiff/ShaderEntry.h
        src/eiff/SimpleFieldChunk.h
        src/Includes.h)

set(COMMON_SRCS
        src/eiff/Chunk.cpp
        src/eiff/ChunkContainer.cpp
        src/eiff/DictionaryTextChunk.cpp
        src/eiff/LineDictionary.cpp
        src/eiff/MaterialTextChunk.cpp
        src/eiff/MaterialInterfaceBlockChunk.cpp
        src/eiff/SimpleFieldChunk.cpp
        src/shaders/CodeGenerator.cpp
        src/shaders/ShaderGenerator.cpp
        src/Enums.cpp
        src/Includes.cpp
        src/MaterialBuilder.cpp
        src/MaterialVariants.cpp
        src/SamplerBindingMap.cpp
        src/SibGenerator.cpp
        src/UibGenerator.cpp
)

# Sources and headers for filamat

set(PRIVATE_HDRS
        ${COMMON_PRIVATE_HDRS}
        src/eiff/BlobDictionary.h
        src/eiff/DictionarySpirvChunk.h
        src/eiff/MaterialSpirvChunk.h
        src/GLSLPostProcessor.h
        src/ShaderMinifier.h
        src/sca/ASTHelpers.h
        src/sca/GLSLTools.h
        src/sca/builtinResource.h)

set(SRCS
        ${COMMON_SRCS}
        src/eiff/BlobDictionary.cpp
        src/eiff/DictionarySpirvChunk.cpp
        src/eiff/MaterialSpirvChunk.cpp
        src/sca/ASTHelpers.cpp
        src/sca/GLSLTools.cpp
        src/GLSLPostProcessor.cpp
        src/ShaderMinifier.cpp)

# Sources and headers for filamat lite

set(LITE_PRIVATE_HDRS
        ${COMMON_PRIVATE_HDRS}
        src/sca/GLSLToolsLite.h)

set(LITE_SRCS
        ${COMMON_SRCS}
        src/sca/GLSLToolsLite.cpp)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})
include_directories(${CMAKE_BINARY_DIR})

# Filamat
add_library(${TARGET} STATIC ${HDRS} ${PRIVATE_HDRS} ${SRCS})
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)
target_link_libraries(${TARGET} shaders filabridge utils smol-v)

if (CMAKE_COMPILER_IS_GNUCXX)
    set_target_properties(${TARGET} PROPERTIES
                          C_VISIBILITY_PRESET default
                          CXX_VISIBILITY_PRESET default)
endif()

# Filamat Lite
add_library(filamat_lite STATIC ${HDRS} ${LITE_PRIVATE_HDRS} ${LITE_SRCS})
target_include_directories(filamat_lite PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(filamat_lite PROPERTIES FOLDER Libs)
target_link_libraries(filamat_lite shaders filabridge utils)

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

target_compile_definitions(filamat_lite PRIVATE FILAMAT_LITE)

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0 /Zc:__cplusplus")
endif()

# ==================================================================================================
# Installation
# ==================================================================================================

# Filamat has dependencies on a bunch of SPIRV-related libraries. To make things simpler, we bundle
# them together into a single shared library and copy this into the installation folder. This
# requires us to explicitly list the dependencies below, as CMake doesn't have a way to recursively
# query dependencies.
set(FILAMAT_DEPS
        OGLCompiler
        OSDependent
        SPIRV
        SPIRV-Tools
        SPIRV-Tools-opt
        filamat
        glslang
        spirv-cross-core
        spirv-cross-glsl
        spirv-cross-msl
        )

set(FILAMAT_COMBINED_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libfilamat_combined.a")
combine_static_libs(filamat "${FILAMAT_COMBINED_OUTPUT}" "${FILAMAT_DEPS}")

set(FILAMAT_LIB_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}filamat${CMAKE_STATIC_LIBRARY_SUFFIX})
install(FILES "${FILAMAT_COMBINED_OUTPUT}" DESTINATION lib/${DIST_DIR} RENAME ${FILAMAT_LIB_NAME})
install(DIRECTORY ${PUBLIC_HDR_DIR}/filamat DESTINATION include)

install(TARGETS filamat_lite ARCHIVE DESTINATION lib/${DIST_DIR})

# ==================================================================================================
# Tests
# ==================================================================================================
project(test_filamat)
set(TARGET test_filamat)
set(SRCS
        tests/test_filamat.cpp
        tests/test_includes.cpp)

add_executable(${TARGET} ${SRCS})

target_include_directories(${TARGET} PRIVATE src)

target_link_libraries(${TARGET} filamat gtest)

set_target_properties(${TARGET} PROPERTIES FOLDER Tests)

set(TARGET test_filamat_lite)
set(SRCS
        tests/test_filamat_lite.cpp)

add_executable(${TARGET} ${SRCS})

target_include_directories(${TARGET} PRIVATE src)

target_link_libraries(${TARGET} filamat_lite gtest)

set_target_properties(${TARGET} PROPERTIES FOLDER Tests)
