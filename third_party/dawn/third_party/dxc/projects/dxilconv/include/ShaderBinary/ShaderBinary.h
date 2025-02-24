///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ShaderBinary.h                                                            //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Vertex shader binary format parsing and encoding.                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

// has dependencies on D3D10TokenizedProgramFormat.hpp! make sure to include
// that too!

typedef UINT CShaderToken;

//*****************************************************************************
//
// GetNumVertices
//
// Returns the number of vertices in a complete primitive
//
//*****************************************************************************
inline UINT GetNumVertices(D3D10_SB_PRIMITIVE PrimType) {
  switch (PrimType) {
  case D3D10_SB_PRIMITIVE_POINT:
    return 1;
  case D3D10_SB_PRIMITIVE_LINE:
    return 2;
  case D3D10_SB_PRIMITIVE_TRIANGLE:
    return 3;
  case D3D10_SB_PRIMITIVE_LINE_ADJ:
    return 4;
  case D3D10_SB_PRIMITIVE_TRIANGLE_ADJ:
    return 6;
  case D3D11_SB_PRIMITIVE_1_CONTROL_POINT_PATCH:
    return 1;
  case D3D11_SB_PRIMITIVE_2_CONTROL_POINT_PATCH:
    return 2;
  case D3D11_SB_PRIMITIVE_3_CONTROL_POINT_PATCH:
    return 3;
  case D3D11_SB_PRIMITIVE_4_CONTROL_POINT_PATCH:
    return 4;
  case D3D11_SB_PRIMITIVE_5_CONTROL_POINT_PATCH:
    return 5;
  case D3D11_SB_PRIMITIVE_6_CONTROL_POINT_PATCH:
    return 6;
  case D3D11_SB_PRIMITIVE_7_CONTROL_POINT_PATCH:
    return 7;
  case D3D11_SB_PRIMITIVE_8_CONTROL_POINT_PATCH:
    return 8;
  case D3D11_SB_PRIMITIVE_9_CONTROL_POINT_PATCH:
    return 9;
  case D3D11_SB_PRIMITIVE_10_CONTROL_POINT_PATCH:
    return 10;
  case D3D11_SB_PRIMITIVE_11_CONTROL_POINT_PATCH:
    return 11;
  case D3D11_SB_PRIMITIVE_12_CONTROL_POINT_PATCH:
    return 12;
  case D3D11_SB_PRIMITIVE_13_CONTROL_POINT_PATCH:
    return 13;
  case D3D11_SB_PRIMITIVE_14_CONTROL_POINT_PATCH:
    return 14;
  case D3D11_SB_PRIMITIVE_15_CONTROL_POINT_PATCH:
    return 15;
  case D3D11_SB_PRIMITIVE_16_CONTROL_POINT_PATCH:
    return 16;
  case D3D11_SB_PRIMITIVE_17_CONTROL_POINT_PATCH:
    return 17;
  case D3D11_SB_PRIMITIVE_18_CONTROL_POINT_PATCH:
    return 18;
  case D3D11_SB_PRIMITIVE_19_CONTROL_POINT_PATCH:
    return 19;
  case D3D11_SB_PRIMITIVE_20_CONTROL_POINT_PATCH:
    return 20;
  case D3D11_SB_PRIMITIVE_21_CONTROL_POINT_PATCH:
    return 21;
  case D3D11_SB_PRIMITIVE_22_CONTROL_POINT_PATCH:
    return 22;
  case D3D11_SB_PRIMITIVE_23_CONTROL_POINT_PATCH:
    return 23;
  case D3D11_SB_PRIMITIVE_24_CONTROL_POINT_PATCH:
    return 24;
  case D3D11_SB_PRIMITIVE_25_CONTROL_POINT_PATCH:
    return 25;
  case D3D11_SB_PRIMITIVE_26_CONTROL_POINT_PATCH:
    return 26;
  case D3D11_SB_PRIMITIVE_27_CONTROL_POINT_PATCH:
    return 27;
  case D3D11_SB_PRIMITIVE_28_CONTROL_POINT_PATCH:
    return 28;
  case D3D11_SB_PRIMITIVE_29_CONTROL_POINT_PATCH:
    return 29;
  case D3D11_SB_PRIMITIVE_30_CONTROL_POINT_PATCH:
    return 30;
  case D3D11_SB_PRIMITIVE_31_CONTROL_POINT_PATCH:
    return 31;
  case D3D11_SB_PRIMITIVE_32_CONTROL_POINT_PATCH:
    return 32;
  default:
    return 0;
  }
}

/*==========================================================================;
 *
 *  D3D10ShaderBinary namespace
 *
 *  File:       ShaderBinary.h
 *  Content:    Vertex shader assembler support
 *
 ***************************************************************************/

namespace D3D10ShaderBinary {

const UINT MAX_INSTRUCTION_LENGTH = 128;
const UINT D3D10_SB_MAX_INSTRUCTION_OPERANDS = 8;
const UINT D3D11_SB_MAX_CALL_OPERANDS = 0x10000;
const UINT D3D11_SB_MAX_NUM_TYPES = 0x10000;

typedef enum D3D10_SB_OPCODE_CLASS {
  D3D10_SB_FLOAT_OP,
  D3D10_SB_INT_OP,
  D3D10_SB_UINT_OP,
  D3D10_SB_BIT_OP,
  D3D10_SB_FLOW_OP,
  D3D10_SB_TEX_OP,
  D3D10_SB_DCL_OP,
  D3D11_SB_ATOMIC_OP,
  D3D11_SB_MEM_OP,
  D3D11_SB_DOUBLE_OP,
  D3D11_SB_FLOAT_TO_DOUBLE_OP,
  D3D11_SB_DOUBLE_TO_FLOAT_OP,
  D3D11_SB_DEBUG_OP,
} D3D10_SB_OPCODE_CLASS;

struct CInstructionInfo {
  void Set(BYTE NumOperands, LPCSTR Name, D3D10_SB_OPCODE_CLASS OpClass,
           BYTE InPrecisionFromOutMask) {
    m_NumOperands = NumOperands;
    m_InPrecisionFromOutMask = InPrecisionFromOutMask;

    StringCchCopyA(m_Name, sizeof(m_Name), Name);

    m_OpClass = OpClass;
  }

  char m_Name[64];
  BYTE m_NumOperands;
  BYTE m_InPrecisionFromOutMask;
  D3D10_SB_OPCODE_CLASS m_OpClass;
};

extern CInstructionInfo g_InstructionInfo[D3D10_SB_NUM_OPCODES];

UINT GetNumInstructionOperands(D3D10_SB_OPCODE_TYPE OpCode);
void InitInstructionInfo();

//*****************************************************************************
//
// class COperandIndex
//
// Represents a dimension index of an operand
//
//*****************************************************************************

class COperandIndex {
public:
  COperandIndex() : m_bExtendedOperand(FALSE) {}
  // Value for the immediate index type
  union {
    UINT m_RegIndex;
    UINT m_RegIndexA[2];
    INT64 m_RegIndex64;
  };
  // Data for the relative index type
  D3D10_SB_OPERAND_TYPE m_RelRegType;
  D3D10_SB_4_COMPONENT_NAME m_ComponentName;
  D3D10_SB_OPERAND_INDEX_DIMENSION m_IndexDimension;

  BOOL m_bExtendedOperand;
  D3D11_SB_OPERAND_MIN_PRECISION m_MinPrecision;
  BOOL m_Nonuniform;
  D3D10_SB_EXTENDED_OPERAND_TYPE m_ExtendedOperandType;

  // First index of the relative register
  union {
    UINT m_RelIndex;
    UINT m_RelIndexA[2];
    INT64 m_RelIndex64;
  };
  // Second index of the relative register
  union {
    UINT m_RelIndex1;
    UINT m_RelIndexA1[2];
    INT64 m_RelIndex641;
  };

  void SetMinPrecision(D3D11_SB_OPERAND_MIN_PRECISION MinPrec) {
    m_MinPrecision = MinPrec;
    if (MinPrec != D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
      m_bExtendedOperand = true;
      m_ExtendedOperandType =
          D3D10_SB_EXTENDED_OPERAND_MODIFIER; // piggybacking on modifier token
                                              // for minprecision
    }
  }

  void SetNonuniformIndex(bool bNonuniform = false) {
    m_Nonuniform = bNonuniform;
    if (bNonuniform) {
      m_bExtendedOperand = true;
      m_ExtendedOperandType = D3D10_SB_EXTENDED_OPERAND_MODIFIER;
    }
  }
};

enum MinPrecQuantizeFunctionIndex // Used by reference rasterizer (IHVs can
                                  // ignore)
{
  MinPrecFuncDefault = 0,
  MinPrecFunc2_8,
  MinPrecFunc16,
  MinPrecFuncUint16,
  MinPrecFuncInt16,
};

//*****************************************************************************
//
// class COperandBase
//
// A base class for shader instruction operands
//
//*****************************************************************************

class COperandBase {
public:
  COperandBase() { Clear(); }
  COperandBase(const COperandBase &Op) { memcpy(this, &Op, sizeof(*this)); }
  COperandBase &operator=(const COperandBase &Op) {
    if (this != &Op)
      memcpy(this, &Op, sizeof(*this));
    return *this;
  }
  D3D10_SB_OPERAND_TYPE OperandType() const { return m_Type; }
  const COperandIndex *OperandIndex(UINT Index) const {
    return &m_Index[Index];
  }
  D3D10_SB_OPERAND_INDEX_REPRESENTATION OperandIndexType(UINT Index) const {
    return m_IndexType[Index];
  }
  D3D10_SB_OPERAND_INDEX_DIMENSION OperandIndexDimension() const {
    return m_IndexDimension;
  }
  D3D10_SB_OPERAND_NUM_COMPONENTS NumComponents() const {
    return m_NumComponents;
  }
  // Get the register index for a given dimension
  UINT RegIndex(UINT Dimension = 0) const {
    return m_Index[Dimension].m_RegIndex;
  }
  // Get the register index from the lowest dimension
  UINT RegIndexForMinorDimension() const {
    switch (m_IndexDimension) {
    default:
    case D3D10_SB_OPERAND_INDEX_1D:
      return RegIndex(0);
    case D3D10_SB_OPERAND_INDEX_2D:
      return RegIndex(1);
    case D3D10_SB_OPERAND_INDEX_3D:
      return RegIndex(2);
    }
  }
  // Get the write mask
  UINT WriteMask() const { return m_WriteMask; }
  // Get the swizzle
  UINT SwizzleComponent(UINT index) const { return m_Swizzle[index]; }
  // Get immediate 32 bit value
  UINT Imm32() const { return m_Value[0]; }
  void SetModifier(D3D10_SB_OPERAND_MODIFIER Modifier) {
    m_Modifier = Modifier;
    if (Modifier != D3D10_SB_OPERAND_MODIFIER_NONE) {
      m_bExtendedOperand = true;
      m_ExtendedOperandType = D3D10_SB_EXTENDED_OPERAND_MODIFIER;
    }
  }
  void SetMinPrecision(D3D11_SB_OPERAND_MIN_PRECISION MinPrec) {
    m_MinPrecision = MinPrec;
    if (m_MinPrecision != D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
      m_bExtendedOperand = true;
      m_ExtendedOperandType =
          D3D10_SB_EXTENDED_OPERAND_MODIFIER; // reusing same extended operand
                                              // token as modifiers.
    }
  }
  void SetNonuniform(bool bNonuniform = false) {
    m_Nonuniform = bNonuniform;
    if (bNonuniform) {
      m_bExtendedOperand = true;
      m_ExtendedOperandType = D3D10_SB_EXTENDED_OPERAND_MODIFIER;
    }
  }
  D3D10_SB_OPERAND_MODIFIER Modifier() const { return m_Modifier; }
  void SetSwizzle(BYTE SwizzleX = D3D10_SB_4_COMPONENT_X,
                  BYTE SwizzleY = D3D10_SB_4_COMPONENT_Y,
                  BYTE SwizzleZ = D3D10_SB_4_COMPONENT_Z,
                  BYTE SwizzleW = D3D10_SB_4_COMPONENT_W) {
    m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SWIZZLE_MODE;
    m_Swizzle[0] = SwizzleX;
    m_Swizzle[1] = SwizzleY;
    m_Swizzle[2] = SwizzleZ;
    m_Swizzle[3] = SwizzleW;
  }
  void SelectComponent(
      D3D10_SB_4_COMPONENT_NAME ComponentName = D3D10_SB_4_COMPONENT_X) {
    m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE;
    m_ComponentName = ComponentName;
  }
  void SetMask(UINT Mask = D3D10_SB_OPERAND_4_COMPONENT_MASK_ALL) {
    m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE;
    m_WriteMask = Mask;
  }
  void SetIndex(UINT Dim, UINT Imm32) {
    m_IndexType[Dim] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
    m_Index[Dim].m_RegIndex = Imm32;
  }
  void SetIndex(UINT Dim, UINT Offset, D3D10_SB_OPERAND_TYPE RelRegType,
                UINT RelRegIndex0, UINT RelRegIndex1,
                D3D10_SB_4_COMPONENT_NAME RelComponentName,
                D3D11_SB_OPERAND_MIN_PRECISION RelRegMinPrecision =
                    D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_IndexType[Dim] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32;
    if (Offset == 0)
      m_IndexType[Dim] = D3D10_SB_OPERAND_INDEX_RELATIVE;
    else
      m_IndexType[Dim] = D3D10_SB_OPERAND_INDEX_IMMEDIATE32_PLUS_RELATIVE;
    m_Index[Dim].m_RegIndex = Offset; // immediate offset, such as the 3 in
                                      // cb0[x1[2].x + 3] or cb0[r1.x + 3]
    m_Index[Dim].m_RelRegType = RelRegType;
    if (RelRegType == D3D10_SB_OPERAND_TYPE_INDEXABLE_TEMP)
      m_Index[Dim].m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
    else
      m_Index[Dim].m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
    m_Index[Dim].m_RelIndex =
        RelRegIndex0; // relative register index, such as the 1 in cb0[x1[2].x +
                      // 3] or cb0[r1.x + 3]
    m_Index[Dim].m_RelIndex1 =
        RelRegIndex1; // relative register second dimension index, such as the 2
                      // in cb0[x1[2].x + 3]
    m_Index[Dim].m_ComponentName = RelComponentName;
    m_Index[Dim].SetMinPrecision(RelRegMinPrecision);
  }

public: // esp in the unions...it's just redundant to not directly access things
  void Clear() { memset(this, 0, sizeof(*this)); }
  MinPrecQuantizeFunctionIndex
      m_MinPrecQuantizeFunctionIndex; // used by ref for low precision (IHVs can
                                      // ignore)
  D3D10_SB_OPERAND_TYPE m_Type;
  COperandIndex m_Index[3];
  D3D10_SB_OPERAND_NUM_COMPONENTS m_NumComponents;
  D3D10_SB_OPERAND_4_COMPONENT_SELECTION_MODE m_ComponentSelection;
  BOOL m_bExtendedOperand;
  D3D10_SB_OPERAND_MODIFIER m_Modifier;
  D3D11_SB_OPERAND_MIN_PRECISION m_MinPrecision;
  BOOL m_Nonuniform;
  D3D10_SB_EXTENDED_OPERAND_TYPE m_ExtendedOperandType;
  union {
    UINT m_WriteMask;
    BYTE m_Swizzle[4];
  };
  D3D10_SB_4_COMPONENT_NAME m_ComponentName;
  union {
    UINT m_Value[4];
    float m_Valuef[4];
    INT64 m_Value64[2];
    double m_Valued[2];
  };

#pragma warning(suppress : 4201) // Warning about nameless structure.
  struct {
    D3D10_SB_OPERAND_INDEX_REPRESENTATION m_IndexType[3];
    D3D10_SB_OPERAND_INDEX_DIMENSION m_IndexDimension;
#pragma warning(suppress : 4201) // Warning about nameless structure.
  };

  friend class CShaderAsm;
  friend class CShaderCodeParser;
  friend class CInstruction;
  friend class COperand;
  friend class COperandDst;
};

//*****************************************************************************
//
// class COperand
//
// Encapsulates a source operand in shader instructions
//
//*****************************************************************************

class COperand : public COperandBase {
public:
  COperand() : COperandBase() {}
  COperand(UINT Imm32) : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    m_WriteMask = 0;
    m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE32;
    m_bExtendedOperand = FALSE;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
    m_Value[0] = Imm32;
    m_NumComponents = D3D10_SB_OPERAND_1_COMPONENT;
  }
  COperand(int Imm32) : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    m_WriteMask = 0;
    m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE32;
    m_bExtendedOperand = FALSE;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
    m_Value[0] = Imm32;
    m_NumComponents = D3D10_SB_OPERAND_1_COMPONENT;
  }
  COperand(float Imm32) : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    m_WriteMask = 0;
    m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE32;
    m_bExtendedOperand = FALSE;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
    m_Valuef[0] = Imm32;
    m_NumComponents = D3D10_SB_OPERAND_1_COMPONENT;
  }
  COperand(INT64 Imm64) : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    m_WriteMask = 0;
    m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE64;
    m_bExtendedOperand = FALSE;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
    m_Value64[0] = Imm64;
    m_NumComponents = D3D10_SB_OPERAND_1_COMPONENT;
  }
  COperand(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex,
           D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
               D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    m_Type = Type;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_0_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
    SetIndex(0, RegIndex);
  }
  // Immediate constant
  COperand(float v1, float v2, float v3, float v4) : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle();
    m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE32;
    m_bExtendedOperand = FALSE;
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
    m_Valuef[0] = v1;
    m_Valuef[1] = v2;
    m_Valuef[2] = v3;
    m_Valuef[3] = v4;
  }
  // Immediate constant
  COperand(double v1, double v2) : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle();
    m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE64;
    m_bExtendedOperand = FALSE;
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
    m_Valued[0] = v1;
    m_Valued[1] = v2;
  }
  // Immediate constant
  COperand(float v1, float v2, float v3, float v4, BYTE SwizzleX, BYTE SwizzleY,
           BYTE SwizzleZ, BYTE SwizzleW)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle(SwizzleX, SwizzleY, SwizzleZ, SwizzleW);
    m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE32;
    m_bExtendedOperand = FALSE;
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
    m_Valuef[0] = v1;
    m_Valuef[1] = v2;
    m_Valuef[2] = v3;
    m_Valuef[3] = v4;
  }

  // Immediate constant
  COperand(int v1, int v2, int v3, int v4) : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle();
    m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE32;
    m_bExtendedOperand = FALSE;
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
    m_Value[0] = v1;
    m_Value[1] = v2;
    m_Value[2] = v3;
    m_Value[3] = v4;
  }
  // Immediate constant
  COperand(int v1, int v2, int v3, int v4, BYTE SwizzleX, BYTE SwizzleY,
           BYTE SwizzleZ, BYTE SwizzleW)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle(SwizzleX, SwizzleY, SwizzleZ, SwizzleW);
    m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE32;
    m_bExtendedOperand = FALSE;
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
    m_Value[0] = v1;
    m_Value[1] = v2;
    m_Value[2] = v3;
    m_Value[3] = v4;
  }
  COperand(INT64 v1, INT64 v2) : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle();
    m_Type = D3D10_SB_OPERAND_TYPE_IMMEDIATE64;
    m_bExtendedOperand = FALSE;
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
    m_Value64[0] = v1;
    m_Value64[1] = v2;
  }

  COperand(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex, BYTE SwizzleX,
           BYTE SwizzleY, BYTE SwizzleZ, BYTE SwizzleW,
           D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
               D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle(SwizzleX, SwizzleY, SwizzleZ, SwizzleW);
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
    SetIndex(0, RegIndex);
  }

  // Used for operands without indices
  COperand(D3D10_SB_OPERAND_TYPE Type,
           D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
               D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Type = Type;
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
    if ((Type == D3D10_SB_OPERAND_TYPE_INPUT_PRIMITIVEID) ||
        (Type == D3D11_SB_OPERAND_TYPE_OUTPUT_CONTROL_POINT_ID) ||
        (Type == D3D11_SB_OPERAND_TYPE_INPUT_COVERAGE_MASK) ||
        (Type == D3D11_SB_OPERAND_TYPE_INNER_COVERAGE) ||
        (Type == D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP_FLATTENED) ||
        (Type == D3D11_SB_OPERAND_TYPE_INPUT_GS_INSTANCE_ID)) {
      m_NumComponents = D3D10_SB_OPERAND_1_COMPONENT;
    } else if ((Type == D3D11_SB_OPERAND_TYPE_INPUT_DOMAIN_POINT) ||
               (Type == D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID) ||
               (Type == D3D11_SB_OPERAND_TYPE_INPUT_THREAD_GROUP_ID) ||
               (Type == D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP) ||
               (Type == D3D11_SB_OPERAND_TYPE_CYCLE_COUNTER)) {
      m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    } else {
      m_NumComponents = D3D10_SB_OPERAND_0_COMPONENT;
    }
  }

  // source operand with relative addressing
  COperand(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex,
           D3D10_SB_OPERAND_TYPE RelRegType, UINT RelRegIndex,
           D3D10_SB_4_COMPONENT_NAME RelComponentName,
           D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
               D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT,
           D3D11_SB_OPERAND_MIN_PRECISION RelRegMinPrecision =
               D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_0_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
    SetIndex(0, RegIndex, RelRegType, RelRegIndex, 0xFFFFFFFF, RelComponentName,
             RelRegMinPrecision);
  }

  friend class CShaderAsm;
  friend class CShaderCodeParser;
  friend class CInstruction;
};

//*****************************************************************************
//
// class COperand4
//
// Encapsulates a source operand with 4 components in shader instructions
//
//*****************************************************************************

class COperand4 : public COperandBase {
public:
  COperand4(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex,
            D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle();
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
    SetIndex(0, RegIndex);
  }
  COperand4(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex,
            D3D10_SB_4_COMPONENT_NAME Component,
            D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE;
    m_ComponentName = Component;
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
    SetIndex(0, RegIndex);
  }
  // single component select on reg, 1D indexing on address
  COperand4(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex,
            D3D10_SB_4_COMPONENT_NAME Component,
            D3D10_SB_OPERAND_TYPE RelRegType, UINT RelRegIndex,
            D3D10_SB_4_COMPONENT_NAME RelComponentName,
            D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT,
            D3D11_SB_OPERAND_MIN_PRECISION RelRegMinPrecision =
                D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE;
    m_ComponentName = Component;
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
    SetIndex(0, RegIndex, RelRegType, RelRegIndex, 0xFFFFFFFF, RelComponentName,
             RelRegMinPrecision);
  }
  // 4-component source operand with relative addressing
  COperand4(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex,
            D3D10_SB_OPERAND_TYPE RelRegType, UINT RelRegIndex,
            D3D10_SB_4_COMPONENT_NAME RelComponentName,
            D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT,
            D3D11_SB_OPERAND_MIN_PRECISION RelRegMinPrecision =
                D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle();
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
    SetIndex(0, RegIndex, RelRegType, RelRegIndex, 0xFFFFFFFF, RelComponentName,
             RelRegMinPrecision);
  }
  // 4-component source operand with relative addressing
  COperand4(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex,
            D3D10_SB_OPERAND_TYPE RelRegType, UINT RelRegIndex,
            UINT RelRegIndex1, D3D10_SB_4_COMPONENT_NAME RelComponentName,
            D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT,
            D3D11_SB_OPERAND_MIN_PRECISION RelRegMinPrecision =
                D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle();
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
    SetIndex(0, RegIndex, RelRegType, RelRegIndex, RelRegIndex1,
             RelComponentName, RelRegMinPrecision);
  }
  COperand4(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex, BYTE SwizzleX,
            BYTE SwizzleY, BYTE SwizzleZ, BYTE SwizzleW,
            D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle(SwizzleX, SwizzleY, SwizzleZ, SwizzleW);
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
    SetIndex(0, RegIndex);
  }
  // 4-component source operand with relative addressing
  COperand4(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex, BYTE SwizzleX,
            BYTE SwizzleY, BYTE SwizzleZ, BYTE SwizzleW,
            D3D10_SB_OPERAND_TYPE RelRegType, UINT RelRegIndex,
            UINT RelRegIndex1, D3D10_SB_4_COMPONENT_NAME RelComponentName,
            D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT,
            D3D11_SB_OPERAND_MIN_PRECISION RelRegMinPrecision =
                D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle(SwizzleX, SwizzleY, SwizzleZ, SwizzleW);
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
    SetIndex(0, RegIndex, RelRegType, RelRegIndex, RelRegIndex1,
             RelComponentName, RelRegMinPrecision);
  }

  friend class CShaderAsm;
  friend class CShaderCodeParser;
  friend class CInstruction;
};
//*****************************************************************************
//
// class COperandDst
//
// Encapsulates a destination operand in shader instructions
//
//*****************************************************************************

class COperandDst : public COperandBase {
public:
  COperandDst(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex,
              D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                  D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetMask();
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
    SetIndex(0, RegIndex);
  }
  COperandDst(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex, UINT WriteMask,
              D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                  D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetMask(WriteMask);
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
    SetIndex(0, RegIndex);
  }
  COperandDst(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex, UINT WriteMask,
              D3D10_SB_OPERAND_TYPE RelRegType, UINT RelRegIndex,
              UINT RelRegIndex1, D3D10_SB_4_COMPONENT_NAME RelComponentName,
              D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                  D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT,
              D3D11_SB_OPERAND_MIN_PRECISION RelRegMinPrecision =
                  D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetMask(WriteMask);
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_1D;
    SetIndex(0, RegIndex, RelRegType, RelRegIndex, RelRegIndex1,
             RelComponentName, RelRegMinPrecision);
  }
  COperandDst(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex, UINT WriteMask,
              D3D10_SB_OPERAND_TYPE RelRegType, UINT RelRegIndex,
              UINT RelRegIndex1, D3D10_SB_4_COMPONENT_NAME RelComponentName,
              UINT,
              D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                  D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT,
              D3D11_SB_OPERAND_MIN_PRECISION RelReg1MinPrecision =
                  D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE;
    m_WriteMask = WriteMask;
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
    SetIndex(0, RegIndex);
    SetIndex(1, RelRegIndex, RelRegType, RelRegIndex1, 0, RelComponentName,
             RelReg1MinPrecision);
  }
  // 2D dst (e.g. for GS input decl)
  COperandDst(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex0, UINT RegIndex1,
              UINT WriteMask,
              D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                  D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE;
    m_WriteMask = WriteMask;
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
    SetIndex(0, RegIndex0);
    SetIndex(1, RegIndex1);
  }
  // Used for operands without indices
  COperandDst(D3D10_SB_OPERAND_TYPE Type,
              D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                  D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    switch (Type) {
    case D3D10_SB_OPERAND_TYPE_OUTPUT_DEPTH:
    case D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_GREATER_EQUAL:
    case D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_LESS_EQUAL:
    case D3D11_SB_OPERAND_TYPE_OUTPUT_STENCIL_REF:
      m_NumComponents = D3D10_SB_OPERAND_1_COMPONENT;
      break;
    default:
      m_NumComponents = D3D10_SB_OPERAND_0_COMPONENT;
      break;
    }
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
  }
  COperandDst(UINT WriteMask, D3D10_SB_OPERAND_TYPE Type,
              D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                  D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() // param order disambiguates from another constructor.
  {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_MASK_MODE;
    m_WriteMask = WriteMask;
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_0D;
  }

  friend class CShaderAsm;
  friend class CShaderCodeParser;
  friend class CInstruction;
};

//*****************************************************************************
//
// class COperand2D
//
// Encapsulates 2 dimensional source operand with 4 components in shader
// instructions
//
//*****************************************************************************

class COperand2D : public COperandBase {
public:
  COperand2D(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex0, UINT RegIndex1,
             D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                 D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle();
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
    SetIndex(0, RegIndex0);
    SetIndex(1, RegIndex1);
  }
  COperand2D(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex0, UINT RegIndex1,
             D3D10_SB_4_COMPONENT_NAME Component,
             D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                 D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    m_ComponentSelection = D3D10_SB_OPERAND_4_COMPONENT_SELECT_1_MODE;
    m_ComponentName = Component;
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
    SetIndex(0, RegIndex0);
    SetIndex(1, RegIndex1);
  }
  // 2-dimensional 4-component operand with relative addressing the second index
  // For example:
  //      c2[x12[3].w + 7]
  //  Type = c
  //  RelRegType = x
  //  RegIndex0 = 2
  //  RegIndex1 = 7
  //  RelRegIndex = 12
  //  RelRegIndex1 = 3
  //  RelComponentName = w
  //
  COperand2D(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex0, UINT RegIndex1,
             D3D10_SB_OPERAND_TYPE RelRegType, UINT RelRegIndex,
             UINT RelRegIndex1, D3D10_SB_4_COMPONENT_NAME RelComponentName,
             D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                 D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT,
             D3D11_SB_OPERAND_MIN_PRECISION RelReg1MinPrecision =
                 D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle();
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
    SetIndex(0, RegIndex0);
    SetIndex(1, RegIndex1, RelRegType, RelRegIndex, RelRegIndex1,
             RelComponentName, RelReg1MinPrecision);
  }
  // 2-dimensional 4-component operand with relative addressing a second index
  // For example:
  //      c2[r12.y + 7]
  //  Type = c
  //  RelRegType = r
  //  RegIndex0 = 2
  //  RegIndex1 = 7
  //  RelRegIndex = 12
  //  RelRegIndex1 = 3
  //  RelComponentName = y
  //
  COperand2D(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex0, UINT RegIndex1,
             D3D10_SB_OPERAND_TYPE RelRegType, UINT RelRegIndex,
             D3D10_SB_4_COMPONENT_NAME RelComponentName,
             D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                 D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT,
             D3D11_SB_OPERAND_MIN_PRECISION RelReg1MinPrecision =
                 D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle();
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
    SetIndex(0, RegIndex0);
    SetIndex(1, RegIndex1, RelRegType, RelRegIndex, 0, RelComponentName,
             RelReg1MinPrecision);
  }
  // 2-dimensional 4-component operand with relative addressing both operands
  COperand2D(D3D10_SB_OPERAND_TYPE Type, BOOL bIndexRelative0,
             BOOL bIndexRelative1, UINT RegIndex0, UINT RegIndex1,
             D3D10_SB_OPERAND_TYPE RelRegType0, UINT RelRegIndex0,
             UINT RelRegIndex10, D3D10_SB_4_COMPONENT_NAME RelComponentName0,
             D3D10_SB_OPERAND_TYPE RelRegType1, UINT RelRegIndex1,
             UINT RelRegIndex11, D3D10_SB_4_COMPONENT_NAME RelComponentName1,
             D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                 D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT,
             D3D11_SB_OPERAND_MIN_PRECISION RelReg0MinPrecision =
                 D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT,
             D3D11_SB_OPERAND_MIN_PRECISION RelReg1MinPrecision =
                 D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle();
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
    if (bIndexRelative0)
      SetIndex(0, RegIndex0, RelRegType0, RelRegIndex0, RelRegIndex10,
               RelComponentName0, RelReg0MinPrecision);
    else
      SetIndex(0, RegIndex0);
    if (bIndexRelative1)
      SetIndex(1, RegIndex1, RelRegType1, RelRegIndex1, RelRegIndex11,
               RelComponentName1, RelReg1MinPrecision);
    else
      SetIndex(1, RegIndex1);
  }
  COperand2D(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex0, UINT RegIndex1,
             BYTE SwizzleX, BYTE SwizzleY, BYTE SwizzleZ, BYTE SwizzleW,
             D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                 D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle(SwizzleX, SwizzleY, SwizzleZ, SwizzleW);
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
    SetIndex(0, RegIndex0);
    SetIndex(1, RegIndex1);
  }
  // 2-dimensional 4-component operand with relative addressing and swizzle
  COperand2D(D3D10_SB_OPERAND_TYPE Type, BYTE SwizzleX, BYTE SwizzleY,
             BYTE SwizzleZ, BYTE SwizzleW, BOOL bIndexRelative0,
             BOOL bIndexRelative1, UINT RegIndex0,
             D3D10_SB_OPERAND_TYPE RelRegType0, UINT RelRegIndex0,
             UINT RelRegIndex10, D3D10_SB_4_COMPONENT_NAME RelComponentName0,
             UINT RegIndex1, D3D10_SB_OPERAND_TYPE RelRegType1,
             UINT RelRegIndex1, UINT RelRegIndex11,
             D3D10_SB_4_COMPONENT_NAME RelComponentName1,
             D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                 D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT,
             D3D11_SB_OPERAND_MIN_PRECISION RelReg0MinPrecision =
                 D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT,
             D3D11_SB_OPERAND_MIN_PRECISION RelReg1MinPrecision =
                 D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle(SwizzleX, SwizzleY, SwizzleZ, SwizzleW);
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_2D;
    if (bIndexRelative0)
      SetIndex(0, RegIndex0, RelRegType0, RelRegIndex0, RelRegIndex10,
               RelComponentName0, RelReg0MinPrecision);
    else
      SetIndex(0, RegIndex0);

    if (bIndexRelative1)
      SetIndex(1, RegIndex1, RelRegType1, RelRegIndex1, RelRegIndex11,
               RelComponentName1, RelReg1MinPrecision);
    else
      SetIndex(1, RegIndex1);
  }

  friend class CShaderAsm;
  friend class CShaderCodeParser;
  friend class CInstruction;
};

class COperand3D : public COperandBase {
public:
  COperand3D(D3D10_SB_OPERAND_TYPE Type, UINT RegIndex0, UINT RegIndex1,
             UINT RegIndex2,
             D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                 D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT)
      : COperandBase() {
    m_Modifier = D3D10_SB_OPERAND_MODIFIER_NONE;
    SetSwizzle();
    m_Type = Type;
    m_bExtendedOperand = FALSE;
    SetMinPrecision(MinPrecision);
    m_NumComponents = D3D10_SB_OPERAND_4_COMPONENT;
    m_IndexDimension = D3D10_SB_OPERAND_INDEX_3D;
    SetIndex(0, RegIndex0);
    SetIndex(1, RegIndex1);
    SetIndex(2, RegIndex2);
  }

  friend class CShaderAsm;
  friend class CShaderCodeParser;
  friend class CInstruction;
};

//*****************************************************************************
//
//  CInstruction
//
//*****************************************************************************

// Structures for additional per-instruction fields unioned in CInstruction.
// These structures don't contain ALL info used by the particular instruction,
// only additional info not already in CInstruction.  Some instructions don't
// need such structures because CInstruction already has the correct data
// fields.

struct CGlobalFlagsDecl {
  UINT Flags;
};

struct CInputSystemInterpretedValueDecl {
  D3D10_SB_NAME Name;
};

struct CInputSystemGeneratedValueDecl {
  D3D10_SB_NAME Name;
};

struct CInputPSDecl {
  D3D10_SB_INTERPOLATION_MODE InterpolationMode;
};

struct CInputPSSystemInterpretedValueDecl {
  D3D10_SB_NAME Name;
  D3D10_SB_INTERPOLATION_MODE InterpolationMode;
};

struct CInputPSSystemGeneratedValueDecl {
  D3D10_SB_NAME Name;
  D3D10_SB_INTERPOLATION_MODE InterpolationMode;
};

struct COutputSystemInterpretedValueDecl {
  D3D10_SB_NAME Name;
};

struct COutputSystemGeneratedValueDecl {
  D3D10_SB_NAME Name;
};

struct CIndexRangeDecl {
  UINT RegCount;
};

struct CResourceDecl {
  D3D10_SB_RESOURCE_DIMENSION Dimension;
  D3D10_SB_RESOURCE_RETURN_TYPE ReturnType[4];
  UINT SampleCount;
  UINT Space;
};

struct CConstantBufferDecl {
  D3D10_SB_CONSTANT_BUFFER_ACCESS_PATTERN AccessPattern;
  UINT Size;
  UINT Space;
};

struct COutputTopologyDecl {
  D3D10_SB_PRIMITIVE_TOPOLOGY Topology;
};

struct CInputPrimitiveDecl {
  D3D10_SB_PRIMITIVE Primitive;
};

struct CGSMaxOutputVertexCountDecl {
  UINT MaxOutputVertexCount;
};

struct CGSInstanceCountDecl {
  UINT InstanceCount;
};

struct CSamplerDecl {
  D3D10_SB_SAMPLER_MODE SamplerMode;
  UINT Space;
};

struct CStreamDecl {
  UINT Stream;
};

struct CTempsDecl {
  UINT NumTemps;
};

struct CIndexableTempDecl {
  UINT IndexableTempNumber;
  UINT NumRegisters;
  UINT Mask; // .x, .xy, .xzy or .xyzw (D3D10_SB_OPERAND_4_COMPONENT_MASK_* )
};

struct CHSDSInputControlPointCountDecl {
  UINT InputControlPointCount;
};

struct CHSOutputControlPointCountDecl {
  UINT OutputControlPointCount;
};

struct CTessellatorDomainDecl {
  D3D11_SB_TESSELLATOR_DOMAIN TessellatorDomain;
};

struct CTessellatorPartitioningDecl {
  D3D11_SB_TESSELLATOR_PARTITIONING TessellatorPartitioning;
};

struct CTessellatorOutputPrimitiveDecl {
  D3D11_SB_TESSELLATOR_OUTPUT_PRIMITIVE TessellatorOutputPrimitive;
};

struct CHSMaxTessFactorDecl {
  float MaxTessFactor;
};

struct CHSForkPhaseInstanceCountDecl {
  UINT InstanceCount;
};

struct CHSJoinPhaseInstanceCountDecl {
  UINT InstanceCount;
};

struct CShaderMessage {
  D3D11_SB_SHADER_MESSAGE_ID MessageID;
  D3D11_SB_SHADER_MESSAGE_FORMAT FormatStyle;
  PCSTR pFormatString;
  UINT NumOperands;
  COperandBase *pOperands;
};

struct CCustomData {
  D3D10_SB_CUSTOMDATA_CLASS Type;
  UINT DataSizeInBytes;
  void *pData;

  union {
    CShaderMessage ShaderMessage;
  };
};

struct CFunctionTableDecl {
  UINT FunctionTableNumber;
  UINT TableLength;
  UINT *pFunctionIdentifiers;
};

struct CInterfaceDecl {
  WORD InterfaceNumber;
  WORD ArrayLength;
  UINT ExpectedTableSize;
  UINT TableLength;
  UINT *pFunctionTableIdentifiers;
  bool bDynamicallyIndexed;
};

struct CFunctionBodyDecl {
  UINT FunctionBodyNumber;
};

struct CInterfaceCall {
  UINT FunctionIndex;
  COperandBase *pInterfaceOperand;
};

struct CThreadGroupDeclaration {
  UINT x;
  UINT y;
  UINT z;
};

struct CTypedUAVDeclaration {
  D3D10_SB_RESOURCE_DIMENSION Dimension;
  D3D10_SB_RESOURCE_RETURN_TYPE ReturnType[4];
  UINT Flags;
  UINT Space;
};

struct CStructuredUAVDeclaration {
  UINT ByteStride;
  UINT Flags;
  UINT Space;
};

struct CRawUAVDeclaration {
  UINT Flags;
  UINT Space;
};

struct CRawTGSMDeclaration {
  UINT ByteCount;
};

struct CStructuredTGSMDeclaration {
  UINT StructByteStride;
  UINT StructCount;
};

struct CRawSRVDeclaration {
  UINT Space;
};

struct CStructuredSRVDeclaration {
  UINT ByteStride;
  UINT Space;
};

struct CSyncFlags {
  bool bThreadsInGroup;
  bool bThreadGroupSharedMemory;
  bool bUnorderedAccessViewMemoryGlobal;
  bool bUnorderedAccessViewMemoryGroup; // exclusive to global
};

class CInstruction {
protected:
  static const UINT MAX_PRIVATE_DATA_COUNT = 2;

public:
  CInstruction() : m_OpCode(D3D10_SB_OPCODE_ADD) { Clear(); }
  CInstruction(D3D10_SB_OPCODE_TYPE OpCode) {
    Clear();
    m_OpCode = OpCode;
    m_NumOperands = 0;
    m_ExtendedOpCodeCount = 0;
  }
  CInstruction(D3D10_SB_OPCODE_TYPE OpCode, COperandBase &Operand0,
               D3D10_SB_INSTRUCTION_TEST_BOOLEAN Test) {
    Clear();
    m_OpCode = OpCode;
    m_NumOperands = 1;
    m_ExtendedOpCodeCount = 0;
    m_Test = Test;
    m_Operands[0] = Operand0;
  }
  CInstruction(D3D10_SB_OPCODE_TYPE OpCode, COperandBase &Operand0,
               COperandBase &Operand1) {
    Clear();
    m_OpCode = OpCode;
    m_NumOperands = 2;
    m_ExtendedOpCodeCount = 0;
    m_Operands[0] = Operand0;
    m_Operands[1] = Operand1;
  }
  CInstruction(D3D10_SB_OPCODE_TYPE OpCode, COperandBase &Operand0,
               COperandBase &Operand1, COperandBase &Operand2) {
    Clear();
    m_OpCode = OpCode;
    m_NumOperands = 3;
    m_ExtendedOpCodeCount = 0;
    m_Operands[0] = Operand0;
    m_Operands[1] = Operand1;
    m_Operands[2] = Operand2;
  }
  CInstruction(D3D10_SB_OPCODE_TYPE OpCode, COperandBase &Operand0,
               COperandBase &Operand1, COperandBase &Operand2,
               COperandBase &Operand3) {
    Clear();
    m_OpCode = OpCode;
    m_NumOperands = 4;
    m_ExtendedOpCodeCount = 0;
    m_Operands[0] = Operand0;
    m_Operands[1] = Operand1;
    m_Operands[2] = Operand2;
    m_Operands[3] = Operand3;
    memset(m_TexelOffset, 0, sizeof(m_TexelOffset));
  }
  void ClearAllocations() {
    if (m_OpCode == D3D10_SB_OPCODE_CUSTOMDATA) {
      free(m_CustomData.pData);
      if (m_CustomData.Type == D3D11_SB_CUSTOMDATA_SHADER_MESSAGE) {
        free(m_CustomData.ShaderMessage.pOperands);
      }
    } else if (m_OpCode == D3D11_SB_OPCODE_DCL_FUNCTION_TABLE) {
      free(m_FunctionTableDecl.pFunctionIdentifiers);
    } else if (m_OpCode == D3D11_SB_OPCODE_DCL_INTERFACE) {
      free(m_InterfaceDecl.pFunctionTableIdentifiers);
    }
  }
  void Clear(bool bIncludeCustomData = false) {
    if (bIncludeCustomData) // don't need to do this on initial constructor,
                            // only if recycling the object.
    {
      ClearAllocations();
    }
    memset(this, 0, sizeof(*this));
  }
  ~CInstruction() { ClearAllocations(); }
  const COperandBase &Operand(UINT Index) const { return m_Operands[Index]; }
  D3D10_SB_OPCODE_TYPE OpCode() const { return m_OpCode; }
  void SetNumOperands(UINT NumOperands) { m_NumOperands = NumOperands; }
  UINT NumOperands() const { return m_NumOperands; }
  void SetTest(D3D10_SB_INSTRUCTION_TEST_BOOLEAN Test) { m_Test = Test; }
  void SetPreciseMask(UINT PreciseMask) { m_PreciseMask = PreciseMask; }
  D3D10_SB_INSTRUCTION_TEST_BOOLEAN Test() const { return m_Test; }
  void SetTexelOffset(const INT8 texelOffset[3]) {
    m_OpCodeEx[m_ExtendedOpCodeCount++] =
        D3D10_SB_EXTENDED_OPCODE_SAMPLE_CONTROLS;
    memcpy(m_TexelOffset, texelOffset, sizeof(m_TexelOffset));
  }
  void SetTexelOffset(INT8 x, INT8 y, INT8 z) {
    m_OpCodeEx[m_ExtendedOpCodeCount++] =
        D3D10_SB_EXTENDED_OPCODE_SAMPLE_CONTROLS;
    m_TexelOffset[0] = x;
    m_TexelOffset[1] = y;
    m_TexelOffset[2] = z;
  }
  void SetResourceDim(D3D10_SB_RESOURCE_DIMENSION Dim,
                      D3D10_SB_RESOURCE_RETURN_TYPE RetType[4],
                      UINT StructureStride) {
    m_OpCodeEx[m_ExtendedOpCodeCount++] = D3D11_SB_EXTENDED_OPCODE_RESOURCE_DIM;
    m_OpCodeEx[m_ExtendedOpCodeCount++] =
        D3D11_SB_EXTENDED_OPCODE_RESOURCE_RETURN_TYPE;
    m_ResourceDimEx = Dim;
    m_ResourceDimStructureStrideEx = StructureStride;
    memcpy(m_ResourceReturnTypeEx, RetType,
           4 * sizeof(D3D10_SB_RESOURCE_RETURN_TYPE));
  }
  BOOL Disassemble(__out_ecount(StringSize) LPSTR pString, UINT StringSize);

  // Private data is used by D3D runtime
  void SetPrivateData(UINT Value, UINT index = 0) {
    if (index < MAX_PRIVATE_DATA_COUNT) {
      m_PrivateData[index] = Value;
    }
  }
  UINT PrivateData(UINT index = 0) const {
    if (index >= MAX_PRIVATE_DATA_COUNT)
      return 0xFFFFFFFF;
    return m_PrivateData[index];
  }
  // Get the precise mask
  UINT GetPreciseMask() const { return m_PreciseMask; }

  D3D10_SB_OPCODE_TYPE m_OpCode;
  COperandBase m_Operands[D3D10_SB_MAX_INSTRUCTION_OPERANDS];
  UINT m_NumOperands;
  UINT m_ExtendedOpCodeCount;
  UINT m_PreciseMask;
  D3D10_SB_EXTENDED_OPCODE_TYPE
      m_OpCodeEx[D3D11_SB_MAX_SIMULTANEOUS_EXTENDED_OPCODES];
  INT8 m_TexelOffset[3];                       // for extended opcode only
  D3D10_SB_RESOURCE_DIMENSION m_ResourceDimEx; // for extended opcode only
  UINT m_ResourceDimStructureStrideEx;         // for extended opcode only
  D3D10_SB_RESOURCE_RETURN_TYPE
      m_ResourceReturnTypeEx[4];   // for extended opcode only
  BOOL m_bNonuniformResourceIndex; // for extended opcode only
  BOOL m_bNonuniformSamplerIndex;  // for extended opcode only
  UINT m_PrivateData[MAX_PRIVATE_DATA_COUNT];
  BOOL m_bSaturate;
  union // extra info needed by some instructions
  {
    CInputSystemInterpretedValueDecl m_InputDeclSIV;
    CInputSystemGeneratedValueDecl m_InputDeclSGV;
    CInputPSDecl m_InputPSDecl;
    CInputPSSystemInterpretedValueDecl m_InputPSDeclSIV;
    CInputPSSystemGeneratedValueDecl m_InputPSDeclSGV;
    COutputSystemInterpretedValueDecl m_OutputDeclSIV;
    COutputSystemGeneratedValueDecl m_OutputDeclSGV;
    CIndexRangeDecl m_IndexRangeDecl;
    CResourceDecl m_ResourceDecl;
    CConstantBufferDecl m_ConstantBufferDecl;
    CInputPrimitiveDecl m_InputPrimitiveDecl;
    COutputTopologyDecl m_OutputTopologyDecl;
    CGSMaxOutputVertexCountDecl m_GSMaxOutputVertexCountDecl;
    CGSInstanceCountDecl m_GSInstanceCountDecl;
    CSamplerDecl m_SamplerDecl;
    CStreamDecl m_StreamDecl;
    CTempsDecl m_TempsDecl;
    CIndexableTempDecl m_IndexableTempDecl;
    CGlobalFlagsDecl m_GlobalFlagsDecl;
    CCustomData m_CustomData;
    CInterfaceDecl m_InterfaceDecl;
    CFunctionTableDecl m_FunctionTableDecl;
    CFunctionBodyDecl m_FunctionBodyDecl;
    CInterfaceCall m_InterfaceCall;
    D3D10_SB_INSTRUCTION_TEST_BOOLEAN m_Test;
    D3D10_SB_RESINFO_INSTRUCTION_RETURN_TYPE m_ResInfoReturnType;
    D3D10_SB_INSTRUCTION_RETURN_TYPE m_InstructionReturnType;
    CHSDSInputControlPointCountDecl m_InputControlPointCountDecl;
    CHSOutputControlPointCountDecl m_OutputControlPointCountDecl;
    CTessellatorDomainDecl m_TessellatorDomainDecl;
    CTessellatorPartitioningDecl m_TessellatorPartitioningDecl;
    CTessellatorOutputPrimitiveDecl m_TessellatorOutputPrimitiveDecl;
    CHSMaxTessFactorDecl m_HSMaxTessFactorDecl;
    CHSForkPhaseInstanceCountDecl m_HSForkPhaseInstanceCountDecl;
    CHSJoinPhaseInstanceCountDecl m_HSJoinPhaseInstanceCountDecl;
    CThreadGroupDeclaration m_ThreadGroupDecl;
    CTypedUAVDeclaration m_TypedUAVDecl;
    CStructuredUAVDeclaration m_StructuredUAVDecl;
    CRawUAVDeclaration m_RawUAVDecl;
    CStructuredTGSMDeclaration m_StructuredTGSMDecl;
    CRawSRVDeclaration m_RawSRVDecl;
    CStructuredSRVDeclaration m_StructuredSRVDecl;
    CRawTGSMDeclaration m_RawTGSMDecl;
    CSyncFlags m_SyncFlags;
  };
};

// ****************************************************************************
//
// class CShaderAsm
//
// The class is used to build a binary representation of a shader.
// Usage scenario:
//      1. Call Init with the initial internal buffer size in UINTs. The
//         internal buffer will grow if needed
//      2. Call StartShader()
//      3. Call Emit*() functions to assemble a shader
//      4. Call EndShader()
//      5. Call GetShader() to get the binary representation
//
//
// ****************************************************************************
class CShaderAsm {
public:
  CShaderAsm()
      : m_dwFunc(NULL), m_Index(0), m_StartOpIndex(0), m_BufferSize(0) {
    Init(1024);
  };
  ~CShaderAsm() { free(m_dwFunc); };
  // Initializes the object with the initial buffer size in UINTs
  HRESULT Init(UINT BufferSize) {
    if (BufferSize >= UINT(-1) / sizeof(UINT)) {
      return E_OUTOFMEMORY;
    }
    m_dwFunc = (UINT *)malloc(BufferSize * sizeof(UINT));
    if (m_dwFunc == NULL) {
      return E_OUTOFMEMORY;
    }
    m_BufferSize = BufferSize;
    Reset();
    return S_OK;
  }
  UINT *GetShader() { return m_dwFunc; }
  UINT ShaderSizeInDWORDs() { return m_Index; }
  UINT ShaderSizeInBytes() { return ShaderSizeInDWORDs() * sizeof(*m_dwFunc); }
  UINT LastInstOffsetInDWORDs() { return m_StartOpIndex; }
  UINT LastInstOffsetInBytes() {
    return LastInstOffsetInDWORDs() * sizeof(*m_dwFunc);
  }

  // This function should be called to mark the start of a shader
  void StartShader(D3D10_SB_TOKENIZED_PROGRAM_TYPE ShaderType, UINT vermajor,
                   UINT verminor) {
    Reset();
    UINT Token = ENCODE_D3D10_SB_TOKENIZED_PROGRAM_VERSION_TOKEN(
        ShaderType, vermajor, verminor);
    OPCODE(Token);
    OPCODE(0); // Reserve space for length
  }
  // Should be called at the end of the shader
  void EndShader() {
    if (1 < m_BufferSize)
      m_dwFunc[1] = ENCODE_D3D10_SB_TOKENIZED_PROGRAM_LENGTH(m_Index);
  }
  // Emit a resource declaration
  void EmitResourceDecl(D3D10_SB_RESOURCE_DIMENSION Dimension, UINT TRegIndex,
                        D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForX,
                        D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForY,
                        D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForZ,
                        D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForW) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_RESOURCE) |
           ENCODE_D3D10_SB_RESOURCE_DIMENSION(Dimension));
    EmitOperand(COperand(D3D10_SB_OPERAND_TYPE_RESOURCE, TRegIndex));
    FUNC(ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForX, 0) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForY, 1) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForZ, 2) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForW, 3));
    ENDINSTRUCTION();
  }
  // Emit D3D12 resource declaration
  void EmitIndexableResourceDecl(UINT uTable, UINT uTableLB, UINT uTableUB,
                                 D3D10_SB_RESOURCE_DIMENSION Dimension,
                                 D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForX,
                                 D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForY,
                                 D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForZ,
                                 D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForW,
                                 UINT uSpace) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_RESOURCE) |
           ENCODE_D3D10_SB_RESOURCE_DIMENSION(Dimension));
    EmitOperand(
        COperand3D(D3D10_SB_OPERAND_TYPE_RESOURCE, uTable, uTableLB, uTableUB));
    FUNC(ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForX, 0) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForY, 1) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForZ, 2) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForW, 3));
    FUNC(uSpace);
    ENDINSTRUCTION();
  }
  // Emit a resource declaration (multisampled)
  void EmitResourceMSDecl(D3D10_SB_RESOURCE_DIMENSION Dimension, UINT TRegIndex,
                          D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForX,
                          D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForY,
                          D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForZ,
                          D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForW,
                          UINT SampleCount) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_RESOURCE) |
           ENCODE_D3D10_SB_RESOURCE_DIMENSION(Dimension) |
           ENCODE_D3D10_SB_RESOURCE_SAMPLE_COUNT(SampleCount));
    EmitOperand(COperand(D3D10_SB_OPERAND_TYPE_RESOURCE, TRegIndex));
    FUNC(ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForX, 0) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForY, 1) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForZ, 2) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForW, 3));
    ENDINSTRUCTION();
  }
  // Emit D3D12 resource declaration (multisampled)
  void EmitIndexableResourceMSDecl(UINT uTable, UINT uTableLB, UINT uTableUB,
                                   D3D10_SB_RESOURCE_DIMENSION Dimension,
                                   D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForX,
                                   D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForY,
                                   D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForZ,
                                   D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForW,
                                   UINT SampleCount, UINT uSpace) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_RESOURCE) |
           ENCODE_D3D10_SB_RESOURCE_DIMENSION(Dimension) |
           ENCODE_D3D10_SB_RESOURCE_SAMPLE_COUNT(SampleCount));
    EmitOperand(
        COperand3D(D3D10_SB_OPERAND_TYPE_RESOURCE, uTable, uTableLB, uTableUB));
    FUNC(ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForX, 0) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForY, 1) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForZ, 2) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForW, 3));
    FUNC(uSpace);
    ENDINSTRUCTION();
  }
  // Emit a sampler declaration
  void EmitSamplerDecl(UINT SRegIndex, D3D10_SB_SAMPLER_MODE Mode) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_SAMPLER) |
           ENCODE_D3D10_SB_SAMPLER_MODE(Mode));
    EmitOperand(COperand(D3D10_SB_OPERAND_TYPE_SAMPLER, SRegIndex));
    ENDINSTRUCTION();
  }
  // Emit D3D12 sampler declaration
  void EmitIndexableSamplerDecl(UINT uTable, UINT uTableLB, UINT uTableUB,
                                D3D10_SB_SAMPLER_MODE Mode, UINT uSpace) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_SAMPLER) |
           ENCODE_D3D10_SB_SAMPLER_MODE(Mode));
    EmitOperand(
        COperand3D(D3D10_SB_OPERAND_TYPE_SAMPLER, uTable, uTableLB, uTableUB));
    FUNC(uSpace);
    ENDINSTRUCTION();
  }

  // Emit a stream declaration
  void EmitStreamDecl(UINT SRegIndex) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D11_SB_OPCODE_DCL_STREAM));
    EmitOperand(COperand(D3D11_SB_OPERAND_TYPE_STREAM, SRegIndex));
    ENDINSTRUCTION();
  }

  // Emit an input declaration
  void EmitInputDecl(D3D10_SB_OPERAND_TYPE RegType, UINT RegIndex,
                     UINT WriteMask,
                     D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                         D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
    EmitOperand(COperandDst(RegType, RegIndex, WriteMask, MinPrecision));
    ENDINSTRUCTION();
  }
  void EmitInputDecl2D(D3D10_SB_OPERAND_TYPE RegType, UINT RegIndex,
                       UINT RegIndex2, UINT WriteMask,
                       D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                           D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
    EmitOperand(
        COperandDst(RegType, RegIndex, RegIndex2, WriteMask, MinPrecision));
    ENDINSTRUCTION();
  }

  // Emit an input declaration for a system interpreted value
  void EmitInputSystemInterpretedValueDecl(
      D3D10_SB_OPERAND_TYPE RegType, UINT RegIndex, UINT WriteMask,
      D3D10_SB_NAME Name,
      D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
          D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT_SIV));
    EmitOperand(COperandDst(RegType, RegIndex, WriteMask, MinPrecision));
    FUNC(ENCODE_D3D10_SB_NAME(Name));
    ENDINSTRUCTION();
  }
  void EmitInputSystemInterpretedValueDecl2D(
      UINT RegIndex, UINT RegIndex2, UINT WriteMask, D3D10_SB_NAME Name,
      D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
          D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT_SIV));
    EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, RegIndex2,
                            WriteMask, MinPrecision));
    FUNC(ENCODE_D3D10_SB_NAME(Name));
    ENDINSTRUCTION();
  }
  // Emit an input declaration for a system generated value
  void EmitInputSystemGeneratedValueDecl(
      UINT RegIndex, UINT WriteMask, D3D10_SB_NAME Name,
      D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
          D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT_SGV));
    EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, WriteMask,
                            MinPrecision));
    FUNC(ENCODE_D3D10_SB_NAME(Name));
    ENDINSTRUCTION();
  }
  void EmitInputSystemGeneratedValueDecl2D(
      UINT RegIndex, UINT RegIndex2, UINT WriteMask, D3D10_SB_NAME Name,
      D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
          D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT_SGV));
    EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, RegIndex2,
                            WriteMask, MinPrecision));
    FUNC(ENCODE_D3D10_SB_NAME(Name));
    ENDINSTRUCTION();
  }
  // Emit a PS input declaration
  void EmitPSInputDecl(UINT RegIndex, UINT WriteMask,
                       D3D10_SB_INTERPOLATION_MODE Mode,
                       D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                           D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT_PS) |
           ENCODE_D3D10_SB_INPUT_INTERPOLATION_MODE(Mode));
    EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, WriteMask,
                            MinPrecision));
    ENDINSTRUCTION();
  }
  // Emit a PS input declaration for a system interpreted value
  void EmitPSInputSystemInterpretedValueDecl(
      UINT RegIndex, UINT WriteMask, D3D10_SB_INTERPOLATION_MODE Mode,
      D3D10_SB_NAME Name,
      D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
          D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT_PS_SIV) |
           ENCODE_D3D10_SB_INPUT_INTERPOLATION_MODE(Mode));
    EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, WriteMask,
                            MinPrecision));
    FUNC(ENCODE_D3D10_SB_NAME(Name));
    ENDINSTRUCTION();
  }
  // Emit a PS input declaration for a system generated value
  void EmitPSInputSystemGeneratedValueDecl(
      UINT RegIndex, UINT WriteMask, D3D10_SB_INTERPOLATION_MODE Mode,
      D3D10_SB_NAME Name,
      D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
          D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT_PS_SGV) |
           ENCODE_D3D10_SB_INPUT_INTERPOLATION_MODE(Mode));
    EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, WriteMask,
                            MinPrecision));
    FUNC(ENCODE_D3D10_SB_NAME(Name));
    ENDINSTRUCTION();
  }
  // Emit input coverage mask declaration
  void EmitInputCoverageMaskDecl(D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                                     D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
    EmitOperand(
        COperand(D3D11_SB_OPERAND_TYPE_INPUT_COVERAGE_MASK, MinPrecision));
    ENDINSTRUCTION();
  }
  // Emit inner coverage declaration
  void EmitInnerCoverageDecl(D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                                 D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
    EmitOperand(COperand(D3D11_SB_OPERAND_TYPE_INNER_COVERAGE, MinPrecision));
    ENDINSTRUCTION();
  }
  // Emit cycle counter decl
  void EmitCycleCounterDecl(UINT WriteMask) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
    EmitOperand(COperandDst(WriteMask, D3D11_SB_OPERAND_TYPE_CYCLE_COUNTER));
    ENDINSTRUCTION();
  }

  // Emit input primitive id declaration
  void EmitInputPrimIdDecl(D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                               D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
    EmitOperand(
        COperandDst(D3D10_SB_OPERAND_TYPE_INPUT_PRIMITIVEID, MinPrecision));
    ENDINSTRUCTION();
  }
  // Emit input domain point declaration
  void EmitInputDomainPointDecl(UINT WriteMask,
                                D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                                    D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
    EmitOperand(COperandDst(WriteMask, D3D11_SB_OPERAND_TYPE_INPUT_DOMAIN_POINT,
                            MinPrecision));
    ENDINSTRUCTION();
  }
  // Emit and oDepth declaration
  void EmitODepthDecl(D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                          D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_OUTPUT));
    EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_OUTPUT_DEPTH, MinPrecision));
    ENDINSTRUCTION();
  }
  // Emit and oDepthGE declaration
  void EmitODepthDeclGE(D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                            D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_OUTPUT));
    EmitOperand(COperandDst(D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_GREATER_EQUAL,
                            MinPrecision));
    ENDINSTRUCTION();
  }
  // Emit and oDepthLE declaration
  void EmitODepthDeclLE(D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                            D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_OUTPUT));
    EmitOperand(COperandDst(D3D11_SB_OPERAND_TYPE_OUTPUT_DEPTH_LESS_EQUAL,
                            MinPrecision));
    ENDINSTRUCTION();
  }
  // Emit an oMask declaration
  void EmitOMaskDecl(D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                         D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_OUTPUT));
    EmitOperand(
        COperandDst(D3D10_SB_OPERAND_TYPE_OUTPUT_COVERAGE_MASK, MinPrecision));
    ENDINSTRUCTION();
  }
  // Emit an oStencilRef declaration
  void EmitOStencilRefDecl(D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                               D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_OUTPUT));
    EmitOperand(
        COperandDst(D3D11_SB_OPERAND_TYPE_OUTPUT_STENCIL_REF, MinPrecision));
    ENDINSTRUCTION();
  }
  // Emit an output declaration
  void EmitOutputDecl(UINT RegIndex, UINT WriteMask,
                      D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                          D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_OUTPUT));
    EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_OUTPUT, RegIndex, WriteMask,
                            MinPrecision));
    ENDINSTRUCTION();
  }
  // Emit an output declaration for a system interpreted value
  void EmitOutputSystemInterpretedValueDecl(
      UINT RegIndex, UINT WriteMask, D3D10_SB_NAME Name,
      D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
          D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_OUTPUT_SIV));
    EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_OUTPUT, RegIndex, WriteMask,
                            MinPrecision));
    FUNC(ENCODE_D3D10_SB_NAME(Name));
    ENDINSTRUCTION();
  }
  // Emit an output declaration for a system generated value
  void EmitOutputSystemGeneratedValueDecl(
      UINT RegIndex, UINT WriteMask, D3D10_SB_NAME Name,
      D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
          D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_OUTPUT_SGV));
    EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_OUTPUT, RegIndex, WriteMask,
                            MinPrecision));
    FUNC(ENCODE_D3D10_SB_NAME(Name));
    ENDINSTRUCTION();
  }

  // Emit an input register indexing range declaration
  void EmitInputIndexingRangeDecl(UINT RegIndex, UINT Count, UINT WriteMask) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INDEX_RANGE));
    EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, WriteMask));
    FUNC((UINT)Count);
    ENDINSTRUCTION();
  }

  // 2D indexing range decl (indexing is for second dimension)
  void EmitInputIndexingRangeDecl2D(UINT RegIndex, UINT RegIndex2Min,
                                    UINT Reg2Count, UINT WriteMask) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INDEX_RANGE));
    EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_INPUT, RegIndex, RegIndex2Min,
                            WriteMask));
    FUNC((UINT)Reg2Count);
    ENDINSTRUCTION();
  }

  // Emit an output register indexing range declaration
  void EmitOutputIndexingRangeDecl(UINT RegIndex, UINT Count, UINT WriteMask) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INDEX_RANGE));
    EmitOperand(COperandDst(D3D10_SB_OPERAND_TYPE_OUTPUT, RegIndex, WriteMask));
    FUNC((UINT)Count);
    ENDINSTRUCTION();
  }

  // Emit indexing range decl taking reg type as parameter
  // (for things other than plain input or output regs)
  void EmitIndexingRangeDecl(D3D10_SB_OPERAND_TYPE RegType, UINT RegIndex,
                             UINT Count, UINT WriteMask) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INDEX_RANGE));
    EmitOperand(COperandDst(RegType, RegIndex, WriteMask));
    FUNC((UINT)Count);
    ENDINSTRUCTION();
  }

  // 2D indexing range decl (indexing is for second dimension)
  // Emit indexing range decl taking reg type as parameter
  // (for things other than plain input or output regs)
  void EmitIndexingRangeDecl2D(D3D10_SB_OPERAND_TYPE RegType, UINT RegIndex,
                               UINT RegIndex2Min, UINT Reg2Count,
                               UINT WriteMask) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INDEX_RANGE));
    EmitOperand(COperandDst(RegType, RegIndex, RegIndex2Min, WriteMask));
    FUNC((UINT)Reg2Count);
    ENDINSTRUCTION();
  }

  // Emit a temp registers ( r0...r(n-1) ) declaration
  void EmitTempsDecl(UINT NumTemps) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_TEMPS));
    FUNC((UINT)NumTemps);
    ENDINSTRUCTION();
  }

  // Emit an indexable temp register (x#) declaration
  void EmitIndexableTempDecl(UINT TempNumber, UINT RegCount,
                             UINT ComponentCount) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INDEXABLE_TEMP));
    FUNC((UINT)TempNumber);
    FUNC((UINT)RegCount);
    FUNC((UINT)ComponentCount);
    ENDINSTRUCTION();
  }

  // Emit a constant buffer (cb#) declaration
  void EmitConstantBufferDecl(
      UINT RegIndex, UINT Size, // size 0 means unknown/any size
      D3D10_SB_CONSTANT_BUFFER_ACCESS_PATTERN AccessPattern) {
    m_bExecutableInstruction = FALSE;
    OPCODE(
        ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER) |
        ENCODE_D3D10_SB_D3D10_SB_CONSTANT_BUFFER_ACCESS_PATTERN(AccessPattern));
    EmitOperand(
        COperand2D(D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER, RegIndex, Size));
    ENDINSTRUCTION();
  }

  // Emit D3D12 constant buffer (cb#) declaration.
  void EmitIndexableConstantBufferDecl(
      UINT uCBufferVarIndex, UINT uLB, UINT uUB,
      UINT Size, // size 0 means unknown/any size
      D3D10_SB_CONSTANT_BUFFER_ACCESS_PATTERN AccessPattern, UINT uSpace) {
    m_bExecutableInstruction = FALSE;
    OPCODE(
        ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_CONSTANT_BUFFER) |
        ENCODE_D3D10_SB_D3D10_SB_CONSTANT_BUFFER_ACCESS_PATTERN(AccessPattern));
    EmitOperand(COperand3D(D3D10_SB_OPERAND_TYPE_CONSTANT_BUFFER,
                           uCBufferVarIndex, uLB, uUB));
    FUNC(Size);
    FUNC(uSpace);
    ENDINSTRUCTION();
  }

  // Emit Immediate Constant Buffer (icb) declaration
  void
  EmitImmediateConstantBufferDecl(UINT Num4Tuples,
                                  const UINT *pImmediateConstantBufferData) {
    m_bExecutableInstruction = FALSE;
    EmitCustomData(D3D10_SB_CUSTOMDATA_DCL_IMMEDIATE_CONSTANT_BUFFER,
                   4 * Num4Tuples /*2 UINTS will be added during encoding */,
                   pImmediateConstantBufferData);
  }

  // Emit a GS input primitive declaration
  void EmitGSInputPrimitiveDecl(D3D10_SB_PRIMITIVE Primitive) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_GS_INPUT_PRIMITIVE) |
           ENCODE_D3D10_SB_GS_INPUT_PRIMITIVE(Primitive));
    ENDINSTRUCTION();
  }

  // Emit a GS output topology declaration
  void EmitGSOutputTopologyDecl(D3D10_SB_PRIMITIVE_TOPOLOGY Topology) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(
               D3D10_SB_OPCODE_DCL_GS_OUTPUT_PRIMITIVE_TOPOLOGY) |
           ENCODE_D3D10_SB_GS_OUTPUT_PRIMITIVE_TOPOLOGY(Topology));
    ENDINSTRUCTION();
  }

  // Emit GS Maximum Output Vertex Count declaration
  void EmitGSMaxOutputVertexCountDecl(UINT Count) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(
        D3D10_SB_OPCODE_DCL_MAX_OUTPUT_VERTEX_COUNT));
    FUNC((UINT)Count);
    ENDINSTRUCTION();
  }
  // Emit input GS instance count declaration
  void EmitInputGSInstanceCountDecl(UINT Instances) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D11_SB_OPCODE_DCL_GS_INSTANCE_COUNT));
    FUNC(Instances);
    ENDINSTRUCTION();
  }
  // Emit input GS instance ID declaration
  void EmitInputGSInstanceIDDecl(D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                                     D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
    EmitOperand(
        COperandDst(D3D11_SB_OPERAND_TYPE_INPUT_GS_INSTANCE_ID, MinPrecision));
    ENDINSTRUCTION();
  }

  // Emit global flags declaration
  void EmitGlobalFlagsDecl(UINT Flags) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_GLOBAL_FLAGS) |
           ENCODE_D3D10_SB_GLOBAL_FLAGS(Flags));
    ENDINSTRUCTION();
  }
  // Emit interface function body declaration
  void EmitFunctionBodyDecl(UINT uFunctionID) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D11_SB_OPCODE_DCL_FUNCTION_BODY));
    FUNC(uFunctionID);
    ENDINSTRUCTION();
  }

  void EmitFunctionTableDecl(UINT uFunctionTableID, UINT uTableSize,
                             UINT *pTableEntries) {
    m_bExecutableInstruction = FALSE;
    bool bExtended = (3 + uTableSize) > MAX_INSTRUCTION_LENGTH;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D11_SB_OPCODE_DCL_FUNCTION_TABLE) |
           ENCODE_D3D10_SB_OPCODE_EXTENDED(bExtended));

    if (bExtended)
      FUNC(0);

    FUNC(uFunctionTableID);
    FUNC(uTableSize);

    if (m_Index + uTableSize >= m_BufferSize) {
      Reserve(uTableSize);
    }

    memcpy(&m_dwFunc[m_Index], pTableEntries, sizeof(UINT) * uTableSize);
    m_Index += uTableSize;

    ENDLONGINSTRUCTION(bExtended);
  }

  void EmitInterfaceDecl(UINT uInterfaceID, bool bDynamicIndexed,
                         UINT uArrayLength, UINT uExpectedTableSize,
                         __in_range(0, D3D11_SB_MAX_NUM_TYPES) UINT uNumTypes,
                         __in_ecount(uNumTypes) UINT *pTableEntries) {
    m_bExecutableInstruction = FALSE;
    bool bExtended = (4 + uNumTypes) > MAX_INSTRUCTION_LENGTH;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D11_SB_OPCODE_DCL_INTERFACE) |
           ENCODE_D3D11_SB_INTERFACE_INDEXED_BIT(bDynamicIndexed) |
           ENCODE_D3D10_SB_OPCODE_EXTENDED(bExtended));

    if (bExtended)
      FUNC(0);

    FUNC(uInterfaceID);
    FUNC(uExpectedTableSize);
    FUNC(ENCODE_D3D11_SB_INTERFACE_TABLE_LENGTH(uNumTypes) |
         ENCODE_D3D11_SB_INTERFACE_ARRAY_LENGTH(uArrayLength));

    if (m_Index + uNumTypes >= m_BufferSize) {
      Reserve(uNumTypes);
    }

    memcpy(&m_dwFunc[m_Index], pTableEntries, sizeof(UINT) * uNumTypes);
    m_Index += uNumTypes;

    ENDLONGINSTRUCTION(bExtended);
  }
  void EmitInterfaceCall(COperandBase &InterfaceOperand, UINT uFunctionIndex) {
    m_bExecutableInstruction = TRUE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D11_SB_OPCODE_INTERFACE_CALL));
    FUNC(uFunctionIndex);
    EmitOperand(InterfaceOperand);
    ENDINSTRUCTION();
  }

  void EmitInputControlPointCountDecl(UINT Count) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(
               D3D11_SB_OPCODE_DCL_INPUT_CONTROL_POINT_COUNT) |
           ENCODE_D3D11_SB_INPUT_CONTROL_POINT_COUNT(Count));
    ENDINSTRUCTION();
  }

  void EmitOutputControlPointCountDecl(UINT Count) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(
               D3D11_SB_OPCODE_DCL_OUTPUT_CONTROL_POINT_COUNT) |
           ENCODE_D3D11_SB_OUTPUT_CONTROL_POINT_COUNT(Count));
    ENDINSTRUCTION();
  }

  void EmitTessellatorDomainDecl(D3D11_SB_TESSELLATOR_DOMAIN Domain) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D11_SB_OPCODE_DCL_TESS_DOMAIN) |
           ENCODE_D3D11_SB_TESS_DOMAIN(Domain));
    ENDINSTRUCTION();
  }

  void EmitTessellatorPartitioningDecl(
      D3D11_SB_TESSELLATOR_PARTITIONING Partitioning) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D11_SB_OPCODE_DCL_TESS_PARTITIONING) |
           ENCODE_D3D11_SB_TESS_PARTITIONING(Partitioning));
    ENDINSTRUCTION();
  }

  void EmitTessellatorOutputPrimitiveDecl(
      D3D11_SB_TESSELLATOR_OUTPUT_PRIMITIVE OutputPrimitive) {
    m_bExecutableInstruction = FALSE;
    OPCODE(
        ENCODE_D3D10_SB_OPCODE_TYPE(D3D11_SB_OPCODE_DCL_TESS_OUTPUT_PRIMITIVE) |
        ENCODE_D3D11_SB_TESS_OUTPUT_PRIMITIVE(OutputPrimitive));
    ENDINSTRUCTION();
  }

  void EmitHSMaxTessFactorDecl(float MaxTessFactor) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D11_SB_OPCODE_DCL_HS_MAX_TESSFACTOR));
    UINT uTemp = *(UINT *)&MaxTessFactor;
    FUNC(uTemp);
    ENDINSTRUCTION();
  }

  void EmitHSForkPhaseInstanceCountDecl(UINT InstanceCount) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(
        D3D11_SB_OPCODE_DCL_HS_FORK_PHASE_INSTANCE_COUNT));
    FUNC(InstanceCount);
    ENDINSTRUCTION();
  }

  void EmitHSJoinPhaseInstanceCountDecl(UINT InstanceCount) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(
        D3D11_SB_OPCODE_DCL_HS_JOIN_PHASE_INSTANCE_COUNT));
    FUNC(InstanceCount);
    ENDINSTRUCTION();
  }

  void EmitHSBeginPhase(D3D10_SB_OPCODE_TYPE Phase) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(Phase));
    ENDINSTRUCTION();
  }

  void EmitInputOutputControlPointIDDecl(
      D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
          D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
    EmitOperand(COperandDst(D3D11_SB_OPERAND_TYPE_OUTPUT_CONTROL_POINT_ID,
                            MinPrecision));
    ENDINSTRUCTION();
  }

  void EmitInputForkInstanceIDDecl(D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                                       D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
    EmitOperand(COperandDst(D3D11_SB_OPERAND_TYPE_INPUT_FORK_INSTANCE_ID,
                            MinPrecision));
    ENDINSTRUCTION();
  }

  void EmitInputJoinInstanceIDDecl(D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                                       D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
    EmitOperand(COperandDst(D3D11_SB_OPERAND_TYPE_INPUT_JOIN_INSTANCE_ID,
                            MinPrecision));
    ENDINSTRUCTION();
  }

  void EmitThreadGroupDecl(UINT x, UINT y, UINT z) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D11_SB_OPCODE_DCL_THREAD_GROUP));
    FUNC(x);
    FUNC(y);
    FUNC(z);
    ENDINSTRUCTION();
  }

  void EmitInputThreadIDDecl(UINT WriteMask,
                             D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                                 D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
    EmitOperand(COperandDst(WriteMask, D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID,
                            MinPrecision));
    ENDINSTRUCTION();
  }

  void EmitInputThreadGroupIDDecl(UINT WriteMask,
                                  D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                                      D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
    EmitOperand(COperandDst(
        WriteMask, D3D11_SB_OPERAND_TYPE_INPUT_THREAD_GROUP_ID, MinPrecision));
    ENDINSTRUCTION();
  }

  void
  EmitInputThreadIDInGroupDecl(UINT WriteMask,
                               D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
                                   D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
    EmitOperand(COperandDst(WriteMask,
                            D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP,
                            MinPrecision));
    ENDINSTRUCTION();
  }

  void EmitInputThreadIDInGroupFlattenedDecl(
      D3D11_SB_OPERAND_MIN_PRECISION MinPrecision =
          D3D11_SB_OPERAND_MIN_PRECISION_DEFAULT) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D10_SB_OPCODE_DCL_INPUT));
    EmitOperand(
        COperandDst(D3D11_SB_OPERAND_TYPE_INPUT_THREAD_ID_IN_GROUP_FLATTENED,
                    MinPrecision));
    ENDINSTRUCTION();
  }

  void EmitTypedUnorderedAccessViewDecl(
      D3D10_SB_RESOURCE_DIMENSION Dimension, UINT URegIndex,
      D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForX,
      D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForY,
      D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForZ,
      D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForW, UINT Flags) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(
               D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_TYPED) |
           ENCODE_D3D10_SB_RESOURCE_DIMENSION(Dimension) |
           ENCODE_D3D11_SB_RESOURCE_FLAGS(Flags));
    EmitOperand(
        COperand(D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW, URegIndex));
    FUNC(ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForX, 0) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForY, 1) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForZ, 2) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForW, 3));
    ENDINSTRUCTION();
  }
  // Emit D3D12 UAV declaration.
  void EmitIndexableTypedUnorderedAccessViewDecl(
      UINT uTable, UINT uTableLB, UINT uTableUB,
      D3D10_SB_RESOURCE_DIMENSION Dimension,
      D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForX,
      D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForY,
      D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForZ,
      D3D10_SB_RESOURCE_RETURN_TYPE ReturnTypeForW, UINT Flags, UINT uSpace) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(
               D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_TYPED) |
           ENCODE_D3D10_SB_RESOURCE_DIMENSION(Dimension) |
           ENCODE_D3D11_SB_RESOURCE_FLAGS(Flags));
    EmitOperand(COperand3D(D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW, uTable,
                           uTableLB, uTableUB));
    FUNC(ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForX, 0) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForY, 1) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForZ, 2) |
         ENCODE_D3D10_SB_RESOURCE_RETURN_TYPE(ReturnTypeForW, 3));
    FUNC(uSpace);
    ENDINSTRUCTION();
  }

  void EmitRawUnorderedAccessViewDecl(UINT URegIndex, UINT Flags) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(
               D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_RAW) |
           ENCODE_D3D11_SB_RESOURCE_FLAGS(Flags));
    EmitOperand(
        COperand(D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW, URegIndex));
    ENDINSTRUCTION();
  }
  // Emit D3D12 raw UAV declaration.
  void EmitIndexableRawUnorderedAccessViewDecl(UINT uTable, UINT uTableLB,
                                               UINT uTableUB, UINT Flags,
                                               UINT uSpace) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(
               D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_RAW) |
           ENCODE_D3D11_SB_RESOURCE_FLAGS(Flags));
    EmitOperand(COperand3D(D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW, uTable,
                           uTableLB, uTableUB));
    FUNC(uSpace);
    ENDINSTRUCTION();
  }

  void EmitStructuredUnorderedAccessViewDecl(UINT URegIndex, UINT ByteStride,
                                             UINT Flags) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(
               D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED) |
           ENCODE_D3D11_SB_RESOURCE_FLAGS(Flags));
    EmitOperand(
        COperand(D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW, URegIndex));
    FUNC(ByteStride);
    ENDINSTRUCTION();
  }
  // Emit D3D12 structured UAV declaration.
  void EmitIndexableStructuredUnorderedAccessViewDecl(UINT uTable,
                                                      UINT uTableLB,
                                                      UINT uTableUB,
                                                      UINT ByteStride,
                                                      UINT Flags, UINT uSpace) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(
               D3D11_SB_OPCODE_DCL_UNORDERED_ACCESS_VIEW_STRUCTURED) |
           ENCODE_D3D11_SB_RESOURCE_FLAGS(Flags));
    EmitOperand(COperand3D(D3D11_SB_OPERAND_TYPE_UNORDERED_ACCESS_VIEW, uTable,
                           uTableLB, uTableUB));
    FUNC(ByteStride);
    FUNC(uSpace);
    ENDINSTRUCTION();
  }

  void EmitRawThreadGroupSharedMemoryDecl(UINT GRegIndex, UINT ByteCount) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(
        D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_RAW));
    EmitOperand(
        COperand(D3D11_SB_OPERAND_TYPE_THREAD_GROUP_SHARED_MEMORY, GRegIndex));
    FUNC(ByteCount);
    ENDINSTRUCTION();
  }

  void EmitStructuredThreadGroupSharedMemoryDecl(UINT GRegIndex,
                                                 UINT ByteStride,
                                                 UINT StructCount) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(
        D3D11_SB_OPCODE_DCL_THREAD_GROUP_SHARED_MEMORY_STRUCTURED));
    EmitOperand(
        COperand(D3D11_SB_OPERAND_TYPE_THREAD_GROUP_SHARED_MEMORY, GRegIndex));
    FUNC(ByteStride);
    FUNC(StructCount);
    ENDINSTRUCTION();
  }

  void EmitRawShaderResourceViewDecl(UINT TRegIndex) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D11_SB_OPCODE_DCL_RESOURCE_RAW));
    EmitOperand(COperand(D3D10_SB_OPERAND_TYPE_RESOURCE, TRegIndex));
    ENDINSTRUCTION();
  }

  // Emit D3D12 byte address buffer declaration
  void EmitIndexableRawShaderResourceViewDecl(UINT uTable, UINT uTableLB,
                                              UINT uTableUB, UINT uSpace) {
    m_bExecutableInstruction = FALSE;
    OPCODE(ENCODE_D3D10_SB_OPCODE_TYPE(D3D11_SB_OPCODE_DCL_RESOURCE_RAW));
    EmitOperand(
        COperand3D(D3D10_SB_OPERAND_TYPE_RESOURCE, uTable, uTableLB, uTableUB));
    FUNC(uSpace);
    ENDINSTRUCTION();
  }

  void EmitStructuredShaderResourceViewDecl(UINT TRegIndex, UINT ByteStride) {
    m_bExecutableInstruction = FALSE;
    OPCODE(
        ENCODE_D3D10_SB_OPCODE_TYPE(D3D11_SB_OPCODE_DCL_RESOURCE_STRUCTURED));
    EmitOperand(COperand(D3D10_SB_OPERAND_TYPE_RESOURCE, TRegIndex));
    FUNC(ByteStride);
    ENDINSTRUCTION();
  }

  // Emit D3D12 structured buffer declaration
  void EmitIndexableStructuredShaderResourceViewDecl(UINT uTable, UINT uTableLB,
                                                     UINT uTableUB,
                                                     UINT ByteStride,
                                                     UINT uSpace) {
    m_bExecutableInstruction = FALSE;
    OPCODE(
        ENCODE_D3D10_SB_OPCODE_TYPE(D3D11_SB_OPCODE_DCL_RESOURCE_STRUCTURED));
    EmitOperand(
        COperand3D(D3D10_SB_OPERAND_TYPE_RESOURCE, uTable, uTableLB, uTableUB));
    FUNC(ByteStride);
    FUNC(uSpace);
    ENDINSTRUCTION();
  }

  // Emit an instruction. Custom-data is not handled by this function.
  void EmitInstruction(const CInstruction &instruction);
  // Emit an operand
  void EmitOperand(const COperandBase &operand);
  // Emit an instruction without operands
  void Emit(UINT OpCode) {
    OPCODE(OpCode);
    ENDINSTRUCTION();
  }
  void StartComplexEmit(UINT OpCode,
                        UINT ReserveCount = MAX_INSTRUCTION_LENGTH) {
    OPCODE(OpCode);
    Reserve(ReserveCount);
  }
  void AddComplexEmit(UINT Data) { FUNC(Data); }
  void EndComplexEmit(bool bPatchLength = false) {
    ENDLONGINSTRUCTION(bPatchLength, !bPatchLength);
  }
  UINT GetComplexEmitPosition() { return m_Index; }
  void UpdateComplexEmitPosition(UINT Pos, UINT Data) {
    if (Pos < m_BufferSize) {
      m_dwFunc[Pos] = Data;
    }
  }
  void
  EmitCustomData(D3D10_SB_CUSTOMDATA_CLASS CustomDataClass,
                 UINT SizeInUINTs /*2 UINTS will be added during encoding */,
                 const UINT *pCustomData) {
    UINT FullSizeInUINTs = SizeInUINTs + 2; // include opcode and size
    if (FullSizeInUINTs < SizeInUINTs ||
        FullSizeInUINTs + m_Index < FullSizeInUINTs) // check for overflow
    {
      throw E_FAIL;
    }
    if (m_Index + FullSizeInUINTs >=
        m_BufferSize) // If custom data is going to overflow the buffer, reserve
                      // more memory
    {
      Reserve(FullSizeInUINTs);
    }
    assert(m_Index + FullSizeInUINTs <
           m_BufferSize); // Otherwise there's a bug in Reserve()
    m_dwFunc[m_Index++] = ENCODE_D3D10_SB_CUSTOMDATA_CLASS(CustomDataClass);
    m_dwFunc[m_Index++] = FullSizeInUINTs;
    memcpy(&m_dwFunc[m_Index], pCustomData, sizeof(UINT) * SizeInUINTs);
    m_Index += SizeInUINTs;
    if (m_Index >= m_BufferSize) // If custom data is exactly fully filled the
                                 // buffer, reserve more memory
    {
      Reserve(1024);
    }
  }
  // Returns number of executable instructions in the current shader
  UINT GetNumExecutableInstructions() { return m_NumExecutableInstructions; }

protected:
  void OPCODE(UINT x) {
    if (m_Index < m_BufferSize) {
      m_dwFunc[m_Index] = x;
      m_StartOpIndex = m_Index++;
    }
    if (m_Index >= m_BufferSize)
      Reserve(1024);
  }
  // Should be called after end of each instruction
  void ENDINSTRUCTION() {
    if (m_StartOpIndex < m_Index) {
      m_dwFunc[m_StartOpIndex] |= ENCODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(
          m_Index - m_StartOpIndex);
      Reserve(MAX_INSTRUCTION_LENGTH);
      m_StatementIndex++;
      if (m_bExecutableInstruction)
        m_NumExecutableInstructions++;
      m_bExecutableInstruction = true;
    }
  }
  void ENDLONGINSTRUCTION(bool bExtendedLength, bool bBaseLength = true) {
    if (m_StartOpIndex < m_Index) {
      if (bBaseLength) {
        m_dwFunc[m_StartOpIndex] |=
            ENCODE_D3D10_SB_TOKENIZED_INSTRUCTION_LENGTH(m_Index -
                                                         m_StartOpIndex);
      }
      if (bExtendedLength) {
        assert(m_StartOpIndex + 1 < m_Index);
        m_dwFunc[m_StartOpIndex + 1] = m_Index - m_StartOpIndex;
      }
      Reserve(MAX_INSTRUCTION_LENGTH);
      m_StatementIndex++;
      if (m_bExecutableInstruction)
        m_NumExecutableInstructions++;
      m_bExecutableInstruction = true;
    }
  }
  void FUNC(UINT x) {
    if (m_Index < m_BufferSize)
      m_dwFunc[m_Index++] = x;
    if (m_Index >= m_BufferSize)
      Reserve(1024);
  }
  // Prepare assembler for a new shader
  void Reset() {
    m_Index = 0;
    m_StartOpIndex = 0;
    m_StatementIndex = 1;
    m_NumExecutableInstructions = 0;
    m_bExecutableInstruction = TRUE;
  }
  // Reserve SizeInUINTs UINTs in the m_dwFunc array
  void Reserve(UINT SizeInUINTs) {
    // The following overflow check may be sligltly over-cautious when (m_Index
    // + SizeInUINTs < m_BufferSize), but this only matters when m_BufferSize
    // cannot be grown one more step without overflowing.
    UINT NewSize = m_BufferSize + SizeInUINTs + 1024;
    if (m_Index > m_BufferSize || // invalid state
        (m_BufferSize + SizeInUINTs) <
            m_BufferSize || // overflow with adding SizeInUINTs
        NewSize < (m_BufferSize + SizeInUINTs)) // overflow with adding 1024
    {
      throw E_FAIL;
    }
    if (m_BufferSize < (m_Index + SizeInUINTs)) {
      UINT *pNewBuffer = (UINT *)malloc(NewSize * sizeof(UINT));
      if (pNewBuffer == NULL) {
        throw E_OUTOFMEMORY;
      }
      memcpy(pNewBuffer, m_dwFunc, sizeof(UINT) * m_Index);
      free(m_dwFunc);
      m_dwFunc = pNewBuffer;
      m_BufferSize = NewSize;
    }
  }
  // Buffer where the binary representation is built
  __field_ecount_part(m_BufferSize, m_Index) UINT *m_dwFunc;
  // Index where to place the next token in the m_dwFunc array
  UINT m_Index;
  // Index of the start of the current instruction in the m_dwFunc array
  UINT m_StartOpIndex;
  // Current buffer size in UINTs
  UINT m_BufferSize;
  // Current statement index of the current vertex shader
  UINT m_StatementIndex;
  // Number of executable instructions in the shader
  UINT m_NumExecutableInstructions;
  // "true" when the current instruction is executable
  bool m_bExecutableInstruction;
};

//*****************************************************************************
//
//  CShaderCodeParser
//
//*****************************************************************************

class CShaderCodeParser {
public:
  CShaderCodeParser()
      : m_pCurrentToken(NULL), m_pShaderCode(NULL), m_pShaderEndToken(NULL) {
    InitInstructionInfo();
  }
  CShaderCodeParser(CONST CShaderToken *pBuffer)
      : m_pCurrentToken(NULL), m_pShaderCode(NULL), m_pShaderEndToken(NULL) {
    InitInstructionInfo();
    SetShader(pBuffer);
  }
  ~CShaderCodeParser() {}
  void SetShader(CONST CShaderToken *pBuffer);
  void ParseInstruction(CInstruction *pInstruction);
  void ParseIndex(COperandIndex *pOperandIndex,
                  D3D10_SB_OPERAND_INDEX_REPRESENTATION IndexType);
  void ParseOperand(COperandBase *pOperand);
  BOOL EndOfShader() { return m_pCurrentToken >= m_pShaderEndToken; }
  D3D10_SB_TOKENIZED_PROGRAM_TYPE ShaderType();
  UINT ShaderMinorVersion();
  UINT ShaderMajorVersion();
  UINT ShaderLengthInTokens();
  UINT CurrentTokenOffset();
  UINT CurrentTokenOffsetInBytes() {
    return CurrentTokenOffset() * sizeof(CShaderToken);
  }
  void SetCurrentTokenOffset(UINT Offset);

  CONST CShaderToken *ParseOperandAt(COperandBase *pOperand,
                                     CONST CShaderToken *pBuffer,
                                     CONST CShaderToken *pBufferEnd) {
    const CShaderToken *pCurTok = m_pCurrentToken;
    const CShaderToken *pEndTok = m_pShaderEndToken;
    const CShaderToken *pRet;

    m_pCurrentToken = (pBuffer);
    m_pShaderEndToken = (pBufferEnd);

    ParseOperand(pOperand);
    pRet = m_pCurrentToken;

    m_pCurrentToken = pCurTok;
    m_pShaderEndToken = pEndTok;

    return pRet;
  }

protected:
  const CShaderToken *m_pCurrentToken;
  const CShaderToken *m_pShaderCode;
  // Points to the last token of the current shader
  const CShaderToken *m_pShaderEndToken;
};

}; // namespace D3D10ShaderBinary
