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

namespace {

uint64_t MakeDXCVersion(uint64_t majorVersion, uint64_t minorVersion) {
    return (majorVersion << 32) + minorVersion;
}

}  // anonymous namespace

Backend::Backend(InstanceBase* instance) : Base(instance, wgpu::BackendType::D3D12) {}

MaybeError Backend::Initialize() {
    {
        // Put function initialization in curly braces to avoid the temptation to use the
        // std::move-ed `functions` variable later in the method.
        auto functions = std::make_unique<PlatformFunctions>();
        DAWN_TRY(functions->LoadFunctions());

        DAWN_TRY(Base::Initialize(std::move(functions)));
    }

    // Check if DXC is available and cache DXC version information
    if (!GetFunctions()->IsDXCBinaryAvailable()) {
        // DXC version information is not available if DXC binaries are not available.
        mDxcVersionInfo = DxcUnavailable{"DXC binary is not available"};
    } else {
        // Check the DXC version information and validate them being not lower than pre-defined
        // minimum version.
        AcquireDxcVersionInformation();

        // Check that DXC version information is acquired successfully.
        if (std::holds_alternative<DxcVersionInfo>(mDxcVersionInfo)) {
            const DxcVersionInfo& dxcVersionInfo = std::get<DxcVersionInfo>(mDxcVersionInfo);

            // The required minimum version for DXC compiler and validator.
            // Notes about requirement consideration:
            //   * DXC version 1.4 has some known issues when compiling Tint generated HLSL program,
            //   please
            //     refer to crbug.com/tint/1719
            //   * Windows SDK 20348 provides DXC compiler and validator version 1.6
            // Here the minimum version requirement for DXC compiler and validator are both set
            // to 1.6.
            constexpr uint64_t minimumCompilerMajorVersion = 1;
            constexpr uint64_t minimumCompilerMinorVersion = 6;
            constexpr uint64_t minimumValidatorMajorVersion = 1;
            constexpr uint64_t minimumValidatorMinorVersion = 6;

            // Check that DXC compiler and validator version are not lower than minimum.
            if (dxcVersionInfo.DxcCompilerVersion <
                    MakeDXCVersion(minimumCompilerMajorVersion, minimumCompilerMinorVersion) ||
                dxcVersionInfo.DxcValidatorVersion <
                    MakeDXCVersion(minimumValidatorMajorVersion, minimumValidatorMinorVersion)) {
                // If DXC version is lower than required minimum, set mDxcVersionInfo to
                // DxcUnavailable to indicate that DXC is not available.
                std::ostringstream ss;
                ss << "DXC version too low: dxil.dll required version 1.6, actual version "
                   << (dxcVersionInfo.DxcValidatorVersion >> 32) << "."
                   << (dxcVersionInfo.DxcValidatorVersion & ((uint64_t(1) << 32) - 1))
                   << ", dxcompiler.dll required version 1.6, actual version "
                   << (dxcVersionInfo.DxcCompilerVersion >> 32) << "."
                   << (dxcVersionInfo.DxcCompilerVersion & ((uint64_t(1) << 32) - 1));
                mDxcVersionInfo = DxcUnavailable{ss.str()};
            }
        }
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

#ifdef DAWN_USE_BUILT_DXC
    DAWN_INVALID_IF(!IsDXCAvailable(), "DXC dlls were built, but are not available");
#endif

    return {};
}

const PlatformFunctions* Backend::GetFunctions() const {
    return static_cast<const PlatformFunctions*>(Base::GetFunctions());
}

MaybeError Backend::EnsureDxcLibrary() {
    if (mDxcLibrary == nullptr) {
        DAWN_TRY(CheckHRESULT(
            GetFunctions()->dxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&mDxcLibrary)),
            "DXC create library"));
        DAWN_ASSERT(mDxcLibrary != nullptr);
    }
    return {};
}

MaybeError Backend::EnsureDxcCompiler() {
    if (mDxcCompiler == nullptr) {
        DAWN_TRY(CheckHRESULT(
            GetFunctions()->dxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&mDxcCompiler)),
            "DXC create compiler"));
        DAWN_ASSERT(mDxcCompiler != nullptr);
    }
    return {};
}

MaybeError Backend::EnsureDxcValidator() {
    if (mDxcValidator == nullptr) {
        DAWN_TRY(CheckHRESULT(
            GetFunctions()->dxcCreateInstance(CLSID_DxcValidator, IID_PPV_ARGS(&mDxcValidator)),
            "DXC create validator"));
        DAWN_ASSERT(mDxcValidator != nullptr);
    }
    return {};
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

void Backend::AcquireDxcVersionInformation() {
    DAWN_ASSERT(std::holds_alternative<DxcUnavailable>(mDxcVersionInfo));

    auto tryAcquireDxcVersionInfo = [this]() -> ResultOrError<DxcVersionInfo> {
        DAWN_TRY(EnsureDxcValidator());
        DAWN_TRY(EnsureDxcCompiler());

        ComPtr<IDxcVersionInfo> compilerVersionInfo;

        DAWN_TRY(CheckHRESULT(mDxcCompiler.As(&compilerVersionInfo),
                              "D3D12 QueryInterface IDxcCompiler3 to IDxcVersionInfo"));
        uint32_t compilerMajor, compilerMinor;
        DAWN_TRY(CheckHRESULT(compilerVersionInfo->GetVersion(&compilerMajor, &compilerMinor),
                              "IDxcVersionInfo::GetVersion"));

        ComPtr<IDxcVersionInfo> validatorVersionInfo;

        DAWN_TRY(CheckHRESULT(mDxcValidator.As(&validatorVersionInfo),
                              "D3D12 QueryInterface IDxcValidator to IDxcVersionInfo"));
        uint32_t validatorMajor, validatorMinor;
        DAWN_TRY(CheckHRESULT(validatorVersionInfo->GetVersion(&validatorMajor, &validatorMinor),
                              "IDxcVersionInfo::GetVersion"));

        // Pack major and minor version number into a single version number.
        uint64_t compilerVersion = MakeDXCVersion(compilerMajor, compilerMinor);
        uint64_t validatorVersion = MakeDXCVersion(validatorMajor, validatorMinor);
        return DxcVersionInfo{compilerVersion, validatorVersion};
    };

    auto dxcVersionInfoOrError = tryAcquireDxcVersionInfo();

    if (dxcVersionInfoOrError.IsSuccess()) {
        // Cache the DXC version information.
        mDxcVersionInfo = dxcVersionInfoOrError.AcquireSuccess();
    } else {
        // Error occurs when acquiring DXC version information, set the cache to unavailable and
        // record the error message.
        std::string errorMessage = dxcVersionInfoOrError.AcquireError()->GetFormattedMessage();
        dawn::ErrorLog() << errorMessage;
        mDxcVersionInfo = DxcUnavailable{errorMessage};
    }
}

// Return both DXC compiler and DXC validator version, assert that DXC version information is
// acquired successfully.
DxcVersionInfo Backend::GetDxcVersion() const {
    DAWN_ASSERT(std::holds_alternative<DxcVersionInfo>(mDxcVersionInfo));
    return DxcVersionInfo(std::get<DxcVersionInfo>(mDxcVersionInfo));
}

// Return true if and only if DXC binary is available, and the DXC version is validated to
// be no older than a pre-defined minimum version.
bool Backend::IsDXCAvailable() const {
    // mDxcVersionInfo hold DxcVersionInfo instead of DxcUnavailable if and only if DXC binaries and
    // version are validated in `Initialize`.
    return std::holds_alternative<DxcVersionInfo>(mDxcVersionInfo);
}

// Return true if and only if IsDXCAvailable() return true, and the DXC compiler and validator
// version are validated to be no older than the minimum version given in parameter.
bool Backend::IsDXCAvailableAndVersionAtLeast(uint64_t minimumCompilerMajorVersion,
                                              uint64_t minimumCompilerMinorVersion,
                                              uint64_t minimumValidatorMajorVersion,
                                              uint64_t minimumValidatorMinorVersion) const {
    // mDxcVersionInfo hold DxcVersionInfo instead of DxcUnavailable if and only if DXC binaries and
    // version are validated in `Initialize`.
    if (std::holds_alternative<DxcVersionInfo>(mDxcVersionInfo)) {
        const DxcVersionInfo& dxcVersionInfo = std::get<DxcVersionInfo>(mDxcVersionInfo);
        // Check that DXC compiler and validator version are not lower than given requirements.
        if (dxcVersionInfo.DxcCompilerVersion >=
                MakeDXCVersion(minimumCompilerMajorVersion, minimumCompilerMinorVersion) &&
            dxcVersionInfo.DxcValidatorVersion >=
                MakeDXCVersion(minimumValidatorMajorVersion, minimumValidatorMinorVersion)) {
            return true;
        }
    }
    return false;
}

ResultOrError<Ref<PhysicalDeviceBase>> Backend::CreatePhysicalDeviceFromIDXGIAdapter(
    ComPtr<IDXGIAdapter> dxgiAdapter) {
    // IDXGIAdapter4 is supported since Windows 8 and Platform Update for Windows 7.
    ComPtr<IDXGIAdapter4> dxgiAdapter4;
    DAWN_TRY(CheckHRESULT(dxgiAdapter.As(&dxgiAdapter4), "DXGIAdapter retrieval"));
    Ref<PhysicalDevice> physicalDevice =
        AcquireRef(new PhysicalDevice(this, std::move(dxgiAdapter4)));
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
