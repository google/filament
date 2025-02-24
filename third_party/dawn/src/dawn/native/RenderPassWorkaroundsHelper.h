// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_RENDERPASSWORKAROUNDSHELPER_H_
#define SRC_DAWN_NATIVE_RENDERPASSWORKAROUNDSHELPER_H_

#include <functional>

#include "dawn/common/NonMovable.h"
#include "dawn/common/Ref.h"
#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/RenderPassEncoder.h"

namespace dawn::native {
struct BeginRenderPassCmd;
class CommandEncoder;
struct RenderPassDescriptor;
class RenderPassResourceUsageTracker;
class TextureBase;
class TextureViewBase;

// This class does several render pass' workarounds at various steps:
// - When post processing encoded BeginRenderPassCmd.
// - After render pass starts.
class RenderPassWorkaroundsHelper : NonMovable {
  public:
    RenderPassWorkaroundsHelper();
    ~RenderPassWorkaroundsHelper();

    MaybeError Initialize(CommandEncoder* encoder,
                          const UnpackedPtr<RenderPassDescriptor>& renderPassDescriptor);
    MaybeError ApplyOnPostEncoding(CommandEncoder* encoder,
                                   const UnpackedPtr<RenderPassDescriptor>& renderPassDescriptor,
                                   RenderPassResourceUsageTracker* usageTracker,
                                   BeginRenderPassCmd* cmd,
                                   RenderPassEncoder::EndCallback* passEndCallbackOut);

    MaybeError ApplyOnRenderPassStart(RenderPassEncoder* rpEncoder,
                                      const UnpackedPtr<RenderPassDescriptor>& rpDesc);

  private:
    struct TextureAndView {
        Ref<TextureBase> texture;
        Ref<TextureViewBase> view;
    };
    // Temporary resolve targets to replace the original resolve targets.
    PerColorAttachment<TextureAndView> mTempResolveTargets;
    ColorAttachmentMask mTempResolveTargetsMask;

    // Should we apply ExpandResolveTexture step?
    bool mShouldApplyExpandResolveEmulation = false;
};

}  // namespace dawn::native

#endif
