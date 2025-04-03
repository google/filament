// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/d3d12/PlatformFunctionsD3D12.h"

#include <array>
#include <sstream>
#include <string>
#include <utility>

namespace dawn::native::d3d12 {

namespace {

// Extract Version from "10.0.{Version}.0" if possible, otherwise return 0.
uint32_t GetWindowsSDKVersionFromDirectoryName(const char* directoryName) {
    constexpr char kPrefix[] = "10.0.";
    constexpr char kPostfix[] = ".0";

    constexpr uint32_t kPrefixLen = sizeof(kPrefix) - 1;
    constexpr uint32_t kPostfixLen = sizeof(kPostfix) - 1;
    const uint32_t directoryNameLen = strlen(directoryName);

    if (directoryNameLen < kPrefixLen + kPostfixLen + 1) {
        return 0;
    }

    // Check if directoryName starts with "10.0.".
    if (strncmp(directoryName, kPrefix, kPrefixLen) != 0) {
        return 0;
    }

    // Check if directoryName ends with ".0".
    if (strncmp(directoryName + (directoryNameLen - kPostfixLen), kPostfix, kPostfixLen) != 0) {
        return 0;
    }

    // Extract Version from "10.0.{Version}.0" and convert Version into an integer.
    return atoi(directoryName + kPrefixLen);
}

class ScopedFileHandle final {
  public:
    explicit ScopedFileHandle(HANDLE handle) : mHandle(handle) {}
    ~ScopedFileHandle() {
        if (mHandle != INVALID_HANDLE_VALUE) {
            DAWN_ASSERT(FindClose(mHandle));
        }
    }
    HANDLE GetHandle() const { return mHandle; }

  private:
    HANDLE mHandle;
};

std::string GetWindowsSDKBasePath() {
    const char* kDefaultWindowsSDKPath = "C:\\Program Files (x86)\\Windows Kits\\10\\bin\\*";
    WIN32_FIND_DATAA fileData;
    ScopedFileHandle handle(FindFirstFileA(kDefaultWindowsSDKPath, &fileData));
    if (handle.GetHandle() == INVALID_HANDLE_VALUE) {
        return "";
    }

    uint32_t highestWindowsSDKVersion = 0;
    do {
        if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            continue;
        }

        highestWindowsSDKVersion = std::max(
            highestWindowsSDKVersion, GetWindowsSDKVersionFromDirectoryName(fileData.cFileName));
    } while (FindNextFileA(handle.GetHandle(), &fileData));

    if (highestWindowsSDKVersion == 0) {
        return "";
    }

    // Currently we only support using DXC on x64.
    std::ostringstream ostream;
    ostream << "C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0." << highestWindowsSDKVersion
            << ".0\\x64\\";

    return ostream.str();
}

}  // anonymous namespace

PlatformFunctions::PlatformFunctions() = default;
PlatformFunctions::~PlatformFunctions() = default;

MaybeError PlatformFunctions::LoadFunctions() {
    DAWN_TRY(Base::LoadFunctions());
    LoadDXCLibraries();
    DAWN_TRY(LoadD3D12());
    DAWN_TRY(LoadD3D11());
    LoadPIXRuntime();
    return {};
}

MaybeError PlatformFunctions::LoadD3D12() {
#if DAWN_PLATFORM_IS(WINUWP)
    d3d12CreateDevice = &D3D12CreateDevice;
    d3d12GetDebugInterface = &D3D12GetDebugInterface;
    d3d12SerializeRootSignature = &D3D12SerializeRootSignature;
    d3d12CreateRootSignatureDeserializer = &D3D12CreateRootSignatureDeserializer;
    d3d12SerializeVersionedRootSignature = &D3D12SerializeVersionedRootSignature;
    d3d12CreateVersionedRootSignatureDeserializer = &D3D12CreateVersionedRootSignatureDeserializer;
#else
    std::string error;
    if (!mD3D12Lib.OpenSystemLibrary(L"d3d12.dll", &error) ||
        !mD3D12Lib.GetProc(&d3d12CreateDevice, "D3D12CreateDevice", &error) ||
        !mD3D12Lib.GetProc(&d3d12GetDebugInterface, "D3D12GetDebugInterface", &error) ||
        !mD3D12Lib.GetProc(&d3d12SerializeRootSignature, "D3D12SerializeRootSignature", &error) ||
        !mD3D12Lib.GetProc(&d3d12CreateRootSignatureDeserializer,
                           "D3D12CreateRootSignatureDeserializer", &error) ||
        !mD3D12Lib.GetProc(&d3d12SerializeVersionedRootSignature,
                           "D3D12SerializeVersionedRootSignature", &error) ||
        !mD3D12Lib.GetProc(&d3d12CreateVersionedRootSignatureDeserializer,
                           "D3D12CreateVersionedRootSignatureDeserializer", &error)) {
        return DAWN_INTERNAL_ERROR(error.c_str());
    }
#endif

    return {};
}

MaybeError PlatformFunctions::LoadD3D11() {
#if DAWN_PLATFORM_IS(WINUWP)
    d3d11on12CreateDevice = &D3D11On12CreateDevice;
#else
    std::string error;
    if (!mD3D11Lib.OpenSystemLibrary(L"d3d11.dll", &error) ||
        !mD3D11Lib.GetProc(&d3d11on12CreateDevice, "D3D11On12CreateDevice", &error)) {
        return DAWN_INTERNAL_ERROR(error.c_str());
    }
#endif

    return {};
}

bool PlatformFunctions::IsPIXEventRuntimeLoaded() const {
    return mPIXEventRuntimeLib.Valid();
}

void PlatformFunctions::LoadPIXRuntime() {
    // TODO(dawn:766):
    // In UWP PIX should be statically linked WinPixEventRuntime_UAP.lib
    // So maybe we should put WinPixEventRuntime as a third party package
    // Currently PIX is not going to be loaded in UWP since the following
    // mPIXEventRuntimeLib.Open will fail.
    if (!mPIXEventRuntimeLib.Open("WinPixEventRuntime.dll") ||
        !mPIXEventRuntimeLib.GetProc(&pixBeginEventOnCommandList, "PIXBeginEventOnCommandList") ||
        !mPIXEventRuntimeLib.GetProc(&pixEndEventOnCommandList, "PIXEndEventOnCommandList") ||
        !mPIXEventRuntimeLib.GetProc(&pixSetMarkerOnCommandList, "PIXSetMarkerOnCommandList")) {
        mPIXEventRuntimeLib.Close();
    }
}

void PlatformFunctions::LoadDXCLibraries() {
    // TODO(dawn:766)
    // Statically linked with dxcompiler.lib in UWP
    // currently linked with dxcompiler.lib making CoreApp unable to activate
    // LoadDXIL and LoadDXCompiler will fail in UWP, but LoadFunctions() can still be
    // successfully executed.

    const std::string& windowsSDKBasePath = GetWindowsSDKBasePath();

    LoadDXIL(windowsSDKBasePath);
    LoadDXCompiler(windowsSDKBasePath);
}

void PlatformFunctions::LoadDXIL(const std::string& baseWindowsSDKPath) {
    constexpr char kDxilDLLName[] = "dxil.dll";
    const std::array kDxilDLLPaths{
#ifdef DAWN_USE_BUILT_DXC
        std::string{kDxilDLLName},
#else
        std::string{kDxilDLLName},
        baseWindowsSDKPath + kDxilDLLName,
#endif
    };

    for (const std::string& dxilDLLPath : kDxilDLLPaths) {
        if (mDXILLib.Open(dxilDLLPath, nullptr)) {
            return;
        }
    }
    DAWN_ASSERT(!mDXILLib.Valid());
}

void PlatformFunctions::LoadDXCompiler(const std::string& baseWindowsSDKPath) {
    // DXIL must be loaded before DXC, otherwise shader signing is unavailable
    if (!mDXILLib.Valid()) {
        return;
    }

    constexpr char kDxCompilerDLLName[] = "dxcompiler.dll";
    const std::array kDxCompilerDLLPaths{
#ifdef DAWN_USE_BUILT_DXC
        std::string{kDxCompilerDLLName},
#else
        std::string{kDxCompilerDLLName}, baseWindowsSDKPath + kDxCompilerDLLName
#endif
    };

    DynamicLib dxCompilerLib;
    for (const std::string& dllName : kDxCompilerDLLPaths) {
        if (dxCompilerLib.Open(dllName, nullptr)) {
            break;
        }
    }

    if (dxCompilerLib.Valid() &&
        dxCompilerLib.GetProc(&dxcCreateInstance, "DxcCreateInstance", nullptr)) {
        mDXCompilerLib = std::move(dxCompilerLib);
    } else {
        mDXILLib.Close();
    }
}

// Use Backend::IsDXCAvailable if possible, which also check the DXC is no older than a given
// version
bool PlatformFunctions::IsDXCBinaryAvailable() const {
    return mDXILLib.Valid() && mDXCompilerLib.Valid();
}

}  // namespace dawn::native::d3d12
