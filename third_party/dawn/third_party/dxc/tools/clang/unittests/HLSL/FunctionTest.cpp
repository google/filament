///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// FunctionTest.cpp                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// The testing is done primarily through the compiler interface to avoid     //
// linking the full Clang libraries.                                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Test/CompilationResult.h"
#include "dxc/Test/HLSLTestData.h"
#include <memory>
#include <string>
#include <vector>

#undef _read
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/Global.h"
#include "dxc/Test/DxcTestUtils.h"
#include "dxc/Test/HlslTestUtils.h"

#ifdef _WIN32
class FunctionTest {
#else
class FunctionTest : public ::testing::Test {
#endif
public:
  BEGIN_TEST_CLASS(FunctionTest)
  TEST_CLASS_PROPERTY(L"Parallel", L"true")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()
  TEST_METHOD(AllowedStorageClass)
  TEST_METHOD(AllowedInParamUsesClass)
  TEST_METHOD(ParseRootSignature)

  dxc::DxcDllSupport m_support;
  std::vector<char> rootSigText;

  std::string BuildSampleFunction(const char *StorageClassKeyword) {
    char result[128];
    sprintf_s(result, _countof(result), "%s float ps(float o) { return o; }",
              StorageClassKeyword);

    return std::string(result);
  }

  void CheckCompiles(const std::string &text, bool expected) {
    CheckCompiles(text.c_str(), text.size(), expected);
  }

  void CheckCompiles(const char *text, size_t textLen, bool expected) {
    CompilationResult result(
        CompilationResult::CreateForProgram(text, textLen));

    EXPECT_EQ(expected, result.ParseSucceeded()); // << "for program " << text;
  }

  void TestHLSLRootSignatureVerCase(const char *pStr,
                                    const std::wstring &forceVer,
                                    HRESULT expected) {
    WEX::TestExecution::SetVerifyOutput verifySettings(
        WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
    CComPtr<IDxcLibrary> pLibrary;
    CComPtr<IDxcCompiler> pCompiler;
    CComPtr<IDxcOperationResult> pResult;
    CComPtr<IDxcBlobEncoding> pSource;
    HRESULT resultStatus;
    CComPtr<IDxcIncludeHandler> pIncludeHandler;

    VERIFY_SUCCEEDED(m_support.CreateInstance(CLSID_DxcLibrary, &pLibrary));
    const char pFormat[] = "[RootSignature(\"%s\")]\r\n"
                           "float4 main() : SV_Target { return 0; }";
    size_t len = strlen(pStr) + strlen(pFormat) +
                 1; // Actually bigger than needed because of '%s'
    rootSigText.resize(len);
    sprintf_s(rootSigText.data(), rootSigText.size(), pFormat, pStr);
    Utf8ToBlob(m_support, rootSigText.data(), &pSource);
    VERIFY_SUCCEEDED(pLibrary->CreateIncludeHandler(&pIncludeHandler));
    VERIFY_SUCCEEDED(m_support.CreateInstance(CLSID_DxcCompiler, &pCompiler));
    std::vector<LPCWSTR> flags;
    if (!forceVer.empty()) {
      flags.push_back(L"/force_rootsig_ver");
      flags.push_back(forceVer.c_str());
    }
    VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"hlsl.hlsl", L"main",
                                        L"ps_6_0", flags.data(), flags.size(),
                                        nullptr, 0, pIncludeHandler, &pResult));
    VERIFY_SUCCEEDED(pResult->GetStatus(&resultStatus));
    if (expected != resultStatus && FAILED(resultStatus)) {
      // Unexpected failure, log results.
      CComPtr<IDxcBlobEncoding> pErrors;
      pResult->GetErrorBuffer(&pErrors);
      std::string text = BlobToUtf8(pErrors);
      CA2W textW(text.c_str());
      WEX::Logging::Log::Comment(textW.m_psz);
    }
    VERIFY_ARE_EQUAL(expected, resultStatus);
    if (SUCCEEDED(resultStatus)) {
      CComPtr<IDxcContainerReflection> pReflection;
      CComPtr<IDxcBlob> pContainer;

      VERIFY_SUCCEEDED(pResult->GetResult(&pContainer));
      VERIFY_SUCCEEDED(
          m_support.CreateInstance(CLSID_DxcContainerReflection, &pReflection));
      VERIFY_SUCCEEDED(pReflection->Load(pContainer));
      UINT count;
      bool found = false;
      VERIFY_SUCCEEDED(pReflection->GetPartCount(&count));
      for (UINT i = 0; i < count; ++i) {
        UINT kind;
        VERIFY_SUCCEEDED(pReflection->GetPartKind(i, &kind));
        if (kind == hlsl::DFCC_RootSignature) {
          found = true;
          break;
        }
      }
      VERIFY_IS_TRUE(found);
    }
  }

  void TestHLSLRootSignature10Case(const char *pStr, HRESULT hr) {
    TestHLSLRootSignatureVerCase(pStr, L"rootsig_1_0", hr);
  }

  void TestHLSLRootSignature11Case(const char *pStr, HRESULT hr) {
    TestHLSLRootSignatureVerCase(pStr, L"rootsig_1_1", hr);
  }

  void TestHLSLRootSignatureCase(const char *pStr, HRESULT hr) {
    TestHLSLRootSignatureVerCase(pStr, L"", hr);
    TestHLSLRootSignature10Case(pStr, hr);
    TestHLSLRootSignature11Case(pStr, hr);
  }
};

TEST_F(FunctionTest, AllowedStorageClass) {
  for (const auto &sc : StorageClassData) {
    CheckCompiles(BuildSampleFunction(sc.Keyword), sc.IsValid);
  }
}

TEST_F(FunctionTest, AllowedInParamUsesClass) {
  const char *fragments[] = {"f", "1.0f"};
  for (const auto &iop : InOutParameterModifierData) {
    for (unsigned i = 0; i < _countof(fragments); i++) {
      char program[256];
      sprintf_s(program, _countof(program),
                "float ps(%s float o) { return o; }\n"
                "void caller() { float f; ps(%s); }",
                iop.Keyword, fragments[i]);
      bool callerIsRef = i == 0;

      bool expectedSucceeds =
          (callerIsRef == iop.ActsAsReference) || !iop.ActsAsReference;
      CheckCompiles(program, strlen(program), expectedSucceeds);
    }
  }
}

TEST_F(FunctionTest, ParseRootSignature) {
#ifdef _WIN32 // - dxil.dll can only be found on Windows
  struct AutoModule {
    HMODULE m_module;
    AutoModule(const wchar_t *pName) { m_module = LoadLibraryW(pName); }
    ~AutoModule() {
      if (m_module != NULL)
        FreeLibrary(m_module);
    }
  };
  AutoModule dxilAM(
      L"dxil.dll"); // Pin this if available to avoid reloading on each compile.
#endif              // _WIN32 - dxil.dll can only be found on Windows

  VERIFY_SUCCEEDED(m_support.Initialize());

  // Empty
  TestHLSLRootSignatureCase("", S_OK);
  TestHLSLRootSignatureCase("    ", S_OK);
  TestHLSLRootSignatureCase(" 324  ;jk ", E_FAIL);

  // Flags
  TestHLSLRootSignatureCase("RootFlags( 0 )", S_OK);
  TestHLSLRootSignatureCase("RootFlags( 20 )", E_FAIL);
  TestHLSLRootSignatureCase("RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)",
                            S_OK);
  TestHLSLRootSignatureCase("RootFlags(ALLOW_STREAM_OUTPUT)", S_OK);
  TestHLSLRootSignatureCase("RootFlags(LOCAL_ROOT_SIGNATURE)", E_FAIL);
  TestHLSLRootSignatureCase("RootFlags( LLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)",
                            E_FAIL);
  TestHLSLRootSignatureCase(
      "  RootFlags   ( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT   ) ", S_OK);
  TestHLSLRootSignatureCase(
      "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | "
      "DENY_VERTEX_SHADER_ROOT_ACCESS | DENY_HULL_SHADER_ROOT_ACCESS | "
      "DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS | "
      "DENY_PIXEL_SHADER_ROOT_ACCESS)",
      S_OK);
  TestHLSLRootSignatureCase("RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT & "
                            "DENY_VERTEX_SHADER_ROOT_ACCESS)",
                            E_FAIL);
  TestHLSLRootSignatureCase("RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | 7)",
                            E_FAIL);

  // RootConstants: RootConstants(num32BitConstants=3, b2 [, space = 5] )
  TestHLSLRootSignatureCase("RootConstants( num32BitConstants=3, b2)", S_OK);
  TestHLSLRootSignatureCase(
      "RootConstants( num32BitConstants=3, b2, space = 5)", S_OK);
  TestHLSLRootSignatureCase(
      "RootConstants( b2, num32BitConstants=3, space = 5)", S_OK);
  TestHLSLRootSignatureCase("RootConstants( num32BitConstants=3, b2, "
                            "visibility=SHADER_VISIBILITY_PIXEL)",
                            S_OK);
  TestHLSLRootSignatureCase("RootConstants( num32BitConstants=3, b2, space = "
                            "5, visibility = SHADER_VISIBILITY_PIXEL)",
                            S_OK);
  TestHLSLRootSignatureCase(
      "RootConstants( visibility = SHADER_VISIBILITY_PIXEL, space = 5, "
      "num32BitConstants=3, b2)",
      S_OK);
  TestHLSLRootSignatureCase(
      "RootConstants( visibility = SHADER_VISIBILITY_PIXEL, space = 5, "
      "num32BitConstants=3, b2, visibility = SHADER_VISIBILITY_ALL)",
      E_FAIL);
  TestHLSLRootSignatureCase(
      "RootConstants( visibility = SHADER_VISIBILITY_PIXEL, space = 5, space = "
      "5, num32BitConstants=3, b2)",
      E_FAIL);
  TestHLSLRootSignatureCase(
      "RootConstants( num32BitConstants=7, visibility = "
      "SHADER_VISIBILITY_PIXEL, space = 5, num32BitConstants=3, b2)",
      E_FAIL);
  TestHLSLRootSignatureCase(
      "RootConstants( b10, visibility = SHADER_VISIBILITY_PIXEL, space = 5, "
      "num32BitConstants=3, b2)",
      E_FAIL);

  // RS CBV: CBV(b0 [, space=3, flags=0, visibility = SHADER_VISIBILITY_ALL ] )
  TestHLSLRootSignatureCase("CBV(b2)", S_OK);
  TestHLSLRootSignatureCase("CBV(t2)", E_FAIL);
  TestHLSLRootSignatureCase("CBV(u2)", E_FAIL);
  TestHLSLRootSignatureCase("CBV(s2)", E_FAIL);
  TestHLSLRootSignatureCase("CBV(b4294967295)", S_OK);
  TestHLSLRootSignatureCase("CBV(b2, space = 5)", S_OK);
  TestHLSLRootSignatureCase("CBV(b2, space = 4294967279)", S_OK);
  TestHLSLRootSignatureCase("CBV(b2, visibility = SHADER_VISIBILITY_PIXEL)",
                            S_OK);
  TestHLSLRootSignatureCase(
      "CBV(b2, space = 5, visibility = SHADER_VISIBILITY_PIXEL)", S_OK);
  TestHLSLRootSignatureCase(
      "CBV(space = 5, visibility = SHADER_VISIBILITY_PIXEL, b2)", S_OK);
  TestHLSLRootSignatureCase(
      "CBV(b2, space = 5, b2, visibility = SHADER_VISIBILITY_PIXEL)", E_FAIL);
  TestHLSLRootSignatureCase(
      "CBV(space = 4, b2, space = 5, visibility = SHADER_VISIBILITY_PIXEL)",
      E_FAIL);
  TestHLSLRootSignatureCase("CBV(b2, visibility = SHADER_VISIBILITY_PIXEL, "
                            "space = 5, visibility = SHADER_VISIBILITY_PIXEL)",
                            E_FAIL);
  TestHLSLRootSignatureCase(
      "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), CBV(b2, space = 5, "
      "visibility = SHADER_VISIBILITY_PIXEL)",
      S_OK);
  TestHLSLRootSignatureCase(
      "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), "
      "CBV(b2, space = 5, visibility = SHADER_VISIBILITY_PIXEL), "
      "CBV(b4, space = 7, visibility = SHADER_VISIBILITY_VERTEX)",
      S_OK);

  // RS SRV: SRV(t0 [, space=3, flags=0, visibility = SHADER_VISIBILITY_ALL ] )
  TestHLSLRootSignatureCase("SRV(t2)", S_OK);
  TestHLSLRootSignatureCase("SRV(b2)", E_FAIL);
  TestHLSLRootSignatureCase("SRV(u2)", E_FAIL);
  TestHLSLRootSignatureCase("SRV(s2)", E_FAIL);
  TestHLSLRootSignatureCase("SRV(t2, space = 5)", S_OK);
  TestHLSLRootSignatureCase("SRV(t2, visibility = SHADER_VISIBILITY_PIXEL)",
                            S_OK);
  TestHLSLRootSignatureCase(
      "SRV(t2, space = 5, visibility = SHADER_VISIBILITY_PIXEL)", S_OK);
  TestHLSLRootSignatureCase(
      "SRV(space = 5, visibility = SHADER_VISIBILITY_PIXEL, t2)", S_OK);
  TestHLSLRootSignatureCase(
      "SRV(t2, space = 5, t2, visibility = SHADER_VISIBILITY_PIXEL)", E_FAIL);
  TestHLSLRootSignatureCase(
      "SRV(space = 4, t2, space = 5, visibility = SHADER_VISIBILITY_PIXEL)",
      E_FAIL);
  TestHLSLRootSignatureCase("SRV(t2, visibility = SHADER_VISIBILITY_PIXEL, "
                            "space = 5, visibility = SHADER_VISIBILITY_PIXEL)",
                            E_FAIL);
  TestHLSLRootSignatureCase(
      "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), SRV(t2, space = 5, "
      "visibility = SHADER_VISIBILITY_PIXEL)",
      S_OK);
  TestHLSLRootSignatureCase(
      "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), "
      "SRV(t2, space = 5, visibility = SHADER_VISIBILITY_PIXEL), "
      "SRV(t4, space = 7, visibility = SHADER_VISIBILITY_VERTEX)",
      S_OK);

  // RS UAV: UAV(u0 [, space=3, flags=0, visibility = SHADER_VISIBILITY_ALL ] )
  TestHLSLRootSignatureCase("UAV(u2)", S_OK);
  TestHLSLRootSignatureCase("UAV(b2)", E_FAIL);
  TestHLSLRootSignatureCase("UAV(t2)", E_FAIL);
  TestHLSLRootSignatureCase("UAV(s2)", E_FAIL);
  TestHLSLRootSignatureCase("UAV(u2, space = 5)", S_OK);
  TestHLSLRootSignatureCase("UAV(u2, visibility = SHADER_VISIBILITY_PIXEL)",
                            S_OK);
  TestHLSLRootSignatureCase(
      "UAV(u2, space = 5, visibility = SHADER_VISIBILITY_PIXEL)", S_OK);
  TestHLSLRootSignatureCase(
      "UAV(space = 5, visibility = SHADER_VISIBILITY_PIXEL, u2)", S_OK);
  TestHLSLRootSignatureCase(
      "UAV(u2, space = 5, u2, visibility = SHADER_VISIBILITY_PIXEL)", E_FAIL);
  TestHLSLRootSignatureCase(
      "UAV(space = 4, u2, space = 5, visibility = SHADER_VISIBILITY_PIXEL)",
      E_FAIL);
  TestHLSLRootSignatureCase("UAV(u2, visibility = SHADER_VISIBILITY_PIXEL, "
                            "space = 5, visibility = SHADER_VISIBILITY_PIXEL)",
                            E_FAIL);
  TestHLSLRootSignatureCase(
      "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), "
      "UAV(u2, space = 5, visibility = SHADER_VISIBILITY_PIXEL)",
      S_OK);
  TestHLSLRootSignatureCase(
      "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), "
      "UAV(u2, space = 5, visibility = SHADER_VISIBILITY_PIXEL), "
      "UAV(u4, space = 7, visibility = SHADER_VISIBILITY_VERTEX)",
      S_OK);

  // RS1.1 root descriptor flags.
  TestHLSLRootSignature11Case("CBV(b2, flags=0)", S_OK);
  TestHLSLRootSignature11Case("CBV(b2, flags=DATA_VOLATILE)", S_OK);
  TestHLSLRootSignature11Case("SRV(t2, flags=DATA_STATIC)", S_OK);
  TestHLSLRootSignature11Case("UAV(u2, flags=DATA_STATIC_WHILE_SET_AT_EXECUTE)",
                              S_OK);
  TestHLSLRootSignature11Case(
      "UAV(u2, flags=DATA_STATIC_WHILE_SET_AT_EXECUTE, space = 5)", S_OK);
  TestHLSLRootSignature10Case("CBV(b2, flags=0)", E_FAIL);
  TestHLSLRootSignature10Case("CBV(b2, flags=DATA_VOLATILE)", E_FAIL);
  TestHLSLRootSignature10Case("SRV(t2, flags=DATA_STATIC)", E_FAIL);
  TestHLSLRootSignature10Case("UAV(u2, flags=DATA_STATIC_WHILE_SET_AT_EXECUTE)",
                              E_FAIL);
  TestHLSLRootSignature11Case("CBV(b2, flags=DATA_VOLATILE |  DATA_STATIC)",
                              E_FAIL);
  TestHLSLRootSignature11Case(
      "CBV(b2, flags=DATA_STATIC_WHILE_SET_AT_EXECUTE| DATA_STATIC)", E_FAIL);
  TestHLSLRootSignature11Case(
      "CBV(b2, flags=DATA_STATIC_WHILE_SET_AT_EXECUTE|DATA_VOLATILE)", E_FAIL);
  TestHLSLRootSignature11Case(
      "UAV(u2, flags=DATA_STATIC_WHILE_SET_AT_EXECUTE, )", E_FAIL);

  // DT: DescriptorTable( SRV(t2, numDescriptors=6), UAV(u0, numDescriptors=4,
  // offset = 17), visibility = SHADER_VISIBILITY_ALL )
  TestHLSLRootSignatureCase("DescriptorTable(CBV(b2))", S_OK);
  TestHLSLRootSignatureCase("DescriptorTable(CBV(t2))", E_FAIL);
  TestHLSLRootSignatureCase("DescriptorTable(CBV(u2))", E_FAIL);
  TestHLSLRootSignatureCase("DescriptorTable(CBV(s2))", E_FAIL);
  TestHLSLRootSignatureCase("DescriptorTable(SRV(t2))", S_OK);
  TestHLSLRootSignatureCase("DescriptorTable(UAV(u2))", S_OK);
  TestHLSLRootSignatureCase("DescriptorTable(Sampler(s2))", S_OK);
  TestHLSLRootSignatureCase("DescriptorTable(CBV(b2, numDescriptors = 4))",
                            S_OK);
  TestHLSLRootSignatureCase("DescriptorTable(CBV(b2, space=3))", S_OK);
  TestHLSLRootSignatureCase("DescriptorTable(CBV(b2, offset=17))", S_OK);
  TestHLSLRootSignatureCase("DescriptorTable(CBV(b2, numDescriptors = 4))",
                            S_OK);
  TestHLSLRootSignatureCase(
      "DescriptorTable(CBV(b2, numDescriptors = 4, space=3))", S_OK);
  TestHLSLRootSignatureCase(
      "DescriptorTable(CBV(b2, numDescriptors = 4, offset = 17))", S_OK);
  TestHLSLRootSignatureCase(
      "DescriptorTable(Sampler(s2, numDescriptors = 4, space=3, offset =17))",
      S_OK);
  TestHLSLRootSignatureCase(
      "DescriptorTable(Sampler(offset =17, numDescriptors = 4, s2, space=3))",
      S_OK);
  TestHLSLRootSignatureCase("DescriptorTable(Sampler(offset =17, "
                            "numDescriptors = unbounded, s2, space=3))",
                            S_OK);
  TestHLSLRootSignatureCase("DescriptorTable(Sampler(offset =17, "
                            "numDescriptors = 4, offset = 1, s2, space=3))",
                            E_FAIL);
  TestHLSLRootSignatureCase("DescriptorTable(Sampler(s1, offset =17, "
                            "numDescriptors = 4, s2, space=3))",
                            E_FAIL);
  TestHLSLRootSignatureCase(
      "DescriptorTable(Sampler(offset =17, numDescriptors = 4, s2, space=3, "
      "numDescriptors =1))",
      E_FAIL);
  TestHLSLRootSignatureCase("DescriptorTable(Sampler(offset =17, "
                            "numDescriptors = 4, s2, space=3, space=4))",
                            E_FAIL);
  TestHLSLRootSignatureCase("DescriptorTable(CBV(b2), UAV(u3))", S_OK);
  TestHLSLRootSignatureCase(
      "DescriptorTable(CBV(b2), UAV(u3), visibility = SHADER_VISIBILITY_HULL)",
      S_OK);
  TestHLSLRootSignatureCase(
      "DescriptorTable(CBV(b2), visibility = shader_visibility_hull, UAV(u3))",
      S_OK);
  TestHLSLRootSignatureCase(
      "DescriptorTable(CBV(b2), visibility = SHADER_VISIBILITY_HULL, UAV(u3), "
      "visibility = SHADER_VISIBILITY_HULL)",
      E_FAIL);

  // RS1.1 descriptor range flags.
  TestHLSLRootSignature11Case("DescriptorTable(CBV(b2, flags = 0))", S_OK);
  TestHLSLRootSignature11Case(
      "DescriptorTable(SRV(t2, flags = DESCRIPTORS_VOLATILE))", S_OK);
  TestHLSLRootSignature11Case(
      "DescriptorTable(SRV(t2, flags = DESCRIPTORS_VOLATILE | DATA_VOLATILE))",
      S_OK);
  TestHLSLRootSignature11Case("DescriptorTable(UAV(u2, flags = DATA_VOLATILE))",
                              S_OK);
  TestHLSLRootSignature11Case(
      "DescriptorTable(UAV(u2, flags = DATA_STATIC_WHILE_SET_AT_EXECUTE))",
      S_OK);
  TestHLSLRootSignature11Case("DescriptorTable(UAV(u2, flags = DATA_STATIC))",
                              S_OK);
  TestHLSLRootSignature11Case(
      "DescriptorTable(UAV(u2, flags = DESCRIPTORS_VOLATILE | DATA_VOLATILE))",
      S_OK);
  TestHLSLRootSignature11Case(
      "DescriptorTable(UAV(u2, flags = DESCRIPTORS_VOLATILE | "
      "DATA_STATIC_WHILE_SET_AT_EXECUTE))",
      S_OK);
  TestHLSLRootSignature11Case(
      "DescriptorTable(Sampler(s2, flags = DESCRIPTORS_VOLATILE))", S_OK);
  TestHLSLRootSignature11Case(
      "DescriptorTable(UAV(u2, flags = DESCRIPTORS_VOLATILE | "
      "DATA_STATIC_WHILE_SET_AT_EXECUTE, offset =17))",
      S_OK);
  TestHLSLRootSignature11Case(
      "DescriptorTable(UAV(u2, flags = "
      "DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS))",
      S_OK);

  TestHLSLRootSignature10Case("DescriptorTable(CBV(b2, flags = 0))", E_FAIL);
  TestHLSLRootSignature10Case(
      "DescriptorTable(SRV(t2, flags = DESCRIPTORS_VOLATILE))", E_FAIL);
  TestHLSLRootSignature10Case("DescriptorTable(UAV(u2, flags = DATA_VOLATILE))",
                              E_FAIL);
  TestHLSLRootSignature10Case(
      "DescriptorTable(UAV(u2, flags = DATA_STATIC_WHILE_SET_AT_EXECUTE))",
      E_FAIL);
  TestHLSLRootSignature10Case("DescriptorTable(UAV(u2, flags = DATA_STATIC))",
                              E_FAIL);
  TestHLSLRootSignature10Case(
      "DescriptorTable(Sampler(s2, flags = DESCRIPTORS_VOLATILE))", E_FAIL);

  TestHLSLRootSignature11Case(
      "DescriptorTable(Sampler(s2, flags = DATA_VOLATILE))", E_FAIL);
  TestHLSLRootSignature11Case("DescriptorTable(Sampler(s2, flags = "
                              "DESCRIPTORS_VOLATILE | DATA_VOLATILE))",
                              E_FAIL);
  TestHLSLRootSignature11Case(
      "DescriptorTable(Sampler(s2, flags = DATA_STATIC_WHILE_SET_AT_EXECUTE))",
      E_FAIL);
  TestHLSLRootSignature11Case(
      "DescriptorTable(Sampler(s2, flags = DATA_STATIC))", E_FAIL);
  TestHLSLRootSignature11Case(
      "DescriptorTable(UAV(u2, flags = DESCRIPTORS_VOLATILE | DATA_VOLATILE | "
      "DATA_STATIC_WHILE_SET_AT_EXECUTE))",
      E_FAIL);
  TestHLSLRootSignature11Case("DescriptorTable(CBV(b2, flags = DATA_VOLATILE | "
                              "DATA_STATIC_WHILE_SET_AT_EXECUTE))",
                              E_FAIL);
  TestHLSLRootSignature11Case(
      "DescriptorTable(CBV(b2, flags = DATA_VOLATILE | DATA_STATIC))", E_FAIL);
  TestHLSLRootSignature11Case(
      "DescriptorTable(CBV(b2, flags = DATA_STATIC_WHILE_SET_AT_EXECUTE | "
      "DATA_STATIC))",
      E_FAIL);
  TestHLSLRootSignature11Case(
      "DescriptorTable(CBV(b2, flags = DESCRIPTORS_VOLATILE | DATA_STATIC))",
      E_FAIL);
  TestHLSLRootSignature11Case(
      "DescriptorTable(UAV(u2, flags = DESCRIPTORS_VOLATILE | "
      "DATA_STATIC_WHILE_SET_AT_EXECUTE, ))",
      E_FAIL);

  // StaticSampler(   s0,
  //                [ Filter = FILTER_ANISOTROPIC,
  //                  AddressU = TEXTURE_ADDRESS_WRAP,
  //                  AddressV = TEXTURE_ADDRESS_WRAP,
  //                  AddressW = TEXTURE_ADDRESS_WRAP,
  //                  MaxAnisotropy = 16,
  //                  ComparisonFunc = COMPARISON_LESS_EQUAL,
  //                  BorderColor = STATIC_BORDER_COLOR_OPAQUE_WHITE,
  //                  space = 0,
  //                  visibility = SHADER_VISIBILITY_ALL ] )
  // SReg
  TestHLSLRootSignatureCase("StaticSampler(s2)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(t2)", E_FAIL);
  TestHLSLRootSignatureCase("StaticSampler(b2)", E_FAIL);
  TestHLSLRootSignatureCase("StaticSampler(u2)", E_FAIL);
  TestHLSLRootSignatureCase("StaticSampler(s0, s2)", E_FAIL);
  // Filter
  TestHLSLRootSignatureCase(
      "StaticSampler(filter = FILTER_MIN_MAG_MIP_POINT, s2)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MIN_MAG_POINT_MIP_LINEAR)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT)",
      S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MIN_POINT_MAG_MIP_LINEAR)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MIN_LINEAR_MAG_MIP_POINT)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR)",
      S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MIN_MAG_LINEAR_MIP_POINT)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MIN_MAG_MIP_LINEAR)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, filter = FILTER_ANISOTROPIC)",
                            S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_COMPARISON_MIN_MAG_MIP_POINT)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR)",
      S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, filter = "
                            "FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT)",
                            S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR)",
      S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT)",
      S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = "
      "FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR)",
      S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT)",
      S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_COMPARISON_MIN_MAG_MIP_LINEAR)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_COMPARISON_ANISOTROPIC)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MINIMUM_MIN_MAG_MIP_POINT)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR)",
      S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, filter = "
                            "FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT)",
                            S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR)",
      S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT)",
      S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, filter = "
                            "FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR)",
                            S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT)",
      S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MINIMUM_MIN_MAG_MIP_LINEAR)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MINIMUM_ANISOTROPIC)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MAXIMUM_MIN_MAG_MIP_POINT)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR)",
      S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, filter = "
                            "FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT)",
                            S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR)",
      S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT)",
      S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, filter = "
                            "FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR)",
                            S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT)",
      S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, filter = FILTER_MAXIMUM_ANISOTROPIC)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(filter = FILTER_MAXIMUM_ANISOTROPIC, s2, filter = "
      "FILTER_MAXIMUM_ANISOTROPIC)",
      E_FAIL);
  // AddressU
  TestHLSLRootSignatureCase(
      "StaticSampler(addressU = TEXTURE_ADDRESS_WRAP, s2)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, addressU = TEXTURE_ADDRESS_MIRROR)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, addressU = TEXTURE_ADDRESS_CLAMP)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, addressU = TEXTURE_ADDRESS_BORDER)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, addressU = TEXTURE_ADDRESS_MIRROR_ONCE)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(addressU = TEXTURE_ADDRESS_MIRROR, "
                            "s2, addressU = TEXTURE_ADDRESS_BORDER)",
                            E_FAIL);
  // AddressV
  TestHLSLRootSignatureCase(
      "StaticSampler(addressV = TEXTURE_ADDRESS_WRAP, s2)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, addressV = TEXTURE_ADDRESS_MIRROR)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, addressV = TEXTURE_ADDRESS_CLAMP)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, addressV = TEXTURE_ADDRESS_BORDER)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, addressV = TEXTURE_ADDRESS_MIRROR_ONCE)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(addressV = TEXTURE_ADDRESS_MIRROR, "
                            "s2, addressV = TEXTURE_ADDRESS_BORDER)",
                            E_FAIL);
  // AddressW
  TestHLSLRootSignatureCase(
      "StaticSampler(addressW = TEXTURE_ADDRESS_WRAP, s2)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, addressW = TEXTURE_ADDRESS_MIRROR)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, addressW = TEXTURE_ADDRESS_CLAMP)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, addressW = TEXTURE_ADDRESS_BORDER)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, addressW = TEXTURE_ADDRESS_MIRROR_ONCE)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(addressW = TEXTURE_ADDRESS_MIRROR, "
                            "s2, addressW = TEXTURE_ADDRESS_BORDER)",
                            E_FAIL);
  // Mixed address
  TestHLSLRootSignatureCase(
      "StaticSampler(addressW = TEXTURE_ADDRESS_MIRROR, s2, addressU = "
      "TEXTURE_ADDRESS_CLAMP, addressV = TEXTURE_ADDRESS_MIRROR_ONCE)",
      S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(addressW = TEXTURE_ADDRESS_MIRROR, s2, addressU = "
      "TEXTURE_ADDRESS_CLAMP, addressU = TEXTURE_ADDRESS_CLAMP, addressV = "
      "TEXTURE_ADDRESS_MIRROR_ONCE)",
      E_FAIL);
  // MipLODBias
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=0)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=-0)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=+0)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=0.)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=0.f)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=0.0)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=0.0f)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=0.1)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=+0.1)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=-0.1)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=.1)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=2)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=-2)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=2.3)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=+2.3)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=-2.3)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=2.3f)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=)", E_FAIL);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=sdfgsdf)", E_FAIL);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=--2)", E_FAIL);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=-+2)", E_FAIL);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=.)", E_FAIL);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=-.)", E_FAIL);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=+.)", E_FAIL);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=.e2)", E_FAIL);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=.1e)", E_FAIL);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=.1e.4)", E_FAIL);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=2e100)", E_FAIL);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=-2e100)", E_FAIL);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=2e100000)", E_FAIL);
  TestHLSLRootSignatureCase("StaticSampler(s2, mipLODBias=-2e100000)", E_FAIL);
  // MaxAnisotropy
  TestHLSLRootSignatureCase("StaticSampler(s2, maxAnisotropy=2)", S_OK);
  // Comparison function
  TestHLSLRootSignatureCase(
      "StaticSampler(ComparisonFunc = COMPARISON_NEVER, s2)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, ComparisonFunc = COMPARISON_LESS)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, ComparisonFunc = COMPARISON_EQUAL)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, ComparisonFunc = COMPARISON_LESS_EQUAL)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, ComparisonFunc = COMPARISON_GREATER)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, ComparisonFunc = COMPARISON_NOT_EQUAL)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, ComparisonFunc = COMPARISON_GREATER_EQUAL)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, ComparisonFunc = COMPARISON_ALWAYS)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(ComparisonFunc = COMPARISON_NOT_EQUAL, s2, ComparisonFunc "
      "= COMPARISON_ALWAYS)",
      E_FAIL);
  // Border color
  TestHLSLRootSignatureCase(
      "StaticSampler(BorderColor = STATIC_BORDER_COLOR_TRANSPARENT_BLACK, s2)",
      S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, BorderColor = STATIC_BORDER_COLOR_OPAQUE_BLACK)",
      S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, BorderColor = STATIC_BORDER_COLOR_OPAQUE_WHITE)",
      S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, BorderColor = STATIC_BORDER_COLOR_OPAQUE_BLACK_UINT)",
      S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, BorderColor = STATIC_BORDER_COLOR_OPAQUE_WHITE_UINT)",
      S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(BorderColor = STATIC_BORDER_COLOR_TRANSPARENT_BLACK, s2, "
      "BorderColor = STATIC_BORDER_COLOR_OPAQUE_WHITE)",
      E_FAIL);

  // MinLOD
  TestHLSLRootSignatureCase("StaticSampler(s2, minLOD=-4.5)", S_OK);
  // MinLOD
  TestHLSLRootSignatureCase("StaticSampler(s2, maxLOD=5.77)", S_OK);
  // Space
  TestHLSLRootSignatureCase("StaticSampler(s2, space=7)", S_OK);
  TestHLSLRootSignatureCase("StaticSampler(s2, space=7, space=9)", E_FAIL);
  // Visibility
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, visibility=SHADER_visibility_ALL)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(visibility=SHADER_VISIBILITY_VERTEX, s2)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, visibility=SHADER_VISIBILITY_HULL)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, visibility=SHADER_VISIBILITY_DOMAIN)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, visibility=SHADER_VISIBILITY_GEOMETRY)", S_OK);
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, visibility=SHADER_VISIBILITY_PIXEL)", S_OK);
  // StaticSampler complex
  TestHLSLRootSignatureCase(
      "StaticSampler(s2, "
      "              Filter = FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR, "
      "              AddressU = TEXTURE_ADDRESS_WRAP, "
      "              AddressV = TEXTURE_ADDRESS_CLAMP, "
      "              AddressW = TEXTURE_ADDRESS_MIRROR_ONCE, "
      "              MaxAnisotropy = 8, "
      "              ComparisonFunc = COMPARISON_NOT_EQUAL, "
      "              BorderColor = STATIC_BORDER_COLOR_OPAQUE_BLACK, "
      "              space = 3, "
      "              visibility = SHADER_VISIBILITY_PIXEL), "
      "StaticSampler(s7, "
      "              Filter = FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR, "
      "              AddressU = TEXTURE_ADDRESS_MIRROR_ONCE, "
      "              AddressV = TEXTURE_ADDRESS_WRAP, "
      "              AddressW = TEXTURE_ADDRESS_CLAMP, "
      "              MaxAnisotropy = 1, "
      "              ComparisonFunc = COMPARISON_ALWAYS, "
      "              BorderColor = STATIC_BORDER_COLOR_OPAQUE_BLACK, "
      "              space = 3, "
      "              visibility = SHADER_VISIBILITY_HULL), ",
      S_OK);

  // Mixed
  TestHLSLRootSignatureCase("RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), "
                            "DescriptorTable(CBV(b2), "
                            "CBV(b3, numDescriptors = 4, space=3), "
                            "SRV(t4, numDescriptors=3), "
                            "visibility = SHADER_VISIBILITY_HULL), "
                            "UAV(u1, visibility = SHADER_VISIBILITY_PIXEL), "
                            "RootConstants( num32BitConstants=3, b2, space = "
                            "5, visibility = SHADER_VISIBILITY_PIXEL), "
                            "DescriptorTable(CBV(b20, space=4)), "
                            "DescriptorTable(CBV(b20, space=9)), "
                            "RootConstants( num32BitConstants=8, b2, "
                            "visibility = SHADER_VISIBILITY_PIXEL), "
                            "SRV(t9, space = 0)",
                            S_OK);
  TestHLSLRootSignatureCase(
      "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), "
      "DescriptorTable(CBV(b2), "
      "CBV(b3, numDescriptors = 4, space=3), "
      "SRV(t4, numDescriptors=3), "
      "visibility = SHADER_VISIBILITY_HULL), "
      "UAV(u1, visibility = SHADER_VISIBILITY_PIXEL), "
      "RootConstants( num32BitConstants=3, b2, space = 5, visibility = "
      "SHADER_VISIBILITY_PIXEL), "
      "DescriptorTable(CBV(b20, space=4, numDescriptors = unbounded)), "
      "DescriptorTable(CBV(b20, space=9)), "
      "RootConstants( num32BitConstants=8, b2, visibility = "
      "SHADER_VISIBILITY_PIXEL), "
      "SRV(t9, space = 0), "
      "StaticSampler(s7, "
      "              Filter = FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR, "
      "              AddressU = TEXTURE_ADDRESS_MIRROR_ONCE, "
      "              AddressV = TEXTURE_ADDRESS_WRAP, "
      "              AddressW = TEXTURE_ADDRESS_CLAMP, "
      "              MaxAnisotropy = 12, "
      "              ComparisonFunc = COMPARISON_ALWAYS, "
      "              BorderColor = STATIC_BORDER_COLOR_OPAQUE_BLACK, "
      "              space = 3, "
      "              visibility = SHADER_VISIBILITY_HULL), ",
      S_OK);
}
