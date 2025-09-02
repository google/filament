///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcutil.cpp                                                               //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides helper code for dxcompiler.                                      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxcutil.h"
#include "dxc/DXIL/DxilModule.h"
#include "dxc/DxilContainer/DxilContainerAssembler.h"
#include "dxc/DxilRootSignature/DxilRootSignature.h"
#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/Support/dxcapi.impl.h"
#include "dxc/dxcapi.h"
#include "clang/Basic/Diagnostic.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "llvm/Support/Path.h"

using namespace llvm;
using namespace hlsl;

// This declaration is used for the locally-linked validator.
HRESULT CreateDxcValidator(REFIID riid, LPVOID *ppv);
// This internal call allows the validator to avoid having to re-deserialize
// the module. It trusts that the caller didn't make any changes and is
// kept internal because the layout of the module class may change based
// on changes across modules, or picking a different compiler version or CRT.
HRESULT RunInternalValidator(IDxcValidator *pValidator,
                             llvm::Module *pDebugModule, IDxcBlob *pShader,
                             UINT32 Flags, IDxcOperationResult **ppResult);

namespace {
// AssembleToContainer helper functions.

// return true if the internal validator was used, false otherwise
void CreateValidator(CComPtr<IDxcValidator> &pValidator) {
  IFT(CreateDxcValidator(IID_PPV_ARGS(&pValidator)));
}

} // namespace

namespace dxcutil {

AssembleInputs::AssembleInputs(
    std::unique_ptr<llvm::Module> &&pM, CComPtr<IDxcBlob> &pOutputContainerBlob,
    IMalloc *pMalloc, hlsl::SerializeDxilFlags SerializeFlags,
    CComPtr<hlsl::AbstractMemoryStream> &pModuleBitcode,
    uint32_t ValidationFlags, llvm::StringRef DebugName,
    clang::DiagnosticsEngine *pDiag, hlsl::DxilShaderHash *pShaderHashOut,
    AbstractMemoryStream *pReflectionOut, AbstractMemoryStream *pRootSigOut,
    CComPtr<IDxcBlob> pRootSigBlob, CComPtr<IDxcBlob> pPrivateBlob)
    : pM(std::move(pM)), pOutputContainerBlob(pOutputContainerBlob),
      pMalloc(pMalloc), SerializeFlags(SerializeFlags),
      ValidationFlags(ValidationFlags), pModuleBitcode(pModuleBitcode),
      DebugName(DebugName), pDiag(pDiag), pShaderHashOut(pShaderHashOut),
      pReflectionOut(pReflectionOut), pRootSigOut(pRootSigOut),
      pRootSigBlob(pRootSigBlob), pPrivateBlob(pPrivateBlob) {}

void GetValidatorVersion(unsigned *pMajor, unsigned *pMinor) {
  if (pMajor == nullptr || pMinor == nullptr)
    return;

  CComPtr<IDxcValidator> pValidator;
  CreateValidator(pValidator);

  CComPtr<IDxcVersionInfo> pVersionInfo;
  if (SUCCEEDED(pValidator.QueryInterface(&pVersionInfo))) {
    IFT(pVersionInfo->GetVersion(pMajor, pMinor));
  } else {
    // Default to 1.0
    *pMajor = 1;
    *pMinor = 0;
  }
}

void AssembleToContainer(AssembleInputs &inputs) {
  CComPtr<AbstractMemoryStream> pContainerStream;
  IFT(CreateMemoryStream(inputs.pMalloc, &pContainerStream));
  if (!(inputs.SerializeFlags & SerializeDxilFlags::StripRootSignature) &&
      inputs.pRootSigBlob) {
    IFT(SetRootSignature(&inputs.pM->GetOrCreateDxilModule(),
                         inputs.pRootSigBlob));
  } // Update the module root signature from file
  if (inputs.pPrivateBlob) {
    SerializeDxilContainerForModule(
        &inputs.pM->GetOrCreateDxilModule(), inputs.pModuleBitcode,
        inputs.pVersionInfo, pContainerStream, inputs.DebugName,
        inputs.SerializeFlags, inputs.pShaderHashOut, inputs.pReflectionOut,
        inputs.pRootSigOut, inputs.pPrivateBlob->GetBufferPointer(),
        inputs.pPrivateBlob->GetBufferSize());
  } else {
    SerializeDxilContainerForModule(
        &inputs.pM->GetOrCreateDxilModule(), inputs.pModuleBitcode,
        inputs.pVersionInfo, pContainerStream, inputs.DebugName,
        inputs.SerializeFlags, inputs.pShaderHashOut, inputs.pReflectionOut,
        inputs.pRootSigOut);
  }
  inputs.pOutputContainerBlob.Release();
  IFT(pContainerStream.QueryInterface(&inputs.pOutputContainerBlob));
}

void ReadOptsAndValidate(hlsl::options::MainArgs &mainArgs,
                         hlsl::options::DxcOpts &opts,
                         AbstractMemoryStream *pOutputStream,
                         IDxcOperationResult **ppResult, bool &finished) {
  const llvm::opt::OptTable *table = ::options::getHlslOptTable();
  raw_stream_ostream outStream(pOutputStream);
  if (0 != hlsl::options::ReadDxcOpts(table, hlsl::options::CompilerFlags,
                                      mainArgs, opts, outStream)) {
    CComPtr<IDxcBlob> pErrorBlob;
    IFT(pOutputStream->QueryInterface(&pErrorBlob));
    outStream.flush();
    IFT(DxcResult::Create(
        E_INVALIDARG, DXC_OUT_NONE,
        {DxcOutputObject::ErrorOutput(opts.DefaultTextCodePage,
                                      (LPCSTR)pErrorBlob->GetBufferPointer(),
                                      pErrorBlob->GetBufferSize())},
        ppResult));
    finished = true;
    return;
  }
  DXASSERT(opts.HLSLVersion > hlsl::LangStd::v2015,
           "else ReadDxcOpts didn't fail for non-isense");
  finished = false;
}

HRESULT ValidateAndAssembleToContainer(AssembleInputs &inputs) {
  HRESULT valHR = S_OK;

  // If we have debug info, this will be a clone of the module before debug info
  // is stripped. This is used with internal validator to provide more useful
  // error messages.
  std::unique_ptr<llvm::Module> llvmModuleWithDebugInfo;

  CComPtr<IDxcValidator> pValidator;
  CreateValidator(pValidator);

  if (llvm::getDebugMetadataVersionFromModule(*inputs.pM) != 0)
    llvmModuleWithDebugInfo.reset(llvm::CloneModule(inputs.pM.get()));

  AssembleToContainer(inputs);

  CComPtr<IDxcOperationResult> pValResult;
  // In-place edit to avoid an extra copy
  inputs.ValidationFlags |= DxcValidatorFlags_InPlaceEdit;
  IFT(RunInternalValidator(pValidator, llvmModuleWithDebugInfo.get(),
                           inputs.pOutputContainerBlob, inputs.ValidationFlags,
                           &pValResult));
  IFT(pValResult->GetStatus(&valHR));
  if (inputs.pDiag) {
    if (FAILED(valHR)) {
      CComPtr<IDxcBlobEncoding> pErrors;
      CComPtr<IDxcBlobUtf8> pErrorsUtf8;
      IFT(pValResult->GetErrorBuffer(&pErrors));
      IFT(hlsl::DxcGetBlobAsUtf8(pErrors, inputs.pMalloc, &pErrorsUtf8));
      StringRef errRef(pErrorsUtf8->GetStringPointer(),
                       pErrorsUtf8->GetStringLength());
      unsigned DiagID = inputs.pDiag->getCustomDiagID(
          clang::DiagnosticsEngine::Error, "validation errors\r\n%0");
      inputs.pDiag->Report(DiagID) << errRef;
    }
  }
  CComPtr<IDxcBlob> pValidatedBlob;
  IFT(pValResult->GetResult(&pValidatedBlob));
  if (pValidatedBlob != nullptr) {
    std::swap(inputs.pOutputContainerBlob, pValidatedBlob);
  }
  pValidator.Release();

  return valHR;
}

HRESULT ValidateRootSignatureInContainer(IDxcBlob *pRootSigContainer,
                                         clang::DiagnosticsEngine *pDiag) {
  HRESULT valHR = S_OK;
  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pValResult;
  CreateValidator(pValidator);
  IFT(pValidator->Validate(pRootSigContainer,
                           DxcValidatorFlags_RootSignatureOnly |
                               DxcValidatorFlags_InPlaceEdit,
                           &pValResult));
  IFT(pValResult->GetStatus(&valHR));
  if (pDiag) {
    if (FAILED(valHR)) {
      CComPtr<IDxcBlobEncoding> pErrors;
      CComPtr<IDxcBlobUtf8> pErrorsUtf8;
      IFT(pValResult->GetErrorBuffer(&pErrors));
      IFT(hlsl::DxcGetBlobAsUtf8(pErrors, nullptr, &pErrorsUtf8));
      StringRef errRef(pErrorsUtf8->GetStringPointer(),
                       pErrorsUtf8->GetStringLength());
      unsigned DiagID =
          pDiag->getCustomDiagID(clang::DiagnosticsEngine::Error,
                                 "root signature validation errors\r\n%0");
      pDiag->Report(DiagID) << errRef;
    }
  }
  return valHR;
}

HRESULT SetRootSignature(hlsl::DxilModule *pModule, CComPtr<IDxcBlob> pSource) {
  const char *pName = "RTS0";
  const UINT32 partKind = ((UINT32)pName[0] | ((UINT32)pName[1] << 8) |
                           ((UINT32)pName[2] << 16) | ((UINT32)pName[3] << 24));

  CComPtr<IDxcContainerReflection> pReflection;
  UINT32 partCount;
  IFT(DxcCreateInstance(CLSID_DxcContainerReflection,
                        __uuidof(IDxcContainerReflection),
                        (void **)&pReflection));
  IFT(pReflection->Load(pSource));
  IFT(pReflection->GetPartCount(&partCount));

  CComPtr<IDxcBlob> pContent;
  for (UINT32 i = 0; i < partCount; ++i) {
    UINT32 curPartKind;
    IFT(pReflection->GetPartKind(i, &curPartKind));
    if (curPartKind == partKind) {
      CComPtr<IDxcBlob> pContent;
      IFT(pReflection->GetPartContent(i, &pContent));

      try {
        const void *serializedData = pContent->GetBufferPointer();
        uint32_t serializedSize = pContent->GetBufferSize();
        hlsl::RootSignatureHandle rootSig;
        rootSig.LoadSerialized(static_cast<const uint8_t *>(serializedData),
                               serializedSize);
        std::vector<uint8_t> serializedRootSignature;
        serializedRootSignature.assign(rootSig.GetSerializedBytes(),
                                       rootSig.GetSerializedBytes() +
                                           rootSig.GetSerializedSize());
        pModule->ResetSerializedRootSignature(serializedRootSignature);
      } catch (...) {
        return DXC_E_INCORRECT_ROOT_SIGNATURE;
      }
    }
  }
  return S_OK;
}

void CreateOperationResultFromOutputs(
    DXC_OUT_KIND resultKind, UINT32 textEncoding, IDxcBlob *pResultBlob,
    CComPtr<IStream> &pErrorStream, const std::string &warnings,
    bool hasErrorOccurred, IDxcOperationResult **ppResult) {
  CComPtr<DxcResult> pResult = DxcResult::Alloc(DxcGetThreadMallocNoRef());
  IFT(pResult->SetEncoding(textEncoding));
  IFT(pResult->SetStatusAndPrimaryResult(hasErrorOccurred ? E_FAIL : S_OK,
                                         resultKind));
  IFT(pResult->SetOutputObject(resultKind, pResultBlob));
  CComPtr<IDxcBlob> pErrorBlob;
  IFT(pErrorStream.QueryInterface(&pErrorBlob));
  if (IsBlobNullOrEmpty(pErrorBlob)) {
    IFT(pResult->SetOutputString(DXC_OUT_ERRORS, warnings.c_str(),
                                 warnings.size()));
  } else {
    IFT(pResult->SetOutputObject(DXC_OUT_ERRORS, pErrorBlob));
  }
  IFT(pResult.QueryInterface(ppResult));
}

void CreateOperationResultFromOutputs(IDxcBlob *pResultBlob,
                                      CComPtr<IStream> &pErrorStream,
                                      const std::string &warnings,
                                      bool hasErrorOccurred,
                                      IDxcOperationResult **ppResult) {
  CreateOperationResultFromOutputs(DXC_OUT_OBJECT, DXC_CP_UTF8, pResultBlob,
                                   pErrorStream, warnings, hasErrorOccurred,
                                   ppResult);
}

bool IsAbsoluteOrCurDirRelative(const llvm::Twine &T) {
  if (llvm::sys::path::is_absolute(T)) {
    return true;
  }
  if (T.isSingleStringRef()) {
    StringRef r = T.getSingleStringRef();
    if (r.size() < 2)
      return false;
    const char *pData = r.data();
    return pData[0] == '.' && (pData[1] == '\\' || pData[1] == '/');
  }
  DXASSERT(false, "twine kind not supported");
  return false;
}

} // namespace dxcutil
