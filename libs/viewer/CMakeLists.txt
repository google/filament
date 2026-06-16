cmake_minimum_required(VERSION 3.19)
project(viewer C ASM)

set(TARGET viewer)
set(PUBLIC_HDR_DIR include)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR})
set(RESOURCE_DIR "${GENERATION_ROOT}/generated/resources")
set(MATERIAL_DIR "${GENERATION_ROOT}/generated/material")

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/viewer/AutomationEngine.h
        include/viewer/AutomationSpec.h
        include/viewer/RemoteServer.h
        include/viewer/Settings.h
        include/viewer/ViewerGui.h
        include/viewer/TweakableProperty.h
        include/viewer/TweakableMaterial.h
        include/viewer/CustomFileDialogs.h
        include/viewer/json.hpp
)

set(PRIVATE_HDRS)

set(SRCS
        src/jsonParseUtils.h
        src/AutomationEngine.cpp
        src/AutomationSpec.cpp
        src/RemoteServer.cpp
        src/Settings.cpp
        src/Settings_generated.cpp
        src/Settings_generated.h
        src/ViewerGui.cpp
        src/TweakableMaterial.cpp
        src/CustomFileDialogs.cpp
)

set(MATERIAL_SRCS
        src/materials/Opaque.mat
        src/materials/Transparent.mat
        src/materials/Refractive.mat
        src/materials/Subsurface.mat
        src/materials/Cloth.mat
        src/materials/Masked.mat
)

get_resgen_vars(${RESOURCE_DIR} shapr_materials)
list(APPEND PRIVATE_HDRS ${RESGEN_HEADER})
list(APPEND SRCS ${RESGEN_SOURCE})

if (APPLE)
    list(APPEND SRCS
        src/MacosCustomFileDialogs.mm
    )

    find_library(UTI_FRAMEWORK UniformTypeIdentifiers)
    mark_as_advanced(UTI_FRAMEWORK)

    set(PLATFORM_ADDITIONAL_LIBS ${UTI_FRAMEWORK})
endif()

# ==================================================================================================
# Generate all .filamat: ShaprMaterials
# ==================================================================================================

if (CMAKE_CROSSCOMPILING)
    include(${IMPORT_EXECUTABLES})
endif()

set(MATERIAL_BINS)
file(MAKE_DIRECTORY ${MATERIAL_DIR})

foreach (mat_src ${MATERIAL_SRCS})
    get_filename_component(localname "${mat_src}" NAME_WE)
    get_filename_component(fullname "${mat_src}" ABSOLUTE)
    set(output_path "${MATERIAL_DIR}/${localname}.filamat")

    add_custom_command(
            OUTPUT ${output_path}
            COMMAND matc ${MATC_BASE_FLAGS} -o ${output_path} ${fullname}
            MAIN_DEPENDENCY ${fullname}
            DEPENDS matc
            COMMENT "Compiling material ${mat_src} to ${output_path} with options '${MATC_BASE_FLAGS}'"
    )
    list(APPEND MATERIAL_BINS ${output_path})
endforeach()

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/Cloth.filamat"
        DEPENDS src/materials/MaterialHelpersShading.fxh
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/Opaque.filamat"
        DEPENDS src/materials/MaterialHelpersShading.fxh
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/Refractive.filamat"
        DEPENDS src/materials/MaterialHelpersShading.fxh
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/Subsurface.filamat"
        DEPENDS src/materials/MaterialHelpersShading.fxh
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/Transparent.filamat"
        DEPENDS src/materials/MaterialHelpersShading.fxh
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/Masked.filamat"
        DEPENDS src/materials/MaterialHelpersShading.fxh
        APPEND
)

add_custom_command(
        OUTPUT ${RESGEN_OUTPUTS}
        COMMAND resgen ${RESGEN_FLAGS} ${MATERIAL_BINS}
        DEPENDS resgen ${MATERIAL_BINS}
        COMMENT "Aggregating Shapr3D materials"
)

if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()

# ==================================================================================================
# Include and target definitions
# ==================================================================================================
add_library(${TARGET} STATIC ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})
target_link_libraries(${TARGET} PUBLIC imgui filament gltfio_core filagui jsmn civetweb ${PLATFORM_ADDITIONAL_LIBS})
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
target_include_directories(${TARGET} PRIVATE ${GENERATION_ROOT})
set_target_properties(${TARGET} PROPERTIES FOLDER Libs)

# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (MSVC)
else()
    target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)
endif()

# ==================================================================================================
# Installation
# ==================================================================================================
install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})
install(DIRECTORY ${PUBLIC_HDR_DIR}/viewer DESTINATION include)

# ==================================================================================================
# Tests
# ==================================================================================================
if (NOT ANDROID AND NOT WEBGL AND NOT IOS)
    add_executable(test_settings tests/test_settings.cpp)
    target_link_libraries(test_settings PRIVATE ${TARGET} gtest)
    set_target_properties(test_settings PROPERTIES FOLDER Tests)
endif()
