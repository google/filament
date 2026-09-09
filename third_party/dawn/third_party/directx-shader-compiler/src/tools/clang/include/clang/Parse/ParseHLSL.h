//===--- ParseHLSL.h - Standalone HLSL parsing -----------------*- C++ -*-===//
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ParseHLSL.h                                                               //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//  This file defines the clang::ParseAST method.                            //
//
///////////////////////////////////////////////////////////////////////////////

#ifndef LLVM_CLANG_PARSE_PARSEHLSL_H
#define LLVM_CLANG_PARSE_PARSEHLSL_H

namespace llvm {
class raw_ostream;
}

namespace hlsl {
enum class DxilRootSignatureVersion;
enum class DxilRootSignatureCompilationFlags;
struct DxilVersionedRootSignatureDesc;
class RootSignatureHandle;
} // namespace hlsl

namespace clang {
class DiagnosticsEngine;

bool ParseHLSLRootSignature(const char *pData, unsigned Len,
                            hlsl::DxilRootSignatureVersion Ver,
                            hlsl::DxilRootSignatureCompilationFlags Flags,
                            hlsl::DxilVersionedRootSignatureDesc **ppDesc,
                            clang::SourceLocation Loc,
                            clang::DiagnosticsEngine &Diags);
void ReportHLSLRootSigError(clang::DiagnosticsEngine &Diags,
                            clang::SourceLocation Loc, const char *pData,
                            unsigned Len);
void CompileRootSignature(StringRef rootSigStr, DiagnosticsEngine &Diags,
                          SourceLocation SLoc,
                          hlsl::DxilRootSignatureVersion rootSigVer,
                          hlsl::DxilRootSignatureCompilationFlags flags,
                          hlsl::RootSignatureHandle *pRootSigHandle);
} // namespace clang

#endif
