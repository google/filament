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

#include "dxc/DXIL/DxilModule.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/microcom.h"
#include "dxc/dxcapi.h"
#include "llvm/ADT/StringRef.h"

#include <memory>

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
                 uint32_t ValidationFlags = 0,
                 llvm::StringRef DebugName = llvm::StringRef(),
                 clang::DiagnosticsEngine *pDiag = nullptr,
                 hlsl::DxilShaderHash *pShaderHashOut = nullptr,
                 hlsl::AbstractMemoryStream *pReflectionOut = nullptr,
                 hlsl::AbstractMemoryStream *pRootSigOut = nullptr,
                 CComPtr<IDxcBlob> pRootSigBlob = nullptr,
                 CComPtr<IDxcBlob> pPrivateBlob = nullptr);
  std::unique_ptr<llvm::Module> pM;
  CComPtr<IDxcBlob> &pOutputContainerBlob;
  IDxcVersionInfo *pVersionInfo = nullptr;
  IMalloc *pMalloc;
  hlsl::SerializeDxilFlags SerializeFlags;
  uint32_t ValidationFlags = 0;
  CComPtr<hlsl::AbstractMemoryStream> &pModuleBitcode;
  llvm::StringRef DebugName = llvm::StringRef();
  clang::DiagnosticsEngine *pDiag;
  hlsl::DxilShaderHash *pShaderHashOut = nullptr;
  hlsl::AbstractMemoryStream *pReflectionOut = nullptr;
  hlsl::AbstractMemoryStream *pRootSigOut = nullptr;
  CComPtr<IDxcBlob> pRootSigBlob = nullptr;
  CComPtr<IDxcBlob> pPrivateBlob = nullptr;
};
HRESULT ValidateAndAssembleToContainer(AssembleInputs &inputs);
HRESULT
ValidateRootSignatureInContainer(IDxcBlob *pRootSigContainer,
                                 clang::DiagnosticsEngine *pDiag = nullptr);
HRESULT SetRootSignature(hlsl::DxilModule *pModule, CComPtr<IDxcBlob> pSource);
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
