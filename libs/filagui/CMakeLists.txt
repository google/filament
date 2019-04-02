cmake_minimum_required(VERSION 3.1)
project(filagui)

set(TARGET filagui)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        ${PUBLIC_HDR_DIR}/${TARGET}/ImGuiExtensions.h
        ${PUBLIC_HDR_DIR}/${TARGET}/ImGuiHelper.h)

set(SRCS
        src/ImGuiHelper.cpp
        src/ImGuiExtensions.cpp
)

# ==================================================================================================
# Compile material files (mat) into headers (inc)
# ==================================================================================================

if (CMAKE_CROSSCOMPILING)
    include(${IMPORT_EXECUTABLES})
endif()

set(MATERIAL_SRCS
        src/materials/uiBlit.mat)

set(MATERIAL_BINS)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR})
file(MAKE_DIRECTORY "${GENERATION_ROOT}/generated/material/")

# Target system.
if (ANDROID OR WEBGL)
    set(MATC_TARGET mobile)
else()
    set(MATC_TARGET desktop)
endif()

# Optimize materials for performance in Release mode.
set(MATC_FLAGS -a all)
if (NOT CMAKE_BUILD_TYPE MATCHES Release)
    set(MATC_FLAGS ${MATC_FLAGS} -g)
endif()

foreach (mat_src ${MATERIAL_SRCS})
    get_filename_component(localname "${mat_src}" NAME_WE)
    get_filename_component(fullname "${mat_src}" ABSOLUTE)
    set(output_path "${GENERATION_ROOT}/generated/material/${localname}.inc")
    add_custom_command(
            OUTPUT ${output_path}
            COMMAND matc ${MATC_FLAGS} -p ${MATC_TARGET} -m material -f header
                    -o ${output_path} ${fullname}
            DEPENDS ${mat_src} matc
            COMMENT "Compiling material ${mat_src} to ${output_path}"
    )
    list(APPEND MATERIAL_BINS ${output_path})
endforeach()

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
include_directories(${PUBLIC_HDR_DIR})
include_directories(${GENERATION_ROOT})

add_library(${TARGET} STATIC ${PUBLIC_HDRS} ${SRCS} ${MATERIAL_BINS})

target_link_libraries(${TARGET} PUBLIC imgui filament filamat)

target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

# ==================================================================================================
# Compiler flags
# ==================================================================================================
target_compile_options(${TARGET} PRIVATE
        -Wno-deprecated-register
        $<$<CONFIG:Release>:-ffast-math>
)
