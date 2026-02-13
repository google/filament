// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/native/d3d12/D3D12Info.h"

#include <utility>

#include "dawn/common/GPUInfo.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/BackendD3D12.h"
#include "dawn/native/d3d12/PhysicalDeviceD3D12.h"
#include "dawn/native/d3d12/PlatformFunctionsD3D12.h"

namespace dawn::native::d3d12 {

ResultOrError<D3D12DeviceInfo> GatherDeviceInfo(const PhysicalDevice& physicalDevice) {
    D3D12DeviceInfo info = {};

    // Newer builds replace D3D_FEATURE_DATA_ARCHITECTURE with
    // D3D_FEATURE_DATA_ARCHITECTURE1. However, D3D_FEATURE_DATA_ARCHITECTURE can be used
    // for backwards compat.
    // https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ne-d3d12-d3d12_feature
    D3D12_FEATURE_DATA_ARCHITECTURE arch = {};
    DAWN_TRY(CheckHRESULT(physicalDevice.GetDevice()->CheckFeatureSupport(
                              D3D12_FEATURE_ARCHITECTURE, &arch, sizeof(arch)),
                          "ID3D12Device::CheckFeatureSupport"));

    info.isUMA = arch.UMA;
    info.isCacheCoherentUMA = arch.CacheCoherentUMA;

    D3D12_FEATURE_DATA_D3D12_OPTIONS featureOptions = {};
    DAWN_TRY(CheckHRESULT(physicalDevice.GetDevice()->CheckFeatureSupport(
                              D3D12_FEATURE_D3D12_OPTIONS, &featureOptions, sizeof(featureOptions)),
                          "ID3D12Device::CheckFeatureSupport"));
    info.resourceHeapTier = featureOptions.ResourceHeapTier;

    D3D12_FEATURE_DATA_D3D12_OPTIONS2 featureOptions2 = {};
    if (SUCCEEDED(physicalDevice.GetDevice()->CheckFeatureSupport(
            D3D12_FEATURE_D3D12_OPTIONS2, &featureOptions2, sizeof(featureOptions2)))) {
        info.programmableSamplePositionsTier = featureOptions2.ProgrammableSamplePositionsTier;
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS3 featureOptions3 = {};
    if (SUCCEEDED(physicalDevice.GetDevice()->CheckFeatureSupport(
            D3D12_FEATURE_D3D12_OPTIONS3, &featureOptions3, sizeof(featureOptions3)))) {
        info.supportsCastingFullyTypedFormat = featureOptions3.CastingFullyTypedFormatSupported;
    }

    // Used to share resources cross-API. If we query CheckFeatureSupport for
    // D3D12_FEATURE_D3D12_OPTIONS4 successfully, then we can use cross-API sharing.
    info.supportsSharedResourceCapabilityTier1 = false;
    D3D12_FEATURE_DATA_D3D12_OPTIONS4 featureOptions4 = {};
    if (SUCCEEDED(physicalDevice.GetDevice()->CheckFeatureSupport(
            D3D12_FEATURE_D3D12_OPTIONS4, &featureOptions4, sizeof(featureOptions4)))) {
        // Tier 1 support additionally enables the NV12 format. Since only the NV12 format
        // is used by Dawn, check for Tier 1.
        if (featureOptions4.SharedResourceCompatibilityTier >=
            D3D12_SHARED_RESOURCE_COMPATIBILITY_TIER_1) {
            info.supportsSharedResourceCapabilityTier1 = true;
        }

        // featureOptions4.MSAA64KBAlignedTextureSupported indicates whether 64KB-aligned MSAA
        // textures are supported.
        info.use64KBAlignedMSAATexture = featureOptions4.MSAA64KBAlignedTextureSupported;

        // To support shader f16 feature, both featureOptions4.Native16BitShaderOpsSupported and
        // using shader model version >= 6.2 are required.
        info.supportsNative16BitShaderOps = featureOptions4.Native16BitShaderOpsSupported;
    }

#if D3D12_SDK_VERSION >= 612
    D3D12_FEATURE_DATA_D3D12_OPTIONS18 featureOptions18 = {};
    if (SUCCEEDED(physicalDevice.GetDevice()->CheckFeatureSupport(
            D3D12_FEATURE_D3D12_OPTIONS18, &featureOptions18, sizeof(featureOptions18)))) {
        info.supportsRenderPass = featureOptions18.RenderPassesValid;
    }
#endif

    // D3D12_HEAP_FLAG_CREATE_NOT_ZEROED is available anytime that ID3D12Device8 is exposed, or a
    // check for D3D12_FEATURE_D3D12_OPTIONS7 succeeds.
    D3D12_FEATURE_DATA_D3D12_OPTIONS7 featureOptions7 = {};
    if (SUCCEEDED(physicalDevice.GetDevice()->CheckFeatureSupport(
            D3D12_FEATURE_D3D12_OPTIONS7, &featureOptions7, sizeof(featureOptions7)))) {
        info.supportsHeapFlagCreateNotZeroed = true;
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS13 featureOptions13 = {};
    if (SUCCEEDED(physicalDevice.GetDevice()->CheckFeatureSupport(
            D3D12_FEATURE_D3D12_OPTIONS13, &featureOptions13, sizeof(featureOptions13)))) {
        info.supportsTextureCopyBetweenDimensions =
            featureOptions13.TextureCopyBetweenDimensionsSupported;
        info.supportsUnrestrictedBufferTextureCopyPitch =
            featureOptions13.UnrestrictedBufferTextureCopyPitchSupported;
    }

    info.supportsRootSignatureVersion1_1 = false;
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureDataRootSignature = {};
    featureDataRootSignature.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (SUCCEEDED(physicalDevice.GetDevice()->CheckFeatureSupport(
            D3D12_FEATURE_ROOT_SIGNATURE, &featureDataRootSignature,
            sizeof(featureDataRootSignature)))) {
        info.supportsRootSignatureVersion1_1 =
            featureDataRootSignature.HighestVersion >= D3D_ROOT_SIGNATURE_VERSION_1_1;
    }

    D3D12_FEATURE_DATA_EXISTING_HEAPS existingHeapInfo = {};
    if (SUCCEEDED(physicalDevice.GetDevice()->CheckFeatureSupport(
            D3D12_FEATURE_EXISTING_HEAPS, &existingHeapInfo, sizeof(existingHeapInfo)))) {
        info.supportsExistingHeap = existingHeapInfo.Supported;
    }

    D3D12_FEATURE_DATA_SHADER_MODEL knownShaderModels[] = {
        {D3D_SHADER_MODEL_6_6}, {D3D_SHADER_MODEL_6_5}, {D3D_SHADER_MODEL_6_4},
        {D3D_SHADER_MODEL_6_3}, {D3D_SHADER_MODEL_6_2}, {D3D_SHADER_MODEL_6_1},
        {D3D_SHADER_MODEL_6_0}, {D3D_SHADER_MODEL_5_1}};
    uint32_t driverShaderModel = 0;
    for (D3D12_FEATURE_DATA_SHADER_MODEL shaderModel : knownShaderModels) {
        if (SUCCEEDED(physicalDevice.GetDevice()->CheckFeatureSupport(
                D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))) {
            driverShaderModel = shaderModel.HighestShaderModel;
            break;
        }
    }

    if (driverShaderModel < D3D_SHADER_MODEL_5_1) {
        return DAWN_INTERNAL_ERROR("Driver doesn't support Shader Model 5.1 or higher");
    }

    // D3D_SHADER_MODEL is encoded as 0xMm with M the major version and m the minor version
    DAWN_ASSERT(driverShaderModel <= 0xFF);
    uint32_t shaderModelMajor = (driverShaderModel & 0xF0) >> 4;
    uint32_t shaderModelMinor = (driverShaderModel & 0xF);

    DAWN_ASSERT(shaderModelMajor < 10);
    DAWN_ASSERT(shaderModelMinor < 10);
    info.highestSupportedShaderModel = 10 * shaderModelMajor + shaderModelMinor;

    // Device support wave intrinsics if shader model >= SM6.0 and capabilities flag WaveOps is set.
    // https://github.com/Microsoft/DirectXShaderCompiler/wiki/Wave-Intrinsics
    if (driverShaderModel >= D3D_SHADER_MODEL_6_0) {
        D3D12_FEATURE_DATA_D3D12_OPTIONS1 featureOptions1 = {};
        if (SUCCEEDED(physicalDevice.GetDevice()->CheckFeatureSupport(
                D3D12_FEATURE_D3D12_OPTIONS1, &featureOptions1, sizeof(featureOptions1)))) {
            info.supportsWaveOps = featureOptions1.WaveOps;
            info.waveLaneCountMin = featureOptions1.WaveLaneCountMin;
            // Currently the WaveLaneCountMax queried from D3D12 API is not reliable and the meaning
            // is unclear. The result is recorded into D3D12DeviceInfo, but is not intended to be
            // used now.
            info.waveLaneCountMax = featureOptions1.WaveLaneCountMax;
        }
    }

    DXGI_ADAPTER_DESC adapterDesc;
    DAWN_TRY(CheckHRESULT(physicalDevice.GetHardwareAdapter()->GetDesc(&adapterDesc),
                          "IDXGIAdapter3::GetDesc"));
    info.dedicatedVideoMemory = adapterDesc.DedicatedVideoMemory;
    info.sharedSystemMemory = adapterDesc.SharedSystemMemory;

    return std::move(info);
}

}  // namespace dawn::native::d3d12
