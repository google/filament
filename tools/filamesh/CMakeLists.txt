cmake_minimum_required(VERSION 3.19)
project(filamesh)

set(TARGET filamesh)

# ==================================================================================================
# Sources
# ==================================================================================================
set(SRCS
    src/MeshWriter.cpp
    src/MeshWriter.h
    src/main.cpp)

# ==================================================================================================
# Target definitions
# ==================================================================================================
add_executable(${TARGET} ${SRCS})

target_link_libraries(${TARGET} PRIVATE assimp getopt filameshio meshoptimizer)
set_target_properties(${TARGET} PROPERTIES FOLDER Tools)

# ==================================================================================================
# Compile options and optimizations
# ==================================================================================================
if(NOT MSVC)
    target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)

    if (NOT LINUX)
        target_compile_options(${TARGET} PRIVATE
                -Wno-address-of-packed-member
        )
    endif()
endif()

# =================================================================================================
# Licenses
# ==================================================================================================
set(MODULE_LICENSES getopt libassimp meshoptimizer)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
list_licenses(${GENERATION_ROOT}/licenses/licenses.inc ${MODULE_LICENSES})
target_include_directories(${TARGET} PRIVATE ${GENERATION_ROOT})

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
install(FILES "README.md" DESTINATION docs/ RENAME "${TARGET}.md")
