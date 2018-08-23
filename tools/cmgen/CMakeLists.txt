cmake_minimum_required(VERSION 3.1)
project(cmgen)

set(TARGET cmgen)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(HDRS
    src/JobQueue.h
    src/Cubemap.h
    src/CubemapIBL.h
    src/CubemapSH.h
    src/CubemapUtils.h
    src/Image.h
    src/ProgressUpdater.h
    src/utilities.h
)

set(SRCS
    src/cmgen.cpp
    src/Cubemap.cpp
    src/CubemapIBL.cpp
    src/CubemapSH.cpp
    src/CubemapUtils.cpp
    src/Image.cpp
    src/JobQueue.cpp
    src/ProgressUpdater.cpp
)

# ==================================================================================================
# Target definitions
# ==================================================================================================
add_executable(${TARGET} ${HDRS} ${SRCS})

target_link_libraries(${TARGET} PRIVATE math utils z image imageio getopt)

# ==================================================================================================
# Compile options and optimizations
# ==================================================================================================
if (MSVC OR CLANG_CL)
    target_compile_options(${TARGET} PRIVATE /fp:fast)
else()
    target_compile_options(${TARGET} PRIVATE -ffast-math)
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
if (NOT ANDROID)
    add_executable(test_${TARGET} tests/test_cmgen.cpp)
    target_link_libraries(test_${TARGET} PRIVATE image imageio gtest)
endif()
