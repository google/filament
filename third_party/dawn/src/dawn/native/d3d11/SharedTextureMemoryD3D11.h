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

#ifndef SRC_DAWN_NATIVE_D3D11_SHARED_TEXTURE_MEMORY_D3D11_H_
#define SRC_DAWN_NATIVE_D3D11_SHARED_TEXTURE_MEMORY_D3D11_H_

#include "dawn/native/Error.h"
#include "dawn/native/d3d/SharedTextureMemoryD3D.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native {
namespace d3d {
class KeyedMutex;
}  // namespace d3d

namespace d3d11 {
class Device;
struct SharedTextureMemoryD3D11Texture2DDescriptor;

class SharedTextureMemory final : public d3d::SharedTextureMemory {
  public:
    static ResultOrError<Ref<SharedTextureMemory>> Create(
        Device* device,
        StringView label,
        const SharedTextureMemoryDXGISharedHandleDescriptor* descriptor);

    static ResultOrError<Ref<SharedTextureMemory>> Create(
        Device* device,
        StringView label,
        const SharedTextureMemoryD3D11Texture2DDescriptor* descriptor);

    ID3D11Resource* GetD3DResource() const;

    d3d::KeyedMutex* GetKeyedMutex() const;

  private:
    SharedTextureMemory(Device* device,
                        StringView label,
                        SharedTextureMemoryProperties properties,
                        ComPtr<ID3D11Resource> resource);

    void DestroyImpl() override;

    ResultOrError<Ref<TextureBase>> CreateTextureImpl(
        const UnpackedPtr<TextureDescriptor>& descriptor) override;

    ComPtr<ID3D11Resource> mResource;
    Ref<d3d::KeyedMutex> mKeyedMutex;
};
}  // namespace d3d11
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_D3D11_SHARED_TEXTURE_MEMORY_D3D11_H_
