cmake_minimum_required(VERSION 3.19)
project(smol-v)

set(TARGET smol-v)

set(PUBLIC_HDR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../source)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../source)

set(SRCS
    ${SRC_DIR}/smolv.cpp
)

add_library(${TARGET} STATIC ${SRCS})

target_compile_options(${TARGET} PRIVATE
        $<$<PLATFORM_ID:Linux>:-fPIC>
)

if (NOT MSVC)
    target_compile_options(${TARGET} PRIVATE -Wno-tautological-unsigned-enum-zero-compare)
endif()

target_include_directories(${TARGET} PUBLIC
    $<BUILD_INTERFACE:${PUBLIC_HDR_DIR}>
    $<INSTALL_INTERFACE:source>
)
set_target_properties(${TARGET} PROPERTIES FOLDER ThirdParty)

install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
