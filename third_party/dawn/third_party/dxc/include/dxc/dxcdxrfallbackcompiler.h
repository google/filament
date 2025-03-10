
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxcapi.h                                                                  //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides declarations for the DirectX Compiler API entry point.           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef __DXC_DXR_FALLBACK_COMPILER_API__
#define __DXC_DXR_FALLBACK_COMPILER_API__
#include "dxcapi.h"

enum class ShaderType : unsigned int {
  Raygen,
  AnyHit,
  ClosestHit,
  Intersection,
  Miss,
  Callable,
  Lib,
};

struct DxcShaderInfo {
  UINT32 Identifier;
  UINT32 StackSize;
  ShaderType Type;
};

struct DxcShaderBytecode {
  LPBYTE pData;
  UINT32 Size;
};

struct DxcExportDesc {
  LPCWSTR ExportToRename;
  LPCWSTR ExportName;
};

struct __declspec(uuid("76bb3c85-006d-4b72-9e10-63cd97df57f0"))
    IDxcDxrFallbackCompiler : public IUnknown {

  // If set to true then shaders not listed in pShaderNames in Compile() but
  // called by shaders in pShaderNames are added to the final computer shader.
  // Otherwise these are considered errors. This is intended for testing
  // purposes.
  virtual HRESULT STDMETHODCALLTYPE SetFindCalledShaders(bool val) = 0;

  virtual HRESULT STDMETHODCALLTYPE SetDebugOutput(int val) = 0;

  virtual HRESULT STDMETHODCALLTYPE RenameAndLink(
      DxcShaderBytecode *pLibs, UINT32 libCount, DxcExportDesc *pExports,
      UINT32 ExportCount, IDxcOperationResult **ppResult) = 0;

  virtual HRESULT STDMETHODCALLTYPE PatchShaderBindingTables(
      const LPCWSTR pEntryName, DxcShaderBytecode *pShaderBytecode,
      void *pShaderInfo, IDxcOperationResult **ppResult) = 0;

  // Compiles libs together to create a raytracing compute shader. One of the
  // libs should be the fallback implementation lib that defines functions like
  // Fallback_TraceRay(), Fallback_ReportHit(), etc. Fallback_TraceRay() should
  // be one of the shader names so that it gets included in the compile.
  virtual HRESULT STDMETHODCALLTYPE Compile(
      DxcShaderBytecode *pLibs,    // Array of libraries containing shaders
      UINT32 libCount,             // Number of libraries containing shaders
      const LPCWSTR *pShaderNames, // Array of shader names to compile
      DxcShaderInfo
          *pShaderInfo,   // Array of shaderInfo corresponding to pShaderNames
      UINT32 shaderCount, // Number of shaders to compile
      UINT32 maxAttributeSize,
      IDxcOperationResult *
          *ppResult // Compiler output status, buffer, and errors
      ) = 0;

  virtual HRESULT STDMETHODCALLTYPE Link(
      const LPCWSTR
          pEntryName, // Name of entry function, null if compiling a collection
      IDxcBlob **pLibs,            // Array of libraries containing shaders
      UINT32 libCount,             // Number of libraries containing shaders
      const LPCWSTR *pShaderNames, // Array of shader names to compile
      DxcShaderInfo
          *pShaderInfo,   // Array of shaderInfo corresponding to pShaderNames
      UINT32 shaderCount, // Number of shaders to compile
      UINT32 maxAttributeSize,
      UINT32 stackSizeInBytes, // Continuation stack size. Use 0 for default.
      IDxcOperationResult *
          *ppResult // Compiler output status, buffer, and errors
      ) = 0;
};

// Note: __declspec(selectany) requires 'extern'
// On Linux __declspec(selectany) is removed and using 'extern' results in link
// error.
#ifdef _MSC_VER
#define CLSID_SCOPE __declspec(selectany) extern
#else
#define CLSID_SCOPE
#endif

// {76bb3c85-006d-4b72-9e10-63cd97df57f0}
CLSID_SCOPE const GUID CLSID_DxcDxrFallbackCompiler = {
    0x76bb3c85,
    0x006d,
    0x4b72,
    {0x9e, 0x10, 0x63, 0xcd, 0x97, 0xdf, 0x57, 0xf0}};

typedef HRESULT(__stdcall *DxcCreateDxrFallbackCompilerProc)(REFCLSID rclsid,
                                                             REFIID riid,
                                                             LPVOID *ppv);

#endif
