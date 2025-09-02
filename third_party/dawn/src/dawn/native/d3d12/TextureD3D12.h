// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_D3D12_TEXTURED3D12_H_
#define SRC_DAWN_NATIVE_D3D12_TEXTURED3D12_H_

#include <optional>
#include <vector>

#include "dawn/native/Error.h"

#include "dawn/native/DawnNative.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/PassResourceUsage.h"
#include "dawn/native/Texture.h"
#include "dawn/native/d3d12/IntegerTypes.h"
#include "dawn/native/d3d12/ResourceHeapAllocationD3D12.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native {
namespace d3d {
class KeyedMutex;
}  // namespace d3d

namespace d3d12 {
class SharedTextureMemory;
class CommandRecordingContext;
class Device;

class Texture final : public TextureBase {
  public:
    static ResultOrError<Ref<Texture>> Create(Device* device,
                                              const UnpackedPtr<TextureDescriptor>& descriptor);
    static ResultOrError<Ref<Texture>> CreateForSwapChain(
        Device* device,
        const UnpackedPtr<TextureDescriptor>& descriptor,
        ComPtr<ID3D12Resource> d3d12Texture,
        D3D12_RESOURCE_STATES state);
    static ResultOrError<Ref<Texture>> CreateFromSharedTextureMemory(
        SharedTextureMemory* memory,
        const UnpackedPtr<TextureDescriptor>& descriptor);

    DXGI_FORMAT GetD3D12Format() const;
    ID3D12Resource* GetD3D12Resource() const;
    DXGI_FORMAT GetD3D12CopyableSubresourceFormat(Aspect aspect) const;
    D3D12_RESOURCE_FLAGS GetD3D12ResourceFlags() const;

    D3D12_RENDER_TARGET_VIEW_DESC GetRTVDescriptor(const Format& format,
                                                   uint32_t mipLevel,
                                                   uint32_t baseSlice,
                                                   uint32_t sliceCount,
                                                   uint32_t planeSlice) const;
    D3D12_DEPTH_STENCIL_VIEW_DESC GetDSVDescriptor(uint32_t mipLevel,
                                                   uint32_t baseArrayLayer,
                                                   uint32_t layerCount,
                                                   Aspect aspects,
                                                   bool depthReadOnly,
                                                   bool stencilReadOnly) const;

    MaybeError EnsureSubresourceContentInitialized(CommandRecordingContext* commandContext,
                                                   const SubresourceRange& range);

    MaybeError SynchronizeTextureBeforeUse(CommandRecordingContext* commandContext);

    void NotifySwapChainPresentToPIX();

    void SetIsSwapchainTexture(bool isSwapChainTexture);

    void TrackUsageAndGetResourceBarrierForPass(CommandRecordingContext* commandContext,
                                                std::vector<D3D12_RESOURCE_BARRIER>* barrier,
                                                const TextureSubresourceSyncInfo& textureSyncInfos);
    void TransitionUsageAndGetResourceBarrier(CommandRecordingContext* commandContext,
                                              std::vector<D3D12_RESOURCE_BARRIER>* barrier,
                                              wgpu::TextureUsage usage,
                                              const SubresourceRange& range);
    void TrackUsageAndTransitionNow(CommandRecordingContext* commandContext,
                                    wgpu::TextureUsage usage,
                                    const SubresourceRange& range);
    void TrackUsageAndTransitionNow(CommandRecordingContext* commandContext,
                                    D3D12_RESOURCE_STATES newState,
                                    const SubresourceRange& range);
    // Reset the D3D12_RESOURCE_STATES and decay tracking to indicate that
    // all subresources are now in the COMMON state.
    void ResetSubresourceStateAndDecayToCommon();

    D3D12_RESOURCE_STATES GetCurrentStateForSwapChain() const;

  private:
    using Base = TextureBase;

    Texture(Device* device, const UnpackedPtr<TextureDescriptor>& descriptor);
    ~Texture() override;

    MaybeError InitializeAsInternalTexture();
    MaybeError InitializeAsExternalTexture(ComPtr<IUnknown> d3dTexture,
                                           Ref<d3d::KeyedMutex> keyedMutex,
                                           std::vector<FenceAndSignalValue> waitFences,
                                           bool isSwapChainTexture);
    MaybeError InitializeAsSwapChainTexture(ComPtr<ID3D12Resource> d3d12Texture,
                                            D3D12_RESOURCE_STATES state);

    void SetLabelHelper(const char* prefix);

    // Dawn API
    void SetLabelImpl() override;
    void DestroyImpl() override;

    MaybeError ClearTexture(CommandRecordingContext* commandContext,
                            const SubresourceRange& range,
                            TextureBase::ClearValue clearValue);

    // Barriers implementation details.
    struct StateAndDecay {
        D3D12_RESOURCE_STATES lastState;
        ExecutionSerial lastDecaySerial;
        bool isValidToDecay;

        bool operator==(const StateAndDecay& other) const = default;
    };

    SubresourceStorage<StateAndDecay> InitialSubresourceStateAndDecay() const;

    void TransitionUsageAndGetResourceBarrier(CommandRecordingContext* commandContext,
                                              std::vector<D3D12_RESOURCE_BARRIER>* barrier,
                                              D3D12_RESOURCE_STATES newState,
                                              const SubresourceRange& range);
    void TransitionSubresourceRange(std::vector<D3D12_RESOURCE_BARRIER>* barriers,
                                    const SubresourceRange& range,
                                    StateAndDecay* state,
                                    D3D12_RESOURCE_STATES subresourceNewState,
                                    ExecutionSerial pendingCommandSerial) const;
    void HandleTransitionSpecialCases(CommandRecordingContext* commandContext);

    D3D12_RESOURCE_FLAGS mD3D12ResourceFlags;
    ResourceHeapAllocation mResourceAllocation;

    Ref<d3d::KeyedMutex> mKeyedMutex;

    // TODO(crbug.com/1515640): Remove wait fences once Chromium has migrated to
    // SharedTextureMemory.
    std::vector<FenceAndSignalValue> mWaitFences;

    bool mSwapChainTexture = false;

    SubresourceStorage<StateAndDecay> mSubresourceStateAndDecay;
};

class TextureView final : public TextureViewBase {
  public:
    static Ref<TextureView> Create(TextureBase* texture,
                                   const UnpackedPtr<TextureViewDescriptor>& descriptor);

    DXGI_FORMAT GetD3D12Format() const;

    const D3D12_SHADER_RESOURCE_VIEW_DESC& GetSRVDescriptor() const;
    D3D12_RENDER_TARGET_VIEW_DESC GetRTVDescriptor(uint32_t depthSlice = 0u) const;
    D3D12_DEPTH_STENCIL_VIEW_DESC GetDSVDescriptor(bool depthReadOnly, bool stencilReadOnly) const;
    D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDescriptor() const;

  private:
    TextureView(TextureBase* texture, const UnpackedPtr<TextureViewDescriptor>& descriptor);

    D3D12_SHADER_RESOURCE_VIEW_DESC mSrvDesc;
};
}  // namespace d3d12
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_D3D12_TEXTURED3D12_H_
