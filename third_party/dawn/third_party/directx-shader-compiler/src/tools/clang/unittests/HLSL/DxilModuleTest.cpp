///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// DxilModuleTest.cpp                                                        //
//                                                                           //
// Provides unit tests for DxilModule.                                       //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DXIL/DxilModule.h"
#include "dxc/DXIL/DxilInstructions.h"
#include "dxc/DXIL/DxilOperations.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/HLSL/HLOperationLowerExtension.h"
#include "dxc/HlslIntrinsicOp.h"
#include "dxc/Support/microcom.h"
#include "dxc/Test/CompilationResult.h"
#include "dxc/Test/DxcTestUtils.h"
#include "dxc/Test/HlslTestUtils.h"
#include "dxc/dxcapi.internal.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MSFileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Regex.h"

using namespace hlsl;
using namespace llvm;

///////////////////////////////////////////////////////////////////////////////
// DxilModule unit tests.

#ifdef _WIN32
class DxilModuleTest {
#else
class DxilModuleTest : public ::testing::Test {
#endif
public:
  BEGIN_TEST_CLASS(DxilModuleTest)
  TEST_CLASS_PROPERTY(L"Parallel", L"true")
  TEST_METHOD_PROPERTY(L"Priority", L"0")
  END_TEST_CLASS()

  TEST_CLASS_SETUP(InitSupport);

  dxc::DxCompilerDllLoader m_dllSupport;
  VersionSupportInfo m_ver;

  // Basic loading tests.
  TEST_METHOD(LoadDxilModule_1_0)
  TEST_METHOD(LoadDxilModule_1_1)
  TEST_METHOD(LoadDxilModule_1_2)

  // Precise query tests.
  TEST_METHOD(Precise1)
  TEST_METHOD(Precise2)
  TEST_METHOD(Precise3)
  TEST_METHOD(Precise4)
  TEST_METHOD(Precise5)
  TEST_METHOD(Precise6)
  TEST_METHOD(Precise7)

  TEST_METHOD(CSGetNumThreads)
  TEST_METHOD(MSGetNumThreads)
  TEST_METHOD(ASGetNumThreads)

  TEST_METHOD(SetValidatorVersion)

  TEST_METHOD(PayloadQualifier)

  TEST_METHOD(CanonicalSystemValueSemantic)

  void VerifyValidatorVersionFails(LPCWSTR shaderModel,
                                   const std::vector<LPCWSTR> &arguments,
                                   const std::vector<LPCSTR> &expectedErrors);
  void VerifyValidatorVersionMatches(LPCWSTR shaderModel,
                                     const std::vector<LPCWSTR> &arguments,
                                     unsigned expectedMajor = UINT_MAX,
                                     unsigned expectedMinor = UINT_MAX);
};

bool DxilModuleTest::InitSupport() {
  if (!m_dllSupport.IsEnabled()) {
    VERIFY_SUCCEEDED(m_dllSupport.Initialize());
    m_ver.Initialize(m_dllSupport);
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Compilation and dxil module loading support.

namespace {
class Compiler {
public:
  Compiler(dxc::DxCompilerDllLoader &dll)
      : m_dllSupport(dll), m_msf(CreateMSFileSystem()), m_pts(m_msf.get()) {
    m_ver.Initialize(m_dllSupport);
    VERIFY_SUCCEEDED(
        m_dllSupport.CreateInstance(CLSID_DxcCompiler, &pCompiler));
  }

  bool SkipDxil_Test(unsigned major, unsigned minor) {
    return m_ver.SkipDxilVersion(major, minor);
  }

  IDxcOperationResult *Compile(const char *program,
                               LPCWSTR shaderModel = L"ps_6_0") {
    return Compile(program, shaderModel, {}, {});
  }

  IDxcOperationResult *Compile(const char *program, LPCWSTR shaderModel,
                               const std::vector<LPCWSTR> &arguments,
                               const std::vector<DxcDefine> defs) {
    Utf8ToBlob(m_dllSupport, program, &pCodeBlob);
    VERIFY_SUCCEEDED(pCompiler->Compile(
        pCodeBlob, L"hlsl.hlsl", L"main", shaderModel,
        const_cast<LPCWSTR *>(arguments.data()), arguments.size(), defs.data(),
        defs.size(), nullptr, &pCompileResult));

    return pCompileResult;
  }

  std::string Disassemble() {
    CComPtr<IDxcBlob> pBlob;
    CheckOperationSucceeded(pCompileResult, &pBlob);
    return DisassembleProgram(m_dllSupport, pBlob);
  }

  DxilModule &GetDxilModule() {
    // Make sure we compiled successfully.
    CComPtr<IDxcBlob> pBlob;
    CheckOperationSucceeded(pCompileResult, &pBlob);

    // Verify we have a valid dxil container.
    const DxilContainerHeader *pContainer =
        IsDxilContainerLike(pBlob->GetBufferPointer(), pBlob->GetBufferSize());
    VERIFY_IS_NOT_NULL(pContainer);
    VERIFY_IS_TRUE(IsValidDxilContainer(pContainer, pBlob->GetBufferSize()));

    // Get Dxil part from container.
    DxilPartIterator it = std::find_if(begin(pContainer), end(pContainer),
                                       DxilPartIsType(DFCC_DXIL));
    VERIFY_IS_FALSE(it == end(pContainer));

    const DxilProgramHeader *pProgramHeader =
        reinterpret_cast<const DxilProgramHeader *>(GetDxilPartData(*it));
    VERIFY_IS_TRUE(IsValidDxilProgramHeader(pProgramHeader, (*it)->PartSize));

    // Get a pointer to the llvm bitcode.
    const char *pIL;
    uint32_t pILLength;
    GetDxilProgramBitcode(pProgramHeader, &pIL, &pILLength);

    // Parse llvm bitcode into a module.
    std::unique_ptr<llvm::MemoryBuffer> pBitcodeBuf(
        llvm::MemoryBuffer::getMemBuffer(llvm::StringRef(pIL, pILLength), "",
                                         false));
    llvm::ErrorOr<std::unique_ptr<llvm::Module>> pModule(
        llvm::parseBitcodeFile(pBitcodeBuf->getMemBufferRef(), m_llvmContext));
    if (std::error_code ec = pModule.getError()) {
      VERIFY_FAIL();
    }
    m_module = std::move(pModule.get());

    // Grab the dxil module;
    DxilModule *DM = DxilModule::TryGetDxilModule(m_module.get());
    VERIFY_IS_NOT_NULL(DM);
    return *DM;
  }

public:
  static ::llvm::sys::fs::MSFileSystem *CreateMSFileSystem() {
    ::llvm::sys::fs::MSFileSystem *msfPtr;
    VERIFY_SUCCEEDED(CreateMSFileSystemForDisk(&msfPtr));
    return msfPtr;
  }

  dxc::DxCompilerDllLoader &m_dllSupport;
  VersionSupportInfo m_ver;
  CComPtr<IDxcCompiler> pCompiler;
  CComPtr<IDxcBlobEncoding> pCodeBlob;
  CComPtr<IDxcOperationResult> pCompileResult;
  llvm::LLVMContext m_llvmContext;
  std::unique_ptr<llvm::Module> m_module;
  std::unique_ptr<::llvm::sys::fs::MSFileSystem> m_msf;
  ::llvm::sys::fs::AutoPerThreadSystem m_pts;
};
} // namespace

///////////////////////////////////////////////////////////////////////////////
// Unit Test Implementation
TEST_F(DxilModuleTest, LoadDxilModule_1_0) {
  Compiler c(m_dllSupport);
  c.Compile("float4 main() : SV_Target {\n"
            "  return 0;\n"
            "}\n",
            L"ps_6_0");

  // Basic sanity check on dxil version in dxil module.
  DxilModule &DM = c.GetDxilModule();
  unsigned vMajor, vMinor;
  DM.GetDxilVersion(vMajor, vMinor);
  VERIFY_IS_TRUE(vMajor == 1);
  VERIFY_IS_TRUE(vMinor == 0);
}

TEST_F(DxilModuleTest, LoadDxilModule_1_1) {
  Compiler c(m_dllSupport);
  if (c.SkipDxil_Test(1, 1))
    return;
  c.Compile("float4 main() : SV_Target {\n"
            "  return 0;\n"
            "}\n",
            L"ps_6_1");

  // Basic sanity check on dxil version in dxil module.
  DxilModule &DM = c.GetDxilModule();
  unsigned vMajor, vMinor;
  DM.GetDxilVersion(vMajor, vMinor);
  VERIFY_IS_TRUE(vMajor == 1);
  VERIFY_IS_TRUE(vMinor == 1);
}

TEST_F(DxilModuleTest, LoadDxilModule_1_2) {
  Compiler c(m_dllSupport);
  if (c.SkipDxil_Test(1, 2))
    return;
  c.Compile("float4 main() : SV_Target {\n"
            "  return 0;\n"
            "}\n",
            L"ps_6_2");

  // Basic sanity check on dxil version in dxil module.
  DxilModule &DM = c.GetDxilModule();
  unsigned vMajor, vMinor;
  DM.GetDxilVersion(vMajor, vMinor);
  VERIFY_IS_TRUE(vMajor == 1);
  VERIFY_IS_TRUE(vMinor == 2);
}

TEST_F(DxilModuleTest, Precise1) {
  Compiler c(m_dllSupport);
  c.Compile("precise float main(float x : X, float y : Y) : SV_Target {\n"
            "  return sqrt(x) + y;\n"
            "}\n");

  // Make sure sqrt and add are marked precise.
  DxilModule &DM = c.GetDxilModule();
  Function *F = DM.GetEntryFunction();
  int numChecks = 0;
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *Inst = &*I;
    if (DxilInst_Sqrt(Inst)) {
      numChecks++;
      VERIFY_IS_TRUE(DM.IsPrecise(Inst));
    } else if (LlvmInst_FAdd(Inst)) {
      numChecks++;
      VERIFY_IS_TRUE(DM.IsPrecise(Inst));
    }
  }
  VERIFY_ARE_EQUAL(numChecks, 2);
}

TEST_F(DxilModuleTest, Precise2) {
  Compiler c(m_dllSupport);
  c.Compile("float main(float x : X, float y : Y) : SV_Target {\n"
            "  return sqrt(x) + y;\n"
            "}\n");

  // Make sure sqrt and add are not marked precise.
  DxilModule &DM = c.GetDxilModule();
  Function *F = DM.GetEntryFunction();
  int numChecks = 0;
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *Inst = &*I;
    if (DxilInst_Sqrt(Inst)) {
      numChecks++;
      VERIFY_IS_FALSE(DM.IsPrecise(Inst));
    } else if (LlvmInst_FAdd(Inst)) {
      numChecks++;
      VERIFY_IS_FALSE(DM.IsPrecise(Inst));
    }
  }
  VERIFY_ARE_EQUAL(numChecks, 2);
}

TEST_F(DxilModuleTest, Precise3) {
  // TODO: Enable this test when precise metadata is inserted for Gis.
  if (const bool GisIsBroken = true)
    return;
  Compiler c(m_dllSupport);
  c.Compile("float main(float x : X, float y : Y) : SV_Target {\n"
            "  return sqrt(x) + y;\n"
            "}\n",
            L"ps_6_0", {L"/Gis"}, {});

  // Make sure sqrt and add are marked precise.
  DxilModule &DM = c.GetDxilModule();
  Function *F = DM.GetEntryFunction();
  int numChecks = 0;
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *Inst = &*I;
    if (DxilInst_Sqrt(Inst)) {
      numChecks++;
      VERIFY_IS_TRUE(DM.IsPrecise(Inst));
    } else if (LlvmInst_FAdd(Inst)) {
      numChecks++;
      VERIFY_IS_TRUE(DM.IsPrecise(Inst));
    }
  }
  VERIFY_ARE_EQUAL(numChecks, 2);
}

TEST_F(DxilModuleTest, Precise4) {
  Compiler c(m_dllSupport);
  c.Compile("float main(float x : X, float y : Y) : SV_Target {\n"
            "  precise float sx = 1 / sqrt(x);\n"
            "  return sx + y;\n"
            "}\n");

  // Make sure sqrt and div are marked precise, and add is not.
  DxilModule &DM = c.GetDxilModule();
  Function *F = DM.GetEntryFunction();
  int numChecks = 0;
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *Inst = &*I;
    if (DxilInst_Sqrt(Inst)) {
      numChecks++;
      VERIFY_IS_TRUE(DM.IsPrecise(Inst));
    } else if (LlvmInst_FDiv(Inst)) {
      numChecks++;
      VERIFY_IS_TRUE(DM.IsPrecise(Inst));
    } else if (LlvmInst_FAdd(Inst)) {
      numChecks++;
      VERIFY_IS_FALSE(DM.IsPrecise(Inst));
    }
  }
  VERIFY_ARE_EQUAL(numChecks, 3);
}

TEST_F(DxilModuleTest, Precise5) {
  Compiler c(m_dllSupport);
  c.Compile("float C[10];\n"
            "float main(float x : X, float y : Y, int i : I) : SV_Target {\n"
            "  float A[2];\n"
            "  A[0] = x;\n"
            "  A[1] = y;\n"
            "  return A[i] + C[i];\n"
            "}\n");

  // Make sure load and extract value are not reported as precise.
  DxilModule &DM = c.GetDxilModule();
  Function *F = DM.GetEntryFunction();
  int numChecks = 0;
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *Inst = &*I;
    if (LlvmInst_ExtractValue(Inst)) {
      numChecks++;
      VERIFY_IS_FALSE(DM.IsPrecise(Inst));
    } else if (LlvmInst_Load(Inst)) {
      numChecks++;
      VERIFY_IS_FALSE(DM.IsPrecise(Inst));
    } else if (LlvmInst_FAdd(Inst)) {
      numChecks++;
      VERIFY_IS_FALSE(DM.IsPrecise(Inst));
    }
  }
  VERIFY_ARE_EQUAL(numChecks, 3);
}

TEST_F(DxilModuleTest, Precise6) {
  Compiler c(m_dllSupport);
  c.Compile("precise float2 main(float2 x : A, float2 y : B) : SV_Target {\n"
            "  return sqrt(x * y);\n"
            "}\n");

  // Make sure sqrt and mul are marked precise.
  DxilModule &DM = c.GetDxilModule();
  Function *F = DM.GetEntryFunction();
  int numChecks = 0;
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *Inst = &*I;
    if (DxilInst_Sqrt(Inst)) {
      numChecks++;
      VERIFY_IS_TRUE(DM.IsPrecise(Inst));
    } else if (LlvmInst_FMul(Inst)) {
      numChecks++;
      VERIFY_IS_TRUE(DM.IsPrecise(Inst));
    }
  }
  VERIFY_ARE_EQUAL(numChecks, 4);
}

TEST_F(DxilModuleTest, Precise7) {
  Compiler c(m_dllSupport);
  c.Compile("float2 main(float2 x : A, float2 y : B) : SV_Target {\n"
            "  return sqrt(x * y);\n"
            "}\n");

  // Make sure sqrt and mul are not marked precise.
  DxilModule &DM = c.GetDxilModule();
  Function *F = DM.GetEntryFunction();
  int numChecks = 0;
  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    Instruction *Inst = &*I;
    if (DxilInst_Sqrt(Inst)) {
      numChecks++;
      VERIFY_IS_FALSE(DM.IsPrecise(Inst));
    } else if (LlvmInst_FMul(Inst)) {
      numChecks++;
      VERIFY_IS_FALSE(DM.IsPrecise(Inst));
    }
  }
  VERIFY_ARE_EQUAL(numChecks, 4);
}

TEST_F(DxilModuleTest, CSGetNumThreads) {
  Compiler c(m_dllSupport);
  c.Compile("[numthreads(8, 4, 2)]\n"
            "void main() {\n"
            "}\n",
            L"cs_6_0");

  DxilModule &DM = c.GetDxilModule();
  VERIFY_ARE_EQUAL(8u, DM.GetNumThreads(0));
  VERIFY_ARE_EQUAL(4u, DM.GetNumThreads(1));
  VERIFY_ARE_EQUAL(2u, DM.GetNumThreads(2));
}

TEST_F(DxilModuleTest, MSGetNumThreads) {
  Compiler c(m_dllSupport);
  if (c.SkipDxil_Test(1, 5))
    return;
  c.Compile("struct MeshPerVertex { float4 pos : SV_Position; };\n"
            "[numthreads(8, 4, 2)]\n"
            "[outputtopology(\"triangle\")]\n"
            "void main(\n"
            "          out indices uint3 primIndices[1]\n"
            ") {\n"
            "    SetMeshOutputCounts(0, 0);\n"
            "}\n",
            L"ms_6_5");

  DxilModule &DM = c.GetDxilModule();
  VERIFY_ARE_EQUAL(8u, DM.GetNumThreads(0));
  VERIFY_ARE_EQUAL(4u, DM.GetNumThreads(1));
  VERIFY_ARE_EQUAL(2u, DM.GetNumThreads(2));
}

TEST_F(DxilModuleTest, ASGetNumThreads) {
  Compiler c(m_dllSupport);
  if (c.SkipDxil_Test(1, 5))
    return;
  c.Compile("struct Payload { uint i; };\n"
            "[numthreads(8, 4, 2)]\n"
            "void main() {\n"
            "  Payload pld = {0};\n"
            "    DispatchMesh(1, 1, 1, pld);\n"
            "}\n",
            L"as_6_5");

  DxilModule &DM = c.GetDxilModule();
  VERIFY_ARE_EQUAL(8u, DM.GetNumThreads(0));
  VERIFY_ARE_EQUAL(4u, DM.GetNumThreads(1));
  VERIFY_ARE_EQUAL(2u, DM.GetNumThreads(2));
}

void DxilModuleTest::VerifyValidatorVersionFails(
    LPCWSTR shaderModel, const std::vector<LPCWSTR> &arguments,
    const std::vector<LPCSTR> &expectedErrors) {

  LPCSTR shader = "[shader(\"pixel\")]"
                  "float4 main() : SV_Target {\n"
                  "  return 0;\n"
                  "}\n";

  Compiler c(m_dllSupport);
  c.Compile(shader, shaderModel, arguments, {});
  CheckOperationResultMsgs(c.pCompileResult, expectedErrors, false, false);
}

void DxilModuleTest::VerifyValidatorVersionMatches(
    LPCWSTR shaderModel, const std::vector<LPCWSTR> &arguments,
    unsigned expectedMajor, unsigned expectedMinor) {

  LPCSTR shader = "[shader(\"pixel\")]"
                  "float4 main() : SV_Target {\n"
                  "  return 0;\n"
                  "}\n";

  Compiler c(m_dllSupport);
  c.Compile(shader, shaderModel, arguments, {});
  DxilModule &DM = c.GetDxilModule();
  unsigned vMajor, vMinor;
  DM.GetValidatorVersion(vMajor, vMinor);

  if (expectedMajor == UINT_MAX) {
    // Expect current version
    VERIFY_ARE_EQUAL(vMajor, c.m_ver.m_ValMajor);
    VERIFY_ARE_EQUAL(vMinor, c.m_ver.m_ValMinor);
  } else {
    VERIFY_ARE_EQUAL(vMajor, expectedMajor);
    VERIFY_ARE_EQUAL(vMinor, expectedMinor);
  }
}

TEST_F(DxilModuleTest, SetValidatorVersion) {
  Compiler c(m_dllSupport);
  if (c.SkipDxil_Test(1, 4))
    return;

  // Current version
  VerifyValidatorVersionMatches(L"ps_6_2", {});
  VerifyValidatorVersionMatches(L"lib_6_3", {});

  // Current version, with validation disabled
  VerifyValidatorVersionMatches(L"ps_6_2", {L"-Vd"});
  VerifyValidatorVersionMatches(L"lib_6_3", {L"-Vd"});

  // Override validator version
  VerifyValidatorVersionMatches(L"ps_6_2", {L"-validator-version", L"1.2"}, 1,
                                2);
  VerifyValidatorVersionMatches(L"lib_6_3", {L"-validator-version", L"1.3"}, 1,
                                3);

  // Override validator version, with validation disabled
  VerifyValidatorVersionMatches(L"ps_6_2",
                                {L"-Vd", L"-validator-version", L"1.2"}, 1, 2);
  VerifyValidatorVersionMatches(L"lib_6_3",
                                {L"-Vd", L"-validator-version", L"1.3"}, 1, 3);

  // Never can validate (version 0,0):
  VerifyValidatorVersionMatches(L"lib_6_1", {L"-Vd"}, 0, 0);
  VerifyValidatorVersionMatches(L"lib_6_2", {L"-Vd"}, 0, 0);
  VerifyValidatorVersionMatches(L"lib_6_2",
                                {L"-Vd", L"-validator-version", L"0.0"}, 0, 0);
  VerifyValidatorVersionMatches(L"lib_6_x", {}, 0, 0);
  VerifyValidatorVersionMatches(L"lib_6_x", {L"-validator-version", L"0.0"}, 0,
                                0);

  // Failure cases:
  VerifyValidatorVersionFails(
      L"ps_6_2", {L"-validator-version", L"1.1"},
      {"validator version 1,1 does not support target profile."});

  VerifyValidatorVersionFails(
      L"lib_6_2", {L"-Tlib_6_2"},
      {"Must disable validation for unsupported lib_6_1 or lib_6_2 targets"});

  VerifyValidatorVersionFails(L"lib_6_2",
                              {L"-Vd", L"-validator-version", L"1.2"},
                              {"-validator-version cannot be used with library "
                               "profiles lib_6_1 or lib_6_2."});

  VerifyValidatorVersionFails(L"lib_6_x", {L"-validator-version", L"1.3"},
                              {"Offline library profile cannot be used with "
                               "non-zero -validator-version."});
}

TEST_F(DxilModuleTest, PayloadQualifier) {
  if (m_ver.SkipDxilVersion(1, 6))
    return;
  std::vector<LPCWSTR> arguments = {L"-enable-payload-qualifiers"};
  Compiler c(m_dllSupport);

  LPCSTR shader = "struct [raypayload] Payload\n"
                  "{\n"
                  "  double a : read(caller, closesthit, anyhit) : "
                  "write(caller, miss, closesthit);\n"
                  "  int b : read(caller) : write(miss);\n"
                  "};\n\n"
                  "[shader(\"miss\")]\n"
                  "void Miss( inout Payload payload ) { payload.a = 4.2; "
                  "payload.b = 1; }\n";

  c.Compile(shader, L"lib_6_6", arguments, {});

  DxilModule &DM = c.GetDxilModule();
  const DxilTypeSystem &DTS = DM.GetTypeSystem();

  for (auto &p : DTS.GetPayloadAnnotationMap()) {
    const DxilPayloadAnnotation &plAnnotation = *p.second;
    {
      const DxilPayloadFieldAnnotation &fieldAnnotation =
          plAnnotation.GetFieldAnnotation(0);
      VERIFY_IS_TRUE(fieldAnnotation.HasAnnotations());
      VERIFY_ARE_EQUAL(DXIL::PayloadAccessQualifier::ReadWrite,
                       fieldAnnotation.GetPayloadFieldQualifier(
                           DXIL::PayloadAccessShaderStage::Caller));
      VERIFY_ARE_EQUAL(DXIL::PayloadAccessQualifier::ReadWrite,
                       fieldAnnotation.GetPayloadFieldQualifier(
                           DXIL::PayloadAccessShaderStage::Closesthit));
      VERIFY_ARE_EQUAL(DXIL::PayloadAccessQualifier::Write,
                       fieldAnnotation.GetPayloadFieldQualifier(
                           DXIL::PayloadAccessShaderStage::Miss));
      VERIFY_ARE_EQUAL(DXIL::PayloadAccessQualifier::Read,
                       fieldAnnotation.GetPayloadFieldQualifier(
                           DXIL::PayloadAccessShaderStage::Anyhit));
    }
    {
      const DxilPayloadFieldAnnotation &fieldAnnotation =
          plAnnotation.GetFieldAnnotation(1);
      VERIFY_IS_TRUE(fieldAnnotation.HasAnnotations());
      VERIFY_ARE_EQUAL(DXIL::PayloadAccessQualifier::Read,
                       fieldAnnotation.GetPayloadFieldQualifier(
                           DXIL::PayloadAccessShaderStage::Caller));
      VERIFY_ARE_EQUAL(DXIL::PayloadAccessQualifier::NoAccess,
                       fieldAnnotation.GetPayloadFieldQualifier(
                           DXIL::PayloadAccessShaderStage::Closesthit));
      VERIFY_ARE_EQUAL(DXIL::PayloadAccessQualifier::Write,
                       fieldAnnotation.GetPayloadFieldQualifier(
                           DXIL::PayloadAccessShaderStage::Miss));
      VERIFY_ARE_EQUAL(DXIL::PayloadAccessQualifier::NoAccess,
                       fieldAnnotation.GetPayloadFieldQualifier(
                           DXIL::PayloadAccessShaderStage::Anyhit));
    }
  }
}

TEST_F(DxilModuleTest, CanonicalSystemValueSemantic) {
  // Verify that the semantic name is canonicalized.

  // This is difficult to do from a FileCheck test because system value
  // semantics get canonicallized during the metadata emit.  However, having a
  // non-canonical semantic name at earlier stages can lead to inconsistency
  // between the strings used earlier and the final canonicalized version.  This
  // makes sure the string gets canonicalized when the signature elsement is
  // created.

  std::unique_ptr<hlsl::DxilSignatureElement> newElt =
      std::make_unique<hlsl::DxilSignatureElement>(
          hlsl::DXIL::SigPointKind::VSOut);
  newElt->Initialize("sV_pOsItIoN", hlsl::CompType::getF32(),
                     hlsl::InterpolationMode(
                         hlsl::DXIL::InterpolationMode::LinearNoperspective),
                     1, 4, 0, 0, 0, {0});
  VERIFY_ARE_EQUAL_STR("SV_Position", newElt->GetSemanticName().data());
}
