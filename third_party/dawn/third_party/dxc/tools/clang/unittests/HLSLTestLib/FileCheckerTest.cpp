///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// FileCheckerTest.cpp                                                       //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides tests that are based on FileChecker.                             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef UNICODE
#define UNICODE
#endif

#include "dxc/Support/WinIncludes.h"
#include "dxc/dxcapi.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <memory>
#include <string>
#include <vector>
#ifdef _WIN32
#include <atlfile.h>
#endif

#include "dxc/Test/DxcTestUtils.h"
#include "dxc/Test/HLSLTestData.h"
#include "dxc/Test/HlslTestUtils.h"

#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/D3DReflection.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/dxcapi.use.h"
#include "dxc/Support/microcom.h"
#include "dxc/dxctools.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MD5.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Support/raw_ostream.h"

#include "dxc/DxilContainer/DxilRuntimeReflection.h"
#include "dxc/Test/D3DReflectionDumper.h"
#include "dxc/Test/RDATDumper.h"

using namespace hlsl::dump;

using namespace std;
using namespace hlsl_test;

FileRunCommandPart::FileRunCommandPart(const std::string &command,
                                       const std::string &arguments,
                                       LPCWSTR commandFileName)
    : Command(command), Arguments(arguments), CommandFileName(commandFileName) {
}

FileRunCommandResult
FileRunCommandPart::RunHashTests(dxc::DxcDllSupport &DllSupport) {
  if (0 == _stricmp(Command.c_str(), "%dxc")) {
    return RunDxcHashTest(DllSupport);
  } else {
    return FileRunCommandResult::Success();
  }
}

FileRunCommandResult
FileRunCommandPart::Run(dxc::DxcDllSupport &DllSupport,
                        const FileRunCommandResult *Prior,
                        PluginToolsPaths *pPluginToolsPaths /*=nullptr*/,
                        LPCWSTR dumpName /*=nullptr*/) {
  bool isFileCheck = 0 == _stricmp(Command.c_str(), "FileCheck") ||
                     0 == _stricmp(Command.c_str(), "%FileCheck");
  bool isXFail = 0 == _stricmp(Command.c_str(), "xfail");
  bool consumeErrors = isFileCheck || isXFail;

  // Stop the pipeline if on errors unless the command can consume them.
  if (Prior != nullptr && Prior->ExitCode && !consumeErrors) {
    FileRunCommandResult result = *Prior;
    result.AbortPipeline = true;
    return result;
  }

  // We would add support for 'not' and 'llc' here.
  if (isFileCheck) {
    return RunFileChecker(Prior, dumpName);
  } else if (isXFail) {
    return RunXFail(Prior);
  } else if (0 == _stricmp(Command.c_str(), "tee")) {
    return RunTee(Prior);
  } else if (0 == _stricmp(Command.c_str(), "fc")) {
    return RunFileCompareText(Prior);
  } else if (0 == _stricmp(Command.c_str(), "%dxilver")) {
    return RunDxilVer(DllSupport, Prior);
  } else if (0 == _stricmp(Command.c_str(), "%dxc")) {
    return RunDxc(DllSupport, Prior);
  } else if (0 == _stricmp(Command.c_str(), "%dxv")) {
    return RunDxv(DllSupport, Prior);
  } else if (0 == _stricmp(Command.c_str(), "%opt")) {
    return RunOpt(DllSupport, Prior);
  } else if (0 == _stricmp(Command.c_str(), "%listparts")) {
    return RunListParts(DllSupport, Prior);
  } else if (0 == _stricmp(Command.c_str(), "%D3DReflect")) {
    return RunD3DReflect(DllSupport, Prior);
  } else if (0 == _stricmp(Command.c_str(), "%dxr")) {
    return RunDxr(DllSupport, Prior);
  } else if (0 == _stricmp(Command.c_str(), "%dxl")) {
    return RunLink(DllSupport, Prior);
  } else if (pPluginToolsPaths != nullptr) {
    auto it = pPluginToolsPaths->find(Command.c_str());
    if (it != pPluginToolsPaths->end()) {
      return RunFromPath(it->second, Prior);
    }
  }
  FileRunCommandResult result{};
  result.ExitCode = 1;
  result.StdErr = "Unrecognized command ";
  result.StdErr += Command;
  return result;
}

FileRunCommandResult
FileRunCommandPart::RunFileChecker(const FileRunCommandResult *Prior,
                                   LPCWSTR dumpName /*=nullptr*/) {
  if (!Prior)
    return FileRunCommandResult::Error(
        "Prior command required to generate stdin");

  FileCheckForTest t;
  t.CheckFilename = CW2A(CommandFileName);
  t.InputForStdin = Prior->ExitCode ? Prior->StdErr : Prior->StdOut;

  // Parse command arguments
  static constexpr char checkPrefixStr[] = "-check-prefix=";
  static constexpr char checkPrefixesStr[] = "-check-prefixes=";
  static constexpr char defineStr[] = "-D";
  bool hasInputFilename = false;
  auto args = strtok(Arguments);
  for (const std::string &arg : args) {
    if (arg == "%s")
      hasInputFilename = true;
    else if (arg == "-input-file=stderr") {
      t.InputForStdin = Prior->StdErr;
      t.AllowEmptyInput = true;
    } else if (strstartswith(arg, checkPrefixStr))
      t.CheckPrefixes.emplace_back(arg.substr(sizeof(checkPrefixStr) - 1));
    else if (strstartswith(arg, checkPrefixesStr)) {
      auto prefixes = strtok(arg.substr(sizeof(checkPrefixesStr) - 1), ", ");
      for (auto &prefix : prefixes)
        t.CheckPrefixes.emplace_back(prefix);
    } else if (strstartswith(arg, defineStr)) {
      auto kv = strtok(arg.substr(sizeof(defineStr) - 1), "=");
      if (kv.size() != 2)
        return FileRunCommandResult::Error("Invalid argument");
      t.VariableTable[kv[0]] = kv[1];
    } else
      return FileRunCommandResult::Error("Invalid argument");
  }
  if (!hasInputFilename)
    return FileRunCommandResult::Error("Missing input filename");

  if (dumpName) {
    // Dump t.InputForStdin to file for comparison purposes
    CW2A dumpNameUtf8(dumpName);
    llvm::StringRef dumpPath(dumpNameUtf8.m_psz);
    llvm::sys::fs::create_directories(llvm::sys::path::parent_path(dumpPath),
                                      /*IgnoreExisting*/ true);
    std::error_code ec;
    llvm::raw_fd_ostream os(dumpPath, ec, llvm::sys::fs::OpenFlags::F_Text);
    if (!ec) {
      os << t.InputForStdin;
    }
  }

  FileRunCommandResult result{};
  // Run
  result.ExitCode = t.Run();
  result.StdOut = t.test_outs;
  result.StdErr = t.test_errs;
  // Capture the input as well.
  if (result.ExitCode != 0 && Prior != nullptr) {
    result.StdErr += "\n<full input to FileCheck>\n";
    result.StdErr += t.InputForStdin;
  }

  return result;
}

FileRunCommandResult
FileRunCommandPart::ReadOptsForDxc(hlsl::options::MainArgs &argStrings,
                                   hlsl::options::DxcOpts &Opts,
                                   unsigned flagsToInclude) {
  std::string args(strtrim(Arguments));
  const char *inputPos = strstr(args.c_str(), "%s");
  if (inputPos == nullptr)
    return FileRunCommandResult::Error(
        "Only supported pattern includes input file as argument");
  args.erase(inputPos - args.c_str(), strlen("%s"));

  llvm::StringRef argsRef = args;
  llvm::SmallVector<llvm::StringRef, 8> splitArgs;
  argsRef.split(splitArgs, " ");
  argStrings = hlsl::options::MainArgs(splitArgs);
  std::string errorString;
  llvm::raw_string_ostream errorStream(errorString);
  int RunResult = ReadDxcOpts(hlsl::options::getHlslOptTable(), flagsToInclude,
                              argStrings, Opts, errorStream);
  errorStream.flush();
  if (RunResult)
    return FileRunCommandResult::Error(RunResult, errorString);

  return FileRunCommandResult::Success("");
}

// Simple virtual file system include handler for test, fall back to default
// include handler
class IncludeHandlerVFSOverlayForTest : public IDxcIncludeHandler {
private:
  DXC_MICROCOM_TM_REF_FIELDS()

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL()
  DXC_MICROCOM_TM_CTOR(IncludeHandlerVFSOverlayForTest)

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcIncludeHandler>(this, iid, ppvObject);
  }

  const FileMap *pVFS = nullptr;
  CComPtr<IDxcIncludeHandler> pInnerIncludeHandler;

  HRESULT STDMETHODCALLTYPE LoadSource(
      LPCWSTR pFilename,         // Candidate filename.
      IDxcBlob **ppIncludeSource // Resultant source object for included file,
                                 // nullptr if not found.
      ) override {
    if (!ppIncludeSource)
      return E_INVALIDARG;
    *ppIncludeSource = nullptr;
    if (!pFilename)
      return E_INVALIDARG;
    try {
      if (pVFS) {
        auto it = pVFS->find(pFilename);
        if (it != pVFS->end()) {
          return it->second.QueryInterface(ppIncludeSource);
        }
      }
      if (pInnerIncludeHandler) {
        return pInnerIncludeHandler->LoadSource(pFilename, ppIncludeSource);
      }
      return E_FAIL;
    }
    CATCH_CPP_RETURN_HRESULT();
  }
};

static CComPtr<IncludeHandlerVFSOverlayForTest>
AllocVFSIncludeHandler(IUnknown *pUnkLibrary, const FileMap *pVFS) {
  CComPtr<IncludeHandlerVFSOverlayForTest> pVFSIncludeHandler =
      IncludeHandlerVFSOverlayForTest::Alloc(DxcGetThreadMallocNoRef());
  IFTBOOL(pVFSIncludeHandler, E_OUTOFMEMORY);
  if (pUnkLibrary) {
    CComPtr<IDxcIncludeHandler> pInnerIncludeHandler;
    CComPtr<IDxcUtils> pUtils;
    if (SUCCEEDED(pUnkLibrary->QueryInterface(IID_PPV_ARGS(&pUtils)))) {
      IFT(pUtils->CreateDefaultIncludeHandler(&pInnerIncludeHandler));
    } else {
      CComPtr<IDxcLibrary> pLibrary;
      if (SUCCEEDED(pUnkLibrary->QueryInterface(IID_PPV_ARGS(&pLibrary)))) {
        IFT(pLibrary->CreateIncludeHandler(&pInnerIncludeHandler));
      }
    }
    pVFSIncludeHandler->pInnerIncludeHandler = pInnerIncludeHandler;
  }
  pVFSIncludeHandler->pVFS = pVFS;
  return pVFSIncludeHandler;
}

static void AddOutputsToFileMap(IUnknown *pUnkResult, FileMap *pVFS) {
  // If there is IDxcResult, save named output blobs to Files.
  if (pUnkResult && pVFS) {
    CComPtr<IDxcResult> pResult;
    if (SUCCEEDED(pUnkResult->QueryInterface(IID_PPV_ARGS(&pResult)))) {
      for (unsigned i = 0; i < pResult->GetNumOutputs(); i++) {
        CComPtr<IDxcBlob> pOutput;
        CComPtr<IDxcBlobWide> pOutputName;
        if (SUCCEEDED(pResult->GetOutput(pResult->GetOutputByIndex(i),
                                         IID_PPV_ARGS(&pOutput),
                                         &pOutputName)) &&
            pOutput && pOutputName && pOutputName->GetStringLength() > 0) {
          (*pVFS)[pOutputName->GetStringPointer()] = pOutput;
        }
      }
    }
  }
}

static HRESULT CompileForHash(hlsl::options::DxcOpts &opts,
                              LPCWSTR CommandFileName,
                              dxc::DxcDllSupport &DllSupport,
                              std::vector<LPCWSTR> &flags,
                              IDxcBlob **ppHashBlob, std::string &output) {
  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcCompiler2> pCompiler2;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlob> pCompiledBlob;
  CComPtr<IDxcBlob> pCompiledName;
  CComPtr<IDxcIncludeHandler> pIncludeHandler;
  CComHeapPtr<WCHAR> pDebugName;
  CComPtr<IDxcBlob> pPDBBlob;

  std::wstring entry =
      Unicode::UTF8ToWideStringOrThrow(opts.EntryPoint.str().c_str());
  std::wstring profile =
      Unicode::UTF8ToWideStringOrThrow(opts.TargetProfile.str().c_str());

  IFT(DllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
  IFT(pLibrary->CreateBlobFromFile(CommandFileName, nullptr, &pSource));
  IFT(pLibrary->CreateIncludeHandler(&pIncludeHandler));
  IFT(DllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));
  IFT(pCompiler.QueryInterface(&pCompiler2));
  IFT(pCompiler2->CompileWithDebug(pSource, CommandFileName, entry.c_str(),
                                   profile.c_str(), flags.data(), flags.size(),
                                   nullptr, 0, pIncludeHandler, &pResult,
                                   &pDebugName, &pPDBBlob));

  HRESULT resultStatus = 0;
  IFT(pResult->GetStatus(&resultStatus));
  if (SUCCEEDED(resultStatus)) {

    IFT(pResult->GetResult(&pCompiledBlob));

    CComPtr<IDxcContainerReflection> pReflection;
    IFT(DllSupport.CreateInstance(CLSID_DxcContainerReflection, &pReflection));

    // If failed to load here, it's likely some non-compile operation thing.
    // Just fail the hash generation.
    if (FAILED(pReflection->Load(pCompiledBlob)))
      return E_FAIL;

    *ppHashBlob = nullptr;
    UINT32 uHashIdx = 0;
    if (SUCCEEDED(
            pReflection->FindFirstPartKind(hlsl::DFCC_ShaderHash, &uHashIdx))) {
      CComPtr<IDxcBlob> pHashBlob;
      IFT(pReflection->GetPartContent(uHashIdx, &pHashBlob));
      *ppHashBlob = pHashBlob.Detach();
    }

    // Test that PDB is generated correctly.
    // This test needs to be done elsewhere later, ideally a fully
    // customizable test on all our test set with different compile options.
    if (pPDBBlob) {
      IFT(pReflection->Load(pPDBBlob));
      UINT32 uDebugInfoIndex = 0;
      IFT(pReflection->FindFirstPartKind(hlsl::DFCC_ShaderDebugInfoDXIL,
                                         &uDebugInfoIndex));
    }

    return S_OK;
  } else {
    CComPtr<IDxcBlobEncoding> pErrors;
    IFT(pResult->GetErrorBuffer(&pErrors));
    const char *errors = (char *)pErrors->GetBufferPointer();
    output = errors;
    return resultStatus;
  }
}

FileRunCommandResult
FileRunCommandPart::RunDxcHashTest(dxc::DxcDllSupport &DllSupport) {
  hlsl::options::MainArgs args;
  hlsl::options::DxcOpts opts;
  ReadOptsForDxc(args, opts);

  std::vector<std::wstring> argWStrings;
  CopyArgsToWStrings(opts.Args, hlsl::options::CoreOption, argWStrings);

  // Extract the vanilla flags for the test (i.e. no debug or ast-dump)
  std::vector<LPCWSTR> original_flags;
  bool skipNext = false;
  for (const std::wstring &a : argWStrings) {
    if (skipNext) {
      skipNext = false;
      continue;
    }
    if (a.find(L"ast-dump") != std::wstring::npos)
      continue;
    if (a.find(L"Zi") != std::wstring::npos)
      continue;
    std::wstring optValVer(L"validator-version");
    if (a.substr(1, optValVer.length()).compare(optValVer) == 0) {
      skipNext = a.length() == optValVer.length() + 1;
      continue;
    }
    original_flags.push_back(a.data());
  }

  std::string originalOutput;
  CComPtr<IDxcBlob> pOriginalHash;
  // If failed the original compilation, just pass the test. The original test
  // was likely testing for failure.
  if (FAILED(CompileForHash(opts, CommandFileName, DllSupport, original_flags,
                            &pOriginalHash, originalOutput)))
    return FileRunCommandResult::Success();

  // Results of our compilations
  CComPtr<IDxcBlob> pHash1;
  std::string Output0;
  CComPtr<IDxcBlob> pHash0;
  std::string Output1;

  // Fail if -Qstrip_reflect failed the compilation
  std::vector<LPCWSTR> normal_flags = original_flags;
  normal_flags.push_back(L"-Qstrip_reflect");
  normal_flags.push_back(L"-Zsb");
  std::string StdErr;
  if (FAILED(CompileForHash(opts, CommandFileName, DllSupport, normal_flags,
                            &pHash0, Output0))) {
    StdErr += "Adding Qstrip_reflect failed compilation.";
    StdErr += originalOutput;
    StdErr += Output0;
    return FileRunCommandResult::Error(StdErr);
  }

  // Fail if -Qstrip_reflect failed the compilation
  std::vector<LPCWSTR> dbg_flags = original_flags;
  dbg_flags.push_back(L"/Zi");
  dbg_flags.push_back(L"-Qstrip_reflect");
  dbg_flags.push_back(L"-Zsb");
  if (FAILED(CompileForHash(opts, CommandFileName, DllSupport, dbg_flags,
                            &pHash1, Output1))) {
    return FileRunCommandResult::Error(
        "Adding Qstrip_reflect and Zi failed compilation.");
  }

  if (pHash0->GetBufferSize() != pHash1->GetBufferSize() ||
      0 != memcmp(pHash0->GetBufferPointer(), pHash1->GetBufferPointer(),
                  pHash1->GetBufferSize())) {
    StdErr = "Hashes do not match between normal and debug!!!\n";
    StdErr += Output0;
    StdErr += Output1;
    return FileRunCommandResult::Error(StdErr);
  }

  return FileRunCommandResult::Success();
}

static FileRunCommandResult CheckDxilVer(dxc::DxcDllSupport &DllSupport,
                                         unsigned RequiredDxilMajor,
                                         unsigned RequiredDxilMinor,
                                         bool bCheckValidator = true) {
  bool Supported = true;

  // If the following fails, we have Dxil 1.0 compiler
  unsigned DxilMajor = 1, DxilMinor = 0;
  GetVersion(DllSupport, CLSID_DxcCompiler, DxilMajor, DxilMinor);
  Supported &=
      hlsl::DXIL::CompareVersions(DxilMajor, DxilMinor, RequiredDxilMajor,
                                  RequiredDxilMinor) >= 0;

  if (bCheckValidator) {
    // If the following fails, we have validator 1.0
    unsigned ValMajor = 1, ValMinor = 0;
    GetVersion(DllSupport, CLSID_DxcValidator, ValMajor, ValMinor);
    Supported &=
        hlsl::DXIL::CompareVersions(ValMajor, ValMinor, RequiredDxilMajor,
                                    RequiredDxilMinor) >= 0;
  }

  if (!Supported) {
    FileRunCommandResult result{};
    result.StdErr = "Skipping test due to unsupported dxil version";
    result.ExitCode = 0; // Succeed the test
    result.AbortPipeline = true;
    return result;
  }

  return FileRunCommandResult::Success();
}

FileRunCommandResult
FileRunCommandPart::RunDxc(dxc::DxcDllSupport &DllSupport,
                           const FileRunCommandResult *Prior) {
  // Support piping stdin from prior if needed.
  UNREFERENCED_PARAMETER(Prior);
  hlsl::options::MainArgs args;
  hlsl::options::DxcOpts opts;
  FileRunCommandResult readOptsResult = ReadOptsForDxc(args, opts);
  if (readOptsResult.ExitCode)
    return readOptsResult;

  std::wstring entry =
      Unicode::UTF8ToWideStringOrThrow(opts.EntryPoint.str().c_str());
  std::wstring profile =
      Unicode::UTF8ToWideStringOrThrow(opts.TargetProfile.str().c_str());
  std::vector<LPCWSTR> flags;
  if (opts.CodeGenHighLevel) {
    flags.push_back(L"-fcgl");
  }

  // Skip targets that require a newer compiler or validator.
  // Some features may require newer compiler/validator than indicated by the
  // shader model, but these should use %dxilver explicitly.
  {
    unsigned RequiredDxilMajor = 1, RequiredDxilMinor = 0;
    llvm::StringRef stage;
    IFTBOOL(ParseTargetProfile(opts.TargetProfile, stage, RequiredDxilMajor,
                               RequiredDxilMinor),
            E_INVALIDARG);
    if (RequiredDxilMinor != 0xF && stage.compare("rootsig") != 0) {
      // Convert stage to minimum dxil/validator version:
      RequiredDxilMajor = std::max(RequiredDxilMajor, (unsigned)6) - 5;

      bool bInternalValidator =
          opts.SelectValidator == hlsl::options::ValidatorSelection::Internal;
      bool bValVerExplicit = opts.ValVerMajor != UINT_MAX;

      // Normally we must check the validator version as well, but there are
      // two scenarios where the validator version doesn't need to be checked
      // against the version based on the shader model:
      // 1. The test selects internal validator.
      // 2. The test explicitly requests a specific validator version.
      FileRunCommandResult result =
          CheckDxilVer(DllSupport, RequiredDxilMajor, RequiredDxilMinor,
                       !(bInternalValidator || bValVerExplicit));
      if (result.AbortPipeline)
        return result;

      // Additionally, if the test explicitly requests a specific non-zero
      // validator version, and doesn't select internal validator or disable
      // validation, we must check that the validator version is at least as
      // high as the requested version.
      // When ValVerMajor is 0, validation cannot be run against the module.
      if (bValVerExplicit && opts.ValVerMajor != 0 &&
          !(bInternalValidator || opts.DisableValidation))
        result = CheckDxilVer(DllSupport, opts.ValVerMajor, opts.ValVerMinor);
      if (result.AbortPipeline)
        return result;
    }
  }

  // For now, too many tests are sensitive to stripping the refleciton info
  // from the main module, so use this flag to prevent this until tests
  // can be updated.
  // That is, unless the test explicitly requests -Qstrip_reflect_from_dxil or
  // -Qstrip_reflect
  if (!opts.StripReflectionFromDxil && !opts.StripReflection) {
    flags.push_back(L"-Qkeep_reflect_in_dxil");
  }

  std::vector<std::wstring> argWStrings;
  CopyArgsToWStrings(opts.Args, hlsl::options::CoreOption, argWStrings);
  for (const std::wstring &a : argWStrings)
    flags.push_back(a.data());

  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlobEncoding> pDisassembly;
  CComPtr<IDxcBlob> pCompiledBlob;

  HRESULT resultStatus;

  IFT(DllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
  IFT(pLibrary->CreateBlobFromFile(CommandFileName, nullptr, &pSource));
  CComPtr<IncludeHandlerVFSOverlayForTest> pIncludeHandler =
      AllocVFSIncludeHandler(pLibrary, pVFS);
  IFT(DllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));
  IFT(pCompiler->Compile(pSource, CommandFileName, entry.c_str(),
                         profile.c_str(), flags.data(), flags.size(), nullptr,
                         0, pIncludeHandler, &pResult));
  IFT(pResult->GetStatus(&resultStatus));

  FileRunCommandResult result = {};
  if (SUCCEEDED(resultStatus)) {
    IFT(pResult->GetResult(&pCompiledBlob));
    if (!opts.AstDump && !opts.DumpDependencies && !opts.VerifyDiagnostics) {
      IFT(pCompiler->Disassemble(pCompiledBlob, &pDisassembly));
      result.StdOut = BlobToUtf8(pDisassembly);
    } else {
      result.StdOut = BlobToUtf8(pCompiledBlob);
    }
    CComPtr<IDxcBlobEncoding> pStdErr;
    IFT(pResult->GetErrorBuffer(&pStdErr));
    result.StdErr = BlobToUtf8(pStdErr);
    result.ExitCode = 0;
  } else {
    IFT(pResult->GetErrorBuffer(&pDisassembly));
    result.StdErr = BlobToUtf8(pDisassembly);
    result.ExitCode = resultStatus;
  }

  result.OpResult = pResult;
  return result;
}

FileRunCommandResult
FileRunCommandPart::RunDxv(dxc::DxcDllSupport &DllSupport,
                           const FileRunCommandResult *Prior) {
  std::string args(strtrim(Arguments));
  const char *inputPos = strstr(args.c_str(), "%s");
  if (inputPos == nullptr) {
    return FileRunCommandResult::Error(
        "Only supported pattern includes input file as argument");
  }
  args.erase(inputPos - args.c_str(), strlen("%s"));

  llvm::StringRef argsRef = args;
  llvm::SmallVector<llvm::StringRef, 8> splitArgs;
  argsRef.split(splitArgs, " ");
  IFTMSG(splitArgs.size() == 1, "wrong arg num for dxv");

  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcAssembler> pAssembler;
  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pResult;

  CComPtr<IDxcBlobEncoding> pSource;

  CComPtr<IDxcBlob> pContainerBlob;
  HRESULT resultStatus;

  IFT(DllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
  IFT(pLibrary->CreateBlobFromFile(CommandFileName, nullptr, &pSource));
  IFT(DllSupport.CreateInstance(CLSID_DxcAssembler, &pAssembler));
  IFT(pAssembler->AssembleToContainer(pSource, &pResult));
  IFT(pResult->GetStatus(&resultStatus));
  if (FAILED(resultStatus)) {
    CComPtr<IDxcBlobEncoding> pAssembleBlob;
    IFT(pResult->GetErrorBuffer(&pAssembleBlob));
    return FileRunCommandResult::Error(resultStatus, BlobToUtf8(pAssembleBlob));
  }
  IFT(pResult->GetResult(&pContainerBlob));

  IFT(DllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
  CComPtr<IDxcOperationResult> pValidationResult;
  IFT(pValidator->Validate(pContainerBlob, DxcValidatorFlags_InPlaceEdit,
                           &pValidationResult));
  IFT(pValidationResult->GetStatus(&resultStatus));

  if (FAILED(resultStatus)) {
    CComPtr<IDxcBlobEncoding> pValidateBlob;
    IFT(pValidationResult->GetErrorBuffer(&pValidateBlob));
    return FileRunCommandResult::Success(BlobToUtf8(pValidateBlob));
  }

  return FileRunCommandResult::Success("");
}

FileRunCommandResult
FileRunCommandPart::RunOpt(dxc::DxcDllSupport &DllSupport,
                           const FileRunCommandResult *Prior) {
  std::string args(strtrim(Arguments));
  const char *inputPos = strstr(args.c_str(), "%s");
  if (inputPos == nullptr && Prior == nullptr) {
    return FileRunCommandResult::Error(
        "Only supported patterns are input file as argument or prior "
        "command with disassembly");
  }

  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcOptimizer> pOptimizer;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlobEncoding> pOutputText;
  CComPtr<IDxcBlob> pOutputModule;

  IFT(DllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
  IFT(DllSupport.CreateInstance(CLSID_DxcOptimizer, &pOptimizer));

  if (inputPos != nullptr) {
    args.erase(inputPos - args.c_str(), strlen("%s"));
    IFT(pLibrary->CreateBlobFromFile(CommandFileName, nullptr, &pSource));
  } else {
    assert(Prior != nullptr && "else early check should have returned");
    CComPtr<IDxcAssembler> pAssembler;
    IFT(DllSupport.CreateInstance(CLSID_DxcAssembler, &pAssembler));
    IFT(pLibrary->CreateBlobWithEncodingFromPinned(
        Prior->StdOut.c_str(), Prior->StdOut.size(), CP_UTF8, &pSource));
  }

  args = strtrim(args);
  llvm::StringRef argsRef = args;
  llvm::SmallVector<llvm::StringRef, 8> splitArgs;
  argsRef.split(splitArgs, " ");
  std::vector<LPCWSTR> options;
  std::vector<std::wstring> optionStrings;
  for (llvm::StringRef S : splitArgs) {
    optionStrings.push_back(
        Unicode::UTF8ToWideStringOrThrow(strtrim(S.str()).c_str()));
  }

  // Add the options outside the above loop in case the vector is resized.
  for (const std::wstring &str : optionStrings)
    options.push_back(str.c_str());

  IFT(pOptimizer->RunOptimizer(pSource, options.data(), options.size(),
                               &pOutputModule, &pOutputText));
  return FileRunCommandResult::Success(BlobToUtf8(pOutputText));
}

FileRunCommandResult
FileRunCommandPart::RunListParts(dxc::DxcDllSupport &DllSupport,
                                 const FileRunCommandResult *Prior) {
  std::string args(strtrim(Arguments));
  const char *inputPos = strstr(args.c_str(), "%s");
  if (inputPos == nullptr && Prior == nullptr) {
    return FileRunCommandResult::Error(
        "Only supported patterns are input file as argument or prior "
        "command with disassembly");
  }

  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcAssembler> pAssembler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcContainerReflection> containerReflection;
  uint32_t partCount;
  CComPtr<IDxcBlob> pContainerBlob;
  HRESULT resultStatus;
  std::ostringstream ss;

  IFT(DllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
  IFT(DllSupport.CreateInstance(CLSID_DxcAssembler, &pAssembler));

  if (inputPos != nullptr) {
    args.erase(inputPos - args.c_str(), strlen("%s"));
    IFT(pLibrary->CreateBlobFromFile(CommandFileName, nullptr, &pSource));
  } else {
    assert(Prior != nullptr && "else early check should have returned");
    CComPtr<IDxcAssembler> pAssembler;
    IFT(DllSupport.CreateInstance(CLSID_DxcAssembler, &pAssembler));
    IFT(pLibrary->CreateBlobWithEncodingFromPinned(
        Prior->StdOut.c_str(), Prior->StdOut.size(), CP_UTF8, &pSource));
  }

  IFT(pAssembler->AssembleToContainer(pSource, &pResult));
  IFT(pResult->GetStatus(&resultStatus));
  if (FAILED(resultStatus)) {
    CComPtr<IDxcBlobEncoding> pAssembleBlob;
    IFT(pResult->GetErrorBuffer(&pAssembleBlob));
    return FileRunCommandResult::Error(resultStatus, BlobToUtf8(pAssembleBlob));
  }
  IFT(pResult->GetResult(&pContainerBlob));

  IFT(DllSupport.CreateInstance(CLSID_DxcContainerReflection,
                                &containerReflection));
  IFT(containerReflection->Load(pContainerBlob));
  IFT(containerReflection->GetPartCount(&partCount));

  ss << "Part count: " << partCount << "\n";

  for (uint32_t i = 0; i < partCount; ++i) {
    uint32_t kind;
    IFT(containerReflection->GetPartKind(i, &kind));
    CComPtr<IDxcBlob> pPart;
    IFT(containerReflection->GetPartContent(i, &pPart));
    // #0 - SFI0 (8 bytes)
    ss << "#" << i << " - ";
    ss << (char)(kind & 0xff) << (char)((kind >> 8) & 0xff)
       << (char)((kind >> 16) & 0xff) << (char)((kind >> 24) & 0xff);
    ss << " (" << pPart->GetBufferSize() << " bytes)\n";
  }

  ss.flush();

  return FileRunCommandResult::Success(ss.str());
}

FileRunCommandResult
FileRunCommandPart::RunD3DReflect(dxc::DxcDllSupport &DllSupport,
                                  const FileRunCommandResult *Prior) {
  std::string args(strtrim(Arguments));
  if (args != "%s")
    return FileRunCommandResult::Error(
        "Only supported pattern is a plain input file");
  if (!Prior)
    return FileRunCommandResult::Error(
        "Prior command required to generate stdin");

  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcAssembler> pAssembler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<ID3D12ShaderReflection> pShaderReflection;
  CComPtr<ID3D12LibraryReflection> pLibraryReflection;
  CComPtr<IDxcContainerReflection> containerReflection;
  uint32_t partCount;
  CComPtr<IDxcBlob> pContainerBlob;
  HRESULT resultStatus;
  bool blobFound = false;
  std::ostringstream ss;
  D3DReflectionDumper dumper(ss);

  IFT(DllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
  IFT(DllSupport.CreateInstance(CLSID_DxcAssembler, &pAssembler));

  // It would be better to properly honour %s for input, but for now, if prior
  // output is empty, use the file as input.
  if (Prior->StdOut.size()) {
    IFT(pLibrary->CreateBlobWithEncodingFromPinned(
        (LPCVOID)Prior->StdOut.c_str(), Prior->StdOut.size(), CP_UTF8,
        &pSource));
  } else {
    IFT(pLibrary->CreateBlobFromFile(CommandFileName, nullptr, &pSource));
  }

  IFT(pAssembler->AssembleToContainer(pSource, &pResult));
  IFT(pResult->GetStatus(&resultStatus));
  if (FAILED(resultStatus)) {
    CComPtr<IDxcBlobEncoding> pAssembleBlob;
    IFT(pResult->GetErrorBuffer(&pAssembleBlob));
    return FileRunCommandResult::Error(resultStatus, BlobToUtf8(pAssembleBlob));
  }
  IFT(pResult->GetResult(&pContainerBlob));

  VERIFY_SUCCEEDED(DllSupport.CreateInstance(CLSID_DxcContainerReflection,
                                             &containerReflection));
  VERIFY_SUCCEEDED(containerReflection->Load(pContainerBlob));
  VERIFY_SUCCEEDED(containerReflection->GetPartCount(&partCount));

  for (uint32_t i = 0; i < partCount; ++i) {
    uint32_t kind;
    VERIFY_SUCCEEDED(containerReflection->GetPartKind(i, &kind));
    if (kind == (uint32_t)hlsl::DxilFourCC::DFCC_DXIL) {
      blobFound = true;
      CComPtr<IDxcBlob> pPart;
      IFT(containerReflection->GetPartContent(i, &pPart));
      const hlsl::DxilProgramHeader *pProgramHeader =
          reinterpret_cast<const hlsl::DxilProgramHeader *>(
              pPart->GetBufferPointer());
      VERIFY_IS_TRUE(IsValidDxilProgramHeader(
          pProgramHeader, (uint32_t)pPart->GetBufferSize()));
      hlsl::DXIL::ShaderKind SK =
          hlsl::GetVersionShaderType(pProgramHeader->ProgramVersion);
      if (SK == hlsl::DXIL::ShaderKind::Library)
        VERIFY_SUCCEEDED(containerReflection->GetPartReflection(
            i, IID_PPV_ARGS(&pLibraryReflection)));
      else
        VERIFY_SUCCEEDED(containerReflection->GetPartReflection(
            i, IID_PPV_ARGS(&pShaderReflection)));
      break;
    } else if (kind == (uint32_t)hlsl::DxilFourCC::DFCC_RuntimeData) {
      CComPtr<IDxcBlob> pPart;
      IFT(containerReflection->GetPartContent(i, &pPart));
      hlsl::RDAT::DxilRuntimeData rdat(pPart->GetBufferPointer(),
                                       pPart->GetBufferSize());
      DumpContext d(ss);
      DumpRuntimeData(rdat, d);
    }
  }

  if (!blobFound) {
    return FileRunCommandResult::Error("Unable to find DXIL part");
  } else if (pShaderReflection) {
    dumper.Dump(pShaderReflection);
  } else if (pLibraryReflection) {
    dumper.Dump(pLibraryReflection);
  }

  ss.flush();

  return FileRunCommandResult::Success(ss.str());
}

FileRunCommandResult
FileRunCommandPart::RunDxr(dxc::DxcDllSupport &DllSupport,
                           const FileRunCommandResult *Prior) {
  // Support piping stdin from prior if needed.
  UNREFERENCED_PARAMETER(Prior);
  hlsl::options::MainArgs args;
  hlsl::options::DxcOpts opts;
  FileRunCommandResult readOptsResult =
      ReadOptsForDxc(args, opts, hlsl::options::HlslFlags::RewriteOption);
  if (readOptsResult.ExitCode)
    return readOptsResult;

  std::wstring entry =
      Unicode::UTF8ToWideStringOrThrow(opts.EntryPoint.str().c_str());

  std::vector<LPCWSTR> flags;
  std::vector<std::wstring> argWStrings;
  CopyArgsToWStrings(opts.Args, hlsl::options::RewriteOption, argWStrings);
  for (const std::wstring &a : argWStrings)
    flags.push_back(a.data());

  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcRewriter2> pRewriter;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlob> pResultBlob;

  IFT(DllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
  IFT(pLibrary->CreateBlobFromFile(CommandFileName, nullptr, &pSource));
  CComPtr<IncludeHandlerVFSOverlayForTest> pIncludeHandler =
      AllocVFSIncludeHandler(pLibrary, pVFS);
  IFT(DllSupport.CreateInstance(CLSID_DxcRewriter, &pRewriter));
  IFT(pRewriter->RewriteWithOptions(pSource, CommandFileName, flags.data(),
                                    flags.size(), nullptr, 0, pIncludeHandler,
                                    &pResult));

  HRESULT resultStatus;
  IFT(pResult->GetStatus(&resultStatus));

  FileRunCommandResult result = {};
  CComPtr<IDxcBlobEncoding> pStdErr;
  IFT(pResult->GetErrorBuffer(&pStdErr));
  result.StdErr = BlobToUtf8(pStdErr);
  result.ExitCode = resultStatus;
  if (SUCCEEDED(resultStatus)) {
    IFT(pResult->GetResult(&pResultBlob));
    result.StdOut = BlobToUtf8(pResultBlob);
  }

  result.OpResult = pResult;
  return result;
}

FileRunCommandResult
FileRunCommandPart::RunLink(dxc::DxcDllSupport &DllSupport,
                            const FileRunCommandResult *Prior) {
  hlsl::options::MainArgs args;
  hlsl::options::DxcOpts opts;
  FileRunCommandResult readOptsResult =
      ReadOptsForDxc(args, opts, hlsl::options::HlslFlags::CoreOption);
  if (readOptsResult.ExitCode)
    return readOptsResult;

  std::wstring entry =
      Unicode::UTF8ToWideStringOrThrow(opts.EntryPoint.str().c_str());
  std::wstring profile =
      Unicode::UTF8ToWideStringOrThrow(opts.TargetProfile.str().c_str());
  std::vector<LPCWSTR> flags;

  // Skip targets that require a newer compiler or validator.
  // Some features may require newer compiler/validator than indicated by the
  // shader model, but these should use %dxilver explicitly.
  {
    unsigned RequiredDxilMajor = 1, RequiredDxilMinor = 0;
    llvm::StringRef stage;
    IFTBOOL(ParseTargetProfile(opts.TargetProfile, stage, RequiredDxilMajor,
                               RequiredDxilMinor),
            E_INVALIDARG);
    if (RequiredDxilMinor != 0xF && stage.compare("rootsig") != 0) {
      // Convert stage to minimum dxil/validator version:
      RequiredDxilMajor = std::max(RequiredDxilMajor, (unsigned)6) - 5;
      FileRunCommandResult result =
          CheckDxilVer(DllSupport, RequiredDxilMajor, RequiredDxilMinor,
                       !opts.DisableValidation);
      if (result.AbortPipeline) {
        return result;
      }
    }
  }

  // For now, too many tests are sensitive to stripping the refleciton info
  // from the main module, so use this flag to prevent this until tests
  // can be updated.
  // That is, unless the test explicitly requests -Qstrip_reflect_from_dxil or
  // -Qstrip_reflect
  if (!opts.StripReflectionFromDxil && !opts.StripReflection) {
    flags.push_back(L"-Qkeep_reflect_in_dxil");
  }

  std::vector<std::wstring> argWStrings;
  CopyArgsToWStrings(opts.Args, hlsl::options::CoreOption, argWStrings);
  for (const std::wstring &a : argWStrings)
    flags.push_back(a.data());

  // Parse semicolon separated list of library names.
  llvm::StringRef optLibraries =
      opts.Args.getLastArgValue(hlsl::options::OPT_INPUT);
  auto libs_utf8 = strtok(optLibraries.str().c_str(), ";");
  std::vector<std::wstring> libs_wide;
  for (auto name : libs_utf8)
    libs_wide.emplace_back(Unicode::UTF8ToWideStringOrThrow(name.c_str()));
  std::vector<LPCWSTR> libNames;
  for (auto &name : libs_wide)
    libNames.emplace_back(name.data());

  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcLinker> pLinker;
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pDisassembly;
  CComPtr<IDxcBlob> pCompiledBlob;

  HRESULT resultStatus;

  IFT(DllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
  CComPtr<IncludeHandlerVFSOverlayForTest> pIncludeHandler =
      AllocVFSIncludeHandler(pLibrary, pVFS);
  IFT(DllSupport.CreateInstance(CLSID_DxcLinker, &pLinker));
  IFT(DllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));

  for (auto name : libNames) {
    CComPtr<IDxcBlob> pLibBlob;
    IFT(pIncludeHandler->LoadSource(name, &pLibBlob));
    IFT(pLinker->RegisterLibrary(name, pLibBlob));
  }

  IFT(pLinker->Link(entry.c_str(), profile.c_str(), libNames.data(),
                    libNames.size(), flags.data(), flags.size(), &pResult));
  IFT(pResult->GetStatus(&resultStatus));

  FileRunCommandResult result = {};
  if (SUCCEEDED(resultStatus)) {
    IFT(pResult->GetResult(&pCompiledBlob));
    if (!opts.AstDump) {
      IFT(pCompiler->Disassemble(pCompiledBlob, &pDisassembly));
      result.StdOut = BlobToUtf8(pDisassembly);
    } else {
      result.StdOut = BlobToUtf8(pCompiledBlob);
    }
    CComPtr<IDxcBlobEncoding> pStdErr;
    IFT(pResult->GetErrorBuffer(&pStdErr));
    result.StdErr = BlobToUtf8(pStdErr);
    result.ExitCode = 0;
  } else {
    IFT(pResult->GetErrorBuffer(&pDisassembly));
    result.StdErr = BlobToUtf8(pDisassembly);
    result.ExitCode = resultStatus;
  }

  result.OpResult = pResult;
  return result;
}

FileRunCommandResult
FileRunCommandPart::RunTee(const FileRunCommandResult *Prior) {
  if (Prior == nullptr) {
    return FileRunCommandResult::Error("tee requires a prior command");
  }

  // Ignore commands for now - simply log out through test framework.
  {
    CA2W outWide(Prior->StdOut.c_str());
    WEX::Logging::Log::Comment(outWide.m_psz);
  }
  if (!Prior->StdErr.empty()) {
    CA2W errWide(Prior->StdErr.c_str());
    WEX::Logging::Log::Comment(L"<stderr>");
    WEX::Logging::Log::Comment(errWide.m_psz);
  }

  return *Prior;
}

void FileRunCommandPart::SubstituteFilenameVars(std::string &args) {
  size_t pos;
  std::string baseFileName = LPSTR(CW2A(CommandFileName));
  if ((pos = baseFileName.find_last_of(".")) != std::string::npos) {
    baseFileName = baseFileName.substr(0, pos);
  }
  while ((pos = args.find("%t")) != std::string::npos) {
    args.replace(pos, 2, baseFileName.c_str());
  }
  while ((pos = args.find("%b")) != std::string::npos) {
    args.replace(pos, 2, baseFileName.c_str());
  }
}

#if _WIN32
bool FileRunCommandPart::ReadFileContentToString(HANDLE hFile,
                                                 std::string &str) {
  char buffer[1024];
  DWORD len;

  size_t size = ::GetFileSize(hFile, nullptr);
  if (size == INVALID_FILE_SIZE) {
    return false;
  }
  str.reserve(size);

  if (::SetFilePointer(hFile, 0, nullptr, FILE_BEGIN) ==
      INVALID_SET_FILE_POINTER) {
    return false;
  }

  while (::ReadFile(hFile, buffer, sizeof(buffer), &len, nullptr) && len > 0) {
    str.append(buffer, len);
  }
  return true;
}
#endif

FileRunCommandResult
FileRunCommandPart::RunFileCompareText(const FileRunCommandResult *Prior) {
  if (Prior != nullptr) {
    return FileRunCommandResult::Error("prior command not supported");
  }

  FileRunCommandResult result;
  result.ExitCode = 1;

  // strip leading and trailing spaces and split
  std::string args(strtrim(Arguments));
  size_t pos;
  if ((pos = args.find_first_of(' ')) == std::string::npos) {
    return FileRunCommandResult::Error(
        "RunFileCompareText expected 2 file arguments.");
  }
  std::string fileName1 = args.substr(0, pos);
  std::string fileName2 = strtrim(args.substr(pos + 1));

  // replace %t and %b with the command file name without extension
  SubstituteFilenameVars(fileName1);
  SubstituteFilenameVars(fileName2);

  // read file content and compare
  CA2W fileName1W(fileName1.c_str());
  CA2W fileName2W(fileName2.c_str());
  hlsl_test::LogCommentFmt(L"Comparing files %s and %s", fileName1W.m_psz,
                           fileName2W.m_psz);

  std::ifstream ifs1(fileName1, std::ifstream::in);
  if (ifs1.fail()) {
    hlsl_test::LogCommentFmt(L"Failed to open %s", fileName1W.m_psz);
    return result;
  }
  std::string file1Content((std::istreambuf_iterator<char>(ifs1)),
                           (std::istreambuf_iterator<char>()));

  std::ifstream ifs2(fileName2, std::ifstream::in);
  if (ifs2.fail()) {
    hlsl_test::LogCommentFmt(L"Failed to open %s", fileName2W.m_psz);
    return result;
  }
  std::string file2Content((std::istreambuf_iterator<char>(ifs2)),
                           (std::istreambuf_iterator<char>()));

  if (file1Content.compare(file2Content) == 0) {
    hlsl_test::LogCommentFmt(L"No differences found.");
    result.ExitCode = 0;
  } else {
    hlsl_test::LogCommentFmt(L"Files are different!");
  }
  return result;
}

FileRunCommandResult
FileRunCommandPart::RunXFail(const FileRunCommandResult *Prior) {
  if (Prior == nullptr)
    return FileRunCommandResult::Error("XFail requires a prior command");

  if (Prior->ExitCode == 0) {
    return FileRunCommandResult::Error(
        "XFail expected a failure from previous command");
  } else {
    return FileRunCommandResult::Success("");
  }
}

FileRunCommandResult
FileRunCommandPart::RunDxilVer(dxc::DxcDllSupport &DllSupport,
                               const FileRunCommandResult *Prior) {
  Arguments = strtrim(Arguments);
  if (Arguments.size() != 3 || !std::isdigit(Arguments[0]) ||
      Arguments[1] != '.' || !std::isdigit(Arguments[2])) {
    return FileRunCommandResult::Error("Invalid dxil version format");
  }

  unsigned RequiredDxilMajor = Arguments[0] - '0';
  unsigned RequiredDxilMinor = Arguments[2] - '0';

  return CheckDxilVer(DllSupport, RequiredDxilMajor, RequiredDxilMinor);
}

#ifndef _WIN32
FileRunCommandResult
FileRunCommandPart::RunFromPath(const std::string &toolPath,
                                const FileRunCommandResult *Prior) {
  return FileRunCommandResult::Error("RunFromPath not supported");
}
#else  //_WIN32
FileRunCommandResult
FileRunCommandPart::RunFromPath(const std::string &toolPath,
                                const FileRunCommandResult *Prior) {
  if (Prior != nullptr) {
    return FileRunCommandResult::Error("prior command not supported");
  }

  std::string args = Arguments;

  // replace %s with command file name
  size_t pos;
  while ((pos = args.find("%s")) != std::string::npos) {
    args.replace(pos, 2, CW2A(CommandFileName));
  }

  // replace %t and %b with the command file name without extension
  SubstituteFilenameVars(args);

  // Run the tool via CreateProcess, redirect stdout and strerr to temporary
  // files
  std::wstring stdOutFileName = std::wstring(CommandFileName) + L".tmp_stdout";
  std::wstring stdErrFileName = std::wstring(CommandFileName) + L".tmp_stderr";

  SECURITY_ATTRIBUTES sa;
  ZeroMemory(&sa, sizeof(sa));
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.bInheritHandle = true;

  HANDLE hStdOutFile =
      CreateFileW(stdOutFileName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, &sa,
                  CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, nullptr);
  IFT(hStdOutFile != INVALID_HANDLE_VALUE);
  HANDLE hStdErrFile =
      CreateFileW(stdErrFileName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, &sa,
                  CREATE_ALWAYS, FILE_FLAG_DELETE_ON_CLOSE, nullptr);
  IFT(hStdErrFile != INVALID_HANDLE_VALUE);

  STARTUPINFOA si;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  si.hStdOutput = hStdOutFile;
  si.hStdError = hStdErrFile;
  si.dwFlags |= STARTF_USESTDHANDLES;

  PROCESS_INFORMATION pi;
  ZeroMemory(&pi, sizeof(pi));

  std::vector<char> args2(
      args.c_str(), args.c_str() + args.size() +
                        1); // args to CreateProcess cannot be const char *
  if (!CreateProcessA(toolPath.c_str(), args2.data(), nullptr, nullptr, true, 0,
                      nullptr, nullptr, &si, &pi)) {
    return FileRunCommandResult::Error("CreateProcess failed.");
  }
  ::WaitForSingleObject(pi.hProcess, 10000); // 10s timeout

  // Get exit code of the process
  FileRunCommandResult result;
  DWORD exitCode;
  if (!::GetExitCodeProcess(pi.hProcess, &exitCode)) {
    result = FileRunCommandResult::Error("GetExitCodeProcess failed.");
  } else {
    result.ExitCode = exitCode;
  }

  // Close process and thread handles
  ::CloseHandle(pi.hProcess);
  ::CloseHandle(pi.hThread);

  // Read stdout and strerr output from temporary files
  if (!ReadFileContentToString(hStdOutFile, result.StdOut) ||
      !ReadFileContentToString(hStdErrFile, result.StdErr)) {
    result = FileRunCommandResult::Error("RunFromPaths failed.");
  }

  // Close temporary file handles - will delete the files
  IFT(::CloseHandle(hStdOutFile));
  IFT(::CloseHandle(hStdErrFile));

  return result;
}
#endif //_WIN32

class FileRunTestResultImpl : public FileRunTestResult {
  dxc::DxcDllSupport &m_support;
  PluginToolsPaths *m_pPluginToolsPaths;
  LPCWSTR m_dumpName = nullptr;
  // keep track of virtual files for duration of this test (for all RUN lines)
  FileMap Files;

  void RunHashTestFromCommands(LPCSTR commands, LPCWSTR fileName) {
    std::vector<FileRunCommandPart> parts;
    ParseCommandParts(commands, fileName, parts);
    FileRunCommandResult result;
    bool ran = false;
    for (FileRunCommandPart &part : parts) {
      result = part.RunHashTests(m_support);
      ran = true;
      break;
    }
    if (ran) {
      this->RunResult = result.ExitCode;
      this->ErrorMessage = result.StdErr;
    } else {
      this->RunResult = 0;
    }
  }

  void RunFileCheckFromCommands(LPCSTR commands, LPCWSTR fileName,
                                LPCWSTR dumpName = nullptr) {
    std::vector<FileRunCommandPart> parts;
    ParseCommandParts(commands, fileName, parts);

    if (parts.empty()) {
      this->RunResult = 1;
      this->ErrorMessage = "FileCheck found no commands to run";
      return;
    }
    FileRunCommandResult result;
    FileRunCommandResult *previousResult = nullptr;
    FileRunCommandPart *pPrior = nullptr;
    for (FileRunCommandPart &part : parts) {
      int priorExitCode = result.ExitCode;
      part.pVFS = &Files;
      result =
          part.Run(m_support, previousResult, m_pPluginToolsPaths, dumpName);

      // If there is IDxcResult, save named output blobs to Files.
      AddOutputsToFileMap(result.OpResult, &Files);

      // When current failing stage is FileCheck, print prior command,
      // as well as FileCheck command that failed, to help identify
      // failing commands in longer run chains.
      if (result.ExitCode &&
          (0 == _stricmp(part.Command.c_str(), "FileCheck") ||
           0 == _stricmp(part.Command.c_str(), "%FileCheck"))) {
        std::ostringstream oss;
        if (pPrior) {
          oss << "Prior (" << priorExitCode << "): " << pPrior->Command
              << pPrior->Arguments << endl;
        }
        oss << "Error (" << result.ExitCode << "): " << part.Command
            << part.Arguments << endl;
        oss << result.StdErr;
        result.StdErr = oss.str();
      }

      if (result.AbortPipeline)
        break;
      previousResult = &result;
      pPrior = &part;
    }

    this->RunResult = result.ExitCode;
    this->ErrorMessage = result.StdErr;
  }

public:
  FileRunTestResultImpl(dxc::DxcDllSupport &support,
                        PluginToolsPaths *pPluginToolsPaths = nullptr,
                        LPCWSTR dumpName = nullptr)
      : m_support(support), m_pPluginToolsPaths(pPluginToolsPaths),
        m_dumpName(dumpName) {}
  void RunFileCheckFromFileCommands(LPCWSTR fileName) {
    // Assume UTF-8 files.
    auto cmds = GetRunLines(fileName);
    // Iterate over all RUN lines
    unsigned runIdx = 0;
    for (auto &cmd : cmds) {
      std::wstring dumpStr;
      std::wstringstream os;
      LPCWSTR dumpName = nullptr;
      if (m_dumpName) {
        os << m_dumpName << L"." << runIdx << L".txt";
        dumpStr = os.str();
        dumpName = dumpStr.c_str();
      }
      RunFileCheckFromCommands(cmd.c_str(), fileName, dumpName);
      // If any of the RUN cmd fails then skip executing remaining cmds
      // and report the error
      if (this->RunResult != 0) {
        this->ErrorMessage = cmd + "\n" + this->ErrorMessage;
        break;
      }
      runIdx += 1;
    }
  }

  void RunHashTestFromFileCommands(LPCWSTR fileName) {
    // Assume UTF-8 files.
    std::string commands(GetFirstLine(fileName));
    return RunHashTestFromCommands(commands.c_str(), fileName);
  }
};

FileRunTestResult
FileRunTestResult::RunHashTestFromFileCommands(LPCWSTR fileName) {
  dxc::DxcDllSupport dllSupport;
  IFT(dllSupport.Initialize());
  FileRunTestResultImpl result(dllSupport);
  result.RunHashTestFromFileCommands(fileName);
  return std::move(result);
}

FileRunTestResult FileRunTestResult::RunFromFileCommands(
    LPCWSTR fileName, PluginToolsPaths *pPluginToolsPaths /*=nullptr*/,
    LPCWSTR dumpName /*=nullptr*/) {
  dxc::DxcDllSupport dllSupport;
  IFT(dllSupport.Initialize());
  FileRunTestResultImpl result(dllSupport, pPluginToolsPaths, dumpName);
  result.RunFileCheckFromFileCommands(fileName);
  return std::move(result);
}

FileRunTestResult FileRunTestResult::RunFromFileCommands(
    LPCWSTR fileName, dxc::DxcDllSupport &dllSupport,
    PluginToolsPaths *pPluginToolsPaths /*=nullptr*/,
    LPCWSTR dumpName /*=nullptr*/) {
  FileRunTestResultImpl result(dllSupport, pPluginToolsPaths, dumpName);
  result.RunFileCheckFromFileCommands(fileName);
  return std::move(result);
}

void ParseCommandParts(LPCSTR commands, LPCWSTR fileName,
                       std::vector<FileRunCommandPart> &parts) {
  // Barely enough parsing here.
  commands = strstr(commands, "RUN: ");
  if (!commands) {
    return;
  }
  commands += strlen("RUN: ");

  LPCSTR endCommands = strchr(commands, '\0');
  while (commands != endCommands) {
    LPCSTR nextStart;
    LPCSTR thisEnd = strchr(commands, '|');
    if (!thisEnd) {
      nextStart = thisEnd = endCommands;
    } else {
      nextStart = thisEnd + 2;
    }
    LPCSTR commandEnd = strchr(commands, ' ');
    if (!commandEnd)
      commandEnd = endCommands;
    parts.emplace_back(std::string(commands, commandEnd),
                       std::string(commandEnd, thisEnd), fileName);
    commands = nextStart;
  }
}

void ParseCommandPartsFromFile(LPCWSTR fileName,
                               std::vector<FileRunCommandPart> &parts) {
  // Assume UTF-8 files.
  std::string commands(GetFirstLine(fileName));
  ParseCommandParts(commands.c_str(), fileName, parts);
}
