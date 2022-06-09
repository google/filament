cmake_minimum_required(VERSION 3.19)
project(filament C ASM)

set(TARGET backend)
set(PUBLIC_HDR_DIR include)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR})

# ==================================================================================================
# Sources and headers
# ==================================================================================================
set(PUBLIC_HDRS
        include/backend/BufferDescriptor.h
        include/backend/CallbackHandler.h
        include/backend/DriverApiForward.h
        include/backend/DriverEnums.h
        include/backend/Handle.h
        include/backend/PipelineState.h
        include/backend/PixelBufferDescriptor.h
        include/backend/Platform.h
        include/backend/Program.h
        include/backend/SamplerDescriptor.h
        include/backend/TargetBufferInfo.h
)

set(SRCS
        src/BackendUtils.cpp
        src/Callable.cpp
        src/CallbackHandler.cpp
        src/CircularBuffer.cpp
        src/CommandBufferQueue.cpp
        src/CommandStream.cpp
        src/Driver.cpp
        src/Handle.cpp
        src/HandleAllocator.cpp
        src/ostream.cpp
        src/noop/NoopDriver.cpp
        src/noop/PlatformNoop.cpp
        src/Platform.cpp
        src/Program.cpp
        src/SamplerGroup.cpp
)

set(PRIVATE_HDRS
        include/private/backend/AcquiredImage.h
        include/private/backend/CircularBuffer.h
        include/private/backend/CommandBufferQueue.h
        include/private/backend/CommandStream.h
        include/private/backend/Dispatcher.h
        include/private/backend/Driver.h
        include/private/backend/DriverApi.h
        include/private/backend/DriverAPI.inc
        include/private/backend/HandleAllocator.h
        include/private/backend/SamplerGroup.h
        src/CommandStreamDispatcher.h
        src/DataReshaper.h
        src/DriverBase.h
)

# ==================================================================================================
# OpenGL / OpenGL ES Sources
# ==================================================================================================

if (FILAMENT_SUPPORTS_OPENGL AND NOT FILAMENT_USE_EXTERNAL_GLES3 AND NOT FILAMENT_USE_SWIFTSHADER)
    list(APPEND SRCS
            src/opengl/gl_headers.cpp
            src/opengl/gl_headers.h
            src/opengl/GLUtils.cpp
            src/opengl/GLUtils.h
            src/opengl/OpenGLContext.cpp
            src/opengl/OpenGLContext.h
            src/opengl/OpenGLDriver.cpp
            src/opengl/OpenGLDriver.h
            src/opengl/OpenGLDriverFactory.h
            src/opengl/OpenGLProgram.cpp
            src/opengl/OpenGLProgram.h
            src/opengl/OpenGLPlatform.cpp
            src/opengl/OpenGLTimerQuery.cpp
            src/opengl/OpenGLTimerQuery.h
            include/private/backend/OpenGLPlatform.h
    )
    if (EGL)
        list(APPEND SRCS src/opengl/platforms/PlatformEGL.cpp)
    endif()
    if (ANDROID)
        # FIXME: this should be included when we build for JAVA (which is implied by ANDROID)
        #        but is not specific to ANDROID
        list(APPEND SRCS src/VirtualMachineEnv.cpp)
    endif ()
    if (ANDROID)
        list(APPEND SRCS src/opengl/platforms/ExternalStreamManagerAndroid.cpp)
        list(APPEND SRCS src/opengl/platforms/PlatformEGLAndroid.cpp)
    elseif (IOS)
        list(APPEND SRCS src/opengl/platforms/PlatformCocoaTouchGL.mm)
        list(APPEND SRCS src/opengl/platforms/CocoaTouchExternalImage.mm)
    elseif (APPLE)
        list(APPEND SRCS src/opengl/platforms/PlatformCocoaGL.mm)
    elseif (WEBGL)
        list(APPEND SRCS src/opengl/platforms/PlatformWebGL.cpp)
    elseif (LINUX)
        if (FILAMENT_SUPPORTS_X11)
            list(APPEND SRCS src/opengl/platforms/PlatformGLX.cpp)
        elseif (FILAMENT_SUPPORTS_EGL_ON_LINUX)
            list(APPEND SRCS src/opengl/platforms/PlatformEGLHeadless.cpp)
        endif()
    elseif (WIN32)
        list(APPEND SRCS src/opengl/platforms/PlatformWGL.cpp)
    endif()
endif()

# ==================================================================================================
# Metal Sources
# ==================================================================================================

if (FILAMENT_SUPPORTS_METAL)
    set(METAL_SRCS
            src/metal/MetalBlitter.mm
            src/metal/MetalBuffer.mm
            src/metal/MetalBufferPool.mm
            src/metal/MetalContext.mm
            src/metal/MetalDriver.mm
            src/metal/MetalEnums.mm
            src/metal/MetalExternalImage.mm
            src/metal/MetalHandles.mm
            src/metal/MetalPlatform.mm
            src/metal/MetalResourceTracker.cpp
            src/metal/MetalState.mm
            src/metal/MetalTimerQuery.mm
            src/metal/MetalUtils.mm
    )

    list(APPEND SRCS ${METAL_SRCS})

    if (IOS)
        # Objective-C++ sources need an additional compiler flag on iOS to disable exceptions.
        set_property(SOURCE
            ${METAL_SRCS}
                src/opengl/platforms/PlatformCocoaTouchGL.mm
                src/opengl/platforms/CocoaTouchExternalImage.mm
                src/opengl/platforms/PlatformCocoaGL.mm
            PROPERTY COMPILE_FLAGS -fno-objc-exceptions)
    endif()

    option(FILAMENT_METAL_PROFILING "Enable profiling for the Metal backend" OFF)
endif()

# ==================================================================================================
# Vulkan Sources
# ==================================================================================================

# See root CMakeLists.txt for platforms that support Vulkan
if (FILAMENT_SUPPORTS_VULKAN)
    list(APPEND SRCS
            src/vulkan/VulkanBlitter.cpp
            src/vulkan/VulkanBlitter.h
            src/vulkan/VulkanBuffer.cpp
            src/vulkan/VulkanBuffer.h
            src/vulkan/VulkanCommands.cpp
            src/vulkan/VulkanCommands.h
            src/vulkan/VulkanConstants.h
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
            src/vulkan/VulkanMemory.h
            src/vulkan/VulkanMemory.cpp
            src/vulkan/VulkanPipelineCache.cpp
            src/vulkan/VulkanPipelineCache.h
            src/vulkan/VulkanPlatform.cpp
            src/vulkan/VulkanPlatform.h
            src/vulkan/VulkanSamplerCache.cpp
            src/vulkan/VulkanSamplerCache.h
            src/vulkan/VulkanStagePool.cpp
            src/vulkan/VulkanStagePool.h
            src/vulkan/VulkanSwapChain.cpp
            src/vulkan/VulkanSwapChain.h
            src/vulkan/VulkanTexture.cpp
            src/vulkan/VulkanTexture.h
            src/vulkan/VulkanUtility.cpp
            src/vulkan/VulkanUtility.h
    )
    if (LINUX)
        if (FILAMENT_SUPPORTS_WAYLAND)
            list(APPEND SRCS src/vulkan/PlatformVkLinuxWayland.cpp)
        elseif (FILAMENT_SUPPORTS_X11)
            list(APPEND SRCS src/vulkan/PlatformVkLinuxX11.cpp)
        endif()
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
include_directories(${GENERATION_ROOT})

# we're building a library
add_library(${TARGET} STATIC ${PRIVATE_HDRS} ${PUBLIC_HDRS} ${SRCS})

if (CMAKE_COMPILER_IS_GNUCXX)
    set_target_properties(${TARGET} PROPERTIES
                          C_VISIBILITY_PRESET default
                          CXX_VISIBILITY_PRESET default)
endif()

# specify where the public headers of this library are
target_include_directories(${TARGET} PUBLIC ${PUBLIC_HDR_DIR})

# add this subproject to the Filament folder
set_target_properties(${TARGET} PROPERTIES FOLDER Filament)

# ==================================================================================================
# Expose a header-only target to minimize dependencies.
# ==================================================================================================

add_library(${TARGET}_headers INTERFACE)
target_include_directories(${TARGET}_headers INTERFACE ${PUBLIC_HDR_DIR})

# ==================================================================================================
# Build SPIRV snippets used by the Vulkan backend.
# ==================================================================================================

set(VKSHADERS_DIR  "${CMAKE_CURRENT_BINARY_DIR}/generated/vkshaders")

file(MAKE_DIRECTORY ${VKSHADERS_DIR})

get_resgen_vars(${VKSHADERS_DIR} vkshaders)

set(VKSHADER_BINS)
function(build_vkshader SOURCE TARGET_PATH)
    set(SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/vulkan/${SOURCE}")
    set(VKSHADER_BINS ${VKSHADER_BINS} ${TARGET_PATH} PARENT_SCOPE)
    add_custom_command(
            OUTPUT ${TARGET_PATH}
            COMMAND matc --raw -o ${TARGET_PATH} ${SOURCE_PATH}
            MAIN_DEPENDENCY ${SOURCE_PATH}
            DEPENDS matc ${SOURCE_PATH}
            COMMENT "Building SPIR-V")
endfunction()
build_vkshader("BlitDepth.vs" "BlitDepthVs.spv")
build_vkshader("BlitDepth.fs" "BlitDepthFs.spv")

add_custom_command(
        OUTPUT ${RESGEN_OUTPUTS}
        COMMAND resgen ${RESGEN_FLAGS} ${VKSHADER_BINS}
        DEPENDS resgen ${VKSHADER_BINS}
        COMMENT "Aggregating compiled VK shaders")

if (DEFINED RESGEN_SOURCE_FLAGS)
    set_source_files_properties(${RESGEN_SOURCE} PROPERTIES COMPILE_FLAGS ${RESGEN_SOURCE_FLAGS})
endif()

set(DUMMY_SRC "${VKSHADERS_DIR}/dummy.c")
add_custom_command(OUTPUT ${DUMMY_SRC} COMMAND echo "//" > ${DUMMY_SRC})

add_library(vkshaders STATIC ${DUMMY_SRC} ${RESGEN_SOURCE})
set_target_properties(vkshaders PROPERTIES FOLDER Filament/Generated)

# ==================================================================================================
# Dependencies
# ==================================================================================================

if (ANDROID)
    if (FILAMENT_SUPPORTS_OPENGL)
        target_link_libraries(${TARGET} PUBLIC GLESv3 EGL)
    endif()
    target_link_libraries(${TARGET} PUBLIC android)
endif()

if (FILAMENT_USE_SWIFTSHADER)
    target_link_libraries(${TARGET} PUBLIC ${SWIFTSHADER_VK})
endif()

if (APPLE AND NOT IOS)
    target_link_libraries(${TARGET} PRIVATE "-framework Cocoa")
endif()

target_link_libraries(${TARGET} PUBLIC math)
target_link_libraries(${TARGET} PUBLIC utils)

# Android, iOS, and WebGL do not use bluegl.
if(FILAMENT_SUPPORTS_OPENGL AND NOT IOS AND NOT ANDROID AND NOT WEBGL)
    target_link_libraries(${TARGET} PRIVATE bluegl)
endif()

if (FILAMENT_SUPPORTS_VULKAN)
    target_link_libraries(${TARGET} PUBLIC bluevk vkmemalloc vkshaders smol-v)
endif()

if (FILAMENT_SUPPORTS_METAL)
    target_link_libraries(${TARGET} PUBLIC "-framework Metal -framework CoreVideo")
endif()

if (LINUX)
    target_link_libraries(${TARGET} PRIVATE dl)
    if(FILAMENT_SUPPORTS_EGL_ON_LINUX)
       target_link_libraries(${TARGET} PUBLIC EGL)
    endif()
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
    set(OPTIMIZATION_FLAGS
        -ffast-math
        -ffp-contract=fast
        -fvisibility-inlines-hidden
    )
endif()

set(LINUX_LINKER_OPTIMIZATION_FLAGS
        -Wl,--exclude-libs,bluegl
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

if (APPLE)
    # Turn on Automatic Reference Counting.
    target_compile_options(${TARGET} PRIVATE "-fobjc-arc")
endif()

target_compile_options(${TARGET} PRIVATE
        ${FILAMENT_WARNINGS}
        $<$<CONFIG:Release>:${OPTIMIZATION_FLAGS}>
        $<$<AND:$<PLATFORM_ID:Darwin>,$<CONFIG:Release>>:${DARWIN_OPTIMIZATION_FLAGS}>
)

if (FILAMENT_SUPPORTS_METAL)
    target_compile_definitions(${TARGET} PRIVATE $<$<BOOL:${FILAMENT_METAL_PROFILING}>:FILAMENT_METAL_PROFILING>)
endif()

target_link_libraries(${TARGET} PRIVATE
        $<$<AND:$<PLATFORM_ID:Linux>,$<CONFIG:Release>>:${LINUX_LINKER_OPTIMIZATION_FLAGS}>
)

# ==================================================================================================
# Installation
# ==================================================================================================
set(INSTALL_TYPE ARCHIVE)
install(TARGETS ${TARGET} ${INSTALL_TYPE} DESTINATION lib/${DIST_DIR})
install(TARGETS vkshaders ${INSTALL_TYPE} DESTINATION lib/${DIST_DIR})
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
        test/test_FeedbackLoops.cpp
        test/test_Blit.cpp
        test/test_MissingRequiredAttributes.cpp
        test/test_ReadPixels.cpp
        test/test_BufferUpdates.cpp
        test/test_MRT.cpp
        test/test_LoadImage.cpp
        test/test_RenderExternalImage.cpp
        test/test_StencilBuffer.cpp
        test/test_Scissor.cpp
        )

    target_link_libraries(backend_test PRIVATE
        backend
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
            getopt
            gtest
            glslang
            spirv-cross-core
            spirv-cross-glsl
            spirv-cross-msl
            )

    if (NOT IOS)
        target_link_libraries(backend_test PRIVATE image imageio)
        list(APPEND BACKEND_TEST_DEPS image imageio)
    endif()

    set(BACKEND_TEST_COMBINED_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/libbackendtest_combined.a")
    combine_static_libs(backend_test "${BACKEND_TEST_COMBINED_OUTPUT}" "${BACKEND_TEST_DEPS}")

    set(BACKEND_TEST_LIB_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}backend_test${CMAKE_STATIC_LIBRARY_SUFFIX})

    if (INSTALL_BACKEND_TEST)
        install(FILES "${BACKEND_TEST_COMBINED_OUTPUT}" DESTINATION lib/${DIST_DIR} RENAME ${BACKEND_TEST_LIB_NAME})
        install(FILES test/PlatformRunner.h DESTINATION include/backend_test)
    endif()

    set_target_properties(backend_test PROPERTIES FOLDER Tests)
endif()

if (APPLE AND NOT IOS)
    add_executable(backend_test_mac test/mac_runner.mm)
    target_link_libraries(backend_test_mac PRIVATE "-framework Metal -framework AppKit -framework QuartzCore")
    # Because each test case is a separate file, the -force_load flag is necessary to prevent the
    # linker from removing "unused" symbols.
    target_link_libraries(backend_test_mac PRIVATE -force_load backend_test)
    set_target_properties(backend_test_mac PROPERTIES FOLDER Tests)
endif()

# ==================================================================================================
# Compute tests

if (NOT IOS AND NOT WEBGL)

add_executable(compute_test
        test/ComputeTest.cpp
        test/Arguments.cpp
        test/test_ComputeBasic.cpp
        )

target_link_libraries(compute_test PRIVATE
        backend
        getopt
        gtest
        )

set_target_properties(compute_test PROPERTIES FOLDER Tests)

endif()
