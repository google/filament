///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxbcUtil.cpp                                                              //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Utilities to convert from DXBC to DXIL.                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DXIL/DxilResource.h"
#include "dxc/DXIL/DxilSampler.h"
#include "dxc/Support/Global.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Casting.h"

#include "DxbcUtil.h"
#include "Support/DXIncludes.h"

using namespace llvm;

namespace hlsl {

//------------------------------------------------------------------------------
//
//  CMask methods.
//
CMask::CMask() : m_Mask(0) {}

CMask::CMask(BYTE Mask) : m_Mask(Mask) {
  DXASSERT(Mask <= DXBC::kAllCompMask, "otherwise the caller did not check");
}

CMask::CMask(BYTE c0, BYTE c1, BYTE c2, BYTE c3) {
  DXASSERT(c0 <= 1 && c1 <= 1 && c2 <= 1 && c3 <= 1,
           "otherwise the caller did not check");
  m_Mask = c0 | (c1 << 1) | (c2 << 2) | (c3 << 3);
}

CMask::CMask(BYTE StartComp, BYTE NumComp) {
  DXASSERT(StartComp < DXBC::kAllCompMask && NumComp <= DXBC::kAllCompMask &&
               (StartComp + NumComp - 1) < DXBC::kAllCompMask,
           "otherwise the caller did not check");
  m_Mask = 0;
  for (BYTE c = StartComp; c < StartComp + NumComp; c++) {
    m_Mask |= (1 << c);
  }
}

BYTE CMask::ToByte() const {
  DXASSERT(m_Mask <= DXBC::kAllCompMask, "otherwise the caller did not check");
  return m_Mask;
}

bool CMask::IsSet(BYTE c) const {
  DXASSERT(c < DXBC::kWidth, "otherwise the caller did not check");
  return (m_Mask & (1 << c)) != 0;
}

void CMask::Set(BYTE c) {
  DXASSERT(c < DXBC::kWidth, "otherwise the caller did not check");
  m_Mask = m_Mask | (1 << c);
}

CMask CMask::operator|(const CMask &o) { return CMask(m_Mask | o.m_Mask); }

BYTE CMask::GetNumActiveComps() const {
  DXASSERT(m_Mask <= DXBC::kAllCompMask, "otherwise the caller did not check");
  BYTE n = 0;
  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    n += (m_Mask >> c) & 1;
  }
  return n;
}

BYTE CMask::GetNumActiveRangeComps() const {
  DXASSERT(m_Mask <= DXBC::kAllCompMask, "otherwise the caller did not check");
  if ((m_Mask & DXBC::kAllCompMask) == 0)
    return 0;

  BYTE FirstComp = 0;
  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if (m_Mask & (1 << c)) {
      FirstComp = c;
      break;
    }
  }
  BYTE LastComp = 0;
  for (BYTE c1 = 0; c1 < DXBC::kWidth; c1++) {
    BYTE c = DXBC::kWidth - 1 - c1;
    if (m_Mask & (1 << c)) {
      LastComp = c;
      break;
    }
  }

  return LastComp - FirstComp + 1;
}

BYTE CMask::MakeMask(BYTE c0, BYTE c1, BYTE c2, BYTE c3) {
  return CMask(c0, c1, c2, c3).ToByte();
}

CMask CMask::MakeXYZWMask() { return CMask(DXBC::kAllCompMask); }

CMask CMask::MakeFirstNCompMask(BYTE n) {
  switch (n) {
  case 0:
    return CMask(0, 0, 0, 0);
  case 1:
    return CMask(1, 0, 0, 0);
  case 2:
    return CMask(1, 1, 0, 0);
  case 3:
    return CMask(1, 1, 1, 0);
  default:
    DXASSERT(
        n == 4,
        "otherwise the caller did not pass the right number of components");
    return CMask(1, 1, 1, 1);
  }
}

CMask CMask::MakeCompMask(BYTE Component) {
  DXASSERT(
      Component < DXBC::kWidth,
      "otherwise the caller should have checked that the mask is non-zero");
  return CMask((BYTE)(1 << Component));
}

CMask CMask::MakeXMask() { return MakeCompMask(0); }

bool CMask::IsValidDoubleMask(const CMask &Mask) {
  BYTE b = Mask.ToByte();
  return b == 0xF || b == 0xC || b == 0x3;
}

CMask CMask::GetMaskForDoubleOperation(const CMask &Mask) {
  switch (Mask.GetNumActiveComps()) {
  case 0:
    return CMask(0, 0, 0, 0);
  case 1:
    return CMask(1, 1, 0, 0);
  case 2:
    return CMask(1, 1, 1, 1);
  }
  DXASSERT(false, "otherwise missed a case");
  return CMask();
}

BYTE CMask::GetFirstActiveComp() const {
  DXASSERT(
      m_Mask > 0,
      "otherwise the caller should have checked that the mask is non-zero");
  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    if ((m_Mask >> c) & 1)
      return c;
  }
  return _UI8_MAX;
}

CMask CMask::FromDXBC(const unsigned DxbcMask) {
  return CMask(DxbcMask >> D3D10_SB_OPERAND_4_COMPONENT_MASK_SHIFT);
}

//------------------------------------------------------------------------------
//
//  OperandValue methods.
//
OperandValue::OperandValue() {
  m_pVal[0] = m_pVal[1] = m_pVal[2] = m_pVal[3] = nullptr;
}

OperandValue::PValue &OperandValue::operator[](BYTE c) {
  DXASSERT_NOMSG(c < DXBC::kWidth);
  return m_pVal[c];
}

const OperandValue::PValue &OperandValue::operator[](BYTE c) const {
  DXASSERT_NOMSG(c < DXBC::kWidth);
  DXASSERT(m_pVal[c] != nullptr,
           "otherwise required component value has not been set");
  return m_pVal[c];
}

//------------------------------------------------------------------------------
//
//  OperandValue methods.
//
OperandValueHelper::OperandValueHelper()
    : m_pOpValue(nullptr), m_Index(DXBC::kWidth) {
  m_Components[0] = m_Components[1] = m_Components[2] = m_Components[3] =
      kBadComp;
}

OperandValueHelper::OperandValueHelper(OperandValue &OpValue, const CMask &Mask,
                                       const D3D10ShaderBinary::COperandBase &O)
    : m_pOpValue(&OpValue) {
  switch (O.m_ComponentSelection) {
  case D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE:
    Initialize(Mask, O.m_Swizzle);
    break;
  case D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE: {
    BYTE Swizzle[DXBC::kWidth] = {
        (BYTE)O.m_ComponentName, (BYTE)O.m_ComponentName,
        (BYTE)O.m_ComponentName, (BYTE)O.m_ComponentName};
    Initialize(Mask, Swizzle);
    break;
  }
  case D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE: {
    BYTE Swizzle[DXBC::kWidth] = {
        (BYTE)D3D10_SB_4_COMPONENT_X, (BYTE)D3D10_SB_4_COMPONENT_Y,
        (BYTE)D3D10_SB_4_COMPONENT_Z, (BYTE)D3D10_SB_4_COMPONENT_W};
    Initialize(Mask, Swizzle);
    break;
  }
  default:
    DXASSERT_DXBC(false);
  }
}

void OperandValueHelper::Initialize(const CMask &Mask,
                                    const BYTE CompSwizzle[DXBC::kWidth]) {
  DXASSERT(Mask.GetNumActiveComps() > 0,
           "otherwise the caller passed incorrect mask");
  for (BYTE c = 0; c < DXBC::kWidth; c++) {
    DXASSERT(m_pOpValue->m_pVal[c] == nullptr,
             "otherwise the caller passed a stale/corrupt OpValue");
    if (Mask.IsSet(c))
      m_Components[c] = CompSwizzle[c];
    else
      m_Components[c] = kBadComp;
  }
  for (m_Index = 0; m_Index < DXBC::kWidth; m_Index++)
    if (m_Components[m_Index] != kBadComp)
      break;
  DXASSERT_NOMSG(m_Index < DXBC::kWidth);
}

BYTE OperandValueHelper::GetComp() const {
  return (m_Index < DXBC::kWidth) ? m_Components[m_Index] : kBadComp;
}

bool OperandValueHelper::IsDone() const { return m_Index == DXBC::kWidth; }

void OperandValueHelper::Advance() {
  if (IsDone()) {
    DXASSERT(false, "otherwise Advance got called past the last active "
                    "component, which is not the intended use");
    return;
  }

  // 1. Look for the next component that needs a value.
  // 2. Disable m_Components[c] that are equal to Comp to iterate only through
  // unique components.
  BYTE Comp = m_Components[m_Index];
  DXASSERT_NOMSG(Comp < DXBC::kWidth);
  m_Components[m_Index] = kBadComp;
  BYTE StartComp = m_Index + 1;
  m_Index = DXBC::kWidth;
  for (BYTE c = StartComp; c < DXBC::kWidth; c++) {
    if (m_Components[c] == Comp) {
      m_Components[c] = kBadComp;
    } else if (m_Components[c] != kBadComp) {
      if (m_Index == DXBC::kWidth)
        m_Index = c;
    }
  }
}

void OperandValueHelper::SetValue(llvm::Value *pValue) {
  DXASSERT(m_Index < DXBC::kWidth, "otherwise the client uses the instance "
                                   "after all unique components have been set");
  DXASSERT(m_pOpValue->m_pVal[m_Index] == nullptr,
           "otherwise the client tried to redefine a value, which is not the "
           "intended use");
  BYTE Comp = m_Components[m_Index];
  DXASSERT_NOMSG(Comp < DXBC::kWidth);
  for (BYTE c = m_Index; c < DXBC::kWidth; c++) {
    if (m_Components[c] == Comp) {
      DXASSERT_NOMSG(m_pOpValue->m_pVal[c] == nullptr);
      m_pOpValue->m_pVal[c] = pValue;
    }
  }
}

//------------------------------------------------------------------------------
//
//  DXBC namespace functions.
//
namespace DXBC {

ShaderModel::Kind GetShaderModelKind(D3D10_SB_TOKENIZED_PROGRAM_TYPE Type) {
  switch (Type) {
  case D3D10_SB_PIXEL_SHADER:
    return ShaderModel::Kind::Pixel;
  case D3D10_SB_VERTEX_SHADER:
    return ShaderModel::Kind::Vertex;
  case D3D10_SB_GEOMETRY_SHADER:
    return ShaderModel::Kind::Geometry;
  case D3D11_SB_HULL_SHADER:
    return ShaderModel::Kind::Hull;
  case D3D11_SB_DOMAIN_SHADER:
    return ShaderModel::Kind::Domain;
  case D3D11_SB_COMPUTE_SHADER:
    return ShaderModel::Kind::Compute;
  default:
    return ShaderModel::Kind::Invalid;
  }
}

bool IsFlagDisableOptimizations(unsigned Flags) {
  return (Flags & D3D11_1_SB_GLOBAL_FLAG_SKIP_OPTIMIZATION) != 0;
}
bool IsFlagDisableMathRefactoring(unsigned Flags) {
  return (Flags & D3D10_SB_GLOBAL_FLAG_REFACTORING_ALLOWED) == 0;
}
bool IsFlagEnableDoublePrecision(unsigned Flags) {
  return (Flags & D3D11_SB_GLOBAL_FLAG_ENABLE_DOUBLE_PRECISION_FLOAT_OPS) != 0;
}
bool IsFlagForceEarlyDepthStencil(unsigned Flags) {
  return (Flags & D3D11_SB_GLOBAL_FLAG_FORCE_EARLY_DEPTH_STENCIL) != 0;
}
bool IsFlagEnableRawAndStructuredBuffers(unsigned Flags) {
  return (Flags & D3D11_SB_GLOBAL_FLAG_ENABLE_RAW_AND_STRUCTURED_BUFFERS) != 0;
}
bool IsFlagEnableMinPrecision(unsigned Flags) {
  return (Flags & D3D11_1_SB_GLOBAL_FLAG_ENABLE_MINIMUM_PRECISION) != 0;
}
bool IsFlagEnableDoubleExtensions(unsigned Flags) {
  return (Flags & D3D11_1_SB_GLOBAL_FLAG_ENABLE_DOUBLE_EXTENSIONS) != 0;
}
bool IsFlagEnableMSAD(unsigned Flags) {
  return (Flags & D3D11_1_SB_GLOBAL_FLAG_ENABLE_SHADER_EXTENSIONS) != 0;
}
bool IsFlagAllResourcesBound(unsigned Flags) {
  return (Flags & D3D12_SB_GLOBAL_FLAG_ALL_RESOURCES_BOUND) != 0;
}

InterpolationMode::Kind GetInterpolationModeKind(D3D_INTERPOLATION_MODE Mode) {
  switch (Mode) {
  case D3D_INTERPOLATION_UNDEFINED:
    return InterpolationMode::Kind::Undefined;
  case D3D_INTERPOLATION_CONSTANT:
    return InterpolationMode::Kind::Constant;
  case D3D_INTERPOLATION_LINEAR:
    return InterpolationMode::Kind::Linear;
  case D3D_INTERPOLATION_LINEAR_CENTROID:
    return InterpolationMode::Kind::LinearCentroid;
  case D3D_INTERPOLATION_LINEAR_NOPERSPECTIVE:
    return InterpolationMode::Kind::LinearNoperspective;
  case D3D_INTERPOLATION_LINEAR_NOPERSPECTIVE_CENTROID:
    return InterpolationMode::Kind::LinearNoperspectiveCentroid;
  case D3D_INTERPOLATION_LINEAR_SAMPLE:
    return InterpolationMode::Kind::LinearSample;
  case D3D_INTERPOLATION_LINEAR_NOPERSPECTIVE_SAMPLE:
    return InterpolationMode::Kind::LinearNoperspectiveSample;
  }
  DXASSERT(false, "otherwise the caller did not check the range");
  return InterpolationMode::Kind::Invalid;
}

D3D10_SB_OPERAND_TYPE GetOperandRegType(Semantic::Kind Kind, bool IsOutput) {
  switch (Kind) {
  case Semantic::Kind::Coverage:
    if (IsOutput)
      return D3D10_SB_OPERAND_TYPE_OUTPUT_COVERAGE_MASK;
    else
      return D3D11_SB_OPERAND_TYPE_INPUT_COVERAGE_MASK;
  case Semantic::Kind::InnerCoverage:
    return D3D11_SB_OPERAND_TYPE_INNER_COVERAGE;
  case Semantic::Kind::PrimitiveID:
    return D3D10_SB_OPERAND_TYPE_INPUT_PRIMITIVEID;
  case Semantic::Kind::Depth:
    return D3D10_SB_OPERAND_TYPE_OUTPUT_DEPTH;
  case Semantic::Kind::DepthLessEqual:
    return D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_LESS_EQUAL;
  case Semantic::Kind::DepthGreaterEqual:
    return D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_GREATER_EQUAL;
  case Semantic::Kind::StencilRef:
    return D3D11_SB_OPERAND_TYPE_OUTPUT_STENCIL_REF;
  }
  DXASSERT(false, "otherwise the caller passed wrong semantic type");
  return D3D10_SB_OPERAND_TYPE_TEMP;
}

DxilResource::Kind GetResourceKind(D3D10_SB_RESOURCE_DIMENSION ResType) {
  switch (ResType) {
  case D3D10_SB_RESOURCE_DIMENSION_UNKNOWN:
    return DxilResource::Kind::Invalid;
  case D3D10_SB_RESOURCE_DIMENSION_BUFFER:
    return DxilResource::Kind::TypedBuffer;
  case D3D10_SB_RESOURCE_DIMENSION_TEXTURE1D:
    return DxilResource::Kind::Texture1D;
  case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2D:
    return DxilResource::Kind::Texture2D;
  case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMS:
    return DxilResource::Kind::Texture2DMS;
  case D3D10_SB_RESOURCE_DIMENSION_TEXTURE3D:
    return DxilResource::Kind::Texture3D;
  case D3D10_SB_RESOURCE_DIMENSION_TEXTURECUBE:
    return DxilResource::Kind::TextureCube;
  case D3D10_SB_RESOURCE_DIMENSION_TEXTURE1DARRAY:
    return DxilResource::Kind::Texture1DArray;
  case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DARRAY:
    return DxilResource::Kind::Texture2DArray;
  case D3D10_SB_RESOURCE_DIMENSION_TEXTURE2DMSARRAY:
    return DxilResource::Kind::Texture2DMSArray;
  case D3D10_SB_RESOURCE_DIMENSION_TEXTURECUBEARRAY:
    return DxilResource::Kind::TextureCubeArray;
  case D3D11_SB_RESOURCE_DIMENSION_RAW_BUFFER:
    return DxilResource::Kind::RawBuffer;
  case D3D11_SB_RESOURCE_DIMENSION_STRUCTURED_BUFFER:
    return DxilResource::Kind::RawBuffer;
  }
  DXASSERT(false, "otherwise the caller did not check the range");
  return DxilResource::Kind::Invalid;
}

BYTE GetNumResCoords(DxilResource::Kind ResKind) {
  switch (ResKind) {
  case DxilResource::Kind::Texture1D:
    return 1;
  case DxilResource::Kind::Texture2D:
    return 2;
  case DxilResource::Kind::Texture2DMS:
    return 2;
  case DxilResource::Kind::Texture3D:
    return 3;
  case DxilResource::Kind::TextureCube:
    return 3;
  case DxilResource::Kind::Texture1DArray:
    return 2;
  case DxilResource::Kind::Texture2DArray:
    return 3;
  case DxilResource::Kind::Texture2DMSArray:
    return 3;
  case DxilResource::Kind::TextureCubeArray:
    return 4;
  case DxilResource::Kind::TypedBuffer:
    return 1;
  case DxilResource::Kind::RawBuffer:
    return 1;
  }
  DXASSERT(false, "otherwise the caller did not pass correct resource kind");
  return 0;
}

BYTE GetNumResOffsets(DxilResource::Kind ResKind) {
  switch (ResKind) {
  case DxilResource::Kind::Texture1D:
    return 1;
  case DxilResource::Kind::Texture2D:
    return 2;
  case DxilResource::Kind::Texture2DMS:
    return 2;
  case DxilResource::Kind::Texture3D:
    return 3;
  case DxilResource::Kind::TextureCube:
    return 3;
  case DxilResource::Kind::Texture1DArray:
    return 1;
  case DxilResource::Kind::Texture2DArray:
    return 2;
  case DxilResource::Kind::Texture2DMSArray:
    return 2;
  case DxilResource::Kind::TextureCubeArray:
    return 3;
  case DxilResource::Kind::TypedBuffer:
    return 0;
  case DxilResource::Kind::RawBuffer:
    return 0;
  }
  DXASSERT(false, "otherwise the caller did not pass correct resource kind");
  return 0;
}

CompType GetCompType(D3D_REGISTER_COMPONENT_TYPE CompTy) {
  switch (CompTy) {
  case D3D_REGISTER_COMPONENT_FLOAT32:
    return CompType::getF32();
  case D3D_REGISTER_COMPONENT_SINT32:
    return CompType::getI32();
  case D3D_REGISTER_COMPONENT_UINT32:
    return CompType::getU32();
  }
  DXASSERT(false, "incorrect component type value");
  return CompType();
}

CompType GetCompTypeWithMinPrec(D3D_REGISTER_COMPONENT_TYPE BaseCompTy,
                                D3D11_SB_OPERAND_MIN_PRECISION MinPrec) {
  switch (BaseCompTy) {
  case D3D_REGISTER_COMPONENT_FLOAT32:
    switch (MinPrec) {
    case D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT:
      return CompType::getF32();
    case D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_16:
      LLVM_FALLTHROUGH;
    case D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_2_8:
      return CompType::getF16();
    }
    break;
  case D3D_REGISTER_COMPONENT_SINT32:
    switch (MinPrec) {
    case D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT:
      return CompType::getI32();
    case D3D11_SB_OPERAND_MIN_PRECISION_SINT_16:
      return CompType::getI16();
    case D3D11_SB_OPERAND_MIN_PRECISION_UINT_16:
      return CompType::getU16();
    }
    break;
  case D3D_REGISTER_COMPONENT_UINT32:
    switch (MinPrec) {
    case D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT:
      return CompType::getU32();
    case D3D11_SB_OPERAND_MIN_PRECISION_SINT_16:
      return CompType::getI16();
    case D3D11_SB_OPERAND_MIN_PRECISION_UINT_16:
      return CompType::getU16();
    }
    break;
  }
  DXASSERT(false, "otherwise incorrect combination of type and min-precision");
  return CompType();
}

CompType GetCompTypeWithMinPrec(CompType BaseCompTy,
                                D3D11_SB_OPERAND_MIN_PRECISION MinPrec) {
  switch (BaseCompTy.GetKind()) {
  case CompType::Kind::F32:
    switch (MinPrec) {
    case D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT:
      return CompType::getF32();
    case D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_16:
      LLVM_FALLTHROUGH;
    case D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_2_8:
      return CompType::getF16();
    }
    break;
  case CompType::Kind::I32:
    switch (MinPrec) {
    case D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT:
      return CompType::getI32();
    case D3D11_SB_OPERAND_MIN_PRECISION_SINT_16:
      return CompType::getI16();
    case D3D11_SB_OPERAND_MIN_PRECISION_UINT_16:
      return CompType::getU16();
    }
    break;
  case CompType::Kind::U32:
    switch (MinPrec) {
    case D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT:
      return CompType::getU32();
    case D3D11_SB_OPERAND_MIN_PRECISION_SINT_16:
      return CompType::getI16();
    case D3D11_SB_OPERAND_MIN_PRECISION_UINT_16:
      return CompType::getU16();
    }
    break;
  case CompType::Kind::F64:
    return CompType::getF64();
  }
  DXASSERT(false, "otherwise incorrect combination of type and min-precision");
  return CompType();
}

static CompType GetFullPrecCompType(CompType CompTy) {
  switch (CompTy.GetKind()) {
  case CompType::Kind::F16:
    return CompType::getF32();
  case CompType::Kind::I16:
    return CompType::getI32();
  case CompType::Kind::U16:
    return CompType::getU32();
  default:
    return CompTy;
  }
}

CompType GetCompTypeFromMinPrec(D3D11_SB_OPERAND_MIN_PRECISION MinPrec,
                                CompType DefaultPrecCompType) {
  switch (MinPrec) {
  case D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT:
    return GetFullPrecCompType(DefaultPrecCompType);
  case D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_16:
    LLVM_FALLTHROUGH;
  case D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_2_8:
    return CompType::getF16();
  case D3D11_SB_OPERAND_MIN_PRECISION_SINT_16:
    return CompType::getI16();
  case D3D11_SB_OPERAND_MIN_PRECISION_UINT_16:
    return CompType::getU16();
  default:
    DXASSERT_DXBC(false);
    return GetFullPrecCompType(DefaultPrecCompType);
  }
}

CompType GetResCompType(D3D10_SB_RESOURCE_RETURN_TYPE CompTy) {
  switch (CompTy) {
  case D3D10_SB_RETURN_TYPE_UNORM:
    return CompType::getF32();
  case D3D10_SB_RETURN_TYPE_SNORM:
    return CompType::getF32();
  case D3D10_SB_RETURN_TYPE_SINT:
    return CompType::getI32();
  case D3D10_SB_RETURN_TYPE_UINT:
    return CompType::getU32();
  case D3D10_SB_RETURN_TYPE_FLOAT:
    return CompType::getF32();
  case D3D10_SB_RETURN_TYPE_MIXED:
    return CompType::getInvalid();
  case D3D11_SB_RETURN_TYPE_DOUBLE:
    return CompType::getF64();
  case D3D11_SB_RETURN_TYPE_CONTINUED:
    return CompType::getInvalid();
  case D3D11_SB_RETURN_TYPE_UNUSED:
    return CompType::getInvalid();
  default:
    DXASSERT(false, "invalid comp type");
    return CompType::getInvalid();
  }
}

CompType GetDeclResCompType(D3D10_SB_RESOURCE_RETURN_TYPE CompTy) {
  switch (CompTy) {
  case D3D10_SB_RETURN_TYPE_UNORM:
    return CompType::getUNormF32();
  case D3D10_SB_RETURN_TYPE_SNORM:
    return CompType::getSNormF32();
  case D3D10_SB_RETURN_TYPE_SINT:
    return CompType::getI32();
  case D3D10_SB_RETURN_TYPE_UINT:
    return CompType::getU32();
  case D3D10_SB_RETURN_TYPE_FLOAT:
    return CompType::getF32();
  case D3D10_SB_RETURN_TYPE_MIXED:
    return CompType::getInvalid();
  case D3D11_SB_RETURN_TYPE_DOUBLE:
    return CompType::getF64();
  case D3D11_SB_RETURN_TYPE_CONTINUED:
    return CompType::getInvalid();
  case D3D11_SB_RETURN_TYPE_UNUSED:
    return CompType::getInvalid();
  default:
    DXASSERT(false, "invalid comp type");
    return CompType::getInvalid();
  }
}

static const char s_ComponentName[kWidth] = {'x', 'y', 'z', 'w'};
char GetCompName(BYTE c) {
  DXASSERT(c < kWidth,
           "otherwise the caller did not pass the right component value");
  return s_ComponentName[c];
}

DxilSampler::SamplerKind GetSamplerKind(D3D10_SB_SAMPLER_MODE Mode) {
  switch (Mode) {
  case D3D10_SB_SAMPLER_MODE_DEFAULT:
    return DxilSampler::SamplerKind::Default;
  case D3D10_SB_SAMPLER_MODE_COMPARISON:
    return DxilSampler::SamplerKind::Comparison;
  case D3D10_SB_SAMPLER_MODE_MONO:
    return DxilSampler::SamplerKind::Mono;
  }
  DXASSERT(false, "otherwise the caller did not pass the right Mode");
  return DxilSampler::SamplerKind::Invalid;
}

unsigned GetRegIndex(unsigned Reg, unsigned Comp) { return Reg * 4 + Comp; }

DXIL::AtomicBinOpCode GetAtomicBinOp(D3D10_SB_OPCODE_TYPE DxbcOpCode) {
  switch (DxbcOpCode) {
  case D3D11_SB_OPCODE_ATOMIC_IADD:
    return DXIL::AtomicBinOpCode::Add;
  case D3D11_SB_OPCODE_ATOMIC_AND:
    return DXIL::AtomicBinOpCode::And;
  case D3D11_SB_OPCODE_ATOMIC_OR:
    return DXIL::AtomicBinOpCode::Or;
  case D3D11_SB_OPCODE_ATOMIC_XOR:
    return DXIL::AtomicBinOpCode::Xor;
  case D3D11_SB_OPCODE_ATOMIC_IMAX:
    return DXIL::AtomicBinOpCode::IMax;
  case D3D11_SB_OPCODE_ATOMIC_IMIN:
    return DXIL::AtomicBinOpCode::IMin;
  case D3D11_SB_OPCODE_ATOMIC_UMAX:
    return DXIL::AtomicBinOpCode::UMax;
  case D3D11_SB_OPCODE_ATOMIC_UMIN:
    return DXIL::AtomicBinOpCode::UMin;
  case D3D11_SB_OPCODE_IMM_ATOMIC_EXCH:
    return DXIL::AtomicBinOpCode::Exchange;
  case D3D11_SB_OPCODE_IMM_ATOMIC_IADD:
    return DXIL::AtomicBinOpCode::Add;
  case D3D11_SB_OPCODE_IMM_ATOMIC_AND:
    return DXIL::AtomicBinOpCode::And;
  case D3D11_SB_OPCODE_IMM_ATOMIC_OR:
    return DXIL::AtomicBinOpCode::Or;
  case D3D11_SB_OPCODE_IMM_ATOMIC_XOR:
    return DXIL::AtomicBinOpCode::Xor;
  case D3D11_SB_OPCODE_IMM_ATOMIC_IMAX:
    return DXIL::AtomicBinOpCode::IMax;
  case D3D11_SB_OPCODE_IMM_ATOMIC_IMIN:
    return DXIL::AtomicBinOpCode::IMin;
  case D3D11_SB_OPCODE_IMM_ATOMIC_UMAX:
    return DXIL::AtomicBinOpCode::UMax;
  case D3D11_SB_OPCODE_IMM_ATOMIC_UMIN:
    return DXIL::AtomicBinOpCode::UMin;

  case D3D11_SB_OPCODE_ATOMIC_CMP_STORE:
  case D3D11_SB_OPCODE_IMM_ATOMIC_CMP_EXCH:
  default:
    DXASSERT(false, "otherwise the caller did not pass the right OpCode");
  }

  return DXIL::AtomicBinOpCode::Invalid;
}

llvm::AtomicRMWInst::BinOp GetLlvmAtomicBinOp(D3D10_SB_OPCODE_TYPE DxbcOpCode) {
  switch (DxbcOpCode) {
  case D3D11_SB_OPCODE_ATOMIC_IADD:
    return llvm::AtomicRMWInst::Add;
  case D3D11_SB_OPCODE_ATOMIC_AND:
    return llvm::AtomicRMWInst::And;
  case D3D11_SB_OPCODE_ATOMIC_OR:
    return llvm::AtomicRMWInst::Or;
  case D3D11_SB_OPCODE_ATOMIC_XOR:
    return llvm::AtomicRMWInst::Xor;
  case D3D11_SB_OPCODE_ATOMIC_IMAX:
    return llvm::AtomicRMWInst::Max;
  case D3D11_SB_OPCODE_ATOMIC_IMIN:
    return llvm::AtomicRMWInst::Min;
  case D3D11_SB_OPCODE_ATOMIC_UMAX:
    return llvm::AtomicRMWInst::UMax;
  case D3D11_SB_OPCODE_ATOMIC_UMIN:
    return llvm::AtomicRMWInst::UMin;
  case D3D11_SB_OPCODE_IMM_ATOMIC_EXCH:
    return llvm::AtomicRMWInst::Xchg;
  case D3D11_SB_OPCODE_IMM_ATOMIC_IADD:
    return llvm::AtomicRMWInst::Add;
  case D3D11_SB_OPCODE_IMM_ATOMIC_AND:
    return llvm::AtomicRMWInst::And;
  case D3D11_SB_OPCODE_IMM_ATOMIC_OR:
    return llvm::AtomicRMWInst::Or;
  case D3D11_SB_OPCODE_IMM_ATOMIC_XOR:
    return llvm::AtomicRMWInst::Xor;
  case D3D11_SB_OPCODE_IMM_ATOMIC_IMAX:
    return llvm::AtomicRMWInst::Max;
  case D3D11_SB_OPCODE_IMM_ATOMIC_IMIN:
    return llvm::AtomicRMWInst::Min;
  case D3D11_SB_OPCODE_IMM_ATOMIC_UMAX:
    return llvm::AtomicRMWInst::UMax;
  case D3D11_SB_OPCODE_IMM_ATOMIC_UMIN:
    return llvm::AtomicRMWInst::UMin;

  case D3D11_SB_OPCODE_ATOMIC_CMP_STORE:
  case D3D11_SB_OPCODE_IMM_ATOMIC_CMP_EXCH:
  default:
    DXASSERT(false, "otherwise the caller did not pass the right OpCode");
  }

  return llvm::AtomicRMWInst::BAD_BINOP;
}

bool AtomicBinOpHasReturn(D3D10_SB_OPCODE_TYPE DxbcOpCode) {
  switch (DxbcOpCode) {
  case D3D11_SB_OPCODE_ATOMIC_AND:
  case D3D11_SB_OPCODE_ATOMIC_OR:
  case D3D11_SB_OPCODE_ATOMIC_XOR:
  case D3D11_SB_OPCODE_ATOMIC_IADD:
  case D3D11_SB_OPCODE_ATOMIC_IMAX:
  case D3D11_SB_OPCODE_ATOMIC_IMIN:
  case D3D11_SB_OPCODE_ATOMIC_UMAX:
  case D3D11_SB_OPCODE_ATOMIC_UMIN:
  case D3D11_SB_OPCODE_ATOMIC_CMP_STORE:
    return false;

  case D3D11_SB_OPCODE_IMM_ATOMIC_IADD:
  case D3D11_SB_OPCODE_IMM_ATOMIC_AND:
  case D3D11_SB_OPCODE_IMM_ATOMIC_OR:
  case D3D11_SB_OPCODE_IMM_ATOMIC_XOR:
  case D3D11_SB_OPCODE_IMM_ATOMIC_EXCH:
  case D3D11_SB_OPCODE_IMM_ATOMIC_IMAX:
  case D3D11_SB_OPCODE_IMM_ATOMIC_IMIN:
  case D3D11_SB_OPCODE_IMM_ATOMIC_UMAX:
  case D3D11_SB_OPCODE_IMM_ATOMIC_UMIN:
  case D3D11_SB_OPCODE_IMM_ATOMIC_CMP_EXCH:
    return true;
  }

  DXASSERT(false, "otherwise the caller did not pass the right OpCode");
  return false;
}

bool IsCompareExchAtomicBinOp(D3D10_SB_OPCODE_TYPE DxbcOpCode) {
  switch (DxbcOpCode) {
  case D3D11_SB_OPCODE_ATOMIC_CMP_STORE:
  case D3D11_SB_OPCODE_IMM_ATOMIC_CMP_EXCH:
    return true;

  case D3D11_SB_OPCODE_ATOMIC_AND:
  case D3D11_SB_OPCODE_ATOMIC_OR:
  case D3D11_SB_OPCODE_ATOMIC_XOR:
  case D3D11_SB_OPCODE_ATOMIC_IADD:
  case D3D11_SB_OPCODE_ATOMIC_IMAX:
  case D3D11_SB_OPCODE_ATOMIC_IMIN:
  case D3D11_SB_OPCODE_ATOMIC_UMAX:
  case D3D11_SB_OPCODE_ATOMIC_UMIN:
  case D3D11_SB_OPCODE_IMM_ATOMIC_IADD:
  case D3D11_SB_OPCODE_IMM_ATOMIC_AND:
  case D3D11_SB_OPCODE_IMM_ATOMIC_OR:
  case D3D11_SB_OPCODE_IMM_ATOMIC_XOR:
  case D3D11_SB_OPCODE_IMM_ATOMIC_EXCH:
  case D3D11_SB_OPCODE_IMM_ATOMIC_IMAX:
  case D3D11_SB_OPCODE_IMM_ATOMIC_IMIN:
  case D3D11_SB_OPCODE_IMM_ATOMIC_UMAX:
  case D3D11_SB_OPCODE_IMM_ATOMIC_UMIN:
    return false;
  }

  DXASSERT(false, "otherwise the caller did not pass the right OpCode");
  return false;
}

bool HasFeedback(D3D10_SB_OPCODE_TYPE OpCode) {
  switch (OpCode) {
  case D3DWDDM1_3_SB_OPCODE_GATHER4_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_GATHER4_C_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_GATHER4_PO_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_GATHER4_PO_C_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_LD_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_LD_MS_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_LD_UAV_TYPED_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_LD_RAW_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_LD_STRUCTURED_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_SAMPLE_L_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_SAMPLE_C_LZ_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_SAMPLE_CLAMP_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_SAMPLE_B_CLAMP_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_SAMPLE_D_CLAMP_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_SAMPLE_C_CLAMP_FEEDBACK:
    return true;
  }
  return false;
}

unsigned GetResourceSlot(D3D10_SB_OPCODE_TYPE OpCode) {
  switch (OpCode) {
  case D3D11_SB_OPCODE_STORE_UAV_TYPED:
  case D3D11_SB_OPCODE_STORE_RAW:
  case D3D11_SB_OPCODE_STORE_STRUCTURED:
  case D3D11_SB_OPCODE_ATOMIC_AND:
  case D3D11_SB_OPCODE_ATOMIC_OR:
  case D3D11_SB_OPCODE_ATOMIC_XOR:
  case D3D11_SB_OPCODE_ATOMIC_IADD:
  case D3D11_SB_OPCODE_ATOMIC_IMAX:
  case D3D11_SB_OPCODE_ATOMIC_IMIN:
  case D3D11_SB_OPCODE_ATOMIC_UMAX:
  case D3D11_SB_OPCODE_ATOMIC_UMIN:
  case D3D11_SB_OPCODE_ATOMIC_CMP_STORE:
    return 0;

  case D3D11_SB_OPCODE_IMM_ATOMIC_IADD:
  case D3D11_SB_OPCODE_IMM_ATOMIC_AND:
  case D3D11_SB_OPCODE_IMM_ATOMIC_OR:
  case D3D11_SB_OPCODE_IMM_ATOMIC_XOR:
  case D3D11_SB_OPCODE_IMM_ATOMIC_EXCH:
  case D3D11_SB_OPCODE_IMM_ATOMIC_IMAX:
  case D3D11_SB_OPCODE_IMM_ATOMIC_IMIN:
  case D3D11_SB_OPCODE_IMM_ATOMIC_UMAX:
  case D3D11_SB_OPCODE_IMM_ATOMIC_UMIN:
  case D3D11_SB_OPCODE_IMM_ATOMIC_CMP_EXCH:
  case D3D10_1_SB_OPCODE_SAMPLE_INFO:
  case D3D10_1_SB_OPCODE_SAMPLE_POS:
    return 1;

  case D3D10_SB_OPCODE_SAMPLE:
  case D3D10_SB_OPCODE_SAMPLE_B:
  case D3D10_SB_OPCODE_SAMPLE_L:
  case D3D10_SB_OPCODE_SAMPLE_D:
  case D3D10_SB_OPCODE_SAMPLE_C:
  case D3D10_SB_OPCODE_SAMPLE_C_LZ:
  case D3D10_SB_OPCODE_LD:
  case D3D10_SB_OPCODE_LD_MS:
  case D3D11_SB_OPCODE_LD_UAV_TYPED:
  case D3D11_SB_OPCODE_LD_RAW:
  case D3D10_SB_OPCODE_RESINFO:
  case D3D10_1_SB_OPCODE_LOD:
  case D3D10_1_SB_OPCODE_GATHER4:
  case D3D11_SB_OPCODE_GATHER4_C:
    return 2;

  case D3DWDDM1_3_SB_OPCODE_SAMPLE_CLAMP_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_SAMPLE_B_CLAMP_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_SAMPLE_L_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_SAMPLE_D_CLAMP_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_SAMPLE_C_CLAMP_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_SAMPLE_C_LZ_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_LD_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_LD_MS_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_LD_UAV_TYPED_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_LD_RAW_FEEDBACK:
  case D3D11_SB_OPCODE_LD_STRUCTURED:
  case D3DWDDM1_3_SB_OPCODE_GATHER4_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_GATHER4_C_FEEDBACK:
  case D3D11_SB_OPCODE_GATHER4_PO:
  case D3D11_SB_OPCODE_GATHER4_PO_C:
    return 3;

  case D3DWDDM1_3_SB_OPCODE_LD_STRUCTURED_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_GATHER4_PO_FEEDBACK:
  case D3DWDDM1_3_SB_OPCODE_GATHER4_PO_C_FEEDBACK:
    return 4;
  }

  DXASSERT_NOMSG(false);
  return 0;
}

DXIL::BarrierMode GetBarrierMode(bool bSyncThreadGroup, bool bUAVFenceGlobal,
                                 bool bUAVFenceThreadGroup, bool bTGSMFence) {
  unsigned M = 0;
  if (bSyncThreadGroup)
    M |= (unsigned)DXIL::BarrierMode::SyncThreadGroup;

  if (bUAVFenceGlobal)
    M |= (unsigned)DXIL::BarrierMode::UAVFenceGlobal;

  if (bUAVFenceThreadGroup)
    M |= (unsigned)DXIL::BarrierMode::UAVFenceThreadGroup;

  if (bTGSMFence)
    M |= (unsigned)DXIL::BarrierMode::TGSMFence;

  return (DXIL::BarrierMode)M;
}

DXIL::InputPrimitive GetInputPrimitive(D3D10_SB_PRIMITIVE Primitive) {
  switch (Primitive) {
  case D3D10_SB_PRIMITIVE_UNDEFINED:
    return DXIL::InputPrimitive::Undefined;
  case D3D10_SB_PRIMITIVE_POINT:
    return DXIL::InputPrimitive::Point;
  case D3D10_SB_PRIMITIVE_LINE:
    return DXIL::InputPrimitive::Line;
  case D3D10_SB_PRIMITIVE_TRIANGLE:
    return DXIL::InputPrimitive::Triangle;
  case D3D10_SB_PRIMITIVE_LINE_ADJ:
    return DXIL::InputPrimitive::LineWithAdjacency;
  case D3D10_SB_PRIMITIVE_TRIANGLE_ADJ:
    return DXIL::InputPrimitive::TriangleWithAdjacency;
  case D3D11_SB_PRIMITIVE_1_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch1;
  case D3D11_SB_PRIMITIVE_2_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch2;
  case D3D11_SB_PRIMITIVE_3_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch3;
  case D3D11_SB_PRIMITIVE_4_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch4;
  case D3D11_SB_PRIMITIVE_5_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch5;
  case D3D11_SB_PRIMITIVE_6_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch6;
  case D3D11_SB_PRIMITIVE_7_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch7;
  case D3D11_SB_PRIMITIVE_8_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch8;
  case D3D11_SB_PRIMITIVE_9_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch9;
  case D3D11_SB_PRIMITIVE_10_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch10;
  case D3D11_SB_PRIMITIVE_11_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch11;
  case D3D11_SB_PRIMITIVE_12_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch12;
  case D3D11_SB_PRIMITIVE_13_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch13;
  case D3D11_SB_PRIMITIVE_14_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch14;
  case D3D11_SB_PRIMITIVE_15_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch15;
  case D3D11_SB_PRIMITIVE_16_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch16;
  case D3D11_SB_PRIMITIVE_17_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch17;
  case D3D11_SB_PRIMITIVE_18_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch18;
  case D3D11_SB_PRIMITIVE_19_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch19;
  case D3D11_SB_PRIMITIVE_20_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch20;
  case D3D11_SB_PRIMITIVE_21_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch21;
  case D3D11_SB_PRIMITIVE_22_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch22;
  case D3D11_SB_PRIMITIVE_23_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch23;
  case D3D11_SB_PRIMITIVE_24_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch24;
  case D3D11_SB_PRIMITIVE_25_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch25;
  case D3D11_SB_PRIMITIVE_26_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch26;
  case D3D11_SB_PRIMITIVE_27_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch27;
  case D3D11_SB_PRIMITIVE_28_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch28;
  case D3D11_SB_PRIMITIVE_29_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch29;
  case D3D11_SB_PRIMITIVE_30_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch30;
  case D3D11_SB_PRIMITIVE_31_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch31;
  case D3D11_SB_PRIMITIVE_32_CONTROL_POINT_PATCH:
    return DXIL::InputPrimitive::ControlPointPatch32;
  }

  DXASSERT_NOMSG(false);
  return DXIL::InputPrimitive::Undefined;
}

DXIL::PrimitiveTopology
GetPrimitiveTopology(D3D10_SB_PRIMITIVE_TOPOLOGY Topology) {
  switch (Topology) {
  case D3D10_SB_PRIMITIVE_TOPOLOGY_UNDEFINED:
    return DXIL::PrimitiveTopology::Undefined;
  case D3D10_SB_PRIMITIVE_TOPOLOGY_POINTLIST:
    return DXIL::PrimitiveTopology::PointList;
  case D3D10_SB_PRIMITIVE_TOPOLOGY_LINELIST:
    return DXIL::PrimitiveTopology::LineList;
  case D3D10_SB_PRIMITIVE_TOPOLOGY_LINESTRIP:
    return DXIL::PrimitiveTopology::LineStrip;
  case D3D10_SB_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
    return DXIL::PrimitiveTopology::TriangleList;
  case D3D10_SB_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
    return DXIL::PrimitiveTopology::TriangleStrip;
  case D3D10_SB_PRIMITIVE_TOPOLOGY_LINELIST_ADJ:
    LLVM_FALLTHROUGH; // The ADJ versions are redundant in DXBC and are ot used,
                      // probably put there by mistake.
  case D3D10_SB_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
    LLVM_FALLTHROUGH;
  case D3D10_SB_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ:
    LLVM_FALLTHROUGH;
  case D3D10_SB_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
    LLVM_FALLTHROUGH;
  default:
    break;
  }

  IFTBOOL(false, DXC_E_INCORRECT_DXBC);
  return DXIL::PrimitiveTopology::Undefined;
}

const char *GetD3D10SBName(D3D10_SB_NAME D3DName) {
  switch (D3DName) {
  case D3D10_SB_NAME_UNDEFINED:
    return "undefined";
  case D3D10_SB_NAME_POSITION:
    return "SV_Position";
  case D3D10_SB_NAME_CLIP_DISTANCE:
    return "SV_ClipDistance";
  case D3D10_SB_NAME_CULL_DISTANCE:
    return "SV_CullDistance";
  case D3D10_SB_NAME_RENDER_TARGET_ARRAY_INDEX:
    return "SV_RenderTargetArrayIndex";
  case D3D10_SB_NAME_VIEWPORT_ARRAY_INDEX:
    return "SV_ViewportArrayIndex";
  case D3D10_SB_NAME_VERTEX_ID:
    return "SV_VertexID";
  case D3D10_SB_NAME_PRIMITIVE_ID:
    return "SV_PrimitiveID";
  case D3D10_SB_NAME_INSTANCE_ID:
    return "SV_InstanceID";
  case D3D10_SB_NAME_IS_FRONT_FACE:
    return "SV_IsFrontFace";
  case D3D10_SB_NAME_SAMPLE_INDEX:
    return "SV_SampleIndex";
  case D3D11_SB_NAME_FINAL_QUAD_U_EQ_0_EDGE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_QUAD_V_EQ_0_EDGE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_QUAD_U_EQ_1_EDGE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_QUAD_V_EQ_1_EDGE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_TRI_U_EQ_0_EDGE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_TRI_V_EQ_0_EDGE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_TRI_W_EQ_0_EDGE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_LINE_DETAIL_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_LINE_DENSITY_TESSFACTOR:
    return "SV_TessFactor";
  case D3D11_SB_NAME_FINAL_QUAD_U_INSIDE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_QUAD_V_INSIDE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_TRI_INSIDE_TESSFACTOR:
    return "SV_InsideTessFactor";
  default:
    IFT(DXC_E_INCORRECT_DXBC);
    return "unknown";
  }
}

unsigned GetD3D10SBSemanticIndex(D3D10_SB_NAME D3DName) {
  switch (D3DName) {
  case D3D10_SB_NAME_UNDEFINED:
  case D3D10_SB_NAME_POSITION:
  case D3D10_SB_NAME_CLIP_DISTANCE:
  case D3D10_SB_NAME_CULL_DISTANCE:
  case D3D10_SB_NAME_RENDER_TARGET_ARRAY_INDEX:
  case D3D10_SB_NAME_VIEWPORT_ARRAY_INDEX:
  case D3D10_SB_NAME_VERTEX_ID:
  case D3D10_SB_NAME_PRIMITIVE_ID:
  case D3D10_SB_NAME_INSTANCE_ID:
  case D3D10_SB_NAME_IS_FRONT_FACE:
  case D3D10_SB_NAME_SAMPLE_INDEX:
    return 0;
  case D3D11_SB_NAME_FINAL_QUAD_U_EQ_0_EDGE_TESSFACTOR:
    return 0;
  case D3D11_SB_NAME_FINAL_QUAD_V_EQ_0_EDGE_TESSFACTOR:
    return 1;
  case D3D11_SB_NAME_FINAL_QUAD_U_EQ_1_EDGE_TESSFACTOR:
    return 2;
  case D3D11_SB_NAME_FINAL_QUAD_V_EQ_1_EDGE_TESSFACTOR:
    return 3;
  case D3D11_SB_NAME_FINAL_QUAD_U_INSIDE_TESSFACTOR:
    return 0;
  case D3D11_SB_NAME_FINAL_QUAD_V_INSIDE_TESSFACTOR:
    return 1;
  case D3D11_SB_NAME_FINAL_TRI_U_EQ_0_EDGE_TESSFACTOR:
    return 0;
  case D3D11_SB_NAME_FINAL_TRI_V_EQ_0_EDGE_TESSFACTOR:
    return 1;
  case D3D11_SB_NAME_FINAL_TRI_W_EQ_0_EDGE_TESSFACTOR:
    return 2;
  case D3D11_SB_NAME_FINAL_TRI_INSIDE_TESSFACTOR:
    return 0;
  case D3D11_SB_NAME_FINAL_LINE_DETAIL_TESSFACTOR:
    return 0;
  case D3D11_SB_NAME_FINAL_LINE_DENSITY_TESSFACTOR:
    return 1;
  default:
    return 0;
  }
}

D3D_REGISTER_COMPONENT_TYPE GetD3DRegCompType(D3D10_SB_NAME D3DName) {
  switch (D3DName) {
  case D3D10_SB_NAME_POSITION:
  case D3D10_SB_NAME_CLIP_DISTANCE:
  case D3D10_SB_NAME_CULL_DISTANCE:
    return D3D_REGISTER_COMPONENT_FLOAT32;
  case D3D10_SB_NAME_RENDER_TARGET_ARRAY_INDEX:
  case D3D10_SB_NAME_VIEWPORT_ARRAY_INDEX:
  case D3D10_SB_NAME_VERTEX_ID:
  case D3D10_SB_NAME_PRIMITIVE_ID:
  case D3D10_SB_NAME_INSTANCE_ID:
  case D3D10_SB_NAME_IS_FRONT_FACE:
  case D3D10_SB_NAME_SAMPLE_INDEX:
    return D3D_REGISTER_COMPONENT_UINT32;
  case D3D10_SB_NAME_UNDEFINED: // this shpild not be called for an undefined
                                // name.
  case D3D11_SB_NAME_FINAL_QUAD_U_EQ_0_EDGE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_QUAD_V_EQ_0_EDGE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_QUAD_U_EQ_1_EDGE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_QUAD_V_EQ_1_EDGE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_QUAD_U_INSIDE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_QUAD_V_INSIDE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_TRI_U_EQ_0_EDGE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_TRI_V_EQ_0_EDGE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_TRI_W_EQ_0_EDGE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_TRI_INSIDE_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_LINE_DETAIL_TESSFACTOR:
  case D3D11_SB_NAME_FINAL_LINE_DENSITY_TESSFACTOR:
  default:
    IFT(DXC_E_INCORRECT_DXBC);
    return D3D_REGISTER_COMPONENT_UNKNOWN;
  }
}

const char *GetSemanticNameFromD3DName(D3D_NAME D3DName) {
  switch (D3DName) {
  case D3D_NAME_UNDEFINED:
    return "undefined";
  case D3D_NAME_POSITION:
    return "SV_Position";
  case D3D_NAME_CLIP_DISTANCE:
    return "SV_ClipDistance";
  case D3D_NAME_CULL_DISTANCE:
    return "SV_CullDistance";
  case D3D_NAME_RENDER_TARGET_ARRAY_INDEX:
    return "SV_RenderTargetArrayIndex";
  case D3D_NAME_VIEWPORT_ARRAY_INDEX:
    return "SV_ViewportArrayIndex";
  case D3D_NAME_VERTEX_ID:
    return "SV_VertexID";
  case D3D_NAME_PRIMITIVE_ID:
    return "SV_PrimitiveID";
  case D3D_NAME_INSTANCE_ID:
    return "SV_InstanceID";
  case D3D_NAME_IS_FRONT_FACE:
    return "SV_IsFrontFace";
  case D3D_NAME_SAMPLE_INDEX:
    return "SV_SampleIndex";
  case D3D_NAME_FINAL_QUAD_EDGE_TESSFACTOR:
    return "SV_TessFactor";
  case D3D_NAME_FINAL_QUAD_INSIDE_TESSFACTOR:
    return "SV_InsideTessFactor";
  case D3D_NAME_FINAL_TRI_EDGE_TESSFACTOR:
    return "SV_TessFactor";
  case D3D_NAME_FINAL_TRI_INSIDE_TESSFACTOR:
    return "SV_InsideTessFactor";
  case D3D_NAME_FINAL_LINE_DETAIL_TESSFACTOR:
    return "SV_TessFactor";
  case D3D_NAME_FINAL_LINE_DENSITY_TESSFACTOR:
    return "SV_TessFactor";
  case D3D_NAME_TARGET:
    return "SV_Target";
  case D3D_NAME_DEPTH:
    return "SV_Depth";
  case D3D_NAME_COVERAGE:
    return "SV_Coverage";
  case D3D_NAME_DEPTH_GREATER_EQUAL:
    return "SV_DepthGreaterEqual";
  case D3D_NAME_DEPTH_LESS_EQUAL:
    return "SV_DepthLessEqual";
  case D3D_NAME_STENCIL_REF:
    return "SV_StencilRef";
  case D3D_NAME_INNER_COVERAGE:
    return "SV_InnerCoverage";
  default:
    DXASSERT_NOMSG(false);
    return "undefined";
  }
}

unsigned GetSemanticIndexFromD3DName(D3D_NAME D3DName) {
  switch (D3DName) {
  case D3D_NAME_UNDEFINED:
  case D3D_NAME_POSITION:
  case D3D_NAME_CLIP_DISTANCE:
  case D3D_NAME_CULL_DISTANCE:
  case D3D_NAME_RENDER_TARGET_ARRAY_INDEX:
  case D3D_NAME_VIEWPORT_ARRAY_INDEX:
  case D3D_NAME_VERTEX_ID:
  case D3D_NAME_PRIMITIVE_ID:
  case D3D_NAME_INSTANCE_ID:
  case D3D_NAME_IS_FRONT_FACE:
  case D3D_NAME_SAMPLE_INDEX:
  case D3D_NAME_FINAL_QUAD_EDGE_TESSFACTOR:
  case D3D_NAME_FINAL_QUAD_INSIDE_TESSFACTOR:
  case D3D_NAME_FINAL_TRI_EDGE_TESSFACTOR:
  case D3D_NAME_FINAL_TRI_INSIDE_TESSFACTOR:
  case D3D_NAME_TARGET:
  case D3D_NAME_DEPTH:
  case D3D_NAME_COVERAGE:
  case D3D_NAME_DEPTH_GREATER_EQUAL:
  case D3D_NAME_DEPTH_LESS_EQUAL:
  case D3D_NAME_STENCIL_REF:
  case D3D_NAME_INNER_COVERAGE:
    return UINT_MAX;
  case D3D_NAME_FINAL_LINE_DETAIL_TESSFACTOR:
    return 0;
  case D3D_NAME_FINAL_LINE_DENSITY_TESSFACTOR:
    return 1;
  default:
    DXASSERT_NOMSG(false);
    return UINT_MAX;
  }
}

DXIL::TessellatorDomain
GetTessellatorDomain(D3D11_SB_TESSELLATOR_DOMAIN TessDomain) {
  switch (TessDomain) {
  case D3D11_SB_TESSELLATOR_DOMAIN_UNDEFINED:
    return DXIL::TessellatorDomain::Undefined;
  case D3D11_SB_TESSELLATOR_DOMAIN_ISOLINE:
    return DXIL::TessellatorDomain::IsoLine;
  case D3D11_SB_TESSELLATOR_DOMAIN_TRI:
    return DXIL::TessellatorDomain::Tri;
  case D3D11_SB_TESSELLATOR_DOMAIN_QUAD:
    return DXIL::TessellatorDomain::Quad;
  }

  IFTBOOL(false, DXC_E_INCORRECT_DXBC);
  return DXIL::TessellatorDomain::Undefined;
}

DXIL::TessellatorPartitioning
GetTessellatorPartitioning(D3D11_SB_TESSELLATOR_PARTITIONING TessPartitioning) {
  switch (TessPartitioning) {
  case D3D11_SB_TESSELLATOR_PARTITIONING_UNDEFINED:
    return DXIL::TessellatorPartitioning::Undefined;
  case D3D11_SB_TESSELLATOR_PARTITIONING_INTEGER:
    return DXIL::TessellatorPartitioning::Integer;
  case D3D11_SB_TESSELLATOR_PARTITIONING_POW2:
    return DXIL::TessellatorPartitioning::Pow2;
  case D3D11_SB_TESSELLATOR_PARTITIONING_FRACTIONAL_ODD:
    return DXIL::TessellatorPartitioning::FractionalOdd;
  case D3D11_SB_TESSELLATOR_PARTITIONING_FRACTIONAL_EVEN:
    return DXIL::TessellatorPartitioning::FractionalEven;
  }

  IFTBOOL(false, DXC_E_INCORRECT_DXBC);
  return DXIL::TessellatorPartitioning::Undefined;
}

DXIL::TessellatorOutputPrimitive GetTessellatorOutputPrimitive(
    D3D11_SB_TESSELLATOR_OUTPUT_PRIMITIVE TessOutputPrimitive) {
  switch (TessOutputPrimitive) {
  case D3D11_SB_TESSELLATOR_OUTPUT_UNDEFINED:
    return DXIL::TessellatorOutputPrimitive::Undefined;
  case D3D11_SB_TESSELLATOR_OUTPUT_POINT:
    return DXIL::TessellatorOutputPrimitive::Point;
  case D3D11_SB_TESSELLATOR_OUTPUT_LINE:
    return DXIL::TessellatorOutputPrimitive::Line;
  case D3D11_SB_TESSELLATOR_OUTPUT_TRIANGLE_CW:
    return DXIL::TessellatorOutputPrimitive::TriangleCW;
  case D3D11_SB_TESSELLATOR_OUTPUT_TRIANGLE_CCW:
    return DXIL::TessellatorOutputPrimitive::TriangleCCW;
  }

  IFTBOOL(false, DXC_E_INCORRECT_DXBC);
  return DXIL::TessellatorOutputPrimitive::Undefined;
}

} // namespace DXBC

//------------------------------------------------------------------------------
//
//  Asserts to match DXBC and DXIL constant values.
//
using namespace DXIL;

#define MSG "Constant value mismatch between DXBC and DXIL"

static_assert(kMaxTempRegCount == D3D11_COMMONSHADER_TEMP_REGISTER_COUNT, MSG);
static_assert(kMaxCBufferSize == D3D10_REQ_CONSTANT_BUFFER_ELEMENT_COUNT, MSG);

static_assert((int)DxilSampler::SamplerKind::Default ==
                  D3D10_SB_SAMPLER_MODE_DEFAULT,
              MSG);
static_assert((int)DxilSampler::SamplerKind::Comparison ==
                  D3D10_SB_SAMPLER_MODE_COMPARISON,
              MSG);
static_assert((int)DxilSampler::SamplerKind::Mono == D3D10_SB_SAMPLER_MODE_MONO,
              MSG);

static_assert(D3D10_SB_4_COMPONENT_X == 0, MSG);
static_assert(D3D10_SB_4_COMPONENT_Y == 1, MSG);
static_assert(D3D10_SB_4_COMPONENT_Z == 2, MSG);
static_assert(D3D10_SB_4_COMPONENT_W == 3, MSG);

static_assert(
    D3D_MIN_PRECISION_DEFAULT ==
        static_cast<D3D_MIN_PRECISION>(D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT),
    MSG);
static_assert(
    D3D_MIN_PRECISION_FLOAT_16 ==
        static_cast<D3D_MIN_PRECISION>(D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_16),
    MSG);
static_assert(D3D_MIN_PRECISION_FLOAT_2_8 ==
                  static_cast<D3D_MIN_PRECISION>(
                      D3D11_SB_OPERAND_MIN_PRECISION_FLOAT_2_8),
              MSG);
static_assert(
    D3D_MIN_PRECISION_SINT_16 ==
        static_cast<D3D_MIN_PRECISION>(D3D11_SB_OPERAND_MIN_PRECISION_SINT_16),
    MSG);
static_assert(
    D3D_MIN_PRECISION_UINT_16 ==
        static_cast<D3D_MIN_PRECISION>(D3D11_SB_OPERAND_MIN_PRECISION_UINT_16),
    MSG);

} // namespace hlsl
