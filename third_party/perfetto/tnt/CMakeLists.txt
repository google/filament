cmake_minimum_required(VERSION 3.19)
project(perfetto)

set(OUR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

set(TARGET              perfetto)
set(SRC_DIR             ${OUR_DIR}/${TARGET})
set(PUBLIC_HDR_DIR      ${OUR_DIR})

set(PUBLIC_HDRS
    ${PUBLIC_HDR_DIR}/${TARGET}/perfetto.h
)

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRC_DIR}/perfetto.cc)

if (WIN32)
    # The perfetto library contains many symbols, so it needs the big object
    # format.
    target_compile_options(perfetto PRIVATE "/bigobj")
    # Disable legacy features in windows.h.
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
    # On Windows we should link to WinSock2.
    target_link_libraries(${TARGET} ws2_32)
endif (WIN32)

# specify where the public headers of this library are
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(${TARGET} PROPERTIES FOLDER ThirdParty)

install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
