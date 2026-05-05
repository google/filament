# Copyright 2026 The Dawn Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

################################################################################
# dawn_test_infra - Code shared by all test kinds
################################################################################

add_library(dawn_test_infra OBJECT
    "AdapterTestConfig.cpp"
    "AdapterTestConfig.h"
    "DawnTest.cpp"
    "DawnTest.h"
    "PartitionAllocSupport.cpp"
    "PartitionAllocSupport.h"
    "ToggleParser.cpp"
    "ToggleParser.h"
    "mocks/platform/CachingInterfaceMock.cpp"
    "mocks/platform/CachingInterfaceMock.h"
)
common_compile_options(dawn_test_infra)
target_link_libraries(dawn_test_infra PRIVATE
    dawn_native
    dawn_wgpu_utils
    dawncpp
    gmock
)

################################################################################
# dawn_end2end_test_objects - Tests that only use the public Dawn API
################################################################################

set(end2end_conditional_private_platform_depends)
set(end2end_sources
    "end2end/AdapterCreationTests.cpp"
    "end2end/AdapterEnumerationTests.cpp"
    "end2end/AdapterFormatCapabilitiesVkTests.cpp"
    "end2end/AdapterPropertiesD3DTests.cpp"
    "end2end/AdapterPropertiesVkTests.cpp"
    "end2end/AdapterPropertiesDrmTests.cpp"
    "end2end/ArchTierLimitsExhaustive.cpp"
    "end2end/BasicTests.cpp"
    "end2end/BindGroupTests.cpp"
    "end2end/BindingArrayTests.cpp"
    "end2end/BufferHostMappedPointerTests.cpp"
    "end2end/BufferHostMappedPointerTests.h"
    "end2end/BufferRobustnessTests.cpp"
    "end2end/BufferTests.cpp"
    "end2end/BufferZeroInitTests.cpp"
    "end2end/ClipDistancesTests.cpp"
    "end2end/ClipSpaceTests.cpp"
    "end2end/ColorStateTests.cpp"
    "end2end/CommandEncoderTests.cpp"
    "end2end/CompressedTextureFormatTests.cpp"
    "end2end/ComputeCopyStorageBufferTests.cpp"
    "end2end/ComputeDispatchTests.cpp"
    "end2end/ComputeFlowControlTests.cpp"
    "end2end/ComputeLayoutMemoryBufferTests.cpp"
    "end2end/ComputeSharedMemoryTests.cpp"
    "end2end/ComputeStorageBufferBarrierTests.cpp"
    "end2end/CopyExternalTextureForBrowserTests.cpp"
    "end2end/CopyTests.cpp"
    "end2end/CopyTextureForBrowserTests.cpp"
    "end2end/CreateDestroyTests.cpp"
    "end2end/CreatePipelineAsyncTests.cpp"
    "end2end/CullingTests.cpp"
    "end2end/DawnTestInfraTests.cpp"
    "end2end/DebugMarkerTests.cpp"
    "end2end/DepthBiasTests.cpp"
    "end2end/DepthStencilCopyTests.cpp"
    "end2end/DepthStencilLoadOpTests.cpp"
    "end2end/DepthStencilSamplingTests.cpp"
    "end2end/DepthStencilStateTests.cpp"
    "end2end/DestroyTests.cpp"
    "end2end/DeviceInitializationTests.cpp"
    "end2end/DeviceLifetimeTests.cpp"
    "end2end/DeviceLostTests.cpp"
    "end2end/DiscardBasicTests.cpp"
    "end2end/DrawIndexedIndirectTests.cpp"
    "end2end/DrawIndexedTests.cpp"
    "end2end/DrawIndirectTests.cpp"
    "end2end/DrawTests.cpp"
    "end2end/DualSourceBlendTests.cpp"
    "end2end/DynamicBufferOffsetTests.cpp"
    "end2end/EntryPointTests.cpp"
    "end2end/EventTests.cpp"
    "end2end/ExternalTextureTests.cpp"
    "end2end/FirstIndexOffsetTests.cpp"
    "end2end/FragDepthTests.cpp"
    "end2end/FramebufferFetchTests.cpp"
    "end2end/GpuMemorySynchronizationTests.cpp"
    "end2end/HistogramTests.cpp"
    "end2end/ImmediateDataBufferLengthTests.cpp"
    "end2end/ImmediateDataTests.cpp"
    "end2end/IndexFormatTests.cpp"
    "end2end/InfiniteLoopTests.cpp"
    "end2end/MaxLimitTests.cpp"
    "end2end/MemoryAllocationStressTests.cpp"
    "end2end/MemoryHeapPropertiesTests.cpp"
    "end2end/MultiDrawIndexedIndirectTests.cpp"
    "end2end/MultiDrawIndirectTests.cpp"
    "end2end/MultisampledInterpolationTests.cpp"
    "end2end/MultisampledRenderingTests.cpp"
    "end2end/MultisampledSamplingTests.cpp"
    "end2end/MultithreadTests.cpp"
    "end2end/NonzeroBufferCreationTests.cpp"
    "end2end/NonzeroTextureCreationTests.cpp"
    "end2end/OpArrayLengthTests.cpp"
    "end2end/PackUnpack4x8NormTests.cpp"
    "end2end/Packed4x8IntegerDotProductTests.cpp"
    "end2end/PipelineCachingTests.cpp"
    "end2end/PipelineLayoutTests.cpp"
    "end2end/PixelLocalStorageTests.cpp"
    "end2end/PolyfillBuiltinSimpleTests.cpp"
    "end2end/PrimitiveIndexTests.cpp"
    "end2end/PrimitiveStateTests.cpp"
    "end2end/PrimitiveTopologyTests.cpp"
    "end2end/QueryTests.cpp"
    "end2end/QueueTests.cpp"
    "end2end/QueueTimelineTests.cpp"
    "end2end/ReadOnlyDepthStencilAttachmentTests.cpp"
    "end2end/RenderAttachmentTests.cpp"
    "end2end/RenderBundleTests.cpp"
    "end2end/RenderPassLoadOpTests.cpp"
    "end2end/RenderPassTests.cpp"
    "end2end/RequiredBufferSizeInCopyTests.cpp"
    "end2end/ResourceTableTests.cpp"
    "end2end/SamplerFilterAnisotropicTests.cpp"
    "end2end/SamplerTests.cpp"
    "end2end/ScissorTests.cpp"
    "end2end/SelectNonShortCircuiting.cpp"
    "end2end/ShaderAtomicTests.cpp"
    "end2end/ShaderBuiltinPartialConstArgsErrorTests.cpp"
    "end2end/ShaderF16Tests.cpp"
    "end2end/ShaderModuleCachingTests.cpp"
    "end2end/ShaderPrintTests.cpp"
    "end2end/ShaderTests.cpp"
    "end2end/ShaderValidationTests.cpp"
    "end2end/StorageTextureTests.cpp"
    "end2end/SubgroupMatrixTests.cpp"
    "end2end/SubgroupsTests.cpp"
    "end2end/SubresourceRenderAttachmentTests.cpp"
    "end2end/SupportedFeatureArch.cpp"
    "end2end/TexelBufferTests.cpp"
    "end2end/Texture3DTests.cpp"
    "end2end/TextureComponentSwizzleTests.cpp"
    "end2end/TextureCorruptionTests.cpp"
    "end2end/TextureFormatTests.cpp"
    "end2end/TextureFormatsTier1Tests.cpp"
    "end2end/TextureShaderBuiltinTests.cpp"
    "end2end/TextureSubresourceTests.cpp"
    "end2end/TextureViewTests.cpp"
    "end2end/TextureZeroInitTests.cpp"
    "end2end/VertexFormatTests.cpp"
    "end2end/VertexOnlyRenderPipelineTests.cpp"
    "end2end/VertexStateTests.cpp"
    "end2end/ViewportOrientationTests.cpp"
    "end2end/ViewportTests.cpp"
)

if (NOT IS_DEBUG_BUILD)
    list(APPEND end2end_sources
        "end2end/RuntimeTimeoutCompute.cpp"
    )
endif()

if (DAWN_ENABLE_D3D11 OR DAWN_ENABLE_D3D12)
    list(APPEND end2end_conditional_private_platform_depends
        d3d11.lib
        d3d12.lib
        dxgi.lib
    )
    list(APPEND end2end_sources
        "end2end/VideoViewsTests_win.cpp"
    )
endif()

if (DAWN_ENABLE_D3D12)
    list(APPEND end2end_sources
        "end2end/D3D12CachingTests.cpp"
    )
endif()

if (APPLE)
    if (DAWN_ENABLE_METAL)
        list(APPEND end2end_sources
            "end2end/VideoViewsTests_mac.cpp"
        )
    endif()
    list(APPEND end2end_sources
        "end2end/BufferHostMappedPointerTests_apple.mm"
    )

    list(APPEND conditional_private_platform_depends
        "-framework IOSurface"
    )
endif()

if (UNIX)
    list(APPEND end2end_sources
        "end2end/BufferHostMappedPointerTests_posix.cpp"
    )
endif()

if (WIN32)
    list(APPEND end2end_sources
        "end2end/BufferHostMappedPointerTests_win.cpp"
    )
endif()

if (DAWN_SUPPORTS_GLFW_FOR_WINDOWING)
    list(APPEND end2end_conditional_private_platform_depends
        glfw
        dawn::dawn_glfw
    )
    if (DAWN_USE_X11)
        find_package(X11 REQUIRED)
        list(APPEND end2end_conditional_private_platform_depends
            ${X11_LIBRARIES}
        )
    endif()

    list(APPEND end2end_sources
        "end2end/SurfaceConfigurationValidationTests.cpp"
        "end2end/SurfaceTests.cpp"
        "end2end/WindowSurfaceTests.cpp"
    )
endif()

if (DAWN_ENABLE_VULKAN)
    list(APPEND end2end_sources
        "end2end/AllocatorMemoryInstrumentationTests.cpp"
    )
endif()

if (DAWN_ENABLE_WEBGPU_ON_WEBGPU)
    list(APPEND end2end_sources
        "end2end/CaptureAndReplayTests.cpp"
    )
endif()

if (DAWN_ENABLE_D3D11 OR DAWN_ENABLE_D3D12 OR DAWN_ENABLE_METAL OR (DAWN_ENABLE_VULKAN AND NOT ANDROID))
    list(APPEND end2end_sources
        "end2end/VideoViewsTests.cpp"
        "end2end/VideoViewsTests.h"
    )
endif()

if (DAWN_ENABLE_VULKAN AND (UNIX AND NOT ANDROID))
    list(APPEND end2end_conditional_private_platform_depends
        "gbm"
    )
    list(APPEND end2end_sources
        "end2end/VideoViewsTests_gbm.cpp"
    )
endif()

if (DAWN_ENABLE_OPENGLES)
    list(APPEND end2end_sources
        "end2end/GLExplicitExtensionsTests.cpp"
    )
endif()

add_library(dawn_end2end_test_objects OBJECT ${end2end_sources})
common_compile_options(dawn_end2end_test_objects)
target_link_libraries(dawn_end2end_test_objects PRIVATE
    absl_strings
    dawn_common
    dawn_internal_config
    dawn_native
    dawn_platform
    dawn_proc
    dawn_shared_utils_chromium_test_compat
    dawn_system_utils
    dawn_test_infra
    dawn_test_utils
    dawn_wgpu_utils
    dawncpp
    gmock

    ${end2end_conditional_private_platform_depends}
)

################################################################################
# dawn_whitebox_test_objects - Tests that can use dawn::native internals
################################################################################

set(whitebox_conditional_private_platform_depends)
set(whitebox_sources
    "white_box/ImmediateConstantOffsetTests.cpp"
    "white_box/ShaderModuleTests.cpp"
    "white_box/SharedBufferMemoryTests.cpp"
    "white_box/SharedBufferMemoryTests.h"
    "white_box/SharedTextureMemoryTests.cpp"
    "white_box/SharedTextureMemoryTests.h"
    "white_box/WaitListEventTests.cpp"
)

if (APPLE)
    list(APPEND whitebox_sources
        "white_box/SharedTextureMemoryTests_apple.mm"
    )
endif()

if (DAWN_ENABLE_D3D11)
    list(APPEND whitebox_sources
        "white_box/D3D11BufferTests.cpp"
        "white_box/D3D11MultithreadTests.cpp"
        "white_box/D3D11ShaderCachingTests.cpp"
        "white_box/SharedTextureMemoryTests_win.cpp"
    )
endif()

if (DAWN_ENABLE_D3D12)
    list(APPEND whitebox_sources
        "white_box/D3D12DescriptorHeapTests.cpp"
        "white_box/D3D12ResidencyTests.cpp"
        "white_box/D3D12ResourceHeapTests.cpp"
        "white_box/GPUTimestampCalibrationTests.cpp"
        "white_box/GPUTimestampCalibrationTests.h"
        "white_box/GPUTimestampCalibrationTests_D3D12.cpp"
        "white_box/SharedBufferMemoryTests_win.cpp"
        "white_box/SharedTextureMemoryTests_win_d3d12.cpp"
    )
endif()

if (DAWN_ENABLE_METAL)
    list(APPEND whitebox_sources
      "white_box/GPUTimestampCalibrationTests.cpp"
      "white_box/GPUTimestampCalibrationTests.h"
      "white_box/GPUTimestampCalibrationTests_Metal.mm"
      "white_box/MetalAutoreleasePoolTests.mm"
      "white_box/MetalBackendTests.cpp"
    )
endif()

if (DAWN_ENABLE_OPENGLES)
    list(APPEND whitebox_conditional_private_platform_depends dawn_khronos_platform)
    list(APPEND whitebox_sources
        "white_box/EGLImageWrappingTests.cpp"
        "white_box/GLTextureWrappingTests.cpp"
    )
endif()

if (DAWN_ENABLE_VULKAN)
    list(APPEND whitebox_conditional_private_platform_depends
        Vulkan::Headers
    )
    list(APPEND whitebox_sources
        "white_box/BufferAllocatedSizeTests.cpp"
        "white_box/InternalResolveAttachmentSampleTypeTests.cpp"
        "white_box/InternalResourceUsageTests.cpp"
        "white_box/InternalStorageBufferBindingTests.cpp"
        "white_box/QueryInternalShaderTests.cpp"
    )

    if (UNIX AND NOT ANDROID)
        list(APPEND whitebox_sources
            "white_box/SharedTextureMemoryTests_dmabuf.cpp"
            "white_box/SharedTextureMemoryTests_opaquefd.cpp"
            "white_box/VulkanImageWrappingTests.cpp"
            "white_box/VulkanImageWrappingTests.h"
            "white_box/VulkanImageWrappingTests_DmaBuf.cpp"
            "white_box/VulkanImageWrappingTests_DmaBuf.h"
            "white_box/VulkanImageWrappingTests_OpaqueFD.cpp"
            "white_box/VulkanImageWrappingTests_OpaqueFD.h"
        )
    endif()

    if (ANDROID)
        list(APPEND whitebox_sources
            "white_box/SharedTextureMemoryTests_android.cpp"
            "white_box/SharedTextureMemoryYCbCrAndroidTests.cpp"
        )
    endif()

    if (DAWN_ENABLE_ERROR_INJECTION)
        list(APPEND whitebox_sources
            "white_box/VulkanErrorInjectorTests.cpp"
        )
    endif()
endif()

add_library(dawn_whitebox_test_objects OBJECT ${whitebox_sources})
common_compile_options(dawn_whitebox_test_objects)
target_link_libraries(dawn_whitebox_test_objects PRIVATE
    dawn_common
    dawn_internal_config
    dawn_native
    dawn_native_objects
    dawn_platform
    dawn_proc
    dawn_shared_utils_chromium_test_compat
    dawn_system_utils
    dawn_test_infra
    dawn_test_utils
    dawn_wgpu_utils
    dawncpp
    gmock

    ${whitebox_conditional_private_platform_depends}
)

################################################################################
# dawn_end2end_tests
################################################################################

add_executable(dawn_end2end_tests "End2EndTestsMain.cpp")
common_compile_options(dawn_end2end_tests)
target_link_libraries(dawn_end2end_tests PRIVATE
    dawn_internal_config
    dawn_end2end_test_objects
    dawn_whitebox_test_objects
    dawn_test_infra
    gmock
    partition_alloc
)
