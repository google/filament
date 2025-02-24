// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_D3D11_DEVICEINFOD3D11_H_
#define SRC_DAWN_NATIVE_D3D11_DEVICEINFOD3D11_H_

#include "dawn/native/Error.h"
#include "dawn/native/PerStage.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d11 {

class PhysicalDevice;

struct DeviceInfo {
    bool isUMA;

    // shaderModel indicates the maximum supported shader model, for example, the value 62
    // indicates that current driver supports the maximum shader model is shader model 6.2.
    uint32_t shaderModel;
    PerStage<std::wstring> shaderProfiles;
    bool supportsSharedResourceCapabilityTier2;
    bool supportsROV;
    size_t dedicatedVideoMemory;
    size_t sharedSystemMemory;
    bool supportsMonitoredFence;
    bool supportsNonMonitoredFence;
    bool supportsMapNoOverwriteDynamicBuffers;
    bool supportsPartialConstantBufferUpdate;
};

ResultOrError<DeviceInfo> GatherDeviceInfo(const ComPtr<IDXGIAdapter4>& adapter,
                                           const ComPtr<ID3D11Device>& device);

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_DEVICEINFOD3D11_H_
