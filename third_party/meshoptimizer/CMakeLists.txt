project(meshoptimizer)
cmake_minimum_required(VERSION 3.0)

option(BUILD_DEMO "Build demo" OFF)

set(SOURCES
    src/meshoptimizer.h
    src/clusterizer.cpp
    src/indexcodec.cpp
    src/indexgenerator.cpp
    src/overdrawanalyzer.cpp
    src/overdrawoptimizer.cpp
    src/simplifier.cpp
    src/stripifier.cpp
    src/vcacheanalyzer.cpp
    src/vcacheoptimizer.cpp
    src/vertexcodec.cpp
    src/vfetchanalyzer.cpp
    src/vfetchoptimizer.cpp
)

add_library(meshoptimizer STATIC ${SOURCES})
target_include_directories(meshoptimizer INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/src")

if(MSVC)
	target_compile_options(meshoptimizer PRIVATE /W4 /WX)
else()
	target_compile_options(meshoptimizer PRIVATE -Wall -Wextra -Werror)
endif()

if(CLANG_CL)
    # Clang for Windows needs an additional flag for SSSE3 instructions.
    target_compile_options(meshoptimizer PRIVATE -mssse3)
endif()

if(BUILD_DEMO)
    add_executable(demo demo/main.cpp demo/miniz.cpp demo/objparser.cpp)
    target_link_libraries(demo meshoptimizer)
endif()

install(TARGETS meshoptimizer ARCHIVE DESTINATION lib/${DIST_DIR})
