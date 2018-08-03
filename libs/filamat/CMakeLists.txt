cmake_minimum_required(VERSION 3.1)
project(filamat)

set(TARGET         filamat)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
file(GLOB_RECURSE HDRS include/filamat/*.h)
file(GLOB_RECURSE PRIVATE_HDRS src/*.h)

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

target_link_libraries(${TARGET} filabridge filaflat utils)

add_dependencies(${TARGET} shaders)

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
install(DIRECTORY ${PUBLIC_HDR_DIR}/filamat DESTINATION include)
install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
