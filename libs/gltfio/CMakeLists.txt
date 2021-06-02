cmake_minimum_required(VERSION 3.19)
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
        include/gltfio/ResourceLoader.h
        include/gltfio/FilamentAsset.h
        include/gltfio/FilamentInstance.h
)

set(SRCS
        src/Animator.cpp
        src/AssetLoader.cpp
        src/DependencyGraph.cpp
        src/DependencyGraph.h
        src/DracoCache.h
        src/DracoCache.cpp
		src/FAssetLoader.h
        src/FFilamentAsset.h
        src/FilamentAsset.cpp
        src/FFilamentInstance.h
        src/FilamentInstance.cpp
		src/FResourceLoader.cpp
        src/GltfEnums.h
        src/MaterialProvider.cpp
        src/MorphHelper.h
        src/MorphHelper.cpp
        src/ResourceLoader.cpp
        src/TangentsJob.h
        src/TangentsJob.cpp
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

function(generate_mat TEMPLATE SHADINGMODEL BLENDING)
    set(DOUBLESIDED false)
    set(GENERATED_MAT "${RESOURCE_DIR}/${SHADINGMODEL}_${BLENDING}.mat")
    configure_file(materials/${TEMPLATE}.mat.in ${GENERATED_MAT})
    set(MATERIAL_SRCS ${MATERIAL_SRCS} ${GENERATED_MAT} PARENT_SCOPE)
endfunction()

set(MATERIAL_SRCS)
generate_mat(ubershader lit fade)
generate_mat(ubershader lit opaque)
generate_mat(ubershader lit masked)
generate_mat(ubershader specularGlossiness fade)
generate_mat(ubershader specularGlossiness opaque)
generate_mat(ubershader specularGlossiness masked)
generate_mat(ubershader unlit fade)
generate_mat(ubershader unlit opaque)
generate_mat(ubershader unlit masked)
generate_mat(transmission lit transmission)
generate_mat(sheen lit sheen)

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

set(LITE_DIR ${CMAKE_CURRENT_BINARY_DIR}/lite)

function(generate_lite_mat BLENDING)
    set(DOUBLESIDED false)
    set(SHADINGMODEL lit)
    set(GENERATED_MAT "${LITE_DIR}/${SHADINGMODEL}_${BLENDING}.mat")
    configure_file(materials/gltflite.mat.in ${GENERATED_MAT})
    set(MATERIAL_SRCS ${MATERIAL_SRCS} ${GENERATED_MAT} PARENT_SCOPE)
endfunction()

set(MATERIAL_SRCS)
generate_lite_mat(fade)
generate_lite_mat(opaque)

foreach (input_path ${MATERIAL_SRCS})
    get_filename_component(basename "${input_path}" NAME_WE)
    set(output_path "${LITE_DIR}/${basename}.filamat")
    add_custom_command(
            OUTPUT ${output_path}
            COMMAND matc ${MATC_BASE_FLAGS} -o ${output_path} ${input_path}
            MAIN_DEPENDENCY ${input_path}
            DEPENDS matc
            COMMENT "Compiling material ${input_path} to ${output_path}"
    )
endforeach()

set(RESOURCE_BINS
    "${LITE_DIR}/lit_opaque.filamat"
    "${LITE_DIR}/lit_fade.filamat")

get_resgen_vars(${LITE_DIR} gltfresources_lite)

add_custom_command(
        OUTPUT ${RESGEN_OUTPUTS}
        COMMAND resgen ${RESGEN_FLAGS} ${RESOURCE_BINS}
        DEPENDS resgen ${RESOURCE_BINS}
)

if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()

set(DUMMY_SRC "${LITE_DIR}/dummy.c")
add_custom_command(OUTPUT ${DUMMY_SRC} COMMAND echo "//" > ${DUMMY_SRC})

add_library(gltfio_resources_lite ${DUMMY_SRC} ${RESGEN_SOURCE})

# ==================================================================================================
# Build the "core" library (no filamat)
# ==================================================================================================

include_directories(${PUBLIC_HDR_DIR} ${RESOURCE_DIR})
link_libraries(math utils filament cgltf stb geometry gltfio_resources tsl trie)

add_library(gltfio_core STATIC ${PUBLIC_HDRS} ${SRCS})

target_include_directories(gltfio_core PUBLIC ${PUBLIC_HDR_DIR})

target_compile_definitions(gltfio_core PUBLIC -DGLTFIO_DRACO_SUPPORTED=1)
target_link_libraries(gltfio_core PUBLIC dracodec)

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
    if (MSVC)
        target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:/fp:fast>)
    else()
        target_compile_options(${TARGET} PRIVATE $<$<CONFIG:Release>:-ffast-math>)
        target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)
    endif()

    # ==================================================================================================
    # Installation
    # ==================================================================================================
    install(TARGETS ${TARGET} gltfio_core gltfio_resources gltfio_resources_lite ARCHIVE DESTINATION lib/${DIST_DIR})
    install(DIRECTORY ${PUBLIC_HDR_DIR}/gltfio DESTINATION include)

else()

    install(TARGETS gltfio_core gltfio_resources gltfio_resources_lite ARCHIVE DESTINATION lib/${DIST_DIR})
    install(DIRECTORY ${PUBLIC_HDR_DIR}/gltfio DESTINATION include)
    install(FILES ${RESOURCE_DIR}/gltfresources.h DESTINATION include/gltfio/resources)
    install(FILES ${LITE_DIR}/gltfresources_lite.h DESTINATION include/gltfio/resources)

endif()
