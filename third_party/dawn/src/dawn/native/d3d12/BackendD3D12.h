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

#ifndef SRC_DAWN_NATIVE_D3D12_BACKENDD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_BACKENDD3D12_H_

#include <string>
#include <variant>
#include <vector>

#include "dawn/native/BackendConnection.h"

#include "dawn/native/d3d/BackendD3D.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class PlatformFunctions;

// DxcVersionInfo holds both DXC compiler (dxcompiler.dll) version and DXC validator (dxil.dll)
// version, which are not necessarily identical. Both are in uint64_t type, as the result of
// MakeDXCVersion.
struct DxcVersionInfo {
    uint64_t DxcCompilerVersion;
    uint64_t DxcValidatorVersion;
};

// If DXC version information is not available due to no DXC binary or error occurs when acquiring
// version, DxcUnavailable indicates the version information being unavailable and holds the
// detailed error information.
struct DxcUnavailable {
    std::string ErrorMessage;
};

class Backend final : public d3d::Backend {
  public:
    explicit Backend(InstanceBase* instance);

    MaybeError Initialize();
    MaybeError EnsureDxcLibrary();
    MaybeError EnsureDxcCompiler();
    MaybeError EnsureDxcValidator();
    ComPtr<IDxcLibrary> GetDxcLibrary() const;
    ComPtr<IDxcCompiler3> GetDxcCompiler() const;
    ComPtr<IDxcValidator> GetDxcValidator() const;

    // Return true if and only if DXC binary is available, and the DXC compiler and validator
    // version are validated to be no older than a specific minimum version, currently 1.6.
    bool IsDXCAvailable() const;

    // Return true if and only if mIsDXCAvailable is true, and the DXC compiler and validator
    // version are validated to be no older than the minimum version given in parameter.
    bool IsDXCAvailableAndVersionAtLeast(uint64_t minimumCompilerMajorVersion,
                                         uint64_t minimumCompilerMinorVersion,
                                         uint64_t minimumValidatorMajorVersion,
                                         uint64_t minimumValidatorMinorVersion) const;

    // Return the DXC version information cached in mDxcVersionInformation, assert that the version
    // information is valid. Must be called after ensuring `IsDXCAvailable()` return true.
    DxcVersionInfo GetDxcVersion() const;

    const PlatformFunctions* GetFunctions() const;

  protected:
    ResultOrError<Ref<PhysicalDeviceBase>> CreatePhysicalDeviceFromIDXGIAdapter(
        ComPtr<IDXGIAdapter> dxgiAdapter) override;

  private:
    // Acquiring DXC version information and store the result in mDxcVersionInfo. This function
    // should be called only once, during startup in `Initialize`.
    void AcquireDxcVersionInformation();

    ComPtr<IDxcLibrary> mDxcLibrary;
    ComPtr<IDxcCompiler3> mDxcCompiler;
    ComPtr<IDxcValidator> mDxcValidator;

    // DXC binaries and DXC version information are checked when start up in `Initialize`. There are
    // two possible states:
    //   1. The DXC binary is not available, or error occurs when checking the version information
    //      and therefore no DXC version information available, or the DXC version is lower than
    //      requested minimum and therefore DXC is not available, represented by DxcUnavailable
    //      in which a error message is held;
    //   3. The DXC version information is acquired successfully and validated not lower than
    //      requested minimum, stored in DxcVersionInfo.
    std::variant<DxcUnavailable, DxcVersionInfo> mDxcVersionInfo;

    using Base = d3d::Backend;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_BACKENDD3D12_H_
