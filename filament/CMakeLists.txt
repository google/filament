cmake_minimum_required(VERSION 3.19)
project(filament C ASM)

set(TARGET filament)
set(PUBLIC_HDR_DIR include)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR})
set(RESOURCE_DIR "${GENERATION_ROOT}/generated/resources")
set(MATERIAL_DIR "${GENERATION_ROOT}/generated/material")

# ==================================================================================================
# Sources and headers
# ==================================================================================================

set(PUBLIC_HDRS
        include/filament/Box.h
        include/filament/BufferObject.h
        include/filament/Camera.h
        include/filament/Color.h
        include/filament/ColorGrading.h
        include/filament/ColorSpace.h
        include/filament/DebugRegistry.h
        include/filament/Engine.h
        include/filament/Exposure.h
        include/filament/Fence.h
        include/filament/FilamentAPI.h
        include/filament/Frustum.h
        include/filament/IndexBuffer.h
        include/filament/IndirectLight.h
        include/filament/InstanceBuffer.h
        include/filament/LightManager.h
        include/filament/Material.h
        include/filament/MaterialInstance.h
        include/filament/MorphTargetBuffer.h
        include/filament/Options.h
        include/filament/RenderTarget.h
        include/filament/RenderableManager.h
        include/filament/Renderer.h
        include/filament/Scene.h
        include/filament/SkinningBuffer.h
        include/filament/Skybox.h
        include/filament/Stream.h
        include/filament/SwapChain.h
        include/filament/Texture.h
        include/filament/TextureSampler.h
        include/filament/ToneMapper.h
        include/filament/TransformManager.h
        include/filament/VertexBuffer.h
        include/filament/View.h
        include/filament/Viewport.h
)

set(SRCS
        src/materials/antiAliasing/fxaa/fxaa.cpp
        src/materials/antiAliasing/taa/taa.cpp
        src/materials/bloom/bloom.cpp
        src/materials/colorGrading/colorGrading.cpp
        src/materials/dof/dof.cpp
        src/materials/flare/flare.cpp
        src/materials/fsr/fsr.cpp
        src/materials/sgsr/sgsr.cpp
        src/materials/ssao/ssao.cpp
        src/AtlasAllocator.cpp
        src/BufferObject.cpp
        src/Camera.cpp
        src/Color.cpp
        src/ColorSpaceUtils.cpp
        src/Culler.cpp
        src/DFG.cpp
        src/DebugRegistry.cpp
        src/Engine.cpp
        src/Exposure.cpp
        src/Fence.cpp
        src/FilamentBuilder.cpp
        src/FrameInfo.cpp
        src/FrameSkipper.cpp
        src/Froxelizer.cpp
        src/Frustum.cpp
        src/HwDescriptorSetLayoutFactory.cpp
        src/HwRenderPrimitiveFactory.cpp
        src/HwVertexBufferInfoFactory.cpp
        src/IndexBuffer.cpp
        src/IndirectLight.cpp
        src/InstanceBuffer.cpp
        src/LightManager.cpp
        src/Material.cpp
        src/MaterialInstance.cpp
        src/MaterialInstanceManager.cpp
        src/MaterialParser.cpp
        src/MorphTargetBuffer.cpp
        src/PostProcessManager.cpp
        src/RenderPass.cpp
        src/RenderPrimitive.cpp
        src/RenderTarget.cpp
        src/RenderableManager.cpp
        src/Renderer.cpp
        src/RendererUtils.cpp
        src/ResourceAllocator.cpp
        src/ResourceList.cpp
        src/Scene.cpp
        src/ShadowMap.cpp
        src/ShadowMapManager.cpp
        src/SkinningBuffer.cpp
        src/Skybox.cpp
        src/Stream.cpp
        src/SwapChain.cpp
        src/Texture.cpp
        src/ToneMapper.cpp
        src/TransformManager.cpp
        src/UniformBuffer.cpp
        src/VertexBuffer.cpp
        src/View.cpp
        src/components/CameraManager.cpp
        src/components/LightManager.cpp
        src/components/RenderableManager.cpp
        src/components/TransformManager.cpp
        src/details/BufferObject.cpp
        src/details/Camera.cpp
        src/details/ColorGrading.cpp
        src/details/DebugRegistry.cpp
        src/details/Engine.cpp
        src/details/Fence.cpp
        src/details/IndexBuffer.cpp
        src/details/IndirectLight.cpp
        src/details/InstanceBuffer.cpp
        src/details/Material.cpp
        src/details/MaterialInstance.cpp
        src/details/MorphTargetBuffer.cpp
        src/details/RenderTarget.cpp
        src/details/Renderer.cpp
        src/details/Scene.cpp
        src/details/SkinningBuffer.cpp
        src/details/Skybox.cpp
        src/details/Stream.cpp
        src/details/SwapChain.cpp
        src/details/Texture.cpp
        src/details/VertexBuffer.cpp
        src/details/View.cpp
        src/ds/PerViewDescriptorSetUtils.cpp
        src/ds/ColorPassDescriptorSet.cpp
        src/ds/DescriptorSet.cpp
        src/ds/DescriptorSetLayout.cpp
        src/ds/PostProcessDescriptorSet.cpp
        src/ds/ShadowMapDescriptorSet.cpp
        src/ds/SsrPassDescriptorSet.cpp
        src/ds/StructureDescriptorSet.cpp
        src/fg/Blackboard.cpp
        src/fg/DependencyGraph.cpp
        src/fg/FrameGraph.cpp
        src/fg/FrameGraphPass.cpp
        src/fg/FrameGraphResources.cpp
        src/fg/FrameGraphTexture.cpp
        src/fg/PassNode.cpp
        src/fg/ResourceNode.cpp
        src/fsr.cpp
)

set(PRIVATE_HDRS
        src/materials/StaticMaterialInfo.h
        src/Allocators.h
        src/Bimap.h
        src/BufferPoolAllocator.h
        src/ColorSpaceUtils.h
        src/Culler.h
        src/DFG.h
        src/FilamentAPI-impl.h
        src/FrameHistory.h
        src/FrameInfo.h
        src/FrameSkipper.h
        src/Froxelizer.h
        src/HwDescriptorSetLayoutFactory.h
        src/HwRenderPrimitiveFactory.h
        src/HwVertexBufferInfoFactory.h
        src/Intersections.h
        src/MaterialParser.h
        src/MaterialInstanceManager.h
        src/PIDController.h
        src/PostProcessManager.h
        src/RenderPass.h
        src/RenderPrimitive.h
        src/RendererUtils.h
        src/ResourceAllocator.h
        src/ResourceList.h
        src/ShadowMap.h
        src/ShadowMapManager.h
        src/SharedHandle.h
        src/UniformBuffer.h
        src/components/CameraManager.h
        src/components/LightManager.h
        src/components/RenderableManager.h
        src/components/TransformManager.h
        src/details/BufferObject.h
        src/details/Camera.h
        src/details/ColorGrading.h
        src/details/DebugRegistry.h
        src/details/Engine.h
        src/details/Fence.h
        src/details/IndexBuffer.h
        src/details/IndirectLight.h
        src/details/InstanceBuffer.h
        src/details/Material.h
        src/details/MaterialInstance.h
        src/details/MorphTargetBuffer.h
        src/details/RenderTarget.h
        src/details/Renderer.h
        src/details/Scene.h
        src/details/SkinningBuffer.h
        src/details/Skybox.h
        src/details/Stream.h
        src/details/SwapChain.h
        src/details/Texture.h
        src/details/VertexBuffer.h
        src/details/View.h
        src/downcast.h
        src/ds/ColorPassDescriptorSet.h
        src/ds/DescriptorSetLayout.h
        src/ds/PostProcessDescriptorSet.h
        src/ds/ShadowMapDescriptorSet.h
        src/ds/SsrPassDescriptorSet.h
        src/ds/TypedBuffer.h
        src/ds/TypedUniformBuffer.h
        src/fg/Blackboard.h
        src/fg/FrameGraph.h
        src/fg/FrameGraphId.h
        src/fg/FrameGraphPass.h
        src/fg/FrameGraphRenderPass.h
        src/fg/FrameGraphResources.h
        src/fg/FrameGraphTexture.h
        src/fg/Resource.cpp
        src/fg/details/DependencyGraph.h
        src/fg/details/PassNode.h
        src/fg/details/Resource.h
        src/fg/details/ResourceNode.h
        src/fg/details/Utilities.h
        src/fsr.h
        src/materials/fsr/ffx_a.h
        src/materials/fsr/ffx_fsr1.h
        src/materials/fsr/ffx_fsr1_mobile.fs
)

set(MATERIAL_SRCS
        src/materials/antiAliasing/fxaa/fxaa.mat
        src/materials/antiAliasing/taa/taa.mat
        src/materials/blitDepth.mat
        src/materials/blitLow.mat
        src/materials/blitArray.mat
        src/materials/bloom/bloomDownsample.mat
        src/materials/bloom/bloomDownsample2x.mat
        src/materials/bloom/bloomDownsample9.mat
        src/materials/bloom/bloomUpsample.mat
        src/materials/colorGrading/colorGrading.mat
        src/materials/colorGrading/colorGradingAsSubpass.mat
        src/materials/colorGrading/customResolveAsSubpass.mat
        src/materials/debugShadowCascades.mat
        src/materials/defaultMaterial.mat
        src/materials/dof/dof.mat
        src/materials/dof/dofCoc.mat
        src/materials/dof/dofCombine.mat
        src/materials/dof/dofDilate.mat
        src/materials/dof/dofDownsample.mat
        src/materials/dof/dofMedian.mat
        src/materials/dof/dofMipmap.mat
        src/materials/dof/dofTiles.mat
        src/materials/dof/dofTilesSwizzle.mat
        src/materials/flare/flare.mat
        src/materials/fsr/fsr_easu.mat
        src/materials/fsr/fsr_easu_mobile.mat
        src/materials/fsr/fsr_easu_mobileF.mat
        src/materials/fsr/fsr_rcas.mat
        src/materials/sgsr/sgsr1.mat
        src/materials/resolveDepth.mat
        src/materials/separableGaussianBlur.mat
        src/materials/skybox.mat
        src/materials/shadowmap.mat
        src/materials/ssao/bilateralBlur.mat
        src/materials/ssao/bilateralBlurBentNormals.mat
        src/materials/ssao/mipmapDepth.mat
        src/materials/ssao/sao.mat
        src/materials/ssao/saoBentNormals.mat
        src/materials/vsmMipmap.mat
)

if (NOT FILAMENT_DISABLE_GTAO)
    list(APPEND MATERIAL_SRCS
        src/materials/ssao/gtao.mat
        src/materials/ssao/gtaoBentNormals.mat
    )
endif()

set(MATERIAL_FL0_SRCS
        src/materials/defaultMaterial.mat
        src/materials/skybox.mat
)

set(MATERIAL_MULTIVIEW_SRCS
        src/materials/defaultMaterial.mat
        src/materials/skybox.mat
)

# ==================================================================================================
# Configuration
# ==================================================================================================

# whether we're building for mobile -- this can affect some default quality settings
if (IS_MOBILE_TARGET)
    add_definitions(-DFILAMENT_TARGET_MOBILE=1)
endif()

# Size of the DFG lookup table
if (NOT DFG_LUT_SIZE)
    if (IS_MOBILE_TARGET)
        set(DFG_LUT_SIZE 64)
    else()
        set(DFG_LUT_SIZE 128)
    endif()
endif()
message(STATUS "DFG LUT size set to ${DFG_LUT_SIZE}x${DFG_LUT_SIZE}")

# Whether to include FL0 materials.
if (FILAMENT_ENABLE_FEATURE_LEVEL_0)
    add_definitions(-DFILAMENT_ENABLE_FEATURE_LEVEL_0)
endif()

# Whether to include MULTIVIEW materials.
if (FILAMENT_ENABLE_MULTIVIEW)
    add_definitions(-DFILAMENT_ENABLE_MULTIVIEW)
endif()

# Whether to force the profiling mode.
if (FILAMENT_FORCE_PROFILING_MODE)
    add_definitions(-DFILAMENT_FORCE_PROFILING_MODE)
endif()

if (FILAMENT_DISABLE_GTAO)
    add_definitions(-DFILAMENT_DISABLE_GTAO)
endif()

# ==================================================================================================
# Definitions
# ==================================================================================================

add_definitions(-DFILAMENT_DFG_LUT_SIZE=${DFG_LUT_SIZE})
add_definitions(
    -DFILAMENT_PER_RENDER_PASS_ARENA_SIZE_IN_MB=${FILAMENT_PER_RENDER_PASS_ARENA_SIZE_IN_MB}
    -DFILAMENT_PER_FRAME_COMMANDS_SIZE_IN_MB=${FILAMENT_PER_FRAME_COMMANDS_SIZE_IN_MB}
    -DFILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB=${FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB}
    -DFILAMENT_OPENGL_HANDLE_ARENA_SIZE_IN_MB=${FILAMENT_OPENGL_HANDLE_ARENA_SIZE_IN_MB}
    -DFILAMENT_METAL_HANDLE_ARENA_SIZE_IN_MB=${FILAMENT_METAL_HANDLE_ARENA_SIZE_IN_MB}
)

# ==================================================================================================
# Generate all .filamat: default material, skyboxes, and post-process
# ==================================================================================================

if (CMAKE_CROSSCOMPILING)
    include(${IMPORT_EXECUTABLES})
endif()

file(MAKE_DIRECTORY ${MATERIAL_DIR})
file(MAKE_DIRECTORY ${RESOURCE_DIR})

set(GENERATED_RESOURCE_HDRS)
set(GENERATED_RESOURCE_SRCS)

# Get all unique directories containing materials
set(MATERIAL_DIRS)
foreach(mat_src ${MATERIAL_SRCS})
    get_filename_component(mat_dir ${mat_src} DIRECTORY)
    list(APPEND MATERIAL_DIRS ${mat_dir})
endforeach()
list(REMOVE_DUPLICATES MATERIAL_DIRS)

# For each directory, generate a single resource package
foreach(mat_dir ${MATERIAL_DIRS})
    get_filename_component(package_name ${mat_dir} NAME)

    set(MATERIALS_IN_DIR)
    foreach(mat_src ${MATERIAL_SRCS})
        get_filename_component(current_mat_dir ${mat_src} DIRECTORY)
        if(current_mat_dir STREQUAL mat_dir)
            list(APPEND MATERIALS_IN_DIR ${mat_src})
        endif()
    endforeach()

    set(FILAMAT_FILES_FOR_GROUP)
    set(FILAMAT_TARGETS_FOR_GROUP)

    # Compile each material in the group
    foreach(mat_src ${MATERIALS_IN_DIR})
        get_filename_component(localname "${mat_src}" NAME_WE)
        get_filename_component(fullname "${mat_src}" ABSOLUTE)

        # --- Base material ---
        set(output_path "${MATERIAL_DIR}/${localname}.filamat")
        add_custom_command(
                OUTPUT ${output_path}
                COMMAND matc ${MATC_BASE_FLAGS} -o ${output_path} ${fullname}
                MAIN_DEPENDENCY ${fullname}
                DEPENDS matc
                COMMENT "Compiling material ${fullname}"
        )
        list(APPEND FILAMAT_FILES_FOR_GROUP ${output_path})
        list(APPEND FILAMAT_TARGETS_FOR_GROUP ${output_path})

        # --- Feature Level 0 variant ---
        list(FIND MATERIAL_FL0_SRCS ${mat_src} index)
        if (${index} GREATER -1 AND FILAMENT_ENABLE_FEATURE_LEVEL_0)
            string(REGEX REPLACE "[.]filamat$" "_fl0.filamat" output_path_fl0 ${output_path})
            add_custom_command(
                    OUTPUT ${output_path_fl0}
                    COMMAND matc ${MATC_BASE_FLAGS} -PfeatureLevel=0 -o ${output_path_fl0} ${fullname}
                    MAIN_DEPENDENCY ${fullname}
                    DEPENDS matc
                    COMMENT "Compiling material ${fullname} (FL0)"
            )
            list(APPEND FILAMAT_FILES_FOR_GROUP ${output_path_fl0})
            list(APPEND FILAMAT_TARGETS_FOR_GROUP ${output_path_fl0})
        endif()

        # --- Multiview variant ---
        list(FIND MATERIAL_MULTIVIEW_SRCS ${mat_src} index)
        if (${index} GREATER -1 AND FILAMENT_ENABLE_MULTIVIEW)
            string(REGEX REPLACE "[.]filamat$" "_multiview.filamat" output_path_multiview ${output_path})
            add_custom_command(
                    OUTPUT ${output_path_multiview}
                    COMMAND matc ${MATC_BASE_FLAGS} -PstereoscopicType=multiview -o ${output_path_multiview} ${fullname}
                    MAIN_DEPENDENCY ${fullname}
                    DEPENDS matc
                    COMMENT "Compiling material ${fullname} (Multiview)"
            )
            list(APPEND FILAMAT_FILES_FOR_GROUP ${output_path_multiview})
            list(APPEND FILAMAT_TARGETS_FOR_GROUP ${output_path_multiview})
        endif()
    endforeach()

    # Generate a single resource file for the whole group
    get_resgen_vars(${RESOURCE_DIR} ${package_name})
    add_custom_command(
            OUTPUT ${RESGEN_OUTPUTS}
            COMMAND resgen ${RESGEN_FLAGS} ${FILAMAT_FILES_FOR_GROUP}
            DEPENDS resgen ${FILAMAT_TARGETS_FOR_GROUP}
            COMMENT "Generating resource package for ${package_name}"
    )
    list(APPEND GENERATED_RESOURCE_HDRS ${RESGEN_HEADER})
    list(APPEND GENERATED_RESOURCE_SRCS ${RESGEN_SOURCE})
endforeach()

# Additional dependencies on included files for materials

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/colorGrading.filamat"
        DEPENDS ../shaders/src/inline_dithering.fs
        DEPENDS ../shaders/src/inline_vignette.fs
        DEPENDS src/materials/colorGrading/colorGrading.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/colorGradingAsSubpass.filamat"
        DEPENDS ../shaders/src/inline_dithering.fs
        DEPENDS ../shaders/src/inline_vignette.fs
        DEPENDS src/materials/colorGrading/colorGrading.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/fxaa.filamat"
        DEPENDS src/materials/antiAliasing/fxaa/fxaa.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/dofDownsample.filamat"
        DEPENDS src/materials/dof/dofUtils.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/dofMipmap.filamat"
        DEPENDS src/materials/dof/dofUtils.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/dofDilate.filamat"
        DEPENDS src/materials/dof/dofUtils.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/dof.filamat"
        DEPENDS src/materials/dof/dofUtils.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/dofMedian.filamat"
        DEPENDS src/materials/dof/dofUtils.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/dofCombine.filamat"
        DEPENDS src/materials/dof/dofUtils.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/sao.filamat"
        DEPENDS src/materials/ssao/ssaoUtils.fs
        DEPENDS src/materials/ssao/ssct.fs
        DEPENDS src/materials/utils/depthUtils.fs
        DEPENDS src/materials/utils/geometry.fs
        DEPENDS src/materials/ssao/saoImpl.fs
        DEPENDS src/materials/ssao/ssctImpl.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/saoBentNormals.filamat"
        DEPENDS src/materials/ssao/ssaoUtils.fs
        DEPENDS src/materials/ssao/ssct.fs
        DEPENDS src/materials/utils/depthUtils.fs
        DEPENDS src/materials/utils/geometry.fs
        DEPENDS src/materials/ssao/saoImpl.fs
        DEPENDS src/materials/ssao/ssctImpl.fs
        APPEND
)

if (NOT FILAMENT_DISABLE_GTAO)
    add_custom_command(
            OUTPUT "${MATERIAL_DIR}/gtao.filamat"
            DEPENDS src/materials/ssao/ssaoUtils.fs
            DEPENDS src/materials/ssao/ssct.fs
            DEPENDS src/materials/utils/depthUtils.fs
            DEPENDS src/materials/utils/geometry.fs
            DEPENDS src/materials/ssao/gtaoImpl.fs
            DEPENDS src/materials/ssao/ssctImpl.fs
            APPEND
    )

    add_custom_command(
            OUTPUT "${MATERIAL_DIR}/gtaoBentNormals.filamat"
            DEPENDS src/materials/ssao/ssaoUtils.fs
            DEPENDS src/materials/ssao/ssct.fs
            DEPENDS src/materials/utils/depthUtils.fs
            DEPENDS src/materials/utils/geometry.fs
            DEPENDS src/materials/ssao/gtaoImpl.fs
            DEPENDS src/materials/ssao/ssctImpl.fs
            APPEND
    )
endif()

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/bilateralBlur.filamat"
        DEPENDS src/materials/ssao/ssaoUtils.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/bilateralBlurBentNormals.filamat"
        DEPENDS src/materials/ssao/ssaoUtils.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/fsr_easu.filamat"
        DEPENDS src/materials/fsr/ffx_a.h
        DEPENDS src/materials/fsr/ffx_fsr1.h
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/fsr_easu_mobile.filamat"
        DEPENDS src/materials/fsr/ffx_a.h
        DEPENDS src/materials/fsr/ffx_fsr1.h
        DEPENDS src/materials/fsr/ffx_fsr1_mobile.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/fsr_easu_mobileF.filamat"
        DEPENDS src/materials/fsr/ffx_a.h
        DEPENDS src/materials/fsr/ffx_fsr1.h
        DEPENDS src/materials/fsr/ffx_fsr1_mobile.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/fsr_rcas.filamat"
        DEPENDS src/materials/fsr/ffx_a.h
        DEPENDS src/materials/fsr/ffx_fsr1.h
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/sgsr1.filamat"
        DEPENDS src/materials/sgsr/sgsr1_shader_mobile.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/separableGaussianBlur.filamat"
        DEPENDS src/materials/separableGaussianBlur.vs
        DEPENDS src/materials/separableGaussianBlur.fs
        APPEND
)

# Add the generated resource files to the source and header lists.
list(APPEND PRIVATE_HDRS ${GENERATED_RESOURCE_HDRS})
list(APPEND SRCS ${GENERATED_RESOURCE_SRCS})

if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${GENERATED_RESOURCE_SRCS} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()

file(MAKE_DIRECTORY "${GENERATION_ROOT}/generated/data/")

set(output_path "${GENERATION_ROOT}/generated/data/dfg.inc")
add_custom_command(
        OUTPUT ${output_path}
        COMMAND cmgen --quiet --size=${DFG_LUT_SIZE} --ibl-dfg-multiscatter --ibl-dfg-cloth --ibl-dfg=${output_path}
        DEPENDS cmgen
        COMMENT "Generating DFG LUT ${output_path}"
)
list(APPEND DATA_BINS ${output_path})

# ==================================================================================================
# Includes & target definition
# ==================================================================================================
# specify where our headers are
include_directories(${PUBLIC_HDR_DIR})
include_directories(${GENERATION_ROOT})
include_directories(src)

# we're building a library
add_library(${TARGET} STATIC ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS} ${DATA_BINS})

# specify where the public headers of this library are
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

# add this subproject to the Filament folder
set_target_properties(${TARGET} PROPERTIES FOLDER Filament)

# ==================================================================================================
# Dependencies
# ==================================================================================================

target_link_libraries(${TARGET} PUBLIC backend)
target_link_libraries(${TARGET} PUBLIC math)
target_link_libraries(${TARGET} PUBLIC utils)
target_link_libraries(${TARGET} PUBLIC filaflat)
target_link_libraries(${TARGET} PUBLIC filabridge)

if (FILAMENT_USE_ABSEIL_LOGGING)
    target_link_libraries(${TARGET} PUBLIC absl::log)
endif()

if (FILAMENT_ENABLE_FGVIEWER)
    target_link_libraries(${TARGET} PUBLIC fgviewer)
    add_definitions(-DFILAMENT_ENABLE_FGVIEWER=1)
else()
    add_definitions(-DFILAMENT_ENABLE_FGVIEWER=0)
endif()

if (FILAMENT_ENABLE_MATDBG)
    target_link_libraries(${TARGET} PUBLIC matdbg)
    add_definitions(-DFILAMENT_ENABLE_MATDBG=1)
else()
    add_definitions(-DFILAMENT_ENABLE_MATDBG=0)
endif()

if (LINUX)
    target_link_libraries(${TARGET} PRIVATE dl)
endif()

# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (MSVC)
    set(OPTIMIZATION_FLAGS
        /fp:fast
    )
elseif(WEBGL)
    # Avoid strict-vtable-pointers here, it is broken in WebAssembly.
    set(OPTIMIZATION_FLAGS -fvisibility-inlines-hidden)
else()
    set(OPTIMIZATION_FLAGS
        -ffast-math
        -fno-finite-math-only
        -ffp-contract=fast
        # TODO: aggressive vectorization is currently broken on Android
        #        -fslp-vectorize-aggressive
        -fvisibility-inlines-hidden
        -fstrict-vtable-pointers
    )
endif()

set(LINUX_LINKER_OPTIMIZATION_FLAGS
        -Wl,--exclude-libs,bluegl
)

set(LINUX_COMPILER_FLAGS
)

if (MSVC)
    set(FILAMENT_WARNINGS /W3)
else()
    set(FILAMENT_WARNINGS
            -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers
            -Wextra-semi -Wnewline-eof -Wdeprecated -Wundef
            -Wgnu-conditional-omitted-operand
            -Wweak-vtables -Wnon-virtual-dtor -Wclass-varargs -Wimplicit-fallthrough
            -Wover-aligned
            -Werror
    )
    if (CMAKE_CXX_STANDARD EQUAL 20)
        # The lambdas for passes in PostProcessManager.cpp capture this
        # implicitly in a way that's deprecated in c++20, but can't easily be
        # fixed in a way that's backwards compatible with c++17:
        # https://www.nextptr.com/tutorial/ta1430524603/capture-this-in-lambda-expression-timeline-of-change
        list(APPEND FILAMENT_WARNINGS -Wno-deprecated-this-capture)
    endif()
endif()

target_compile_options(${TARGET} PRIVATE
        ${FILAMENT_WARNINGS}
        $<$<CONFIG:Release>:${OPTIMIZATION_FLAGS}>
        $<$<AND:$<PLATFORM_ID:Darwin>,$<CONFIG:Release>>:${DARWIN_OPTIMIZATION_FLAGS}>
        $<$<PLATFORM_ID:Linux>:${LINUX_COMPILER_FLAGS}>
)

target_link_libraries(${TARGET} PRIVATE
        $<$<AND:$<PLATFORM_ID:Linux>,$<CONFIG:Release>>:${LINUX_LINKER_OPTIMIZATION_FLAGS}>
)

# ==================================================================================================
# Installation
# ==================================================================================================
set(INSTALL_TYPE ARCHIVE)
install(TARGETS ${TARGET} ${INSTALL_TYPE} DESTINATION lib/${DIST_DIR})
install(DIRECTORY ${PUBLIC_HDR_DIR}/filament DESTINATION include)
install(FILES "README.md" DESTINATION .)
install(FILES "../LICENSE" DESTINATION .)

# ==================================================================================================
# Sub-projects
# ==================================================================================================
add_subdirectory(backend)
add_subdirectory(test)
add_subdirectory(benchmark)
