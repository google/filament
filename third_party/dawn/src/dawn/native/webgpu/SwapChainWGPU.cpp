// Copyright 2026 The Dawn & Tint Authors
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

#include "src/dawn/native/webgpu/SwapChainWGPU.h"

#include <utility>
#include <vector>

#include "src/dawn/common/StringViewUtils.h"
#include "src/dawn/native/Adapter.h"
#include "src/dawn/native/ChainUtils.h"
#include "src/dawn/native/Surface.h"
#include "src/dawn/native/webgpu/CaptureContext.h"
#include "src/dawn/native/webgpu/DeviceWGPU.h"
#include "src/dawn/native/webgpu/QueueWGPU.h"
#include "src/dawn/native/webgpu/TextureWGPU.h"
#include "src/dawn/native/webgpu/ToWGPU.h"
#include "src/dawn/native/webgpu/WebGPUError.h"
#include "src/utils/compiler.h"

namespace dawn::native::webgpu {

ResultOrError<WGPUSurface> CreateWGPUSurface(const DawnProcTable& procs,
                                             WGPUInstance instance,
                                             const Surface* surface) {
    WGPUSurfaceDescriptor descriptor = WGPU_SURFACE_DESCRIPTOR_INIT;
    descriptor.label = ToOutputStringView(surface->GetLabel());

    // Reconstruct the chained descriptor based on the surface type.
    WGPUSurfaceSourceAndroidNativeWindow androidDesc =
        WGPU_SURFACE_SOURCE_ANDROID_NATIVE_WINDOW_INIT;
    WGPUSurfaceSourceMetalLayer metalDesc = WGPU_SURFACE_SOURCE_METAL_LAYER_INIT;
    WGPUSurfaceSourceWindowsHWND hwndDesc = WGPU_SURFACE_SOURCE_WINDOWS_HWND_INIT;
    WGPUSurfaceSourceWaylandSurface waylandDesc = WGPU_SURFACE_SOURCE_WAYLAND_SURFACE_INIT;
    WGPUSurfaceSourceXlibWindow xlibDesc = WGPU_SURFACE_SOURCE_XLIB_WINDOW_INIT;
    WGPUSurfaceDescriptorFromWindowsCoreWindow coreWindowDesc =
        WGPU_SURFACE_DESCRIPTOR_FROM_WINDOWS_CORE_WINDOW_INIT;
    WGPUSurfaceDescriptorFromWindowsUWPSwapChainPanel uwpDesc =
        WGPU_SURFACE_DESCRIPTOR_FROM_WINDOWS_UWP_SWAP_CHAIN_PANEL_INIT;
    WGPUSurfaceDescriptorFromWindowsWinUISwapChainPanel winuiDesc =
        WGPU_SURFACE_DESCRIPTOR_FROM_WINDOWS_WINUI_SWAP_CHAIN_PANEL_INIT;

    switch (surface->GetType()) {
        case Surface::Type::AndroidWindow:
            androidDesc.chain.sType = WGPUSType_SurfaceSourceAndroidNativeWindow;
            androidDesc.window = surface->GetAndroidNativeWindow();
            descriptor.nextInChain = &androidDesc.chain;
            break;
        case Surface::Type::MetalLayer:
            metalDesc.chain.sType = WGPUSType_SurfaceSourceMetalLayer;
            metalDesc.layer = surface->GetMetalLayer();
            descriptor.nextInChain = &metalDesc.chain;
            break;
        case Surface::Type::WindowsHWND:
            hwndDesc.chain.sType = WGPUSType_SurfaceSourceWindowsHWND;
            hwndDesc.hinstance = surface->GetHInstance();
            hwndDesc.hwnd = surface->GetHWND();
            descriptor.nextInChain = &hwndDesc.chain;
            break;
        case Surface::Type::WindowsCoreWindow:
            coreWindowDesc.chain.sType = WGPUSType_SurfaceDescriptorFromWindowsCoreWindow;
            coreWindowDesc.coreWindow = surface->GetCoreWindow();
            descriptor.nextInChain = &coreWindowDesc.chain;
            break;
        case Surface::Type::WindowsUWPSwapChainPanel:
            uwpDesc.chain.sType = WGPUSType_SurfaceDescriptorFromWindowsUWPSwapChainPanel;
            uwpDesc.swapChainPanel = surface->GetUWPSwapChainPanel();
            descriptor.nextInChain = &uwpDesc.chain;
            break;
        case Surface::Type::WindowsWinUISwapChainPanel:
            winuiDesc.chain.sType = WGPUSType_SurfaceDescriptorFromWindowsWinUISwapChainPanel;
            winuiDesc.swapChainPanel = surface->GetWinUISwapChainPanel();
            descriptor.nextInChain = &winuiDesc.chain;
            break;
        case Surface::Type::WaylandSurface:
            waylandDesc.chain.sType = WGPUSType_SurfaceSourceWaylandSurface;
            waylandDesc.display = surface->GetWaylandDisplay();
            waylandDesc.surface = surface->GetWaylandSurface();
            descriptor.nextInChain = &waylandDesc.chain;
            break;
        case Surface::Type::XlibWindow:
            xlibDesc.chain.sType = WGPUSType_SurfaceSourceXlibWindow;
            xlibDesc.display = surface->GetXDisplay();
            xlibDesc.window = surface->GetXWindow();
            descriptor.nextInChain = &xlibDesc.chain;
            break;
        default:
            return DAWN_VALIDATION_ERROR("Unknown surface type %s.", surface->GetType());
    }

    WGPUSurface innerSurface = procs.instanceCreateSurface(instance, &descriptor);
    DAWN_ASSERT(innerSurface);
    return innerSurface;
}

// static
ResultOrError<Ref<SwapChain>> SwapChain::Create(Device* device,
                                                Surface* surface,
                                                SwapChainBase* previousSwapChain,
                                                const SurfaceConfiguration* config) {
    WGPUSurface innerSurface = nullptr;

    if (previousSwapChain != nullptr) {
        // Transfer the inner surface ownership.
        SwapChain* backendSwapChain = ToBackend(previousSwapChain);
        innerSurface = backendSwapChain->mInnerHandle;
        backendSwapChain->mInnerHandle = nullptr;
    } else {
        DAWN_TRY_ASSIGN(innerSurface,
                        CreateWGPUSurface(*device->wgpu, device->GetInnerInstance(), surface));
    }
    DAWN_ASSERT(innerSurface);

    std::vector<WGPUTextureFormat> viewFormats;
    for (wgpu::TextureFormat viewFormat : config->viewFormats) {
        viewFormats.push_back(ToAPI(viewFormat));
    }

    WGPUSurfaceConfiguration innerConfig = {};
    innerConfig.device = device->GetInnerHandle();
    innerConfig.format = ToAPI(config->format);
    innerConfig.usage = ToAPI(config->usage);
    innerConfig.viewFormatCount = viewFormats.size();
    innerConfig.viewFormats = viewFormats.data();
    innerConfig.alphaMode = ToAPI(config->alphaMode);
    innerConfig.width = config->width;
    innerConfig.height = config->height;
    innerConfig.presentMode = ToAPI(config->presentMode);

    device->wgpu->surfaceConfigure(innerSurface, &innerConfig);

    if (ToBackend(device->GetQueue())->IsCapturing()) {
        CaptureContext* context = ToBackend(device->GetQueue())->GetCaptureContext();
        context->CaptureSurfaceConfigure(surface, config);
    }

    return AcquireRef(new SwapChain(device, surface, config, innerSurface));
}

SwapChain::SwapChain(Device* device,
                     Surface* surface,
                     const SurfaceConfiguration* config,
                     WGPUSurface innerSurface)
    : SwapChainBase(device, surface, config), ObjectWGPU(device->wgpu->surfaceRelease) {
    mInnerHandle = innerSurface;
}

SwapChain::~SwapChain() = default;

WGPUSurface SwapChain::GetInnerSurface() const {
    DAWN_ASSERT(mInnerHandle);
    return mInnerHandle;
}

ResultOrError<SwapChainTextureInfo> SwapChain::GetCurrentTextureImpl() {
    Device* device = ToBackend(GetDevice());
    WGPUSurfaceTexture innerSurfaceTexture = WGPU_SURFACE_TEXTURE_INIT;
    device->wgpu->surfaceGetCurrentTexture(mInnerHandle, &innerSurfaceTexture);

    SwapChainTextureInfo info;
    info.status = FromAPI(innerSurfaceTexture.status);

    if (info.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
        info.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
        DAWN_ASSERT(innerSurfaceTexture.texture == nullptr);
        return info;
    }

    TextureDescriptor desc = GetSwapChainBaseTextureDescriptor(this);

    // Note: We're creating a Texture wrapper around the inner handle.
    Ref<Texture> texture;
    UnpackedPtr<TextureDescriptor> unpacked = Unpack(&desc);
    texture = Texture::CreateFromSurfaceTexture(device, unpacked, innerSurfaceTexture);

    if (ToBackend(device->GetQueue())->IsCapturing()) {
        CaptureContext* context = ToBackend(device->GetQueue())->GetCaptureContext();
        DAWN_TRY(context->CaptureSurfaceGetCurrentTexture(GetSurface(), texture.Get()));
    }

    mCurrentTexture = texture;
    info.texture = std::move(texture);

    return info;
}

MaybeError SwapChain::PresentImpl() {
    Device* device = ToBackend(GetDevice());

    WGPUStatus status = device->wgpu->surfacePresent(mInnerHandle);

    DAWN_TRY(CheckWGPUSuccess(status, "surfacePresent"));

    if (ToBackend(device->GetQueue())->IsCapturing()) {
        CaptureContext* context = ToBackend(device->GetQueue())->GetCaptureContext();
        context->CaptureSurfacePresent(GetSurface());
    }

    DAWN_ASSERT(mCurrentTexture != nullptr);
    mCurrentTexture->Destroy();
    mCurrentTexture = nullptr;

    return {};
}

void SwapChain::DetachFromSurfaceImpl() {
    if (mCurrentTexture != nullptr) {
        mCurrentTexture->Destroy();
        mCurrentTexture = nullptr;
    }
}

}  // namespace dawn::native::webgpu
