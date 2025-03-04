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

#ifndef SRC_DAWN_NATIVE_DAWN_PLATFORM_H_
#define SRC_DAWN_NATIVE_DAWN_PLATFORM_H_

// Use webgpu_cpp to have the enum and bitfield definitions
#include <webgpu/webgpu_cpp.h>

#include "dawn/native/dawn_platform_autogen.h"

namespace dawn::native {

// kEnumCount is a constant specifying the number of enums in a WebGPU enum type,
// if the enums are contiguous, making it suitable for iteration.
// It is defined in dawn_platform_autogen.h
template <typename T>
constexpr uint32_t kEnumCount = EnumCount<T>::value;

// Extra buffer usages
// Add an extra buffer usage and an extra binding type for binding the buffers with QueryResolve
// usage as storage buffer in the internal pipeline.
static constexpr wgpu::BufferUsage kInternalStorageBuffer =
    static_cast<wgpu::BufferUsage>(1u << 31);

// Add an extra buffer usage (readonly storage buffer usage) for render pass resource tracking
static constexpr wgpu::BufferUsage kReadOnlyStorageBuffer =
    static_cast<wgpu::BufferUsage>(1u << 30);

// Add an extra buffer usage (copy-src buffer usage) that can be combined with MapRead
static constexpr wgpu::BufferUsage kInternalCopySrcBuffer =
    static_cast<wgpu::BufferUsage>(1u << 29);

// Add an extra buffer usage (indirect-for-backend-resource-tracking) for backend resource
// tracking. We won't do buffer usage transitions when the new buffer usage only contains
// wgpu::BufferUsage::Indirect and doesn't contain kInternalIndirectBufferForBackendResourceTracking
// on the backends.
static constexpr wgpu::BufferUsage kIndirectBufferForBackendResourceTracking =
    static_cast<wgpu::BufferUsage>(1u << 28);

// Define `kIndirectBufferForFrontendValidation` as an alias of wgpu::BufferUsage::Indirect just
// for front-end validation on the indirect buffer usage.
static constexpr wgpu::BufferUsage kIndirectBufferForFrontendValidation =
    wgpu::BufferUsage::Indirect;

// Extra texture usages
// Usage to denote an extra tag value used in system specific ways.
//  - Used to store that attachments are used more than once in PassResourceUsageTracker.
//  - Used to store mixed read-only vs. not depth-stencil layouts in Vulkan.
static constexpr wgpu::TextureUsage kReservedTextureUsage =
    static_cast<wgpu::TextureUsage>(1u << 31);

// Extra texture usages for textures that are used with the presentation engine.
// Acquire is that Dawn is acquiring the texture from the presentation engine while Release is Dawn
// releasing is to the presentation engine.
static constexpr wgpu::TextureUsage kPresentAcquireTextureUsage =
    static_cast<wgpu::TextureUsage>(1u << 30);
static constexpr wgpu::TextureUsage kPresentReleaseTextureUsage =
    static_cast<wgpu::TextureUsage>(1u << 29);

// Add an extra texture usage (readonly render attachment usage) for render pass resource
// tracking
static constexpr wgpu::TextureUsage kReadOnlyRenderAttachment =
    static_cast<wgpu::TextureUsage>(1u << 28);

// Add an extra texture usage (readonly storage texture usage) for resource tracking
static constexpr wgpu::TextureUsage kReadOnlyStorageTexture =
    static_cast<wgpu::TextureUsage>(1u << 27);

// Add an extra texture usage (writeonly storage texture usage) for resource tracking
static constexpr wgpu::TextureUsage kWriteOnlyStorageTexture =
    static_cast<wgpu::TextureUsage>(1u << 26);

// Add an extra texture usage (load resolve texture to MSAA) for render pass resource tracking
static constexpr wgpu::TextureUsage kResolveAttachmentLoadingUsage =
    static_cast<wgpu::TextureUsage>(1u << 25);

// Extra BufferBindingType for internal storage buffer binding.
static constexpr wgpu::BufferBindingType kInternalStorageBufferBinding =
    static_cast<wgpu::BufferBindingType>(~0u);

// Extra TextureSampleType for sampling from a resolve attachment.
static constexpr wgpu::TextureSampleType kInternalResolveAttachmentSampleType =
    static_cast<wgpu::TextureSampleType>(~0u);

// Extra TextureViewDimension for input attachment.
static constexpr wgpu::TextureViewDimension kInternalInputAttachmentDim =
    static_cast<wgpu::TextureViewDimension>(~0u);

static constexpr uint32_t kEnumPrefixMask = 0xFFFF'0000;
static constexpr uint32_t kDawnEnumPrefix = 0x0005'0000;

struct Rect2D {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_DAWN_PLATFORM_H_
