// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_APPLYCLEARVALUEWITHDRAWHELPER_H_
#define SRC_DAWN_NATIVE_APPLYCLEARVALUEWITHDRAWHELPER_H_

#include <vector>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/Ref.h"
#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"

namespace dawn::native {
class BufferBase;
class CommandEncoder;
class RenderPassEncoder;
struct RenderPassDescriptor;

struct KeyOfApplyClearColorValueWithDrawPipelines {
    uint8_t colorAttachmentCount = 0;
    PerColorAttachment<wgpu::TextureFormat> colorTargetFormats;
    ColorAttachmentMask colorTargetsToApplyClearColorValue;
    uint32_t sampleCount = 0;
    wgpu::TextureFormat depthStencilFormat = wgpu::TextureFormat::Undefined;
    bool hasPLS = false;
    uint64_t totalPixelLocalStorageSize;
    std::vector<wgpu::PipelineLayoutStorageAttachment> plsAttachments;
};

struct KeyOfApplyClearColorValueWithDrawPipelinesHashFunc {
    size_t operator()(KeyOfApplyClearColorValueWithDrawPipelines key) const;
};
struct KeyOfApplyClearColorValueWithDrawPipelinesEqualityFunc {
    bool operator()(KeyOfApplyClearColorValueWithDrawPipelines key1,
                    const KeyOfApplyClearColorValueWithDrawPipelines key2) const;
};
using ApplyClearColorValueWithDrawPipelinesCache =
    absl::flat_hash_map<KeyOfApplyClearColorValueWithDrawPipelines,
                        Ref<RenderPipelineBase>,
                        KeyOfApplyClearColorValueWithDrawPipelinesHashFunc,
                        KeyOfApplyClearColorValueWithDrawPipelinesEqualityFunc>;

class ClearWithDrawHelper {
  public:
    ClearWithDrawHelper();
    ~ClearWithDrawHelper();

    MaybeError Initialize(CommandEncoder* encoder,
                          const UnpackedPtr<RenderPassDescriptor>& renderPassDescriptor);
    MaybeError Apply(RenderPassEncoder* renderPassEncoder);

    // Get the mask indicating the color attachments in the render pass that the workaround applies.
    static ColorAttachmentMask GetAppliedColorAttachments(const DeviceBase* device,
                                                          BeginRenderPassCmd* renderPass);

  private:
    bool mShouldRun = false;
    KeyOfApplyClearColorValueWithDrawPipelines mKey;
    Ref<BufferBase> mUniformBufferWithClearColorValues;
};

}  // namespace dawn::native

#endif
