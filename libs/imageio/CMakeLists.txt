cmake_minimum_required(VERSION 3.19)
project(imageio)

set(TARGET imageio)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/imageio/BasisEncoder.h
        include/imageio/HDRDecoder.h
        include/imageio/ImageDecoder.h
        include/imageio/ImageDiffer.h
        include/imageio/ImageEncoder.h
)

set(SRCS
        src/BasisEncoder.cpp
        src/HDRDecoder.cpp
        src/ImageDecoder.cpp
        src/ImageDiffer.cpp
        src/ImageEncoder.cpp
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_include_directories(${TARGET} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/${PUBLIC_HDR_DIR}>
    $<INSTALL_INTERFACE:${PUBLIC_HDR_DIR}>
)
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)

target_link_libraries(${TARGET} PUBLIC image math png tinyexr utils z basis_encoder)
if (WIN32)
    target_link_libraries(${TARGET} PRIVATE wsock32)
endif()

# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (NOT MSVC)
    target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)
endif()
