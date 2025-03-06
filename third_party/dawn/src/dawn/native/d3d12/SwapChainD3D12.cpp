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

#include "dawn/native/d3d12/SwapChainD3D12.h"

#if defined(DAWN_USE_WINDOWS_UI)
#include <windows.ui.xaml.media.dxinterop.h>
#endif  // defined(DAWN_USE_WINDOWS_UI)

#include <utility>

#include "dawn/native/ChainUtils.h"
#include "dawn/native/Surface.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/UtilsD3D.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/QueueD3D12.h"
#include "dawn/native/d3d12/TextureD3D12.h"

namespace dawn::native::d3d12 {
// static
ResultOrError<Ref<SwapChain>> SwapChain::Create(Device* device,
                                                Surface* surface,
                                                SwapChainBase* previousSwapChain,
                                                const SurfaceConfiguration* config) {
    Ref<SwapChain> swapchain = AcquireRef(new SwapChain(device, surface, config));
    DAWN_TRY(swapchain->Initialize(previousSwapChain));
    return swapchain;
}

SwapChain::~SwapChain() = default;

IUnknown* SwapChain::GetD3DDeviceForCreatingSwapChain() {
    return ToBackend(GetDevice()->GetQueue())->GetCommandQueue();
}

void SwapChain::ReuseBuffers(SwapChainBase* previousSwapChain) {
    SwapChain* previousD3DSwapChain = ToBackend(previousSwapChain);
    mBuffers = std::move(previousD3DSwapChain->mBuffers);

    // Remember the current state of the ID3D12Resource for the current buffer if we didn't have
    // chance to present it yet.
    if (previousD3DSwapChain->mApiTexture != nullptr) {
        D3D12_RESOURCE_STATES state =
            previousD3DSwapChain->mApiTexture->GetCurrentStateForSwapChain();
        mBuffers[previousD3DSwapChain->mCurrentBuffer].acquireState = state;
    }
}

MaybeError SwapChain::CollectSwapChainBuffers() {
    DAWN_ASSERT(GetDXGISwapChain() != nullptr);
    DAWN_ASSERT(mBuffers.empty());

    IDXGISwapChain3* dxgiSwapChain = GetDXGISwapChain();
    const auto& config = GetConfig();

    mBuffers.resize(config.bufferCount);
    for (uint32_t i = 0; i < config.bufferCount; i++) {
        DAWN_TRY(CheckHRESULT(dxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBuffers[i].resource)),
                              "Getting IDXGISwapChain buffer"));
    }

    return {};
}

MaybeError SwapChain::PresentImpl() {
    Queue* queue = ToBackend(GetDevice()->GetQueue());

    // Transition the texture to the present state as required by IDXGISwapChain1::Present()
    // TODO(crbug.com/dawn/269): Remove the need for this by eagerly transitioning the
    // presentable texture to present at the end of submits that use them.
    CommandRecordingContext* commandContext = queue->GetPendingCommandContext();
    mApiTexture->TrackUsageAndTransitionNow(commandContext, kPresentReleaseTextureUsage,
                                            mApiTexture->GetAllSubresources());
    DAWN_TRY(queue->SubmitPendingCommands());

    DAWN_TRY(PresentDXGISwapChain());

    // Record that "new" is the last time the buffer has been used.
    DAWN_TRY(queue->NextSerial());
    mBuffers[mCurrentBuffer].lastUsed = queue->GetLastSubmittedCommandSerial();
    mBuffers[mCurrentBuffer].acquireState = D3D12_RESOURCE_STATE_COMMON;

    mApiTexture->APIDestroy();
    mApiTexture = nullptr;

    return {};
}

ResultOrError<SwapChainTextureInfo> SwapChain::GetCurrentTextureImpl() {
    Queue* queue = ToBackend(GetDevice()->GetQueue());

    // Synchronously wait until previous operations on the next swapchain buffer are finished.
    // This is the logic that performs frame pacing.
    // TODO(crbug.com/dawn/269): Consider whether this should  be lifted for Mailbox so that
    // there is not frame pacing.
    mCurrentBuffer = GetDXGISwapChain()->GetCurrentBackBufferIndex();
    const Buffer& buffer = mBuffers[mCurrentBuffer];

    DAWN_TRY(queue->WaitForSerial(buffer.lastUsed));

    // Create the API side objects for this use of the swapchain's buffer.
    TextureDescriptor descriptor = GetSwapChainBaseTextureDescriptor(this);
    DAWN_TRY_ASSIGN(mApiTexture,
                    Texture::CreateForSwapChain(ToBackend(GetDevice()), Unpack(&descriptor),
                                                buffer.resource, buffer.acquireState));

    SwapChainTextureInfo info;
    info.texture = mApiTexture;
    info.status = wgpu::SurfaceGetCurrentTextureStatus::Success;
    // TODO(dawn:2320): Check for optimality
    info.suboptimal = false;
    return info;
}

MaybeError SwapChain::DetachAndWaitForDeallocation() {
    DetachFromSurface();

    // DetachFromSurface calls Texture->Destroy that enqueues the D3D12 resource in a
    // SerialQueue with the current "pending serial" so that we don't destroy the texture
    // before it is finished being used. Flush the commands and wait for that serial to be
    // passed, then Tick the device to make sure the reference to the D3D12 texture is removed.
    Queue* queue = ToBackend(GetDevice()->GetQueue());
    DAWN_TRY(queue->EnsureCommandsFlushed(queue->GetPendingCommandSerial()));
    DAWN_TRY(queue->WaitForSerial(queue->GetLastSubmittedCommandSerial()));
    return ToBackend(GetDevice())->TickImpl();
}

void SwapChain::DetachFromSurfaceImpl() {
    if (mApiTexture != nullptr) {
        mApiTexture->APIDestroy();
        mApiTexture = nullptr;
    }
    mBuffers.clear();

    ReleaseDXGISwapChain();
}

}  // namespace dawn::native::d3d12
