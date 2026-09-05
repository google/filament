///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// SystemValueTest.cpp                                                       //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Test system values at various signature points                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/WinIncludes.h"
#include "dxc/dxcapi.h"
#include <algorithm>
#include <cassert>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "dxc/Test/DxcTestUtils.h"
#include "dxc/Test/HlslTestUtils.h"

#include "dxc/Support/Global.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/dxcapi.use.h"
#include "llvm/Support/raw_os_ostream.h"

#include "dxc/DXIL/DxilConstants.h"
#include "dxc/DXIL/DxilSemantic.h"
#include "dxc/DXIL/DxilShaderModel.h"
#include "dxc/DXIL/DxilSigPoint.h"

#include <fstream>

using namespace std;
using namespace hlsl_test;
using namespace hlsl;

#ifdef _WIN32
class SystemValueTest {
#else
class SystemValueTest : public ::testing::Test {
#endif
public:
  BEGIN_TEST_CLASS(SystemValueTest)
  TEST_CLASS_PROPERTY(L"Parallel", L"true")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_CLASS_SETUP(InitSupport);

  TEST_METHOD(VerifyArbitrarySupport)
  TEST_METHOD(VerifyNotAvailableFail)
  TEST_METHOD(VerifySVAsArbitrary)
  TEST_METHOD(VerifySVAsSV)
  TEST_METHOD(VerifySGV)
  TEST_METHOD(VerifySVNotPacked)
  TEST_METHOD(VerifySVNotInSig)
  TEST_METHOD(VerifyVertexPacking)
  TEST_METHOD(VerifyPatchConstantPacking)
  TEST_METHOD(VerifyTargetPacking)
  TEST_METHOD(VerifyTessFactors)
  TEST_METHOD(VerifyShadowEntries)
  TEST_METHOD(VerifyVersionedSemantics)
  TEST_METHOD(VerifyMissingSemanticFailure)

  void CompileHLSLTemplate(CComPtr<IDxcOperationResult> &pResult,
                           DXIL::SigPointKind sigPointKind,
                           DXIL::SemanticKind semKind, bool addArb,
                           unsigned Major = 0, unsigned Minor = 0) {
    const Semantic *sem = Semantic::Get(semKind);
    const char *pSemName = sem->GetName();
    std::wstring sigDefValue(L"");
    if (semKind < DXIL::SemanticKind::Invalid && pSemName) {
      if (Semantic::HasSVPrefix(pSemName))
        pSemName += 3;
      CA2W semNameW(pSemName);
      sigDefValue = L"Def_";
      sigDefValue += semNameW;
    }
    if (addArb) {
      if (!sigDefValue.empty())
        sigDefValue += L" ";
      sigDefValue += L"Def_Arb(uint, arb0, ARB0)";
    }
    return CompileHLSLTemplate(pResult, sigPointKind, sigDefValue, Major,
                               Minor);
  }

  void CompileHLSLTemplate(CComPtr<IDxcOperationResult> &pResult,
                           DXIL::SigPointKind sigPointKind,
                           const std::wstring &sigDefValue, unsigned Major = 0,
                           unsigned Minor = 0) {
    const SigPoint *sigPoint = SigPoint::GetSigPoint(sigPointKind);
    DXIL::ShaderKind shaderKind = sigPoint->GetShaderKind();

    std::wstring path = hlsl_test::GetPathToHlslDataFile(L"system-values.hlsl");

    CComPtr<IDxcCompiler> pCompiler;

    VERIFY_SUCCEEDED(
        m_dllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));

    if (!m_pSource) {
      CComPtr<IDxcLibrary> library;
      IFT(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &library));
      IFT(library->CreateBlobFromFile(path.c_str(), nullptr, &m_pSource));
    }

    LPCWSTR entry, profile;
    wchar_t profile_buf[] = L"vs_6_1";
    switch (shaderKind) {
    case DXIL::ShaderKind::Vertex:
      entry = L"VSMain";
      profile = L"vs_6_1";
      break;
    case DXIL::ShaderKind::Pixel:
      entry = L"PSMain";
      profile = L"ps_6_1";
      break;
    case DXIL::ShaderKind::Geometry:
      entry = L"GSMain";
      profile = L"gs_6_1";
      break;
    case DXIL::ShaderKind::Hull:
      entry = L"HSMain";
      profile = L"hs_6_1";
      break;
    case DXIL::ShaderKind::Domain:
      entry = L"DSMain";
      profile = L"ds_6_1";
      break;
    case DXIL::ShaderKind::Compute:
      entry = L"CSMain";
      profile = L"cs_6_1";
      break;
    case DXIL::ShaderKind::Mesh:
      entry = L"MSMain";
      profile = L"ms_6_5";
      break;
    case DXIL::ShaderKind::Amplification:
      entry = L"ASMain";
      profile = L"as_6_5";
      break;
    case DXIL::ShaderKind::Library:
    case DXIL::ShaderKind::Invalid:
      assert(!"invalid shaderKind");
      break;
    }
    if (Major == 0) {
      Major = m_HighestMajor;
      Minor = m_HighestMinor;
    }
    if (Major != 6 || Minor != 1) {
      profile_buf[0] = profile[0];
      profile_buf[3] = L'0' + (wchar_t)Major;
      profile_buf[5] = L'0' + (wchar_t)Minor;
      profile = profile_buf;
    }

    CA2W sigPointNameW(sigPoint->GetName());
    // Strip SV_ from semantic name
    std::wstring sigDefName(sigPointNameW);
    sigDefName += L"_Defs";
    DxcDefine define;
    define.Name = sigDefName.c_str();
    define.Value = sigDefValue.c_str();

    VERIFY_SUCCEEDED(pCompiler->Compile(m_pSource, path.c_str(), entry, profile,
                                        nullptr, 0, &define, 1, nullptr,
                                        &pResult));
  }

  void CheckAnyOperationResultMsg(IDxcOperationResult *pResult,
                                  const char **pErrorMsgArray = nullptr,
                                  unsigned ErrorMsgCount = 0) {
    HRESULT status;
    VERIFY_SUCCEEDED(pResult->GetStatus(&status));
    if (pErrorMsgArray == nullptr || ErrorMsgCount == 0) {
      VERIFY_SUCCEEDED(status);
      return;
    }
    VERIFY_FAILED(status);
    CComPtr<IDxcBlobEncoding> text;
    VERIFY_SUCCEEDED(pResult->GetErrorBuffer(&text));
    const char *pStart = (const char *)text->GetBufferPointer();
    const char *pEnd = pStart + text->GetBufferSize();
    bool bMessageFound = false;
    for (unsigned i = 0; i < ErrorMsgCount; i++) {
      const char *pErrorMsg = pErrorMsgArray[i];
      const char *pMatch =
          std::search(pStart, pEnd, pErrorMsg, pErrorMsg + strlen(pErrorMsg));
      if (pEnd != pMatch)
        bMessageFound = true;
    }
    VERIFY_IS_TRUE(bMessageFound);
  }

  dxc::DxCompilerDllLoader m_dllSupport;
  VersionSupportInfo m_ver;
  unsigned m_HighestMajor, m_HighestMinor; // Shader Model Supported

  CComPtr<IDxcBlobEncoding> m_pSource;
};

bool SystemValueTest::InitSupport() {
  if (!m_dllSupport.IsEnabled()) {
    VERIFY_SUCCEEDED(m_dllSupport.Initialize());
    m_ver.Initialize(m_dllSupport);
    m_HighestMajor = 6;
    m_HighestMinor = 0;
    if ((m_ver.m_DxilMajor > 1 ||
         (m_ver.m_DxilMajor == 1 && m_ver.m_DxilMinor > 1)) &&
        (m_ver.m_ValMajor > 1 ||
         (m_ver.m_ValMajor == 1 && m_ver.m_ValMinor > 1))) {
      m_HighestMinor = 1;
    }
  }
  return true;
}

static bool ArbAllowed(DXIL::SigPointKind sp) {
  switch (sp) {
  case DXIL::SigPointKind::VSIn:
  case DXIL::SigPointKind::VSOut:
  case DXIL::SigPointKind::GSVIn:
  case DXIL::SigPointKind::GSOut:
  case DXIL::SigPointKind::HSCPIn:
  case DXIL::SigPointKind::HSCPOut:
  case DXIL::SigPointKind::PCOut:
  case DXIL::SigPointKind::DSCPIn:
  case DXIL::SigPointKind::DSIn:
  case DXIL::SigPointKind::DSOut:
  case DXIL::SigPointKind::PSIn:
  case DXIL::SigPointKind::MSOut:
  case DXIL::SigPointKind::MSPOut:
    return true;
  default:
    return false;
  }
  return false;
}

TEST_F(SystemValueTest, VerifyArbitrarySupport) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  for (DXIL::SigPointKind sp = (DXIL::SigPointKind)0;
       sp < DXIL::SigPointKind::Invalid;
       sp = (DXIL::SigPointKind)((unsigned)sp + 1)) {
    if (sp >= DXIL::SigPointKind::MSIn && sp <= DXIL::SigPointKind::ASIn) {
      // TODO: add tests for mesh/amplification shaders to system-values.hlsl
      continue;
    }
    CComPtr<IDxcOperationResult> pResult;
    CompileHLSLTemplate(pResult, sp, DXIL::SemanticKind::Invalid, true);
    HRESULT result;
    VERIFY_SUCCEEDED(pResult->GetStatus(&result));
    if (ArbAllowed(sp)) {
      CheckAnyOperationResultMsg(pResult);
    } else {
      // TODO: We should probably improve this error message since it pertains
      // to a parameter at a particular signature point, not necessarily the
      // whole shader model. These are a couple of possible errors: error:
      // invalid semantic 'ARB' for <sm> error: Semantic ARB is invalid for
      // shader model <sm> error: invalid semantic found in <sm>
      const char *Errors[] = {
          "error: Semantic ARB is invalid for shader model",
          "error: invalid semantic 'ARB' for",
          "error: invalid semantic found in CS",
      };
      CheckAnyOperationResultMsg(pResult, Errors, _countof(Errors));
    }
  }
}

TEST_F(SystemValueTest, VerifyNotAvailableFail) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  for (DXIL::SigPointKind sp = (DXIL::SigPointKind)0;
       sp < DXIL::SigPointKind::Invalid;
       sp = (DXIL::SigPointKind)((unsigned)sp + 1)) {
    if (sp >= DXIL::SigPointKind::MSIn && sp <= DXIL::SigPointKind::ASIn) {
      // TODO: add tests for mesh/amplification shaders to system-values.hlsl
      continue;
    }
    for (DXIL::SemanticKind sv =
             (DXIL::SemanticKind)((unsigned)DXIL::SemanticKind::Arbitrary + 1);
         sv < DXIL::SemanticKind::Invalid;
         sv = (DXIL::SemanticKind)((unsigned)sv + 1)) {
      if (sv == DXIL::SemanticKind::CullPrimitive) {
        // TODO: add tests for CullPrimitive
        continue;
      }
      DXIL::SemanticInterpretationKind interpretation =
          hlsl::SigPoint::GetInterpretation(sv, sp, m_HighestMajor,
                                            m_HighestMinor);
      if (interpretation == DXIL::SemanticInterpretationKind::NA) {
        CComPtr<IDxcOperationResult> pResult;
        CompileHLSLTemplate(pResult, sp, sv, false);
        // error: Semantic SV_SampleIndex is invalid for shader model: vs
        // error: invalid semantic 'SV_VertexID' for gs
        // error: invalid semantic found in CS
        const Semantic *pSemantic = Semantic::Get(sv);
        const char *SemName = pSemantic->GetName();
        std::string ErrorStrs[] = {
            std::string("error: Semantic ") + SemName +
                " is invalid for shader model:",
            std::string("error: invalid semantic '") + SemName + "' for",
            "error: invalid semantic found in CS",
        };
        const char *Errors[_countof(ErrorStrs)];
        for (unsigned i = 0; i < _countof(ErrorStrs); i++)
          Errors[i] = ErrorStrs[i].c_str();
        CheckAnyOperationResultMsg(pResult, Errors, _countof(Errors));
      }
    }
  }
}

TEST_F(SystemValueTest, VerifySVAsArbitrary) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  for (DXIL::SigPointKind sp = (DXIL::SigPointKind)0;
       sp < DXIL::SigPointKind::Invalid;
       sp = (DXIL::SigPointKind)((unsigned)sp + 1)) {
    if (sp >= DXIL::SigPointKind::MSIn && sp <= DXIL::SigPointKind::ASIn) {
      // TODO: add tests for mesh/amplification shaders to system-values.hlsl
      continue;
    }
    for (DXIL::SemanticKind sv =
             (DXIL::SemanticKind)((unsigned)DXIL::SemanticKind::Arbitrary + 1);
         sv < DXIL::SemanticKind::Invalid;
         sv = (DXIL::SemanticKind)((unsigned)sv + 1)) {
      DXIL::SemanticInterpretationKind interpretation =
          hlsl::SigPoint::GetInterpretation(sv, sp, m_HighestMajor,
                                            m_HighestMinor);
      if (interpretation == DXIL::SemanticInterpretationKind::Arb) {
        CComPtr<IDxcOperationResult> pResult;
        CompileHLSLTemplate(pResult, sp, sv, false);
        HRESULT result;
        VERIFY_SUCCEEDED(pResult->GetStatus(&result));
        VERIFY_SUCCEEDED(result);
        // TODO: Verify system value item is included in signature, treated as
        // arbitrary,
        //  and that the element id is used in load input instruction.
      }
    }
  }
}

TEST_F(SystemValueTest, VerifySVAsSV) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  for (DXIL::SigPointKind sp = (DXIL::SigPointKind)0;
       sp < DXIL::SigPointKind::Invalid;
       sp = (DXIL::SigPointKind)((unsigned)sp + 1)) {
    if (sp >= DXIL::SigPointKind::MSIn && sp <= DXIL::SigPointKind::ASIn) {
      // TODO: add tests for mesh/amplification shaders to system-values.hlsl
      continue;
    }
    for (DXIL::SemanticKind sv =
             (DXIL::SemanticKind)((unsigned)DXIL::SemanticKind::Arbitrary + 1);
         sv < DXIL::SemanticKind::Invalid;
         sv = (DXIL::SemanticKind)((unsigned)sv + 1)) {
      DXIL::SemanticInterpretationKind interpretation =
          hlsl::SigPoint::GetInterpretation(sv, sp, m_HighestMajor,
                                            m_HighestMinor);
      if (interpretation == DXIL::SemanticInterpretationKind::SV ||
          interpretation == DXIL::SemanticInterpretationKind::SGV) {
        CComPtr<IDxcOperationResult> pResult;
        CompileHLSLTemplate(pResult, sp, sv, false);
        HRESULT result;
        VERIFY_SUCCEEDED(pResult->GetStatus(&result));
        VERIFY_SUCCEEDED(result);
        // TODO: Verify system value is included in signature, system value enum
        // is appropriately set,
        //  and that the element id is used in load input instruction.
      }
    }
  }
}

TEST_F(SystemValueTest, VerifySGV) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  for (DXIL::SigPointKind sp = (DXIL::SigPointKind)0;
       sp < DXIL::SigPointKind::Invalid;
       sp = (DXIL::SigPointKind)((unsigned)sp + 1)) {
    if (sp >= DXIL::SigPointKind::MSIn && sp <= DXIL::SigPointKind::ASIn) {
      // TODO: add tests for mesh/amplification shaders to system-values.hlsl
      continue;
    }
    for (DXIL::SemanticKind sv =
             (DXIL::SemanticKind)((unsigned)DXIL::SemanticKind::Arbitrary + 1);
         sv < DXIL::SemanticKind::Invalid;
         sv = (DXIL::SemanticKind)((unsigned)sv + 1)) {
      DXIL::SemanticInterpretationKind interpretation =
          hlsl::SigPoint::GetInterpretation(sv, sp, m_HighestMajor,
                                            m_HighestMinor);
      if (interpretation == DXIL::SemanticInterpretationKind::SGV) {
        CComPtr<IDxcOperationResult> pResult;
        CompileHLSLTemplate(pResult, sp, sv, true);
        HRESULT result;
        VERIFY_SUCCEEDED(pResult->GetStatus(&result));
        VERIFY_SUCCEEDED(result);
        // TODO: Verify system value is included in signature and arbitrary is
        // packed before system value
        //  Or: verify failed when using greedy signature packing
        // TODO: Verify warning about declaring the system value last for fxc
        // HLSL compatibility.
      }
    }
  }
}

TEST_F(SystemValueTest, VerifySVNotPacked) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  for (DXIL::SigPointKind sp = (DXIL::SigPointKind)0;
       sp < DXIL::SigPointKind::Invalid;
       sp = (DXIL::SigPointKind)((unsigned)sp + 1)) {
    if (sp >= DXIL::SigPointKind::MSIn && sp <= DXIL::SigPointKind::ASIn) {
      // TODO: add tests for mesh/amplification shaders to system-values.hlsl
      continue;
    }
    for (DXIL::SemanticKind sv =
             (DXIL::SemanticKind)((unsigned)DXIL::SemanticKind::Arbitrary + 1);
         sv < DXIL::SemanticKind::Invalid;
         sv = (DXIL::SemanticKind)((unsigned)sv + 1)) {
      DXIL::SemanticInterpretationKind interpretation =
          hlsl::SigPoint::GetInterpretation(sv, sp, m_HighestMajor,
                                            m_HighestMinor);
      if (interpretation == DXIL::SemanticInterpretationKind::NotPacked) {
        CComPtr<IDxcOperationResult> pResult;
        CompileHLSLTemplate(pResult, sp, sv, false);
        HRESULT result;
        VERIFY_SUCCEEDED(pResult->GetStatus(&result));
        VERIFY_SUCCEEDED(result);
        // TODO: Verify system value is included in signature and has packing
        // location (-1, -1),
        //  and that the element id is used in load input instruction.
      }
    }
  }
}

TEST_F(SystemValueTest, VerifySVNotInSig) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  for (DXIL::SigPointKind sp = (DXIL::SigPointKind)0;
       sp < DXIL::SigPointKind::Invalid;
       sp = (DXIL::SigPointKind)((unsigned)sp + 1)) {
    if (sp >= DXIL::SigPointKind::MSIn && sp <= DXIL::SigPointKind::ASIn) {
      // TODO: add tests for mesh/amplification shaders to system-values.hlsl
      continue;
    }
    for (DXIL::SemanticKind sv =
             (DXIL::SemanticKind)((unsigned)DXIL::SemanticKind::Arbitrary + 1);
         sv < DXIL::SemanticKind::Invalid;
         sv = (DXIL::SemanticKind)((unsigned)sv + 1)) {
      DXIL::SemanticInterpretationKind interpretation =
          hlsl::SigPoint::GetInterpretation(sv, sp, m_HighestMajor,
                                            m_HighestMinor);
      if (interpretation == DXIL::SemanticInterpretationKind::NotInSig) {
        CComPtr<IDxcOperationResult> pResult;
        CompileHLSLTemplate(pResult, sp, sv, false);
        HRESULT result;
        VERIFY_SUCCEEDED(pResult->GetStatus(&result));
        VERIFY_SUCCEEDED(result);
        // TODO: Verify system value is not included in signature,
        //  that intrinsic function is used, and that the element id is not used
        //  in load input instruction.
      }
    }
  }
}

TEST_F(SystemValueTest, VerifyVertexPacking) {
  // TODO: Implement
  VERIFY_IS_TRUE("Not Implemented");

  for (DXIL::SigPointKind sp = (DXIL::SigPointKind)0;
       sp < DXIL::SigPointKind::Invalid;
       sp = (DXIL::SigPointKind)((unsigned)sp + 1)) {
    if (sp >= DXIL::SigPointKind::MSIn && sp <= DXIL::SigPointKind::ASIn) {
      // TODO: add tests for mesh/amplification shaders to system-values.hlsl
      continue;
    }
    DXIL::PackingKind pk = SigPoint::GetSigPoint(sp)->GetPackingKind();
    if (pk == DXIL::PackingKind::Vertex) {
      // TBD: Test constraints here, or add constraints to validator and just
      // generate cases to pack here, expecting success?
    }
  }
}

TEST_F(SystemValueTest, VerifyPatchConstantPacking) {
  // TODO: Implement
  VERIFY_IS_TRUE("Not Implemented");

  for (DXIL::SigPointKind sp = (DXIL::SigPointKind)0;
       sp < DXIL::SigPointKind::Invalid;
       sp = (DXIL::SigPointKind)((unsigned)sp + 1)) {
    if (sp >= DXIL::SigPointKind::MSIn && sp <= DXIL::SigPointKind::ASIn) {
      // TODO: add tests for mesh/amplification shaders to system-values.hlsl
      continue;
    }
    DXIL::PackingKind pk = SigPoint::GetSigPoint(sp)->GetPackingKind();
    if (pk == DXIL::PackingKind::PatchConstant) {
      // TBD: Test constraints here, or add constraints to validator and just
      // generate cases to pack here, expecting success?
    }
  }
}

TEST_F(SystemValueTest, VerifyTargetPacking) {
  // TODO: Implement
  VERIFY_IS_TRUE("Not Implemented");

  for (DXIL::SigPointKind sp = (DXIL::SigPointKind)0;
       sp < DXIL::SigPointKind::Invalid;
       sp = (DXIL::SigPointKind)((unsigned)sp + 1)) {
    if (sp >= DXIL::SigPointKind::MSIn && sp <= DXIL::SigPointKind::ASIn) {
      // TODO: add tests for mesh/amplification shaders to system-values.hlsl
      continue;
    }
    DXIL::PackingKind pk = SigPoint::GetSigPoint(sp)->GetPackingKind();
    if (pk == DXIL::PackingKind::Target) {
      // TBD: Test constraints here, or add constraints to validator and just
      // generate cases to pack here, expecting success?
    }
  }
}

TEST_F(SystemValueTest, VerifyTessFactors) {
  // TODO: Implement
  VERIFY_IS_TRUE("Not Implemented");
  // TBD: Split between return and out params?
}

TEST_F(SystemValueTest, VerifyShadowEntries) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  for (DXIL::SigPointKind sp = (DXIL::SigPointKind)0;
       sp < DXIL::SigPointKind::Invalid;
       sp = (DXIL::SigPointKind)((unsigned)sp + 1)) {
    if (sp >= DXIL::SigPointKind::MSIn && sp <= DXIL::SigPointKind::ASIn) {
      // TODO: add tests for mesh/amplification shaders to system-values.hlsl
      continue;
    }
    for (DXIL::SemanticKind sv =
             (DXIL::SemanticKind)((unsigned)DXIL::SemanticKind::Arbitrary + 1);
         sv < DXIL::SemanticKind::Invalid;
         sv = (DXIL::SemanticKind)((unsigned)sv + 1)) {
      DXIL::SemanticInterpretationKind interpretation =
          hlsl::SigPoint::GetInterpretation(sv, sp, m_HighestMajor,
                                            m_HighestMinor);
      if (interpretation == DXIL::SemanticInterpretationKind::Shadow) {
        CComPtr<IDxcOperationResult> pResult;
        CompileHLSLTemplate(pResult, sp, sv, false);
        HRESULT result;
        VERIFY_SUCCEEDED(pResult->GetStatus(&result));
        VERIFY_SUCCEEDED(result);
        // TODO: Verify system value is included in corresponding signature
        // (with fallback),
        //  that intrinsic function is used, and that the element id is not used
        //  in load input instruction.
      }
    }
  }
}

TEST_F(SystemValueTest, VerifyVersionedSemantics) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  struct TestInfo {
    DXIL::SigPointKind sp;
    DXIL::SemanticKind sv;
    unsigned Major, Minor;
  };
  const unsigned kNumTests = 13;
  TestInfo info[kNumTests] = {
      {DXIL::SigPointKind::PSIn, DXIL::SemanticKind::SampleIndex, 4, 1},
      {DXIL::SigPointKind::PSIn, DXIL::SemanticKind::Coverage, 5, 0},
      {DXIL::SigPointKind::PSOut, DXIL::SemanticKind::Coverage, 4, 1},
      {DXIL::SigPointKind::PSIn, DXIL::SemanticKind::InnerCoverage, 5, 0},
      {DXIL::SigPointKind::PSOut, DXIL::SemanticKind::DepthLessEqual, 5, 0},
      {DXIL::SigPointKind::PSOut, DXIL::SemanticKind::DepthGreaterEqual, 5, 0},
      {DXIL::SigPointKind::PSOut, DXIL::SemanticKind::StencilRef, 5, 0},
      {DXIL::SigPointKind::VSIn, DXIL::SemanticKind::ViewID, 6, 1},
      {DXIL::SigPointKind::HSIn, DXIL::SemanticKind::ViewID, 6, 1},
      {DXIL::SigPointKind::PCIn, DXIL::SemanticKind::ViewID, 6, 1},
      {DXIL::SigPointKind::DSIn, DXIL::SemanticKind::ViewID, 6, 1},
      {DXIL::SigPointKind::GSIn, DXIL::SemanticKind::ViewID, 6, 1},
      {DXIL::SigPointKind::PSIn, DXIL::SemanticKind::ViewID, 6, 1},
  };

  for (unsigned i = 0; i < kNumTests; i++) {
    TestInfo &test = info[i];
    unsigned MajorLower = test.Major, MinorLower = test.Minor;
    if (MinorLower > 0)
      MinorLower--;
    else {
      MajorLower--;
      MinorLower = 1;
    }
    DXIL::SemanticInterpretationKind SI = hlsl::SigPoint::GetInterpretation(
        test.sv, test.sp, test.Major, test.Minor);
    VERIFY_IS_TRUE(SI != DXIL::SemanticInterpretationKind::NA);
    DXIL::SemanticInterpretationKind SILower =
        hlsl::SigPoint::GetInterpretation(test.sv, test.sp, MajorLower,
                                          MinorLower);
    VERIFY_IS_TRUE(SILower == DXIL::SemanticInterpretationKind::NA);

    // Don't try compiling to pre-dxil targets:
    if (MajorLower < 6)
      continue;

    // Don't try targets our compiler/validator combination do not support.
    if (test.Major > m_HighestMajor || test.Minor > m_HighestMinor)
      continue;

    {
      CComPtr<IDxcOperationResult> pResult;
      CompileHLSLTemplate(pResult, test.sp, test.sv, false, test.Major,
                          test.Minor);
      HRESULT result;
      VERIFY_SUCCEEDED(pResult->GetStatus(&result));
      VERIFY_SUCCEEDED(result);
    }
    {
      CComPtr<IDxcOperationResult> pResult;
      CompileHLSLTemplate(pResult, test.sp, test.sv, false, MajorLower,
                          MinorLower);
      HRESULT result;
      VERIFY_SUCCEEDED(pResult->GetStatus(&result));
      VERIFY_FAILED(result);
      const char *Errors[] = {"is invalid for shader model",
                              "error: invalid semantic"};
      CheckAnyOperationResultMsg(pResult, Errors, _countof(Errors));
    }
  }
}

TEST_F(SystemValueTest, VerifyMissingSemanticFailure) {
  WEX::TestExecution::SetVerifyOutput verifySettings(
      WEX::TestExecution::VerifyOutputSettings::LogOnlyFailures);
  for (DXIL::SigPointKind sp = (DXIL::SigPointKind)0;
       sp < DXIL::SigPointKind::Invalid;
       sp = (DXIL::SigPointKind)((unsigned)sp + 1)) {
    if (sp >= DXIL::SigPointKind::MSIn && sp <= DXIL::SigPointKind::ASIn) {
      // TODO: add tests for mesh/amplification shaders to system-values.hlsl
      continue;
    }

    std::wstring sigDefValue(L"Def_Arb_NoSem(uint, arb0)");
    CComPtr<IDxcOperationResult> pResult;
    CompileHLSLTemplate(pResult, sp, sigDefValue);
    const char *Errors[] = {
        "error: Semantic must be defined for all parameters of an entry "
        "function or patch constant function",
        "error: Semantic must be defined for all outputs of an entry function "
        "or patch constant function"};
    CheckAnyOperationResultMsg(pResult, Errors, _countof(Errors));
  }
}
