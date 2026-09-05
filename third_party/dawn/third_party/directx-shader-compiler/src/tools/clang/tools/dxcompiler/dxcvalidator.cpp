///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcvalidator.cpp                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implements the DirectX Validator object.                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxcvalidator.h"

#include "dxc/Support/WinIncludes.h"

#include "dxc/Support/Global.h"
#include "dxc/Support/microcom.h"
#include "dxc/dxcapi.h"

#ifdef SUPPORT_QUERY_GIT_COMMIT_INFO
#include "clang/Basic/Version.h"
#endif // SUPPORT_QUERY_GIT_COMMIT_INFO

using namespace llvm;
using namespace hlsl;

class DxcValidator : public IDxcValidator2,
#ifdef SUPPORT_QUERY_GIT_COMMIT_INFO
                     public IDxcVersionInfo2
#else
                     public IDxcVersionInfo
#endif // SUPPORT_QUERY_GIT_COMMIT_INFO
{
private:
  DXC_MICROCOM_TM_REF_FIELDS()

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_CTOR(DxcValidator)

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcValidator, IDxcValidator2,
                                 IDxcVersionInfo>(this, iid, ppvObject);
  }

  // For internal use only.
  HRESULT ValidateWithOptDebugModule(
      IDxcBlob *pShader,          // Shader to validate.
      UINT32 Flags,               // Validation flags.
      llvm::Module *pDebugModule, // Debug module to validate, if available
      IDxcOperationResult *
          *ppResult // Validation output status, buffer, and errors
  );

  // IDxcValidator
  HRESULT STDMETHODCALLTYPE Validate(
      IDxcBlob *pShader, // Shader to validate.
      UINT32 Flags,      // Validation flags.
      IDxcOperationResult *
          *ppResult // Validation output status, buffer, and errors
      ) override;

  // IDxcValidator2
  HRESULT STDMETHODCALLTYPE ValidateWithDebug(
      IDxcBlob *pShader,           // Shader to validate.
      UINT32 Flags,                // Validation flags.
      DxcBuffer *pOptDebugBitcode, // Optional debug module bitcode to provide
                                   // line numbers
      IDxcOperationResult *
          *ppResult // Validation output status, buffer, and errors
      ) override;

  // IDxcVersionInfo
  HRESULT STDMETHODCALLTYPE GetVersion(UINT32 *pMajor, UINT32 *pMinor) override;
  HRESULT STDMETHODCALLTYPE GetFlags(UINT32 *pFlags) override;

#ifdef SUPPORT_QUERY_GIT_COMMIT_INFO
  // IDxcVersionInfo2
  HRESULT STDMETHODCALLTYPE GetCommitInfo(UINT32 *pCommitCount,
                                          char **pCommitHash) override;
#endif
};

// Compile a single entry point to the target shader model
HRESULT STDMETHODCALLTYPE DxcValidator::Validate(
    IDxcBlob *pShader, // Shader to validate.
    UINT32 Flags,      // Validation flags.
    IDxcOperationResult *
        *ppResult // Validation output status, buffer, and errors
) {
  return hlsl::validateWithDebug(pShader, Flags, nullptr, ppResult);
}

HRESULT STDMETHODCALLTYPE DxcValidator::ValidateWithDebug(
    IDxcBlob *pShader,           // Shader to validate.
    UINT32 Flags,                // Validation flags.
    DxcBuffer *pOptDebugBitcode, // Optional debug module bitcode to provide
                                 // line numbers
    IDxcOperationResult *
        *ppResult // Validation output status, buffer, and errors
) {
  return hlsl::validateWithDebug(pShader, Flags, pOptDebugBitcode, ppResult);
}

HRESULT DxcValidator::ValidateWithOptDebugModule(
    IDxcBlob *pShader,          // Shader to validate.
    UINT32 Flags,               // Validation flags.
    llvm::Module *pDebugModule, // Debug module to validate, if available
    IDxcOperationResult *
        *ppResult // Validation output status, buffer, and errors
) {
  return hlsl::validateWithOptDebugModule(pShader, Flags, pDebugModule,
                                          ppResult);
}

HRESULT STDMETHODCALLTYPE DxcValidator::GetVersion(UINT32 *pMajor,
                                                   UINT32 *pMinor) {
  return hlsl::getValidationVersion(pMajor, pMinor);
}

#ifdef SUPPORT_QUERY_GIT_COMMIT_INFO
HRESULT STDMETHODCALLTYPE DxcValidator::GetCommitInfo(UINT32 *pCommitCount,
                                                      char **pCommitHash) {
  if (pCommitCount == nullptr || pCommitHash == nullptr)
    return E_INVALIDARG;

  char *const hash = (char *)CoTaskMemAlloc(
      8 + 1); // 8 is guaranteed by utils/GetCommitInfo.py
  if (hash == nullptr)
    return E_OUTOFMEMORY;
  std::strcpy(hash, clang::getGitCommitHash());

  *pCommitHash = hash;
  *pCommitCount = clang::getGitCommitCount();

  return S_OK;
}
#endif // SUPPORT_QUERY_GIT_COMMIT_INFO

HRESULT STDMETHODCALLTYPE DxcValidator::GetFlags(UINT32 *pFlags) {
  if (pFlags == nullptr)
    return E_INVALIDARG;
  *pFlags = DxcVersionInfoFlags_None;
#ifndef NDEBUG
  *pFlags |= DxcVersionInfoFlags_Debug;
#endif
  *pFlags |= DxcVersionInfoFlags_Internal;
  return S_OK;
}

///////////////////////////////////////////////////////////////////////////////

HRESULT RunInternalValidator(IDxcValidator *pValidator,
                             llvm::Module *pDebugModule, IDxcBlob *pShader,
                             UINT32 Flags, IDxcOperationResult **ppResult) {
  DXASSERT_NOMSG(pValidator != nullptr);
  DXASSERT_NOMSG(pShader != nullptr);
  DXASSERT_NOMSG(ppResult != nullptr);

  DxcValidator *pInternalValidator = (DxcValidator *)pValidator;
  return pInternalValidator->ValidateWithOptDebugModule(pShader, Flags,
                                                        pDebugModule, ppResult);
}

HRESULT CreateDxcValidator(REFIID riid, LPVOID *ppv) {
  try {
    CComPtr<DxcValidator> result(
        DxcValidator::Alloc(DxcGetThreadMallocNoRef()));
    IFROOM(result.p);
    return result.p->QueryInterface(riid, ppv);
  }
  CATCH_CPP_RETURN_HRESULT();
}
