// Copyright 2018 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_BACKENDCONNECTION_H_
#define SRC_DAWN_NATIVE_BACKENDCONNECTION_H_

#include <memory>
#include <vector>

#include "dawn/common/NonMovable.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/Toggles.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

// An common interface for all backends. Mostly used to create adapters for a particular
// backend.
class BackendConnection : NonMovable {
  public:
    BackendConnection(InstanceBase* instance, wgpu::BackendType type);
    virtual ~BackendConnection() = default;

    wgpu::BackendType GetType() const;
    InstanceBase* GetInstance() const;

    // Returns physical devices capable of supporting the `options`.
    // Calling this multiple times in succession should return a vector with duplicate
    // references to the same PhysicalDevices (i.e. the backend should cache them).
    virtual std::vector<Ref<PhysicalDeviceBase>> DiscoverPhysicalDevices(
        const UnpackedPtr<RequestAdapterOptions>& options) = 0;

  private:
    raw_ptr<InstanceBase> mInstance = nullptr;
    wgpu::BackendType mType;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BACKENDCONNECTION_H_
