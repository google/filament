cmake_minimum_required(VERSION 3.19)
project(shaders C ASM)

set(TARGET shaders)

if (CMAKE_CROSSCOMPILING)
    include(${IMPORT_EXECUTABLES})
endif()

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(SHADERS
        src/ambient_occlusion.fs
        src/brdf.fs
        src/common_defines.glsl
        src/common_getters.glsl
        src/common_instancing.glsl
        src/common_graphics.fs
        src/common_lighting.fs
        src/common_material.fs
        src/common_math.glsl
        src/common_shading.fs
        src/common_shadowing.glsl
        src/common_types.glsl
        src/depth_main.fs
        src/fog.fs
        src/getters.cs
        src/getters.fs
        src/getters.vs
        src/light_directional.fs
        src/light_indirect.fs
        src/light_reflections.fs
        src/light_punctual.fs
        src/main.cs
        src/main.fs
        src/main.vs
        src/material_inputs.fs
        src/material_inputs.vs
        src/post_process.fs
        src/post_process.vs
        src/post_process_getters.vs
        src/post_process_inputs.fs
        src/post_process_inputs.vs
        src/shading_lit.fs
        src/shading_lit_custom.fs
        src/shading_model_cloth.fs
        src/shading_model_standard.fs
        src/shading_model_subsurface.fs
        src/shading_parameters.fs
        src/shading_reflections.fs
        src/shading_unlit.fs
        src/shadowing.fs
        src/varyings.glsl
        src/vignette.fs
)

# These files aren't part of libshaders, but are included by materials inside
# filament/src/materials:
#
# tone_mapping.fs
# conversion_functions.fs
# dithering.fs

set(MINIFIED_DIR ${CMAKE_CURRENT_BINARY_DIR}/minified)

file(MAKE_DIRECTORY ${MINIFIED_DIR})

# After the foreach loop, SHADERS_MIN will contain the paths to the minified shaders.
set(SHADERS_MIN)
foreach(SHADER_FILE ${SHADERS})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    set(SHADER_RAW ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_FILE})
    set(SHADER_MIN ${MINIFIED_DIR}/${SHADER_NAME})
    # For Debug builds, pass two additional flags to help with debugging:
    # -Onone        performs no minification.
    # -lshader_name adds a #line directive at the beginning to help identify sources of errors.
    set(OPT_FLAG "$<$<CONFIG:DEBUG>:-Onone>")
    set(LINE_FLAG "$<$<CONFIG:DEBUG>:-l${SHADER_NAME}>")
    add_custom_command(
        OUTPUT ${SHADER_MIN}
        COMMAND glslminifier ${OPT_FLAG} ${LINE_FLAG} -o ${SHADER_MIN} ${SHADER_RAW}
        DEPENDS glslminifier ${SHADER_RAW}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Minifying shader ${SHADER_NAME}"
    )
    list(APPEND SHADERS_MIN ${SHADER_MIN})
endforeach()

# ==================================================================================================
# Code generation
# ==================================================================================================
set(RESOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

get_resgen_vars(${RESOURCE_DIR} shaders)

add_custom_command(
        OUTPUT ${RESGEN_OUTPUTS}
        COMMAND resgen --text --keep ${RESGEN_FLAGS} ${SHADERS_MIN}
        DEPENDS resgen ${SHADERS_MIN}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Aggregating shaders"
)

if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
add_library(${TARGET} STATIC ${RESGEN_SOURCE})
target_include_directories(${TARGET} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)
set_target_properties(${TARGET} PROPERTIES FOLDER Filament/Shaders)

# ==================================================================================================
# Installation
# ==================================================================================================

install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
