///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcutil.h                                                                 //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides helper code for dxcompiler.                                      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dxc/Support/microcom.h"
#include "dxc/dxcapi.h"
#include "llvm/ADT/StringRef.h"
#include <memory>

#define DISABLE_GET_CUSTOM_DIAG_ID 1

namespace clang {
class DiagnosticsEngine;
}

namespace llvm {
class LLVMContext;
class MemoryBuffer;
class Module;
class raw_string_ostream;
class Twine;
} // namespace llvm

namespace hlsl {
enum class SerializeDxilFlags : uint32_t;
struct DxilShaderHash;
class AbstractMemoryStream;
namespace options {
class MainArgs;
class DxcOpts;
} // namespace options
} // namespace hlsl

namespace dxcutil {
struct AssembleInputs {
  AssembleInputs(std::unique_ptr<llvm::Module> &&pM,
                 CComPtr<IDxcBlob> &pOutputContainerBlob, IMalloc *pMalloc,
                 hlsl::SerializeDxilFlags SerializeFlags,
                 CComPtr<hlsl::AbstractMemoryStream> &pModuleBitcode,
                 bool bDebugInfo = false,
                 llvm::StringRef DebugName = llvm::StringRef(),
                 clang::DiagnosticsEngine *pDiag = nullptr,
                 hlsl::DxilShaderHash *pShaderHashOut = nullptr,
                 hlsl::AbstractMemoryStream *pReflectionOut = nullptr,
                 hlsl::AbstractMemoryStream *pRootSigOut = nullptr);
  std::unique_ptr<llvm::Module> pM;
  CComPtr<IDxcBlob> &pOutputContainerBlob;
  IMalloc *pMalloc;
  hlsl::SerializeDxilFlags SerializeFlags;
  CComPtr<hlsl::AbstractMemoryStream> &pModuleBitcode;
  bool bDebugInfo;
  llvm::StringRef DebugName = llvm::StringRef();
  clang::DiagnosticsEngine *pDiag;
  hlsl::DxilShaderHash *pShaderHashOut = nullptr;
  hlsl::AbstractMemoryStream *pReflectionOut = nullptr;
  hlsl::AbstractMemoryStream *pRootSigOut = nullptr;
};
HRESULT ValidateAndAssembleToContainer(AssembleInputs &inputs);
HRESULT
ValidateRootSignatureInContainer(IDxcBlob *pRootSigContainer,
                                 clang::DiagnosticsEngine *pDiag = nullptr);
void GetValidatorVersion(unsigned *pMajor, unsigned *pMinor);
void AssembleToContainer(AssembleInputs &inputs);
HRESULT Disassemble(IDxcBlob *pProgram, llvm::raw_string_ostream &Stream);
void ReadOptsAndValidate(hlsl::options::MainArgs &mainArgs,
                         hlsl::options::DxcOpts &opts,
                         hlsl::AbstractMemoryStream *pOutputStream,
                         IDxcOperationResult **ppResult, bool &finished);
void CreateOperationResultFromOutputs(
    DXC_OUT_KIND resultKind, UINT32 textEncoding, IDxcBlob *pResultBlob,
    CComPtr<IStream> &pErrorStream, const std::string &warnings,
    bool hasErrorOccurred, IDxcOperationResult **ppResult);
void CreateOperationResultFromOutputs(IDxcBlob *pResultBlob,
                                      CComPtr<IStream> &pErrorStream,
                                      const std::string &warnings,
                                      bool hasErrorOccurred,
                                      IDxcOperationResult **ppResult);

bool IsAbsoluteOrCurDirRelative(const llvm::Twine &T);

} // namespace dxcutil
