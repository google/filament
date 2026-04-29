// Copyright 2026 The Dawn & Tint Authors
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

#ifndef INCLUDE_DAWN_REPLAY_BLITBUFFERTODEPTHTEXTURE_H_
#define INCLUDE_DAWN_REPLAY_BLITBUFFERTODEPTHTEXTURE_H_

#include <unordered_map>

#include "dawn/replay/Error.h"
#include "dawn/webgpu_cpp.h"

namespace dawn::replay {

class BlitBufferToDepthTexture {
  public:
    MaybeError Blit(wgpu::Device device,
                    const wgpu::TexelCopyTextureInfo& dst,
                    void const* data,
                    size_t dataSize,
                    const wgpu::TexelCopyBufferLayout& src,
                    const wgpu::Extent3D& copyExtent);

  private:
    std::unordered_map<wgpu::TextureFormat, wgpu::RenderPipeline> mPipelines;
};

}  // namespace dawn::replay

#endif  // INCLUDE_DAWN_REPLAY_BLITBUFFERTODEPTHTEXTURE_H_
