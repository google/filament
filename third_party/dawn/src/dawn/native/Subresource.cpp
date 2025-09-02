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

#include "dawn/native/Subresource.h"

#include <bit>

#include "dawn/common/Assert.h"
#include "dawn/common/Math.h"
#include "dawn/native/Format.h"

namespace dawn::native {

Aspect ConvertSingleAspect(const Format& format, wgpu::TextureAspect aspect) {
    Aspect aspectMask = ConvertAspect(format, aspect);
    DAWN_ASSERT(HasOneBit(aspectMask));
    return aspectMask;
}

Aspect ConvertAspect(const Format& format, wgpu::TextureAspect aspect) {
    Aspect aspectMask = SelectFormatAspects(format, aspect);
    DAWN_ASSERT(aspectMask != Aspect::None);
    return aspectMask;
}

Aspect ConvertViewAspect(const Format& format, wgpu::TextureAspect aspect) {
    // Color view |format| must be treated as the same plane |aspect|.
    if (format.aspects == Aspect::Color) {
        switch (aspect) {
            case wgpu::TextureAspect::Plane0Only:
                return Aspect::Plane0;
            case wgpu::TextureAspect::Plane1Only:
                return Aspect::Plane1;
            case wgpu::TextureAspect::Plane2Only:
                return Aspect::Plane2;
            default:
                break;
        }
    }
    return ConvertAspect(format, aspect);
}

Aspect GetPlaneAspect(const Format& format, uint32_t planeIndex) {
    wgpu::TextureAspect textureAspect;
    switch (planeIndex) {
        case 0:
            textureAspect = wgpu::TextureAspect::Plane0Only;
            break;
        case 1:
            textureAspect = wgpu::TextureAspect::Plane1Only;
            break;
        case 2:
            textureAspect = wgpu::TextureAspect::Plane2Only;
            break;
        default:
            DAWN_UNREACHABLE();
    }

    return ConvertAspect(format, textureAspect);
}

Aspect SelectFormatAspects(const Format& format, wgpu::TextureAspect aspect) {
    switch (aspect) {
        case wgpu::TextureAspect::All:
            return format.aspects;
        case wgpu::TextureAspect::DepthOnly:
            return format.aspects & Aspect::Depth;
        case wgpu::TextureAspect::StencilOnly:
            return format.aspects & Aspect::Stencil;
        case wgpu::TextureAspect::Plane0Only:
            return format.aspects & Aspect::Plane0;
        case wgpu::TextureAspect::Plane1Only:
            return format.aspects & Aspect::Plane1;
        case wgpu::TextureAspect::Plane2Only:
            return format.aspects & Aspect::Plane2;
        case wgpu::TextureAspect::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

uint8_t GetAspectIndex(Aspect aspect) {
    DAWN_ASSERT(HasOneBit(aspect));
    switch (aspect) {
        case Aspect::Color:
        case Aspect::Depth:
        case Aspect::Plane0:
        case Aspect::CombinedDepthStencil:
            return 0;
        case Aspect::Plane1:
        case Aspect::Stencil:
            return 1;
        case Aspect::Plane2:
            return 2;
        default:
            DAWN_UNREACHABLE();
    }
}

uint8_t GetAspectCount(Aspect aspects) {
    if (aspects == Aspect::Stencil) {
        // Fake a the existence of a depth aspect so that the stencil data stays at index 1.
        DAWN_ASSERT(GetAspectIndex(Aspect::Stencil) == 1);
        return 2;
    }
    return static_cast<uint8_t>(std::popcount(static_cast<uint8_t>(aspects)));
}

SubresourceRange::SubresourceRange(Aspect aspects,
                                   FirstAndCountRange<uint32_t> arrayLayerParam,
                                   FirstAndCountRange<uint32_t> mipLevelParams)
    : aspects(aspects),
      baseArrayLayer(arrayLayerParam.first),
      layerCount(arrayLayerParam.count),
      baseMipLevel(mipLevelParams.first),
      levelCount(mipLevelParams.count) {}

SubresourceRange::SubresourceRange()
    : aspects(Aspect::None), baseArrayLayer(0), layerCount(0), baseMipLevel(0), levelCount(0) {}

// static
SubresourceRange SubresourceRange::SingleMipAndLayer(uint32_t baseMipLevel,
                                                     uint32_t baseArrayLayer,
                                                     Aspect aspects) {
    return {aspects, {baseArrayLayer, 1}, {baseMipLevel, 1}};
}

// static
SubresourceRange SubresourceRange::MakeSingle(Aspect aspect,
                                              uint32_t baseArrayLayer,
                                              uint32_t baseMipLevel) {
    DAWN_ASSERT(HasOneBit(aspect));
    return {aspect, {baseArrayLayer, 1}, {baseMipLevel, 1}};
}

// static
SubresourceRange SubresourceRange::MakeFull(Aspect aspects,
                                            uint32_t layerCount,
                                            uint32_t levelCount) {
    return {aspects, {0, layerCount}, {0, levelCount}};
}

}  // namespace dawn::native
