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

#ifndef SRC_DAWN_NATIVE_D3D11_TEXTURED3D11_H_
#define SRC_DAWN_NATIVE_D3D11_TEXTURED3D11_H_

#include <vector>

#include "dawn/native/DawnNative.h"
#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/PassResourceUsage.h"
#include "dawn/native/d3d/TextureD3D.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native {
struct CopyTextureToTextureCmd;
}  // namespace dawn::native

namespace dawn::native::d3d {
class Fence;
class KeyedMutex;
}  // namespace dawn::native::d3d

namespace dawn::native::d3d11 {

class Device;
class TextureView;
class ScopedCommandRecordingContext;
class SharedTextureMemory;

class Texture final : public d3d::Texture {
  public:
    static ResultOrError<Ref<Texture>> Create(Device* device,
                                              const UnpackedPtr<TextureDescriptor>& descriptor);
    static ResultOrError<Ref<Texture>> Create(Device* device,
                                              const UnpackedPtr<TextureDescriptor>& descriptor,
                                              ComPtr<ID3D11Resource> d3d11Texture);
    static ResultOrError<Ref<Texture>> CreateFromSharedTextureMemory(
        SharedTextureMemory* memory,
        const UnpackedPtr<TextureDescriptor>& descriptor);
    ID3D11Resource* GetD3D11Resource() const;

    ResultOrError<ComPtr<ID3D11RenderTargetView>> CreateD3D11RenderTargetView(
        wgpu::TextureFormat format,
        uint32_t mipLevel,
        uint32_t baseSlice,
        uint32_t sliceCount,
        uint32_t planeSlice) const;
    ResultOrError<ComPtr<ID3D11DepthStencilView>> CreateD3D11DepthStencilView(
        const SubresourceRange& singleLevelRange,
        bool depthReadOnly,
        bool stencilReadOnly) const;
    MaybeError EnsureSubresourceContentInitialized(
        const ScopedCommandRecordingContext* commandContext,
        const SubresourceRange& range);

    MaybeError SynchronizeTextureBeforeUse(const ScopedCommandRecordingContext* commandContext);

    MaybeError Write(const ScopedCommandRecordingContext* commandContext,
                     const SubresourceRange& subresources,
                     const Origin3D& origin,
                     const Extent3D& size,
                     const uint8_t* data,
                     uint32_t bytesPerRow,
                     uint32_t rowsPerImage);
    using ReadCallback = std::function<MaybeError(const uint8_t* data, size_t offset, size_t size)>;
    MaybeError Read(const ScopedCommandRecordingContext* commandContext,
                    const SubresourceRange& subresources,
                    const Origin3D& origin,
                    Extent3D size,
                    uint32_t bytesPerRow,
                    uint32_t rowsPerImage,
                    ReadCallback callback);
    static MaybeError Copy(const ScopedCommandRecordingContext* commandContext,
                           CopyTextureToTextureCmd* copy);

    ResultOrError<ExecutionSerial> EndAccess() override;

    // As D3D11 SRV doesn't support 'Shader4ComponentMapping' for depth-stencil textures, we can't
    // sample the stencil component directly. As a workaround we create an internal R8Uint texture,
    // holding the copy of its stencil data, and use the internal texture's SRV instead.
    ResultOrError<ComPtr<ID3D11ShaderResourceView>> GetStencilSRV(
        const ScopedCommandRecordingContext* commandContext,
        const TextureView* view);

  private:
    using Base = d3d::Texture;

    enum class Kind { Normal, Staging, Interim };

    struct D3D11ClearValue {
        float color[4];
        float depth;
        uint8_t stencil;
    };

    static ResultOrError<Ref<Texture>>
    CreateInternal(Device* device, const UnpackedPtr<TextureDescriptor>& descriptor, Kind kind);

    Texture(Device* device, const UnpackedPtr<TextureDescriptor>& descriptor, Kind kind);
    ~Texture() override;

    template <typename T>
    T GetD3D11TextureDesc() const;

    MaybeError InitializeAsInternalTexture();
    MaybeError InitializeAsSwapChainTexture(ComPtr<ID3D11Resource> d3d11Texture);
    MaybeError InitializeAsExternalTexture(ComPtr<IUnknown> d3dTexture,
                                           Ref<d3d::KeyedMutex> keyedMutex);
    void SetLabelHelper(const char* prefix);

    // Dawn API
    void SetLabelImpl() override;
    void DestroyImpl() override;

    MaybeError Clear(const ScopedCommandRecordingContext* commandContext,
                     const SubresourceRange& range,
                     TextureBase::ClearValue clearValue);
    MaybeError ClearRenderable(const ScopedCommandRecordingContext* commandContext,
                               const SubresourceRange& range,
                               TextureBase::ClearValue clearValue,
                               const D3D11ClearValue& d3d11ClearValue);
    MaybeError ClearNonRenderable(const ScopedCommandRecordingContext* commandContext,
                                  const SubresourceRange& range,
                                  TextureBase::ClearValue clearValue);
    MaybeError ClearCompressed(const ScopedCommandRecordingContext* commandContext,
                               const SubresourceRange& range,
                               TextureBase::ClearValue clearValue);

    MaybeError ReadStaging(const ScopedCommandRecordingContext* commandContext,
                           const SubresourceRange& subresources,
                           const Origin3D& origin,
                           Extent3D size,
                           uint32_t bytesPerRow,
                           uint32_t rowsPerImage,
                           ReadCallback callback);

    // Write the texture without the content initialization bookkeeping.
    MaybeError WriteInternal(const ScopedCommandRecordingContext* commandContext,
                             const SubresourceRange& subresources,
                             const Origin3D& origin,
                             const Extent3D& size,
                             const uint8_t* data,
                             uint32_t bytesPerRow,
                             uint32_t rowsPerImage);

    // Write the depth-stencil texture without the content initialization bookkeeping.
    MaybeError WriteDepthStencilInternal(const ScopedCommandRecordingContext* commandContext,
                                         const SubresourceRange& subresources,
                                         const Origin3D& origin,
                                         const Extent3D& size,
                                         const uint8_t* data,
                                         uint32_t bytesPerRow,
                                         uint32_t rowsPerImage);

    // Copy the textures without the content initialization bookkeeping.
    static MaybeError CopyInternal(const ScopedCommandRecordingContext* commandContext,
                                   CopyTextureToTextureCmd* copy);

    const Kind mKind = Kind::Normal;
    ComPtr<ID3D11Resource> mD3d11Resource;
    Ref<d3d::KeyedMutex> mKeyedMutex;

    // The internal 'R8Uint' texture for sampling stencil from depth-stencil textures.
    Ref<Texture> mTextureForStencilSampling;
};

class TextureView final : public TextureViewBase {
  public:
    static Ref<TextureView> Create(TextureBase* texture,
                                   const UnpackedPtr<TextureViewDescriptor>& descriptor);

    ResultOrError<ID3D11ShaderResourceView*> GetOrCreateD3D11ShaderResourceView();
    ResultOrError<ID3D11RenderTargetView*> GetOrCreateD3D11RenderTargetView(
        uint32_t depthSlice = 0u);
    ResultOrError<ID3D11DepthStencilView*> GetOrCreateD3D11DepthStencilView(bool depthReadOnly,
                                                                            bool stencilReadOnly);
    ResultOrError<ID3D11UnorderedAccessView*> GetOrCreateD3D11UnorderedAccessView();

  private:
    using TextureViewBase::TextureViewBase;

    ~TextureView() override;
    void DestroyImpl() override;

    ComPtr<ID3D11ShaderResourceView> mD3d11SharedResourceView;

    std::vector<ComPtr<ID3D11RenderTargetView>> mD3d11RenderTargetViews;

    bool mD3d11DepthStencilViewDepthReadOnly = false;
    bool mD3d11DepthStencilViewStencilReadOnly = false;
    ComPtr<ID3D11DepthStencilView> mD3d11DepthStencilView;

    ComPtr<ID3D11UnorderedAccessView> mD3d11UnorderedAccessView;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_TEXTURED3D11_H_
