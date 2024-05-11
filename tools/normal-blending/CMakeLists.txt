cmake_minimum_required(VERSION 3.19)
project(normal-blending)

set(TARGET normal-blending)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
file(GLOB_RECURSE HDRS src/*.h)

set(SRCS src/main.cpp)

# ==================================================================================================
# Target definitions
# ==================================================================================================
add_executable(${TARGET} ${HDRS} ${SRCS})

target_link_libraries(${TARGET} PRIVATE imageio getopt)

set_target_properties(${TARGET} PROPERTIES FOLDER Tools)

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

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
install(FILES "README.md" DESTINATION docs/ RENAME "${TARGET}.md")
