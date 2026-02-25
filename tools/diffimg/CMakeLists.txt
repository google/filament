cmake_minimum_required(VERSION 3.19)
project(diffimg)

set(TARGET diffimg)

add_executable(${TARGET} main.cpp)

target_link_libraries(${TARGET} PRIVATE
    getopt
    imagediff
    imageio
    imageio-lite
    image
    utils
)

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
