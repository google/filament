cmake_minimum_required(VERSION 3.19)
project(filament C ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR})
set(RESOURCE_DIR  "${GENERATION_ROOT}/generated/resources")
set(MATERIAL_DIR  "${GENERATION_ROOT}/generated/material")
set(TEXTURE_DIR "${GENERATION_ROOT}/generated/texture")
set(RESOURCE_BINS)

# Materials
set(MATERIAL_SRCS
        ${ROOT_DIR}/samples/materials/aiDefaultMat.mat
        ${ROOT_DIR}/samples/materials/bakedColor.mat
        ${ROOT_DIR}/samples/materials/bakedTexture.mat
        ${ROOT_DIR}/samples/materials/aoPreview.mat
        ${ROOT_DIR}/samples/materials/arrayTexture.mat
        ${ROOT_DIR}/samples/materials/groundShadow.mat
        ${ROOT_DIR}/samples/materials/heightfield.mat
        ${ROOT_DIR}/samples/materials/image.mat
        ${ROOT_DIR}/samples/materials/mirror.mat
        ${ROOT_DIR}/samples/materials/overdraw.mat
        ${ROOT_DIR}/samples/materials/sandboxCloth.mat
        ${ROOT_DIR}/samples/materials/sandboxLit.mat
        ${ROOT_DIR}/samples/materials/sandboxLitFade.mat
        ${ROOT_DIR}/samples/materials/sandboxLitTransparent.mat
        ${ROOT_DIR}/samples/materials/sandboxLitThinRefraction.mat
        ${ROOT_DIR}/samples/materials/sandboxLitThinRefractionSsr.mat
        ${ROOT_DIR}/samples/materials/sandboxLitSolidRefraction.mat
        ${ROOT_DIR}/samples/materials/sandboxLitSolidRefractionSsr.mat
        ${ROOT_DIR}/samples/materials/sandboxSpecGloss.mat
        ${ROOT_DIR}/samples/materials/sandboxSubsurface.mat
        ${ROOT_DIR}/samples/materials/sandboxUnlit.mat
        ${ROOT_DIR}/samples/materials/texturedLit.mat
)

# Tint does not support setting gl_PointSize, disable the relevant sample if using WebGPU
# https://github.com/gpuweb/gpuweb/issues/1190
if (NOT FILAMENT_SUPPORTS_WEBGPU)
    set(MATERIAL_SRCS ${MATERIAL_SRCS} materials/pointSprites.mat)
endif ()

if (CMAKE_CROSSCOMPILING)
    include(${IMPORT_EXECUTABLES})
endif()

file(MAKE_DIRECTORY ${MATERIAL_DIR})

set (MATC_FLAGS ${MATC_BASE_FLAGS})
if (FILAMENT_SAMPLES_STEREO_TYPE STREQUAL "instanced")
    set (MATC_FLAGS ${MATC_FLAGS} -PstereoscopicType=instanced)
    add_definitions(-DFILAMENT_SAMPLES_STEREO_TYPE_INSTANCED)
elseif (FILAMENT_SAMPLES_STEREO_TYPE STREQUAL "multiview")
    set (MATC_FLAGS ${MATC_FLAGS} -PstereoscopicType=multiview)
    add_definitions(-DFILAMENT_SAMPLES_STEREO_TYPE_MULTIVIEW)
endif ()

foreach (mat_src ${MATERIAL_SRCS})
    get_filename_component(localname "${mat_src}" NAME_WE)
    get_filename_component(fullname "${mat_src}" ABSOLUTE)
    set(output_path "${MATERIAL_DIR}/${localname}.filamat")
    add_custom_command(
            OUTPUT ${output_path}
            COMMAND matc ${MATC_FLAGS} -o ${output_path} ${fullname}
            MAIN_DEPENDENCY ${mat_src}
            DEPENDS matc
            COMMENT "Compiling material ${mat_src} to ${output_path}"
    )
    list(APPEND RESOURCE_BINS ${output_path})
endforeach()

# Resources
file(MAKE_DIRECTORY ${RESOURCE_DIR})

get_resgen_vars(${RESOURCE_DIR} resources)

add_custom_command(
        OUTPUT ${RESGEN_OUTPUTS}
        COMMAND resgen ${RESGEN_FLAGS} ${RESOURCE_BINS}
        DEPENDS resgen ${RESOURCE_BINS}
        COMMENT "Aggregating resources"
)

if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()

add_library(gltf-resources ${DUMMY_SRC})
set_target_properties(gltf-resources PROPERTIES FOLDER Samples/Resources)

set(GLTF_DEMO_RESOURCES
        ${ROOT_DIR}/third_party/models/DamagedHelmet/DamagedHelmet.glb
        ${MATERIAL_DIR}/groundShadow.filamat
        ${MATERIAL_DIR}/overdraw.filamat
)

get_resgen_vars(${RESOURCE_DIR} gltf_demo)

add_custom_command(
        OUTPUT ${RESGEN_OUTPUTS}
        COMMAND resgen ${RESGEN_FLAGS} ${GLTF_DEMO_RESOURCES}
        DEPENDS resgen ${GLTF_DEMO_RESOURCES}
)

if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()

target_sources(gltf-resources PRIVATE ${RESGEN_SOURCE})


# CMake fails to invoke ar on Windows unless there is at least one C/C++ file in the library.
set(DUMMY_SRC "${RESOURCE_DIR}/dummy.c")
add_custom_command(OUTPUT ${DUMMY_SRC} COMMAND echo "//" > ${DUMMY_SRC})

# Test binary
set(GLTF_COMPARISON_SOURCES
        src/test_CompareGLTF.cpp
        src/GLTFViewer.cpp
        src/ImageExpectations.cpp
        src/RunOnMain.cpp
        src/CrossThreadTask.cpp
        src/main.cpp
    )

file(COPY glTF_cases DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/images/actual_images)
file(COPY expected_images DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/images)

enable_testing()

add_executable(gltf_comparison ${GLTF_COMPARISON_SOURCES})
target_include_directories(gltf_comparison PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(gltf_comparison
        gtest
        filamentapp
        absl::str_format
        uberarchive
        gltf-resources
        gltfio
        viewer
)



include(GoogleTest)
gtest_discover_tests(gltf_comparison)