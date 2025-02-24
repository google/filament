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

using namespace llvm;
using namespace hlsl;

class DxcValidator : public IDxcValidator, public IDxcVersionInfo {
private:
  DXC_MICROCOM_TM_REF_FIELDS()

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_CTOR(DxcValidator)

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcValidator, IDxcVersionInfo>(this, iid,
                                                                 ppvObject);
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

  // IDxcVersionInfo
  HRESULT STDMETHODCALLTYPE GetVersion(UINT32 *pMajor, UINT32 *pMinor) override;
  HRESULT STDMETHODCALLTYPE GetFlags(UINT32 *pFlags) override;
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
