///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// CompilerTest.cpp                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides tests for the compiler API.                                      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef UNICODE
#define UNICODE
#endif

// clang-format off
// Includes on Windows are highly order dependent.
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <cassert>
#include <sstream>
#include <algorithm>
#include <cfloat>
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/Support/D3DReflection.h"
#include "dxc/dxcapi.h"
#ifdef _WIN32
#include "dxc/dxcpix.h"
#include <atlfile.h>
#include <d3dcompiler.h>
#include "dia2.h"
#else // _WIN32
#ifndef __ANDROID__
#include <execinfo.h>
#define CaptureStackBackTrace(FramesToSkip, FramesToCapture, BackTrace,        \
                              BackTraceHash)                                   \
  backtrace(BackTrace, FramesToCapture)
#endif // __ANDROID__
#endif // _WIN32

#include "dxc/Test/HLSLTestData.h"
#include "dxc/Test/HlslTestUtils.h"
#include "dxc/Test/DxcTestUtils.h"

#include "llvm/Support/raw_os_ostream.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/dxcapi.use.h"
#include "dxc/Support/microcom.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/Unicode.h"

#include <fstream>
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MSFileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringSwitch.h"
// clang-format on

// These are helper macros for adding slashes down below
// based on platform. The PP_ prefixed ones are for matching
// slashes in preprocessed HLSLs, since backslashes that
// appear in #line directives are double backslashes.
#ifdef _WIN32
#define SLASH_W L"\\"
#define SLASH "\\"
#else
#define SLASH_W L"/"
#define SLASH "/"
#endif

using namespace std;
using namespace hlsl_test;

class TestIncludeHandler : public IDxcIncludeHandler {
  DXC_MICROCOM_REF_FIELD(m_dwRef)
public:
  DXC_MICROCOM_ADDREF_RELEASE_IMPL(m_dwRef)
  dxc::DxcDllSupport &m_dllSupport;
  HRESULT m_defaultErrorCode = E_FAIL;
  TestIncludeHandler(dxc::DxcDllSupport &dllSupport)
      : m_dwRef(0), m_dllSupport(dllSupport), callIndex(0) {}
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcIncludeHandler>(this, iid, ppvObject);
  }

  struct LoadSourceCallInfo {
    std::wstring Filename; // Filename as written in #include statement
    LoadSourceCallInfo(LPCWSTR pFilename) : Filename(pFilename) {}
  };
  std::vector<LoadSourceCallInfo> CallInfos;
  std::wstring GetAllFileNames() const {
    std::wstringstream s;
    for (size_t i = 0; i < CallInfos.size(); ++i) {
      s << CallInfos[i].Filename << ';';
    }
    return s.str();
  }
  struct LoadSourceCallResult {
    HRESULT hr;
    std::string source;
    UINT32 codePage;
    LoadSourceCallResult() : hr(E_FAIL), codePage(0) {}
    LoadSourceCallResult(const char *pSource, UINT32 codePage = CP_UTF8)
        : hr(S_OK), source(pSource), codePage(codePage) {}
    LoadSourceCallResult(const void *pSource, size_t size,
                         UINT32 codePage = CP_ACP)
        : hr(S_OK), source((const char *)pSource, size), codePage(codePage) {}
  };
  std::vector<LoadSourceCallResult> CallResults;
  size_t callIndex;

  HRESULT STDMETHODCALLTYPE LoadSource(
      LPCWSTR pFilename,         // Filename as written in #include statement
      IDxcBlob **ppIncludeSource // Resultant source object for included file
      ) override {
    CallInfos.push_back(LoadSourceCallInfo(pFilename));

    *ppIncludeSource = nullptr;
    if (callIndex >= CallResults.size()) {
      return m_defaultErrorCode;
    }
    if (FAILED(CallResults[callIndex].hr)) {
      return CallResults[callIndex++].hr;
    }
    MultiByteStringToBlob(m_dllSupport, CallResults[callIndex].source,
                          CallResults[callIndex].codePage, ppIncludeSource);
    return CallResults[callIndex++].hr;
  }
};

#ifdef _WIN32
class CompilerTest {
#else
class CompilerTest : public ::testing::Test {
#endif
public:
  BEGIN_TEST_CLASS(CompilerTest)
  TEST_CLASS_PROPERTY(L"Parallel", L"true")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_CLASS_SETUP(InitSupport);

  TEST_METHOD(CompileWhenDefinesThenApplied)
  TEST_METHOD(CompileWhenDefinesManyThenApplied)
  TEST_METHOD(CompileWhenEmptyThenFails)
  TEST_METHOD(CompileWhenIncorrectThenFails)
  TEST_METHOD(CompileWhenWorksThenDisassembleWorks)
  TEST_METHOD(CompileWhenDebugWorksThenStripDebug)
  TEST_METHOD(CompileWhenWorksThenAddRemovePrivate)
  TEST_METHOD(CompileThenAddCustomDebugName)
  TEST_METHOD(CompileThenTestReflectionWithProgramHeader)
  TEST_METHOD(CompileThenTestPdbUtils)
  TEST_METHOD(CompileThenTestPdbUtilsWarningOpt)
  TEST_METHOD(CompileThenTestPdbInPrivate)
  TEST_METHOD(CompileThenTestPdbUtilsStripped)
  TEST_METHOD(CompileThenTestPdbUtilsEmptyEntry)
  TEST_METHOD(CompileThenTestPdbUtilsRelativePath)
  TEST_METHOD(CompileSameFilenameAndEntryThenTestPdbUtilsArgs)
  TEST_METHOD(CompileWithRootSignatureThenStripRootSignature)
  TEST_METHOD(CompileThenSetRootSignatureThenValidate)
  TEST_METHOD(CompileSetPrivateThenWithStripPrivate)
  TEST_METHOD(CompileWithMultiplePrivateOptionsThenFail)
  TEST_METHOD(TestPdbUtilsWithEmptyDefine)

  void CompileThenTestReflectionThreadSize(const char *source,
                                           const WCHAR *target, UINT expectedX,
                                           UINT expectedY, UINT expectedZ);

  TEST_METHOD(CompileThenTestReflectionThreadSizeMS)
  TEST_METHOD(CompileThenTestReflectionThreadSizeAS)
  TEST_METHOD(CompileThenTestReflectionThreadSizeCS)

  void TestResourceBindingImpl(const char *bindingFileContent,
                               const std::wstring &errors = std::wstring(),
                               bool noIncludeHandler = false);
  TEST_METHOD(CompileWithResourceBindingFileThenOK)

  TEST_METHOD(CompileWhenIncludeThenLoadInvoked)
  TEST_METHOD(CompileWhenIncludeThenLoadUsed)
  TEST_METHOD(CompileWhenIncludeAbsoluteThenLoadAbsolute)
  TEST_METHOD(CompileWhenIncludeLocalThenLoadRelative)
  TEST_METHOD(CompileWhenIncludeSystemThenLoadNotRelative)
  TEST_METHOD(CompileWhenAllIncludeCombinations)
  TEST_METHOD(TestPdbUtilsPathNormalizations)
  TEST_METHOD(CompileWithIncludeThenTestNoLexicalBlockFile)
  TEST_METHOD(CompileWhenIncludeSystemMissingThenLoadAttempt)
  TEST_METHOD(CompileWhenIncludeFlagsThenIncludeUsed)
  TEST_METHOD(CompileThenCheckDisplayIncludeProcess)
  TEST_METHOD(CompileThenPrintTimeReport)
  TEST_METHOD(CompileThenPrintTimeTrace)
  TEST_METHOD(CompileWhenIncludeMissingThenFail)
  TEST_METHOD(CompileWhenIncludeHasPathThenOK)
  TEST_METHOD(CompileWhenIncludeEmptyThenOK)

  TEST_METHOD(CompileWhenODumpThenPassConfig)
  TEST_METHOD(CompileWhenODumpThenCheckNoSink)
  TEST_METHOD(CompileWhenODumpThenOptimizerMatch)
  TEST_METHOD(CompileWhenVdThenProducesDxilContainer)

  void TestEncodingImpl(const void *sourceData, size_t sourceSize,
                        UINT32 codePage, const void *includedData,
                        size_t includedSize, const WCHAR *encoding = nullptr);
  TEST_METHOD(CompileWithEncodeFlagTestSource)

#if _ITERATOR_DEBUG_LEVEL == 0
  // CompileWhenNoMemThenOOM can properly detect leaks only when debug iterators
  // are disabled
  BEGIN_TEST_METHOD(CompileWhenNoMemThenOOM)
  // Disabled because there are problems where we try to allocate memory in
  // destructors, which causes more bad_alloc() throws while unwinding
  // bad_alloc(), which asserts If only failing one allocation, there are
  // allocations where failing them is lost, such as in ~raw_string_ostream(),
  // where it flushes, then eats bad_alloc(), if thrown.
  TEST_METHOD_PROPERTY(L"Ignore", L"true")
  END_TEST_METHOD()
#endif
  TEST_METHOD(CompileWhenShaderModelMismatchAttributeThenFail)
  TEST_METHOD(CompileBadHlslThenFail)
  TEST_METHOD(CompileLegacyShaderModelThenFail)
  TEST_METHOD(CompileWhenRecursiveAlbeitStaticTermThenFail)

  TEST_METHOD(CompileWhenRecursiveThenFail)

  TEST_METHOD(CompileHlsl2015ThenFail)
  TEST_METHOD(CompileHlsl2016ThenOK)
  TEST_METHOD(CompileHlsl2017ThenOK)
  TEST_METHOD(CompileHlsl2018ThenOK)
  TEST_METHOD(CompileHlsl2019ThenFail)
  TEST_METHOD(CompileHlsl2020ThenFail)
  TEST_METHOD(CompileHlsl2021ThenOK)
  TEST_METHOD(CompileHlsl2022ThenFail)

  TEST_METHOD(CodeGenFloatingPointEnvironment)
  TEST_METHOD(CodeGenLibCsEntry)
  TEST_METHOD(CodeGenLibCsEntry2)
  TEST_METHOD(CodeGenLibCsEntry3)
  TEST_METHOD(CodeGenLibEntries)
  TEST_METHOD(CodeGenLibEntries2)
  TEST_METHOD(CodeGenLibResource)
  TEST_METHOD(CodeGenLibUnusedFunc)

  TEST_METHOD(CodeGenRootSigProfile)
  TEST_METHOD(CodeGenRootSigProfile2)
  TEST_METHOD(CodeGenRootSigProfile5)
  TEST_METHOD(CodeGenVectorIsnan)
  TEST_METHOD(CodeGenVectorAtan2)
  TEST_METHOD(PreprocessWhenValidThenOK)
  TEST_METHOD(LibGVStore)
  TEST_METHOD(PreprocessWhenExpandTokenPastingOperandThenAccept)
  TEST_METHOD(PreprocessWithDebugOptsThenOk)
  TEST_METHOD(PreprocessCheckBuiltinIsOk)
  TEST_METHOD(WhenSigMismatchPCFunctionThenFail)
  TEST_METHOD(CompileOtherModesWithDebugOptsThenOk)

  TEST_METHOD(BatchSamples)
  TEST_METHOD(BatchD3DReflect)
  TEST_METHOD(BatchDxil)
  TEST_METHOD(BatchHLSL)
  TEST_METHOD(BatchInfra)
  TEST_METHOD(BatchPasses)
  TEST_METHOD(BatchShaderTargets)
  TEST_METHOD(BatchValidation)
  TEST_METHOD(BatchPIX)

  TEST_METHOD(CodeGenHashStabilityD3DReflect)
  TEST_METHOD(CodeGenHashStabilityDisassembler)
  TEST_METHOD(CodeGenHashStabilityDXIL)
  TEST_METHOD(CodeGenHashStabilityHLSL)
  TEST_METHOD(CodeGenHashStabilityInfra)
  TEST_METHOD(CodeGenHashStabilityPIX)
  TEST_METHOD(CodeGenHashStabilityRewriter)
  TEST_METHOD(CodeGenHashStabilitySamples)
  TEST_METHOD(CodeGenHashStabilityShaderTargets)
  TEST_METHOD(CodeGenHashStabilityValidation)

  TEST_METHOD(SubobjectCodeGenErrors)
  BEGIN_TEST_METHOD(ManualFileCheckTest)
  TEST_METHOD_PROPERTY(L"Ignore", L"true")
  END_TEST_METHOD()

  dxc::DxcDllSupport m_dllSupport;
  VersionSupportInfo m_ver;

  void CreateBlobPinned(LPCVOID data, SIZE_T size, UINT32 codePage,
                        IDxcBlobEncoding **ppBlob) {
    CComPtr<IDxcLibrary> library;
    IFT(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &library));
    IFT(library->CreateBlobWithEncodingFromPinned(data, size, codePage,
                                                  ppBlob));
  }

  void CreateBlobFromFile(LPCWSTR name, IDxcBlobEncoding **ppBlob) {
    CComPtr<IDxcLibrary> library;
    IFT(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &library));
    const std::wstring path = hlsl_test::GetPathToHlslDataFile(name);
    IFT(library->CreateBlobFromFile(path.c_str(), nullptr, ppBlob));
  }

  void CreateBlobFromText(const char *pText, IDxcBlobEncoding **ppBlob) {
    CreateBlobPinned(pText, strlen(pText) + 1, CP_UTF8, ppBlob);
  }

  HRESULT CreateCompiler(IDxcCompiler **ppResult) {
    return m_dllSupport.CreateInstance(CLSID_DxcCompiler, ppResult);
  }

  void TestPdbUtils(bool bSlim, bool bLegacy, bool bStrip,
                    bool bTestEntryPoint);

  HRESULT CreateContainerBuilder(IDxcContainerBuilder **ppResult) {
    return m_dllSupport.CreateInstance(CLSID_DxcContainerBuilder, ppResult);
  }

  template <typename T, typename TDefault, typename TIface>
  void WriteIfValue(TIface *pSymbol, std::wstringstream &o,
                    TDefault defaultValue, LPCWSTR valueLabel,
                    HRESULT (__stdcall TIface::*pFn)(T *)) {
    T value;
    HRESULT hr = (pSymbol->*(pFn))(&value);
    if (SUCCEEDED(hr) && value != defaultValue) {
      o << L", " << valueLabel << L": " << value;
    }
  }

  std::string GetOption(std::string &cmd, char *opt) {
    std::string option = cmd.substr(cmd.find(opt));
    option = option.substr(option.find_first_of(' '));
    option = option.substr(option.find_first_not_of(' '));
    return option.substr(0, option.find_first_of(' '));
  }

  void CodeGenTest(std::wstring name) {
    CComPtr<IDxcCompiler> pCompiler;
    CComPtr<IDxcOperationResult> pResult;
    CComPtr<IDxcBlobEncoding> pSource;

    name.insert(0, L"..\\CodeGenHLSL\\");

    VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
    CreateBlobFromFile(name.c_str(), &pSource);

    std::string cmdLine = GetFirstLine(name.c_str());

    llvm::StringRef argsRef = cmdLine;
    llvm::SmallVector<llvm::StringRef, 8> splitArgs;
    argsRef.split(splitArgs, " ");
    hlsl::options::MainArgs argStrings(splitArgs);
    std::string errorString;
    llvm::raw_string_ostream errorStream(errorString);
    hlsl::options::DxcOpts opts;
    IFT(ReadDxcOpts(hlsl::options::getHlslOptTable(), /*flagsToInclude*/ 0,
                    argStrings, opts, errorStream));
    std::wstring entry =
        Unicode::UTF8ToWideStringOrThrow(opts.EntryPoint.str().c_str());
    std::wstring profile =
        Unicode::UTF8ToWideStringOrThrow(opts.TargetProfile.str().c_str());

    std::vector<std::wstring> argLists;
    CopyArgsToWStrings(opts.Args, hlsl::options::CoreOption, argLists);

    std::vector<LPCWSTR> args;
    args.reserve(argLists.size());
    for (const std::wstring &a : argLists)
      args.push_back(a.data());

    VERIFY_SUCCEEDED(pCompiler->Compile(
        pSource, name.c_str(), entry.c_str(), profile.c_str(), args.data(),
        args.size(), opts.Defines.data(), opts.Defines.size(), nullptr,
        &pResult));
    VERIFY_IS_NOT_NULL(pResult, L"Failed to compile - pResult NULL");
    HRESULT result;
    VERIFY_SUCCEEDED(pResult->GetStatus(&result));
    if (FAILED(result)) {
      CComPtr<IDxcBlobEncoding> pErr;
      IFT(pResult->GetErrorBuffer(&pErr));
      std::string errString(BlobToUtf8(pErr));
      CA2W errStringW(errString.c_str());
      WEX::Logging::Log::Comment(L"Failed to compile - errors follow");
      WEX::Logging::Log::Comment(errStringW);
    }
    VERIFY_SUCCEEDED(result);

    CComPtr<IDxcBlob> pProgram;
    VERIFY_SUCCEEDED(pResult->GetResult(&pProgram));
    if (opts.IsRootSignatureProfile())
      return;

    CComPtr<IDxcBlobEncoding> pDisassembleBlob;
    VERIFY_SUCCEEDED(pCompiler->Disassemble(pProgram, &pDisassembleBlob));

    std::string disassembleString(BlobToUtf8(pDisassembleBlob));
    VERIFY_ARE_NOT_EQUAL(0U, disassembleString.size());
  }

  void CodeGenTestHashFullPath(LPCWSTR fullPath) {
    FileRunTestResult t =
        FileRunTestResult::RunHashTestFromFileCommands(fullPath);
    if (t.RunResult != 0) {
      CA2W commentWide(t.ErrorMessage.c_str());
      WEX::Logging::Log::Comment(commentWide);
      WEX::Logging::Log::Error(L"Run result is not zero");
    }
  }

  void CodeGenTestHash(LPCWSTR name, bool implicitDir) {
    std::wstring path = name;
    if (implicitDir) {
      path.insert(0, L"..\\CodeGenHLSL\\");
      path = hlsl_test::GetPathToHlslDataFile(path.c_str());
    }
    CodeGenTestHashFullPath(path.c_str());
  }

  void CodeGenTestCheckBatchHash(std::wstring suitePath,
                                 bool implicitDir = true) {
    using namespace llvm;
    using namespace WEX::TestExecution;

    if (implicitDir)
      suitePath.insert(0, L"..\\HLSLFileCheck\\");

    ::llvm::sys::fs::MSFileSystem *msfPtr;
    VERIFY_SUCCEEDED(CreateMSFileSystemForDisk(&msfPtr));
    std::unique_ptr<::llvm::sys::fs::MSFileSystem> msf(msfPtr);
    ::llvm::sys::fs::AutoPerThreadSystem pts(msf.get());
    IFTLLVM(pts.error_code());

    CW2A pUtf8Filename(suitePath.c_str());
    if (!llvm::sys::path::is_absolute(pUtf8Filename.m_psz)) {
      suitePath = hlsl_test::GetPathToHlslDataFile(suitePath.c_str());
    }

    CW2A utf8SuitePath(suitePath.c_str());

    unsigned numTestsRun = 0;

    std::error_code EC;
    llvm::SmallString<128> DirNative;
    llvm::sys::path::native(utf8SuitePath.m_psz, DirNative);
    for (llvm::sys::fs::recursive_directory_iterator Dir(DirNative, EC), DirEnd;
         Dir != DirEnd && !EC; Dir.increment(EC)) {
      // Check whether this entry has an extension typically associated with
      // headers.
      if (!llvm::StringSwitch<bool>(llvm::sys::path::extension(Dir->path()))
               .Cases(".hlsl", ".ll", true)
               .Default(false))
        continue;
      StringRef filename = Dir->path();
      std::string filetag = Dir->path();
      filetag += "<HASH>";

      CA2W wRelTag(filetag.data());
      CA2W wRelPath(filename.data());

      WEX::Logging::Log::StartGroup(wRelTag);
      CodeGenTestHash(wRelPath, /*implicitDir*/ false);
      WEX::Logging::Log::EndGroup(wRelTag);

      numTestsRun++;
    }

    VERIFY_IS_GREATER_THAN(numTestsRun, (unsigned)0,
                           L"No test files found in batch directory.");
  }

  void CodeGenTestCheckFullPath(LPCWSTR fullPath, LPCWSTR dumpPath = nullptr) {
    // Create file system if needed
    llvm::sys::fs::MSFileSystem *msfPtr =
        llvm::sys::fs::GetCurrentThreadFileSystem();
    std::unique_ptr<llvm::sys::fs::MSFileSystem> msf;
    if (!msfPtr) {
      VERIFY_SUCCEEDED(CreateMSFileSystemForDisk(&msfPtr));
      msf.reset(msfPtr);
    }
    llvm::sys::fs::AutoPerThreadSystem pts(msfPtr);
    IFTLLVM(pts.error_code());

    FileRunTestResult t = FileRunTestResult::RunFromFileCommands(
        fullPath,
        /*pPluginToolsPaths*/ nullptr, dumpPath);
    if (t.RunResult != 0) {
      CA2W commentWide(t.ErrorMessage.c_str());
      WEX::Logging::Log::Comment(commentWide);
      WEX::Logging::Log::Error(L"Run result is not zero");
    }
  }

  void CodeGenTestCheck(LPCWSTR name, bool implicitDir = true,
                        LPCWSTR dumpPath = nullptr) {
    std::wstring path = name;
    std::wstring dumpStr;
    if (implicitDir) {
      path.insert(0, L"..\\CodeGenHLSL\\");
      path = hlsl_test::GetPathToHlslDataFile(path.c_str());
      if (!dumpPath) {
        dumpStr = hlsl_test::GetPathToHlslDataFile(path.c_str(),
                                                   FILECHECKDUMPDIRPARAM);
        dumpPath = dumpStr.empty() ? nullptr : dumpStr.c_str();
      }
    }
    CodeGenTestCheckFullPath(path.c_str(), dumpPath);
  }

  void CodeGenTestCheckBatchDir(std::wstring suitePath,
                                bool implicitDir = true) {
    using namespace llvm;
    using namespace WEX::TestExecution;

    if (implicitDir)
      suitePath.insert(0, L"..\\HLSLFileCheck\\");

    ::llvm::sys::fs::MSFileSystem *msfPtr;
    VERIFY_SUCCEEDED(CreateMSFileSystemForDisk(&msfPtr));
    std::unique_ptr<::llvm::sys::fs::MSFileSystem> msf(msfPtr);
    ::llvm::sys::fs::AutoPerThreadSystem pts(msf.get());
    IFTLLVM(pts.error_code());

    std::wstring dumpPath;
    CW2A pUtf8Filename(suitePath.c_str());
    if (!llvm::sys::path::is_absolute(pUtf8Filename.m_psz)) {
      dumpPath = hlsl_test::GetPathToHlslDataFile(suitePath.c_str(),
                                                  FILECHECKDUMPDIRPARAM);
      suitePath = hlsl_test::GetPathToHlslDataFile(suitePath.c_str());
    }

    CW2A utf8SuitePath(suitePath.c_str());

    unsigned numTestsRun = 0;

    std::error_code EC;
    llvm::SmallString<128> DirNative;
    llvm::sys::path::native(utf8SuitePath.m_psz, DirNative);
    for (llvm::sys::fs::recursive_directory_iterator Dir(DirNative, EC), DirEnd;
         Dir != DirEnd && !EC; Dir.increment(EC)) {
      // Check whether this entry has an extension typically associated with
      // headers.
      if (!llvm::StringSwitch<bool>(llvm::sys::path::extension(Dir->path()))
               .Cases(".hlsl", ".ll", true)
               .Default(false))
        continue;
      StringRef filename = Dir->path();
      CA2W wRelPath(filename.data());
      std::wstring dumpStr;
      if (!dumpPath.empty() &&
          suitePath.compare(0, suitePath.size(), wRelPath.m_psz,
                            suitePath.size()) == 0) {
        dumpStr = dumpPath + (wRelPath.m_psz + suitePath.size());
      }

      class ScopedLogGroup {
        LPWSTR m_path;

      public:
        ScopedLogGroup(LPWSTR path) : m_path(path) {
          WEX::Logging::Log::StartGroup(m_path);
        }
        ~ScopedLogGroup() { WEX::Logging::Log::EndGroup(m_path); }
      };

      ScopedLogGroup cleanup(wRelPath);
      CodeGenTestCheck(wRelPath, /*implicitDir*/ false,
                       dumpStr.empty() ? nullptr : dumpStr.c_str());

      numTestsRun++;
    }

    VERIFY_IS_GREATER_THAN(numTestsRun, (unsigned)0,
                           L"No test files found in batch directory.");
  }

  std::string VerifyCompileFailed(LPCSTR pText, LPCWSTR pTargetProfile,
                                  LPCSTR pErrorMsg) {
    return VerifyCompileFailed(pText, pTargetProfile, pErrorMsg, L"main");
  }

  std::string VerifyCompileFailed(LPCSTR pText, LPCWSTR pTargetProfile,
                                  LPCSTR pErrorMsg, LPCWSTR pEntryPoint) {
    CComPtr<IDxcCompiler> pCompiler;
    CComPtr<IDxcOperationResult> pResult;
    CComPtr<IDxcBlobEncoding> pSource;
    CComPtr<IDxcBlobEncoding> pErrors;

    VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
    CreateBlobFromText(pText, &pSource);

    VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", pEntryPoint,
                                        pTargetProfile, nullptr, 0, nullptr, 0,
                                        nullptr, &pResult));

    HRESULT status;
    VERIFY_SUCCEEDED(pResult->GetStatus(&status));
    VERIFY_FAILED(status);
    VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&pErrors));
    if (pErrorMsg && *pErrorMsg) {
      CheckOperationResultMsgs(pResult, &pErrorMsg, 1, false, false);
    }
    return BlobToUtf8(pErrors);
  }

  void VerifyOperationSucceeded(IDxcOperationResult *pResult) {
    HRESULT result;
    VERIFY_SUCCEEDED(pResult->GetStatus(&result));
    if (FAILED(result)) {
      CComPtr<IDxcBlobEncoding> pErrors;
      VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&pErrors));
      CA2W errorsWide(BlobToUtf8(pErrors).c_str());
      WEX::Logging::Log::Comment(errorsWide);
    }
    VERIFY_SUCCEEDED(result);
  }

  std::string VerifyOperationFailed(IDxcOperationResult *pResult) {
    HRESULT result;
    VERIFY_SUCCEEDED(pResult->GetStatus(&result));
    VERIFY_FAILED(result);
    CComPtr<IDxcBlobEncoding> pErrors;
    VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&pErrors));
    return BlobToUtf8(pErrors);
  }

#ifdef _WIN32 // - exclude dia stuff
  HRESULT CreateDiaSourceForCompile(const char *hlsl,
                                    IDiaDataSource **ppDiaSource) {
    if (!ppDiaSource)
      return E_POINTER;

    CComPtr<IDxcCompiler> pCompiler;
    CComPtr<IDxcOperationResult> pResult;
    CComPtr<IDxcBlobEncoding> pSource;
    CComPtr<IDxcBlob> pProgram;

    VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
    CreateBlobFromText(hlsl, &pSource);
    LPCWSTR args[] = {L"/Zi", L"/Qembed_debug"};
    VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                        L"ps_6_0", args, _countof(args),
                                        nullptr, 0, nullptr, &pResult));
    VERIFY_SUCCEEDED(pResult->GetResult(&pProgram));

    // Disassemble the compiled (stripped) program.
    {
      CComPtr<IDxcBlobEncoding> pDisassembly;
      VERIFY_SUCCEEDED(pCompiler->Disassemble(pProgram, &pDisassembly));
      std::string disText = BlobToUtf8(pDisassembly);
      CA2W disTextW(disText.c_str());
      // WEX::Logging::Log::Comment(disTextW);
    }

    // CONSIDER: have the dia data source look for the part if passed a whole
    // container.
    CComPtr<IDiaDataSource> pDiaSource;
    CComPtr<IStream> pProgramStream;
    CComPtr<IDxcLibrary> pLib;
    VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));
    const hlsl::DxilContainerHeader *pContainer = hlsl::IsDxilContainerLike(
        pProgram->GetBufferPointer(), pProgram->GetBufferSize());
    VERIFY_IS_NOT_NULL(pContainer);
    hlsl::DxilPartIterator partIter =
        std::find_if(hlsl::begin(pContainer), hlsl::end(pContainer),
                     hlsl::DxilPartIsType(hlsl::DFCC_ShaderDebugInfoDXIL));
    const hlsl::DxilProgramHeader *pProgramHeader =
        (const hlsl::DxilProgramHeader *)hlsl::GetDxilPartData(*partIter);
    uint32_t bitcodeLength;
    const char *pBitcode;
    CComPtr<IDxcBlob> pProgramPdb;
    hlsl::GetDxilProgramBitcode(pProgramHeader, &pBitcode, &bitcodeLength);
    VERIFY_SUCCEEDED(pLib->CreateBlobFromBlob(
        pProgram, pBitcode - (char *)pProgram->GetBufferPointer(),
        bitcodeLength, &pProgramPdb));

    // Disassemble the program with debug information.
    {
      CComPtr<IDxcBlobEncoding> pDbgDisassembly;
      VERIFY_SUCCEEDED(pCompiler->Disassemble(pProgramPdb, &pDbgDisassembly));
      std::string disText = BlobToUtf8(pDbgDisassembly);
      CA2W disTextW(disText.c_str());
      // WEX::Logging::Log::Comment(disTextW);
    }

    // Create a short text dump of debug information.
    VERIFY_SUCCEEDED(
        pLib->CreateStreamFromBlobReadOnly(pProgramPdb, &pProgramStream));
    VERIFY_SUCCEEDED(
        m_dllSupport.CreateInstance(CLSID_DxcDiaDataSource, &pDiaSource));
    VERIFY_SUCCEEDED(pDiaSource->loadDataFromIStream(pProgramStream));
    *ppDiaSource = pDiaSource.Detach();
    return S_OK;
  }
#endif // _WIN32 - exclude dia stuff
};

// Useful for debugging.
#if SUPPORT_FXC_PDB
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
HRESULT GetBlobPdb(IDxcBlob *pBlob, IDxcBlob **ppDebugInfo) {
  return D3DGetBlobPart(pBlob->GetBufferPointer(), pBlob->GetBufferSize(),
                        D3D_BLOB_PDB, 0, (ID3DBlob **)ppDebugInfo);
}

std::string FourCCStr(uint32_t val) {
  std::stringstream o;
  char c[5];
  c[0] = val & 0xFF;
  c[1] = (val & 0xFF00) >> 8;
  c[2] = (val & 0xFF0000) >> 16;
  c[3] = (val & 0xFF000000) >> 24;
  c[4] = '\0';
  o << c << " (" << std::hex << val << std::dec << ")";
  return o.str();
}
std::string DumpParts(IDxcBlob *pBlob) {
  std::stringstream o;

  hlsl::DxilContainerHeader *pContainer =
      (hlsl::DxilContainerHeader *)pBlob->GetBufferPointer();
  o << "Container:" << std::endl
    << " Size: " << pContainer->ContainerSizeInBytes << std::endl
    << " FourCC: " << FourCCStr(pContainer->HeaderFourCC) << std::endl
    << " Part count: " << pContainer->PartCount << std::endl;
  for (uint32_t i = 0; i < pContainer->PartCount; ++i) {
    hlsl::DxilPartHeader *pPart = hlsl::GetDxilContainerPart(pContainer, i);
    o << "Part " << i << std::endl
      << " FourCC: " << FourCCStr(pPart->PartFourCC) << std::endl
      << " Size: " << pPart->PartSize << std::endl;
  }
  return o.str();
}

HRESULT CreateDiaSourceFromDxbcBlob(IDxcLibrary *pLib, IDxcBlob *pDxbcBlob,
                                    IDiaDataSource **ppDiaSource) {
  HRESULT hr = S_OK;
  CComPtr<IDxcBlob> pdbBlob;
  CComPtr<IStream> pPdbStream;
  CComPtr<IDiaDataSource> pDiaSource;
  IFR(GetBlobPdb(pDxbcBlob, &pdbBlob));
  IFR(pLib->CreateStreamFromBlobReadOnly(pdbBlob, &pPdbStream));
  IFR(CoCreateInstance(CLSID_DiaSource, NULL, CLSCTX_INPROC_SERVER,
                       __uuidof(IDiaDataSource), (void **)&pDiaSource));
  IFR(pDiaSource->loadDataFromIStream(pPdbStream));
  *ppDiaSource = pDiaSource.Detach();
  return hr;
}
#endif

bool CompilerTest::InitSupport() {
  if (!m_dllSupport.IsEnabled()) {
    VERIFY_SUCCEEDED(m_dllSupport.Initialize());
    m_ver.Initialize(m_dllSupport);
  }
  return true;
}

TEST_F(CompilerTest, CompileWhenDefinesThenApplied) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  DxcDefine defines[] = {{L"F4", L"float4"}};

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("F4 main() : SV_Target { return 0; }", &pSource);

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", nullptr, 0, defines,
                                      _countof(defines), nullptr, &pResult));
}

TEST_F(CompilerTest, CompileWhenDefinesManyThenApplied) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  LPCWSTR args[] = {L"/DVAL1=1",  L"/DVAL2=2",  L"/DVAL3=3",  L"/DVAL4=2",
                    L"/DVAL5=4",  L"/DNVAL1",   L"/DNVAL2",   L"/DNVAL3",
                    L"/DNVAL4",   L"/DNVAL5",   L"/DCVAL1=1", L"/DCVAL2=2",
                    L"/DCVAL3=3", L"/DCVAL4=2", L"/DCVAL5=4", L"/DCVALNONE="};

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("float4 main() : SV_Target {\r\n"
                     "#ifndef VAL1\r\n"
                     "#error VAL1 not defined\r\n"
                     "#endif\r\n"
                     "#ifndef NVAL5\r\n"
                     "#error NVAL5 not defined\r\n"
                     "#endif\r\n"
                     "#ifndef CVALNONE\r\n"
                     "#error CVALNONE not defined\r\n"
                     "#endif\r\n"
                     "return 0; }",
                     &pSource);
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, _countof(args), nullptr,
                                      0, nullptr, &pResult));
  HRESULT compileStatus;
  VERIFY_SUCCEEDED(pResult->GetStatus(&compileStatus));
  if (FAILED(compileStatus)) {
    CComPtr<IDxcBlobEncoding> pErrors;
    VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&pErrors));
    OutputDebugStringA((LPCSTR)pErrors->GetBufferPointer());
  }
  VERIFY_SUCCEEDED(compileStatus);
}

TEST_F(CompilerTest, CompileWhenEmptyThenFails) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlobEncoding> pSourceBad;
  LPCWSTR pProfile = L"ps_6_0";
  LPCWSTR pEntryPoint = L"main";

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("float4 main() : SV_Target { return 0; }", &pSource);
  CreateBlobFromText("float4 main() : SV_Target { return undef; }",
                     &pSourceBad);

  // correct version
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", pEntryPoint,
                                      pProfile, nullptr, 0, nullptr, 0, nullptr,
                                      &pResult));
  pResult.Release();

  // correct version with compilation errors
  VERIFY_SUCCEEDED(pCompiler->Compile(pSourceBad, L"source.hlsl", pEntryPoint,
                                      pProfile, nullptr, 0, nullptr, 0, nullptr,
                                      &pResult));
  pResult.Release();

  // null source
  VERIFY_FAILED(pCompiler->Compile(nullptr, L"source.hlsl", pEntryPoint,
                                   pProfile, nullptr, 0, nullptr, 0, nullptr,
                                   &pResult));

  // null profile
  VERIFY_FAILED(pCompiler->Compile(pSourceBad, L"source.hlsl", pEntryPoint,
                                   nullptr, nullptr, 0, nullptr, 0, nullptr,
                                   &pResult));

  // null source name succeeds
  VERIFY_SUCCEEDED(pCompiler->Compile(pSourceBad, nullptr, pEntryPoint,
                                      pProfile, nullptr, 0, nullptr, 0, nullptr,
                                      &pResult));
  pResult.Release();

  // empty source name (as opposed to null) also suceeds
  VERIFY_SUCCEEDED(pCompiler->Compile(pSourceBad, L"", pEntryPoint, pProfile,
                                      nullptr, 0, nullptr, 0, nullptr,
                                      &pResult));
  pResult.Release();

  // null result
  VERIFY_FAILED(pCompiler->Compile(pSource, L"source.hlsl", pEntryPoint,
                                   pProfile, nullptr, 0, nullptr, 0, nullptr,
                                   nullptr));
}

TEST_F(CompilerTest, CompileWhenIncorrectThenFails) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("float4_undefined main() : SV_Target { return 0; }",
                     &pSource);

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", nullptr, 0, nullptr, 0,
                                      nullptr, &pResult));
  HRESULT result;
  VERIFY_SUCCEEDED(pResult->GetStatus(&result));
  VERIFY_FAILED(result);

  CComPtr<IDxcBlobEncoding> pErrorBuffer;
  VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&pErrorBuffer));
  std::string errorString(BlobToUtf8(pErrorBuffer));
  VERIFY_ARE_NOT_EQUAL(0U, errorString.size());
  // Useful for examining actual error message:
  // CA2W errorStringW(errorString.c_str());
  // WEX::Logging::Log::Comment(errorStringW.m_psz);
}

TEST_F(CompilerTest, CompileWhenWorksThenDisassembleWorks) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("float4 main() : SV_Target { return 0; }", &pSource);

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", nullptr, 0, nullptr, 0,
                                      nullptr, &pResult));
  HRESULT result;
  VERIFY_SUCCEEDED(pResult->GetStatus(&result));
  VERIFY_SUCCEEDED(result);

  CComPtr<IDxcBlob> pProgram;
  VERIFY_SUCCEEDED(pResult->GetResult(&pProgram));

  CComPtr<IDxcBlobEncoding> pDisassembleBlob;
  VERIFY_SUCCEEDED(pCompiler->Disassemble(pProgram, &pDisassembleBlob));

  std::string disassembleString(BlobToUtf8(pDisassembleBlob));
  VERIFY_ARE_NOT_EQUAL(0U, disassembleString.size());
  // Useful for examining disassembly:
  // CA2W disassembleStringW(disassembleString.c_str());
  // WEX::Logging::Log::Comment(disassembleStringW.m_psz);
}

TEST_F(CompilerTest, CompileWhenDebugWorksThenStripDebug) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlob> pProgram;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("float4 main(float4 pos : SV_Position) : SV_Target {\r\n"
                     "  float4 local = abs(pos);\r\n"
                     "  return local;\r\n"
                     "}",
                     &pSource);
  LPCWSTR args[] = {L"/Zi", L"/Qembed_debug"};

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, _countof(args), nullptr,
                                      0, nullptr, &pResult));
  VERIFY_SUCCEEDED(pResult->GetResult(&pProgram));
  // Check if it contains debug blob
  hlsl::DxilContainerHeader *pHeader = hlsl::IsDxilContainerLike(
      pProgram->GetBufferPointer(), pProgram->GetBufferSize());
  VERIFY_SUCCEEDED(
      hlsl::IsValidDxilContainer(pHeader, pProgram->GetBufferSize()));
  hlsl::DxilPartHeader *pPartHeader = hlsl::GetDxilPartByType(
      pHeader, hlsl::DxilFourCC::DFCC_ShaderDebugInfoDXIL);
  VERIFY_IS_NOT_NULL(pPartHeader);
  // Check debug info part does not exist after strip debug info

  CComPtr<IDxcBlob> pNewProgram;
  CComPtr<IDxcContainerBuilder> pBuilder;
  VERIFY_SUCCEEDED(CreateContainerBuilder(&pBuilder));
  VERIFY_SUCCEEDED(pBuilder->Load(pProgram));
  VERIFY_SUCCEEDED(
      pBuilder->RemovePart(hlsl::DxilFourCC::DFCC_ShaderDebugInfoDXIL));
  pResult.Release();
  VERIFY_SUCCEEDED(pBuilder->SerializeContainer(&pResult));
  VERIFY_SUCCEEDED(pResult->GetResult(&pNewProgram));
  pHeader = hlsl::IsDxilContainerLike(pNewProgram->GetBufferPointer(),
                                      pNewProgram->GetBufferSize());
  VERIFY_SUCCEEDED(
      hlsl::IsValidDxilContainer(pHeader, pNewProgram->GetBufferSize()));
  pPartHeader = hlsl::GetDxilPartByType(
      pHeader, hlsl::DxilFourCC::DFCC_ShaderDebugInfoDXIL);
  VERIFY_IS_NULL(pPartHeader);
}

TEST_F(CompilerTest, CompileWhenWorksThenAddRemovePrivate) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlob> pProgram;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("float4 main() : SV_Target {\r\n"
                     "  return 0;\r\n"
                     "}",
                     &pSource);
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", nullptr, 0, nullptr, 0,
                                      nullptr, &pResult));
  VERIFY_SUCCEEDED(pResult->GetResult(&pProgram));
  // Append private data blob
  CComPtr<IDxcContainerBuilder> pBuilder;
  VERIFY_SUCCEEDED(CreateContainerBuilder(&pBuilder));

  std::string privateTxt("private data");
  CComPtr<IDxcBlobEncoding> pPrivate;
  CreateBlobFromText(privateTxt.c_str(), &pPrivate);
  VERIFY_SUCCEEDED(pBuilder->Load(pProgram));
  VERIFY_SUCCEEDED(
      pBuilder->AddPart(hlsl::DxilFourCC::DFCC_PrivateData, pPrivate));
  pResult.Release();
  VERIFY_SUCCEEDED(pBuilder->SerializeContainer(&pResult));

  CComPtr<IDxcBlob> pNewProgram;
  VERIFY_SUCCEEDED(pResult->GetResult(&pNewProgram));
  hlsl::DxilContainerHeader *pContainerHeader = hlsl::IsDxilContainerLike(
      pNewProgram->GetBufferPointer(), pNewProgram->GetBufferSize());
  VERIFY_SUCCEEDED(hlsl::IsValidDxilContainer(pContainerHeader,
                                              pNewProgram->GetBufferSize()));
  hlsl::DxilPartHeader *pPartHeader = hlsl::GetDxilPartByType(
      pContainerHeader, hlsl::DxilFourCC::DFCC_PrivateData);
  VERIFY_IS_NOT_NULL(pPartHeader);
  // compare data
  std::string privatePart((const char *)(pPartHeader + 1), privateTxt.size());
  VERIFY_IS_TRUE(strcmp(privatePart.c_str(), privateTxt.c_str()) == 0);

  // Remove private data blob
  pBuilder.Release();
  VERIFY_SUCCEEDED(CreateContainerBuilder(&pBuilder));
  VERIFY_SUCCEEDED(pBuilder->Load(pNewProgram));
  VERIFY_SUCCEEDED(pBuilder->RemovePart(hlsl::DxilFourCC::DFCC_PrivateData));
  pResult.Release();
  VERIFY_SUCCEEDED(pBuilder->SerializeContainer(&pResult));

  pNewProgram.Release();
  VERIFY_SUCCEEDED(pResult->GetResult(&pNewProgram));
  pContainerHeader = hlsl::IsDxilContainerLike(pNewProgram->GetBufferPointer(),
                                               pNewProgram->GetBufferSize());
  VERIFY_SUCCEEDED(hlsl::IsValidDxilContainer(pContainerHeader,
                                              pNewProgram->GetBufferSize()));
  pPartHeader = hlsl::GetDxilPartByType(pContainerHeader,
                                        hlsl::DxilFourCC::DFCC_PrivateData);
  VERIFY_IS_NULL(pPartHeader);
}

TEST_F(CompilerTest, CompileThenAddCustomDebugName) {
  // container builders prior to 1.3 did not support adding debug name parts
  if (m_ver.SkipDxilVersion(1, 3))
    return;
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlob> pProgram;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("float4 main() : SV_Target {\r\n"
                     "  return 0;\r\n"
                     "}",
                     &pSource);

  LPCWSTR args[] = {L"/Zi", L"/Qembed_debug", L"/Zss"};

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, _countof(args), nullptr,
                                      0, nullptr, &pResult));
  VERIFY_SUCCEEDED(pResult->GetResult(&pProgram));
  // Append private data blob
  CComPtr<IDxcContainerBuilder> pBuilder;
  VERIFY_SUCCEEDED(CreateContainerBuilder(&pBuilder));

  const char pNewName[] = "MyOwnUniqueName.lld";
  // include null terminator:
  size_t nameBlobPartSize =
      sizeof(hlsl::DxilShaderDebugName) + _countof(pNewName);
  // round up to four-byte size:
  size_t allocatedSize = (nameBlobPartSize + 3) & ~3;
  auto pNameBlobContent =
      reinterpret_cast<hlsl::DxilShaderDebugName *>(malloc(allocatedSize));
  ZeroMemory(pNameBlobContent,
             allocatedSize); // just to make sure trailing nulls are nulls.
  pNameBlobContent->Flags = 0;
  pNameBlobContent->NameLength =
      _countof(pNewName) - 1; // this is not supposed to include null terminator
  memcpy(pNameBlobContent + 1, pNewName, _countof(pNewName));

  CComPtr<IDxcBlobEncoding> pDebugName;

  CreateBlobPinned(pNameBlobContent, allocatedSize, DXC_CP_ACP, &pDebugName);

  VERIFY_SUCCEEDED(pBuilder->Load(pProgram));
  // should fail since it already exists:
  VERIFY_FAILED(
      pBuilder->AddPart(hlsl::DxilFourCC::DFCC_ShaderDebugName, pDebugName));
  VERIFY_SUCCEEDED(
      pBuilder->RemovePart(hlsl::DxilFourCC::DFCC_ShaderDebugName));
  VERIFY_SUCCEEDED(
      pBuilder->AddPart(hlsl::DxilFourCC::DFCC_ShaderDebugName, pDebugName));
  pResult.Release();
  VERIFY_SUCCEEDED(pBuilder->SerializeContainer(&pResult));

  CComPtr<IDxcBlob> pNewProgram;
  VERIFY_SUCCEEDED(pResult->GetResult(&pNewProgram));
  hlsl::DxilContainerHeader *pContainerHeader = hlsl::IsDxilContainerLike(
      pNewProgram->GetBufferPointer(), pNewProgram->GetBufferSize());
  VERIFY_SUCCEEDED(hlsl::IsValidDxilContainer(pContainerHeader,
                                              pNewProgram->GetBufferSize()));
  hlsl::DxilPartHeader *pPartHeader = hlsl::GetDxilPartByType(
      pContainerHeader, hlsl::DxilFourCC::DFCC_ShaderDebugName);
  VERIFY_IS_NOT_NULL(pPartHeader);
  // compare data
  VERIFY_IS_TRUE(memcmp(pPartHeader + 1, pNameBlobContent, allocatedSize) == 0);

  free(pNameBlobContent);

  // Remove private data blob
  pBuilder.Release();
  VERIFY_SUCCEEDED(CreateContainerBuilder(&pBuilder));
  VERIFY_SUCCEEDED(pBuilder->Load(pNewProgram));
  VERIFY_SUCCEEDED(
      pBuilder->RemovePart(hlsl::DxilFourCC::DFCC_ShaderDebugName));
  pResult.Release();
  VERIFY_SUCCEEDED(pBuilder->SerializeContainer(&pResult));

  pNewProgram.Release();
  VERIFY_SUCCEEDED(pResult->GetResult(&pNewProgram));
  pContainerHeader = hlsl::IsDxilContainerLike(pNewProgram->GetBufferPointer(),
                                               pNewProgram->GetBufferSize());
  VERIFY_SUCCEEDED(hlsl::IsValidDxilContainer(pContainerHeader,
                                              pNewProgram->GetBufferSize()));
  pPartHeader = hlsl::GetDxilPartByType(pContainerHeader,
                                        hlsl::DxilFourCC::DFCC_ShaderDebugName);
  VERIFY_IS_NULL(pPartHeader);
}

TEST_F(CompilerTest, CompileThenTestReflectionWithProgramHeader) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcOperationResult> pOperationResult;

  const char *source = R"x(
      cbuffer cb : register(b1) {
        float foo;
      };
      [RootSignature("CBV(b1)")]
      float4 main(float a : A) : SV_Target {
        return a + foo;
      }
  )x";
  std::string included_File = "#define ZERO 0";

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(source, &pSource);

  const WCHAR *args[] = {
      L"-Zi",
  };

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, _countof(args), nullptr,
                                      0, nullptr, &pOperationResult));

  HRESULT CompileStatus = S_OK;
  VERIFY_SUCCEEDED(pOperationResult->GetStatus(&CompileStatus));
  VERIFY_SUCCEEDED(CompileStatus);

  CComPtr<IDxcResult> pResult;
  VERIFY_SUCCEEDED(pOperationResult.QueryInterface(&pResult));

  CComPtr<IDxcBlob> pPdbBlob;
  VERIFY_SUCCEEDED(
      pResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPdbBlob), nullptr));

  CComPtr<IDxcContainerReflection> pContainerReflection;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcContainerReflection,
                                               &pContainerReflection));

  VERIFY_SUCCEEDED(pContainerReflection->Load(pPdbBlob));
  UINT32 index = 0;
  VERIFY_SUCCEEDED(pContainerReflection->FindFirstPartKind(
      hlsl::DFCC_ShaderDebugInfoDXIL, &index));

  CComPtr<IDxcBlob> pDebugDxilBlob;
  VERIFY_SUCCEEDED(
      pContainerReflection->GetPartContent(index, &pDebugDxilBlob));

  CComPtr<IDxcUtils> pUtils;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcUtils, &pUtils));

  DxcBuffer buf = {};
  buf.Ptr = pDebugDxilBlob->GetBufferPointer();
  buf.Size = pDebugDxilBlob->GetBufferSize();

  CComPtr<ID3D12ShaderReflection> pReflection;
  VERIFY_SUCCEEDED(pUtils->CreateReflection(&buf, IID_PPV_ARGS(&pReflection)));

  ID3D12ShaderReflectionConstantBuffer *cb =
      pReflection->GetConstantBufferByName("cb");
  VERIFY_IS_TRUE(cb != nullptr);
}

void CompilerTest::CompileThenTestReflectionThreadSize(const char *source,
                                                       const WCHAR *target,
                                                       UINT expectedX,
                                                       UINT expectedY,
                                                       UINT expectedZ) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcOperationResult> pOperationResult;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(source, &pSource);

  const WCHAR *args[] = {
      L"-Zs",
  };

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main", target,
                                      args, _countof(args), nullptr, 0, nullptr,
                                      &pOperationResult));

  HRESULT CompileStatus = S_OK;
  VERIFY_SUCCEEDED(pOperationResult->GetStatus(&CompileStatus));
  VERIFY_SUCCEEDED(CompileStatus);

  CComPtr<IDxcBlob> pBlob;
  VERIFY_SUCCEEDED(pOperationResult->GetResult(&pBlob));

  CComPtr<IDxcUtils> pUtils;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcUtils, &pUtils));

  DxcBuffer buf = {};
  buf.Ptr = pBlob->GetBufferPointer();
  buf.Size = pBlob->GetBufferSize();

  CComPtr<ID3D12ShaderReflection> pReflection;
  VERIFY_SUCCEEDED(pUtils->CreateReflection(&buf, IID_PPV_ARGS(&pReflection)));

  UINT x = 0, y = 0, z = 0;
  VERIFY_SUCCEEDED(pReflection->GetThreadGroupSize(&x, &y, &z));
  VERIFY_ARE_EQUAL(x, expectedX);
  VERIFY_ARE_EQUAL(y, expectedY);
  VERIFY_ARE_EQUAL(z, expectedZ);
}

TEST_F(CompilerTest, CompileThenTestReflectionThreadSizeCS) {
  const char *source = R"x(
    [numthreads(2, 3, 1)]
    void main()
    {
    }
  )x";

  CompileThenTestReflectionThreadSize(source, L"cs_6_5", 2, 3, 1);
}

TEST_F(CompilerTest, CompileThenTestReflectionThreadSizeAS) {
  const char *source = R"x(
    struct Payload {
        float2 dummy;
        float4 pos;
        float color[2];
    };

    [numthreads(2, 3, 1)]
    void main()
    {
        Payload pld;
        pld.dummy = float2(1.0,2.0);
        pld.pos = float4(3.0,4.0,5.0,6.0);
        pld.color[0] = 7.0;
        pld.color[1] = 8.0;
        DispatchMesh(2, 3, 1, pld);
    }
  )x";

  CompileThenTestReflectionThreadSize(source, L"as_6_5", 2, 3, 1);
}

TEST_F(CompilerTest, CompileThenTestReflectionThreadSizeMS) {
  const char *source = R"x(
    [NumThreads(2,3,1)]
    [OutputTopology("triangle")]
    void main() {
      int x = 2;
    }
  )x";
  CompileThenTestReflectionThreadSize(source, L"ms_6_5", 2, 3, 1);
}

static void VerifyPdbUtil(
    dxc::DxcDllSupport &dllSupport, IDxcBlob *pBlob, IDxcPdbUtils *pPdbUtils,
    const WCHAR *pMainFileName,
    llvm::ArrayRef<std::pair<const WCHAR *, const WCHAR *>> ExpectedArgs,
    llvm::ArrayRef<std::pair<const WCHAR *, const WCHAR *>> ExpectedFlags,
    llvm::ArrayRef<const WCHAR *> ExpectedDefines, IDxcCompiler *pCompiler,
    bool HasVersion, bool IsFullPDB, bool HasHashAndPdbName,
    bool TestReflection, bool TestEntryPoint, const std::string &MainSource,
    const std::string &IncludedFile) {

  std::wstring MainFileName = std::wstring(L"." SLASH_W) + pMainFileName;

  VERIFY_SUCCEEDED(pPdbUtils->Load(pBlob));

  // Compiler version comparison
  if (!HasVersion) {
    CComPtr<IDxcVersionInfo> pVersion;
    VERIFY_FAILED(pPdbUtils->GetVersionInfo(&pVersion));
  } else {
    CComPtr<IDxcVersionInfo> pVersion;
    VERIFY_SUCCEEDED(pPdbUtils->GetVersionInfo(&pVersion));

    CComPtr<IDxcVersionInfo2> pVersion2;
    VERIFY_IS_NOT_NULL(pVersion);
    VERIFY_SUCCEEDED(pVersion.QueryInterface(&pVersion2));

    CComPtr<IDxcVersionInfo3> pVersion3;
    VERIFY_SUCCEEDED(pVersion.QueryInterface(&pVersion3));

    CComPtr<IDxcVersionInfo> pCompilerVersion;
    pCompiler->QueryInterface(&pCompilerVersion);

    if (pCompilerVersion) {
      UINT32 uCompilerMajor = 0;
      UINT32 uCompilerMinor = 0;
      UINT32 uCompilerFlags = 0;
      VERIFY_SUCCEEDED(
          pCompilerVersion->GetVersion(&uCompilerMajor, &uCompilerMinor));
      VERIFY_SUCCEEDED(pCompilerVersion->GetFlags(&uCompilerFlags));

      UINT32 uMajor = 0;
      UINT32 uMinor = 0;
      UINT32 uFlags = 0;
      VERIFY_SUCCEEDED(pVersion->GetVersion(&uMajor, &uMinor));
      VERIFY_SUCCEEDED(pVersion->GetFlags(&uFlags));

      VERIFY_ARE_EQUAL(uMajor, uCompilerMajor);
      VERIFY_ARE_EQUAL(uMinor, uCompilerMinor);
      VERIFY_ARE_EQUAL(uFlags, uCompilerFlags);

      // IDxcVersionInfo2
      UINT32 uCommitCount = 0;
      CComHeapPtr<char> CommitVersionHash;
      VERIFY_SUCCEEDED(
          pVersion2->GetCommitInfo(&uCommitCount, &CommitVersionHash));

      CComPtr<IDxcVersionInfo2> pCompilerVersion2;
      if (SUCCEEDED(pCompiler->QueryInterface(&pCompilerVersion2))) {
        UINT32 uCompilerCommitCount = 0;
        CComHeapPtr<char> CompilerCommitVersionHash;
        VERIFY_SUCCEEDED(pCompilerVersion2->GetCommitInfo(
            &uCompilerCommitCount, &CompilerCommitVersionHash));

        VERIFY_IS_TRUE(0 ==
                       strcmp(CommitVersionHash, CompilerCommitVersionHash));
        VERIFY_ARE_EQUAL(uCommitCount, uCompilerCommitCount);
      }

      // IDxcVersionInfo3
      CComHeapPtr<char> VersionString;
      VERIFY_SUCCEEDED(pVersion3->GetCustomVersionString(&VersionString));
      VERIFY_IS_TRUE(VersionString && strlen(VersionString) != 0);

      {
        CComPtr<IDxcVersionInfo3> pCompilerVersion3;
        VERIFY_SUCCEEDED(pCompiler->QueryInterface(&pCompilerVersion3));
        CComHeapPtr<char> CompilerVersionString;
        VERIFY_SUCCEEDED(
            pCompilerVersion3->GetCustomVersionString(&CompilerVersionString));
        VERIFY_IS_TRUE(0 == strcmp(CompilerVersionString, VersionString));
      }
    }
  }

  // Target profile
  {
    CComBSTR str;
    VERIFY_SUCCEEDED(pPdbUtils->GetTargetProfile(&str));
    VERIFY_ARE_EQUAL_WSTR(L"ps_6_0", str.m_str);
  }

  // Entry point
  {
    CComBSTR str;
    VERIFY_SUCCEEDED(pPdbUtils->GetEntryPoint(&str));
    if (TestEntryPoint) {
      VERIFY_ARE_EQUAL_WSTR(L"main", str.m_str);
    } else {
      VERIFY_ARE_EQUAL_WSTR(L"PSMain", str.m_str);
    }
  }

  // PDB file path
  if (HasHashAndPdbName) {
    CComBSTR pName;
    VERIFY_SUCCEEDED(pPdbUtils->GetName(&pName));
    std::wstring suffix = L".pdb";
    VERIFY_IS_TRUE(pName.Length() >= suffix.size());
    VERIFY_IS_TRUE(0 == std::memcmp(suffix.c_str(),
                                    &pName[pName.Length() - suffix.size()],
                                    suffix.size()));
  }

  // Main file name
  {
    CComBSTR pPdbMainFileName;
    VERIFY_SUCCEEDED(pPdbUtils->GetMainFileName(&pPdbMainFileName));
    VERIFY_ARE_EQUAL(MainFileName, pPdbMainFileName.m_str);
  }

  // There is hash and hash is not empty
  if (HasHashAndPdbName) {
    CComPtr<IDxcBlob> pHash;
    VERIFY_SUCCEEDED(pPdbUtils->GetHash(&pHash));
    hlsl::DxilShaderHash EmptyHash = {};
    VERIFY_ARE_EQUAL(pHash->GetBufferSize(), sizeof(EmptyHash));
    VERIFY_IS_FALSE(0 == std::memcmp(pHash->GetBufferPointer(), &EmptyHash,
                                     sizeof(EmptyHash)));
  }

  // Source files
  {
    UINT32 uSourceCount = 0;
    VERIFY_SUCCEEDED(pPdbUtils->GetSourceCount(&uSourceCount));
    for (UINT32 i = 0; i < uSourceCount; i++) {
      CComBSTR pFileName;
      CComPtr<IDxcBlobEncoding> pFileContent;
      VERIFY_SUCCEEDED(pPdbUtils->GetSourceName(i, &pFileName));
      VERIFY_SUCCEEDED(pPdbUtils->GetSource(i, &pFileContent));

      CComPtr<IDxcBlobUtf8> pFileContentUtf8;
      VERIFY_SUCCEEDED(pFileContent.QueryInterface(&pFileContentUtf8));
      llvm::StringRef FileContentRef(pFileContentUtf8->GetStringPointer(),
                                     pFileContentUtf8->GetStringLength());

      if (MainFileName == pFileName.m_str) {
        VERIFY_ARE_EQUAL(FileContentRef, MainSource);
      } else {
        VERIFY_ARE_EQUAL(FileContentRef, IncludedFile);
      }
    }
  }

  // Defines
  {
    UINT32 uDefineCount = 0;
    std::map<std::wstring, int> tally;
    VERIFY_SUCCEEDED(pPdbUtils->GetDefineCount(&uDefineCount));
    VERIFY_IS_TRUE(uDefineCount == 2);
    for (UINT32 i = 0; i < uDefineCount; i++) {
      CComBSTR def;
      VERIFY_SUCCEEDED(pPdbUtils->GetDefine(i, &def));
      tally[std::wstring(def)]++;
    }
    auto Expected = ExpectedDefines;
    for (size_t i = 0; i < Expected.size(); i++) {
      auto it = tally.find(Expected[i]);
      VERIFY_IS_TRUE(it != tally.end() && it->second == 1);
      tally.erase(it);
    }
    VERIFY_IS_TRUE(tally.size() == 0);
  }

  // Arg pairs
  {
    std::vector<std::pair<std::wstring, std::wstring>> ArgPairs;
    UINT32 uCount = 0;
    VERIFY_SUCCEEDED(pPdbUtils->GetArgPairCount(&uCount));
    for (unsigned i = 0; i < uCount; i++) {
      CComBSTR pName;
      CComBSTR pValue;
      VERIFY_SUCCEEDED(pPdbUtils->GetArgPair(i, &pName, &pValue));

      VERIFY_IS_TRUE(pName || pValue);

      std::pair<std::wstring, std::wstring> NewPair;
      if (pName)
        NewPair.first = std::wstring(pName);
      if (pValue)
        NewPair.second = std::wstring(pValue);
      ArgPairs.push_back(std::move(NewPair));
    }

    for (size_t i = 0; i < ExpectedArgs.size(); i++) {
      auto ExpectedPair = ExpectedArgs[i];
      bool Found = false;
      for (size_t j = 0; j < ArgPairs.size(); j++) {
        auto Pair = ArgPairs[j];
        if ((!ExpectedPair.first || Pair.first == ExpectedPair.first) &&
            (!ExpectedPair.second || Pair.second == ExpectedPair.second)) {
          Found = true;
          break;
        }
      }
      VERIFY_SUCCEEDED(Found);
    }
  }

  auto TestArgumentPair =
      [](llvm::ArrayRef<std::wstring> Args,
         llvm::ArrayRef<std::pair<const WCHAR *, const WCHAR *>> Expected) {
        for (size_t i = 0; i < Expected.size(); i++) {
          auto Pair = Expected[i];
          bool found = false;
          for (size_t j = 0; j < Args.size(); j++) {
            if (!Pair.second && Args[j] == Pair.first) {
              found = true;
              break;
            } else if (!Pair.first && Args[j] == Pair.second) {
              found = true;
              break;
            } else if (Pair.first && Pair.second && Args[j] == Pair.first &&
                       j + 1 < Args.size() && Args[j + 1] == Pair.second) {
              found = true;
              break;
            }
          }

          VERIFY_IS_TRUE(found);
        }
      };

  // Flags
  {
    UINT32 uCount = 0;
    std::vector<std::wstring> Flags;
    VERIFY_SUCCEEDED(pPdbUtils->GetFlagCount(&uCount));
    VERIFY_IS_TRUE(uCount == ExpectedFlags.size());
    for (UINT32 i = 0; i < uCount; i++) {
      CComBSTR item;
      VERIFY_SUCCEEDED(pPdbUtils->GetFlag(i, &item));
      Flags.push_back(std::wstring(item));
    }

    TestArgumentPair(Flags, ExpectedFlags);
  }

  // Args
  {
    UINT32 uCount = 0;
    std::vector<std::wstring> Args;
    VERIFY_SUCCEEDED(pPdbUtils->GetArgCount(&uCount));
    for (UINT32 i = 0; i < uCount; i++) {
      CComBSTR item;
      VERIFY_SUCCEEDED(pPdbUtils->GetArg(i, &item));
      Args.push_back(std::wstring(item));
    }

    TestArgumentPair(Args, ExpectedArgs);
  }

  // Shader reflection
  if (TestReflection) {
    CComPtr<IDxcUtils> pUtils;
    VERIFY_SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcUtils, &pUtils));

    DxcBuffer buf = {};
    buf.Ptr = pBlob->GetBufferPointer();
    buf.Size = pBlob->GetBufferSize();
    buf.Encoding = CP_ACP;

    CComPtr<ID3D12ShaderReflection> pRefl;
    VERIFY_SUCCEEDED(pUtils->CreateReflection(&buf, IID_PPV_ARGS(&pRefl)));

    D3D12_SHADER_DESC desc = {};
    VERIFY_SUCCEEDED(pRefl->GetDesc(&desc));

    VERIFY_ARE_EQUAL(desc.ConstantBuffers, 1u);
    ID3D12ShaderReflectionConstantBuffer *pCB =
        pRefl->GetConstantBufferByIndex(0);

    D3D12_SHADER_BUFFER_DESC cbDesc = {};
    VERIFY_SUCCEEDED(pCB->GetDesc(&cbDesc));

    VERIFY_IS_TRUE(0 == strcmp(cbDesc.Name, "MyCbuffer"));
    VERIFY_ARE_EQUAL(cbDesc.Variables, 1u);

    ID3D12ShaderReflectionVariable *pVar = pCB->GetVariableByIndex(0);
    D3D12_SHADER_VARIABLE_DESC varDesc = {};
    VERIFY_SUCCEEDED(pVar->GetDesc(&varDesc));

    VERIFY_ARE_EQUAL(varDesc.uFlags, D3D_SVF_USED);
    VERIFY_IS_TRUE(0 == strcmp(varDesc.Name, "my_cbuf_foo"));
    VERIFY_ARE_EQUAL(varDesc.Size, sizeof(float) * 4);
  }

  // Make the pix debug info
  if (IsFullPDB) {
    VERIFY_IS_TRUE(pPdbUtils->IsFullPDB());
  } else {
    VERIFY_IS_FALSE(pPdbUtils->IsFullPDB());
  }

// Limit dia tests to Windows.
#ifdef _WIN32
  // Now, test that dia interface doesn't crash (even if it fails).
  {
    CComPtr<IDiaDataSource> pDataSource;
    VERIFY_SUCCEEDED(
        dllSupport.CreateInstance(CLSID_DxcDiaDataSource, &pDataSource));

    CComPtr<IDxcLibrary> pLib;
    VERIFY_SUCCEEDED(dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));

    CComPtr<IStream> pStream;
    VERIFY_SUCCEEDED(pLib->CreateStreamFromBlobReadOnly(pBlob, &pStream));
    if (SUCCEEDED(pDataSource->loadDataFromIStream(pStream))) {
      CComPtr<IDiaSession> pSession;
      if (SUCCEEDED(pDataSource->openSession(&pSession))) {
        CComPtr<IDxcPixDxilDebugInfoFactory> pFactory;
        VERIFY_SUCCEEDED(pSession->QueryInterface(&pFactory));

        CComPtr<IDxcPixCompilationInfo> pCompilationInfo;
        if (SUCCEEDED(pFactory->NewDxcPixCompilationInfo(&pCompilationInfo))) {
          CComBSTR args;
          CComBSTR defs;
          CComBSTR mainName;
          CComBSTR entryPoint;
          CComBSTR entryPointFile;
          CComBSTR target;
          pCompilationInfo->GetArguments(&args);
          pCompilationInfo->GetMacroDefinitions(&defs);
          pCompilationInfo->GetEntryPoint(&entryPoint);
          pCompilationInfo->GetEntryPointFile(&entryPointFile);
          pCompilationInfo->GetHlslTarget(&target);
          for (DWORD i = 0;; i++) {
            CComBSTR sourceName;
            CComBSTR sourceContent;
            if (FAILED(pCompilationInfo->GetSourceFile(i, &sourceName,
                                                       &sourceContent)))
              break;
          }
        }

        CComPtr<IDxcPixDxilDebugInfo> pDebugInfo;
        pFactory->NewDxcPixDxilDebugInfo(&pDebugInfo);
      }
    }
  }
#endif
}

TEST_F(CompilerTest, CompileThenTestPdbUtilsStripped) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;
  CComPtr<TestIncludeHandler> pInclude;
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcOperationResult> pOperationResult;

  std::string main_source = "#include \"helper.h\"\r\n"
                            "float4 PSMain() : SV_Target { return ZERO; }";
  std::string included_File = "#define ZERO 0";

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(main_source.c_str(), &pSource);

  pInclude = new TestIncludeHandler(m_dllSupport);
  pInclude->CallResults.emplace_back(included_File.c_str());

  const WCHAR *pArgs[] = {L"/Zi", L"/Od", L"-flegacy-macro-expansion",
                          L"-Qstrip_debug", L"/DTHIS_IS_A_DEFINE=HELLO"};
  const DxcDefine pDefines[] = {{L"THIS_IS_ANOTHER_DEFINE", L"1"}};

  VERIFY_SUCCEEDED(pCompiler->Compile(
      pSource, L"source.hlsl", L"PSMain", L"ps_6_0", pArgs, _countof(pArgs),
      pDefines, _countof(pDefines), pInclude, &pOperationResult));

  CComPtr<IDxcBlob> pCompiledBlob;
  VERIFY_SUCCEEDED(pOperationResult->GetResult(&pCompiledBlob));

  CComPtr<IDxcPdbUtils> pPdbUtils;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcPdbUtils, &pPdbUtils));

  VERIFY_SUCCEEDED(pPdbUtils->Load(pCompiledBlob));

  // PDB file path
  {
    CComBSTR pName;
    VERIFY_SUCCEEDED(pPdbUtils->GetName(&pName));
    std::wstring suffix = L".pdb";
    VERIFY_IS_TRUE(pName.Length() >= suffix.size());
    VERIFY_IS_TRUE(0 == std::memcmp(suffix.c_str(),
                                    &pName[pName.Length() - suffix.size()],
                                    suffix.size()));
  }

  // There is hash and hash is not empty
  {
    CComPtr<IDxcBlob> pHash;
    VERIFY_SUCCEEDED(pPdbUtils->GetHash(&pHash));
    hlsl::DxilShaderHash EmptyHash = {};
    VERIFY_ARE_EQUAL(pHash->GetBufferSize(), sizeof(EmptyHash));
    VERIFY_IS_FALSE(0 == std::memcmp(pHash->GetBufferPointer(), &EmptyHash,
                                     sizeof(EmptyHash)));
  }

  {
    VERIFY_IS_FALSE(pPdbUtils->IsFullPDB());
    UINT32 uSourceCount = 0;
    VERIFY_SUCCEEDED(pPdbUtils->GetSourceCount(&uSourceCount));
    VERIFY_ARE_EQUAL(uSourceCount, 0u);
  }
}

void CompilerTest::TestPdbUtils(bool bSlim, bool bSourceInDebugModule,
                                bool bStrip, bool bTestEntryPoint) {
  CComPtr<TestIncludeHandler> pInclude;
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcOperationResult> pOperationResult;

  std::string entryPointName = bTestEntryPoint ? "main" : "PSMain";
  std::wstring entryPointNameWide = bTestEntryPoint ? L"" : L"PSMain";

  std::string main_source = R"x(
      #include "helper.h"
      cbuffer MyCbuffer : register(b1) {
        float4 my_cbuf_foo;
      }

      [RootSignature("CBV(b1)")]
      float4 )x" + entryPointName +
                            R"x(() : SV_Target {
        return ZERO + my_cbuf_foo;
      }
  )x";
  std::string included_File = "#define ZERO 0";

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(main_source.c_str(), &pSource);

  pInclude = new TestIncludeHandler(m_dllSupport);
  pInclude->CallResults.emplace_back(included_File.c_str());

  std::vector<const WCHAR *> args;
  std::vector<std::pair<const WCHAR *, const WCHAR *>> expectedArgs;
  std::vector<std::pair<const WCHAR *, const WCHAR *>> expectedFlags;
  std::vector<const WCHAR *> expectedDefines;

  auto AddArg = [&args, &expectedFlags, &expectedArgs](
                    const WCHAR *arg, const WCHAR *value, bool isDefine) {
    args.push_back(arg);
    if (value)
      args.push_back(value);

    std::pair<const WCHAR *, const WCHAR *> pair(arg, value);
    expectedArgs.push_back(pair);

    if (!isDefine) {
      expectedFlags.push_back(pair);
    }
  };

  AddArg(L"-Od", nullptr, false);
  AddArg(L"-flegacy-macro-expansion", nullptr, false);

  if (bStrip) {
    AddArg(L"-Qstrip_debug", nullptr, false);
  } else {
    AddArg(L"-Qembed_debug", nullptr, false);
  }

  if (bSourceInDebugModule) {
    AddArg(L"-Qsource_in_debug_module", nullptr, false);
  }
  if (bSlim) {
    AddArg(L"-Zs", nullptr, false);
  } else {
    AddArg(L"-Zi", nullptr, false);
  }

  AddArg(L"-D", L"THIS_IS_A_DEFINE=HELLO", true);

  const DxcDefine pDefines[] = {{L"THIS_IS_ANOTHER_DEFINE", L"1"}};
  expectedDefines.push_back(L"THIS_IS_ANOTHER_DEFINE=1");
  expectedDefines.push_back(L"THIS_IS_A_DEFINE=HELLO");

  VERIFY_SUCCEEDED(
      pCompiler->Compile(pSource, L"source.hlsl", entryPointNameWide.data(),
                         L"ps_6_0", args.data(), args.size(), pDefines,
                         _countof(pDefines), pInclude, &pOperationResult));

  HRESULT CompileStatus = S_OK;
  VERIFY_SUCCEEDED(pOperationResult->GetStatus(&CompileStatus));
  VERIFY_SUCCEEDED(CompileStatus);

  CComPtr<IDxcBlob> pCompiledBlob;
  VERIFY_SUCCEEDED(pOperationResult->GetResult(&pCompiledBlob));

  CComPtr<IDxcResult> pResult;
  VERIFY_SUCCEEDED(pOperationResult.QueryInterface(&pResult));

  CComPtr<IDxcBlob> pPdbBlob;
  VERIFY_SUCCEEDED(
      pResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPdbBlob), nullptr));

  CComPtr<IDxcPdbUtils> pPdbUtils;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcPdbUtils, &pPdbUtils));

  CComPtr<IDxcBlob> pProgramHeaderBlob;
  if (bSourceInDebugModule) {
    CComPtr<IDxcContainerReflection> pRef;
    VERIFY_SUCCEEDED(
        m_dllSupport.CreateInstance(CLSID_DxcContainerReflection, &pRef));
    VERIFY_SUCCEEDED(pRef->Load(pPdbBlob));
    UINT32 uIndex = 0;
    VERIFY_SUCCEEDED(
        pRef->FindFirstPartKind(hlsl::DFCC_ShaderDebugInfoDXIL, &uIndex));
    VERIFY_SUCCEEDED(pRef->GetPartContent(uIndex, &pProgramHeaderBlob));

    VerifyPdbUtil(
        m_dllSupport, pProgramHeaderBlob, pPdbUtils, L"source.hlsl",
        expectedArgs, expectedFlags, expectedDefines, pCompiler,
        /*HasVersion*/ false,
        /*IsFullPDB*/ true,
        /*HasHashAndPdbName*/ false,
        /*TestReflection*/ false, // Reflection creation interface doesn't
                                  // support just the DxilProgramHeader.
        /*TestEntryPoint*/ bTestEntryPoint, main_source, included_File);
  }

  VerifyPdbUtil(m_dllSupport, pPdbBlob, pPdbUtils, L"source.hlsl", expectedArgs,
                expectedFlags, expectedDefines, pCompiler,
                /*HasVersion*/ true,
                /*IsFullPDB*/ !bSlim,
                /*HasHashAndPdbName*/ true,
                /*TestReflection*/ true,
                /*TestEntryPoint*/ bTestEntryPoint, main_source, included_File);

  if (!bStrip) {
    VerifyPdbUtil(m_dllSupport, pCompiledBlob, pPdbUtils, L"source.hlsl",
                  expectedArgs, expectedFlags, expectedDefines, pCompiler,
                  /*HasVersion*/ false,
                  /*IsFullPDB*/ true,
                  /*HasHashAndPdbName*/ true,
                  /*TestReflection*/ true,
                  /*TestEntryPoint*/ bTestEntryPoint, main_source,
                  included_File);
  }

  {
    CComPtr<IDxcPdbUtils2> pPdbUtils2;
    VERIFY_SUCCEEDED(pPdbUtils.QueryInterface(&pPdbUtils2));
    {
      CComPtr<IDxcPdbUtils> pPdbUtils_Again;
      VERIFY_SUCCEEDED(pPdbUtils2.QueryInterface(&pPdbUtils_Again));
      {
        CComPtr<IDxcPdbUtils2> pPdbUtils2_Again;
        VERIFY_SUCCEEDED(pPdbUtils_Again.QueryInterface(&pPdbUtils2_Again));
        VERIFY_ARE_EQUAL(pPdbUtils2_Again, pPdbUtils2);

        VERIFY_ARE_EQUAL(pPdbUtils2.p->AddRef(), 5u);
        VERIFY_ARE_EQUAL(pPdbUtils2.p->Release(), 4u);
      }
      VERIFY_ARE_EQUAL(pPdbUtils_Again, pPdbUtils);

      VERIFY_ARE_EQUAL(pPdbUtils2.p->AddRef(), 4u);
      VERIFY_ARE_EQUAL(pPdbUtils2.p->Release(), 3u);
    }

    VERIFY_ARE_EQUAL(pPdbUtils2.p->AddRef(), 3u);
    VERIFY_ARE_EQUAL(pPdbUtils2.p->Release(), 2u);
  }

  VERIFY_ARE_EQUAL(pPdbUtils.p->AddRef(), 2u);
  VERIFY_ARE_EQUAL(pPdbUtils.p->Release(), 1u);
}

TEST_F(CompilerTest, CompileThenTestPdbUtils) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;
  TestPdbUtils(/*bSlim*/ true, /*bSourceInDebugModule*/ false, /*strip*/ true,
               /*bTestEntryPoint*/ false); // Slim PDB, where source info is
                                           // stored in its own part, and debug
                                           // module is NOT present

  TestPdbUtils(/*bSlim*/ false, /*bSourceInDebugModule*/ true, /*strip*/ false,
               /*bTestEntryPoint*/ false); // Old PDB format, where source info
                                           // is embedded in the module
  TestPdbUtils(/*bSlim*/ false, /*bSourceInDebugModule*/ false, /*strip*/ false,
               /*bTestEntryPoint*/ false); // Full PDB, where source info is
                                           // stored in its own part, and a
                                           // debug module which is present

  TestPdbUtils(/*bSlim*/ false, /*bSourceInDebugModule*/ true, /*strip*/ true,
               /*bTestEntryPoint*/ false); // Legacy PDB, where source info is
                                           // embedded in the module
  TestPdbUtils(/*bSlim*/ false, /*bSourceInDebugModule*/ true, /*strip*/ true,
               /*bTestEntryPoint*/ true); // Same as above, except this time we
                                          // test the default entry point.
  TestPdbUtils(
      /*bSlim*/ false, /*bSourceInDebugModule*/ false, /*strip*/ true,
      /*bTestEntryPoint*/ false); // Full PDB, where source info is stored in
                                  // its own part, and debug module is present
}

TEST_F(CompilerTest, CompileThenTestPdbUtilsWarningOpt) {
  CComPtr<IDxcCompiler> pCompiler;
  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));

  std::string main_source = R"x(
      cbuffer MyCbuffer : register(b1) {
        float4 my_cbuf_foo;
      }

      [RootSignature("CBV(b1)")]
      float4 main() : SV_Target {
        return my_cbuf_foo;
      }
  )x";

  CComPtr<IDxcUtils> pUtils;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcUtils, &pUtils));

  CComPtr<IDxcCompiler3> pCompiler3;
  VERIFY_SUCCEEDED(pCompiler.QueryInterface(&pCompiler3));

  const WCHAR *args[] = {
      L"/Zs",       L".\redundant_input", L"-Wno-parentheses-equality",
      L"hlsl.hlsl", L"/Tps_6_0",          L"/Emain",
  };

  DxcBuffer buf = {};
  buf.Ptr = main_source.c_str();
  buf.Size = main_source.size();
  buf.Encoding = CP_UTF8;

  CComPtr<IDxcResult> pResult;
  VERIFY_SUCCEEDED(pCompiler3->Compile(&buf, args, _countof(args), nullptr,
                                       IID_PPV_ARGS(&pResult)));

  CComPtr<IDxcBlob> pPdb;
  VERIFY_SUCCEEDED(
      pResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPdb), nullptr));

  auto TestPdb = [](IDxcPdbUtils *pPdbUtils) {
    UINT32 uArgsCount = 0;
    VERIFY_SUCCEEDED(pPdbUtils->GetArgCount(&uArgsCount));
    bool foundArg = false;
    for (UINT32 i = 0; i < uArgsCount; i++) {
      CComBSTR pArg;
      VERIFY_SUCCEEDED(pPdbUtils->GetArg(i, &pArg));
      if (pArg) {
        std::wstring arg(pArg);
        if (arg == L"-Wno-parentheses-equality" ||
            arg == L"/Wno-parentheses-equality") {
          foundArg = true;
        } else {
          // Make sure arg value "no-parentheses-equality" doesn't show up
          // as its own argument token.
          VERIFY_ARE_NOT_EQUAL(arg, L"no-parentheses-equality");

          // Make sure the presence of the argument ".\redundant_input"
          // doesn't cause "<input>" to show up.
          VERIFY_ARE_NOT_EQUAL(arg, L"<input>");
        }
      }
    }
    VERIFY_IS_TRUE(foundArg);

    UINT32 uFlagsCount = 0;
    VERIFY_SUCCEEDED(pPdbUtils->GetFlagCount(&uFlagsCount));
    bool foundFlag = false;
    for (UINT32 i = 0; i < uFlagsCount; i++) {
      CComBSTR pFlag;
      VERIFY_SUCCEEDED(pPdbUtils->GetFlag(i, &pFlag));
      if (pFlag) {
        std::wstring arg(pFlag);
        if (arg == L"-Wno-parentheses-equality" ||
            arg == L"/Wno-parentheses-equality") {
          foundFlag = true;
        } else {
          // Make sure arg value "no-parentheses-equality" doesn't show up
          // as its own flag token.
          VERIFY_ARE_NOT_EQUAL(arg, L"no-parentheses-equality");
        }
      }
    }
    VERIFY_IS_TRUE(foundFlag);

    CComBSTR pMainFileName;
    VERIFY_SUCCEEDED(pPdbUtils->GetMainFileName(&pMainFileName));
    std::wstring mainFileName = static_cast<const wchar_t *>(pMainFileName);
    VERIFY_ARE_EQUAL(mainFileName, L"." SLASH_W L"hlsl.hlsl");
  };

  CComPtr<IDxcPdbUtils> pPdbUtils;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcPdbUtils, &pPdbUtils));

  VERIFY_SUCCEEDED(pPdbUtils->Load(pPdb));
  TestPdb(pPdbUtils);
}

TEST_F(CompilerTest, CompileThenTestPdbInPrivate) {
  CComPtr<IDxcCompiler> pCompiler;
  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));

  std::string main_source = R"x(
      cbuffer MyCbuffer : register(b1) {
        float4 my_cbuf_foo;
      }

      [RootSignature("CBV(b1)")]
      float4 main() : SV_Target {
        return my_cbuf_foo;
      }
  )x";

  CComPtr<IDxcUtils> pUtils;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcUtils, &pUtils));

  CComPtr<IDxcBlobEncoding> pSource;
  VERIFY_SUCCEEDED(pUtils->CreateBlobFromPinned(
      main_source.c_str(), main_source.size(), CP_UTF8, &pSource));

  const WCHAR *args[] = {
      L"/Zs",
      L"/Qpdb_in_private",
  };

  CComPtr<IDxcOperationResult> pOpResult;
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"hlsl.hlsl", L"main", L"ps_6_0",
                                      args, _countof(args), nullptr, 0, nullptr,
                                      &pOpResult));

  CComPtr<IDxcResult> pResult;
  VERIFY_SUCCEEDED(pOpResult.QueryInterface(&pResult));

  CComPtr<IDxcBlob> pShader;
  VERIFY_SUCCEEDED(
      pResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), nullptr));

  CComPtr<IDxcContainerReflection> pRefl;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcContainerReflection, &pRefl));
  VERIFY_SUCCEEDED(pRefl->Load(pShader));

  UINT32 uIndex = 0;
  VERIFY_SUCCEEDED(pRefl->FindFirstPartKind(hlsl::DFCC_PrivateData, &uIndex));

  CComPtr<IDxcBlob> pPdbBlob;
  VERIFY_SUCCEEDED(
      pResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPdbBlob), nullptr));

  CComPtr<IDxcBlob> pPrivatePdbBlob;
  VERIFY_SUCCEEDED(pRefl->GetPartContent(uIndex, &pPrivatePdbBlob));

  VERIFY_ARE_EQUAL(pPdbBlob->GetBufferSize(), pPrivatePdbBlob->GetBufferSize());
  VERIFY_ARE_EQUAL(0, memcmp(pPdbBlob->GetBufferPointer(),
                             pPrivatePdbBlob->GetBufferPointer(),
                             pPdbBlob->GetBufferSize()));
}

TEST_F(CompilerTest, CompileThenTestPdbUtilsRelativePath) {
  std::string main_source = R"x(
      #include "helper.h"
      cbuffer MyCbuffer : register(b1) {
        float4 my_cbuf_foo;
      }

      [RootSignature("CBV(b1)")]
      float4 main() : SV_Target {
        return my_cbuf_foo;
      }
  )x";

  CComPtr<IDxcCompiler3> pCompiler;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));

  DxcBuffer SourceBuf = {};
  SourceBuf.Ptr = main_source.c_str();
  SourceBuf.Size = main_source.size();
  SourceBuf.Encoding = CP_UTF8;

  std::vector<const WCHAR *> args;
  args.push_back(L"/Tps_6_0");
  args.push_back(L"/Zs");
  args.push_back(L"shaders/Shader.hlsl");

  CComPtr<TestIncludeHandler> pInclude;
  std::string included_File = "#define ZERO 0";
  pInclude = new TestIncludeHandler(m_dllSupport);
  pInclude->CallResults.emplace_back(included_File.c_str());

  CComPtr<IDxcResult> pResult;
  VERIFY_SUCCEEDED(pCompiler->Compile(&SourceBuf, args.data(), args.size(),
                                      pInclude, IID_PPV_ARGS(&pResult)));

  CComPtr<IDxcBlob> pPdb;
  CComPtr<IDxcBlobWide> pPdbName;
  VERIFY_SUCCEEDED(
      pResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPdb), &pPdbName));

  CComPtr<IDxcPdbUtils> pPdbUtils;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcPdbUtils, &pPdbUtils));

  VERIFY_SUCCEEDED(pPdbUtils->Load(pPdb));
}

TEST_F(CompilerTest, CompileSameFilenameAndEntryThenTestPdbUtilsArgs) {
  // This is a regression test for a bug where if entry point has the same
  // value as the input filename, the entry point gets omitted from the arg
  // list in debug module and PDB, making them useless for recompilation.
  CComPtr<IDxcCompiler> pCompiler;
  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));

  std::string shader = R"x(
    [RootSignature("")] float PSMain() : SV_Target  {
      return 0;
    }
  )x";

  CComPtr<IDxcUtils> pUtils;
  VERIFY_SUCCEEDED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils)));

  CComPtr<IDxcOperationResult> pOpResult;

  std::wstring EntryPoint = L"PSMain";
  CComPtr<IDxcBlobEncoding> pShaderBlob;

  VERIFY_SUCCEEDED(pUtils->CreateBlob(shader.data(),
                                      shader.size() * sizeof(shader[0]),
                                      DXC_CP_UTF8, &pShaderBlob));

  const WCHAR *OtherInputs[] = {
      L"AnotherInput1",
      L"AnotherInput2",
      L"AnotherInput3",
      L"AnotherInput4",
  };

  const WCHAR *Args[] = {
      OtherInputs[0], OtherInputs[1], L"-Od",
      OtherInputs[2], L"-Zi",         OtherInputs[3],
  };
  VERIFY_SUCCEEDED(pCompiler->Compile(
      pShaderBlob, EntryPoint.c_str(), EntryPoint.c_str(), L"ps_6_0", Args,
      _countof(Args), nullptr, 0, nullptr, &pOpResult));

  HRESULT compileStatus = S_OK;
  VERIFY_SUCCEEDED(pOpResult->GetStatus(&compileStatus));
  VERIFY_SUCCEEDED(compileStatus);

  CComPtr<IDxcBlob> pDxil;
  VERIFY_SUCCEEDED(pOpResult->GetResult(&pDxil));
  CComPtr<IDxcResult> pResult;
  VERIFY_SUCCEEDED(pOpResult.QueryInterface(&pResult));
  CComPtr<IDxcBlob> pPdb;
  VERIFY_SUCCEEDED(
      pResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPdb), nullptr));

  IDxcBlob *PdbLikes[] = {
      pDxil,
      pPdb,
  };

  for (IDxcBlob *pPdbLike : PdbLikes) {
    CComPtr<IDxcPdbUtils2> pPdbUtils;
    VERIFY_SUCCEEDED(
        DxcCreateInstance(CLSID_DxcPdbUtils, IID_PPV_ARGS(&pPdbUtils)));
    VERIFY_SUCCEEDED(pPdbUtils->Load(pPdbLike));

    CComPtr<IDxcBlobWide> pEntryPoint;
    VERIFY_SUCCEEDED(pPdbUtils->GetEntryPoint(&pEntryPoint));
    VERIFY_IS_NOT_NULL(pEntryPoint);
    VERIFY_ARE_EQUAL(std::wstring(pEntryPoint->GetStringPointer(),
                                  pEntryPoint->GetStringLength()),
                     EntryPoint);

    std::set<std::wstring> ArgSet;
    UINT uNumArgs = 0;
    VERIFY_SUCCEEDED(pPdbUtils->GetArgCount(&uNumArgs));
    for (UINT i = 0; i < uNumArgs; i++) {
      CComPtr<IDxcBlobWide> pArg;
      VERIFY_SUCCEEDED(pPdbUtils->GetArg(i, &pArg));
      ArgSet.insert(
          std::wstring(pArg->GetStringPointer(), pArg->GetStringLength()));
    }

    for (const WCHAR *OtherInputs : OtherInputs) {
      VERIFY_ARE_EQUAL(ArgSet.end(), ArgSet.find(OtherInputs));
    }
    VERIFY_ARE_NOT_EQUAL(ArgSet.end(), ArgSet.find(L"-Od"));
    VERIFY_ARE_NOT_EQUAL(ArgSet.end(), ArgSet.find(L"-Zi"));
  }
}

TEST_F(CompilerTest, CompileThenTestPdbUtilsEmptyEntry) {
  std::string main_source = R"x(
      cbuffer MyCbuffer : register(b1) {
        float4 my_cbuf_foo;
      }

      [RootSignature("CBV(b1)")]
      float4 main() : SV_Target {
        return my_cbuf_foo;
      }
  )x";

  CComPtr<IDxcCompiler3> pCompiler;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));

  DxcBuffer SourceBuf = {};
  SourceBuf.Ptr = main_source.c_str();
  SourceBuf.Size = main_source.size();
  SourceBuf.Encoding = CP_UTF8;

  std::vector<const WCHAR *> args;
  args.push_back(L"/Tps_6_0");
  args.push_back(L"/Zi");

  CComPtr<IDxcResult> pResult;
  VERIFY_SUCCEEDED(pCompiler->Compile(&SourceBuf, args.data(), args.size(),
                                      nullptr, IID_PPV_ARGS(&pResult)));

  CComPtr<IDxcBlob> pPdb;
  CComPtr<IDxcBlobWide> pPdbName;
  VERIFY_SUCCEEDED(
      pResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPdb), &pPdbName));

  CComPtr<IDxcPdbUtils> pPdbUtils;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcPdbUtils, &pPdbUtils));

  VERIFY_SUCCEEDED(pPdbUtils->Load(pPdb));

  CComBSTR pEntryName;
  VERIFY_SUCCEEDED(pPdbUtils->GetEntryPoint(&pEntryName));

  VERIFY_ARE_EQUAL_WSTR(L"main", pEntryName.m_str);
}

TEST_F(CompilerTest, TestPdbUtilsWithEmptyDefine) {
#include "TestHeaders/TestDxilWithEmptyDefine.h"
  CComPtr<IDxcUtils> pUtils;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcUtils, &pUtils));

  CComPtr<IDxcBlobEncoding> pBlob;
  VERIFY_SUCCEEDED(pUtils->CreateBlobFromPinned(
      g_TestDxilWithEmptyDefine, sizeof(g_TestDxilWithEmptyDefine), CP_ACP,
      &pBlob));

  CComPtr<IDxcPdbUtils> pPdbUtils;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcPdbUtils, &pPdbUtils));
  VERIFY_SUCCEEDED(pPdbUtils->Load(pBlob));

  UINT32 uCount = 0;
  VERIFY_SUCCEEDED(pPdbUtils->GetDefineCount(&uCount));
  for (UINT i = 0; i < uCount; i++) {
    CComBSTR pDefine;
    VERIFY_SUCCEEDED(pPdbUtils->GetDefine(i, &pDefine));
  }
}

void CompilerTest::TestResourceBindingImpl(const char *bindingFileContent,
                                           const std::wstring &errors,
                                           bool noIncludeHandler) {

  class IncludeHandler : public IDxcIncludeHandler {
    DXC_MICROCOM_REF_FIELD(m_dwRef)
  public:
    DXC_MICROCOM_ADDREF_RELEASE_IMPL(m_dwRef)
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                             void **ppvObject) override {
      return DoBasicQueryInterface<IDxcIncludeHandler>(this, iid, ppvObject);
    }

    CComPtr<IDxcBlob> pBindingFileBlob;
    IncludeHandler() : m_dwRef(0) {}

    HRESULT STDMETHODCALLTYPE LoadSource(
        LPCWSTR pFilename,         // Filename as written in #include statement
        IDxcBlob **ppIncludeSource // Resultant source object for included file
        ) override {

      if (0 == wcscmp(pFilename, L"binding-file.txt")) {
        return pBindingFileBlob.QueryInterface(ppIncludeSource);
      }
      return E_FAIL;
    }
  };

  CComPtr<IDxcBlobEncoding> pBindingFileBlob;
  CreateBlobFromText(bindingFileContent, &pBindingFileBlob);

  CComPtr<IncludeHandler> pIncludeHandler;
  if (!noIncludeHandler) {
    pIncludeHandler = new IncludeHandler();
    pIncludeHandler->pBindingFileBlob = pBindingFileBlob;
  }

  const char *source = R"x(
    cbuffer cb {
      float a;
    };
    cbuffer resource {
      float b;
    };

    SamplerState samp0;
    Texture2D resource;
    RWTexture1D<float> uav_0;

    [RootSignature("CBV(b10,space=30), CBV(b42,space=999), DescriptorTable(Sampler(s1,space=2)), DescriptorTable(SRV(t1,space=2)), DescriptorTable(UAV(u0,space=0))")]
    float main(float2 uv : UV, uint i : I) :SV_Target {
      return a + b + resource.Sample(samp0, uv).r + uav_0[i];
    }
  )x";

  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcOperationResult> pResult;

  const WCHAR *args[] = {L"-import-binding-table", L"binding-file.txt"};

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));

  CreateBlobFromText(source, &pSource);
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, _countof(args), nullptr,
                                      0, pIncludeHandler, &pResult));

  HRESULT compileResult = S_OK;
  VERIFY_SUCCEEDED(pResult->GetStatus(&compileResult));

  if (errors.empty()) {
    VERIFY_SUCCEEDED(compileResult);
  } else {
    VERIFY_FAILED(compileResult);
    CComPtr<IDxcBlobEncoding> pErrors;
    VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&pErrors));

    BOOL bEncodingKnown = FALSE;
    UINT32 uCodePage = 0;
    VERIFY_SUCCEEDED(pErrors->GetEncoding(&bEncodingKnown, &uCodePage));

    std::wstring actualError = BlobToWide(pErrors);
    if (actualError.find(errors) == std::wstring::npos) {
      VERIFY_SUCCEEDED(E_FAIL);
    }
  }
}

TEST_F(CompilerTest, CompileWithResourceBindingFileThenOK) {
  // Normal test
  // List of things tested in this first test:
  //  - Arbitrary capitalization of the headers
  //  - Quotes
  //  - Optional trailing commas
  //  - Arbitrary spaces
  //  - Resources with the same names (but different classes)
  //  - Using include handler
  //
  TestResourceBindingImpl(
      R"(
         ResourceName, binding,  spacE  
         
         "cb",         b10,      0x1e ,  
         resource,     b42,      999  ,
         samp0,        s1,       0x02 ,
         resource,     t1,       2
         uav_0,        u0,       0,
    )");

  // Reordered the columns 1
  TestResourceBindingImpl(
      R"(
         ResourceName, space,   Binding,
                     
         "cb",         0x1e ,   b10,   
         resource,     999  ,   b42,  
         samp0,        0x02 ,   s1,   
         resource,     2,       t1,   
         uav_0,        0,       u0,   
    )",
      std::wstring());

  // Reordered the columns 2
  TestResourceBindingImpl(
      R"(
         space,   binding, ResourceName,         
                                       
         0x1e ,   b10,     "cb",         
         999  ,   b42,     resource,         
         0x02 ,   s1,      samp0,        
         2,       t1,      resource,    
         0,       u0,      uav_0,        
    )");

  // Extra cell at the end of row
  TestResourceBindingImpl(
      R"(
         ResourceName, Binding,  space  
         
         "cb",         b10,      0x1e ,  
         resource,     b42,      999  ,
         samp0,        s1,       0x02,    extra_cell
         resource,     t1,       2
         uav_0,        u0,       0,
    )",
      L"Unexpected cell at the end of row. There should only be 3");

  // Missing cell in row
  TestResourceBindingImpl(
      R"(
         ResourceName, Binding,  space  
         
         "cb",         b10,      0x1e ,  
         resource,     b42,      999  ,
         samp0,        s1,
         resource,     t1,       2
         uav_0,        u0,       0,
    )",
      L"Row ended after just 2 columns. Expected 3.");

  // Missing column
  TestResourceBindingImpl(
      R"(
         ResourceName, Binding,
         "cb",         b10,   
    )",
      L"Input format is csv with headings: ResourceName, Binding, Space.");

  // Empty file
  TestResourceBindingImpl(" \r\n  ", L"Unexpected EOF when parsing cell.");

  // Invalid resource binding type
  TestResourceBindingImpl(
      R"(
         ResourceName, Binding,  space  
         "cb",         10,       30,  
    )",
      L"Invalid resource class");

  // Invalid resource binding type 2
  TestResourceBindingImpl(
      R"(
         ResourceName, Binding,  space  
         "cb",         e10,      30,  
    )",
      L"Invalid resource class.");

  // Index Integer out of bounds
  TestResourceBindingImpl(
      R"(
         ResourceName, Binding,  space  
         "cb",         b99999999999999999999999999999999999,    30,  
    )",
      L"'99999999999999999999999999999999999' is out of range of an 32-bit "
      L"unsigned integer.");

  // Empty resource
  TestResourceBindingImpl(
      R"(
         ResourceName, Binding,  space  
         "cb",         ,         30,  
    )",
      L"Resource binding cannot be empty.");

  // Index Integer out of bounds
  TestResourceBindingImpl(
      R"(
         ResourceName, Binding,  space  
         "cb",         b,        30,  
    )",
      L"'b' is not a valid resource binding.");

  // Integer out of bounds
  TestResourceBindingImpl(
      R"(
         ResourceName, Binding,  space  
         "cb",         b10,      99999999999999999999999999999999999,  
    )",
      L"'99999999999999999999999999999999999' is out of range of an 32-bit "
      L"unsigned integer.");

  // Integer out of bounds 2
  TestResourceBindingImpl(
      R"(
         ResourceName,         Binding,  space  
         "cb",         b10,    0xffffffffffffffffffffffffffffff,  
    )",
      L"'0xffffffffffffffffffffffffffffff' is out of range of an 32-bit "
      L"unsigned integer.");

  // Integer invalid
  TestResourceBindingImpl(
      R"(
         ResourceName, Binding,  space  
         "cb",         b10,      abcd,  
    )",
      L"'abcd' is not a valid 32-bit unsigned integer.");

  // Integer empty
  TestResourceBindingImpl(
      R"(
         ResourceName, Binding, space  
         "cb",         b10,     ,  
    )",
      L"Expected unsigned 32-bit integer for resource space, but got empty "
      L"cell.");

  // No Include handler
  TestResourceBindingImpl(
      R"(
         ResourceName, Binding, space  
         "cb",         b10,     30,  
    )",
      L"Binding table binding file 'binding-file.txt' specified, but no "
      L"include handler was given",
      /* noIncludeHandler */ true);

  // Comma in a cell
  TestResourceBindingImpl(
      R"(
         ResourceName, Binding,  spacE,  extra_column,
         
         "cb",         b10,      0x1e ,  " ,, ,",
         resource,     b42,      999  ,  " ,, ,",
         samp0,        s1,       0x02 ,  " ,, ,",
         resource,     t1,       2,      " ,, ,",
         uav_0,        u0,       0,      " ,, ,",
    )");

  // Newline in the middle of a quote
  TestResourceBindingImpl(
      R"(
         ResourceName, Binding,  spacE,  extra_column,
         
         "cb",         b10,      0x1e ,  " ,, ,",
         resource,     b42,      999  ,  " ,, ,",
         samp0,        s1,       0x02 ,  " ,, ,",
         resource,     t1,       2,      " ,, ,",
         uav_0,        u0,       0,      " ,


    )",
      L"Unexpected newline inside quotation.");
}

TEST_F(CompilerTest, CompileWithRootSignatureThenStripRootSignature) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlob> pProgram;
  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("[RootSignature(\"\")] \r\n"
                     "float4 main(float a : A) : SV_Target {\r\n"
                     "  return a;\r\n"
                     "}",
                     &pSource);
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", nullptr, 0, nullptr, 0,
                                      nullptr, &pResult));
  VERIFY_IS_NOT_NULL(pResult);
  HRESULT status;
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);
  VERIFY_SUCCEEDED(pResult->GetResult(&pProgram));
  VERIFY_IS_NOT_NULL(pProgram);
  hlsl::DxilContainerHeader *pContainerHeader = hlsl::IsDxilContainerLike(
      pProgram->GetBufferPointer(), pProgram->GetBufferSize());
  VERIFY_SUCCEEDED(
      hlsl::IsValidDxilContainer(pContainerHeader, pProgram->GetBufferSize()));
  hlsl::DxilPartHeader *pPartHeader = hlsl::GetDxilPartByType(
      pContainerHeader, hlsl::DxilFourCC::DFCC_RootSignature);
  VERIFY_IS_NOT_NULL(pPartHeader);
  pResult.Release();

  // Remove root signature
  CComPtr<IDxcBlob> pProgramRootSigRemoved;
  CComPtr<IDxcContainerBuilder> pBuilder;
  VERIFY_SUCCEEDED(CreateContainerBuilder(&pBuilder));
  VERIFY_SUCCEEDED(pBuilder->Load(pProgram));
  VERIFY_SUCCEEDED(pBuilder->RemovePart(hlsl::DxilFourCC::DFCC_RootSignature));
  VERIFY_SUCCEEDED(pBuilder->SerializeContainer(&pResult));
  VERIFY_SUCCEEDED(pResult->GetResult(&pProgramRootSigRemoved));
  pContainerHeader =
      hlsl::IsDxilContainerLike(pProgramRootSigRemoved->GetBufferPointer(),
                                pProgramRootSigRemoved->GetBufferSize());
  VERIFY_SUCCEEDED(hlsl::IsValidDxilContainer(
      pContainerHeader, pProgramRootSigRemoved->GetBufferSize()));
  hlsl::DxilPartHeader *pPartHeaderShouldBeNull = hlsl::GetDxilPartByType(
      pContainerHeader, hlsl::DxilFourCC::DFCC_RootSignature);
  VERIFY_IS_NULL(pPartHeaderShouldBeNull);
  pBuilder.Release();
  pResult.Release();

  // Add root signature back
  CComPtr<IDxcBlobEncoding> pRootSignatureBlob;
  CComPtr<IDxcLibrary> pLibrary;
  CComPtr<IDxcBlob> pProgramRootSigAdded;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
  VERIFY_SUCCEEDED(pLibrary->CreateBlobWithEncodingFromPinned(
      hlsl::GetDxilPartData(pPartHeader), pPartHeader->PartSize, 0,
      &pRootSignatureBlob));
  VERIFY_SUCCEEDED(CreateContainerBuilder(&pBuilder));
  VERIFY_SUCCEEDED(pBuilder->Load(pProgramRootSigRemoved));
  pBuilder->AddPart(hlsl::DxilFourCC::DFCC_RootSignature, pRootSignatureBlob);
  pBuilder->SerializeContainer(&pResult);
  VERIFY_SUCCEEDED(pResult->GetResult(&pProgramRootSigAdded));
  pContainerHeader =
      hlsl::IsDxilContainerLike(pProgramRootSigAdded->GetBufferPointer(),
                                pProgramRootSigAdded->GetBufferSize());
  VERIFY_SUCCEEDED(hlsl::IsValidDxilContainer(
      pContainerHeader, pProgramRootSigAdded->GetBufferSize()));
  pPartHeader = hlsl::GetDxilPartByType(pContainerHeader,
                                        hlsl::DxilFourCC::DFCC_RootSignature);
  VERIFY_IS_NOT_NULL(pPartHeader);
}

TEST_F(CompilerTest, CompileThenSetRootSignatureThenValidate) {
  CComPtr<IDxcCompiler> pCompiler;
  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CComPtr<IDxcOperationResult> pResult;
  HRESULT status;

  // Compile with Root Signature in Shader source
  CComPtr<IDxcBlobEncoding> pSourceBlobWithRS;
  CComPtr<IDxcBlob> pProgramSourceRS;
  CreateBlobFromText("[RootSignature(\"\")] \r\n"
                     "float4 main(float a : A) : SV_Target {\r\n"
                     "  return a;\r\n"
                     "}",
                     &pSourceBlobWithRS);
  VERIFY_SUCCEEDED(pCompiler->Compile(pSourceBlobWithRS, L"source.hlsl",
                                      L"main", L"ps_6_0", nullptr, 0, nullptr,
                                      0, nullptr, &pResult));
  VERIFY_IS_NOT_NULL(pResult);
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);
  VERIFY_SUCCEEDED(pResult->GetResult(&pProgramSourceRS));
  VERIFY_IS_NOT_NULL(pProgramSourceRS);

  // Verify RS
  hlsl::DxilContainerHeader *pContainerHeader = hlsl::IsDxilContainerLike(
      pProgramSourceRS->GetBufferPointer(), pProgramSourceRS->GetBufferSize());
  VERIFY_SUCCEEDED(hlsl::IsValidDxilContainer(
      pContainerHeader, pProgramSourceRS->GetBufferSize()));
  hlsl::DxilPartHeader *pPartHeader = hlsl::GetDxilPartByType(
      pContainerHeader, hlsl::DxilFourCC::DFCC_RootSignature);
  VERIFY_IS_NOT_NULL(pPartHeader);

  // Extract the serialized root signature
  CComPtr<IDxcBlob> pRSBlob;
  CComPtr<IDxcResult> pResultSourceRS;
  pResult.QueryInterface(&pResultSourceRS);
  VERIFY_SUCCEEDED(pResultSourceRS->GetOutput(DXC_OUT_ROOT_SIGNATURE,
                                              IID_PPV_ARGS(&pRSBlob), nullptr));
  VERIFY_IS_NOT_NULL(pRSBlob);

  // Add Serialized Root Signature source to include handler
  CComPtr<TestIncludeHandler> pInclude;
  pInclude = new TestIncludeHandler(m_dllSupport);
  pInclude->CallResults.emplace_back(pRSBlob->GetBufferPointer(),
                                     pRSBlob->GetBufferSize());

  // Compile with Set Root Signature
  pResult.Release();
  CComPtr<IDxcBlobEncoding> pSourceNoRS;
  CComPtr<IDxcBlob> pProgramSetRS;
  CreateBlobFromText("float4 main(float a : A) : SV_Target {\r\n"
                     "  return a;\r\n"
                     "}",
                     &pSourceNoRS);
  LPCWSTR args[] = {L"-setrootsignature", L"rootsignaturesource.hlsl"};
  VERIFY_SUCCEEDED(pCompiler->Compile(pSourceNoRS, L"source.hlsl", L"main",
                                      L"ps_6_0", args, _countof(args), nullptr,
                                      0, pInclude, &pResult));
  VERIFY_IS_NOT_NULL(pResult);
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);
  VERIFY_SUCCEEDED(pResult->GetResult(&pProgramSetRS));
  VERIFY_IS_NOT_NULL(pProgramSetRS);

  // Verify RS in container
  hlsl::DxilContainerHeader *pContainerHeaderSet = hlsl::IsDxilContainerLike(
      pProgramSetRS->GetBufferPointer(), pProgramSetRS->GetBufferSize());
  VERIFY_SUCCEEDED(hlsl::IsValidDxilContainer(pContainerHeaderSet,
                                              pProgramSetRS->GetBufferSize()));
  hlsl::DxilPartHeader *pPartHeaderSet = hlsl::GetDxilPartByType(
      pContainerHeaderSet, hlsl::DxilFourCC::DFCC_RootSignature);
  VERIFY_IS_NOT_NULL(pPartHeaderSet);

  // Extract the serialized root signature
  CComPtr<IDxcBlob> pRSBlobSet;
  CComPtr<IDxcResult> pResultSetRS;
  pResult.QueryInterface(&pResultSetRS);
  VERIFY_SUCCEEDED(pResultSetRS->GetOutput(DXC_OUT_ROOT_SIGNATURE,
                                           IID_PPV_ARGS(&pRSBlobSet), nullptr));
  VERIFY_IS_NOT_NULL(pRSBlobSet);

  // Verify RS equal from source and using setrootsignature option
  VERIFY_ARE_EQUAL(pRSBlob->GetBufferSize(), pRSBlobSet->GetBufferSize());
  VERIFY_ARE_EQUAL(0, memcmp(pRSBlob->GetBufferPointer(),
                             pRSBlobSet->GetBufferPointer(),
                             pRSBlob->GetBufferSize()));

  // Change root signature and validate
  pResult.Release();
  CComPtr<IDxcBlobEncoding> pReplaceRS;
  CComPtr<IDxcBlob> pProgramReplaceRS;
  CreateBlobFromText("[RootSignature(\" CBV(b1) \")] \r\n"
                     "float4 main(float a : A) : SV_Target {\r\n"
                     "  return a;\r\n"
                     "}",
                     &pReplaceRS);
  // Add Serialized Root Signature source to include handler
  CComPtr<TestIncludeHandler> pInclude3;
  pInclude3 = new TestIncludeHandler(m_dllSupport);
  pInclude3->CallResults.emplace_back(pRSBlob->GetBufferPointer(),
                                      pRSBlob->GetBufferSize());
  VERIFY_SUCCEEDED(pCompiler->Compile(pReplaceRS, L"source.hlsl", L"main",
                                      L"ps_6_0", args, _countof(args), nullptr,
                                      0, pInclude3, &pResult));
  VERIFY_IS_NOT_NULL(pResult);
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);
  VERIFY_SUCCEEDED(pResult->GetResult(&pProgramReplaceRS));
  VERIFY_IS_NOT_NULL(pProgramReplaceRS);

  // Verify RS
  hlsl::DxilContainerHeader *pContainerHeaderReplace =
      hlsl::IsDxilContainerLike(pProgramReplaceRS->GetBufferPointer(),
                                pProgramReplaceRS->GetBufferSize());
  VERIFY_SUCCEEDED(hlsl::IsValidDxilContainer(
      pContainerHeaderReplace, pProgramReplaceRS->GetBufferSize()));
  hlsl::DxilPartHeader *pPartHeaderReplace = hlsl::GetDxilPartByType(
      pContainerHeaderReplace, hlsl::DxilFourCC::DFCC_RootSignature);
  VERIFY_IS_NOT_NULL(pPartHeaderReplace);

  // Extract the serialized root signature
  CComPtr<IDxcBlob> pRSBlobReplace;
  CComPtr<IDxcResult> pResultReplace;
  pResult.QueryInterface(&pResultReplace);
  VERIFY_SUCCEEDED(pResultReplace->GetOutput(
      DXC_OUT_ROOT_SIGNATURE, IID_PPV_ARGS(&pRSBlobReplace), nullptr));
  VERIFY_IS_NOT_NULL(pRSBlobReplace);

  // Verify RS equal from source and replacing existing RS using
  // setrootsignature option
  VERIFY_ARE_EQUAL(pRSBlob->GetBufferSize(), pRSBlobReplace->GetBufferSize());
  VERIFY_ARE_EQUAL(0, memcmp(pRSBlob->GetBufferPointer(),
                             pRSBlobReplace->GetBufferPointer(),
                             pRSBlob->GetBufferSize()));
}

TEST_F(CompilerTest, CompileSetPrivateThenWithStripPrivate) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlob> pProgram;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("float4 main() : SV_Target {\r\n"
                     "  return 0;\r\n"
                     "}",
                     &pSource);
  std::string privateTxt("private data");
  CComPtr<IDxcBlobEncoding> pPrivate;
  CreateBlobFromText(privateTxt.c_str(), &pPrivate);

  // Add private data source to include handler
  CComPtr<TestIncludeHandler> pInclude;
  pInclude = new TestIncludeHandler(m_dllSupport);
  pInclude->CallResults.emplace_back(pPrivate->GetBufferPointer(),
                                     pPrivate->GetBufferSize());

  LPCWSTR args[] = {L"-setprivate", L"privatesource.hlsl"};
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, _countof(args), nullptr,
                                      0, pInclude, &pResult));
  VERIFY_SUCCEEDED(pResult->GetResult(&pProgram));

  hlsl::DxilContainerHeader *pContainerHeader = hlsl::IsDxilContainerLike(
      pProgram->GetBufferPointer(), pProgram->GetBufferSize());
  VERIFY_SUCCEEDED(
      hlsl::IsValidDxilContainer(pContainerHeader, pProgram->GetBufferSize()));
  hlsl::DxilPartHeader *pPartHeader = hlsl::GetDxilPartByType(
      pContainerHeader, hlsl::DxilFourCC::DFCC_PrivateData);
  VERIFY_IS_NOT_NULL(pPartHeader);
  // Compare private data
  std::string privatePart((const char *)(pPartHeader + 1), privateTxt.size());
  VERIFY_IS_TRUE(strcmp(privatePart.c_str(), privateTxt.c_str()) == 0);

  pResult.Release();
  pProgram.Release();

  // Add private data source to include handler
  CComPtr<TestIncludeHandler> pInclude2;
  pInclude2 = new TestIncludeHandler(m_dllSupport);
  pInclude2->CallResults.emplace_back(pPrivate->GetBufferPointer(),
                                      pPrivate->GetBufferSize());

  LPCWSTR args2[] = {L"-setprivate", L"privatesource.hlsl", L"-Qstrip_priv"};
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args2, _countof(args2),
                                      nullptr, 0, pInclude2, &pResult));

  // Check error message when using Qstrip_private and setprivate together
  HRESULT status;
  CComPtr<IDxcBlobEncoding> pErrors;
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_FAILED(status);
  VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&pErrors));
  LPCSTR pErrorMsg = "Cannot specify /Qstrip_priv and /setprivate together.";
  CheckOperationResultMsgs(pResult, &pErrorMsg, 1, false, false);
}

TEST_F(CompilerTest, CompileWithMultiplePrivateOptionsThenFail) {
  CComPtr<IDxcCompiler> pCompiler;
  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));

  std::string main_source = R"x(
      cbuffer MyCbuffer : register(b1) {
        float4 my_cbuf_foo;
      }

      [RootSignature("CBV(b1)")]
      float4 main() : SV_Target {
        return my_cbuf_foo;
      }
  )x";

  CComPtr<IDxcUtils> pUtils;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcUtils, &pUtils));

  CComPtr<IDxcBlobEncoding> pSource;
  VERIFY_SUCCEEDED(pUtils->CreateBlobFromPinned(
      main_source.c_str(), main_source.size(), CP_UTF8, &pSource));

  const WCHAR *args[] = {L"/Zs", L"/Qpdb_in_private", L"/Qstrip_priv"};

  CComPtr<IDxcOperationResult> pResult;
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"hlsl.hlsl", L"main", L"ps_6_0",
                                      args, _countof(args), nullptr, 0, nullptr,
                                      &pResult));

  HRESULT status;
  CComPtr<IDxcBlobEncoding> pErrors;
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_FAILED(status);
  VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&pErrors));
  LPCSTR pErrorMsg =
      "Cannot specify /Qstrip_priv and /Qpdb_in_private together.";
  CheckOperationResultMsgs(pResult, &pErrorMsg, 1, false, false);

  pResult.Release();

  const WCHAR *args2[] = {L"/Zs", L"/Qpdb_in_private", L"/setprivate",
                          L"privatesource.hlsl"};

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"hlsl.hlsl", L"main", L"ps_6_0",
                                      args2, _countof(args2), nullptr, 0,
                                      nullptr, &pResult));

  pErrors.Release();
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_FAILED(status);
  VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&pErrors));
  LPCSTR pErrorMsg2 =
      "Cannot specify /Qpdb_in_private and /setprivate together.";
  CheckOperationResultMsgs(pResult, &pErrorMsg2, 1, false, false);
}

TEST_F(CompilerTest, CompileWhenIncludeThenLoadInvoked) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<TestIncludeHandler> pInclude;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("#include \"helper.h\"\r\n"
                     "float4 main() : SV_Target { return 0; }",
                     &pSource);

  pInclude = new TestIncludeHandler(m_dllSupport);
  pInclude->CallResults.emplace_back("");

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", nullptr, 0, nullptr, 0,
                                      pInclude, &pResult));
  VerifyOperationSucceeded(pResult);
  VERIFY_ARE_EQUAL_WSTR(L"." SLASH_W L"helper.h;",
                        pInclude->GetAllFileNames().c_str());
}

TEST_F(CompilerTest, CompileWhenIncludeThenLoadUsed) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<TestIncludeHandler> pInclude;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("#include \"helper.h\"\r\n"
                     "float4 main() : SV_Target { return ZERO; }",
                     &pSource);

  pInclude = new TestIncludeHandler(m_dllSupport);
  pInclude->CallResults.emplace_back("#define ZERO 0");

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", nullptr, 0, nullptr, 0,
                                      pInclude, &pResult));
  VerifyOperationSucceeded(pResult);
  VERIFY_ARE_EQUAL_WSTR(L"." SLASH_W L"helper.h;",
                        pInclude->GetAllFileNames().c_str());
}

static std::wstring NormalizeForPlatform(const std::wstring &s) {
#ifdef _WIN32
  wchar_t From = L'/';
  wchar_t To = L'\\';
#else
  wchar_t From = L'\\';
  wchar_t To = L'/';
#endif
  std::wstring ret = s;
  for (wchar_t &c : ret) {
    if (c == From)
      c = To;
  }
  return ret;
};

class SimpleIncludeHanlder : public IDxcIncludeHandler {
  DXC_MICROCOM_REF_FIELD(m_dwRef)
public:
  DXC_MICROCOM_ADDREF_RELEASE_IMPL(m_dwRef)
  dxc::DxcDllSupport &m_dllSupport;
  HRESULT m_defaultErrorCode = E_FAIL;
  SimpleIncludeHanlder(dxc::DxcDllSupport &dllSupport)
      : m_dwRef(0), m_dllSupport(dllSupport) {}
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void **ppvObject) override {
    return DoBasicQueryInterface<IDxcIncludeHandler>(this, iid, ppvObject);
  }

  std::wstring Path;
  std::string Content;

  HRESULT STDMETHODCALLTYPE LoadSource(
      LPCWSTR pFilename,         // Filename as written in #include statement
      IDxcBlob **ppIncludeSource // Resultant source object for included file
      ) override {
    if (pFilename == Path) {
      MultiByteStringToBlob(m_dllSupport, Content, CP_UTF8, ppIncludeSource);
      return S_OK;
    }
    return E_FAIL;
  }
};

TEST_F(CompilerTest, CompileWithIncludeThenTestNoLexicalBlockFile) {
  std::string includeFile = R"x(
    [RootSignature("")]
    float main(uint x : X) : SV_Target {
      float ret = 0;
      if (x) {
        float other_ret = 1;
        ret = other_ret;
      }
      return ret;
    }
    )x";
  std::string mainFile = R"(
    #include "include.h"
  )";

  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<SimpleIncludeHanlder> pInclude;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(mainFile.c_str(), &pSource);

  pInclude = new SimpleIncludeHanlder(m_dllSupport);
  pInclude->Content = includeFile;
  pInclude->Path = L"." SLASH_W "include.h";

  std::vector<const WCHAR *> args = {L"/Zi", L"/Od"};
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"MyShader.hlsl", L"main",
                                      L"ps_6_0", args.data(), args.size(),
                                      nullptr, 0, pInclude, &pResult));

  CComPtr<IDxcResult> pRealResult;
  VERIFY_SUCCEEDED(pResult.QueryInterface(&pRealResult));

  CComPtr<IDxcBlob> pDxil;
  VERIFY_SUCCEEDED(
      pRealResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pDxil), nullptr));

  CComPtr<IDxcBlobEncoding> pDisasm;
  VERIFY_SUCCEEDED(pCompiler->Disassemble(pDxil, &pDisasm));
  CComPtr<IDxcBlobUtf8> pDisasmUtf8;
  VERIFY_SUCCEEDED(pDisasm.QueryInterface(&pDisasmUtf8));

  std::string disasm(pDisasmUtf8->GetStringPointer(),
                     pDisasmUtf8->GetStringLength());
  VERIFY_IS_TRUE(disasm.find("!DILexicalBlock") != std::string::npos);
  VERIFY_IS_TRUE(disasm.find("!DILexicalBlockFile") == std::string::npos);
}

TEST_F(CompilerTest, TestPdbUtilsPathNormalizations) {
#include "TestHeaders/TestPdbUtilsPathNormalizations.h"
  struct TestCase {
    std::string MainName;
    std::string IncludeName;
  };

  TestCase tests[] = {
      {R"(main.hlsl)", R"(include.h)"},
      {R"(.\5Cmain.hlsl)", R"(.\5Cinclude.h)"},
      {R"(/path/main.hlsl)", R"(/path/include.h)"},
      {R"(\5Cpath/main.hlsl)", R"(\5Cpath\5Cinclude.h)"},
      {R"(..\5Cmain.hlsl)", R"(..\5Cinclude.h)"},
      {R"(..\5Cdir\5Cmain.hlsl)", R"(..\5Cdir\5Cinclude.h)"},
      {R"(F:\5C\5Cdir\5Cmain.hlsl)", R"(F:\5C\5Cdir\5Cinclude.h)"},
      {R"(\5C\5Cdir\5Cmain.hlsl)", R"(\5C\5Cdir\5Cinclude.h)"},
      {R"(\5C\5C\5Cdir\5Cmain.hlsl)", R"(\5C\5C\5Cdir\5Cinclude.h)"},
      {R"(//dir\5Cmain.hlsl)", R"(//dir/include.h)"},
      {R"(///dir/main.hlsl)", R"(///dir\5Cinclude.h)"},
  };

  for (TestCase &test : tests) {
    std::string oldPdb = kTestPdbUtilsPathNormalizationsIR;

    size_t findPos = std::string::npos;
    std::string mainPattern = "<MAIN_FILE>";
    std::string includePattern = "<INCLUDE_FILE>";
    while ((findPos = oldPdb.find(mainPattern)) != std::string::npos) {
      oldPdb.replace(oldPdb.begin() + findPos,
                     oldPdb.begin() + findPos + mainPattern.size(),
                     test.MainName);
    }
    while ((findPos = oldPdb.find(includePattern)) != std::string::npos) {
      oldPdb.replace(oldPdb.begin() + findPos,
                     oldPdb.begin() + findPos + includePattern.size(),
                     test.IncludeName);
    }

    CComPtr<IDxcAssembler> pAssembler;
    VERIFY_SUCCEEDED(
        m_dllSupport.CreateInstance(CLSID_DxcAssembler, &pAssembler));
    CComPtr<IDxcUtils> pUtils;
    VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcUtils, &pUtils));

    CComPtr<IDxcBlobEncoding> pBlobEncoding;
    VERIFY_SUCCEEDED(pUtils->CreateBlobFromPinned(oldPdb.data(), oldPdb.size(),
                                                  CP_UTF8, &pBlobEncoding));

    CComPtr<IDxcOperationResult> pOpResult;
    VERIFY_SUCCEEDED(
        pAssembler->AssembleToContainer(pBlobEncoding, &pOpResult));
    VerifyOperationSucceeded(pOpResult);
    CComPtr<IDxcBlob> pDxil;
    VERIFY_SUCCEEDED(pOpResult->GetResult(&pDxil));

    CComPtr<IDxcPdbUtils> pPdbUtils;
    VERIFY_SUCCEEDED(
        m_dllSupport.CreateInstance(CLSID_DxcPdbUtils, &pPdbUtils));
    VERIFY_SUCCEEDED(pPdbUtils->Load(pDxil));

    CComBSTR pMainName;
    CComBSTR pIncludeName;
    CComBSTR pMainName2;
    CComPtr<IDxcBlobEncoding> pIncludeContent;
    CComPtr<IDxcBlobEncoding> pMainContent;
    VERIFY_SUCCEEDED(pPdbUtils->GetSourceName(0, &pMainName));
    VERIFY_SUCCEEDED(pPdbUtils->GetSourceName(1, &pIncludeName));
    VERIFY_SUCCEEDED(pPdbUtils->GetMainFileName(&pMainName2));
    VERIFY_SUCCEEDED(pPdbUtils->GetSource(0, &pMainContent));
    VERIFY_SUCCEEDED(pPdbUtils->GetSource(1, &pIncludeContent));

    VERIFY_ARE_EQUAL(0, wcscmp(pMainName.m_str, pMainName2.m_str));

    CComPtr<IDxcBlobUtf8> pMainContentUtf8;
    CComPtr<IDxcBlobUtf8> pIncludeContentUtf8;
    VERIFY_SUCCEEDED(pMainContent.QueryInterface(&pMainContentUtf8));
    VERIFY_SUCCEEDED(pIncludeContent.QueryInterface(&pIncludeContentUtf8));

    CComPtr<SimpleIncludeHanlder> pRecompileInclude =
        new SimpleIncludeHanlder(m_dllSupport);
    pRecompileInclude->Content =
        std::string(pIncludeContentUtf8->GetStringPointer(),
                    pIncludeContentUtf8->GetStringLength());
    pRecompileInclude->Path = pIncludeName;

    CComPtr<IDxcOperationResult> pRecompileOpResult;
    const WCHAR *args[] = {L"-Zi"};

    CComPtr<IDxcCompiler> pCompiler;
    VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
    VERIFY_SUCCEEDED(pCompiler->Compile(
        pMainContentUtf8, pMainName, L"main", L"ps_6_0", args, _countof(args),
        nullptr, 0, pRecompileInclude, &pRecompileOpResult));
    VerifyOperationSucceeded(pRecompileOpResult);
  }
}

TEST_F(CompilerTest, CompileWhenAllIncludeCombinations) {
  struct File {
    std::wstring name;
    std::string content;
  };
  struct TestCase {
    std::wstring mainFileArg;
    File mainFile;
    File includeFile;
    std::vector<const WCHAR *> extraArgs;
  };
  std::string commonIncludeFile = "float foo() { return 10; }";

  TestCase tests[] = {
      {L"main.hlsl",
       {L".\\main.hlsl",
        R"(#include "include.h"
           float main() : SV_Target { return foo(); } )"},
       {L".\\include.h", commonIncludeFile},
       {}},

      {L"./main.hlsl",
       {L".\\main.hlsl",
        R"(#include "include.h"
           float main() : SV_Target { return foo(); } )"},
       {L".\\include.h", commonIncludeFile},
       {}},

      {L"../main.hlsl",
       {L".\\..\\main.hlsl",
        R"(#include "include.h"\n
           float main() : SV_Target { return foo(); } )"},
       {L".\\..\\include.h", commonIncludeFile},
       {}},

      {L"../main.hlsl",
       {L".\\..\\main.hlsl",
        R"(#include "include_dir/include.h"
           float main() : SV_Target { return foo(); } )"},
       {L".\\..\\include_dir\\include.h", commonIncludeFile},
       {}},

      {L"../main.hlsl",
       {L".\\..\\main.hlsl",
        R"(#include "../include.h"
           float main() : SV_Target { return foo(); } )"},
       {L".\\..\\..\\include.h", commonIncludeFile},
       {}},

      {L"../main.hlsl",
       {L".\\..\\main.hlsl",
        R"(#include "second_dir/include.h"
           float main() : SV_Target { return foo(); } )"},
       {L".\\..\\my_include_dir\\second_dir\\include.h", commonIncludeFile},
       {L"-I", L"../my_include_dir"}},

      {L"../main.hlsl",
       {L".\\..\\main.hlsl",
        R"(#include "second_dir/include.h"
           float main() : SV_Target { return foo(); } )"},
       {L".\\my_include_dir\\second_dir\\include.h", commonIncludeFile},
       {L"-I", L"my_include_dir"}},

#ifdef _WIN32
      {L"../main.hlsl",
       {L".\\..\\main.hlsl",
        R"(#include "\\my_include_dir\\second_dir/include.h" // <-- Network path
           float main() : SV_Target { return foo(); } )"},
       {L"\\\\my_include_dir\\second_dir\\include.h", commonIncludeFile},
       {}},
#else
      {L"../main.hlsl",
       {L".\\..\\main.hlsl",
        R"(#include "/my_include_dir\\second_dir/include.h" // <-- Unix absolute path
           float main() : SV_Target { return foo(); } )"},
       {L"/my_include_dir\\second_dir\\include.h", commonIncludeFile},
       {}},
#endif
  };

  for (TestCase &t : tests) {
    CComPtr<IDxcCompiler> pCompiler;
    CComPtr<IDxcOperationResult> pResult;
    CComPtr<IDxcBlobEncoding> pSource;
    CComPtr<SimpleIncludeHanlder> pInclude;

    VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
    CreateBlobFromText(t.mainFile.content.c_str(), &pSource);

    pInclude = new SimpleIncludeHanlder(m_dllSupport);
    pInclude->Content = t.includeFile.content;
    pInclude->Path = NormalizeForPlatform(t.includeFile.name);

    std::vector<const WCHAR *> args = {L"-Zi", L"-Qembed_debug"};
    args.insert(args.end(), t.extraArgs.begin(), t.extraArgs.end());

    VERIFY_SUCCEEDED(pCompiler->Compile(pSource, t.mainFileArg.c_str(), L"main",
                                        L"ps_6_0", args.data(), args.size(),
                                        nullptr, 0, pInclude, &pResult));

    CComPtr<IDxcBlob> pPdb;
    CComPtr<IDxcBlob> pDxil;
    CComPtr<IDxcResult> pRealResult;
    VERIFY_SUCCEEDED(pResult.QueryInterface(&pRealResult));
    VERIFY_SUCCEEDED(
        pRealResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPdb), nullptr));
    VERIFY_SUCCEEDED(
        pRealResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pDxil), nullptr));

    IDxcBlob *debugBlobs[] = {pPdb, pDxil};
    for (IDxcBlob *pDbgBlob : debugBlobs) {
      CComPtr<IDxcPdbUtils> pPdbUtils;
      VERIFY_SUCCEEDED(
          m_dllSupport.CreateInstance(CLSID_DxcPdbUtils, &pPdbUtils));
      VERIFY_SUCCEEDED(pPdbUtils->Load(pDbgBlob));

      CComBSTR pMainFileName;
      VERIFY_SUCCEEDED(pPdbUtils->GetMainFileName(&pMainFileName));
      VERIFY_ARE_EQUAL(NormalizeForPlatform(t.mainFile.name),
                       pMainFileName.m_str);

      pMainFileName.Empty();
      VERIFY_SUCCEEDED(pPdbUtils->GetSourceName(0, &pMainFileName));
      VERIFY_ARE_EQUAL(NormalizeForPlatform(t.mainFile.name),
                       pMainFileName.m_str);

      CComBSTR pIncludeName;
      VERIFY_SUCCEEDED(pPdbUtils->GetSourceName(1, &pIncludeName));
      VERIFY_ARE_EQUAL(NormalizeForPlatform(t.includeFile.name),
                       pIncludeName.m_str);

      CComPtr<IDxcBlobEncoding> pMainSource;
      VERIFY_SUCCEEDED(pPdbUtils->GetSource(0, &pMainSource));

      CComPtr<SimpleIncludeHanlder> pRecompileInclude =
          new SimpleIncludeHanlder(m_dllSupport);
      pRecompileInclude->Content = t.includeFile.content;
      pRecompileInclude->Path = pIncludeName;

      CComPtr<IDxcOperationResult> pRecompileOpResult;
      VERIFY_SUCCEEDED(pCompiler->Compile(
          pMainSource, pMainFileName, L"main", L"ps_6_0", args.data(),
          args.size(), nullptr, 0, pRecompileInclude, &pRecompileOpResult));
      VerifyOperationSucceeded(pRecompileOpResult);
    }
  }
}

TEST_F(CompilerTest, CompileWhenIncludeAbsoluteThenLoadAbsolute) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<TestIncludeHandler> pInclude;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
#ifdef _WIN32 // OS-specific root
  CreateBlobFromText("#include \"C:\\helper.h\"\r\n"
                     "float4 main() : SV_Target { return ZERO; }",
                     &pSource);
#else
  CreateBlobFromText("#include \"/helper.h\"\n"
                     "float4 main() : SV_Target { return ZERO; }",
                     &pSource);
#endif

  pInclude = new TestIncludeHandler(m_dllSupport);
  pInclude->CallResults.emplace_back("#define ZERO 0");

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", nullptr, 0, nullptr, 0,
                                      pInclude, &pResult));
  VerifyOperationSucceeded(pResult);
#ifdef _WIN32 // OS-specific root
  VERIFY_ARE_EQUAL_WSTR(L"C:\\helper.h;", pInclude->GetAllFileNames().c_str());
#else
  VERIFY_ARE_EQUAL_WSTR(L"/helper.h;", pInclude->GetAllFileNames().c_str());
#endif
}

TEST_F(CompilerTest, CompileWhenIncludeLocalThenLoadRelative) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<TestIncludeHandler> pInclude;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("#include \"..\\helper.h\"\r\n"
                     "float4 main() : SV_Target { return ZERO; }",
                     &pSource);

  pInclude = new TestIncludeHandler(m_dllSupport);
  pInclude->CallResults.emplace_back("#define ZERO 0");

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", nullptr, 0, nullptr, 0,
                                      pInclude, &pResult));
  VerifyOperationSucceeded(pResult);
#ifdef _WIN32 // OS-specific directory dividers
  VERIFY_ARE_EQUAL_WSTR(L".\\..\\helper.h;",
                        pInclude->GetAllFileNames().c_str());
#else
  VERIFY_ARE_EQUAL_WSTR(L"./../helper.h;", pInclude->GetAllFileNames().c_str());
#endif
}

TEST_F(CompilerTest, CompileWhenIncludeSystemThenLoadNotRelative) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<TestIncludeHandler> pInclude;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("#include \"subdir/other/file.h\"\r\n"
                     "float4 main() : SV_Target { return ZERO; }",
                     &pSource);

  LPCWSTR args[] = {L"-Ifoo"};
  pInclude = new TestIncludeHandler(m_dllSupport);
  pInclude->CallResults.emplace_back("#include <helper.h>");
  pInclude->CallResults.emplace_back("#define ZERO 0");

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, _countof(args), nullptr,
                                      0, pInclude, &pResult));
  VerifyOperationSucceeded(pResult);
#ifdef _WIN32 // OS-specific directory dividers
  VERIFY_ARE_EQUAL_WSTR(L".\\subdir\\other\\file.h;.\\foo\\helper.h;",
                        pInclude->GetAllFileNames().c_str());
#else
  VERIFY_ARE_EQUAL_WSTR(L"./subdir/other/file.h;./foo/helper.h;",
                        pInclude->GetAllFileNames().c_str());
#endif
}

TEST_F(CompilerTest, CompileWhenIncludeSystemMissingThenLoadAttempt) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<TestIncludeHandler> pInclude;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("#include \"subdir/other/file.h\"\r\n"
                     "float4 main() : SV_Target { return ZERO; }",
                     &pSource);

  pInclude = new TestIncludeHandler(m_dllSupport);
  pInclude->CallResults.emplace_back("#include <helper.h>");
  pInclude->CallResults.emplace_back("#define ZERO 0");

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", nullptr, 0, nullptr, 0,
                                      pInclude, &pResult));
  std::string failLog(VerifyOperationFailed(pResult));
  VERIFY_ARE_NOT_EQUAL(
      std::string::npos,
      failLog.find("<angled>")); // error message should prompt to use <angled>
                                 // rather than "quotes"
#ifdef _WIN32
  VERIFY_ARE_EQUAL_WSTR(L".\\subdir\\other\\file.h;.\\subdir\\other\\helper.h;",
                        pInclude->GetAllFileNames().c_str());
#else
  VERIFY_ARE_EQUAL_WSTR(L"./subdir/other/file.h;./subdir/other/helper.h;",
                        pInclude->GetAllFileNames().c_str());
#endif
}

TEST_F(CompilerTest, CompileWhenIncludeFlagsThenIncludeUsed) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<TestIncludeHandler> pInclude;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("#include <helper.h>\r\n"
                     "float4 main() : SV_Target { return ZERO; }",
                     &pSource);

  pInclude = new TestIncludeHandler(m_dllSupport);
  pInclude->CallResults.emplace_back("#define ZERO 0");

#ifdef _WIN32 // OS-specific root
  LPCWSTR args[] = {L"-I\\\\server\\share"};
#else
  LPCWSTR args[] = {L"-I/server/share"};
#endif
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, _countof(args), nullptr,
                                      0, pInclude, &pResult));
  VerifyOperationSucceeded(pResult);
#ifdef _WIN32 // OS-specific root
  VERIFY_ARE_EQUAL_WSTR(L"\\\\server\\share\\helper.h;",
                        pInclude->GetAllFileNames().c_str());
#else
  VERIFY_ARE_EQUAL_WSTR(L"/server/share/helper.h;",
                        pInclude->GetAllFileNames().c_str());
#endif
}

TEST_F(CompilerTest, CompileThenCheckDisplayIncludeProcess) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<TestIncludeHandler> pInclude;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("#include \"inc/helper.h\"\r\n"
                     "float4 main() : SV_Target { return ZERO; }",
                     &pSource);

  pInclude = new TestIncludeHandler(m_dllSupport);
  pInclude->CallResults.emplace_back("#define ZERO 0");

  LPCWSTR args[] = {L"-I inc", L"-Vi"};
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, _countof(args), nullptr,
                                      0, pInclude, &pResult));
  VerifyOperationSucceeded(pResult);

  CComPtr<IDxcResult> pCompileResult;
  CComPtr<IDxcBlob> pRemarkBlob;
  pResult->QueryInterface(&pCompileResult);
  VERIFY_SUCCEEDED(pCompileResult->GetOutput(
      DXC_OUT_REMARKS, IID_PPV_ARGS(&pRemarkBlob), nullptr));
  std::string text(BlobToUtf8(pRemarkBlob));

  VERIFY_ARE_NOT_EQUAL(
      string::npos, text.find("Opening file [./inc/helper.h], stack top [0]"));
}

TEST_F(CompilerTest, CompileThenPrintTimeReport) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<TestIncludeHandler> pInclude;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("float4 main() : SV_Target { return 0.0; }", &pSource);

  LPCWSTR args[] = {L"-ftime-report"};
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, _countof(args), nullptr,
                                      0, pInclude, &pResult));
  VerifyOperationSucceeded(pResult);

  CComPtr<IDxcResult> pCompileResult;
  CComPtr<IDxcBlob> pReportBlob;
  pResult->QueryInterface(&pCompileResult);
  VERIFY_SUCCEEDED(pCompileResult->GetOutput(
      DXC_OUT_TIME_REPORT, IID_PPV_ARGS(&pReportBlob), nullptr));
  std::string text(BlobToUtf8(pReportBlob));

  VERIFY_ARE_NOT_EQUAL(string::npos,
                       text.find("... Pass execution timing report ..."));
}

TEST_F(CompilerTest, CompileThenPrintTimeTrace) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<TestIncludeHandler> pInclude;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("float4 main() : SV_Target { return 0.0; }", &pSource);

  LPCWSTR args[] = {L"-ftime-trace"};
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, _countof(args), nullptr,
                                      0, pInclude, &pResult));
  VerifyOperationSucceeded(pResult);

  CComPtr<IDxcResult> pCompileResult;
  CComPtr<IDxcBlob> pReportBlob;
  pResult->QueryInterface(&pCompileResult);
  VERIFY_SUCCEEDED(pCompileResult->GetOutput(
      DXC_OUT_TIME_TRACE, IID_PPV_ARGS(&pReportBlob), nullptr));
  std::string text(BlobToUtf8(pReportBlob));

  VERIFY_ARE_NOT_EQUAL(string::npos, text.find("{ \"traceEvents\": ["));
}

TEST_F(CompilerTest, CompileWhenIncludeMissingThenFail) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<TestIncludeHandler> pInclude;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("#include \"file.h\"\r\n"
                     "float4 main() : SV_Target { return 0; }",
                     &pSource);

  pInclude = new TestIncludeHandler(m_dllSupport);

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", nullptr, 0, nullptr, 0,
                                      pInclude, &pResult));
  HRESULT hr;
  VERIFY_SUCCEEDED(pResult->GetStatus(&hr));
  VERIFY_FAILED(hr);
}

TEST_F(CompilerTest, CompileWhenIncludeHasPathThenOK) {
  CComPtr<IDxcCompiler> pCompiler;
  LPCWSTR Source = L"c:\\temp\\OddIncludes\\main.hlsl";
  LPCWSTR Args[] = {L"/I", L"c:\\temp"};
  LPCWSTR ArgsUp[] = {L"/I", L"c:\\Temp"};
  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  bool useUpValues[] = {false, true};
  for (bool useUp : useUpValues) {
    CComPtr<IDxcOperationResult> pResult;
    CComPtr<IDxcBlobEncoding> pSource;
#if TEST_ON_DISK
    CComPtr<IDxcLibrary> pLibrary;
    VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
    VERIFY_SUCCEEDED(pLibrary->CreateIncludeHandler(&pInclude));
    VERIFY_SUCCEEDED(pLibrary->CreateBlobFromFile(Source, nullptr, &pSource));
#else
    CComPtr<TestIncludeHandler> pInclude;
    pInclude = new TestIncludeHandler(m_dllSupport);
    pInclude->CallResults.emplace_back("// Empty");
    CreateBlobFromText("#include \"include.hlsl\"\r\n"
                       "float4 main() : SV_Target { return 0; }",
                       &pSource);
#endif

    VERIFY_SUCCEEDED(pCompiler->Compile(pSource, Source, L"main", L"ps_6_0",
                                        useUp ? ArgsUp : Args, _countof(Args),
                                        nullptr, 0, pInclude, &pResult));
    HRESULT hr;
    VERIFY_SUCCEEDED(pResult->GetStatus(&hr));
    VERIFY_SUCCEEDED(hr);
  }
}

TEST_F(CompilerTest, CompileWhenIncludeEmptyThenOK) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<TestIncludeHandler> pInclude;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("#include \"empty.h\"\r\n"
                     "float4 main() : SV_Target { return 0; }",
                     &pSource);

  pInclude = new TestIncludeHandler(m_dllSupport);
  pInclude->CallResults.emplace_back(
      "", CP_ACP); // An empty file would get detected as ACP code page

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", nullptr, 0, nullptr, 0,
                                      pInclude, &pResult));
  VerifyOperationSucceeded(pResult);
  VERIFY_ARE_EQUAL_WSTR(L"." SLASH_W L"empty.h;",
                        pInclude->GetAllFileNames().c_str());
}

static const char EmptyCompute[] = "[numthreads(8,8,1)] void main() { }";

TEST_F(CompilerTest, CompileWhenODumpThenCheckNoSink) {
  struct Check {
    std::vector<const WCHAR *> Args;
    std::vector<const WCHAR *> Passes;
  };

  Check Checks[] = {
      {{L"-Odump"},
       {L"-instcombine,NoSink=0", L"-dxil-loop-deletion,NoSink=0"}},
      {{L"-Odump", L"-opt-disable sink"},
       {L"-instcombine,NoSink=1", L"-dxil-loop-deletion,NoSink=1"}},
  };

  for (Check &C : Checks) {

    CComPtr<IDxcCompiler> pCompiler;
    CComPtr<IDxcOperationResult> pResult;
    CComPtr<IDxcBlobEncoding> pSource;

    VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
    CreateBlobFromText(EmptyCompute, &pSource);

    VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                        L"cs_6_0", C.Args.data(), C.Args.size(),
                                        nullptr, 0, nullptr, &pResult));

    VerifyOperationSucceeded(pResult);
    CComPtr<IDxcBlob> pResultBlob;
    VERIFY_SUCCEEDED(pResult->GetResult(&pResultBlob));
    wstring passes = BlobToWide(pResultBlob);

    for (const WCHAR *pPattern : C.Passes) {
      VERIFY_ARE_NOT_EQUAL(wstring::npos, passes.find(pPattern));
    }
  }
}

TEST_F(CompilerTest, CompileWhenODumpThenPassConfig) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(EmptyCompute, &pSource);

  LPCWSTR Args[] = {L"/Odump"};

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"cs_6_0", Args, _countof(Args), nullptr,
                                      0, nullptr, &pResult));
  VerifyOperationSucceeded(pResult);
  CComPtr<IDxcBlob> pResultBlob;
  VERIFY_SUCCEEDED(pResult->GetResult(&pResultBlob));
  wstring passes = BlobToWide(pResultBlob);
  VERIFY_ARE_NOT_EQUAL(wstring::npos, passes.find(L"inline"));
}

TEST_F(CompilerTest, CompileWhenVdThenProducesDxilContainer) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(EmptyCompute, &pSource);

  LPCWSTR Args[] = {L"/Vd"};

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"cs_6_0", Args, _countof(Args), nullptr,
                                      0, nullptr, &pResult));
  VerifyOperationSucceeded(pResult);
  CComPtr<IDxcBlob> pResultBlob;
  VERIFY_SUCCEEDED(pResult->GetResult(&pResultBlob));
  VERIFY_IS_TRUE(
      hlsl::IsValidDxilContainer(reinterpret_cast<hlsl::DxilContainerHeader *>(
                                     pResultBlob->GetBufferPointer()),
                                 pResultBlob->GetBufferSize()));
}

void CompilerTest::TestEncodingImpl(const void *sourceData, size_t sourceSize,
                                    UINT32 codePage, const void *includedData,
                                    size_t includedSize,
                                    const WCHAR *encoding) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<TestIncludeHandler> pInclude;
  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobPinned((const char *)sourceData, sourceSize, codePage, &pSource);
  pInclude = new TestIncludeHandler(m_dllSupport);
  pInclude->CallResults.emplace_back(includedData, includedSize, CP_ACP);

  const WCHAR *pArgs[] = {L"-encoding", encoding};
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", pArgs, _countof(pArgs),
                                      nullptr, 0, pInclude, &pResult));
  HRESULT status;
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);
}

TEST_F(CompilerTest, CompileWithEncodeFlagTestSource) {

  std::string sourceUtf8 = "#include \"include.hlsl\"\r\n"
                           "float4 main() : SV_Target { return 0; }";
  std::string includeUtf8 = "// Comment\n";
  std::string utf8BOM = "\xEF"
                        "\xBB"
                        "\xBF"; // UTF-8 BOM
  std::string includeUtf8BOM = utf8BOM + includeUtf8;

  std::wstring sourceWide = L"#include \"include.hlsl\"\r\n"
                            L"float4 main() : SV_Target { return 0; }";
  std::wstring includeWide = L"// Comments\n";
  std::wstring utf16BOM = L"\xFEFF"; // UTF-16 LE BOM
  std::wstring includeUtf16BOM = utf16BOM + includeWide;

  // Included files interpreted with encoding option if no BOM
  TestEncodingImpl(sourceUtf8.data(), sourceUtf8.size(), DXC_CP_UTF8,
                   includeUtf8.data(), includeUtf8.size(), L"utf8");

  TestEncodingImpl(sourceWide.data(), sourceWide.size() * sizeof(L'A'),
                   DXC_CP_WIDE, includeWide.data(),
                   includeWide.size() * sizeof(L'A'), L"wide");

  // Encoding option ignored if BOM present
  TestEncodingImpl(sourceUtf8.data(), sourceUtf8.size(), DXC_CP_UTF8,
                   includeUtf8BOM.data(), includeUtf8BOM.size(), L"wide");

  TestEncodingImpl(sourceWide.data(), sourceWide.size() * sizeof(L'A'),
                   DXC_CP_WIDE, includeUtf16BOM.data(),
                   includeUtf16BOM.size() * sizeof(L'A'), L"utf8");

  // Source file interpreted according to DxcBuffer encoding if not CP_ACP
  // Included files interpreted with encoding option if no BOM
  TestEncodingImpl(sourceUtf8.data(), sourceUtf8.size(), DXC_CP_UTF8,
                   includeWide.data(), includeWide.size() * sizeof(L'A'),
                   L"wide");

  TestEncodingImpl(sourceWide.data(), sourceWide.size() * sizeof(L'A'),
                   DXC_CP_WIDE, includeUtf8.data(), includeUtf8.size(),
                   L"utf8");

  // Source file interpreted by encoding option if source DxcBuffer encoding =
  // CP_ACP (default)
  TestEncodingImpl(sourceUtf8.data(), sourceUtf8.size(), DXC_CP_ACP,
                   includeUtf8.data(), includeUtf8.size(), L"utf8");

  TestEncodingImpl(sourceWide.data(), sourceWide.size() * sizeof(L'A'),
                   DXC_CP_ACP, includeWide.data(),
                   includeWide.size() * sizeof(L'A'), L"wide");
}

TEST_F(CompilerTest, CompileWhenODumpThenOptimizerMatch) {
  LPCWSTR OptLevels[] = {L"/Od", L"/O1", L"/O2"};
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOptimizer> pOptimizer;
  CComPtr<IDxcAssembler> pAssembler;
  CComPtr<IDxcValidator> pValidator;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcAssembler, &pAssembler));
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcOptimizer, &pOptimizer));
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
  for (LPCWSTR OptLevel : OptLevels) {
    CComPtr<IDxcOperationResult> pResult;
    CComPtr<IDxcBlobEncoding> pSource;
    CComPtr<IDxcBlob> pHighLevelBlob;
    CComPtr<IDxcBlob> pOptimizedModule;
    CComPtr<IDxcBlob> pAssembledBlob;

    // Could use EmptyCompute and cs_6_0, but there is an issue where properties
    // don't round-trip properly at high-level, so validation fails because
    // dimensions are set to zero. Workaround by using pixel shader instead.
    LPCWSTR Target = L"ps_6_0";
    CreateBlobFromText("float4 main() : SV_Target { return 0; }", &pSource);

    LPCWSTR Args[2] = {OptLevel, L"/Odump"};

    // Get the passes for this optimization level.
    VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                        Target, Args, _countof(Args), nullptr,
                                        0, nullptr, &pResult));
    VerifyOperationSucceeded(pResult);
    CComPtr<IDxcBlob> pResultBlob;
    VERIFY_SUCCEEDED(pResult->GetResult(&pResultBlob));
    wstring passes = BlobToWide(pResultBlob);

    // Get wchar_t version and prepend hlsl-hlensure, to do a split
    // high-level/opt compilation pass.
    std::vector<LPCWSTR> Options;
    SplitPassList(const_cast<LPWSTR>(passes.data()), Options);

    // Now compile directly.
    pResult.Release();
    VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                        Target, Args, 1, nullptr, 0, nullptr,
                                        &pResult));
    VerifyOperationSucceeded(pResult);

    // Now compile via a high-level compile followed by the optimization passes.
    pResult.Release();
    Args[_countof(Args) - 1] = L"/fcgl";
    VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                        Target, Args, _countof(Args), nullptr,
                                        0, nullptr, &pResult));
    VerifyOperationSucceeded(pResult);
    VERIFY_SUCCEEDED(pResult->GetResult(&pHighLevelBlob));
    VERIFY_SUCCEEDED(pOptimizer->RunOptimizer(pHighLevelBlob, Options.data(),
                                              Options.size(), &pOptimizedModule,
                                              nullptr));

    string text = DisassembleProgram(m_dllSupport, pOptimizedModule);
    WEX::Logging::Log::Comment(L"Final program:");
    WEX::Logging::Log::Comment(CA2W(text.c_str()));

    // At the very least, the module should be valid.
    pResult.Release();
    VERIFY_SUCCEEDED(
        pAssembler->AssembleToContainer(pOptimizedModule, &pResult));
    VerifyOperationSucceeded(pResult);
    VERIFY_SUCCEEDED(pResult->GetResult(&pAssembledBlob));
    pResult.Release();
    VERIFY_SUCCEEDED(pValidator->Validate(pAssembledBlob,
                                          DxcValidatorFlags_Default, &pResult));
    VerifyOperationSucceeded(pResult);
  }
}

static const UINT CaptureStacks = 0; // Set to 1 to enable captures
static const UINT StackFrameCount = 12;

struct InstrumentedHeapMalloc : public IMalloc {
private:
  HANDLE m_Handle; // Heap handle.
  ULONG m_RefCount =
      0; // Reference count. Used for reference leaks, not for lifetime.
  ULONG m_AllocCount = 0; // Total # of alloc and realloc requests.
  ULONG m_AllocSize = 0;  // Total # of alloc and realloc bytes.
  ULONG m_Size = 0;       // Current # of alloc'ed bytes.
  ULONG m_FailAlloc = 0;  // If nonzero, the alloc/realloc call to fail.
  // Each allocation also tracks the following information:
  // - allocation callstack
  // - deallocation callstack
  // - prior/next blocks in a list of allocated blocks
  LIST_ENTRY AllocList;
  struct PtrData {
    LIST_ENTRY Entry;
    LPVOID AllocFrames[CaptureStacks ? StackFrameCount * CaptureStacks : 1];
    LPVOID FreeFrames[CaptureStacks ? StackFrameCount * CaptureStacks : 1];
    UINT64 AllocAtCount;
    DWORD AllocFrameCount;
    DWORD FreeFrameCount;
    SIZE_T Size;
    PtrData *Self;
  };
  PtrData *DataFromPtr(void *p) {
    if (p == nullptr)
      return nullptr;
    PtrData *R = ((PtrData *)p) - 1;
    if (R != R->Self) {
      VERIFY_FAIL(); // p is invalid or underrun
    }
    return R;
  }

public:
  InstrumentedHeapMalloc() : m_Handle(nullptr) { ResetCounts(); }
  ~InstrumentedHeapMalloc() {
    if (m_Handle)
      HeapDestroy(m_Handle);
  }
  void ResetHeap() {
    if (m_Handle) {
      HeapDestroy(m_Handle);
      m_Handle = nullptr;
    }
    m_Handle = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
  }
  ULONG GetRefCount() const { return m_RefCount; }
  ULONG GetAllocCount() const { return m_AllocCount; }
  ULONG GetAllocSize() const { return m_AllocSize; }
  ULONG GetSize() const { return m_Size; }

  void ResetCounts() {
    m_RefCount = m_AllocCount = m_AllocSize = m_Size = 0;
    AllocList.Blink = AllocList.Flink = &AllocList;
  }
  void SetFailAlloc(ULONG index) { m_FailAlloc = index; }

  ULONG STDMETHODCALLTYPE AddRef() override { return ++m_RefCount; }
  ULONG STDMETHODCALLTYPE Release() override {
    if (m_RefCount == 0)
      VERIFY_FAIL();
    return --m_RefCount;
  }
  STDMETHODIMP QueryInterface(REFIID iid, void **ppvObject) override {
    return DoBasicQueryInterface<IMalloc>(this, iid, ppvObject);
  }
  virtual void *STDMETHODCALLTYPE Alloc(SIZE_T cb) override {
    ++m_AllocCount;
    if (m_FailAlloc && m_AllocCount >= m_FailAlloc) {
      return nullptr; // breakpoint for i failure - m_FailAlloc == 1+VAL
    }
    m_AllocSize += cb;
    m_Size += cb;
    PtrData *P =
        (PtrData *)HeapAlloc(m_Handle, HEAP_ZERO_MEMORY, sizeof(PtrData) + cb);
    P->Entry.Flink = AllocList.Flink;
    P->Entry.Blink = &AllocList;
    AllocList.Flink->Blink = &(P->Entry);
    AllocList.Flink = &(P->Entry);
    // breakpoint for i failure on NN alloc - m_FailAlloc == 1+VAL &&
    // m_AllocCount == NN breakpoint for happy path for NN alloc - m_AllocCount
    // == NN
    P->AllocAtCount = m_AllocCount;
#ifndef __ANDROID__
    if (CaptureStacks)
      P->AllocFrameCount =
          CaptureStackBackTrace(1, StackFrameCount, P->AllocFrames, nullptr);
#endif // __ANDROID__
    P->Size = cb;
    P->Self = P;
    return P + 1;
  }

  virtual void *STDMETHODCALLTYPE Realloc(void *pv, SIZE_T cb) override {
    SIZE_T priorSize = pv == nullptr ? (SIZE_T)0 : GetSize(pv);
    void *R = Alloc(cb);
    if (!R)
      return nullptr;
    SIZE_T copySize = std::min(cb, priorSize);
    memcpy(R, pv, copySize);
    Free(pv);
    return R;
  }

  virtual void STDMETHODCALLTYPE Free(void *pv) override {
    if (!pv)
      return;
    PtrData *P = DataFromPtr(pv);
    if (P->FreeFrameCount)
      VERIFY_FAIL(); // double-free detected
    m_Size -= P->Size;
    P->Entry.Flink->Blink = P->Entry.Blink;
    P->Entry.Blink->Flink = P->Entry.Flink;
#ifndef __ANDROID__
    if (CaptureStacks)
      P->FreeFrameCount =
          CaptureStackBackTrace(1, StackFrameCount, P->FreeFrames, nullptr);
#endif // __ANDROID__
  }

  virtual SIZE_T STDMETHODCALLTYPE GetSize(void *pv) override {
    if (pv == nullptr)
      return 0;
    return DataFromPtr(pv)->Size;
  }

  virtual int STDMETHODCALLTYPE DidAlloc(void *pv) override {
    return -1; // don't know
  }

  virtual void STDMETHODCALLTYPE HeapMinimize(void) override {}

  void DumpLeaks() {
    PtrData *ptr = (PtrData *)AllocList.Flink;
    ;
    PtrData *end = (PtrData *)AllocList.Blink;
    ;

    WEX::Logging::Log::Comment(
        FormatToWString(L"Leaks total size: %d", (signed int)m_Size).data());
    while (ptr != end) {
      WEX::Logging::Log::Comment(
          FormatToWString(L"Memory leak at 0x0%X, size %d, alloc# %d", ptr + 1,
                          ptr->Size, ptr->AllocAtCount)
              .data());
      ptr = (PtrData *)ptr->Entry.Flink;
    }
  }
};

#if _ITERATOR_DEBUG_LEVEL == 0
// CompileWhenNoMemThenOOM can properly detect leaks only when debug iterators
// are disabled
#ifdef _WIN32
TEST_F(CompilerTest, CompileWhenNoMemThenOOM) {
#else
// Disabled it is ignored above
TEST_F(CompilerTest, DISABLED_CompileWhenNoMemThenOOM) {
#endif
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);

  CComPtr<IDxcBlobEncoding> pSource;
  CreateBlobFromText(EmptyCompute, &pSource);

  InstrumentedHeapMalloc InstrMalloc;
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  ULONG allocCount = 0;
  ULONG allocSize = 0;
  ULONG initialRefCount;

  InstrMalloc.ResetHeap();

  VERIFY_IS_TRUE(m_dllSupport.HasCreateWithMalloc());

  // Verify a simple object creation.
  initialRefCount = InstrMalloc.GetRefCount();
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance2(&InstrMalloc, CLSID_DxcCompiler,
                                                &pCompiler));
  pCompiler.Release();
  VERIFY_IS_TRUE(0 == InstrMalloc.GetSize());
  VERIFY_ARE_EQUAL(initialRefCount, InstrMalloc.GetRefCount());
  InstrMalloc.ResetCounts();
  InstrMalloc.ResetHeap();

  // First time, run to completion and capture stats.
  initialRefCount = InstrMalloc.GetRefCount();
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance2(&InstrMalloc, CLSID_DxcCompiler,
                                                &pCompiler));
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"cs_6_0", nullptr, 0, nullptr, 0,
                                      nullptr, &pResult));
  allocCount = InstrMalloc.GetAllocCount();
  allocSize = InstrMalloc.GetAllocSize();

  HRESULT hrWithMemory;
  VERIFY_SUCCEEDED(pResult->GetStatus(&hrWithMemory));
  VERIFY_SUCCEEDED(hrWithMemory);

  pCompiler.Release();
  pResult.Release();

  VERIFY_IS_TRUE(allocSize > allocCount);

  // Ensure that after all resources are released, there are no outstanding
  // allocations or references.
  //
  // First leak is in ((InstrumentedHeapMalloc::PtrData
  // *)InstrMalloc.AllocList.Flink)
  if (InstrMalloc.GetSize() != 0) {
    WEX::Logging::Log::Comment(L"Memory leak(s) detected");
    InstrMalloc.DumpLeaks();
    VERIFY_IS_TRUE(0 == InstrMalloc.GetSize());
  }

  VERIFY_ARE_EQUAL(initialRefCount, InstrMalloc.GetRefCount());

  // In Debug, without /D_ITERATOR_DEBUG_LEVEL=0, debug iterators will be used;
  // this causes a problem where std::string is specified as noexcept, and yet
  // a sentinel is allocated that may fail and throw.
  if (m_ver.SkipOutOfMemoryTest())
    return;

  // Now, fail each allocation and make sure we get an error.
  for (ULONG i = 0; i <= allocCount; ++i) {
    // LogCommentFmt(L"alloc fail %u", i);
    bool isLast = i == allocCount;
    InstrMalloc.ResetCounts();
    InstrMalloc.ResetHeap();
    InstrMalloc.SetFailAlloc(i + 1);
    HRESULT hrOp = m_dllSupport.CreateInstance2(&InstrMalloc, CLSID_DxcCompiler,
                                                &pCompiler);
    if (SUCCEEDED(hrOp)) {
      hrOp = pCompiler->Compile(pSource, L"source.hlsl", L"main", L"cs_6_0",
                                nullptr, 0, nullptr, 0, nullptr, &pResult);
      if (SUCCEEDED(hrOp)) {
        pResult->GetStatus(&hrOp);
      }
    }
    if (FAILED(hrOp)) {
      // This is true in *almost* every case. When the OOM happens during stream
      // handling, there is no specific error set; by the time it's detected,
      // it propagates as E_FAIL.
      // VERIFY_ARE_EQUAL(hrOp, E_OUTOFMEMORY);
      VERIFY_IS_TRUE(hrOp == E_OUTOFMEMORY || hrOp == E_FAIL);
    }
    if (isLast)
      VERIFY_SUCCEEDED(hrOp);
    else
      VERIFY_FAILED(hrOp);
    pCompiler.Release();
    pResult.Release();

    if (InstrMalloc.GetSize() != 0) {
      WEX::Logging::Log::Comment(
          FormatToWString(L"Memory leak(s) detected, allocCount = %d", i)
              .data());
      InstrMalloc.DumpLeaks();
      VERIFY_IS_TRUE(0 == InstrMalloc.GetSize());
    }
    VERIFY_ARE_EQUAL(initialRefCount, InstrMalloc.GetRefCount());
  }
}
#endif

TEST_F(CompilerTest, CompileWhenShaderModelMismatchAttributeThenFail) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(EmptyCompute, &pSource);

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", nullptr, 0, nullptr, 0,
                                      nullptr, &pResult));
  std::string failLog(VerifyOperationFailed(pResult));
  VERIFY_ARE_NOT_EQUAL(string::npos,
                       failLog.find("attribute numthreads only valid for CS"));
}

TEST_F(CompilerTest, CompileBadHlslThenFail) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("bad hlsl", &pSource);

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", nullptr, 0, nullptr, 0,
                                      nullptr, &pResult));

  HRESULT status;
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_FAILED(status);
}

TEST_F(CompilerTest, CompileLegacyShaderModelThenFail) {
  VerifyCompileFailed(
      "float4 main(float4 pos : SV_Position) : SV_Target { return pos; }",
      L"ps_5_1", nullptr);
}

TEST_F(CompilerTest, CompileWhenRecursiveAlbeitStaticTermThenFail) {
  // This shader will compile under fxc because if execution is
  // simulated statically, it does terminate. dxc changes this behavior
  // to avoid imposing the requirement on the compiler.
  const char ShaderText[] =
      "static int i = 10;\r\n"
      "float4 f(); // Forward declaration\r\n"
      "float4 g() { if (i > 10) { i--; return f(); } else return 0; } // "
      "Recursive call to 'f'\r\n"
      "float4 f() { return g(); } // First call to 'g'\r\n"
      "float4 VS() : SV_Position{\r\n"
      "  return f(); // First call to 'f'\r\n"
      "}\r\n";
  VerifyCompileFailed(ShaderText, L"vs_6_0",
                      "recursive functions are not allowed: function "
                      "'VS' calls recursive function 'f'",
                      L"VS");
}

TEST_F(CompilerTest, CompileWhenRecursiveThenFail) {
  const char ShaderTextSimple[] =
      "float4 f(); // Forward declaration\r\n"
      "float4 g() { return f(); } // Recursive call to 'f'\r\n"
      "float4 f() { return g(); } // First call to 'g'\r\n"
      "float4 main() : SV_Position{\r\n"
      "  return f(); // First call to 'f'\r\n"
      "}\r\n";
  VerifyCompileFailed(ShaderTextSimple, L"vs_6_0",
                      "recursive functions are not allowed: "
                      "function 'main' calls recursive function 'f'");

  const char ShaderTextIndirect[] =
      "float4 f(); // Forward declaration\r\n"
      "float4 g() { return f(); } // Recursive call to 'f'\r\n"
      "float4 f() { return g(); } // First call to 'g'\r\n"
      "float4 main() : SV_Position{\r\n"
      "  return f(); // First call to 'f'\r\n"
      "}\r\n";
  VerifyCompileFailed(ShaderTextIndirect, L"vs_6_0",
                      "recursive functions are not allowed: "
                      "function 'main' calls recursive function 'f'");

  const char ShaderTextSelf[] = "float4 main() : SV_Position{\r\n"
                                "  return main();\r\n"
                                "}\r\n";
  VerifyCompileFailed(ShaderTextSelf, L"vs_6_0",
                      "recursive functions are not allowed: "
                      "function 'main' calls recursive function 'main'");

  const char ShaderTextMissing[] = "float4 mainz() : SV_Position{\r\n"
                                   "  return 1;\r\n"
                                   "}\r\n";
  VerifyCompileFailed(ShaderTextMissing, L"vs_6_0",
                      "missing entry point definition");
}

TEST_F(CompilerTest, CompileHlsl2015ThenFail) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlobEncoding> pErrors;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(
      "float4 main(float4 pos : SV_Position) : SV_Target { return pos; }",
      &pSource);

  LPCWSTR args[2] = {L"-HV", L"2015"};

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, 2, nullptr, 0, nullptr,
                                      &pResult));

  HRESULT status;
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_ARE_EQUAL(status, E_INVALIDARG);
  VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&pErrors));
  LPCSTR pErrorMsg =
      "HLSL Version 2015 is only supported for language services";
  CheckOperationResultMsgs(pResult, &pErrorMsg, 1, false, false);
}

TEST_F(CompilerTest, CompileHlsl2016ThenOK) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlobEncoding> pErrors;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(
      "float4 main(float4 pos : SV_Position) : SV_Target { return pos; }",
      &pSource);

  LPCWSTR args[2] = {L"-HV", L"2016"};

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, 2, nullptr, 0, nullptr,
                                      &pResult));

  HRESULT status;
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);
}

TEST_F(CompilerTest, CompileHlsl2017ThenOK) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlobEncoding> pErrors;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(
      "float4 main(float4 pos : SV_Position) : SV_Target { return pos; }",
      &pSource);

  LPCWSTR args[2] = {L"-HV", L"2017"};

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, 2, nullptr, 0, nullptr,
                                      &pResult));

  HRESULT status;
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);
}

TEST_F(CompilerTest, CompileHlsl2018ThenOK) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlobEncoding> pErrors;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(
      "float4 main(float4 pos : SV_Position) : SV_Target { return pos; }",
      &pSource);

  LPCWSTR args[2] = {L"-HV", L"2018"};

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, 2, nullptr, 0, nullptr,
                                      &pResult));

  HRESULT status;
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);
}

TEST_F(CompilerTest, CompileHlsl2019ThenFail) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlobEncoding> pErrors;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(
      "float4 main(float4 pos : SV_Position) : SV_Target { return pos; }",
      &pSource);

  LPCWSTR args[2] = {L"-HV", L"2019"};

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, 2, nullptr, 0, nullptr,
                                      &pResult));

  HRESULT status;
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_ARE_EQUAL(status, E_INVALIDARG);
  VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&pErrors));
  LPCSTR pErrorMsg = "Unknown HLSL version";
  CheckOperationResultMsgs(pResult, &pErrorMsg, 1, false, false);
}

TEST_F(CompilerTest, CompileHlsl2020ThenFail) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlobEncoding> pErrors;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(
      "float4 main(float4 pos : SV_Position) : SV_Target { return pos; }",
      &pSource);

  LPCWSTR args[2] = {L"-HV", L"2020"};

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, 2, nullptr, 0, nullptr,
                                      &pResult));

  HRESULT status;
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_ARE_EQUAL(status, E_INVALIDARG);
  VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&pErrors));
  LPCSTR pErrorMsg = "Unknown HLSL version";
  CheckOperationResultMsgs(pResult, &pErrorMsg, 1, false, false);
}

TEST_F(CompilerTest, CompileHlsl2021ThenOK) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlobEncoding> pErrors;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(
      "float4 main(float4 pos : SV_Position) : SV_Target { return pos; }",
      &pSource);

  LPCWSTR args[2] = {L"-HV", L"2021"};

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, 2, nullptr, 0, nullptr,
                                      &pResult));

  HRESULT status;
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);
}

TEST_F(CompilerTest, CompileHlsl2022ThenFail) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlobEncoding> pErrors;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(
      "float4 main(float4 pos : SV_Position) : SV_Target { return pos; }",
      &pSource);

  LPCWSTR args[2] = {L"-HV", L"2022"};

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"ps_6_0", args, 2, nullptr, 0, nullptr,
                                      &pResult));

  HRESULT status;
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_ARE_EQUAL(status, E_INVALIDARG);
  VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&pErrors));
  LPCSTR pErrorMsg = "Unknown HLSL version";
  CheckOperationResultMsgs(pResult, &pErrorMsg, 1, false, false);
}

// this test has issues on ARM64 and clang_cl, disable until we figure out what
// it going on
#if defined(_WIN32) &&                                                         \
    !(defined(_M_ARM64) || defined(_M_ARM64EC) || defined(__clang__))

#pragma fenv_access(on)
#pragma optimize("", off)
#pragma warning(disable : 4723)

// Define test state as something weird that we can verify was restored
static const unsigned int fpTestState =
    (_MCW_EM & (~_EM_ZERODIVIDE)) |   // throw on div by zero
    _DN_FLUSH_OPERANDS_SAVE_RESULTS | // denorm flush operands & save results
    _RC_UP;                           // round up
static const unsigned int fpTestMask = _MCW_EM | _MCW_DN | _MCW_RC;

struct FPTestScope {
  // _controlfp_s is non-standard and <cfenv> doesn't have a function to enable
  // exceptions
  unsigned int fpSavedState;
  FPTestScope() {
    VERIFY_IS_TRUE(_controlfp_s(&fpSavedState, 0, 0) == 0);
    unsigned int newValue;
    VERIFY_IS_TRUE(_controlfp_s(&newValue, fpTestState, fpTestMask) == 0);
  }
  ~FPTestScope() {
    unsigned int newValue;
    errno_t error = _controlfp_s(&newValue, fpSavedState, fpTestMask);
    DXASSERT_LOCALVAR(error, error == 0,
                      "Failed to restore floating-point environment.");
  }
};

void VerifyDivByZeroThrows() {
  bool bCaughtExpectedException = false;
  __try {
    float one = 1.0;
    float zero = 0.0;
    float val = one / zero;
    (void)val;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    bCaughtExpectedException = true;
  }
  VERIFY_IS_TRUE(bCaughtExpectedException);
}

TEST_F(CompilerTest, CodeGenFloatingPointEnvironment) {
  unsigned int fpOriginal;
  VERIFY_IS_TRUE(_controlfp_s(&fpOriginal, 0, 0) == 0);

  {
    FPTestScope fpTestScope;
    // Get state before/after compilation, making sure it's our test state,
    // and that it is restored after the compile.
    unsigned int fpBeforeCompile;
    VERIFY_IS_TRUE(_controlfp_s(&fpBeforeCompile, 0, 0) == 0);
    VERIFY_ARE_EQUAL((fpBeforeCompile & fpTestMask), fpTestState);

    CodeGenTestCheck(L"fpexcept.hlsl");

    // Verify excpetion environment was restored
    unsigned int fpAfterCompile;
    VERIFY_IS_TRUE(_controlfp_s(&fpAfterCompile, 0, 0) == 0);
    VERIFY_ARE_EQUAL((fpBeforeCompile & fpTestMask),
                     (fpAfterCompile & fpTestMask));

    // Make sure round up is set
    VERIFY_ARE_EQUAL(rint(12.25), 13);

    // Make sure we actually enabled div-by-zero exception
    VerifyDivByZeroThrows();
  }

  // Verify original state has been restored
  unsigned int fpLocal;
  VERIFY_IS_TRUE(_controlfp_s(&fpLocal, 0, 0) == 0);
  VERIFY_ARE_EQUAL(fpLocal, fpOriginal);
}

#pragma optimize("", on)

#else //  defined(_WIN32) && !defined(_M_ARM64)

// Only implemented on Win32
TEST_F(CompilerTest, CodeGenFloatingPointEnvironment) { VERIFY_IS_TRUE(true); }

#endif //  defined(_WIN32) && !defined(_M_ARM64)

TEST_F(CompilerTest, CodeGenLibCsEntry) {
  CodeGenTestCheck(L"lib_cs_entry.hlsl");
}

TEST_F(CompilerTest, CodeGenLibCsEntry2) {
  CodeGenTestCheck(L"lib_cs_entry2.hlsl");
}

TEST_F(CompilerTest, CodeGenLibCsEntry3) {
  CodeGenTestCheck(L"lib_cs_entry3.hlsl");
}

TEST_F(CompilerTest, CodeGenLibEntries) {
  CodeGenTestCheck(L"lib_entries.hlsl");
}

TEST_F(CompilerTest, CodeGenLibEntries2) {
  CodeGenTestCheck(L"lib_entries2.hlsl");
}

TEST_F(CompilerTest, CodeGenLibResource) {
  CodeGenTestCheck(L"lib_resource.hlsl");
}

TEST_F(CompilerTest, CodeGenLibUnusedFunc) {
  CodeGenTestCheck(L"lib_unused_func.hlsl");
}

TEST_F(CompilerTest, CodeGenRootSigProfile) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;
  CodeGenTest(L"rootSigProfile.hlsl");
}

TEST_F(CompilerTest, CodeGenRootSigProfile2) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;
  // TODO: Verify the result when reflect the structures.
  CodeGenTest(L"rootSigProfile2.hlsl");
}

TEST_F(CompilerTest, CodeGenRootSigProfile5) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;
  CodeGenTest(L"rootSigProfile5.hlsl");
}

TEST_F(CompilerTest, CodeGenVectorIsnan) {
  CodeGenTestCheck(L"isnan_vector_argument.hlsl");
}

TEST_F(CompilerTest, CodeGenVectorAtan2) {
  CodeGenTestCheck(L"atan2_vector_argument.hlsl");
}

TEST_F(CompilerTest, LibGVStore) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcContainerReflection> pReflection;
  CComPtr<IDxcAssembler> pAssembler;

  VERIFY_SUCCEEDED(this->m_dllSupport.CreateInstance(
      CLSID_DxcContainerReflection, &pReflection));
  VERIFY_SUCCEEDED(
      this->m_dllSupport.CreateInstance(CLSID_DxcAssembler, &pAssembler));
  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(
      R"(
      struct T {
      RWByteAddressBuffer outputBuffer;
      RWByteAddressBuffer outputBuffer2;
      };

      struct D {
        float4 a;
        int4 b;
      };

      struct T2 {
         RWStructuredBuffer<D> uav;
      };

      T2 resStruct(T t, uint2 id);

      RWByteAddressBuffer outputBuffer;
      RWByteAddressBuffer outputBuffer2;

      [shader("compute")]
      [numthreads(8, 8, 1)]
      void main( uint2 id : SV_DispatchThreadID )
      {
          T t = {outputBuffer,outputBuffer2};
          T2 t2 = resStruct(t, id);
          uint counter = t2.uav.IncrementCounter();
          t2.uav[counter].b.xy = id;
      }
    )",
      &pSource);

  const WCHAR *pArgs[] = {
      L"/Zi",
  };
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"file.hlsl", L"", L"lib_6_x",
                                      pArgs, _countof(pArgs), nullptr, 0,
                                      nullptr, &pResult));

  CComPtr<IDxcBlob> pShader;
  VERIFY_SUCCEEDED(pResult->GetResult(&pShader));
  VERIFY_SUCCEEDED(pReflection->Load(pShader));

  UINT32 index = 0;
  VERIFY_SUCCEEDED(pReflection->FindFirstPartKind(hlsl::DFCC_DXIL, &index));

  CComPtr<IDxcBlob> pBitcode;
  VERIFY_SUCCEEDED(pReflection->GetPartContent(index, &pBitcode));

  const char *bitcode = hlsl::GetDxilBitcodeData(
      (hlsl::DxilProgramHeader *)pBitcode->GetBufferPointer());
  unsigned bitcode_size = hlsl::GetDxilBitcodeSize(
      (hlsl::DxilProgramHeader *)pBitcode->GetBufferPointer());

  CComPtr<IDxcBlobEncoding> pBitcodeBlob;
  CreateBlobPinned(bitcode, bitcode_size, DXC_CP_ACP, &pBitcodeBlob);

  CComPtr<IDxcBlob> pReassembled;
  CComPtr<IDxcOperationResult> pReassembleResult;
  VERIFY_SUCCEEDED(
      pAssembler->AssembleToContainer(pBitcodeBlob, &pReassembleResult));
  VERIFY_SUCCEEDED(pReassembleResult->GetResult(&pReassembled));

  CComPtr<IDxcBlobEncoding> pTextBlob;
  VERIFY_SUCCEEDED(pCompiler->Disassemble(pReassembled, &pTextBlob));

  std::wstring Text = BlobToWide(pTextBlob);
  VERIFY_ARE_NOT_EQUAL(std::wstring::npos, Text.find(L"store"));
}

TEST_F(CompilerTest, PreprocessWhenValidThenOK) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  DxcDefine defines[2];
  defines[0].Name = L"MYDEF";
  defines[0].Value = L"int";
  defines[1].Name = L"MYOTHERDEF";
  defines[1].Value = L"123";
  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("// First line\r\n"
                     "MYDEF g_int = MYOTHERDEF;\r\n"
                     "#define FOO BAR\r\n"
                     "int FOO;",
                     &pSource);
  VERIFY_SUCCEEDED(pCompiler->Preprocess(pSource, L"file.hlsl", nullptr, 0,
                                         defines, _countof(defines), nullptr,
                                         &pResult));
  HRESULT hrOp;
  VERIFY_SUCCEEDED(pResult->GetStatus(&hrOp));
  VERIFY_SUCCEEDED(hrOp);

  CComPtr<IDxcBlob> pOutText;
  VERIFY_SUCCEEDED(pResult->GetResult(&pOutText));
  std::string text(BlobToUtf8(pOutText));
  VERIFY_ARE_EQUAL_STR("#line 1 \"file.hlsl\"\n"
                       "\n"
                       "int g_int = 123;\n"
                       "\n"
                       "int BAR;\n",
                       text.c_str());
}

TEST_F(CompilerTest, PreprocessWhenExpandTokenPastingOperandThenAccept) {
  // Tests that we can turn on fxc's behavior (pre-expanding operands before
  // performing token-pasting) using -flegacy-macro-expansion

  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;

  LPCWSTR expandOption = L"-flegacy-macro-expansion";

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));

  CreateBlobFromText(R"(
#define SET_INDEX0                10
#define BINDING_INDEX0            5

#define SET(INDEX)                SET_INDEX##INDEX
#define BINDING(INDEX)            BINDING_INDEX##INDEX

#define SET_BIND(NAME,SET,BIND)   resource_set_##SET##_bind_##BIND##_##NAME

#define RESOURCE(NAME,INDEX)      SET_BIND(NAME, SET(INDEX), BINDING(INDEX))

    Texture2D<float4> resource_set_10_bind_5_tex;

  float4 main() : SV_Target{
    return RESOURCE(tex, 0)[uint2(1, 2)];
  }
)",
                     &pSource);
  VERIFY_SUCCEEDED(pCompiler->Preprocess(pSource, L"file.hlsl", &expandOption,
                                         1, nullptr, 0, nullptr, &pResult));
  HRESULT hrOp;
  VERIFY_SUCCEEDED(pResult->GetStatus(&hrOp));
  VERIFY_SUCCEEDED(hrOp);

  CComPtr<IDxcBlob> pOutText;
  VERIFY_SUCCEEDED(pResult->GetResult(&pOutText));
  std::string text(BlobToUtf8(pOutText));
  VERIFY_ARE_EQUAL_STR("#line 1 \"file.hlsl\"\n"
                       "#line 12 \"file.hlsl\"\n"
                       "    Texture2D<float4> resource_set_10_bind_5_tex;\n"
                       "\n"
                       "  float4 main() : SV_Target{\n"
                       "    return resource_set_10_bind_5_tex[uint2(1, 2)];\n"
                       "  }\n",
                       text.c_str());
}

TEST_F(CompilerTest, PreprocessWithDebugOptsThenOk) {
  // Make sure debug options, such as -Zi and -Fd,
  // are simply ignored when preprocessing

  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  DxcDefine defines[2];
  defines[0].Name = L"MYDEF";
  defines[0].Value = L"int";
  defines[1].Name = L"MYOTHERDEF";
  defines[1].Value = L"123";
  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("// First line\r\n"
                     "MYDEF g_int = MYOTHERDEF;\r\n"
                     "#define FOO BAR\r\n"
                     "int FOO;",
                     &pSource);

  LPCWSTR extraOptions[] = {L"-Zi", L"-Fd", L"file.pdb", L"-Qembed_debug"};

  VERIFY_SUCCEEDED(pCompiler->Preprocess(pSource, L"file.hlsl", extraOptions,
                                         _countof(extraOptions), defines,
                                         _countof(defines), nullptr, &pResult));
  HRESULT hrOp;
  VERIFY_SUCCEEDED(pResult->GetStatus(&hrOp));
  VERIFY_SUCCEEDED(hrOp);

  CComPtr<IDxcBlob> pOutText;
  VERIFY_SUCCEEDED(pResult->GetResult(&pOutText));
  std::string text(BlobToUtf8(pOutText));
  VERIFY_ARE_EQUAL_STR("#line 1 \"file.hlsl\"\n"
                       "\n"
                       "int g_int = 123;\n"
                       "\n"
                       "int BAR;\n",
                       text.c_str());
}

// Make sure that '#line 1 "<built-in>"' won't blow up when preprocessing.
TEST_F(CompilerTest, PreprocessCheckBuiltinIsOk) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("#line 1 \"<built-in>\"\r\n"
                     "int x;",
                     &pSource);
  VERIFY_SUCCEEDED(pCompiler->Preprocess(pSource, L"file.hlsl", nullptr, 0,
                                         nullptr, 0, nullptr, &pResult));
  HRESULT hrOp;
  VERIFY_SUCCEEDED(pResult->GetStatus(&hrOp));
  VERIFY_SUCCEEDED(hrOp);

  CComPtr<IDxcBlob> pOutText;
  VERIFY_SUCCEEDED(pResult->GetResult(&pOutText));
  std::string text(BlobToUtf8(pOutText));
  VERIFY_ARE_EQUAL_STR("#line 1 \"file.hlsl\"\n\n", text.c_str());
}

TEST_F(CompilerTest, CompileOtherModesWithDebugOptsThenOk) {
  // Make sure debug options, such as -Zi and -Fd,
  // are simply ignored when compiling in modes:
  // /Odump -ast-dump -fcgl -rootsig_1_0

  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcBlobEncoding> pSource;
  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText("#define RS \"CBV(b0)\"\n"
                     "[RootSignature(RS)]\n"
                     "float main(float i : IN) : OUT { return i * 2.0F; }",
                     &pSource);

  auto testWithOpts = [&](LPCWSTR entry, LPCWSTR target,
                          llvm::ArrayRef<LPCWSTR> mainOpts) -> HRESULT {
    std::vector<LPCWSTR> opts(mainOpts);
    opts.insert(opts.end(), {L"-Zi", L"-Fd", L"file.pdb"});
    CComPtr<IDxcOperationResult> pResult;
    VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"file.hlsl", entry, target,
                                        opts.data(), opts.size(), nullptr, 0,
                                        nullptr, &pResult));
    HRESULT hrOp;
    VERIFY_SUCCEEDED(pResult->GetStatus(&hrOp));
    return hrOp;
  };
  VERIFY_SUCCEEDED(testWithOpts(L"main", L"vs_6_0", {L"/Odump"}));
  VERIFY_SUCCEEDED(testWithOpts(L"main", L"vs_6_0", {L"-ast-dump"}));
  VERIFY_SUCCEEDED(testWithOpts(L"main", L"vs_6_0", {L"-fcgl"}));
  VERIFY_SUCCEEDED(testWithOpts(L"RS", L"rootsig_1_0", {}));
}

TEST_F(CompilerTest, WhenSigMismatchPCFunctionThenFail) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;

  VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));
  CreateBlobFromText(
      "struct PSSceneIn \n\
    { \n\
      float4 pos  : SV_Position; \n\
      float2 tex  : TEXCOORD0; \n\
      float3 norm : NORMAL; \n\
    }; \n"
      "struct HSPerPatchData {  \n\
      float edges[ 3 ] : SV_TessFactor; \n\
      float inside : SV_InsideTessFactor; \n\
      float foo : FOO; \n\
    }; \n"
      "HSPerPatchData HSPerPatchFunc( InputPatch< PSSceneIn, 3 > points, \n\
      OutputPatch<PSSceneIn, 3> outpoints) { \n\
      HSPerPatchData d = (HSPerPatchData)0; \n\
      d.edges[ 0 ] = points[0].tex.x + outpoints[0].tex.x; \n\
      d.edges[ 1 ] = 1; \n\
      d.edges[ 2 ] = 1; \n\
      d.inside = 1; \n\
      return d; \n\
    } \n"
      "[domain(\"tri\")] \n\
    [partitioning(\"fractional_odd\")] \n\
    [outputtopology(\"triangle_cw\")] \n\
    [patchconstantfunc(\"HSPerPatchFunc\")] \n\
    [outputcontrolpoints(3)] \n"
      "void main(const uint id : SV_OutputControlPointID, \n\
               const InputPatch< PSSceneIn, 3 > points ) { \n\
    } \n",
      &pSource);

  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"main",
                                      L"hs_6_0", nullptr, 0, nullptr, 0,
                                      nullptr, &pResult));
  std::string failLog(VerifyOperationFailed(pResult));
  VERIFY_ARE_NOT_EQUAL(
      string::npos,
      failLog.find(
          "Signature element SV_Position, referred to by patch constant "
          "function, is not found in corresponding hull shader output."));
}

TEST_F(CompilerTest, SubobjectCodeGenErrors) {
  struct SubobjectErrorTestCase {
    const char *shaderText;
    const char *expectedError;
  };
  SubobjectErrorTestCase testCases[] = {
      {"GlobalRootSignature grs;",
       "1:1: error: subobject needs to be initialized"},
      {"StateObjectConfig soc;",
       "1:1: error: subobject needs to be initialized"},
      {"LocalRootSignature lrs;",
       "1:1: error: subobject needs to be initialized"},
      {"SubobjectToExportsAssociation sea;",
       "1:1: error: subobject needs to be initialized"},
      {"RaytracingShaderConfig rsc;",
       "1:1: error: subobject needs to be initialized"},
      {"RaytracingPipelineConfig rpc;",
       "1:1: error: subobject needs to be initialized"},
      {"RaytracingPipelineConfig1 rpc1;",
       "1:1: error: subobject needs to be initialized"},
      {"TriangleHitGroup hitGt;",
       "1:1: error: subobject needs to be initialized"},
      {"ProceduralPrimitiveHitGroup hitGt;",
       "1:1: error: subobject needs to be initialized"},
      {"GlobalRootSignature grs2 = {\"\"};",
       "1:29: error: empty string not expected here"},
      {"LocalRootSignature lrs2 = {\"\"};",
       "1:28: error: empty string not expected here"},
      {"SubobjectToExportsAssociation sea2 = { \"\", \"x\" };",
       "1:40: error: empty string not expected here"},
      {"string s; SubobjectToExportsAssociation sea4 = { \"x\", s };",
       "1:55: error: cannot convert to constant string"},
      {"extern int v; RaytracingPipelineConfig rpc2 = { v + 16 };",
       "1:49: error: cannot convert to constant unsigned int"},
      {"string s; TriangleHitGroup trHitGt2_8 = { s, \"foo\" };",
       "1:43: error: cannot convert to constant string"},
      {"string s; ProceduralPrimitiveHitGroup ppHitGt2_8 = { s, \"\", s };",
       "1:54: error: cannot convert to constant string"},
      {"ProceduralPrimitiveHitGroup ppHitGt2_9 = { \"a\", \"b\", \"\"};",
       "1:54: error: empty string not expected here"}};

  for (unsigned i = 0; i < _countof(testCases); i++) {
    CComPtr<IDxcCompiler> pCompiler;
    CComPtr<IDxcOperationResult> pResult;
    CComPtr<IDxcBlobEncoding> pSource;
    VERIFY_SUCCEEDED(CreateCompiler(&pCompiler));

    CreateBlobFromText(testCases[i].shaderText, &pSource);
    VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"",
                                        L"lib_6_4", nullptr, 0, nullptr, 0,
                                        nullptr, &pResult));
    std::string failLog(VerifyOperationFailed(pResult));
    VERIFY_ARE_NOT_EQUAL(string::npos,
                         failLog.find(testCases[i].expectedError));
  }
}

#ifdef _WIN32
TEST_F(CompilerTest, ManualFileCheckTest) {
#else
TEST_F(CompilerTest, DISABLED_ManualFileCheckTest) {
#endif
  using namespace llvm;
  using namespace WEX::TestExecution;

  WEX::Common::String value;
  VERIFY_SUCCEEDED(RuntimeParameters::TryGetValue(L"InputPath", value));

  std::wstring path = static_cast<const wchar_t *>(value);
  if (!llvm::sys::path::is_absolute(CW2A(path.c_str()).m_psz)) {
    path = hlsl_test::GetPathToHlslDataFile(path.c_str());
  }

  bool isDirectory;
  {
    // Temporarily setup the filesystem for testing whether the path is a
    // directory. If it is, CodeGenTestCheckBatchDir will create its own
    // instance.
    llvm::sys::fs::MSFileSystem *msfPtr;
    VERIFY_SUCCEEDED(CreateMSFileSystemForDisk(&msfPtr));
    std::unique_ptr<llvm::sys::fs::MSFileSystem> msf(msfPtr);
    llvm::sys::fs::AutoPerThreadSystem pts(msf.get());
    IFTLLVM(pts.error_code());
    isDirectory = llvm::sys::fs::is_directory(CW2A(path.c_str()).m_psz);
  }

  if (isDirectory) {
    CodeGenTestCheckBatchDir(path, /*implicitDir*/ false);
  } else {
    CodeGenTestCheck(path.c_str(), /*implicitDir*/ false);
  }
}

TEST_F(CompilerTest, CodeGenHashStabilityD3DReflect) {
  CodeGenTestCheckBatchHash(L"d3dreflect");
}

TEST_F(CompilerTest, CodeGenHashStabilityDisassembler) {
  CodeGenTestCheckBatchHash(L"disassembler");
}

TEST_F(CompilerTest, CodeGenHashStabilityDXIL) {
  CodeGenTestCheckBatchHash(L"dxil");
}

TEST_F(CompilerTest, CodeGenHashStabilityHLSL) {
  CodeGenTestCheckBatchHash(L"hlsl");
}

TEST_F(CompilerTest, CodeGenHashStabilityInfra) {
  CodeGenTestCheckBatchHash(L"infra");
}

TEST_F(CompilerTest, CodeGenHashStabilityPIX) {
  CodeGenTestCheckBatchHash(L"pix");
}

TEST_F(CompilerTest, CodeGenHashStabilityRewriter) {
  CodeGenTestCheckBatchHash(L"rewriter");
}

TEST_F(CompilerTest, CodeGenHashStabilitySamples) {
  CodeGenTestCheckBatchHash(L"samples");
}

TEST_F(CompilerTest, CodeGenHashStabilityShaderTargets) {
  CodeGenTestCheckBatchHash(L"shader_targets");
}

TEST_F(CompilerTest, CodeGenHashStabilityValidation) {
  CodeGenTestCheckBatchHash(L"validation");
}

TEST_F(CompilerTest, BatchD3DReflect) {
  CodeGenTestCheckBatchDir(L"d3dreflect");
}

TEST_F(CompilerTest, BatchDxil) { CodeGenTestCheckBatchDir(L"dxil"); }

TEST_F(CompilerTest, BatchHLSL) { CodeGenTestCheckBatchDir(L"hlsl"); }

TEST_F(CompilerTest, BatchInfra) { CodeGenTestCheckBatchDir(L"infra"); }

TEST_F(CompilerTest, BatchPasses) { CodeGenTestCheckBatchDir(L"passes"); }

TEST_F(CompilerTest, BatchShaderTargets) {
  CodeGenTestCheckBatchDir(L"shader_targets");
}

TEST_F(CompilerTest, BatchValidation) {
  CodeGenTestCheckBatchDir(L"validation");
}

TEST_F(CompilerTest, BatchPIX) { CodeGenTestCheckBatchDir(L"pix"); }

TEST_F(CompilerTest, BatchSamples) { CodeGenTestCheckBatchDir(L"samples"); }
