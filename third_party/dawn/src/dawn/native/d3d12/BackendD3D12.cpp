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

#include "dawn/native/d3d12/BackendD3D12.h"

#include <memory>
#include <utility>

#include "dawn/common/Log.h"
#include "dawn/native/D3D12Backend.h"
#include "dawn/native/Instance.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/PhysicalDeviceD3D12.h"
#include "dawn/native/d3d12/PlatformFunctionsD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"

namespace dawn::native::d3d12 {

Backend::Backend(InstanceBase* instance) : Base(instance, wgpu::BackendType::D3D12) {}

MaybeError Backend::Initialize() {
    {
        // Put function initialization in curly braces to avoid the temptation to use the
        // std::move-ed `functions` variable later in the method.
        auto functions = std::make_unique<PlatformFunctions>();
        DAWN_TRY(functions->Initialize(GetInstance()->GetRuntimeSearchPaths()));

        DAWN_TRY(Base::Initialize(std::move(functions)));
    }

    // Enable the debug layer (requires the Graphics Tools "optional feature").
    const auto instance = GetInstance();
    if (instance->GetBackendValidationLevel() != BackendValidationLevel::Disabled) {
        ComPtr<ID3D12Debug3> debugController;
        if (SUCCEEDED(GetFunctions()->d3d12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            DAWN_ASSERT(debugController != nullptr);
            debugController->EnableDebugLayer();
            if (instance->GetBackendValidationLevel() == BackendValidationLevel::Full) {
                debugController->SetEnableGPUBasedValidation(true);
            }
        }
    }

    if (instance->IsBeginCaptureOnStartupEnabled()) {
        ComPtr<IDXGraphicsAnalysis> graphicsAnalysis;
        if (GetFunctions()->dxgiGetDebugInterface1 != nullptr &&
            SUCCEEDED(GetFunctions()->dxgiGetDebugInterface1(0, IID_PPV_ARGS(&graphicsAnalysis)))) {
            graphicsAnalysis->BeginCapture();
        }
    }

    return {};
}

const PlatformFunctions* Backend::GetFunctions() const {
    return static_cast<const PlatformFunctions*>(Base::GetFunctions());
}

MaybeError Backend::EnsureDXC() {
#if DAWN_USE_BUILT_DXC
    // If components are already loaded, return early
    if (mDxcLibrary != nullptr) {
        // Since all components are assigned atomically, if one is loaded, all should be loaded
        DAWN_CHECK(mDxcCompiler);
        DAWN_CHECK(mDxcValidator);
        return {};
    }

    DAWN_TRY(const_cast<PlatformFunctions*>(GetFunctions())
                 ->EnsureDXCLibraries(GetInstance()->GetRuntimeSearchPaths()));

    ComPtr<IDxcLibrary> dxcLibrary;
    DAWN_TRY(
        CheckHRESULT(GetFunctions()->dxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&dxcLibrary)),
                     "DXC create library"));

    ComPtr<IDxcCompiler3> dxcCompiler;
    DAWN_TRY(CheckHRESULT(
        GetFunctions()->dxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler)),
        "DXC create compiler"));

    ComPtr<IDxcValidator> dxcValidator;
    DAWN_TRY(CheckHRESULT(
        GetFunctions()->dxcCreateInstance(CLSID_DxcValidator, IID_PPV_ARGS(&dxcValidator)),
        "DXC create validator"));

    mDxcLibrary = std::move(dxcLibrary);
    mDxcCompiler = std::move(dxcCompiler);
    mDxcValidator = std::move(dxcValidator);

    return {};
#else
    DAWN_UNREACHABLE();
#endif
}

ComPtr<IDxcLibrary> Backend::GetDxcLibrary() const {
    DAWN_ASSERT(mDxcLibrary != nullptr);
    return mDxcLibrary;
}

ComPtr<IDxcCompiler3> Backend::GetDxcCompiler() const {
    DAWN_ASSERT(mDxcCompiler != nullptr);
    return mDxcCompiler;
}

ComPtr<IDxcValidator> Backend::GetDxcValidator() const {
    DAWN_ASSERT(mDxcValidator != nullptr);
    return mDxcValidator;
}

ResultOrError<Ref<PhysicalDeviceBase>> Backend::CreatePhysicalDeviceFromIDXGIAdapter(
    ComPtr<IDXGIAdapter> dxgiAdapter) {
    ComPtr<IDXGIAdapter3> dxgiAdapter3;
    DAWN_TRY(CheckHRESULT(dxgiAdapter.As(&dxgiAdapter3), "DXGIAdapter retrieval"));
    Ref<PhysicalDevice> physicalDevice =
        AcquireRef(new PhysicalDevice(this, std::move(dxgiAdapter3)));
    DAWN_TRY(physicalDevice->Initialize());

    return {std::move(physicalDevice)};
}

BackendConnection* Connect(InstanceBase* instance) {
    Backend* backend = new Backend(instance);

    if (instance->ConsumedError(backend->Initialize())) {
        delete backend;
        return nullptr;
    }

    return backend;
}

}  // namespace dawn::native::d3d12
