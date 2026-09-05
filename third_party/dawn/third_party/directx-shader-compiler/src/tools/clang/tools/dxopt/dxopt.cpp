///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// dxopt.cpp                                                                 //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides the entry point for the dxopt console program.                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/Global.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/Support/dxcapi.use.h"
#include <string>
#include <vector>

#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/WinFunctions.h"
#include "dxc/Support/dxcapi.use.h"
#include "dxc/Support/microcom.h"
#include "dxc/dxcapi.h"
#include "dxc/dxcapi.internal.h"
#include "dxc/dxctools.h"

#include <iostream>
#include <limits>

#include "llvm/Support/FileSystem.h"

extern const char *kDxCompilerLib;

inline bool wcseq(LPCWSTR a, LPCWSTR b) {
  return (a == nullptr && b == nullptr) ||
         (a != nullptr && b != nullptr && wcscmp(a, b) == 0);
}
inline bool wcsieq(LPCWSTR a, LPCWSTR b) { return _wcsicmp(a, b) == 0; }
inline bool wcsistarts(LPCWSTR text, LPCWSTR prefix) {
  return wcslen(text) >= wcslen(prefix) &&
         _wcsnicmp(text, prefix, wcslen(prefix)) == 0;
}
inline bool wcsieqopt(LPCWSTR text, LPCWSTR opt) {
  return (text[0] == L'-' || text[0] == L'/') && wcsieq(text + 1, opt);
}

static dxc::SpecificDllLoader g_DxcSupport;

enum class ProgramAction {
  PrintHelp,
  PrintPasses,
  PrintPassesWithDetails,
  RunOptimizer,
};

const wchar_t *STDIN_FILE_NAME = L"-";
bool isStdIn(LPCWSTR fName) { return wcscmp(STDIN_FILE_NAME, fName) == 0; }

// Arg does not start with '-' or '/' and so assume it is a filename,
// or next arg equals '-' which is the name of stdin.
bool isFileInputArg(LPCWSTR arg) {
  const bool isNonOptionArg =
      !wcsistarts(arg, L"-") && wcsrchr(arg, L'/') != arg;
  return isNonOptionArg || isStdIn(arg);
}

static HRESULT ReadStdin(std::string &input) {
  bool emptyLine = false;
  while (!std::cin.eof()) {
    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) {
      emptyLine = true;
      continue;
    }

    std::copy(line.begin(), line.end(), std::back_inserter(input));
    input.push_back('\n');
  }

  DWORD lastError = GetLastError();
  // Make sure we reached finished successfully.
  if (std::cin.eof())
    return S_OK;
  // Or reached the end of a pipe.
  else if (!std::cin.good() && emptyLine && lastError == ERROR_BROKEN_PIPE)
    return S_OK;
  else
    return HRESULT_FROM_WIN32(lastError);
}

static void BlobFromFile(LPCWSTR pFileName, IDxcBlob **ppBlob) {
  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcBlobEncoding> pFileBlob;
  IFT(g_DxcSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
  if (isStdIn(pFileName)) {
    std::string input;
    IFT(ReadStdin(input));
    IFTBOOL(input.size() < std::numeric_limits<UINT32>::max(), E_FAIL);
    IFT(pLibrary->CreateBlobWithEncodingOnHeapCopy(
        input.data(), (UINT32)input.size(), CP_UTF8, &pFileBlob))
  } else {
    IFT(pLibrary->CreateBlobFromFile(pFileName, nullptr, &pFileBlob));
  }
  *ppBlob = pFileBlob.Detach();
}

static void PrintOptOutput(LPCWSTR pFileName, IDxcBlob *pBlob,
                           IDxcBlobEncoding *pOutputText) {
  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcBlobEncoding> pOutputText16;
  IFT(g_DxcSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
#ifdef _WIN32
  IFT(pLibrary->GetBlobAsWide(pOutputText, &pOutputText16));
  wprintf(L"%*s", (int)pOutputText16->GetBufferSize(),
          (wchar_t *)pOutputText16->GetBufferPointer());
#else
  IFT(pLibrary->GetBlobAsUtf8(pOutputText, &pOutputText16));
  printf("%*s", (int)pOutputText16->GetBufferSize(),
         (char *)pOutputText16->GetBufferPointer());
#endif
  if (pBlob && pFileName && *pFileName) {
    dxc::WriteBlobToFile(pBlob, pFileName,
                         DXC_CP_UTF8); // TODO: Support DefaultTextCodePage
  }
}

static void PrintPasses(IDxcOptimizer *pOptimizer, bool includeDetails) {
  UINT32 passCount;
  IFT(pOptimizer->GetAvailablePassCount(&passCount));
  for (UINT32 i = 0; i < passCount; ++i) {
    CComPtr<IDxcOptimizerPass> pPass;
    CComHeapPtr<wchar_t> pName;
    IFT(pOptimizer->GetAvailablePass(i, &pPass));
    IFT(pPass->GetOptionName(&pName));
    if (!includeDetails) {
      wprintf(L"%s\n", pName.m_pData);
      continue;
    }

    CComHeapPtr<wchar_t> pDescription;
    IFT(pPass->GetDescription(&pDescription));
    if (*pDescription) {
      wprintf(L"%s - %s\n", pName.m_pData, pDescription.m_pData);
    } else {
      wprintf(L"%s\n", pName.m_pData);
    }
    UINT32 argCount;
    IFT(pPass->GetOptionArgCount(&argCount));
    if (argCount) {
      wprintf(L"%s", L"Options:\n");
      for (UINT32 argIdx = 0; argIdx < argCount; ++argIdx) {
        CComHeapPtr<wchar_t> pArgName;
        CComHeapPtr<wchar_t> pArgDescription;
        IFT(pPass->GetOptionArgName(argIdx, &pArgName));
        IFT(pPass->GetOptionArgDescription(argIdx, &pArgDescription));
        if (pArgDescription.m_pData && *pArgDescription.m_pData &&
            !wcsieq(L"None", pArgDescription.m_pData)) {
          wprintf(L"  %s - %s\n", pArgName.m_pData, pArgDescription.m_pData);
        } else {
          wprintf(L"  %s\n", pArgName.m_pData);
        }
      }
      wprintf(L"%s", L"\n");
    }
  }
}

static void ReadFileOpts(LPCWSTR pPassFileName, IDxcBlobEncoding **ppPassOpts,
                         std::vector<LPCWSTR> &passes, LPCWSTR **pOptArgs,
                         UINT32 *pOptArgCount) {
  *ppPassOpts = nullptr;
  // If there is no file, there is no work to be done.
  if (!pPassFileName || !*pPassFileName) {
    return;
  }

  CComPtr<IDxcBlob> pPassOptsBlob;
  CComPtr<IDxcBlobWide> pPassOpts;
  BlobFromFile(pPassFileName, &pPassOptsBlob);
  IFT(hlsl::DxcGetBlobAsWide(pPassOptsBlob, hlsl::GetGlobalHeapMalloc(),
                             &pPassOpts));
  LPWSTR pCursor = const_cast<LPWSTR>(pPassOpts->GetStringPointer());
  while (*pCursor) {
    passes.push_back(pCursor);
    while (*pCursor && *pCursor != L'\n' && *pCursor != L'\r') {
      ++pCursor;
    }
    while (*pCursor && (*pCursor == L'\n' || *pCursor == L'\r')) {
      *pCursor = L'\0';
      ++pCursor;
    }
  }

  // Remove empty entries and comments.
  size_t i = passes.size();
  do {
    --i;
    if (wcslen(passes[i]) == 0 || passes[i][0] == L'#') {
      passes.erase(passes.begin() + i);
    }
  } while (i != 0);

  *pOptArgs = passes.data();
  *pOptArgCount = passes.size();
  pPassOpts->QueryInterface(ppPassOpts);
}

static void PrintHelp() {
  wprintf(
      L"%s",
      L"Performs optimizations on a bitcode file by running a sequence of "
      L"passes.\n\n"
      L"dxopt [-? | -passes | -pass-details | -pf [PASS-FILE] | [-o=OUT-FILE] "
      L"| IN-FILE OPT-ARGUMENTS ...]\n\n"
      L"Arguments:\n"
      L"  -?  Displays this help message\n"
      L"  -passes        Displays a list of pass names\n"
      L"  -pass-details  Displays a list of passes with detailed information\n"
      L"  -pf PASS-FILE  Loads passes from the specified file\n"
      L"  -o=OUT-FILE    Output file for processed module\n"
      L"  IN-FILE        File with with bitcode to optimize\n"
      L"  OPT-ARGUMENTS  One or more passes to run in sequence\n"
      L"\n"
      L"Text that is traced during optimization is written to the standard "
      L"output.\n");
}

#ifdef _WIN32
int __cdecl wmain(int argc, const wchar_t **argv_) {
#else
int main(int argc, const char **argv) {
  // Convert argv to wchar.
  WArgV ArgV(argc, argv);
  const wchar_t **argv_ = ArgV.argv();
#endif
  const char *pStage = "Operation";
  int retVal = 0;
  if (llvm::sys::fs::SetupPerThreadFileSystem())
    return 1;
  llvm::sys::fs::AutoCleanupPerThreadFileSystem auto_cleanup_fs;
  try {
    // Parse command line options.
    pStage = "Argument processing";

    ProgramAction action = ProgramAction::PrintHelp;
    LPCWSTR inFileName = nullptr;
    LPCWSTR outFileName = nullptr;
    LPCWSTR externalLib = nullptr;
    LPCWSTR externalFn = nullptr;
    LPCWSTR passFileName = nullptr;
    const wchar_t **optArgs = nullptr;
    UINT32 optArgCount = 0;

    int argIdx = 1;
    while (argIdx < argc) {
      LPCWSTR arg = argv_[argIdx];
      if (wcsieqopt(arg, L"?")) {
        action = ProgramAction::PrintHelp;
      } else if (wcsieqopt(arg, L"passes")) {
        action = ProgramAction::PrintPasses;
      } else if (wcsieqopt(arg, L"pass-details")) {
        action = ProgramAction::PrintPassesWithDetails;
      } else if (wcsieqopt(arg, L"external")) {
        ++argIdx;
        if (argIdx == argc) {
          PrintHelp();
          return 1;
        }
        externalLib = argv_[argIdx];
      } else if (wcsieqopt(arg, L"external-fn")) {
        ++argIdx;
        if (argIdx == argc) {
          PrintHelp();
          return 1;
        }
        externalFn = argv_[argIdx];
      } else if (wcsieqopt(arg, L"pf")) {
        ++argIdx;
        if (argIdx == argc) {
          PrintHelp();
          return 1;
        }
        passFileName = argv_[argIdx];
      } else if (wcsistarts(arg, L"-o=")) {
        outFileName = argv_[argIdx] + 3;
      } else {
        action = ProgramAction::RunOptimizer;
        // See if arg is file input specifier.
        if (isFileInputArg(arg)) {
          inFileName = arg;
          argIdx++;
        }
        // No filename argument give so read from stdin.
        else {
          inFileName = STDIN_FILE_NAME;
        }

        // The remaining arguments are optimizer args.
        optArgs = argv_ + argIdx;
        optArgCount = argc - argIdx;
        break;
      }
      ++argIdx;
    }

    if (action == ProgramAction::PrintHelp) {
      PrintHelp();
      return retVal;
    }

    if (passFileName && optArgCount) {
      wprintf(L"%s", L"Cannot specify both command-line options and an pass "
                     L"option file.\n");
      return 1;
    }

    if (externalLib) {
      CW2A externalFnA(externalFn);
      CW2A externalLibA(externalLib);
      IFT(g_DxcSupport.InitializeForDll(externalLibA, externalFnA));
    } else {
      IFT(g_DxcSupport.InitializeForDll(dxc::kDxCompilerLib,
                                        "DxcCreateInstance"));
    }

    CComPtr<IDxcBlob> pBlob;
    CComPtr<IDxcBlob> pOutputModule;
    CComPtr<IDxcBlobEncoding> pOutputText;
    CComPtr<IDxcOptimizer> pOptimizer;
    CComPtr<IDxcBlobEncoding> pPassOpts;
    std::vector<LPCWSTR> passes;
    IFT(g_DxcSupport.CreateInstance(CLSID_DxcOptimizer, &pOptimizer));
    switch (action) {
    case ProgramAction::PrintPasses:
      pStage = "Printing passes...";
      PrintPasses(pOptimizer, false);
      break;
    case ProgramAction::PrintPassesWithDetails:
      pStage = "Printing pass details...";
      PrintPasses(pOptimizer, true);
      break;
    case ProgramAction::RunOptimizer:
      pStage = "Optimizer processing";
      BlobFromFile(inFileName, &pBlob);
      ReadFileOpts(passFileName, &pPassOpts, passes, &optArgs, &optArgCount);
      IFT(pOptimizer->RunOptimizer(pBlob, optArgs, optArgCount, &pOutputModule,
                                   &pOutputText));
      PrintOptOutput(outFileName, pOutputModule, pOutputText);
      break;
    }
  } catch (const ::hlsl::Exception &hlslException) {
    try {
      const char *msg = hlslException.what();
      Unicode::acp_char
          printBuffer[128]; // printBuffer is safe to treat as
                            // UTF-8 because we use ASCII only errors
      if (msg == nullptr || *msg == '\0') {
        sprintf_s(printBuffer, _countof(printBuffer),
                  "Operation failed - error code 0x%08x.\n", hlslException.hr);
        msg = printBuffer;
      }

      dxc::WriteUtf8ToConsoleSizeT(msg, strlen(msg));
      printf("\n");
    } catch (...) {
      printf("%s failed - unable to retrieve error message.\n", pStage);
    }

    return 1;
  } catch (std::bad_alloc &) {
    printf("%s failed - out of memory.\n", pStage);
    return 1;
  } catch (...) {
    printf("%s failed - unknown error.\n", pStage);
    return 1;
  }

  return retVal;
}
