cmake_minimum_required(VERSION 3.10)
project(filament C ASM)

set(TARGET filament)
set(PUBLIC_HDR_DIR include)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR})
set(RESOURCE_DIR  "${GENERATION_ROOT}/generated/resources")
set(MATERIAL_DIR  "${GENERATION_ROOT}/generated/material")

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/filament/Box.h
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
        include/filament/RenderableManager.h
        include/filament/RenderTarget.h
        include/filament/Renderer.h
        include/filament/Scene.h
        include/filament/Skybox.h
        include/filament/Stream.h
        include/filament/SwapChain.h
        include/filament/Texture.h
        include/filament/TextureSampler.h
        include/filament/TransformManager.h
        include/filament/VertexBuffer.h
        include/filament/View.h
        include/filament/Viewport.h)

set(SRCS
        src/components/CameraManager.cpp
        src/components/LightManager.cpp
        src/components/RenderableManager.cpp
        src/components/TransformManager.cpp
        src/fg/Blackboard.cpp
        src/fg/FrameGraph.cpp
        src/fg/FrameGraphHandle.cpp
        src/fg/FrameGraphPassResources.cpp
        src/fg/fg/PassNode.cpp
        src/fg/fg/RenderTargetResourceEntry.cpp
        src/fg/fg/ResourceEntry.cpp
        src/fg/ResourceAllocator.cpp
        src/Box.cpp
        src/Camera.cpp
        src/Color.cpp
        src/ColorGrading.cpp
        src/Culler.cpp
        src/DebugRegistry.cpp
        src/DFG.cpp
        src/VertexBuffer.cpp
        src/Engine.cpp
        src/Exposure.cpp
        src/Fence.cpp
        src/FrameInfo.cpp
        src/FrameSkipper.cpp
        src/Froxelizer.cpp
        src/Frustum.cpp
        src/GPUBuffer.cpp
        src/IndexBuffer.cpp
        src/IndirectLight.cpp
        src/Material.cpp
        src/MaterialParser.cpp
        src/MaterialInstance.cpp
        src/PostProcessManager.cpp
        src/Renderer.cpp
        src/RenderPass.cpp
        src/RenderPrimitive.cpp
        src/RenderTarget.cpp
        src/Scene.cpp
        src/ShadowMap.cpp
        src/ShadowMapManager.cpp
        src/Skybox.cpp
        src/SwapChain.cpp
        src/Stream.cpp
        src/Texture.cpp
        src/UniformBuffer.cpp
        src/View.cpp
        src/Viewport.cpp
)

set(PRIVATE_HDRS
        src/components/CameraManager.h
        src/components/LightManager.h
        src/components/RenderableManager.h
        src/components/TransformManager.h
        src/fg/Blackboard.h
        src/fg/FrameGraph.h
        src/fg/FrameGraphPass.h
        src/fg/FrameGraphPassResources.h
        src/fg/FrameGraphHandle.h
        src/fg/fg/PassNode.h
        src/fg/fg/ResourceEntry.h
        src/fg/fg/RenderTargetResourceEntry.h
        src/fg/fg/ResourceNode.h
        src/fg/ResourceAllocator.h
        src/fg/fg/VirtualResource.h
        src/details/Allocators.h
        src/details/Camera.h
        src/details/ColorGrading.h
        src/details/Culler.h
        src/details/DebugRegistry.h
        src/details/DFG.h
        src/details/Engine.h
        src/details/Fence.h
        src/details/FrameSkipper.h
        src/details/Froxelizer.h
        src/details/IndexBuffer.h
        src/details/IndirectLight.h
        src/details/Material.h
        src/details/MaterialInstance.h
        src/details/RenderPrimitive.h
        src/details/Renderer.h
        src/details/RenderTarget.h
        src/details/ResourceList.h
        src/details/Scene.h
        src/details/ShadowMap.h
        src/details/ShadowMapManager.h
        src/details/Skybox.h
        src/details/Stream.h
        src/details/SwapChain.h
        src/details/Texture.h
        src/details/VertexBuffer.h
        src/details/View.h
        src/FilamentAPI-impl.h
        src/FrameInfo.h
        src/GPUBuffer.h
        src/Intersections.h
        src/MaterialParser.h
        src/PostProcessManager.h
        src/RenderPass.h
        src/ToneMapping.h
        src/UniformBuffer.h
        src/upcast.h)

set(MATERIAL_SRCS
        src/materials/colorGrading.mat
        src/materials/colorGradingAsSubpass.mat
        src/materials/defaultMaterial.mat
        src/materials/dof.mat
        src/materials/dofBlur.mat
        src/materials/blitLow.mat
        src/materials/blitMedium.mat
        src/materials/blitHigh.mat
        src/materials/bloomDownsample.mat
        src/materials/bloomUpsample.mat
        src/materials/bilateralBlur.mat
        src/materials/mipmapDepth.mat
        src/materials/skybox.mat
        src/materials/sao.mat
        src/materials/separableGaussianBlur.mat
        src/materials/fxaa.mat
)

# Embed the binary resource blob for materials.
get_resgen_vars(${RESOURCE_DIR} materials)
list(APPEND PRIVATE_HDRS ${RESGEN_HEADER})
list(APPEND SRCS ${RESGEN_SOURCE})

# ==================================================================================================
# Configuration
# ==================================================================================================

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
        DEPENDS ../shaders/src/bloom.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/colorGradingAsSubpass.filamat"
        DEPENDS ../shaders/src/dithering.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/fxaa.filamat"
        DEPENDS ../shaders/src/fxaa.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/dof.filamat"
        DEPENDS src/materials/dofUtils.fs
        APPEND
)

add_custom_command(
        OUTPUT "${MATERIAL_DIR}/dofBlur.filamat"
        DEPENDS src/materials/dofUtils.fs
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

set(DATA_BINS)
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

# ==================================================================================================
# Dependencies
# ==================================================================================================

target_link_libraries(${TARGET} PUBLIC backend)
target_link_libraries(${TARGET} PUBLIC math)
target_link_libraries(${TARGET} PUBLIC utils)
target_link_libraries(${TARGET} PUBLIC geometry) # TODO: remove this dependency after deprecating VertexBuffer::populateTangentQuaternions
target_link_libraries(${TARGET} PUBLIC filaflat)
target_link_libraries(${TARGET} PUBLIC filabridge)
target_link_libraries(${TARGET} PUBLIC image_headers)
target_link_libraries(${TARGET} PUBLIC ibl)

# Link in matdbg for Desktop + Debug only since it pulls in filamat and a web server.
if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT IOS AND NOT ANDROID AND NOT WEBGL)
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
    set(OPTIMIZATION_FLAGS -fvisibility=hidden -fvisibility-inlines-hidden)
else()
    set(OPTIMIZATION_FLAGS
        -ffast-math
        -ffp-contract=fast
        # TODO: aggressive vectorization is currently broken on Android
        #        -fslp-vectorize-aggressive
        -fvisibility=hidden
        -fvisibility-inlines-hidden
        -fstrict-vtable-pointers
    )
endif()

set(LINUX_LINKER_OPTIMIZATION_FLAGS
        -Wl,--exclude-libs,bluegl
)

set(LINUX_COMPILER_FLAGS
        -fPIC
)

if (MSVC)
    set(FILAMENT_WARNINGS /W3)
else()
    set(FILAMENT_WARNINGS
            -Wall -Wextra -Wno-unused-parameter
            -Wextra-semi -Wnewline-eof -Wdeprecated -Wundef
            -Wgnu-conditional-omitted-operand
            -Wweak-vtables -Wnon-virtual-dtor -Wclass-varargs -Wimplicit-fallthrough
            -Wover-aligned
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

# ==================================================================================================
# Sub-projects
# ==================================================================================================
add_subdirectory(backend)
add_subdirectory(test)
add_subdirectory(benchmark)
add_subdirectory(tools)
