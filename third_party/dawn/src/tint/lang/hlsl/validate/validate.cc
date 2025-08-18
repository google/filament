// Copyright 2021 The Dawn & Tint Authors
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

#include <string>
#include <vector>

#include "src/tint/lang/hlsl/validate/validate.h"

#include "src/tint/utils/command/command.h"
#include "src/tint/utils/file/tmpfile.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/text/string.h"

#ifdef _WIN32
#include <Windows.h>
#include <atlbase.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <wrl.h>
#else
#include <dlfcn.h>
#endif  // _WIN32

// dxc headers
TINT_BEGIN_DISABLE_ALL_WARNINGS();
#ifdef __clang__
// # Use UUID emulation with clang to avoid compiling with ms-extensions
#define __EMULATE_UUID
#endif
#include "dxc/dxcapi.h"
TINT_END_DISABLE_ALL_WARNINGS();

// Disable warnings about old-style casts which result from using
// the SUCCEEDED and FAILED macros that C-style cast to HRESULT.
TINT_DISABLE_WARNING_OLD_STYLE_CAST

namespace {
using PFN_DXC_CREATE_INSTANCE = HRESULT(__stdcall*)(REFCLSID rclsid,
                                                    REFIID riid,
                                                    LPVOID* ppCompiler);

// Wrap the call to DxcCreateInstance via the dlsym-loaded function pointer
// to disable UBSAN on it. This is to workaround a known UBSAN false
// positive: https://github.com/google/sanitizers/issues/911
DAWN_NO_SANITIZE("undefined")
HRESULT CallDxcCreateInstance(PFN_DXC_CREATE_INSTANCE dxc_create_instance,
                              CComPtr<IDxcCompiler3>& dxc_compiler) {
    return dxc_create_instance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxc_compiler));
}
}  // namespace

namespace tint::hlsl::validate {

Result ValidateUsingDXC(const std::string& dxc_path,
                        const std::string& source,
                        const std::string& entry_point_name,
                        core::ir::Function::PipelineStage pipeline_stage,
                        bool require_16bit_types,
                        uint32_t hlsl_shader_model) {
    Result result;

    if (entry_point_name.empty()) {
        result.output = "No entrypoint found";
        result.failed = true;
        return result;
    }

    // Native 16-bit types, e.g. float16_t, require SM6.2. Otherwise we use SM6.0.
    if (hlsl_shader_model < 60 || hlsl_shader_model > 66) {
        result.output = "Invalid HLSL shader model " + std::to_string(hlsl_shader_model);
        result.failed = true;
        return result;
    }
    if (require_16bit_types && hlsl_shader_model < 62) {
        result.output = "The HLSL shader model " + std::to_string(hlsl_shader_model) +
                        " is not enough for float16_t.";
        result.failed = true;
        return result;
    }

#define CHECK_HR(hr, error_msg)        \
    do {                               \
        if (FAILED(hr)) {              \
            result.output = error_msg; \
            result.failed = true;      \
            return result;             \
        }                              \
    } while (false)

    HRESULT hr;

    // Load the dll and get the DxcCreateInstance function
    PFN_DXC_CREATE_INSTANCE dxc_create_instance = nullptr;
#ifdef _WIN32
    HMODULE dxcLib = LoadLibraryA(dxc_path.c_str());
    if (dxcLib == nullptr) {
        result.output = "Failed to load dxc: " + dxc_path;
        result.failed = true;
        return result;
    }
    // Avoid ASAN false positives when unloading DLL: https://github.com/google/sanitizers/issues/89
#if !DAWN_ASAN_ENABLED()
    TINT_DEFER({ FreeLibrary(dxcLib); });
#endif

    dxc_create_instance =
        reinterpret_cast<PFN_DXC_CREATE_INSTANCE>(GetProcAddress(dxcLib, "DxcCreateInstance"));
#else
    void* dxcLib = dlopen(dxc_path.c_str(), RTLD_LAZY | RTLD_GLOBAL | RTLD_NODELETE);
    if (dxcLib == nullptr) {
        result.output = "Failed to load dxc: " + dxc_path;
        result.failed = true;
        return result;
    }
    // Avoid ASAN false positives when unloading DLL: https://github.com/google/sanitizers/issues/89
#if !DAWN_ASAN_ENABLED()
    TINT_DEFER({ dlclose(dxcLib); });
#endif

    dxc_create_instance =
        reinterpret_cast<PFN_DXC_CREATE_INSTANCE>(dlsym(dxcLib, "DxcCreateInstance"));
#endif
    if (dxc_create_instance == nullptr) {
        result.output = "GetProcAccess failed";
        result.failed = true;
        return result;
    }

    CComPtr<IDxcCompiler3> dxc_compiler;
    hr = CallDxcCreateInstance(dxc_create_instance, dxc_compiler);
    CHECK_HR(hr, "DxcCreateInstance failed");

    const wchar_t* stage_prefix = L"";
    switch (pipeline_stage) {
        case core::ir::Function::PipelineStage::kUndefined:
            result.output = "Invalid PipelineStage";
            result.failed = true;
            return result;
        case core::ir::Function::PipelineStage::kVertex:
            stage_prefix = L"vs";
            break;
        case core::ir::Function::PipelineStage::kFragment:
            stage_prefix = L"ps";
            break;
        case core::ir::Function::PipelineStage::kCompute:
            stage_prefix = L"cs";
            break;
    }

    // Match Dawn's compile flags
    // See dawn\src\dawn_native\d3d12\RenderPipelineD3D12.cpp
    // and dawn_native\d3d\ShaderUtils.cpp (GetDXCArguments)
    std::wstring shader_model_version =
        std::to_wstring(hlsl_shader_model / 10) + L"_" + std::to_wstring(hlsl_shader_model % 10);
    std::wstring profile = std::wstring(stage_prefix) + L"_" + shader_model_version;
    std::wstring entry_point = std::wstring(entry_point_name.begin(), entry_point_name.end());
    std::vector<const wchar_t*> args{
        L"-T",                                              // Profile
        profile.c_str(),                                    //
        L"-HV 2018",                                        // Use HLSL 2018
        L"-E",                                              // Entry point
        entry_point.c_str(),                                //
        L"/Zpr",                                            // D3DCOMPILE_PACK_MATRIX_ROW_MAJOR
        L"/Gis",                                            // D3DCOMPILE_IEEE_STRICTNESS
        require_16bit_types ? L"-enable-16bit-types" : L""  // Enable 16-bit if required
    };

    DxcBuffer source_buffer;
    source_buffer.Ptr = source.c_str();
    source_buffer.Size = source.length();
    source_buffer.Encoding = DXC_CP_UTF8;
    CComPtr<IDxcResult> compile_result;
    hr = dxc_compiler->Compile(&source_buffer, args.data(), static_cast<UINT32>(args.size()),
                               nullptr, IID_PPV_ARGS(&compile_result));
    CHECK_HR(hr, "Compile call failed");

    HRESULT compile_status;
    hr = compile_result->GetStatus(&compile_status);
    CHECK_HR(hr, "GetStatus call failed");

    if (FAILED(compile_status)) {
        CComPtr<IDxcBlobEncoding> errors;
        hr = compile_result->GetErrorBuffer(&errors);
        CHECK_HR(hr, "GetErrorBuffer call failed");
        result.output = static_cast<char*>(errors->GetBufferPointer());
        result.failed = true;
        return result;
    }

    // Compilation succeeded, get compiled shader blob and disassemble it.
    CComPtr<IDxcBlob> compiled_shader;
    hr = compile_result->GetResult(&compiled_shader);
    CHECK_HR(hr, "GetResult call failed");

    DxcBuffer compiled_shader_buffer;
    compiled_shader_buffer.Ptr = compiled_shader->GetBufferPointer();
    compiled_shader_buffer.Size = compiled_shader->GetBufferSize();
    compiled_shader_buffer.Encoding = DXC_CP_UTF8;
    CComPtr<IDxcResult> dis_result;
    hr = dxc_compiler->Disassemble(&compiled_shader_buffer, IID_PPV_ARGS(&dis_result));
    CHECK_HR(hr, "Disassemble call failed");

    CComPtr<IDxcBlobEncoding> disassembly;
    if ((dis_result != nullptr) && dis_result->HasOutput(DXC_OUT_DISASSEMBLY) &&
        SUCCEEDED(
            dis_result->GetOutput(DXC_OUT_DISASSEMBLY, IID_PPV_ARGS(&disassembly), nullptr))) {
        result.output = static_cast<char*>(disassembly->GetBufferPointer());
    } else {
        result.output = "Failed to disassemble shader";
    }

    return result;
}

#ifdef _WIN32
Result ValidateUsingFXC(const std::string& fxc_path,
                        const std::string& source,
                        const std::string& entry_point_name,
                        core::ir::Function::PipelineStage pipeline_stage) {
    Result result;

    if (entry_point_name.empty()) {
        result.output = "No entrypoint found";
        result.failed = true;
        return result;
    }

    HMODULE fxcLib = LoadLibraryA(fxc_path.c_str());
    if (fxcLib == nullptr) {
        result.output = "Couldn't load FXC";
        result.failed = true;
        return result;
    }
    TINT_DEFER({ FreeLibrary(fxcLib); });

    auto* d3dCompile = reinterpret_cast<pD3DCompile>(
        reinterpret_cast<void*>(GetProcAddress(fxcLib, "D3DCompile")));
    auto* d3dDisassemble = reinterpret_cast<pD3DDisassemble>(
        reinterpret_cast<void*>(GetProcAddress(fxcLib, "D3DDisassemble")));

    if (d3dCompile == nullptr) {
        result.output = "Couldn't load D3DCompile from FXC";
        result.failed = true;
        return result;
    }
    if (d3dDisassemble == nullptr) {
        result.output = "Couldn't load D3DDisassemble from FXC";
        result.failed = true;
        return result;
    }

    const char* profile = "";
    switch (pipeline_stage) {
        case core::ir::Function::PipelineStage::kUndefined:
            result.output = "Invalid PipelineStage";
            result.failed = true;
            return result;
        case core::ir::Function::PipelineStage::kVertex:
            profile = "vs_5_1";
            break;
        case core::ir::Function::PipelineStage::kFragment:
            profile = "ps_5_1";
            break;
        case core::ir::Function::PipelineStage::kCompute:
            profile = "cs_5_1";
            break;
    }

    // Match Dawn's compile flags
    // See dawn\src\dawn_native\d3d12\RenderPipelineD3D12.cpp
    UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR |
                        D3DCOMPILE_IEEE_STRICTNESS;

    CComPtr<ID3DBlob> compiledShader;
    CComPtr<ID3DBlob> errors;
    HRESULT res = d3dCompile(source.c_str(),            // pSrcData
                             source.length(),           // SrcDataSize
                             nullptr,                   // pSourceName
                             nullptr,                   // pDefines
                             nullptr,                   // pInclude
                             entry_point_name.c_str(),  // pEntrypoint
                             profile,                   // pTarget
                             compileFlags,              // Flags1
                             0,                         // Flags2
                             &compiledShader,           // ppCode
                             &errors);                  // ppErrorMsgs
    if (FAILED(res)) {
        result.output = static_cast<char*>(errors->GetBufferPointer());
        result.failed = true;
        return result;
    } else {
        CComPtr<ID3DBlob> disassembly;
        res = d3dDisassemble(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), 0,
                             "", &disassembly);
        if (FAILED(res)) {
            result.output = "Failed to disassemble shader";
        } else {
            result.output = static_cast<char*>(disassembly->GetBufferPointer());
        }
    }

    return result;
}
#endif  // _WIN32

}  // namespace tint::hlsl::validate
