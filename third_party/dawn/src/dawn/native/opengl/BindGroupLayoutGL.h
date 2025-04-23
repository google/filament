// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_OPENGL_BINDGROUPLAYOUTGL_H_
#define SRC_DAWN_NATIVE_OPENGL_BINDGROUPLAYOUTGL_H_

#include "dawn/common/MutexProtected.h"
#include "dawn/common/SlabAllocator.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/opengl/BindGroupGL.h"

namespace dawn::native::opengl {

class Device;

class BindGroupLayout final : public BindGroupLayoutInternalBase {
  public:
    BindGroupLayout(DeviceBase* device, const BindGroupLayoutDescriptor* descriptor);

    Ref<BindGroup> AllocateBindGroup(Device* device, const BindGroupDescriptor* descriptor);
    void DeallocateBindGroup(BindGroup* bindGroup);
    void ReduceMemoryUsage() override;

  private:
    ~BindGroupLayout() override = default;
    MutexProtected<SlabAllocator<BindGroup>> mBindGroupAllocator;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_BINDGROUPLAYOUTGL_H_
