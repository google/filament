cmake_minimum_required(VERSION 3.1)
project(filamat)

set(TARGET         filamat)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(HDRS
        include/filamat/MaterialBuilder.h
        include/filamat/Package.h)

set(PRIVATE_HDRS
        src/eiff/BlobDictionary.h
        src/eiff/Chunk.h
        src/eiff/ChunkContainer.h
        src/eiff/DictionaryGlslChunk.h
        src/eiff/DictionarySpirvChunk.h
        src/eiff/Flattener.h
        src/eiff/GlslChunk.h
        src/eiff/LineDictionary.h
        src/eiff/MaterialGlslChunk.h
        src/eiff/MaterialInterfaceBlockChunk.h
        src/eiff/MaterialSpirvChunk.h
        src/eiff/ShaderEntry.h
        src/eiff/SimpleFieldChunk.h
        src/shaders/Shaders.h)

set(SRCS
        src/eiff/BlobDictionary.cpp
        src/eiff/Chunk.cpp
        src/eiff/ChunkContainer.cpp
        src/eiff/DictionaryGlslChunk.cpp
        src/eiff/DictionarySpirvChunk.cpp
        src/eiff/LineDictionary.cpp
        src/eiff/MaterialGlslChunk.cpp
        src/eiff/MaterialSpirvChunk.cpp
        src/eiff/MaterialInterfaceBlockChunk.cpp
        src/eiff/SimpleFieldChunk.cpp
        src/shaders/CodeGenerator.cpp
        src/shaders/ShaderGenerator.cpp
        src/MaterialBuilder.cpp)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})
include_directories(${CMAKE_BINARY_DIR})

add_library(${TARGET} STATIC ${HDRS} ${PRIVATE_HDRS} ${SRCS})
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

target_link_libraries(${TARGET} shaders filabridge filaflat utils)

# ==================================================================================================
# Compiler flags
# ==================================================================================================
target_compile_options(${TARGET} PRIVATE
# TODO: use hidden by default and expose what we need (we'll need to do the same in dependencies)
#        -fvisibility=hidden
        $<$<PLATFORM_ID:Linux>:-fPIC>
)

# ==================================================================================================
# Installation
# ==================================================================================================
# Currently disabled because we don't need it
# To proper support Vulkan, filamat needs to support SPIR-V compilation
# install(DIRECTORY ${PUBLIC_HDR_DIR}/filamat DESTINATION include)
# install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
