// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_METAL_PIPELINELAYOUTMTL_H_
#define SRC_DAWN_NATIVE_METAL_PIPELINELAYOUTMTL_H_

#import <Metal/Metal.h>

#include "dawn/common/ityp_stack_vec.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/PerStage.h"
#include "dawn/native/PipelineLayout.h"

namespace dawn::native::metal {

class Device;

// The number of Metal buffers usable by applications in general
inline constexpr size_t kMetalBufferTableSize = 31;
// The Metal buffer slot that Dawn reserves for immediate block.
// The layout of ImmediateBlock:
// struct ImmediateBlock {
//    - Normal render/compute immediates, ref to ImmediateConstantLayout.h
//    - Optional Paddings to align the following vec4 to 16 bytes
//    - Storage Buffer sizes - vec4<u32> arrays
// };
inline constexpr size_t kImmediateBlockBufferSlot = kMetalBufferTableSize - 1;
// The number of Metal buffers Dawn can use in a generic way (i.e. that aren't reserved)
inline constexpr size_t kGenericMetalBufferSlots = kMetalBufferTableSize - 1;

// The Last buffer slot to be used by argument buffers
inline constexpr size_t kArgumentBufferSlotMax = kImmediateBlockBufferSlot - 1;

inline constexpr BindGroupIndex kPullingBufferBindingSet = BindGroupIndex(kMaxBindGroups);

class PipelineLayout final : public PipelineLayoutBase {
  public:
    static Ref<PipelineLayout> Create(Device* device,
                                      const UnpackedPtr<PipelineLayoutDescriptor>& descriptor);

    using BindingIndexInfo =
        PerBindGroup<ityp::stack_vec<BindingIndex, uint32_t, kMaxOptimalBindingsPerGroup>>;
    const BindingIndexInfo& GetBindingIndexInfo(SingleShaderStage stage) const;

    // The number of Metal vertex stage buffers used for the whole pipeline layout.
    uint32_t GetBufferBindingCount(SingleShaderStage stage) const;

  private:
    PipelineLayout(Device* device, const UnpackedPtr<PipelineLayoutDescriptor>& descriptor);
    ~PipelineLayout() override;
    PerStage<BindingIndexInfo> mIndexInfo;
    PerStage<uint32_t> mBufferBindingCount;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_PIPELINELAYOUTMTL_H_
