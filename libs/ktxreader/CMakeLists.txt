cmake_minimum_required(VERSION 3.19)
project(ktxreader)

set(TARGET ktxreader)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/ktxreader/Ktx1Reader.h
        include/ktxreader/Ktx2Reader.h
)

set(SRCS
        src/Ktx1Reader.cpp
        src/Ktx2Reader.cpp
)

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS})

target_link_libraries(${TARGET} PUBLIC utils image filament basis_transcoder)

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
install(DIRECTORY ${PUBLIC_HDR_DIR}/${TARGET} DESTINATION include)

# ==================================================================================================
# Tests
# ==================================================================================================

function(add_testfile filename)
    set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/tests/${filename}")
    set(target_path "${PROJECT_BINARY_DIR}/${filename}")
    set(TESTFILES ${TESTFILES} ${target_path} PARENT_SCOPE)
    add_custom_command(
        OUTPUT ${target_path}
        COMMAND ${CMAKE_COMMAND} -E copy ${source_path} ${target_path}
        MAIN_DEPENDENCY ${source_path})
endfunction()

add_testfile(color_grid_uastc_zstd.ktx2)
add_testfile(lightroom_ibl.ktx)

if(FILAMENT_TESTS AND NOT ANDROID AND NOT WEBGL AND NOT IOS)
    add_executable(test_ktxreader tests/test_ktxreader.cpp ${TESTFILES})
    target_link_libraries(test_ktxreader PRIVATE ${TARGET} gtest)
    set_target_properties(test_ktxreader PROPERTIES FOLDER Tests)
endif()
