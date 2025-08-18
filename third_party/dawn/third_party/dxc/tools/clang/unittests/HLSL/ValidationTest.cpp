///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ValidationTest.cpp                                                        //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define NOMINMAX

#include "dxc/Support/WinIncludes.h"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/DxilContainer/DxilContainerAssembler.h"
#include "dxc/DxilContainer/DxilPipelineStateValidation.h"
#include "dxc/DxilHash/DxilHash.h"
#include "dxc/Support/WinIncludes.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Regex.h"

#ifdef _WIN32
#include <atlbase.h>
#endif
#include "dxc/Support/FileIOHelper.h"
#include "dxc/Support/Global.h"

#include "dxc/DXIL/DxilShaderModel.h"
#include "dxc/Test/DxcTestUtils.h"
#include "dxc/Test/HlslTestUtils.h"

using namespace std;
using namespace hlsl;

#ifdef _WIN32
class ValidationTest {
#else
class ValidationTest : public ::testing::Test {
#endif
public:
  BEGIN_TEST_CLASS(ValidationTest)
  TEST_CLASS_PROPERTY(L"Parallel", L"true")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_CLASS_SETUP(InitSupport);

  TEST_METHOD(WhenCorrectThenOK)
  TEST_METHOD(WhenMisalignedThenFail)
  TEST_METHOD(WhenEmptyFileThenFail)
  TEST_METHOD(WhenIncorrectMagicThenFail)
  TEST_METHOD(WhenIncorrectTargetTripleThenFail)
  TEST_METHOD(WhenIncorrectModelThenFail)
  TEST_METHOD(WhenIncorrectPSThenFail)

  TEST_METHOD(WhenWaveAffectsGradientThenFail)

  TEST_METHOD(WhenMultipleModulesThenFail)
  TEST_METHOD(WhenUnexpectedEOFThenFail)
  TEST_METHOD(WhenUnknownBlocksThenFail)
  TEST_METHOD(WhenZeroInputPatchCountWithInputThenFail)

  TEST_METHOD(Float32DenormModeAttribute)
  TEST_METHOD(LoadOutputControlPointNotInPatchConstantFunction)
  TEST_METHOD(StorePatchControlNotInPatchConstantFunction)
  TEST_METHOD(OutputControlPointIDInPatchConstantFunction)
  TEST_METHOD(GsVertexIDOutOfBound)
  TEST_METHOD(StreamIDOutOfBound)
  TEST_METHOD(SignatureDataWidth)
  TEST_METHOD(SignatureStreamIDForNonGS)
  TEST_METHOD(TypedUAVStoreFullMask0)
  TEST_METHOD(TypedUAVStoreFullMask1)
  TEST_METHOD(UAVStoreMaskMatch)
  TEST_METHOD(UAVStoreMaskGap)
  TEST_METHOD(UAVStoreMaskGap2)
  TEST_METHOD(UAVStoreMaskGap3)
  TEST_METHOD(Recursive)
  TEST_METHOD(ResourceRangeOverlap0)
  TEST_METHOD(ResourceRangeOverlap1)
  TEST_METHOD(ResourceRangeOverlap2)
  TEST_METHOD(ResourceRangeOverlap3)
  TEST_METHOD(CBufferOverlap0)
  TEST_METHOD(CBufferOverlap1)
  TEST_METHOD(ControlFlowHint)
  TEST_METHOD(ControlFlowHint1)
  TEST_METHOD(ControlFlowHint2)
  TEST_METHOD(SemanticLength1)
  TEST_METHOD(SemanticLength64)
  TEST_METHOD(PullModelPosition)
  TEST_METHOD(StructBufStrideAlign)
  TEST_METHOD(StructBufStrideOutOfBound)
  TEST_METHOD(StructBufGlobalCoherentAndCounter)
  TEST_METHOD(StructBufLoadCoordinates)
  TEST_METHOD(StructBufStoreCoordinates)
  TEST_METHOD(TypedBufRetType)
  TEST_METHOD(VsInputSemantic)
  TEST_METHOD(VsOutputSemantic)
  TEST_METHOD(HsInputSemantic)
  TEST_METHOD(HsOutputSemantic)
  TEST_METHOD(PatchConstSemantic)
  TEST_METHOD(DsInputSemantic)
  TEST_METHOD(DsOutputSemantic)
  TEST_METHOD(GsInputSemantic)
  TEST_METHOD(GsOutputSemantic)
  TEST_METHOD(PsInputSemantic)
  TEST_METHOD(PsOutputSemantic)
  TEST_METHOD(ArrayOfSVTarget)
  TEST_METHOD(InfiniteLog)
  TEST_METHOD(InfiniteAsin)
  TEST_METHOD(InfiniteAcos)
  TEST_METHOD(InfiniteDdxDdy)
  TEST_METHOD(IDivByZero)
  TEST_METHOD(UDivByZero)
  TEST_METHOD(UnusedMetadata)
  TEST_METHOD(MemoryOutOfBound)
  TEST_METHOD(LocalRes2)
  TEST_METHOD(LocalRes3)
  TEST_METHOD(LocalRes5)
  TEST_METHOD(LocalRes5Dbg)
  TEST_METHOD(LocalRes6)
  TEST_METHOD(LocalRes6Dbg)
  TEST_METHOD(AddrSpaceCast)
  TEST_METHOD(PtrBitCast)
  TEST_METHOD(MinPrecisionBitCast)
  TEST_METHOD(StructBitCast)
  TEST_METHOD(MultiDimArray)
  TEST_METHOD(SimpleGs8)
  TEST_METHOD(SimpleGs9)
  TEST_METHOD(SimpleGs10)
  TEST_METHOD(IllegalSampleOffset3)
  TEST_METHOD(IllegalSampleOffset4)
  TEST_METHOD(NoFunctionParam)
  TEST_METHOD(I8Type)

  // TODO: enable this.
  // TEST_METHOD(TGSMRaceCond)
  // TEST_METHOD(TGSMRaceCond2)
  TEST_METHOD(AddUint64Odd)

  TEST_METHOD(BarycentricFloat4Fail)
  TEST_METHOD(BarycentricMaxIndexFail)
  TEST_METHOD(BarycentricNoInterpolationFail)
  TEST_METHOD(BarycentricSamePerspectiveFail)
  TEST_METHOD(ClipCullMaxComponents)
  TEST_METHOD(ClipCullMaxRows)
  TEST_METHOD(DuplicateSysValue)
  TEST_METHOD(FunctionAttributes)
  TEST_METHOD(GSMainMissingAttributeFail)
  TEST_METHOD(GSOtherMissingAttributeFail)
  TEST_METHOD(GetAttributeAtVertexInVSFail)
  TEST_METHOD(GetAttributeAtVertexIn60Fail)
  TEST_METHOD(GetAttributeAtVertexInterpFail)
  TEST_METHOD(SemTargetMax)
  TEST_METHOD(SemTargetIndexMatchesRow)
  TEST_METHOD(SemTargetCol0)
  TEST_METHOD(SemIndexMax)
  TEST_METHOD(SemTessFactorIndexMax)
  TEST_METHOD(SemInsideTessFactorIndexMax)
  TEST_METHOD(SemShouldBeAllocated)
  TEST_METHOD(SemShouldNotBeAllocated)
  TEST_METHOD(SemComponentOrder)
  TEST_METHOD(SemComponentOrder2)
  TEST_METHOD(SemComponentOrder3)
  TEST_METHOD(SemIndexConflictArbSV)
  TEST_METHOD(SemIndexConflictTessfactors)
  TEST_METHOD(SemIndexConflictTessfactors2)
  TEST_METHOD(SemRowOutOfRange)
  TEST_METHOD(SemPackOverlap)
  TEST_METHOD(SemPackOverlap2)
  TEST_METHOD(SemMultiDepth)

  TEST_METHOD(WhenInstrDisallowedThenFail)
  TEST_METHOD(WhenDepthNotFloatThenFail)
  TEST_METHOD(BarrierFail)
  TEST_METHOD(CBufferLegacyOutOfBoundFail)
  TEST_METHOD(CsThreadSizeFail)
  TEST_METHOD(DeadLoopFail)
  TEST_METHOD(EvalFail)
  TEST_METHOD(GetDimCalcLODFail)
  TEST_METHOD(HsAttributeFail)
  TEST_METHOD(InnerCoverageFail)
  TEST_METHOD(InterpChangeFail)
  TEST_METHOD(InterpOnIntFail)
  TEST_METHOD(InvalidSigCompTyFail)
  TEST_METHOD(MultiStream2Fail)
  TEST_METHOD(PhiTGSMFail)
  TEST_METHOD(QuadOpInVS)
  TEST_METHOD(ReducibleFail)
  TEST_METHOD(SampleBiasFail)
  TEST_METHOD(SamplerKindFail)
  TEST_METHOD(SemaOverlapFail)
  TEST_METHOD(SigOutOfRangeFail)
  TEST_METHOD(SigOverlapFail)
  TEST_METHOD(SimpleHs1Fail)
  TEST_METHOD(SimpleHs3Fail)
  TEST_METHOD(SimpleHs4Fail)
  TEST_METHOD(SimpleDs1Fail)
  TEST_METHOD(SimpleGs1Fail)
  TEST_METHOD(UavBarrierFail)
  TEST_METHOD(UndefValueFail)
  TEST_METHOD(ValidationFailNoHash)
  TEST_METHOD(UpdateCounterFail)
  TEST_METHOD(LocalResCopy)
  TEST_METHOD(ResCounter)

  TEST_METHOD(WhenSmUnknownThenFail)
  TEST_METHOD(WhenSmLegacyThenFail)

  TEST_METHOD(WhenMetaFlagsUsageDeclThenOK)
  TEST_METHOD(WhenMetaFlagsUsageThenFail)

  TEST_METHOD(WhenRootSigMismatchThenFail)
  TEST_METHOD(WhenRootSigCompatThenSucceed)
  TEST_METHOD(WhenRootSigMatchShaderSucceed_RootConstVis)
  TEST_METHOD(WhenRootSigMatchShaderFail_RootConstVis)
  TEST_METHOD(WhenRootSigMatchShaderSucceed_RootCBV)
  TEST_METHOD(WhenRootSigMatchShaderFail_RootCBV_Range)
  TEST_METHOD(WhenRootSigMatchShaderFail_RootCBV_Space)
  TEST_METHOD(WhenRootSigMatchShaderSucceed_RootSRV)
  TEST_METHOD(WhenRootSigMatchShaderFail_RootSRV_ResType)
  TEST_METHOD(WhenRootSigMatchShaderSucceed_RootUAV)
  TEST_METHOD(WhenRootSigMatchShaderSucceed_DescTable)
  TEST_METHOD(WhenRootSigMatchShaderSucceed_DescTable_GoodRange)
  TEST_METHOD(WhenRootSigMatchShaderSucceed_DescTable_Unbounded)
  TEST_METHOD(WhenRootSigMatchShaderFail_DescTable_Range1)
  TEST_METHOD(WhenRootSigMatchShaderFail_DescTable_Range2)
  TEST_METHOD(WhenRootSigMatchShaderFail_DescTable_Range3)
  TEST_METHOD(WhenRootSigMatchShaderFail_DescTable_Space)
  TEST_METHOD(WhenRootSigMatchShaderSucceed_Unbounded)
  TEST_METHOD(WhenRootSigMatchShaderFail_Unbounded1)
  TEST_METHOD(WhenRootSigMatchShaderFail_Unbounded2)
  TEST_METHOD(WhenRootSigMatchShaderFail_Unbounded3)
  TEST_METHOD(WhenProgramOutSigMissingThenFail)
  TEST_METHOD(WhenProgramOutSigUnexpectedThenFail)
  TEST_METHOD(WhenProgramSigMismatchThenFail)
  TEST_METHOD(WhenProgramInSigMissingThenFail)
  TEST_METHOD(WhenProgramSigMismatchThenFail2)
  TEST_METHOD(WhenProgramPCSigMissingThenFail)
  TEST_METHOD(WhenPSVMismatchThenFail)
  TEST_METHOD(WhenRDATMismatchThenFail)
  TEST_METHOD(WhenFeatureInfoMismatchThenFail)
  TEST_METHOD(RayShaderWithSignaturesFail)

  TEST_METHOD(ViewIDInCSFail)
  TEST_METHOD(ViewIDIn60Fail)
  TEST_METHOD(ViewIDNoSpaceFail)

  TEST_METHOD(LibFunctionResInSig)
  TEST_METHOD(RayPayloadIsStruct)
  TEST_METHOD(RayAttrIsStruct)
  TEST_METHOD(CallableParamIsStruct)
  TEST_METHOD(RayShaderExtraArg)
  TEST_METHOD(ResInShaderStruct)
  TEST_METHOD(WhenPayloadSizeTooSmallThenFail)
  TEST_METHOD(WhenMissingPayloadThenFail)
  TEST_METHOD(ShaderFunctionReturnTypeVoid)

  TEST_METHOD(WhenDisassembleInvalidBlobThenFail)

  TEST_METHOD(MeshMultipleSetMeshOutputCounts)
  TEST_METHOD(MeshMissingSetMeshOutputCounts)
  TEST_METHOD(MeshNonDominatingSetMeshOutputCounts)
  TEST_METHOD(MeshOversizePayload)
  TEST_METHOD(MeshOversizeOutput)
  TEST_METHOD(MeshOversizePayloadOutput)
  TEST_METHOD(MeshMultipleGetMeshPayload)
  TEST_METHOD(MeshOutofRangeMaxVertexCount)
  TEST_METHOD(MeshOutofRangeMaxPrimitiveCount)
  TEST_METHOD(MeshLessThanMinX)
  TEST_METHOD(MeshGreaterThanMaxX)
  TEST_METHOD(MeshLessThanMinY)
  TEST_METHOD(MeshGreaterThanMaxY)
  TEST_METHOD(MeshLessThanMinZ)
  TEST_METHOD(MeshGreaterThanMaxZ)
  TEST_METHOD(MeshGreaterThanMaxXYZ)
  TEST_METHOD(MeshGreaterThanMaxVSigRowCount)
  TEST_METHOD(MeshGreaterThanMaxPSigRowCount)
  TEST_METHOD(MeshGreaterThanMaxTotalSigRowCount)
  TEST_METHOD(MeshOversizeSM)
  TEST_METHOD(AmplificationMultipleDispatchMesh)
  TEST_METHOD(AmplificationMissingDispatchMesh)
  TEST_METHOD(AmplificationNonDominatingDispatchMesh)
  TEST_METHOD(AmplificationOversizePayload)
  TEST_METHOD(AmplificationLessThanMinX)
  TEST_METHOD(AmplificationGreaterThanMaxX)
  TEST_METHOD(AmplificationLessThanMinY)
  TEST_METHOD(AmplificationGreaterThanMaxY)
  TEST_METHOD(AmplificationLessThanMinZ)
  TEST_METHOD(AmplificationGreaterThanMaxZ)
  TEST_METHOD(AmplificationGreaterThanMaxXYZ)

  TEST_METHOD(ValidateRootSigContainer)
  TEST_METHOD(ValidatePrintfNotAllowed)

  TEST_METHOD(ValidateWithHash)
  TEST_METHOD(ValidateVersionNotAllowed)
  TEST_METHOD(ValidatePreviewBypassHash)
  TEST_METHOD(ValidateProgramVersionAgainstDxilModule)
  TEST_METHOD(CreateHandleNotAllowedSM66)

  TEST_METHOD(AtomicsConsts)
  TEST_METHOD(AtomicsInvalidDests)
  TEST_METHOD(ComputeNodeCompatibility)
  TEST_METHOD(NodeInputCompatibility)
  TEST_METHOD(NodeInputMultiplicity)

  TEST_METHOD(CacheInitWithMinPrec)
  TEST_METHOD(CacheInitWithLowPrec)

  TEST_METHOD(PSVStringTableReorder)
  TEST_METHOD(PSVSemanticIndexTableReorder)
  TEST_METHOD(PSVContentValidationVS)
  TEST_METHOD(PSVContentValidationHS)
  TEST_METHOD(PSVContentValidationDS)
  TEST_METHOD(PSVContentValidationGS)
  TEST_METHOD(PSVContentValidationPS)
  TEST_METHOD(PSVContentValidationCS)
  TEST_METHOD(PSVContentValidationMS)
  TEST_METHOD(PSVContentValidationAS)
  TEST_METHOD(WrongPSVSize)
  TEST_METHOD(WrongPSVSizeOnZeros)
  TEST_METHOD(WrongPSVVersion)

  dxc::DxcDllSupport m_dllSupport;
  VersionSupportInfo m_ver;

  void TestCheck(LPCWSTR name) {
    std::wstring fullPath = hlsl_test::GetPathToHlslDataFile(name);
    FileRunTestResult t =
        FileRunTestResult::RunFromFileCommands(fullPath.c_str());
    if (t.RunResult != 0) {
      CA2W commentWide(t.ErrorMessage.c_str());
      WEX::Logging::Log::Comment(commentWide);
      WEX::Logging::Log::Error(L"Run result is not zero");
    }
  }

  void CheckValidationMsgs(IDxcBlob *pBlob, llvm::ArrayRef<LPCSTR> pErrorMsgs,
                           bool bRegex = false,
                           UINT32 Flags = DxcValidatorFlags_Default) {
    CComPtr<IDxcValidator> pValidator;
    CComPtr<IDxcOperationResult> pResult;

    if (!IsDxilContainerLike(pBlob->GetBufferPointer(),
                             pBlob->GetBufferSize())) {
      // Validation of raw bitcode as opposed to DxilContainer is not supported
      // through DXIL.dll
      if (!m_ver.m_InternalValidator) {
        WEX::Logging::Log::Comment(
            L"Test skipped due to validation of raw bitcode without container "
            L"and use of external DXIL.dll validator.");
        return;
      }
      Flags |= DxcValidatorFlags_ModuleOnly;
    }

    VERIFY_SUCCEEDED(
        m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
    VERIFY_SUCCEEDED(pValidator->Validate(pBlob, Flags, &pResult));

    CheckOperationResultMsgs(pResult, pErrorMsgs, false, bRegex);
  }

  void CheckValidationMsgs(const char *pBlob, size_t blobSize,
                           llvm::ArrayRef<LPCSTR> pErrorMsgs,
                           bool bRegex = false,
                           UINT32 Flags = DxcValidatorFlags_Default) {
    CComPtr<IDxcLibrary> pLibrary;
    CComPtr<IDxcBlobEncoding>
        pBlobEncoding; // Encoding doesn't actually matter, it's binary.
    VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
    VERIFY_SUCCEEDED(pLibrary->CreateBlobWithEncodingFromPinned(
        pBlob, blobSize, DXC_CP_ACP, &pBlobEncoding));
    CheckValidationMsgs(pBlobEncoding, pErrorMsgs, bRegex, Flags);
  }

  bool CompileSource(IDxcBlobEncoding *pSource, LPCSTR pShaderModel,
                     LPCWSTR *pArguments, UINT32 argCount,
                     const DxcDefine *pDefines, UINT32 defineCount,
                     IDxcBlob **pResultBlob) {
    CComPtr<IDxcCompiler> pCompiler;
    CComPtr<IDxcOperationResult> pResult;
    CComPtr<IDxcBlob> pProgram;

    CA2W shWide(pShaderModel);

    const wchar_t *pEntryName = L"main";

    llvm::StringRef stage;
    unsigned RequiredDxilMajor = 1, RequiredDxilMinor = 0;
    if (ParseTargetProfile(pShaderModel, stage, RequiredDxilMajor,
                           RequiredDxilMinor)) {
      if (stage.compare("lib") == 0)
        pEntryName = L"";
      if (stage.compare("rootsig") != 0) {
        RequiredDxilMajor = std::max(RequiredDxilMajor, (unsigned)6) - 5;
        if (m_ver.SkipDxilVersion(RequiredDxilMajor, RequiredDxilMinor))
          return false;
      }
    }

    std::vector<LPCWSTR> args;
    args.reserve(argCount + 1);
    args.insert(args.begin(), pArguments, pArguments + argCount);
    args.emplace_back(L"-Qkeep_reflect_in_dxil");

    VERIFY_SUCCEEDED(
        m_dllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));
    VERIFY_SUCCEEDED(pCompiler->Compile(
        pSource, L"hlsl.hlsl", pEntryName, shWide, args.data(),
        (UINT32)args.size(), pDefines, defineCount, nullptr, &pResult));
    CheckOperationResultMsgs(pResult, nullptr, false, false);
    VERIFY_SUCCEEDED(pResult->GetResult(pResultBlob));
    return true;
  }

  bool CompileFile(LPCWSTR fileName, LPCSTR pShaderModel,
                   IDxcBlob **pResultBlob) {
    std::wstring fullPath = hlsl_test::GetPathToHlslDataFile(fileName);
    CComPtr<IDxcLibrary> pLibrary;
    CComPtr<IDxcBlobEncoding> pSource;
    VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
    VERIFY_SUCCEEDED(
        pLibrary->CreateBlobFromFile(fullPath.c_str(), nullptr, &pSource));
    return CompileSource(pSource, pShaderModel, nullptr, 0, nullptr, 0,
                         pResultBlob);
  }

  bool CompileFile(LPCWSTR fileName, LPCSTR pShaderModel, LPCWSTR *pArguments,
                   UINT32 argCount, IDxcBlob **pResultBlob) {
    std::wstring fullPath = hlsl_test::GetPathToHlslDataFile(fileName);
    CComPtr<IDxcLibrary> pLibrary;
    CComPtr<IDxcBlobEncoding> pSource;
    VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
    VERIFY_SUCCEEDED(
        pLibrary->CreateBlobFromFile(fullPath.c_str(), nullptr, &pSource));
    return CompileSource(pSource, pShaderModel, pArguments, argCount, nullptr,
                         0, pResultBlob);
  }

  bool CompileSource(IDxcBlobEncoding *pSource, LPCSTR pShaderModel,
                     IDxcBlob **pResultBlob) {
    return CompileSource(pSource, pShaderModel, nullptr, 0, nullptr, 0,
                         pResultBlob);
  }

  bool CompileSource(LPCSTR pSource, LPCSTR pShaderModel,
                     IDxcBlob **pResultBlob) {
    CComPtr<IDxcBlobEncoding> pSourceBlob;
    Utf8ToBlob(m_dllSupport, pSource, &pSourceBlob);
    return CompileSource(pSourceBlob, pShaderModel, nullptr, 0, nullptr, 0,
                         pResultBlob);
  }

  void DisassembleProgram(IDxcBlob *pProgram, std::string *text) {
    *text = ::DisassembleProgram(m_dllSupport, pProgram);
  }

  bool RewriteAssemblyCheckMsg(IDxcBlobEncoding *pSource, LPCSTR pShaderModel,
                               LPCWSTR *pArguments, UINT32 argCount,
                               const DxcDefine *pDefines, UINT32 defineCount,
                               llvm::ArrayRef<LPCSTR> pLookFors,
                               llvm::ArrayRef<LPCSTR> pReplacements,
                               llvm::ArrayRef<LPCSTR> pErrorMsgs,
                               bool bRegex = false) {
    CComPtr<IDxcBlob> pText;
    if (!RewriteAssemblyToText(pSource, pShaderModel, pArguments, argCount,
                               pDefines, defineCount, pLookFors, pReplacements,
                               &pText, bRegex))
      return false;
    CComPtr<IDxcAssembler> pAssembler;
    CComPtr<IDxcOperationResult> pAssembleResult;
    VERIFY_SUCCEEDED(
        m_dllSupport.CreateInstance(CLSID_DxcAssembler, &pAssembler));
    VERIFY_SUCCEEDED(pAssembler->AssembleToContainer(pText, &pAssembleResult));

    if (!CheckOperationResultMsgs(pAssembleResult, pErrorMsgs, true, bRegex)) {
      // Assembly succeeded, try validation.
      CComPtr<IDxcBlob> pBlob;
      VERIFY_SUCCEEDED(pAssembleResult->GetResult(&pBlob));
      CheckValidationMsgs(pBlob, pErrorMsgs, bRegex);
    }
    return true;
  }

  void RewriteAssemblyCheckMsg(LPCSTR pSource, LPCSTR pShaderModel,
                               LPCWSTR *pArguments, UINT32 argCount,
                               const DxcDefine *pDefines, UINT32 defineCount,
                               llvm::ArrayRef<LPCSTR> pLookFors,
                               llvm::ArrayRef<LPCSTR> pReplacements,
                               llvm::ArrayRef<LPCSTR> pErrorMsgs,
                               bool bRegex = false) {
    CComPtr<IDxcBlobEncoding> pSourceBlob;
    Utf8ToBlob(m_dllSupport, pSource, &pSourceBlob);
    RewriteAssemblyCheckMsg(pSourceBlob, pShaderModel, pArguments, argCount,
                            pDefines, defineCount, pLookFors, pReplacements,
                            pErrorMsgs, bRegex);
  }

  void RewriteAssemblyCheckMsg(LPCSTR pSource, LPCSTR pShaderModel,
                               llvm::ArrayRef<LPCSTR> pLookFors,
                               llvm::ArrayRef<LPCSTR> pReplacements,
                               llvm::ArrayRef<LPCSTR> pErrorMsgs,
                               bool bRegex = false) {
    RewriteAssemblyCheckMsg(pSource, pShaderModel, nullptr, 0, nullptr, 0,
                            pLookFors, pReplacements, pErrorMsgs, bRegex);
  }

  void RewriteAssemblyCheckMsg(LPCWSTR name, LPCSTR pShaderModel,
                               LPCWSTR *pArguments, UINT32 argCount,
                               const DxcDefine *pDefines, UINT32 defCount,
                               llvm::ArrayRef<LPCSTR> pLookFors,
                               llvm::ArrayRef<LPCSTR> pReplacements,
                               llvm::ArrayRef<LPCSTR> pErrorMsgs,
                               bool bRegex = false) {
    std::wstring fullPath = hlsl_test::GetPathToHlslDataFile(name);
    CComPtr<IDxcLibrary> pLibrary;
    CComPtr<IDxcBlobEncoding> pSource;
    VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
    VERIFY_SUCCEEDED(
        pLibrary->CreateBlobFromFile(fullPath.c_str(), nullptr, &pSource));
    RewriteAssemblyCheckMsg(pSource, pShaderModel, pArguments, argCount,
                            pDefines, defCount, pLookFors, pReplacements,
                            pErrorMsgs, bRegex);
  }

  void RewriteAssemblyCheckMsg(LPCWSTR name, LPCSTR pShaderModel,
                               llvm::ArrayRef<LPCSTR> pLookFors,
                               llvm::ArrayRef<LPCSTR> pReplacements,
                               llvm::ArrayRef<LPCSTR> pErrorMsgs,
                               bool bRegex = false) {
    RewriteAssemblyCheckMsg(name, pShaderModel, nullptr, 0, nullptr, 0,
                            pLookFors, pReplacements, pErrorMsgs, bRegex);
  }

  void PerformReplacementOnDisassembly(std::string disassembly,
                                       llvm::ArrayRef<LPCSTR> pLookFors,
                                       llvm::ArrayRef<LPCSTR> pReplacements,
                                       IDxcBlob **pBlob, bool bRegex = false) {
    for (unsigned i = 0; i < pLookFors.size(); ++i) {
      LPCSTR pLookFor = pLookFors[i];
      bool bOptional = false;
      if (pLookFor[0] == '?') {
        bOptional = true;
        pLookFor++;
      }
      LPCSTR pReplacement = pReplacements[i];
      if (pLookFor && *pLookFor) {
        if (bRegex) {
          llvm::Regex RE(pLookFor);
          std::string reErrors;
          if (!RE.isValid(reErrors)) {
            WEX::Logging::Log::Comment(WEX::Common::String().Format(
                L"Regex errors:\r\n%.*S\r\nWhile compiling expression '%S'",
                (unsigned)reErrors.size(), reErrors.data(), pLookFor));
          }
          VERIFY_IS_TRUE(RE.isValid(reErrors));
          std::string replaced = RE.sub(pReplacement, disassembly, &reErrors);
          if (!bOptional) {
            if (!reErrors.empty()) {
              WEX::Logging::Log::Comment(WEX::Common::String().Format(
                  L"Regex errors:\r\n%.*S\r\nWhile searching for '%S' in "
                  L"text:\r\n%.*S",
                  (unsigned)reErrors.size(), reErrors.data(), pLookFor,
                  (unsigned)disassembly.size(), disassembly.data()));
            }
            VERIFY_ARE_NOT_EQUAL(disassembly, replaced);
            VERIFY_IS_TRUE(reErrors.empty());
          }
          disassembly = std::move(replaced);
        } else {
          bool found = false;
          size_t pos = 0;
          size_t lookForLen = strlen(pLookFor);
          size_t replaceLen = strlen(pReplacement);
          for (;;) {
            pos = disassembly.find(pLookFor, pos);
            if (pos == std::string::npos)
              break;
            found = true; // at least once
            disassembly.replace(pos, lookForLen, pReplacement);
            pos += replaceLen;
          }
          if (!bOptional) {
            if (!found) {
              WEX::Logging::Log::Comment(WEX::Common::String().Format(
                  L"String not found: '%S' in text:\r\n%.*S", pLookFor,
                  (unsigned)disassembly.size(), disassembly.data()));
            }
            VERIFY_IS_TRUE(found);
          }
        }
      }
    }
    Utf8ToBlob(m_dllSupport, disassembly.c_str(), pBlob);
  }

  bool RewriteAssemblyToText(IDxcBlobEncoding *pSource, LPCSTR pShaderModel,
                             LPCWSTR *pArguments, UINT32 argCount,
                             const DxcDefine *pDefines, UINT32 defineCount,
                             llvm::ArrayRef<LPCSTR> pLookFors,
                             llvm::ArrayRef<LPCSTR> pReplacements,
                             IDxcBlob **pBlob, bool bRegex = false) {
    CComPtr<IDxcBlob> pProgram;
    std::string disassembly;
    if (!CompileSource(pSource, pShaderModel, pArguments, argCount, pDefines,
                       defineCount, &pProgram))
      return false;
    DisassembleProgram(pProgram, &disassembly);
    PerformReplacementOnDisassembly(disassembly, pLookFors, pReplacements,
                                    pBlob, bRegex);
    return true;
  }

  // compile one or two sources, validate module from 1 with container parts
  // from 2, check messages
  bool ReplaceContainerPartsCheckMsgs(LPCSTR pSource1, LPCSTR pSource2,
                                      LPCSTR pShaderModel,
                                      llvm::ArrayRef<DxilFourCC> PartsToReplace,
                                      llvm::ArrayRef<LPCSTR> pErrorMsgs) {
    CComPtr<IDxcBlob> pProgram1, pProgram2;
    if (!CompileSource(pSource1, pShaderModel, &pProgram1))
      return false;
    VERIFY_IS_NOT_NULL(pProgram1);
    if (pSource2) {
      if (!CompileSource(pSource2, pShaderModel, &pProgram2))
        return false;
      VERIFY_IS_NOT_NULL(pProgram2);
    } else {
      pProgram2 = pProgram1;
    }

    // construct container with module from pProgram1 with other parts from
    // pProgram2:
    const DxilContainerHeader *pHeader1 = IsDxilContainerLike(
        pProgram1->GetBufferPointer(), pProgram1->GetBufferSize());
    VERIFY_IS_NOT_NULL(pHeader1);
    const DxilContainerHeader *pHeader2 = IsDxilContainerLike(
        pProgram2->GetBufferPointer(), pProgram2->GetBufferSize());
    VERIFY_IS_NOT_NULL(pHeader2);

    unique_ptr<DxilContainerWriter> pContainerWriter(NewDxilContainerWriter(
        DXIL::CompareVersions(m_ver.m_ValMajor, m_ver.m_ValMinor, 1, 7) < 0));

    // Add desired parts from first container
    for (auto pPart : pHeader1) {
      for (auto dfcc : PartsToReplace) {
        if (dfcc == pPart->PartFourCC) {
          pPart = nullptr;
          break;
        }
      }
      if (!pPart)
        continue;
      pContainerWriter->AddPart(pPart->PartFourCC, pPart->PartSize,
                                [=](AbstractMemoryStream *pStream) {
                                  ULONG cbWritten = 0;
                                  pStream->Write(GetDxilPartData(pPart),
                                                 pPart->PartSize, &cbWritten);
                                });
    }

    // Add desired parts from second container
    for (auto pPart : pHeader2) {
      for (auto dfcc : PartsToReplace) {
        if (dfcc == pPart->PartFourCC) {
          pContainerWriter->AddPart(pPart->PartFourCC, pPart->PartSize,
                                    [=](AbstractMemoryStream *pStream) {
                                      ULONG cbWritten = 0;
                                      pStream->Write(GetDxilPartData(pPart),
                                                     pPart->PartSize,
                                                     &cbWritten);
                                    });
          break;
        }
      }
    }

    // Write the container
    CComPtr<IMalloc> pMalloc;
    VERIFY_SUCCEEDED(DxcCoGetMalloc(1, &pMalloc));
    CComPtr<AbstractMemoryStream> pOutputStream;
    VERIFY_SUCCEEDED(CreateMemoryStream(pMalloc, &pOutputStream));
    pOutputStream->Reserve(pContainerWriter->size());
    pContainerWriter->write(pOutputStream);

    CheckValidationMsgs((const char *)pOutputStream->GetPtr(),
                        pOutputStream->GetPtrSize(), pErrorMsgs,
                        /*bRegex*/ false);
    return true;
  }
};

bool ValidationTest::InitSupport() {
  if (!m_dllSupport.IsEnabled()) {
    VERIFY_SUCCEEDED(m_dllSupport.Initialize());
    m_ver.Initialize(m_dllSupport);
  }
  return true;
}

TEST_F(ValidationTest, WhenCorrectThenOK) {
  CComPtr<IDxcBlob> pProgram;
  CompileSource("float4 main() : SV_Target { return 1; }", "ps_6_0", &pProgram);
  CheckValidationMsgs(pProgram, nullptr);
}

// Lots of these going on below for simplicity in setting up payloads.
//
// warning C4838: conversion from 'int' to 'const char' requires a narrowing
// conversion warning C4309: 'initializing': truncation of constant value
#pragma warning(disable : 4838)
#pragma warning(disable : 4309)

TEST_F(ValidationTest, WhenMisalignedThenFail) {
  // Bitcode size must 4-byte aligned
  const char blob[] = {
      'B',
      'C',
  };
  CheckValidationMsgs(blob, _countof(blob), "Invalid bitcode size");
}

TEST_F(ValidationTest, WhenEmptyFileThenFail) {
  // No blocks after signature.
  const char blob[] = {'B', 'C', (char)0xc0, (char)0xde};
  CheckValidationMsgs(blob, _countof(blob), "Malformed IR file");
}

TEST_F(ValidationTest, WhenIncorrectMagicThenFail) {
  // Signature isn't 'B', 'C', 0xC0 0xDE
  const char blob[] = {'B', 'C', (char)0xc0, (char)0xdd};
  CheckValidationMsgs(blob, _countof(blob), "Invalid bitcode signature");
}

TEST_F(ValidationTest, WhenIncorrectTargetTripleThenFail) {
  const char blob[] = {'B', 'C', (char)0xc0, (char)0xde};
  CheckValidationMsgs(blob, _countof(blob), "Malformed IR file");
}

TEST_F(ValidationTest, WhenMultipleModulesThenFail) {
  const char blob[] = {
      'B', 'C', (char)0xc0, (char)0xde, 0x21, 0x0c, 0x00,
      0x00, // Enter sub-block, BlockID = 8, Code Size=3, padding x2
      0x00, 0x00, 0x00, 0x00, // NumWords = 0
      0x08, 0x00, 0x00, 0x00, // End-of-block, padding
      // At this point, this is valid bitcode (but missing required DXIL
      // metadata) Trigger the case we're looking for now
      0x21, 0x0c, 0x00,
      0x00, // Enter sub-block, BlockID = 8, Code Size=3, padding x2
  };
  CheckValidationMsgs(blob, _countof(blob), "Unused bits in buffer");
}

TEST_F(ValidationTest, WhenUnexpectedEOFThenFail) {
  // Importantly, this is testing the usage of report_fatal_error during
  // deserialization.
  const char blob[] = {
      'B',  'C',  (char)0xc0, (char)0xde, 0x21,
      0x0c, 0x00, 0x00, // Enter sub-block, BlockID = 8, Code Size=3, padding x2
      0x00, 0x00, 0x00,       0x00, // NumWords = 0
  };
  CheckValidationMsgs(blob, _countof(blob), "Invalid record");
}

TEST_F(ValidationTest, WhenUnknownBlocksThenFail) {
  const char blob[] = {
      'B',  'C',  (char)0xc0, (char)0xde, // Signature
      0x31, 0x00, 0x00,       0x00        // Enter sub-block, BlockID != 8
  };
  CheckValidationMsgs(blob, _countof(blob), "Unrecognized block found");
}

TEST_F(ValidationTest, WhenZeroInputPatchCountWithInputThenFail) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\SimpleHs1.hlsl", "hs_6_0",
                          "void ()* "
                          "@\"\\01?HSPerPatchFunc@@YA?AUHSPerPatchData@@V?$"
                          "InputPatch@UPSSceneIn@@$02@@@Z\", i32 3, i32 3",
                          "void ()* "
                          "@\"\\01?HSPerPatchFunc@@YA?AUHSPerPatchData@@V?$"
                          "InputPatch@UPSSceneIn@@$02@@@Z\", i32 0, i32 3",
                          "When HS input control point count is 0, no input "
                          "signature should exist");
}

TEST_F(ValidationTest, WhenInstrDisallowedThenFail) {
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\abs2.hlsl", "ps_6_0",
      {
          "target triple = \"dxil-ms-dx\"",
          "ret void",
          "dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 3, i32 undef)",
          "!\"ps\", i32 6, i32 0",
      },
      {
          "target triple = \"dxil-ms-dx\"\n%dx.types.wave_t = type { i8* }",
          "unreachable",
          "dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 3, i32 "
          "undef)\n%wave_local = alloca %dx.types.wave_t",
          "!\"vs\", i32 6, i32 0",
      },
      {
          "Semantic 'SV_Target' is invalid as vs Output",
          "Declaration '%dx.types.wave_t = type { i8* }' uses a reserved "
          "prefix",
          "Instructions must be of an allowed type",
      });
}

TEST_F(ValidationTest, WhenDepthNotFloatThenFail) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\IntegerDepth2.hlsl", "ps_6_0",
                          {
                              "!\"SV_Depth\", i8 9",
                          },
                          {
                              "!\"SV_Depth\", i8 4",
                          },
                          {
                              "SV_Depth must be float",
                          });
}

TEST_F(ValidationTest, BarrierFail) {
  if (m_ver.SkipIRSensitiveTest())
    return;
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\barrier.hlsl", "cs_6_0",
      {
          "dx.op.barrier(i32 80, i32 8)",
          "dx.op.barrier(i32 80, i32 9)",
          "dx.op.barrier(i32 80, i32 11)",
          "%\"hostlayout.class.RWStructuredBuffer<matrix<float, 2, 2> >\" = "
          "type { [2 x <2 x float>] }\n",
          "call i32 @dx.op.flattenedThreadIdInGroup.i32(i32 96)",
      },
      {
          "dx.op.barrier(i32 80, i32 15)",
          "dx.op.barrier(i32 80, i32 0)",
          "dx.op.barrier(i32 80, i32 %rem)",
          "%\"hostlayout.class.RWStructuredBuffer<matrix<float, 2, 2> >\" = "
          "type { [2 x <2 x float>] }\n"
          "@dx.typevar.8 = external addrspace(1) constant "
          "%\"hostlayout.class.RWStructuredBuffer<matrix<float, 2, 2> >\"\n"
          "@\"internalGV\" = internal global [64 x <4 x float>] undef\n",
          "call i32 @dx.op.flattenedThreadIdInGroup.i32(i32 96)\n"
          "%load = load %\"hostlayout.class.RWStructuredBuffer<matrix<float, "
          "2, 2> >\", %\"hostlayout.class.RWStructuredBuffer<matrix<float, 2, "
          "2> >\" addrspace(1)* @dx.typevar.8",
      },
      {"Internal declaration 'internalGV' is unused",
       "External declaration 'dx.typevar.8' is unused",
       "Vector type '<4 x float>' is not allowed",
       "Mode of Barrier must be an immediate constant",
       "sync must include some form of memory barrier - _u (UAV) and/or _g "
       "(Thread Group Shared Memory)",
       "sync can't specify both _ugroup and _uglobal. If both are needed, just "
       "specify _uglobal"});
}
TEST_F(ValidationTest, CBufferLegacyOutOfBoundFail) {
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\cbuffer1.50.hlsl", "ps_6_0",
      "cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %Foo2_cbuffer, i32 0)",
      "cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %Foo2_cbuffer, i32 6)",
      "Cbuffer access out of bound");
}

TEST_F(ValidationTest, CsThreadSizeFail) {
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\share_mem1.hlsl", "cs_6_0",
      {"!{i32 8, i32 8, i32 1", "[256 x float]"},
      {"!{i32 1025, i32 1025, i32 1025", "[64000000 x float]"},
      {
          "Declared Thread Group X size 1025 outside valid range",
          "Declared Thread Group Y size 1025 outside valid range",
          "Declared Thread Group Z size 1025 outside valid range",
          "Declared Thread Group Count 1076890625 (X*Y*Z) is beyond the valid "
          "maximum",
          "Total Thread Group Shared Memory storage is 256000000, exceeded "
          "32768",
      });
}
TEST_F(ValidationTest, DeadLoopFail) {
  if (m_ver.SkipIRSensitiveTest())
    return;
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\loop1.hlsl", "ps_6_0",
      {"br i1 %exitcond, label %for.end.loopexit, label %for.body, !llvm.loop "
       "!([0-9]+)",
       "?%add(\\.lcssa)? = phi float \\[ %add, %for.body \\]",
       "!dx.entryPoints = !\\{!([0-9]+)\\}",
       "\\[ %add(\\.lcssa)?, %for.end.loopexit \\]"},
      {"br label %for.body", "",
       "!dx.entryPoints = !\\{!\\1\\}\n!dx.unused = !\\{!\\1\\}",
       "[ 0.000000e+00, %for.end.loopexit ]"},
      {
          "Loop must have break",
          "Named metadata 'dx.unused' is unknown",
      },
      /*bRegex*/ true);
}
TEST_F(ValidationTest, EvalFail) {
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\eval.hlsl", "ps_6_0",
      "!\"A\", i8 9, i8 0, !([0-9]+), i8 2, i32 1, i8 4",
      "!\"A\", i8 9, i8 0, !\\1, i8 0, i32 1, i8 4",
      "Interpolation mode on A used with eval_\\* instruction must be ",
      /*bRegex*/ true);
}
TEST_F(ValidationTest, GetDimCalcLODFail) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\GetDimCalcLOD.hlsl", "ps_6_0",
      {"extractvalue %dx.types.Dimensions %([0-9]+), 1",
       "float 1.000000e\\+00, i1 true"},
      {"extractvalue %dx.types.Dimensions %\\1, 2", "float undef, i1 true"},
      {"GetDimensions used undef dimension z on TextureCube",
       "coord uninitialized"},
      /*bRegex*/ true);
}
TEST_F(ValidationTest, HsAttributeFail) {
  if (m_ver.SkipDxilVersion(1, 8))
    return;
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\hsAttribute.hlsl", "hs_6_0",
      {"i32 3, i32 3, i32 2, i32 3, i32 3, float 6.400000e+01"},
      {"i32 36, i32 36, i32 0, i32 0, i32 0, float 6.500000e+01"},
      {"HS input control point count must be [0..32].  36 specified",
       "Invalid Tessellator Domain specified. Must be isoline, tri or quad",
       "Invalid Tessellator Partitioning specified",
       "Invalid Tessellator Output Primitive specified",
       "Hull Shader MaxTessFactor must be [1.000000..64.000000].  65.000000 "
       "specified",
       "output control point count must be [1..32].  36 specified"});
}
TEST_F(ValidationTest, InnerCoverageFail) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\InnerCoverage2.hlsl", "ps_6_0",
      {"dx.op.coverage.i32(i32 91)", "declare i32 @dx.op.coverage.i32(i32)"},
      {"dx.op.coverage.i32(i32 91)\n  %inner = call i32 "
       "@dx.op.innerCoverage.i32(i32 92)",
       "declare i32 @dx.op.coverage.i32(i32)\n"
       "declare i32 @dx.op.innerCoverage.i32(i32)"},
      "InnerCoverage and Coverage are mutually exclusive.");
}
TEST_F(ValidationTest, InterpChangeFail) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\interpChange.hlsl", "ps_6_0",
      {"i32 1, i8 0, (.*)}", "?!dx.viewIdState ="},
      {"i32 0, i8 2, \\1}", "!1012 ="},
      "interpolation mode that differs from another element packed",
      /*bRegex*/ true);
}
TEST_F(ValidationTest, InterpOnIntFail) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\interpOnInt2.hlsl", "ps_6_0",
                          "!\"A\", i8 5, i8 0, !([0-9]+), i8 1",
                          "!\"A\", i8 5, i8 0, !\\1, i8 2",
                          "signature element A specifies invalid interpolation "
                          "mode for integer component type",
                          /*bRegex*/ true);
}
TEST_F(ValidationTest, InvalidSigCompTyFail) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\abs2.hlsl", "ps_6_0",
                          "!\"A\", i8 4", "!\"A\", i8 0",
                          "A specifies unrecognized or invalid component type");
}
TEST_F(ValidationTest, MultiStream2Fail) {
  if (m_ver.SkipDxilVersion(1, 7))
    return;
  // dxilver 1.7 because PSV0 data was incorrectly filled in before this point,
  // making this test fail if running against prior validator versions.
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\multiStreamGS.hlsl", "gs_6_0",
      "i32 1, i32 12, i32 7, i32 1, i32 1",
      "i32 1, i32 12, i32 7, i32 2, i32 1",
      "Multiple GS output streams are used but 'XXX' is not pointlist");
}
TEST_F(ValidationTest, PhiTGSMFail) {
  if (m_ver.SkipIRSensitiveTest())
    return;
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\phiTGSM.hlsl", "cs_6_0", "ret void",
      "%arrayPhi = phi i32 addrspace(3)* [ %arrayidx, %if.then ], [ "
      "%arrayidx2, %if.else ]\n"
      "%phiAtom = atomicrmw add i32 addrspace(3)* %arrayPhi, i32 1 seq_cst\n"
      "ret void",
      "TGSM pointers must originate from an unambiguous TGSM global variable");
}

TEST_F(ValidationTest, QuadOpInVS) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;
  RewriteAssemblyCheckMsg(
      "struct PerThreadData { int "
      "input; int output; }; RWStructuredBuffer<PerThreadData> g_sb; "
      "void main(uint vid : SV_VertexID)"
      "{ g_sb[vid].output = WaveActiveBitAnd((uint)g_sb[vid].input); }",
      "vs_6_0",
      {"@dx.op.waveActiveBit.i32(i32 120",
       "declare i32 @dx.op.waveActiveBit.i32(i32, i32, i8)"},
      {"@dx.op.quadOp.i32(i32 123",
       "declare i32 @dx.op.quadOp.i32(i32, i32, i8)"},
      "QuadOp not valid in shader model vs_6_0");
}

TEST_F(ValidationTest, ReducibleFail) {
  if (m_ver.SkipIRSensitiveTest())
    return;
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\reducible.hlsl", "ps_6_0",
                          {"%conv\n"
                           "  br label %if.end",
                           "to float\n"
                           "  br label %if.end"},
                          {"%conv\n"
                           "  br i1 %cmp, label %if.else, label %if.end",
                           "to float\n"
                           "  br i1 %cmp, label %if.then, label %if.end"},
                          "Execution flow must be reducible");
}
TEST_F(ValidationTest, SampleBiasFail) {
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\sampleBias.hlsl", "ps_6_0", {"float -1.600000e+01"},
      {"float 1.800000e+01"},
      "bias amount for sample_b must be in the range [-16.000000,15.990000]");
}
TEST_F(ValidationTest, SamplerKindFail) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\SamplerKind.hlsl", "ps_6_0",
                          {
                              "uav1_UAV_2d = call %dx.types.Handle "
                              "@dx.op.createHandle(i32 57, i8 1",
                              "g_txDiffuse_texture_2d = call %dx.types.Handle "
                              "@dx.op.createHandle(i32 57, i8 0",
                              "\"g_samLinear\", i32 0, i32 0, i32 1, i32 0",
                              "\"g_samLinearC\", i32 0, i32 1, i32 1, i32 1",
                          },
                          {
                              "uav1_UAV_2d = call %dx.types.Handle "
                              "@dx.op.createHandle(i32 57, i8 0",
                              "g_txDiffuse_texture_2d = call %dx.types.Handle "
                              "@dx.op.createHandle(i32 57, i8 1",
                              "\"g_samLinear\", i32 0, i32 0, i32 1, i32 3",
                              "\"g_samLinearC\", i32 0, i32 1, i32 1, i32 3",
                          },
                          {"Invalid sampler mode",
                           "require sampler declared in comparison mode",
                           "requires sampler declared in default mode",
                           // 1.4: "should", 1.5: "should be "
                           "on srv resource"});
}
TEST_F(ValidationTest, SemaOverlapFail) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\semaOverlap1.hlsl", "ps_6_0",
                          {
                              "!\\{i32 0, !\"A\", i8 9, i8 0, !([0-9]+), i8 2, "
                              "i32 1, i8 4, i32 0, i8 0, (.*)"
                              "!\\{i32 1, !\"A\", i8 9, i8 0, !([0-9]+)",
                          },
                          {
                              "!\\{i32 0, !\"A\", i8 9, i8 0, !\\1, i8 2, i32 "
                              "1, i8 4, i32 0, i8 0, \\2"
                              "!\\{i32 1, !\"A\", i8 9, i8 0, !\\1",
                          },
                          {"Semantic 'A' overlap at 0"},
                          /*bRegex*/ true);
}
TEST_F(ValidationTest, SigOutOfRangeFail) {
  return; // Skip for now since this fails AssembleToContainer in PSV creation
          // due to out of range start row
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\semaOverlap1.hlsl", "ps_6_0",
      {
          "i32 1, i8 0, null}",
      },
      {
          "i32 8000, i8 0, null}",
      },
      {"signature element A at location (8000,0) size (1,4) is out of range"});
}
TEST_F(ValidationTest, SigOverlapFail) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\semaOverlap1.hlsl", "ps_6_0",
      {"i8 2, i32 1, i8 4, i32 1, i8 0,", "?!dx.viewIdState ="},
      {"i8 2, i32 1, i8 4, i32 0, i8 0,", "!1012 ="},
      {"signature element A at location (0,0) size (1,4) overlaps another "
       "signature element"});
}
TEST_F(ValidationTest, SimpleHs1Fail) {
  if (m_ver.SkipDxilVersion(1, 8))
    return;
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\SimpleHs1.hlsl", "hs_6_0",
      {
          "i32 3, i32 3, i32 2, i32 3, i32 3, float 6.400000e+01}",
          "\"SV_TessFactor\", i8 9, i8 25",
          "\"SV_InsideTessFactor\", i8 9, i8 26",
      },
      {
          "i32 3, i32 3000, i32 2, i32 3, i32 3, float 6.400000e+01}",
          "\"TessFactor\", i8 9, i8 0",
          "\"InsideTessFactor\", i8 9, i8 0",
      },
      {
          "output control point count must be [1..32].  3000 specified",
          "Required TessFactor for domain not found declared anywhere in Patch "
          "Constant data",
          // TODO: enable this after support pass thru hull shader.
          //"For pass thru hull shader, input control point count must match
          // output control point count", "Total number of scalars across all HS
          // output control points must not exceed",
      });
}
TEST_F(ValidationTest, SimpleHs3Fail) {
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\SimpleHs3.hlsl", "hs_6_0",
      {
          "i32 3, i32 3, i32 2, i32 3, i32 3, float 6.400000e+01}",
      },
      {
          "i32 3, i32 3, i32 2, i32 3, i32 2, float 6.400000e+01}",
      },
      {"Hull Shader declared with Tri Domain must specify output primitive "
       "point, triangle_cw or triangle_ccw. Line output is not compatible with "
       "the Tri domain"});
}
TEST_F(ValidationTest, SimpleHs4Fail) {
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\SimpleHs4.hlsl", "hs_6_0",
      {
          "i32 2, i32 2, i32 1, i32 3, i32 2, float 6.400000e+01}",
      },
      {
          "i32 2, i32 2, i32 1, i32 3, i32 3, float 6.400000e+01}",
      },
      {"Hull Shader declared with IsoLine Domain must specify output primitive "
       "point or line. Triangle_cw or triangle_ccw output are not compatible "
       "with the IsoLine Domain"});
}
TEST_F(ValidationTest, SimpleDs1Fail) {
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\SimpleDs1.hlsl", "ds_6_0", {"!{i32 2, i32 3}"},
      {"!{i32 4, i32 36}"},
      {"DS input control point count must be [0..32].  36 specified",
       "Invalid Tessellator Domain specified. Must be isoline, tri or quad",
       "DomainLocation component index out of bounds for the domain"});
}
TEST_F(ValidationTest, SimpleGs1Fail) {
  return; // Skip for now since this fails AssembleToContainer in PSV creation
          // due to out of range stream index
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\SimpleGS1.hlsl", "gs_6_0",
      {"!{i32 1, i32 3, i32 1, i32 5, i32 1}",
       "i8 4, i32 1, i8 4, i32 2, i8 0, null}"},
      {"!{i32 5, i32 1025, i32 1, i32 0, i32 33}",
       "i8 4, i32 1, i8 4, i32 2, i8 0, !100}\n"
       "!100 = !{i32 0, i32 5}"},
      {"GS output vertex count must be [0..1024].  1025 specified",
       "GS instance count must be [1..32].  33 specified",
       "GS output primitive topology unrecognized",
       "GS input primitive unrecognized",
       "Stream index (5) must between 0 and 3"});
}
TEST_F(ValidationTest, UavBarrierFail) {
  if (m_ver.SkipIRSensitiveTest())
    return;
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\uavBarrier.hlsl", "ps_6_0",
      {
          "dx.op.barrier(i32 80, i32 2)",
          "textureLoad.f32(i32 66, %dx.types.Handle %uav1_UAV_2d, i32 undef",
          "i32 undef, i32 undef, i32 undef, i32 undef)",
          "float %add9.i3, i8 15)",
      },
      {
          "dx.op.barrier(i32 80, i32 9)",
          "textureLoad.f32(i32 66, %dx.types.Handle %uav1_UAV_2d, i32 1",
          "i32 1, i32 2, i32 undef, i32 undef)",
          "float undef, i8 7)",
      },
      {"uav load don't support offset",
       "uav load don't support mipLevel/sampleIndex",
       "store on typed uav must write to all four components of the UAV",
       "sync in a non-", // 1.4: "Compute" 1.5: "Compute/Amplification/Mesh"
       " Shader must only sync UAV (sync_uglobal)"});
}
TEST_F(ValidationTest, UndefValueFail) {
  TestCheck(L"..\\CodeGenHLSL\\UndefValue.hlsl");
}
// verify that containers that are not valid DXIL do not
// get assigned a hash.
TEST_F(ValidationTest, ValidationFailNoHash) {
  if (m_ver.SkipDxilVersion(1, 8))
    return;
  CComPtr<IDxcBlob> pProgram;

  // We need any shader that will pass compilation but fail validation.
  // This shader reads from uninitialized 'float a', which works for now.
  LPCSTR pSource = R"(
    float main(snorm float b : B) : SV_DEPTH
    {
        float a;
        return b + a;
    }
)";

  CComPtr<IDxcBlobEncoding> pSourceBlob;
  Utf8ToBlob(m_dllSupport, pSource, &pSourceBlob);
  std::vector<LPCWSTR> pArguments = {L"-Vd"};
  LPCSTR pShaderModel = "ps_6_0";
  bool result = CompileSource(pSourceBlob, pShaderModel, pArguments.data(), 1,
                              nullptr, 0, &pProgram);

  VERIFY_IS_TRUE(result);

  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pResult;
  unsigned Flags = 0;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));

  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pResult));
  HRESULT status;
  VERIFY_IS_NOT_NULL(pResult);
  CComPtr<IDxcBlob> pValidationOutput;
  pResult->GetStatus(&status);

  // expect validation to fail
  VERIFY_FAILED(status);
  pResult->GetResult(&pValidationOutput);
  // Make sure the validation output is not null even when validation fails
  VERIFY_SUCCEEDED(pValidationOutput != nullptr);

  hlsl::DxilContainerHeader *pHeader = IsDxilContainerLike(
      pProgram->GetBufferPointer(), pProgram->GetBufferSize());
  VERIFY_IS_NOT_NULL(pHeader);

  BYTE ZeroHash[DxilContainerHashSize] = {0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0};

  // Should be equal, this proves the hash isn't written when validation fails
  VERIFY_ARE_EQUAL(memcmp(ZeroHash, pHeader->Hash.Digest, sizeof(ZeroHash)), 0);
}
TEST_F(ValidationTest, UpdateCounterFail) {
  if (m_ver.SkipIRSensitiveTest())
    return;
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\updateCounter2.hlsl", "ps_6_0",
      {"%2 = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle "
       "%buf2_UAV_structbuf, i8 1)",
       "%3 = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle "
       "%buf2_UAV_structbuf, i8 1)"},
      {"%2 = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle "
       "%buf2_UAV_structbuf, i8 -1)",
       "%3 = call i32 @dx.op.bufferUpdateCounter(i32 70, %dx.types.Handle "
       "%buf2_UAV_structbuf, i8 1)\n"
       "%srvUpdate = call i32 @dx.op.bufferUpdateCounter(i32 70, "
       "%dx.types.Handle %buf1_texture_buf, i8 undef)"},
      {"BufferUpdateCounter valid only on UAV",
       "BufferUpdateCounter valid only on structured buffers",
       "inc of BufferUpdateCounter must be an immediate constant",
       "RWStructuredBuffers may increment or decrement their counters, but not "
       "both"});
}

TEST_F(ValidationTest, LocalResCopy) {
  // error updated, so must exclude previous validator versions.
  if (m_ver.SkipDxilVersion(1, 6))
    return;
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\resCopy.hlsl", "cs_6_0", {"ret void"},
      {"%H = alloca %dx.types.ResRet.i32\n"
       "ret void"},
      {"Dxil struct types should only be used by ExtractValue"});
}

TEST_F(ValidationTest, WhenIncorrectModelThenFail) {
  TestCheck(L"..\\CodeGenHLSL\\val-failures.hlsl");
}

TEST_F(ValidationTest, WhenIncorrectPSThenFail) {
  TestCheck(L"..\\CodeGenHLSL\\val-failures-ps.hlsl");
}

TEST_F(ValidationTest, WhenSmUnknownThenFail) {
  RewriteAssemblyCheckMsg("float4 main() : SV_Target { return 1; }", "ps_6_0",
                          {"{!\"ps\", i32 6, i32 0}"},
                          {"{!\"ps\", i32 1, i32 2}"},
                          "Unknown shader model 'ps_1_2'");
}

TEST_F(ValidationTest, WhenSmLegacyThenFail) {
  RewriteAssemblyCheckMsg("float4 main() : SV_Target { return 1; }", "ps_6_0",
                          "{!\"ps\", i32 6, i32 0}", "{!\"ps\", i32 5, i32 1}",
                          "Unknown shader model 'ps_5_1'");
}

TEST_F(ValidationTest, WhenMetaFlagsUsageDeclThenOK) {
  RewriteAssemblyCheckMsg(
      "uint u; float4 main() : SV_Target { uint64_t n = u; n *= u; return "
      "(uint)(n >> 32); }",
      "ps_6_0", "1048576",
      "1048577", // inhibit optimization, which should work fine
      nullptr);
}

TEST_F(ValidationTest, GsVertexIDOutOfBound) {
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\SimpleGS1.hlsl", "gs_6_0",
      "dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 2, i32 0)",
      "dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 2, i32 1)",
      "expect VertexID between 0~1, got 1");
}

TEST_F(ValidationTest, StreamIDOutOfBound) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\SimpleGS1.hlsl", "gs_6_0",
                          "dx.op.emitStream(i32 97, i8 0)",
                          "dx.op.emitStream(i32 97, i8 1)",
                          "expect StreamID between 0 , got 1");
}

TEST_F(ValidationTest, SignatureDataWidth) {
  if (m_ver.SkipDxilVersion(1, 2))
    return;
  std::vector<LPCWSTR> pArguments = {L"-enable-16bit-types", L"-HV", L"2018"};
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\signature_packing_by_width.hlsl", "ps_6_2",
      pArguments.data(), 3, nullptr, 0,
      {"i8 8, i8 0, (![0-9]+), i8 2, i32 1, i8 2, i32 0, i8 0, null}"},
      {"i8 9, i8 0, \\1, i8 2, i32 1, i8 2, i32 0, i8 0, null}"},
      "signature element F at location \\(0, 2\\) size \\(1, 2\\) has data "
      "width that differs from another element packed into the same row.",
      true);
}

TEST_F(ValidationTest, SignatureStreamIDForNonGS) {
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\abs1.hlsl", "ps_6_0",
      {", i8 0, i32 1, i8 4, i32 0, i8 0, [^,]+}", "?!dx.viewIdState ="},
      {", i8 0, i32 1, i8 4, i32 0, i8 0, !1019}\n!1019 = !{i32 0, i32 1}",
       "!1012 ="},
      "Stream index \\(1\\) must between 0 and 0", true);
}

TEST_F(ValidationTest, TypedUAVStoreFullMask0) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\uav_typed_store.hlsl", "ps_6_0",
                          "float 2.000000e+00, i8 15)",
                          "float 2.000000e+00, i8 undef)",
                          "Mask of TextureStore must be an immediate constant");
}

TEST_F(ValidationTest, TypedUAVStoreFullMask1) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\uav_typed_store.hlsl", "ps_6_0",
                          "float 3.000000e+00, i8 15)",
                          "float 3.000000e+00, i8 undef)",
                          "Mask of BufferStore must be an immediate constant");
}

TEST_F(ValidationTest, UAVStoreMaskMatch) {
  // error updated, so must exclude previous validator versions.
  if (m_ver.SkipDxilVersion(1, 6))
    return;
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\uav_store.hlsl", "ps_6_0",
                          "i32 2, i8 15)", "i32 2, i8 7)",
                          "uav store write mask must match store value mask, "
                          "write mask is 7 and store value mask is 15.");
}

TEST_F(ValidationTest, UAVStoreMaskGap) {
  if (m_ver.SkipDxilVersion(1, 7))
    return;
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\uav_store.hlsl", "ps_6_0",
                          "i32 2, i32 2, i32 2, i32 2, i8 15)",
                          "i32 undef, i32 2, i32 undef, i32 2, i8 10)",
                          "UAV write mask must be contiguous, starting at x: "
                          ".x, .xy, .xyz, or .xyzw.");
}

TEST_F(ValidationTest, UAVStoreMaskGap2) {
  if (m_ver.SkipDxilVersion(1, 7))
    return;
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\uav_store.hlsl", "ps_6_0",
                          "i32 2, i32 2, i32 2, i32 2, i8 15)",
                          "i32 undef, i32 2, i32 2, i32 2, i8 14)",
                          "UAV write mask must be contiguous, starting at x: "
                          ".x, .xy, .xyz, or .xyzw.");
}

TEST_F(ValidationTest, UAVStoreMaskGap3) {
  if (m_ver.SkipDxilVersion(1, 7))
    return;
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\uav_store.hlsl", "ps_6_0",
                          "i32 2, i32 2, i32 2, i32 2, i8 15)",
                          "i32 undef, i32 undef, i32 undef, i32 2, i8 8)",
                          "UAV write mask must be contiguous, starting at x: "
                          ".x, .xy, .xyz, or .xyzw.");
}

TEST_F(ValidationTest, Recursive) {
  // Includes coverage for user-defined functions.
  TestCheck(L"..\\CodeGenHLSL\\recursive.ll");
}

TEST_F(ValidationTest, ResourceRangeOverlap0) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\resource_overlap.hlsl", "ps_6_0",
                          "!\"B\", i32 0, i32 1", "!\"B\", i32 0, i32 0",
                          "Resource B with base 0 size 1 overlap");
}

TEST_F(ValidationTest, ResourceRangeOverlap1) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\resource_overlap.hlsl", "ps_6_0",
                          "!\"s1\", i32 0, i32 1", "!\"s1\", i32 0, i32 0",
                          "Resource s1 with base 0 size 1 overlap");
}

TEST_F(ValidationTest, ResourceRangeOverlap2) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\resource_overlap.hlsl", "ps_6_0",
                          "!\"uav2\", i32 0, i32 0", "!\"uav2\", i32 0, i32 3",
                          "Resource uav2 with base 3 size 1 overlap");
}

TEST_F(ValidationTest, ResourceRangeOverlap3) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\resource_overlap.hlsl", "ps_6_0",
                          "!\"srv2\", i32 0, i32 1", "!\"srv2\", i32 0, i32 0",
                          "Resource srv2 with base 0 size 1 overlap");
}

TEST_F(ValidationTest, CBufferOverlap0) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\cbufferOffset.hlsl", "ps_6_0",
                          "i32 6, !\"g2\", i32 3, i32 0",
                          "i32 6, !\"g2\", i32 3, i32 8",
                          "CBuffer Foo1 has offset overlaps at 16");
}

TEST_F(ValidationTest, CBufferOverlap1) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\cbufferOffset.hlsl", "ps_6_0", " = !{i32 32, !",
      " = !{i32 16, !",
      "CBuffer Foo1 size insufficient for element at offset 16");
}

TEST_F(ValidationTest, ControlFlowHint) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\if1.hlsl", "ps_6_0",
                          "!\"dx.controlflow.hints\", i32 1",
                          "!\"dx.controlflow.hints\", i32 5",
                          "Attribute forcecase only works for switch");
}

TEST_F(ValidationTest, ControlFlowHint1) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\if1.hlsl", "ps_6_0",
                          "!\"dx.controlflow.hints\", i32 1",
                          "!\"dx.controlflow.hints\", i32 1, i32 2",
                          "Can't use branch and flatten attributes together");
}

TEST_F(ValidationTest, ControlFlowHint2) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\if1.hlsl", "ps_6_0",
                          "!\"dx.controlflow.hints\", i32 1",
                          "!\"dx.controlflow.hints\", i32 3",
                          "Invalid control flow hint");
}

TEST_F(ValidationTest, SemanticLength1) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\binary1.hlsl", "ps_6_0",
                          "!\"C\"", "!\"\"",
                          "Semantic length must be at least 1 and at most 64");
}

TEST_F(ValidationTest, SemanticLength64) {
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\binary1.hlsl", "ps_6_0", "!\"C\"",
      "!\"CSESESESESESESESESESESESESESESESESESESESESESESESESESESESESESESESE\"",
      "Semantic length must be at least 1 and at most 64");
}

TEST_F(ValidationTest, PullModelPosition) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\eval.hlsl", "ps_6_0",
                          "!\"A\", i8 9, i8 0", "!\"SV_Position\", i8 9, i8 3",
                          "does not support pull-model evaluation of position");
}

TEST_F(ValidationTest, StructBufGlobalCoherentAndCounter) {
  // error updated, so must exclude previous validator versions.
  if (m_ver.SkipDxilVersion(1, 6))
    return;
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\struct_buf1.hlsl", "ps_6_0",
      "!\"buf2\", i32 0, i32 0, i32 1, i32 12, i1 false, i1 false",
      "!\"buf2\", i32 0, i32 0, i32 1, i32 12, i1 true, i1 true",
      "globallycoherent cannot be used on buffer with counter 'buf2'");
}

TEST_F(ValidationTest, StructBufStrideAlign) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\struct_buf1.hlsl", "ps_6_0",
                          "= !{i32 1, i32 52}", "= !{i32 1, i32 50}",
                          "structured buffer element size must be a multiple "
                          "of 4 bytes (actual size 50 bytes)");
}

TEST_F(ValidationTest, StructBufStrideOutOfBound) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\struct_buf1.hlsl", "ps_6_0",
                          "= !{i32 1, i32 52}", "= !{i32 1, i32 2052}",
                          "structured buffer elements cannot be larger than "
                          "2048 bytes (actual size 2052 bytes)");
}

TEST_F(ValidationTest, StructBufLoadCoordinates) {
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\struct_buf1.hlsl", "ps_6_0",
      "bufferLoad.f32(i32 68, %dx.types.Handle "
      "%buf1_texture_structbuf, i32 1, i32 8)",
      "bufferLoad.f32(i32 68, %dx.types.Handle "
      "%buf1_texture_structbuf, i32 1, i32 undef)",
      "structured buffer requires defined index and offset coordinates");
}

TEST_F(ValidationTest, StructBufStoreCoordinates) {
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\struct_buf1.hlsl", "ps_6_0",
      "bufferStore.f32(i32 69, %dx.types.Handle "
      "%buf2_UAV_structbuf, i32 0, i32 0",
      "bufferStore.f32(i32 69, %dx.types.Handle "
      "%buf2_UAV_structbuf, i32 0, i32 undef",
      "structured buffer requires defined index and offset coordinates");
}

TEST_F(ValidationTest, TypedBufRetType) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\sample5.hlsl", "ps_6_0",
                          " = type { <4 x float>", " = type { <4 x double>",
                          "elements of typed buffers and textures must fit in "
                          "four 32-bit quantities");
}

TEST_F(ValidationTest, VsInputSemantic) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\clip_planes.hlsl", "vs_6_0",
                          "!\"POSITION\", i8 9, i8 0",
                          "!\"SV_Target\", i8 9, i8 16",
                          "Semantic 'SV_Target' is invalid as vs Input");
}

TEST_F(ValidationTest, VsOutputSemantic) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\clip_planes.hlsl", "vs_6_0",
                          "!\"NORMAL\", i8 9, i8 0",
                          "!\"SV_Target\", i8 9, i8 16",
                          "Semantic 'SV_Target' is invalid as vs Output");
}

TEST_F(ValidationTest, HsInputSemantic) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\SimpleHs1.hlsl", "hs_6_0",
                          "!\"TEXCOORD\", i8 9, i8 0",
                          "!\"VertexID\", i8 4, i8 1",
                          "Semantic 'VertexID' is invalid as hs Input");
}

TEST_F(ValidationTest, HsOutputSemantic) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\SimpleHs1.hlsl", "hs_6_0",
                          "!\"TEXCOORD\", i8 9, i8 0",
                          "!\"VertexID\", i8 4, i8 1",
                          "Semantic 'VertexID' is invalid as hs Output");
}

TEST_F(ValidationTest, PatchConstSemantic) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\SimpleHs1.hlsl", "hs_6_0",
                          "!\"SV_TessFactor\", i8 9, i8 25",
                          "!\"VertexID\", i8 4, i8 1",
                          "Semantic 'VertexID' is invalid as hs PatchConstant");
}

TEST_F(ValidationTest, DsInputSemantic) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\SimpleDs1.hlsl", "ds_6_0",
                          "!\"TEXCOORD\", i8 9, i8 0",
                          "!\"VertexID\", i8 4, i8 1",
                          "Semantic 'VertexID' is invalid as ds Input");
}

TEST_F(ValidationTest, DsOutputSemantic) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\SimpleDs1.hlsl", "ds_6_0",
                          "!\"TEXCOORD\", i8 9, i8 0",
                          "!\"VertexID\", i8 4, i8 1",
                          "Semantic 'VertexID' is invalid as ds Output");
}

TEST_F(ValidationTest, GsInputSemantic) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\SimpleGS1.hlsl", "gs_6_0",
                          "!\"POSSIZE\", i8 9, i8 0",
                          "!\"VertexID\", i8 4, i8 1",
                          "Semantic 'VertexID' is invalid as gs Input");
}

TEST_F(ValidationTest, GsOutputSemantic) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\SimpleGS1.hlsl", "gs_6_0",
                          "!\"TEXCOORD\", i8 9, i8 0",
                          "!\"VertexID\", i8 4, i8 1",
                          "Semantic 'VertexID' is invalid as gs Output");
}

TEST_F(ValidationTest, PsInputSemantic) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\abs2.hlsl", "ps_6_0",
                          "!\"A\", i8 4, i8 0", "!\"VertexID\", i8 4, i8 1",
                          "Semantic 'VertexID' is invalid as ps Input");
}

TEST_F(ValidationTest, PsOutputSemantic) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\abs2.hlsl", "ps_6_0",
                          "!\"SV_Target\", i8 9, i8 16",
                          "!\"VertexID\", i8 4, i8 1",
                          "Semantic 'VertexID' is invalid as ps Output");
}

TEST_F(ValidationTest, ArrayOfSVTarget) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\targetArray.hlsl", "ps_6_0",
                          "i32 2, !\"SV_Target\", i8 9, i8 16, !([0-9]+), i8 "
                          "0, i32 1, i8 4, i32 0, i8 0, (.*)}",
                          "i32 2, !\"SV_Target\", i8 9, i8 16, !101, i8 0, i32 "
                          "2, i8 4, i32 0, i8 0, \\2}\n!101 = !{i32 5, i32 6}",
                          "Pixel shader output registers are not indexable.",
                          /*bRegex*/ true);
}

TEST_F(ValidationTest, InfiniteLog) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\intrinsic_val_imm.hlsl", "ps_6_0",
                          "op.unary.f32\\(i32 23, float %[0-9+]\\)",
                          "op.unary.f32(i32 23, float 0x7FF0000000000000)",
                          "No indefinite logarithm",
                          /*bRegex*/ true);
}

TEST_F(ValidationTest, InfiniteAsin) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\intrinsic_val_imm.hlsl", "ps_6_0",
                          "op.unary.f32\\(i32 16, float %[0-9]+\\)",
                          "op.unary.f32(i32 16, float 0x7FF0000000000000)",
                          "No indefinite arcsine",
                          /*bRegex*/ true);
}

TEST_F(ValidationTest, InfiniteAcos) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\intrinsic_val_imm.hlsl", "ps_6_0",
                          "op.unary.f32\\(i32 15, float %[0-9]+\\)",
                          "op.unary.f32(i32 15, float 0x7FF0000000000000)",
                          "No indefinite arccosine",
                          /*bRegex*/ true);
}

TEST_F(ValidationTest, InfiniteDdxDdy) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\intrinsic_val_imm.hlsl", "ps_6_0",
                          "op.unary.f32\\(i32 85, float %[0-9]+\\)",
                          "op.unary.f32(i32 85, float 0x7FF0000000000000)",
                          "No indefinite derivative calculation",
                          /*bRegex*/ true);
}

TEST_F(ValidationTest, IDivByZero) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\intrinsic_val_imm.hlsl", "ps_6_0",
                          "sdiv i32 %([0-9]+), %[0-9]+", "sdiv i32 %\\1, 0",
                          "No signed integer division by zero",
                          /*bRegex*/ true);
}

TEST_F(ValidationTest, UDivByZero) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\intrinsic_val_imm.hlsl", "ps_6_0",
                          "udiv i32 %([0-9]+), %[0-9]+", "udiv i32 %\\1, 0",
                          "No unsigned integer division by zero",
                          /*bRegex*/ true);
}

TEST_F(ValidationTest, UnusedMetadata) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\loop2.hlsl", "ps_6_0",
                          ", !llvm.loop ", ", !llvm.loop2 ",
                          "All metadata must be used by dxil");
}

TEST_F(ValidationTest, MemoryOutOfBound) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\targetArray.hlsl", "ps_6_0",
      "getelementptr [4 x float], [4 x float]* %7, i32 0, i32 3",
      "getelementptr [4 x float], [4 x float]* %7, i32 0, i32 10",
      "Access to out-of-bounds memory is disallowed");
}

TEST_F(ValidationTest, LocalRes2) {
  TestCheck(L"..\\CodeGenHLSL\\local_resource2.hlsl");
}

TEST_F(ValidationTest, LocalRes3) {
  TestCheck(L"..\\CodeGenHLSL\\local_resource3.hlsl");
}

TEST_F(ValidationTest, LocalRes5) {
  TestCheck(L"..\\CodeGenHLSL\\local_resource5.hlsl");
}

TEST_F(ValidationTest, LocalRes5Dbg) {
  TestCheck(L"..\\CodeGenHLSL\\local_resource5_dbg.hlsl");
}

TEST_F(ValidationTest, LocalRes6) {
  TestCheck(L"..\\CodeGenHLSL\\local_resource6.hlsl");
}

TEST_F(ValidationTest, LocalRes6Dbg) {
  TestCheck(L"..\\CodeGenHLSL\\local_resource6_dbg.hlsl");
}

TEST_F(ValidationTest, AddrSpaceCast) {
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\staticGlobals.hlsl", "ps_6_0",
      "%([0-9]+) = getelementptr \\[4 x i32\\], \\[4 x i32\\]\\* %([0-9]+), "
      "i32 0, i32 0\n"
      "  store i32 %([0-9]+), i32\\* %\\1, align 4",
      "%\\1 = getelementptr [4 x i32], [4 x i32]* %\\2, i32 0, i32 0\n"
      "  %X = addrspacecast i32* %\\1 to i32 addrspace(1)*    \n"
      "  store i32 %\\3, i32 addrspace(1)* %X, align 4",
      "generic address space",
      /*bRegex*/ true);
}

TEST_F(ValidationTest, PtrBitCast) {
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\staticGlobals.hlsl", "ps_6_0",
      "%([0-9]+) = getelementptr \\[4 x i32\\], \\[4 x i32\\]\\* %([0-9]+), "
      "i32 0, i32 0\n"
      "  store i32 %([0-9]+), i32\\* %\\1, align 4",
      "%\\1 = getelementptr [4 x i32], [4 x i32]* %\\2, i32 0, i32 0\n"
      "  %X = bitcast i32* %\\1 to double*    \n"
      "  store i32 %\\3, i32* %\\1, align 4",
      "Pointer type bitcast must be have same size",
      /*bRegex*/ true);
}

TEST_F(ValidationTest, MinPrecisionBitCast) {
  if (m_ver.SkipDxilVersion(1, 2))
    return;
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\staticGlobals.hlsl", "ps_6_0",
      "%([0-9]+) = getelementptr \\[4 x i32\\], \\[4 x i32\\]\\* %([0-9]+), "
      "i32 0, i32 0\n"
      "  store i32 %([0-9]+), i32\\* %\\1, align 4",
      "%\\1 = getelementptr [4 x i32], [4 x i32]* %\\2, i32 0, i32 0\n"
      "  %X = bitcast i32* %\\1 to half* \n"
      "  store i32 %\\3, i32* %\\1, align 4",
      "Bitcast on minprecison types is not allowed",
      /*bRegex*/ true);
}

TEST_F(ValidationTest, StructBitCast) {
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\staticGlobals.hlsl", "ps_6_0",
      "%([0-9]+) = getelementptr \\[4 x i32\\], \\[4 x i32\\]\\* %([0-9]+), "
      "i32 0, i32 0\n"
      "  store i32 %([0-9]+), i32\\* %\\1, align 4",
      "%\\1 = getelementptr [4 x i32], [4 x i32]* %\\2, i32 0, i32 0\n"
      "  %X = bitcast i32* %\\1 to %dx.types.Handle*    \n"
      "  store i32 %\\3, i32* %\\1, align 4",
      "Bitcast on struct types is not allowed",
      /*bRegex*/ true);
}

TEST_F(ValidationTest, MultiDimArray) {
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\staticGlobals.hlsl", "ps_6_0",
                          "= alloca [4 x i32]",
                          "= alloca [4 x i32]\n"
                          "  %md = alloca [2 x [4 x float]]",
                          "Only one dimension allowed for array type");
}

TEST_F(ValidationTest, SimpleGs8) {
  TestCheck(L"..\\CodeGenHLSL\\SimpleGS8.hlsl");
}

TEST_F(ValidationTest, SimpleGs9) {
  TestCheck(L"..\\CodeGenHLSL\\SimpleGS9.hlsl");
}

TEST_F(ValidationTest, SimpleGs10) {
  TestCheck(L"..\\CodeGenHLSL\\SimpleGS10.hlsl");
}

TEST_F(ValidationTest, IllegalSampleOffset3) {
  TestCheck(L"..\\DXILValidation\\optForNoOpt3.hlsl");
}

TEST_F(ValidationTest, IllegalSampleOffset4) {
  TestCheck(L"..\\DXILValidation\\optForNoOpt4.hlsl");
}

TEST_F(ValidationTest, NoFunctionParam) {
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\abs2.hlsl", "ps_6_0",
      {"define void @main\\(\\)",
       "void \\(\\)\\* @main, !([0-9]+)\\}(.*)!\\1 = !\\{!([0-9]+)\\}",
       "void \\(\\)\\* @main"},
      {"define void @main(<4 x i32> %mainArg)",
       "void (<4 x i32>)* @main, !\\1}\\2!\\1 = !{!\\3, !\\3}",
       "void (<4 x i32>)* @main"},
      "with parameter is not permitted",
      /*bRegex*/ true);
}

TEST_F(ValidationTest, I8Type) {
  // error updated, so must exclude previous validator versions.
  if (m_ver.SkipDxilVersion(1, 6))
    return;
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\staticGlobals.hlsl", "ps_6_0",
      "%([0-9]+) = alloca \\[4 x i32\\]",
      "%\\1 = alloca [4 x i32]\n"
      "  %m8 = alloca i8",
      "I8 can only be used as immediate value for intrinsic",
      /*bRegex*/ true);
}

// TODO: enable this.
// TEST_F(ValidationTest, TGSMRaceCond) {
//  TestCheck(L"..\\CodeGenHLSL\\RaceCond.hlsl");
//}
//
// TEST_F(ValidationTest, TGSMRaceCond2) {
//    RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\structInBuffer.hlsl", "cs_6_0",
//        "ret void",
//        "%TID = call i32 @dx.op.flattenedThreadIdInGroup.i32(i32 96)\n"
//        "store i32 %TID, i32 addrspace(3)* @\"\\01?sharedData@@3UFoo@@A.3\",
//        align 4\n" "ret void", "Race condition writing to shared memory
//        detected, consider making this write conditional");
//}

TEST_F(ValidationTest, AddUint64Odd) {
  TestCheck(L"..\\CodeGenHLSL\\AddUint64Odd.hlsl");
}

TEST_F(ValidationTest, WhenWaveAffectsGradientThenFail) {
  TestCheck(L"..\\CodeGenHLSL\\val-wave-failures-ps.hlsl");
}

TEST_F(ValidationTest, WhenMetaFlagsUsageThenFail) {
  RewriteAssemblyCheckMsg("uint u; float4 main() : SV_Target { uint64_t n = u; "
                          "n *= u; return (uint)(n >> 32); }",
                          "ps_6_0", "1048576", "0", // remove the int64 flag
                          "Flags must match usage");
}

TEST_F(ValidationTest, StorePatchControlNotInPatchConstantFunction) {
  if (m_ver.SkipDxilVersion(1, 3))
    return;
  RewriteAssemblyCheckMsg("struct PSSceneIn \
    { \
    float4 pos  : SV_Position; \
    float2 tex  : TEXCOORD0; \
    float3 norm : NORMAL; \
    }; \
       \
    struct HSPerVertexData  \
    { \
    PSSceneIn v; \
    }; \
    struct HSPerPatchData  \
{  \
	float	edges[ 3 ]	: SV_TessFactor; \
	float	inside		: SV_InsideTessFactor; \
};  \
HSPerPatchData HSPerPatchFunc( const InputPatch< PSSceneIn, 3 > points, \
     OutputPatch<HSPerVertexData, 3> outpoints) \
{ \
    HSPerPatchData d; \
     \
    d.edges[ 0 ] = points[0].tex.x + outpoints[0].v.tex.x; \
    d.edges[ 1 ] = 1; \
    d.edges[ 2 ] = 1; \
    d.inside = 1; \
    \
    return d; \
}\
[domain(\"tri\")]\
[partitioning(\"fractional_odd\")]\
[outputtopology(\"triangle_cw\")]\
[patchconstantfunc(\"HSPerPatchFunc\")]\
[outputcontrolpoints(3)]\
HSPerVertexData main( const uint id : SV_OutputControlPointID,\
                               const InputPatch< PSSceneIn, 3 > points )\
{\
    HSPerVertexData v;\
    \
    v.v = points[ id ];\
    \
	return v;\
}\
    ",
                          "hs_6_0", "dx.op.storeOutput.f32(i32 5",
                          "dx.op.storePatchConstant.f32(i32 106",
                          "opcode 'StorePatchConstant' should only be used in "
                          "'PatchConstant function'");
}

TEST_F(ValidationTest, LoadOutputControlPointNotInPatchConstantFunction) {
  if (m_ver.SkipDxilVersion(1, 3))
    return;
  RewriteAssemblyCheckMsg("struct PSSceneIn \
    { \
    float4 pos  : SV_Position; \
    float2 tex  : TEXCOORD0; \
    float3 norm : NORMAL; \
    }; \
       \
    struct HSPerVertexData  \
    { \
    PSSceneIn v; \
    }; \
    struct HSPerPatchData  \
{  \
	float	edges[ 3 ]	: SV_TessFactor; \
	float	inside		: SV_InsideTessFactor; \
};  \
HSPerPatchData HSPerPatchFunc( const InputPatch< PSSceneIn, 3 > points, \
     OutputPatch<HSPerVertexData, 3> outpoints) \
{ \
    HSPerPatchData d; \
     \
    d.edges[ 0 ] = points[0].tex.x + outpoints[0].v.tex.x; \
    d.edges[ 1 ] = 1; \
    d.edges[ 2 ] = 1; \
    d.inside = 1; \
    \
    return d; \
}\
[domain(\"tri\")]\
[partitioning(\"fractional_odd\")]\
[outputtopology(\"triangle_cw\")]\
[patchconstantfunc(\"HSPerPatchFunc\")]\
[outputcontrolpoints(3)]\
HSPerVertexData main( const uint id : SV_OutputControlPointID,\
                               const InputPatch< PSSceneIn, 3 > points )\
{\
    HSPerVertexData v;\
    \
    v.v = points[ id ];\
    \
	return v;\
}\
    ",
                          "hs_6_0", "dx.op.loadInput.f32(i32 4",
                          "dx.op.loadOutputControlPoint.f32(i32 103",
                          "opcode 'LoadOutputControlPoint' should only be used "
                          "in 'PatchConstant function'");
}

TEST_F(ValidationTest, OutputControlPointIDInPatchConstantFunction) {
  if (m_ver.SkipDxilVersion(1, 3))
    return;
  RewriteAssemblyCheckMsg(
      "struct PSSceneIn \
    { \
    float4 pos  : SV_Position; \
    float2 tex  : TEXCOORD0; \
    float3 norm : NORMAL; \
    }; \
       \
    struct HSPerVertexData  \
    { \
    PSSceneIn v; \
    }; \
    struct HSPerPatchData  \
{  \
	float	edges[ 3 ]	: SV_TessFactor; \
	float	inside		: SV_InsideTessFactor; \
};  \
HSPerPatchData HSPerPatchFunc( const InputPatch< PSSceneIn, 3 > points, \
     OutputPatch<HSPerVertexData, 3> outpoints) \
{ \
    HSPerPatchData d; \
     \
    d.edges[ 0 ] = points[0].tex.x + outpoints[0].v.tex.x; \
    d.edges[ 1 ] = 1; \
    d.edges[ 2 ] = 1; \
    d.inside = 1; \
    \
    return d; \
}\
[domain(\"tri\")]\
[partitioning(\"fractional_odd\")]\
[outputtopology(\"triangle_cw\")]\
[patchconstantfunc(\"HSPerPatchFunc\")]\
[outputcontrolpoints(3)]\
HSPerVertexData main( const uint id : SV_OutputControlPointID,\
                               const InputPatch< PSSceneIn, 3 > points )\
{\
    HSPerVertexData v;\
    \
    v.v = points[ id ];\
    \
	return v;\
}\
    ",
      "hs_6_0", "ret void",
      "call i32 @dx.op.outputControlPointID.i32(i32 107)\n ret void",
      "opcode 'OutputControlPointID' should only be used in 'hull function'");
}

TEST_F(ValidationTest, ClipCullMaxComponents) {
  RewriteAssemblyCheckMsg(" \
struct VSOut { \
  float3 clip0 : SV_ClipDistance; \
  float3 clip1 : SV_ClipDistance1; \
  float cull0 : SV_CullDistance; \
  float cull1 : SV_CullDistance1; \
  float cull2 : CullDistance2; \
}; \
VSOut main() { \
  VSOut Out; \
  Out.clip0 = 0.1; \
  Out.clip1 = 0.2; \
  Out.cull0 = 0.3; \
  Out.cull1 = 0.4; \
  Out.cull2 = 0.5; \
  return Out; \
} \
    ",
                          "vs_6_0", "!{i32 4, !\"CullDistance\", i8 9, i8 0,",
                          "!{i32 4, !\"SV_CullDistance\", i8 9, i8 7,",
                          "ClipDistance and CullDistance use more than the "
                          "maximum of 8 components combined.");
}

TEST_F(ValidationTest, ClipCullMaxRows) {
  RewriteAssemblyCheckMsg(" \
struct VSOut { \
  float3 clip0 : SV_ClipDistance; \
  float3 clip1 : SV_ClipDistance1; \
  float2 cull0 : CullDistance; \
}; \
VSOut main() { \
  VSOut Out; \
  Out.clip0 = 0.1; \
  Out.clip1 = 0.2; \
  Out.cull0 = 0.3; \
  return Out; \
} \
    ",
                          "vs_6_0", "!{i32 2, !\"CullDistance\", i8 9, i8 0,",
                          "!{i32 2, !\"SV_CullDistance\", i8 9, i8 7,",
                          "ClipDistance and CullDistance occupy more than the "
                          "maximum of 2 rows combined.");
}

TEST_F(ValidationTest, DuplicateSysValue) {
  RewriteAssemblyCheckMsg(" \
float4 main(uint vid : SV_VertexID, uint iid : SV_InstanceID) : SV_Position { \
  return (float4)0 + vid + iid; \
} \
    ",
                          "vs_6_0", "!{i32 1, !\"SV_InstanceID\", i8 5, i8 2,",
                          "!{i32 1, !\"\", i8 5, i8 1,",
                          //"System value SV_VertexID appears more than once in
                          // the same signature.");
                          "Semantic 'SV_VertexID' overlap at 0");
}

TEST_F(ValidationTest, SemTargetMax) {
  RewriteAssemblyCheckMsg(" \
float4 main(float4 col : COLOR) : SV_Target7 { return col; } \
    ",
                          "ps_6_0",
                          {"!{i32 0, !\"SV_Target\", i8 9, i8 16, ![0-9]+, i8 "
                           "0, i32 1, i8 4, i32 7, i8 0, (.*)}",
                           "?!dx.viewIdState ="},
                          {"!{i32 0, !\"SV_Target\", i8 9, i8 16, !101, i8 0, "
                           "i32 1, i8 4, i32 8, i8 0, \\1}\n!101 = !{i32 8}",
                           "!1012 ="},
                          "SV_Target semantic index exceeds maximum \\(7\\)",
                          /*bRegex*/ true);
}

TEST_F(ValidationTest, SemTargetIndexMatchesRow) {
  RewriteAssemblyCheckMsg(
      " \
float4 main(float4 col : COLOR) : SV_Target7 { return col; } \
    ",
      "ps_6_0",
      {"!{i32 0, !\"SV_Target\", i8 9, i8 16, !([0-9]+), i8 0, i32 1, i8 4, "
       "i32 7, i8 0, (.*)}",
       "?!dx.viewIdState ="},
      {"!{i32 0, !\"SV_Target\", i8 9, i8 16, !\\1, i8 0, i32 1, i8 4, i32 6, "
       "i8 0, \\2}",
       "!1012 ="},
      "SV_Target semantic index must match packed row location",
      /*bRegex*/ true);
}

TEST_F(ValidationTest, SemTargetCol0) {
  RewriteAssemblyCheckMsg(" \
float3 main(float4 col : COLOR) : SV_Target7 { return col.xyz; } \
    ",
                          "ps_6_0",
                          "!{i32 0, !\"SV_Target\", i8 9, i8 16, !([0-9]+), i8 "
                          "0, i32 1, i8 3, i32 7, i8 0, (.*)}",
                          "!{i32 0, !\"SV_Target\", i8 9, i8 16, !\\1, i8 0, "
                          "i32 1, i8 3, i32 7, i8 1, \\2}",
                          "SV_Target packed location must start at column 0",
                          /*bRegex*/ true);
}

TEST_F(ValidationTest, SemIndexMax) {
  RewriteAssemblyCheckMsg(" \
float4 main(uint vid : SV_VertexID, uint iid : SV_InstanceID) : SV_Position { \
  return (float4)0 + vid + iid; \
} \
    ",
                          "vs_6_0",
                          "!{i32 0, !\"SV_VertexID\", i8 5, i8 1, ![0-9]+, i8 "
                          "0, i32 1, i8 1, i32 0, i8 0, (.*)}",
                          "!{i32 0, !\"SV_VertexID\", i8 5, i8 1, !101, i8 0, "
                          "i32 1, i8 1, i32 0, i8 0, \\1}\n!101 = !{i32 1}",
                          "SV_VertexID semantic index exceeds maximum \\(0\\)",
                          /*bRegex*/ true);
}

TEST_F(ValidationTest, SemTessFactorIndexMax) {
  RewriteAssemblyCheckMsg(
      " \
struct Vertex { \
  float4 pos : SV_Position; \
}; \
struct PatchConstant { \
  float edges[ 3 ]  : SV_TessFactor; \
  float inside    : SV_InsideTessFactor; \
}; \
PatchConstant PCMain( InputPatch<Vertex, 3> patch) { \
  PatchConstant PC; \
  PC.edges = (float[3])patch[1].pos.xyz; \
  PC.inside = patch[1].pos.w; \
  return PC; \
} \
[domain(\"tri\")] \
[partitioning(\"fractional_odd\")] \
[outputtopology(\"triangle_cw\")] \
[patchconstantfunc(\"PCMain\")] \
[outputcontrolpoints(3)] \
Vertex main(uint id : SV_OutputControlPointID, InputPatch< Vertex, 3 > patch) { \
  Vertex Out = patch[id]; \
  Out.pos.w += 0.25; \
  return Out; \
} \
    ",
      "hs_6_0",
      "!{i32 0, !\"SV_TessFactor\", i8 9, i8 25, ![0-9]+, i8 0, i32 3, i8 1, "
      "i32 0, i8 3, (.*)}",
      "!{i32 0, !\"SV_TessFactor\", i8 9, i8 25, !101, i8 0, i32 2, i8 1, i32 "
      "0, i8 3, \\1}\n!101 = !{i32 0, i32 1}",
      "TessFactor rows, columns \\(2, 1\\) invalid for domain Tri.  Expected 3 "
      "rows and 1 column.",
      /*bRegex*/ true);
}

TEST_F(ValidationTest, SemInsideTessFactorIndexMax) {
  RewriteAssemblyCheckMsg(
      " \
struct Vertex { \
  float4 pos : SV_Position; \
}; \
struct PatchConstant { \
  float edges[ 3 ]  : SV_TessFactor; \
  float inside    : SV_InsideTessFactor; \
}; \
PatchConstant PCMain( InputPatch<Vertex, 3> patch) { \
  PatchConstant PC; \
  PC.edges = (float[3])patch[1].pos.xyz; \
  PC.inside = patch[1].pos.w; \
  return PC; \
} \
[domain(\"tri\")] \
[partitioning(\"fractional_odd\")] \
[outputtopology(\"triangle_cw\")] \
[patchconstantfunc(\"PCMain\")] \
[outputcontrolpoints(3)] \
Vertex main(uint id : SV_OutputControlPointID, InputPatch< Vertex, 3 > patch) { \
  Vertex Out = patch[id]; \
  Out.pos.w += 0.25; \
  return Out; \
} \
    ",
      "hs_6_0",
      {"!{i32 1, !\"SV_InsideTessFactor\", i8 9, i8 26, !([0-9]+), i8 0, i32 "
       "1, i8 1, i32 3, i8 0, (.*)}",
       "?!dx.viewIdState ="},
      {"!{i32 1, !\"SV_InsideTessFactor\", i8 9, i8 26, !101, i8 0, i32 2, i8 "
       "1, i32 3, i8 0, \\2}\n!101 = !{i32 0, i32 1}",
       "!1012 ="},
      "InsideTessFactor rows, columns \\(2, 1\\) invalid for domain Tri.  "
      "Expected 1 rows and 1 column.",
      /*bRegex*/ true);
}

TEST_F(ValidationTest, SemShouldBeAllocated) {
  RewriteAssemblyCheckMsg(" \
struct Vertex { \
  float4 pos : SV_Position; \
}; \
struct PatchConstant { \
  float edges[ 3 ]  : SV_TessFactor; \
  float inside    : SV_InsideTessFactor; \
}; \
PatchConstant PCMain( InputPatch<Vertex, 3> patch) { \
  PatchConstant PC; \
  PC.edges = (float[3])patch[1].pos.xyz; \
  PC.inside = patch[1].pos.w; \
  return PC; \
} \
[domain(\"tri\")] \
[partitioning(\"fractional_odd\")] \
[outputtopology(\"triangle_cw\")] \
[patchconstantfunc(\"PCMain\")] \
[outputcontrolpoints(3)] \
Vertex main(uint id : SV_OutputControlPointID, InputPatch< Vertex, 3 > patch) { \
  Vertex Out = patch[id]; \
  Out.pos.w += 0.25; \
  return Out; \
} \
    ",
                          "hs_6_0",
                          "!{i32 0, !\"SV_TessFactor\", i8 9, i8 25, "
                          "!([0-9]+), i8 0, i32 3, i8 1, i32 0, i8 3, (.*)}",
                          "!{i32 0, !\"SV_TessFactor\", i8 9, i8 25, !\\1, i8 "
                          "0, i32 3, i8 1, i32 -1, i8 -1, \\2}",
                          "PatchConstant Semantic 'SV_TessFactor' should have "
                          "a valid packing location",
                          /*bRegex*/ true);
}

TEST_F(ValidationTest, SemShouldNotBeAllocated) {
  RewriteAssemblyCheckMsg(
      " \
float4 main(float4 col : COLOR, out uint coverage : SV_Coverage) : SV_Target7 { coverage = 7; return col; } \
    ",
      "ps_6_0",
      "!\"SV_Coverage\", i8 5, i8 14, !([0-9]+), i8 0, i32 1, i8 1, i32 -1, i8 "
      "-1, (.*)}",
      "!\"SV_Coverage\", i8 5, i8 14, !\\1, i8 0, i32 1, i8 1, i32 2, i8 0, "
      "\\2}",
      "Output Semantic 'SV_Coverage' should have a packing location of -1",
      /*bRegex*/ true);
}

TEST_F(ValidationTest, SemComponentOrder) {
  // error updated, so must exclude previous validator versions.
  if (m_ver.SkipDxilVersion(1, 6))
    return;
  RewriteAssemblyCheckMsg(
      " \
void main( \
  float2 f2in : f2in, \
  float3 f3in : f3in, \
  uint vid : SV_VertexID, \
  uint iid : SV_InstanceID, \
  out float4 pos : SV_Position, \
  out float2 f2out : f2out, \
  out float3 f3out : f3out, \
  out float2 ClipDistance : SV_ClipDistance, \
  out float CullDistance : SV_CullDistance) \
{ \
  pos = float4(f3in, f2in.x); \
  ClipDistance = f2in.x; \
  CullDistance = f2in.y; \
} \
    ",
      "vs_6_0",

      {"= !{i32 1, !\"f2out\", i8 9, i8 0, !([0-9]+), i8 2, i32 1, i8 2, i32 "
       "1, i8 0, (.*)}\n"
       "!([0-9]+) = !{i32 2, !\"f3out\", i8 9, i8 0, !([0-9]+), i8 2, i32 1, "
       "i8 3, i32 2, i8 0, (.*)}\n"
       "!([0-9]+) = !{i32 3, !\"SV_ClipDistance\", i8 9, i8 6, !([0-9]+), i8 "
       "2, i32 1, i8 2, i32 3, i8 0, (.*)}\n"
       "!([0-9]+) = !{i32 4, !\"SV_CullDistance\", i8 9, i8 7, !([0-9]+), i8 "
       "2, i32 1, i8 1, i32 3, i8 2, (.*)}\n",
       "?!dx.viewIdState ="},

      {"= !{i32 1, !\"f2out\", i8 9, i8 0, !\\1, i8 2, i32 1, i8 2, i32 1, i8 "
       "2, \\2}\n"
       "!\\3 = !{i32 2, !\"f3out\", i8 9, i8 0, !\\4, i8 2, i32 1, i8 3, i32 "
       "2, i8 1, \\5}\n"
       "!\\6 = !{i32 3, !\"SV_ClipDistance\", i8 9, i8 6, !\\7, i8 2, i32 1, "
       "i8 2, i32 2, i8 0, \\8}\n"
       "!\\9 = !{i32 4, !\"SV_CullDistance\", i8 9, i8 7, !\\10, i8 2, i32 1, "
       "i8 1, i32 1, i8 0, \\11}\n",
       "!1012 ="},

      {"signature element SV_ClipDistance at location \\(2,0\\) size \\(1,2\\) "
       "violates component ordering rule \\(arb < sv < sgv\\).",
       "signature element SV_CullDistance at location \\(1,0\\) size \\(1,1\\) "
       "violates component ordering rule \\(arb < sv < sgv\\)."},
      /*bRegex*/ true);
}

TEST_F(ValidationTest, SemComponentOrder2) {
  // error updated, so must exclude previous validator versions.
  if (m_ver.SkipDxilVersion(1, 6))
    return;
  RewriteAssemblyCheckMsg(
      " \
float4 main( \
  float4 col : Color, \
  uint2 val : Value, \
  uint pid : SV_PrimitiveID, \
  bool ff : SV_IsFrontFace) : SV_Target \
{ \
  return col; \
} \
    ",
      "ps_6_0",

      "= !{i32 1, !\"Value\", i8 5, i8 0, !([0-9]+), i8 1, i32 1, i8 2, i32 1, "
      "i8 0, null}\n"
      "!([0-9]+) = !{i32 2, !\"SV_PrimitiveID\", i8 5, i8 10, !([0-9]+), i8 1, "
      "i32 1, i8 1, i32 1, i8 2, null}\n"
      "!([0-9]+) = !{i32 3, !\"SV_IsFrontFace\", i8 ([15]), i8 13, !([0-9]+), "
      "i8 1, i32 1, i8 1, i32 1, i8 3, null}\n",

      "= !{i32 1, !\"Value\", i8 5, i8 0, !\\1, i8 1, i32 1, i8 2, i32 1, i8 "
      "2, null}\n"
      "!\\2 = !{i32 2, !\"SV_PrimitiveID\", i8 5, i8 10, !\\3, i8 1, i32 1, i8 "
      "1, i32 1, i8 0, null}\n"
      "!\\4 = !{i32 3, !\"SV_IsFrontFace\", i8 \\5, i8 13, !\\6, i8 1, i32 1, "
      "i8 1, i32 1, i8 1, null}\n",

      {"signature element SV_PrimitiveID at location \\(1,0\\) size \\(1,1\\) "
       "violates component ordering rule \\(arb < sv < sgv\\).",
       "signature element SV_IsFrontFace at location \\(1,1\\) size \\(1,1\\) "
       "violates component ordering rule \\(arb < sv < sgv\\)."},
      /*bRegex*/ true);
}

TEST_F(ValidationTest, SemComponentOrder3) {
  // error updated, so must exclude previous validator versions.
  if (m_ver.SkipDxilVersion(1, 6))
    return;
  RewriteAssemblyCheckMsg(
      " \
float4 main( \
  float4 col : Color, \
  uint val : Value, \
  uint pid : SV_PrimitiveID, \
  bool ff : SV_IsFrontFace, \
  uint vpid : ViewPortArrayIndex) : SV_Target \
{ \
  return col; \
} \
    ",
      "ps_6_0",

      {"= !{i32 1, !\"Value\", i8 5, i8 0, !([0-9]+), i8 1, i32 1, i8 1, i32 "
       "1, i8 0, null}\n"
       "!([0-9]+) = !{i32 2, !\"SV_PrimitiveID\", i8 5, i8 10, !([0-9]+), i8 "
       "1, i32 1, i8 1, i32 1, i8 1, null}\n"
       "!([0-9]+) = !{i32 3, !\"SV_IsFrontFace\", i8 ([15]), i8 13, !([0-9]+), "
       "i8 1, i32 1, i8 1, i32 1, i8 2, null}\n"
       "!([0-9]+) = !{i32 4, !\"ViewPortArrayIndex\", i8 5, i8 0, !([0-9]+), "
       "i8 1, i32 1, i8 1, i32 2, i8 0, null}\n",
       "?!dx.viewIdState ="},

      {"= !{i32 1, !\"Value\", i8 5, i8 0, !\\1, i8 1, i32 1, i8 1, i32 1, i8 "
       "1, null}\n"
       "!\\2 = !{i32 2, !\"SV_PrimitiveID\", i8 5, i8 10, !\\3, i8 1, i32 1, "
       "i8 1, i32 1, i8 0, null}\n"
       "!\\4 = !{i32 3, !\"SV_IsFrontFace\", i8 \\5, i8 13, !\\6, i8 1, i32 1, "
       "i8 1, i32 1, i8 2, null}\n"
       "!\\7 = !{i32 4, !\"ViewPortArrayIndex\", i8 5, i8 0, !\\8, i8 1, i32 "
       "1, i8 1, i32 1, i8 3, null}\n",
       "!1012 ="},

      {"signature element SV_PrimitiveID at location \\(1,0\\) size \\(1,1\\) "
       "violates component ordering rule \\(arb < sv < sgv\\).",
       "signature element ViewPortArrayIndex at location \\(1,3\\) size "
       "\\(1,1\\) violates component ordering rule \\(arb < sv < sgv\\)."},
      /*bRegex*/ true);
}

TEST_F(ValidationTest, SemIndexConflictArbSV) {
  RewriteAssemblyCheckMsg(
      " \
void main( \
  float4 inpos : Position, \
  uint iid : SV_InstanceID, \
  out float4 pos : SV_Position, \
  out uint id[2] : Array, \
  out uint vpid : SV_ViewPortArrayIndex, \
  out float2 ClipDistance : SV_ClipDistance, \
  out float CullDistance : SV_CullDistance) \
{ \
  pos = inpos; \
  ClipDistance = inpos.x; \
  CullDistance = inpos.y; \
  vpid = iid; \
  id[0] = iid; \
  id[1] = iid + 1; \
} \
    ",
      "vs_6_0",

      "!{i32 2, !\"SV_ViewportArrayIndex\", i8 5, i8 5, !([0-9]+), i8 1, i32 "
      "1, i8 1, i32 3, i8 0, (.*)}",
      "!{i32 2, !\"SV_ViewportArrayIndex\", i8 5, i8 5, !\\1, i8 1, i32 1, i8 "
      "1, i32 1, i8 3, \\2}",

      "signature element SV_ViewportArrayIndex at location \\(1,3\\) size "
      "\\(1,1\\) has an indexing conflict with another signature element "
      "packed into the same row.",
      /*bRegex*/ true);
}

TEST_F(ValidationTest, SemIndexConflictTessfactors) {
  RewriteAssemblyCheckMsg(
      " \
struct Vertex { \
  float4 pos : SV_Position; \
}; \
struct PatchConstant { \
  float edges[ 4 ]  : SV_TessFactor; \
  float inside[ 2 ] : SV_InsideTessFactor; \
}; \
PatchConstant PCMain( InputPatch<Vertex, 4> patch) { \
  PatchConstant PC; \
  PC.edges = (float[4])patch[1].pos; \
  PC.inside = (float[2])patch[1].pos.xy; \
  return PC; \
} \
[domain(\"quad\")] \
[partitioning(\"fractional_odd\")] \
[outputtopology(\"triangle_cw\")] \
[patchconstantfunc(\"PCMain\")] \
[outputcontrolpoints(4)] \
Vertex main(uint id : SV_OutputControlPointID, InputPatch< Vertex, 4 > patch) { \
  Vertex Out = patch[id]; \
  Out.pos.w += 0.25; \
  return Out; \
} \
    ",
      "hs_6_0",
      //!{i32 0, !"SV_TessFactor", i8 9, i8 25, !23, i8 0, i32 4, i8 1, i32 0,
      //! i8 3, null}
      {"!{i32 1, !\"SV_InsideTessFactor\", i8 9, i8 26, !([0-9]+), i8 0, i32 "
       "2, i8 1, i32 4, i8 3, (.*)}",
       "?!dx.viewIdState ="},
      {"!{i32 1, !\"SV_InsideTessFactor\", i8 9, i8 26, !\\1, i8 0, i32 2, i8 "
       "1, i32 0, i8 2, \\2}",
       "!1012 ="},
      "signature element SV_InsideTessFactor at location \\(0,2\\) size "
      "\\(2,1\\) has an indexing conflict with another signature element "
      "packed into the same row.",
      /*bRegex*/ true);
}

TEST_F(ValidationTest, SemIndexConflictTessfactors2) {
  RewriteAssemblyCheckMsg(" \
struct Vertex { \
  float4 pos : SV_Position; \
}; \
struct PatchConstant { \
  float edges[ 4 ]  : SV_TessFactor; \
  float inside[ 2 ] : SV_InsideTessFactor; \
  float arb [ 3 ] : Arb; \
}; \
PatchConstant PCMain( InputPatch<Vertex, 4> patch) { \
  PatchConstant PC; \
  PC.edges = (float[4])patch[1].pos; \
  PC.inside = (float[2])patch[1].pos.xy; \
  PC.arb[0] = 1; PC.arb[1] = 2; PC.arb[2] = 3; \
  return PC; \
} \
[domain(\"quad\")] \
[partitioning(\"fractional_odd\")] \
[outputtopology(\"triangle_cw\")] \
[patchconstantfunc(\"PCMain\")] \
[outputcontrolpoints(4)] \
Vertex main(uint id : SV_OutputControlPointID, InputPatch< Vertex, 4 > patch) { \
  Vertex Out = patch[id]; \
  Out.pos.w += 0.25; \
  return Out; \
} \
    ",
                          "hs_6_0",
                          "!{i32 2, !\"Arb\", i8 9, i8 0, !([0-9]+), i8 0, i32 "
                          "3, i8 1, i32 0, i8 0, (.*)}",
                          "!{i32 2, !\"Arb\", i8 9, i8 0, !\\1, i8 0, i32 3, "
                          "i8 1, i32 2, i8 0, \\2}",
                          "signature element Arb at location \\(2,0\\) size "
                          "\\(3,1\\) has an indexing conflict with another "
                          "signature element packed into the same row.",
                          /*bRegex*/ true);
}

TEST_F(ValidationTest, SemRowOutOfRange) {
  RewriteAssemblyCheckMsg(" \
struct Vertex { \
  float4 pos : SV_Position; \
}; \
struct PatchConstant { \
  float edges[ 4 ]  : SV_TessFactor; \
  float inside[ 2 ] : SV_InsideTessFactor; \
  float arb [ 3 ] : Arb; \
}; \
PatchConstant PCMain( InputPatch<Vertex, 4> patch) { \
  PatchConstant PC; \
  PC.edges = (float[4])patch[1].pos; \
  PC.inside = (float[2])patch[1].pos.xy; \
  PC.arb[0] = 1; PC.arb[1] = 2; PC.arb[2] = 3; \
  return PC; \
} \
[domain(\"quad\")] \
[partitioning(\"fractional_odd\")] \
[outputtopology(\"triangle_cw\")] \
[patchconstantfunc(\"PCMain\")] \
[outputcontrolpoints(4)] \
Vertex main(uint id : SV_OutputControlPointID, InputPatch< Vertex, 4 > patch) { \
  Vertex Out = patch[id]; \
  Out.pos.w += 0.25; \
  return Out; \
} \
    ",
                          "hs_6_0",
                          {"!{i32 2, !\"Arb\", i8 9, i8 0, !([0-9]+), i8 0, "
                           "i32 3, i8 1, i32 0, i8 0, (.*)}",
                           "?!dx.viewIdState ="},
                          {"!{i32 2, !\"Arb\", i8 9, i8 0, !\\1, i8 0, i32 3, "
                           "i8 1, i32 31, i8 0, \\2}",
                           "!1012 ="},
                          "signature element Arb at location \\(31,0\\) size "
                          "\\(3,1\\) is out of range.",
                          /*bRegex*/ true);
}

TEST_F(ValidationTest, SemPackOverlap) {
  RewriteAssemblyCheckMsg(" \
struct Vertex { \
  float4 pos : SV_Position; \
}; \
struct PatchConstant { \
  float edges[ 4 ]  : SV_TessFactor; \
  float inside[ 2 ] : SV_InsideTessFactor; \
  float arb [ 3 ] : Arb; \
}; \
PatchConstant PCMain( InputPatch<Vertex, 4> patch) { \
  PatchConstant PC; \
  PC.edges = (float[4])patch[1].pos; \
  PC.inside = (float[2])patch[1].pos.xy; \
  PC.arb[0] = 1; PC.arb[1] = 2; PC.arb[2] = 3; \
  return PC; \
} \
[domain(\"quad\")] \
[partitioning(\"fractional_odd\")] \
[outputtopology(\"triangle_cw\")] \
[patchconstantfunc(\"PCMain\")] \
[outputcontrolpoints(4)] \
Vertex main(uint id : SV_OutputControlPointID, InputPatch< Vertex, 4 > patch) { \
  Vertex Out = patch[id]; \
  Out.pos.w += 0.25; \
  return Out; \
} \
    ",
                          "hs_6_0",
                          "!{i32 2, !\"Arb\", i8 9, i8 0, !([0-9]+), i8 0, i32 "
                          "3, i8 1, i32 0, i8 0, (.*)}",
                          "!{i32 2, !\"Arb\", i8 9, i8 0, !\\1, i8 0, i32 3, "
                          "i8 1, i32 1, i8 3, \\2}",
                          "signature element Arb at location \\(1,3\\) size "
                          "\\(3,1\\) overlaps another signature element.",
                          /*bRegex*/ true);
}

TEST_F(ValidationTest, SemPackOverlap2) {
  RewriteAssemblyCheckMsg(" \
void main( \
  float4 inpos : Position, \
  uint iid : SV_InstanceID, \
  out float4 pos : SV_Position, \
  out uint id[2] : Array, \
  out uint3 value : Value, \
  out float2 ClipDistance : SV_ClipDistance, \
  out float CullDistance : SV_CullDistance) \
{ \
  pos = inpos; \
  ClipDistance = inpos.x; \
  CullDistance = inpos.y; \
  value = iid; \
  id[0] = iid; \
  id[1] = iid + 1; \
} \
    ",
                          "vs_6_0",

                          {"!{i32 1, !\"Array\", i8 5, i8 0, !([0-9]+), i8 1, "
                           "i32 2, i8 1, i32 1, i8 0, (.*)}(.*)"
                           "!\\1 = !{i32 0, i32 1}\n",
                           "= !{i32 2, !\"Value\", i8 5, i8 0, !([0-9]+), i8 "
                           "1, i32 1, i8 3, i32 1, i8 1, (.*)}"},

                          {"!{i32 1, !\"Array\", i8 5, i8 0, !\\1, i8 1, i32 "
                           "2, i8 1, i32 1, i8 1, \\2}\\3"
                           "!\\1 = !{i32 0, i32 1}\n",
                           "= !{i32 2, !\"Value\", i8 5, i8 0, !\\1, i8 1, i32 "
                           "1, i8 3, i32 2, i8 0, \\2}"},

                          "signature element Value at location \\(2,0\\) size "
                          "\\(1,3\\) overlaps another signature element.",
                          /*bRegex*/ true);
}

TEST_F(ValidationTest, SemMultiDepth) {
  RewriteAssemblyCheckMsg(
      " \
float4 main(float4 f4 : Input, out float d0 : SV_Depth, out float d1 : SV_Target) : SV_Target1 \
{ d0 = f4.z; d1 = f4.w; return f4; } \
    ",
      "ps_6_0",
      {"!{i32 2, !\"SV_Target\", i8 9, i8 16, !([0-9]+), i8 0, i32 1, i8 1, "
       "i32 0, i8 0, (.*)}"},
      {"!{i32 2, !\"SV_DepthGreaterEqual\", i8 9, i8 19, !\\1, i8 0, i32 1, i8 "
       "1, i32 -1, i8 -1, \\2}"},
      "Pixel Shader only allows one type of depth semantic to be declared",
      /*bRegex*/ true);
}

TEST_F(ValidationTest, WhenRootSigMismatchThenFail) {
  ReplaceContainerPartsCheckMsgs(
      "float c; [RootSignature ( \"RootConstants(b0, num32BitConstants = 1)\" "
      ")] float4 main() : semantic { return c; }",
      "[RootSignature ( \"\" )] float4 main() : semantic { return 0; }",
      "vs_6_0", {DFCC_RootSignature},
      {"Root Signature in DXIL container is not compatible with shader.",
       "Validation failed."});
}
TEST_F(ValidationTest, WhenRootSigCompatThenSucceed) {
  ReplaceContainerPartsCheckMsgs(
      "[RootSignature ( \"\" )] float4 main() : semantic { return 0; }",
      "float c; [RootSignature ( \"RootConstants(b0, num32BitConstants = 1)\" "
      ")] float4 main() : semantic { return c; }",
      "vs_6_0", {DFCC_RootSignature}, {});
}

TEST_F(ValidationTest, WhenRootSigMatchShaderSucceed_RootConstVis) {
  ReplaceContainerPartsCheckMsgs(
      "float c; float4 main() : semantic { return c; }",
      "[RootSignature ( \"RootConstants(b0, visibility = "
      "SHADER_VISIBILITY_VERTEX, num32BitConstants = 1)\" )]"
      "  float4 main() : semantic { return 0; }",
      "vs_6_0", {DFCC_RootSignature}, {});
}
TEST_F(ValidationTest, WhenRootSigMatchShaderFail_RootConstVis) {
  ReplaceContainerPartsCheckMsgs(
      "float c; float4 main() : semantic { return c; }",
      "[RootSignature ( \"RootConstants(b0, visibility = "
      "SHADER_VISIBILITY_PIXEL, num32BitConstants = 1)\" )]"
      "  float4 main() : semantic { return 0; }",
      "vs_6_0", {DFCC_RootSignature},
      {"Root Signature in DXIL container is not compatible with shader.",
       "Validation failed."});
}

TEST_F(ValidationTest, WhenRootSigMatchShaderSucceed_RootCBV) {
  ReplaceContainerPartsCheckMsgs(
      "struct Foo { float a; int4 b; }; "
      "ConstantBuffer<Foo> cb1 : register(b2, space5); "
      "float4 main() : semantic { return cb1.b.x; }",
      "[RootSignature ( \"CBV(b2, space = 5)\" )]"
      "  float4 main() : semantic { return 0; }",
      "vs_6_0", {DFCC_RootSignature}, {});
}
TEST_F(ValidationTest, WhenRootSigMatchShaderFail_RootCBV_Range) {
  ReplaceContainerPartsCheckMsgs(
      "struct Foo { float a; int4 b; }; "
      "ConstantBuffer<Foo> cb1 : register(b0, space5); "
      "float4 main() : semantic { return cb1.b.x; }",
      "[RootSignature ( \"CBV(b2, space = 5)\" )]"
      "  float4 main() : semantic { return 0; }",
      "vs_6_0", {DFCC_RootSignature},
      {"Root Signature in DXIL container is not compatible with shader.",
       "Validation failed."});
}
TEST_F(ValidationTest, WhenRootSigMatchShaderFail_RootCBV_Space) {
  ReplaceContainerPartsCheckMsgs(
      "struct Foo { float a; int4 b; }; "
      "ConstantBuffer<Foo> cb1 : register(b2, space7); "
      "float4 main() : semantic { return cb1.b.x; }",
      "[RootSignature ( \"CBV(b2, space = 5)\" )]"
      "  float4 main() : semantic { return 0; }",
      "vs_6_0", {DFCC_RootSignature},
      {"Root Signature in DXIL container is not compatible with shader.",
       "Validation failed."});
}

TEST_F(ValidationTest, WhenRootSigMatchShaderSucceed_RootSRV) {
  ReplaceContainerPartsCheckMsgs(
      "struct Foo { float4 a; }; "
      "StructuredBuffer<Foo> buf1 : register(t1, space3); "
      "float4 main(float4 a : AAA) : SV_Target { return buf1[a.x].a; }",
      "[RootSignature ( \"SRV(t1, space = 3)\" )]"
      "  float4 main() : SV_Target { return 0; }",
      "ps_6_0", {DFCC_RootSignature}, {});
}
TEST_F(ValidationTest, WhenRootSigMatchShaderFail_RootSRV_ResType) {
  ReplaceContainerPartsCheckMsgs(
      "struct Foo { float4 a; }; "
      "StructuredBuffer<Foo> buf1 : register(t1, space3); "
      "float4 main(float4 a : AAA) : SV_Target { return buf1[a.x].a; }",
      "[RootSignature ( \"UAV(u1, space = 3)\" )]"
      "  float4 main() : SV_Target { return 0; }",
      "ps_6_0", {DFCC_RootSignature},
      {"Root Signature in DXIL container is not compatible with shader.",
       "Validation failed."});
}

TEST_F(ValidationTest, WhenRootSigMatchShaderSucceed_RootUAV) {
  ReplaceContainerPartsCheckMsgs(
      "struct Foo { float4 a; }; "
      "RWStructuredBuffer<Foo> buf1 : register(u1, space3); "
      "float4 main(float4 a : AAA) : SV_Target { return buf1[a.x].a; }",
      "[RootSignature ( \"UAV(u1, space = 3)\" )]"
      "  float4 main() : SV_Target { return 0; }",
      "ps_6_0", {DFCC_RootSignature}, {});
}

TEST_F(ValidationTest, WhenRootSigMatchShaderSucceed_DescTable) {
  ReplaceContainerPartsCheckMsgs(
      "struct Foo { int a; float4 b; };"
      ""
      "ConstantBuffer<Foo> cb1[4] : register(b2, space5);"
      "Texture2D<float4> tex1[8]  : register(t1, space3);"
      "RWBuffer<float4> buf1[6]   : register(u33, space17);"
      "SamplerState sampler1[5]   : register(s0, space0);"
      ""
      "float4 main(float4 a : AAA) : SV_TARGET"
      "{"
      "  return buf1[a.x][a.y] + cb1[a.x].b + tex1[a.x].Sample(sampler1[a.x], "
      "a.xy);"
      "}",
      "[RootSignature(\"DescriptorTable( SRV(t1,space=3,numDescriptors=8), "
      "CBV(b2,space=5,numDescriptors=4), "
      "UAV(u33,space=17,numDescriptors=6)), "
      "DescriptorTable(Sampler(s0, numDescriptors=5))\")]"
      "  float4 main() : SV_Target { return 0; }",
      "ps_6_0", {DFCC_RootSignature}, {});
}

TEST_F(ValidationTest, WhenRootSigMatchShaderSucceed_DescTable_GoodRange) {
  ReplaceContainerPartsCheckMsgs(
      "struct Foo { int a; float4 b; };"
      ""
      "ConstantBuffer<Foo> cb1[4] : register(b2, space5);"
      "Texture2D<float4> tex1[8]  : register(t1, space3);"
      "RWBuffer<float4> buf1[6]   : register(u33, space17);"
      "SamplerState sampler1[5]   : register(s0, space0);"
      ""
      "float4 main(float4 a : AAA) : SV_TARGET"
      "{"
      "  return buf1[a.x][a.y] + cb1[a.x].b + tex1[a.x].Sample(sampler1[a.x], "
      "a.xy);"
      "}",
      "[RootSignature(\"DescriptorTable( SRV(t0,space=3,numDescriptors=20), "
      "CBV(b2,space=5,numDescriptors=4), "
      "UAV(u33,space=17,numDescriptors=6)), "
      "DescriptorTable(Sampler(s0, numDescriptors=5))\")]"
      "  float4 main() : SV_Target { return 0; }",
      "ps_6_0", {DFCC_RootSignature}, {});
}

TEST_F(ValidationTest, WhenRootSigMatchShaderSucceed_DescTable_Unbounded) {
  ReplaceContainerPartsCheckMsgs(
      "struct Foo { int a; float4 b; };"
      ""
      "ConstantBuffer<Foo> cb1[4] : register(b2, space5);"
      "Texture2D<float4> tex1[8]  : register(t1, space3);"
      "RWBuffer<float4> buf1[6]   : register(u33, space17);"
      "SamplerState sampler1[5]   : register(s0, space0);"
      ""
      "float4 main(float4 a : AAA) : SV_TARGET"
      "{"
      "  return buf1[a.x][a.y] + cb1[a.x].b + tex1[a.x].Sample(sampler1[a.x], "
      "a.xy);"
      "}",
      "[RootSignature(\"DescriptorTable( CBV(b2,space=5,numDescriptors=4), "
      "SRV(t1,space=3,numDescriptors=8), "
      "UAV(u10,space=17,numDescriptors=unbounded)), "
      "DescriptorTable(Sampler(s0, numDescriptors=5))\")]"
      "  float4 main() : SV_Target { return 0; }",
      "ps_6_0", {DFCC_RootSignature}, {});
}

TEST_F(ValidationTest, WhenRootSigMatchShaderFail_DescTable_Range1) {
  ReplaceContainerPartsCheckMsgs(
      "struct Foo { int a; float4 b; };"
      ""
      "ConstantBuffer<Foo> cb1[4] : register(b2, space5);"
      "Texture2D<float4> tex1[8]  : register(t1, space3);"
      "RWBuffer<float4> buf1[6]   : register(u33, space17);"
      "SamplerState sampler1[5]   : register(s0, space0);"
      ""
      "float4 main(float4 a : AAA) : SV_TARGET"
      "{"
      "  return buf1[a.x][a.y] + cb1[a.x].b + tex1[a.x].Sample(sampler1[a.x], "
      "a.xy);"
      "}",
      "[RootSignature(\"DescriptorTable( CBV(b2,space=5,numDescriptors=4), "
      "SRV(t2,space=3,numDescriptors=8), "
      "UAV(u33,space=17,numDescriptors=6)), "
      "DescriptorTable(Sampler(s0, numDescriptors=5))\")]"
      "  float4 main() : SV_Target { return 0; }",
      "ps_6_0", {DFCC_RootSignature},
      {"Shader SRV descriptor range (RegisterSpace=3, NumDescriptors=8, "
       "BaseShaderRegister=1) is not fully bound in root signature.",
       "Root Signature in DXIL container is not compatible with shader.",
       "Validation failed."});
}

TEST_F(ValidationTest, WhenRootSigMatchShaderFail_DescTable_Range2) {
  ReplaceContainerPartsCheckMsgs(
      "struct Foo { int a; float4 b; };"
      ""
      "ConstantBuffer<Foo> cb1[4] : register(b2, space5);"
      "Texture2D<float4> tex1[8]  : register(t1, space3);"
      "RWBuffer<float4> buf1[6]   : register(u33, space17);"
      "SamplerState sampler1[5]   : register(s0, space0);"
      ""
      "float4 main(float4 a : AAA) : SV_TARGET"
      "{"
      "  return buf1[a.x][a.y] + cb1[a.x].b + tex1[a.x].Sample(sampler1[a.x], "
      "a.xy);"
      "}",
      "[RootSignature(\"DescriptorTable( SRV(t2,space=3,numDescriptors=8), "
      "CBV(b20,space=5,numDescriptors=4), "
      "UAV(u33,space=17,numDescriptors=6)), "
      "DescriptorTable(Sampler(s0, numDescriptors=5))\")]"
      "  float4 main() : SV_Target { return 0; }",
      "ps_6_0", {DFCC_RootSignature},
      {"Root Signature in DXIL container is not compatible with shader.",
       "Validation failed."});
}

TEST_F(ValidationTest, WhenRootSigMatchShaderFail_DescTable_Range3) {
  ReplaceContainerPartsCheckMsgs(
      "struct Foo { int a; float4 b; };"
      ""
      "ConstantBuffer<Foo> cb1[4] : register(b2, space5);"
      "Texture2D<float4> tex1[8]  : register(t1, space3);"
      "RWBuffer<float4> buf1[6]   : register(u33, space17);"
      "SamplerState sampler1[5]   : register(s0, space0);"
      ""
      "float4 main(float4 a : AAA) : SV_TARGET"
      "{"
      "  return buf1[a.x][a.y] + cb1[a.x].b + tex1[a.x].Sample(sampler1[a.x], "
      "a.xy);"
      "}",
      "[RootSignature(\"DescriptorTable( CBV(b2,space=5,numDescriptors=4), "
      "SRV(t1,space=3,numDescriptors=8), "
      "UAV(u33,space=17,numDescriptors=5)), "
      "DescriptorTable(Sampler(s0, numDescriptors=5))\")]"
      "  float4 main() : SV_Target { return 0; }",
      "ps_6_0", {DFCC_RootSignature},
      {"Root Signature in DXIL container is not compatible with shader.",
       "Validation failed."});
}

TEST_F(ValidationTest, WhenRootSigMatchShaderFail_DescTable_Space) {
  ReplaceContainerPartsCheckMsgs(
      "struct Foo { int a; float4 b; };"
      ""
      "ConstantBuffer<Foo> cb1[4] : register(b2, space5);"
      "Texture2D<float4> tex1[8]  : register(t1, space3);"
      "RWBuffer<float4> buf1[6]   : register(u33, space17);"
      "SamplerState sampler1[5]   : register(s0, space0);"
      ""
      "float4 main(float4 a : AAA) : SV_TARGET"
      "{"
      "  return buf1[a.x][a.y] + cb1[a.x].b + tex1[a.x].Sample(sampler1[a.x], "
      "a.xy);"
      "}",
      "[RootSignature(\"DescriptorTable( SRV(t2,space=3,numDescriptors=8), "
      "CBV(b2,space=5,numDescriptors=4), "
      "UAV(u33,space=0,numDescriptors=6)), "
      "DescriptorTable(Sampler(s0, numDescriptors=5))\")]"
      "  float4 main() : SV_Target { return 0; }",
      "ps_6_0", {DFCC_RootSignature},
      {"Root Signature in DXIL container is not compatible with shader.",
       "Validation failed."});
}

TEST_F(ValidationTest, WhenRootSigMatchShaderSucceed_Unbounded) {
  ReplaceContainerPartsCheckMsgs(
      "struct Foo { int a; float4 b; };"
      ""
      "ConstantBuffer<Foo> cb1[]  : register(b2, space5);"
      "Texture2D<float4> tex1[]   : register(t1, space3);"
      "RWBuffer<float4> buf1[]    : register(u33, space17);"
      "SamplerState sampler1[]    : register(s0, space0);"
      ""
      "float4 main(float4 a : AAA) : SV_TARGET"
      "{"
      "  return buf1[a.x][a.y] + cb1[a.x].b + tex1[a.x].Sample(sampler1[a.x], "
      "a.xy);"
      "}",
      "[RootSignature(\"DescriptorTable( CBV(b2,space=5,numDescriptors=1)), "
      "DescriptorTable( SRV(t1,space=3,numDescriptors=unbounded)), "
      "DescriptorTable( UAV(u10,space=17,numDescriptors=100)), "
      "DescriptorTable(Sampler(s0, numDescriptors=5))\")]"
      "  float4 main() : SV_Target { return 0; }",
      "ps_6_0", {DFCC_RootSignature}, {});
}

TEST_F(ValidationTest, WhenRootSigMatchShaderFail_Unbounded1) {
  ReplaceContainerPartsCheckMsgs(
      "struct Foo { int a; float4 b; };"
      ""
      "ConstantBuffer<Foo> cb1[]  : register(b2, space5);"
      "Texture2D<float4> tex1[]   : register(t1, space3);"
      "RWBuffer<float4> buf1[]    : register(u33, space17);"
      "SamplerState sampler1[]    : register(s0, space0);"
      ""
      "float4 main(float4 a : AAA) : SV_TARGET"
      "{"
      "  return buf1[a.x][a.y] + cb1[a.x].b + tex1[a.x].Sample(sampler1[a.x], "
      "a.xy);"
      "}",
      "[RootSignature(\"DescriptorTable( CBV(b3,space=5,numDescriptors=1)), "
      "DescriptorTable( SRV(t1,space=3,numDescriptors=unbounded)), "
      "DescriptorTable( UAV(u10,space=17,numDescriptors=unbounded)), "
      "DescriptorTable( Sampler(s0, numDescriptors=5))\")]"
      "  float4 main() : SV_Target { return 0; }",
      "ps_6_0", {DFCC_RootSignature},
      {"Root Signature in DXIL container is not compatible with shader.",
       "Validation failed."});
}

TEST_F(ValidationTest, WhenRootSigMatchShaderFail_Unbounded2) {
  ReplaceContainerPartsCheckMsgs(
      "struct Foo { int a; float4 b; };"
      ""
      "ConstantBuffer<Foo> cb1[]  : register(b2, space5);"
      "Texture2D<float4> tex1[]   : register(t1, space3);"
      "RWBuffer<float4> buf1[]    : register(u33, space17);"
      "SamplerState sampler1[]    : register(s0, space0);"
      ""
      "float4 main(float4 a : AAA) : SV_TARGET"
      "{"
      "  return buf1[a.x][a.y] + cb1[a.x].b + tex1[a.x].Sample(sampler1[a.x], "
      "a.xy);"
      "}",
      "[RootSignature(\"DescriptorTable( CBV(b2,space=5,numDescriptors=1)), "
      "DescriptorTable( SRV(t1,space=3,numDescriptors=unbounded)), "
      "DescriptorTable( UAV(u10,space=17,numDescriptors=unbounded)), "
      "DescriptorTable( Sampler(s5, numDescriptors=unbounded))\")]"
      "  float4 main() : SV_Target { return 0; }",
      "ps_6_0", {DFCC_RootSignature},
      {"Root Signature in DXIL container is not compatible with shader.",
       "Validation failed."});
}

TEST_F(ValidationTest, WhenRootSigMatchShaderFail_Unbounded3) {
  ReplaceContainerPartsCheckMsgs(
      "struct Foo { int a; float4 b; };"
      ""
      "ConstantBuffer<Foo> cb1[]  : register(b2, space5);"
      "Texture2D<float4> tex1[]   : register(t1, space3);"
      "RWBuffer<float4> buf1[]    : register(u33, space17);"
      "SamplerState sampler1[]    : register(s0, space0);"
      ""
      "float4 main(float4 a : AAA) : SV_TARGET"
      "{"
      "  return buf1[a.x][a.y] + cb1[a.x].b + tex1[a.x].Sample(sampler1[a.x], "
      "a.xy);"
      "}",
      "[RootSignature(\"DescriptorTable( CBV(b2,space=5,numDescriptors=1)), "
      "DescriptorTable( SRV(t1,space=3,numDescriptors=unbounded)), "
      "DescriptorTable( UAV(u10,space=17,numDescriptors=7)), "
      "DescriptorTable(Sampler(s0, numDescriptors=5))\")]"
      "  float4 main() : SV_Target { return 0; }",
      "ps_6_0", {DFCC_RootSignature},
      {"Root Signature in DXIL container is not compatible with shader.",
       "Validation failed."});
}

#define VERTEX_STRUCT1                                                         \
  "struct PSSceneIn \n\
    { \n\
      float4 pos  : SV_Position; \n\
      float2 tex  : TEXCOORD0; \n\
      float3 norm : NORMAL; \n\
    }; \n"
#define VERTEX_STRUCT2                                                         \
  "struct PSSceneIn \n\
    { \n\
      float4 pos  : SV_Position; \n\
      float2 tex  : TEXCOORD0; \n\
    }; \n"
#define PC_STRUCT1                                                             \
  "struct HSPerPatchData {  \n\
      float edges[ 3 ] : SV_TessFactor; \n\
      float inside : SV_InsideTessFactor; \n\
      float foo : FOO; \n\
    }; \n"
#define PC_STRUCT2                                                             \
  "struct HSPerPatchData {  \n\
      float edges[ 3 ] : SV_TessFactor; \n\
      float inside : SV_InsideTessFactor; \n\
    }; \n"
#define PC_FUNC                                                                \
  "HSPerPatchData HSPerPatchFunc( InputPatch< PSSceneIn, 3 > points, \n\
      OutputPatch<PSSceneIn, 3> outpoints) { \n\
      HSPerPatchData d = (HSPerPatchData)0; \n\
      d.edges[ 0 ] = points[0].tex.x + outpoints[0].tex.x; \n\
      d.edges[ 1 ] = 1; \n\
      d.edges[ 2 ] = 1; \n\
      d.inside = 1; \n\
      return d; \n\
    } \n"
#define PC_FUNC_NOOUT                                                          \
  "HSPerPatchData HSPerPatchFunc( InputPatch< PSSceneIn, 3 > points ) { \n\
      HSPerPatchData d = (HSPerPatchData)0; \n\
      d.edges[ 0 ] = points[0].tex.x; \n\
      d.edges[ 1 ] = 1; \n\
      d.edges[ 2 ] = 1; \n\
      d.inside = 1; \n\
      return d; \n\
    } \n"
#define PC_FUNC_NOIN                                                           \
  "HSPerPatchData HSPerPatchFunc( OutputPatch<PSSceneIn, 3> outpoints) { \n\
      HSPerPatchData d = (HSPerPatchData)0; \n\
      d.edges[ 0 ] = outpoints[0].tex.x; \n\
      d.edges[ 1 ] = 1; \n\
      d.edges[ 2 ] = 1; \n\
      d.inside = 1; \n\
      return d; \n\
    } \n"
#define HS_ATTR                                                                \
  "[domain(\"tri\")] \n\
    [partitioning(\"fractional_odd\")] \n\
    [outputtopology(\"triangle_cw\")] \n\
    [patchconstantfunc(\"HSPerPatchFunc\")] \n\
    [outputcontrolpoints(3)] \n"
#define HS_FUNC                                                                \
  "PSSceneIn main(const uint id : SV_OutputControlPointID, \n\
                    const InputPatch< PSSceneIn, 3 > points ) { \n\
      return points[ id ]; \n\
    } \n"
#define HS_FUNC_NOOUT                                                          \
  "void main(const uint id : SV_OutputControlPointID, \n\
               const InputPatch< PSSceneIn, 3 > points ) { \n\
    } \n"
#define HS_FUNC_NOIN                                                           \
  "PSSceneIn main( const uint id : SV_OutputControlPointID ) { \n\
      return (PSSceneIn)0; \n\
    } \n"
#define DS_FUNC                                                                \
  "[domain(\"tri\")] PSSceneIn main(const float3 bary : SV_DomainLocation, \n\
                                      const OutputPatch<PSSceneIn, 3> patch, \n\
                                      const HSPerPatchData perPatchData) { \n\
      PSSceneIn v = patch[0]; \n\
      v.pos = patch[0].pos * bary.x; \n\
      v.pos += patch[1].pos * bary.y; \n\
      v.pos += patch[2].pos * bary.z; \n\
      return v; \n\
    } \n"
#define DS_FUNC_NOPC                                                           \
  "[domain(\"tri\")] PSSceneIn main(const float3 bary : SV_DomainLocation, \n\
                                      const OutputPatch<PSSceneIn, 3> patch) { \n\
      PSSceneIn v = patch[0]; \n\
      v.pos = patch[0].pos * bary.x; \n\
      v.pos += patch[1].pos * bary.y; \n\
      v.pos += patch[2].pos * bary.z; \n\
      return v; \n\
    } \n"

TEST_F(ValidationTest, WhenProgramOutSigMissingThenFail) {
  ReplaceContainerPartsCheckMsgs(
      VERTEX_STRUCT1 PC_STRUCT1 PC_FUNC HS_ATTR HS_FUNC,

      VERTEX_STRUCT1 PC_STRUCT1 PC_FUNC_NOOUT HS_ATTR HS_FUNC_NOOUT,

      "hs_6_0",
      {DFCC_InputSignature, DFCC_OutputSignature, DFCC_PatchConstantSignature},
      {"Container part 'Program Output Signature' does not match expected for "
       "module.",
       "Validation failed."});
}

TEST_F(ValidationTest, WhenProgramOutSigUnexpectedThenFail) {
  ReplaceContainerPartsCheckMsgs(
      VERTEX_STRUCT1 PC_STRUCT1 PC_FUNC_NOOUT HS_ATTR HS_FUNC_NOOUT,

      VERTEX_STRUCT1 PC_STRUCT1 PC_FUNC HS_ATTR HS_FUNC,

      "hs_6_0",
      {DFCC_InputSignature, DFCC_OutputSignature, DFCC_PatchConstantSignature},
      {"Container part 'Program Output Signature' does not match expected for "
       "module.",
       "Validation failed."});
}

TEST_F(ValidationTest, WhenProgramSigMismatchThenFail) {
  ReplaceContainerPartsCheckMsgs(
      VERTEX_STRUCT1 PC_STRUCT1 PC_FUNC HS_ATTR HS_FUNC,

      VERTEX_STRUCT2 PC_STRUCT2 PC_FUNC HS_ATTR HS_FUNC,

      "hs_6_0",
      {DFCC_InputSignature, DFCC_OutputSignature, DFCC_PatchConstantSignature},
      {"Container part 'Program Input Signature' does not match expected for "
       "module.",
       "Container part 'Program Output Signature' does not match expected for "
       "module.",
       "Container part 'Program Patch Constant Signature' does not match "
       "expected for module.",
       "Validation failed."});
}

TEST_F(ValidationTest, WhenProgramInSigMissingThenFail) {
  ReplaceContainerPartsCheckMsgs(
      VERTEX_STRUCT1 PC_STRUCT1 PC_FUNC HS_ATTR HS_FUNC,

      // Compiling the HS_FUNC_NOIN produces the following error
      // error: validation errors
      // HS input control point count must be [1..32].  0 specified
      VERTEX_STRUCT1 PC_STRUCT1 PC_FUNC_NOIN HS_ATTR HS_FUNC_NOIN, "hs_6_0",
      {DFCC_InputSignature, DFCC_OutputSignature, DFCC_PatchConstantSignature},
      {"Container part 'Program Input Signature' does not match expected for "
       "module.",
       "Validation failed."});
}

TEST_F(ValidationTest, WhenProgramSigMismatchThenFail2) {
  ReplaceContainerPartsCheckMsgs(
      VERTEX_STRUCT1 PC_STRUCT1 DS_FUNC,

      VERTEX_STRUCT2 PC_STRUCT2 DS_FUNC,

      "ds_6_0",
      {DFCC_InputSignature, DFCC_OutputSignature, DFCC_PatchConstantSignature},
      {"Container part 'Program Input Signature' does not match expected for "
       "module.",
       "Container part 'Program Output Signature' does not match expected for "
       "module.",
       "Container part 'Program Patch Constant Signature' does not match "
       "expected for module.",
       "Validation failed."});
}

TEST_F(ValidationTest, WhenProgramPCSigMissingThenFail) {
  ReplaceContainerPartsCheckMsgs(
      VERTEX_STRUCT1 PC_STRUCT1 DS_FUNC,

      VERTEX_STRUCT2 PC_STRUCT2 DS_FUNC_NOPC,

      "ds_6_0",
      {DFCC_InputSignature, DFCC_OutputSignature, DFCC_PatchConstantSignature},
      {"Container part 'Program Input Signature' does not match expected for "
       "module.",
       "Container part 'Program Output Signature' does not match expected for "
       "module.",
       "Missing part 'Program Patch Constant Signature' required by module.",
       "Validation failed."});
}

#undef VERTEX_STRUCT1
#undef VERTEX_STRUCT2
#undef PC_STRUCT1
#undef PC_STRUCT2
#undef PC_FUNC
#undef PC_FUNC_NOOUT
#undef PC_FUNC_NOIN
#undef HS_ATTR
#undef HS_FUNC
#undef HS_FUNC_NOOUT
#undef HS_FUNC_NOIN
#undef DS_FUNC
#undef DS_FUNC_NOPC

TEST_F(ValidationTest, WhenPSVMismatchThenFail) {
  ReplaceContainerPartsCheckMsgs(
      "float c; [RootSignature ( \"RootConstants(b0, num32BitConstants = 1)\" "
      ")] float4 main() : semantic { return c; }",
      "[RootSignature ( \"\" )] float4 main() : semantic { return 0; }",
      "vs_6_0", {DFCC_PipelineStateValidation},
      {"Container part 'Pipeline State Validation' does not match expected for "
       "module.",
       "Validation failed."});
}

TEST_F(ValidationTest, WhenRDATMismatchThenFail) {
  ReplaceContainerPartsCheckMsgs(
      "export float4 main(float f) : semantic { return f; }",
      "export float4 main() : semantic { return 0; }", "lib_6_3",
      {DFCC_RuntimeData},
      {"Container part 'Runtime Data (RDAT)' does not match expected for "
       "module.",
       "Validation failed."});
}

TEST_F(ValidationTest, WhenFeatureInfoMismatchThenFail) {
  ReplaceContainerPartsCheckMsgs(
      "float4 main(uint2 foo : FOO) : SV_Target { return asdouble(foo.x, "
      "foo.y) * 2.0; }",
      "float4 main() : SV_Target { return 0; }", "ps_6_0", {DFCC_FeatureInfo},
      {"Container part 'Feature Info' does not match expected for module.",
       "Validation failed."});
}

TEST_F(ValidationTest, RayShaderWithSignaturesFail) {
  if (m_ver.SkipDxilVersion(1, 3))
    return;
  RewriteAssemblyCheckMsg(
      "struct Payload { float f; }; struct Attributes { float2 b; };\n"
      "struct Param { float f; };\n"
      "[shader(\"raygeneration\")] void RayGenProto() { return; }\n"
      "[shader(\"intersection\")] void IntersectionProto() { return; }\n"
      "[shader(\"anyhit\")] void AnyHitProto(inout Payload p, in Attributes a) "
      "{ p.f += a.b.x; }\n"
      "[shader(\"closesthit\")] void ClosestHitProto(inout Payload p, in "
      "Attributes a) { p.f += a.b.y; }\n"
      "[shader(\"miss\")] void MissProto(inout Payload p) { p.f += 1.0; }\n"
      "[shader(\"callable\")] void CallableProto(inout Param p) { p.f += 1.0; "
      "}\n"
      "[shader(\"vertex\")] float VSOutOnly() : OUTPUT { return 1; }\n"
      "[shader(\"vertex\")] void VSInOnly(float f : INPUT) : OUTPUT {}\n"
      "[shader(\"vertex\")] float VSInOut(float f : INPUT) : OUTPUT { return "
      "f; }\n",
      "lib_6_3",
      {"!{void \\(\\)\\* @VSInOnly, !\"VSInOnly\", !([0-9]+), null,(.*)!\\1 = ",
       "!{void \\(\\)\\* @VSOutOnly, !\"VSOutOnly\", !([0-9]+), null,(.*)!\\1 "
       "= ",
       "!{void \\(\\)\\* @VSInOut, !\"VSInOut\", !([0-9]+), null,(.*)!\\1 = ",
       "!{void \\(\\)\\* @\"\\\\01\\?RayGenProto@@YAXXZ\", "
       "!\"\\\\01\\?RayGenProto@@YAXXZ\", null, null,",
       "!{void \\(\\)\\* @\"\\\\01\\?IntersectionProto@@YAXXZ\", "
       "!\"\\\\01\\?IntersectionProto@@YAXXZ\", null, null,",
       "!{void \\(%struct.Payload\\*, %struct.Attributes\\*\\)\\* "
       "@\"\\\\01\\?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\\\01\\?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\", null, null,",
       "!{void \\(%struct.Payload\\*, %struct.Attributes\\*\\)\\* "
       "@\"\\\\01\\?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\\\01\\?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\", null, "
       "null,",
       "!{void \\(%struct.Payload\\*\\)\\* "
       "@\"\\\\01\\?MissProto@@YAXUPayload@@@Z\", "
       "!\"\\\\01\\?MissProto@@YAXUPayload@@@Z\", null, null,",
       "!{void \\(%struct.Param\\*\\)\\* "
       "@\"\\\\01\\?CallableProto@@YAXUParam@@@Z\", "
       "!\"\\\\01\\?CallableProto@@YAXUParam@@@Z\", null, null,"},
      {"!{void ()* @VSInOnly, !\"VSInOnly\", !1001, null,\\2!1001 = ",
       "!{void ()* @VSOutOnly, !\"VSOutOnly\", !1002, null,\\2!1002 = ",
       "!{void ()* @VSInOut, !\"VSInOut\", !1003, null,\\2!1003 = ",
       "!{void ()* @\"\\\\01?RayGenProto@@YAXXZ\", "
       "!\"\\\\01?RayGenProto@@YAXXZ\", !1001, null,",
       "!{void ()* @\"\\\\01?IntersectionProto@@YAXXZ\", "
       "!\"\\\\01?IntersectionProto@@YAXXZ\", !1002, null,",
       "!{void (%struct.Payload*, %struct.Attributes*)* "
       "@\"\\\\01?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\\\01?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\", !1003, null,",
       "!{void (%struct.Payload*, %struct.Attributes*)* "
       "@\"\\\\01?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\\\01?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\", !1001, "
       "null,",
       "!{void (%struct.Payload*)* @\"\\\\01?MissProto@@YAXUPayload@@@Z\", "
       "!\"\\\\01?MissProto@@YAXUPayload@@@Z\", !1002, null,",
       "!{void (%struct.Param*)* @\"\\\\01?CallableProto@@YAXUParam@@@Z\", "
       "!\"\\\\01?CallableProto@@YAXUParam@@@Z\", !1003, null,"},
      {"Ray tracing shader '\\\\01\\?RayGenProto@@YAXXZ' should not have any "
       "shader signatures",
       "Ray tracing shader '\\\\01\\?IntersectionProto@@YAXXZ' should not have "
       "any shader signatures",
       "Ray tracing shader "
       "'\\\\01\\?AnyHitProto@@YAXUPayload@@UAttributes@@@Z' should not have "
       "any shader signatures",
       "Ray tracing shader "
       "'\\\\01\\?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z' should not "
       "have any shader signatures",
       "Ray tracing shader '\\\\01\\?MissProto@@YAXUPayload@@@Z' should not "
       "have any shader signatures",
       "Ray tracing shader '\\\\01\\?CallableProto@@YAXUParam@@@Z' should not "
       "have any shader signatures"},
      /*bRegex*/ true);
}

TEST_F(ValidationTest, ViewIDInCSFail) {
  if (m_ver.SkipDxilVersion(1, 1))
    return;
  RewriteAssemblyCheckMsg(
      " \
RWStructuredBuffer<uint> Buf; \
[numthreads(1,1,1)] \
void main(uint id : SV_GroupIndex) \
{ Buf[id] = 0; } \
    ",
      "cs_6_1",
      {"dx.op.flattenedThreadIdInGroup.i32(i32 96",
       "declare i32 @dx.op.flattenedThreadIdInGroup.i32(i32)"},
      {"dx.op.viewID.i32(i32 138", "declare i32 @dx.op.viewID.i32(i32)"},
      "Opcode ViewID not valid in shader model cs_6_1",
      /*bRegex*/ false);
}

TEST_F(ValidationTest, ViewIDIn60Fail) {
  if (m_ver.SkipDxilVersion(1, 1))
    return;
  RewriteAssemblyCheckMsg(
      " \
[domain(\"tri\")] \
float4 main(float3 pos : Position, uint id : SV_PrimitiveID) : SV_Position \
{ return float4(pos, id); } \
    ",
      "ds_6_0",
      {"dx.op.primitiveID.i32(i32 108",
       "declare i32 @dx.op.primitiveID.i32(i32)"},
      {"dx.op.viewID.i32(i32 138", "declare i32 @dx.op.viewID.i32(i32)"},
      "Opcode ViewID not valid in shader model ds_6_0",
      /*bRegex*/ false);
}

TEST_F(ValidationTest, ViewIDNoSpaceFail) {
  if (m_ver.SkipDxilVersion(1, 1))
    return;
  RewriteAssemblyCheckMsg(
      " \
float4 main(uint vid : SV_ViewID, float3 In[31] : INPUT) : SV_Target \
{ return float4(In[vid], 1); } \
    ",
      "ps_6_1",
      {"!{i32 0, !\"INPUT\", i8 9, i8 0, !([0-9]+), i8 2, i32 31",
       "!{i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 "
       "9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 "
       "18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, "
       "i32 27, i32 28, i32 29, i32 30}",
       "?!dx.viewIdState ="},
      {"!{i32 0, !\"INPUT\", i8 9, i8 0, !\\1, i8 2, i32 32",
       "!{i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 "
       "9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 "
       "18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, "
       "i32 27, i32 28, i32 29, i32 30, i32 31}",
       "!1012 ="},
      "Pixel shader input signature lacks available space for ViewID",
      /*bRegex*/ true);
}

// Regression test for a double-delete when failing to parse bitcode.
TEST_F(ValidationTest, WhenDisassembleInvalidBlobThenFail) {
  if (!m_dllSupport.IsEnabled()) {
    VERIFY_SUCCEEDED(m_dllSupport.Initialize());
  }

  CComPtr<IDxcBlobEncoding> pInvalidBitcode;
  Utf8ToBlob(m_dllSupport, "This text is certainly not bitcode",
             &pInvalidBitcode);

  CComPtr<IDxcCompiler> pCompiler;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));

  CComPtr<IDxcBlobEncoding> pDisassembly;
  VERIFY_FAILED(pCompiler->Disassemble(pInvalidBitcode, &pDisassembly));
}

TEST_F(ValidationTest, GSMainMissingAttributeFail) {
  TestCheck(L"..\\CodeGenHLSL\\attributes-gs-no-inout-main.hlsl");
}

TEST_F(ValidationTest, GSOtherMissingAttributeFail) {
  TestCheck(L"..\\CodeGenHLSL\\attributes-gs-no-inout-other.hlsl");
}

TEST_F(ValidationTest, GetAttributeAtVertexInVSFail) {
  if (m_ver.SkipDxilVersion(1, 1))
    return;
  RewriteAssemblyCheckMsg(
      "float4 main(float4 pos: POSITION) : SV_POSITION { return pos.x; }",
      "vs_6_1",
      {"call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)",
       "declare float @dx.op.loadInput.f32(i32, i32, i32, i8, i32)"},
      {"call float @dx.op.attributeAtVertex.f32(i32 137, i32 0, i32 0, i8 0, "
       "i8 0)",
       "declare float @dx.op.attributeAtVertex.f32(i32, i32, i32, i8, i8)"},
      "Opcode AttributeAtVertex not valid in shader model vs_6_1",
      /*bRegex*/ false);
}

TEST_F(ValidationTest, GetAttributeAtVertexIn60Fail) {
  if (m_ver.SkipDxilVersion(1, 1))
    return;
  RewriteAssemblyCheckMsg(
      "float4 main(float4 col : COLOR) : "
      "SV_Target { return EvaluateAttributeCentroid(col).x; }",
      "ps_6_0",
      {"call float @dx.op.evalCentroid.f32(i32 89, i32 0, i32 0, i8 0)",
       "declare float @dx.op.evalCentroid.f32(i32, i32, i32, i8)"},
      {"call float @dx.op.attributeAtVertex.f32(i32 137, i32 0, i32 0, i8 0, "
       "i8 0)",
       "declare float @dx.op.attributeAtVertex.f32(i32, i32, i32, i8, i8)"},
      "Opcode AttributeAtVertex not valid in shader model ps_6_0",
      /*bRegex*/ false);
}

TEST_F(ValidationTest, GetAttributeAtVertexInterpFail) {
  if (m_ver.SkipDxilVersion(1, 1))
    return;
  RewriteAssemblyCheckMsg("float4 main(nointerpolation float4 col : COLOR) : "
                          "SV_Target { return GetAttributeAtVertex(col, 0); }",
                          "ps_6_1", {"!\"COLOR\", i8 9, i8 0, (![0-9]+), i8 1"},
                          {"!\"COLOR\", i8 9, i8 0, \\1, i8 2"},
                          "Attribute COLOR must have nointerpolation mode in "
                          "order to use GetAttributeAtVertex function.",
                          /*bRegex*/ true);
}

TEST_F(ValidationTest, BarycentricMaxIndexFail) {
  if (m_ver.SkipDxilVersion(1, 1))
    return;
  RewriteAssemblyCheckMsg(
      "float4 main(float3 bary : SV_Barycentrics, noperspective float3 bary1 : "
      "SV_Barycentrics1) : SV_Target { return 1; }",
      "ps_6_1",
      {"!([0-9]+) = !{i32 0, !\"SV_Barycentrics\", i8 9, i8 28, !([0-9]+), i8 "
       "2, i32 1, i8 3, i32 -1, i8 -1, null}\n"
       "!([0-9]+) = !{i32 0}"},
      {"!\\1 = !{i32 0, !\"SV_Barycentrics\", i8 9, i8 28, !\\2, i8 2, i32 1, "
       "i8 3, i32 -1, i8 -1, null}\n"
       "!\\3 = !{i32 2}"},
      "SV_Barycentrics semantic index exceeds maximum", /*bRegex*/ true);
}

TEST_F(ValidationTest, BarycentricNoInterpolationFail) {
  if (m_ver.SkipDxilVersion(1, 1))
    return;
  RewriteAssemblyCheckMsg(
      "float4 main(float3 bary : SV_Barycentrics) : "
      "SV_Target { return bary.x * float4(1,0,0,0) + bary.y * float4(0,1,0,0) "
      "+ bary.z * float4(0,0,1,0); }",
      "ps_6_1", {"!\"SV_Barycentrics\", i8 9, i8 28, (![0-9]+), i8 2"},
      {"!\"SV_Barycentrics\", i8 9, i8 28, \\1, i8 1"},
      "SV_Barycentrics cannot be used with 'nointerpolation' type",
      /*bRegex*/ true);
}

TEST_F(ValidationTest, BarycentricFloat4Fail) {
  if (m_ver.SkipDxilVersion(1, 1))
    return;
  RewriteAssemblyCheckMsg(
      "float4 main(float4 col : COLOR) : SV_Target { return col; }", "ps_6_1",
      {"!\"COLOR\", i8 9, i8 0"}, {"!\"SV_Barycentrics\", i8 9, i8 28"},
      "only 'float3' type is allowed for SV_Barycentrics.", false);
}

TEST_F(ValidationTest, BarycentricSamePerspectiveFail) {
  if (m_ver.SkipDxilVersion(1, 1))
    return;
  RewriteAssemblyCheckMsg(
      "float4 main(float3 bary : SV_Barycentrics, noperspective float3 bary1 : "
      "SV_Barycentrics1) : SV_Target { return 1; }",
      "ps_6_1", {"!\"SV_Barycentrics\", i8 9, i8 28, (![0-9]+), i8 4"},
      {"!\"SV_Barycentrics\", i8 9, i8 28, \\1, i8 2"},
      "There can only be up to two input attributes of SV_Barycentrics with "
      "different perspective interpolation mode.",
      true);
}

TEST_F(ValidationTest, Float32DenormModeAttribute) {
  if (m_ver.SkipDxilVersion(1, 2))
    return;
  std::vector<LPCWSTR> pArguments = {L"-denorm", L"ftz"};
  RewriteAssemblyCheckMsg(
      "float4 main(float4 col: COL) : SV_Target { return col; }", "ps_6_2",
      pArguments.data(), 2, nullptr, 0, {"\"fp32-denorm-mode\"=\"ftz\""},
      {"\"fp32-denorm-mode\"=\"invalid_mode\""},
      "contains invalid attribute 'fp32-denorm-mode' with value 'invalid_mode'",
      false);
}

TEST_F(ValidationTest, ResCounter) {
  if (m_ver.SkipDxilVersion(1, 3))
    return;
  RewriteAssemblyCheckMsg(
      "RWStructuredBuffer<float4> buf; export float GetCounter() {return "
      "buf.IncrementCounter();}",
      "lib_6_3",
      {"!\"buf\", i32 -1, i32 -1, i32 1, i32 12, i1 false, i1 true, i1 false, "
       "!"},
      {"!\"buf\", i32 -1, i32 -1, i32 1, i32 12, i1 false, i1 false, i1 false, "
       "!"},
      "BufferUpdateCounter valid only when HasCounter is true", true);
}

TEST_F(ValidationTest, FunctionAttributes) {
  if (m_ver.SkipDxilVersion(1, 2))
    return;
  std::vector<LPCWSTR> pArguments = {L"-denorm", L"ftz"};
  RewriteAssemblyCheckMsg(
      "float4 main(float4 col: COL) : SV_Target { return col; }", "ps_6_2",
      pArguments.data(), 2, nullptr, 0, {"\"fp32-denorm-mode\"=\"ftz\""},
      {"\"dummy_attribute\"=\"invalid_mode\""},
      "contains invalid attribute 'dummy_attribute' with value 'invalid_mode'",
      false);
} // TODO: reject non-zero padding

TEST_F(ValidationTest, LibFunctionResInSig) {
  if (m_ver.SkipDxilVersion(1, 3))
    return;
  RewriteAssemblyCheckMsg(
      "Texture2D<float4> T1;\n"
      "struct ResInStruct { float f; Texture2D<float4> T; };\n"
      "struct ResStructInStruct { float f; ResInStruct S; };\n"
      "ResStructInStruct fnResInReturn(float f) : SV_Target {\n"
      "  ResStructInStruct S1; S1.f = S1.S.f = f; S1.S.T = T1;\n"
      "  return S1; }\n"
      "float fnResInArg(ResStructInStruct S1) : SV_Target {\n"
      "  return S1.f; }\n"
      "struct Data { float f; };\n"
      "float fnStreamInArg(float f, inout PointStream<Data> S1) : SV_Target {\n"
      "  S1.Append((Data)f); return 1.0; }\n",
      "lib_6_x",
      {"!{!\"lib\", i32 6, i32 15}", "!dx.valver = !{!([0-9]+)}",
       "= !{i32 20, !([0-9]+), !([0-9]+), !([0-9]+)}"},
      {"!{!\"lib\", i32 6, i32 3}",
       "!dx.valver = !{!100\\1}\n!1002 = !{i32 1, i32 3}",
       "= !{i32 20, !\\1, !\\2}"},
      {"Function '\\\\01\\?fnResInReturn@@YA\\?AUResStructInStruct@@M@Z' uses "
       "resource in function signature",
       "Function '\\\\01\\?fnResInArg@@YAMUResStructInStruct@@@Z' uses "
       "resource in function signature",
       "Function '\\\\01\\?fnStreamInArg@@YAMMV\\?\\$PointStream@UData@@@@@Z' "
       "uses resource in function signature"
       // TODO: Unable to lower stream append, since it's used in a non-GS
       // function. Should we fail to compile earlier (even on lib_6_x), or add
       // lowering to linker?
       ,
       "Function 'dx\\.hl\\.op\\.\\.void \\(i32, "
       "%\"class\\.PointStream<Data>\"\\*, float\\*\\)' uses resource in "
       "function signature"},
      /*bRegex*/ true);
}

TEST_F(ValidationTest, RayPayloadIsStruct) {
  if (m_ver.SkipDxilVersion(1, 3))
    return;
  RewriteAssemblyCheckMsg(
      "struct Payload { float f; }; struct Attributes { float2 b; };\n"
      "[shader(\"anyhit\")] void AnyHitProto(inout Payload p, in Attributes a) "
      "{ p.f += a.b.x; }\n"
      "export void BadAnyHit(inout float f, in Attributes a) { f += a.b.x; }\n"
      "[shader(\"closesthit\")] void ClosestHitProto(inout Payload p, in "
      "Attributes a) { p.f += a.b.y; }\n"
      "export void BadClosestHit(inout float f, in Attributes a) { f += a.b.y; "
      "}\n"
      "[shader(\"miss\")] void MissProto(inout Payload p) { p.f += 1.0; }\n"
      "export void BadMiss(inout float f) { f += 1.0; }\n",
      "lib_6_3",
      {"!{void (%struct.Payload*, %struct.Attributes*)* "
       "@\"\\01?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\01?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\",",
       "!{void (%struct.Payload*, %struct.Attributes*)* "
       "@\"\\01?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\01?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\",",
       "!{void (%struct.Payload*)* @\"\\01?MissProto@@YAXUPayload@@@Z\", "
       "!\"\\01?MissProto@@YAXUPayload@@@Z\","},
      {"!{void (float*, %struct.Attributes*)* "
       "@\"\\01?BadAnyHit@@YAXAIAMUAttributes@@@Z\", "
       "!\"\\01?BadAnyHit@@YAXAIAMUAttributes@@@Z\",",
       "!{void (float*, %struct.Attributes*)* "
       "@\"\\01?BadClosestHit@@YAXAIAMUAttributes@@@Z\", "
       "!\"\\01?BadClosestHit@@YAXAIAMUAttributes@@@Z\",",
       "!{void (float*)* @\"\\01?BadMiss@@YAXAIAM@Z\", "
       "!\"\\01?BadMiss@@YAXAIAM@Z\","},
      {"Argument 'f' must be a struct type for payload in shader function "
       "'\\01?BadAnyHit@@YAXAIAMUAttributes@@@Z'",
       "Argument 'f' must be a struct type for payload in shader function "
       "'\\01?BadClosestHit@@YAXAIAMUAttributes@@@Z'",
       "Argument 'f' must be a struct type for payload in shader function "
       "'\\01?BadMiss@@YAXAIAM@Z'"},
      false);
}

TEST_F(ValidationTest, RayAttrIsStruct) {
  if (m_ver.SkipDxilVersion(1, 3))
    return;
  RewriteAssemblyCheckMsg(
      "struct Payload { float f; }; struct Attributes { float2 b; };\n"
      "[shader(\"anyhit\")] void AnyHitProto(inout Payload p, in Attributes a) "
      "{ p.f += a.b.x; }\n"
      "export void BadAnyHit(inout Payload p, in float a) { p.f += a; }\n"
      "[shader(\"closesthit\")] void ClosestHitProto(inout Payload p, in "
      "Attributes a) { p.f += a.b.y; }\n"
      "export void BadClosestHit(inout Payload p, in float a) { p.f += a; }\n",
      "lib_6_3",
      {"!{void (%struct.Payload*, %struct.Attributes*)* "
       "@\"\\01?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\01?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\",",
       "!{void (%struct.Payload*, %struct.Attributes*)* "
       "@\"\\01?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\01?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\","},
      {"!{void (%struct.Payload*, float)* "
       "@\"\\01?BadAnyHit@@YAXUPayload@@M@Z\", "
       "!\"\\01?BadAnyHit@@YAXUPayload@@M@Z\",",
       "!{void (%struct.Payload*, float)* "
       "@\"\\01?BadClosestHit@@YAXUPayload@@M@Z\", "
       "!\"\\01?BadClosestHit@@YAXUPayload@@M@Z\","},
      {"Argument 'a' must be a struct type for attributes in shader function "
       "'\\01?BadAnyHit@@YAXUPayload@@M@Z'",
       "Argument 'a' must be a struct type for attributes in shader function "
       "'\\01?BadClosestHit@@YAXUPayload@@M@Z'"},
      false);
}

TEST_F(ValidationTest, CallableParamIsStruct) {
  if (m_ver.SkipDxilVersion(1, 3))
    return;
  RewriteAssemblyCheckMsg(
      "struct Param { float f; };\n"
      "[shader(\"callable\")] void CallableProto(inout Param p) { p.f += 1.0; "
      "}\n"
      "export void BadCallable(inout float f) { f += 1.0; }\n",
      "lib_6_3",
      {"!{void (%struct.Param*)* @\"\\01?CallableProto@@YAXUParam@@@Z\", "
       "!\"\\01?CallableProto@@YAXUParam@@@Z\","},
      {"!{void (float*)* @\"\\01?BadCallable@@YAXAIAM@Z\", "
       "!\"\\01?BadCallable@@YAXAIAM@Z\","},
      {"Argument 'f' must be a struct type for callable shader function "
       "'\\01?BadCallable@@YAXAIAM@Z'"},
      false);
}

TEST_F(ValidationTest, RayShaderExtraArg) {
  if (m_ver.SkipDxilVersion(1, 3))
    return;
  RewriteAssemblyCheckMsg(
      "struct Payload { float f; }; struct Attributes { float2 b; };\n"
      "struct Param { float f; };\n"
      "[shader(\"anyhit\")] void AnyHitProto(inout Payload p, in Attributes a) "
      "{ p.f += a.b.x; }\n"
      "[shader(\"closesthit\")] void ClosestHitProto(inout Payload p, in "
      "Attributes a) { p.f += a.b.y; }\n"
      "[shader(\"miss\")] void MissProto(inout Payload p) { p.f += 1.0; }\n"
      "[shader(\"callable\")] void CallableProto(inout Param p) { p.f += 1.0; "
      "}\n"
      "export void BadAnyHit(inout Payload p, in Attributes a, float f) { p.f "
      "+= f; }\n"
      "export void BadClosestHit(inout Payload p, in Attributes a, float f) { "
      "p.f += f; }\n"
      "export void BadMiss(inout Payload p, float f) { p.f += f; }\n"
      "export void BadCallable(inout Param p, float f) { p.f += f; }\n",
      "lib_6_3",
      {"!{void (%struct.Payload*, %struct.Attributes*)* "
       "@\"\\01?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\01?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\"",
       "!{void (%struct.Payload*, %struct.Attributes*)* "
       "@\"\\01?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\01?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\"",
       "!{void (%struct.Payload*)* @\"\\01?MissProto@@YAXUPayload@@@Z\", "
       "!\"\\01?MissProto@@YAXUPayload@@@Z\"",
       "!{void (%struct.Param*)* @\"\\01?CallableProto@@YAXUParam@@@Z\", "
       "!\"\\01?CallableProto@@YAXUParam@@@Z\""},
      {"!{void (%struct.Payload*, %struct.Attributes*, float)* "
       "@\"\\01?BadAnyHit@@YAXUPayload@@UAttributes@@M@Z\", "
       "!\"\\01?BadAnyHit@@YAXUPayload@@UAttributes@@M@Z\"",
       "!{void (%struct.Payload*, %struct.Attributes*, float)* "
       "@\"\\01?BadClosestHit@@YAXUPayload@@UAttributes@@M@Z\", "
       "!\"\\01?BadClosestHit@@YAXUPayload@@UAttributes@@M@Z\"",
       "!{void (%struct.Payload*, float)* @\"\\01?BadMiss@@YAXUPayload@@M@Z\", "
       "!\"\\01?BadMiss@@YAXUPayload@@M@Z\"",
       "!{void (%struct.Param*, float)* @\"\\01?BadCallable@@YAXUParam@@M@Z\", "
       "!\"\\01?BadCallable@@YAXUParam@@M@Z\""},
      {"Extra argument 'f' not allowed for shader function "
       "'\\01?BadAnyHit@@YAXUPayload@@UAttributes@@M@Z'",
       "Extra argument 'f' not allowed for shader function "
       "'\\01?BadClosestHit@@YAXUPayload@@UAttributes@@M@Z'",
       "Extra argument 'f' not allowed for shader function "
       "'\\01?BadMiss@@YAXUPayload@@M@Z'",
       "Extra argument 'f' not allowed for shader function "
       "'\\01?BadCallable@@YAXUParam@@M@Z'"},
      false);
}

TEST_F(ValidationTest, ResInShaderStruct) {
  if (m_ver.SkipDxilVersion(1, 3))
    return;
  // Verify resource not used in shader argument structure
  RewriteAssemblyCheckMsg(
      "struct ResInStruct { float f; Texture2D<float4> T; };\n"
      "struct ResStructInStruct { float f; ResInStruct S; };\n"
      "struct Payload { float f; }; struct Attributes { float2 b; };\n"
      "[shader(\"anyhit\")] void AnyHitProto(inout Payload p, in Attributes a) "
      "{ p.f += a.b.x; }\n"
      "export void BadAnyHit(inout ResStructInStruct p, in Attributes a) { p.f "
      "+= a.b.x; }\n"
      "[shader(\"closesthit\")] void ClosestHitProto(inout Payload p, in "
      "Attributes a) { p.f += a.b.y; }\n"
      "export void BadClosestHit(inout ResStructInStruct p, in Attributes a) { "
      "p.f += a.b.x; }\n"
      "[shader(\"miss\")] void MissProto(inout Payload p) { p.f += 1.0; }\n"
      "export void BadMiss(inout ResStructInStruct p) { p.f += 1.0; }\n"
      "struct Param { float f; };\n"
      "[shader(\"callable\")] void CallableProto(inout Param p) { p.f += 1.0; "
      "}\n"
      "export void BadCallable(inout ResStructInStruct p) { p.f += 1.0; }\n",
      "lib_6_x",
      {"!{!\"lib\", i32 6, i32 15}", "!dx.valver = !{!([0-9]+)}",
       "= !{i32 20, !([0-9]+), !([0-9]+), !([0-9]+)}",
       "!{void \\(%struct\\.Payload\\*, %struct\\.Attributes\\*\\)\\* "
       "@\"\\\\01\\?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\\\01\\?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\",",
       "!{void \\(%struct\\.Payload\\*, %struct\\.Attributes\\*\\)\\* "
       "@\"\\\\01\\?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\\\01\\?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\",",
       "!{void \\(%struct\\.Payload\\*\\)\\* "
       "@\"\\\\01\\?MissProto@@YAXUPayload@@@Z\", "
       "!\"\\\\01\\?MissProto@@YAXUPayload@@@Z\",",
       "!{void \\(%struct\\.Param\\*\\)\\* "
       "@\"\\\\01\\?CallableProto@@YAXUParam@@@Z\", "
       "!\"\\\\01\\?CallableProto@@YAXUParam@@@Z\","},
      {
          "!{!\"lib\", i32 6, i32 3}",
          "!dx.valver = !{!100\\1}\n!1002 = !{i32 1, i32 3}",
          "= !{i32 20, !\\1, !\\2}",
          "!{void (%struct.ResStructInStruct*, %struct.Attributes*)* "
          "@\"\\\\01?BadAnyHit@@YAXUResStructInStruct@@UAttributes@@@Z\", "
          "!\"\\\\01?BadAnyHit@@YAXUResStructInStruct@@UAttributes@@@Z\",",
          "!{void (%struct.ResStructInStruct*, %struct.Attributes*)* "
          "@\"\\\\01?BadClosestHit@@YAXUResStructInStruct@@UAttributes@@@Z\", "
          "!\"\\\\01?BadClosestHit@@YAXUResStructInStruct@@UAttributes@@@Z\",",
          "!{void (%struct.ResStructInStruct*)* "
          "@\"\\\\01?BadMiss@@YAXUResStructInStruct@@@Z\", "
          "!\"\\\\01?BadMiss@@YAXUResStructInStruct@@@Z\",",
          "!{void (%struct.ResStructInStruct*)* "
          "@\"\\\\01?BadCallable@@YAXUResStructInStruct@@@Z\", "
          "!\"\\\\01?BadCallable@@YAXUResStructInStruct@@@Z\",",
      },
      {"Function '\\\\01\\?BadAnyHit@@YAXUResStructInStruct@@UAttributes@@@Z' "
       "uses resource in function signature",
       "Function "
       "'\\\\01\\?BadClosestHit@@YAXUResStructInStruct@@UAttributes@@@Z' uses "
       "resource in function signature",
       "Function '\\\\01\\?BadMiss@@YAXUResStructInStruct@@@Z' uses resource "
       "in function signature",
       "Function '\\\\01\\?BadCallable@@YAXUResStructInStruct@@@Z' uses "
       "resource in function signature"},
      /*bRegex*/ true);
}

TEST_F(ValidationTest, WhenPayloadSizeTooSmallThenFail) {
  if (m_ver.SkipDxilVersion(1, 3))
    return;
  RewriteAssemblyCheckMsg(
      "struct Payload { float f; }; struct Attributes { float2 b; };\n"
      "struct Param { float f; };\n"
      "[shader(\"raygeneration\")] void RayGenProto() { return; }\n"
      "[shader(\"intersection\")] void IntersectionProto() { return; }\n"
      "[shader(\"anyhit\")] void AnyHitProto(inout Payload p, in Attributes a) "
      "{ p.f += a.b.x; }\n"
      "[shader(\"closesthit\")] void ClosestHitProto(inout Payload p, in "
      "Attributes a) { p.f += a.b.y; }\n"
      "[shader(\"miss\")] void MissProto(inout Payload p) { p.f += 1.0; }\n"
      "[shader(\"callable\")] void CallableProto(inout Param p) { p.f += 1.0; "
      "}\n"
      "\n"
      "struct BadPayload { float2 f; }; struct BadAttributes { float3 b; };\n"
      "struct BadParam { float2 f; };\n"
      "export void BadRayGen() { return; }\n"
      "export void BadIntersection() { return; }\n"
      "export void BadAnyHit(inout BadPayload p, in BadAttributes a) { p.f += "
      "a.b.x; }\n"
      "export void BadClosestHit(inout BadPayload p, in BadAttributes a) { p.f "
      "+= a.b.y; }\n"
      "export void BadMiss(inout BadPayload p) { p.f += 1.0; }\n"
      "export void BadCallable(inout BadParam p) { p.f += 1.0; }\n",
      "lib_6_3",
      {"!{void ()* @\"\\01?RayGenProto@@YAXXZ\", !\"\\01?RayGenProto@@YAXXZ\",",
       "!{void ()* @\"\\01?IntersectionProto@@YAXXZ\", "
       "!\"\\01?IntersectionProto@@YAXXZ\",",
       "!{void (%struct.Payload*, %struct.Attributes*)* "
       "@\"\\01?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\01?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\",",
       "!{void (%struct.Payload*, %struct.Attributes*)* "
       "@\"\\01?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\01?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\",",
       "!{void (%struct.Payload*)* @\"\\01?MissProto@@YAXUPayload@@@Z\", "
       "!\"\\01?MissProto@@YAXUPayload@@@Z\",",
       "!{void (%struct.Param*)* @\"\\01?CallableProto@@YAXUParam@@@Z\", "
       "!\"\\01?CallableProto@@YAXUParam@@@Z\","},
      {"!{void ()* @\"\\01?BadRayGen@@YAXXZ\", !\"\\01?BadRayGen@@YAXXZ\",",
       "!{void ()* @\"\\01?BadIntersection@@YAXXZ\", "
       "!\"\\01?BadIntersection@@YAXXZ\",",
       "!{void (%struct.BadPayload*, %struct.BadAttributes*)* "
       "@\"\\01?BadAnyHit@@YAXUBadPayload@@UBadAttributes@@@Z\", "
       "!\"\\01?BadAnyHit@@YAXUBadPayload@@UBadAttributes@@@Z\",",
       "!{void (%struct.BadPayload*, %struct.BadAttributes*)* "
       "@\"\\01?BadClosestHit@@YAXUBadPayload@@UBadAttributes@@@Z\", "
       "!\"\\01?BadClosestHit@@YAXUBadPayload@@UBadAttributes@@@Z\",",
       "!{void (%struct.BadPayload*)* @\"\\01?BadMiss@@YAXUBadPayload@@@Z\", "
       "!\"\\01?BadMiss@@YAXUBadPayload@@@Z\",",
       "!{void (%struct.BadParam*)* @\"\\01?BadCallable@@YAXUBadParam@@@Z\", "
       "!\"\\01?BadCallable@@YAXUBadParam@@@Z\","},
      {"For shader '\\01?BadAnyHit@@YAXUBadPayload@@UBadAttributes@@@Z', "
       "payload size is smaller than argument's allocation size",
       "For shader '\\01?BadAnyHit@@YAXUBadPayload@@UBadAttributes@@@Z', "
       "attribute size is smaller than argument's allocation size",
       "For shader '\\01?BadClosestHit@@YAXUBadPayload@@UBadAttributes@@@Z', "
       "payload size is smaller than argument's allocation size",
       "For shader '\\01?BadClosestHit@@YAXUBadPayload@@UBadAttributes@@@Z', "
       "attribute size is smaller than argument's allocation size",
       "For shader '\\01?BadMiss@@YAXUBadPayload@@@Z', payload size is smaller "
       "than argument's allocation size",
       "For shader '\\01?BadCallable@@YAXUBadParam@@@Z', params size is "
       "smaller than argument's allocation size"},
      false);
}

TEST_F(ValidationTest, WhenMissingPayloadThenFail) {
  if (m_ver.SkipDxilVersion(1, 3))
    return;
  RewriteAssemblyCheckMsg(
      "struct Payload { float f; }; struct Attributes { float2 b; };\n"
      "struct Param { float f; };\n"
      "[shader(\"anyhit\")] void AnyHitProto(inout Payload p, in Attributes a) "
      "{ p.f += a.b.x; }\n"
      "[shader(\"closesthit\")] void ClosestHitProto(inout Payload p, in "
      "Attributes a) { p.f += a.b.y; }\n"
      "[shader(\"miss\")] void MissProto(inout Payload p) { p.f += 1.0; }\n"
      "[shader(\"callable\")] void CallableProto(inout Param p) { p.f += 1.0; "
      "}\n"
      "export void BadAnyHit(inout Payload p) { p.f += 1.0; }\n"
      "export void BadClosestHit() {}\n"
      "export void BadMiss() {}\n"
      "export void BadCallable() {}\n",
      "lib_6_3",
      {"!{void (%struct.Payload*, %struct.Attributes*)* "
       "@\"\\01?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\01?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\",",
       "!{void (%struct.Payload*, %struct.Attributes*)* "
       "@\"\\01?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\01?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\",",
       "!{void (%struct.Payload*)* @\"\\01?MissProto@@YAXUPayload@@@Z\", "
       "!\"\\01?MissProto@@YAXUPayload@@@Z\",",
       "!{void (%struct.Param*)* @\"\\01?CallableProto@@YAXUParam@@@Z\", "
       "!\"\\01?CallableProto@@YAXUParam@@@Z\","},
      {"!{void (%struct.Payload*)* @\"\\01?BadAnyHit@@YAXUPayload@@@Z\", "
       "!\"\\01?BadAnyHit@@YAXUPayload@@@Z\",",
       "!{void ()* @\"\\01?BadClosestHit@@YAXXZ\", "
       "!\"\\01?BadClosestHit@@YAXXZ\",",
       "!{void ()* @\"\\01?BadMiss@@YAXXZ\", !\"\\01?BadMiss@@YAXXZ\",",
       "!{void ()* @\"\\01?BadCallable@@YAXXZ\", "
       "!\"\\01?BadCallable@@YAXXZ\","},
      {"anyhit shader '\\01?BadAnyHit@@YAXUPayload@@@Z' missing required "
       "attributes parameter",
       "closesthit shader '\\01?BadClosestHit@@YAXXZ' missing required payload "
       "parameter",
       "closesthit shader '\\01?BadClosestHit@@YAXXZ' missing required "
       "attributes parameter",
       "miss shader '\\01?BadMiss@@YAXXZ' missing required payload parameter",
       "callable shader '\\01?BadCallable@@YAXXZ' missing required params "
       "parameter"},
      false);
}

TEST_F(ValidationTest, ShaderFunctionReturnTypeVoid) {
  if (m_ver.SkipDxilVersion(1, 3))
    return;
  // Verify resource not used in shader argument structure
  RewriteAssemblyCheckMsg(
      "struct Payload { float f; }; struct Attributes { float2 b; };\n"
      "struct Param { float f; };\n"
      "[shader(\"raygeneration\")] void RayGenProto() { return; }\n"
      "[shader(\"anyhit\")] void AnyHitProto(inout Payload p, in Attributes a) "
      "{ p.f += a.b.x; }\n"
      "[shader(\"closesthit\")] void ClosestHitProto(inout Payload p, in "
      "Attributes a) { p.f += a.b.y; }\n"
      "[shader(\"miss\")] void MissProto(inout Payload p) { p.f += 1.0; }\n"
      "[shader(\"callable\")] void CallableProto(inout Param p) { p.f += 1.0; "
      "}\n"
      "export float BadRayGen() { return 1; }\n"
      "export float BadAnyHit(inout Payload p, in Attributes a) { return p.f; "
      "}\n"
      "export float BadClosestHit(inout Payload p, in Attributes a) { return "
      "p.f; }\n"
      "export float BadMiss(inout Payload p) { return p.f; }\n"
      "export float BadCallable(inout Param p) { return p.f; }\n",
      "lib_6_3",
      {"!{void ()* @\"\\01?RayGenProto@@YAXXZ\", "
       "!\"\\01?RayGenProto@@YAXXZ\",",
       "!{void (%struct.Payload*, %struct.Attributes*)* "
       "@\"\\01?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\01?AnyHitProto@@YAXUPayload@@UAttributes@@@Z\",",
       "!{void (%struct.Payload*, %struct.Attributes*)* "
       "@\"\\01?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\", "
       "!\"\\01?ClosestHitProto@@YAXUPayload@@UAttributes@@@Z\",",
       "!{void (%struct.Payload*)* @\"\\01?MissProto@@YAXUPayload@@@Z\", "
       "!\"\\01?MissProto@@YAXUPayload@@@Z\",",
       "!{void (%struct.Param*)* @\"\\01?CallableProto@@YAXUParam@@@Z\", "
       "!\"\\01?CallableProto@@YAXUParam@@@Z\","},
      {"!{float ()* @\"\\01?BadRayGen@@YAMXZ\", "
       "!\"\\01?BadRayGen@@YAMXZ\",",
       "!{float (%struct.Payload*, %struct.Attributes*)* "
       "@\"\\01?BadAnyHit@@YAMUPayload@@UAttributes@@@Z\", "
       "!\"\\01?BadAnyHit@@YAMUPayload@@UAttributes@@@Z\",",
       "!{float (%struct.Payload*, %struct.Attributes*)* "
       "@\"\\01?BadClosestHit@@YAMUPayload@@UAttributes@@@Z\", "
       "!\"\\01?BadClosestHit@@YAMUPayload@@UAttributes@@@Z\",",
       "!{float (%struct.Payload*)* @\"\\01?BadMiss@@YAMUPayload@@@Z\", "
       "!\"\\01?BadMiss@@YAMUPayload@@@Z\",",
       "!{float (%struct.Param*)* @\"\\01?BadCallable@@YAMUParam@@@Z\", "
       "!\"\\01?BadCallable@@YAMUParam@@@Z\","},
      {"Shader function '\\01?BadRayGen@@YAMXZ' must have void return type",
       "Shader function '\\01?BadAnyHit@@YAMUPayload@@UAttributes@@@Z' must "
       "have void return type",
       "Shader function '\\01?BadClosestHit@@YAMUPayload@@UAttributes@@@Z' "
       "must have void return type",
       "Shader function '\\01?BadMiss@@YAMUPayload@@@Z' must have void return "
       "type",
       "Shader function '\\01?BadCallable@@YAMUParam@@@Z' must have void "
       "return type"},
      false);
}

TEST_F(ValidationTest, MeshMultipleSetMeshOutputCounts) {
  TestCheck(L"..\\CodeGenHLSL\\mesh-val\\multipleSetMeshOutputCounts.hlsl");
}

TEST_F(ValidationTest, MeshMissingSetMeshOutputCounts) {
  TestCheck(L"..\\CodeGenHLSL\\mesh-val\\missingSetMeshOutputCounts.hlsl");
}

TEST_F(ValidationTest, MeshNonDominatingSetMeshOutputCounts) {
  TestCheck(
      L"..\\CodeGenHLSL\\mesh-val\\nonDominatingSetMeshOutputCounts.hlsl");
}

TEST_F(ValidationTest, MeshOversizePayload) {
  TestCheck(L"..\\CodeGenHLSL\\mesh-val\\msOversizePayload.hlsl");
}

TEST_F(ValidationTest, MeshOversizeOutput) {
  TestCheck(L"..\\CodeGenHLSL\\mesh-val\\msOversizeOutput.hlsl");
}

TEST_F(ValidationTest, MeshOversizePayloadOutput) {
  TestCheck(L"..\\CodeGenHLSL\\mesh-val\\msOversizePayloadOutput.hlsl");
}

TEST_F(ValidationTest, MeshMultipleGetMeshPayload) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\mesh-val\\mesh.hlsl", "ms_6_5",
      "%([0-9]+) = call %struct.MeshPayload\\* "
      "@dx.op.getMeshPayload.struct.MeshPayload\\(i32 170\\)  ; "
      "GetMeshPayload\\(\\)",
      "%\\1 = call %struct.MeshPayload* "
      "@dx.op.getMeshPayload.struct.MeshPayload(i32 170)  ; GetMeshPayload()\n"
      "  %.extra.unused.payload. = call %struct.MeshPayload* "
      "@dx.op.getMeshPayload.struct.MeshPayload(i32 170)  ; GetMeshPayload()",
      "GetMeshPayload cannot be called multiple times.", true);
}

TEST_F(ValidationTest, MeshOutofRangeMaxVertexCount) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\mesh-val\\mesh.hlsl", "ms_6_5",
      "= !{!([0-9]+), i32 32, i32 16, i32 2, i32 40}",
      "= !{!\\1, i32 257, i32 16, i32 2, i32 40}",
      "MS max vertex output count must be \\[0..256\\].  257 specified", true);
}

TEST_F(ValidationTest, MeshOutofRangeMaxPrimitiveCount) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\mesh-val\\mesh.hlsl", "ms_6_5",
      "= !{!([0-9]+), i32 32, i32 16, i32 2, i32 40}",
      "= !{!\\1, i32 32, i32 257, i32 2, i32 40}",
      "MS max primitive output count must be \\[0..256\\].  257 specified",
      true);
}

TEST_F(ValidationTest, MeshLessThanMinX) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\mesh-val\\mesh.hlsl", "ms_6_5",
      "= !{i32 32, i32 1, i32 1}", "= !{i32 0, i32 1, i32 1}",
      "Declared Thread Group X size 0 outside valid range [1..128]");
}

TEST_F(ValidationTest, MeshGreaterThanMaxX) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\mesh-val\\mesh.hlsl", "ms_6_5",
      "= !{i32 32, i32 1, i32 1}", "= !{i32 129, i32 1, i32 1}",
      "Declared Thread Group X size 129 outside valid range [1..128]");
}

TEST_F(ValidationTest, MeshLessThanMinY) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\mesh-val\\mesh.hlsl", "ms_6_5",
      "= !{i32 32, i32 1, i32 1}", "= !{i32 32, i32 0, i32 1}",
      "Declared Thread Group Y size 0 outside valid range [1..128]");
}

TEST_F(ValidationTest, MeshGreaterThanMaxY) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\mesh-val\\mesh.hlsl", "ms_6_5",
      "= !{i32 32, i32 1, i32 1}", "= !{i32 1, i32 129, i32 1}",
      "Declared Thread Group Y size 129 outside valid range [1..128]");
}

TEST_F(ValidationTest, MeshLessThanMinZ) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\mesh-val\\mesh.hlsl", "ms_6_5",
      "= !{i32 32, i32 1, i32 1}", "= !{i32 32, i32 1, i32 0}",
      "Declared Thread Group Z size 0 outside valid range [1..128]");
}

TEST_F(ValidationTest, MeshGreaterThanMaxZ) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\mesh-val\\mesh.hlsl", "ms_6_5",
      "= !{i32 32, i32 1, i32 1}", "= !{i32 1, i32 1, i32 129}",
      "Declared Thread Group Z size 129 outside valid range [1..128]");
}

TEST_F(ValidationTest, MeshGreaterThanMaxXYZ) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\mesh-val\\mesh.hlsl", "ms_6_5",
                          "= !{i32 32, i32 1, i32 1}",
                          "= !{i32 32, i32 2, i32 4}",
                          "Declared Thread Group Count 256 (X*Y*Z) is beyond "
                          "the valid maximum of 128");
}

TEST_F(ValidationTest, MeshGreaterThanMaxVSigRowCount) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\mesh-val\\mesh.hlsl", "ms_6_5",
                          "!([0-9]+) = !{i32 1, !\"COLOR\", i8 9, i8 0, "
                          "!([0-9]+), i8 2, i32 4, i8 1, i32 1, i8 0, (.*)"
                          "!\\2 = !{i32 0, i32 1, i32 2, i32 3}",
                          "!\\1 = !{i32 1, !\"COLOR\", i8 9, i8 0, !\\2, i8 2, "
                          "i32 32, i8 1, i32 1, i8 0, \\3"
                          "!\\2 = !{i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, "
                          "i32 6, i32 7, i32 8, i32 9, i32 10,"
                          "i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 "
                          "17, i32 18, i32 19, i32 20,"
                          "i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 "
                          "27, i32 28, i32 29, i32 30, i32 31}",
                          "For shader 'main', vertex output signatures are "
                          "taking up more than 32 rows",
                          true);
}

TEST_F(ValidationTest, MeshGreaterThanMaxPSigRowCount) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\mesh-val\\mesh.hlsl", "ms_6_5",
                          "!([0-9]+) = !{i32 4, !\"LAYER\", i8 4, i8 0, "
                          "!([0-9]+), i8 1, i32 6, i8 1, i32 1, i8 0, (.*)"
                          "!\\2 = !{i32 0, i32 1, i32 2, i32 3, i32 4, i32 5}",
                          "!\\1 = !{i32 4, !\"LAYER\", i8 4, i8 0, !\\2, i8 1, "
                          "i32 32, i8 1, i32 1, i8 0, \\3"
                          "!\\2 = !{i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, "
                          "i32 6, i32 7, i32 8, i32 9, i32 10,"
                          "i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 "
                          "17, i32 18, i32 19, i32 20,"
                          "i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 "
                          "27, i32 28, i32 29, i32 30, i32 31}",
                          "For shader 'main', primitive output signatures are "
                          "taking up more than 32 rows",
                          true);
}

TEST_F(ValidationTest, MeshGreaterThanMaxTotalSigRowCount) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\mesh-val\\mesh.hlsl", "ms_6_5",
      {"!([0-9]+) = !{i32 1, !\"COLOR\", i8 9, i8 0, !([0-9]+), i8 2, i32 4, "
       "i8 1, i32 1, i8 0, (.*)"
       "!\\2 = !{i32 0, i32 1, i32 2, i32 3}",
       "!([0-9]+) = !{i32 4, !\"LAYER\", i8 4, i8 0, !([0-9]+), i8 1, i32 6, "
       "i8 1, i32 1, i8 0, (.*)"
       "!\\2 = !{i32 0, i32 1, i32 2, i32 3, i32 4, i32 5}"},
      {
          "!\\1 = !{i32 1, !\"COLOR\", i8 9, i8 0, !\\2, i8 2, i32 16, i8 1, "
          "i32 1, i8 0, \\3"
          "!\\2 = !{i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, "
          "i32 8, i32 9, i32 10,"
          "i32 11, i32 12, i32 13, i32 14, i32 15}",
          "!\\1 = !{i32 4, !\"LAYER\", i8 4, i8 0, !\\2, i8 1, i32 16, i8 1, "
          "i32 1, i8 0, \\3"
          "!\\2 = !{i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, "
          "i32 8, i32 9, i32 10,"
          "i32 11, i32 12, i32 13, i32 14, i32 15}",
      },
      "For shader 'main', vertex and primitive output signatures are taking up "
      "more than 32 rows",
      true);
}

TEST_F(ValidationTest, MeshOversizeSM) {
  TestCheck(L"..\\CodeGenHLSL\\mesh-val\\oversizeSM.hlsl");
}

TEST_F(ValidationTest, AmplificationMultipleDispatchMesh) {
  TestCheck(L"..\\CodeGenHLSL\\mesh-val\\multipleDispatchMesh.hlsl");
}

TEST_F(ValidationTest, AmplificationMissingDispatchMesh) {
  TestCheck(L"..\\CodeGenHLSL\\mesh-val\\missingDispatchMesh.hlsl");
}

TEST_F(ValidationTest, AmplificationNonDominatingDispatchMesh) {
  TestCheck(L"..\\CodeGenHLSL\\mesh-val\\nonDominatingDispatchMesh.hlsl");
}

TEST_F(ValidationTest, AmplificationOversizePayload) {
  TestCheck(L"..\\CodeGenHLSL\\mesh-val\\asOversizePayload.hlsl");
}

TEST_F(ValidationTest, AmplificationLessThanMinX) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\mesh-val\\amplification.hlsl", "as_6_5",
      "= !{i32 32, i32 1, i32 1}", "= !{i32 0, i32 1, i32 1}",
      "Declared Thread Group X size 0 outside valid range [1..128]");
}

TEST_F(ValidationTest, AmplificationGreaterThanMaxX) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\mesh-val\\amplification.hlsl", "as_6_5",
      "= !{i32 32, i32 1, i32 1}", "= !{i32 129, i32 1, i32 1}",
      "Declared Thread Group X size 129 outside valid range [1..128]");
}

TEST_F(ValidationTest, AmplificationLessThanMinY) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\mesh-val\\amplification.hlsl", "as_6_5",
      "= !{i32 32, i32 1, i32 1}", "= !{i32 32, i32 0, i32 1}",
      "Declared Thread Group Y size 0 outside valid range [1..128]");
}

TEST_F(ValidationTest, AmplificationGreaterThanMaxY) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\mesh-val\\amplification.hlsl", "as_6_5",
      "= !{i32 32, i32 1, i32 1}", "= !{i32 1, i32 129, i32 1}",
      "Declared Thread Group Y size 129 outside valid range [1..128]");
}

TEST_F(ValidationTest, AmplificationLessThanMinZ) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\mesh-val\\amplification.hlsl", "as_6_5",
      "= !{i32 32, i32 1, i32 1}", "= !{i32 32, i32 1, i32 0}",
      "Declared Thread Group Z size 0 outside valid range [1..128]");
}

TEST_F(ValidationTest, AmplificationGreaterThanMaxZ) {
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\mesh-val\\amplification.hlsl", "as_6_5",
      "= !{i32 32, i32 1, i32 1}", "= !{i32 1, i32 1, i32 129}",
      "Declared Thread Group Z size 129 outside valid range [1..128]");
}

TEST_F(ValidationTest, AmplificationGreaterThanMaxXYZ) {
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\mesh-val\\amplification.hlsl",
                          "as_6_5", "= !{i32 32, i32 1, i32 1}",
                          "= !{i32 32, i32 2, i32 4}",
                          "Declared Thread Group Count 256 (X*Y*Z) is beyond "
                          "the valid maximum of 128");
}

TEST_F(ValidationTest, ValidateRootSigContainer) {
  // Validation of root signature-only container not supported until 1.5
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  LPCSTR pSource = "#define main \"DescriptorTable(UAV(u0))\"";
  CComPtr<IDxcBlob> pObject;
  if (!CompileSource(pSource, "rootsig_1_0", &pObject))
    return;
  CheckValidationMsgs(pObject, {}, false,
                      DxcValidatorFlags_RootSignatureOnly |
                          DxcValidatorFlags_InPlaceEdit);
  pObject.Release();
  if (!CompileSource(pSource, "rootsig_1_1", &pObject))
    return;
  CheckValidationMsgs(pObject, {}, false,
                      DxcValidatorFlags_RootSignatureOnly |
                          DxcValidatorFlags_InPlaceEdit);
}

TEST_F(ValidationTest, ValidatePrintfNotAllowed) {
  TestCheck(L"..\\CodeGenHLSL\\printf.hlsl");
}

TEST_F(ValidationTest, ValidateWithHash) {
  if (m_ver.SkipDxilVersion(1, ShaderModel::kHighestReleasedMinor))
    return;
  CComPtr<IDxcBlob> pProgram;
  CompileSource("float4 main(float a:A, float b:B) : SV_Target { return 1; }",
                "ps_6_0", &pProgram);

  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pResult;
  unsigned Flags = 0;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
  // With hash.
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pResult));
  // Make sure the validation was successful.
  HRESULT status;
  VERIFY_IS_NOT_NULL(pResult);
  CComPtr<IDxcBlob> pValidationOutput;
  pResult->GetStatus(&status);
  VERIFY_SUCCEEDED(status);
  pResult->GetResult(&pValidationOutput);
  // Make sure the validation output is not null when hashing.
  VERIFY_SUCCEEDED(pValidationOutput != nullptr);

  hlsl::DxilContainerHeader *pHeader =
      (hlsl::DxilContainerHeader *)pProgram->GetBufferPointer();
  // Validate the hash.
  constexpr uint32_t HashStartOffset =
      offsetof(struct DxilContainerHeader, Version);
  auto *DataToHash = (const BYTE *)pHeader + HashStartOffset;
  UINT AmountToHash = pHeader->ContainerSizeInBytes - HashStartOffset;
  BYTE Result[DxilContainerHashSize];
  ComputeHashRetail(DataToHash, AmountToHash, Result);
  VERIFY_ARE_EQUAL(memcmp(Result, pHeader->Hash.Digest, sizeof(Result)), 0);
}

TEST_F(ValidationTest, ValidatePreviewBypassHash) {
  if (m_ver.SkipDxilVersion(1, ShaderModel::kHighestMinor))
    return;
  // If there is no available pre-release version to test, return
  if (DXIL::CompareVersions(ShaderModel::kHighestMajor,
                            ShaderModel::kHighestMinor,
                            ShaderModel::kHighestReleasedMajor,
                            ShaderModel::kHighestReleasedMinor) <= 0) {
    return;
  }

  // Now test a pre-release version.
  CComPtr<IDxcBlob> pProgram;
  LPCSTR pSource =
      R"(float4 main(float a:A, float b:B) : SV_Target { return 1; })";

  CComPtr<IDxcBlobEncoding> pSourceBlob;
  Utf8ToBlob(m_dllSupport, pSource, &pSourceBlob);

  LPCSTR pShaderModel =
      ShaderModel::Get(ShaderModel::Kind::Pixel, ShaderModel::kHighestMajor,
                       ShaderModel::kHighestMinor)
          ->GetName();

  bool result = CompileSource(pSourceBlob, pShaderModel, nullptr, 0, nullptr, 0,
                              &pProgram);
  VERIFY_IS_TRUE(result);

  hlsl::DxilContainerHeader *pHeader =
      (hlsl::DxilContainerHeader *)pProgram->GetBufferPointer();

  // Should be equal, this proves the hash is set to the preview bypass hash
  // when a prerelease version is used
  VERIFY_ARE_EQUAL(memcmp(&hlsl::PreviewByPassHash, pHeader->Hash.Digest,
                          sizeof(hlsl::PreviewByPassHash)),
                   0);
}

TEST_F(ValidationTest, ValidateProgramVersionAgainstDxilModule) {
  if (m_ver.SkipDxilVersion(1, 8))
    return;

  CComPtr<IDxcBlob> pProgram;
  LPCSTR pSource =
      R"(float4 main(float a:A, float b:B) : SV_Target { return 1; })";

  CComPtr<IDxcBlobEncoding> pSourceBlob;
  Utf8ToBlob(m_dllSupport, pSource, &pSourceBlob);

  LPCSTR pShaderModel =
      ShaderModel::Get(ShaderModel::Kind::Pixel, 6, 0)->GetName();

  bool result = CompileSource(pSourceBlob, pShaderModel, nullptr, 0, nullptr, 0,
                              &pProgram);
  VERIFY_IS_TRUE(result);

  hlsl::DxilContainerHeader *pHeader =
      (hlsl::DxilContainerHeader *)pProgram->GetBufferPointer();
  // test that when the program version differs from the dxil module shader
  // model version, the validator fails
  DxilPartHeader *pPart = GetDxilPartByType(pHeader, DxilFourCC::DFCC_DXIL);

  DxilProgramHeader *pMutableProgramHeader =
      reinterpret_cast<DxilProgramHeader *>(GetDxilPartData(pPart));
  int oldMajor = 0;
  int oldMinor = 0;
  int newMajor = 0;
  int newMinor = 0;
  VERIFY_IS_NOT_NULL(pMutableProgramHeader);
  uint32_t &PV = pMutableProgramHeader->ProgramVersion;
  oldMajor = (PV >> 4) & 0xF; // Extract the major version (next 4 bits)
  oldMinor = PV & 0xF;        // Extract the minor version (lowest 4 bits)

  // Add one to the last bit of the program version, which is 0, because
  // the program version (shader model version) is 6.0, and we want to
  // test that the validation fails when the program version is changed to 6.1
  PV += 1;

  newMajor = (PV >> 4) & 0xF; // Extract the major version (next 4 bits)
  newMinor = PV & 0xF;        // Extract the new minor version (lowest 4 bits)

  // now test that the validation fails
  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pResult;
  unsigned Flags = 0;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));

  HRESULT status;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pResult));
  VERIFY_IS_NOT_NULL(pResult);
  pResult->GetStatus(&status);

  // expect validation to fail
  VERIFY_FAILED(status);
  // validation succeeded prior, so by inference we know that oldMajor /
  // oldMinor were the old dxil module shader model versions
  char buffer[100];
  std::snprintf(buffer, sizeof(buffer),
                "error: Program Version is %d.%d but Dxil Module shader model "
                "version is %d.%d.\nValidation failed.\n",
                newMajor, newMinor, oldMajor, oldMinor);
  std::string formattedString = buffer;

  CheckOperationResultMsgs(pResult, {buffer}, false, false);
}

TEST_F(ValidationTest, ValidateVersionNotAllowed) {
  if (m_ver.SkipDxilVersion(1, 6))
    return;
  // When validator version is < dxil verrsion, compiler has a newer version
  // than validator.  We are checking the validator, so only use the validator
  // version.
  // This will also assume that the versions are tied together.  This has always
  // been the case, but it's not assumed that it has to be the case.  If the
  // versions diverged, it would be impossible to tell what DXIL version a
  // validator supports just from the version returned in the IDxcVersion
  // interface, without separate knowledge of the supported dxil version based
  // on the validator version.  If these versions must diverge in the future, we
  // could rev the IDxcVersion interface to accomodate.
  std::string maxMinor = std::to_string(m_ver.m_ValMinor);
  std::string higherMinor = std::to_string(m_ver.m_ValMinor + 1);
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\basic.hlsl", "ps_6_0",
                          ("= !{i32 1, i32 " + maxMinor + "}").c_str(),
                          ("= !{i32 1, i32 " + higherMinor + "}").c_str(),
                          ("error: Validator version in metadata (1." +
                           higherMinor + ") is not supported; maximum: (1." +
                           maxMinor + ")")
                              .c_str());
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\basic.hlsl", "ps_6_0",
                          "= !{i32 1, i32 0}", "= !{i32 1, i32 1}",
                          "error: Shader model requires Dxil Version 1.0");
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\basic.hlsl", "ps_6_0",
                          "= !{i32 1, i32 0}",
                          ("= !{i32 1, i32 " + higherMinor + "}").c_str(),
                          ("error: Dxil version in metadata (1." + higherMinor +
                           ") is not supported; maximum: (1." + maxMinor + ")")
                              .c_str());
}

TEST_F(ValidationTest, CreateHandleNotAllowedSM66) {
  if (m_ver.SkipDxilVersion(1, 6))
    return;
  RewriteAssemblyCheckMsg(L"..\\CodeGenHLSL\\basic.hlsl", "ps_6_5",
                          {"= !{i32 1, i32 5}", "= !{!\"ps\", i32 6, i32 5}"},
                          {"= !{i32 1, i32 6}", "= !{!\"ps\", i32 6, i32 6}"},
                          "opcode 'CreateHandle' should only be used in "
                          "'Shader model 6.5 and below'");
  RewriteAssemblyCheckMsg(
      L"..\\CodeGenHLSL\\basic.hlsl", "lib_6_5",
      {"call %dx.types.Handle "
       "@\"dx.op.createHandleForLib.class.Buffer<vector<float, 4> >\"\\(i32 "
       "160, %\"class.Buffer<vector<float, 4> >\" %[0-9]+\\)",
       "declare %dx.types.Handle "
       "@\"dx.op.createHandleForLib.class.Buffer<vector<float, 4> >\"\\(i32, "
       "%\"class.Buffer<vector<float, 4> >\"\\) #1"},
      {"call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 0, "
       "i1 false)",
       "declare %dx.types.Handle @dx.op.createHandle(i32, i8, i32, i32, i1) "
       "#1"},
      "opcode 'CreateHandle' should only be used in 'non-library targets'",
      true);
}

// Check for validation errors for various const dests to atomics
TEST_F(ValidationTest, AtomicsConsts) {
  if (m_ver.SkipDxilVersion(1, 7))
    return;
  std::vector<LPCWSTR> pArguments = {L"-HV", L"2021", L"-Zi"};

  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\atomics.hlsl", "cs_6_0",
                          pArguments.data(), 3, nullptr, 0,
                          {"call i32 @dx.op.atomicBinOp.i32(i32 78, "
                           "%dx.types.Handle %rw_structbuf_UAV_structbuf"},
                          {"call i32 @dx.op.atomicBinOp.i32(i32 78, "
                           "%dx.types.Handle %ro_structbuf_texture_structbuf"},
                          "Non-UAV destination to atomic intrinsic.", false);
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\atomics.hlsl", "cs_6_0",
                          pArguments.data(), 3, nullptr, 0,
                          {"call i32 @dx.op.atomicCompareExchange.i32(i32 79, "
                           "%dx.types.Handle %rw_structbuf_UAV_structbuf"},
                          {"call i32 @dx.op.atomicCompareExchange.i32(i32 79, "
                           "%dx.types.Handle %ro_structbuf_texture_structbuf"},
                          "Non-UAV destination to atomic intrinsic.", false);
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\atomics.hlsl", "cs_6_0",
                          pArguments.data(), 3, nullptr, 0,
                          {"call i32 @dx.op.atomicBinOp.i32(i32 78, "
                           "%dx.types.Handle %rw_buf_UAV_buf"},
                          {"call i32 @dx.op.atomicBinOp.i32(i32 78, "
                           "%dx.types.Handle %ro_buf_texture_buf"},
                          "Non-UAV destination to atomic intrinsic.", false);
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\atomics.hlsl", "cs_6_0",
                          pArguments.data(), 3, nullptr, 0,
                          {"call i32 @dx.op.atomicCompareExchange.i32(i32 79, "
                           "%dx.types.Handle %rw_buf_UAV_buf"},
                          {"call i32 @dx.op.atomicCompareExchange.i32(i32 79, "
                           "%dx.types.Handle %ro_buf_texture_buf"},
                          "Non-UAV destination to atomic intrinsic.", false);

  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\atomics.hlsl", "cs_6_0",
                          pArguments.data(), 3, nullptr, 0,
                          {"call i32 @dx.op.atomicBinOp.i32(i32 78, "
                           "%dx.types.Handle %rw_tex_UAV_1d"},
                          {"call i32 @dx.op.atomicBinOp.i32(i32 78, "
                           "%dx.types.Handle %ro_tex_texture_1d"},
                          "Non-UAV destination to atomic intrinsic.", false);
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\atomics.hlsl", "cs_6_0",
                          pArguments.data(), 3, nullptr, 0,
                          {"call i32 @dx.op.atomicCompareExchange.i32(i32 79, "
                           "%dx.types.Handle %rw_tex_UAV_1d"},
                          {"call i32 @dx.op.atomicCompareExchange.i32(i32 79, "
                           "%dx.types.Handle %ro_tex_texture_1d"},
                          "Non-UAV destination to atomic intrinsic.", false);

  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\atomics.hlsl", "cs_6_0", pArguments.data(), 3,
      nullptr, 0,
      {"call i32 @dx.op.atomicBinOp.i32(i32 78, %dx.types.Handle "
       "%rw_buf_UAV_buf"},
      {"call i32 @dx.op.atomicBinOp.i32(i32 78, %dx.types.Handle %CB_cbuffer"},
      "Non-UAV destination to atomic intrinsic.", false);
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\atomics.hlsl", "cs_6_0",
                          pArguments.data(), 3, nullptr, 0,
                          {"call i32 @dx.op.atomicCompareExchange.i32(i32 79, "
                           "%dx.types.Handle %rw_buf_UAV_buf"},
                          {"call i32 @dx.op.atomicCompareExchange.i32(i32 79, "
                           "%dx.types.Handle %CB_cbuffer"},
                          "Non-UAV destination to atomic intrinsic.", false);

  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\atomics.hlsl", "cs_6_0",
                          pArguments.data(), 3, nullptr, 0,
                          {"call i32 @dx.op.atomicBinOp.i32(i32 78, "
                           "%dx.types.Handle %rw_buf_UAV_buf"},
                          {"call i32 @dx.op.atomicBinOp.i32(i32 78, "
                           "%dx.types.Handle %\"$Globals_cbuffer\""},
                          "Non-UAV destination to atomic intrinsic.", false);
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\atomics.hlsl", "cs_6_0",
                          pArguments.data(), 3, nullptr, 0,
                          {"call i32 @dx.op.atomicCompareExchange.i32(i32 79, "
                           "%dx.types.Handle %rw_buf_UAV_buf"},
                          {"call i32 @dx.op.atomicCompareExchange.i32(i32 79, "
                           "%dx.types.Handle %\"$Globals_cbuffer\""},
                          "Non-UAV destination to atomic intrinsic.", false);

  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\atomics.hlsl", "cs_6_0", pArguments.data(), 3,
      nullptr, 0, {"atomicrmw add i32 addrspace(3)* @\"\\01?gs_var@@3IA\""},
      {"atomicrmw add i32 addrspace(3)* @\"\\01?cgs_var@@3IB\""},
      "Constant destination to atomic.", false);
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\atomics.hlsl", "cs_6_0",
                          pArguments.data(), 3, nullptr, 0,
                          {"cmpxchg i32 addrspace(3)* @\"\\01?gs_var@@3IA\""},
                          {"cmpxchg i32 addrspace(3)* @\"\\01?cgs_var@@3IB\""},
                          "Constant destination to atomic.", false);

  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\atomics.hlsl", "cs_6_0", pArguments.data(), 3,
      nullptr, 0,
      {"%([a-zA-Z0-9]+) = getelementptr \\[3 x i32\\], \\[3 x i32\\] "
       "addrspace\\(3\\)\\* @\"\\\\01\\?cgs_arr@@3QBIB\"([^\n]*)"},
      {"%\\1 = getelementptr \\[3 x i32\\], \\[3 x i32\\] addrspace\\(3\\)\\* "
       "@\"\\\\01\\?cgs_arr@@3QBIB\"\\2\n"
       "%dummy = atomicrmw add i32 addrspace\\(3\\)\\* %\\1, i32 1 seq_cst, "
       "!dbg !104 ; line:51 col:3"},
      "Constant destination to atomic.", true);
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\atomics.hlsl", "cs_6_0", pArguments.data(), 3,
      nullptr, 0,
      {"%([a-zA-Z0-9]+) = getelementptr \\[3 x i32\\], \\[3 x i32\\] "
       "addrspace\\(3\\)\\* @\"\\\\01\\?cgs_arr@@3QBIB\"([^\n]*)"},
      {"%\\1 = getelementptr \\[3 x i32\\], \\[3 x i32\\] addrspace\\(3\\)\\* "
       "@\"\\\\01\\?cgs_arr@@3QBIB\"\\2\n"
       "%dummy = cmpxchg i32 addrspace\\(3\\)\\* %\\1, i32 1, i32 2 seq_cst "
       "seq_cst, !dbg !105 ;"},
      "Constant destination to atomic.", true);
}

// Check validation error for non-groupshared dest
TEST_F(ValidationTest, AtomicsInvalidDests) {
  // Technically, an error is generated for validator version 1.7, however,
  // the error message was updated in validator version 1.8, so this test now
  // requires version 1.8.
  if (m_ver.SkipDxilVersion(1, 8))
    return;
  std::vector<LPCWSTR> pArguments = {L"-HV", L"2021", L"-Zi"};
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\atomics.hlsl", "lib_6_3", pArguments.data(), 2,
      nullptr, 0, {"atomicrmw add i32 addrspace(3)* @\"\\01?gs_var@@3IA\""},
      {"atomicrmw add i32* %res"},
      "Non-groupshared or node record destination to atomic operation.", false);
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\atomics.hlsl", "lib_6_3", pArguments.data(), 2,
      nullptr, 0, {"cmpxchg i32 addrspace(3)* @\"\\01?gs_var@@3IA\""},
      {"cmpxchg i32* %res"},
      "Non-groupshared or node record destination to atomic operation.", false);
}

// Errors are expected for compute shaders when:
// - a broadcasting node has an input record and/or output records
// - the launch type is not broadcasting
// This test operates by changing the [Shader("node")] metadata entry
// to [Shader("compute")] for each shader in turn.
// It also changes the coalescing to thread in the 2nd to last test case,
// after swapping out the shader kind to compute.
TEST_F(ValidationTest, ComputeNodeCompatibility) {
  if (m_ver.SkipDxilVersion(1, 7))
    return;
  std::vector<LPCWSTR> pArguments = {L"-HV", L"2021"};
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\compute_node_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"!\"node01\", null, null, !([0-9]+)}\n"
       "!\\1 = !{i32 8, i32 15"}, // original: node shader
      {"!\"node01\", null, null, !\\1}\n"
       "!\\1 = !{i32 8, i32 5"}, // changed to: compute shader
      "Compute entry 'node01' has unexpected node shader metadata", true);
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\compute_node_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"!\"node02\", null, null, !([0-9]+)}\n"
       "!\\1 = !{i32 8, i32 15"}, // original: node shader
      {"!\"node02\", null, null, !\\1}\n"
       "!\\1 = !{i32 8, i32 5"}, // changed to: compute shader
      "Compute entry 'node02' has unexpected node shader metadata", true);
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\compute_node_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"!\"node03\", null, null, !([0-9]+)}\n"
       "!\\1 = !{i32 8, i32 15"}, // original: node shader
      {"!\"node03\", null, null, !\\1}\n"
       "!\\1 = !{i32 8, i32 5"}, // changed to: compute shader
      "Compute entry 'node03' has unexpected node shader metadata", true);
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\compute_node_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"!\"node04\", null, null, !([0-9]+)}\n"
       "!\\1 = !{i32 8, i32 15"}, // original: node shader
      {"!\"node04\", null, null, !\\1}\n"
       "!\\1 = !{i32 8, i32 5"}, // changed to: compute shader
      "Compute entry 'node04' has unexpected node shader metadata", true);
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\compute_node_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"!\"node04\", null, null, !([0-9]+)}\n"
       "!\\1 = !{i32 8, i32 15, i32 13, i32 2"}, // original: node shader
                                                 // coalesing launch type
      {"!\"node04\", null, null, !\\1}\n"
       "!\\1 = !{i32 8, i32 5, i32 13, i32 3"}, // changed to: compute shader
                                                // thread launch type
      "Compute entry 'node04' has unexpected node shader metadata", true);
}

// Check validation error for incompatible node input record types
TEST_F(ValidationTest, NodeInputCompatibility) {
  if (m_ver.SkipDxilVersion(1, 7))
    return;
  std::vector<LPCWSTR> pArguments = {L"-HV", L"2021"};
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\node_input_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"= !{i32 1, i32 97"}, // DispatchNodeInputRecord
      {"= !{i32 1, i32 65"}, // GroupNodeInputRecords
      "broadcasting node shader 'node01' has incompatible input record type "
      "(should be {RW}DispatchNodeInputRecord)");
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\node_input_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"= !{i32 1, i32 97"}, // DispatchNodeInputRecord
      {"= !{i32 1, i32 69"}, // RWGroupNodeInputRecords
      "broadcasting node shader 'node01' has incompatible input record type "
      "(should be {RW}DispatchNodeInputRecord)");
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\node_input_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"= !{i32 1, i32 97"}, // DispatchNodeInputRecord
      {"= !{i32 1, i32 33"}, // ThreadNodeInputRecord
      "broadcasting node shader 'node01' has incompatible input record type "
      "(should be {RW}DispatchNodeInputRecord)");
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\node_input_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"= !{i32 1, i32 97"}, // DispatchNodeInputRecord
      {"= !{i32 1, i32 37"}, // RWThreadNodeInputRecord
      "broadcasting node shader 'node01' has incompatible input record type "
      "(should be {RW}DispatchNodeInputRecord)");
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\node_input_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"= !{i32 1, i32 65"}, // GroupNodeInputRecords
      {"= !{i32 1, i32 97"}, // DispatchNodeInputRecord
      "coalescing node shader 'node02' has incompatible input record type "
      "(should be {RW}GroupNodeInputRecords or EmptyNodeInput)");
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\node_input_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"= !{i32 1, i32 65"},  // GroupNodeInputRecords
      {"= !{i32 1, i32 101"}, // RWDispatchNodeInputRecord
      "coalescing node shader 'node02' has incompatible input record type "
      "(should be {RW}GroupNodeInputRecords or EmptyNodeInput)");
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\node_input_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"= !{i32 1, i32 65"}, // GroupNodeInputRecords
      {"= !{i32 1, i32 33"}, // ThreadNodeInputRecord
      "coalescing node shader 'node02' has incompatible input record type "
      "(should be {RW}GroupNodeInputRecords or EmptyNodeInput)");
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\node_input_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"= !{i32 1, i32 65"}, // GroupNodeInputRecords
      {"= !{i32 1, i32 37"}, // RWThreadNodeInputRecord
      "coalescing node shader 'node02' has incompatible input record type "
      "(should be {RW}GroupNodeInputRecords or EmptyNodeInput)");
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\node_input_compatibility.hlsl",
                          "lib_6_8", pArguments.data(), 2, nullptr, 0,
                          {"= !{i32 1, i32 33"}, // ThreadNodeInputRecord
                          {"= !{i32 1, i32 97"}, // DispatchNodeInputRecord
                          "thread node shader 'node03' has incompatible input "
                          "record type (should be {RW}ThreadNodeInputRecord)");
  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\node_input_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"= !{i32 1, i32 33"},  // ThreadNodeInputRecord
      {"= !{i32 1, i32 101"}, // RWDispatchNodeInputRecord
      "thread node shader 'node03' has incompatible input record type (should "
      "be {RW}ThreadNodeInputRecord)");
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\node_input_compatibility.hlsl",
                          "lib_6_8", pArguments.data(), 2, nullptr, 0,
                          {"= !{i32 1, i32 33"}, // ThreadNodeInputRecord
                          {"= !{i32 1, i32 65"}, // GroupNodeInputRecords
                          "thread node shader 'node03' has incompatible input "
                          "record type (should be {RW}ThreadNodeInputRecord)");
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\node_input_compatibility.hlsl",
                          "lib_6_8", pArguments.data(), 2, nullptr, 0,
                          {"= !{i32 1, i32 33"}, // ThreadNodeInputRecord
                          {"= !{i32 1, i32 69"}, // RWGroupNodeInputRecords
                          "thread node shader 'node03' has incompatible input "
                          "record type (should be {RW}ThreadNodeInputRecord)");
  RewriteAssemblyCheckMsg(L"..\\DXILValidation\\node_input_compatibility.hlsl",
                          "lib_6_8", pArguments.data(), 2, nullptr, 0,
                          {"= !{i32 1, i32 33"}, // ThreadNodeInputRecord
                          {"= !{i32 1, i32 69"}, // RWGroupNodeInputRecords
                          "thread node shader 'node03' has incompatible input "
                          "record type (should be {RW}ThreadNodeInputRecord)");
}

// Check validation error for multiple input nodes of different types
TEST_F(ValidationTest, NodeInputMultiplicity) {
  if (m_ver.SkipDxilVersion(1, 7))
    return;
  std::vector<LPCWSTR> pArguments = {L"-HV", L"2021"};

  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\node_input_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"= !{i32 8, i32 15, i32 13, i32 1,([^\n]*)i32 20, !([0-9]+)(.*)"
       "!\\2 = !{!([0-9]+)}",                   // input records
       "= !{i32 1, i32 33, i32 2, !([0-9]+)}"}, // end of output
      {"= !{i32 8, i32 15, i32 13, i32 1,\\1i32 20, !\\2\\3"
       "!\\2 = !{!\\4, !100}", // multiple input records
       "= !{i32 1, i32 33, i32 2, !\\1}\n"
       "!100 = !{i32 1, i32 97, i32 2, !\\1}\n"}, // extra
                                                  // DispatchNodeInputRecord
      "node shader 'node01' may not have more than one input record \\(2 are "
      "declared\\)",
      true);

  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\node_input_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"= !{i32 8, i32 15, i32 13, i32 1,([^\n]*)i32 20, !([0-9]+)(.*)"
       "!\\2 = !{!([0-9]+)}",                   // input records
       "= !{i32 1, i32 33, i32 2, !([0-9]+)}"}, // end of output
      {"= !{i32 8, i32 15, i32 13, i32 1,\\1i32 20, !\\2\\3"
       "!\\2 = !{!\\4, !100}", // multiple input records
       "= !{i32 1, i32 33, i32 2, !\\1}\n"
       "!100 = !{i32 1, i32 9, i32 2, !\\1}\n"}, // extra EmptyNodeInput
      "node shader 'node01' may not have more than one input record \\(2 are "
      "declared\\)",
      true);

  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\node_input_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"= !{i32 8, i32 15, i32 13, i32 2,([^\n]*)i32 20, !([0-9]+)(.*)"
       "!\\2 = !{!([0-9]+)}",                   // input records
       "= !{i32 1, i32 33, i32 2, !([0-9]+)}"}, // end of output
      {"= !{i32 8, i32 15, i32 13, i32 2,\\1i32 20, !\\2\\3"
       "!\\2 = !{!\\4, !100}", // multiple input records
       "= !{i32 1, i32 33, i32 2, !\\1}\n"
       "!100 = !{i32 1, i32 65, i32 2, !\\1}\n"}, // extra GroupNodeInputRecords
      "node shader 'node02' may not have more than one input record \\(2 are "
      "declared\\)",
      true);

  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\node_input_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"= !{i32 8, i32 15, i32 13, i32 2,([^\n]*)i32 20, !([0-9]+)(.*)"
       "!\\2 = !{!([0-9]+)}",                   // input records
       "= !{i32 1, i32 33, i32 2, !([0-9]+)}"}, // end of output
      {"= !{i32 8, i32 15, i32 13, i32 2,\\1i32 20, !\\2\\3"
       "!\\2 = !{!\\4, !100}", // multiple input records
       "= !{i32 1, i32 33, i32 2, !\\1}\n"
       "!100 = !{i32 1, i32 9, i32 2, !\\1}\n"}, // extra EmptyNodeInput
      "node shader 'node02' may not have more than one input record \\(2 are "
      "declared\\)",
      true);

  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\node_input_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"= !{i32 8, i32 15, i32 13, i32 3,([^\n]*)i32 20, !([0-9]+)(.*)"
       "!\\2 = !{!([0-9]+)}",                   // input records
       "= !{i32 1, i32 33, i32 2, !([0-9]+)}"}, // end of output
      {"= !{i32 8, i32 15, i32 13, i32 3,\\1i32 20, !\\2\\3"
       "!\\2 = !{!\\4, !100}", // multiple input records
       "= !{i32 1, i32 33, i32 2, !\\1}\n"
       "!100 = !{i32 1, i32 33, i32 2, !\\1}\n"}, // extra ThreadNodeInputRecord
      "node shader 'node03' may not have more than one input record \\(2 are "
      "declared\\)",
      true);

  RewriteAssemblyCheckMsg(
      L"..\\DXILValidation\\node_input_compatibility.hlsl", "lib_6_8",
      pArguments.data(), 2, nullptr, 0,
      {"= !{i32 8, i32 15, i32 13, i32 3,([^\n]*)i32 20, !([0-9]+)(.*)"
       "!\\2 = !{!([0-9]+)}",                   // input records
       "= !{i32 1, i32 33, i32 2, !([0-9]+)}"}, // end of output
      {"= !{i32 8, i32 15, i32 13, i32 3,\\1i32 20, !\\2\\3"
       "!\\2 = !{!\\4, !100}", // multiple input records
       "= !{i32 1, i32 33, i32 2, !\\1}\n"
       "!100 = !{i32 1, i32 9, i32 2, !\\1}\n"}, // extra EmptyNodeInput
      "node shader 'node03' may not have more than one input record \\(2 are "
      "declared\\)",
      true);
}

TEST_F(ValidationTest, CacheInitWithMinPrec) {
  if (!m_ver.m_InternalValidator)
    if (m_ver.SkipDxilVersion(1, 8))
      return;
  // Ensures type cache is property initialized when in min-precision mode
  TestCheck(L"..\\DXILValidation\\val-dx-type-minprec.ll");
}

TEST_F(ValidationTest, CacheInitWithLowPrec) {
  if (!m_ver.m_InternalValidator)
    if (m_ver.SkipDxilVersion(1, 8))
      return;
  // Ensures type cache is property initialized when in exact low-precision mode
  TestCheck(L"..\\DXILValidation\\val-dx-type-lowprec.ll");
}

TEST_F(ValidationTest, PSVStringTableReorder) {
  if (!m_ver.m_InternalValidator)
    if (m_ver.SkipDxilVersion(1, 8))
      return;

  CComPtr<IDxcBlob> pProgram;
  CompileSource("float4 main(float a:A, float b:B) : SV_Target { return 1; }",
                "ps_6_0", &pProgram);

  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pResult;
  unsigned Flags = 0;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pResult));
  // Make sure the validation was successful.
  HRESULT status;
  VERIFY_IS_NOT_NULL(pResult);
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);

  // Update string table.
  hlsl::DxilContainerHeader *pHeader;
  hlsl::DxilPartIterator pPartIter(nullptr, 0);
  pHeader = (hlsl::DxilContainerHeader *)pProgram->GetBufferPointer();
  pPartIter =
      std::find_if(hlsl::begin(pHeader), hlsl::end(pHeader),
                   hlsl::DxilPartIsType(hlsl::DFCC_PipelineStateValidation));
  VERIFY_ARE_NOT_EQUAL(hlsl::end(pHeader), pPartIter);

  const DxilPartHeader *pPSVPart = (const DxilPartHeader *)(*pPartIter);
  const uint32_t *PSVPtr = (const uint32_t *)GetDxilPartData(pPSVPart);

  uint32_t PSVRuntimeInfo_size = *(PSVPtr++);
  VERIFY_ARE_EQUAL(sizeof(PSVRuntimeInfo3), PSVRuntimeInfo_size);
  PSVRuntimeInfo3 *PSVInfo =
      const_cast<PSVRuntimeInfo3 *>((const PSVRuntimeInfo3 *)PSVPtr);
  VERIFY_ARE_EQUAL(2u, PSVInfo->SigInputElements);
  PSVPtr += PSVRuntimeInfo_size / 4;
  uint32_t ResourceCount = *(PSVPtr++);
  VERIFY_ARE_EQUAL(0u, ResourceCount);
  uint32_t StringTableSize = *(PSVPtr++);
  VERIFY_ARE_EQUAL(12u, StringTableSize);
  const char *StringTable = (const char *)PSVPtr;
  VERIFY_ARE_EQUAL('\0', StringTable[0]);
  PSVPtr += StringTableSize / 4;
  uint32_t SemanticIndexTableEntries = *(PSVPtr++);
  VERIFY_ARE_EQUAL(1u, SemanticIndexTableEntries);
  PSVPtr += sizeof(SemanticIndexTableEntries) / 4;
  uint32_t PSVSignatureElement_size = *(PSVPtr++);
  VERIFY_ARE_EQUAL(sizeof(PSVSignatureElement0), PSVSignatureElement_size);
  PSVSignatureElement0 *SigInput =
      const_cast<PSVSignatureElement0 *>((const PSVSignatureElement0 *)PSVPtr);
  PSVSignatureElement0 *SigInput1 = SigInput + 1;
  PSVSignatureElement0 *SigOutput = SigInput + 2;
  // Update StringTable only.
  const char OrigStringTable[12] = {0,   'A', 0,   'B', 0, 'm',
                                    'a', 'i', 'n', 0,   0, 0};
  VERIFY_ARE_EQUAL(0, memcmp(OrigStringTable, StringTable, 12));
  const char UpdatedStringTable[12] = {'B', 0,   'A', 0, 'm', 'a',
                                       'i', 'n', 0,   0, 0,   0};
  memcpy((void *)StringTable, (void *)UpdatedStringTable, 12);

  // Run validation again.
  CComPtr<IDxcOperationResult> pUpdatedTableResult;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pUpdatedTableResult));
  // Make sure the validation was fail.
  VERIFY_IS_NOT_NULL(pUpdatedTableResult);
  VERIFY_SUCCEEDED(pUpdatedTableResult->GetStatus(&status));
  VERIFY_FAILED(status);
  CheckOperationResultMsgs(
      pUpdatedTableResult,
      {"error: DXIL container mismatch for 'SigInputElement' between 'PSV0' "
       "part:('PSVSignatureElement:",
       "  SemanticName: ",
       "  SemanticIndex: 0 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 1",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 2",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "') and DXIL module:('PSVSignatureElement:",
       "  SemanticName: A",
       "  SemanticIndex: 0 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 1",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 2",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "')",
       "error: DXIL container mismatch for 'SigInputElement' between 'PSV0' "
       "part:('PSVSignatureElement:",
       "  SemanticName: ",
       "  SemanticIndex: 0 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 1",
       "  Rows: 1",
       "  Cols: 1",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 2",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "') and DXIL module:('PSVSignatureElement:",
       "  SemanticName: B",
       "  SemanticIndex: 0 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 1",
       "  Rows: 1",
       "  Cols: 1",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 2",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "')",
       "error: DXIL container mismatch for 'EntryFunctionName' between 'PSV0' "
       "part:('ain') and DXIL module:('main')",
       "error: In 'StringTable', 'A' is not used",
       "error: In 'StringTable', 'main' is not used",
       "error: Container part 'Pipeline State Validation' does not match "
       "expected for module.",
       "Validation failed."},
      /*maySucceedAnyway*/ false, /*bRegex*/ false);

  // Update string table index.
  SigInput->SemanticName = 2;
  SigInput1->SemanticName = 0;
  PSVInfo->EntryFunctionName = 4;
  SigOutput->SemanticName = 1;

  // Run validation again.
  CComPtr<IDxcOperationResult> pUpdatedResult;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pUpdatedResult));
  // Make sure the validation was successful.
  VERIFY_IS_NOT_NULL(pUpdatedResult);
  VERIFY_SUCCEEDED(pUpdatedResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);

  // Create unused name in String table.
  PSVInfo->EntryFunctionName = UINT32_MAX;

  // Run validation again.
  CComPtr<IDxcOperationResult> pUpdatedTableResult2;
  VERIFY_SUCCEEDED(
      pValidator->Validate(pProgram, Flags, &pUpdatedTableResult2));
  // Make sure the validation was fail.
  VERIFY_IS_NOT_NULL(pUpdatedTableResult2);
  VERIFY_SUCCEEDED(pUpdatedTableResult2->GetStatus(&status));
  VERIFY_FAILED(status);
  CheckOperationResultMsgs(
      pUpdatedTableResult2,
      {
          "In 'PSV0 part', 'EntryFunctionName' is not well-formed",
          "error: In 'StringTable', 'main' is not used",
      },
      /*maySucceedAnyway*/ false, /*bRegex*/ false);
}

class SemanticIndexRotator {
  llvm::MutableArrayRef<PSVSignatureElement0> SignatureElements;

public:
  SemanticIndexRotator(PSVSignatureElement0 *SigInput,
                       unsigned NumSignatureElements)
      : SignatureElements(SigInput, NumSignatureElements) {}

  void Rotate(uint32_t SemanticIndexTableEntries) {
    for (unsigned i = 0; i < SignatureElements.size(); ++i)
      if (SignatureElements[i].SemanticIndexes == 0)
        SignatureElements[i].SemanticIndexes = SemanticIndexTableEntries - 1;
      else
        SignatureElements[i].SemanticIndexes =
            SignatureElements[i].SemanticIndexes - 1;
  }
  void Clear(unsigned Index) {
    for (unsigned i = 0; i < SignatureElements.size(); ++i)
      SignatureElements[i].SemanticIndexes = Index;
  }
};

TEST_F(ValidationTest, PSVSemanticIndexTableReorder) {
  if (!m_ver.m_InternalValidator)
    if (m_ver.SkipDxilVersion(1, 8))
      return;

  CComPtr<IDxcBlob> pProgram;
  CompileFile(L"..\\DXILValidation\\hs_signatures.hlsl", "hs_6_0", &pProgram);

  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pResult;
  unsigned Flags = 0;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pResult));
  // Make sure the validation was successful.
  HRESULT status;
  VERIFY_IS_NOT_NULL(pResult);
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);

  // Update input signature semantic index table.
  hlsl::DxilContainerHeader *pHeader;
  hlsl::DxilPartIterator pPartIter(nullptr, 0);
  pHeader = (hlsl::DxilContainerHeader *)pProgram->GetBufferPointer();
  pPartIter =
      std::find_if(hlsl::begin(pHeader), hlsl::end(pHeader),
                   hlsl::DxilPartIsType(hlsl::DFCC_PipelineStateValidation));
  VERIFY_ARE_NOT_EQUAL(hlsl::end(pHeader), pPartIter);

  const DxilPartHeader *pPSVPart = (const DxilPartHeader *)(*pPartIter);
  const uint32_t *PSVPtr = (const uint32_t *)GetDxilPartData(pPSVPart);

  uint32_t PSVRuntimeInfo_size = *(PSVPtr++);
  VERIFY_ARE_EQUAL(sizeof(PSVRuntimeInfo3), PSVRuntimeInfo_size);
  PSVRuntimeInfo3 *PSVInfo =
      const_cast<PSVRuntimeInfo3 *>((const PSVRuntimeInfo3 *)PSVPtr);
  VERIFY_ARE_EQUAL(PSVInfo->SigInputElements, 3u);
  VERIFY_ARE_EQUAL(PSVInfo->SigOutputElements, 3u);
  VERIFY_ARE_EQUAL(PSVInfo->SigPatchConstOrPrimElements, 2u);
  PSVPtr += PSVRuntimeInfo_size / 4;
  uint32_t ResourceCount = *(PSVPtr++);
  VERIFY_ARE_EQUAL(ResourceCount, 0u);

  uint32_t StringTableSize = *(PSVPtr++);
  PSVPtr += StringTableSize / 4;

  uint32_t SemanticIndexTableEntries = *(PSVPtr++);
  llvm::MutableArrayRef<uint32_t> SemanticTable(const_cast<uint32_t *>(PSVPtr),
                                                SemanticIndexTableEntries);

  PSVPtr += SemanticIndexTableEntries;

  uint32_t PSVSignatureElement_size = *(PSVPtr++);
  VERIFY_ARE_EQUAL(PSVSignatureElement_size, sizeof(PSVSignatureElement0));

  SemanticIndexRotator InputRotator(
      const_cast<PSVSignatureElement0 *>((const PSVSignatureElement0 *)PSVPtr),
      PSVInfo->SigInputElements);

  PSVPtr += PSVSignatureElement_size * PSVInfo->SigInputElements / 4;
  SemanticIndexRotator OutputRotator(
      const_cast<PSVSignatureElement0 *>((const PSVSignatureElement0 *)PSVPtr),
      PSVInfo->SigOutputElements);

  PSVPtr += PSVSignatureElement_size * PSVInfo->SigOutputElements / 4;
  SemanticIndexRotator PatchConstOrPrimRotator(
      const_cast<PSVSignatureElement0 *>((const PSVSignatureElement0 *)PSVPtr),
      PSVInfo->SigPatchConstOrPrimElements);

  // Update SemanticTable by rotating.
  std::rotate(SemanticTable.begin(), SemanticTable.begin() + 1,
              SemanticTable.end());

  // Run validation again.
  CComPtr<IDxcOperationResult> pParitalUpdatedResult;
  VERIFY_SUCCEEDED(
      pValidator->Validate(pProgram, Flags, &pParitalUpdatedResult));
  // Make sure the validation was fail.
  VERIFY_IS_NOT_NULL(pParitalUpdatedResult);
  VERIFY_SUCCEEDED(pParitalUpdatedResult->GetStatus(&status));
  VERIFY_FAILED(status);

  CheckOperationResultMsgs(
      pParitalUpdatedResult,
      {"error: DXIL container mismatch for 'SigInputElement' between 'PSV0' "
       "part:('PSVSignatureElement:",
       "  SemanticName: ",
       "  SemanticIndex: 2 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 4",
       "  SemanticKind: Position",
       "  InterpolationMode: 4",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "') and DXIL module:('PSVSignatureElement:",
       "  SemanticName: ",
       "  SemanticIndex: 0 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 4",
       "  SemanticKind: Position",
       "  InterpolationMode: 4",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "')",
       "error: DXIL container mismatch for 'SigInputElement' between 'PSV0' "
       "part:('PSVSignatureElement:",
       "  SemanticName: TEXCOORD",
       "  SemanticIndex: 3 ",
       "  IsAllocated: 1",
       "  StartRow: 1",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 2",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 2",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "') and DXIL module:('PSVSignatureElement:",
       "  SemanticName: TEXCOORD",
       "  SemanticIndex: 2 ",
       "  IsAllocated: 1",
       "  StartRow: 1",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 2",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 2",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "')",
       "error: DXIL container mismatch for 'SigInputElement' between 'PSV0' "
       "part:('PSVSignatureElement:",
       "  SemanticName: NORMAL",
       "  SemanticIndex: 0 ",
       "  IsAllocated: 1",
       "  StartRow: 2",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 3",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 2",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "') and DXIL module:('PSVSignatureElement:",
       "  SemanticName: NORMAL",
       "  SemanticIndex: 3 ",
       "  IsAllocated: 1",
       "  StartRow: 2",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 3",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 2",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "')",
       "error: DXIL container mismatch for 'SigOutputElement' between 'PSV0' "
       "part:('PSVSignatureElement:",
       "  SemanticName: ",
       "  SemanticIndex: 2 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 4",
       "  SemanticKind: Position",
       "  InterpolationMode: 4",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "') and DXIL module:('PSVSignatureElement:",
       "  SemanticName: ",
       "  SemanticIndex: 0 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 4",
       "  SemanticKind: Position",
       "  InterpolationMode: 4",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "')",
       "error: DXIL container mismatch for 'SigOutputElement' between 'PSV0' "
       "part:('PSVSignatureElement:",
       "  SemanticName: TEXCOORD",
       "  SemanticIndex: 3 ",
       "  IsAllocated: 1",
       "  StartRow: 1",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 2",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 2",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "') and DXIL module:('PSVSignatureElement:",
       "  SemanticName: TEXCOORD",
       "  SemanticIndex: 2 ",
       "  IsAllocated: 1",
       "  StartRow: 1",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 2",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 2",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "')",
       "error: DXIL container mismatch for 'SigOutputElement' between 'PSV0' "
       "part:('PSVSignatureElement:",
       "  SemanticName: NORMAL",
       "  SemanticIndex: 0 ",
       "  IsAllocated: 1",
       "  StartRow: 2",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 3",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 2",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "') and DXIL module:('PSVSignatureElement:",
       "  SemanticName: NORMAL",
       "  SemanticIndex: 3 ",
       "  IsAllocated: 1",
       "  StartRow: 2",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 3",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 2",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "')",
       "error: DXIL container mismatch for 'SigPatchConstantOrPrimElement' "
       "between 'PSV0' part:('PSVSignatureElement:",
       "  SemanticName: ",
       "  SemanticIndex: 1 0 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 3",
       "  Rows: 2",
       "  Cols: 1",
       "  SemanticKind: TessFactor",
       "  InterpolationMode: 0",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "') and DXIL module:('PSVSignatureElement:",
       "  SemanticName: ",
       "  SemanticIndex: 0 1 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 3",
       "  Rows: 2",
       "  Cols: 1",
       "  SemanticKind: TessFactor",
       "  InterpolationMode: 0",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "')",
       "error: DXIL container mismatch for 'SigPatchConstantOrPrimElement' "
       "between 'PSV0' part:('PSVSignatureElement:",
       "  SemanticName: PN_POSITION",
       "  SemanticIndex: 2 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 1",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 0",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "') and DXIL module:('PSVSignatureElement:",
       "  SemanticName: PN_POSITION",
       "  SemanticIndex: 0 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 1",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 0",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "')",
       "error: Container part 'Pipeline State Validation' does not match "
       "expected for module.",
       "Validation failed."

      },
      /*maySucceedAnyway*/ false, /*bRegex*/ false);
  // Update SemanticIndexes.
  InputRotator.Rotate(SemanticIndexTableEntries);
  OutputRotator.Rotate(SemanticIndexTableEntries);
  PatchConstOrPrimRotator.Rotate(SemanticIndexTableEntries);

  // Run validation again.
  CComPtr<IDxcOperationResult> pUpdatedResult;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pUpdatedResult));
  // Make sure the validation was successful.
  VERIFY_IS_NOT_NULL(pUpdatedResult);
  VERIFY_SUCCEEDED(pUpdatedResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);

  // Clear SemanticIndexes.
  InputRotator.Clear(UINT32_MAX);
  OutputRotator.Clear(UINT32_MAX);
  PatchConstOrPrimRotator.Clear(UINT32_MAX);

  // Run validation again.
  CComPtr<IDxcOperationResult> pUpdatedResult2;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pUpdatedResult2));
  // Make sure the validation was successful.
  VERIFY_IS_NOT_NULL(pUpdatedResult2);
  VERIFY_SUCCEEDED(pUpdatedResult2->GetStatus(&status));
  VERIFY_FAILED(status);

  CheckOperationResultMsgs(
      pUpdatedResult2,
      {"error: In 'PSV0 part', 'SemanticIndex' is not well-formed",
       "error: In 'SemanticIndexTable', '0' is not used",
       "error: In 'SemanticIndexTable', '2' is not used",
       "error: In 'SemanticIndexTable', '3' is not used",
       "error: In 'SemanticIndexTable', '4' is not used",
       "error: Container part 'Pipeline State Validation' "
       "does not match expected for module.",
       "Validation failed."},
      /*maySucceedAnyway*/ false, /*bRegex*/ false);
}

struct SimplePSV {
  PSVRuntimeInfo3 *PSVInfo;
  llvm::MutableArrayRef<PSVResourceBindInfo1> ResourceBindInfo;
  llvm::MutableArrayRef<char> StringTable;
  llvm::MutableArrayRef<uint32_t> SemanticIndexTable;

  llvm::MutableArrayRef<PSVSignatureElement0> SigInput;
  llvm::MutableArrayRef<PSVSignatureElement0> SigOutput;
  llvm::MutableArrayRef<PSVSignatureElement0> SigPatchConstOrPrim;

  llvm::MutableArrayRef<uint32_t> InputToOutputTable[DXIL::kNumOutputStreams];
  llvm::MutableArrayRef<uint32_t> InputToPCOutputTable;
  llvm::MutableArrayRef<uint32_t> PCInputToOutputTable;
  llvm::MutableArrayRef<uint32_t> ViewIDOutputMask[DXIL::kNumOutputStreams];
  llvm::MutableArrayRef<uint32_t> ViewIDPCOutputMask;
  SimplePSV(const DxilPartHeader *pPSVPart);
};

SimplePSV::SimplePSV(const DxilPartHeader *pPSVPart) {
  uint32_t PartSize = pPSVPart->PartSize;
  uint32_t *PSVPtr =
      const_cast<uint32_t *>((const uint32_t *)GetDxilPartData(pPSVPart));
  const uint32_t *PSVPtrEnd = PSVPtr + PartSize / 4;

  uint32_t PSVRuntimeInfoSize = *(PSVPtr++);
  VERIFY_ARE_EQUAL(sizeof(PSVRuntimeInfo3), PSVRuntimeInfoSize);
  PSVRuntimeInfo3 *PSVInfo3 =
      const_cast<PSVRuntimeInfo3 *>((const PSVRuntimeInfo3 *)PSVPtr);
  PSVInfo = PSVInfo3;

  PSVPtr += PSVRuntimeInfoSize / 4;
  uint32_t ResourceCount = *(PSVPtr++);
  if (ResourceCount) {
    uint32_t ResourceBindInfoSize = *(PSVPtr++);
    VERIFY_ARE_EQUAL(sizeof(PSVResourceBindInfo1), ResourceBindInfoSize);
    ResourceBindInfo = llvm::MutableArrayRef<PSVResourceBindInfo1>(
        (PSVResourceBindInfo1 *)PSVPtr, ResourceCount);
  }
  PSVPtr += ResourceCount * sizeof(PSVResourceBindInfo1) / 4;
  uint32_t StringTableSize = *(PSVPtr++);
  if (StringTableSize)
    StringTable = llvm::MutableArrayRef<char>((char *)PSVPtr, StringTableSize);
  PSVPtr += StringTableSize / 4;
  uint32_t SemanticIndexTableEntries = *(PSVPtr++);
  if (SemanticIndexTableEntries)
    SemanticIndexTable = llvm::MutableArrayRef<uint32_t>(
        (uint32_t *)PSVPtr, SemanticIndexTableEntries);
  PSVPtr += SemanticIndexTableEntries;
  if (PSVInfo3->SigInputElements || PSVInfo3->SigOutputElements ||
      PSVInfo3->SigPatchConstOrPrimElements) {
    uint32_t PSVSignatureElementSize = *(PSVPtr++);
    VERIFY_ARE_EQUAL(sizeof(PSVSignatureElement0), PSVSignatureElementSize);
  }
  if (PSVInfo3->SigInputElements)
    SigInput = llvm::MutableArrayRef<PSVSignatureElement0>(
        (PSVSignatureElement0 *)PSVPtr, PSVInfo->SigInputElements);
  PSVPtr += PSVInfo3->SigInputElements * sizeof(PSVSignatureElement0) / 4;

  if (PSVInfo3->SigOutputElements)
    SigOutput = llvm::MutableArrayRef<PSVSignatureElement0>(
        (PSVSignatureElement0 *)PSVPtr, PSVInfo->SigOutputElements);
  PSVPtr += PSVInfo3->SigOutputElements * sizeof(PSVSignatureElement0) / 4;

  if (PSVInfo3->SigPatchConstOrPrimElements)
    SigPatchConstOrPrim = llvm::MutableArrayRef<PSVSignatureElement0>(
        (PSVSignatureElement0 *)PSVPtr, PSVInfo->SigPatchConstOrPrimElements);
  PSVPtr +=
      PSVInfo3->SigPatchConstOrPrimElements * sizeof(PSVSignatureElement0) / 4;

  if (PSVInfo3->UsesViewID) {

    for (unsigned i = 0; i < DXIL::kNumOutputStreams; i++) {
      if (PSVInfo3->SigOutputVectors[i] == 0)
        continue;
      ViewIDOutputMask[i] = llvm::MutableArrayRef<uint32_t>(
          (uint32_t *)PSVPtr,
          llvm::RoundUpToAlignment(PSVInfo3->SigOutputVectors[i], 8) / 8);
      PSVPtr += ViewIDOutputMask[i].size();
    }
    if ((PSVInfo3->ShaderStage == static_cast<uint8_t>(PSVShaderKind::Hull) ||
         PSVInfo3->ShaderStage == static_cast<uint8_t>(PSVShaderKind::Mesh)) &&
        PSVInfo3->SigPatchConstOrPrimVectors != 0) {
      ViewIDPCOutputMask = llvm::MutableArrayRef<uint32_t>(
          (uint32_t *)PSVPtr,
          llvm::RoundUpToAlignment(PSVInfo3->SigPatchConstOrPrimVectors, 8) /
              8);
      PSVPtr += ViewIDPCOutputMask.size();
    }
  }

  for (unsigned i = 0; i < DXIL::kNumOutputStreams; i++) {
    if (PSVInfo3->SigInputVectors == 0 || PSVInfo3->SigOutputVectors[i] == 0)
      continue;
    InputToOutputTable[i] = llvm::MutableArrayRef<uint32_t>(
        (uint32_t *)PSVPtr,
        4 * PSVInfo3->SigInputVectors *
            llvm::RoundUpToAlignment(PSVInfo3->SigOutputVectors[i], 8) / 8);
    PSVPtr += InputToOutputTable[i].size();
  }
  if ((PSVInfo3->ShaderStage == static_cast<uint8_t>(PSVShaderKind::Hull) ||
       PSVInfo3->ShaderStage == static_cast<uint8_t>(PSVShaderKind::Mesh)) &&
      PSVInfo3->SigInputVectors != 0 &&
      PSVInfo3->SigPatchConstOrPrimVectors != 0) {
    InputToPCOutputTable = llvm::MutableArrayRef<uint32_t>(
        (uint32_t *)PSVPtr,
        4 * PSVInfo3->SigInputVectors *
            llvm::RoundUpToAlignment(PSVInfo3->SigPatchConstOrPrimVectors, 8) /
            8);
    PSVPtr += InputToPCOutputTable.size();
  } else if (PSVInfo3->ShaderStage ==
                 static_cast<uint8_t>(PSVShaderKind::Domain) &&
             PSVInfo3->SigOutputVectors[0] != 0 &&
             PSVInfo3->SigPatchConstOrPrimVectors != 0) {
    PCInputToOutputTable = llvm::MutableArrayRef<uint32_t>(
        (uint32_t *)PSVPtr,
        4 * PSVInfo3->SigPatchConstOrPrimVectors *
            llvm::RoundUpToAlignment(PSVInfo3->SigOutputVectors[0], 8) / 8);
    PSVPtr += PCInputToOutputTable.size();
  }
  VERIFY_ARE_EQUAL(PSVPtr, PSVPtrEnd);
}

TEST_F(ValidationTest, PSVContentValidationVS) {
  if (!m_ver.m_InternalValidator)
    if (m_ver.SkipDxilVersion(1, 8))
      return;

  CComPtr<IDxcBlob> pProgram;
  CompileFile(L"..\\DXC\\dumpPSV_VS.hlsl", "vs_6_8", &pProgram);

  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pResult;
  unsigned Flags = 0;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pResult));
  // Make sure the validation was successful.
  HRESULT status;
  VERIFY_IS_NOT_NULL(pResult);
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);

  // Get PSV part.
  hlsl::DxilContainerHeader *pHeader;
  hlsl::DxilPartIterator pPartIter(nullptr, 0);
  pHeader = (hlsl::DxilContainerHeader *)pProgram->GetBufferPointer();
  pPartIter =
      std::find_if(hlsl::begin(pHeader), hlsl::end(pHeader),
                   hlsl::DxilPartIsType(hlsl::DFCC_PipelineStateValidation));
  VERIFY_ARE_NOT_EQUAL(hlsl::end(pHeader), pPartIter);

  const DxilPartHeader *pPSVPart = (const DxilPartHeader *)(*pPartIter);
  SimplePSV PSV(pPSVPart);

  // Update PSV.
  PSV.SigInput[0].InterpolationMode = 20;
  PSV.SigOutput[0].InterpolationMode = 20;
  PSV.ResourceBindInfo[0].ResFlags = 20;
  memset(PSV.InputToOutputTable[0].data(), 0,
         PSV.InputToOutputTable[0].size() * sizeof(uint32_t));
  // Run validation again.
  CComPtr<IDxcOperationResult> pUpdatedTableResult;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pUpdatedTableResult));
  // Make sure the validation was fail.
  VERIFY_IS_NOT_NULL(pUpdatedTableResult);
  VERIFY_SUCCEEDED(pUpdatedTableResult->GetStatus(&status));
  VERIFY_FAILED(status);
  CheckOperationResultMsgs(
      pUpdatedTableResult,
      {"error: DXIL container mismatch for 'ResourceBindInfo' between 'PSV0' "
       "part:('PSVResourceBindInfo:",
       "  Space: 0",
       "  LowerBound: 5",
       "  UpperBound: 5",
       "  ResType: CBV",
       "  ResKind: CBuffer",
       "  ResFlags: ",
       "') and DXIL module:('PSVResourceBindInfo:",
       "  Space: 0",
       "  LowerBound: 5",
       "  UpperBound: 5",
       "  ResType: CBV",
       "  ResKind: CBuffer",
       "  ResFlags: None",
       "')",
       "error: DXIL container mismatch for 'SigInputElement' between 'PSV0' "
       "part:('PSVSignatureElement:",
       "  SemanticName: POSITION",
       "  SemanticIndex: 0 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 3",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 20",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "') and DXIL module:('PSVSignatureElement:",
       "  SemanticName: POSITION",
       "  SemanticIndex: 0 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 3",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 0",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "')",
       "error: DXIL container mismatch for 'SigOutputElement' between 'PSV0' "
       "part:('PSVSignatureElement:",
       "  SemanticName: NORMAL",
       "  SemanticIndex: 0 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 3",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 20",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "') and DXIL module:('PSVSignatureElement:",
       "  SemanticName: NORMAL",
       "  SemanticIndex: 0 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 0",
       "  Rows: 1",
       "  Cols: 3",
       "  SemanticKind: Arbitrary",
       "  InterpolationMode: 2",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "')",
       "error: DXIL container mismatch for 'ViewIDState' between 'PSV0' "
       "part:('Outputs affected by inputs as a table of bitmasks for stream 0:",
       "Inputs contributing to computation of Outputs[0]:",
       "  Inputs[0] influencing Outputs[0] :  None",
       "  Inputs[1] influencing Outputs[0] :  None",
       "  Inputs[2] influencing Outputs[0] :  None",
       "  Inputs[3] influencing Outputs[0] :  None",
       "  Inputs[4] influencing Outputs[0] :  None",
       "  Inputs[5] influencing Outputs[0] :  None",
       "  Inputs[6] influencing Outputs[0] :  None",
       "  Inputs[7] influencing Outputs[0] :  None",
       "  Inputs[8] influencing Outputs[0] :  None",
       "  Inputs[9] influencing Outputs[0] :  None",
       "  Inputs[10] influencing Outputs[0] :  None",
       "  Inputs[11] influencing Outputs[0] :  None",
       "') and DXIL module:('Outputs affected by inputs as a table of bitmasks "
       "for stream 0:",
       "Inputs contributing to computation of Outputs[0]:",
       "  Inputs[0] influencing Outputs[0] : 8  9  10  11 ",
       "  Inputs[1] influencing Outputs[0] : 8  9  10  11 ",
       "  Inputs[2] influencing Outputs[0] : 8  9  10  11 ",
       "  Inputs[3] influencing Outputs[0] :  None",
       "  Inputs[4] influencing Outputs[0] : 0  1  2 ",
       "  Inputs[5] influencing Outputs[0] : 0  1  2 ",
       "  Inputs[6] influencing Outputs[0] : 0  1  2 ",
       "  Inputs[7] influencing Outputs[0] :  None",
       "  Inputs[8] influencing Outputs[0] : 4 ",
       "  Inputs[9] influencing Outputs[0] : 5 ",
       "  Inputs[10] influencing Outputs[0] :  None",
       "  Inputs[11] influencing Outputs[0] :  None",
       "')",
       "error: Container part 'Pipeline State Validation' does not match "
       "expected for module.",
       "Validation failed."},
      /*maySucceedAnyway*/ false, /*bRegex*/ false);
}

TEST_F(ValidationTest, PSVContentValidationHS) {
  if (!m_ver.m_InternalValidator)
    if (m_ver.SkipDxilVersion(1, 8))
      return;

  CComPtr<IDxcBlob> pProgram;
  CompileFile(L"..\\DXC\\dumpPSV_HS.hlsl", "hs_6_8", &pProgram);

  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pResult;
  unsigned Flags = 0;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pResult));
  // Make sure the validation was successful.
  HRESULT status;
  VERIFY_IS_NOT_NULL(pResult);
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);

  // Get PSV part.
  hlsl::DxilContainerHeader *pHeader;
  hlsl::DxilPartIterator pPartIter(nullptr, 0);
  pHeader = (hlsl::DxilContainerHeader *)pProgram->GetBufferPointer();
  pPartIter =
      std::find_if(hlsl::begin(pHeader), hlsl::end(pHeader),
                   hlsl::DxilPartIsType(hlsl::DFCC_PipelineStateValidation));
  VERIFY_ARE_NOT_EQUAL(hlsl::end(pHeader), pPartIter);

  const DxilPartHeader *pPSVPart = (const DxilPartHeader *)(*pPartIter);
  SimplePSV PSV(pPSVPart);

  // Update PSV.
  PSV.SigPatchConstOrPrim[0].InterpolationMode = 20;
  memset(PSV.InputToPCOutputTable.data(), 0,
         PSV.InputToPCOutputTable.size() * sizeof(uint32_t));
  memset(PSV.ViewIDPCOutputMask.data(), 0,
         PSV.ViewIDPCOutputMask.size() * sizeof(uint32_t));

  // Run validation again.
  CComPtr<IDxcOperationResult> pUpdatedResult;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pUpdatedResult));
  // Make sure the validation was fail.
  VERIFY_IS_NOT_NULL(pUpdatedResult);
  VERIFY_SUCCEEDED(pUpdatedResult->GetStatus(&status));
  VERIFY_FAILED(status);

  CheckOperationResultMsgs(
      pUpdatedResult,
      {"error: DXIL container mismatch for 'SigPatchConstantOrPrimElement' "
       "between 'PSV0' part:('PSVSignatureElement:",
       "  SemanticName: ",
       "  SemanticIndex: 0 1 2 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 3",
       "  Rows: 3",
       "  Cols: 1",
       "  SemanticKind: TessFactor",
       "  InterpolationMode: 20",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "') and DXIL module:('PSVSignatureElement:",
       "  SemanticName: ",
       "  SemanticIndex: 0 1 2 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 3",
       "  Rows: 3",
       "  Cols: 1",
       "  SemanticKind: TessFactor",
       "  InterpolationMode: 0",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "')",
       "error: DXIL container mismatch for 'ViewIDState' between 'PSV0' "
       "part:('Outputs affected by ViewID as a bitmask for stream 0:",
       "   ViewID influencing Outputs[0] : 8  9  10 ",
       "PCOutputs affected by ViewID as a bitmask:",
       "  ViewID influencing PCOutputs :  None",
       "Outputs affected by inputs as a table of bitmasks for stream 0:",
       "Inputs contributing to computation of Outputs[0]:",
       "  Inputs[0] influencing Outputs[0] : 0 ",
       "  Inputs[1] influencing Outputs[0] : 1 ",
       "  Inputs[2] influencing Outputs[0] : 2 ",
       "  Inputs[3] influencing Outputs[0] : 3 ",
       "  Inputs[4] influencing Outputs[0] : 4 ",
       "  Inputs[5] influencing Outputs[0] : 5 ",
       "  Inputs[6] influencing Outputs[0] :  None",
       "  Inputs[7] influencing Outputs[0] :  None",
       "  Inputs[8] influencing Outputs[0] : 8 ",
       "  Inputs[9] influencing Outputs[0] : 9 ",
       "  Inputs[10] influencing Outputs[0] : 10 ",
       "  Inputs[11] influencing Outputs[0] :  None",
       "Patch constant outputs affected by inputs as a table of bitmasks:",
       "Inputs contributing to computation of PatchConstantOutputs:",
       "  Inputs[0] influencing PatchConstantOutputs :  None",
       "  Inputs[1] influencing PatchConstantOutputs :  None",
       "  Inputs[2] influencing PatchConstantOutputs :  None",
       "  Inputs[3] influencing PatchConstantOutputs :  None",
       "  Inputs[4] influencing PatchConstantOutputs :  None",
       "  Inputs[5] influencing PatchConstantOutputs :  None",
       "  Inputs[6] influencing PatchConstantOutputs :  None",
       "  Inputs[7] influencing PatchConstantOutputs :  None",
       "  Inputs[8] influencing PatchConstantOutputs :  None",
       "  Inputs[9] influencing PatchConstantOutputs :  None",
       "  Inputs[10] influencing PatchConstantOutputs :  None",
       "  Inputs[11] influencing PatchConstantOutputs :  None",
       "') and DXIL module:('Outputs affected by ViewID as a bitmask for "
       "stream 0:",
       "   ViewID influencing Outputs[0] : 8  9  10 ",
       "PCOutputs affected by ViewID as a bitmask:",
       "  ViewID influencing PCOutputs : 12 ",
       "Outputs affected by inputs as a table of bitmasks for stream 0:",
       "Inputs contributing to computation of Outputs[0]:",
       "  Inputs[0] influencing Outputs[0] : 0 ",
       "  Inputs[1] influencing Outputs[0] : 1 ",
       "  Inputs[2] influencing Outputs[0] : 2 ",
       "  Inputs[3] influencing Outputs[0] : 3 ",
       "  Inputs[4] influencing Outputs[0] : 4 ",
       "  Inputs[5] influencing Outputs[0] : 5 ",
       "  Inputs[6] influencing Outputs[0] :  None",
       "  Inputs[7] influencing Outputs[0] :  None",
       "  Inputs[8] influencing Outputs[0] : 8 ",
       "  Inputs[9] influencing Outputs[0] : 9 ",
       "  Inputs[10] influencing Outputs[0] : 10 ",
       "  Inputs[11] influencing Outputs[0] :  None",
       "Patch constant outputs affected by inputs as a table of bitmasks:",
       "Inputs contributing to computation of PatchConstantOutputs:",
       "  Inputs[0] influencing PatchConstantOutputs : 3 ",
       "  Inputs[1] influencing PatchConstantOutputs :  None",
       "  Inputs[2] influencing PatchConstantOutputs :  None",
       "  Inputs[3] influencing PatchConstantOutputs :  None",
       "  Inputs[4] influencing PatchConstantOutputs :  None",
       "  Inputs[5] influencing PatchConstantOutputs :  None",
       "  Inputs[6] influencing PatchConstantOutputs :  None",
       "  Inputs[7] influencing PatchConstantOutputs :  None",
       "  Inputs[8] influencing PatchConstantOutputs :  None",
       "  Inputs[9] influencing PatchConstantOutputs :  None",
       "  Inputs[10] influencing PatchConstantOutputs :  None",
       "  Inputs[11] influencing PatchConstantOutputs :  None",
       "')",
       "error: Container part 'Pipeline State Validation' does not match "
       "expected for module.",
       "Validation failed."},
      /*maySucceedAnyway*/ false, /*bRegex*/ false);
}

TEST_F(ValidationTest, PSVContentValidationDS) {
  if (!m_ver.m_InternalValidator)
    if (m_ver.SkipDxilVersion(1, 8))
      return;

  CComPtr<IDxcBlob> pProgram;
  CompileFile(L"..\\DXC\\dumpPSV_DS.hlsl", "ds_6_8", &pProgram);

  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pResult;
  unsigned Flags = 0;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pResult));
  // Make sure the validation was successful.
  HRESULT status;
  VERIFY_IS_NOT_NULL(pResult);
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);

  // Get PSV part.
  hlsl::DxilContainerHeader *pHeader;
  hlsl::DxilPartIterator pPartIter(nullptr, 0);
  pHeader = (hlsl::DxilContainerHeader *)pProgram->GetBufferPointer();
  pPartIter =
      std::find_if(hlsl::begin(pHeader), hlsl::end(pHeader),
                   hlsl::DxilPartIsType(hlsl::DFCC_PipelineStateValidation));
  VERIFY_ARE_NOT_EQUAL(hlsl::end(pHeader), pPartIter);

  const DxilPartHeader *pPSVPart = (const DxilPartHeader *)(*pPartIter);
  SimplePSV PSV(pPSVPart);

  // Update PSV.
  PSV.SigPatchConstOrPrim[0].InterpolationMode = 20;
  memset(PSV.PCInputToOutputTable.data(), 0,
         PSV.PCInputToOutputTable.size() * sizeof(uint32_t));

  // Run validation again.
  CComPtr<IDxcOperationResult> pUpdatedResult;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pUpdatedResult));
  // Make sure the validation was fail.
  VERIFY_IS_NOT_NULL(pUpdatedResult);
  VERIFY_SUCCEEDED(pUpdatedResult->GetStatus(&status));
  VERIFY_FAILED(status);

  CheckOperationResultMsgs(
      pUpdatedResult,
      {"error: DXIL container mismatch for 'SigPatchConstantOrPrimElement' "
       "between 'PSV0' part:('PSVSignatureElement:",
       "  SemanticName: ",
       "  SemanticIndex: 0 1 2 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 3",
       "  Rows: 3",
       "  Cols: 1",
       "  SemanticKind: TessFactor",
       "  InterpolationMode: 20",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "') and DXIL module:('PSVSignatureElement:",
       "  SemanticName: ",
       "  SemanticIndex: 0 1 2 ",
       "  IsAllocated: 1",
       "  StartRow: 0",
       "  StartCol: 3",
       "  Rows: 3",
       "  Cols: 1",
       "  SemanticKind: TessFactor",
       "  InterpolationMode: 0",
       "  OutputStream: 0",
       "  ComponentType: 3",
       "  DynamicIndexMask: 0",
       "')",
       "error: DXIL container mismatch for 'ViewIDState' between 'PSV0' "
       "part:('Outputs affected by inputs as a table of bitmasks for stream "
       "0:",
       "Inputs contributing to computation of Outputs[0]:",
       "  Inputs[0] influencing Outputs[0] : 0 ",
       "  Inputs[1] influencing Outputs[0] : 1 ",
       "  Inputs[2] influencing Outputs[0] : 2 ",
       "  Inputs[3] influencing Outputs[0] : 3 ",
       "  Inputs[4] influencing Outputs[0] : 4 ",
       "  Inputs[5] influencing Outputs[0] : 5 ",
       "  Inputs[6] influencing Outputs[0] :  None",
       "  Inputs[7] influencing Outputs[0] :  None",
       "  Inputs[8] influencing Outputs[0] : 8 ",
       "  Inputs[9] influencing Outputs[0] : 9 ",
       "  Inputs[10] influencing Outputs[0] : 10 ",
       "  Inputs[11] influencing Outputs[0] :  None",
       "  Inputs[12] influencing Outputs[0] :  None",
       "  Inputs[13] influencing Outputs[0] :  None",
       "  Inputs[14] influencing Outputs[0] :  None",
       "  Inputs[15] influencing Outputs[0] :  None",
       "Outputs affected by patch constant inputs as a table of bitmasks:",
       "PatchConstantInputs contributing to computation of Outputs:",
       "  PatchConstantInputs[0] influencing Outputs :  None",
       "  PatchConstantInputs[1] influencing Outputs :  None",
       "  PatchConstantInputs[2] influencing Outputs :  None",
       "  PatchConstantInputs[3] influencing Outputs :  None",
       "  PatchConstantInputs[4] influencing Outputs :  None",
       "  PatchConstantInputs[5] influencing Outputs :  None",
       "  PatchConstantInputs[6] influencing Outputs :  None",
       "  PatchConstantInputs[7] influencing Outputs :  None",
       "  PatchConstantInputs[8] influencing Outputs :  None",
       "  PatchConstantInputs[9] influencing Outputs :  None",
       "  PatchConstantInputs[10] influencing Outputs :  None",
       "  PatchConstantInputs[11] influencing Outputs :  None",
       "  PatchConstantInputs[12] influencing Outputs :  None",
       "  PatchConstantInputs[13] influencing Outputs :  None",
       "  PatchConstantInputs[14] influencing Outputs :  None",
       "  PatchConstantInputs[15] influencing Outputs :  None",
       "') and DXIL module:('Outputs affected by inputs as a table of "
       "bitmasks for stream 0:",
       "Inputs contributing to computation of Outputs[0]:",
       "  Inputs[0] influencing Outputs[0] : 0 ",
       "  Inputs[1] influencing Outputs[0] : 1 ",
       "  Inputs[2] influencing Outputs[0] : 2 ",
       "  Inputs[3] influencing Outputs[0] : 3 ",
       "  Inputs[4] influencing Outputs[0] : 4 ",
       "  Inputs[5] influencing Outputs[0] : 5 ",
       "  Inputs[6] influencing Outputs[0] :  None",
       "  Inputs[7] influencing Outputs[0] :  None",
       "  Inputs[8] influencing Outputs[0] : 8 ",
       "  Inputs[9] influencing Outputs[0] : 9 ",
       "  Inputs[10] influencing Outputs[0] : 10 ",
       "  Inputs[11] influencing Outputs[0] :  None",
       "  Inputs[12] influencing Outputs[0] :  None",
       "  Inputs[13] influencing Outputs[0] :  None",
       "  Inputs[14] influencing Outputs[0] :  None",
       "  Inputs[15] influencing Outputs[0] :  None",
       "Outputs affected by patch constant inputs as a table of bitmasks:",
       "PatchConstantInputs contributing to computation of Outputs:",
       "  PatchConstantInputs[0] influencing Outputs :  None",
       "  PatchConstantInputs[1] influencing Outputs :  None",
       "  PatchConstantInputs[2] influencing Outputs :  None",
       "  PatchConstantInputs[3] influencing Outputs : 4  5 ",
       "  PatchConstantInputs[4] influencing Outputs :  None",
       "  PatchConstantInputs[5] influencing Outputs :  None",
       "  PatchConstantInputs[6] influencing Outputs :  None",
       "  PatchConstantInputs[7] influencing Outputs : 0  1  2  3 ",
       "  PatchConstantInputs[8] influencing Outputs :  None",
       "  PatchConstantInputs[9] influencing Outputs :  None",
       "  PatchConstantInputs[10] influencing Outputs :  None",
       "  PatchConstantInputs[11] influencing Outputs :  None",
       "  PatchConstantInputs[12] influencing Outputs : 8  9  10 ",
       "  PatchConstantInputs[13] influencing Outputs :  None",
       "  PatchConstantInputs[14] influencing Outputs :  None",
       "  PatchConstantInputs[15] influencing Outputs :  None",
       "')",
       "error: Container part 'Pipeline State Validation' does not match "
       "expected for module.",
       "Validation failed."},
      /*maySucceedAnyway*/ false, /*bRegex*/ false);
}

TEST_F(ValidationTest, PSVContentValidationGS) {
  if (!m_ver.m_InternalValidator)
    if (m_ver.SkipDxilVersion(1, 8))
      return;

  CComPtr<IDxcBlob> pProgram;
  CompileFile(L"..\\DXC\\dumpPSV_GS.hlsl", "gs_6_8", &pProgram);

  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pResult;
  unsigned Flags = 0;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pResult));
  // Make sure the validation was successful.
  HRESULT status;
  VERIFY_IS_NOT_NULL(pResult);
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);

  // Get PSV part.
  hlsl::DxilContainerHeader *pHeader;
  hlsl::DxilPartIterator pPartIter(nullptr, 0);
  pHeader = (hlsl::DxilContainerHeader *)pProgram->GetBufferPointer();
  pPartIter =
      std::find_if(hlsl::begin(pHeader), hlsl::end(pHeader),
                   hlsl::DxilPartIsType(hlsl::DFCC_PipelineStateValidation));
  VERIFY_ARE_NOT_EQUAL(hlsl::end(pHeader), pPartIter);

  const DxilPartHeader *pPSVPart = (const DxilPartHeader *)(*pPartIter);
  SimplePSV PSV(pPSVPart);
  // Update PSV.
  PSV.PSVInfo->MaxVertexCount = 2;

  // Run validation again.
  CComPtr<IDxcOperationResult> pUpdatedResult;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pUpdatedResult));
  // Make sure the validation was fail.
  VERIFY_IS_NOT_NULL(pUpdatedResult);
  VERIFY_SUCCEEDED(pUpdatedResult->GetStatus(&status));
  VERIFY_FAILED(status);

  CheckOperationResultMsgs(
      pUpdatedResult,
      {"error: DXIL container mismatch for 'PSVRuntimeInfo' between 'PSV0' "
       "part:('PSVRuntimeInfo:",
       " Geometry Shader",
       " InputPrimitive=point",
       " OutputTopology=triangle",
       " OutputStreamMask=1",
       " OutputPositionPresent=1",
       " MinimumExpectedWaveLaneCount: 0",
       " MaximumExpectedWaveLaneCount: 4294967295",
       " UsesViewID: false",
       " SigInputElements: 3",
       " SigOutputElements: 3",
       " SigPatchConstOrPrimElements: 0",
       " SigInputVectors: 3",
       " SigOutputVectors[0]: 3",
       " SigOutputVectors[1]: 0",
       " SigOutputVectors[2]: 0",
       " SigOutputVectors[3]: 0",
       " EntryFunctionName: main",
       "') and DXIL module:('PSVRuntimeInfo:",
       " Geometry Shader",
       " InputPrimitive=point",
       " OutputTopology=triangle",
       " OutputStreamMask=1",
       " OutputPositionPresent=1",
       " MinimumExpectedWaveLaneCount: 0",
       " MaximumExpectedWaveLaneCount: 4294967295",
       " UsesViewID: false",
       " SigInputElements: 3",
       " SigOutputElements: 3",
       " SigPatchConstOrPrimElements: 0",
       " SigInputVectors: 3",
       " SigOutputVectors[0]: 3",
       " SigOutputVectors[1]: 0",
       " SigOutputVectors[2]: 0",
       " SigOutputVectors[3]: 0",
       " EntryFunctionName: main",
       "')",
       "error: Container part 'Pipeline State Validation' does not match "
       "expected for module.",
       "Validation failed."},
      /*maySucceedAnyway*/ false, /*bRegex*/ false);
}

TEST_F(ValidationTest, PSVContentValidationPS) {
  if (!m_ver.m_InternalValidator)
    if (m_ver.SkipDxilVersion(1, 8))
      return;

  CComPtr<IDxcBlob> pProgram;
  CompileFile(L"..\\DXC\\dumpPSV_PS.hlsl", "ps_6_8", &pProgram);

  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pResult;
  unsigned Flags = 0;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pResult));
  // Make sure the validation was successful.
  HRESULT status;
  VERIFY_IS_NOT_NULL(pResult);
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);

  // Get PSV part.
  hlsl::DxilContainerHeader *pHeader;
  hlsl::DxilPartIterator pPartIter(nullptr, 0);
  pHeader = (hlsl::DxilContainerHeader *)pProgram->GetBufferPointer();
  pPartIter =
      std::find_if(hlsl::begin(pHeader), hlsl::end(pHeader),
                   hlsl::DxilPartIsType(hlsl::DFCC_PipelineStateValidation));
  VERIFY_ARE_NOT_EQUAL(hlsl::end(pHeader), pPartIter);

  const DxilPartHeader *pPSVPart = (const DxilPartHeader *)(*pPartIter);
  SimplePSV PSV(pPSVPart);

  // Update PSV.
  PSV.PSVInfo->PS.DepthOutput = 1;

  // Run validation again.
  CComPtr<IDxcOperationResult> pUpdatedResult;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pUpdatedResult));
  // Make sure the validation was fail.
  VERIFY_IS_NOT_NULL(pUpdatedResult);
  VERIFY_SUCCEEDED(pUpdatedResult->GetStatus(&status));
  VERIFY_FAILED(status);

  CheckOperationResultMsgs(
      pUpdatedResult,
      {"error: DXIL container mismatch for 'PSVRuntimeInfo' between 'PSV0' "
       "part:('PSVRuntimeInfo:",
       " Pixel Shader",
       " DepthOutput=0",
       " SampleFrequency=1",
       " MinimumExpectedWaveLaneCount: 0",
       " MaximumExpectedWaveLaneCount: 4294967295",
       " UsesViewID: false",
       " SigInputElements: 2",
       " SigOutputElements: 1",
       " SigPatchConstOrPrimElements: 0",
       " SigInputVectors: 2",
       " SigOutputVectors[0]: 1",
       " SigOutputVectors[1]: 0",
       " SigOutputVectors[2]: 0",
       " SigOutputVectors[3]: 0",
       " EntryFunctionName: main",
       "') and DXIL module:('PSVRuntimeInfo:",
       " Pixel Shader",
       " DepthOutput=1",
       " SampleFrequency=1",
       " MinimumExpectedWaveLaneCount: 0",
       " MaximumExpectedWaveLaneCount: 4294967295",
       " UsesViewID: false",
       " SigInputElements: 2",
       " SigOutputElements: 1",
       " SigPatchConstOrPrimElements: 0",
       " SigInputVectors: 2",
       " SigOutputVectors[0]: 1",
       " SigOutputVectors[1]: 0",
       " SigOutputVectors[2]: 0",
       " SigOutputVectors[3]: 0",
       " EntryFunctionName: main",
       "')",
       "error: Container part 'Pipeline State Validation' does not match "
       "expected for module.",
       "Validation failed."},
      /*maySucceedAnyway*/ false, /*bRegex*/ false);
}

TEST_F(ValidationTest, PSVContentValidationCS) {
  if (!m_ver.m_InternalValidator)
    if (m_ver.SkipDxilVersion(1, 8))
      return;

  CComPtr<IDxcBlob> pProgram;
  CompileFile(L"..\\DXC\\dumpPSV_CS.hlsl", "cs_6_8", &pProgram);

  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pResult;
  unsigned Flags = 0;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pResult));
  // Make sure the validation was successful.
  HRESULT status;
  VERIFY_IS_NOT_NULL(pResult);
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);

  // Get PSV part.
  hlsl::DxilContainerHeader *pHeader;
  hlsl::DxilPartIterator pPartIter(nullptr, 0);
  pHeader = (hlsl::DxilContainerHeader *)pProgram->GetBufferPointer();
  pPartIter =
      std::find_if(hlsl::begin(pHeader), hlsl::end(pHeader),
                   hlsl::DxilPartIsType(hlsl::DFCC_PipelineStateValidation));
  VERIFY_ARE_NOT_EQUAL(hlsl::end(pHeader), pPartIter);

  const DxilPartHeader *pPSVPart = (const DxilPartHeader *)(*pPartIter);
  SimplePSV PSV(pPSVPart);
  // Update PSV.
  PSV.PSVInfo->NumThreadsX = 1;

  // Run validation again.
  CComPtr<IDxcOperationResult> pUpdatedResult;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pUpdatedResult));
  // Make sure the validation was fail.
  VERIFY_IS_NOT_NULL(pUpdatedResult);
  VERIFY_SUCCEEDED(pUpdatedResult->GetStatus(&status));
  VERIFY_FAILED(status);

  CheckOperationResultMsgs(
      pUpdatedResult,
      {"error: DXIL container mismatch for 'PSVRuntimeInfo' between 'PSV0' "
       "part:('PSVRuntimeInfo:",
       " Compute Shader",
       " NumThreads=(128,1,1)",
       " MinimumExpectedWaveLaneCount: 0",
       " MaximumExpectedWaveLaneCount: 4294967295",
       " UsesViewID: false",
       " SigInputElements: 0",
       " SigOutputElements: 0",
       " SigPatchConstOrPrimElements: 0",
       " SigInputVectors: 0",
       " SigOutputVectors[0]: 0",
       " SigOutputVectors[1]: 0",
       " SigOutputVectors[2]: 0",
       " SigOutputVectors[3]: 0",
       " EntryFunctionName: main",
       "') and DXIL module:('PSVRuntimeInfo:",
       " Compute Shader",
       " NumThreads=(1,1,1)",
       " MinimumExpectedWaveLaneCount: 0",
       " MaximumExpectedWaveLaneCount: 4294967295",
       " UsesViewID: false",
       " SigInputElements: 0",
       " SigOutputElements: 0",
       " SigPatchConstOrPrimElements: 0",
       " SigInputVectors: 0",
       " SigOutputVectors[0]: 0",
       " SigOutputVectors[1]: 0",
       " SigOutputVectors[2]: 0",
       " SigOutputVectors[3]: 0",
       " EntryFunctionName: main",
       "')",
       "error: Container part 'Pipeline State Validation' does not match "
       "expected for module.",
       "Validation failed."},
      /*maySucceedAnyway*/ false, /*bRegex*/ false);
}

TEST_F(ValidationTest, PSVContentValidationMS) {
  if (!m_ver.m_InternalValidator)
    if (m_ver.SkipDxilVersion(1, 8))
      return;

  CComPtr<IDxcBlob> pProgram;
  CompileFile(L"..\\DXC\\dumpPSV_MS.hlsl", "ms_6_8", &pProgram);

  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pResult;
  unsigned Flags = 0;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pResult));
  // Make sure the validation was successful.
  HRESULT status;
  VERIFY_IS_NOT_NULL(pResult);
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);

  // Get PSV part.
  hlsl::DxilContainerHeader *pHeader;
  hlsl::DxilPartIterator pPartIter(nullptr, 0);
  pHeader = (hlsl::DxilContainerHeader *)pProgram->GetBufferPointer();
  pPartIter =
      std::find_if(hlsl::begin(pHeader), hlsl::end(pHeader),
                   hlsl::DxilPartIsType(hlsl::DFCC_PipelineStateValidation));
  VERIFY_ARE_NOT_EQUAL(hlsl::end(pHeader), pPartIter);

  const DxilPartHeader *pPSVPart = (const DxilPartHeader *)(*pPartIter);
  SimplePSV PSV(pPSVPart);
  // Update PSV.
  memset(PSV.ViewIDOutputMask[0].data(), 0,
         PSV.ViewIDOutputMask[0].size() * sizeof(uint32_t));
  memset(PSV.ViewIDPCOutputMask.data(), 0,
         PSV.ViewIDPCOutputMask.size() * sizeof(uint32_t));

  // Run validation again.
  CComPtr<IDxcOperationResult> pUpdatedResult;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pUpdatedResult));
  // Make sure the validation was fail.
  VERIFY_IS_NOT_NULL(pUpdatedResult);
  VERIFY_SUCCEEDED(pUpdatedResult->GetStatus(&status));
  VERIFY_FAILED(status);

  CheckOperationResultMsgs(
      pUpdatedResult,
      {"error: DXIL container mismatch for 'ViewIDState' between 'PSV0' "
       "part:('Outputs affected by ViewID as a bitmask for stream 0:",
       "   ViewID influencing Outputs[0] :  None",
       "PCOutputs affected by ViewID as a bitmask:",
       "  ViewID influencing PCOutputs :  None",
       "Outputs affected by inputs as a table of bitmasks for stream 0:",
       "Inputs contributing to computation of Outputs[0]:  None",
       "') and DXIL module:('Outputs affected by ViewID as a bitmask for "
       "stream 0:",
       "   ViewID influencing Outputs[0] : 0  1  2  3  4  8  12  16 ",
       "PCOutputs affected by ViewID as a bitmask:",
       "  ViewID influencing PCOutputs : 3 ",
       "Outputs affected by inputs as a table of bitmasks for stream 0:",
       "Inputs contributing to computation of Outputs[0]:  None", "')",
       "error: Container part 'Pipeline State Validation' does not match "
       "expected for module.",
       "Validation failed."},
      /*maySucceedAnyway*/ false, /*bRegex*/ false);
}

TEST_F(ValidationTest, PSVContentValidationAS) {
  if (!m_ver.m_InternalValidator)
    if (m_ver.SkipDxilVersion(1, 8))
      return;

  CComPtr<IDxcBlob> pProgram;
  CompileFile(L"..\\DXC\\dumpPSV_AS.hlsl", "as_6_8", &pProgram);

  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pResult;
  unsigned Flags = 0;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pResult));
  // Make sure the validation was successful.
  HRESULT status;
  VERIFY_IS_NOT_NULL(pResult);
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);

  // Get PSV part.
  hlsl::DxilContainerHeader *pHeader;
  hlsl::DxilPartIterator pPartIter(nullptr, 0);
  pHeader = (hlsl::DxilContainerHeader *)pProgram->GetBufferPointer();
  pPartIter =
      std::find_if(hlsl::begin(pHeader), hlsl::end(pHeader),
                   hlsl::DxilPartIsType(hlsl::DFCC_PipelineStateValidation));
  VERIFY_ARE_NOT_EQUAL(hlsl::end(pHeader), pPartIter);

  const DxilPartHeader *pPSVPart = (const DxilPartHeader *)(*pPartIter);
  SimplePSV PSV(pPSVPart);

  // Update PSV.
  PSV.PSVInfo->AS.PayloadSizeInBytes = 0;

  // Run validation again.
  CComPtr<IDxcOperationResult> pUpdatedResult;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pUpdatedResult));
  // Make sure the validation was fail.
  VERIFY_IS_NOT_NULL(pUpdatedResult);
  VERIFY_SUCCEEDED(pUpdatedResult->GetStatus(&status));
  VERIFY_FAILED(status);

  CheckOperationResultMsgs(
      pUpdatedResult,
      {"error: DXIL container mismatch for 'PSVRuntimeInfo' between 'PSV0' "
       "part:('PSVRuntimeInfo:",
       " Amplification Shader",
       " NumThreads=(32,1,1)",
       " MinimumExpectedWaveLaneCount: 0",
       " MaximumExpectedWaveLaneCount: 4294967295",
       " UsesViewID: false",
       " SigInputElements: 0",
       " SigOutputElements: 0",
       " SigPatchConstOrPrimElements: 0",
       " SigInputVectors: 0",
       " SigOutputVectors[0]: 0",
       " SigOutputVectors[1]: 0",
       " SigOutputVectors[2]: 0",
       " SigOutputVectors[3]: 0",
       " EntryFunctionName: main",
       "') and DXIL module:('PSVRuntimeInfo:",
       " Amplification Shader",
       " NumThreads=(32,1,1)",
       " MinimumExpectedWaveLaneCount: 0",
       " MaximumExpectedWaveLaneCount: 4294967295",
       " UsesViewID: false",
       " SigInputElements: 0",
       " SigOutputElements: 0",
       " SigPatchConstOrPrimElements: 0",
       " SigInputVectors: 0",
       " SigOutputVectors[0]: 0",
       " SigOutputVectors[1]: 0",
       " SigOutputVectors[2]: 0",
       " SigOutputVectors[3]: 0",
       " EntryFunctionName: main",
       "')",
       "error: Container part 'Pipeline State Validation' does not match "
       "expected for module.",
       "Validation failed."},
      /*maySucceedAnyway*/ false, /*bRegex*/ false);
}

struct SimpleContainer {
  hlsl::DxilContainerHeader *Header;
  std::vector<uint32_t> PartOffsets;
  std::vector<const DxilPartHeader *> PartHeaders;
  SimpleContainer(void *Ptr) {
    Header = (hlsl::DxilContainerHeader *)Ptr;
    hlsl::DxilPartIterator pPartIter(nullptr, 0);
    pPartIter = hlsl::begin(Header);
    for (unsigned i = 0; i < Header->PartCount; ++i) {
      VERIFY_IS_TRUE(pPartIter != hlsl::end(Header));
      PartOffsets.push_back(
          (uint32_t)((const char *)(*pPartIter) - (const char *)Header));
      PartHeaders.push_back((const DxilPartHeader *)(*pPartIter));
      ++pPartIter;
    }
    VERIFY_ARE_EQUAL(pPartIter, hlsl::end(Header));
  }
};

TEST_F(ValidationTest, WrongPSVSize) {
  if (!m_ver.m_InternalValidator)
    if (m_ver.SkipDxilVersion(1, 8))
      return;

  CComPtr<IDxcBlob> pProgram;
  CompileFile(L"..\\DXC\\dumpPSV_AS.hlsl", "as_6_8", &pProgram);

  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pResult;
  unsigned Flags = 0;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pResult));
  // Make sure the validation was successful.
  HRESULT status;
  VERIFY_IS_NOT_NULL(pResult);
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);

  hlsl::DxilContainerHeader *pHeader;
  hlsl::DxilPartIterator pPartIter(nullptr, 0);
  pHeader = (hlsl::DxilContainerHeader *)pProgram->GetBufferPointer();
  // Make sure the PSV part exists.
  pPartIter =
      std::find_if(hlsl::begin(pHeader), hlsl::end(pHeader),
                   hlsl::DxilPartIsType(hlsl::DFCC_PipelineStateValidation));
  VERIFY_ARE_NOT_EQUAL(hlsl::end(pHeader), pPartIter);

  // Create a new Blob which is 16 bytes larger than the original one.
  std::vector<char> pProgram2Data(pProgram->GetBufferSize() + 16, 0);
  // Copy data from the original blob part by part.
  // Copy all parts to program2.
  SimpleContainer Container(pProgram->GetBufferPointer());
  uint32_t PartOffsetsSize = pHeader->PartCount * sizeof(uint32_t);
  uint32_t Offset = sizeof(hlsl::DxilContainerHeader) + PartOffsetsSize;
  std::vector<uint32_t> PartOffsets;
  const uint32_t ExtraSize = 16;
  // copy all parts to program2.
  for (unsigned i = 0; i < pHeader->PartCount; ++i) {
    PartOffsets.emplace_back(Offset);
    const DxilPartHeader *pPartHeader = Container.PartHeaders[i];

    // Copy part header.
    memcpy(pProgram2Data.data() + Offset, pPartHeader, sizeof(DxilPartHeader));
    Offset += sizeof(DxilPartHeader);

    // Copy part content.
    uint32_t *PartPtr =
        const_cast<uint32_t *>((const uint32_t *)GetDxilPartData(pPartHeader));
    memcpy(pProgram2Data.data() + Offset, PartPtr, pPartHeader->PartSize);

    Offset += pPartHeader->PartSize;

    if (pPartHeader->PartFourCC == hlsl::DFCC_PipelineStateValidation) {
      // Update the size of PSV part.
      DxilPartHeader *pPSVPartHeader =
          (DxilPartHeader *)(pProgram2Data.data() + PartOffsets.back());
      pPSVPartHeader->PartSize += ExtraSize;
      Offset += ExtraSize;
    }
  }
  // Copy header.
  pHeader->ContainerSizeInBytes += ExtraSize;
  memcpy(pProgram2Data.data(), pHeader, sizeof(hlsl::DxilContainerHeader));
  // Copy partOffsets.
  memcpy(pProgram2Data.data() + sizeof(hlsl::DxilContainerHeader),
         PartOffsets.data(), PartOffsetsSize);

  // Create a new Blob from pProgram2Data.
  CComPtr<IDxcBlobEncoding> pProgram2;
  CComPtr<IDxcLibrary> pLibrary;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
  VERIFY_SUCCEEDED(pLibrary->CreateBlobWithEncodingFromPinned(
      pProgram2Data.data(), pProgram2Data.size(), CP_UTF8, &pProgram2));

  // Run validation on updated container.
  CComPtr<IDxcOperationResult> pUpdatedResult;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram2, Flags, &pUpdatedResult));
  // Make sure the validation was fail.
  VERIFY_IS_NOT_NULL(pUpdatedResult);
  VERIFY_SUCCEEDED(pUpdatedResult->GetStatus(&status));
  VERIFY_FAILED(status);

  CheckOperationResultMsgs(
      pUpdatedResult, {"In 'DxilContainer', 'PSV0 part' is not well-formed"},
      /*maySucceedAnyway*/ false, /*bRegex*/ false);
}

TEST_F(ValidationTest, WrongPSVSizeOnZeros) {
  if (!m_ver.m_InternalValidator)
    if (m_ver.SkipDxilVersion(1, 8))
      return;

  CComPtr<IDxcBlob> pProgram;
  CompileFile(L"..\\DXC\\dumpPSV_PS.hlsl", "ps_6_8", &pProgram);

  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pResult;
  unsigned Flags = 0;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pResult));
  // Make sure the validation was successful.
  HRESULT status;
  VERIFY_IS_NOT_NULL(pResult);
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);

  hlsl::DxilContainerHeader *pHeader;
  hlsl::DxilPartIterator pPartIter(nullptr, 0);
  pHeader = (hlsl::DxilContainerHeader *)pProgram->GetBufferPointer();
  // Make sure the PSV part exists.
  pPartIter =
      std::find_if(hlsl::begin(pHeader), hlsl::end(pHeader),
                   hlsl::DxilPartIsType(hlsl::DFCC_PipelineStateValidation));
  VERIFY_ARE_NOT_EQUAL(hlsl::end(pHeader), pPartIter);

  const DxilPartHeader *pPSVPart = (const DxilPartHeader *)(*pPartIter);
  const uint32_t *PSVPtr = (const uint32_t *)GetDxilPartData(pPSVPart);

  uint32_t PSVRuntimeInfo_size = *(PSVPtr++);
  VERIFY_ARE_EQUAL(sizeof(PSVRuntimeInfo3), PSVRuntimeInfo_size);
  PSVRuntimeInfo3 *PSVInfo =
      const_cast<PSVRuntimeInfo3 *>((const PSVRuntimeInfo3 *)PSVPtr);
  VERIFY_ARE_EQUAL(2u, PSVInfo->SigInputElements);
  PSVPtr += PSVRuntimeInfo_size / 4;
  uint32_t *ResourceCountPtr = const_cast<uint32_t *>(PSVPtr++);
  uint32_t ResourceCount = *ResourceCountPtr;
  VERIFY_ARE_NOT_EQUAL(0u, ResourceCount);
  uint32_t ResourceBindingsSize = *(PSVPtr++);
  PSVPtr += (ResourceCount * ResourceBindingsSize) / 4;
  uint32_t *StringTableSizePtr = const_cast<uint32_t *>(PSVPtr++);
  uint32_t StringTableSize = *StringTableSizePtr;
  // Skip string table.
  PSVPtr += StringTableSize / 4;
  uint32_t *SemanticIndexTableEntriesPtr = const_cast<uint32_t *>(PSVPtr++);
  uint32_t SemanticIndexTableEntries = *SemanticIndexTableEntriesPtr;

  *SemanticIndexTableEntriesPtr = 0;

  // Run validation on updated container.
  CComPtr<IDxcOperationResult> pUpdatedResult1;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pUpdatedResult1));
  // Make sure the validation was fail.
  VERIFY_IS_NOT_NULL(pUpdatedResult1);
  VERIFY_SUCCEEDED(pUpdatedResult1->GetStatus(&status));
  VERIFY_FAILED(status);

  CheckOperationResultMsgs(
      pUpdatedResult1, {"In 'DxilContainer', 'PSV0 part' is not well-formed"},
      /*maySucceedAnyway*/ false, /*bRegex*/ false);

  *SemanticIndexTableEntriesPtr = SemanticIndexTableEntries;
  *StringTableSizePtr = 0;

  // Run validation on updated container.
  CComPtr<IDxcOperationResult> pUpdatedResult2;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pUpdatedResult2));
  // Make sure the validation was fail.
  VERIFY_IS_NOT_NULL(pUpdatedResult2);
  VERIFY_SUCCEEDED(pUpdatedResult2->GetStatus(&status));
  VERIFY_FAILED(status);

  CheckOperationResultMsgs(
      pUpdatedResult2, {"In 'DxilContainer', 'PSV0 part' is not well-formed"},
      /*maySucceedAnyway*/ false, /*bRegex*/ false);

  *StringTableSizePtr = StringTableSize;
  *ResourceCountPtr = 0;

  // Run validation on updated container.
  CComPtr<IDxcOperationResult> pUpdatedResult3;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram, Flags, &pUpdatedResult3));
  // Make sure the validation was fail.
  VERIFY_IS_NOT_NULL(pUpdatedResult3);
  VERIFY_SUCCEEDED(pUpdatedResult3->GetStatus(&status));
  VERIFY_FAILED(status);

  CheckOperationResultMsgs(
      pUpdatedResult3, {"In 'DxilContainer', 'PSV0 part' is not well-formed"},
      /*maySucceedAnyway*/ false, /*bRegex*/ false);
  *ResourceCountPtr = ResourceCount;
}

TEST_F(ValidationTest, WrongPSVVersion) {
  if (!m_ver.m_InternalValidator)
    if (m_ver.SkipDxilVersion(1, 8))
      return;

  CComPtr<IDxcBlob> pProgram60;
  std::vector<LPCWSTR> args;
  args.emplace_back(L"-validator-version");
  args.emplace_back(L"1.0");
  CompileFile(L"..\\DXC\\dumpPSV_CS.hlsl", "cs_6_0", args.data(), args.size(),
              &pProgram60);

  CComPtr<IDxcValidator> pValidator;
  CComPtr<IDxcOperationResult> pResult;
  unsigned Flags = DxcValidatorFlags_InPlaceEdit;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcValidator, &pValidator));
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram60, Flags, &pResult));
  // Make sure the validation was successful.
  HRESULT status;
  VERIFY_IS_NOT_NULL(pResult);
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);

  hlsl::DxilContainerHeader *pHeader60;
  hlsl::DxilPartIterator pPartIter(nullptr, 0);
  pHeader60 = (hlsl::DxilContainerHeader *)pProgram60->GetBufferPointer();
  // Make sure the PSV part exists.
  pPartIter =
      std::find_if(hlsl::begin(pHeader60), hlsl::end(pHeader60),
                   hlsl::DxilPartIsType(hlsl::DFCC_PipelineStateValidation));
  VERIFY_ARE_NOT_EQUAL(hlsl::end(pHeader60), pPartIter);

  CComPtr<IDxcBlob> pProgram68;

  CompileFile(L"..\\DXC\\dumpPSV_CS.hlsl", "cs_6_8", &pProgram68);
  CComPtr<IDxcOperationResult> pResult2;
  VERIFY_SUCCEEDED(pValidator->Validate(pProgram68, Flags, &pResult2));
  // Make sure the validation was successful.
  VERIFY_IS_NOT_NULL(pResult);
  VERIFY_SUCCEEDED(pResult->GetStatus(&status));
  VERIFY_SUCCEEDED(status);

  hlsl::DxilContainerHeader *pHeader68;
  pHeader68 = (hlsl::DxilContainerHeader *)pProgram68->GetBufferPointer();
  // Make sure the PSV part exists.
  pPartIter =
      std::find_if(hlsl::begin(pHeader68), hlsl::end(pHeader68),
                   hlsl::DxilPartIsType(hlsl::DFCC_PipelineStateValidation));
  VERIFY_ARE_NOT_EQUAL(hlsl::end(pHeader68), pPartIter);

  // Switch the PSV part between 6.0 to 6.8.
  SimpleContainer Container60(pProgram60->GetBufferPointer());
  SimpleContainer Container68(pProgram68->GetBufferPointer());
  uint32_t Container60WithPSV68Size = sizeof(hlsl::DxilContainerHeader) +
                                      pHeader60->PartCount * sizeof(uint32_t);
  uint32_t Container68WithPSV60Size = sizeof(hlsl::DxilContainerHeader) +
                                      pHeader60->PartCount * sizeof(uint32_t);
  unsigned Container68PartSkipped = 0;
  for (unsigned i = 0; i < pHeader60->PartCount; ++i) {
    const DxilPartHeader *pPartHeader60 = Container60.PartHeaders[i];
    const DxilPartHeader *pPartHeader68 =
        Container68.PartHeaders[i + Container68PartSkipped];
    if (pPartHeader68->PartFourCC == hlsl::DFCC_ShaderHash) {
      Container68PartSkipped++;
      pPartHeader68 = Container68.PartHeaders[i + Container68PartSkipped];
    }

    VERIFY_ARE_EQUAL(pPartHeader60->PartFourCC, pPartHeader68->PartFourCC);
    Container60WithPSV68Size += sizeof(DxilPartHeader);
    Container68WithPSV60Size += sizeof(DxilPartHeader);
    if (pPartHeader60->PartFourCC == hlsl::DFCC_PipelineStateValidation) {
      Container60WithPSV68Size += pPartHeader68->PartSize;
      Container68WithPSV60Size += pPartHeader60->PartSize;
    } else {
      Container60WithPSV68Size += pPartHeader60->PartSize;
      Container68WithPSV60Size += pPartHeader68->PartSize;
    }
  }

  // Create mixed container.
  std::vector<char> pProgram60WithPSV68Data(Container60WithPSV68Size, 0);
  std::vector<char> pProgram68WithPSV60Data(Container68WithPSV60Size, 0);

  uint32_t PartOffsetsSize = pHeader60->PartCount * sizeof(uint32_t);
  uint32_t Offset60 = sizeof(hlsl::DxilContainerHeader) + PartOffsetsSize;
  std::vector<uint32_t> PartOffsets60;
  uint32_t Offset68 = sizeof(hlsl::DxilContainerHeader) + PartOffsetsSize;
  std::vector<uint32_t> PartOffsets68;
  Container68PartSkipped = 0;
  for (unsigned i = 0; i < pHeader60->PartCount; ++i) {
    PartOffsets60.emplace_back(Offset60);
    PartOffsets68.emplace_back(Offset68);

    const DxilPartHeader *pPartHeader60 = Container60.PartHeaders[i];
    const DxilPartHeader *pPartHeader68 =
        Container68.PartHeaders[i + Container68PartSkipped];
    if (pPartHeader68->PartFourCC == hlsl::DFCC_ShaderHash) {
      Container68PartSkipped++;
      pPartHeader68 = Container68.PartHeaders[i + Container68PartSkipped];
    }

    if (pPartHeader60->PartFourCC == hlsl::DFCC_PipelineStateValidation) {
      // Copy PSV part from 6.8 to 6.0.
      memcpy(pProgram60WithPSV68Data.data() + Offset60, pPartHeader68,
             sizeof(DxilPartHeader));
      Offset60 += sizeof(DxilPartHeader);
      memcpy(pProgram60WithPSV68Data.data() + Offset60,
             GetDxilPartData(pPartHeader68), pPartHeader68->PartSize);
      Offset60 += pPartHeader68->PartSize;
      // Copy PSV part from 6.0 to 6.8.
      memcpy(pProgram68WithPSV60Data.data() + Offset68, pPartHeader60,
             sizeof(DxilPartHeader));
      Offset68 += sizeof(DxilPartHeader);
      memcpy(pProgram68WithPSV60Data.data() + Offset68,
             GetDxilPartData(pPartHeader60), pPartHeader60->PartSize);

      Offset68 += pPartHeader60->PartSize;
    } else {
      // Copy PSV part from 6.0 to 6.0.
      memcpy(pProgram60WithPSV68Data.data() + Offset60, pPartHeader60,
             sizeof(DxilPartHeader));
      Offset60 += sizeof(DxilPartHeader);
      memcpy(pProgram60WithPSV68Data.data() + Offset60,
             GetDxilPartData(pPartHeader60), pPartHeader60->PartSize);
      Offset60 += pPartHeader60->PartSize;
      // Copy PSV part from 6.8 to 6.8.
      memcpy(pProgram68WithPSV60Data.data() + Offset68, pPartHeader68,
             sizeof(DxilPartHeader));
      Offset68 += sizeof(DxilPartHeader);
      memcpy(pProgram68WithPSV60Data.data() + Offset68,
             GetDxilPartData(pPartHeader68), pPartHeader68->PartSize);
      Offset68 += pPartHeader68->PartSize;
    }
  }

  // Copy header.
  VERIFY_ARE_EQUAL(Container60WithPSV68Size, Offset60);
  pHeader60->ContainerSizeInBytes = Container60WithPSV68Size;
  memcpy(pProgram60WithPSV68Data.data(), pHeader60,
         sizeof(hlsl::DxilContainerHeader));
  VERIFY_ARE_EQUAL(Container68WithPSV60Size, Offset68);
  pHeader68->ContainerSizeInBytes = Container68WithPSV60Size;
  pHeader68->PartCount -= Container68PartSkipped;
  memcpy(pProgram68WithPSV60Data.data(), pHeader68,
         sizeof(hlsl::DxilContainerHeader));
  // Copy partOffsets.
  memcpy(pProgram60WithPSV68Data.data() + sizeof(hlsl::DxilContainerHeader),
         PartOffsets60.data(), PartOffsetsSize);
  memcpy(pProgram68WithPSV60Data.data() + sizeof(hlsl::DxilContainerHeader),
         PartOffsets68.data(), PartOffsetsSize);

  // Create a new Blob.
  CComPtr<IDxcBlobEncoding> pProgram60WithPSV68;
  CComPtr<IDxcLibrary> pLibrary;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));
  VERIFY_SUCCEEDED(pLibrary->CreateBlobWithEncodingFromPinned(
      pProgram60WithPSV68Data.data(), pProgram60WithPSV68Data.size(), CP_UTF8,
      &pProgram60WithPSV68));

  // Run validation on new containers.
  CComPtr<IDxcOperationResult> p60WithPSV68Result;
  VERIFY_SUCCEEDED(
      pValidator->Validate(pProgram60WithPSV68, Flags, &p60WithPSV68Result));
  // Make sure the validation was fail.
  VERIFY_IS_NOT_NULL(p60WithPSV68Result);
  VERIFY_SUCCEEDED(p60WithPSV68Result->GetStatus(&status));
  VERIFY_FAILED(status);
  CheckOperationResultMsgs(
      p60WithPSV68Result,
      {"DXIL container mismatch for 'PSVRuntimeInfoSize' between 'PSV0' "
       "part:('52') and DXIL module:('24')"},
      /*maySucceedAnyway*/ false, /*bRegex*/ false);

  // Create a new Blob.
  CComPtr<IDxcBlobEncoding> pProgram68WithPSV60;
  VERIFY_SUCCEEDED(pLibrary->CreateBlobWithEncodingFromPinned(
      pProgram68WithPSV60Data.data(), pProgram68WithPSV60Data.size(), CP_UTF8,
      &pProgram68WithPSV60));
  CComPtr<IDxcOperationResult> p68WithPSV60Result;
  VERIFY_SUCCEEDED(
      pValidator->Validate(pProgram68WithPSV60, Flags, &p68WithPSV60Result));
  // Make sure the validation was fail.
  VERIFY_IS_NOT_NULL(p68WithPSV60Result);
  VERIFY_SUCCEEDED(p68WithPSV60Result->GetStatus(&status));
  VERIFY_FAILED(status);
  CheckOperationResultMsgs(
      p68WithPSV60Result,
      {"DXIL container mismatch for 'PSVRuntimeInfoSize' between 'PSV0' "
       "part:('24') and DXIL module:('52')"},
      /*maySucceedAnyway*/ false, /*bRegex*/ false);
}
