cmake_minimum_required(VERSION 3.10)
project(skygen)

set(TARGET skygen)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(SRCS src/main.cpp)

# ==================================================================================================
# Target definitions
# ==================================================================================================
add_executable(${TARGET} ${SRCS})

target_link_libraries(${TARGET} PRIVATE math utils z image imageio skylight getopt)

# ==================================================================================================
# Compile options and optimizations
# ==================================================================================================
if (NOT LINUX)
    target_compile_options(${TARGET} PRIVATE )
endif()

# =================================================================================================
# Licenses
# ==================================================================================================
set(MODULE_LICENSES getopt skylight libpng tinyexr libz)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
list_licenses(${GENERATION_ROOT}/licenses/licenses.inc ${MODULE_LICENSES})
target_include_directories(${TARGET} PRIVATE ${GENERATION_ROOT})

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
install(FILES "README.md" DESTINATION docs/ RENAME "${TARGET}.md")
