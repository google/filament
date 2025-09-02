///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilOperations.h                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implementation of DXIL operation tables.                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace llvm {
class LLVMContext;
class Module;
class Type;
class StructType;
class PointerType;
class Function;
class Constant;
class Value;
class Instruction;
class CallInst;
} // namespace llvm
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Attributes.h"

#include "DxilConstants.h"
#include <unordered_map>

namespace hlsl {

/// Use this utility class to interact with DXIL operations.
class OP {
public:
  using OpCode = DXIL::OpCode;
  using OpCodeClass = DXIL::OpCodeClass;

public:
  OP() = delete;
  OP(llvm::LLVMContext &Ctx, llvm::Module *pModule);

  // InitWithMinPrecision sets the low-precision mode and calls
  // FixOverloadNames() and RefreshCache() to set up caches for any existing
  // DXIL operations and types used in the module.
  void InitWithMinPrecision(bool bMinPrecision);

  // FixOverloadNames fixes the names of DXIL operation overloads, particularly
  // when they depend on user defined type names. User defined type names can be
  // modified by name collisions from multiple modules being loaded into the
  // same llvm context, such as during module linking.
  void FixOverloadNames();

  // RefreshCache places DXIL types and operation overloads from the module into
  // caches.
  void RefreshCache();

  // The single llvm::Type * "OverloadType" has one of these forms:
  // No overloads (NumOverloadDims == 0):
  //  - TS_Void: VoidTy
  // For single overload dimension (NumOverloadDims == 1):
  //  - TS_F*, TS_I*: a scalar numeric type (half, float, i1, i64, etc.),
  //  - TS_UDT: a pointer to a StructType representing a User Defined Type,
  //  - TS_Object: a named StructType representing a built-in object, or
  //  - TS_Vector: a vector type (<4 x float>, <16 x i16>, etc.)
  // For multiple overload dimensions (TS_Extended, NumOverloadDims > 1):
  //  - an unnamed StructType containing each type for the corresponding
  //    dimension, such as: type { i32, <2 x float> }
  //  - contained type options are the same as for single dimension.

  llvm::Function *GetOpFunc(OpCode OpCode, llvm::Type *pOverloadType);

  // N-dimension convenience version of GetOpFunc:
  llvm::Function *GetOpFunc(OpCode OpCode,
                            llvm::ArrayRef<llvm::Type *> OverloadTypes);

  const llvm::SmallMapVector<llvm::Type *, llvm::Function *, 8> &
  GetOpFuncList(OpCode OpCode) const;
  bool IsDxilOpUsed(OpCode opcode) const;
  void RemoveFunction(llvm::Function *F);
  llvm::LLVMContext &GetCtx() { return m_Ctx; }
  llvm::Module *GetModule() { return m_pModule; }
  llvm::Type *GetHandleType() const;
  llvm::Type *GetHitObjectType() const;
  llvm::Type *GetNodeHandleType() const;
  llvm::Type *GetNodeRecordHandleType() const;
  llvm::Type *GetResourcePropertiesType() const;
  llvm::Type *GetNodePropertiesType() const;
  llvm::Type *GetNodeRecordPropertiesType() const;
  llvm::Type *GetResourceBindingType() const;
  llvm::Type *GetDimensionsType() const;
  llvm::Type *GetSamplePosType() const;
  llvm::Type *GetBinaryWithCarryType() const;
  llvm::Type *GetBinaryWithTwoOutputsType() const;
  llvm::Type *GetSplitDoubleType() const;
  llvm::Type *GetFourI32Type() const;
  llvm::Type *GetFourI16Type() const;

  llvm::Type *GetResRetType(llvm::Type *pOverloadType);
  llvm::Type *GetCBufferRetType(llvm::Type *pOverloadType);
  llvm::Type *GetStructVectorType(unsigned numElements,
                                  llvm::Type *pOverloadType);
  bool IsResRetType(llvm::Type *Ty);

  // Construct an unnamed struct type containing the set of member types.
  llvm::StructType *
  GetExtendedOverloadType(llvm::ArrayRef<llvm::Type *> OverloadTypes);

  // Try to get the opcode class for a function.
  // Return true and set `opClass` if the given function is a dxil function.
  // Return false if the given function is not a dxil function.
  bool GetOpCodeClass(const llvm::Function *F, OpCodeClass &opClass);

  // To check if operation uses strict precision types
  bool UseMinPrecision();

  // Get the size of the type for a given layout
  uint64_t GetAllocSizeForType(llvm::Type *Ty);

  // LLVM helpers. Perhaps, move to a separate utility class.
  llvm::Constant *GetI1Const(bool v);
  llvm::Constant *GetI8Const(char v);
  llvm::Constant *GetU8Const(unsigned char v);
  llvm::Constant *GetI16Const(int v);
  llvm::Constant *GetU16Const(unsigned v);
  llvm::Constant *GetI32Const(int v);
  llvm::Constant *GetU32Const(unsigned v);
  llvm::Constant *GetU64Const(unsigned long long v);
  llvm::Constant *GetFloatConst(float v);
  llvm::Constant *GetDoubleConst(double v);

  static OP::OpCode getOpCode(const llvm::Instruction *I);
  static llvm::Type *GetOverloadType(OpCode OpCode, llvm::Function *F);
  static OpCode GetDxilOpFuncCallInst(const llvm::Instruction *I);
  static const char *GetOpCodeName(OpCode OpCode);
  static const char *GetAtomicOpName(DXIL::AtomicBinOpCode OpCode);
  static OpCodeClass GetOpCodeClass(OpCode OpCode);
  static const char *GetOpCodeClassName(OpCode OpCode);
  static llvm::Attribute::AttrKind GetMemAccessAttr(OpCode opCode);
  static bool IsOverloadLegal(OpCode OpCode, llvm::Type *pType);
  static bool CheckOpCodeTable();
  static bool IsDxilOpFuncName(llvm::StringRef name);
  static bool IsDxilOpFunc(const llvm::Function *F);
  static bool IsDxilOpFuncCallInst(const llvm::Instruction *I);
  static bool IsDxilOpFuncCallInst(const llvm::Instruction *I, OpCode opcode);
  static bool IsDxilOpWave(OpCode C);
  static bool IsDxilOpGradient(OpCode C);
  static bool IsDxilOpFeedback(OpCode C);
  static bool IsDxilOpBarrier(OpCode C);
  static bool BarrierRequiresGroup(const llvm::CallInst *CI);
  static bool BarrierRequiresNode(const llvm::CallInst *CI);
  static bool BarrierRequiresReorder(const llvm::CallInst *CI);
  static DXIL::BarrierMode TranslateToBarrierMode(const llvm::CallInst *CI);
  static void GetMinShaderModelAndMask(OpCode C, bool bWithTranslation,
                                       unsigned &major, unsigned &minor,
                                       unsigned &mask);
  static void GetMinShaderModelAndMask(const llvm::CallInst *CI,
                                       bool bWithTranslation, unsigned valMajor,
                                       unsigned valMinor, unsigned &major,
                                       unsigned &minor, unsigned &mask);

  static bool IsDxilOpExtendedOverload(OpCode C);

  // Return true if the overload name suffix for this operation may be
  // constructed based on a user-defined or user-influenced type name
  // that may not represent the same type in different linked modules.
  static bool MayHaveNonCanonicalOverload(OpCode OC);

private:
  // Per-module properties.
  llvm::LLVMContext &m_Ctx;
  llvm::Module *m_pModule;

  llvm::Type *m_pHandleType;
  llvm::Type *m_pHitObjectType;
  llvm::Type *m_pNodeHandleType;
  llvm::Type *m_pNodeRecordHandleType;
  llvm::Type *m_pResourcePropertiesType;
  llvm::Type *m_pNodePropertiesType;
  llvm::Type *m_pNodeRecordPropertiesType;
  llvm::Type *m_pResourceBindingType;
  llvm::Type *m_pDimensionsType;
  llvm::Type *m_pSamplePosType;
  llvm::Type *m_pBinaryWithCarryType;
  llvm::Type *m_pBinaryWithTwoOutputsType;
  llvm::Type *m_pSplitDoubleType;
  llvm::Type *m_pFourI32Type;
  llvm::Type *m_pFourI16Type;

  DXIL::LowPrecisionMode m_LowPrecisionMode;

  // Overload types are split into "basic" overload types and special types
  // Basic: void, half, float, double, i1, i8, i16, i32, i64
  //  - These have one canonical overload per TypeSlot
  // Special: udt, obj, vec, extended
  //  - These may have many overloads per type slot
  enum TypeSlot : unsigned {
    TS_F16 = 0,
    TS_F32 = 1,
    TS_F64 = 2,
    TS_I1 = 3,
    TS_I8 = 4,
    TS_I16 = 5,
    TS_I32 = 6,
    TS_I64 = 7,
    TS_BasicCount,
    TS_UDT = 8,      // Ex: %"struct.MyStruct" *
    TS_Object = 9,   // Ex: %"class.StructuredBuffer<Foo>"
    TS_Vector = 10,  // Ex: <8 x i16>
    TS_MaskBitCount, // Types used in Mask end here
    // TS_Extended is only used to identify the unnamed struct type used to wrap
    // multiple overloads when using GetTypeSlot.
    TS_Extended, // Ex: type { float, <16 x i32> }
    TS_Invalid = UINT_MAX,
  };

  llvm::Type *m_pResRetType[TS_BasicCount];
  llvm::Type *m_pCBufferRetType[TS_BasicCount];

  struct OpCodeCacheItem {
    llvm::SmallMapVector<llvm::Type *, llvm::Function *, 8> pOverloads;
  };
  OpCodeCacheItem m_OpCodeClassCache[(unsigned)OpCodeClass::NumOpClasses];
  std::unordered_map<const llvm::Function *, OpCodeClass> m_FunctionToOpClass;
  void UpdateCache(OpCodeClass opClass, llvm::Type *Ty, llvm::Function *F);

private:
  // Static properties.
  struct OverloadMask {
    // mask of type slot bits as (1 << TypeSlot)
    uint16_t SlotMask;
    static_assert(TS_MaskBitCount <= (sizeof(SlotMask) * 8));
    bool operator[](unsigned TypeSlot) const {
      return (TypeSlot < TS_MaskBitCount) ? (bool)(SlotMask & (1 << TypeSlot))
                                          : 0;
    }
    operator bool() const { return SlotMask != 0; }
  };
  struct OpCodeProperty {
    OpCode opCode;
    const char *pOpCodeName;
    OpCodeClass opCodeClass;
    const char *pOpCodeClassName;
    llvm::Attribute::AttrKind FuncAttr;

    // Number of overload dimensions used by the operation.
    unsigned int NumOverloadDims;

    // Mask of supported overload types for each overload dimension.
    OverloadMask AllowedOverloads[DXIL::kDxilMaxOloadDims];

    // Mask of scalar components allowed for each demension where
    // AllowedOverloads[n][TS_Vector] is true.
    OverloadMask AllowedVectorElements[DXIL::kDxilMaxOloadDims];
  };
  static const OpCodeProperty m_OpCodeProps[(unsigned)OpCode::NumOpCodes];

  static const char *m_OverloadTypeName[TS_BasicCount];
  static const char *m_NamePrefix;
  static const char *m_TypePrefix;
  static const char *m_MatrixTypePrefix;
  static unsigned GetTypeSlot(llvm::Type *pType);
  static const char *GetOverloadTypeName(unsigned TypeSlot);
  static llvm::StringRef GetTypeName(llvm::Type *Ty,
                                     llvm::SmallVectorImpl<char> &Storage);
  static llvm::StringRef
  ConstructOverloadName(llvm::Type *Ty, DXIL::OpCode opCode,
                        llvm::SmallVectorImpl<char> &Storage);
};

} // namespace hlsl
