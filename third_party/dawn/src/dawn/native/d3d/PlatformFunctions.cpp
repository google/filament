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

#include "dawn/native/d3d/PlatformFunctions.h"

#include <comdef.h>
#include <versionhelpers.h>

#include <vector>

namespace dawn::native::d3d {

namespace {
// Referenced from base/win/registry.cc in Chromium
uint64_t ReadFromSZRegistryKey(HKEY registerKey, const char* registerKeyName) {
    DWORD valueType;
    DWORD returnSize;
    if (RegQueryValueExA(registerKey, registerKeyName, nullptr, &valueType, nullptr, &returnSize) !=
        ERROR_SUCCESS) {
        return 0;
    }
    std::vector<char> returnStringValue(returnSize);
    auto hr = RegQueryValueExA(registerKey, registerKeyName, nullptr, &valueType,
                               reinterpret_cast<LPBYTE>(returnStringValue.data()), &returnSize);
    if (hr != ERROR_SUCCESS || valueType != REG_SZ) {
        return 0;
    }
    constexpr int32_t kRadix = 10;
    return strtol(returnStringValue.data(), nullptr, kRadix);
}

}  // anonymous namespace

PlatformFunctions::PlatformFunctions() : mCurrentBuildNumber(0) {}

PlatformFunctions::~PlatformFunctions() = default;

MaybeError PlatformFunctions::LoadFunctions() {
    DAWN_TRY(LoadDXGI());
    DAWN_TRY(LoadFXCompiler());
    DAWN_TRY(LoadKernelBase());
    InitWindowsVersion();
    return {};
}

MaybeError PlatformFunctions::LoadDXGI() {
#if DAWN_PLATFORM_IS(WINUWP)
#if defined(_DEBUG)
    // DXGIGetDebugInterface1 is tagged as a development-only capability
    // which implies that linking to this function will cause
    // the application to fail Windows store certification
    // But we need it when debuging using VS Graphics Diagnostics or PIX
    // So we only link to it in debug build
    dxgiGetDebugInterface1 = &DXGIGetDebugInterface1;
#endif
    createDxgiFactory2 = &CreateDXGIFactory2;
#else
    std::string error;
    if (!mDXGILib.OpenSystemLibrary(L"dxgi.dll", &error) ||
        !mDXGILib.GetProc(&dxgiGetDebugInterface1, "DXGIGetDebugInterface1", &error) ||
        !mDXGILib.GetProc(&createDxgiFactory2, "CreateDXGIFactory2", &error)) {
        return DAWN_INTERNAL_ERROR(error.c_str());
    }
#endif

    return {};
}

MaybeError PlatformFunctions::LoadFXCompiler() {
#if DAWN_PLATFORM_IS(WINUWP)
    d3dCompile = &D3DCompile;
    d3dDisassemble = &D3DDisassemble;
#else
    std::string error;
    if (!mFXCompilerLib.Open("d3dcompiler_47.dll", &error) ||
        !mFXCompilerLib.GetProc(&d3dCompile, "D3DCompile", &error) ||
        !mFXCompilerLib.GetProc(&d3dDisassemble, "D3DDisassemble", &error)) {
        return DAWN_INTERNAL_ERROR(error.c_str());
    }
#endif
    return {};
}

MaybeError PlatformFunctions::LoadKernelBase() {
#if DAWN_PLATFORM_IS(WINUWP)
    compareObjectHandles = &CompareObjectHandles;
#else
    std::string error;
    if (!mKernelBaseLib.Open("kernelbase.dll", &error) ||
        !mKernelBaseLib.GetProc(&compareObjectHandles, "CompareObjectHandles", &error)) {
        return DAWN_INTERNAL_ERROR(error.c_str());
    }
#endif
    return {};
}

void PlatformFunctions::InitWindowsVersion() {
    // Currently we only care about the build number of Windows 10 and Windows 11.
    if (!IsWindows10OrGreater()) {
        return;
    }

    // Referenced from base/win/windows_version.cc in Chromium
    constexpr wchar_t kRegKeyWindowsNTCurrentVersion[] =
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";

    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, kRegKeyWindowsNTCurrentVersion, 0, KEY_QUERY_VALUE,
                      &hKey) != ERROR_SUCCESS) {
        return;
    }

    mCurrentBuildNumber = ReadFromSZRegistryKey(hKey, "CurrentBuildNumber");

    RegCloseKey(hKey);
}

uint64_t PlatformFunctions::GetWindowsBuildNumber() const {
    return mCurrentBuildNumber;
}

}  // namespace dawn::native::d3d
