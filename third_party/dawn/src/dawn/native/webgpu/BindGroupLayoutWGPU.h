// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_WEBGPU_BINDGROUPLAYOUTWGPU_H_
#define SRC_DAWN_NATIVE_WEBGPU_BINDGROUPLAYOUTWGPU_H_

#include "dawn/common/MutexProtected.h"
#include "dawn/common/SlabAllocator.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/webgpu/BindGroupWGPU.h"
#include "dawn/native/webgpu/ObjectWGPU.h"
#include "dawn/native/webgpu/RecordableObject.h"

namespace dawn::native::webgpu {

class BindGroupLayout final : public BindGroupLayoutInternalBase,
                              public RecordableObject,
                              public ObjectWGPU<WGPUBindGroupLayout> {
  public:
    static ResultOrError<Ref<BindGroupLayout>> Create(
        Device* device,
        const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor);

    Ref<BindGroup> AllocateBindGroup(const UnpackedPtr<BindGroupDescriptor>& descriptor);
    void DeallocateBindGroup(BindGroup* bindGroup);
    void ReduceMemoryUsage() override;

    MaybeError AddReferenced(CaptureContext& captureContext) override;
    MaybeError CaptureCreationParameters(CaptureContext& context) override;

  private:
    BindGroupLayout(Device* device, const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor);
    ~BindGroupLayout() override = default;
    void SetLabelImpl() override;

    MutexProtected<SlabAllocator<BindGroup>> mBindGroupAllocator;
};

}  // namespace dawn::native::webgpu

#endif  // SRC_DAWN_NATIVE_WEBGPU_BINDGROUPLAYOUTWGPU_H_
