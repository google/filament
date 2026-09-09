///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxbcUtil.h                                                                //
// Copyright (c) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Utilities to convert from DXBC to DXIL.                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Support/DXIncludes.h"

#include "dxc/DXIL/DxilCompType.h"
#include "dxc/DXIL/DxilConstants.h"
#include "dxc/DXIL/DxilInterpolationMode.h"
#include "dxc/DXIL/DxilResource.h"
#include "dxc/DXIL/DxilSampler.h"
#include "dxc/DXIL/DxilSemantic.h"
#include "dxc/DXIL/DxilShaderModel.h"

#include "llvm/IR/Instructions.h"

namespace llvm {
class Type;
class LLVMContext;
class Value;
} // namespace llvm

#define DXASSERT_DXBC(__exp)                                                   \
  DXASSERT(__exp, "otherwise incorrect assumption about DXBC")

namespace hlsl {

namespace DXBC {

// Width of DXBC vector operand.
const BYTE kWidth = 4;
// DXBC mask with all active components.
const BYTE kAllCompMask = 0x0F;

ShaderModel::Kind GetShaderModelKind(D3D10_SB_TOKENIZED_PROGRAM_TYPE Type);

// Query DXBC shader flags.
bool IsFlagDisableOptimizations(unsigned Flags);
bool IsFlagDisableMathRefactoring(unsigned Flags);
bool IsFlagEnableDoublePrecision(unsigned Flags);
bool IsFlagForceEarlyDepthStencil(unsigned Flags);
bool IsFlagEnableRawAndStructuredBuffers(unsigned Flags);
bool IsFlagEnableMinPrecision(unsigned Flags);
bool IsFlagEnableDoubleExtensions(unsigned Flags);
bool IsFlagEnableMSAD(unsigned Flags);
bool IsFlagAllResourcesBound(unsigned Flags);

InterpolationMode::Kind GetInterpolationModeKind(D3D_INTERPOLATION_MODE Mode);

D3D10_SB_OPERAND_TYPE GetOperandRegType(Semantic::Kind Kind, bool IsOutput);

DxilResource::Kind GetResourceKind(D3D10_SB_RESOURCE_DIMENSION ResType);
BYTE GetNumResCoords(DxilResource::Kind ResKind);
BYTE GetNumResOffsets(DxilResource::Kind ResKind);

CompType GetCompType(D3D_REGISTER_COMPONENT_TYPE CompTy);
CompType GetCompTypeWithMinPrec(D3D_REGISTER_COMPONENT_TYPE BaseCompTy,
                                D3D11_SB_OPERAND_MIN_PRECISION MinPrec);
CompType GetCompTypeWithMinPrec(CompType BaseCompTy,
                                D3D11_SB_OPERAND_MIN_PRECISION MinPrec);
CompType GetCompTypeFromMinPrec(D3D11_SB_OPERAND_MIN_PRECISION MinPrec,
                                CompType DefaultPrecCompType);

CompType GetResCompType(D3D10_SB_RESOURCE_RETURN_TYPE CompTy);
CompType GetDeclResCompType(D3D10_SB_RESOURCE_RETURN_TYPE CompTy);

char GetCompName(BYTE c);

DxilSampler::SamplerKind GetSamplerKind(D3D10_SB_SAMPLER_MODE Mode);

unsigned GetRegIndex(unsigned Reg, unsigned Comp);

DXIL::AtomicBinOpCode GetAtomicBinOp(D3D10_SB_OPCODE_TYPE DxbcOpCode);
llvm::AtomicRMWInst::BinOp GetLlvmAtomicBinOp(D3D10_SB_OPCODE_TYPE DxbcOpCode);
bool AtomicBinOpHasReturn(D3D10_SB_OPCODE_TYPE DxbcOpCode);
bool IsCompareExchAtomicBinOp(D3D10_SB_OPCODE_TYPE DxbcOpCode);

bool HasFeedback(D3D10_SB_OPCODE_TYPE OpCode);
unsigned GetResourceSlot(D3D10_SB_OPCODE_TYPE OpCode);

DXIL::BarrierMode GetBarrierMode(bool bSyncThreadGroup, bool bUAVFenceGlobal,
                                 bool bUAVFenceThreadGroup, bool bTGSMFence);

DXIL::InputPrimitive GetInputPrimitive(D3D10_SB_PRIMITIVE Primitive);
DXIL::PrimitiveTopology
GetPrimitiveTopology(D3D10_SB_PRIMITIVE_TOPOLOGY Topology);

const char *GetD3D10SBName(D3D10_SB_NAME D3DName);
unsigned GetD3D10SBSemanticIndex(D3D10_SB_NAME D3DName);
D3D_REGISTER_COMPONENT_TYPE GetD3DRegCompType(D3D10_SB_NAME D3DName);
const char *GetSemanticNameFromD3DName(D3D_NAME D3DName);
unsigned GetSemanticIndexFromD3DName(D3D_NAME D3DName);

DXIL::TessellatorDomain
GetTessellatorDomain(D3D11_SB_TESSELLATOR_DOMAIN TessDomain);
DXIL::TessellatorPartitioning
GetTessellatorPartitioning(D3D11_SB_TESSELLATOR_PARTITIONING TessPartitioning);
DXIL::TessellatorOutputPrimitive GetTessellatorOutputPrimitive(
    D3D11_SB_TESSELLATOR_OUTPUT_PRIMITIVE TessOutputPrimitive);

} // namespace DXBC

/// Use this class to represent DXBC register component mask.
class CMask {
public:
  CMask();
  CMask(BYTE Mask);
  CMask(BYTE c0, BYTE c1, BYTE c2, BYTE c3);
  CMask(BYTE StartComp, BYTE NumComp);

  BYTE ToByte() const;

  static bool IsSet(BYTE Mask, BYTE c);
  bool IsSet(BYTE c) const;
  void Set(BYTE c);

  CMask operator|(const CMask &o);

  BYTE GetNumActiveComps() const;
  BYTE GetNumActiveRangeComps() const;
  bool IsZero() const { return GetNumActiveComps() == 0; }

  BYTE GetFirstActiveComp() const;

  static BYTE MakeMask(BYTE c0, BYTE c1, BYTE c2, BYTE c3);
  static CMask MakeXYZWMask();
  static CMask MakeFirstNCompMask(BYTE n);
  static CMask MakeCompMask(BYTE Component);
  static CMask MakeXMask();

  static bool IsValidDoubleMask(const CMask &Mask);
  static CMask GetMaskForDoubleOperation(const CMask &Mask);

  static CMask FromDXBC(const unsigned DxbcMask);

protected:
  BYTE m_Mask;
};

/// Use this class to pass around DXBC register component values.
class OperandValue {
  friend class OperandValueHelper;
  typedef llvm::Value *PValue;
  PValue m_pVal[DXBC::kWidth];

public:
  OperandValue();
  PValue &operator[](BYTE c);
  const PValue &operator[](BYTE c) const;
};

/// \brief Use this one-time-iterator class to set up component values of input
/// operands, replicating the same value to all components with the same
/// swizzled name.
///
/// After creation an instance serves as an iterator to iterate through
/// uniques components and set their values in the OperandValue instance.
/// After the iterator is done, the instance is not usable anymore.
///
/// Usage:
///    OperandValueHelper OVH(OpVal, Mask, Swizzle);
///    for (; !OVH.IsDone(); OVH.Advance()) {
///      BYTE Comp = OVH.GetComp();
///      ...  // Create llvm::Value *pVal
///      OHV.SetValue(pVal); // for all components with the same swizzle name
///      }
class OperandValueHelper {
public:
  OperandValueHelper();
  OperandValueHelper(OperandValue &OpValue, const CMask &Mask,
                     const D3D10ShaderBinary::COperandBase &O);

  /// Returns the value of the current active wrt to Mask component.
  BYTE GetComp() const;
  /// Returns true is there are no more active components.
  bool IsDone() const;
  /// Advances the iterator to the next unique, active component.
  void Advance();
  /// Sets the value of all active components with the same swizzle name in
  /// OperandValue OpValue.
  void SetValue(llvm::Value *pValue);

private:
  static const BYTE kBadComp = 0xFF;
  OperandValue *m_pOpValue;
  BYTE m_Components[DXBC::kWidth];
  BYTE m_Index;

  void Initialize(const CMask &Mask, const BYTE CompSwizzle[DXBC::kWidth]);
};

} // namespace hlsl
