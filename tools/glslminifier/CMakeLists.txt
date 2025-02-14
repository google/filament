cmake_minimum_required(VERSION 3.19)
project(glslminifier)

set(TARGET glslminifier)

# ==================================================================================================
# Source files
# ==================================================================================================
set(SRCS src/main.cpp src/GlslMinify.cpp)

# ==================================================================================================
# Target definitions
# ==================================================================================================
add_executable(${TARGET} ${SRCS})
target_link_libraries(${TARGET} PRIVATE utils getopt)
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

# ==================================================================================================
# Tests
# ==================================================================================================
if(FILAMENT_TESTS AND NOT ANDROID)
    add_executable(test_${TARGET}
            src/GlslMinify.cpp
            tests/test_glslminifier.cpp
    )
    target_include_directories(test_${TARGET} PRIVATE src)
    target_link_libraries(test_${TARGET} PRIVATE gtest)
    set_target_properties(test_${TARGET} PROPERTIES FOLDER Tests)
endif()
