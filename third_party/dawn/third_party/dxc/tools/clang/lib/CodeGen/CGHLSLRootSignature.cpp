//===----- CGHLSLRootSignature.cpp - Compile root signature---------------===//
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// CGHLSLRootSignature.cpp                                                   //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//  This provides clang::CompileRootSignature.                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

#include "dxc/DXIL/DxilConstants.h"
#include "dxc/DxilRootSignature/DxilRootSignature.h"
#include "dxc/Support/WinIncludes.h" // stream support
#include "dxc/dxcapi.h"              // stream support
#include "dxc/dxcapi.h"
#include "clang/Parse/ParseHLSL.h" // root sig would be in Parser if part of lang

using namespace llvm;

void clang::CompileRootSignature(StringRef rootSigStr, DiagnosticsEngine &Diags,
                                 SourceLocation SLoc,
                                 hlsl::DxilRootSignatureVersion rootSigVer,
                                 hlsl::DxilRootSignatureCompilationFlags flags,
                                 hlsl::RootSignatureHandle *pRootSigHandle) {
  std::string OSStr;
  llvm::raw_string_ostream OS(OSStr);
  hlsl::DxilVersionedRootSignatureDesc *D = nullptr;

  if (ParseHLSLRootSignature(rootSigStr.data(), rootSigStr.size(), rootSigVer,
                             flags, &D, SLoc, Diags)) {
    CComPtr<IDxcBlob> pSignature;
    CComPtr<IDxcBlobEncoding> pErrors;
    hlsl::SerializeRootSignature(D, &pSignature, &pErrors, false);
    if (pSignature == nullptr) {
      assert(pErrors != nullptr && "else serialize failed with no msg");
      ReportHLSLRootSigError(Diags, SLoc, (char *)pErrors->GetBufferPointer(),
                             pErrors->GetBufferSize());
      hlsl::DeleteRootSignature(D);
    } else {
      pRootSigHandle->Assign(D, pSignature);
    }
  }
}