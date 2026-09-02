///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// LinkerTest.cpp                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Test/CompilationResult.h"
#include "dxc/Test/HLSLTestData.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/ManagedStatic.h"
#include <memory>
#include <string>
#include <vector>

#include <fstream>

#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/Global.h" // for IFT macro
#include "dxc/Test/DxcTestUtils.h"
#include "dxc/Test/HlslTestUtils.h"
#include "dxc/dxcapi.h"

using namespace std;
using namespace hlsl;
using namespace llvm;

// The test fixture.
#ifdef _WIN32
class LinkerTest {
#else
class LinkerTest : public ::testing::Test {
#endif
public:
  BEGIN_TEST_CLASS(LinkerTest)
  TEST_CLASS_PROPERTY(L"Parallel", L"true")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_CLASS_SETUP(InitSupport)

  TEST_METHOD(RunLinkResource)
  TEST_METHOD(RunLinkModulesDifferentVersions)
  TEST_METHOD(RunLinkResourceWithBinding)
  TEST_METHOD(RunLinkAllProfiles)
  TEST_METHOD(RunLinkFailNoDefine)
  TEST_METHOD(RunLinkFailReDefine)
  TEST_METHOD(RunLinkGlobalInit)
  TEST_METHOD(RunLinkNoAlloca)
  TEST_METHOD(RunLinkMatArrayParam)
  TEST_METHOD(RunLinkMatParam)
  TEST_METHOD(RunLinkMatParamToLib)
  TEST_METHOD(RunLinkResRet)
  TEST_METHOD(RunLinkToLib)
  TEST_METHOD(RunLinkToLibOdNops)
  TEST_METHOD(RunLinkToLibExport)
  TEST_METHOD(RunLinkToLibExportShadersOnly)
  TEST_METHOD(RunLinkFailReDefineGlobal)
  TEST_METHOD(RunLinkFailProfileMismatch)
  TEST_METHOD(RunLinkFailEntryNoProps)
  TEST_METHOD(RunLinkFailSelectRes)
  TEST_METHOD(RunLinkToLibWithUnresolvedFunctions)
  TEST_METHOD(RunLinkToLibWithUnresolvedFunctionsExports)
  TEST_METHOD(RunLinkToLibWithExportNamesSwapped)
  TEST_METHOD(RunLinkToLibWithExportCollision)
  TEST_METHOD(RunLinkToLibWithUnusedExport)
  TEST_METHOD(RunLinkToLibWithNoExports)
  TEST_METHOD(RunLinkWithPotentialIntrinsicNameCollisions)
  TEST_METHOD(RunLinkWithValidatorVersion)
  TEST_METHOD(RunLinkWithInvalidValidatorVersion)
  TEST_METHOD(RunLinkWithTempReg)
  TEST_METHOD(RunLinkToLibWithGlobalCtor)
  TEST_METHOD(LinkSm63ToSm66)
  TEST_METHOD(RunLinkWithRootSig)
  TEST_METHOD(RunLinkWithDxcResultOutputs)
  TEST_METHOD(RunLinkWithDxcResultNames)
  TEST_METHOD(RunLinkWithDxcResultRdat)
  TEST_METHOD(RunLinkWithDxcResultErrors)

  dxc::DxCompilerDllLoader m_dllSupport;
  VersionSupportInfo m_ver;

  void CreateLinker(IDxcLinker **pResultLinker) {
    VERIFY_SUCCEEDED(
        m_dllSupport.CreateInstance(CLSID_DxcLinker, pResultLinker));
  }

  void Compile(LPCWSTR filename, IDxcBlob **pResultBlob,
               llvm::ArrayRef<LPCWSTR> pArguments = {}, LPCWSTR pEntry = L"",
               LPCWSTR pShaderTarget = L"lib_6_x") {
    std::wstring fullPath = hlsl_test::GetPathToHlslDataFile(filename);
    CComPtr<IDxcBlobEncoding> pSource;
    CComPtr<IDxcLibrary> pLibrary;
    VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));

    VERIFY_SUCCEEDED(
        pLibrary->CreateBlobFromFile(fullPath.c_str(), nullptr, &pSource));

    CComPtr<IDxcIncludeHandler> pIncludeHandler;
    VERIFY_SUCCEEDED(pLibrary->CreateIncludeHandler(&pIncludeHandler));

    CComPtr<IDxcCompiler> pCompiler;
    CComPtr<IDxcOperationResult> pResult;
    CComPtr<IDxcBlob> pProgram;

    VERIFY_SUCCEEDED(
        m_dllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));
    VERIFY_SUCCEEDED(pCompiler->Compile(
        pSource, fullPath.c_str(), pEntry, pShaderTarget,
        const_cast<LPCWSTR *>(pArguments.data()), pArguments.size(), nullptr, 0,
        pIncludeHandler, &pResult));
    CheckOperationSucceeded(pResult, pResultBlob);
  }

  void CompileLib(LPCWSTR filename, IDxcBlob **pResultBlob,
                  llvm::ArrayRef<LPCWSTR> pArguments = {},
                  LPCWSTR pShaderTarget = L"lib_6_x") {
    Compile(filename, pResultBlob, pArguments, L"", pShaderTarget);
  }

  void AssembleLib(LPCWSTR filename, IDxcBlob **pResultBlob) {
    std::wstring fullPath = hlsl_test::GetPathToHlslDataFile(filename);
    CComPtr<IDxcLibrary> pLibrary;
    VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
    CComPtr<IDxcBlobEncoding> pSource;
    VERIFY_SUCCEEDED(
        pLibrary->CreateBlobFromFile(fullPath.c_str(), nullptr, &pSource));
    CComPtr<IDxcAssembler> pAssembler;
    VERIFY_SUCCEEDED(
        m_dllSupport.CreateInstance(CLSID_DxcAssembler, &pAssembler));
    CComPtr<IDxcOperationResult> pResult;
    VERIFY_SUCCEEDED(pAssembler->AssembleToContainer(pSource, &pResult));
    CheckOperationSucceeded(pResult, pResultBlob);
  }

  void RegisterDxcModule(LPCWSTR pLibName, IDxcBlob *pBlob,
                         IDxcLinker *pLinker) {
    VERIFY_SUCCEEDED(pLinker->RegisterLibrary(pLibName, pBlob));
  }

  void Link(LPCWSTR pEntryName, LPCWSTR pShaderModel, IDxcLinker *pLinker,
            ArrayRef<LPCWSTR> libNames, llvm::ArrayRef<LPCSTR> pCheckMsgs,
            llvm::ArrayRef<LPCSTR> pCheckNotMsgs,
            llvm::ArrayRef<LPCWSTR> pArguments = {}, bool bRegEx = false,
            IDxcResult **ppResult = nullptr) {
    CComPtr<IDxcOperationResult> pResult;
    VERIFY_SUCCEEDED(pLinker->Link(pEntryName, pShaderModel, libNames.data(),
                                   libNames.size(), pArguments.data(),
                                   pArguments.size(), &pResult));
    CComPtr<IDxcBlob> pProgram;
    CheckOperationSucceeded(pResult, &pProgram);

    CComPtr<IDxcCompiler> pCompiler;
    CComPtr<IDxcBlobEncoding> pDisassembly;

    VERIFY_SUCCEEDED(
        m_dllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));
    VERIFY_SUCCEEDED(pCompiler->Disassemble(pProgram, &pDisassembly));
    std::string IR = BlobToUtf8(pDisassembly);
    CheckMsgs(IR.c_str(), IR.size(), pCheckMsgs.data(), pCheckMsgs.size(),
              bRegEx);
    CheckNotMsgs(IR.c_str(), IR.size(), pCheckNotMsgs.data(),
                 pCheckNotMsgs.size(), bRegEx);

    if (ppResult)
      VERIFY_SUCCEEDED(pResult->QueryInterface(ppResult));
  }

  void LinkCheckMsg(LPCWSTR pEntryName, LPCWSTR pShaderModel,
                    IDxcLinker *pLinker, ArrayRef<LPCWSTR> libNames,
                    llvm::ArrayRef<LPCSTR> pErrorMsgs,
                    llvm::ArrayRef<LPCWSTR> pArguments = {},
                    bool bRegex = false, IDxcResult **ppResult = nullptr) {
    CComPtr<IDxcOperationResult> pResult;
    VERIFY_SUCCEEDED(pLinker->Link(pEntryName, pShaderModel, libNames.data(),
                                   libNames.size(), pArguments.data(),
                                   pArguments.size(), &pResult));
    CheckOperationResultMsgs(pResult, pErrorMsgs.data(), pErrorMsgs.size(),
                             false, bRegex);

    if (ppResult)
      VERIFY_SUCCEEDED(pResult->QueryInterface(ppResult));
  }
};

bool LinkerTest::InitSupport() {
  if (!m_dllSupport.IsEnabled()) {
    VERIFY_SUCCEEDED(m_dllSupport.Initialize());
    m_ver.Initialize(m_dllSupport);
  }

  return true;
}

TEST_F(LinkerTest, RunLinkResource) {
  CComPtr<IDxcBlob> pResLib;
  CompileLib(L"..\\CodeGenHLSL\\lib_resource2.hlsl", &pResLib);
  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\lib_cs_entry.hlsl", &pEntryLib);
  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);
  LPCWSTR libName = L"entry";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LPCWSTR libResName = L"res";
  RegisterDxcModule(libResName, pResLib, pLinker);

  Link(L"entry", L"cs_6_0", pLinker, {libResName, libName}, {}, {});
}

TEST_F(LinkerTest, RunLinkResourceWithBinding) {
  // These two libraries both have a ConstantBuffer resource named g_buf.
  // These are explicitly bound to different slots, and the types don't match.
  // This test runs a pass to rename resources to prevent merging of resource
  // globals. Then tests linking these, which requires dxil op overload renaming
  // because of a typename collision between the two libraries.
  CComPtr<IDxcBlob> pLib1;
  CompileLib(L"..\\CodeGenHLSL\\lib_res_bound1.hlsl", &pLib1);
  CComPtr<IDxcBlob> pLib2;
  CompileLib(L"..\\CodeGenHLSL\\lib_res_bound2.hlsl", &pLib2);

  LPCWSTR optOptions[] = {
      L"-dxil-rename-resources,prefix=lib1",
      L"-dxil-rename-resources,prefix=lib2",
  };

  CComPtr<IDxcOptimizer> pOptimizer;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcOptimizer, &pOptimizer));

  CComPtr<IDxcContainerReflection> pContainerReflection;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcContainerReflection,
                                               &pContainerReflection));
  UINT32 partIdx = 0;
  VERIFY_SUCCEEDED(pContainerReflection->Load(pLib1));
  VERIFY_SUCCEEDED(
      pContainerReflection->FindFirstPartKind(DXC_PART_DXIL, &partIdx));
  CComPtr<IDxcBlob> pLib1Module;
  VERIFY_SUCCEEDED(pContainerReflection->GetPartContent(partIdx, &pLib1Module));

  CComPtr<IDxcBlob> pLib1ModuleRenamed;
  VERIFY_SUCCEEDED(pOptimizer->RunOptimizer(pLib1Module, &optOptions[0], 1,
                                            &pLib1ModuleRenamed, nullptr));
  pLib1Module.Release();
  pLib1.Release();
  AssembleToContainer(m_dllSupport, pLib1ModuleRenamed, &pLib1);

  VERIFY_SUCCEEDED(pContainerReflection->Load(pLib2));
  VERIFY_SUCCEEDED(
      pContainerReflection->FindFirstPartKind(DXC_PART_DXIL, &partIdx));
  CComPtr<IDxcBlob> pLib2Module;
  VERIFY_SUCCEEDED(pContainerReflection->GetPartContent(partIdx, &pLib2Module));

  CComPtr<IDxcBlob> pLib2ModuleRenamed;
  VERIFY_SUCCEEDED(pOptimizer->RunOptimizer(pLib2Module, &optOptions[1], 1,
                                            &pLib2ModuleRenamed, nullptr));
  pLib2Module.Release();
  pLib2.Release();
  AssembleToContainer(m_dllSupport, pLib2ModuleRenamed, &pLib2);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);
  LPCWSTR lib1Name = L"lib1";
  RegisterDxcModule(lib1Name, pLib1, pLinker);

  LPCWSTR lib2Name = L"lib2";
  RegisterDxcModule(lib2Name, pLib2, pLinker);

  Link(L"main", L"cs_6_0", pLinker, {lib1Name, lib2Name}, {}, {});
}

TEST_F(LinkerTest, RunLinkAllProfiles) {
  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"entry";
  LPCWSTR option[] = {L"-Zi", L"-Qembed_debug"};

  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\lib_entries2.hlsl", &pEntryLib, option);
  RegisterDxcModule(libName, pEntryLib, pLinker);

  Link(L"vs_main", L"vs_6_0", pLinker, {libName}, {}, {});
  Link(L"hs_main", L"hs_6_0", pLinker, {libName}, {}, {});
  Link(L"ds_main", L"ds_6_0", pLinker, {libName}, {}, {});
  Link(L"gs_main", L"gs_6_0", pLinker, {libName}, {}, {});
  Link(L"ps_main", L"ps_6_0", pLinker, {libName}, {}, {});

  CComPtr<IDxcBlob> pResLib;
  CompileLib(L"..\\CodeGenHLSL\\lib_resource2.hlsl", &pResLib);

  LPCWSTR libResName = L"res";
  RegisterDxcModule(libResName, pResLib, pLinker);
  Link(L"cs_main", L"cs_6_0", pLinker, {libName, libResName}, {}, {});
}

TEST_F(LinkerTest, RunLinkModulesDifferentVersions) {
  CComPtr<IDxcLinker> pLinker1, pLinker2, pLinker3;
  CreateLinker(&pLinker1);
  CreateLinker(&pLinker2);
  CreateLinker(&pLinker3);

  LPCWSTR libName = L"entry";
  LPCWSTR option[] = {L"-Zi", L"-Qembed_debug"};
  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\lib_entries2.hlsl", &pEntryLib, option);

  // the 2nd blob is the "good" blob that stays constant
  CComPtr<IDxcBlob> pResLib;
  CompileLib(L"..\\CodeGenHLSL\\lib_resource2.hlsl", &pResLib);
  LPCWSTR libResName = L"res";

  // modify the first compiled blob to force it to have a different compiler
  // version
  CComPtr<IDxcContainerReflection> containerReflection;
  IFT(m_dllSupport.CreateInstance(CLSID_DxcContainerReflection,
                                  &containerReflection));
  IFT(containerReflection->Load(pEntryLib));
  UINT part_index;
  VERIFY_SUCCEEDED(containerReflection->FindFirstPartKind(
      hlsl::DFCC_CompilerVersion, &part_index));
  CComPtr<IDxcBlob> pBlob;
  IFT(containerReflection->GetPartContent(part_index, &pBlob));
  void *pBlobPtr = pBlob->GetBufferPointer();

  std::string commonMismatchStr =
      "error: Cannot link libraries with conflicting compiler versions.";

  hlsl::DxilCompilerVersion *pDCV = (hlsl::DxilCompilerVersion *)pBlobPtr;
  if (pDCV->VersionStringListSizeInBytes) {
    // first just get both strings, the compiler version and the commit hash
    char *pVersionStringListPtr =
        (char *)pBlobPtr + sizeof(hlsl::DxilCompilerVersion);
    std::string commitHashStr(pVersionStringListPtr);
    std::string oldCommitHashStr = commitHashStr;
    std::string compilerVersionStr(pVersionStringListPtr +
                                   commitHashStr.size() + 1);
    std::string oldCompilerVersionStr = compilerVersionStr;

    // flip a character to change the commit hash
    *pVersionStringListPtr = commitHashStr[0] == 'a' ? 'b' : 'a';
    // store the modified compiler version part.
    RegisterDxcModule(libName, pEntryLib, pLinker1);
    // and the "good" version part
    RegisterDxcModule(libResName, pResLib, pLinker1);
    // 2 blobs with different compiler versions should not be linkable
    LinkCheckMsg(L"hs_main", L"hs_6_1", pLinker1, {libName, libResName},
                 {commonMismatchStr.c_str()});

    // reset the modified part back to normal
    *pVersionStringListPtr = oldCommitHashStr[0];

    // flip a character to change the compiler version
    *(pVersionStringListPtr + commitHashStr.size() + 1) =
        compilerVersionStr[0] == '1' ? '2' : '1';
    // store the modified compiler version part.
    RegisterDxcModule(libName, pEntryLib, pLinker2);
    // and the "good" version part
    RegisterDxcModule(libResName, pResLib, pLinker2);
    // 2 blobs with different compiler versions should not be linkable
    LinkCheckMsg(L"hs_main", L"hs_6_1", pLinker2, {libName, libResName},
                 {commonMismatchStr.c_str()});

    // reset the modified part back to normal
    *(pVersionStringListPtr + commitHashStr.size() + 1) =
        oldCompilerVersionStr[0];
  } else {
    // The blob can't be changed by adding data, since the
    // the data after this header could be a subsequent header.
    // The test should announce that it can't make any version string
    // modifications
#if _WIN32
    WEX::Logging::Log::Warning(
        L"Cannot modify compiler version string for test");
#else
    FAIL() << "Cannot modify compiler version string for test";
#endif
  }

  // finally, test that a difference is detected if a member of the struct, say
  // the major member, is different.
  pDCV->Major = pDCV->Major == 2 ? 1 : 2;
  // store the modified compiler version part.
  RegisterDxcModule(libName, pEntryLib, pLinker3);
  // and the "good" version part
  RegisterDxcModule(libResName, pResLib, pLinker3);
  // 2 blobs with different compiler versions should not be linkable
  LinkCheckMsg(L"hs_main", L"hs_6_1", pLinker3, {libName, libResName},
               {commonMismatchStr.c_str()});
}

TEST_F(LinkerTest, RunLinkFailNoDefine) {
  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\lib_cs_entry.hlsl", &pEntryLib);
  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"entry";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LinkCheckMsg(L"entry", L"cs_6_0", pLinker, {libName},
               {"Cannot find definition of function"});
}

TEST_F(LinkerTest, RunLinkFailReDefine) {
  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\lib_cs_entry.hlsl", &pEntryLib);
  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"entry";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LPCWSTR libName2 = L"entry2";
  RegisterDxcModule(libName2, pEntryLib, pLinker);

  LinkCheckMsg(L"entry", L"cs_6_0", pLinker, {libName, libName2},
               {"Definition already exists for function"});
}

TEST_F(LinkerTest, RunLinkGlobalInit) {
  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\lib_global.hlsl", &pEntryLib, {}, L"lib_6_3");
  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"entry";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  Link(L"test", L"ps_6_0", pLinker, {libName},
       // Make sure cbuffer load is generated.
       {"dx.op.cbufferLoad"}, {});
}

TEST_F(LinkerTest, RunLinkFailReDefineGlobal) {
  LPCWSTR option[] = {L"-default-linkage", L"external"};
  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\lib_global2.hlsl", &pEntryLib, option,
             L"lib_6_3");

  CComPtr<IDxcBlob> pLib0;
  CompileLib(L"..\\CodeGenHLSL\\lib_global3.hlsl", &pLib0, option, L"lib_6_3");

  CComPtr<IDxcBlob> pLib1;
  CompileLib(L"..\\CodeGenHLSL\\lib_global4.hlsl", &pLib1, option, L"lib_6_3");

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"entry";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LPCWSTR libName1 = L"lib0";
  RegisterDxcModule(libName1, pLib0, pLinker);

  LPCWSTR libName2 = L"lib1";
  RegisterDxcModule(libName2, pLib1, pLinker);

  LinkCheckMsg(L"entry", L"cs_6_0", pLinker, {libName, libName1, libName2},
               {"Definition already exists for global variable",
                "Resource already exists"});
}

TEST_F(LinkerTest, RunLinkFailProfileMismatch) {
  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\lib_global.hlsl", &pEntryLib);
  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"entry";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LinkCheckMsg(L"test", L"cs_6_0", pLinker, {libName},
               {"Profile mismatch between entry function and target profile"});
}

TEST_F(LinkerTest, RunLinkFailEntryNoProps) {
  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\lib_global.hlsl", &pEntryLib);
  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"entry";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LinkCheckMsg(L"\01?update@@YAXXZ", L"cs_6_0", pLinker, {libName},
               {"Cannot find function property for entry function"});
}

TEST_F(LinkerTest, RunLinkNoAlloca) {
  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\lib_no_alloca.hlsl", &pEntryLib);
  CComPtr<IDxcBlob> pLib;
  CompileLib(L"..\\CodeGenHLSL\\lib_no_alloca.h", &pLib);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"ps_main";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LPCWSTR libName2 = L"test";
  RegisterDxcModule(libName2, pLib, pLinker);

  Link(L"ps_main", L"ps_6_0", pLinker, {libName, libName2}, {}, {"alloca"});
}

TEST_F(LinkerTest, RunLinkMatArrayParam) {
  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_mat_entry.hlsl", &pEntryLib);
  CComPtr<IDxcBlob> pLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_mat_cast.hlsl", &pLib);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"ps_main";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LPCWSTR libName2 = L"test";
  RegisterDxcModule(libName2, pLib, pLinker);

  Link(L"main", L"ps_6_0", pLinker, {libName, libName2},
       {"alloca [24 x float]", "getelementptr [12 x float], [12 x float]*"},
       {});

  Link(L"main", L"ps_6_9", pLinker, {libName, libName2},
       {"alloca [2 x <12 x float>]",
        "getelementptr [12 x float], [12 x float]*"},
       {});
}

TEST_F(LinkerTest, RunLinkMatParam) {
  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_mat_entry2.hlsl", &pEntryLib);
  CComPtr<IDxcBlob> pLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_mat_cast2.hlsl", &pLib);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"ps_main";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LPCWSTR libName2 = L"test";
  RegisterDxcModule(libName2, pLib, pLinker);

  Link(L"main", L"ps_6_0", pLinker, {libName, libName2},
       {"alloca [12 x float]"}, {});
}

TEST_F(LinkerTest, RunLinkMatParamToLib) {
  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_mat_entry2.hlsl", &pEntryLib);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"ps_main";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  Link(L"", L"lib_6_3", pLinker, {libName},
       // The bitcast cannot be removed because user function call use it as
       // argument.
       {"bitcast <12 x float>\\* %.* to %class\\.matrix\\.float\\.4\\.3\\*"},
       {}, {}, true);
}

TEST_F(LinkerTest, RunLinkResRet) {
  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_out_param_res.hlsl", &pEntryLib);
  CComPtr<IDxcBlob> pLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_out_param_res_imp.hlsl", &pLib);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"ps_main";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LPCWSTR libName2 = L"test";
  RegisterDxcModule(libName2, pLib, pLinker);

  Link(L"test", L"ps_6_0", pLinker, {libName, libName2}, {}, {"alloca"});
}

TEST_F(LinkerTest, RunLinkToLib) {
  LPCWSTR option[] = {L"-Zi", L"-Qembed_debug"};

  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_mat_entry2.hlsl", &pEntryLib,
             option);
  CComPtr<IDxcBlob> pLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_mat_cast2.hlsl", &pLib, option);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"ps_main";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LPCWSTR libName2 = L"test";
  RegisterDxcModule(libName2, pLib, pLinker);

  Link(L"", L"lib_6_3", pLinker, {libName, libName2}, {"!llvm.dbg.cu"}, {},
       option);
}

TEST_F(LinkerTest, RunLinkToLibOdNops) {
  LPCWSTR option[] = {L"-Od"};

  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_mat_entry2.hlsl", &pEntryLib,
             option);
  CComPtr<IDxcBlob> pLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_mat_cast2.hlsl", &pLib, option);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"ps_main";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LPCWSTR libName2 = L"test";
  RegisterDxcModule(libName2, pLib, pLinker);

  Link(L"", L"lib_6_3", pLinker, {libName, libName2},
       {"load i32, i32* getelementptr inbounds ([1 x i32], [1 x i32]* "
        "@dx.nothing.a, i32 0, i32 0"},
       {}, option);
}

TEST_F(LinkerTest, RunLinkToLibExport) {
  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_mat_entry2.hlsl", &pEntryLib);
  CComPtr<IDxcBlob> pLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_mat_cast2.hlsl", &pLib);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"ps_main";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LPCWSTR libName2 = L"test";
  RegisterDxcModule(libName2, pLib, pLinker);
  Link(L"", L"lib_6_3", pLinker, {libName, libName2},
       {"@\"\\01?renamed_test@@", "@\"\\01?cloned_test@@", "@main"},
       {"@\"\\01?mat_test", "@renamed_test", "@cloned_test"},
       {L"-exports", L"renamed_test,cloned_test=\\01?mat_test@@YA?AV?$vector@M$"
                     L"02@@V?$vector@M$03@@0AIAV?$matrix@M$03$02@@@Z;main"});
}

TEST_F(LinkerTest, RunLinkToLibExportShadersOnly) {
  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_mat_entry2.hlsl", &pEntryLib);
  CComPtr<IDxcBlob> pLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_mat_cast2.hlsl", &pLib);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"ps_main";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LPCWSTR libName2 = L"test";
  RegisterDxcModule(libName2, pLib, pLinker);
  Link(L"", L"lib_6_3", pLinker, {libName, libName2}, {"@main"},
       {"@\"\\01?mat_test"}, {L"-export-shaders-only"});
}

TEST_F(LinkerTest, RunLinkFailSelectRes) {
  if (m_ver.SkipDxilVersion(1, 3))
    return;
  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\lib_select_res_entry.hlsl", &pEntryLib);
  CComPtr<IDxcBlob> pLib;
  CompileLib(L"..\\CodeGenHLSL\\lib_select_res.hlsl", &pLib);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"main";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LPCWSTR libName2 = L"test";
  RegisterDxcModule(libName2, pLib, pLinker);

  LinkCheckMsg(
      L"main", L"ps_6_0", pLinker, {libName, libName2},
      {"local resource not guaranteed to map to unique global resource"});
}

TEST_F(LinkerTest, RunLinkToLibWithUnresolvedFunctions) {
  LPCWSTR option[] = {L"-Zi", L"-Qembed_debug"};

  CComPtr<IDxcBlob> pLib1;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_unresolved_func1.hlsl", &pLib1,
             option);
  CComPtr<IDxcBlob> pLib2;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_unresolved_func2.hlsl", &pLib2,
             option);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName1 = L"lib1";
  RegisterDxcModule(libName1, pLib1, pLinker);

  LPCWSTR libName2 = L"lib2";
  RegisterDxcModule(libName2, pLib2, pLinker);

  Link(
      L"", L"lib_6_3", pLinker, {libName1, libName2},
      {"declare float @\"\\01?external_fn1@@YAMXZ\"()",
       "declare float @\"\\01?external_fn2@@YAMXZ\"()",
       "declare float @\"\\01?external_fn@@YAMXZ\"()",
       "define float @\"\\01?lib1_fn@@YAMXZ\"()",
       "define float @\"\\01?lib2_fn@@YAMXZ\"()",
       "define float @\"\\01?call_lib1@@YAMXZ\"()",
       "define float @\"\\01?call_lib2@@YAMXZ\"()"},
      {"declare float @\"\\01?unused_fn1", "declare float @\"\\01?unused_fn2"});
}

TEST_F(LinkerTest, RunLinkToLibWithUnresolvedFunctionsExports) {
  LPCWSTR option[] = {L"-Zi", L"-Qembed_debug"};

  CComPtr<IDxcBlob> pLib1;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_unresolved_func1.hlsl", &pLib1,
             option);
  CComPtr<IDxcBlob> pLib2;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_unresolved_func2.hlsl", &pLib2,
             option);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName1 = L"lib1";
  RegisterDxcModule(libName1, pLib1, pLinker);

  LPCWSTR libName2 = L"lib2";
  RegisterDxcModule(libName2, pLib2, pLinker);

  Link(L"", L"lib_6_3", pLinker, {libName1, libName2},
       {"declare float @\"\\01?external_fn1@@YAMXZ\"()",
        "declare float @\"\\01?external_fn2@@YAMXZ\"()",
        "declare float @\"\\01?external_fn@@YAMXZ\"()",
        "define float @\"\\01?renamed_lib1@@YAMXZ\"()",
        "define float @\"\\01?call_lib2@@YAMXZ\"()"},
       {"float @\"\\01?unused_fn1", "float @\"\\01?unused_fn2",
        "float @\"\\01?lib1_fn", "float @\"\\01?lib2_fn",
        "float @\"\\01?call_lib1"},
       {L"-exports", L"renamed_lib1=call_lib1", L"-exports", L"call_lib2"});
}

TEST_F(LinkerTest, RunLinkToLibWithExportNamesSwapped) {
  LPCWSTR option[] = {L"-Zi", L"-Qembed_debug"};

  CComPtr<IDxcBlob> pLib1;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_unresolved_func1.hlsl", &pLib1,
             option);
  CComPtr<IDxcBlob> pLib2;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_unresolved_func2.hlsl", &pLib2,
             option);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName1 = L"lib1";
  RegisterDxcModule(libName1, pLib1, pLinker);

  LPCWSTR libName2 = L"lib2";
  RegisterDxcModule(libName2, pLib2, pLinker);

  Link(L"", L"lib_6_3", pLinker, {libName1, libName2},
       {"declare float @\"\\01?external_fn1@@YAMXZ\"()",
        "declare float @\"\\01?external_fn2@@YAMXZ\"()",
        "declare float @\"\\01?external_fn@@YAMXZ\"()",
        "define float @\"\\01?call_lib1@@YAMXZ\"()",
        "define float @\"\\01?call_lib2@@YAMXZ\"()"},
       {"float @\"\\01?unused_fn1", "float @\"\\01?unused_fn2",
        "float @\"\\01?lib1_fn", "float @\"\\01?lib2_fn"},
       {L"-exports", L"call_lib2=call_lib1;call_lib1=call_lib2"});
}

TEST_F(LinkerTest, RunLinkToLibWithExportCollision) {
  LPCWSTR option[] = {L"-Zi", L"-Qembed_debug"};

  CComPtr<IDxcBlob> pLib1;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_unresolved_func1.hlsl", &pLib1,
             option);
  CComPtr<IDxcBlob> pLib2;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_unresolved_func2.hlsl", &pLib2,
             option);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName1 = L"lib1";
  RegisterDxcModule(libName1, pLib1, pLinker);

  LPCWSTR libName2 = L"lib2";
  RegisterDxcModule(libName2, pLib2, pLinker);

  LinkCheckMsg(
      L"", L"lib_6_3", pLinker, {libName1, libName2},
      {"Export name collides with another export: \\01?call_lib2@@YAMXZ"},
      {L"-exports", L"call_lib2=call_lib1;call_lib2"});
}

TEST_F(LinkerTest, RunLinkToLibWithUnusedExport) {
  LPCWSTR option[] = {L"-Zi", L"-Qembed_debug"};

  CComPtr<IDxcBlob> pLib1;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_unresolved_func1.hlsl", &pLib1,
             option);
  CComPtr<IDxcBlob> pLib2;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_unresolved_func2.hlsl", &pLib2,
             option);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName1 = L"lib1";
  RegisterDxcModule(libName1, pLib1, pLinker);

  LPCWSTR libName2 = L"lib2";
  RegisterDxcModule(libName2, pLib2, pLinker);

  LinkCheckMsg(L"", L"lib_6_3", pLinker, {libName1, libName2},
               {"Could not find target for export: call_lib"},
               {L"-exports", L"call_lib2=call_lib;call_lib1"});
}

TEST_F(LinkerTest, RunLinkToLibWithNoExports) {
  LPCWSTR option[] = {L"-Zi", L"-Qembed_debug"};

  CComPtr<IDxcBlob> pLib1;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_unresolved_func1.hlsl", &pLib1,
             option);
  CComPtr<IDxcBlob> pLib2;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_unresolved_func2.hlsl", &pLib2,
             option);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName1 = L"lib1";
  RegisterDxcModule(libName1, pLib1, pLinker);

  LPCWSTR libName2 = L"lib2";
  RegisterDxcModule(libName2, pLib2, pLinker);

  LinkCheckMsg(L"", L"lib_6_3", pLinker, {libName1, libName2},
               {"Library has no functions to export"},
               {L"-exports", L"call_lib2=call_lib"});
}

TEST_F(LinkerTest, RunLinkWithPotentialIntrinsicNameCollisions) {
  LPCWSTR option[] = {L"-Zi", L"-Qembed_debug", L"-default-linkage",
                      L"external"};

  CComPtr<IDxcBlob> pLib1;
  CompileLib(L"..\\CodeGenHLSL\\linker\\createHandle_multi.hlsl", &pLib1,
             option, L"lib_6_3");
  CComPtr<IDxcBlob> pLib2;
  CompileLib(L"..\\CodeGenHLSL\\linker\\createHandle_multi2.hlsl", &pLib2,
             option, L"lib_6_3");

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName1 = L"lib1";
  RegisterDxcModule(libName1, pLib1, pLinker);

  LPCWSTR libName2 = L"lib2";
  RegisterDxcModule(libName2, pLib2, pLinker);

  Link(L"", L"lib_6_3", pLinker, {libName1, libName2},
       {"declare %dx.types.Handle "
        "@\"dx.op.createHandleForLib.class.Texture2D<vector<float, 4> >\"(i32, "
        "%\"class.Texture2D<vector<float, 4> >\")",
        "declare %dx.types.Handle "
        "@\"dx.op.createHandleForLib.class.Texture2D<float>\"(i32, "
        "%\"class.Texture2D<float>\")"},
       {});
}

TEST_F(LinkerTest, RunLinkWithValidatorVersion) {
  if (m_ver.SkipDxilVersion(1, 4))
    return;

  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_mat_entry2.hlsl", &pEntryLib, {});
  CComPtr<IDxcBlob> pLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_mat_cast2.hlsl", &pLib, {});

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"ps_main";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LPCWSTR libName2 = L"test";
  RegisterDxcModule(libName2, pLib, pLinker);

  Link(L"", L"lib_6_3", pLinker, {libName, libName2},
       {"!dx.valver = !{(![0-9]+)}.*\n\\1 = !{i32 1, i32 3}"}, {},
       {L"-validator-version", L"1.3"}, /*regex*/ true);
}

TEST_F(LinkerTest, RunLinkWithInvalidValidatorVersion) {
  if (m_ver.SkipDxilVersion(1, 4))
    return;

  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_mat_entry2.hlsl", &pEntryLib, {});
  CComPtr<IDxcBlob> pLib;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_mat_cast2.hlsl", &pLib, {});

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"ps_main";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LPCWSTR libName2 = L"test";
  RegisterDxcModule(libName2, pLib, pLinker);

  LinkCheckMsg(L"", L"lib_6_3", pLinker, {libName, libName2},
               {"Validator version does not support target profile lib_6_3"},
               {L"-validator-version", L"1.2"});
}

TEST_F(LinkerTest, RunLinkWithTempReg) {
  // TempRegLoad/TempRegStore not normally usable from HLSL.
  // This assembly library exposes these through overloaded wrapper functions
  // void sreg(uint index, <type> value) to store register, overloaded for uint,
  // int, and float uint ureg(uint index) to load register as uint int ireg(int
  // index) to load register as int float freg(uint index) to load register as
  // float

  // This test verifies this scenario works, by assembling this library,
  // compiling a library with an entry point that uses sreg/ureg,
  // then linking these to a final standard vs_6_0 DXIL shader.

  CComPtr<IDxcBlob> pTempRegLib;
  AssembleLib(L"..\\HLSLFileCheck\\dxil\\linker\\TempReg.ll", &pTempRegLib);
  CComPtr<IDxcBlob> pEntryLib;
  CompileLib(L"..\\HLSLFileCheck\\dxil\\linker\\use-TempReg.hlsl", &pEntryLib,
             {L"-validator-version", L"1.3"}, L"lib_6_3");
  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);
  LPCWSTR libName = L"entry";
  RegisterDxcModule(libName, pEntryLib, pLinker);

  LPCWSTR libResName = L"TempReg";
  RegisterDxcModule(libResName, pTempRegLib, pLinker);

  Link(L"main", L"vs_6_0", pLinker, {libResName, libName},
       {"call void @dx.op.tempRegStore.i32", "call i32 @dx.op.tempRegLoad.i32"},
       {});
}

TEST_F(LinkerTest, RunLinkToLibWithGlobalCtor) {
  CComPtr<IDxcBlob> pLib0;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_static_cb_init.hlsl", &pLib0, {});
  CComPtr<IDxcBlob> pLib1;
  CompileLib(L"..\\CodeGenHLSL\\linker\\lib_use_static_cb_init.hlsl", &pLib1,
             {});

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"foo";
  RegisterDxcModule(libName, pLib0, pLinker);

  LPCWSTR libName2 = L"bar";
  RegisterDxcModule(libName2, pLib1, pLinker);
  // Make sure global_ctors created for lib to lib.
  Link(L"", L"lib_6_3", pLinker, {libName, libName2},
       {"@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ "
        "i32, void ()*, i8* } { i32 65535, void ()* "
        "@foo._GLOBAL__sub_I_lib_static_cb_init.hlsl, i8* null }]"},
       {}, {});
}

TEST_F(LinkerTest, LinkSm63ToSm66) {
  if (m_ver.SkipDxilVersion(1, 6))
    return;
  CComPtr<IDxcBlob> pLib0;
  CompileLib(L"..\\CodeGenHLSL\\linker\\link_to_sm66.hlsl", &pLib0, {},
             L"lib_6_3");

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"foo";
  RegisterDxcModule(libName, pLib0, pLinker);
  // Make sure add annotateHandle when link lib_6_3 to ps_6_6.
  Link(L"ps_main", L"ps_6_6", pLinker, {libName},
       {"call %dx.types.Handle @dx.op.annotateHandle\\(i32 216, "
        "%dx.types.Handle "
        "%(.*), %dx.types.ResourceProperties { i32 13, i32 4 }\\)"},
       {}, {}, true);
}

TEST_F(LinkerTest, RunLinkWithRootSig) {
  CComPtr<IDxcBlob> pLib0;
  CompileLib(L"..\\CodeGenHLSL\\linker\\link_with_root_sig.hlsl", &pLib0,
             {L"-HV", L"2018"}, L"lib_6_x");

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);

  LPCWSTR libName = L"foo";
  RegisterDxcModule(libName, pLib0, pLinker);

  LPCWSTR pEntryName = L"vs_main";
  LPCWSTR pShaderModel = L"vs_6_6";

  LPCWSTR libNames[] = {libName};
  CComPtr<IDxcOperationResult> pResult;

  VERIFY_SUCCEEDED(
      pLinker->Link(pEntryName, pShaderModel, libNames, 1, {}, 0, &pResult));
  CComPtr<IDxcBlob> pLinkedProgram;
  CheckOperationSucceeded(pResult, &pLinkedProgram);
  VERIFY_IS_TRUE(pLinkedProgram);

  CComPtr<IDxcBlob> pProgram;
  Compile(L"..\\CodeGenHLSL\\linker\\link_with_root_sig.hlsl", &pProgram,
          {L"-HV", L"2018"}, pEntryName, pShaderModel);
  VERIFY_IS_TRUE(pProgram);

  const DxilContainerHeader *pLinkedContainer = IsDxilContainerLike(
      pLinkedProgram->GetBufferPointer(), pLinkedProgram->GetBufferSize());

  VERIFY_IS_TRUE(pLinkedContainer);

  const DxilContainerHeader *pContainer = IsDxilContainerLike(
      pProgram->GetBufferPointer(), pProgram->GetBufferSize());
  VERIFY_IS_TRUE(pContainer);

  const DxilPartHeader *pLinkedRSPart =
      GetDxilPartByType(pLinkedContainer, DFCC_RootSignature);
  VERIFY_IS_TRUE(pLinkedRSPart);
  const DxilPartHeader *pRSPart =
      GetDxilPartByType(pContainer, DFCC_RootSignature);
  VERIFY_IS_TRUE(pRSPart);
  VERIFY_IS_TRUE(pRSPart->PartSize == pLinkedRSPart->PartSize);

  const uint8_t *pRS = (const uint8_t *)GetDxilPartData(pRSPart);
  const uint8_t *pLinkedRS = (const uint8_t *)GetDxilPartData(pLinkedRSPart);
  for (unsigned i = 0; i < pLinkedRSPart->PartSize; i++) {
    VERIFY_IS_TRUE(pRS[i] == pLinkedRS[i]);
  }
}

// Discriminate between the two different forms of outputs
// we handle from IDxcResult outputs.
enum OutputBlobKind {
  OUTPUT_IS_RAW_DATA,  // Output is raw data.
  OUTPUT_IS_CONTAINER, // Output is wrapped in its own dxil container.
};

// Verify that the contents of the IDxcResult matches what is in the
// dxil container. Some of the output data in the IDxcResult is wrapped
// in a dxil container so we can optionally retrieve that data based
// on the `OutputBlobKind` parameter.
static void VerifyPartsMatch(const DxilContainerHeader *pContainer,
                             hlsl::DxilFourCC DxilPart, IDxcResult *pResult,
                             DXC_OUT_KIND ResultKind,
                             OutputBlobKind OutputBlobKind,
                             LPCWSTR pResultName = nullptr) {
  const DxilPartHeader *pPart = GetDxilPartByType(pContainer, DxilPart);
  VERIFY_IS_NOT_NULL(pPart);

  CComPtr<IDxcBlob> pOutBlob;
  CComPtr<IDxcBlobWide> pOutObjName;
  VERIFY_IS_TRUE(pResult->HasOutput(ResultKind));
  VERIFY_SUCCEEDED(
      pResult->GetOutput(ResultKind, IID_PPV_ARGS(&pOutBlob), &pOutObjName));

  LPVOID PartData = (LPVOID)GetDxilPartData(pPart);
  SIZE_T PartSize = pPart->PartSize;
  LPVOID OutData = pOutBlob->GetBufferPointer();
  SIZE_T OutSize = pOutBlob->GetBufferSize();
  if (OutputBlobKind == OUTPUT_IS_CONTAINER) {
    const DxilContainerHeader *pOutContainer =
        IsDxilContainerLike(OutData, OutSize);
    VERIFY_IS_NOT_NULL(pOutContainer);
    const DxilPartHeader *pOutPart = GetDxilPartByType(pOutContainer, DxilPart);
    VERIFY_IS_NOT_NULL(pOutPart);
    OutData = (LPVOID)GetDxilPartData(pOutPart);
    OutSize = pOutPart->PartSize;
  }

  VERIFY_ARE_EQUAL(OutSize, PartSize);
  VERIFY_IS_TRUE(0 == memcmp(OutData, PartData, PartSize));

  if (pResultName) {
    VERIFY_IS_TRUE(pOutObjName->GetStringLength() == wcslen(pResultName));
    VERIFY_IS_TRUE(0 == wcsncmp(pOutObjName->GetStringPointer(), pResultName,
                                pOutObjName->GetStringLength()));
  }
}

// Check that the output exists in the IDxcResult and optionally validate the
// name.
void VerifyHasOutput(IDxcResult *pResult, DXC_OUT_KIND ResultKind, REFIID riid,
                     LPVOID *ppV, LPCWSTR pResultName = nullptr) {
  CComPtr<IDxcBlob> pOutBlob;
  CComPtr<IDxcBlobWide> pOutObjName;
  VERIFY_IS_TRUE(pResult->HasOutput(ResultKind));
  VERIFY_SUCCEEDED(pResult->GetOutput(ResultKind, riid, ppV, &pOutObjName));

  if (pResultName) {
    VERIFY_IS_TRUE(pOutObjName->GetStringLength() == wcslen(pResultName));
    VERIFY_IS_TRUE(0 == wcsncmp(pOutObjName->GetStringPointer(), pResultName,
                                pOutObjName->GetStringLength()));
  }
}

// Test that validates the DxcResult outputs after linking.
//
// Checks for DXC_OUT_OBJECT, DXC_OUT_ROOT_SIGNATURE, DXC_OUT_SHADER_HASH.
//
// Exercises the case that the outputs exist even when they
// are not given an explicit name (e.g. with /Fo).
TEST_F(LinkerTest, RunLinkWithDxcResultOutputs) {
  CComPtr<IDxcBlob> pLib;
  LPCWSTR LibName = L"MyLib.lib";
  CompileLib(L"..\\CodeGenHLSL\\linker\\link_with_root_sig.hlsl", &pLib,
             {L"-HV", L"2018", L"/Fo", LibName}, L"lib_6_x");

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);
  RegisterDxcModule(LibName, pLib, pLinker);

  CComPtr<IDxcResult> pLinkResult;
  Link(L"vs_main", L"vs_6_6", pLinker, {LibName}, {}, {}, {}, false,
       &pLinkResult);

  // Validate the output contains the DXC_OUT_OBJECT.
  CComPtr<IDxcBlob> pBlob;
  VerifyHasOutput(pLinkResult, DXC_OUT_OBJECT, IID_PPV_ARGS(&pBlob));

  // Get the container header from the output.
  const DxilContainerHeader *pContainer =
      IsDxilContainerLike(pBlob->GetBufferPointer(), pBlob->GetBufferSize());
  VERIFY_IS_NOT_NULL(pContainer);

  // Check that the output from the DxcResult matches the data in the container.
  VerifyPartsMatch(pContainer, DFCC_RootSignature, pLinkResult,
                   DXC_OUT_ROOT_SIGNATURE, OUTPUT_IS_CONTAINER);
  VerifyPartsMatch(pContainer, DFCC_ShaderHash, pLinkResult,
                   DXC_OUT_SHADER_HASH, OUTPUT_IS_RAW_DATA);
}

// Test that validates the DxcResult outputs after linking.
//
// Checks for DXC_OUT_OBJECT, DXC_OUT_ROOT_SIGNATURE, DXC_OUT_SHADER_HASH.
//
// Exercises the case that the outputs are retrieved with
// the expected names.
TEST_F(LinkerTest, RunLinkWithDxcResultNames) {
  CComPtr<IDxcBlob> pLib;
  LPCWSTR LibName = L"MyLib.lib";
  CompileLib(L"..\\CodeGenHLSL\\linker\\link_with_root_sig.hlsl", &pLib,
             {L"-HV", L"2018", L"/Fo", LibName}, L"lib_6_x");

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);
  RegisterDxcModule(LibName, pLib, pLinker);

  LPCWSTR ObjectName = L"MyLib.vso";
  LPCWSTR RootSigName = L"Rootsig.bin";
  LPCWSTR HashName = L"Hash.bin";
  CComPtr<IDxcResult> pLinkResult;
  Link(L"vs_main", L"vs_6_6", pLinker, {LibName}, {}, {},
       {L"/Fo", ObjectName, L"/Frs", RootSigName, L"/Fsh", HashName}, false,
       &pLinkResult);

  CComPtr<IDxcBlob> pObjBlob, pRsBlob, pHashBlob;
  VerifyHasOutput(pLinkResult, DXC_OUT_OBJECT, IID_PPV_ARGS(&pObjBlob),
                  ObjectName);
  VerifyHasOutput(pLinkResult, DXC_OUT_ROOT_SIGNATURE, IID_PPV_ARGS(&pRsBlob),
                  RootSigName);
  VerifyHasOutput(pLinkResult, DXC_OUT_SHADER_HASH, IID_PPV_ARGS(&pHashBlob),
                  HashName);
}

// Test that validates the DxcResult outputs after linking.
//
// Checks for DXC_OUT_REFLECTION
//
// This is done as a separate test because the reflection output
// is only available for library targets.
TEST_F(LinkerTest, RunLinkWithDxcResultRdat) {
  CComPtr<IDxcBlob> pLib;
  LPCWSTR LibName = L"MyLib.lib";
  CompileLib(L"..\\CodeGenHLSL\\linker\\createHandle_multi.hlsl", &pLib,
             L"lib_6_x");

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);
  RegisterDxcModule(LibName, pLib, pLinker);

  // Test that we get the IDxcResult outputs even without setting the name.
  {
    CComPtr<IDxcResult> pLinkResult;
    Link(L"", L"lib_6_3", pLinker, {LibName}, {}, {}, {}, false, &pLinkResult);
    CComPtr<IDxcBlob> pReflBlob;
    VerifyHasOutput(pLinkResult, DXC_OUT_REFLECTION, IID_PPV_ARGS(&pReflBlob));
  }

  // Test that we get the name set in the IDxcResult outputs.
  {
    LPCWSTR ReflName = L"MyLib.vso";
    CComPtr<IDxcResult> pLinkResult;
    Link(L"", L"lib_6_3", pLinker, {LibName}, {}, {},
         {
             L"/Fre",
             ReflName,
         },
         false, &pLinkResult);
    CComPtr<IDxcBlob> pReflBlob;
    VerifyHasOutput(pLinkResult, DXC_OUT_REFLECTION, IID_PPV_ARGS(&pReflBlob),
                    ReflName);
  }
}

// Test that validates the DxcResult outputs after linking.
//
// Checks for DXC_OUT_ERRORS
//
// This is done as a separate test because we need to generate an error
// message by failing the link job.
TEST_F(LinkerTest, RunLinkWithDxcResultErrors) {
  CComPtr<IDxcBlob> pLib;
  LPCWSTR LibName = L"MyLib.lib";
  CompileLib(L"..\\CodeGenHLSL\\lib_cs_entry.hlsl", &pLib);

  CComPtr<IDxcLinker> pLinker;
  CreateLinker(&pLinker);
  RegisterDxcModule(LibName, pLib, pLinker);

  // Test that we get the IDxcResult error outputs without setting the name.
  const char *ErrorMessage =
      "error: Cannot find definition of function non_existent_entry\n";
  {
    CComPtr<IDxcResult> pLinkResult;
    LinkCheckMsg(L"non_existent_entry", L"cs_6_0", pLinker, {LibName},
                 {ErrorMessage}, {}, false, &pLinkResult);
    CComPtr<IDxcBlobUtf8> pErrorOutput;
    VerifyHasOutput(pLinkResult, DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrorOutput));
    VERIFY_IS_TRUE(pErrorOutput->GetStringLength() == strlen(ErrorMessage));
    VERIFY_IS_TRUE(0 == strncmp(pErrorOutput->GetStringPointer(), ErrorMessage,
                                pErrorOutput->GetStringLength()));
  }

  // Test that we get the name set in the IDxcResult error output.
  {
    LPCWSTR ErrName = L"Errors.txt";
    CComPtr<IDxcResult> pLinkResult;
    LinkCheckMsg(L"non_existent_entry", L"cs_6_0", pLinker, {LibName},
                 {ErrorMessage}, {L"/Fe", ErrName}, false, &pLinkResult);
    CComPtr<IDxcBlobUtf8> pErrorOutput;
    VerifyHasOutput(pLinkResult, DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrorOutput),
                    ErrName);
    VERIFY_IS_TRUE(pErrorOutput->GetStringLength() == strlen(ErrorMessage));
    VERIFY_IS_TRUE(0 == strncmp(pErrorOutput->GetStringPointer(), ErrorMessage,
                                pErrorOutput->GetStringLength()));
  }
}
