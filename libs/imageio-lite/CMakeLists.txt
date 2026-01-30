cmake_minimum_required(VERSION 3.19)
project(imageio-lite)

set(TARGET imageio-lite)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/imageio-lite/ImageDecoder.h
        include/imageio-lite/ImageEncoder.h
)

set(SRCS
        src/ImageDecoder.cpp
        src/ImageEncoder.cpp
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)

target_link_libraries(${TARGET} PUBLIC image math utils)

# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (NOT MSVC)
    target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)
endif()

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
install(DIRECTORY ${PUBLIC_HDR_DIR}/imageio-lite DESTINATION include)

# ==================================================================================================
# Tests
# ==================================================================================================
if (NOT ANDROID AND NOT WEBGL AND NOT IOS)
    add_executable(test_${TARGET} tests/test_imageio-lite.cpp)
    target_link_libraries(test_${TARGET} PRIVATE ${TARGET} image gtest)
    set_target_properties(test_${TARGET} PROPERTIES FOLDER Tests)
endif()
