cmake_minimum_required(VERSION 3.19)
project(matinfo)

set(TARGET matinfo)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(SRCS src/main.cpp)

# ==================================================================================================
# Target definitions
# ==================================================================================================
add_executable(${TARGET} ${SRCS})

target_link_libraries(${TARGET} matdbg filaflat backend_headers getopt)

# glslang contains a copy of the SPIRV headers, so let's just use those. The leading ".." in the
# following variable refers to the project name that we define in glslang/tnt, and the trailing ".."
# in the path allows us to do #include <SPIRV/disassemble.h>
target_include_directories(${TARGET} PRIVATE ${../glslang_SOURCE_DIR}/..)

set_target_properties(${TARGET} PROPERTIES FOLDER Tools)

# =================================================================================================
# Licenses
# ==================================================================================================
set(MODULE_LICENSES getopt glslang smol-v)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
list_licenses(${GENERATION_ROOT}/licenses/licenses.inc ${MODULE_LICENSES})
target_include_directories(${TARGET} PRIVATE ${GENERATION_ROOT})

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
install(FILES "README.md" DESTINATION docs/ RENAME "${TARGET}.md")
