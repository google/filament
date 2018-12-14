cmake_minimum_required(VERSION 3.1)
project(shaders C ASM)

set(TARGET shaders)

if (CMAKE_CROSSCOMPILING)
    include(${IMPORT_EXECUTABLES})
endif()

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(SHADERS
        src/brdf.fs
        src/common_getters.fs
        src/common_graphics.fs
        src/common_lighting.fs
        src/common_material.fs
        src/common_material.vs
        src/common_math.fs
        src/common_types.fs
        src/conversion_functions.fs
        src/depth_main.fs
        src/depth_main.vs
        src/dithering.fs
        src/fxaa.fs
        src/getters.fs
        src/getters.vs
        src/light_directional.fs
        src/light_indirect.fs
        src/light_punctual.fs
        src/main.fs
        src/main.vs
        src/post_process.fs
        src/post_process.vs
        src/shading_lit.fs
        src/shading_model_cloth.fs
        src/shading_model_standard.fs
        src/shading_model_subsurface.fs
        src/shading_parameters.fs
        src/shading_unlit.fs
        src/shadowing.fs
        src/shadowing.vs
        src/tone_mapping.fs
        src/variables.fs
        src/variables.vs)

# ==================================================================================================
# Code generation
# ==================================================================================================
set(RESOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

get_resgen_vars(${RESOURCE_DIR} shaders)

add_custom_command(
        OUTPUT ${RESGEN_OUTPUTS}
        COMMAND resgen --text --keep ${RESGEN_FLAGS} ${SHADERS}
        DEPENDS resgen ${SHADERS}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Aggregating shaders"
)

if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
add_library(${TARGET} STATIC ${RESGEN_SOURCE})
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
