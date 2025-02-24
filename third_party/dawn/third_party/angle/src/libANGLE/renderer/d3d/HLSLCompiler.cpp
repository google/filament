//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/d3d/HLSLCompiler.h"

#include <sstream>

#include "common/system_utils.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/Program.h"
#include "libANGLE/features.h"
#include "libANGLE/histogram_macros.h"
#include "libANGLE/renderer/d3d/ContextD3D.h"
#include "libANGLE/trace.h"

namespace
{
#if ANGLE_APPEND_ASSEMBLY_TO_SHADER_DEBUG_INFO
#    ifdef CREATE_COMPILER_FLAG_INFO
#        undef CREATE_COMPILER_FLAG_INFO
#    endif

#    define CREATE_COMPILER_FLAG_INFO(flag) \
        {                                   \
            flag, #flag                     \
        }

struct CompilerFlagInfo
{
    UINT mFlag;
    const char *mName;
};

CompilerFlagInfo CompilerFlagInfos[] = {
    // NOTE: The data below is copied from d3dcompiler.h
    // If something changes there it should be changed here as well
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_DEBUG),                           // (1 << 0)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_SKIP_VALIDATION),                 // (1 << 1)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_SKIP_OPTIMIZATION),               // (1 << 2)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_PACK_MATRIX_ROW_MAJOR),           // (1 << 3)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR),        // (1 << 4)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_PARTIAL_PRECISION),               // (1 << 5)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT),        // (1 << 6)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT),        // (1 << 7)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_NO_PRESHADER),                    // (1 << 8)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_AVOID_FLOW_CONTROL),              // (1 << 9)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_PREFER_FLOW_CONTROL),             // (1 << 10)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_ENABLE_STRICTNESS),               // (1 << 11)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY),  // (1 << 12)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_IEEE_STRICTNESS),                 // (1 << 13)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_OPTIMIZATION_LEVEL0),             // (1 << 14)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_OPTIMIZATION_LEVEL1),             // 0
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_OPTIMIZATION_LEVEL2),  // ((1 << 14) | (1 << 15))
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_OPTIMIZATION_LEVEL3),  // (1 << 15)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_RESERVED16),           // (1 << 16)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_RESERVED17),           // (1 << 17)
    CREATE_COMPILER_FLAG_INFO(D3DCOMPILE_WARNINGS_ARE_ERRORS)   // (1 << 18)
};

#    undef CREATE_COMPILER_FLAG_INFO

bool IsCompilerFlagSet(UINT mask, UINT flag)
{
    bool isFlagSet = IsMaskFlagSet(mask, flag);

    switch (flag)
    {
        case D3DCOMPILE_OPTIMIZATION_LEVEL0:
            return isFlagSet && !IsMaskFlagSet(mask, UINT(D3DCOMPILE_OPTIMIZATION_LEVEL3));

        case D3DCOMPILE_OPTIMIZATION_LEVEL1:
            return (mask & D3DCOMPILE_OPTIMIZATION_LEVEL2) == UINT(0);

        case D3DCOMPILE_OPTIMIZATION_LEVEL3:
            return isFlagSet && !IsMaskFlagSet(mask, UINT(D3DCOMPILE_OPTIMIZATION_LEVEL0));

        default:
            return isFlagSet;
    }
}
#endif  // ANGLE_APPEND_ASSEMBLY_TO_SHADER_DEBUG_INFO

enum D3DCompilerLoadLibraryResult
{
    D3DCompilerDefaultLibrarySuccess,
    D3DCompilerFailure,
    D3DCompilerEnumBoundary,
};
}  // anonymous namespace

namespace rx
{

CompileConfig::CompileConfig() : flags(0), name() {}

CompileConfig::CompileConfig(UINT flags, const std::string &name) : flags(flags), name(name) {}

HLSLCompiler::HLSLCompiler()
    : mInitialized(false),
      mD3DCompilerModule(nullptr),
      mD3DCompileFunc(nullptr),
      mD3DDisassembleFunc(nullptr)
{}

HLSLCompiler::~HLSLCompiler()
{
    release();
}

angle::Result HLSLCompiler::ensureInitialized(d3d::Context *context)
{
    if (mInitialized)
    {
        return angle::Result::Continue;
    }

    ANGLE_TRACE_EVENT0("gpu.angle", "HLSLCompiler::initialize");
#if !defined(ANGLE_ENABLE_WINDOWS_UWP)
#    if defined(ANGLE_PRELOADED_D3DCOMPILER_MODULE_NAMES)
    // Find a D3DCompiler module that had already been loaded based on a predefined list of
    // versions.
    static const char *d3dCompilerNames[] = ANGLE_PRELOADED_D3DCOMPILER_MODULE_NAMES;

    for (size_t i = 0; i < ArraySize(d3dCompilerNames); ++i)
    {
        if (GetModuleHandleExA(0, d3dCompilerNames[i], &mD3DCompilerModule))
        {
            break;
        }
    }
#    endif  // ANGLE_PRELOADED_D3DCOMPILER_MODULE_NAMES

    if (!mD3DCompilerModule)
    {
        // Load the version of the D3DCompiler DLL associated with the Direct3D version ANGLE was
        // built with.
        mD3DCompilerModule = LoadLibraryA(D3DCOMPILER_DLL_A);

        if (mD3DCompilerModule)
        {
            ANGLE_HISTOGRAM_ENUMERATION("GPU.ANGLE.D3DCompilerLoadLibraryResult",
                                        D3DCompilerDefaultLibrarySuccess, D3DCompilerEnumBoundary);
        }
    }

    if (!mD3DCompilerModule)
    {
        DWORD lastError = GetLastError();
        ERR() << "D3D Compiler LoadLibrary failed. GetLastError=" << lastError;
        ANGLE_HISTOGRAM_ENUMERATION("GPU.ANGLE.D3DCompilerLoadLibraryResult", D3DCompilerFailure,
                                    D3DCompilerEnumBoundary);
        ANGLE_TRY_HR(context, E_OUTOFMEMORY, "LoadLibrary failed to load D3D Compiler DLL.");
    }

    mD3DCompileFunc =
        reinterpret_cast<pD3DCompile>(GetProcAddress(mD3DCompilerModule, "D3DCompile"));
    ASSERT(mD3DCompileFunc);

    mD3DDisassembleFunc =
        reinterpret_cast<pD3DDisassemble>(GetProcAddress(mD3DCompilerModule, "D3DDisassemble"));
    ASSERT(mD3DDisassembleFunc);

#else
    // D3D Shader compiler is linked already into this module, so the export
    // can be directly assigned.
    mD3DCompilerModule  = nullptr;
    mD3DCompileFunc     = reinterpret_cast<pD3DCompile>(D3DCompile);
    mD3DDisassembleFunc = reinterpret_cast<pD3DDisassemble>(D3DDisassemble);
#endif

    ANGLE_CHECK_HR(context, mD3DCompileFunc, "Error finding D3DCompile entry point.",
                   E_OUTOFMEMORY);

    mInitialized = true;
    return angle::Result::Continue;
}

void HLSLCompiler::release()
{
    if (mInitialized)
    {
        FreeLibrary(mD3DCompilerModule);
        mD3DCompilerModule  = nullptr;
        mD3DCompileFunc     = nullptr;
        mD3DDisassembleFunc = nullptr;
        mInitialized        = false;
    }
}

angle::Result HLSLCompiler::compileToBinary(d3d::Context *context,
                                            gl::InfoLog &infoLog,
                                            const std::string &hlsl,
                                            const std::string &profile,
                                            const std::vector<CompileConfig> &configs,
                                            const D3D_SHADER_MACRO *overrideMacros,
                                            ID3DBlob **outCompiledBlob,
                                            std::string *outDebugInfo)
{
    ASSERT(mInitialized);

#if !defined(ANGLE_ENABLE_WINDOWS_UWP)
    ASSERT(mD3DCompilerModule);
#endif
    ASSERT(mD3DCompileFunc);

#if !defined(ANGLE_ENABLE_WINDOWS_UWP) && defined(ANGLE_ENABLE_DEBUG_TRACE)
    std::string sourcePath = angle::CreateTemporaryFile().value();
    std::ostringstream stream;
    stream << "#line 2 \"" << sourcePath << "\"\n\n" << hlsl;
    std::string sourceText = stream.str();
    writeFile(sourcePath.c_str(), sourceText.c_str(), sourceText.size());
#endif

    auto *platform = ANGLEPlatformCurrent();

    const D3D_SHADER_MACRO *macros = overrideMacros ? overrideMacros : nullptr;

    for (size_t i = 0; i < configs.size(); ++i)
    {
        ID3DBlob *errorMessage = nullptr;
        ID3DBlob *binary       = nullptr;
        HRESULT result         = S_OK;

        double startTime = platform->currentTime(platform);

        {
            ANGLE_TRACE_EVENT1("gpu.angle", "D3DCompile", "source", hlsl);
            result = mD3DCompileFunc(hlsl.c_str(), hlsl.length(), gl::g_fakepath, macros, nullptr,
                                     "main", profile.c_str(), configs[i].flags, 0, &binary,
                                     &errorMessage);
        }

        double compileTime = platform->currentTime(platform) - startTime;

        if (errorMessage)
        {
            std::string message = static_cast<const char *>(errorMessage->GetBufferPointer());
            SafeRelease(errorMessage);
            ANGLE_TRACE_EVENT1("gpu.angle", "D3DCompile::Error", "error", errorMessage);

            infoLog.appendSanitized(message.c_str());

            // This produces unbelievable amounts of spam in about:gpu.
            // WARN() << std::endl << hlsl;

            WARN() << std::endl << message;

            if (macros != nullptr)
            {
                constexpr const char *kLoopRelatedErrors[] = {
                    // "can't unroll loops marked with loop attribute"
                    "error X3531:",

                    // "cannot have gradient operations inside loops with divergent flow control",
                    // even though it is counter-intuitive to disable unrolling for this error, some
                    // very long shaders have trouble deciding which loops to unroll and turning off
                    // forced unrolls allows them to compile properly.
                    "error X4014:",

                    // "array index out of bounds", loop unrolling can result in invalid array
                    // access if the indices become constant, causing loops that may never be
                    // executed to generate compilation errors
                    "error X3504:",
                };

                bool hasLoopRelatedError = false;
                for (const char *errorType : kLoopRelatedErrors)
                {
                    if (message.find(errorType) != std::string::npos)
                    {
                        hasLoopRelatedError = true;
                        break;
                    }
                }

                if (hasLoopRelatedError)
                {
                    // Disable [loop] and [flatten]
                    macros = nullptr;

                    // Retry without changing compiler flags
                    i--;
                    continue;
                }
            }
        }

        if (SUCCEEDED(result))
        {
            int compileUs = static_cast<int>(compileTime * 1000'000.0);
            ANGLE_HISTOGRAM_COUNTS("GPU.ANGLE.D3DShaderCompilationTimeUs", compileUs);

            ANGLE_HISTOGRAM_MEMORY_KB("GPU.ANGLE.D3DShaderBlobSizeKB",
                                      static_cast<int>(binary->GetBufferSize() / 1024));

            *outCompiledBlob = binary;

            (*outDebugInfo) +=
                "// COMPILER INPUT HLSL BEGIN\n\n" + hlsl + "\n// COMPILER INPUT HLSL END\n";

#if ANGLE_APPEND_ASSEMBLY_TO_SHADER_DEBUG_INFO
            (*outDebugInfo) += "\n\n// ASSEMBLY BEGIN\n\n";
            (*outDebugInfo) += "// Compiler configuration: " + configs[i].name + "\n// Flags:\n";
            for (size_t fIx = 0; fIx < ArraySize(CompilerFlagInfos); ++fIx)
            {
                if (IsCompilerFlagSet(configs[i].flags, CompilerFlagInfos[fIx].mFlag))
                {
                    (*outDebugInfo) += std::string("// ") + CompilerFlagInfos[fIx].mName + "\n";
                }
            }

            (*outDebugInfo) += "// Macros:\n";
            if (macros == nullptr)
            {
                (*outDebugInfo) += "// - : -\n";
            }
            else
            {
                for (const D3D_SHADER_MACRO *mIt = macros; mIt->Name != nullptr; ++mIt)
                {
                    (*outDebugInfo) +=
                        std::string("// ") + mIt->Name + " : " + mIt->Definition + "\n";
                }
            }

            std::string disassembly;
            ANGLE_TRY(disassembleBinary(context, binary, &disassembly));
            (*outDebugInfo) += "\n" + disassembly + "\n// ASSEMBLY END\n";
#endif  // ANGLE_APPEND_ASSEMBLY_TO_SHADER_DEBUG_INFO
            return angle::Result::Continue;
        }

        if (result == E_OUTOFMEMORY)
        {
            *outCompiledBlob = nullptr;
            ANGLE_TRY_HR(context, result, "HLSL compiler had an unexpected failure");
        }

        infoLog << "Warning: D3D shader compilation failed with " << configs[i].name << " flags. ("
                << profile << ")";

        if (i + 1 < configs.size())
        {
            infoLog << " Retrying with " << configs[i + 1].name;
        }
    }

    // None of the configurations succeeded in compiling this shader but the compiler is still
    // intact
    *outCompiledBlob = nullptr;
    return angle::Result::Continue;
}

angle::Result HLSLCompiler::disassembleBinary(d3d::Context *context,
                                              ID3DBlob *shaderBinary,
                                              std::string *disassemblyOut)
{
    ANGLE_TRY(ensureInitialized(context));

    // Retrieve disassembly
    UINT flags = D3D_DISASM_ENABLE_DEFAULT_VALUE_PRINTS | D3D_DISASM_ENABLE_INSTRUCTION_NUMBERING;
    ID3DBlob *disassembly           = nullptr;
    pD3DDisassemble disassembleFunc = reinterpret_cast<pD3DDisassemble>(mD3DDisassembleFunc);
    LPCVOID buffer                  = shaderBinary->GetBufferPointer();
    SIZE_T bufSize                  = shaderBinary->GetBufferSize();
    HRESULT result                  = disassembleFunc(buffer, bufSize, flags, "", &disassembly);

    if (SUCCEEDED(result))
    {
        *disassemblyOut = std::string(static_cast<const char *>(disassembly->GetBufferPointer()));
    }
    else
    {
        *disassemblyOut = "";
    }

    SafeRelease(disassembly);

    return angle::Result::Continue;
}

}  // namespace rx
