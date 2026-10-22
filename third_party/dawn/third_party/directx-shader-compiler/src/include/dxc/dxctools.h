///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxctools.h                                                                //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides declarations for the DirectX Compiler tooling components.        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef __DXC_TOOLS__
#define __DXC_TOOLS__

#include <dxc/dxcapi.h>

enum RewriterOptionMask {
  Default = 0,
  SkipFunctionBody = 1,
  SkipStatic = 2,
  GlobalExternByDefault = 4,
  KeepUserMacro = 8,
};

CROSS_PLATFORM_UUIDOF(IDxcRewriter, "c012115b-8893-4eb9-9c5a-111456ea1c45")
struct IDxcRewriter : public IUnknown {

  virtual HRESULT STDMETHODCALLTYPE RemoveUnusedGlobals(
      IDxcBlobEncoding *pSource, LPCWSTR entryPoint, DxcDefine *pDefines,
      UINT32 defineCount, IDxcOperationResult **ppResult) = 0;

  virtual HRESULT STDMETHODCALLTYPE
  RewriteUnchanged(IDxcBlobEncoding *pSource, DxcDefine *pDefines,
                   UINT32 defineCount, IDxcOperationResult **ppResult) = 0;

  virtual HRESULT STDMETHODCALLTYPE RewriteUnchangedWithInclude(
      IDxcBlobEncoding *pSource,
      // Optional file name for pSource. Used in errors and include handlers.
      LPCWSTR pSourceName, DxcDefine *pDefines, UINT32 defineCount,
      // user-provided interface to handle #include directives (optional)
      IDxcIncludeHandler *pIncludeHandler, UINT32 rewriteOption,
      IDxcOperationResult **ppResult) = 0;
};

#ifdef _MSC_VER
#define CLSID_SCOPE __declspec(selectany) extern
#else
#define CLSID_SCOPE
#endif

CLSID_SCOPE const CLSID
    CLSID_DxcRewriter = {/* b489b951-e07f-40b3-968d-93e124734da4 */
                         0xb489b951,
                         0xe07f,
                         0x40b3,
                         {0x96, 0x8d, 0x93, 0xe1, 0x24, 0x73, 0x4d, 0xa4}};

CROSS_PLATFORM_UUIDOF(IDxcRewriter2, "261afca1-0609-4ec6-a77f-d98c7035194e")
struct IDxcRewriter2 : public IDxcRewriter {

  virtual HRESULT STDMETHODCALLTYPE RewriteWithOptions(
      IDxcBlobEncoding *pSource,
      // Optional file name for pSource. Used in errors and include handlers.
      LPCWSTR pSourceName,
      // Compiler arguments
      LPCWSTR *pArguments, UINT32 argCount,
      // Defines
      DxcDefine *pDefines, UINT32 defineCount,
      // user-provided interface to handle #include directives (optional)
      IDxcIncludeHandler *pIncludeHandler, IDxcOperationResult **ppResult) = 0;
};

#endif
