cmake_minimum_required(VERSION 3.19)
project(zstd)

set(OUR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(LIB_TARGET          zstd)
set(SRC_DIR             ${OUR_DIR}/lib)
set(PUBLIC_HDR_DIR      ${OUR_DIR}/lib)
set(PROGRAM_DIR         ${OUR_DIR}/programs)

# public headers dependencies
set(PUBLIC_HDRS
    ${PUBLIC_HDR_DIR}/zstd.h
    ${PUBLIC_HDR_DIR}/zdict.h
    ${PUBLIC_HDR_DIR}/zstd_errors.h
)

# sources dependencies
file(GLOB CommonSources ${SRC_DIR}/common/*.c)
file(GLOB CompressSources ${SRC_DIR}/compress/*.c)
file(GLOB DecompressSources ${SRC_DIR}/decompress/*.c)
file(GLOB DictBuilderSources ${SRC_DIR}/dictBuilder/*.c)

add_compile_options(-DZSTD_DISABLE_ASM)

set(SRCS
    ${CommonSources}
    ${CompressSources}
    ${DecompressSources}
    ${DictBuilderSources}
)

# we're building a library
add_library(${LIB_TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

option(ZSTD_MULTITHREAD_SUPPORT "Enable multithreading" OFF)

if(ZSTD_MULTITHREAD_SUPPORT)
    target_compile_definitions(${LIB_TARGET} PRIVATE "ZSTD_MULTITHREAD")
else()
    target_compile_definitions(${LIB_TARGET} PRIVATE "ZSTD_MULTITHREAD=0")
endif()

target_compile_definitions(${LIB_TARGET} PRIVATE "XXH_NAMESPACE=ZSTD_")

# specify where the public headers of this library are
target_include_directories(${LIB_TARGET} PUBLIC ${PUBLIC_HDR_DIR})
target_include_directories(${LIB_TARGET} PRIVATE ${SRC_DIR}/common)
set_target_properties(${LIB_TARGET} PROPERTIES FOLDER ThirdParty)

install(TARGETS ${LIB_TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
