cmake_minimum_required(VERSION 3.19)
project(cmgen)

set(TARGET cmgen)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(HDRS
        src/ProgressUpdater.h
)

set(SRCS
    src/cmgen.cpp
        src/ProgressUpdater.cpp
)

# ==================================================================================================
# Target definitions
# ==================================================================================================
add_executable(${TARGET} ${HDRS} ${SRCS})

target_link_libraries(${TARGET} PRIVATE ibl imageio getopt)
set_target_properties(${TARGET} PROPERTIES FOLDER Tools)

# ==================================================================================================
# Compile options and optimizations
# ==================================================================================================
if (MSVC)
    target_compile_options(${TARGET} PRIVATE /fp:fast)
else()
    target_compile_options(${TARGET} PRIVATE -ffast-math -fno-finite-math-only)
endif()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W0 /Zc:__cplusplus")
endif()

# =================================================================================================
# Licenses
# ==================================================================================================
set(MODULE_LICENSES getopt libpng tinyexr libz)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
list_licenses(${GENERATION_ROOT}/licenses/licenses.inc ${MODULE_LICENSES})
target_include_directories(${TARGET} PRIVATE ${GENERATION_ROOT})

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} RUNTIME DESTINATION bin)

# ==================================================================================================
# Tests
# ==================================================================================================
if(FILAMENT_TESTS AND NOT ANDROID)
    add_executable(test_${TARGET} tests/test_cmgen.cpp)
    target_link_libraries(test_${TARGET} PRIVATE imageio gtest)
    set_target_properties(test_${TARGET} PROPERTIES FOLDER Tests)
endif()
