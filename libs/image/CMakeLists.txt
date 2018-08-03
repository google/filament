cmake_minimum_required(VERSION 3.1)
project(images)

set(TARGET image)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
file(GLOB_RECURSE PUBLIC_HDRS ${PUBLIC_HDR_DIR}/${TARGET}/*.h)

set(SRCS
        src/Image.cpp
        src/ImageDecoder.cpp
        src/ImageEncoder.cpp
        src/utilities.h
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_link_libraries(${TARGET} PUBLIC math png tinyexr utils z)
if (WIN32)
    target_link_libraries(${TARGET} PRIVATE wsock32)
endif()

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

# ==================================================================================================
# Compiler flags
# ==================================================================================================
target_compile_options(${TARGET} PRIVATE
        -Wno-deprecated-register
        $<$<CONFIG:Release>:-ffast-math>
)
