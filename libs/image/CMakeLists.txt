cmake_minimum_required(VERSION 3.10)
project(image)

set(TARGET image)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/image/ColorTransform.h
        include/image/ImageOps.h
        include/image/ImageSampler.h
        include/image/KtxBundle.h
        include/image/KtxUtility.h
        include/image/LinearImage.h
)

set(SRCS
        src/ImageOps.cpp
        src/ImageSampler.cpp
        src/KtxBundle.cpp
        src/LinearImage.cpp
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
# Expose header-only utilities to lean-and-mean client projects.
# ==================================================================================================
add_library(image_headers INTERFACE)

if(HUNTER_ENABLED)
  target_include_directories(image_headers INTERFACE  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${PUBLIC_HDR_DIR}>  
    $<INSTALL_INTERFACE:${PUBLIC_HDR_DIR}>)
else()
  target_include_directories(image_headers INTERFACE ${PUBLIC_HDR_DIR})
endif()

# ==================================================================================================
# Installation
# ==================================================================================================
if(HUNTER_ENABLED)
  install(TARGETS ${TARGET} EXPORT ${TARGETS_EXPORT_NAME} ARCHIVE DESTINATION lib/${DIST_DIR})
else()
  install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
endif()
install(DIRECTORY ${PUBLIC_HDR_DIR}/image DESTINATION include)

# ==================================================================================================
# Tests
# ==================================================================================================
if (NOT ANDROID AND NOT WEBGL AND NOT IOS)
    add_executable(test_${TARGET} tests/test_image.cpp)
    if(HUNTER_ENABLED)
      target_link_libraries(test_${TARGET} PRIVATE image imageio GTest::gtest GTest::gtest_main Threads::Threads)
    else()
      target_link_libraries(test_${TARGET} PRIVATE image imageio gtest)
    endif()
endif()
