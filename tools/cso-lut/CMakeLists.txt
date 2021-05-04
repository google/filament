cmake_minimum_required(VERSION 3.10)
project(cso-lut)

set(TARGET cso-lut)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
file(GLOB_RECURSE HDRS src/*.h)

set(SRCS src/main.cpp)

# ==================================================================================================
# Target definitions
# ==================================================================================================
add_executable(${TARGET} ${HDRS} ${SRCS})


if(HUNTER_ENABLED)
  target_link_libraries(${TARGET} PRIVATE math utils ZLIB::zlib image imageio)
  if (MSVC)
    target_link_libraries(${TARGET} PRIVATE getopt::getopt)
  endif()
else()
  target_link_libraries(${TARGET} PRIVATE math utils z image imageio getopt)
endif()

# ==================================================================================================
# Compile options and optimizations
# ==================================================================================================
if (NOT LINUX)
    target_compile_options(${TARGET} PRIVATE )
endif()

# =================================================================================================
# Licenses
# ==================================================================================================
set(MODULE_LICENSES getopt libpng tinyexr libz)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
list_licenses(${GENERATION_ROOT}/licenses/licenses.inc ${MODULE_LICENSES})
target_include_directories(${TARGET} PRIVATE ${GENERATION_ROOT})
