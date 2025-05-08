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

#ifndef SRC_DAWN_NATIVE_SURFACE_H_
#define SRC_DAWN_NATIVE_SURFACE_H_

#include <memory>
#include <string>

#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"
#include "partition_alloc/pointers/raw_ptr.h"

#include "dawn/native/dawn_platform.h"

#include "dawn/common/Platform.h"

#if defined(DAWN_USE_WINDOWS_UI)
#include "dawn/native/d3d/d3d_platform.h"
#endif  // defined(DAWN_USE_WINDOWS_UI)

// Forward declare IUnknown
// GetCoreWindow needs to return an IUnknown pointer
// non-windows platforms don't have this type
struct IUnknown;

namespace dawn::native {

struct PhysicalDeviceSurfaceCapabilities;

// Adapter surface capabilities are cached by the surface
class AdapterSurfaceCapCache;

ResultOrError<UnpackedPtr<SurfaceDescriptor>> ValidateSurfaceDescriptor(
    InstanceBase* instance,
    const SurfaceDescriptor* rawDescriptor);

MaybeError ValidateSurfaceConfiguration(DeviceBase* device,
                                        const PhysicalDeviceSurfaceCapabilities& capabilities,
                                        const SurfaceConfiguration* config,
                                        const Surface* surface);

// A surface is a sum types of all the kind of windows Dawn supports. The OS-specific types
// aren't used because they would cause compilation errors on other OSes (or require
// ObjectiveC).
// The surface is also used to store the current swapchain so that we can detach it when it is
// replaced.
class Surface final : public ErrorMonad {
  public:
    static Ref<Surface> MakeError(InstanceBase* instance);

    Surface(InstanceBase* instance, const UnpackedPtr<SurfaceDescriptor>& descriptor);

    // These are valid to call on all Surfaces.
    enum class Type {
        AndroidWindow,
        MetalLayer,
        WaylandSurface,
        WindowsHWND,
        WindowsCoreWindow,
        WindowsUWPSwapChainPanel,
        WindowsWinUISwapChainPanel,
        XlibWindow,
    };
    Type GetType() const;
    InstanceBase* GetInstance() const;
    DeviceBase* GetCurrentDevice() const;

    // Valid to call if the type is MetalLayer
    void* GetMetalLayer() const;

    // Valid to call if the type is Android
    void* GetAndroidNativeWindow() const;

    // Valid to call if the type is WaylandSurface
    void* GetWaylandDisplay() const;
    void* GetWaylandSurface() const;

    // Valid to call if the type is WindowsHWND
    void* GetHInstance() const;
    void* GetHWND() const;

    // Valid to call if the type is WindowsCoreWindow
    IUnknown* GetCoreWindow() const;

    // Valid to call if the type is WindowsUWPSwapChainPanel
    IUnknown* GetUWPSwapChainPanel() const;

    // Valid to call if the type is WindowsWinUISwapChainPanel
    IUnknown* GetWinUISwapChainPanel() const;

    // Valid to call if the type is WindowsXlib
    void* GetXDisplay() const;
    uint32_t GetXWindow() const;

    const std::string& GetLabel() const;

    // Dawn API
    void APIConfigure(const SurfaceConfiguration* config);
    wgpu::Status APIGetCapabilities(AdapterBase* adapter, SurfaceCapabilities* capabilities) const;
    void APIGetCurrentTexture(SurfaceTexture* surfaceTexture) const;
    void APIPresent();
    void APIUnconfigure();
    void APISetLabel(StringView label);

  private:
    Surface(InstanceBase* instance, ErrorMonad::ErrorTag tag);
    ~Surface() override;

    MaybeError Configure(const SurfaceConfiguration* config);
    MaybeError Unconfigure();

    MaybeError GetCapabilities(AdapterBase* adapter, SurfaceCapabilities* capabilities) const;
    MaybeError GetCurrentTexture(SurfaceTexture* surfaceTexture) const;
    MaybeError Present();

    Ref<InstanceBase> mInstance;
    Type mType;
    std::string mLabel;

    // The surface has an associated device only when it is configured
    Ref<DeviceBase> mCurrentDevice;

    // The swapchain is created when configuring the surface.
    Ref<SwapChainBase> mSwapChain;

    // We keep on storing the previous swap chain after Unconfigure in case we could reuse it
    Ref<SwapChainBase> mRecycledSwapChain;

    // A cache is mutable because potentially modified in const-qualified getters
    std::unique_ptr<AdapterSurfaceCapCache> mCapabilityCache;

    // MetalLayer
    raw_ptr<void> mMetalLayer = nullptr;

    // ANativeWindow
    raw_ptr<void> mAndroidNativeWindow = nullptr;

    // Wayland
    raw_ptr<void> mWaylandDisplay = nullptr;
    raw_ptr<void> mWaylandSurface = nullptr;

    // WindowsHwnd
    raw_ptr<void> mHInstance = nullptr;
    raw_ptr<void> mHWND = nullptr;

#if defined(DAWN_USE_WINDOWS_UI)
    // WindowsCoreWindow
    ComPtr<IUnknown> mCoreWindow;

    // WindowsUWPSwapChainPanel
    ComPtr<IUnknown> mUWPSwapChainPanel;

    // WindowsWinUISwapChainPanel
    ComPtr<IUnknown> mWinUISwapChainPanel;
#endif  // defined(DAWN_USE_WINDOWS_UI)

    // Xlib
    raw_ptr<void> mXDisplay = nullptr;
    uint32_t mXWindow = 0;
};

// Not defined in webgpu_absl_format.h/cpp because you can't forward-declare a nested type.
absl::FormatConvertResult<absl::FormatConversionCharSet::kString>
AbslFormatConvert(Surface::Type value, const absl::FormatConversionSpec& spec, absl::FormatSink* s);

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SURFACE_H_
