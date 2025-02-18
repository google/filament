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

#ifndef SRC_DAWN_NATIVE_D3D_BACKENDD3D_H_
#define SRC_DAWN_NATIVE_D3D_BACKENDD3D_H_

#include <memory>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/TypedInteger.h"
#include "dawn/native/BackendConnection.h"

#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d {

class PlatformFunctions;

class Backend : public BackendConnection {
  public:
    Backend(InstanceBase* instance, wgpu::BackendType type);

    MaybeError Initialize(std::unique_ptr<PlatformFunctions> functions);

    IDXGIFactory4* GetFactory() const;

    const PlatformFunctions* GetFunctions() const;

    std::vector<Ref<PhysicalDeviceBase>> DiscoverPhysicalDevices(
        const UnpackedPtr<RequestAdapterOptions>& options) override;

  protected:
    virtual ResultOrError<Ref<PhysicalDeviceBase>> CreatePhysicalDeviceFromIDXGIAdapter(
        ComPtr<IDXGIAdapter> dxgiAdapter) = 0;

  private:
    ResultOrError<Ref<PhysicalDeviceBase>> GetOrCreatePhysicalDeviceFromLUID(LUID luid);
    ResultOrError<Ref<PhysicalDeviceBase>> GetOrCreatePhysicalDeviceFromIDXGIAdapter(
        ComPtr<IDXGIAdapter> dxgiAdapter);

    // Keep mFunctions as the first member so that in the destructor it is freed last. Otherwise
    // the D3D12 DLLs are unloaded before we are done using them.
    std::unique_ptr<PlatformFunctions> mFunctions;
    ComPtr<IDXGIFactory4> mFactory;

    struct LUIDHashFunc {
        size_t operator()(const LUID& luid) const;
    };
    struct LUIDEqualFunc {
        bool operator()(const LUID& a, const LUID& b) const;
    };

    // Map of LUID to physical device.
    // The LUID is guaranteed to be uniquely identify an adapter on the local
    // machine until restart.
    absl::flat_hash_map<LUID, Ref<PhysicalDeviceBase>, LUIDHashFunc, LUIDEqualFunc>
        mPhysicalDevices;
};

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_BACKENDD3D_H_
