cmake_minimum_required(VERSION 3.19)
project(matedit)

set(TARGET matedit)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(SRCS
    src/main.cpp
    src/ExternalCompile.cpp
)

# ==================================================================================================
# Target definitions
# ==================================================================================================
add_executable(${TARGET} ${SRCS})

target_include_directories(${TARGET} PRIVATE ${filamat_SOURCE_DIR}/src)

target_link_libraries(${TARGET} matdbg getopt)

set_target_properties(${TARGET} PROPERTIES FOLDER Tools)

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
