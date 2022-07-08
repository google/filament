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

set(UBERZ_OUTPUT_PATH "${RESOURCE_DIR}/default.uberz")

function(build_ubershader NAME SRC SHADINGMODEL BLENDING)
    set(DEST "${RESOURCE_DIR}/${NAME}")
    configure_file(materials/${SRC}.mat.in "${DEST}.mat" COPYONLY)
    configure_file(materials/${SRC}.spec.in "${DEST}.spec" COPYONLY)

    set(TEMPLATE_ARGS
        -TCUSTOM_PARAMS="// no custom params"
        -TCUSTOM_VERTEX="// no custom vertex"
        -TCUSTOM_FRAGMENT="// no custom fragment"
        -TDOUBLESIDED=false
        -TTRANSPARENCY=default
        -TSHADINGMODEL=${SHADINGMODEL}
        -TBLENDING=${BLENDING})

    add_custom_command(
        OUTPUT "${NAME}.filamat"
        COMMAND matc ${MATC_BASE_FLAGS} ${TEMPLATE_ARGS} -o "${NAME}.filamat" "${NAME}.mat"
        DEPENDS matc "${DEST}.mat"
        WORKING_DIRECTORY ${RESOURCE_DIR}
        COMMENT "Compiling material ${NAME}")

    set(UBERZ_ARGS -o ${UBERZ_OUTPUT_PATH} ${TEMPLATE_ARGS})

    # Invoke uberz to consume foo.spec + foo.filamat and produce default.uberz
    # The first time this is invoked, it omits the --append argument.
    if (NOT APPEND_UBERSHADERS)
        set(APPEND_UBERSHADERS ON PARENT_SCOPE)
        add_custom_command(
            OUTPUT ${UBERZ_OUTPUT_PATH}
            COMMAND uberz ${UBERZ_ARGS} ${NAME}
            DEPENDS uberz ${DEST}.filamat ${DEST}.spec
            WORKING_DIRECTORY ${RESOURCE_DIR}
            COMMENT "Creating ubershader archive")
    else()
        add_custom_command(
            OUTPUT ${UBERZ_OUTPUT_PATH}
            COMMAND uberz --append ${UBERZ_ARGS} ${NAME}
            DEPENDS uberz ${DEST}.filamat ${DEST}.spec
            WORKING_DIRECTORY ${RESOURCE_DIR}
            APPEND
            COMMENT "Appending to ubershader archive")
    endif()

endfunction()

# NOTE: The default ubershader material is defined as the first material in the list below.

build_ubershader(lit_opaque                base            lit opaque)
build_ubershader(lit_fade                  base            lit fade)
build_ubershader(lit_masked                base            lit masked)
build_ubershader(specularGlossiness_fade   base            specularGlossiness fade)
build_ubershader(specularGlossiness_opaque base            specularGlossiness opaque)
build_ubershader(specularGlossiness_masked base            specularGlossiness masked)
build_ubershader(unlit_fade                base            unlit fade)
build_ubershader(unlit_opaque              base            unlit opaque)
build_ubershader(unlit_masked              base            unlit masked)
build_ubershader(volume                    volume          _ _)
build_ubershader(transmission              transmission    _ _)
build_ubershader(sheen                     sheen           _ _)

add_custom_target(uberz_file DEPENDS ${UBERZ_OUTPUT_PATH})

# ==================================================================================================
# Build a resgen library for the ubershader archive.
# ==================================================================================================

get_resgen_vars("${RESOURCE_DIR}/materials" uberarchive)

add_custom_command(
        OUTPUT ${RESGEN_OUTPUTS}
        COMMAND resgen ${RESGEN_FLAGS} ${UBERZ_OUTPUT_PATH}
        DEPENDS resgen ${UBERZ_OUTPUT_PATH}
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
