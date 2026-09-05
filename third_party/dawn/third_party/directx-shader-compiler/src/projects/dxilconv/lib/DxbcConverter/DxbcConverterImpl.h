///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxbcConverterImpl.h                                                       //
// Copyright (c) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Utilities to convert from DXBC to DXIL.                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "dxc/DXIL/DXIL.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/DxilContainer/DxilContainerReader.h"
#include "dxc/Support/Global.h"
#include "llvm/Analysis/ReducibilityAnalysis.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Bitcode/BitstreamWriter.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileOutputBuffer.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Scalar.h"

#include "Support/DXIncludes.h"
#include "dxc/Support/microcom.h"
#include <atlbase.h>

#include "dxc/Support/FileIOHelper.h"
#include "dxc/dxcapi.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MSFileSystem.h"

#include "DxbcConverter.h"
#include "DxbcUtil.h"

#include "dxc/DxilContainer/DxilPipelineStateValidation.h"

#include "Tracing/DxcRuntimeEtw.h"

#include <algorithm>
#include <map>
#include <vector>

#pragma once
namespace llvm {
using legacy::FunctionPassManager;
using legacy::PassManager;
using legacy::PassManagerBase;
} // namespace llvm

using namespace llvm;
using std::map;
using std::pair;
using std::string;
using std::unique_ptr;
using std::vector;
using std::wstring;

struct D3D12DDIARG_SIGNATURE_ENTRY_0012 {
  D3D10_SB_NAME SystemValue;
  UINT Register;
  BYTE Mask;
  BYTE Stream;
  D3D10_SB_REGISTER_COMPONENT_TYPE RegisterComponentType;
  D3D11_SB_OPERAND_MIN_PRECISION MinPrecision;
};

namespace hlsl {

#define DXBC_FOURCC(ch0, ch1, ch2, ch3)                                        \
  ((UINT)(BYTE)(ch0) | ((UINT)(BYTE)(ch1) << 8) | ((UINT)(BYTE)(ch2) << 16) |  \
   ((UINT)(BYTE)(ch3) << 24))

enum DXBCFourCC {
  DXBC_GenericShader = DXBC_FOURCC('S', 'H', 'D', 'R'),
  DXBC_GenericShaderEx = DXBC_FOURCC('S', 'H', 'E', 'X'),
  DXBC_InputSignature = DXBC_FOURCC('I', 'S', 'G', 'N'),
  DXBC_InputSignature11_1 =
      DXBC_FOURCC('I', 'S', 'G', '1'), // == DFCC_InputSignature
  DXBC_PatchConstantSignature = DXBC_FOURCC('P', 'C', 'S', 'G'),
  DXBC_PatchConstantSignature11_1 =
      DXBC_FOURCC('P', 'S', 'G', '1'), // == DFCC_PatchConstantSignature
  DXBC_OutputSignature = DXBC_FOURCC('O', 'S', 'G', 'N'),
  DXBC_OutputSignature5 = DXBC_FOURCC('O', 'S', 'G', '5'),
  DXBC_OutputSignature11_1 =
      DXBC_FOURCC('O', 'S', 'G', '1'), // == DFCC_OutputSignature
  DXBC_ShaderFeatureInfo =
      DXBC_FOURCC('S', 'F', 'I', '0'),                  // == DFCC_FeatureInfo
  DXBC_RootSignature = DXBC_FOURCC('R', 'T', 'S', '0'), // == DFCC_RootSignature
  DXBC_DXIL = DXBC_FOURCC('D', 'X', 'I', 'L'),          // == DFCC_DXIL
  DXBC_PipelineStateValidation =
      DXBC_FOURCC('P', 'S', 'V', '0'), // == DFCC_PipelineStateValidation
};
#undef DXBC_FOURCC

/// Use this class to parse DXBC signatures.
class SignatureHelper {
public:
  // Signature elements.
  DxilSignature m_Signature;

  // Use this to represent signature element record that comes from either:
  // (1) DXBC signature blob, or (2) DDI signature vector.
  struct ElementRecord {
    string SemanticName;
    unsigned SemanticIndex;
    unsigned StartRow;
    unsigned StartCol;
    unsigned Rows;
    unsigned Cols;
    unsigned Stream;
    CompType ComponentType;
  };
  vector<ElementRecord> m_ElementRecords;

  // Use this to represent register range declaration.
  struct Range {
    unsigned StartRow;
    unsigned StartCol;
    unsigned Rows;
    unsigned Cols;
    BYTE OutputStream;

    Range()
        : StartRow(UINT_MAX), StartCol(UINT_MAX), Rows(0), Cols(0),
          OutputStream(0) {}

    unsigned GetStartRow() const { return StartRow; }
    unsigned GetStartCol() const { return StartCol; }
    unsigned GetEndRow() const { return StartRow + Rows - 1; }
    unsigned GetEndCol() const { return StartCol + Cols - 1; }

    struct LTRangeByStreamAndStartRowAndStartCol {
      bool operator()(const Range &e1, const Range &e2) const {
        if (e1.OutputStream < e2.OutputStream)
          return true;
        else if (e1.OutputStream == e2.OutputStream) {
          if (e1.StartRow < e2.StartRow)
            return true;
          else if (e1.StartRow == e2.StartRow)
            return e1.StartCol < e2.StartCol;
          else
            return false; // e1.StartRow > e2.StartRow
        } else
          return false; // e1.OutputStream > e2.OutputStream
      }
    };
  };

  vector<Range> m_Ranges;

  // Use this to represent input/output/tessellation register declaration.
  struct UsedElement {
    unsigned Row;
    unsigned StartCol;
    unsigned Cols;
    D3D_INTERPOLATION_MODE InterpolationMode;
    D3D11_SB_OPERAND_MIN_PRECISION MinPrecision;
    unsigned NumUnits;
    BYTE OutputStream;

    UsedElement()
        : Row(UINT_MAX), StartCol(UINT_MAX), Cols(0),
          InterpolationMode(D3D_INTERPOLATION_UNDEFINED),
          MinPrecision(D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT), NumUnits(0),
          OutputStream(0) {}

    struct LTByStreamAndStartRowAndStartCol {
      bool operator()(const UsedElement &e1, const UsedElement &e2) const {
        if (e1.OutputStream < e2.OutputStream)
          return true;
        else if (e1.OutputStream == e2.OutputStream) {
          if (e1.Row < e2.Row)
            return true;
          else if (e1.Row == e2.Row)
            return e1.StartCol < e2.StartCol;
          else
            return false; // e1.Row > e2.Row
        } else
          return false; // e1.OutputStream > e2.OutputStream
      }
    };
  };

  // Assume the vector is sorted by <stream,row,col>.
  vector<UsedElement> m_UsedElements;

  // Elements with stream, register and component.
  struct RegAndCompAndStream {
    unsigned Reg;
    unsigned Comp;
    unsigned Stream;
    RegAndCompAndStream(unsigned r, unsigned c, unsigned s)
        : Reg(r), Comp(c), Stream(s) {}
    bool operator<(const RegAndCompAndStream &o) const {
      if (Stream < o.Stream)
        return true;
      else if (Stream == o.Stream) {
        if (Reg < o.Reg)
          return true;
        else if (Reg == o.Reg)
          return Comp < o.Comp;
        else
          return false;
      } else
        return false;
    }
  };
  map<RegAndCompAndStream, unsigned> m_DxbcRegisterToSignatureElement;

  const DxilSignatureElement *GetElement(unsigned Reg, unsigned Comp) const {
    const unsigned Stream = 0;
    return GetElementWithStream(Reg, Comp, Stream);
  }
  const DxilSignatureElement *GetElementWithStream(unsigned Reg, unsigned Comp,
                                                   unsigned Stream) const {
    RegAndCompAndStream Key(Reg, Comp, Stream);
    auto it = m_DxbcRegisterToSignatureElement.find(Key);
    if (it == m_DxbcRegisterToSignatureElement.end()) {
      return nullptr;
    }
    unsigned ElemIdx = it->second;
    const DxilSignatureElement *E = &m_Signature.GetElement(ElemIdx);
    DXASSERT(E->IsAllocated(),
             "otherwise signature elements were not set correctly");
    DXASSERT(E->GetStartRow() <= (int)Reg &&
                 Reg < E->GetStartRow() + E->GetRows(),
             "otherwise signature elements were not set correctly");
    DXASSERT(E->GetStartCol() <= (int)Comp &&
                 Comp < E->GetStartCol() + E->GetCols(),
             "otherwise signature elements were not set correctly");
    return E;
  }

  // Elements that are System Generated Values (SVGs), without register.
  map<D3D10_SB_OPERAND_TYPE, unsigned> m_DxbcSgvToSignatureElement;

  const DxilSignatureElement *
  GetElement(D3D10_SB_OPERAND_TYPE SgvRegType) const {
    DXASSERT(m_DxbcSgvToSignatureElement.find(SgvRegType) !=
                 m_DxbcSgvToSignatureElement.end(),
             "otherwise the element has not been added to the map");
    unsigned ElemIdx = m_DxbcSgvToSignatureElement.find(SgvRegType)->second;
    const DxilSignatureElement *E = &m_Signature.GetElement(ElemIdx);
    DXASSERT(!E->IsAllocated(),
             "otherwise signature elements were not set correctly");
    return E;
  }

  bool IsInput() const { return m_Signature.IsInput(); }
  bool IsOutput() const { return m_Signature.IsOutput(); }

  // Special case SGVs that are not in the signature.
  bool m_bHasInputCoverage;
  bool m_bHasInnerInputCoverage;

  SignatureHelper(DXIL::ShaderKind shaderKind, DXIL::SignatureKind sigKind)
      : m_Signature(shaderKind, sigKind, /*useMinPrecision*/ false),
        m_bHasInputCoverage(false), m_bHasInnerInputCoverage(false) {}
};

/// Use this class to implement the IDxbcConverter inteface for DXBC to DXIL
/// translation.
class DxbcConverter : public IDxbcConverter {
protected:
  DXC_MICROCOM_TM_REF_FIELDS();

public:
  DXC_MICROCOM_TM_ADDREF_RELEASE_IMPL();

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) override {
    return DoBasicQueryInterface<IDxbcConverter>(this, iid, ppv);
  }

  DxbcConverter();
  DxbcConverter(IMalloc *pMalloc) : DxbcConverter() { m_pMalloc = pMalloc; }
  DXC_MICROCOM_TM_ALLOC(DxbcConverter);

  ~DxbcConverter();

  HRESULT STDMETHODCALLTYPE Convert(LPCVOID pDxbc, UINT32 DxbcSize,
                                    LPCWSTR pExtraOptions, LPVOID *ppDxil,
                                    UINT32 *pDxilSize, LPWSTR *ppDiag) override;

  HRESULT STDMETHODCALLTYPE ConvertInDriver(
      const UINT32 *pBytecode, LPCVOID pInputSignature,
      UINT32 NumInputSignatureElements, LPCVOID pOutputSignature,
      UINT32 NumOutputSignatureElements, LPCVOID pPatchConstantSignature,
      UINT32 NumPatchConstantSignatureElements, LPCWSTR pExtraOptions,
      IDxcBlob **ppDxilModule, LPWSTR *ppDiag) override;

protected:
  LLVMContext m_Ctx;
  DxilModule *m_pPR;
  std::unique_ptr<Module> m_pModule;
  OP *m_pOP;
  const ShaderModel *m_pSM;
  unsigned m_DxbcMajor;
  unsigned m_DxbcMinor;
  bool IsSM51Plus() const {
    return m_DxbcMajor > 5 || (m_DxbcMajor == 5 && m_DxbcMinor >= 1);
  }
  std::unique_ptr<IRBuilder<>> m_pBuilder;

  bool m_bDisableHashCheck;
  bool m_bRunDxilCleanup;

  bool m_bLegacyCBufferLoad;

  unique_ptr<SignatureHelper> m_pInputSignature;
  unique_ptr<SignatureHelper> m_pOutputSignature;
  unique_ptr<SignatureHelper> m_pPatchConstantSignature;
  D3D10_SB_OPERAND_TYPE m_DepthRegType;
  bool m_bHasStencilRef;
  bool m_bHasCoverageOut;

  const unsigned kRegCompAlignment = 4;
  const unsigned kLegacyCBufferRegSizeInBytes = 16;

  Value *m_pUnusedF32;
  Value *m_pUnusedI32;

  // Temporary r-registers.
  unsigned m_NumTempRegs;

  // Indexable temporary registers.
  struct IndexableReg {
    Value *pValue32;
    Value *pValue16;
    unsigned NumRegs;
    unsigned NumComps;
    bool bIsAlloca;
  };
  map<unsigned, IndexableReg> m_IndexableRegs;
  map<unsigned, IndexableReg> m_PatchConstantIndexableRegs;

  // Shader resource register/rangeID maps.
  map<unsigned, unsigned> m_SRVRangeMap;
  map<unsigned, unsigned> m_UAVRangeMap;
  map<unsigned, unsigned> m_CBufferRangeMap;
  map<unsigned, unsigned> m_SamplerRangeMap;

  // Cached handles for SM 5.0 or below, key: (Class, LowerBound).
  map<std::pair<unsigned, unsigned>, Value *> m_HandleMap;

  // Immediate constant buffer.
  GlobalVariable *m_pIcbGV;

  // Control flow.
  struct Scope {
    enum Kind : unsigned { Function, If, Loop, Switch, HullLoop, LastKind };

    enum Kind Kind;
    BasicBlock *pPreScopeBB;
    BasicBlock *pPostScopeBB;
    unsigned NameIndex;

    union {
      // If
      struct {
        BasicBlock *pThenBB;
        BasicBlock *pElseBB;
        Value *pCond;
      };

      // Loop
      struct {
        BasicBlock *pLoopBB;
        unsigned ContinueIndex;
        unsigned LoopBreakIndex;
      };

      // Switch
      struct {
        BasicBlock *pDefaultBB;
        Value *pSelector;
        unsigned CaseGroupIndex;
        unsigned SwitchBreakIndex;
      };

      // Function
      struct {
        unsigned LabelIdx;
        unsigned CallIdx;
        unsigned ReturnTokenOffset;
        unsigned ReturnIndex;
        bool bEntryFunc;
      };

      // HullLoop
      struct {
        BasicBlock *pHullLoopBB;
        unsigned HullLoopBreakIndex;
        Value *pInductionVar;
        unsigned HullLoopTripCount;
      };
    };
    vector<pair<unsigned, BasicBlock *>> SwitchCases; // Switch

    Scope()
        : Kind(Kind::Function), pPreScopeBB(nullptr), pPostScopeBB(nullptr),
          NameIndex(0) {
      memset(reinterpret_cast<char *>(&pThenBB), '\0',
             reinterpret_cast<char *>(&SwitchCases) -
                 reinterpret_cast<char *>(&pThenBB));
    }

    void SetEntry(bool b = true) {
      DXASSERT_NOMSG(Kind == Function);
      bEntryFunc = b;
    }
    bool IsEntry() const {
      DXASSERT_NOMSG(Kind == Function);
      return bEntryFunc;
    }
  };

  class ScopeStack {
  public:
    ScopeStack();
    Scope &Top();
    Scope &Push(enum Scope::Kind Kind, BasicBlock *pPreScopeBB);
    void Pop();
    bool IsEmpty() const;
    Scope &FindParentLoop();
    Scope &FindParentLoopOrSwitch();
    Scope &FindParentFunction();
    Scope &FindParentHullLoop();

  private:
    vector<Scope> m_Scopes;
    unsigned m_FuncCount;
    unsigned m_IfCount;
    unsigned m_LoopCount;
    unsigned m_SwitchCount;
    unsigned m_HullLoopCount;
  };
  ScopeStack m_ScopeStack;

  struct LabelEntry {
    Function *pFunc;
  };
  map<unsigned, LabelEntry> m_Labels;
  map<unsigned, LabelEntry> m_InterfaceFunctionBodies;
  bool HasLabels() {
    return !m_Labels.empty() || !m_InterfaceFunctionBodies.empty();
  }

  // Shared memory.
  struct TGSMEntry {
    GlobalVariable *pVar;
    unsigned Stride;
    unsigned Count;
    unsigned Id;
  };
  map<unsigned, TGSMEntry> m_TGSMMap;
  unsigned m_TGSMCount;

  // Geometry shader.
  unsigned GetGSTempRegForOutputReg(unsigned OutputReg) const;

  // Hull shader.
  bool m_bControlPointPhase;
  bool m_bPatchConstantPhase;
  vector<unsigned> m_PatchConstantPhaseInstanceCounts;

  CMask m_PreciseMask;

  // Interfaces
  DxilCBuffer *m_pInterfaceDataBuffer;
  DxilCBuffer *m_pClassInstanceCBuffers;
  DxilSampler *m_pClassInstanceSamplers;
  DxilSampler *m_pClassInstanceComparisonSamplers;

  struct InterfaceShaderResourceKey {
    DxilResource::Kind Kind;
    union {
      DXIL::ComponentType TypedSRVRet;
      unsigned StructureByteStride;
    };
    bool operator<(const InterfaceShaderResourceKey &o) const {
      if (Kind != o.Kind)
        return Kind < o.Kind;
      if (Kind == DxilResource::Kind::StructuredBuffer)
        return StructureByteStride < o.StructureByteStride;
      if (Kind != DxilResource::Kind::RawBuffer)
        return TypedSRVRet < o.TypedSRVRet;
      return false;
    }
  };
  map<InterfaceShaderResourceKey, unsigned> m_ClassInstanceSRVs;
  map<unsigned, vector<unsigned>> m_FunctionTables;
  struct Interface {
    vector<unsigned> Tables;
    bool bDynamicallyIndexed;
    unsigned NumArrayEntries;
  };
  map<unsigned, Interface> m_Interfaces;
  unsigned m_NumIfaces;
  unsigned m_FcallCount;

protected:
  virtual void ConvertImpl(LPCVOID pDxbc, UINT32 DxbcSize,
                           LPCWSTR pExtraOptions, LPVOID *ppDxil,
                           UINT32 *pDxilSize, LPWSTR *ppDiag);

  virtual void ConvertInDriverImpl(
      const UINT32 *pByteCode,
      const D3D12DDIARG_SIGNATURE_ENTRY_0012 *pInputSignature,
      UINT32 NumInputSignatureElements,
      const D3D12DDIARG_SIGNATURE_ENTRY_0012 *pOutputSignature,
      UINT32 NumOutputSignatureElements,
      const D3D12DDIARG_SIGNATURE_ENTRY_0012 *pPatchConstantSignature,
      UINT32 NumPatchConstantSignatureElements, LPCWSTR pExtraOptions,
      IDxcBlob **ppDxcBlob, LPWSTR *ppDiag);

  virtual void LogConvertResult(bool InDriver,
                                const LARGE_INTEGER *pQPCConvertStart,
                                const LARGE_INTEGER *pQPCConvertEnd,
                                LPCVOID pDxbc, UINT32 DxbcSize,
                                LPCWSTR pExtraOptions, LPCVOID pConverted,
                                UINT32 ConvertedSize, HRESULT hr);

  // Callbacks added to support conversion of custom intrinsics.
  virtual HRESULT PreConvertHook(const CShaderToken *pByteCode);
  virtual HRESULT PostConvertHook(const CShaderToken *pByteCode);
  virtual void HandleUnknownInstruction(D3D10ShaderBinary::CInstruction &Inst);
  virtual unsigned GetResourceSlot(D3D10ShaderBinary::CInstruction &Inst);

protected:
  void ParseExtraOptions(const wchar_t *pStr);

  void AnalyzeShader(D3D10ShaderBinary::CShaderCodeParser &Parser);

  void ExtractInputSignatureFromDXBC(DxilContainerReader &dxbcReader,
                                     const void *pMaxPtr);
  void ExtractOutputSignatureFromDXBC(DxilContainerReader &dxbcReader,
                                      const void *pMaxPtr);
  void ExtractPatchConstantSignatureFromDXBC(DxilContainerReader &dxbcReader,
                                             const void *pMaxPtr);
  void ExtractSignatureFromDXBC(const D3D10_INTERNALSHADER_SIGNATURE *pSig,
                                UINT uElemSize, const void *pMaxPtr,
                                SignatureHelper &SigHelper);
  void
  ExtractSignatureFromDDI(const D3D12DDIARG_SIGNATURE_ENTRY_0012 *pElements,
                          unsigned NumElements, SignatureHelper &SigHelper);
  /// Correlates information from decls and signature element records to create
  /// DXIL signature element.
  void ConvertSignature(SignatureHelper &SigHelper, DxilSignature &Sig);

  void ConvertInstructions(D3D10ShaderBinary::CShaderCodeParser &Parser);
  void
  AdvanceDxbcInstructionStream(D3D10ShaderBinary::CShaderCodeParser &Parser,
                               D3D10ShaderBinary::CInstruction &Inst,
                               bool &bDoneParsing);
  bool GetNextDxbcInstruction(D3D10ShaderBinary::CShaderCodeParser &Parser,
                              D3D10ShaderBinary::CInstruction &NextInst);
  void InsertSM50ResourceHandles();
  void InsertInterfacesResourceDecls();
  const DxilResource &
  GetInterfacesSRVDecl(D3D10ShaderBinary::CInstruction &Inst);
  void DeclareIndexableRegisters();
  void CleanupIndexableRegisterDecls(map<unsigned, IndexableReg> &IdxRegMap);
  void RemoveUnreachableBasicBlocks();
  void CleanupGEP();

  void ConvertUnary(OP::OpCode OpCode, const CompType &ElementType,
                    D3D10ShaderBinary::CInstruction &Inst,
                    const unsigned DstIdx = 0, const unsigned SrcIdx = 1);
  void ConvertBinary(OP::OpCode OpCode, const CompType &ElementType,
                     D3D10ShaderBinary::CInstruction &Inst,
                     const unsigned DstIdx = 0, const unsigned SrcIdx1 = 1,
                     const unsigned SrcIdx2 = 2);
  void ConvertBinary(Instruction::BinaryOps OpCode, const CompType &ElementType,
                     D3D10ShaderBinary::CInstruction &Inst,
                     const unsigned DstIdx = 0, const unsigned SrcIdx1 = 1,
                     const unsigned SrcIdx2 = 2);
  void ConvertBinaryWithTwoOuts(OP::OpCode OpCode,
                                D3D10ShaderBinary::CInstruction &Inst,
                                const unsigned DstIdx1 = 0,
                                const unsigned DstIdx2 = 1,
                                const unsigned SrcIdx1 = 2,
                                const unsigned SrcIdx2 = 3);
  void ConvertBinaryWithCarry(OP::OpCode OpCode,
                              D3D10ShaderBinary::CInstruction &Inst,
                              const unsigned DstIdx1 = 0,
                              const unsigned DstIdx2 = 1,
                              const unsigned SrcIdx1 = 2,
                              const unsigned SrcIdx2 = 3);
  void ConvertTertiary(OP::OpCode OpCode, const CompType &ElementType,
                       D3D10ShaderBinary::CInstruction &Inst,
                       const unsigned DstIdx = 0, const unsigned SrcIdx1 = 1,
                       const unsigned SrcIdx2 = 2, const unsigned SrcIdx3 = 3);
  void ConvertQuaternary(OP::OpCode OpCode, const CompType &ElementType,
                         D3D10ShaderBinary::CInstruction &Inst,
                         const unsigned DstIdx = 0, const unsigned SrcIdx1 = 1,
                         const unsigned SrcIdx2 = 2, const unsigned SrcIdx3 = 3,
                         const unsigned SrcIdx4 = 4);
  void ConvertComparison(CmpInst::Predicate Predicate,
                         const CompType &ElementType,
                         D3D10ShaderBinary::CInstruction &Inst,
                         const unsigned DstIdx = 0, const unsigned SrcIdx1 = 1,
                         const unsigned SrcIdx2 = 2);
  void ConvertDotProduct(OP::OpCode OpCode, const BYTE NumComps,
                         const CMask &LoadMask,
                         D3D10ShaderBinary::CInstruction &Inst);
  void ConvertCast(const CompType &SrcElementType,
                   const CompType &DstElementType,
                   D3D10ShaderBinary::CInstruction &Inst,
                   const unsigned DstIdx = 0, const unsigned SrcIdx = 1);
  void ConvertToDouble(const CompType &SrcElementType,
                       D3D10ShaderBinary::CInstruction &Inst);
  void ConvertFromDouble(const CompType &DstElementType,
                         D3D10ShaderBinary::CInstruction &Inst);
  void LoadCommonSampleInputs(D3D10ShaderBinary::CInstruction &Inst,
                              Value *pArgs[], bool bSetOffsets = true);
  void StoreResRetOutputAndStatus(D3D10ShaderBinary::CInstruction &Inst,
                                  Value *pResRet, CompType DstType);
  void StoreGetDimensionsOutput(D3D10ShaderBinary::CInstruction &Inst,
                                Value *pGetDimRet);
  void StoreSamplePosOutput(D3D10ShaderBinary::CInstruction &Inst,
                            Value *pSamplePosVal);
  void StoreBroadcastOutput(D3D10ShaderBinary::CInstruction &Inst,
                            Value *pValue, CompType DstType);
  Value *GetCoordValue(D3D10ShaderBinary::CInstruction &Inst,
                       const unsigned uCoordIdx);
  Value *GetByteOffset(D3D10ShaderBinary::CInstruction &Inst,
                       const unsigned Idx1, const unsigned Idx2,
                       const unsigned Stride);
  void ConvertLoadTGSM(D3D10ShaderBinary::CInstruction &Inst,
                       const unsigned uOpTGSM, const unsigned uOpOutput,
                       CompType SrcType, Value *pByteOffset);
  void ConvertStoreTGSM(D3D10ShaderBinary::CInstruction &Inst,
                        const unsigned uOpTGSM, const unsigned uOpValue,
                        CompType BaseValueType, Value *pByteOffset);

  void EmitGSOutputRegisterStore(unsigned StreamId);

  void SetShaderGlobalFlags(unsigned GlobalFlags);
  Value *CreateHandle(DxilResourceBase::Class Class, unsigned RangeID,
                      Value *pIndex, bool bNonUniformIndex);
  void SetCachedHandle(const DxilResourceBase &R);
  Value *GetCachedHandle(const DxilResourceBase &R);

  void Optimize();

  void CheckDxbcString(const char *pStr, const void *pMaxPtrInclusive);

  Value *LoadConstFloat(float &fVal);
  void SetHasCounter(D3D10ShaderBinary::CInstruction &Inst,
                     const unsigned uOpUAV);
  void LoadOperand(OperandValue &SrcVal, D3D10ShaderBinary::CInstruction &Inst,
                   const unsigned OpIdx, const CMask &Mask,
                   const CompType &ValueType);
  const DxilResource &LoadSRVOperand(OperandValue &SrcVal,
                                     D3D10ShaderBinary::CInstruction &Inst,
                                     const unsigned OpIdx, const CMask &Mask,
                                     const CompType &ValueType);
  const DxilResource &GetSRVFromOperand(D3D10ShaderBinary::CInstruction &Inst,
                                        const unsigned OpIdx);
  void StoreOperand(OperandValue &DstVal,
                    const D3D10ShaderBinary::CInstruction &Inst,
                    const unsigned OpIdx, const CMask &Mask,
                    const CompType &ValueType);
  Value *
  LoadOperandIndex(const D3D10ShaderBinary::COperandIndex &OpIndex,
                   const D3D10_SB_OPERAND_INDEX_REPRESENTATION IndexType);
  Value *
  LoadOperandIndexRelative(const D3D10ShaderBinary::COperandIndex &OpIndex);
  /// Implicit casts of a value.
  Value *CastDxbcValue(Value *pValue, const CompType &SrcType,
                       const CompType &DstType);
  Value *CreateBitCast(Value *pValue, const CompType &SrcType,
                       const CompType &DstType);
  Value *ApplyOperandModifiers(Value *pValue,
                               const D3D10ShaderBinary::COperandBase &O);
  void ApplyInstructionModifiers(OperandValue &DstVal,
                                 const D3D10ShaderBinary::CInstruction &Inst);
  CompType InferOperandType(const D3D10ShaderBinary::CInstruction &Inst,
                            const unsigned OpIdx, const CMask &Mask);

  void CreateBranchIfNeeded(BasicBlock *pBB, BasicBlock *pTargetBB);
  Value *LoadZNZCondition(D3D10ShaderBinary::CInstruction &Inst,
                          const unsigned OpIdx);
  D3D11_SB_OPERAND_MIN_PRECISION
  GetHigherPrecision(D3D11_SB_OPERAND_MIN_PRECISION p1,
                     D3D11_SB_OPERAND_MIN_PRECISION p2);

  string SynthesizeResGVName(const char *pNamePrefix, unsigned ID);
  StructType *GetStructResElemType(unsigned StructSizeInBytes);
  StructType *GetTypedResElemType(CompType CT);
  UndefValue *DeclareUndefPtr(Type *pType, unsigned AddrSpace);
  Value *MarkPrecise(Value *pVal, BYTE Comp = BYTE(-1));

  void SerializeDxil(SmallVectorImpl<char> &DxilBitcode);
};
} // namespace hlsl
