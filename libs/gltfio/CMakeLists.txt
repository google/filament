cmake_minimum_required(VERSION 3.19)
project(gltfio C ASM)

set(TARGET gltfio)
set(PUBLIC_HDR_DIR include)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

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
        include/gltfio/TrsTransformManager.h
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
        src/FTrsTransformManager.h
        src/GltfEnums.h
        src/Ktx2Provider.cpp
        src/MaterialProvider.cpp
        src/NodeManager.cpp
        src/TrsTransformManager.cpp
        src/ResourceLoader.cpp
        src/StbProvider.cpp
        src/TangentsJob.cpp
        src/TangentsJob.h
        src/UbershaderProvider.cpp
        src/Utility.cpp
        src/Utility.h
        src/Wireframe.cpp
        src/Wireframe.h
        src/downcast.h
        src/extended/AssetLoaderExtended.h
        src/extended/TangentsJobExtended.cpp
        src/extended/TangentsJobExtended.h
        src/extended/TangentSpaceMeshWrapper.cpp
        src/extended/TangentSpaceMeshWrapper.h
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

set (MATC_FLAGS ${MATC_BASE_FLAGS})
if (FILAMENT_SAMPLES_STEREO_TYPE STREQUAL "instanced")
    set (MATC_FLAGS ${MATC_FLAGS} -PstereoscopicType=instanced)
elseif (FILAMENT_SAMPLES_STEREO_TYPE STREQUAL "multiview")
    set (MATC_FLAGS ${MATC_FLAGS} -PstereoscopicType=multiview)
endif ()

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
        COMMAND matc ${MATC_FLAGS} ${TEMPLATE_ARGS} -o "${NAME}.filamat" "${NAME}.mat"
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
build_ubershader(transmission_opaque       transmission    lit opaque)
build_ubershader(transmission_fade         transmission    lit fade)
build_ubershader(transmission_masked       transmission    lit masked)
build_ubershader(volume_opaque             volume          lit opaque)
build_ubershader(volume_fade               volume          lit fade)
build_ubershader(volume_masked             volume          lit masked)
build_ubershader(sheen                     sheen           _ _)

add_custom_target(uberz_file DEPENDS ${UBERZ_OUTPUT_PATH})
set_target_properties(uberz_file PROPERTIES FOLDER Libs)

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
set_target_properties(uberarchive PROPERTIES FOLDER Libs)

# ==================================================================================================
# Build the "core" library (no filamat)
# ==================================================================================================

include_directories(${PUBLIC_HDR_DIR} ${RESOURCE_DIR})
link_libraries(math utils filament cgltf stb ktxreader geometry tsl trie uberzlib)

add_library(gltfio_core STATIC ${PUBLIC_HDRS} ${SRCS})

target_include_directories(gltfio_core PUBLIC ${PUBLIC_HDR_DIR})
set_target_properties(gltfio_core PROPERTIES FOLDER Libs)

target_compile_definitions(gltfio_core PUBLIC -DGLTFIO_DRACO_SUPPORTED=1)
target_link_libraries(gltfio_core PUBLIC dracodec meshoptimizer)

if (WEBGL_PTHREADS)
    target_compile_definitions(gltfio_core PUBLIC -DFILAMENT_WASM_THREADS)
endif()

set(GLTFIO_WARNINGS -Wall -Werror)
if (NOT MSVC)
    target_compile_options(gltfio_core PRIVATE ${GLTFIO_WARNINGS})
endif()

if (NOT WEBGL AND NOT ANDROID AND NOT IOS)

    # ==================================================================================================
    # Link the core library with additional dependencies to create the "full" library
    # ==================================================================================================
    add_library(${TARGET} STATIC ${PUBLIC_HDRS} src/JitShaderProvider.cpp)
    target_link_libraries(${TARGET} PUBLIC filamat gltfio_core)
    target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})
    set_target_properties(${TARGET} PROPERTIES FOLDER Libs)
    if (NOT MSVC)
        target_compile_options(${TARGET} PRIVATE -Wno-deprecated-register)
    endif()

    install(TARGETS ${TARGET} ARCHIVE DESTINATION lib/${DIST_DIR})

endif()

# ==================================================================================================
# Tests
# ==================================================================================================

set(GLTF_TEST_FILES)
function(add_test_gltf SOURCE TARGET)
    set(source_path "${ROOT_DIR}/${SOURCE}")
    set(target_path "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}")
    add_custom_command(
        OUTPUT ${target_path}
        DEPENDS ${source_path}
        COMMAND ${CMAKE_COMMAND} -E copy ${source_path} ${target_path}
    )
    list(APPEND GLTF_TEST_FILES "${target_path}")
    set(GLTF_TEST_FILES ${GLTF_TEST_FILES} PARENT_SCOPE)
endfunction()

add_test_gltf("third_party/models/AnimatedMorphCube/AnimatedMorphCube.glb" "AnimatedMorphCube.glb")

add_custom_target(test_gltfio_files DEPENDS ${GLTF_TEST_FILES})

# The following tests rely on private APIs that are stripped
# away in Release builds
if (TNT_DEV AND NOT WEBGL AND NOT ANDROID AND NOT IOS)
    set(TEST_TARGET test_gltfio)

    add_executable(${TEST_TARGET} test/gltfio_test.cpp)
    add_dependencies(${TEST_TARGET} test_gltfio_files)
    set_property(TARGET test_gltfio PROPERTY LINK_LIBRARIES)

    target_link_libraries(${TEST_TARGET} PRIVATE ${TARGET} gtest uberarchive)
    if (NOT MSVC)
        target_compile_options(${TEST_TARGET} PRIVATE ${GLTFIO_WARNINGS})
    endif()
    set_target_properties(${TEST_TARGET} PROPERTIES FOLDER Tests)
endif()

# ==================================================================================================
# Installation
# ==================================================================================================

install(TARGETS gltfio_core uberarchive ARCHIVE DESTINATION lib/${DIST_DIR})
install(DIRECTORY ${PUBLIC_HDR_DIR}/gltfio DESTINATION include)
install(FILES ${RESOURCE_DIR}/materials/uberarchive.h DESTINATION include/gltfio/materials)
