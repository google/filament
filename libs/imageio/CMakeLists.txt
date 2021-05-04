cmake_minimum_required(VERSION 3.10)
project(imageio)

set(TARGET imageio)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/imageio/BlockCompression.h
        include/imageio/ImageDecoder.h
        include/imageio/ImageDiffer.h
        include/imageio/ImageEncoder.h
)

set(SRCS
        src/BlockCompression.cpp
        src/ImageDecoder.cpp
        src/ImageDiffer.cpp
        src/ImageEncoder.cpp
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

if (HUNTER_ENABLED)
  target_link_libraries(${TARGET} PUBLIC image math PNG::PNG tinyexr::tinyexr utils ZLIB::zlib astc-encoder::astcenc stb::stb etc2comp::EtcLib)
else ()
  target_link_libraries(${TARGET} PUBLIC image math png tinyexr utils z astcenc stb EtcLib)
endif()
if (WIN32)
    target_link_libraries(${TARGET} PRIVATE wsock32)
endif()

# ==================================================================================================
# Transitive macro definitions
# ==================================================================================================
target_compile_definitions(${TARGET} PUBLIC IMAGEIO_SUPPORTS_BLOCK_COMPRESSION)

# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (NOT MSVC)
    target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)
endif()

if (MSVC)
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/fp:fast>)
else()
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:-ffast-math>)
endif()
