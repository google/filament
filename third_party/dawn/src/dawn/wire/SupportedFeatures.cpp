// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/wire/SupportedFeatures.h"

namespace dawn::wire {

// Note: Upon updating this list, please also update serialization/deserialization
// of limit structs on Adapter/Device initialization.
bool IsFeatureSupported(WGPUFeatureName feature) {
    switch (feature) {
        case WGPUFeatureName_Force32:
        case WGPUFeatureName_DawnNative:
        case WGPUFeatureName_ImplicitDeviceSynchronization:
        case WGPUFeatureName_D3D11MultithreadProtected:
        case WGPUFeatureName_HostMappedPointer:
        case WGPUFeatureName_BufferMapExtendedUsages:
        case WGPUFeatureName_DawnFormatCapabilities:
        case WGPUFeatureName_DawnDrmFormatCapabilities:
            return false;
        // NOTE: SharedTextureMemory/SharedFence are not actually intended
        // for usage over the wire (and are not exposed over the wire as
        // they're marked as native in dawn.json). However, embedders need
        // to use these features on the server-side for WebGPU devices. To
        // do so they need to add the features on such devices, which requires
        // that the features be listed as wire-supported here to avoid
        // device creation failure that would otherwise occur.
        case WGPUFeatureName_SharedTextureMemoryVkDedicatedAllocation:
        case WGPUFeatureName_SharedTextureMemoryAHardwareBuffer:
        case WGPUFeatureName_SharedTextureMemoryDmaBuf:
        case WGPUFeatureName_SharedTextureMemoryOpaqueFD:
        case WGPUFeatureName_SharedTextureMemoryZirconHandle:
        case WGPUFeatureName_SharedTextureMemoryDXGISharedHandle:
        case WGPUFeatureName_SharedTextureMemoryD3D11Texture2D:
        case WGPUFeatureName_SharedTextureMemoryIOSurface:
        case WGPUFeatureName_SharedTextureMemoryEGLImage:
        case WGPUFeatureName_SharedFenceVkSemaphoreOpaqueFD:
        case WGPUFeatureName_SharedFenceSyncFD:
        case WGPUFeatureName_SharedFenceVkSemaphoreZirconHandle:
        case WGPUFeatureName_SharedFenceDXGISharedHandle:
        case WGPUFeatureName_SharedFenceMTLSharedEvent:
        case WGPUFeatureName_SharedBufferMemoryD3D12Resource:

        case WGPUFeatureName_Depth32FloatStencil8:
        case WGPUFeatureName_TimestampQuery:
        case WGPUFeatureName_ChromiumExperimentalTimestampQueryInsidePasses:
        case WGPUFeatureName_TextureCompressionBC:
        case WGPUFeatureName_TextureCompressionETC2:
        case WGPUFeatureName_TextureCompressionASTC:
        case WGPUFeatureName_IndirectFirstInstance:
        case WGPUFeatureName_DepthClipControl:
        case WGPUFeatureName_DawnInternalUsages:
        case WGPUFeatureName_DawnMultiPlanarFormats:
        case WGPUFeatureName_MultiDrawIndirect:
        case WGPUFeatureName_MultiPlanarFormatExtendedUsages:
        case WGPUFeatureName_MultiPlanarFormatP010:
        case WGPUFeatureName_MultiPlanarFormatP210:
        case WGPUFeatureName_MultiPlanarFormatP410:
        case WGPUFeatureName_MultiPlanarFormatNv16:
        case WGPUFeatureName_MultiPlanarFormatNv24:
        case WGPUFeatureName_MultiPlanarFormatNv12a:
        case WGPUFeatureName_MultiPlanarRenderTargets:
        case WGPUFeatureName_ShaderF16:
        case WGPUFeatureName_RG11B10UfloatRenderable:
        case WGPUFeatureName_BGRA8UnormStorage:
        case WGPUFeatureName_TransientAttachments:
        case WGPUFeatureName_Float32Filterable:
        case WGPUFeatureName_Float32Blendable:
        case WGPUFeatureName_MSAARenderToSingleSampled:
        case WGPUFeatureName_DualSourceBlending:
        case WGPUFeatureName_ANGLETextureSharing:
        case WGPUFeatureName_PixelLocalStorageCoherent:
        case WGPUFeatureName_PixelLocalStorageNonCoherent:
        case WGPUFeatureName_Norm16TextureFormats:
        case WGPUFeatureName_Unorm16TextureFormats:
        case WGPUFeatureName_Snorm16TextureFormats:
        case WGPUFeatureName_FramebufferFetch:
        case WGPUFeatureName_AdapterPropertiesMemoryHeaps:
        case WGPUFeatureName_AdapterPropertiesD3D:
        case WGPUFeatureName_AdapterPropertiesVk:
        case WGPUFeatureName_R8UnormStorage:
        case WGPUFeatureName_StaticSamplers:
        case WGPUFeatureName_YCbCrVulkanSamplers:
        case WGPUFeatureName_ShaderModuleCompilationOptions:
        case WGPUFeatureName_DawnLoadResolveTexture:
        case WGPUFeatureName_DawnPartialLoadResolveTexture:
        case WGPUFeatureName_Subgroups:
        case WGPUFeatureName_SubgroupsF16:
        case WGPUFeatureName_ClipDistances:
        case WGPUFeatureName_ChromiumExperimentalImmediateData:
        case WGPUFeatureName_DawnTexelCopyBufferRowAlignment:
        case WGPUFeatureName_FlexibleTextureViews:
            return true;
    }

    // Catch-all, for unsupported features.
    // "default:" is not used so we get compiler errors for
    // newly added, unhandled features, but still catch completely
    // unknown enums.
    return false;
}

}  // namespace dawn::wire
