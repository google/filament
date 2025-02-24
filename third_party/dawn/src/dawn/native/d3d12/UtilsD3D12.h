// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_D3D12_UTILSD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_UTILSD3D12_H_

#include <string>

#include "dawn/native/Commands.h"
#include "dawn/native/d3d/UtilsD3D.h"
#include "dawn/native/d3d12/BufferD3D12.h"
#include "dawn/native/d3d12/ResourceAllocatorManagerD3D12.h"
#include "dawn/native/d3d12/TextureD3D12.h"
#include "dawn/native/d3d12/d3d12_platform.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native::d3d12 {

D3D12_COMPARISON_FUNC ToD3D12ComparisonFunc(wgpu::CompareFunction func);

D3D12_SHADER_VISIBILITY ShaderVisibilityType(wgpu::ShaderStage visibility);

D3D12_TEXTURE_COPY_LOCATION ComputeTextureCopyLocationForTexture(const Texture* texture,
                                                                 uint32_t level,
                                                                 uint32_t layer,
                                                                 Aspect aspect);

D3D12_BOX ComputeD3D12BoxFromOffsetAndSize(const Origin3D& offset, const Extent3D& copySize);

enum class BufferTextureCopyDirection {
    B2T,
    T2B,
};

void RecordBufferTextureCopyWithBufferHandle(BufferTextureCopyDirection direction,
                                             ID3D12GraphicsCommandList* commandList,
                                             ID3D12Resource* bufferResource,
                                             const uint64_t offset,
                                             const uint32_t bytesPerRow,
                                             const uint32_t rowsPerImage,
                                             const TextureCopy& textureCopy,
                                             const Extent3D& copySize);

void RecordBufferTextureCopy(BufferTextureCopyDirection direction,
                             ID3D12GraphicsCommandList* commandList,
                             const BufferCopy& bufferCopy,
                             const TextureCopy& textureCopy,
                             const Extent3D& copySize);

void SetDebugName(Device* device, ID3D12Object* object, const char* prefix, std::string label = "");

constexpr DXGI_FORMAT GetNullRTVDXGIFormatForD3D12RenderPass() {
    return DXGI_FORMAT_R8G8B8A8_UNORM;
}

D3D12_HEAP_TYPE GetD3D12HeapType(ResourceHeapKind resourceHeapKind);

D3D12_HEAP_PROPERTIES GetD3D12HeapProperties(ResourceHeapKind resourceHeapKind);

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_UTILSD3D12_H_
