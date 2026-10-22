///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxr.cpp                                                                   //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides the entry point for the dxr console program.                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/Global.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/WinFunctions.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/Support/microcom.h"
#include "dxclib/dxc.h"
#include <string>
#include <vector>

#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/WinFunctions.h"
#include "dxc/Support/dxcapi.use.h"
#include "dxc/dxcapi.h"
#include "dxc/dxctools.h"
#include "llvm/Support/raw_ostream.h"

inline bool wcsieq(LPCWSTR a, LPCWSTR b) { return _wcsicmp(a, b) == 0; }

using namespace dxc;
using namespace llvm::opt;
using namespace hlsl::options;

#ifdef _WIN32
int __cdecl wmain(int argc, const wchar_t **argv_) {
#else
int main(int argc, const char **argv) {
  // Convert argv to wchar.
  WArgV ArgV(argc, argv);
  const wchar_t **argv_ = ArgV.argv();
#endif
  if (FAILED(DxcInitThreadMalloc()))
    return 1;
  DxcSetThreadMallocToDefault();
  try {
    if (initHlslOptTable())
      throw std::bad_alloc();

    // Parse command line options.
    const OptTable *optionTable = getHlslOptTable();
    MainArgs argStrings(argc, argv_);
    DxcOpts dxcOpts;
    DXCLibraryDllLoader dxcSupport;

    // Read options and check errors.
    {
      std::string errorString;
      llvm::raw_string_ostream errorStream(errorString);
      int optResult =
          ReadDxcOpts(optionTable, DxrFlags, argStrings, dxcOpts, errorStream);
      errorStream.flush();
      if (errorString.size()) {
        fprintf(stderr, "dxc failed : %s\n", errorString.data());
      }
      if (optResult != 0) {
        return optResult;
      }
    }

    // Apply defaults.
    if (dxcOpts.EntryPoint.empty() && !dxcOpts.RecompileFromBinary) {
      dxcOpts.EntryPoint = "main";
    }

    // Setup a helper DLL.
    {
      std::string dllErrorString;
      llvm::raw_string_ostream dllErrorStream(dllErrorString);
      int dllResult =
          SetupSpecificDllLoader(dxcOpts, dxcSupport, dllErrorStream);
      dllErrorStream.flush();
      if (dllErrorString.size()) {
        fprintf(stderr, "%s\n", dllErrorString.data());
      }
      if (dllResult)
        return dllResult;
    }

    EnsureEnabled(dxcSupport);
    // Handle help request, which overrides any other processing.
    if (dxcOpts.ShowHelp) {
      std::string helpString;
      llvm::raw_string_ostream helpStream(helpString);
      std::string version;
      llvm::raw_string_ostream versionStream(version);
      dxc::WriteDxCompilerVersionInfo(
          versionStream,
          dxcOpts.ExternalLib.empty() ? (LPCSTR) nullptr
                                      : dxcOpts.ExternalLib.data(),
          dxcOpts.ExternalFn.empty() ? (LPCSTR) nullptr
                                     : dxcOpts.ExternalFn.data(),
          dxcSupport);
      versionStream.flush();
      optionTable->PrintHelp(helpStream, "dxr.exe", "HLSL Rewriter",
                             version.c_str(), hlsl::options::RewriteOption,
                             (dxcOpts.ShowHelpHidden ? 0 : HelpHidden));
      helpStream.flush();
      WriteUtf8ToConsoleSizeT(helpString.data(), helpString.size());
      return 0;
    }

    if (dxcOpts.ShowVersion) {
      std::string version;
      llvm::raw_string_ostream versionStream(version);
      dxc::WriteDxCompilerVersionInfo(
          versionStream,
          dxcOpts.ExternalLib.empty() ? (LPCSTR) nullptr
                                      : dxcOpts.ExternalLib.data(),
          dxcOpts.ExternalFn.empty() ? (LPCSTR) nullptr
                                     : dxcOpts.ExternalFn.data(),
          dxcSupport);
      versionStream.flush();
      WriteUtf8ToConsoleSizeT(version.data(), version.size());
      return 0;
    }

    CComPtr<IDxcRewriter2> pRewriter;
    CComPtr<IDxcOperationResult> pRewriteResult;
    CComPtr<IDxcBlobEncoding> pSource;
    std::wstring wName(
        CA2W(dxcOpts.InputFile.empty() ? "" : dxcOpts.InputFile.data()));
    if (!dxcOpts.InputFile.empty())
      ReadFileIntoBlob(dxcSupport, wName.c_str(), &pSource);

    CComPtr<IDxcLibrary> pLibrary;
    CComPtr<IDxcIncludeHandler> pIncludeHandler;
    IFT(dxcSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
    IFT(pLibrary->CreateIncludeHandler(&pIncludeHandler));
    IFT(dxcSupport.CreateInstance(CLSID_DxcRewriter, &pRewriter));

    IFT(pRewriter->RewriteWithOptions(pSource, wName.c_str(), argv_, argc,
                                      nullptr, 0, pIncludeHandler,
                                      &pRewriteResult));

    if (dxcOpts.OutputObject.empty()) {
      // No -Fo, print to console
      WriteOperationResultToConsole(pRewriteResult, !dxcOpts.OutputWarnings);
    } else {
      WriteOperationErrorsToConsole(pRewriteResult, !dxcOpts.OutputWarnings);
      HRESULT hr;
      IFT(pRewriteResult->GetStatus(&hr));
      if (SUCCEEDED(hr)) {
        CA2W wOutputObject(dxcOpts.OutputObject.data());
        CComPtr<IDxcBlob> pObject;
        IFT(pRewriteResult->GetResult(&pObject));
        WriteBlobToFile(pObject, wOutputObject.m_psz,
                        dxcOpts.DefaultTextCodePage);
        printf("Rewrite output: %s", dxcOpts.OutputObject.data());
      }
    }

  } catch (const ::hlsl::Exception &hlslException) {
    try {
      const char *msg = hlslException.what();
      Unicode::acp_char
          printBuffer[128]; // printBuffer is safe to treat as UTF-8 because we
                            // use ASCII contents only
      if (msg == nullptr || *msg == '\0') {
        sprintf_s(printBuffer, _countof(printBuffer),
                  "Compilation failed - error code 0x%08x.", hlslException.hr);
        msg = printBuffer;
      }

      std::string textMessage;
      bool lossy;
      if (!Unicode::UTF8ToConsoleString(msg, &textMessage, &lossy) || lossy) {
        // Do a direct assignment as a last-ditch effort and print out as UTF-8.
        textMessage = msg;
      }

      printf("%s\n", textMessage.c_str());
    } catch (...) {
      printf("Compilation failed - unable to retrieve error message.\n");
    }

    return 1;
  } catch (std::bad_alloc &) {
    printf("Compilation failed - out of memory.\n");
    return 1;
  } catch (...) {
    printf("Compilation failed - unable to retrieve error message.\n");
    return 1;
  }

  return 0;
}
