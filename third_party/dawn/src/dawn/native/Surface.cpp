// Copyright 2020 the Dawn & Tint Authors
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

#include "dawn/native/Surface.h"

#include <memory>
#include <string>
#include <utility>

#include "dawn/common/Platform.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/SwapChain.h"
#include "dawn/native/Texture.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/native/utils/WGPUHelpers.h"

#if DAWN_PLATFORM_IS(WINDOWS)
#include "dawn/common/windows_with_undefs.h"
#endif  // DAWN_PLATFORM_IS(WINDOWS)

#if defined(DAWN_USE_WINDOWS_UI)
#include <windows.ui.core.h>
#include <windows.ui.xaml.controls.h>
#endif  // defined(DAWN_USE_WINDOWS_UI)

#if defined(DAWN_USE_X11)
#include "dawn/common/xlib_with_undefs.h"
#include "dawn/native/X11Functions.h"
#endif  // defined(DAWN_USE_X11)

namespace dawn::native {

absl::FormatConvertResult<absl::FormatConversionCharSet::kString> AbslFormatConvert(
    Surface::Type value,
    const absl::FormatConversionSpec& spec,
    absl::FormatSink* s) {
    switch (value) {
        case Surface::Type::AndroidWindow:
            s->Append("AndroidWindow");
            break;
        case Surface::Type::MetalLayer:
            s->Append("MetalLayer");
            break;
        case Surface::Type::WaylandSurface:
            s->Append("WaylandSurface");
            break;
        case Surface::Type::WindowsHWND:
            s->Append("WindowsHWND");
            break;
        case Surface::Type::WindowsCoreWindow:
            s->Append("WindowsCoreWindow");
            break;
        case Surface::Type::WindowsUWPSwapChainPanel:
            s->Append("WindowsUWPSwapChainPanel");
            break;
        case Surface::Type::WindowsWinUISwapChainPanel:
            s->Append("WindowsWinUISwapChainPanel");
            break;
        case Surface::Type::XlibWindow:
            s->Append("XlibWindow");
            break;
    }
    return {true};
}

#if defined(DAWN_ENABLE_BACKEND_METAL)
bool InheritsFromCAMetalLayer(void* obj);
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)

ResultOrError<UnpackedPtr<SurfaceDescriptor>> ValidateSurfaceDescriptor(
    InstanceBase* instance,
    const SurfaceDescriptor* rawDescriptor) {
    DAWN_INVALID_IF(rawDescriptor->nextInChain == nullptr,
                    "Surface cannot be created with %s. nextInChain is not specified.",
                    rawDescriptor);
    UnpackedPtr<SurfaceDescriptor> descriptor;
    DAWN_TRY_ASSIGN(descriptor, ValidateAndUnpack(rawDescriptor));

    wgpu::SType type;
    DAWN_TRY_ASSIGN(
        type, (descriptor.ValidateBranches<
                  Branch<SurfaceSourceAndroidNativeWindow>, Branch<SurfaceSourceMetalLayer>,
                  Branch<SurfaceSourceWindowsHWND>, Branch<SurfaceDescriptorFromWindowsCoreWindow>,
                  Branch<SurfaceDescriptorFromWindowsUWPSwapChainPanel>,
                  Branch<SurfaceDescriptorFromWindowsWinUISwapChainPanel>,
                  Branch<SurfaceSourceXlibWindow>, Branch<SurfaceSourceWaylandSurface>>()));
    switch (type) {
#if DAWN_PLATFORM_IS(ANDROID)
        case wgpu::SType::SurfaceSourceAndroidNativeWindow: {
            auto* subDesc = descriptor.Get<SurfaceSourceAndroidNativeWindow>();
            DAWN_ASSERT(subDesc != nullptr);
            DAWN_INVALID_IF(subDesc->window == nullptr, "Android window is not set.");
            return descriptor;
        }
#endif  // DAWN_PLATFORM_IS(ANDROID)
#if defined(DAWN_ENABLE_BACKEND_METAL)
        case wgpu::SType::SurfaceSourceMetalLayer: {
            auto* subDesc = descriptor.Get<SurfaceSourceMetalLayer>();
            DAWN_ASSERT(subDesc != nullptr);
            // Check that the layer is a CAMetalLayer (or a derived class).
            DAWN_INVALID_IF(!InheritsFromCAMetalLayer(subDesc->layer),
                            "Layer must be a CAMetalLayer");
            return descriptor;
        }
#endif  // defined(DAWN_ENABLE_BACKEND_METAL)
#if DAWN_PLATFORM_IS(WIN32)
        case wgpu::SType::SurfaceSourceWindowsHWND: {
            auto* subDesc = descriptor.Get<SurfaceSourceWindowsHWND>();
            DAWN_ASSERT(subDesc != nullptr);
            DAWN_INVALID_IF(IsWindow(static_cast<HWND>(subDesc->hwnd)) == 0, "Invalid HWND");
            return descriptor;
        }
#endif  // DAWN_PLATFORM_IS(WIN32)
#if defined(DAWN_USE_WINDOWS_UI)
        case wgpu::SType::SurfaceDescriptorFromWindowsCoreWindow: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromWindowsCoreWindow>();
            DAWN_ASSERT(subDesc != nullptr);
            // Validate the coreWindow by query for ICoreWindow interface
            ComPtr<ABI::Windows::UI::Core::ICoreWindow> coreWindow;
            DAWN_INVALID_IF(subDesc->coreWindow == nullptr ||
                                FAILED(static_cast<IUnknown*>(subDesc->coreWindow)
                                           ->QueryInterface(IID_PPV_ARGS(&coreWindow))),
                            "Invalid CoreWindow");
            return descriptor;
        }
        case wgpu::SType::SurfaceDescriptorFromWindowsUWPSwapChainPanel: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromWindowsUWPSwapChainPanel>();
            DAWN_ASSERT(subDesc != nullptr);
            // Validate the swapChainPanel by querying for ISwapChainPanel interface
            ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel> swapChainPanel;
            DAWN_INVALID_IF(subDesc->swapChainPanel == nullptr ||
                                FAILED(static_cast<IUnknown*>(subDesc->swapChainPanel)
                                           ->QueryInterface(IID_PPV_ARGS(&swapChainPanel))),
                            "Invalid SwapChainPanel");
            return descriptor;
        }
        case wgpu::SType::SurfaceDescriptorFromWindowsWinUISwapChainPanel: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromWindowsWinUISwapChainPanel>();
            DAWN_ASSERT(subDesc != nullptr);
            // Unfortunately, checking whether this is a valid
            // Microsoft.UI.Xaml.Controls.SwapChainPanel would require the WindowsAppSDK as a
            // dependency, which is not trivial. So we'll only check for nullptr here.
            DAWN_INVALID_IF(subDesc->swapChainPanel == nullptr, "SwapChainPanel is nullptr.");
            return descriptor;
        }
#endif  // defined(DAWN_USE_WINDOWS_UI)
#if defined(DAWN_USE_WAYLAND)
        case wgpu::SType::SurfaceSourceWaylandSurface: {
            auto* subDesc = descriptor.Get<SurfaceSourceWaylandSurface>();
            DAWN_ASSERT(subDesc != nullptr);
            // Unfortunately we can't check the validity of wayland objects. Only that they
            // aren't nullptr.
            DAWN_INVALID_IF(subDesc->display == nullptr, "Wayland display is nullptr.");
            DAWN_INVALID_IF(subDesc->surface == nullptr, "Wayland surface is nullptr.");
            return descriptor;
        }
#endif  // defined(DAWN_USE_WAYLAND)
#if defined(DAWN_USE_X11)
        case wgpu::SType::SurfaceSourceXlibWindow: {
            auto* subDesc = descriptor.Get<SurfaceSourceXlibWindow>();
            DAWN_ASSERT(subDesc != nullptr);
            // Check the validity of the window by calling a getter function on the window that
            // returns a status code. If the window is bad the call return a status of zero. We
            // need to set a temporary X11 error handler while doing this because the default
            // X11 error handler exits the program on any error.
            const X11Functions* x11 = instance->GetOrLoadX11Functions();
            DAWN_INVALID_IF(!x11->IsX11Loaded(), "Couldn't load libX11.");

            XErrorHandler oldErrorHandler =
                x11->xSetErrorHandler([](Display*, XErrorEvent*) { return 0; });
            XWindowAttributes attributes;
            int status =
                x11->xGetWindowAttributes(reinterpret_cast<Display*>(subDesc->display),
                                          static_cast<Window>(subDesc->window), &attributes);
            x11->xSetErrorHandler(oldErrorHandler);

            DAWN_INVALID_IF(status == 0, "Invalid X Window");
            return descriptor;
        }
#endif  // defined(DAWN_USE_X11)
        default:
            return DAWN_VALIDATION_ERROR("Unsupported sType (%s)", type);
    }
}

MaybeError ValidateSurfaceConfiguration(DeviceBase* device,
                                        const PhysicalDeviceSurfaceCapabilities& capabilities,
                                        const SurfaceConfiguration* config,
                                        const Surface* surface) {
    UnpackedPtr<SurfaceConfiguration> unpacked;
    DAWN_TRY_ASSIGN(unpacked, ValidateAndUnpack(config));

    DAWN_TRY(config->device->ValidateIsAlive());

    // Check that config matches capabilities, this implicitly validates the value of the enums.
    DAWN_INVALID_IF(!IsSubset(config->usage, capabilities.usages),
                    "Usages requested (%s) are not supported by the adapter (%s) which supports "
                    "only %s for this surface.",
                    config->usage, config->device->GetAdapter(), capabilities.usages);

    auto formatIt =
        std::find(capabilities.formats.begin(), capabilities.formats.end(), config->format);
    DAWN_INVALID_IF(formatIt == capabilities.formats.end(),
                    "Format (%s) is not supported by the adapter (%s) for this surface.",
                    config->format, config->device->GetAdapter());

    auto presentModeIt = std::find(capabilities.presentModes.begin(),
                                   capabilities.presentModes.end(), config->presentMode);
    DAWN_INVALID_IF(presentModeIt == capabilities.presentModes.end(),
                    "Present mode (%s) is not supported by the adapter (%s) for this surface.",
                    config->presentMode, config->device->GetAdapter());

    auto alphaModeIt = std::find(capabilities.alphaModes.begin(), capabilities.alphaModes.end(),
                                 config->alphaMode);
    DAWN_INVALID_IF(alphaModeIt == capabilities.alphaModes.end(),
                    "Alpha mode (%s) is not supported by the adapter (%s) for this surface.",
                    config->alphaMode, config->device->GetAdapter());

    // Validate the surface would produce valid textures.
    TextureDescriptor textureDesc;
    textureDesc.usage = config->usage;
    textureDesc.size = {config->width, config->height};
    textureDesc.format = config->format;
    textureDesc.dimension = wgpu::TextureDimension::e2D;
    textureDesc.viewFormatCount = config->viewFormatCount;
    textureDesc.viewFormats = config->viewFormats;

    UnpackedPtr<TextureDescriptor> unpackedTextureDesc;
    DAWN_TRY_ASSIGN(unpackedTextureDesc, ValidateAndUnpack(&textureDesc));
    DAWN_TRY_CONTEXT(ValidateTextureDescriptor(device, unpackedTextureDesc),
                     "validating the configuration of %s would produce valid textures", surface);

    return {};
}

class AdapterSurfaceCapCache {
  public:
    template <typename F>
    MaybeError WithAdapterCapabilities(AdapterBase* adapter, const Surface* surface, F f) {
        if (mCachedCapabilitiesAdapter.Promote().Get() != adapter) {
            const PhysicalDeviceBase* physicalDevice = adapter->GetPhysicalDevice();
            DAWN_TRY_ASSIGN(mCachedCapabilities, physicalDevice->GetSurfaceCapabilities(
                                                     adapter->GetInstance(), surface));
            mCachedCapabilitiesAdapter = GetWeakRef(adapter);
        }
        return f(mCachedCapabilities);
    }

  private:
    WeakRef<AdapterBase> mCachedCapabilitiesAdapter = nullptr;
    PhysicalDeviceSurfaceCapabilities mCachedCapabilities;
};

// static
Ref<Surface> Surface::MakeError(InstanceBase* instance) {
    return AcquireRef(new Surface(instance, ErrorMonad::kError));
}

Surface::Surface(InstanceBase* instance, ErrorTag tag) : ErrorMonad(tag), mInstance(instance) {}

Surface::Surface(InstanceBase* instance, const UnpackedPtr<SurfaceDescriptor>& descriptor)
    : ErrorMonad(),
      mInstance(instance),
      mCapabilityCache(std::make_unique<AdapterSurfaceCapCache>()) {
    mLabel = std::string(descriptor->label);

    // Type is validated in validation, otherwise this may crash with an assert failure.
    wgpu::SType type =
        descriptor
            .ValidateBranches<
                Branch<SurfaceSourceAndroidNativeWindow>, Branch<SurfaceSourceMetalLayer>,
                Branch<SurfaceSourceWindowsHWND>, Branch<SurfaceDescriptorFromWindowsCoreWindow>,
                Branch<SurfaceDescriptorFromWindowsUWPSwapChainPanel>,
                Branch<SurfaceDescriptorFromWindowsWinUISwapChainPanel>,
                Branch<SurfaceSourceXlibWindow>, Branch<SurfaceSourceWaylandSurface>>()
            .AcquireSuccess();
    switch (type) {
        case wgpu::SType::SurfaceSourceAndroidNativeWindow: {
            auto* subDesc = descriptor.Get<SurfaceSourceAndroidNativeWindow>();
            mType = Type::AndroidWindow;
            mAndroidNativeWindow = subDesc->window;
            break;
        }
        case wgpu::SType::SurfaceSourceMetalLayer: {
            auto* subDesc = descriptor.Get<SurfaceSourceMetalLayer>();
            mType = Type::MetalLayer;
            mMetalLayer = subDesc->layer;
            break;
        }
        case wgpu::SType::SurfaceSourceWindowsHWND: {
            auto* subDesc = descriptor.Get<SurfaceSourceWindowsHWND>();
            mType = Type::WindowsHWND;
            mHInstance = subDesc->hinstance;
            mHWND = subDesc->hwnd;
            break;
        }
#if defined(DAWN_USE_WINDOWS_UI)
        case wgpu::SType::SurfaceDescriptorFromWindowsCoreWindow: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromWindowsCoreWindow>();
            mType = Type::WindowsCoreWindow;
            mCoreWindow = static_cast<IUnknown*>(subDesc->coreWindow);
            break;
        }
        case wgpu::SType::SurfaceDescriptorFromWindowsUWPSwapChainPanel: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromWindowsUWPSwapChainPanel>();
            mType = Type::WindowsUWPSwapChainPanel;
            mUWPSwapChainPanel = static_cast<IUnknown*>(subDesc->swapChainPanel);
            break;
        }
        case wgpu::SType::SurfaceDescriptorFromWindowsWinUISwapChainPanel: {
            auto* subDesc = descriptor.Get<SurfaceDescriptorFromWindowsWinUISwapChainPanel>();
            mType = Type::WindowsWinUISwapChainPanel;
            mWinUISwapChainPanel = static_cast<IUnknown*>(subDesc->swapChainPanel);
            break;
        }
#endif  // defined(DAWN_USE_WINDOWS_UI)
        case wgpu::SType::SurfaceSourceWaylandSurface: {
            auto* subDesc = descriptor.Get<SurfaceSourceWaylandSurface>();
            mType = Type::WaylandSurface;
            mWaylandDisplay = subDesc->display;
            mWaylandSurface = subDesc->surface;
            break;
        }
        case wgpu::SType::SurfaceSourceXlibWindow: {
            auto* subDesc = descriptor.Get<SurfaceSourceXlibWindow>();
            mType = Type::XlibWindow;
            mXDisplay = subDesc->display;
            mXWindow = subDesc->window;
            break;
        }
        default:
            DAWN_UNREACHABLE();
    }
}

Surface::~Surface() {
    if (mSwapChain != nullptr) {
        [[maybe_unused]] bool error = mInstance->ConsumedError(Unconfigure());
    }

    if (mRecycledSwapChain != nullptr) {
        mRecycledSwapChain->DetachFromSurface();
        mRecycledSwapChain = nullptr;
    }
}

InstanceBase* Surface::GetInstance() const {
    return mInstance.Get();
}

DeviceBase* Surface::GetCurrentDevice() const {
    return mCurrentDevice.Get();
}

Surface::Type Surface::GetType() const {
    DAWN_ASSERT(!IsError());
    return mType;
}

void* Surface::GetAndroidNativeWindow() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mType == Type::AndroidWindow);
    return mAndroidNativeWindow;
}

void* Surface::GetMetalLayer() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mType == Type::MetalLayer);
    return mMetalLayer;
}

void* Surface::GetWaylandDisplay() const {
    DAWN_ASSERT(mType == Type::WaylandSurface);
    return mWaylandDisplay;
}

void* Surface::GetWaylandSurface() const {
    DAWN_ASSERT(mType == Type::WaylandSurface);
    return mWaylandSurface;
}

void* Surface::GetHInstance() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mType == Type::WindowsHWND);
    return mHInstance;
}
void* Surface::GetHWND() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mType == Type::WindowsHWND);
    return mHWND;
}

IUnknown* Surface::GetCoreWindow() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mType == Type::WindowsCoreWindow);
#if defined(DAWN_USE_WINDOWS_UI)
    return mCoreWindow.Get();
#else
    return nullptr;
#endif
}

IUnknown* Surface::GetUWPSwapChainPanel() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mType == Type::WindowsUWPSwapChainPanel);
#if defined(DAWN_USE_WINDOWS_UI)
    return mUWPSwapChainPanel.Get();
#else
    return nullptr;
#endif
}

IUnknown* Surface::GetWinUISwapChainPanel() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mType == Type::WindowsWinUISwapChainPanel);
#if defined(DAWN_USE_WINDOWS_UI)
    return mWinUISwapChainPanel.Get();
#else
    return nullptr;
#endif
}

void* Surface::GetXDisplay() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mType == Type::XlibWindow);
    return mXDisplay;
}
uint32_t Surface::GetXWindow() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mType == Type::XlibWindow);
    return mXWindow;
}

MaybeError Surface::Configure(const SurfaceConfiguration* configIn) {
    DAWN_INVALID_IF(IsError(), "%s is invalid.", this);

    SurfaceConfiguration config = *configIn;
    mCurrentDevice = config.device;  // next errors are routed to the new device

    DAWN_TRY(mCapabilityCache->WithAdapterCapabilities(
        GetCurrentDevice()->GetAdapter(), this,
        [&](const PhysicalDeviceSurfaceCapabilities& caps) -> MaybeError {
            // The auto alphaMode default to alphaModes[0].
            if (config.alphaMode == wgpu::CompositeAlphaMode::Auto) {
                config.alphaMode = caps.alphaModes[0];
            }

            return ValidateSurfaceConfiguration(GetCurrentDevice(), caps, &config, this);
        }));

    // Reuse either the current swapchain, or the recycled swap chain
    SwapChainBase* previousSwapChain = mSwapChain.Get();
    if (previousSwapChain == nullptr && mRecycledSwapChain != nullptr &&
        mRecycledSwapChain->GetDevice() == config.device) {
        previousSwapChain = mRecycledSwapChain.Get();
    }

    {
        auto deviceLock(GetCurrentDevice()->GetScopedLock());
        ResultOrError<Ref<SwapChainBase>> maybeNewSwapChain =
            GetCurrentDevice()->CreateSwapChain(this, previousSwapChain, &config);

        // Don't keep swap chains older than 1 call to Configure
        if (mRecycledSwapChain) {
            mRecycledSwapChain->DetachFromSurface();
            mRecycledSwapChain = nullptr;
        }

        if (mSwapChain && maybeNewSwapChain.IsSuccess()) {
            mSwapChain->DetachFromSurface();
        }
        DAWN_TRY_ASSIGN(mSwapChain, std::move(maybeNewSwapChain));

        mSwapChain->SetIsAttached();
    }

    return {};
}

MaybeError Surface::Unconfigure() {
    DAWN_INVALID_IF(IsError(), "%s is invalid.", this);
    DAWN_INVALID_IF(!mSwapChain.Get(), "%s is not configured.", this);

    if (mSwapChain != nullptr) {
        if (mRecycledSwapChain != nullptr) {
            mRecycledSwapChain->DetachFromSurface();
            mRecycledSwapChain = nullptr;
        }
        mRecycledSwapChain = mSwapChain;
        mSwapChain = nullptr;
    }

    return {};
}

MaybeError Surface::GetCapabilities(AdapterBase* adapter, SurfaceCapabilities* capabilities) const {
    DAWN_INVALID_IF(IsError(), "%s is invalid.", this);

    DAWN_TRY(mCapabilityCache->WithAdapterCapabilities(
        adapter, this,
        [&capabilities](const PhysicalDeviceSurfaceCapabilities& caps) -> MaybeError {
            capabilities->nextInChain = nullptr;
            capabilities->usages = caps.usages;
            utils::AllocateApiSeqFromStdVector(&capabilities->formats, &capabilities->formatCount,
                                               caps.formats);
            utils::AllocateApiSeqFromStdVector(&capabilities->presentModes,
                                               &capabilities->presentModeCount, caps.presentModes);
            utils::AllocateApiSeqFromStdVector(&capabilities->alphaModes,
                                               &capabilities->alphaModeCount, caps.alphaModes);
            return {};
        }));

    return {};
}

void APISurfaceCapabilitiesFreeMembers(WGPUSurfaceCapabilities capabilities) {
    utils::FreeApiSeq(&capabilities.formats, &capabilities.formatCount);
    utils::FreeApiSeq(&capabilities.presentModes, &capabilities.presentModeCount);
    utils::FreeApiSeq(&capabilities.alphaModes, &capabilities.alphaModeCount);
}

MaybeError Surface::GetCurrentTexture(SurfaceTexture* surfaceTexture) const {
    surfaceTexture->texture = nullptr;

    // Set an error status that will be overwritten if there is a success or some other more
    // specific error.
    surfaceTexture->status = wgpu::SurfaceGetCurrentTextureStatus::Error;
    DAWN_INVALID_IF(IsError(), "%s is invalid.", this);
    DAWN_INVALID_IF(!mSwapChain.Get(), "%s is not configured.", this);

    if (GetCurrentDevice()->IsLost()) {
        TextureDescriptor textureDesc = GetSwapChainBaseTextureDescriptor(mSwapChain.Get());
        surfaceTexture->status = wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal;
        surfaceTexture->texture =
            ReturnToAPI(TextureBase::MakeError(GetCurrentDevice(), &textureDesc));
        return {};
    }

    auto deviceLock(GetCurrentDevice()->GetScopedLock());
    DAWN_TRY_ASSIGN(*surfaceTexture, mSwapChain->GetCurrentTexture());

    return {};
}

MaybeError Surface::Present() {
    DAWN_INVALID_IF(IsError(), "%s is invalid.", this);
    DAWN_INVALID_IF(!mSwapChain.Get(), "%s is not configured.", this);

    auto deviceLock(GetCurrentDevice()->GetScopedLock());
    DAWN_TRY(mSwapChain->Present());

    return {};
}

const std::string& Surface::GetLabel() const {
    return mLabel;
}

void Surface::APIConfigure(const SurfaceConfiguration* config) {
    MaybeError maybeError = Configure(config);
    if (!GetCurrentDevice()) {
        [[maybe_unused]] bool error = mInstance->ConsumedError(std::move(maybeError));
    } else {
        [[maybe_unused]] bool error = GetCurrentDevice()->ConsumedError(
            std::move(maybeError), "calling %s.Configure().", this);
    }
}

wgpu::Status Surface::APIGetCapabilities(AdapterBase* adapter,
                                         SurfaceCapabilities* capabilities) const {
    MaybeError maybeError = GetCapabilities(adapter, capabilities);
    if (!GetCurrentDevice()) {
        return mInstance->ConsumedError(std::move(maybeError)) ? wgpu::Status::Error
                                                               : wgpu::Status::Success;
    } else {
        return GetCurrentDevice()->ConsumedError(std::move(maybeError), "calling %s.Configure().",
                                                 this)
                   ? wgpu::Status::Error
                   : wgpu::Status::Success;
    }
}

void Surface::APIGetCurrentTexture(SurfaceTexture* surfaceTexture) const {
    MaybeError maybeError = GetCurrentTexture(surfaceTexture);
    if (!GetCurrentDevice()) {
        [[maybe_unused]] bool error = mInstance->ConsumedError(std::move(maybeError));
    } else {
        [[maybe_unused]] bool error = GetCurrentDevice()->ConsumedError(std::move(maybeError));
    }
}

void Surface::APIPresent() {
    MaybeError maybeError = Present();
    if (!GetCurrentDevice()) {
        [[maybe_unused]] bool error = mInstance->ConsumedError(std::move(maybeError));
    } else {
        [[maybe_unused]] bool error = GetCurrentDevice()->ConsumedError(std::move(maybeError));
    }
}

void Surface::APIUnconfigure() {
    MaybeError maybeError = Unconfigure();
    if (!GetCurrentDevice()) {
        [[maybe_unused]] bool error = mInstance->ConsumedError(std::move(maybeError));
    } else {
        [[maybe_unused]] bool error = GetCurrentDevice()->ConsumedError(std::move(maybeError));
    }
}

void Surface::APISetLabel(StringView label) {
    mLabel = utils::NormalizeMessageString(label);
}

}  // namespace dawn::native
