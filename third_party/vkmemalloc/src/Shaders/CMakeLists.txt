# This file will only be executed if VMA_BUILD_SAMPLE_SHADERS is set to ON

find_program(GLSL_VALIDATOR glslangValidator REQUIRED)

if(NOT GLSL_VALIDATOR)
    message(FATAL_ERROR "glslangValidator not found!")
endif()

set(SHADERS
    Shader.vert
    Shader.frag
    SparseBindingTest.comp
)

# Compile each shader using glslangValidator
foreach(SHADER ${SHADERS})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    
    # Put the .spv files into the bin folder
    set(SPIRV ${PROJECT_SOURCE_DIR}/bin/${FILE_NAME}.spv)

    add_custom_command(
        OUTPUT ${SPIRV}
        # Use the same file name and append .spv to the compiled shader
        COMMAND ${GLSL_VALIDATOR} -V ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
    )

    list(APPEND SPIRV_FILES ${SPIRV})
endforeach()

add_custom_target(VmaSampleShaders ALL DEPENDS ${SPIRV_FILES})
