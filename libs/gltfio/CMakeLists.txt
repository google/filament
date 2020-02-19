cmake_minimum_required(VERSION 3.10)
project(gltfio C ASM)

set(TARGET gltfio)
set(PUBLIC_HDR_DIR include)

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/gltfio/Animator.h
        include/gltfio/AssetLoader.h
        include/gltfio/MaterialProvider.h
        include/gltfio/AssetPipeline.h
        include/gltfio/ResourceLoader.h
        include/gltfio/SimpleViewer.h
        include/gltfio/FilamentAsset.h
)

set(SRCS
        src/Animator.cpp
        src/AssetLoader.cpp
        src/DependencyGraph.cpp
        src/DependencyGraph.h
        src/FFilamentAsset.h
        src/FilamentAsset.cpp
        src/GltfEnums.h
        src/MaterialProvider.cpp
        src/ResourceLoader.cpp
        src/UbershaderLoader.cpp
        src/Wireframe.cpp
        src/Wireframe.h
        src/math.h
        src/upcast.h
        src/Image.cpp
)

# ==================================================================================================
# Build materials
# ==================================================================================================

set(RESOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR})

if (CMAKE_CROSSCOMPILING)
    include(${IMPORT_EXECUTABLES})
endif()

function(generate_mat SHADINGMODEL BLENDING)
    set(DOUBLESIDED false)
    set(GENERATED_MAT "${RESOURCE_DIR}/${SHADINGMODEL}_${BLENDING}.mat")
    configure_file(materials/ubershader.mat.in ${GENERATED_MAT})
    set(MATERIAL_SRCS ${MATERIAL_SRCS} ${GENERATED_MAT} PARENT_SCOPE)
endfunction()

set(MATERIAL_SRCS)
generate_mat(lit fade)
generate_mat(lit opaque)
generate_mat(lit masked)
generate_mat(specularGlossiness fade)
generate_mat(specularGlossiness opaque)
generate_mat(specularGlossiness masked)
generate_mat(unlit fade)
generate_mat(unlit opaque)
generate_mat(unlit masked)

set(RESOURCE_BINS)
foreach (input_path ${MATERIAL_SRCS})
    get_filename_component(basename "${input_path}" NAME_WE)
    set(output_path "${RESOURCE_DIR}/${basename}.filamat")
    add_custom_command(
            OUTPUT ${output_path}
            COMMAND matc ${MATC_BASE_FLAGS} -o ${output_path} ${input_path}
            MAIN_DEPENDENCY ${input_path}
            DEPENDS matc
            COMMENT "Compiling material ${input_path} to ${output_path}"
    )
    list(APPEND RESOURCE_BINS ${output_path})
endforeach()

# ==================================================================================================
# Build resources library
# ==================================================================================================

get_resgen_vars(${RESOURCE_DIR} gltfresources)

add_custom_command(
        OUTPUT ${RESGEN_OUTPUTS}
        COMMAND resgen ${RESGEN_FLAGS} ${RESOURCE_BINS}
        DEPENDS resgen ${RESOURCE_BINS}
)

if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()

set(DUMMY_SRC "${RESOURCE_DIR}/dummy.c")
add_custom_command(OUTPUT ${DUMMY_SRC} COMMAND echo "//" > ${DUMMY_SRC})

add_library(gltfio_resources ${DUMMY_SRC} ${RESGEN_SOURCE})

# ==================================================================================================
# Build "lite" resources library, which contains only a few materials
# ==================================================================================================

set(RESOURCE_BINS
    "${RESOURCE_DIR}/lit_opaque.filamat"
    "${RESOURCE_DIR}/lit_fade.filamat")

get_resgen_vars(${RESOURCE_DIR} gltfresources_lite)

add_custom_command(
        OUTPUT ${RESGEN_OUTPUTS}
        COMMAND resgen ${RESGEN_FLAGS} ${RESOURCE_BINS}
        DEPENDS resgen ${RESOURCE_BINS}
)

if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()

set(DUMMY_SRC "${RESOURCE_DIR}/dummy.c")
add_custom_command(OUTPUT ${DUMMY_SRC} COMMAND echo "//" > ${DUMMY_SRC})

add_library(gltfio_resources_lite ${DUMMY_SRC} ${RESGEN_SOURCE})

# ==================================================================================================
# Build the "core" library (no filamat or pipeline)
# ==================================================================================================

include_directories(${PUBLIC_HDR_DIR} ${RESOURCE_DIR})
link_libraries(math utils filament cgltf stb geometry gltfio_resources)

add_library(gltfio_core STATIC ${PUBLIC_HDRS} ${SRCS})

target_include_directories(gltfio_core PUBLIC ${PUBLIC_HDR_DIR})
target_compile_options(gltfio_core PRIVATE $<$<PLATFORM_ID:Linux>:-fPIC>)

if (NOT WEBGL AND NOT ANDROID AND NOT IOS)

    # ==================================================================================================
    # Link the core library with additional dependencies to create the "full" library
    # ==================================================================================================
    add_library(${TARGET} STATIC ${PUBLIC_HDRS} src/MaterialGenerator.cpp)
    target_link_libraries(${TARGET} PUBLIC filamat gltfio_core)
    target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

    # ==================================================================================================
    # Compiler flags
    # ==================================================================================================
    if (NOT MSVC_NATIVE)
        target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)
    endif()

    if (MSVC OR CLANG_CL)
        target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/fp:fast>)
    else()
        target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:-ffast-math>)
    endif()
    target_compile_options(${TARGET} PRIVATE $<$<PLATFORM_ID:Linux>:-fPIC>)

    # ==================================================================================================
    # Installation
    # ==================================================================================================
    install(TARGETS ${TARGET} gltfio_core ARCHIVE DESTINATION lib/${DIST_DIR})
    install(DIRECTORY ${PUBLIC_HDR_DIR}/gltfio DESTINATION include)

else()

    install(TARGETS gltfio_core gltfio_resources gltfio_resources_lite ARCHIVE DESTINATION lib/${DIST_DIR})
    install(DIRECTORY ${PUBLIC_HDR_DIR}/gltfio DESTINATION include)
    install(FILES ${RESOURCE_DIR}/gltfresources.h DESTINATION include/gltfio/resources)
    install(FILES ${RESOURCE_DIR}/gltfresources_lite.h DESTINATION include/gltfio/resources)

endif()

# ==================================================================================================
# Create the glTF pipeline library (used by gltf_baker)
# ==================================================================================================

add_library(gltfio_pipeline STATIC ${PUBLIC_HDRS} src/AssetPipeline.cpp)
target_link_libraries(gltfio_pipeline PUBLIC xatlas meshoptimizer gltfio rays)
target_include_directories(gltfio_pipeline PUBLIC ${PUBLIC_HDR_DIR})
