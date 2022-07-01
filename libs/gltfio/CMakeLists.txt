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
        include/gltfio/FilamentAsset.h
        include/gltfio/FilamentInstance.h
        include/gltfio/MaterialProvider.h
        include/gltfio/NodeManager.h
        include/gltfio/ResourceLoader.h
        include/gltfio/TextureProvider.h
        include/gltfio/math.h
)

set(SRCS
        src/ArchiveCache.cpp
        src/ArchiveCache.h
        src/Animator.cpp
        src/AssetLoader.cpp
        src/DependencyGraph.cpp
        src/DependencyGraph.h
        src/DracoCache.cpp
        src/DracoCache.h
        src/FFilamentAsset.h
        src/FFilamentInstance.h
        src/FilamentAsset.cpp
        src/FilamentInstance.cpp
        src/FNodeManager.h
        src/GltfEnums.h
        src/Ktx2Provider.cpp
        src/MaterialProvider.cpp
        src/NodeManager.cpp
        src/ResourceLoader.cpp
        src/StbProvider.cpp
        src/TangentsJob.cpp
        src/TangentsJob.h
        src/UbershaderProvider.cpp
        src/Wireframe.cpp
        src/Wireframe.h
        src/upcast.h
)

# ==================================================================================================
# Build materials
# ==================================================================================================

set(RESOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR})

if (CMAKE_CROSSCOMPILING)
    include(${IMPORT_EXECUTABLES})
endif()

set(DOUBLESIDED false)
set(TRANSPARENCY default)

function(generate_ubershader_base SHADINGMODEL BLENDING)
    configure_file(materials/base.mat.in "${RESOURCE_DIR}/${SHADINGMODEL}_${BLENDING}.mat")
    configure_file(materials/base.spec.in "${RESOURCE_DIR}/${SHADINGMODEL}_${BLENDING}.spec")
    set(MATERIAL_NAMES ${MATERIAL_NAMES} "${SHADINGMODEL}_${BLENDING}" PARENT_SCOPE)
endfunction()

function(generate_ubershader_custom NAME)
    configure_file(materials/${NAME}.mat.in "${RESOURCE_DIR}/${NAME}.mat")
    configure_file(materials/${NAME}.spec.in "${RESOURCE_DIR}/${NAME}.spec")
    set(MATERIAL_NAMES ${MATERIAL_NAMES} ${NAME} PARENT_SCOPE)
endfunction()

# Generate or copy mat / spec files and build a list of material names.
# The first material becomes the default material in the ubershader archive.

set(MATERIAL_NAMES)
generate_ubershader_base(lit opaque)
generate_ubershader_base(lit fade)
generate_ubershader_base(lit masked)
generate_ubershader_base(specularGlossiness fade)
generate_ubershader_base(specularGlossiness opaque)
generate_ubershader_base(specularGlossiness masked)
generate_ubershader_base(unlit fade)
generate_ubershader_base(unlit opaque)
generate_ubershader_base(unlit masked)
generate_ubershader_custom(volume)
generate_ubershader_custom(transmission)
generate_ubershader_custom(sheen)

# Iterate over MATERIAL_NAMES and call matc for each material.

set(UBERZ_INPUTS)
foreach (NAME ${MATERIAL_NAMES})
    add_custom_command(
            OUTPUT "${NAME}.filamat"
            COMMAND matc ${MATC_BASE_FLAGS} -o "${NAME}.filamat" "${NAME}.mat"
            DEPENDS matc "${RESOURCE_DIR}/${NAME}.mat"
            WORKING_DIRECTORY ${RESOURCE_DIR}
            COMMENT "Compiling material ${NAME}"
    )
endforeach()

# ==================================================================================================
# Create a target for the ubershader archive that invokes the uberz tool.
# ==================================================================================================

set(UBERZ_INPUTS)
foreach (NAME ${MATERIAL_NAMES})
    list(APPEND UBERZ_INPUTS "${RESOURCE_DIR}/${NAME}.filamat")
    list(APPEND UBERZ_INPUTS "${RESOURCE_DIR}/${NAME}.spec")
endforeach()

set(UBERZ_FILE_PATH "${RESOURCE_DIR}/default.uberz")

add_custom_command(
    OUTPUT ${UBERZ_FILE_PATH}
    COMMAND uberz -o ${UBERZ_FILE_PATH} ${MATERIAL_NAMES}
    DEPENDS uberz ${UBERZ_INPUTS}
    WORKING_DIRECTORY ${RESOURCE_DIR}
    COMMENT "Building ubershader archive")

add_custom_target(uberz_file DEPENDS ${UBERZ_FILE_PATH})

# ==================================================================================================
# Build a resgen library for the ubershader archive.
# ==================================================================================================

get_resgen_vars("${RESOURCE_DIR}/materials" uberarchive)

add_custom_command(
        OUTPUT ${RESGEN_OUTPUTS}
        COMMAND resgen ${RESGEN_FLAGS} ${UBERZ_FILE_PATH}
        DEPENDS resgen ${UBERZ_FILE_PATH}
)

if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()

set(DUMMY_SRC "${RESOURCE_DIR}/dummy.c")
add_custom_command(OUTPUT ${DUMMY_SRC} COMMAND echo "//" > ${DUMMY_SRC})

add_library(uberarchive ${DUMMY_SRC} ${RESGEN_SOURCE})
target_include_directories(uberarchive PUBLIC ${RESOURCE_DIR})

# ==================================================================================================
# Build the "core" library (no filamat)
# ==================================================================================================

include_directories(${PUBLIC_HDR_DIR} ${RESOURCE_DIR})
link_libraries(math utils filament cgltf stb ktxreader geometry tsl trie uberzlib)

add_library(gltfio_core STATIC ${PUBLIC_HDRS} ${SRCS})

target_include_directories(gltfio_core PUBLIC ${PUBLIC_HDR_DIR})

target_compile_definitions(gltfio_core PUBLIC -DGLTFIO_DRACO_SUPPORTED=1)
target_link_libraries(gltfio_core PUBLIC dracodec)

if (NOT WEBGL AND NOT ANDROID AND NOT IOS)

    # ==================================================================================================
    # Link the core library with additional dependencies to create the "full" library
    # ==================================================================================================
    add_library(${TARGET} STATIC ${PUBLIC_HDRS} src/JitShaderProvider.cpp)
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

    install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})

endif()

# ==================================================================================================
# Installation
# ==================================================================================================

install(TARGETS gltfio_core uberarchive ARCHIVE DESTINATION lib/${DIST_DIR})
install(DIRECTORY ${PUBLIC_HDR_DIR}/gltfio DESTINATION include)
install(FILES ${RESOURCE_DIR}/materials/uberarchive.h DESTINATION include/gltfio/materials)
