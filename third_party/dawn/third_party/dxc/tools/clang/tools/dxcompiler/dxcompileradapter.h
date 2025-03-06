///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcompileradapater.h                                                      //
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

namespace hlsl {
// This class provides an adapter for the legacy compiler interfaces
// (i.e. IDxcCompiler and IDxcCompiler2) that is backed by an IDxcCompiler3
// implemenation. It allows a single core IDxcCompiler3 implementation to be
// used to implement all IDxcCompiler interfaces.
//
// This must be owned/managed by IDxcCompiler3 instance.
class DxcCompilerAdapter : public IDxcCompiler2 {
private:
  IDxcCompiler3 *m_pCompilerImpl;
  IMalloc *m_pMalloc;

  // Internal wrapper for compile
  HRESULT WrapCompile(
      BOOL bPreprocess,    // Preprocess mode
      IDxcBlob *pSource,   // Source text to compile
      LPCWSTR pSourceName, // Optional file name for pSource. Used in errors and
                           // include handlers.
      LPCWSTR pEntryPoint, // Entry point name
      LPCWSTR pTargetProfile,              // Shader profile to compile
      LPCWSTR *pArguments,                 // Array of pointers to arguments
      UINT32 argCount,                     // Number of arguments
      const DxcDefine *pDefines,           // Array of defines
      UINT32 defineCount,                  // Number of defines
      IDxcIncludeHandler *pIncludeHandler, // user-provided interface to handle
                                           // #include directives (optional)
      IDxcOperationResult *
          *ppResult,           // Compiler output status, buffer, and errors
      LPWSTR *ppDebugBlobName, // Suggested file name for debug blob.
      IDxcBlob **ppDebugBlob   // Debug blob
  );

public:
  DxcCompilerAdapter(IDxcCompiler3 *impl, IMalloc *pMalloc)
      : m_pCompilerImpl(impl), m_pMalloc(pMalloc) {}
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override;

  // ================ IDxcCompiler ================

  // Compile a single entry point to the target shader model
  HRESULT STDMETHODCALLTYPE Compile(
      IDxcBlob *pSource,   // Source text to compile
      LPCWSTR pSourceName, // Optional file name for pSource. Used in errors and
                           // include handlers.
      LPCWSTR pEntryPoint, // entry point name
      LPCWSTR pTargetProfile,              // shader profile to compile
      LPCWSTR *pArguments,                 // Array of pointers to arguments
      UINT32 argCount,                     // Number of arguments
      const DxcDefine *pDefines,           // Array of defines
      UINT32 defineCount,                  // Number of defines
      IDxcIncludeHandler *pIncludeHandler, // user-provided interface to handle
                                           // #include directives (optional)
      IDxcOperationResult *
          *ppResult // Compiler output status, buffer, and errors
      ) override {
    return CompileWithDebug(pSource, pSourceName, pEntryPoint, pTargetProfile,
                            pArguments, argCount, pDefines, defineCount,
                            pIncludeHandler, ppResult, nullptr, nullptr);
  }

  // Preprocess source text
  HRESULT STDMETHODCALLTYPE Preprocess(
      IDxcBlob *pSource,   // Source text to preprocess
      LPCWSTR pSourceName, // Optional file name for pSource. Used in errors and
                           // include handlers.
      LPCWSTR *pArguments, // Array of pointers to arguments
      UINT32 argCount,     // Number of arguments
      const DxcDefine *pDefines,           // Array of defines
      UINT32 defineCount,                  // Number of defines
      IDxcIncludeHandler *pIncludeHandler, // user-provided interface to handle
                                           // #include directives (optional)
      IDxcOperationResult *
          *ppResult // Preprocessor output status, buffer, and errors
      ) override;

  // Disassemble a program.
  HRESULT STDMETHODCALLTYPE Disassemble(
      IDxcBlob *pSource,               // Program to disassemble.
      IDxcBlobEncoding **ppDisassembly // Disassembly text.
      ) override;

  // ================ IDxcCompiler2 ================

  // Compile a single entry point to the target shader model with debug
  // information.
  HRESULT STDMETHODCALLTYPE CompileWithDebug(
      IDxcBlob *pSource,   // Source text to compile
      LPCWSTR pSourceName, // Optional file name for pSource. Used in errors and
                           // include handlers.
      LPCWSTR pEntryPoint, // Entry point name
      LPCWSTR pTargetProfile,              // Shader profile to compile
      LPCWSTR *pArguments,                 // Array of pointers to arguments
      UINT32 argCount,                     // Number of arguments
      const DxcDefine *pDefines,           // Array of defines
      UINT32 defineCount,                  // Number of defines
      IDxcIncludeHandler *pIncludeHandler, // user-provided interface to handle
                                           // #include directives (optional)
      IDxcOperationResult *
          *ppResult,           // Compiler output status, buffer, and errors
      LPWSTR *ppDebugBlobName, // Suggested file name for debug blob.
      IDxcBlob **ppDebugBlob   // Debug blob
      ) override;
};

} // namespace hlsl
