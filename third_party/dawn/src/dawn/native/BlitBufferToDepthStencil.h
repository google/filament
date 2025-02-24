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

#ifndef SRC_DAWN_NATIVE_BLITBUFFERTODEPTHSTENCIL_H_
#define SRC_DAWN_NATIVE_BLITBUFFERTODEPTHSTENCIL_H_

#include "dawn/native/Error.h"

namespace dawn::native {

struct TextureCopy;

// BlitBufferToDepth works around issues where copying from a buffer
// to depth does not work on some drivers.
// Currently, only depth16unorm textures can be CopyDst, so only depth16unorm
// is supported.
// It does the following:
//  - Copies buffer data to an rg8uint texture.
//  - Sets the viewport to the copy rect.
//  - Uploads the copy origin to a uniform buffer.
//  - For each destination layer:
//    - Performs a draw to sample the rg8uint data, computes the
//      floating point depth value, and writes the frag depth.

MaybeError BlitStagingBufferToDepth(DeviceBase* device,
                                    BufferBase* buffer,
                                    const TexelCopyBufferLayout& src,
                                    const TextureCopy& dst,
                                    const Extent3D& copyExtent);

MaybeError BlitBufferToDepth(DeviceBase* device,
                             CommandEncoder* commandEncoder,
                             BufferBase* buffer,
                             const TexelCopyBufferLayout& src,
                             const TextureCopy& dst,
                             const Extent3D& copyExtent);

// BlitR8ToStencil works around issues where upload data to stencil aspect
// is not supported, by copying data from an r8uint texture to dst texture.
// It does the following:
//  - Sets the viewport to the copy rect.
//  - Uploads the copy origin to a uniform buffer.
//  - For each destination layer:
//    - Performs a draw to clear stencil to 0.
//    - Performs 8 draws for each bit of stencil to set the respective
//      stencil bit to 1, if the source r8 texture also has that bit set.
//      If the source r8 texture does not, the fragment is discarded.

MaybeError BlitR8ToStencil(DeviceBase* device,
                           CommandEncoder* commandEncoder,
                           TextureBase* dataTexture,
                           const TextureCopy& dst,
                           const Extent3D& copyExtent);

// BlitBufferToStencil works around issues where copying from a buffer
// to stencil does not work on some drivers.
// It does the following:
//  - Copies buffer data to an r8uint texture.
//  - Calls BlitR8ToStencil.

MaybeError BlitStagingBufferToStencil(DeviceBase* device,
                                      BufferBase* buffer,
                                      const TexelCopyBufferLayout& src,
                                      const TextureCopy& dst,
                                      const Extent3D& copyExtent);

MaybeError BlitBufferToStencil(DeviceBase* device,
                               CommandEncoder* commandEncoder,
                               BufferBase* buffer,
                               const TexelCopyBufferLayout& src,
                               const TextureCopy& dst,
                               const Extent3D& copyExtent);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BLITBUFFERTODEPTHSTENCIL_H_
