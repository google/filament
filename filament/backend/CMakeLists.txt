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
        include/backend/DriverEnums.h
        include/backend/Handle.h
        include/backend/PipelineState.h
        include/backend/PixelBufferDescriptor.h
        include/backend/Platform.h
        include/backend/TargetBufferInfo.h
)

set(SRCS
        src/BackendUtils.cpp
        src/Callable.cpp
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
        include/private/backend/AcquiredImage.h
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

if (NOT FILAMENT_USE_EXTERNAL_GLES3)
    list(APPEND SRCS
            src/opengl/gl_headers.cpp
            src/opengl/gl_headers.h
            src/opengl/GLUtils.cpp
            src/opengl/GLUtils.h
            src/opengl/OpenGLBlitter.cpp
            src/opengl/OpenGLBlitter.h
            src/opengl/OpenGLContext.cpp
            src/opengl/OpenGLContext.h
            src/opengl/OpenGLDriver.cpp
            src/opengl/OpenGLDriver.h
            src/opengl/OpenGLDriverFactory.h
            src/opengl/OpenGLProgram.cpp
            src/opengl/OpenGLProgram.h
            src/opengl/OpenGLPlatform.cpp
            src/opengl/TimerQuery.cpp
            src/opengl/TimerQuery.h
            include/private/backend/OpenGLPlatform.h
    )
    if (EGL)
        list(APPEND SRCS src/opengl/PlatformEGL.cpp)
    endif()
    if (ANDROID)
        list(APPEND SRCS src/android/ExternalStreamManagerAndroid.cpp)
        list(APPEND SRCS src/android/ExternalTextureManagerAndroid.cpp)
        list(APPEND SRCS src/android/VirtualMachineEnv.cpp)
        list(APPEND SRCS src/opengl/PlatformEGLAndroid.cpp)
    elseif (FILAMENT_USE_SWIFTSHADER)
        # no extra sources for SwiftShader, PlatformEGL is enough
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
            src/metal/MetalBuffer.mm
            src/metal/MetalBufferPool.mm
            src/metal/MetalContext.mm
            src/metal/MetalDriver.mm
            src/metal/MetalExternalImage.mm
            src/metal/MetalHandles.mm
            src/metal/MetalResourceTracker.cpp
            src/metal/MetalState.mm
            src/metal/MetalTimerQuery.mm
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
# Expose a header-only target to minimize dependencies.
# ==================================================================================================

add_library(${TARGET}_headers INTERFACE)
target_include_directories(${TARGET}_headers INTERFACE ${PUBLIC_HDR_DIR})

# ==================================================================================================
# Dependencies
# ==================================================================================================

if (ANDROID)
    target_link_libraries(${TARGET} PUBLIC GLESv3 EGL android)
endif()

if (FILAMENT_USE_SWIFTSHADER)
    target_link_libraries(${TARGET} PRIVATE swiftshader PUBLIC ${SWIFTSHADER_EGL} ${SWIFTSHADER_GLES})
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
    target_link_libraries(${TARGET} PUBLIC "-framework Metal -framework CoreVideo")
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

if (APPLE)
    # Turn on Automatic Reference Counting.
    target_compile_options(${TARGET} PRIVATE "-fobjc-arc")
endif()

target_compile_options(${TARGET} PRIVATE
        ${FILAMENT_WARNINGS}
        $<$<CONFIG:Release>:${OPTIMIZATION_FLAGS}>
        $<$<AND:$<PLATFORM_ID:Darwin>,$<CONFIG:Release>>:${DARWIN_OPTIMIZATION_FLAGS}>
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

# ==================================================================================================
# Test
# ==================================================================================================
option(INSTALL_BACKEND_TEST "Install the backend test library so it can be consumed on iOS" OFF)

if (APPLE)
    add_library(backend_test STATIC
        test/BackendTest.cpp
        test/ShaderGenerator.cpp
        test/TrianglePrimitive.cpp
        test/Arguments.cpp
        test/test_MissingRequiredAttributes.cpp
        test/test_ReadPixels.cpp
        test/test_BufferUpdates.cpp
        test/test_MRT.cpp)

    target_link_libraries(backend_test PRIVATE
        backend
        filabridge
        getopt
        gtest
        SPIRV
        spirv-cross-glsl)

    set(BACKEND_TEST_DEPS
            OGLCompiler
            OSDependent
            SPIRV
            SPIRV-Tools
            SPIRV-Tools-opt
            backend_test
            filabridge
            getopt
            gtest
            glslang
            spirv-cross-core
            spirv-cross-glsl
            spirv-cross-msl
            )

    set(BACKEND_TEST_COMBINED_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libbackendtest_combined.a")
    combine_static_libs(backend_test "${BACKEND_TEST_COMBINED_OUTPUT}" "${BACKEND_TEST_DEPS}")

    set(BACKEND_TEST_LIB_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}backend_test${CMAKE_STATIC_LIBRARY_SUFFIX})

    if (INSTALL_BACKEND_TEST)
        install(FILES "${BACKEND_TEST_COMBINED_OUTPUT}" DESTINATION lib/${DIST_DIR} RENAME ${BACKEND_TEST_LIB_NAME})
        install(FILES test/PlatformRunner.h DESTINATION include/backend_test)
    endif()
endif()

if (APPLE AND NOT IOS AND NOT FILAMENT_USE_SWIFTSHADER)
    add_executable(backend_test_mac test/mac_runner.mm)
    target_link_libraries(backend_test_mac PRIVATE "-framework Metal -framework AppKit -framework QuartzCore")
    # Because each test case is a separate file, the -force_load flag is necessary to prevent the
    # linker from removing "unused" symbols.
    target_link_libraries(backend_test_mac PRIVATE -force_load backend_test)
endif()

if (APPLE AND NOT Vulkan_LIBRARY)
    message(STATUS "No Vulkan SDK was found, using prebuilt MoltenVK.")
    set(MOLTENVK_DIR "../../third_party/moltenvk")
    configure_file(
            ${MOLTENVK_DIR}/libvulkan.1.dylib
            ${PROJECT_BINARY_DIR}/libvulkan.1.dylib COPYONLY)
    configure_file(
            ${MOLTENVK_DIR}/MoltenVK_icd.json
            ${PROJECT_BINARY_DIR}/MoltenVK_icd.json COPYONLY)
    configure_file(
            ${MOLTENVK_DIR}/libMoltenVK.dylib
            ${PROJECT_BINARY_DIR}/libMoltenVK.dylib COPYONLY)
endif()
