///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// PixTest.cpp                                                               //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides tests for the PIX-specific components                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef UNICODE
#define UNICODE
#endif

#include <algorithm>
#include <array>
#include <cassert>
#include <cfloat>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/DxilContainer/DxilRuntimeReflection.h"
#include "dxc/DxilRootSignature/DxilRootSignature.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/dxcapi.h"
#include "dxc/dxcpix.h"
#ifdef _WIN32
#include <atlfile.h>
#endif

#include "dxc/DXIL/DxilModule.h"

#include "dxc/Test/DxcTestUtils.h"
#include "dxc/Test/HLSLTestData.h"
#include "dxc/Test/HlslTestUtils.h"

#include "dxc/DXIL/DxilUtil.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/dxcapi.use.h"
#include "dxc/Support/microcom.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ModuleSlotTracker.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MSFileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include <fstream>

#include <../lib/DxilDia/DxcPixLiveVariables_FragmentIterator.h>
#include <dxc/DxilPIXPasses/DxilPIXVirtualRegisters.h>

#include "PixTestUtils.h"

using namespace std;
using namespace hlsl;
using namespace hlsl_test;
using namespace pix_test;

static std::vector<std::string> Tokenize(const std::string &str,
                                         const char *delimiters) {
  std::vector<std::string> tokens;
  std::string copy = str;

  for (auto i = strtok(&copy[0], delimiters); i != nullptr;
       i = strtok(nullptr, delimiters)) {
    tokens.push_back(i);
  }

  return tokens;
}

#ifdef _WIN32
class PixTest {
#else
class PixTest : public ::testing::Test {
#endif
public:
  BEGIN_TEST_CLASS(PixTest)
  TEST_CLASS_PROPERTY(L"Parallel", L"true")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_CLASS_SETUP(InitSupport);

  TEST_METHOD(DebugUAV_CS_6_1)
  TEST_METHOD(DebugUAV_CS_6_2)
  TEST_METHOD(DebugUAV_lib_6_3_through_6_8)

  TEST_METHOD(CompileDebugDisasmPDB)

  TEST_METHOD(AddToASPayload)
  TEST_METHOD(AddToASGroupSharedPayload)
  TEST_METHOD(AddToASGroupSharedPayload_MeshletCullSample)
  TEST_METHOD(SignatureModification_Empty)
  TEST_METHOD(SignatureModification_VertexIdAlready)
  TEST_METHOD(SignatureModification_SomethingElseFirst)

  TEST_METHOD(AccessTracking_ModificationReport_Nothing)
  TEST_METHOD(AccessTracking_ModificationReport_Read)
  TEST_METHOD(AccessTracking_ModificationReport_Write)
  TEST_METHOD(AccessTracking_ModificationReport_SM66)

  TEST_METHOD(PixStructAnnotation_Lib_DualRaygen)

  TEST_METHOD(PixStructAnnotation_Simple)
  TEST_METHOD(PixStructAnnotation_CopiedStruct)
  TEST_METHOD(PixStructAnnotation_MixedSizes)
  TEST_METHOD(PixStructAnnotation_StructWithinStruct)
  TEST_METHOD(PixStructAnnotation_1DArray)
  TEST_METHOD(PixStructAnnotation_2DArray)
  TEST_METHOD(PixStructAnnotation_EmbeddedArray)
  TEST_METHOD(PixStructAnnotation_FloatN)
  TEST_METHOD(PixStructAnnotation_SequentialFloatN)
  TEST_METHOD(PixStructAnnotation_EmbeddedFloatN)
  TEST_METHOD(PixStructAnnotation_Matrix)
  TEST_METHOD(PixStructAnnotation_MemberFunction)
  TEST_METHOD(PixStructAnnotation_BigMess)
  TEST_METHOD(PixStructAnnotation_AlignedFloat4Arrays)
  TEST_METHOD(PixStructAnnotation_Inheritance)
  TEST_METHOD(PixStructAnnotation_ResourceAsMember)
  TEST_METHOD(PixStructAnnotation_WheresMyDbgValue)

  TEST_METHOD(VirtualRegisters_InstructionCounts)
  TEST_METHOD(VirtualRegisters_AlignedOffsets)

  TEST_METHOD(RootSignatureUpgrade_SubObjects)
  TEST_METHOD(RootSignatureUpgrade_Annotation)

  TEST_METHOD(DxilPIXDXRInvocationsLog_SanityTest)
  TEST_METHOD(DxilPIXDXRInvocationsLog_EmbeddedRootSigs)

  TEST_METHOD(DebugInstrumentation_TextOutput)
  TEST_METHOD(DebugInstrumentation_BlockReport)

  TEST_METHOD(DebugInstrumentation_VectorAllocaWrite_Structs)

  TEST_METHOD(NonUniformResourceIndex_Resource)
  TEST_METHOD(NonUniformResourceIndex_DescriptorHeap)
  TEST_METHOD(NonUniformResourceIndex_Raytracing)

  dxc::DxcDllSupport m_dllSupport;
  VersionSupportInfo m_ver;

  HRESULT CreateContainerBuilder(IDxcContainerBuilder **ppResult) {
    return m_dllSupport.CreateInstance(CLSID_DxcContainerBuilder, ppResult);
  }

  std::string GetOption(std::string &cmd, char *opt) {
    std::string option = cmd.substr(cmd.find(opt));
    option = option.substr(option.find_first_of(' '));
    option = option.substr(option.find_first_not_of(' '));
    return option.substr(0, option.find_first_of(' '));
  }

  CComPtr<IDxcBlob> ExtractDxilPart(IDxcBlob *pProgram) {
    CComPtr<IDxcLibrary> pLib;
    VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));
    const hlsl::DxilContainerHeader *pContainer = hlsl::IsDxilContainerLike(
        pProgram->GetBufferPointer(), pProgram->GetBufferSize());
    VERIFY_IS_NOT_NULL(pContainer);
    hlsl::DxilPartIterator partIter =
        std::find_if(hlsl::begin(pContainer), hlsl::end(pContainer),
                     hlsl::DxilPartIsType(hlsl::DFCC_DXIL));
    const hlsl::DxilProgramHeader *pProgramHeader =
        (const hlsl::DxilProgramHeader *)hlsl::GetDxilPartData(*partIter);
    uint32_t bitcodeLength;
    const char *pBitcode;
    CComPtr<IDxcBlob> pDxilBits;
    hlsl::GetDxilProgramBitcode(pProgramHeader, &pBitcode, &bitcodeLength);
    VERIFY_SUCCEEDED(pLib->CreateBlobFromBlob(
        pProgram, pBitcode - (char *)pProgram->GetBufferPointer(),
        bitcodeLength, &pDxilBits));
    return pDxilBits;
  }

  PassOutput RunValueToDeclarePass(IDxcBlob *dxil, int startingLineNumber = 0) {
    CComPtr<IDxcOptimizer> pOptimizer;
    VERIFY_SUCCEEDED(
        m_dllSupport.CreateInstance(CLSID_DxcOptimizer, &pOptimizer));
    std::vector<LPCWSTR> Options;
    Options.push_back(L"-opt-mod-passes");
    Options.push_back(L"-dxil-dbg-value-to-dbg-declare");

    CComPtr<IDxcBlob> pOptimizedModule;
    CComPtr<IDxcBlobEncoding> pText;
    VERIFY_SUCCEEDED(pOptimizer->RunOptimizer(
        dxil, Options.data(), Options.size(), &pOptimizedModule, &pText));

    std::string outputText;
    if (pText->GetBufferSize() != 0) {
      outputText = reinterpret_cast<const char *>(pText->GetBufferPointer());
    }

    return {
        std::move(pOptimizedModule), {}, Tokenize(outputText.c_str(), "\n")};
  }

  PassOutput RunDebugPass(IDxcBlob *dxil, int UAVSize = 1024 * 1024) {
    CComPtr<IDxcOptimizer> pOptimizer;
    VERIFY_SUCCEEDED(
        m_dllSupport.CreateInstance(CLSID_DxcOptimizer, &pOptimizer));
    std::vector<LPCWSTR> Options;
    Options.push_back(L"-opt-mod-passes");
    Options.push_back(L"-dxil-dbg-value-to-dbg-declare");
    Options.push_back(L"-dxil-annotate-with-virtual-regs");
    std::wstring debugArg =
        L"-hlsl-dxil-debug-instrumentation,UAVSize=" + std::to_wstring(UAVSize);
    Options.push_back(debugArg.c_str());
    Options.push_back(L"-viewid-state");
    Options.push_back(L"-hlsl-dxilemit");

    CComPtr<IDxcBlob> pOptimizedModule;
    CComPtr<IDxcBlobEncoding> pText;
    VERIFY_SUCCEEDED(pOptimizer->RunOptimizer(
        dxil, Options.data(), Options.size(), &pOptimizedModule, &pText));

    std::string outputText = BlobToUtf8(pText);

    return {
        std::move(pOptimizedModule), {}, Tokenize(outputText.c_str(), "\n")};
  }

  CComPtr<IDxcBlob> FindModule(hlsl::DxilFourCC fourCC, IDxcBlob *pSource) {
    const UINT32 BC_C0DE = ((INT32)(INT8)'B' | (INT32)(INT8)'C' << 8 |
                            (INT32)0xDEC0 << 16); // BC0xc0de in big endian
    const char *pBitcode = nullptr;
    const hlsl::DxilPartHeader *pDxilPartHeader =
        (hlsl::DxilPartHeader *)
            pSource->GetBufferPointer(); // Initialize assuming that source is
                                         // starting with DXIL part

    if (BC_C0DE == *(UINT32 *)pSource->GetBufferPointer()) {
      return pSource;
    }
    if (hlsl::IsValidDxilContainer(
            (hlsl::DxilContainerHeader *)pSource->GetBufferPointer(),
            pSource->GetBufferSize())) {
      hlsl::DxilContainerHeader *pDxilContainerHeader =
          (hlsl::DxilContainerHeader *)pSource->GetBufferPointer();
      pDxilPartHeader =
          *std::find_if(begin(pDxilContainerHeader), end(pDxilContainerHeader),
                        hlsl::DxilPartIsType(fourCC));
    }
    if (fourCC == pDxilPartHeader->PartFourCC) {
      UINT32 pBlobSize;
      const hlsl::DxilProgramHeader *pDxilProgramHeader =
          (const hlsl::DxilProgramHeader *)(pDxilPartHeader + 1);
      hlsl::GetDxilProgramBitcode(pDxilProgramHeader, &pBitcode, &pBlobSize);
      UINT32 offset =
          (UINT32)(pBitcode - (const char *)pSource->GetBufferPointer());
      CComPtr<IDxcLibrary> library;
      IFT(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &library));
      CComPtr<IDxcBlob> targetBlob;
      library->CreateBlobFromBlob(pSource, offset, pBlobSize, &targetBlob);
      return targetBlob;
    }
    return {};
  }

  void ReplaceDxilBlobPart(const void *originalShaderBytecode,
                           SIZE_T originalShaderLength, IDxcBlob *pNewDxilBlob,
                           IDxcBlob **ppNewShaderOut) {
    CComPtr<IDxcLibrary> pLibrary;
    IFT(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLibrary));

    CComPtr<IDxcBlob> pNewContainer;

    // Use the container assembler to build a new container from the
    // recently-modified DXIL bitcode. This container will contain new copies of
    // things like input signature etc., which will supersede the ones from the
    // original compiled shader's container.
    {
      CComPtr<IDxcAssembler> pAssembler;
      IFT(m_dllSupport.CreateInstance(CLSID_DxcAssembler, &pAssembler));

      CComPtr<IDxcOperationResult> pAssembleResult;
      VERIFY_SUCCEEDED(
          pAssembler->AssembleToContainer(pNewDxilBlob, &pAssembleResult));

      CComPtr<IDxcBlobEncoding> pAssembleErrors;
      VERIFY_SUCCEEDED(pAssembleResult->GetErrorBuffer(&pAssembleErrors));

      if (pAssembleErrors && pAssembleErrors->GetBufferSize() != 0) {
        OutputDebugStringA(
            static_cast<LPCSTR>(pAssembleErrors->GetBufferPointer()));
        VERIFY_SUCCEEDED(E_FAIL);
      }

      VERIFY_SUCCEEDED(pAssembleResult->GetResult(&pNewContainer));
    }

    // Now copy over the blobs from the original container that won't have been
    // invalidated by changing the shader code itself, using the container
    // reflection API
    {
      // Wrap the original code in a container blob
      CComPtr<IDxcBlobEncoding> pContainer;
      VERIFY_SUCCEEDED(pLibrary->CreateBlobWithEncodingFromPinned(
          static_cast<LPBYTE>(const_cast<void *>(originalShaderBytecode)),
          static_cast<UINT32>(originalShaderLength), CP_ACP, &pContainer));

      CComPtr<IDxcContainerReflection> pReflection;
      IFT(m_dllSupport.CreateInstance(CLSID_DxcContainerReflection,
                                      &pReflection));

      // Load the reflector from the original shader
      VERIFY_SUCCEEDED(pReflection->Load(pContainer));

      UINT32 partIndex;

      if (SUCCEEDED(pReflection->FindFirstPartKind(hlsl::DFCC_PrivateData,
                                                   &partIndex))) {
        CComPtr<IDxcBlob> pPart;
        VERIFY_SUCCEEDED(pReflection->GetPartContent(partIndex, &pPart));

        CComPtr<IDxcContainerBuilder> pContainerBuilder;
        IFT(m_dllSupport.CreateInstance(CLSID_DxcContainerBuilder,
                                        &pContainerBuilder));

        VERIFY_SUCCEEDED(pContainerBuilder->Load(pNewContainer));

        VERIFY_SUCCEEDED(
            pContainerBuilder->AddPart(hlsl::DFCC_PrivateData, pPart));

        CComPtr<IDxcOperationResult> pBuildResult;

        VERIFY_SUCCEEDED(pContainerBuilder->SerializeContainer(&pBuildResult));

        CComPtr<IDxcBlobEncoding> pBuildErrors;
        VERIFY_SUCCEEDED(pBuildResult->GetErrorBuffer(&pBuildErrors));

        if (pBuildErrors && pBuildErrors->GetBufferSize() != 0) {
          OutputDebugStringA(
              reinterpret_cast<LPCSTR>(pBuildErrors->GetBufferPointer()));
          VERIFY_SUCCEEDED(E_FAIL);
        }

        VERIFY_SUCCEEDED(pBuildResult->GetResult(&pNewContainer));
      }
    }

    *ppNewShaderOut = pNewContainer.Detach();
  }

  void ValidateAccessTrackingMods(const char *hlsl, bool modsExpected);

  class ModuleAndHangersOn {
    std::unique_ptr<llvm::LLVMContext> llvmContext;
    std::unique_ptr<llvm::Module> llvmModule;
    DxilModule *dxilModule;

  public:
    ModuleAndHangersOn(IDxcBlob *pBlob) {

      // Assume we were given a dxil part first:
      const DxilProgramHeader *pProgramHeader =
          reinterpret_cast<const DxilProgramHeader *>(
              pBlob->GetBufferPointer());
      uint32_t partSize = static_cast<uint32_t>(pBlob->GetBufferSize());
      // Check if we were given a valid dxil container instead:
      const DxilContainerHeader *pContainer = IsDxilContainerLike(
          pBlob->GetBufferPointer(), pBlob->GetBufferSize());
      if (pContainer != nullptr) {
        VERIFY_IS_TRUE(
            IsValidDxilContainer(pContainer, pBlob->GetBufferSize()));

        // Get Dxil part from container.
        DxilPartIterator it =
            std::find_if(begin(pContainer), end(pContainer),
                         DxilPartIsType(DFCC_ShaderDebugInfoDXIL));
        VERIFY_IS_FALSE(it == end(pContainer));

        pProgramHeader =
            reinterpret_cast<const DxilProgramHeader *>(GetDxilPartData(*it));
        partSize = (*it)->PartSize;
      }

      VERIFY_IS_TRUE(IsValidDxilProgramHeader(pProgramHeader, partSize));

      // Get a pointer to the llvm bitcode.
      const char *pIL;
      uint32_t pILLength;
      GetDxilProgramBitcode(pProgramHeader, &pIL, &pILLength);

      // Parse llvm bitcode into a module.
      std::unique_ptr<llvm::MemoryBuffer> pBitcodeBuf(
          llvm::MemoryBuffer::getMemBuffer(llvm::StringRef(pIL, pILLength), "",
                                           false));

      llvmContext.reset(new llvm::LLVMContext);

      llvm::ErrorOr<std::unique_ptr<llvm::Module>> pModule(
          llvm::parseBitcodeFile(pBitcodeBuf->getMemBufferRef(), *llvmContext));
      if (std::error_code ec = pModule.getError()) {
        VERIFY_FAIL();
      }

      llvmModule = std::move(pModule.get());

      dxilModule = DxilModule::TryGetDxilModule(llvmModule.get());
    }

    DxilModule &GetDxilModule() { return *dxilModule; }
  };

  struct AggregateOffsetAndSize {
    unsigned countOfMembers;
    unsigned offset;
    unsigned size;
  };
  struct AllocaWrite {
    std::string memberName;
    uint32_t regBase;
    uint32_t regSize;
    uint64_t index;
  };
  struct TestableResults {
    std::vector<AggregateOffsetAndSize> OffsetAndSizes;
    std::vector<AllocaWrite> AllocaWrites;
  };

  TestableResults TestStructAnnotationCase(const char *hlsl,
                                           const wchar_t *optimizationLevel,
                                           bool validateCoverage = true,
                                           const wchar_t *profile = L"as_6_5");
  void ValidateAllocaWrite(std::vector<AllocaWrite> const &allocaWrites,
                           size_t index, const char *name);
  PassOutput RunShaderAccessTrackingPass(IDxcBlob *blob);
  std::string RunDxilPIXAddTidToAmplificationShaderPayloadPass(IDxcBlob *blob);
  CComPtr<IDxcBlob> RunDxilPIXMeshShaderOutputPass(IDxcBlob *blob);
  CComPtr<IDxcBlob> RunDxilPIXDXRInvocationsLog(IDxcBlob *blob);
  std::vector<std::string>
  RunDxilNonUniformResourceIndexInstrumentation(IDxcBlob *blob,
                                                std::string &outputText);
  void TestNuriCase(const char *source, const wchar_t *target,
                    uint32_t expectedResult);
  void TestPixUAVCase(char const *hlsl, wchar_t const *model,
                      wchar_t const *entry);
  std::string Disassemble(IDxcBlob *pProgram);
};

bool PixTest::InitSupport() {
  if (!m_dllSupport.IsEnabled()) {
    VERIFY_SUCCEEDED(m_dllSupport.Initialize());
    m_ver.Initialize(m_dllSupport);
  }
  return true;
}

void PixTest::TestPixUAVCase(char const *hlsl, wchar_t const *model,
                             wchar_t const *entry) {
  auto mod = Compile(m_dllSupport, hlsl, model, {}, entry);
  CComPtr<IDxcBlob> dxilPart = FindModule(DFCC_ShaderDebugInfoDXIL, mod);
  PassOutput passOutput = RunDebugPass(dxilPart);
  CComPtr<IDxcBlob> modifiedDxilContainer;
  ReplaceDxilBlobPart(mod->GetBufferPointer(), mod->GetBufferSize(),
                      passOutput.blob, &modifiedDxilContainer);

  ModuleAndHangersOn moduleEtc(modifiedDxilContainer);
  auto &compilerGeneratedUAV = moduleEtc.GetDxilModule().GetUAV(0);
  auto &pixDebugGeneratedUAV = moduleEtc.GetDxilModule().GetUAV(1);
  VERIFY_ARE_EQUAL(compilerGeneratedUAV.GetClass(),
                   pixDebugGeneratedUAV.GetClass());
  VERIFY_ARE_EQUAL(compilerGeneratedUAV.GetKind(),
                   pixDebugGeneratedUAV.GetKind());
  VERIFY_ARE_EQUAL(compilerGeneratedUAV.GetHLSLType(),
                   pixDebugGeneratedUAV.GetHLSLType());
  VERIFY_ARE_EQUAL(compilerGeneratedUAV.GetSampleCount(),
                   pixDebugGeneratedUAV.GetSampleCount());
  VERIFY_ARE_EQUAL(compilerGeneratedUAV.GetElementStride(),
                   pixDebugGeneratedUAV.GetElementStride());
  VERIFY_ARE_EQUAL(compilerGeneratedUAV.GetBaseAlignLog2(),
                   pixDebugGeneratedUAV.GetBaseAlignLog2());
  VERIFY_ARE_EQUAL(compilerGeneratedUAV.GetCompType(),
                   pixDebugGeneratedUAV.GetCompType());
  VERIFY_ARE_EQUAL(compilerGeneratedUAV.GetSamplerFeedbackType(),
                   pixDebugGeneratedUAV.GetSamplerFeedbackType());
  VERIFY_ARE_EQUAL(compilerGeneratedUAV.IsGloballyCoherent(),
                   pixDebugGeneratedUAV.IsGloballyCoherent());
  VERIFY_ARE_EQUAL(compilerGeneratedUAV.HasCounter(),
                   pixDebugGeneratedUAV.HasCounter());
  VERIFY_ARE_EQUAL(compilerGeneratedUAV.HasAtomic64Use(),
                   pixDebugGeneratedUAV.HasAtomic64Use());

  VERIFY_ARE_EQUAL(compilerGeneratedUAV.GetGlobalSymbol()->getType(),
                   pixDebugGeneratedUAV.GetGlobalSymbol()->getType());
}

TEST_F(PixTest, DebugUAV_CS_6_1) {
  const char *hlsl = R"(
RWByteAddressBuffer RawUAV : register(u0);
[numthreads(1, 1, 1)]
void CSMain()
{
    RawUAV.Store(0, RawUAV.Load(4));
}
)";
  TestPixUAVCase(hlsl, L"cs_6_1", L"CSMain");
}

TEST_F(PixTest, DebugUAV_CS_6_2) {
  const char *hlsl = R"(
RWByteAddressBuffer RawUAV : register(u0);
[numthreads(1, 1, 1)]
void CSMain()
{
    RawUAV.Store(0, RawUAV.Load(4));
}
)";
  // In 6.2, rawBufferLoad replaced bufferLoad for UAVs, but we don't
  // expect this test to notice the difference. We just test 6.2
  TestPixUAVCase(hlsl, L"cs_6_2", L"CSMain");
}

TEST_F(PixTest, DebugUAV_lib_6_3_through_6_8) {
  const char *hlsl = R"(
RWByteAddressBuffer RawUAV : register(u0);
struct [raypayload] Payload
{
  double a : read(caller, closesthit, anyhit) : write(caller, miss, closesthit);
};
[shader("miss")]
void Miss( inout Payload payload ) 
{ 
    RawUAV.Store(0, RawUAV.Load(4));
    payload.a = 4.2;
})";
  TestPixUAVCase(hlsl, L"lib_6_3", L"");
  TestPixUAVCase(hlsl, L"lib_6_4", L"");
  TestPixUAVCase(hlsl, L"lib_6_5", L"");

  if (m_ver.SkipDxilVersion(1, 6))
    return;
  TestPixUAVCase(hlsl, L"lib_6_6", L"");

  if (m_ver.SkipDxilVersion(1, 7))
    return;
  TestPixUAVCase(hlsl, L"lib_6_7", L"");

  if (m_ver.SkipDxilVersion(1, 8))
    return;
  TestPixUAVCase(hlsl, L"lib_6_8", L"");
}

TEST_F(PixTest, CompileDebugDisasmPDB) {
  const char *hlsl = R"(
    [RootSignature("")]
    float main(float pos : A) : SV_Target {
      float x = abs(pos);
      float y = sin(pos);
      float z = x + y;
      return z;
    }
  )";
  CComPtr<IDxcLibrary> pLib;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcLibrary, &pLib));

  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcCompiler2> pCompiler2;

  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  CComPtr<IDxcBlob> pProgram;
  CComPtr<IDxcBlob> pPdbBlob;
  CComHeapPtr<WCHAR> pDebugName;

  VERIFY_SUCCEEDED(CreateCompiler(m_dllSupport, &pCompiler));
  VERIFY_SUCCEEDED(pCompiler.QueryInterface(&pCompiler2));
  CreateBlobFromText(m_dllSupport, hlsl, &pSource);
  LPCWSTR args[] = {L"/Zi", L"/Qembed_debug"};
  VERIFY_SUCCEEDED(pCompiler2->CompileWithDebug(
      pSource, L"source.hlsl", L"main", L"ps_6_0", args, _countof(args),
      nullptr, 0, nullptr, &pResult, &pDebugName, &pPdbBlob));
  VERIFY_SUCCEEDED(pResult->GetResult(&pProgram));

  // Test that disassembler can consume a PDB container
  CComPtr<IDxcBlobEncoding> pDisasm;
  VERIFY_SUCCEEDED(pCompiler->Disassemble(pPdbBlob, &pDisasm));
}

PassOutput PixTest::RunShaderAccessTrackingPass(IDxcBlob *blob) {
  CComPtr<IDxcOptimizer> pOptimizer;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcOptimizer, &pOptimizer));
  std::vector<LPCWSTR> Options;
  Options.push_back(L"-opt-mod-passes");
  Options.push_back(L"-hlsl-dxil-pix-shader-access-instrumentation,config=U0:0:"
                    L"10i0;U0:1:2i0;.0;0;0.");

  CComPtr<IDxcBlob> pOptimizedModule;
  CComPtr<IDxcBlobEncoding> pText;
  VERIFY_SUCCEEDED(pOptimizer->RunOptimizer(
      blob, Options.data(), Options.size(), &pOptimizedModule, &pText));

  CComPtr<IDxcAssembler> pAssembler;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcAssembler, &pAssembler));

  CComPtr<IDxcOperationResult> pAssembleResult;
  VERIFY_SUCCEEDED(
      pAssembler->AssembleToContainer(pOptimizedModule, &pAssembleResult));

  HRESULT hr;
  VERIFY_SUCCEEDED(pAssembleResult->GetStatus(&hr));
  VERIFY_SUCCEEDED(hr);

  CComPtr<IDxcBlob> pNewContainer;
  VERIFY_SUCCEEDED(pAssembleResult->GetResult(&pNewContainer));

  PassOutput ret;
  ret.blob = pNewContainer;
  std::string outputText = BlobToUtf8(pText);
  ret.lines = Tokenize(outputText.c_str(), "\n");

  return ret;
}

CComPtr<IDxcBlob> PixTest::RunDxilPIXMeshShaderOutputPass(IDxcBlob *blob) {
  CComPtr<IDxcBlob> dxil = FindModule(DFCC_ShaderDebugInfoDXIL, blob);
  CComPtr<IDxcOptimizer> pOptimizer;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcOptimizer, &pOptimizer));
  std::vector<LPCWSTR> Options;
  Options.push_back(L"-opt-mod-passes");
  Options.push_back(L"-hlsl-dxil-pix-meshshader-output-instrumentation,expand-"
                    L"payload=1,UAVSize=8192");

  CComPtr<IDxcBlob> pOptimizedModule;
  CComPtr<IDxcBlobEncoding> pText;
  VERIFY_SUCCEEDED(pOptimizer->RunOptimizer(
      dxil, Options.data(), Options.size(), &pOptimizedModule, &pText));

  std::string outputText;
  if (pText->GetBufferSize() != 0) {
    outputText = reinterpret_cast<const char *>(pText->GetBufferPointer());
  }

  return pOptimizedModule;
}

CComPtr<IDxcBlob> PixTest::RunDxilPIXDXRInvocationsLog(IDxcBlob *blob) {

  CComPtr<IDxcBlob> dxil = FindModule(DFCC_ShaderDebugInfoDXIL, blob);
  CComPtr<IDxcOptimizer> pOptimizer;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcOptimizer, &pOptimizer));
  std::vector<LPCWSTR> Options;
  Options.push_back(
      L"-hlsl-dxil-pix-dxr-invocations-log,maxNumEntriesInLog=24");

  CComPtr<IDxcBlob> pOptimizedModule;
  CComPtr<IDxcBlobEncoding> pText;
  VERIFY_SUCCEEDED(pOptimizer->RunOptimizer(
      blob, Options.data(), Options.size(), &pOptimizedModule, &pText));

  std::string outputText;
  if (pText->GetBufferSize() != 0) {
    outputText = reinterpret_cast<const char *>(pText->GetBufferPointer());
  }

  return pOptimizedModule;
}

std::vector<std::string> PixTest::RunDxilNonUniformResourceIndexInstrumentation(
    IDxcBlob *blob, std::string &outputText) {

  CComPtr<IDxcBlob> dxil = FindModule(DFCC_ShaderDebugInfoDXIL, blob);
  CComPtr<IDxcOptimizer> pOptimizer;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcOptimizer, &pOptimizer));
  std::array<LPCWSTR, 4> Options = {
      L"-opt-mod-passes", L"-dxil-dbg-value-to-dbg-declare",
      L"-dxil-annotate-with-virtual-regs",
      L"-hlsl-dxil-non-uniform-resource-index-instrumentation"};

  CComPtr<IDxcBlob> pOptimizedModule;
  CComPtr<IDxcBlobEncoding> pText;
  VERIFY_SUCCEEDED(pOptimizer->RunOptimizer(
      dxil, Options.data(), Options.size(), &pOptimizedModule, &pText));

  outputText = BlobToUtf8(pText);

  const std::string disassembly = Disassemble(pOptimizedModule);
  return Tokenize(disassembly, "\n");
}

std::string
PixTest::RunDxilPIXAddTidToAmplificationShaderPayloadPass(IDxcBlob *blob) {
  CComPtr<IDxcBlob> dxil = FindModule(DFCC_ShaderDebugInfoDXIL, blob);
  CComPtr<IDxcOptimizer> pOptimizer;
  VERIFY_SUCCEEDED(
      m_dllSupport.CreateInstance(CLSID_DxcOptimizer, &pOptimizer));
  std::vector<LPCWSTR> Options;
  Options.push_back(L"-opt-mod-passes");
  Options.push_back(
      L"-hlsl-dxil-PIX-add-tid-to-as-payload,dispatchArgY=1,dispatchArgZ=2");

  CComPtr<IDxcBlob> pOptimizedModule;
  CComPtr<IDxcBlobEncoding> pText;
  VERIFY_SUCCEEDED(pOptimizer->RunOptimizer(
      dxil, Options.data(), Options.size(), &pOptimizedModule, &pText));

  std::string outputText;
  if (pText->GetBufferSize() != 0) {
    outputText = reinterpret_cast<const char *>(pText->GetBufferPointer());
  }

  return outputText;
}

TEST_F(PixTest, AddToASPayload) {

  const char *hlsl = R"(
struct MyPayload
{
    float f1;
    float f2;
};

[numthreads(1, 1, 1)]
void ASMain(uint gid : SV_GroupID)
{
    MyPayload payload;
    payload.f1 = (float)gid / 4.f;
    payload.f2 = (float)gid * 4.f;
    DispatchMesh(1, 1, 1, payload);
}

struct PSInput
{
    float4 position : SV_POSITION;
};


[outputtopology("triangle")]
[numthreads(3,1,1)]
void MSMain(
    in payload MyPayload small,
    in uint tid : SV_GroupThreadID,
    in uint3 dtid : SV_DispatchThreadID,
    out vertices PSInput verts[3],
    out indices uint3 triangles[1])
{
    SetMeshOutputCounts(3, 1);
    verts[tid].position = float4(small.f1, small.f2, 0, 0);
    triangles[0] = uint3(0, 1, 2);
}

  )";

  auto as = Compile(m_dllSupport, hlsl, L"as_6_6", {}, L"ASMain");
  RunDxilPIXAddTidToAmplificationShaderPayloadPass(as);

  auto ms = Compile(m_dllSupport, hlsl, L"ms_6_6", {}, L"MSMain");
  RunDxilPIXMeshShaderOutputPass(ms);
}
unsigned FindOrAddVSInSignatureElementForInstanceOrVertexID(
    hlsl::DxilSignature &InputSignature, hlsl::DXIL::SemanticKind semanticKind);

TEST_F(PixTest, SignatureModification_Empty) {

  DxilSignature sig(DXIL::ShaderKind::Vertex, DXIL::SignatureKind::Input,
                    false);

  FindOrAddVSInSignatureElementForInstanceOrVertexID(
      sig, DXIL::SemanticKind::InstanceID);
  FindOrAddVSInSignatureElementForInstanceOrVertexID(
      sig, DXIL::SemanticKind::VertexID);

  VERIFY_ARE_EQUAL(2ull, sig.GetElements().size());
  VERIFY_ARE_EQUAL(sig.GetElement(0).GetKind(), DXIL::SemanticKind::InstanceID);
  VERIFY_ARE_EQUAL(sig.GetElement(0).GetCols(), 1u);
  VERIFY_ARE_EQUAL(sig.GetElement(0).GetRows(), 1u);
  VERIFY_ARE_EQUAL(sig.GetElement(0).GetStartCol(), 0);
  VERIFY_ARE_EQUAL(sig.GetElement(0).GetStartRow(), 0);
  VERIFY_ARE_EQUAL(sig.GetElement(1).GetKind(), DXIL::SemanticKind::VertexID);
  VERIFY_ARE_EQUAL(sig.GetElement(1).GetCols(), 1u);
  VERIFY_ARE_EQUAL(sig.GetElement(1).GetRows(), 1u);
  VERIFY_ARE_EQUAL(sig.GetElement(1).GetStartCol(), 0);
  VERIFY_ARE_EQUAL(sig.GetElement(1).GetStartRow(), 1);
}

TEST_F(PixTest, SignatureModification_VertexIdAlready) {

  DxilSignature sig(DXIL::ShaderKind::Vertex, DXIL::SignatureKind::Input,
                    false);

  auto AddedElement =
      llvm::make_unique<DxilSignatureElement>(DXIL::SigPointKind::VSIn);
  AddedElement->Initialize(
      Semantic::Get(DXIL::SemanticKind::VertexID)->GetName(),
      hlsl::CompType::getU32(), DXIL::InterpolationMode::Constant, 1, 1, 0, 0,
      0, {0});
  AddedElement->SetKind(DXIL::SemanticKind::VertexID);
  AddedElement->SetUsageMask(1);
  sig.AppendElement(std::move(AddedElement));

  FindOrAddVSInSignatureElementForInstanceOrVertexID(
      sig, DXIL::SemanticKind::InstanceID);
  FindOrAddVSInSignatureElementForInstanceOrVertexID(
      sig, DXIL::SemanticKind::VertexID);

  VERIFY_ARE_EQUAL(2ull, sig.GetElements().size());
  VERIFY_ARE_EQUAL(sig.GetElement(0).GetKind(), DXIL::SemanticKind::VertexID);
  VERIFY_ARE_EQUAL(sig.GetElement(0).GetCols(), 1u);
  VERIFY_ARE_EQUAL(sig.GetElement(0).GetRows(), 1u);
  VERIFY_ARE_EQUAL(sig.GetElement(0).GetStartCol(), 0);
  VERIFY_ARE_EQUAL(sig.GetElement(0).GetStartRow(), 0);
  VERIFY_ARE_EQUAL(sig.GetElement(1).GetKind(), DXIL::SemanticKind::InstanceID);
  VERIFY_ARE_EQUAL(sig.GetElement(1).GetCols(), 1u);
  VERIFY_ARE_EQUAL(sig.GetElement(1).GetRows(), 1u);
  VERIFY_ARE_EQUAL(sig.GetElement(1).GetStartCol(), 0);
  VERIFY_ARE_EQUAL(sig.GetElement(1).GetStartRow(), 1);
}

TEST_F(PixTest, SignatureModification_SomethingElseFirst) {

  DxilSignature sig(DXIL::ShaderKind::Vertex, DXIL::SignatureKind::Input,
                    false);

  auto AddedElement =
      llvm::make_unique<DxilSignatureElement>(DXIL::SigPointKind::VSIn);
  AddedElement->Initialize("One", hlsl::CompType::getU32(),
                           DXIL::InterpolationMode::Constant, 1, 6, 0, 0, 0,
                           {0});
  AddedElement->SetKind(DXIL::SemanticKind::Arbitrary);
  AddedElement->SetUsageMask(1);
  sig.AppendElement(std::move(AddedElement));

  FindOrAddVSInSignatureElementForInstanceOrVertexID(
      sig, DXIL::SemanticKind::InstanceID);
  FindOrAddVSInSignatureElementForInstanceOrVertexID(
      sig, DXIL::SemanticKind::VertexID);

  VERIFY_ARE_EQUAL(3ull, sig.GetElements().size());
  // Not gonna check the first one cuz that would just be grading our own
  // homework
  VERIFY_ARE_EQUAL(sig.GetElement(1).GetKind(), DXIL::SemanticKind::InstanceID);
  VERIFY_ARE_EQUAL(sig.GetElement(1).GetCols(), 1u);
  VERIFY_ARE_EQUAL(sig.GetElement(1).GetRows(), 1u);
  VERIFY_ARE_EQUAL(sig.GetElement(1).GetStartCol(), 0);
  VERIFY_ARE_EQUAL(sig.GetElement(1).GetStartRow(), 1);
  VERIFY_ARE_EQUAL(sig.GetElement(2).GetKind(), DXIL::SemanticKind::VertexID);
  VERIFY_ARE_EQUAL(sig.GetElement(2).GetCols(), 1u);
  VERIFY_ARE_EQUAL(sig.GetElement(2).GetRows(), 1u);
  VERIFY_ARE_EQUAL(sig.GetElement(2).GetStartCol(), 0);
  VERIFY_ARE_EQUAL(sig.GetElement(2).GetStartRow(), 2);
}

void PixTest::ValidateAccessTrackingMods(const char *hlsl, bool modsExpected) {
  auto code = Compile(m_dllSupport, hlsl, L"ps_6_6", {L"-Od"}, L"main");
  auto result = RunShaderAccessTrackingPass(code).lines;
  bool hasMods = true;
  for (auto const &line : result)
    if (line.find("NotModified") != std::string::npos)
      hasMods = false;
  VERIFY_ARE_EQUAL(modsExpected, hasMods);
}

TEST_F(PixTest, AccessTracking_ModificationReport_Nothing) {
  const char *hlsl = R"(
float main() : SV_Target 
{
  return 0;
}
)";
  ValidateAccessTrackingMods(hlsl, false);
}

TEST_F(PixTest, AccessTracking_ModificationReport_Read) {
  const char *hlsl = R"(
RWByteAddressBuffer g_texture;
float main() : SV_Target 
{
  return g_texture.Load(0);
}
)";
  ValidateAccessTrackingMods(hlsl, true);
}

TEST_F(PixTest, AccessTracking_ModificationReport_Write) {
  const char *hlsl = R"(
RWByteAddressBuffer g_texture;
float main() : SV_Target 
{
  g_texture.Store(0, 0);
  return 0;
}
)";
  ValidateAccessTrackingMods(hlsl, true);
}

TEST_F(PixTest, AccessTracking_ModificationReport_SM66) {
  const char *hlsl = R"(
float main() : SV_Target 
{
    RWByteAddressBuffer g_texture = ResourceDescriptorHeap[0];
    g_texture.Store(0, 0);
    return 0;
}
)";
  ValidateAccessTrackingMods(hlsl, true);
}

TEST_F(PixTest, AddToASGroupSharedPayload) {

  const char *hlsl = R"(
struct Contained
{
    uint j;
    float af[3];
};

struct Bigger
{
    half h;
    void Init() { h = 1.f; }
  Contained a[2];
};

struct MyPayload
{
    uint i;
    Bigger big[3];
};

groupshared MyPayload payload;

[numthreads(1, 1, 1)]
void main(uint gid : SV_GroupID)
{
  DispatchMesh(1, 1, 1, payload);
}

  )";

  auto as = Compile(m_dllSupport, hlsl, L"as_6_6", {L"-Od"}, L"main");
  RunDxilPIXAddTidToAmplificationShaderPayloadPass(as);
}

TEST_F(PixTest, AddToASGroupSharedPayload_MeshletCullSample) {

  const char *hlsl = R"(
struct MyPayload
{
    uint i[32];
};

groupshared MyPayload payload;

[numthreads(1, 1, 1)]
void main(uint gid : SV_GroupID)
{
  DispatchMesh(1, 1, 1, payload);
}

  )";

  auto as = Compile(m_dllSupport, hlsl, L"as_6_6", {L"-Od"}, L"main");
  RunDxilPIXAddTidToAmplificationShaderPayloadPass(as);
}
static llvm::DIType *PeelTypedefs(llvm::DIType *diTy) {
  using namespace llvm;
  const llvm::DITypeIdentifierMap EmptyMap;
  while (1) {
    DIDerivedType *diDerivedTy = dyn_cast<DIDerivedType>(diTy);
    if (!diDerivedTy)
      return diTy;

    switch (diTy->getTag()) {
    case dwarf::DW_TAG_member:
    case dwarf::DW_TAG_inheritance:
    case dwarf::DW_TAG_typedef:
    case dwarf::DW_TAG_reference_type:
    case dwarf::DW_TAG_const_type:
    case dwarf::DW_TAG_restrict_type:
      diTy = diDerivedTy->getBaseType().resolve(EmptyMap);
      break;
    default:
      return diTy;
    }
  }

  return diTy;
}

static unsigned GetDITypeSizeInBits(llvm::DIType *diTy) {
  return PeelTypedefs(diTy)->getSizeInBits();
}

static unsigned GetDITypeAlignmentInBits(llvm::DIType *diTy) {
  return PeelTypedefs(diTy)->getAlignInBits();
}

static bool FindStructMemberFromStore(llvm::StoreInst *S,
                                      std::string *OutMemberName) {
  using namespace llvm;
  Value *Ptr = S->getPointerOperand();
  AllocaInst *Alloca = nullptr;

  auto &DL = S->getModule()->getDataLayout();

  unsigned OffsetInAlloca = 0;
  while (Ptr) {
    if (auto AI = dyn_cast<AllocaInst>(Ptr)) {
      Alloca = AI;
      break;
    } else if (auto Gep = dyn_cast<GEPOperator>(Ptr)) {
      if (Gep->getNumIndices() < 2 || !Gep->hasAllConstantIndices() ||
          0 != cast<ConstantInt>(Gep->getOperand(1))->getLimitedValue()) {
        return false;
      }

      auto GepSrcPtr = Gep->getPointerOperand();
      Type *GepSrcPtrTy = GepSrcPtr->getType()->getPointerElementType();

      Type *PeelingType = GepSrcPtrTy;
      for (unsigned i = 1; i < Gep->getNumIndices(); i++) {
        uint64_t Idx =
            cast<ConstantInt>(Gep->getOperand(1 + i))->getLimitedValue();

        if (PeelingType->isStructTy()) {
          auto StructTy = cast<StructType>(PeelingType);
          unsigned Offset =
              DL.getStructLayout(StructTy)->getElementOffsetInBits(Idx);
          OffsetInAlloca += Offset;
          PeelingType = StructTy->getElementType(Idx);
        } else if (PeelingType->isVectorTy()) {
          OffsetInAlloca +=
              DL.getTypeSizeInBits(PeelingType->getVectorElementType()) * Idx;
          PeelingType = PeelingType->getVectorElementType();
        } else if (PeelingType->isArrayTy()) {
          OffsetInAlloca +=
              DL.getTypeSizeInBits(PeelingType->getArrayElementType()) * Idx;
          PeelingType = PeelingType->getArrayElementType();
        } else {
          return false;
        }
      }

      Ptr = GepSrcPtr;
    } else {
      return false;
    }
  }

  // If there's not exactly one dbg.* inst, give up for now.
  if (hlsl::dxilutil::mdv_user_empty(Alloca) ||
      std::next(hlsl::dxilutil::mdv_users_begin(Alloca)) !=
          hlsl::dxilutil::mdv_users_end(Alloca)) {
    return false;
  }

  auto DI = dyn_cast<DbgDeclareInst>(*hlsl::dxilutil::mdv_users_begin(Alloca));
  if (!DI)
    return false;

  DILocalVariable *diVar = DI->getVariable();
  DIExpression *diExpr = DI->getExpression();
  const llvm::DITypeIdentifierMap EmptyMap;
  DIType *diType = diVar->getType().resolve(EmptyMap);

  unsigned MemberOffset = OffsetInAlloca;
  if (diExpr->isBitPiece()) {
    MemberOffset += diExpr->getBitPieceOffset();
  }

  diType = PeelTypedefs(diType);
  if (!isa<DICompositeType>(diType))
    return false;

  unsigned OffsetInDI = 0;
  std::string MemberName;

  //=====================================================
  // Find the correct member based on size
  while (diType) {
    diType = PeelTypedefs(diType);
    if (DICompositeType *diCompType = dyn_cast<DICompositeType>(diType)) {
      if (diCompType->getTag() == dwarf::DW_TAG_structure_type ||
          diCompType->getTag() == dwarf::DW_TAG_class_type) {
        bool FoundCompositeMember = false;
        for (DINode *Elem : diCompType->getElements()) {
          auto diElemType = dyn_cast<DIType>(Elem);
          if (!diElemType)
            return false;

          StringRef CurMemberName;
          if (diElemType->getTag() == dwarf::DW_TAG_member) {
            CurMemberName = diElemType->getName();
          } else if (diElemType->getTag() == dwarf::DW_TAG_inheritance) {
          } else {
            return false;
          }

          unsigned CompositeMemberSize = GetDITypeSizeInBits(diElemType);
          unsigned CompositeMemberAlignment =
              GetDITypeAlignmentInBits(diElemType);

          assert(CompositeMemberAlignment);
          OffsetInDI =
              llvm::RoundUpToAlignment(OffsetInDI, CompositeMemberAlignment);

          if (OffsetInDI <= MemberOffset &&
              MemberOffset < OffsetInDI + CompositeMemberSize) {
            diType = diElemType;
            if (CurMemberName.size()) {
              if (MemberName.size())
                MemberName += ".";
              MemberName += CurMemberName;
            }
            FoundCompositeMember = true;
            break;
          }

          // TODO: How will we match up the padding?
          OffsetInDI += CompositeMemberSize;
        }

        if (!FoundCompositeMember)
          return false;
      }
      // For arrays, just flatten it for now.
      // TODO: multi-dimension array
      else if (diCompType->getTag() == dwarf::DW_TAG_array_type) {
        if (MemberOffset < OffsetInDI ||
            MemberOffset >= OffsetInDI + diCompType->getSizeInBits())
          return false;
        DIType *diArrayElemType = diCompType->getBaseType().resolve(EmptyMap);

        {
          unsigned CurSize = diCompType->getSizeInBits();
          unsigned CurOffset = MemberOffset - OffsetInDI;
          for (DINode *SubrangeMD : diCompType->getElements()) {
            DISubrange *Range = cast<DISubrange>(SubrangeMD);

            unsigned ElemSize = CurSize / Range->getCount();
            unsigned Idx = CurOffset / ElemSize;

            CurOffset -= ElemSize * Idx;
            CurSize = ElemSize;

            MemberName += "[";
            MemberName += std::to_string(Idx);
            MemberName += "]";
          }
        }

        unsigned ArrayElemSize = GetDITypeSizeInBits(diArrayElemType);
        unsigned FlattenedIdx = (MemberOffset - OffsetInDI) / ArrayElemSize;
        OffsetInDI += FlattenedIdx * ArrayElemSize;
        diType = diArrayElemType;
      } else {
        return false;
      }
    } else if (DIBasicType *diBasicType = dyn_cast<DIBasicType>(diType)) {
      if (OffsetInDI == MemberOffset) {
        *OutMemberName = MemberName;
        return true;
      }

      OffsetInDI += diBasicType->getSizeInBits();
      return false;
    } else {
      return false;
    }
  }

  return false;
}

std::string PixTest::Disassemble(IDxcBlob *pProgram) {
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcOperationResult> pResult;
  CComPtr<IDxcBlobEncoding> pSource;
  VERIFY_SUCCEEDED(CreateCompiler(m_dllSupport, &pCompiler));
  CComPtr<IDxcBlobEncoding> pDisassembly;
  VERIFY_SUCCEEDED(pCompiler->Disassemble(pProgram, &pDisassembly));
  return BlobToUtf8(pDisassembly);
}

// This function lives in lib\DxilPIXPasses\DxilAnnotateWithVirtualRegister.cpp
// Declared here so we can test it.
uint32_t CountStructMembers(llvm::Type const *pType);

PixTest::TestableResults PixTest::TestStructAnnotationCase(
    const char *hlsl, const wchar_t *optimizationLevel, bool validateCoverage,
    const wchar_t *profile) {
  CComPtr<IDxcBlob> pBlob =
      Compile(m_dllSupport, hlsl, profile,
              {optimizationLevel, L"-HV", L"2018", L"-enable-16bit-types"});

  CComPtr<IDxcBlob> pDxil = FindModule(DFCC_ShaderDebugInfoDXIL, pBlob);

  PassOutput passOutput = RunAnnotationPasses(m_dllSupport, pDxil);

  auto pAnnotated = passOutput.blob;

  CComPtr<IDxcBlob> pAnnotatedContainer;
  ReplaceDxilBlobPart(pBlob->GetBufferPointer(), pBlob->GetBufferSize(),
                      pAnnotated, &pAnnotatedContainer);

#if 0 // handy for debugging
  auto disTextW = Disassemble(pAnnotatedContainer);
#endif

  ModuleAndHangersOn moduleEtc(pAnnotatedContainer);
  PixTest::TestableResults ret;

  // For every dbg.declare, run the member iterator and record what it finds:
  auto entryPoints = moduleEtc.GetDxilModule().GetExportedFunctions();
  for (auto &entryFunction : entryPoints) {
    for (auto &block : entryFunction->getBasicBlockList()) {
      for (auto &instruction : block.getInstList()) {
        if (auto *dbgDeclare =
                llvm::dyn_cast<llvm::DbgDeclareInst>(&instruction)) {
          llvm::Value *Address = dbgDeclare->getAddress();
          auto *AddressAsAlloca = llvm::dyn_cast<llvm::AllocaInst>(Address);
          if (AddressAsAlloca != nullptr) {
            auto *Expression = dbgDeclare->getExpression();

            std::unique_ptr<dxil_debug_info::MemberIterator> iterator =
                dxil_debug_info::CreateMemberIterator(
                    dbgDeclare,
                    moduleEtc.GetDxilModule().GetModule()->getDataLayout(),
                    AddressAsAlloca, Expression);

            unsigned int startingBit = 0;
            unsigned int coveredBits = 0;
            unsigned int memberIndex = 0;
            unsigned int memberCount = 0;
            while (iterator->Next(&memberIndex)) {
              memberCount++;
              if (memberIndex == 0) {
                startingBit = iterator->OffsetInBits(memberIndex);
                coveredBits = iterator->SizeInBits(memberIndex);
              } else {
                coveredBits = std::max<unsigned int>(
                    coveredBits, iterator->OffsetInBits(memberIndex) +
                                     iterator->SizeInBits(memberIndex));
              }
            }

            AggregateOffsetAndSize OffsetAndSize = {};
            OffsetAndSize.countOfMembers = memberCount;
            OffsetAndSize.offset = startingBit;
            OffsetAndSize.size = coveredBits;
            ret.OffsetAndSizes.push_back(OffsetAndSize);

            // Use this independent count of number of struct members to test
            // the function that operates on the alloca type:
            llvm::Type *pAllocaTy =
                AddressAsAlloca->getType()->getElementType();
            if (auto *AT = llvm::dyn_cast<llvm::ArrayType>(pAllocaTy)) {
              // This is the case where a struct is passed to a function, and
              // in these tests there should be only one struct behind the
              // pointer.
              VERIFY_ARE_EQUAL(AT->getNumElements(), 1u);
              pAllocaTy = AT->getArrayElementType();
            }

            if (auto *ST = llvm::dyn_cast<llvm::StructType>(pAllocaTy)) {
              uint32_t countOfMembers = CountStructMembers(ST);
              // memberIndex might be greater, because the fragment iterator
              // also includes contained derived types as fragments, in
              // addition to the members of that contained derived types.
              // CountStructMembers only counts the leaf-node types.
              VERIFY_ARE_EQUAL(countOfMembers, memberCount);
            } else if (pAllocaTy->isFloatingPointTy() ||
                       pAllocaTy->isIntegerTy()) {
              // If there's only one member in the struct in the
              // pass-to-function (by pointer) case, then the underlying type
              // will have been reduced to the contained type.
              VERIFY_ARE_EQUAL(1u, memberCount);
            } else {
              VERIFY_IS_TRUE(false);
            }
          }
        }
      }
    }

    // The member iterator should find a solid run of bits that is exactly
    // covered by exactly one of the members found by the annotation pass:
    if (validateCoverage) {
      unsigned CurRegIdx = 0;
      for (AggregateOffsetAndSize const &cover :
           ret.OffsetAndSizes) // For each entry read from member iterators
                               // and dbg.declares
      {
        bool found = false;
        for (ValueLocation const &valueLocation :
             passOutput.valueLocations) // For each allocas and dxil values
        {
          if (CurRegIdx == (unsigned)valueLocation.base &&
              (unsigned)valueLocation.count == cover.countOfMembers) {
            VERIFY_IS_FALSE(found);
            found = true;
          }
        }
        VERIFY_IS_TRUE(found);
        CurRegIdx += cover.countOfMembers;
      }
    }

    // For every store operation to the struct alloca, check that the
    // annotation pass correctly determined which alloca
    for (auto &block : entryFunction->getBasicBlockList()) {
      for (auto &instruction : block.getInstList()) {
        if (auto *store = llvm::dyn_cast<llvm::StoreInst>(&instruction)) {

          AllocaWrite NewAllocaWrite = {};
          if (FindStructMemberFromStore(store, &NewAllocaWrite.memberName)) {
            llvm::Value *index;
            if (pix_dxil::PixAllocaRegWrite::FromInst(
                    store, &NewAllocaWrite.regBase, &NewAllocaWrite.regSize,
                    &index)) {
              auto *asInt = llvm::dyn_cast<llvm::ConstantInt>(index);
              NewAllocaWrite.index = asInt->getLimitedValue();
              ret.AllocaWrites.push_back(NewAllocaWrite);
            }
          }
        }
      }
    }
  }
  return ret;
}

void PixTest::ValidateAllocaWrite(std::vector<AllocaWrite> const &allocaWrites,
                                  size_t index, const char *name) {
  VERIFY_ARE_EQUAL(index,
                   allocaWrites[index].regBase + allocaWrites[index].index);
#ifndef NDEBUG
  // Compilation may add a prefix to the struct member name:
  VERIFY_IS_TRUE(
      0 == strncmp(name, allocaWrites[index].memberName.c_str(), strlen(name)));
#endif
}

struct OptimizationChoice {
  const wchar_t *Flag;
  bool IsOptimized;
};
static const OptimizationChoice OptimizationChoices[] = {
    {L"-Od", false},
    {L"-O1", true},
};

TEST_F(PixTest, PixStructAnnotation_Lib_DualRaygen) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;
    const char *hlsl = R"(

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0);

struct SceneConstantBuffer
{
    float4x4 projectionToWorld;
    float4 cameraPosition;
    float4 lightPosition;
    float4 lightAmbientColor;
    float4 lightDiffuseColor;
};

ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);

struct RayPayload
{
    float4 color;
};

inline void GenerateCameraRay(uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy;// / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a ray.
    float4 world = /*mul(*/float4(screenPos, 0, 1)/*, g_sceneCB.projectionToWorld)*/;

    //world.xyz /= world.w;
    origin = world.xyz; //g_sceneCB.cameraPosition.xyz;
    direction = float3(1,0,0);//normalize(world.xyz - origin);
}

void RaygenCommon()
{
    float3 rayDir;
    float3 origin;
    
    // Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    GenerateCameraRay(DispatchRaysIndex().xy, origin, rayDir);

    // Trace the ray.
    // Set the ray's extents.
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0) };
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);

    // Write the raytraced color to the output texture.
   // RenderTarget[DispatchRaysIndex().xy] = payload.color;
}

[shader("raygeneration")]
void Raygen0()
{
    RaygenCommon();
}

[shader("raygeneration")]
void Raygen1()
{
    RaygenCommon();
}
)";

    // This is just a crash test until we decide what the right way forward
    CComPtr<IDxcBlob> pBlob =
        Compile(m_dllSupport, hlsl, L"lib_6_6", {optimization});
    CComPtr<IDxcBlob> pDxil = FindModule(DFCC_ShaderDebugInfoDXIL, pBlob);
    RunAnnotationPasses(m_dllSupport, pDxil);
  }
}

TEST_F(PixTest, PixStructAnnotation_Simple) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;
    const char *hlsl = R"(
struct smallPayload
{
    uint dummy;
};


[numthreads(1, 1, 1)]
void main()
{
    smallPayload p;
    p.dummy = 42;
    DispatchMesh(1, 1, 1, p);
}
)";

    auto Testables = TestStructAnnotationCase(hlsl, optimization);

    if (!Testables.OffsetAndSizes.empty()) {
      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes.size());
      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[0].countOfMembers);
      VERIFY_ARE_EQUAL(0u, Testables.OffsetAndSizes[0].offset);
      VERIFY_ARE_EQUAL(32u, Testables.OffsetAndSizes[0].size);
    }

    VERIFY_ARE_EQUAL(1u, Testables.AllocaWrites.size());
    ValidateAllocaWrite(Testables.AllocaWrites, 0, "dummy");
  }
}

TEST_F(PixTest, PixStructAnnotation_CopiedStruct) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;
  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;

    const char *hlsl = R"(
struct smallPayload
{
    uint dummy;
};


[numthreads(1, 1, 1)]
void main()
{
    smallPayload p;
    p.dummy = 42;
    smallPayload p2 = p;
    DispatchMesh(1, 1, 1, p2);
}
)";

    auto Testables = TestStructAnnotationCase(hlsl, optimization);

    // 2 in unoptimized case (one for each instance of smallPayload)
    // 1 in optimized case (cuz p2 aliases over p)
    VERIFY_IS_TRUE(Testables.OffsetAndSizes.size() >= 1);

    for (const auto &os : Testables.OffsetAndSizes) {
      VERIFY_ARE_EQUAL(1u, os.countOfMembers);
      VERIFY_ARE_EQUAL(0u, os.offset);
      VERIFY_ARE_EQUAL(32u, os.size);
    }

    VERIFY_ARE_EQUAL(1u, Testables.AllocaWrites.size());
  }
}

TEST_F(PixTest, PixStructAnnotation_MixedSizes) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;

    const char *hlsl = R"(
struct smallPayload
{
    bool b1;
    uint16_t sixteen;
    uint32_t thirtytwo;
    uint64_t sixtyfour;
};


[numthreads(1, 1, 1)]
void main()
{
    smallPayload p;
    p.b1 = true;
    p.sixteen = 16;
    p.thirtytwo = 32;
    p.sixtyfour = 64;
    DispatchMesh(1, 1, 1, p);
}
)";

    auto Testables = TestStructAnnotationCase(hlsl, optimization);

    if (!choice.IsOptimized) {
      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes.size());
      VERIFY_ARE_EQUAL(4u, Testables.OffsetAndSizes[0].countOfMembers);
      VERIFY_ARE_EQUAL(0u, Testables.OffsetAndSizes[0].offset);
      // 8 bytes align for uint64_t:
      VERIFY_ARE_EQUAL(32u + 16u + 16u /*alignment for next field*/ + 32u +
                           32u /*alignment for max align*/ + 64u,
                       Testables.OffsetAndSizes[0].size);
    } else {
      VERIFY_ARE_EQUAL(4u, Testables.OffsetAndSizes.size());

      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[0].countOfMembers);
      VERIFY_ARE_EQUAL(0u, Testables.OffsetAndSizes[0].offset);
      VERIFY_ARE_EQUAL(32u, Testables.OffsetAndSizes[0].size);

      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[1].countOfMembers);
      VERIFY_ARE_EQUAL(32u, Testables.OffsetAndSizes[1].offset);
      VERIFY_ARE_EQUAL(16u, Testables.OffsetAndSizes[1].size);

      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[2].countOfMembers);
      VERIFY_ARE_EQUAL(32u + 32u, Testables.OffsetAndSizes[2].offset);
      VERIFY_ARE_EQUAL(32u, Testables.OffsetAndSizes[2].size);

      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[3].countOfMembers);
      VERIFY_ARE_EQUAL(32u + 32u + 32u + /*padding for alignment*/ 32u,
                       Testables.OffsetAndSizes[3].offset);
      VERIFY_ARE_EQUAL(64u, Testables.OffsetAndSizes[3].size);
    }

    VERIFY_ARE_EQUAL(4u, Testables.AllocaWrites.size());
    ValidateAllocaWrite(Testables.AllocaWrites, 0, "b1");
    ValidateAllocaWrite(Testables.AllocaWrites, 1, "sixteen");
    ValidateAllocaWrite(Testables.AllocaWrites, 2, "thirtytwo");
    ValidateAllocaWrite(Testables.AllocaWrites, 3, "sixtyfour");
  }
}

TEST_F(PixTest, PixStructAnnotation_StructWithinStruct) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;

    const char *hlsl = R"(

struct Contained
{
  uint32_t one;
  uint32_t two;
};

struct smallPayload
{
  uint32_t before;
  Contained contained;
  uint32_t after;
};


[numthreads(1, 1, 1)]
void main()
{
    smallPayload p;
    p.before = 0xb4;
    p.contained.one = 1;
    p.contained.two = 2;
    p.after = 3;
    DispatchMesh(1, 1, 1, p);
}
)";

    auto Testables = TestStructAnnotationCase(hlsl, optimization);

    if (!choice.IsOptimized) {
      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes.size());
      VERIFY_ARE_EQUAL(4u, Testables.OffsetAndSizes[0].countOfMembers);
      VERIFY_ARE_EQUAL(0u, Testables.OffsetAndSizes[0].offset);
      VERIFY_ARE_EQUAL(4u * 32u, Testables.OffsetAndSizes[0].size);
    } else {
      VERIFY_ARE_EQUAL(4u, Testables.OffsetAndSizes.size());
      for (unsigned i = 0; i < 4; i++) {
        VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[i].countOfMembers);
        VERIFY_ARE_EQUAL(i * 32u, Testables.OffsetAndSizes[i].offset);
        VERIFY_ARE_EQUAL(32u, Testables.OffsetAndSizes[i].size);
      }
    }

    ValidateAllocaWrite(Testables.AllocaWrites, 0, "before");
    ValidateAllocaWrite(Testables.AllocaWrites, 1, "contained.one");
    ValidateAllocaWrite(Testables.AllocaWrites, 2, "contained.two");
    ValidateAllocaWrite(Testables.AllocaWrites, 3, "after");
  }
}

TEST_F(PixTest, PixStructAnnotation_1DArray) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;

    const char *hlsl = R"(
struct smallPayload
{
    uint32_t Array[2];
};


[numthreads(1, 1, 1)]
void main()
{
    smallPayload p;
    p.Array[0] = 250;
    p.Array[1] = 251;
    DispatchMesh(1, 1, 1, p);
}
)";

    auto Testables = TestStructAnnotationCase(hlsl, optimization);
    if (!choice.IsOptimized) {
      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes.size());
      VERIFY_ARE_EQUAL(2u, Testables.OffsetAndSizes[0].countOfMembers);
      VERIFY_ARE_EQUAL(0u, Testables.OffsetAndSizes[0].offset);
      VERIFY_ARE_EQUAL(2u * 32u, Testables.OffsetAndSizes[0].size);
    } else {
      VERIFY_ARE_EQUAL(2u, Testables.OffsetAndSizes.size());
      for (unsigned i = 0; i < 2; i++) {
        VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[i].countOfMembers);
        VERIFY_ARE_EQUAL(i * 32u, Testables.OffsetAndSizes[i].offset);
        VERIFY_ARE_EQUAL(32u, Testables.OffsetAndSizes[i].size);
      }
    }
    VERIFY_ARE_EQUAL(2u, Testables.AllocaWrites.size());

    int Idx = 0;
    ValidateAllocaWrite(Testables.AllocaWrites, Idx++, "Array[0]");
    ValidateAllocaWrite(Testables.AllocaWrites, Idx++, "Array[1]");
  }
}

TEST_F(PixTest, PixStructAnnotation_2DArray) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;
    const char *hlsl = R"(
struct smallPayload
{
    uint32_t TwoDArray[2][3];
};


[numthreads(1, 1, 1)]
void main()
{
    smallPayload p;
    p.TwoDArray[0][0] = 250;
    p.TwoDArray[0][1] = 251;
    p.TwoDArray[0][2] = 252;
    p.TwoDArray[1][0] = 253;
    p.TwoDArray[1][1] = 254;
    p.TwoDArray[1][2] = 255;
    DispatchMesh(1, 1, 1, p);
}
)";

    auto Testables = TestStructAnnotationCase(hlsl, optimization);
    if (!choice.IsOptimized) {
      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes.size());
      VERIFY_ARE_EQUAL(6u, Testables.OffsetAndSizes[0].countOfMembers);
      VERIFY_ARE_EQUAL(0u, Testables.OffsetAndSizes[0].offset);
      VERIFY_ARE_EQUAL(2u * 3u * 32u, Testables.OffsetAndSizes[0].size);
    } else {
      VERIFY_ARE_EQUAL(6u, Testables.OffsetAndSizes.size());
      for (unsigned i = 0; i < 6; i++) {
        VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[i].countOfMembers);
        VERIFY_ARE_EQUAL(i * 32u, Testables.OffsetAndSizes[i].offset);
        VERIFY_ARE_EQUAL(32u, Testables.OffsetAndSizes[i].size);
      }
    }
    VERIFY_ARE_EQUAL(6u, Testables.AllocaWrites.size());

    int Idx = 0;
    ValidateAllocaWrite(Testables.AllocaWrites, Idx++, "TwoDArray[0][0]");
    ValidateAllocaWrite(Testables.AllocaWrites, Idx++, "TwoDArray[0][1]");
    ValidateAllocaWrite(Testables.AllocaWrites, Idx++, "TwoDArray[0][2]");
    ValidateAllocaWrite(Testables.AllocaWrites, Idx++, "TwoDArray[1][0]");
    ValidateAllocaWrite(Testables.AllocaWrites, Idx++, "TwoDArray[1][1]");
    ValidateAllocaWrite(Testables.AllocaWrites, Idx++, "TwoDArray[1][2]");
  }
}

TEST_F(PixTest, PixStructAnnotation_EmbeddedArray) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;
    const char *hlsl = R"(

struct Contained
{
  uint32_t array[3];
};

struct smallPayload
{
  uint32_t before;
  Contained contained;
  uint32_t after;
};


[numthreads(1, 1, 1)]
void main()
{
    smallPayload p;
    p.before = 0xb4;
    p.contained.array[0] = 0;
    p.contained.array[1] = 1;
    p.contained.array[2] = 2;
    p.after = 3;
    DispatchMesh(1, 1, 1, p);
}
)";

    auto Testables = TestStructAnnotationCase(hlsl, optimization);

    if (!choice.IsOptimized) {
      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes.size());
      VERIFY_ARE_EQUAL(5u, Testables.OffsetAndSizes[0].countOfMembers);
      VERIFY_ARE_EQUAL(0u, Testables.OffsetAndSizes[0].offset);
      VERIFY_ARE_EQUAL(5u * 32u, Testables.OffsetAndSizes[0].size);
    } else {
      VERIFY_ARE_EQUAL(5u, Testables.OffsetAndSizes.size());
      for (unsigned i = 0; i < 5; i++) {
        VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[i].countOfMembers);
        VERIFY_ARE_EQUAL(i * 32u, Testables.OffsetAndSizes[i].offset);
        VERIFY_ARE_EQUAL(32u, Testables.OffsetAndSizes[i].size);
      }
    }

    ValidateAllocaWrite(Testables.AllocaWrites, 0, "before");
    ValidateAllocaWrite(Testables.AllocaWrites, 1, "contained.array[0]");
    ValidateAllocaWrite(Testables.AllocaWrites, 2, "contained.array[1]");
    ValidateAllocaWrite(Testables.AllocaWrites, 3, "contained.array[2]");
    ValidateAllocaWrite(Testables.AllocaWrites, 4, "after");
  }
}

TEST_F(PixTest, PixStructAnnotation_FloatN) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;
    auto IsOptimized = choice.IsOptimized;
    const char *hlsl = R"(
struct smallPayload
{
    float2 f2;
};


[numthreads(1, 1, 1)]
void main()
{
    smallPayload p;
    p.f2 = float2(1,2);
    DispatchMesh(1, 1, 1, p);
}
)";

    auto Testables = TestStructAnnotationCase(hlsl, optimization);

    if (IsOptimized) {
      VERIFY_ARE_EQUAL(2u, Testables.OffsetAndSizes.size());
      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[0].countOfMembers);
      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[1].countOfMembers);
      VERIFY_ARE_EQUAL(0u, Testables.OffsetAndSizes[0].offset);
      VERIFY_ARE_EQUAL(32u, Testables.OffsetAndSizes[0].size);
      VERIFY_ARE_EQUAL(32u, Testables.OffsetAndSizes[1].offset);
      VERIFY_ARE_EQUAL(32u, Testables.OffsetAndSizes[1].size);
    } else {
      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes.size());
      VERIFY_ARE_EQUAL(2u, Testables.OffsetAndSizes[0].countOfMembers);
      VERIFY_ARE_EQUAL(0u, Testables.OffsetAndSizes[0].offset);
      VERIFY_ARE_EQUAL(32u + 32u, Testables.OffsetAndSizes[0].size);
    }

    VERIFY_ARE_EQUAL(Testables.AllocaWrites.size(), 2u);
    ValidateAllocaWrite(Testables.AllocaWrites, 0, "f2.x");
    ValidateAllocaWrite(Testables.AllocaWrites, 1, "f2.y");
  }
}

TEST_F(PixTest, PixStructAnnotation_SequentialFloatN) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;
    const char *hlsl = R"(
struct smallPayload
{
    float3 color;
    float3 dir;
};


[numthreads(1, 1, 1)]
void main()
{
    smallPayload p;
    p.color = float3(1,2,3);
    p.dir = float3(4,5,6);

    DispatchMesh(1, 1, 1, p);
}
)";

    auto Testables = TestStructAnnotationCase(hlsl, optimization);

    if (choice.IsOptimized) {
      VERIFY_ARE_EQUAL(6u, Testables.OffsetAndSizes.size());
      for (unsigned i = 0; i < 6; i++) {
        VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[i].countOfMembers);
        VERIFY_ARE_EQUAL(i * 32u, Testables.OffsetAndSizes[i].offset);
        VERIFY_ARE_EQUAL(32u, Testables.OffsetAndSizes[i].size);
      }
    } else {
      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes.size());
      VERIFY_ARE_EQUAL(6u, Testables.OffsetAndSizes[0].countOfMembers);
      VERIFY_ARE_EQUAL(0u, Testables.OffsetAndSizes[0].offset);
      VERIFY_ARE_EQUAL(32u * 6u, Testables.OffsetAndSizes[0].size);
    }

    VERIFY_ARE_EQUAL(6u, Testables.AllocaWrites.size());
    ValidateAllocaWrite(Testables.AllocaWrites, 0, "color.x");
    ValidateAllocaWrite(Testables.AllocaWrites, 1, "color.y");
    ValidateAllocaWrite(Testables.AllocaWrites, 2, "color.z");
    ValidateAllocaWrite(Testables.AllocaWrites, 3, "dir.x");
    ValidateAllocaWrite(Testables.AllocaWrites, 4, "dir.y");
    ValidateAllocaWrite(Testables.AllocaWrites, 5, "dir.z");
  }
}

TEST_F(PixTest, PixStructAnnotation_EmbeddedFloatN) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;
    const char *hlsl = R"(

struct Embedded
{
    float2 f2;
};

struct smallPayload
{
  uint32_t i32;
  Embedded e;
};


[numthreads(1, 1, 1)]
void main()
{
    smallPayload p;
    p.i32 = 32;
    p.e.f2 = float2(1,2);
    DispatchMesh(1, 1, 1, p);
}
)";

    auto Testables = TestStructAnnotationCase(hlsl, optimization);

    if (choice.IsOptimized) {
      VERIFY_ARE_EQUAL(3u, Testables.OffsetAndSizes.size());
      for (unsigned i = 0; i < 3; i++) {
        VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[i].countOfMembers);
        VERIFY_ARE_EQUAL(i * 32u, Testables.OffsetAndSizes[i].offset);
        VERIFY_ARE_EQUAL(32u, Testables.OffsetAndSizes[i].size);
      }
    } else {
      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes.size());
      VERIFY_ARE_EQUAL(3u, Testables.OffsetAndSizes[0].countOfMembers);
      VERIFY_ARE_EQUAL(0u, Testables.OffsetAndSizes[0].offset);
      VERIFY_ARE_EQUAL(32u * 3u, Testables.OffsetAndSizes[0].size);
    }

    VERIFY_ARE_EQUAL(3u, Testables.AllocaWrites.size());
    ValidateAllocaWrite(Testables.AllocaWrites, 0, "i32");
    ValidateAllocaWrite(Testables.AllocaWrites, 1, "e.f2.x");
    ValidateAllocaWrite(Testables.AllocaWrites, 2, "e.f2.y");
  }
}

TEST_F(PixTest, PixStructAnnotation_Matrix) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;
    const char *hlsl = R"(
struct smallPayload
{
  float4x4 mat;
};


[numthreads(1, 1, 1)]
void main()
{
  smallPayload p;
  p.mat = float4x4( 1,2,3,4, 5,6,7,8, 9,10,11,12, 13,14,15, 16);
  DispatchMesh(1, 1, 1, p);
}
)";

    auto Testables = TestStructAnnotationCase(hlsl, optimization);
    // Can't test member iterator until dbg.declare instructions are emitted
    // when structs contain pointers-to-pointers
    VERIFY_ARE_EQUAL(16u, Testables.AllocaWrites.size());
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        std::string expected = std::string("mat._") + std::to_string(i + 1) +
                               std::to_string(j + 1);
        ValidateAllocaWrite(Testables.AllocaWrites, i * 4 + j,
                            expected.c_str());
      }
    }
  }
}

TEST_F(PixTest, PixStructAnnotation_MemberFunction) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;
    const char *hlsl = R"(

RWStructuredBuffer<float> floatRWUAV: register(u0);

struct smallPayload
{
    int i;
};

float2 signNotZero(float2 v)
{
 return (v > 0.0f ? float(1).xx : float(-1).xx);
}

float2 unpackUnorm2(uint packed)
{
 return (1.0 / 65535.0) * float2((packed >> 16) & 0xffff, packed & 0xffff);
}

float3 unpackOctahedralSnorm(float2 e)
{
 float3 v = float3(e.xy, 1.0f - abs(e.x) - abs(e.y));
 if (v.z < 0.0f) v.xy = (1.0f - abs(v.yx)) * signNotZero(v.xy);
 return normalize(v);
}

float3 unpackOctahedralUnorm(float2 e)
{
 return unpackOctahedralSnorm(e * 2.0f - 1.0f);
}

float2 unpackHalf2(uint packed)
{
 return float2(f16tof32(packed >> 16), f16tof32(packed & 0xffff));
}

struct Gbuffer
{
	float3 worldNormal;
	float3 objectNormal; //offset:12
	float linearZ; //24
	float prevLinearZ; //28
	float fwidthLinearZ; //32
	float fwidthObjectNormal; //36
	uint materialType; //40
	uint2 materialParams0; //44
	uint4 materialParams1; //52  <--------- this is the variable that's being covered twice (52*8 = 416 416)
	uint instanceId;  //68  <------- and there's one dword left over, as expected
	void load(int2 pixelPos, Texture2DArray<uint4> gbTex)
	{
	uint4 data0 = gbTex.Load(int4(pixelPos, 0, 0));
	uint4 data1 = gbTex.Load(int4(pixelPos, 1, 0));
	uint4 data2 = gbTex.Load(int4(pixelPos, 2, 0));
	worldNormal = unpackOctahedralUnorm(unpackUnorm2(data0.x));
	linearZ = f16tof32((data0.y >> 8) & 0xffff);
	materialType = (data0.y & 0xff);
	materialParams0 = data0.zw;
	materialParams1 = data1.xyzw;
	instanceId = data2.x;
	prevLinearZ = asfloat(data2.y);
	objectNormal = unpackOctahedralUnorm(unpackUnorm2(data2.z));
	float2 fwidth = unpackHalf2(data2.w);
	fwidthLinearZ = fwidth.x;
	fwidthObjectNormal = fwidth.y;
	}
};

Gbuffer loadGbuffer(int2 pixelPos, Texture2DArray<uint4> gbTex)
{
	Gbuffer output;
	output.load(pixelPos, gbTex);
	return output;
}

Texture2DArray<uint4> g_gbuffer : register(t0, space0);

[numthreads(1, 1, 1)]
void main()
{	
	const Gbuffer gbuffer = loadGbuffer(int2(0,0), g_gbuffer);
    smallPayload p;
    p.i = gbuffer.materialParams1.x + gbuffer.materialParams1.y + gbuffer.materialParams1.z + gbuffer.materialParams1.w;
    DispatchMesh(1, 1, 1, p);
}


)";
    auto Testables = TestStructAnnotationCase(hlsl, optimization, true);

    // TODO: Make 'this' work

    // Can't validate # of writes: rel and dbg are different
    // VERIFY_ARE_EQUAL(43, Testables.AllocaWrites.size());

    // Can't test individual writes until struct member names are returned:
    // for (int i = 0; i < 51; ++i)
    //{
    //  ValidateAllocaWrite(Testables.AllocaWrites, i, "");
    //}
  }
}

TEST_F(PixTest, PixStructAnnotation_BigMess) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;

    const char *hlsl = R"(

struct BigStruct
{
    uint64_t bigInt;
    double bigDouble;
};

struct EmbeddedStruct
{
    uint32_t OneInt;
    uint32_t TwoDArray[2][2];
};

struct smallPayload
{
    uint dummy;
    uint vertexCount;
    uint primitiveCount;
    EmbeddedStruct embeddedStruct;
#ifdef PAYLOAD_MATRICES
    float4x4 mat;
#endif
    uint64_t bigOne;
    half littleOne;
    BigStruct bigStruct[2];
    uint lastCheck;
};


[numthreads(1, 1, 1)]
void main()
{
    smallPayload p;
    // Adding enough instructions to make the shader interesting to debug:
    p.dummy = 42;
    p.vertexCount = 3;
    p.primitiveCount = 1;
    p.embeddedStruct.OneInt = 123;
    p.embeddedStruct.TwoDArray[0][0] = 252;
    p.embeddedStruct.TwoDArray[0][1] = 253;
    p.embeddedStruct.TwoDArray[1][0] = 254;
    p.embeddedStruct.TwoDArray[1][1] = 255;
#ifdef PAYLOAD_MATRICES
    p.mat = float4x4( 1,2,3,4, 5,6,7,8, 9,10,11,12, 13,14,15, 16);
#endif
    p.bigOne = 123456789;
    p.littleOne = 1.0;
    p.bigStruct[0].bigInt = 10;
    p.bigStruct[0].bigDouble = 2.0;
    p.bigStruct[1].bigInt = 20;
    p.bigStruct[1].bigDouble = 4.0;
    p.lastCheck = 27;
    DispatchMesh(1, 1, 1, p);
}
)";

    auto Testables = TestStructAnnotationCase(hlsl, optimization);
    if (!choice.IsOptimized) {
      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes.size());
      VERIFY_ARE_EQUAL(15u, Testables.OffsetAndSizes[0].countOfMembers);
      VERIFY_ARE_EQUAL(0u, Testables.OffsetAndSizes[0].offset);
      constexpr uint32_t BigStructBitSize = 64 * 2;
      constexpr uint32_t EmbeddedStructBitSize = 32 * 5;
      VERIFY_ARE_EQUAL(3u * 32u + EmbeddedStructBitSize + 64u + 16u +
                           16u /*alignment for next field*/ +
                           BigStructBitSize * 2u + 32u +
                           32u /*align to max align*/,
                       Testables.OffsetAndSizes[0].size);
    } else {
      VERIFY_ARE_EQUAL(15u, Testables.OffsetAndSizes.size());

      // First 8 members
      for (unsigned i = 0; i < 8; i++) {
        VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[i].countOfMembers);
        VERIFY_ARE_EQUAL(i * 32u, Testables.OffsetAndSizes[i].offset);
        VERIFY_ARE_EQUAL(32u, Testables.OffsetAndSizes[i].size);
      }

      // bigOne
      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[8].countOfMembers);
      VERIFY_ARE_EQUAL(256u, Testables.OffsetAndSizes[8].offset);
      VERIFY_ARE_EQUAL(64u, Testables.OffsetAndSizes[8].size);

      // littleOne
      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[9].countOfMembers);
      VERIFY_ARE_EQUAL(320u, Testables.OffsetAndSizes[9].offset);
      VERIFY_ARE_EQUAL(16u, Testables.OffsetAndSizes[9].size);

      // Each member of BigStruct[2]
      for (unsigned i = 0; i < 4; i++) {
        int idx = i + 10;
        VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[idx].countOfMembers);
        VERIFY_ARE_EQUAL(384 + i * 64u, Testables.OffsetAndSizes[idx].offset);
        VERIFY_ARE_EQUAL(64u, Testables.OffsetAndSizes[idx].size);
      }

      VERIFY_ARE_EQUAL(1u, Testables.OffsetAndSizes[14].countOfMembers);
      VERIFY_ARE_EQUAL(640u, Testables.OffsetAndSizes[14].offset);
      VERIFY_ARE_EQUAL(32u, Testables.OffsetAndSizes[14].size);
    }

    VERIFY_ARE_EQUAL(15u, Testables.AllocaWrites.size());

    size_t Index = 0;
    ValidateAllocaWrite(Testables.AllocaWrites, Index++, "dummy");
    ValidateAllocaWrite(Testables.AllocaWrites, Index++, "vertexCount");
    ValidateAllocaWrite(Testables.AllocaWrites, Index++, "primitiveCount");
    ValidateAllocaWrite(Testables.AllocaWrites, Index++,
                        "embeddedStruct.OneInt");
    ValidateAllocaWrite(Testables.AllocaWrites, Index++,
                        "embeddedStruct.TwoDArray[0][0]");
    ValidateAllocaWrite(Testables.AllocaWrites, Index++,
                        "embeddedStruct.TwoDArray[0][1]");
    ValidateAllocaWrite(Testables.AllocaWrites, Index++,
                        "embeddedStruct.TwoDArray[1][0]");
    ValidateAllocaWrite(Testables.AllocaWrites, Index++,
                        "embeddedStruct.TwoDArray[1][1]");
    ValidateAllocaWrite(Testables.AllocaWrites, Index++, "bigOne");
    ValidateAllocaWrite(Testables.AllocaWrites, Index++, "littleOne");
    ValidateAllocaWrite(Testables.AllocaWrites, Index++, "bigStruct[0].bigInt");
    ValidateAllocaWrite(Testables.AllocaWrites, Index++,
                        "bigStruct[0].bigDouble");
    ValidateAllocaWrite(Testables.AllocaWrites, Index++, "bigStruct[1].bigInt");
    ValidateAllocaWrite(Testables.AllocaWrites, Index++,
                        "bigStruct[1].bigDouble");
    ValidateAllocaWrite(Testables.AllocaWrites, Index++, "lastCheck");
  }
}

TEST_F(PixTest, PixStructAnnotation_AlignedFloat4Arrays) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;

    const char *hlsl = R"(

struct LinearSHSampleData
{
	float4 linearTerms[3];
	float4 hdrColorAO;
	float4 visibilitySH;
} g_lhSampleData;

struct smallPayload
{
    LinearSHSampleData lhSampleData;
};


[numthreads(1, 1, 1)]
void main()
{
    smallPayload p;
    p.lhSampleData.linearTerms[0].x = g_lhSampleData.linearTerms[0].x;
    DispatchMesh(1, 1, 1, p);
}
)";

    auto Testables = TestStructAnnotationCase(hlsl, optimization);
    // Can't test offsets and sizes until dbg.declare instructions are emitted
    // when floatn is used
    // (https://github.com/microsoft/DirectXShaderCompiler/issues/2920)
    // VERIFY_ARE_EQUAL(20, Testables.AllocaWrites.size());
  }
}

TEST_F(PixTest, PixStructAnnotation_Inheritance) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;

    const char *hlsl = R"(
struct Base
{
    float floatValue;
};
typedef Base BaseTypedef;

struct Derived : BaseTypedef
{
	int intValue;
};

[numthreads(1, 1, 1)]
void main()
{
    Derived p;
    p.floatValue = 1.;
    p.intValue = 2;
    DispatchMesh(1, 1, 1, p);
}
)";

    auto Testables = TestStructAnnotationCase(hlsl, optimization);

    // Can't test offsets and sizes until dbg.declare instructions are emitted
    // when floatn is used
    // (https://github.com/microsoft/DirectXShaderCompiler/issues/2920)
    // VERIFY_ARE_EQUAL(20, Testables.AllocaWrites.size());
  }
}

TEST_F(PixTest, PixStructAnnotation_ResourceAsMember) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;

    const char *hlsl = R"(

Buffer g_texture;

struct smallPayload
{
    float value;
};

struct WithEmbeddedObject
{
	Buffer texture;
};

void DispatchIt(WithEmbeddedObject eo)
{
    smallPayload p;
    p.value = eo.texture.Load(0);
    DispatchMesh(1, 1, 1, p);
}

[numthreads(1, 1, 1)]
void main()
{
    WithEmbeddedObject eo;
    eo.texture = g_texture;
    DispatchIt(eo);
}
)";

    auto Testables = TestStructAnnotationCase(hlsl, optimization);
    // Can't test offsets and sizes until dbg.declare instructions are emitted
    // when floatn is used
    // (https://github.com/microsoft/DirectXShaderCompiler/issues/2920)
    // VERIFY_ARE_EQUAL(20, Testables.AllocaWrites.size());
  }
}

TEST_F(PixTest, PixStructAnnotation_WheresMyDbgValue) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;

    const char *hlsl = R"(

struct smallPayload
{
    float f1;
    float2 f2;
};


[numthreads(1, 1, 1)]
void main()
{
    smallPayload p;
    p.f1 = 1;
    p.f2 = float2(2,3);
    DispatchMesh(1, 1, 1, p);
}
)";

    auto Testables = TestStructAnnotationCase(hlsl, optimization);
    // Can't test offsets and sizes until dbg.declare instructions are emitted
    // when floatn is used
    // (https://github.com/microsoft/DirectXShaderCompiler/issues/2920)
    VERIFY_ARE_EQUAL(3u, Testables.AllocaWrites.size());
  }
}

TEST_F(PixTest, VirtualRegisters_InstructionCounts) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  for (auto choice : OptimizationChoices) {
    auto optimization = choice.Flag;
    const char *hlsl = R"(

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0);

struct SceneConstantBuffer
{
    float4x4 projectionToWorld;
    float4 cameraPosition;
    float4 lightPosition;
    float4 lightAmbientColor;
    float4 lightDiffuseColor;
};

ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);

struct RayPayload
{
    float4 color;
};

inline void GenerateCameraRay(uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy;// / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a ray.
    float4 world = /*mul(*/float4(screenPos, 0, 1)/*, g_sceneCB.projectionToWorld)*/;

    //world.xyz /= world.w;
    origin = world.xyz; //g_sceneCB.cameraPosition.xyz;
    direction = float3(1,0,0);//normalize(world.xyz - origin);
}

void RaygenCommon()
{
    float3 rayDir;
    float3 origin;
    
    // Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    GenerateCameraRay(DispatchRaysIndex().xy, origin, rayDir);

    // Trace the ray.
    // Set the ray's extents.
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0) };
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);

    // Write the raytraced color to the output texture.
   // RenderTarget[DispatchRaysIndex().xy] = payload.color;
}

[shader("raygeneration")]
void Raygen0()
{
    RaygenCommon();
}

[shader("raygeneration")]
void Raygen1()
{
    RaygenCommon();
}

typedef BuiltInTriangleIntersectionAttributes MyAttributes;

[shader("closesthit")]
void InnerClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    payload.color = float4(0,1,0,0);
}


[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    payload.color = float4(1, 0, 0, 0);
})";

    CComPtr<IDxcBlob> pBlob =
        Compile(m_dllSupport, hlsl, L"lib_6_6", {optimization});
    CComPtr<IDxcBlob> pDxil = FindModule(DFCC_ShaderDebugInfoDXIL, pBlob);
    auto outputLines = RunAnnotationPasses(m_dllSupport, pDxil).lines;

    const char instructionRangeLabel[] = "InstructionRange:";

    // The numbering pass should have counted  instructions for each
    // "interesting" (to PIX) function and output its start and (end+1)
    // instruction ordinal. End should always be a reasonable number of
    // instructions (>10) and end should always be higher than start, and all
    // four functions above should be represented.
    int countOfInstructionRangeLines = 0;
    for (auto const &line : outputLines) {
      auto tokens = Tokenize(line, " ");
      if (tokens.size() >= 4) {
        if (tokens[0] == instructionRangeLabel) {
          countOfInstructionRangeLines++;
          int instructionStart = atoi(tokens[1].c_str());
          int instructionEnd = atoi(tokens[2].c_str());
          VERIFY_IS_TRUE(instructionEnd > 10);
          VERIFY_IS_TRUE(instructionEnd > instructionStart);
          auto found1 = tokens[3].find("Raygen0@@YAXXZ") != std::string::npos;
          auto found2 = tokens[3].find("Raygen1@@YAXXZ") != std::string::npos;
          auto foundClosest =
              tokens[3].find("InnerClosestHit") != std::string::npos;
          auto foundMiss = tokens[3].find("MyMiss") != std::string::npos;
          VERIFY_IS_TRUE(found1 || found2 || foundClosest || foundMiss);
        }
      }
    }
    VERIFY_ARE_EQUAL(4, countOfInstructionRangeLines);

    // Non-library target:
    const char *PixelShader = R"(
    [RootSignature("")]
    float main(float pos : A) : SV_Target {
      float x = abs(pos);
      float y = sin(pos);
      float z = x + y;
      return z;
    }
  )";
    pBlob = Compile(m_dllSupport, PixelShader, L"ps_6_6", {optimization});
    pDxil = FindModule(DFCC_ShaderDebugInfoDXIL, pBlob);
    outputLines = RunAnnotationPasses(m_dllSupport, pDxil).lines;

    countOfInstructionRangeLines = 0;
    for (auto const &line : outputLines) {
      auto tokens = Tokenize(line, " ");
      if (tokens.size() >= 4) {
        if (tokens[0] == instructionRangeLabel) {
          countOfInstructionRangeLines++;
          int instructionStart = atoi(tokens[1].c_str());
          int instructionEnd = atoi(tokens[2].c_str());
          VERIFY_IS_TRUE(instructionStart == 0);
          VERIFY_IS_TRUE(instructionEnd > 10);
          VERIFY_IS_TRUE(instructionEnd > instructionStart);
          auto foundMain = tokens[3].find("main") != std::string::npos;
          VERIFY_IS_TRUE(foundMain);
        }
      }
    }
    VERIFY_ARE_EQUAL(1, countOfInstructionRangeLines);

    // Now check that the initial value parameter works:
    const int startingInstructionOrdinal = 1234;
    outputLines =
        RunAnnotationPasses(m_dllSupport, pDxil, startingInstructionOrdinal)
            .lines;

    countOfInstructionRangeLines = 0;
    for (auto const &line : outputLines) {
      auto tokens = Tokenize(line, " ");
      if (tokens.size() >= 4) {
        if (tokens[0] == instructionRangeLabel) {
          countOfInstructionRangeLines++;
          int instructionStart = atoi(tokens[1].c_str());
          int instructionEnd = atoi(tokens[2].c_str());
          VERIFY_IS_TRUE(instructionStart == startingInstructionOrdinal);
          VERIFY_IS_TRUE(instructionEnd > instructionStart);
          auto foundMain = tokens[3].find("main") != std::string::npos;
          VERIFY_IS_TRUE(foundMain);
        }
      }
    }
    VERIFY_ARE_EQUAL(1, countOfInstructionRangeLines);
  }
}

TEST_F(PixTest, VirtualRegisters_AlignedOffsets) {
  if (m_ver.SkipDxilVersion(1, 5))
    return;

  {
    const char *hlsl = R"(
cbuffer cbEveryFrame : register(b0)
{
    int i32;
    float f32;
};

struct VS_OUTPUT_ENV
{
    float4 Pos        : SV_Position;
    float2 Tex        : TEXCOORD0;
};

float4 main(VS_OUTPUT_ENV input) : SV_Target
{
    // (BTW we load from i32 and f32 (which are resident in a cb) so that these local variables aren't optimized away)
    bool i1 = i32 != 0;
    min16uint u16 = (min16uint)(i32 / 4);
    min16int s16 = (min16int)(i32/4) * -1; // signed s16 gets -8
    min12int s12 = (min12int)(i32/8) * -1; // signed s12 gets -4
    half h = (half) f32 / 2.f; // f32 is initialized to 32.0 in8he CB, so the 16-bit type now has "16.0" in it
    min16float mf16 = (min16float) f32 / -2.f;
    min10float mf10 = (min10float) f32 / -4.f;
    return float4((float)(i1 + u16) / 2.f, (float)(s16 + s12) / -128.f, h / 128.f, mf16 / 128.f + mf10 / 256.f);
}
)";

    // This is little more than a crash test, designed to exercise a previously
    // over-active assert..
    std::vector<std::pair<const wchar_t *, std::vector<const wchar_t *>>>
        argSets = {
            {L"ps_6_0", {L"-Od"}},
            {L"ps_6_2", {L"-Od", L"-HV", L"2018", L"-enable-16bit-types"}}};
    for (auto const &args : argSets) {

      CComPtr<IDxcBlob> pBlob =
          Compile(m_dllSupport, hlsl, args.first, args.second);
      CComPtr<IDxcBlob> pDxil = FindModule(DFCC_ShaderDebugInfoDXIL, pBlob);
      RunAnnotationPasses(m_dllSupport, pDxil);
    }
  }
}

static void VerifyOperationSucceeded(IDxcOperationResult *pResult) {
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

TEST_F(PixTest, RootSignatureUpgrade_SubObjects) {

  const char *source = R"x(
GlobalRootSignature so_GlobalRootSignature =
{
	"RootConstants(num32BitConstants=1, b8), "
};

StateObjectConfig so_StateObjectConfig = 
{ 
    STATE_OBJECT_FLAGS_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITONS
};

LocalRootSignature so_LocalRootSignature1 = 
{
	"RootConstants(num32BitConstants=3, b2), "
	"UAV(u6),RootFlags(LOCAL_ROOT_SIGNATURE)" 
};

LocalRootSignature so_LocalRootSignature2 = 
{
	"RootConstants(num32BitConstants=3, b2), "
	"UAV(u8, flags=DATA_STATIC), " 
	"RootFlags(LOCAL_ROOT_SIGNATURE)"
};

RaytracingShaderConfig  so_RaytracingShaderConfig =
{
    128, // max payload size
    32   // max attribute size
};

RaytracingPipelineConfig so_RaytracingPipelineConfig =
{
    2 // max trace recursion depth
};

TriangleHitGroup MyHitGroup =
{
    "MyAnyHit",       // AnyHit
    "MyClosestHit",   // ClosestHit
};

SubobjectToExportsAssociation so_Association1 =
{
	"so_LocalRootSignature1", // subobject name
	"MyRayGen"                // export association 
};

SubobjectToExportsAssociation so_Association2 =
{
	"so_LocalRootSignature2", // subobject name
	"MyAnyHit"                // export association 
};

struct MyPayload
{
    float4 color;
};

[shader("raygeneration")]
void MyRayGen()
{
}

[shader("closesthit")]
void MyClosestHit(inout MyPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{  
}

[shader("anyhit")]
void MyAnyHit(inout MyPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
}

[shader("miss")]
void MyMiss(inout MyPayload payload)
{
}

)x";

  CComPtr<IDxcCompiler> pCompiler;
  VERIFY_SUCCEEDED(m_dllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));

  CComPtr<IDxcBlobEncoding> pSource;
  Utf8ToBlob(m_dllSupport, source, &pSource);

  CComPtr<IDxcOperationResult> pResult;
  VERIFY_SUCCEEDED(pCompiler->Compile(pSource, L"source.hlsl", L"", L"lib_6_6",
                                      nullptr, 0, nullptr, 0, nullptr,
                                      &pResult));
  VerifyOperationSucceeded(pResult);
  CComPtr<IDxcBlob> compiled;
  VERIFY_SUCCEEDED(pResult->GetResult(&compiled));

  auto optimizedContainer = RunShaderAccessTrackingPass(compiled).blob;

  const char *pBlobContent =
      reinterpret_cast<const char *>(optimizedContainer->GetBufferPointer());
  unsigned blobSize = optimizedContainer->GetBufferSize();
  const hlsl::DxilContainerHeader *pContainerHeader =
      hlsl::IsDxilContainerLike(pBlobContent, blobSize);

  const hlsl::DxilPartHeader *pPartHeader =
      GetDxilPartByType(pContainerHeader, hlsl::DFCC_RuntimeData);
  VERIFY_ARE_NOT_EQUAL(pPartHeader, nullptr);

  hlsl::RDAT::DxilRuntimeData rdat(GetDxilPartData(pPartHeader),
                                   pPartHeader->PartSize);

  auto const subObjectTableReader = rdat.GetSubobjectTable();

  // There are 9 subobjects in the HLSL above:
  VERIFY_ARE_EQUAL(subObjectTableReader.Count(), 9u);

  bool foundGlobalRS = false;
  for (uint32_t i = 0; i < subObjectTableReader.Count(); ++i) {
    auto subObject = subObjectTableReader[i];
    hlsl::DXIL::SubobjectKind subobjectKind = subObject.getKind();
    switch (subobjectKind) {
    case hlsl::DXIL::SubobjectKind::GlobalRootSignature: {
      foundGlobalRS = true;
      VERIFY_IS_TRUE(0 ==
                     strcmp(subObject.getName(), "so_GlobalRootSignature"));

      auto rootSigReader = subObject.getRootSignature();
      DxilVersionedRootSignatureDesc const *rootSignature = nullptr;
      DeserializeRootSignature(rootSigReader.getData(),
                               rootSigReader.sizeData(), &rootSignature);
      VERIFY_ARE_EQUAL(rootSignature->Version,
                       DxilRootSignatureVersion::Version_1_1);
      VERIFY_ARE_EQUAL(rootSignature->Desc_1_1.NumParameters, 2u);
      VERIFY_ARE_EQUAL(rootSignature->Desc_1_1.pParameters[1].ParameterType,
                       DxilRootParameterType::UAV);
      VERIFY_ARE_EQUAL(rootSignature->Desc_1_1.pParameters[1].ShaderVisibility,
                       DxilShaderVisibility::All);
      VERIFY_ARE_EQUAL(
          rootSignature->Desc_1_1.pParameters[1].Descriptor.RegisterSpace,
          static_cast<uint32_t>(-2));
      VERIFY_ARE_EQUAL(
          rootSignature->Desc_1_1.pParameters[1].Descriptor.ShaderRegister, 0u);
      DeleteRootSignature(rootSignature);
      break;
    }
    }
  }
  VERIFY_IS_TRUE(foundGlobalRS);
}

TEST_F(PixTest, RootSignatureUpgrade_Annotation) {

  const char *dynamicTextureAccess = R"x(
Texture1D<float4> tex[5] : register(t3);
SamplerState SS[3] : register(s2);

[RootSignature("DescriptorTable(SRV(t3, numDescriptors=5)),\
                DescriptorTable(Sampler(s2, numDescriptors=3))")]
float4 main(int i : A, float j : B) : SV_TARGET
{
  float4 r = tex[i].Sample(SS[i], i);
  return r;
}
  )x";

  auto compiled = Compile(m_dllSupport, dynamicTextureAccess, L"ps_6_6");
  auto pOptimizedContainer = RunShaderAccessTrackingPass(compiled).blob;

  const char *pBlobContent =
      reinterpret_cast<const char *>(pOptimizedContainer->GetBufferPointer());
  unsigned blobSize = pOptimizedContainer->GetBufferSize();
  const hlsl::DxilContainerHeader *pContainerHeader =
      hlsl::IsDxilContainerLike(pBlobContent, blobSize);

  const hlsl::DxilPartHeader *pPartHeader =
      GetDxilPartByType(pContainerHeader, hlsl::DFCC_RootSignature);
  VERIFY_ARE_NOT_EQUAL(pPartHeader, nullptr);

  hlsl::RootSignatureHandle RSH;
  RSH.LoadSerialized((const uint8_t *)GetDxilPartData(pPartHeader),
                     pPartHeader->PartSize);

  RSH.Deserialize();

  auto const *desc = RSH.GetDesc();

  bool foundGlobalRS = false;

  VERIFY_ARE_EQUAL(desc->Version, hlsl::DxilRootSignatureVersion::Version_1_1);
  VERIFY_ARE_EQUAL(desc->Desc_1_1.NumParameters, 3u);
  for (unsigned int i = 0; i < desc->Desc_1_1.NumParameters; ++i) {
    hlsl::DxilRootParameter1 const *param = desc->Desc_1_1.pParameters + i;
    switch (param->ParameterType) {
    case hlsl::DxilRootParameterType::UAV:
      VERIFY_ARE_EQUAL(param->Descriptor.RegisterSpace,
                       static_cast<uint32_t>(-2));
      VERIFY_ARE_EQUAL(param->Descriptor.ShaderRegister, 0u);
      foundGlobalRS = true;
      break;
    }
  }

  VERIFY_IS_TRUE(foundGlobalRS);
}

TEST_F(PixTest, DxilPIXDXRInvocationsLog_SanityTest) {

  const char *source = R"x(
struct MyPayload
{
    float4 color;
};

[shader("raygeneration")]
void MyRayGen()
{
}

[shader("closesthit")]
void MyClosestHit(inout MyPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
}

[shader("anyhit")]
void MyAnyHit(inout MyPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
}

[shader("miss")]
void MyMiss(inout MyPayload payload)
{
}

)x";

  auto compiledLib = Compile(m_dllSupport, source, L"lib_6_6", {});
  RunDxilPIXDXRInvocationsLog(compiledLib);
}

TEST_F(PixTest, DxilPIXDXRInvocationsLog_EmbeddedRootSigs) {

  const char *source = R"x(

GlobalRootSignature grs = {"CBV(b0)"};
struct MyPayload
{
    float4 color;
};

[shader("raygeneration")]
void MyRayGen()
{
}

[shader("closesthit")]
void MyClosestHit(inout MyPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
}

[shader("anyhit")]
void MyAnyHit(inout MyPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
}

[shader("miss")]
void MyMiss(inout MyPayload payload)
{
}

)x";

  auto compiledLib = Compile(m_dllSupport, source, L"lib_6_3",
                             {L"-Qstrip_reflect"}, L"RootSig");
  RunDxilPIXDXRInvocationsLog(compiledLib);
}

uint32_t NuriGetWaveInstructionCount(const std::vector<std::string> &lines) {
  // This is the instruction we'll insert into the shader if we detect dynamic
  // resource indexing
  const char *const waveActiveAllEqual = "call i1 @dx.op.waveActiveAllEqual";

  uint32_t instCount = 0;
  for (const std::string &line : lines) {
    instCount += line.find(waveActiveAllEqual) != std::string::npos;
  }
  return instCount;
}

void PixTest::TestNuriCase(const char *source, const wchar_t *target,
                           uint32_t expectedResult) {

  for (const OptimizationChoice &choice : OptimizationChoices) {
    const std::vector<LPCWSTR> compilationOptions = {choice.Flag};

    CComPtr<IDxcBlob> compiledLib =
        Compile(m_dllSupport, source, target, compilationOptions);

    std::string outputText;
    const std::vector<std::string> dxilLines =
        RunDxilNonUniformResourceIndexInstrumentation(compiledLib, outputText);

    VERIFY_ARE_EQUAL(NuriGetWaveInstructionCount(dxilLines), expectedResult);

    bool foundDynamicIndexingNoNuri = false;
    const std::vector<std::string> outputTextLines = Tokenize(outputText, "\n");
    for (const std::string &line : outputTextLines) {
      if (line.find("FoundDynamicIndexingNoNuri") != std::string::npos) {
        foundDynamicIndexingNoNuri = true;
        break;
      }
    }

    VERIFY_ARE_EQUAL((expectedResult != 0), foundDynamicIndexingNoNuri);
  }
}

TEST_F(PixTest, NonUniformResourceIndex_Resource) {

  const char *source = R"x(
Texture2D tex[] : register(t0);
float4 main(float2 uv : TEXCOORD0) : SV_TARGET
{
    uint index = uv.x * uv.y;
    return tex[index].Load(int3(0, 0, 0));
})x";

  const char *sourceWithNuri = R"x(
Texture2D tex[] : register(t0);
float4 main(float2 uv : TEXCOORD0) : SV_TARGET
{
    uint i = uv.x * uv.y;
    return tex[NonUniformResourceIndex(i)].Load(int3(0, 0, 0));
})x";

  TestNuriCase(source, L"ps_6_0", 1);
  TestNuriCase(sourceWithNuri, L"ps_6_0", 0);

  if (m_ver.SkipDxilVersion(1, 6)) {
    return;
  }

  TestNuriCase(source, L"ps_6_6", 1);
  TestNuriCase(sourceWithNuri, L"ps_6_6", 0);
}

TEST_F(PixTest, NonUniformResourceIndex_DescriptorHeap) {

  if (m_ver.SkipDxilVersion(1, 6)) {
    return;
  }

  const char *source = R"x(
Texture2D tex[] : register(t0);
float4 main(float2 uv : TEXCOORD0) : SV_TARGET
{
    uint i = uv.x + uv.y;
    Texture2D<float4> dynResTex = 
        ResourceDescriptorHeap[i];
    SamplerState dynResSampler = 
        SamplerDescriptorHeap[i];
    return dynResTex.Sample(dynResSampler, uv);
})x";

  const char *sourceWithNuri = R"x(
Texture2D tex[] : register(t0);
float4 main(float2 uv : TEXCOORD0) : SV_TARGET
{
    uint i = uv.x + uv.y;
    Texture2D<float4> dynResTex = 
        ResourceDescriptorHeap[NonUniformResourceIndex(i)];
    SamplerState dynResSampler = 
        SamplerDescriptorHeap[NonUniformResourceIndex(i)];
    return dynResTex.Sample(dynResSampler, uv);
})x";

  TestNuriCase(source, L"ps_6_6", 2);
  TestNuriCase(sourceWithNuri, L"ps_6_6", 0);
}

TEST_F(PixTest, NonUniformResourceIndex_Raytracing) {

  if (m_ver.SkipDxilVersion(1, 5)) {
    return;
  }

  const char *source = R"x(
RWTexture2D<float4> RT[] : register(u0);

[noinline]
void FuncNoInline(uint index)
{
    float2 rayIndex = DispatchRaysIndex().xy;
    uint i = index + rayIndex.x * rayIndex.y;
    float4 c = float4(0.5, 0.5, 0.5, 0);
    RT[i][rayIndex.xy] += c;
}

void Func(uint index)
{
    float2 rayIndex = DispatchRaysIndex().xy;
    uint i = index + rayIndex.y;
    float4 c = float4(0, 1, 0, 0);
    RT[i][rayIndex.xy] += c;
}

[shader("raygeneration")]
void Main()
{
    float2 rayIndex = DispatchRaysIndex().xy;

    uint i1 = rayIndex.x;
    float4 c1 = float4(1, 0, 1, 1);
    RT[i1][rayIndex.xy] += c1;

    uint i2 = rayIndex.x * rayIndex.y * 0.25;
    float4 c2 = float4(0.25, 0, 0.25, 0);
    RT[i2][rayIndex.xy] += c2;

    Func(i1);
    FuncNoInline(i2);
})x";

  const char *sourceWithNuri = R"x(
RWTexture2D<float4> RT[] : register(u0);

[noinline]
void FuncNoInline(uint index)
{
    float2 rayIndex = DispatchRaysIndex().xy;
    uint i = index + rayIndex.x * rayIndex.y;
    float4 c = float4(0.5, 0.5, 0.5, 0);
    RT[NonUniformResourceIndex(i)][rayIndex.xy] += c;
}

void Func(uint index)
{
    float2 rayIndex = DispatchRaysIndex().xy;
    uint i = index + rayIndex.y;
    float4 c = float4(0, 1, 0, 0);
    RT[NonUniformResourceIndex(i)][rayIndex.xy] += c;
}

[shader("raygeneration")]
void Main()
{
    float2 rayIndex = DispatchRaysIndex().xy;

    uint i1 = rayIndex.x;
    float4 c1 = float4(1, 0, 1, 1);
    RT[NonUniformResourceIndex(i1)][rayIndex.xy] += c1;

    uint i2 = rayIndex.x * rayIndex.y * 0.25;
    float4 c2 = float4(0.25, 0, 0.25, 0);
    RT[NonUniformResourceIndex(i2)][rayIndex.xy] += c2;

    Func(i1);
    FuncNoInline(i2);
})x";

  TestNuriCase(source, L"lib_6_5", 4);
  TestNuriCase(sourceWithNuri, L"lib_6_5", 0);
}

TEST_F(PixTest, DebugInstrumentation_TextOutput) {

  const char *source = R"x(
float4 main() : SV_Target {
    return float4(0,0,0,0);
})x";

  auto compiled = Compile(m_dllSupport, source, L"ps_6_0", {});
  auto output = RunDebugPass(compiled, 8 /*ludicrously low UAV size limit*/);
  bool foundStaticOverflow = false;
  bool foundCounterOffset = false;
  bool foundThreshold = false;
  for (auto const &line : output.lines) {
    if (line.find("StaticOverflow:12") != std::string::npos)
      foundStaticOverflow = true;
    if (line.find("InterestingCounterOffset:3") != std::string::npos)
      foundCounterOffset = true;
    if (line.find("OverflowThreshold:1") != std::string::npos)
      foundThreshold = true;
  }
  VERIFY_IS_TRUE(foundStaticOverflow);
}

TEST_F(PixTest, DebugInstrumentation_BlockReport) {

  const char *source = R"x(
RWStructuredBuffer<int> UAV: register(u0);
float4 main() : SV_Target {
    // basic int variable
    int v = UAV[0];
    if(v == 0)
        UAV[1] = v;
    else
        UAV[2] = v;
    // float with indexed alloca
    float f[2];
    f[0] = UAV[4];
    f[1] = UAV[5];
    if(v == 2)
        f[0] = v;
    else
        f[1] = v;
    float farray2[2];
    farray2[0] = UAV[4];
    farray2[1] = UAV[5];
    if(v == 4)
        farray2[0] = v;
    else
        farray2[1] = v;
    double d = UAV[8];
    int64_t i64 = UAV[9];
    return float4(d,i64,0,0);
})x";

  auto compiled = Compile(m_dllSupport, source, L"ps_6_0", {L"-Od"});
  auto output = RunDebugPass(compiled);
  bool foundBlock = false;
  bool foundRet = false;
  bool foundUnnumberedVoidProllyADXNothing = false;
  bool found32BitAssignment = false;
  bool foundFloatAssignment = false;
  bool foundDoubleAssignment = false;
  bool found64BitAssignment = false;
  bool found32BitAllocaStore = false;
  for (auto const &line : output.lines) {
    if (line.find("Block#") != std::string::npos) {
      if (line.find("r,0,r;") != std::string::npos)
        foundRet = true;
      if (line.find("v,0,v;") != std::string::npos)
        foundUnnumberedVoidProllyADXNothing = true;
      if (line.find("3,3,a;") != std::string::npos)
        found32BitAssignment = true;
      if (line.find("d,13,a;") != std::string::npos)
        foundDoubleAssignment = true;
      if (line.find("f,19,a;") != std::string::npos)
        foundFloatAssignment = true;
      if (line.find("6,16,a;") != std::string::npos)
        found64BitAssignment = true;
      if (line.find("3,3,s,2+0;") != std::string::npos)
        found32BitAllocaStore = true;
      foundBlock = true;
    }
  }
  VERIFY_IS_TRUE(foundBlock);
  VERIFY_IS_TRUE(foundRet);
  VERIFY_IS_TRUE(foundUnnumberedVoidProllyADXNothing);
  VERIFY_IS_TRUE(found32BitAssignment);
  VERIFY_IS_TRUE(found64BitAssignment);
  VERIFY_IS_TRUE(foundFloatAssignment);
  VERIFY_IS_TRUE(foundDoubleAssignment);
  VERIFY_IS_TRUE(found32BitAllocaStore);
}

std::string ExtractBracedSubstring(std::string const &line) {
  auto open = line.find('{');
  auto close = line.find('}');
  if (open != std::string::npos && close != std::string::npos &&
      open + 1 < close) {
    return line.substr(open + 1, close - open - 1);
  }
  return {};
}

int ExtractMetaInt32Value(std::string const &token) {
  auto findi32 = token.find("i32 ");
  if (findi32 != std::string_view::npos) {
    return atoi(
        std::string(token.data() + findi32 + 4, token.length() - (findi32 + 4))
            .c_str());
  }
  return -1;
}

std::vector<std::string> Split(std::string str, char delimeter) {
  std::vector<std::string> lines;

  auto const *p = str.data();
  auto const *justPastPreviousDelimiter = p;
  while (p < str.data() + str.length()) {
    if (*p == delimeter) {
      lines.emplace_back(std::string(justPastPreviousDelimiter,
                                     p - justPastPreviousDelimiter));
      justPastPreviousDelimiter = p + 1;
      p = justPastPreviousDelimiter;
    } else {
      p++;
    }
  }

  lines.emplace_back(
      std::string(justPastPreviousDelimiter, p - justPastPreviousDelimiter));

  return lines;
}

struct MetadataAllocaDefinition {
  int base;
  int count;
};
using AllocaDefinitions = std::map<int, MetadataAllocaDefinition>;
struct MetadataAllocaWrite {
  int allocaDefMetadataKey;
  int offset;
  int size;
};
using AllocaWrites = std::map<int, MetadataAllocaWrite>;

struct AllocaMetadata {
  AllocaDefinitions allocaDefinitions;
  AllocaWrites allocaWrites;
  std::vector<int> allocaWritesMetaKeys;
};

AllocaMetadata
FindAllocaRelatedMetadata(std::vector<std::string> const &lines) {

  const char *allocaMetaDataAssignment = "= !{i32 1, ";
  const char *allocaRegWRiteAssignment = "= !{i32 2, !";
  const char *allocaRegWriteTag = "!pix-alloca-reg-write !";

  AllocaMetadata ret;
  for (auto const &line : lines) {
    if (line[0] == '!') {
      auto key = atoi(std::string(line.data() + 1, line.length() - 1).c_str());
      if (key != -1) {
        if (line.find(allocaMetaDataAssignment) != std::string::npos) {
          std::string bitInBraces = ExtractBracedSubstring(line);
          if (bitInBraces != "") {
            auto tokens = Split(bitInBraces, ',');
            if (tokens.size() == 3) {
              auto value0 = ExtractMetaInt32Value(tokens[1]);
              auto value1 = ExtractMetaInt32Value(tokens[2]);
              if (value0 != -1 && value1 != -1) {
                MetadataAllocaDefinition def;
                def.base = value0;
                def.count = value1;
                ret.allocaDefinitions[key] = def;
              }
            }
          }
        } else if (line.find(allocaRegWRiteAssignment) != std::string::npos) {
          std::string bitInBraces = ExtractBracedSubstring(line);
          if (bitInBraces != "") {
            auto tokens = Split(bitInBraces, ',');
            if (tokens.size() == 4 && tokens[1][1] == '!') {
              auto allocaKey = atoi(tokens[1].c_str() + 2);
              auto value0 = ExtractMetaInt32Value(tokens[2]);
              auto value1 = ExtractMetaInt32Value(tokens[3]);
              if (value0 != -1 && value1 != -1) {
                MetadataAllocaWrite aw;
                aw.allocaDefMetadataKey = allocaKey;
                aw.size = value0;
                aw.offset = value1;
                ret.allocaWrites[key] = aw;
              }
            }
          }
        }
      }
    } else {
      auto findAw = line.find(allocaRegWriteTag);
      if (findAw != std::string::npos) {
        ret.allocaWritesMetaKeys.push_back(
            atoi(line.c_str() + findAw + strlen(allocaRegWriteTag)));
      }
    }
  }
  return ret;
}

TEST_F(PixTest, DebugInstrumentation_VectorAllocaWrite_Structs) {
  const char *source = R"x(
RaytracingAccelerationStructure Scene : register(t0, space0);
struct RayPayload
{
    float4 color;
};
RWStructuredBuffer<float> UAV: register(u0);
[shader("raygeneration")]
void RaygenInternalName()
{
    RayDesc ray;
    ray.Origin = float3(UAV[0], UAV[1],UAV[3]);
    ray.Direction = float3(4.4,5.5,6.6);
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 1, 0, 1) };
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);
})x";

  auto compiled = Compile(m_dllSupport, source, L"lib_6_6", {L"-Od"});
  auto output = RunDebugPass(compiled);
  auto disassembly = Disassemble(output.blob);
  auto lines = Split(disassembly, '\n');
  auto metaDataKeyToValue = FindAllocaRelatedMetadata(lines);
  // To validate that the RayDesc and RayPayload instances were fully covered,
  // check that there are alloca writes that cover all of them. RayPayload
  // has four elements, and RayDesc has eight.
  std::array<bool, 4> RayPayloadElementCoverage;

  for (auto const &write : metaDataKeyToValue.allocaWrites) {
    // the whole point of the changes with this test is to separate vector
    // writes into individual elements:
    VERIFY_ARE_EQUAL(1, write.second.size);
    auto findAlloca = metaDataKeyToValue.allocaDefinitions.find(
        write.second.allocaDefMetadataKey);
    if (findAlloca != metaDataKeyToValue.allocaDefinitions.end()) {
      if (findAlloca->second.count == 4) {
        RayPayloadElementCoverage[write.second.offset] = true;
      }
    }
  }
  // Check that coverage for every element was emitted:
  for (auto const &b : RayPayloadElementCoverage)
    VERIFY_IS_TRUE(b);
}
