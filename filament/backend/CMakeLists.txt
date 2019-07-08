cmake_minimum_required(VERSION 3.10)
project(filament C ASM)

set(TARGET backend)
set(PUBLIC_HDR_DIR include)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR})

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/backend/BufferDescriptor.h
        include/backend/Handle.h
        include/backend/PipelineState.h
        include/backend/PixelBufferDescriptor.h
        include/backend/Platform.h
        include/backend/TargetBufferInfo.h
)

set(SRCS
        src/CircularBuffer.cpp
        src/CommandBufferQueue.cpp
        src/CommandStream.cpp
        src/Driver.cpp
        src/Handle.cpp
        src/noop/NoopDriver.cpp
        src/noop/PlatformNoop.cpp
        src/Platform.cpp
        src/Program.cpp
        src/SamplerGroup.cpp
        src/TextureReshaper.cpp
)

set(PRIVATE_HDRS
        include/private/backend/CircularBuffer.h
        include/private/backend/CommandBufferQueue.h
        include/private/backend/CommandStream.h
        include/private/backend/Driver.h
        include/private/backend/DriverApi.h
        include/private/backend/DriverAPI.inc
        include/private/backend/DriverApiForward.h
        include/private/backend/Program.h
        include/private/backend/SamplerGroup.h
        src/CommandStreamDispatcher.h
        src/DataReshaper.h
        src/DriverBase.h
        src/TextureReshaper.h
)

# ==================================================================================================
# OpenGL / OpenGL ES Sources
# ==================================================================================================

if (NOT USE_EXTERNAL_GLES3)
    list(APPEND SRCS
            src/opengl/gl_headers.cpp
            src/opengl/gl_headers.h
            src/opengl/GLUtils.cpp
            src/opengl/GLUtils.h
            src/opengl/OpenGLBlitter.cpp
            src/opengl/OpenGLBlitter.h
            src/opengl/OpenGLDriver.cpp
            src/opengl/OpenGLDriver.h
            src/opengl/OpenGLDriverFactory.h
            src/opengl/OpenGLProgram.cpp
            src/opengl/OpenGLProgram.h
            src/opengl/OpenGLPlatform.cpp
            include/private/backend/OpenGLPlatform.h
    )
    if (ANDROID)
        list(APPEND SRCS src/android/ExternalStreamManagerAndroid.cpp)
        list(APPEND SRCS src/android/ExternalTextureManagerAndroid.cpp)
        list(APPEND SRCS src/android/VirtualMachineEnv.cpp)
        list(APPEND SRCS src/opengl/PlatformEGL.cpp)
    elseif (IOS)
        list(APPEND SRCS src/opengl/PlatformCocoaTouchGL.mm)
        list(APPEND SRCS src/opengl/CocoaTouchExternalImage.mm)
    elseif (APPLE)
        list(APPEND SRCS src/opengl/PlatformCocoaGL.mm)
    elseif (WEBGL)
        list(APPEND SRCS src/opengl/PlatformWebGL.cpp)
    elseif (LINUX)
        list(APPEND SRCS src/opengl/PlatformGLX.cpp)
    elseif (WIN32)
        list(APPEND SRCS src/opengl/PlatformWGL.cpp)
    else()
        list(APPEND SRCS src/opengl/PlatformDummyGL.cpp)
    endif()
endif()

# ==================================================================================================
# Metal Sources
# ==================================================================================================

if (FILAMENT_SUPPORTS_METAL)
    list(APPEND SRCS
            src/metal/MetalBlitter.mm
            src/metal/MetalBufferPool.mm
            src/metal/MetalContext.mm
            src/metal/MetalDriver.mm
            src/metal/MetalExternalImage.mm
            src/metal/MetalHandles.mm
            src/metal/MetalResourceTracker.cpp
            src/metal/MetalState.mm
            src/metal/PlatformMetal.mm
    )
endif()

# ==================================================================================================
# Vulkan Sources
# ==================================================================================================

# See root CMakeLists.txt for platforms that support Vulkan
if (FILAMENT_SUPPORTS_VULKAN)
    list(APPEND SRCS
            src/vulkan/VulkanBinder.cpp
            src/vulkan/VulkanBinder.h
            src/vulkan/VulkanBuffer.cpp
            src/vulkan/VulkanBuffer.h
            src/vulkan/VulkanContext.cpp
            src/vulkan/VulkanContext.h
            src/vulkan/VulkanDisposer.cpp
            src/vulkan/VulkanDisposer.h
            src/vulkan/VulkanDriver.cpp
            src/vulkan/VulkanDriver.h
            src/vulkan/VulkanDriverFactory.h
            src/vulkan/VulkanFboCache.cpp
            src/vulkan/VulkanFboCache.h
            src/vulkan/VulkanHandles.cpp
            src/vulkan/VulkanHandles.h
            src/vulkan/VulkanPlatform.cpp
            src/vulkan/VulkanPlatform.h
            src/vulkan/VulkanSamplerCache.cpp
            src/vulkan/VulkanSamplerCache.h
            src/vulkan/VulkanStagePool.cpp
            src/vulkan/VulkanStagePool.h
            src/vulkan/VulkanUtility.cpp
            src/vulkan/VulkanUtility.h
    )
    if (LINUX)
        list(APPEND SRCS src/vulkan/PlatformVkLinux.cpp)
    elseif (APPLE AND NOT IOS)
        list(APPEND SRCS src/vulkan/PlatformVkCocoa.mm)
    elseif (IOS)
        list(APPEND SRCS src/vulkan/PlatformVkCocoaTouch.mm)
    elseif (ANDROID)
        list(APPEND SRCS src/vulkan/PlatformVkAndroid.cpp)
    elseif (WIN32)
        list(APPEND SRCS src/vulkan/PlatformVkWindows.cpp)
    endif()
endif()

# ==================================================================================================
# Definitions
# ==================================================================================================
# "2" corresponds to SYSTRACE_TAG_FILEMENT (See: utils/Systrace.h)
add_definitions(-DSYSTRACE_TAG=2 )


# ==================================================================================================
# Includes & target definition
# ==================================================================================================
# specify where our headers are
include_directories(${PUBLIC_HDR_DIR})
include_directories(src)

# we're building a library
add_library(${TARGET} STATIC ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})

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

# Android, iOS, and WebGL do not use bluegl.
if(NOT IOS AND NOT ANDROID AND NOT WEBGL)
    target_link_libraries(${TARGET} PRIVATE bluegl)
endif()

if (FILAMENT_SUPPORTS_VULKAN)
    target_link_libraries(${TARGET} PUBLIC bluevk vkmemalloc)
endif()

if (FILAMENT_SUPPORTS_METAL)
    target_link_libraries(${TARGET} PUBLIC "-framework Metal")
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
install(DIRECTORY ${PUBLIC_HDR_DIR}/backend DESTINATION include)
