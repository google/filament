cmake_minimum_required(VERSION 3.19)
project(mipgen)

set(TARGET mipgen)

# ==================================================================================================
# Source files
# ==================================================================================================
set(SRCS src/main.cpp)

# ==================================================================================================
# Target definitions
# ==================================================================================================
add_executable(${TARGET} ${SRCS})
target_link_libraries(${TARGET} PRIVATE imageio getopt)
set_target_properties(${TARGET} PROPERTIES FOLDER Tools)

# =================================================================================================
# Licenses
# ==================================================================================================
set(MODULE_LICENSES getopt libpng tinyexr libz stb)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
list_licenses(${GENERATION_ROOT}/licenses/licenses.inc ${MODULE_LICENSES})
target_include_directories(${TARGET} PRIVATE ${GENERATION_ROOT})

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
install(FILES "README.md" DESTINATION docs/ RENAME "${TARGET}.md")
