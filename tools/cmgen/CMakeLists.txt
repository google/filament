cmake_minimum_required(VERSION 3.10)
project(cmgen)

set(TARGET cmgen)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(HDRS
    src/JobQueue.h
    src/ProgressUpdater.h
)

set(SRCS
    src/cmgen.cpp
    src/JobQueue.cpp
    src/ProgressUpdater.cpp
)

# ==================================================================================================
# Target definitions
# ==================================================================================================
add_executable(${TARGET} ${HDRS} ${SRCS})

if(HUNTER_ENABLED)
  target_link_libraries(${TARGET} PRIVATE math utils ibl ZLIB::zlib image imageio)
  if (MSVC)
    target_link_libraries(${TARGET} PRIVATE getopt::getopt)
  endif()
else()
  target_link_libraries(${TARGET} PRIVATE math utils ibl z image imageio getopt)
endif()

# ==================================================================================================
# Compile options and optimizations
# ==================================================================================================
if (MSVC)
    target_compile_options(${TARGET} PRIVATE /fp:fast)
else()
    target_compile_options(${TARGET} PRIVATE -ffast-math)
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
if (NOT ANDROID)
    add_executable(test_${TARGET} tests/test_cmgen.cpp)
    if(HUNTER_ENABLED)
      target_link_libraries(test_${TARGET} PRIVATE image imageio GTest::gtest GTest::gtest_main Threads::Threads)
    else()
      target_link_libraries(test_${TARGET} PRIVATE image imageio gtest)
    endif()
endif()
