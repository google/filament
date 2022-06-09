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
        include/filament/DebugRegistry.h
        include/filament/Engine.h
        include/filament/Exposure.h
        include/filament/Fence.h
        include/filament/FilamentAPI.h
        include/filament/Frustum.h
        include/filament/IndexBuffer.h
        include/filament/IndirectLight.h
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
        src/Box.cpp
        src/BufferObject.cpp
        src/Camera.cpp
        src/Color.cpp
        src/ColorSpace.cpp
        src/Culler.cpp
        src/DFG.cpp
        src/DebugRegistry.cpp
        src/Engine.cpp
        src/Exposure.cpp
        src/Fence.cpp
        src/FrameInfo.cpp
        src/FrameSkipper.cpp
        src/Froxelizer.cpp
        src/Frustum.cpp
        src/HwRenderPrimitiveFactory.cpp
        src/IndexBuffer.cpp
        src/IndirectLight.cpp
        src/LightManager.cpp
        src/Material.cpp
        src/MaterialInstance.cpp
        src/MaterialParser.cpp
        src/MorphTargetBuffer.cpp
        src/PerViewUniforms.cpp
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
        src/Allocators.h
        src/BufferPoolAllocator.h
        src/ColorSpace.h
        src/Culler.h
        src/DFG.h
        src/FilamentAPI-impl.h
        src/FrameHistory.h
        src/FrameInfo.h
        src/FrameSkipper.h
        src/Froxelizer.h
        src/HwRenderPrimitiveFactory.h
        src/Intersections.h
        src/MaterialParser.h
        src/PerViewUniforms.h
        src/PIDController.h
        src/PostProcessManager.h
        src/RendererUtils.h
        src/RenderPass.h
        src/RenderPrimitive.h
        src/ResourceAllocator.h
        src/ResourceList.h
        src/ShadowMap.h
        src/ShadowMapManager.h
        src/TypedUniformBuffer.h
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
        src/upcast.h
)

set(MATERIAL_SRCS
        src/materials/fsr/fsr_easu.mat
        src/materials/fsr/fsr_easu_mobile.mat
        src/materials/fsr/fsr_easu_mobileF.mat
        src/materials/fsr/fsr_rcas.mat
        src/materials/colorGrading/colorGrading.mat
        src/materials/colorGrading/colorGradingAsSubpass.mat
        src/materials/colorGrading/customResolveAsSubpass.mat
        src/materials/defaultMaterial.mat
        src/materials/dof/dof.mat
        src/materials/dof/dofCoc.mat
        src/materials/dof/dofDownsample.mat
        src/materials/dof/dofCombine.mat
        src/materials/dof/dofTiles.mat
        src/materials/dof/dofTilesSwizzle.mat
        src/materials/dof/dofDilate.mat
        src/materials/dof/dofMipmap.mat
        src/materials/dof/dofMedian.mat
        src/materials/flare/flare.mat
        src/materials/blitLow.mat
        src/materials/bloom/bloomDownsample.mat
        src/materials/bloom/bloomUpsample.mat
        src/materials/ssao/bilateralBlur.mat
        src/materials/ssao/bilateralBlurBentNormals.mat
        src/materials/ssao/mipmapDepth.mat
        src/materials/skybox.mat
        src/materials/ssao/sao.mat
        src/materials/ssao/saoBentNormals.mat
        src/materials/separableGaussianBlur1.mat
        src/materials/separableGaussianBlur2.mat
        src/materials/separableGaussianBlur3.mat
        src/materials/separableGaussianBlur4.mat
        src/materials/separableGaussianBlur1L.mat
        src/materials/separableGaussianBlur2L.mat
        src/materials/separableGaussianBlur3L.mat
        src/materials/separableGaussianBlur4L.mat
        src/materials/antiAliasing/fxaa.mat
        src/materials/antiAliasing/taa.mat
        src/materials/vsmMipmap.mat
)

# Embed the binary resource blob for materials.
get_resgen_vars(${RESOURCE_DIR} materials)
list(APPEND PRIVATE_HDRS ${RESGEN_HEADER})
list(APPEND SRCS ${RESGEN_SOURCE})

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

# ==================================================================================================
# Definitions
# ==================================================================================================

# "2" corresponds to SYSTRACE_TAG_FILEMENT (See: utils/Systrace.h)
add_definitions(-DSYSTRACE_TAG=2)
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
            COMMENT "Compiling material ${mat_src} to ${output_path}"
    )
    list(APPEND MATERIAL_BINS ${output_path})
endforeach()

# Additional dependencies on included files for materials

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/colorGrading.filamat"
        DEPENDS ../shaders/src/dithering.fs
        DEPENDS ../shaders/src/vignette.fs
        DEPENDS src/materials/colorGrading/colorGrading.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/colorGradingAsSubpass.filamat"
        DEPENDS ../shaders/src/dithering.fs
        DEPENDS ../shaders/src/vignette.fs
        DEPENDS src/materials/colorGrading/colorGrading.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/fxaa.filamat"
        DEPENDS src/materials/antiAliasing/fxaa.fs
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
        DEPENDS src/materials/ssao/depthUtils.fs
        DEPENDS src/materials/ssao/geometry.fs
        DEPENDS src/materials/ssao/saoImpl.fs
        DEPENDS src/materials/ssao/ssctImpl.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/saoBentNormals.filamat"
        DEPENDS src/materials/ssao/ssaoUtils.fs
        DEPENDS src/materials/ssao/ssct.fs
        DEPENDS src/materials/ssao/depthUtils.fs
        DEPENDS src/materials/ssao/geometry.fs
        DEPENDS src/materials/ssao/saoImpl.fs
        DEPENDS src/materials/ssao/ssctImpl.fs
        APPEND
)

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
        OUTPUT "${MATERIAL_DIR}/separableGaussianBlur1.filamat"
        DEPENDS src/materials/separableGaussianBlur.vs
        DEPENDS src/materials/separableGaussianBlur.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/separableGaussianBlur2.filamat"
        DEPENDS src/materials/separableGaussianBlur.vs
        DEPENDS src/materials/separableGaussianBlur.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/separableGaussianBlur3.filamat"
        DEPENDS src/materials/separableGaussianBlur.vs
        DEPENDS src/materials/separableGaussianBlur.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/separableGaussianBlur4.filamat"
        DEPENDS src/materials/separableGaussianBlur.vs
        DEPENDS src/materials/separableGaussianBlur.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/separableGaussianBlur1L.filamat"
        DEPENDS src/materials/separableGaussianBlur.vs
        DEPENDS src/materials/separableGaussianBlur.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/separableGaussianBlur2L.filamat"
        DEPENDS src/materials/separableGaussianBlur.vs
        DEPENDS src/materials/separableGaussianBlur.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/separableGaussianBlur3L.filamat"
        DEPENDS src/materials/separableGaussianBlur.vs
        DEPENDS src/materials/separableGaussianBlur.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/separableGaussianBlur4L.filamat"
        DEPENDS src/materials/separableGaussianBlur.vs
        DEPENDS src/materials/separableGaussianBlur.fs
        APPEND
)

add_custom_command(
        OUTPUT ${RESGEN_OUTPUTS}
        COMMAND resgen ${RESGEN_FLAGS} ${MATERIAL_BINS}
        DEPENDS resgen ${MATERIAL_BINS}
        COMMENT "Aggregating compiled materials"
)

if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
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
target_link_libraries(${TARGET} PUBLIC ibl-lite)

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
elseif(APPLE)
    set(OPTIMIZATION_FLAGS
        -ffast-math
        -ffp-contract=fast
        # TODO: aggressive vectorization is currently broken on Android
        #        -fslp-vectorize-aggressive
        -fvisibility-inlines-hidden
        -fstrict-vtable-pointers
    )
else()
    set(OPTIMIZATION_FLAGS
    -ffast-math
    -ffp-contract=fast
    -fvisibility-inlines-hidden
    )
endif()

set(LINUX_LINKER_OPTIMIZATION_FLAGS
        -Wl,--exclude-libs,bluegl
)

set(LINUX_COMPILER_FLAGS
)

if (MSVC)
    set(FILAMENT_WARNINGS /W3)
elseif(APPLE)
    set(FILAMENT_WARNINGS
            -Wall -Wextra -Wno-unused-parameter
            -Wextra-semi -Wnewline-eof -Wdeprecated -Wundef
            -Wgnu-conditional-omitted-operand
            -Wweak-vtables -Wnon-virtual-dtor -Wclass-varargs -Wimplicit-fallthrough
            -Wover-aligned
    )
else()
    set(FILAMENT_WARNINGS
            -Wall -Wextra -Wno-unused-parameter
            -Wextra-semi -Wdeprecated -Wundef
            -Wnon-virtual-dtor -Wimplicit-fallthrough
    )
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
