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

#ifndef SRC_DAWN_NATIVE_D3D11_DEVICED3D11_H_
#define SRC_DAWN_NATIVE_D3D11_DEVICED3D11_H_

#include <memory>
#include <vector>

#include "dawn/common/LRUCache.h"
#include "dawn/common/MutexProtected.h"
#include "dawn/common/SerialQueue.h"
#include "dawn/common/Sha3.h"
#include "dawn/native/d3d/DeviceD3D.h"
#include "dawn/native/d3d11/CommandRecordingContextD3D11.h"
#include "dawn/native/d3d11/DeviceInfoD3D11.h"
#include "dawn/native/d3d11/Forward.h"

namespace dawn::native::d3d {
struct CompiledShader;
}

namespace dawn::native::d3d11 {

struct Sha3CacheFuncs {
    size_t operator()(const Sha3_256::Output& key) const;
    bool operator()(const Sha3_256::Output& a, const Sha3_256::Output& b) const;
};

// Definition of backend types
class Device final : public d3d::Device {
  public:
    static ResultOrError<Ref<Device>> Create(AdapterBase* adapter,
                                             const UnpackedPtr<DeviceDescriptor>& descriptor,
                                             const TogglesState& deviceToggles,
                                             Ref<DeviceBase::DeviceLostEvent>&& lostEvent);
    ~Device() override;

    MaybeError Initialize(const UnpackedPtr<DeviceDescriptor>& descriptor);

    ID3D11Device* GetD3D11Device() const;
    ID3D11Device3* GetD3D11Device3() const;
    ID3D11Device5* GetD3D11Device5() const;

    const DeviceInfo& GetDeviceInfo() const;

    void ReferenceUntilUnused(ComPtr<IUnknown> object);

    ResultOrError<Ref<CommandBufferBase>> CreateCommandBuffer(
        CommandEncoder* encoder,
        const CommandBufferDescriptor* descriptor) override;
    MaybeError TickImpl() override;
    MaybeError CopyFromStagingToBuffer(BufferBase* source,
                                       uint64_t sourceOffset,
                                       BufferBase* destination,
                                       uint64_t destinationOffset,
                                       uint64_t size) override;
    MaybeError CopyFromStagingToTextureImpl(BufferBase* source,
                                            const TexelCopyBufferLayout& src,
                                            const TextureCopy& dst,
                                            const Extent3D& copySizePixels) override;
    uint32_t GetOptimalBytesPerRowAlignment() const override;
    uint64_t GetOptimalBufferToTextureCopyOffsetAlignment() const override;
    float GetTimestampPeriodInNS() const override;
    bool MayRequireDuplicationOfIndirectParameters() const override;
    uint64_t GetBufferCopyOffsetAlignmentForDepthStencil() const override;
    bool CanTextureLoadResolveTargetInTheSameRenderpass() const override;
    bool CanAddStorageUsageToBufferWithoutSideEffects(wgpu::BufferUsage storageUsage,
                                                      wgpu::BufferUsage originalUsage,
                                                      size_t bufferSize) const override;
    void SetLabelImpl() override;

    void DisposeKeyedMutex(ComPtr<IDXGIKeyedMutex> dxgiKeyedMutex) override;

    bool ReduceMemoryUsageImpl() override;

    std::optional<DeviceGuard> UseGuardForCreateBindGroup() override;
    std::optional<DeviceGuard> UseGuardForCreateBindGroupLayout() override;
    std::optional<DeviceGuard> UseGuardForCreateBuffer() override;
    std::optional<DeviceGuard> UseGuardForCreateSampler() override;
    std::optional<DeviceGuard> UseGuardForCreateTexture() override;

    uint32_t GetUAVSlotCount() const;

    ResultOrError<TextureViewBase*> GetOrCreateCachedImplicitPixelLocalStorageAttachment(
        uint32_t width,
        uint32_t height,
        uint32_t implicitAttachmentIndex);

    // Grab a staging buffer, the size of which is no less than 'size'.
    // The buffer must be returned before the advancing of the current pending serial.
    ResultOrError<Ref<BufferBase>> GetStagingBuffer(
        const ScopedCommandRecordingContext* commandContext,
        uint64_t size);
    void ReturnStagingBuffer(Ref<BufferBase>&& buffer);

    ResultOrError<ComPtr<ID3D11VertexShader>> GetOrCreateVertexShader(
        const d3d::CompiledShader& args);
    ResultOrError<ComPtr<ID3D11PixelShader>> GetOrCreatePixelShader(
        const d3d::CompiledShader& args);
    ResultOrError<ComPtr<ID3D11ComputeShader>> GetOrCreateComputeShader(
        const d3d::CompiledShader& args);

    void DeferUnmapDestroyedBuffer(ComPtr<ID3D11Buffer> buffer);

  private:
    using Base = d3d::Device;
    Device(AdapterBase* adapter,
           const UnpackedPtr<DeviceDescriptor>& descriptor,
           const TogglesState& deviceToggles,
           Ref<DeviceBase::DeviceLostEvent>&& lostEvent);
    static constexpr uint64_t kMaxStagingBufferSize = 512 * 1024;

    ResultOrError<Ref<BindGroupBase>> CreateBindGroupImpl(
        const UnpackedPtr<BindGroupDescriptor>& descriptor) override;
    ResultOrError<Ref<BindGroupLayoutInternalBase>> CreateBindGroupLayoutImpl(
        const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor) override;
    ResultOrError<Ref<BufferBase>> CreateBufferImpl(
        const UnpackedPtr<BufferDescriptor>& descriptor) override;
    ResultOrError<Ref<PipelineLayoutBase>> CreatePipelineLayoutImpl(
        const UnpackedPtr<PipelineLayoutDescriptor>& descriptor) override;
    ResultOrError<Ref<QuerySetBase>> CreateQuerySetImpl(
        const QuerySetDescriptor* descriptor) override;
    ResultOrError<Ref<ResourceTableBase>> CreateResourceTableImpl(
        const ResourceTableDescriptor* descriptor) override;
    ResultOrError<Ref<SamplerBase>> CreateSamplerImpl(const SamplerDescriptor* descriptor) override;
    ResultOrError<Ref<ShaderModuleBase>> CreateShaderModuleImpl(
        const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
        const std::vector<tint::wgsl::Extension>& internalExtensions) override;
    ResultOrError<Ref<SwapChainBase>> CreateSwapChainImpl(
        Surface* surface,
        SwapChainBase* previousSwapChain,
        const SurfaceConfiguration* config) override;
    ResultOrError<Ref<TextureBase>> CreateTextureImpl(
        const UnpackedPtr<TextureDescriptor>& descriptor) override;
    ResultOrError<Ref<TextureViewBase>> CreateTextureViewImpl(
        TextureBase* texture,
        const UnpackedPtr<TextureViewDescriptor>& descriptor) override;
    Ref<ComputePipelineBase> CreateUninitializedComputePipelineImpl(
        const UnpackedPtr<ComputePipelineDescriptor>& descriptor) override;
    Ref<RenderPipelineBase> CreateUninitializedRenderPipelineImpl(
        const UnpackedPtr<RenderPipelineDescriptor>& descriptor) override;
    void InitializeComputePipelineAsyncImpl(Ref<CreateComputePipelineAsyncEvent> event) override;
    void InitializeRenderPipelineAsyncImpl(Ref<CreateRenderPipelineAsyncEvent> event) override;

    ResultOrError<Ref<SharedTextureMemoryBase>> ImportSharedTextureMemoryImpl(
        const SharedTextureMemoryDescriptor* descriptor) override;
    ResultOrError<Ref<SharedFenceBase>> ImportSharedFenceImpl(
        const SharedFenceDescriptor* descriptor) override;

    void DestroyImpl(DestroyReason reason) override;
    MaybeError CheckDebugLayerAndGenerateErrors();
    void AppendDebugLayerMessages(ErrorData* error) override;
    void AppendDeviceLostMessage(ErrorData* error) override;

    void UnmapDestroyedBuffers();

    ComPtr<ID3D11Device> mD3d11Device;
    bool mIsDebugLayerEnabled = false;
    ComPtr<ID3D11Device3> mD3d11Device3;
    ComPtr<ID3D11Device5> mD3d11Device5;
    SerialQueue<ExecutionSerial, ComPtr<IUnknown>> mUsedComObjectRefs;

    // TODO(dawn:1704): decide when to clear the cached implicit pixel local storage attachments.
    std::array<Ref<TextureViewBase>, kMaxPLSSlots> mImplicitPixelLocalStorageAttachmentTextureViews;

    // The cached staging buffers.
    std::vector<Ref<BufferBase>> mStagingBuffers;
    uint64_t mTotalStagingBufferSize = 0;

    // The cached shader objects:
    // We use the SHA3 hash of the shader blob as the key because it's computed based on the
    // shader blob's hash. SHA3 is a cryptographic hash function, hence it's extremely unlikely
    // to have collisions (in fact, it's impractical).
    LRUCache<Sha3_256::Output, ComPtr<ID3D11VertexShader>, Sha3CacheFuncs> mVertexShaderCache;
    LRUCache<Sha3_256::Output, ComPtr<ID3D11PixelShader>, Sha3CacheFuncs> mPixelShaderCache;
    LRUCache<Sha3_256::Output, ComPtr<ID3D11ComputeShader>, Sha3CacheFuncs> mComputeShaderCache;

    // List of D3D11 buffers that were still mapped when destroyed and need to be unmapped.
    // This allows Buffer::DestroyImpl to defer the unmap operation to avoid acquiring the
    // CommandContext lock during destruction. The unmapping is performed later when
    // Tick() or ReduceMemoryUsage() is called.
    // MutexProtected is required because in the future Buffer::DestroyImpl might no longer be
    // protected by the device guard, making concurrent access possible from multiple threads.
    MutexProtected<std::vector<ComPtr<ID3D11Buffer>>> mPendingDestroyedBufferUnmaps;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_DEVICED3D11_H_
