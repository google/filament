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

#include "dawn/native/d3d11/DeviceD3D11.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <utility>

#include "dawn/common/GPUInfo.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/D3D11Backend.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/KeyedMutex.h"
#include "dawn/native/d3d/UtilsD3D.h"
#include "dawn/native/d3d11/BackendD3D11.h"
#include "dawn/native/d3d11/BindGroupD3D11.h"
#include "dawn/native/d3d11/BindGroupLayoutD3D11.h"
#include "dawn/native/d3d11/BufferD3D11.h"
#include "dawn/native/d3d11/CommandBufferD3D11.h"
#include "dawn/native/d3d11/CommandRecordingContextD3D11.h"
#include "dawn/native/d3d11/ComputePipelineD3D11.h"
#include "dawn/native/d3d11/PhysicalDeviceD3D11.h"
#include "dawn/native/d3d11/PipelineLayoutD3D11.h"
#include "dawn/native/d3d11/PlatformFunctionsD3D11.h"
#include "dawn/native/d3d11/QuerySetD3D11.h"
#include "dawn/native/d3d11/QueueD3D11.h"
#include "dawn/native/d3d11/RenderPipelineD3D11.h"
#include "dawn/native/d3d11/SamplerD3D11.h"
#include "dawn/native/d3d11/ShaderModuleD3D11.h"
#include "dawn/native/d3d11/SharedFenceD3D11.h"
#include "dawn/native/d3d11/SharedTextureMemoryD3D11.h"
#include "dawn/native/d3d11/SwapChainD3D11.h"
#include "dawn/native/d3d11/TextureD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::d3d11 {
namespace {

static constexpr uint64_t kMaxDebugMessagesToPrint = 5;

bool SkipDebugMessage(const D3D11_MESSAGE& message) {
    // Filter out messages that are not errors.
    switch (message.Severity) {
        case D3D11_MESSAGE_SEVERITY_INFO:
        case D3D11_MESSAGE_SEVERITY_MESSAGE:
        case D3D11_MESSAGE_SEVERITY_WARNING:
            return true;
        default:
            break;
    }

    switch (message.ID) {
        // D3D11 Debug layer warns no RTV set, however it is allowed.
        case D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET:
        // D3D11 Debug layer warns SetPrivateData() with same name more than once.
        case D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS:
            return true;
        case D3D11_MESSAGE_ID_DEVICE_CHECKFEATURESUPPORT_UNRECOGNIZED_FEATURE:
        case D3D11_MESSAGE_ID_DEVICE_CHECKFEATURESUPPORT_INVALIDARG_RETURN:
            // We already handle CheckFeatureSupport() failures so ignore the messages from the
            // debug layer.
            return true;
        case D3D11_MESSAGE_ID_DECODERBEGINFRAME_HAZARD:
            // This is video decoder's error which must happen externally because Dawn doesn't
            // handle video directly. So ignore it.
            return true;
        case D3D11_MESSAGE_ID_DEVICE_DRAWINSTANCED_INSTANCEPOS_OVERFLOW:
        case D3D11_MESSAGE_ID_DEVICE_DRAWINDEXEDINSTANCED_INSTANCEPOS_OVERFLOW:
            // Some clients use workarounds such as packing uniform data in a 32 bits BaseInstance
            // value to avoid needing an uniform buffer. See
            // https://source.chromium.org/chromium/chromium/src/+/main:third_party/skia/src/gpu/graphite/dawn/DawnResourceProvider.cpp;drc=d29622dea776ec762e8a69c8c65b87f8e9ee8908;l=398
            // for one example.
            // The embedded data would cause BaseInstance + InstanceCount to overflow. This is not
            // an error because the workarounds always use InstanceCount=1 and we never invoke any
            // vertex shader at (BaseInstance + InstanceCount)-th instance.
            // Furthermore, the behavior of overflown InstanceID is already well documented in
            // https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-input-assembler-stage-using#instanceid
            // i.e. it will wrap to 0.
            return true;
        case D3D11_MESSAGE_ID_CREATETEXTURE2D_INVALIDDIMENSIONS:
        case D3D11_MESSAGE_ID_CREATETEXTURE2D_INVALIDARG_RETURN:
            // External video decoder sometimes attempts to create a multiplanar texture with
            // non-even dimensions. This is not supported by D3D11 runtime, but the error should
            // already be handled by the call sites. Chromium's video decoder already does that so
            // it's better we don't treat this as a fatal error.
            return true;
        default:
            return false;
    }
}

uint64_t AppendDebugLayerMessagesToError(ID3D11InfoQueue* infoQueue,
                                         uint64_t totalErrors,
                                         ErrorData* error) {
    DAWN_ASSERT(totalErrors > 0);
    DAWN_ASSERT(error != nullptr);

    uint64_t errorsEmitted = 0;
    for (uint64_t i = 0; i < totalErrors; ++i) {
        std::ostringstream messageStream;
        SIZE_T messageLength = 0;
        HRESULT hr = infoQueue->GetMessage(i, nullptr, &messageLength);
        if (FAILED(hr)) {
            messageStream << " ID3D11InfoQueue::GetMessage failed with " << hr;
            error->AppendBackendMessage(messageStream.str());
            continue;
        }

        std::unique_ptr<uint8_t[]> messageData(new uint8_t[messageLength]);
        D3D11_MESSAGE* message = reinterpret_cast<D3D11_MESSAGE*>(messageData.get());
        hr = infoQueue->GetMessage(i, message, &messageLength);
        if (FAILED(hr)) {
            messageStream << " ID3D11InfoQueue::GetMessage failed with " << hr;
            error->AppendBackendMessage(messageStream.str());
            continue;
        }

        if (SkipDebugMessage(*message)) {
            continue;
        }

        messageStream << "(" << message->ID << ") " << message->pDescription;
        error->AppendBackendMessage(messageStream.str());

        errorsEmitted++;
        if (errorsEmitted >= kMaxDebugMessagesToPrint) {
            break;
        }
    }

    if (errorsEmitted < totalErrors) {
        std::ostringstream messages;
        messages << (totalErrors - errorsEmitted) << " messages silenced";
        error->AppendBackendMessage(messages.str());
    }

    // We only print up to the first kMaxDebugMessagesToPrint errors
    infoQueue->ClearStoredMessages();

    return errorsEmitted;
}

}  // namespace

// static
ResultOrError<Ref<Device>> Device::Create(AdapterBase* adapter,
                                          const UnpackedPtr<DeviceDescriptor>& descriptor,
                                          const TogglesState& deviceToggles,
                                          Ref<DeviceBase::DeviceLostEvent>&& lostEvent) {
    Ref<Device> device =
        AcquireRef(new Device(adapter, descriptor, deviceToggles, std::move(lostEvent)));
    DAWN_TRY(device->Initialize(descriptor));
    return device;
}

MaybeError Device::Initialize(const UnpackedPtr<DeviceDescriptor>& descriptor) {
    DAWN_TRY_ASSIGN(
        mD3d11Device,
        ToBackend(GetPhysicalDevice())
            ->CreateD3D11Device(GetAdapter()->GetInstance()->IsBackendValidationEnabled()));
    DAWN_ASSERT(mD3d11Device != nullptr);

    mIsDebugLayerEnabled = IsDebugLayerEnabled(mD3d11Device);

    DAWN_TRY(CheckHRESULT(mD3d11Device.As(&mD3d11Device3), "D3D11: getting ID3D11Device3"));

    if (!IsToggleEnabled(Toggle::D3D11DisableFence)) {
        // Get the ID3D11Device5 interface which is need for creating fences. This interface is only
        // available since Win 10 Creators Update so don't return on error here.
        mD3d11Device.As(&mD3d11Device5);
    }

    Ref<Queue> queue;
    DAWN_TRY_ASSIGN(queue, Queue::Create(this, &descriptor->defaultQueue));

    DAWN_TRY(DeviceBase::Initialize(descriptor, queue));
    DAWN_TRY(queue->InitializePendingContext());

    SetLabelImpl();

    return {};
}

Device::~Device() = default;

ID3D11Device* Device::GetD3D11Device() const {
    return mD3d11Device.Get();
}

ID3D11Device3* Device::GetD3D11Device3() const {
    return mD3d11Device3.Get();
}

ID3D11Device5* Device::GetD3D11Device5() const {
    // Some older devices don't support ID3D11Device5. Make sure we avoid calling this method in
    // those cases. An assert here is to verify that.
    DAWN_ASSERT(mD3d11Device5);
    return mD3d11Device5.Get();
}

MaybeError Device::TickImpl() {
    // Check for debug layer messages before executing the command context in case we encounter an
    // error during execution and early out as a result.
    DAWN_TRY(CheckDebugLayerAndGenerateErrors());
    DAWN_TRY(ToBackend(GetQueue())->SubmitPendingCommands());
    return {};
}

void Device::ReferenceUntilUnused(ComPtr<IUnknown> object) {
    mUsedComObjectRefs.Enqueue(object, GetQueue()->GetPendingCommandSerial());
}

ResultOrError<Ref<BindGroupBase>> Device::CreateBindGroupImpl(
    const BindGroupDescriptor* descriptor) {
    return BindGroup::Create(this, descriptor);
}

ResultOrError<Ref<BindGroupLayoutInternalBase>> Device::CreateBindGroupLayoutImpl(
    const BindGroupLayoutDescriptor* descriptor) {
    return BindGroupLayout::Create(this, descriptor);
}

ResultOrError<Ref<BufferBase>> Device::CreateBufferImpl(
    const UnpackedPtr<BufferDescriptor>& descriptor) {
    return Buffer::Create(this, descriptor, /*commandContext=*/nullptr);
}

ResultOrError<Ref<CommandBufferBase>> Device::CreateCommandBuffer(
    CommandEncoder* encoder,
    const CommandBufferDescriptor* descriptor) {
    return CommandBuffer::Create(encoder, descriptor);
}

Ref<ComputePipelineBase> Device::CreateUninitializedComputePipelineImpl(
    const UnpackedPtr<ComputePipelineDescriptor>& descriptor) {
    return ComputePipeline::CreateUninitialized(this, descriptor);
}

ResultOrError<Ref<PipelineLayoutBase>> Device::CreatePipelineLayoutImpl(
    const UnpackedPtr<PipelineLayoutDescriptor>& descriptor) {
    return PipelineLayout::Create(this, descriptor);
}

ResultOrError<Ref<QuerySetBase>> Device::CreateQuerySetImpl(const QuerySetDescriptor* descriptor) {
    return QuerySet::Create(this, descriptor);
}

Ref<RenderPipelineBase> Device::CreateUninitializedRenderPipelineImpl(
    const UnpackedPtr<RenderPipelineDescriptor>& descriptor) {
    return RenderPipeline::CreateUninitialized(this, descriptor);
}

ResultOrError<Ref<SamplerBase>> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
    return Sampler::Create(this, descriptor);
}

ResultOrError<Ref<ShaderModuleBase>> Device::CreateShaderModuleImpl(
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
    const std::vector<tint::wgsl::Extension>& internalExtensions,
    ShaderModuleParseResult* parseResult) {
    return ShaderModule::Create(this, descriptor, internalExtensions, parseResult);
}

ResultOrError<Ref<SwapChainBase>> Device::CreateSwapChainImpl(Surface* surface,
                                                              SwapChainBase* previousSwapChain,
                                                              const SurfaceConfiguration* config) {
    return SwapChain::Create(this, surface, previousSwapChain, config);
}

ResultOrError<Ref<TextureBase>> Device::CreateTextureImpl(
    const UnpackedPtr<TextureDescriptor>& descriptor) {
    return Texture::Create(this, descriptor);
}

ResultOrError<Ref<TextureViewBase>> Device::CreateTextureViewImpl(
    TextureBase* texture,
    const UnpackedPtr<TextureViewDescriptor>& descriptor) {
    return TextureView::Create(texture, descriptor);
}

void Device::InitializeComputePipelineAsyncImpl(Ref<CreateComputePipelineAsyncEvent> event) {
    event->InitializeAsync();
}

void Device::InitializeRenderPipelineAsyncImpl(Ref<CreateRenderPipelineAsyncEvent> event) {
    event->InitializeAsync();
}

ResultOrError<Ref<SharedTextureMemoryBase>> Device::ImportSharedTextureMemoryImpl(
    const SharedTextureMemoryDescriptor* descriptor) {
    UnpackedPtr<SharedTextureMemoryDescriptor> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(descriptor));

    wgpu::SType type;
    DAWN_TRY_ASSIGN(
        type, (unpacked.ValidateBranches<Branch<SharedTextureMemoryDXGISharedHandleDescriptor>,
                                         Branch<SharedTextureMemoryD3D11Texture2DDescriptor>>()));

    switch (type) {
        case wgpu::SType::SharedTextureMemoryDXGISharedHandleDescriptor:
            DAWN_INVALID_IF(!HasFeature(Feature::SharedTextureMemoryDXGISharedHandle),
                            "%s is not enabled.",
                            wgpu::FeatureName::SharedTextureMemoryDXGISharedHandle);
            return SharedTextureMemory::Create(
                this, descriptor->label,
                unpacked.Get<SharedTextureMemoryDXGISharedHandleDescriptor>());
        case wgpu::SType::SharedTextureMemoryD3D11Texture2DDescriptor:
            DAWN_INVALID_IF(!HasFeature(Feature::SharedTextureMemoryD3D11Texture2D),
                            "%s is not enabled.",
                            wgpu::FeatureName::SharedTextureMemoryD3D11Texture2D);
            return SharedTextureMemory::Create(
                this, descriptor->label,
                unpacked.Get<SharedTextureMemoryD3D11Texture2DDescriptor>());
        default:
            DAWN_UNREACHABLE();
    }
}

ResultOrError<Ref<SharedFenceBase>> Device::ImportSharedFenceImpl(
    const SharedFenceDescriptor* descriptor) {
    UnpackedPtr<SharedFenceDescriptor> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(descriptor));

    wgpu::SType type;
    DAWN_TRY_ASSIGN(type,
                    (unpacked.ValidateBranches<Branch<SharedFenceDXGISharedHandleDescriptor>>()));

    switch (type) {
        case wgpu::SType::SharedFenceDXGISharedHandleDescriptor:
            DAWN_INVALID_IF(!HasFeature(Feature::SharedFenceDXGISharedHandle), "%s is not enabled.",
                            wgpu::FeatureName::SharedFenceDXGISharedHandle);
            return SharedFence::Create(this, descriptor->label,
                                       unpacked.Get<SharedFenceDXGISharedHandleDescriptor>());
        default:
            DAWN_UNREACHABLE();
    }
}

MaybeError Device::CopyFromStagingToBuffer(BufferBase* source,
                                           uint64_t sourceOffset,
                                           BufferBase* destination,
                                           uint64_t destinationOffset,
                                           uint64_t size) {
    // D3D11 requires that buffers are unmapped before being used in a copy.
    DAWN_TRY(source->Unmap());

    auto commandContext =
        ToBackend(GetQueue())->GetScopedPendingCommandContext(QueueBase::SubmitMode::Normal);
    return Buffer::Copy(&commandContext, ToBackend(source), sourceOffset, size,
                        ToBackend(destination), destinationOffset);
}

MaybeError Device::CopyFromStagingToTextureImpl(const BufferBase* source,
                                                const TexelCopyBufferLayout& src,
                                                const TextureCopy& dst,
                                                const Extent3D& copySizePixels) {
    return DAWN_UNIMPLEMENTED_ERROR("CopyFromStagingToTextureImpl");
}

const DeviceInfo& Device::GetDeviceInfo() const {
    return ToBackend(GetPhysicalDevice())->GetDeviceInfo();
}

MaybeError Device::CheckDebugLayerAndGenerateErrors() {
    if (!mIsDebugLayerEnabled) {
        return {};
    }

    ComPtr<ID3D11InfoQueue> infoQueue;
    DAWN_TRY(CheckHRESULT(mD3d11Device.As(&infoQueue),
                          "D3D11 QueryInterface ID3D11Device to ID3D11InfoQueue"));

    // We use GetNumStoredMessages instead of applying a retrieval filter because dxcpl.exe
    // and d3dconfig.exe override any filter settings we apply.
    const uint64_t totalErrors = infoQueue->GetNumStoredMessages();
    if (totalErrors == 0) {
        return {};
    }

    auto error = DAWN_INTERNAL_ERROR("The D3D11 debug layer reported uncaught errors.");

    const uint64_t emittedErrors =
        AppendDebugLayerMessagesToError(infoQueue.Get(), totalErrors, error.get());
    if (emittedErrors == 0) {
        return {};
    }

    return error;
}

void Device::AppendDebugLayerMessages(ErrorData* error) {
    if (!GetAdapter()->GetInstance()->IsBackendValidationEnabled()) {
        return;
    }

    ComPtr<ID3D11InfoQueue> infoQueue;
    if (FAILED(mD3d11Device.As(&infoQueue))) {
        return;
    }

    const uint64_t totalErrors = infoQueue->GetNumStoredMessages();
    if (totalErrors == 0) {
        return;
    }

    AppendDebugLayerMessagesToError(infoQueue.Get(), totalErrors, error);
}

void Device::AppendDeviceLostMessage(ErrorData* error) {
    if (mD3d11Device) {
        HRESULT result = mD3d11Device->GetDeviceRemovedReason();
        error->AppendBackendMessage("Device removed reason: %s (0x%08X)",
                                    d3d::HRESULTAsString(result), result);
        RecordDeviceRemovedReason(result);
    }
}

void Device::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the device is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the device.
    // - It may be called when the last ref to the device is dropped and the device
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the device since there are no other live refs.
    DAWN_ASSERT(GetState() == State::Disconnected);

    mImplicitPixelLocalStorageAttachmentTextureViews = {};
    mStagingBuffers.clear();

    Base::DestroyImpl();
}

uint32_t Device::GetOptimalBytesPerRowAlignment() const {
    return 256;
}

uint64_t Device::GetOptimalBufferToTextureCopyOffsetAlignment() const {
    return 1;
}

float Device::GetTimestampPeriodInNS() const {
    return 1.0f;
}

void Device::SetLabelImpl() {}

void Device::DisposeKeyedMutex(ComPtr<IDXGIKeyedMutex> dxgiKeyedMutex) {
    // Nothing to do, the ComPtr will release the keyed mutex.
}

bool Device::ReduceMemoryUsageImpl() {
    // D3D11 defers the deletion of resources until we call Flush().
    // So trigger a Flush() here to force deleting any pending resources.
    auto commandContext =
        ToBackend(GetQueue())
            ->GetScopedPendingCommandContext(ExecutionQueueBase::SubmitMode::Passive);
    commandContext.Flush();

    // Call Trim() to delete any internal resources created by the driver.
    ComPtr<IDXGIDevice3> dxgiDevice3;
    if (SUCCEEDED(mD3d11Device.As(&dxgiDevice3))) {
        dxgiDevice3->Trim();
    }

    return false;
}

bool Device::MayRequireDuplicationOfIndirectParameters() const {
    return true;
}

uint64_t Device::GetBufferCopyOffsetAlignmentForDepthStencil() const {
    return DeviceBase::GetBufferCopyOffsetAlignmentForDepthStencil();
}

bool Device::CanTextureLoadResolveTargetInTheSameRenderpass() const {
    return true;
}

bool Device::CanAddStorageUsageToBufferWithoutSideEffects(wgpu::BufferUsage storageUsage,
                                                          wgpu::BufferUsage originalUsage,
                                                          size_t bufferSize) const {
    return d3d11::CanAddStorageUsageToBufferWithoutSideEffects(this, storageUsage, originalUsage,
                                                               bufferSize);
}

uint32_t Device::GetUAVSlotCount() const {
    return ToBackend(GetPhysicalDevice())->GetUAVSlotCount();
}

ResultOrError<TextureViewBase*> Device::GetOrCreateCachedImplicitPixelLocalStorageAttachment(
    uint32_t width,
    uint32_t height,
    uint32_t implicitAttachmentIndex) {
    DAWN_ASSERT(implicitAttachmentIndex <= kMaxPLSSlots);

    TextureViewBase* currentAttachmentView =
        mImplicitPixelLocalStorageAttachmentTextureViews[implicitAttachmentIndex].Get();
    if (currentAttachmentView == nullptr ||
        currentAttachmentView->GetTexture()->GetWidth(Aspect::Color) < width ||
        currentAttachmentView->GetTexture()->GetHeight(Aspect::Color) < height) {
        // Create one 2D texture for each attachment. Note that currently on D3D11 backend we cannot
        // create a Texture2D UAV on a 2D array texture with baseArrayLayer > 0 because D3D11
        // requires the Unordered Access View dimension declared in the shader code must match the
        // view type bound to the Pixel Shader unit, while TEXTURE2D doesn't match TEXTURE2DARRAY.
        // TODO(dawn:1703): support 2D array storage textures as implicit pixel local storage
        // attachments in WGSL.
        TextureDescriptor desc;
        desc.dimension = wgpu::TextureDimension::e2D;
        desc.format = RenderPipelineBase::kImplicitPLSSlotFormat;
        desc.usage = wgpu::TextureUsage::StorageAttachment;
        desc.size = {width, height, 1};

        Ref<TextureBase> newAttachment;
        DAWN_TRY_ASSIGN(newAttachment, CreateTexture(&desc));
        DAWN_TRY_ASSIGN(mImplicitPixelLocalStorageAttachmentTextureViews[implicitAttachmentIndex],
                        newAttachment->CreateView());
    }
    return mImplicitPixelLocalStorageAttachmentTextureViews[implicitAttachmentIndex].Get();
}

ResultOrError<Ref<BufferBase>> Device::GetStagingBuffer(
    const ScopedCommandRecordingContext* commandContext,
    uint64_t size) {
    constexpr uint64_t kMinStagingBufferSize = 4 * 1024;
    uint64_t bufferSize = Align(size, kMinStagingBufferSize);
    BufferDescriptor descriptor;
    descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    descriptor.size = bufferSize;
    descriptor.mappedAtCreation = false;
    descriptor.label = "DawnDeviceStagingBuffer";
    Ref<BufferBase> buffer;
    // We don't cache the buffer if it's too large.
    if (bufferSize > kMaxStagingBufferSize) {
        DAWN_TRY_ASSIGN(buffer, Buffer::Create(this, Unpack(&descriptor), commandContext,
                                               /*allowUploadBufferEmulation=*/false));
        return buffer;
    }

    ExecutionSerial completedSerial = GetQueue()->GetCompletedCommandSerial();
    for (auto it = mStagingBuffers.begin(); it != mStagingBuffers.end(); ++it) {
        if ((*it)->GetLastUsageSerial() > completedSerial) {
            // This buffer, and none after it are ready. Advance to the end and stop the search.
            break;
        }

        if ((*it)->GetSize() >= bufferSize) {
            // this buffer is large enough. Stop searching and remove.
            buffer = *it;
            mStagingBuffers.erase(it);
            return buffer;
        }
    }

    // Create a new staging buffer as no existing one can be re-used.
    DAWN_TRY_ASSIGN(buffer, Buffer::Create(this, Unpack(&descriptor), commandContext,
                                           /*allowUploadBufferEmulation=*/false));
    mTotalStagingBufferSize += bufferSize;

    // Purge the old staging buffers if the total size is too large.
    constexpr uint64_t kMaxTotalSize = 16 * 1024 * 1024;
    for (auto it = mStagingBuffers.begin(); it != mStagingBuffers.end() &&
                                            mTotalStagingBufferSize > kMaxTotalSize &&
                                            (*it)->GetLastUsageSerial() <= completedSerial;) {
        mTotalStagingBufferSize -= (*it)->GetSize();
        it = mStagingBuffers.erase(it);
    }

    return buffer;
}

void Device::ReturnStagingBuffer(Ref<BufferBase>&& buffer) {
    DAWN_ASSERT(mStagingBuffers.empty() ||
                mStagingBuffers.back()->GetLastUsageSerial() <= buffer->GetLastUsageSerial());
    // Only the cached buffers can be re-used.
    if (buffer->GetSize() <= kMaxStagingBufferSize) {
        mStagingBuffers.push_back(std::move(buffer));
    }
}

}  // namespace dawn::native::d3d11
