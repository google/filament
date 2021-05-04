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

if(HUNTER_ENABLED)
  target_include_directories(${TARGET} PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${PUBLIC_HDR_DIR}>  
    $<INSTALL_INTERFACE:${PUBLIC_HDR_DIR}>)
else()
  target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
endif()

target_link_libraries(${TARGET} math utils)

add_library(${TARGET}-lite ${PUBLIC_HDRS} ${PRIVATE_HDRS} ${SRCS})
target_compile_definitions(${TARGET}-lite PUBLIC -DFILAMENT_IBL_LITE=1)
if(HUNTER_ENABLED)
  target_include_directories(${TARGET}-lite PUBLIC  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${PUBLIC_HDR_DIR}>  
    $<INSTALL_INTERFACE:${PUBLIC_HDR_DIR}>)
else()
  target_include_directories(${TARGET}-lite PUBLIC ${PUBLIC_HDR_DIR})
endif()
target_link_libraries(${TARGET}-lite math utils)

# ==================================================================================================
# Compile options and optimizations
# ==================================================================================================
if (MSVC)
    target_compile_options(${TARGET} PRIVATE /fp:fast)
    target_compile_options(${TARGET}-lite PRIVATE /fp:fast)
else()
    target_compile_options(${TARGET} PRIVATE -ffast-math)
    target_compile_options(${TARGET}-lite PRIVATE -ffast-math)
endif()


# ==================================================================================================
# Installation
# ==================================================================================================
if(HUNTER_ENABLED)
  install(TARGETS ${TARGET} EXPORT ${TARGETS_EXPORT_NAME} ARCHIVE DESTINATION lib/${DIST_DIR})
  install(TARGETS ${TARGET}-lite EXPORT ${TARGETS_EXPORT_NAME} ARCHIVE DESTINATION lib/${DIST_DIR})
else()
  install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
  install(TARGETS ${TARGET}-lite ARCHIVE DESTINATION lib/${DIST_DIR})
endif()
install(DIRECTORY ${PUBLIC_HDR_DIR}/ibl DESTINATION include)
