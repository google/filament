# HLSL Change - generate version include

macro(generate_version_include name input_file output_file gen_flags)
    if ("${input_file}" STREQUAL "")
        FILE(TO_NATIVE_PATH "${output_file}.gen" gen_file)
        add_custom_command(
            OUTPUT ${gen_file}
            COMMAND echo Generating version
	    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/gen_version.py ${gen_flags} > ${gen_file}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gen_version.py
        )
    else ()
        SET(gen_file ${input_file})
    endif ()

    add_custom_target(${name}
        BYPRODUCTS ${output_file}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${gen_file} ${output_file}
        DEPENDS ${gen_file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gen_version.py
    )

    set_target_properties(${name} PROPERTIES
        FOLDER version
    )
endmacro(generate_version_include)

# end HLSL Change - generate version include

if(HLSL_ENABLE_FIXED_VER AND "${HLSL_FIXED_VERSION_LOCATION}" STREQUAL "")
    SET(HLSL_FIXED_VERSION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/version.inc")
    message("Using fixed version file ${HLSL_FIXED_VERSION_FILE}")
else()
    SET(HLSL_FIXED_VERSION_FILE "")
    if (HLSL_OFFICIAL_BUILD OR HLSL_ENABLE_FIXED_VER)
        message("Will generate official build version based on the latest release fork sha and current commit count")
        set(HLSL_GEN_VERSION_OFFICIAL_OPTION "--official")
    else (HLSL_OFFICIAL_BUILD OR HLSL_ENABLE_FIXED_VER)
        message("Will generate dev build version based on current commit count")
        set(HLSL_GEN_VERSION_OFFICIAL_OPTION "")
    endif (HLSL_OFFICIAL_BUILD OR HLSL_ENABLE_FIXED_VER)
endif()


generate_version_include(hlsl_dxcversion_autogen
    "${HLSL_FIXED_VERSION_FILE}"
    "${HLSL_VERSION_LOCATION}/dxcversion.inc"
    "${HLSL_GEN_VERSION_OFFICIAL_OPTION}"
)

if (HLSL_EMBED_VERSION OR HLSL_ENABLE_FIXED_VER)
    # If there is an explicit fixed version location, version.inc should copy from there
    # Used to propagate GDK versions to the RC data
    if (NOT "${HLSL_FIXED_VERSION_LOCATION}" STREQUAL "")
        FILE(TO_CMAKE_PATH "${HLSL_FIXED_VERSION_LOCATION}/version.inc" HLSL_FIXED_VERSION_FILE)
    endif(NOT "${HLSL_FIXED_VERSION_LOCATION}" STREQUAL "")

    message(        ${HLSL_FIXED_VERSION_FILE}
        "${HLSL_VERSION_LOCATION}/version.inc"
        "${HLSL_GEN_VERSION_OFFICIAL_OPTION}")


    generate_version_include(hlsl_version_autogen
        "${HLSL_FIXED_VERSION_FILE}"
        "${HLSL_VERSION_LOCATION}/version.inc"
        "${HLSL_GEN_VERSION_OFFICIAL_OPTION}"
    )
endif(HLSL_EMBED_VERSION OR HLSL_ENABLE_FIXED_VER)

