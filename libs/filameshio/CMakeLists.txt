cmake_minimum_required(VERSION 3.19)
project(filameshio)

set(TARGET filameshio)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
    ${PUBLIC_HDR_DIR}/${TARGET}/filamesh.h
    ${PUBLIC_HDR_DIR}/${TARGET}/MeshReader.h
)

set(DIST_HDRS ${PUBLIC_HDR_DIR}/${TARGET}/MeshReader.h)
set(SRCS src/MeshReader.cpp)

# ==================================================================================================
# Includes and target definition
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})
add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
target_link_libraries(${TARGET}
    PRIVATE meshoptimizer
    PUBLIC filament # Public only because the filamesh API needs Box.h
)

# ==================================================================================================
# Installation
# ==================================================================================================
set(INSTALL_TYPE ARCHIVE)
install(TARGETS ${TARGET} ${INSTALL_TYPE} DESTINATION lib/${DIST_DIR})
install(FILES ${DIST_HDRS} DESTINATION include/${TARGET})

# ==================================================================================================
# Tests
# ==================================================================================================
if (NOT IOS AND NOT WEBGL AND NOT ANDROID)
    add_executable(test_${TARGET} tests/test_filamesh.cpp )
    target_link_libraries(test_${TARGET} PRIVATE filameshio gtest)
endif()
