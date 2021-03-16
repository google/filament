cmake_minimum_required(VERSION 3.10)
project(resgen)

set(TARGET resgen)

# ==================================================================================================
# Source files
# ==================================================================================================
set(SRCS src/resgen.cpp)

# ==================================================================================================
# Target definitions
# ==================================================================================================
add_executable(${TARGET} ${SRCS})
target_link_libraries(${TARGET} PRIVATE getopt)

# =================================================================================================
# Licenses
# ==================================================================================================
set(MODULE_LICENSES getopt)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
list_licenses(${GENERATION_ROOT}/licenses/licenses.inc ${MODULE_LICENSES})
target_include_directories(${TARGET} PRIVATE ${GENERATION_ROOT})

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
