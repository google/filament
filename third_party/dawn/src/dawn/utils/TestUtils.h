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

#ifndef SRC_DAWN_UTILS_TESTUTILS_H_
#define SRC_DAWN_UTILS_TESTUTILS_H_

#include <webgpu/webgpu_cpp.h>

#include <functional>
#include <ostream>

#include "dawn/common/Constants.h"

namespace dawn::utils {

struct RGBA8 {
    constexpr RGBA8() : RGBA8(0, 0, 0, 0) {}
    constexpr RGBA8(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}
    bool operator==(const RGBA8& other) const;
    bool operator!=(const RGBA8& other) const;
    bool operator<=(const RGBA8& other) const;
    bool operator>=(const RGBA8& other) const;

    uint8_t r, g, b, a;

    static const RGBA8 kZero;
    static const RGBA8 kBlack;
    static const RGBA8 kRed;
    static const RGBA8 kGreen;
    static const RGBA8 kBlue;
    static const RGBA8 kYellow;
    static const RGBA8 kWhite;
};
std::ostream& operator<<(std::ostream& stream, const RGBA8& color);

struct TextureDataCopyLayout {
    uint64_t byteLength;
    uint64_t texelBlockCount;
    uint32_t bytesPerRow;
    uint32_t rowsPerImage;
    uint32_t texelBlocksPerRow;
    uint32_t bytesPerImage;
    uint32_t texelBlocksPerImage;
    wgpu::Extent3D mipSize;
};

uint32_t GetMinimumBytesPerRow(wgpu::TextureFormat format,
                               uint32_t width,
                               uint32_t textureBytesPerRowAlignment = kTextureBytesPerRowAlignment);
TextureDataCopyLayout GetTextureDataCopyLayoutForTextureAtLevel(
    wgpu::TextureFormat format,
    wgpu::Extent3D textureSizeAtLevel0,
    uint32_t mipmapLevel,
    wgpu::TextureDimension dimension = wgpu::TextureDimension::e2D,
    uint32_t rowsPerImage = wgpu::kCopyStrideUndefined);

uint64_t RequiredBytesInCopy(uint64_t bytesPerRow,
                             uint64_t rowsPerImage,
                             wgpu::Extent3D copyExtent,
                             wgpu::TextureFormat textureFormat);
uint64_t RequiredBytesInCopy(uint64_t bytesPerRow,
                             uint64_t rowsPerImage,
                             uint64_t widthInBlocks,
                             uint64_t heightInBlocks,
                             uint64_t depth,
                             uint64_t bytesPerBlock);

uint64_t GetTexelCountInCopyRegion(uint64_t bytesPerRow,
                                   uint64_t rowsPerImage,
                                   wgpu::Extent3D copyExtent,
                                   wgpu::TextureFormat textureFormat);

// A helper function used for testing DynamicUploader offset alignment.
// A call of this function will do a Queue::WriteTexture with 1 byte of data,
// so that assuming that WriteTexture uses DynamicUploader, the first RingBuffer
// in it will contain 1 byte of data.
void UnalignDynamicUploader(wgpu::Device device);

uint32_t VertexFormatSize(wgpu::VertexFormat format);

void RunInParallel(uint32_t numThreads,
                   const std::function<void(uint32_t)>& workerFunc,
                   const std::function<void()>& mainThreadFunc = nullptr);

}  // namespace dawn::utils

#endif  // SRC_DAWN_UTILS_TESTUTILS_H_
