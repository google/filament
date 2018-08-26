cmake_minimum_required(VERSION 3.1)
project(filamesh)

set(TARGET filamesh)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
file(GLOB_RECURSE HDRS src/*.h)

set(SRCS src/main.cpp)

# ==================================================================================================
# Target definitions
# ==================================================================================================
add_executable(${TARGET} ${HDRS} ${SRCS})

target_link_libraries(${TARGET} PUBLIC math)
target_link_libraries(${TARGET} PUBLIC utils)
target_link_libraries(${TARGET} PUBLIC assimp)
target_link_libraries(${TARGET} PRIVATE getopt)

# ==================================================================================================
# Compile options and optimizations
# ==================================================================================================
target_compile_options(${TARGET} PRIVATE
        -Wno-deprecated-register
)

if (NOT LINUX)
    target_compile_options(${TARGET} PRIVATE
            -Wno-address-of-packed-member
    )
endif()

# =================================================================================================
# Licenses
# ==================================================================================================
set(MODULE_LICENSES getopt libassimp)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
list_licenses(${GENERATION_ROOT}/licenses/licenses.inc ${MODULE_LICENSES})
target_include_directories(${TARGET} PRIVATE ${GENERATION_ROOT})

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
install(FILES "README.md" DESTINATION docs/ RENAME "${TARGET}.md")
