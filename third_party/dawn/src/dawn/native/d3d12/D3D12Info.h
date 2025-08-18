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

#ifndef SRC_DAWN_NATIVE_D3D12_D3D12INFO_H_
#define SRC_DAWN_NATIVE_D3D12_D3D12INFO_H_

#include "dawn/native/Error.h"
#include "dawn/native/PerStage.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class PhysicalDevice;

struct D3D12DeviceInfo {
    bool isUMA;
    bool isCacheCoherentUMA;
    uint32_t resourceHeapTier;
    bool supportsRenderPass;
    // Whether the device support native 16bit shader ops, required for shader f16 feature. Note
    // that the feature also requires using shader model version >= 6.2.
    bool supportsNative16BitShaderOps;
    // highestSupportedShaderModel indicates the maximum supported shader model, for example, the
    // value 62 indicates that current driver supports the maximum shader model is shader model 6.2.
    uint32_t highestSupportedShaderModel;
    bool supportsSharedResourceCapabilityTier1;
    bool supportsCastingFullyTypedFormat;
    uint32_t programmableSamplePositionsTier;
    bool supportsRootSignatureVersion1_1;
    bool use64KBAlignedMSAATexture;
    bool supportsHeapFlagCreateNotZeroed;
    bool supportsTextureCopyBetweenDimensions;
    bool supportsUnrestrictedBufferTextureCopyPitch;
    // Whether the device support wave intrinsics
    bool supportsWaveOps;
    bool supportsExistingHeap;
    uint32_t waveLaneCountMin;
    // Currently the WaveLaneCountMax queried from D3D12 API is not reliable and the meaning is
    // unclear. Reference:
    // https://github.com/Microsoft/DirectXShaderCompiler/wiki/Wave-Intrinsics#:~:text=UINT%20WaveLaneCountMax
    uint32_t waveLaneCountMax;
    size_t dedicatedVideoMemory;
    size_t sharedSystemMemory;
};

ResultOrError<D3D12DeviceInfo> GatherDeviceInfo(const PhysicalDevice& physicalDevice);
}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_D3D12INFO_H_
