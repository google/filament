cmake_minimum_required(VERSION 3.1)
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
        include/filament/driver/BufferDescriptor.h
        include/filament/driver/Platform.h
        include/filament/driver/PixelBufferDescriptor.h
        include/filament/Box.h
        include/filament/Camera.h
        include/filament/Color.h
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
        src/driver/opengl/gl_headers.cpp
        src/driver/opengl/OpenGLBlitter.cpp
        src/driver/opengl/GLUtils.cpp
        src/driver/opengl/OpenGLDriver.cpp
        src/driver/opengl/OpenGLProgram.cpp
        src/driver/CommandStream.cpp
        src/driver/CommandBufferQueue.cpp
        src/driver/CircularBuffer.cpp
        src/driver/Driver.cpp
        src/driver/DriverAPI.inc
        src/driver/Platform.cpp
        src/driver/GPUBuffer.cpp
        src/driver/Handle.cpp
        src/driver/Program.cpp
        src/driver/SamplerBuffer.cpp
        src/Box.cpp
        src/Camera.cpp
        src/Color.cpp
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
        src/IndexBuffer.cpp
        src/IndirectLight.cpp
        src/Material.cpp
        src/MaterialInstance.cpp
        src/PostProcessManager.cpp
        src/Renderer.cpp
        src/RenderPass.cpp
        src/RenderPrimitive.cpp
        src/RenderTargetPool.cpp
        src/Scene.cpp
        src/ShadowMap.cpp
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
        src/details/Allocators.h
        src/details/Camera.h
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
        src/details/ResourceList.h
        src/details/Scene.h
        src/details/ShadowMap.h
        src/details/Skybox.h
        src/details/Stream.h
        src/details/SwapChain.h
        src/details/Texture.h
        src/details/VertexBuffer.h
        src/details/View.h
        src/driver/CircularBuffer.h
        src/driver/CommandBufferQueue.h
        src/driver/CommandStream.h
        src/driver/CommandStreamDispatcher.h
        src/driver/Driver.h
        src/driver/DriverAPI.inc
        src/driver/DriverApi.h
        src/driver/DriverApiForward.h
        src/driver/DriverBase.h
        src/driver/GPUBuffer.h
        src/driver/Handle.h
        src/driver/Program.h
        src/driver/SamplerBuffer.h
        src/FilamentAPI-impl.h
        src/FrameInfo.h
        src/Intersections.h
        src/PostProcessManager.h
        src/RenderPass.h
        src/RenderTargetPool.h
        src/UniformBuffer.h
        src/upcast.h)

set(MATERIAL_SRCS
        src/materials/defaultMaterial.mat
        src/materials/skybox.mat
        src/materials/skyboxRGBM.mat
)

# The noop driver is only useful for ensuring we don't have certain build issues.
# Remove it from release builds, since it uses some space needlessly.
if (CMAKE_BUILD_TYPE MATCHES Debug)
    list(APPEND SRCS src/driver/noop/NoopDriver.cpp)
    list(APPEND SRCS src/driver/noop/PlatformNoop.cpp)
endif()

if(IOS AND NOT FILAMENT_SUPPORTS_VULKAN)
    message(FATAL_ERROR "Filament for iOS must be built with Vulkan support.")
endif()

# Embed the binary resource blob for materials.
get_resgen_vars(${RESOURCE_DIR} materials)
list(APPEND PRIVATE_HDRS ${RESGEN_HEADER})
list(APPEND SRCS ${RESGEN_SOURCE})

# ==================================================================================================
# OS specific
# ==================================================================================================

# Here set the architecture specific sources
if (USE_EXTERNAL_GLES3)
    # Experimental feature; clients provide their own Context Manager.
elseif (ANDROID)
    list(APPEND SRCS src/driver/android/ExternalStreamManagerAndroid.cpp)
    list(APPEND SRCS src/driver/android/ExternalTextureManagerAndroid.cpp)
    list(APPEND SRCS src/driver/android/VirtualMachineEnv.cpp)
    list(APPEND SRCS src/driver/opengl/PlatformEGL.cpp)
elseif (IOS)
    list(APPEND SRCS src/driver/opengl/PlatformCocoaTouchGL.mm)
elseif (APPLE)
    list(APPEND SRCS src/driver/opengl/PlatformCocoaGL.mm)
elseif (WEBGL)
    list(APPEND SRCS src/driver/opengl/PlatformWebGL.cpp)
elseif (LINUX)
    list(APPEND SRCS src/driver/opengl/PlatformGLX.cpp)
elseif (WIN32)
    list(APPEND SRCS src/driver/opengl/PlatformWGL.cpp)
else()
    list(APPEND SRCS src/driver/opengl/PlatformDummyGL.cpp)
endif()

# ==================================================================================================
# Definitions
# ==================================================================================================
# "2" corresponds to SYSTRACE_TAG_FILEMENT (See: utils/Systrace.h)
add_definitions(-DSYSTRACE_TAG=2 )

# ==================================================================================================
# Vulkan Sources
# ==================================================================================================

# See root CMakeLists.txt for platforms that support Vulkan
if (FILAMENT_SUPPORTS_VULKAN)
    list(APPEND SRCS
            src/driver/vulkan/VulkanBinder.cpp
            src/driver/vulkan/VulkanBuffer.cpp
            src/driver/vulkan/VulkanDriver.cpp
            src/driver/vulkan/VulkanDriverImpl.cpp
            src/driver/vulkan/VulkanFboCache.cpp
            src/driver/vulkan/VulkanHandles.cpp
            src/driver/vulkan/VulkanSamplerCache.cpp
            src/driver/vulkan/VulkanStagePool.cpp
    )
    if (LINUX)
        list(APPEND SRCS src/driver/vulkan/PlatformVkLinux.cpp)
    elseif (APPLE AND NOT IOS)
        list(APPEND SRCS src/driver/vulkan/PlatformVkCocoa.mm)
    elseif (IOS)
        list(APPEND SRCS src/driver/vulkan/PlatformVkCocoaTouch.mm)
    elseif (ANDROID)
        list(APPEND SRCS src/driver/vulkan/PlatformVkAndroid.cpp)
    elseif (WIN32)
        list(APPEND SRCS src/driver/vulkan/PlatformVkWindows.cpp)
    endif()
endif()

# ==================================================================================================
# Generate all .filamat: default material, skyboxes, and post-process
# ==================================================================================================

if (CMAKE_CROSSCOMPILING)
    include(${IMPORT_EXECUTABLES})
endif()

set(MATERIAL_BINS)
file(MAKE_DIRECTORY ${MATERIAL_DIR})

# Target system.
if (ANDROID OR WEBGL OR IOS)
    set(MATC_TARGET mobile)
else()
    set(MATC_TARGET desktop)
endif()

# Include SPIRV when building default materials on platforms that support Vulkan.
if (FILAMENT_SUPPORTS_VULKAN)
    set(MATC_BASE_FLAGS -a all)
else()
    set(MATC_BASE_FLAGS -a opengl)
endif()

# Only optimize materials in Release mode (so error message lines match the source code)
if (NOT CMAKE_BUILD_TYPE MATCHES Release)
    set(MATC_OPT_FLAGS -g)
endif()

foreach (mat_src ${MATERIAL_SRCS})
    get_filename_component(localname "${mat_src}" NAME_WE)
    get_filename_component(fullname "${mat_src}" ABSOLUTE)
    set(output_path "${MATERIAL_DIR}/${localname}.filamat")

    add_custom_command(
            OUTPUT ${output_path}
            COMMAND matc ${MATC_BASE_FLAGS} ${MATC_OPT_FLAGS} -p ${MATC_TARGET} -m material
                    -o ${output_path} ${fullname}
            DEPENDS ${fullname} matc
            COMMENT "Compiling material ${mat_src} to ${output_path}"
    )
    list(APPEND MATERIAL_BINS ${output_path})
endforeach()

set(output_path "${MATERIAL_DIR}/postprocess.filamat")
add_custom_command(
        OUTPUT ${output_path}
        COMMAND matc ${MATC_BASE_FLAGS} ${MATC_OPT_FLAGS} -p ${MATC_TARGET} -m postprocess
                -o ${output_path}
        DEPENDS matc
        COMMENT "Compiling post-process material ${output_path}"
)
list(APPEND MATERIAL_BINS ${output_path})

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
        COMMAND cmgen --quiet --ibl-dfg-multiscatter --ibl-dfg=${output_path}
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

if (ANDROID)
    target_link_libraries(${TARGET} PUBLIC GLESv3 EGL android)
endif()

if (APPLE AND NOT IOS)
    target_link_libraries(${TARGET} PRIVATE "-framework Cocoa")
endif()

target_link_libraries(${TARGET} PUBLIC math)
target_link_libraries(${TARGET} PUBLIC utils)
target_link_libraries(${TARGET} PUBLIC filaflat)
target_link_libraries(${TARGET} PUBLIC filabridge)
target_link_libraries(${TARGET} PUBLIC image_headers)

# Android, iOS, and WebGL do not use bluegl.
if(NOT IOS AND NOT ANDROID AND NOT WEBGL)
    target_link_libraries(${TARGET} PRIVATE bluegl)
endif()

if (FILAMENT_SUPPORTS_VULKAN)
    target_link_libraries(${TARGET} PUBLIC bluevk vkmemalloc)
endif()

if (LINUX)
    target_link_libraries(${TARGET} PRIVATE dl)
endif()

# ==================================================================================================
# Compiler flags
# ==================================================================================================
if (MSVC OR CLANG_CL)
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

set(FILAMENT_WARNINGS
        -Wextra -Wno-unused-parameter
        -Wextra-semi -Wnewline-eof -Wdeprecated -Wundef
        -Wgnu-conditional-omitted-operand
        -Wweak-vtables -Wnon-virtual-dtor -Wclass-varargs -Wimplicit-fallthrough
        -Wover-aligned
)

# clang-cl maps -Wall to -Weverything, which clutters logs with benign -Wc++98-compat warnings
# Use /W4 instead for clang-cl builds
if (CLANG_CL)
    set(FILAMENT_WARNINGS /W4 ${FILAMENT_WARNINGS})
else()
    set(FILAMENT_WARNINGS -Wall ${FILAMENT_WARNINGS})
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
add_subdirectory(test)
add_subdirectory(benchmark)
