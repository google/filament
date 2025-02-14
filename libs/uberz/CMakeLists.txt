cmake_minimum_required(VERSION 3.19)
project(uberzlib)

set(TARGET uberzlib)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
    include/uberz/ArchiveEnums.h
    include/uberz/ReadableArchive.h
    include/uberz/WritableArchive.h
)

set(SRCS
    src/ReadableArchive.cpp
    src/WritableArchive.cpp
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_link_libraries(${TARGET} PUBLIC math utils filabridge zstd)

target_include_directories(${TARGET} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/${PUBLIC_HDR_DIR}>
    $<INSTALL_INTERFACE:${PUBLIC_HDR_DIR}>
)
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)

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
install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
install(DIRECTORY ${PUBLIC_HDR_DIR}/uberz DESTINATION include)
