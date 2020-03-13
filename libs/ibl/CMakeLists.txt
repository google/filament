cmake_minimum_required(VERSION 3.10)
project(cmgen)

set(TARGET ibl)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
    ${PUBLIC_HDR_DIR}/ibl/Cubemap.h
    ${PUBLIC_HDR_DIR}/ibl/CubemapIBL.h
    ${PUBLIC_HDR_DIR}/ibl/CubemapSH.h
    ${PUBLIC_HDR_DIR}/ibl/CubemapUtils.h
    ${PUBLIC_HDR_DIR}/ibl/Image.h
    ${PUBLIC_HDR_DIR}/ibl/utilities.h
)

set(PRIVATE_HDRS
    src/CubemapUtilsImpl.h
)

set(SRCS
    src/Cubemap.cpp
    src/CubemapIBL.cpp
    src/CubemapSH.cpp
    src/CubemapUtils.cpp
    src/Image.cpp
)

# ==================================================================================================
# Target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} ${PUBLIC_HDRS} ${PRIVATE_HDRS} ${SRCS})
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

target_link_libraries(${TARGET} math utils)

# ==================================================================================================
# Compile options and optimizations
# ==================================================================================================
if (MSVC)
    target_compile_options(${TARGET} PRIVATE /fp:fast)
else()
    target_compile_options(${TARGET} PRIVATE -ffast-math $<$<PLATFORM_ID:Linux>:-fPIC>)
endif()


# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
install(DIRECTORY ${PUBLIC_HDR_DIR}/ibl DESTINATION include)
