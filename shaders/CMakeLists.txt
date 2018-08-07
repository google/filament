cmake_minimum_required(VERSION 3.1)
project(shaders)

set(TARGET shaders)

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
set(GENERATION_ROOT ${CMAKE_BINARY_DIR})

set(SHADER_LIST)
file(MAKE_DIRECTORY "${GENERATION_ROOT}/generated/shaders/")

# On windows we cannot use 'cat' executable. We replace it with 'type'.
# On windows we cannot use character " on commandline. It has to be escape with ^".
set(CAT_COMMAND cat)
set(DOUBLEQUOTE "\\\"")
set(OPENP "\\(")
set(CLOSEP "\\)")

if (WIN32)
    set(CAT_COMMAND type)
    set(DOUBLEQUOTE "^\"")
    set(OPENP "(")
    set(CLOSEP ")")
endif()

foreach (shader_file ${SHADERS})
    get_filename_component(localname "${shader_file}" NAME)
    get_filename_component(filename  "${localname}"   NAME_WE)
    get_filename_component(extension "${localname}"   EXT)
    get_filename_component(fullname  "${shader_file}" ABSOLUTE)

    string(SUBSTRING "${extension}" 1 -1 extension)

    if (WIN32)
       string(REPLACE "/" "\\" fullname ${fullname})
    endif()

    set(output_path "${GENERATION_ROOT}/generated/shaders/${localname}.cpp")
    add_custom_command(
            OUTPUT ${output_path}
            COMMAND echo "namespace filament {" > ${output_path}
            COMMAND echo "namespace shaders {" >> ${output_path}
            COMMAND echo "extern const char ${filename}_${extension}[] = R${DOUBLEQUOTE}FILAMENT__${OPENP}" >> ${output_path}
            COMMAND ${CAT_COMMAND} \"${fullname}\" >> \"${output_path}\"
            COMMAND echo "${CLOSEP}FILAMENT__${DOUBLEQUOTE}\;" >> ${output_path}
            COMMAND echo "} // namespace shaders" >> ${output_path}
            COMMAND echo "} // namespace filament" >> ${output_path}
            DEPENDS ${shader_file}
            COMMENT "Processing shader ${shader_file}"
    )

    list(APPEND SHADER_LIST ${output_path})
endforeach()

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
add_library(${TARGET} STATIC ${SHADER_LIST} ${SHADERS})
