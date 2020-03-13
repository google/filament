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

target_link_libraries(${TARGET} PUBLIC image math png tinyexr utils z astcenc stb EtcLib)
if (WIN32)
    target_link_libraries(${TARGET} PRIVATE wsock32)
endif()

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
