# Copyright 2020 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

DawnJSONGenerator(
    TARGET "native_utils"
    PRINT_NAME "Dawn native utilities"
    OUTPUT_HEADERS DAWN_NATIVE_UTILS_GEN_HEADERS
    OUTPUT_SOURCES DAWN_NATIVE_UTILS_GEN_SOURCES
)

set(headers
    "${DAWN_INCLUDE_DIR}/dawn/native/DawnNative.h"
    "${DAWN_INCLUDE_DIR}/dawn/native/dawn_native_export.h"
)

set(private_headers
    "${DAWN_NATIVE_UTILS_GEN_HEADERS}"
    "Adapter.h"
    "ApplyClearColorValueWithDrawHelper.h"
    "AsyncTask.h"
    "AttachmentState.h"
    "BackendConnection.h"
    "BindGroup.h"
    "BindGroupLayout.h"
    "BindGroupLayoutInternal.h"
    "BindGroupTracker.h"
    "BindingInfo.h"
    "BlitBufferToDepthStencil.h"
    "BlitBufferToTexture.h"
    "BlitColorToColorWithDraw.h"
    "BlitDepthToDepth.h"
    "BlitTextureToBuffer.h"
    "Blob.h"
    "BlobCache.h"
    "BlockInfo.h"
    "BuddyAllocator.h"
    "BuddyMemoryAllocator.h"
    "Buffer.h"
    "CachedObject.h"
    "CacheKey.h"
    "CacheRequest.h"
    "CacheResult.h"
    "CallbackTaskManager.h"
    "ChainUtils.h"
    "CommandAllocator.h"
    "CommandBuffer.h"
    "CommandBufferStateTracker.h"
    "CommandEncoder.h"
    "Commands.h"
    "CommandValidation.h"
    "CompilationMessages.h"
    "ComputePassEncoder.h"
    "ComputePipeline.h"
    "CopyTextureForBrowserHelper.h"
    "CreatePipelineAsyncEvent.h"
    "dawn_platform.h"
    "Device.h"
    "DeviceGuard.h"
    "DynamicUploader.h"
    "EncodingContext.h"
    "EnumClassBitmasks.h"
    "EnumMaskIterator.h"
    "Error.h"
    "ErrorData.h"
    "ErrorInjector.h"
    "ErrorScope.h"
    "ErrorSink.h"
    "EventManager.h"
    "ExecutionQueue.h"
    "ExternalTexture.h"
    "Features.h"
    "Format.h"
    "Forward.h"
    "ImmediateConstantsLayout.h"
    "ImmediateConstantsTracker.h"
    "IndirectDrawMetadata.h"
    "IndirectDrawValidationEncoder.h"
    "Instance.h"
    "IntegerTypes.h"
    "InternalPipelineStore.h"
    "Limits.h"
    "LogEmitter.h"
    "ObjectBase.h"
    "ObjectContentHasher.h"
    "PassResourceUsage.h"
    "PassResourceUsageTracker.h"
    "PerStage.h"
    "PhysicalDevice.h"
    "Pipeline.h"
    "PipelineCache.h"
    "PipelineLayout.h"
    "PooledResourceMemoryAllocator.h"
    "ProgrammableEncoder.h"
    "QueryHelper.h"
    "QuerySet.h"
    "Queue.h"
    "RenderBundle.h"
    "RenderBundleEncoder.h"
    "RenderEncoderBase.h"
    "RenderPassEncoder.h"
    "RenderPassWorkaroundsHelper.h"
    "RenderPipeline.h"
    "ResourceHeap.h"
    "ResourceHeapAllocator.h"
    "ResourceMemoryAllocation.h"
    "RingBufferAllocator.h"
    "Sampler.h"
    "ScratchBuffer.h"
    "Serializable.h"
    "ShaderModule.h"
    "ShaderModuleParseRequest.h"
    "SharedBufferMemory.h"
    "SharedFence.h"
    "SharedResourceMemory.h"
    "SharedTextureMemory.h"
    "stream/BlobSource.h"
    "stream/ByteVectorSink.h"
    "stream/Sink.h"
    "stream/Source.h"
    "stream/Stream.h"
    "Subresource.h"
    "SubresourceStorage.h"
    "Surface.h"
    "SwapChain.h"
    "SystemEvent.h"
    "SystemHandle.h"
    "Texture.h"
    "TintUtils.h"
    "ToBackend.h"
    "Toggles.h"
    "UsageValidationMode.h"
    "utils/WGPUHelpers.h"
    "ValidationUtils.h"
    "VisitableMembers.h"
    "WaitAnySystemEvent.h"
    "WaitListEvent.h"
    "webgpu_absl_format.h"
)

set(sources
    ${DAWN_NATIVE_UTILS_GEN_SOURCES}
    "Adapter.cpp"
    "ApplyClearColorValueWithDrawHelper.cpp"
    "AsyncTask.cpp"
    "AttachmentState.cpp"
    "BackendConnection.cpp"
    "BindGroup.cpp"
    "BindGroupLayout.cpp"
    "BindGroupLayoutInternal.cpp"
    "BindingInfo.cpp"
    "BlitBufferToDepthStencil.cpp"
    "BlitBufferToTexture.cpp"
    "BlitColorToColorWithDraw.cpp"
    "BlitDepthToDepth.cpp"
    "BlitTextureToBuffer.cpp"
    "Blob.cpp"
    "BlobCache.cpp"
    "BuddyAllocator.cpp"
    "BuddyMemoryAllocator.cpp"
    "Buffer.cpp"
    "CachedObject.cpp"
    "CacheKey.cpp"
    "CacheRequest.cpp"
    "CallbackTaskManager.cpp"
    "ChainUtilsImpl.inl"
    "CommandAllocator.cpp"
    "CommandBuffer.cpp"
    "CommandBufferStateTracker.cpp"
    "CommandEncoder.cpp"
    "Commands.cpp"
    "CommandValidation.cpp"
    "CompilationMessages.cpp"
    "ComputePassEncoder.cpp"
    "ComputePipeline.cpp"
    "CopyTextureForBrowserHelper.cpp"
    "CreatePipelineAsyncEvent.cpp"
    "Device.cpp"
    "DeviceGuard.cpp"
    "DynamicUploader.cpp"
    "EncodingContext.cpp"
    "Error.cpp"
    "ErrorData.cpp"
    "ErrorInjector.cpp"
    "ErrorScope.cpp"
    "EventManager.cpp"
    "ExecutionQueue.cpp"
    "ExternalTexture.cpp"
    "Features.cpp"
    "Format.cpp"
    "ImmediateConstantsLayout.cpp"
    "ImmediateConstantsTracker.cpp"
    "IndirectDrawMetadata.cpp"
    "IndirectDrawValidationEncoder.cpp"
    "Instance.cpp"
    "InternalPipelineStore.cpp"
    "Limits.cpp"
    "ObjectBase.cpp"
    "ObjectContentHasher.cpp"
    "PassResourceUsageTracker.cpp"
    "PerStage.cpp"
    "PhysicalDevice.cpp"
    "Pipeline.cpp"
    "PipelineCache.cpp"
    "PipelineLayout.cpp"
    "PooledResourceMemoryAllocator.cpp"
    "ProgrammableEncoder.cpp"
    "QueryHelper.cpp"
    "QuerySet.cpp"
    "Queue.cpp"
    "RenderBundle.cpp"
    "RenderBundleEncoder.cpp"
    "RenderEncoderBase.cpp"
    "RenderPassEncoder.cpp"
    "RenderPassWorkaroundsHelper.cpp"
    "RenderPipeline.cpp"
    "ResourceMemoryAllocation.cpp"
    "RingBufferAllocator.cpp"
    "Sampler.cpp"
    "ScratchBuffer.cpp"
    "ShaderModule.cpp"
    "ShaderModuleParseRequest.cpp"
    "SharedBufferMemory.cpp"
    "SharedFence.cpp"
    "SharedResourceMemory.cpp"
    "SharedTextureMemory.cpp"
    "stream/BlobSource.cpp"
    "stream/ByteVectorSink.cpp"
    "stream/Stream.cpp"
    "Subresource.cpp"
    "Surface.cpp"
    "SwapChain.cpp"
    "SystemEvent.cpp"
    "SystemHandle.cpp"
    "Texture.cpp"
    "TintUtils.cpp"
    "Toggles.cpp"
    "utils/WGPUHelpers.cpp"
    "ValidationUtils.cpp"
    "WaitListEvent.cpp"
    "webgpu_absl_format.cpp"
)

set(conditional_public_platform_depends)
set(conditional_private_depends)
set(conditional_public_depends)

if (DAWN_USE_X11)
    find_package(X11 REQUIRED)
    list(APPEND private_headers
        "X11Functions.h"
    )
    list(APPEND sources
        "X11Functions.cpp"
    )
endif()

if (ANDROID)
    list(APPEND private_headers
        "AHBFunctions.h"
    )
    list(APPEND sources
        "AHBFunctions.cpp"
    )
endif()

find_package(Threads REQUIRED)
list(APPEND conditional_private_platform_depends
    Threads::Threads
    ${CMAKE_DL_LIBS}
)

# Only win32 app needs to link with user32.lib and onecore_apiset.lib
# In UWP, all availiable APIs are defined in WindowsApp.lib
# and is automatically linked when WINDOWS_STORE set
if (WIN32 AND NOT WINDOWS_STORE)
  list(APPEND conditional_private_platform_depends
       user32.lib
       onecore_apiset.lib
  )
endif()

# DXGIGetDebugInterface1 is defined in dxgi.lib
# But this API is tagged as a development-only capability
# which implies that linking to this function will cause
# the application to fail Windows store certification
# So we only link to it in debug build when compiling for UWP.
# In win32 we load dxgi.dll using LoadLibrary
# so no need for static linking.
if (WINDOWS_STORE)
  list(APPEND conditional_private_platform_depends debug dxgi.lib)
endif()

if (DAWN_ENABLE_D3D11 OR DAWN_ENABLE_D3D12)
    list(APPEND headers
        "${DAWN_INCLUDE_DIR}/dawn/native/D3DBackend.h"
    )
    list(APPEND private_headers
        "d3d/BackendD3D.h"
        "d3d/BlobD3D.h"
        "d3d/d3d_platform.h"
        "d3d/D3DCompilationRequest.h"
        "d3d/D3DError.h"
        "d3d/DeviceD3D.h"
        "d3d/Forward.h"
        "d3d/KeyedMutex.h"
        "d3d/PhysicalDeviceD3D.h"
        "d3d/PlatformFunctions.h"
        "d3d/QueueD3D.h"
        "d3d/ShaderUtils.h"
        "d3d/SharedFenceD3D.h"
        "d3d/SharedTextureMemoryD3D.h"
        "d3d/SwapChainD3D.h"
        "d3d/UtilsD3D.h"
    )
    list(APPEND sources
        "d3d/BackendD3D.cpp"
        "d3d/BlobD3D.cpp"
        "d3d/D3DError.cpp"
        "d3d/DeviceD3D.cpp"
        "d3d/KeyedMutex.cpp"
        "d3d/PhysicalDeviceD3D.cpp"
        "d3d/PlatformFunctions.cpp"
        "d3d/QueueD3D.cpp"
        "d3d/ShaderUtils.cpp"
        "d3d/SharedFenceD3D.cpp"
        "d3d/SharedTextureMemoryD3D.cpp"
        "d3d/SwapChainD3D.cpp"
        "d3d/UtilsD3D.cpp"
    )
    list(APPEND conditional_private_platform_depends dxguid.lib)
endif()

if (DAWN_ENABLE_D3D11)
    list(APPEND headers
        "${DAWN_INCLUDE_DIR}/dawn/native/D3D11Backend.h"
    )
    list(APPEND private_headers
        "d3d11/BackendD3D11.h"
        "d3d11/BindGroupD3D11.h"
        "d3d11/BindGroupLayoutD3D11.h"
        "d3d11/BindGroupTrackerD3D11.h"
        "d3d11/BufferD3D11.h"
        "d3d11/CommandBufferD3D11.h"
        "d3d11/CommandRecordingContextD3D11.h"
        "d3d11/ComputePipelineD3D11.h"
        "d3d11/DeviceD3D11.h"
        "d3d11/DeviceInfoD3D11.h"
        "d3d11/Forward.h"
        "d3d11/PhysicalDeviceD3D11.h"
        "d3d11/PipelineLayoutD3D11.h"
        "d3d11/PlatformFunctionsD3D11.h"
        "d3d11/QuerySetD3D11.h"
        "d3d11/QueueD3D11.h"
        "d3d11/RenderPipelineD3D11.h"
        "d3d11/SamplerD3D11.h"
        "d3d11/ShaderModuleD3D11.h"
        "d3d11/SharedFenceD3D11.h"
        "d3d11/SharedTextureMemoryD3D11.h"
        "d3d11/SwapChainD3D11.h"
        "d3d11/TextureD3D11.h"
        "d3d11/UtilsD3D11.h"
    )
    list(APPEND sources
        "d3d11/BackendD3D11.cpp"
        "d3d11/BindGroupD3D11.cpp"
        "d3d11/BindGroupLayoutD3D11.cpp"
        "d3d11/BindGroupTrackerD3D11.cpp"
        "d3d11/BufferD3D11.cpp"
        "d3d11/CommandBufferD3D11.cpp"
        "d3d11/CommandRecordingContextD3D11.cpp"
        "d3d11/ComputePipelineD3D11.cpp"
        "d3d11/DeviceD3D11.cpp"
        "d3d11/DeviceInfoD3D11.cpp"
        "d3d11/PhysicalDeviceD3D11.cpp"
        "d3d11/PipelineLayoutD3D11.cpp"
        "d3d11/PlatformFunctionsD3D11.cpp"
        "d3d11/QuerySetD3D11.cpp"
        "d3d11/QueueD3D11.cpp"
        "d3d11/RenderPipelineD3D11.cpp"
        "d3d11/SamplerD3D11.cpp"
        "d3d11/ShaderModuleD3D11.cpp"
        "d3d11/SharedFenceD3D11.cpp"
        "d3d11/SharedTextureMemoryD3D11.cpp"
        "d3d11/SwapChainD3D11.cpp"
        "d3d11/TextureD3D11.cpp"
        "d3d11/UtilsD3D11.cpp"
    )
endif()

if (DAWN_ENABLE_D3D12)
    list(APPEND headers
        "${DAWN_INCLUDE_DIR}/dawn/native/D3D12Backend.h"
    )
    list(APPEND private_headers
        "d3d12/BackendD3D12.h"
        "d3d12/BindGroupD3D12.h"
        "d3d12/BindGroupLayoutD3D12.h"
        "d3d12/BufferD3D12.h"
        "d3d12/CommandBufferD3D12.h"
        "d3d12/CommandRecordingContext.h"
        "d3d12/ComputePipelineD3D12.h"
        "d3d12/CPUDescriptorHeapAllocationD3D12.h"
        "d3d12/d3d12_platform.h"
        "d3d12/D3D12Info.h"
        "d3d12/DeviceD3D12.h"
        "d3d12/Forward.h"
        "d3d12/GPUDescriptorHeapAllocationD3D12.h"
        "d3d12/HeapAllocatorD3D12.h"
        "d3d12/HeapD3D12.h"
        "d3d12/IntegerTypes.h"
        "d3d12/PageableD3D12.h"
        "d3d12/PhysicalDeviceD3D12.h"
        "d3d12/PipelineLayoutD3D12.h"
        "d3d12/PlatformFunctionsD3D12.h"
        "d3d12/QuerySetD3D12.h"
        "d3d12/QueueD3D12.h"
        "d3d12/RenderPassBuilderD3D12.h"
        "d3d12/RenderPipelineD3D12.h"
        "d3d12/ResidencyManagerD3D12.h"
        "d3d12/ResourceAllocatorManagerD3D12.h"
        "d3d12/ResourceHeapAllocationD3D12.h"
        "d3d12/SamplerD3D12.h"
        "d3d12/SamplerHeapCacheD3D12.h"
        "d3d12/ShaderModuleD3D12.h"
        "d3d12/ShaderVisibleDescriptorAllocatorD3D12.h"
        "d3d12/SharedBufferMemoryD3D12.h"
        "d3d12/SharedFenceD3D12.h"
        "d3d12/SharedTextureMemoryD3D12.h"
        "d3d12/StagingDescriptorAllocatorD3D12.h"
        "d3d12/SwapChainD3D12.h"
        "d3d12/TextureCopySplitter.h"
        "d3d12/TextureD3D12.h"
        "d3d12/UtilsD3D12.h"
    )
    list(APPEND sources
        "d3d12/BackendD3D12.cpp"
        "d3d12/BindGroupD3D12.cpp"
        "d3d12/BindGroupLayoutD3D12.cpp"
        "d3d12/BufferD3D12.cpp"
        "d3d12/CommandBufferD3D12.cpp"
        "d3d12/CommandRecordingContext.cpp"
        "d3d12/ComputePipelineD3D12.cpp"
        "d3d12/CPUDescriptorHeapAllocationD3D12.cpp"
        "d3d12/D3D12Info.cpp"
        "d3d12/DeviceD3D12.cpp"
        "d3d12/GPUDescriptorHeapAllocationD3D12.cpp"
        "d3d12/HeapAllocatorD3D12.cpp"
        "d3d12/HeapD3D12.cpp"
        "d3d12/PageableD3D12.cpp"
        "d3d12/PhysicalDeviceD3D12.cpp"
        "d3d12/PipelineLayoutD3D12.cpp"
        "d3d12/PlatformFunctionsD3D12.cpp"
        "d3d12/QuerySetD3D12.cpp"
        "d3d12/QueueD3D12.cpp"
        "d3d12/RenderPassBuilderD3D12.cpp"
        "d3d12/RenderPipelineD3D12.cpp"
        "d3d12/ResidencyManagerD3D12.cpp"
        "d3d12/ResourceAllocatorManagerD3D12.cpp"
        "d3d12/ResourceHeapAllocationD3D12.cpp"
        "d3d12/SamplerD3D12.cpp"
        "d3d12/SamplerHeapCacheD3D12.cpp"
        "d3d12/ShaderModuleD3D12.cpp"
        "d3d12/ShaderVisibleDescriptorAllocatorD3D12.cpp"
        "d3d12/SharedBufferMemoryD3D12.cpp"
        "d3d12/SharedFenceD3D12.cpp"
        "d3d12/SharedTextureMemoryD3D12.cpp"
        "d3d12/StagingDescriptorAllocatorD3D12.cpp"
        "d3d12/StreamImplD3D12.cpp"
        "d3d12/SwapChainD3D12.cpp"
        "d3d12/TextureCopySplitter.cpp"
        "d3d12/TextureD3D12.cpp"
        "d3d12/UtilsD3D12.cpp"
    )
endif()

if (DAWN_ENABLE_METAL)
    list(APPEND headers
        "${DAWN_INCLUDE_DIR}/dawn/native/MetalBackend.h"
    )
    list(APPEND private_headers
        "metal/BackendMTL.h"
        "metal/BindGroupLayoutMTL.h"
        "metal/BindGroupMTL.h"
        "metal/BufferMTL.h"
        "metal/CommandBufferMTL.h"
        "metal/CommandRecordingContext.h"
        "metal/ComputePipelineMTL.h"
        "metal/DeviceMTL.h"
        "metal/Forward.h"
        "metal/MultiDrawEncoder.h"
        "metal/MultiDrawEncoder.mm"
        "metal/PhysicalDeviceMTL.h"
        "metal/PipelineLayoutMTL.h"
        "metal/QuerySetMTL.h"
        "metal/QueueMTL.h"
        "metal/RenderPipelineMTL.h"
        "metal/SamplerMTL.h"
        "metal/ShaderModuleMTL.h"
        "metal/SharedFenceMTL.h"
        "metal/SharedTextureMemoryMTL.h"
        "metal/SwapChainMTL.h"
        "metal/TextureMTL.h"
        "metal/UtilsMetal.h"
    )
    list(APPEND sources
        "metal/BackendMTL.mm"
        "metal/BindGroupLayoutMTL.mm"
        "metal/BindGroupMTL.mm"
        "metal/BufferMTL.mm"
        "metal/CommandBufferMTL.mm"
        "metal/CommandRecordingContext.mm"
        "metal/ComputePipelineMTL.mm"
        "metal/DeviceMTL.mm"
        "metal/PhysicalDeviceMTL.mm"
        "metal/PipelineLayoutMTL.mm"
        "metal/QuerySetMTL.mm"
        "metal/QueueMTL.mm"
        "metal/RenderPipelineMTL.mm"
        "metal/SamplerMTL.mm"
        "metal/ShaderModuleMTL.mm"
        "metal/SharedFenceMTL.mm"
        "metal/SharedTextureMemoryMTL.mm"
        "metal/SwapChainMTL.mm"
        "metal/TextureMTL.mm"
        "metal/UtilsMetal.mm"
        "Surface_metal.mm"
    )
    if (DAWN_TARGET_MACOS)
        list(APPEND conditional_private_platform_depends
            "-framework Cocoa"
            "-framework IOKit"
        )
    endif()
    list(APPEND conditional_private_platform_depends
        "-framework Foundation"
        "-framework IOSurface"
        "-framework QuartzCore"
        "-framework Metal"
    )
endif()

if (DAWN_ENABLE_NULL)
    list(APPEND headers
        "${DAWN_INCLUDE_DIR}/dawn/native/NullBackend.h"
    )
    list(APPEND private_headers
        "null/DeviceNull.h"
    )
    list(APPEND sources
        "null/DeviceNull.cpp"
    )
endif()

if (DAWN_ENABLE_WEBGPU_ON_WEBGPU)
    list(APPEND headers
        "${DAWN_INCLUDE_DIR}/dawn/native/WebGPUBackend.h"
    )
    list(APPEND private_headers
        "webgpu/BackendWGPU.h"
        "webgpu/BufferWGPU.h"
        "webgpu/CommandBufferWGPU.h"
        "webgpu/DeviceWGPU.h"
        "webgpu/Forward.h"
        "webgpu/ObjectWGPU.h"
        "webgpu/PhysicalDeviceWGPU.h"
        "webgpu/QueueWGPU.h"
        "webgpu/WebGPUError.h"
    )
    list(APPEND sources
        "webgpu/BackendWGPU.cpp"
        "webgpu/BufferWGPU.cpp"
        "webgpu/CommandBufferWGPU.cpp"
        "webgpu/DeviceWGPU.cpp"
        "webgpu/PhysicalDeviceWGPU.cpp"
        "webgpu/QueueWGPU.cpp"
    )
endif()

if ((DAWN_ENABLE_OPENGL OR DAWN_ENABLE_VULKAN) AND DAWN_ENABLE_SPIRV_VALIDATION)
    list(APPEND private_headers
        "SpirvValidation.h"
    )
    list(APPEND sources
        "SpirvValidation.cpp"
    )
    list(APPEND conditional_private_depends
        SPIRV-Tools-opt
    )
endif()

if (DAWN_ENABLE_OPENGL)
    DawnGenerator(
        SCRIPT "${Dawn_SOURCE_DIR}/generator/opengl_loader_generator.py"
        PRINT_NAME "OpenGL function loader"
        EXTRA_PARAMETERS "--gl-xml"
             "${Dawn_SOURCE_DIR}/third_party/khronos/OpenGL-Registry/xml/gl.xml"
             "--supported-extensions"
             "${Dawn_SOURCE_DIR}/src/dawn/native/opengl/supported_extensions.json"
        OUTPUT_HEADERS DAWN_NATIVE_OPENGL_AUTOGEN_HEADERS
        OUTPUT_SOURCES DAWN_NATIVE_OPENGL_AUTOGEN_SOURCES
    )

    list(APPEND headers
        "${DAWN_INCLUDE_DIR}/dawn/native/OpenGLBackend.h"
    )
    list(APPEND private_headers
        ${DAWN_NATIVE_OPENGL_AUTOGEN_HEADERS}
        "opengl/BackendGL.h"
        "opengl/BindGroupGL.h"
        "opengl/BindGroupLayoutGL.h"
        "opengl/BufferGL.h"
        "opengl/CommandBufferGL.h"
        "opengl/ComputePipelineGL.h"
        "opengl/ContextEGL.h"
        "opengl/DeviceGL.h"
        "opengl/DisplayEGL.h"
        "opengl/EGLFunctions.h"
        "opengl/Forward.h"
        "opengl/GLFormat.h"
        "opengl/IntegerTypes.h"
        "opengl/opengl_platform.h"
        "opengl/OpenGLFunctions.h"
        "opengl/OpenGLVersion.h"
        "opengl/PersistentPipelineStateGL.h"
        "opengl/PhysicalDeviceGL.h"
        "opengl/PipelineGL.h"
        "opengl/PipelineLayoutGL.h"
        "opengl/QuerySetGL.h"
        "opengl/QueueGL.h"
        "opengl/RenderPipelineGL.h"
        "opengl/SamplerGL.h"
        "opengl/ShaderModuleGL.h"
        "opengl/SharedFenceEGL.h"
        "opengl/SharedFenceGL.h"
        "opengl/SharedTextureMemoryEGL.h"
        "opengl/SharedTextureMemoryGL.h"
        "opengl/SwapChainEGL.h"
        "opengl/TextureGL.h"
        "opengl/UtilsEGL.h"
        "opengl/UtilsGL.h"
    )
    list(APPEND sources
        ${DAWN_NATIVE_OPENGL_AUTOGEN_SOURCES}
        "opengl/BackendGL.cpp"
        "opengl/BindGroupGL.cpp"
        "opengl/BindGroupLayoutGL.cpp"
        "opengl/BufferGL.cpp"
        "opengl/CommandBufferGL.cpp"
        "opengl/ComputePipelineGL.cpp"
        "opengl/ContextEGL.cpp"
        "opengl/DeviceGL.cpp"
        "opengl/DisplayEGL.cpp"
        "opengl/EGLFunctions.cpp"
        "opengl/GLFormat.cpp"
        "opengl/OpenGLFunctions.cpp"
        "opengl/OpenGLVersion.cpp"
        "opengl/PersistentPipelineStateGL.cpp"
        "opengl/PhysicalDeviceGL.cpp"
        "opengl/PipelineGL.cpp"
        "opengl/PipelineLayoutGL.cpp"
        "opengl/QuerySetGL.cpp"
        "opengl/QueueGL.cpp"
        "opengl/RenderPipelineGL.cpp"
        "opengl/SamplerGL.cpp"
        "opengl/ShaderModuleGL.cpp"
        "opengl/SharedFenceEGL.cpp"
        "opengl/SharedFenceGL.cpp"
        "opengl/SharedTextureMemoryEGL.cpp"
        "opengl/SharedTextureMemoryGL.cpp"
        "opengl/SwapChainEGL.cpp"
        "opengl/TextureGL.cpp"
        "opengl/UtilsEGL.cpp"
        "opengl/UtilsGL.cpp"
    )

    list(APPEND conditional_private_depends dawn_khronos_platform)
endif()

if (DAWN_ENABLE_VULKAN)
    list(APPEND headers
        "${DAWN_INCLUDE_DIR}/dawn/native/VulkanBackend.h"
    )
    list(APPEND private_headers
        "vulkan/BackendVk.h"
        "vulkan/BindGroupLayoutVk.h"
        "vulkan/BindGroupVk.h"
        "vulkan/BufferVk.h"
        "vulkan/CommandBufferVk.h"
        "vulkan/CommandRecordingContextVk.h"
        "vulkan/ComputePipelineVk.h"
        "vulkan/DescriptorSetAllocation.h"
        "vulkan/DescriptorSetAllocator.h"
        "vulkan/DeviceVk.h"
        "vulkan/ExternalHandle.h"
        "vulkan/FencedDeleter.h"
        "vulkan/Forward.h"
        "vulkan/FramebufferCache.h"
        "vulkan/PhysicalDeviceVk.h"
        "vulkan/PipelineVk.h"
        "vulkan/PipelineCacheVk.h"
        "vulkan/PipelineLayoutVk.h"
        "vulkan/QuerySetVk.h"
        "vulkan/QueueVk.h"
        "vulkan/RefCountedVkHandle.h"
        "vulkan/RenderPassCache.h"
        "vulkan/RenderPipelineVk.h"
        "vulkan/ResolveTextureLoadingUtilsVk.h"
        "vulkan/ResourceHeapVk.h"
        "vulkan/ResourceMemoryAllocatorVk.h"
        "vulkan/SamplerVk.h"
        "vulkan/ShaderModuleVk.h"
        "vulkan/SharedFenceVk.h"
        "vulkan/SharedTextureMemoryVk.h"
        "vulkan/SwapChainVk.h"
        "vulkan/TextureVk.h"
        "vulkan/UniqueVkHandle.h"
        "vulkan/UtilsVulkan.h"
        "vulkan/VulkanError.h"
        "vulkan/VulkanExtensions.h"
        "vulkan/VulkanFunctions.h"
        "vulkan/VulkanInfo.h"
        "vulkan/external_memory/MemoryImportParams.h"
        "vulkan/external_memory/MemoryService.h"
        "vulkan/external_memory/MemoryServiceImplementation.h"
        "vulkan/external_semaphore/SemaphoreService.h"
        "vulkan/external_semaphore/SemaphoreServiceImplementation.h"
    )
    list(APPEND sources
        "vulkan/BackendVk.cpp"
        "vulkan/BindGroupLayoutVk.cpp"
        "vulkan/BindGroupVk.cpp"
        "vulkan/BufferVk.cpp"
        "vulkan/CommandBufferVk.cpp"
        "vulkan/ComputePipelineVk.cpp"
        "vulkan/CommandRecordingContextVk.cpp"
        "vulkan/DescriptorSetAllocator.cpp"
        "vulkan/DeviceVk.cpp"
        "vulkan/FencedDeleter.cpp"
        "vulkan/FramebufferCache.cpp"
        "vulkan/PhysicalDeviceVk.cpp"
        "vulkan/PipelineVk.cpp"
        "vulkan/PipelineCacheVk.cpp"
        "vulkan/PipelineLayoutVk.cpp"
        "vulkan/QuerySetVk.cpp"
        "vulkan/QueueVk.cpp"
        "vulkan/RenderPassCache.cpp"
        "vulkan/RenderPipelineVk.cpp"
        "vulkan/ResolveTextureLoadingUtilsVk.cpp"
        "vulkan/ResourceHeapVk.cpp"
        "vulkan/ResourceMemoryAllocatorVk.cpp"
        "vulkan/SamplerVk.cpp"
        "vulkan/ShaderModuleVk.cpp"
        "vulkan/SharedFenceVk.cpp"
        "vulkan/SharedTextureMemoryVk.cpp"
        "vulkan/StreamImplVk.cpp"
        "vulkan/SwapChainVk.cpp"
        "vulkan/TextureVk.cpp"
        "vulkan/UtilsVulkan.cpp"
        "vulkan/VulkanError.cpp"
        "vulkan/VulkanExtensions.cpp"
        "vulkan/VulkanFunctions.cpp"
        "vulkan/VulkanInfo.cpp"
        "vulkan/external_memory/MemoryService.cpp"
        "vulkan/external_memory/MemoryServiceImplementation.cpp"
        "vulkan/external_semaphore/SemaphoreService.cpp"
        "vulkan/external_semaphore/SemaphoreServiceImplementation.cpp"
    )

    list(APPEND conditional_public_depends
        Vulkan::Headers
        Vulkan::UtilityHeaders
    )

    if (ANDROID)
        list(APPEND private_headers
            "vulkan/external_memory/MemoryServiceImplementationAHardwareBuffer.h"
            "vulkan/external_semaphore/SemaphoreServiceImplementationFD.h"
        )
        list(APPEND sources
            "vulkan/external_memory/MemoryServiceImplementationAHardwareBuffer.cpp"
            "vulkan/external_semaphore/SemaphoreServiceImplementationFD.cpp"
        )
    elseif (UNIX AND NOT APPLE)
        list(APPEND private_headers
            "vulkan/external_memory/MemoryServiceImplementationDmaBuf.h"
            "vulkan/external_memory/MemoryServiceImplementationOpaqueFD.h"
            "vulkan/external_semaphore/SemaphoreServiceImplementationFD.h"
        )
        list(APPEND sources
            "vulkan/external_memory/MemoryServiceImplementationDmaBuf.cpp"
            "vulkan/external_memory/MemoryServiceImplementationOpaqueFD.cpp"
            "vulkan/external_semaphore/SemaphoreServiceImplementationFD.cpp"
        )
    endif()
endif()

set(dawn_component_srcs)
list(APPEND dawn_component_srcs "DawnNative.cpp")
if (DAWN_ENABLE_D3D11 OR DAWN_ENABLE_D3D12)
    list(APPEND dawn_component_srcs "d3d/D3DBackend.cpp")
endif()
if (DAWN_ENABLE_D3D11)
    list(APPEND dawn_component_srcs "d3d11/D3D11Backend.cpp")
endif()
if (DAWN_ENABLE_D3D12)
    list(APPEND dawn_component_srcs "d3d12/D3D12Backend.cpp")
endif()
if (DAWN_ENABLE_METAL)
    list(APPEND dawn_component_srcs "metal/MetalBackend.mm")
endif()
if (DAWN_ENABLE_NULL)
    list(APPEND dawn_component_srcs "null/NullBackend.cpp")
endif()
if (DAWN_ENABLE_WEBGPU_ON_WEBGPU)
    list(APPEND dawn_component_srcs "webgpu/WebGPUBackend.cpp")
endif()
if (DAWN_ENABLE_OPENGL)
    list(APPEND dawn_component_srcs "opengl/OpenGLBackend.cpp")
endif()
if (DAWN_ENABLE_VULKAN)
    list(APPEND dawn_component_srcs "vulkan/VulkanBackend.cpp")
endif()

set(dawn_native_public_depends
    dawn::dawn_common
    dawn::dawn_platform
    dawn::dawncpp_headers
    dawn::partition_alloc
    ${conditional_public_depends}
)
set(dawn_native_private_depends
    absl::flat_hash_map
    absl::flat_hash_set
    absl::inlined_vector
    absl::str_format_internal
    absl::strings
    libtint
    ${conditional_private_depends}
)

# The same object files are reused to build
# 1. dawn_native (which gets used by tests, samples, benchmarks)
# 2. webgpu_dawn bundle (which gets installed via CMake install rules)
dawn_add_library(
  dawn_native_objects
  UTILITY_TARGET dawn_internal_config
  FORCE_OBJECT
  HEADERS
    ${headers}
  PRIVATE_HEADERS
    ${private_headers}
  SOURCES
    ${sources}
  DEPENDS
    ${dawn_native_public_depends}
  PRIVATE_DEPENDS
    ${dawn_native_private_depends}
    ${conditional_private_platform_depends}
)

dawn_add_library(
  dawn_native
  UTILITY_TARGET dawn_internal_config
  SOURCES
    ${dawn_component_srcs}
  DEPENDS
    dawn::dawn_native_objects
    ${dawn_native_public_depends}
  PRIVATE_DEPENDS
    ${dawn_native_private_depends}
    ${conditional_private_platform_depends}
)

target_compile_definitions(dawn_native PRIVATE "DAWN_NATIVE_IMPLEMENTATION")
if(BUILD_SHARED_LIBS)
    target_compile_definitions(dawn_native PUBLIC "DAWN_NATIVE_SHARED_LIBRARY")
endif()

if (DAWN_USE_X11)
    target_include_directories(dawn_native PRIVATE ${X11_INCLUDE_DIR})
endif()

if (DAWN_ENABLE_D3D12)
    if (DAWN_USE_BUILT_DXC)
        target_compile_definitions(dawn_native PRIVATE "DAWN_USE_BUILT_DXC")
        target_compile_definitions(dawn_native_objects PRIVATE "DAWN_USE_BUILT_DXC")
        add_dependencies(dawn_native copy_dxil_dll)
    endif()
endif()

if ((DAWN_ENABLE_OPENGL OR DAWN_ENABLE_VULKAN) AND DAWN_ENABLE_SPIRV_VALIDATION)
    target_compile_definitions(dawn_native PRIVATE "DAWN_ENABLE_SPIRV_VALIDATION")
    target_compile_definitions(dawn_native_objects PRIVATE "DAWN_ENABLE_SPIRV_VALIDATION")
endif()

if (DAWN_ENABLE_SWIFTSHADER)
    target_compile_definitions(dawn_native PRIVATE "DAWN_ENABLE_SWIFTSHADER")
    target_compile_definitions(dawn_native_objects PRIVATE "DAWN_ENABLE_SWIFTSHADER")
endif()

if (IOS)
    target_compile_options(dawn_native_objects PRIVATE -fno-objc-arc)
    target_compile_options(dawn_native PRIVATE -fno-objc-arc)
endif()

if (DAWN_BUILD_MONOLITHIC_LIBRARY)
    ###############################################################################
    # Do the 'complete_lib' build.
    # A convenience target that bundles dawn_native and procs calling it directly so that
    # applications link against it and have WebGPU work without jumping through more hoops.
    # Note that this library name is referenced in several places, search for it and things like:
    # "{{.*}}_dawn" when you rename it.
    ###############################################################################

    # First make an object library for webgpu_dawn_native_proc sources for the export macros to
    # get applied as well as backend-specific dawn_native entrypoints that also use export macros.
    # This is essentially the same as dawn_native but using OBJECT as a library type and the
    # DAWN_BUILD_MONOLITHIC_LIBRARY option to choose the export macros.
    DawnJSONGenerator(
        TARGET "webgpu_dawn_native_proc"
        PRINT_NAME "Dawn native WebGPU procs"
        OUTPUT_SOURCES WEBGPU_DAWN_NATIVE_PROC_GEN_SOURCES
    )

    dawn_add_library(
      webgpu_dawn_objects
      UTILITY_TARGET dawn_internal_config
      FORCE_OBJECT
      SOURCES
        ${WEBGPU_DAWN_NATIVE_PROC_GEN_SOURCES}
        ${dawn_component_srcs}
      DEPENDS
        ${dawn_native_public_depends}
      PRIVATE_DEPENDS
        dawn::dawn_native_objects
        ${dawn_native_private_depends}
        ${conditional_private_platform_depends}
    )

    target_compile_definitions(webgpu_dawn_objects
        PRIVATE
            "WGPU_IMPLEMENTATION"
            "DAWN_NATIVE_IMPLEMENTATION"
    )
    if (DAWN_BUILD_MONOLITHIC_LIBRARY STREQUAL SHARED)
        target_compile_definitions(webgpu_dawn_objects
            PUBLIC
                "WGPU_SHARED_LIBRARY"
                "DAWN_NATIVE_SHARED_LIBRARY"
        )
    endif()

    # Do the bundling of all the objects and dependencies together.
    include(BundleLibraries)
    bundle_libraries(webgpu_dawn ${DAWN_BUILD_MONOLITHIC_LIBRARY} webgpu_dawn_objects)
    add_library(dawn::webgpu_dawn ALIAS webgpu_dawn)

    # Since this is a bundled library, the only dependency is the config needed to use it.
    target_link_libraries(
      webgpu_dawn
      PUBLIC
        dawn_public_config
      PRIVATE
        ${conditional_private_platform_depends}
    )

    set(webgpu_dawn_public_headers)
    foreach(dawn_target_name IN ITEMS dawn_headers dawncpp_headers dawn_native_objects)
        get_target_property(headers "${dawn_target_name}" INTERFACE_SOURCES)
        list(INSERT webgpu_dawn_public_headers 0 "${headers}")
    endforeach()
    if (DAWN_ENABLE_INSTALL)
        dawn_install_target(dawn_public_config)
        dawn_install_target(webgpu_dawn
            HEADERS ${webgpu_dawn_public_headers})
    endif ()
endif ()

# Linking with dxcompiler after bundling so that the bundling process
# doesn't see the dxcompiler and end up with unresolved linker errors.
# They happen because dxcompiler is declared a shared library and bundle_libraries
# doesn't work well with shared libs
if (DAWN_USE_BUILT_DXC)
    target_link_libraries(dawn_native PRIVATE dxcompiler)
endif()
