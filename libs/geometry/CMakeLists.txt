cmake_minimum_required(VERSION 3.10)
project(geometry)

set(TARGET geometry)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/geometry/SurfaceOrientation.h
        include/geometry/Transcoder.h
)

set(SRCS
        src/SurfaceOrientation.cpp
        src/Transcoder.cpp
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_link_libraries(${TARGET} PUBLIC math utils)

if(HUNTER_ENABLED)
  target_include_directories(${TARGET} PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${PUBLIC_HDR_DIR}>  
    $<INSTALL_INTERFACE:${PUBLIC_HDR_DIR}>)
else()
  target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
endif()

# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (MSVC)
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/fp:fast>)
else()
    target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:-ffast-math>)
    target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)
endif()

# ==================================================================================================
# Installation
# ==================================================================================================
if(HUNTER_ENABLED)
  install(TARGETS ${TARGET} EXPORT ${TARGETS_EXPORT_NAME} ARCHIVE DESTINATION lib/${DIST_DIR})
else()
  install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
endif()

install(DIRECTORY ${PUBLIC_HDR_DIR}/geometry DESTINATION include)

# ==================================================================================================
# Tests
# ==================================================================================================
if (NOT ANDROID AND NOT WEBGL AND NOT IOS)
    add_executable(test_transcoder tests/test_transcoder.cpp)
    if(HUNTER_ENABLED)
      target_link_libraries(test_transcoder PRIVATE ${TARGET} GTest::gtest GTest::gtest_main)
    else()
      target_link_libraries(test_transcoder PRIVATE ${TARGET} gtest)
    endif()
endif()
