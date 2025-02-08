cmake_minimum_required(VERSION 3.19)
project(camutils)

set(TARGET camutils)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/camutils/Bookmark.h
        include/camutils/compiler.h
        include/camutils/Manipulator.h
)

set(SRCS
        src/Bookmark.cpp
        src/FreeFlightManipulator.h
        src/Manipulator.cpp
        src/MapManipulator.h
        src/OrbitManipulator.h
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_link_libraries(${TARGET} PUBLIC math)

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)

# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (MSVC)
else()
    target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)
endif()

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
install(DIRECTORY ${PUBLIC_HDR_DIR}/camutils DESTINATION include)

# ==================================================================================================
# Tests
# ==================================================================================================
if(FILAMENT_TESTS AND NOT ANDROID AND NOT WEBGL AND NOT IOS)
    add_executable(test_${TARGET} tests/test_camutils.cpp)
    target_link_libraries(test_${TARGET} PRIVATE camutils gtest)
    set_target_properties(test_${TARGET} PROPERTIES FOLDER Tests)
endif()
