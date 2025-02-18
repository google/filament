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

#ifndef SRC_DAWN_NATIVE_SUBRESOURCE_H_
#define SRC_DAWN_NATIVE_SUBRESOURCE_H_

#include "dawn/native/EnumClassBitmasks.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

// Note: Subresource indices are computed by iterating the aspects in increasing order.
// D3D12 uses these directly, so the order must match D3D12's indices.
//  - Depth/Stencil textures have Depth as Plane 0, and Stencil as Plane 1.
enum class Aspect : uint8_t {
    None = 0x0,
    Color = 0x1,
    Depth = 0x2,
    Stencil = 0x4,

    // Aspects used to select individual planes in a multi-planar format.
    Plane0 = 0x8,
    Plane1 = 0x10,
    Plane2 = 0x20,

    // An aspect for that represents the combination of both the depth and stencil aspects. It
    // can be ignored outside of the Vulkan backend.
    CombinedDepthStencil = 0x40,
};

template <>
struct EnumBitmaskSize<Aspect> {
    static constexpr unsigned value = 7;
};

// Convert the TextureAspect to an Aspect mask for the format. ASSERTs if the aspect
// does not exist in the format.
// Also ASSERTs if "All" is selected and results in more than one aspect.
Aspect ConvertSingleAspect(const Format& format, wgpu::TextureAspect aspect);

// Convert the TextureAspect to an Aspect mask for the format. ASSERTs if the aspect
// does not exist in the format.
Aspect ConvertAspect(const Format& format, wgpu::TextureAspect aspect);

// Returns the Aspects of the Format that are selected by the wgpu::TextureAspect.
// Note that this can return Aspect::None if the Format doesn't have any of the
// selected aspects.
Aspect SelectFormatAspects(const Format& format, wgpu::TextureAspect aspect);

// Convert TextureAspect to the aspect which corresponds to the view format. This
// special cases per plane view formats before calling ConvertAspect.
Aspect ConvertViewAspect(const Format& format, wgpu::TextureAspect aspect);

// Return aspect which corresponds to a plane.
Aspect GetPlaneAspect(const Format& format, uint32_t planeIndex);

// Helper struct to make it clear that what the parameters of a range mean.
template <typename T>
struct FirstAndCountRange {
    T first;
    T count;
};

struct SubresourceRange {
    SubresourceRange(Aspect aspects,
                     FirstAndCountRange<uint32_t> arrayLayerParam,
                     FirstAndCountRange<uint32_t> mipLevelParams);
    SubresourceRange();

    Aspect aspects;
    uint32_t baseArrayLayer;
    uint32_t layerCount;
    uint32_t baseMipLevel;
    uint32_t levelCount;

    static SubresourceRange SingleMipAndLayer(uint32_t baseMipLevel,
                                              uint32_t baseArrayLayer,
                                              Aspect aspects);
    static SubresourceRange MakeSingle(Aspect aspect,
                                       uint32_t baseArrayLayer,
                                       uint32_t baseMipLevel);

    static SubresourceRange MakeFull(Aspect aspects, uint32_t layerCount, uint32_t levelCount);
};

// Helper function to use aspects as linear indices in arrays.
uint8_t GetAspectIndex(Aspect aspect);
uint8_t GetAspectCount(Aspect aspects);

// The maximum number of planes per format Dawn knows about. Asserts in BuildFormatTable that
// the per plane index does not exceed the known maximum plane count.
static constexpr uint32_t kMaxPlanesPerFormat = 3;

}  // namespace dawn::native

template <>
struct wgpu::IsWGPUBitmask<dawn::native::Aspect> {
    static constexpr bool enable = true;
};

#endif  // SRC_DAWN_NATIVE_SUBRESOURCE_H_
