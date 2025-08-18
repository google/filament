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

#include "dawn/native/d3d/ShaderUtils.h"

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "dawn/native/d3d/BlobD3D.h"
#include "dawn/native/d3d/D3DCompilationRequest.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/DeviceD3D.h"
#include "dawn/native/d3d/PlatformFunctions.h"
#include "dawn/native/d3d/UtilsD3D.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include "tint/tint.h"

namespace dawn::native::d3d {

namespace {

// Be careful that the return vector may contain the pointers that point to non-static memory.
std::vector<const wchar_t*> GetDXCArguments(std::wstring_view entryPointNameW,
                                            const d3d::D3DBytecodeCompilationRequest& r) {
    std::vector<const wchar_t*> arguments;

    arguments.push_back(L"-T");
    arguments.push_back(r.dxcShaderProfile.data());

    arguments.push_back(L"-E");
    arguments.push_back(entryPointNameW.data());

    // TODO(chromium:346595893): Disable buggy DXC pass
    arguments.push_back(L"-opt-disable");
    arguments.push_back(L"structurize-loop-exits-for-unroll");

    uint32_t compileFlags = r.compileFlags;
    if (compileFlags & D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY) {
        arguments.push_back(L"/Gec");
    }
    if (compileFlags & D3DCOMPILE_IEEE_STRICTNESS) {
        arguments.push_back(L"/Gis");
    }
    constexpr uint32_t d3dCompileFlagsBits = D3DCOMPILE_OPTIMIZATION_LEVEL2;
    if (compileFlags & d3dCompileFlagsBits) {
        switch (compileFlags & D3DCOMPILE_OPTIMIZATION_LEVEL2) {
            case D3DCOMPILE_OPTIMIZATION_LEVEL0:
                arguments.push_back(L"/O0");
                break;
            case D3DCOMPILE_OPTIMIZATION_LEVEL2:
                arguments.push_back(L"/O2");
                break;
            case D3DCOMPILE_OPTIMIZATION_LEVEL3:
                arguments.push_back(L"/O3");
                break;
        }
    } else {
        // D3DCOMPILE_OPTIMIZATION_LEVEL1 is defined to 0
        arguments.push_back(L"/O1");
    }
    if (compileFlags & D3DCOMPILE_SKIP_OPTIMIZATION) {
        // DXC will use the last optimization flag passed in (/O[n] and /Od), so we make sure
        // to pass /Od last.
        arguments.push_back(L"/Od");
    }
    if (compileFlags & D3DCOMPILE_DEBUG) {
        arguments.push_back(L"/Zi");
        // Unlike FXC, DXC does not embed debug info into the shader object by default, as it's
        // preferable to save it to pdb files to keep shader objects small. Embed it for now, and we
        // can consider exposing an option for users to supply a path to dump pdbs to in the future.
        arguments.push_back(L"/Qembed_debug");
    }
    if (compileFlags & D3DCOMPILE_PACK_MATRIX_ROW_MAJOR) {
        arguments.push_back(L"/Zpr");
    }
    if (compileFlags & D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR) {
        arguments.push_back(L"/Zpc");
    }
    if (compileFlags & D3DCOMPILE_AVOID_FLOW_CONTROL) {
        arguments.push_back(L"/Gfa");
    }
    if (compileFlags & D3DCOMPILE_PREFER_FLOW_CONTROL) {
        arguments.push_back(L"/Gfp");
    }
    if (compileFlags & D3DCOMPILE_RESOURCES_MAY_ALIAS) {
        arguments.push_back(L"/res_may_alias");
    }

#define ASSERT_UNHANDLED(f) DAWN_ASSERT((compileFlags & f) == 0)
    ASSERT_UNHANDLED(D3DCOMPILE_SKIP_VALIDATION);
    ASSERT_UNHANDLED(D3DCOMPILE_PARTIAL_PRECISION);
    ASSERT_UNHANDLED(D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT);
    ASSERT_UNHANDLED(D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT);
    ASSERT_UNHANDLED(D3DCOMPILE_NO_PRESHADER);
    ASSERT_UNHANDLED(D3DCOMPILE_ENABLE_STRICTNESS);
    ASSERT_UNHANDLED(D3DCOMPILE_RESERVED16);
    ASSERT_UNHANDLED(D3DCOMPILE_RESERVED17);
    ASSERT_UNHANDLED(D3DCOMPILE_WARNINGS_ARE_ERRORS);
    ASSERT_UNHANDLED(D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES);
    ASSERT_UNHANDLED(D3DCOMPILE_ALL_RESOURCES_BOUND);
    ASSERT_UNHANDLED(D3DCOMPILE_DEBUG_NAME_FOR_SOURCE);
    ASSERT_UNHANDLED(D3DCOMPILE_DEBUG_NAME_FOR_BINARY);
#undef ASSERT_UNHANDLED

    if (r.hasShaderF16Feature) {
        // enable-16bit-types are only allowed in -HV 2018 (default)
        arguments.push_back(L"/enable-16bit-types");
    }

    arguments.push_back(L"-HV");
    arguments.push_back(L"2018");

    return arguments;
}

ResultOrError<ComPtr<IDxcBlob>> CompileShaderDXC(const d3d::D3DBytecodeCompilationRequest& r,
                                                 const std::string& entryPointName,
                                                 const std::string& hlslSource,
                                                 bool dumpShadersOnFailure) {
    DxcBuffer dxcBuffer;
    dxcBuffer.Ptr = hlslSource.c_str();
    dxcBuffer.Size = hlslSource.length();
    dxcBuffer.Encoding = DXC_CP_UTF8;

    std::wstring entryPointW;
    DAWN_TRY_ASSIGN(entryPointW, d3d::ConvertStringToWstring(entryPointName));

    // Note that the contents in `arguments` shouldn't be mutated or moved around as some of the
    // pointers in this vector don't have static lifetime.
    std::vector<const wchar_t*> arguments = GetDXCArguments(entryPointW, r);
    ComPtr<IDxcResult> result;
    DAWN_TRY(CheckHRESULT(
        r.dxcCompiler.UnsafeGetValue()->Compile(&dxcBuffer, arguments.data(), arguments.size(),
                                                nullptr, IID_PPV_ARGS(&result)),
        "DXC compile"));

    HRESULT hr;
    DAWN_TRY(CheckHRESULT(result->GetStatus(&hr), "DXC get status"));

    if (FAILED(hr)) {
        const char* hrAsString = HRESULTAsString(hr);
        ComPtr<IDxcBlobEncoding> errors;
        DAWN_TRY(CheckHRESULT(result->GetErrorBuffer(&errors), "DXC get error buffer"));

        if (dumpShadersOnFailure) {
            return DAWN_VALIDATION_ERROR(
                "DXC compile failed with error: %s msg: %s\n/* Generated HLSL: */\n%s\n",
                hrAsString, static_cast<char*>(errors->GetBufferPointer()), hlslSource.c_str());
        }
        return DAWN_VALIDATION_ERROR("DXC compile failed with error: %s.", hrAsString);
    }

    ComPtr<IDxcBlob> compiledShader;
    DAWN_TRY(CheckHRESULT(result->GetResult(&compiledShader), "DXC get result"));
    return std::move(compiledShader);
}

ResultOrError<ComPtr<ID3DBlob>> CompileShaderFXC(const d3d::D3DBytecodeCompilationRequest& r,
                                                 const std::string& entryPointName,
                                                 const std::string& hlslSource,
                                                 bool dumpShadersOnFailure) {
    ComPtr<ID3DBlob> compiledShader;
    ComPtr<ID3DBlob> errors;

    auto result = r.d3dCompile.UnsafeGetValue()(
        hlslSource.c_str(), hlslSource.length(), nullptr, nullptr, nullptr, entryPointName.c_str(),
        r.fxcShaderProfile.data(), r.compileFlags, 0, &compiledShader, &errors);

    if (FAILED(result)) {
        const char* resultAsString = HRESULTAsString(result);
        if (dumpShadersOnFailure) {
            std::string errorMsg = errors ? static_cast<char*>(errors->GetBufferPointer()) : "";
            return DAWN_VALIDATION_ERROR(
                "FXC compile failed with error: %s msg: %s\n/* Generated HLSL: */\n%s\n",
                resultAsString, errorMsg, hlslSource.c_str());
        }

        return DAWN_VALIDATION_ERROR("FXC compile failed with error: %s.", resultAsString);
    }

    return std::move(compiledShader);
}

MaybeError TranslateToHLSL(d3d::HlslCompilationRequest r,
                           UnsafeUnserializedValue<dawn::platform::Platform*> tracePlatform,
                           CompiledShader* compiledShader) {
    r.tintOptions.strip_all_names = !r.disableSymbolRenaming;

    TRACE_EVENT0(tracePlatform.UnsafeGetValue(), General, "tint::hlsl::writer::Generate");

    // Convert the AST program to an IR module.
    tint::Result<tint::core::ir::Module> ir;
    {
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(tracePlatform.UnsafeGetValue(),
                                           "ShaderModuleProgramToIR");

        // Requires Tint Program here right before actual using.
        auto inputProgram = r.inputProgram.UnsafeGetValue()->GetTintProgram();
        const tint::Program* tintInputProgram = &(inputProgram->program);

        ir = tint::wgsl::reader::ProgramToLoweredIR(*tintInputProgram);
        DAWN_INVALID_IF(ir != tint::Success, "An error occurred while generating Tint IR\n%s",
                        ir.Failure().reason);
    }

    {
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(tracePlatform.UnsafeGetValue(),
                                           "ShaderModuleSingleEntryPoint");
        auto singleEntryPointResult =
            tint::core::ir::transform::SingleEntryPoint(ir.Get(), r.entryPointName);
        DAWN_INVALID_IF(singleEntryPointResult != tint::Success,
                        "Pipeline single entry point (IR) failed:\n%s",
                        singleEntryPointResult.Failure().reason);
    }

    // this needs to run after SingleEntryPoint transform which removes unused
    // overrides for the current entry point.
    {
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(tracePlatform.UnsafeGetValue(),
                                           "ShaderModuleSubstituteOverrides");
        tint::core::ir::transform::SubstituteOverridesConfig cfg;
        cfg.map = std::move(r.substituteOverrideConfig);
        auto substituteOverridesResult =
            tint::core::ir::transform::SubstituteOverrides(ir.Get(), cfg);

        DAWN_INVALID_IF(substituteOverridesResult != tint::Success,
                        "Pipeline override substitution (IR) failed:\n%s",
                        substituteOverridesResult.Failure().reason);
    }

    tint::Result<tint::hlsl::writer::Output> result;
    {
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(tracePlatform.UnsafeGetValue(),
                                           "ShaderModuleGenerateHLSL");
        result = tint::hlsl::writer::Generate(ir.Get(), r.tintOptions);
        DAWN_INVALID_IF(result != tint::Success, "An error occurred while generating HLSL:\n%s",
                        result.Failure().reason);
    }

    // Workgroup validation has to come after `Generate` because it may require overrides to
    // have been substituted.
    if (r.stage == SingleShaderStage::Compute) {
        // Validate workgroup size and workgroup storage size.
        Extent3D _;
        DAWN_TRY_ASSIGN(
            _, ValidateComputeStageWorkgroupSize(
                   result->workgroup_info.x, result->workgroup_info.y, result->workgroup_info.z,
                   result->workgroup_info.storage_size, /* usesSubgroupMatrix */ false,
                   r.maxSubgroupSize, r.limits, r.adapterSupportedLimits.UnsafeGetValue()));
    }

    bool usesVertexIndex = false;
    bool usesInstanceIndex = false;
    if (r.stage == SingleShaderStage::Vertex) {
        usesVertexIndex = result->has_vertex_index;
        usesInstanceIndex = result->has_instance_index;
    }

    compiledShader->usesVertexIndex = usesVertexIndex;
    compiledShader->usesInstanceIndex = usesInstanceIndex;
    compiledShader->hlslSource = std::move(result->hlsl);
    return {};
}

}  // anonymous namespace

std::string CompileFlagsToString(uint32_t compileFlags) {
    struct Flag {
        uint32_t value;
        const char* name;
    };
    constexpr Flag flags[] = {
    // Populated from d3dcompiler.h
#define F(f) Flag{f, #f}
        F(D3DCOMPILE_DEBUG),
        F(D3DCOMPILE_SKIP_VALIDATION),
        F(D3DCOMPILE_SKIP_OPTIMIZATION),
        F(D3DCOMPILE_PACK_MATRIX_ROW_MAJOR),
        F(D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR),
        F(D3DCOMPILE_PARTIAL_PRECISION),
        F(D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT),
        F(D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT),
        F(D3DCOMPILE_NO_PRESHADER),
        F(D3DCOMPILE_AVOID_FLOW_CONTROL),
        F(D3DCOMPILE_PREFER_FLOW_CONTROL),
        F(D3DCOMPILE_ENABLE_STRICTNESS),
        F(D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY),
        F(D3DCOMPILE_IEEE_STRICTNESS),
        F(D3DCOMPILE_RESERVED16),
        F(D3DCOMPILE_RESERVED17),
        F(D3DCOMPILE_WARNINGS_ARE_ERRORS),
        F(D3DCOMPILE_RESOURCES_MAY_ALIAS),
        F(D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES),
        F(D3DCOMPILE_ALL_RESOURCES_BOUND),
        F(D3DCOMPILE_DEBUG_NAME_FOR_SOURCE),
        F(D3DCOMPILE_DEBUG_NAME_FOR_BINARY),
#undef F
    };

    std::string result;
    for (const Flag& f : flags) {
        if ((compileFlags & f.value) != 0) {
            result += f.name + std::string("\n");
        }
    }

    // Optimization level must be handled separately as two bits are used, and the values
    // don't map neatly to 0-3.
    constexpr uint32_t d3dCompileFlagsBits = D3DCOMPILE_OPTIMIZATION_LEVEL2;
    switch (compileFlags & d3dCompileFlagsBits) {
        case D3DCOMPILE_OPTIMIZATION_LEVEL0:
            result += "D3DCOMPILE_OPTIMIZATION_LEVEL0";
            break;
        case D3DCOMPILE_OPTIMIZATION_LEVEL1:
            result += "D3DCOMPILE_OPTIMIZATION_LEVEL1";
            break;
        case D3DCOMPILE_OPTIMIZATION_LEVEL2:
            result += "D3DCOMPILE_OPTIMIZATION_LEVEL2";
            break;
        case D3DCOMPILE_OPTIMIZATION_LEVEL3:
            result += "D3DCOMPILE_OPTIMIZATION_LEVEL3";
            break;
    }
    result += std::string("\n");

    return result;
}

ResultOrError<CompiledShader> CompileShader(d3d::D3DCompilationRequest r) {
    CompiledShader compiledShader;
    bool dumpShadersOnFailure = r.hlsl.dumpShaders || r.hlsl.dumpShadersOnFailure;
    // Compile the source shader to HLSL.
    DAWN_TRY(TranslateToHLSL(std::move(r.hlsl), r.tracePlatform, &compiledShader));

    switch (r.bytecode.compiler) {
        case d3d::Compiler::DXC: {
            TRACE_EVENT0(r.tracePlatform.UnsafeGetValue(), General, "CompileShaderDXC");
            ComPtr<IDxcBlob> compiledDXCShader;
            DAWN_TRY_ASSIGN(
                compiledDXCShader,
                CompileShaderDXC(r.bytecode, r.hlsl.tintOptions.remapped_entry_point_name,
                                 compiledShader.hlslSource, dumpShadersOnFailure));
            compiledShader.shaderBlob = CreateBlob(std::move(compiledDXCShader));
            break;
        }
        case d3d::Compiler::FXC: {
            TRACE_EVENT0(r.tracePlatform.UnsafeGetValue(), General, "CompileShaderFXC");
            ComPtr<ID3DBlob> compiledFXCShader;
            DAWN_TRY_ASSIGN(
                compiledFXCShader,
                CompileShaderFXC(r.bytecode, r.hlsl.tintOptions.remapped_entry_point_name,
                                 compiledShader.hlslSource, dumpShadersOnFailure));
            compiledShader.shaderBlob = CreateBlob(std::move(compiledFXCShader));
            break;
        }
    }

    // If dumpShaders is false, we don't need the HLSL for logging. Clear the contents so it
    // isn't stored into the cache.
    if (!r.hlsl.dumpShaders) {
        compiledShader.hlslSource = "";
    }
    return compiledShader;
}

void DumpFXCCompiledShader(Device* device,
                           const CompiledShader& compiledShader,
                           uint32_t compileFlags) {
    std::ostringstream dumpedMsg;
    DAWN_ASSERT(!compiledShader.hlslSource.empty());
    dumpedMsg << "/* Dumped generated HLSL */\n" << compiledShader.hlslSource << "\n";

    const Blob& shaderBlob = compiledShader.shaderBlob;
    DAWN_ASSERT(!shaderBlob.Empty());
    dumpedMsg << "/* FXC compile flags */\n" << CompileFlagsToString(compileFlags) << "\n";
    dumpedMsg << "/* Dumped disassembled DXBC */\n";
    ComPtr<ID3DBlob> disassembly;
    UINT flags =
        // Some literals are printed as floats with precision(6) which is not enough
        // precision for values very close to 0, so always print literals as hex values.
        D3D_DISASM_PRINT_HEX_LITERALS;
    if (FAILED(device->GetFunctions()->d3dDisassemble(shaderBlob.Data(), shaderBlob.Size(), flags,
                                                      nullptr, &disassembly))) {
        dumpedMsg << "D3D disassemble failed\n";
    } else {
        dumpedMsg << std::string_view(static_cast<const char*>(disassembly->GetBufferPointer()),
                                      disassembly->GetBufferSize());
    }

    std::string logMessage = dumpedMsg.str();
    if (!logMessage.empty()) {
        device->EmitLog(wgpu::LoggingType::Info, logMessage.c_str());
    }
}

InterStageShaderVariablesMask ToInterStageShaderVariablesMask(const std::vector<bool>& inputMask) {
    InterStageShaderVariablesMask outputMask;
    DAWN_ASSERT(inputMask.size() <= outputMask.size());
    for (size_t i = 0; i < inputMask.size(); ++i) {
        outputMask[i] = inputMask[i];
    }
    return outputMask;
}

}  // namespace dawn::native::d3d
