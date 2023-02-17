cmake_minimum_required(VERSION 3.19)
project(mikktspace)

set(TARGET mikktspace)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
    ${PUBLIC_HDR_DIR}/mikktspace/mikktspace.h
)

set(SRCS
    src/mikktspace.c
)

# ==================================================================================================
# Target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} ${PUBLIC_HDRS} ${PRIVATE_HDRS} ${SRCS})
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)

# ==================================================================================================
# Compile options and optimizations
# ==================================================================================================
if (MSVC)
    target_compile_options(${TARGET} PRIVATE /fp:fast)
else()
    target_compile_options(${TARGET} PRIVATE -ffast-math)
endif()

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
install(DIRECTORY ${PUBLIC_HDR_DIR}/mikktspace DESTINATION include)
