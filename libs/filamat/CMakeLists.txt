cmake_minimum_required(VERSION 3.1)
project(filamat)

set(TARGET         filamat)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(HDRS
        include/filamat/Enums.h
        include/filamat/MaterialBuilder.h
        include/filamat/Package.h
        include/filamat/PostprocessMaterialBuilder.h)

set(PRIVATE_HDRS
        src/eiff/BlobDictionary.h
        src/eiff/Chunk.h
        src/eiff/ChunkContainer.h
        src/eiff/DictionaryTextChunk.h
        src/eiff/DictionarySpirvChunk.h
        src/eiff/Flattener.h
        src/eiff/TextChunk.h
        src/eiff/LineDictionary.h
        src/eiff/MaterialTextChunk.h
        src/eiff/MaterialInterfaceBlockChunk.h
        src/eiff/MaterialSpirvChunk.h
        src/eiff/ShaderEntry.h
        src/eiff/SimpleFieldChunk.h
        src/sca/ASTHelpers.h
        src/sca/GLSLTools.h
        src/sca/builtinResource.h
        src/GLSLPostProcessor.h)

set(SRCS
        src/eiff/BlobDictionary.cpp
        src/eiff/Chunk.cpp
        src/eiff/ChunkContainer.cpp
        src/eiff/DictionaryTextChunk.cpp
        src/eiff/DictionarySpirvChunk.cpp
        src/eiff/LineDictionary.cpp
        src/eiff/MaterialTextChunk.cpp
        src/eiff/MaterialSpirvChunk.cpp
        src/eiff/MaterialInterfaceBlockChunk.cpp
        src/eiff/SimpleFieldChunk.cpp
        src/sca/ASTHelpers.cpp
        src/sca/GLSLTools.cpp
        src/shaders/CodeGenerator.cpp
        src/shaders/ShaderGenerator.cpp
        src/Enums.cpp
        src/GLSLPostProcessor.cpp
        src/MaterialBuilder.cpp
        src/PostprocessMaterialBuilder.cpp)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})
include_directories(${CMAKE_BINARY_DIR})

add_library(${TARGET} STATIC ${HDRS} ${PRIVATE_HDRS} ${SRCS})
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

target_link_libraries(${TARGET} shaders filabridge filaflat utils smol-v)

# We are being naughty and accessing private headers here
# For spirv-tools, we're just following glslang's example
target_include_directories(${TARGET} PRIVATE ${spirv-tools_SOURCE_DIR}/include)

# glslang libraries have circular dependencies. To make sure the proper object are part of the link
# we need to force archive re-scan on new symbol dependencies via start/end-group.
# Read more about this here https://eli.thegreenplace.net/2013/07/09/library-order-in-static-linking
if (APPLE)
    target_link_libraries(${TARGET} glslang SPIRV SPVRemapper SPIRV-Tools-opt spirv-cross-glsl)
else()
    target_link_libraries(${TARGET}
            -Wl,--start-group glslang SPIRV SPVRemapper SPIRV-Tools-opt spirv-cross-glsl -Wl,--end-group)
endif()

# ==================================================================================================
# Compiler flags
# ==================================================================================================
# this must match options enabled in glslang's CMakeLists.txt
target_compile_options(${TARGET} PRIVATE -DAMD_EXTENSIONS -DNV_EXTENSIONS )

target_compile_options(${TARGET} PRIVATE
# TODO: use hidden by default and expose what we need (we'll need to do the same in dependencies)
#        -fvisibility=hidden
        $<$<PLATFORM_ID:Linux>:-fPIC>
)

# ==================================================================================================
# Installation
# ==================================================================================================
# Currently disabled because we don't need it
# To properly support Vulkan, filamat needs to support SPIR-V compilation
# install(DIRECTORY ${PUBLIC_HDR_DIR}/filamat DESTINATION include)
# install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})

# ==================================================================================================
# Tests
# ==================================================================================================
project(test_filamat)
set(TARGET test_filamat)
set(SRCS
        tests/test_filamat.cpp)

add_executable(${TARGET} ${SRCS})

target_include_directories(${TARGET} PRIVATE src)

target_link_libraries(${TARGET} filamat gtest)
