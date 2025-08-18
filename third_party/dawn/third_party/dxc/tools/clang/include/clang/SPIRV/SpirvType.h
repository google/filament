//===-- SpirvType.h - SPIR-V Type -----------------------------*- C++ -*---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_SPIRV_SPIRVTYPE_H
#define LLVM_CLANG_SPIRV_SPIRVTYPE_H

#include <string>
#include <utility>
#include <vector>

#include "spirv/unified1/spirv.hpp11"
#include "clang/AST/Attr.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

namespace clang {
namespace spirv {

class HybridType;

enum class StructInterfaceType : uint32_t {
  InternalStorage = 0,
  StorageBuffer = 1,
  UniformBuffer = 2,
};

struct BitfieldInfo {
  // Offset of the bitfield, in bits, from the basetype start.
  uint32_t offsetInBits;
  // Size of the bitfield, in bits.
  uint32_t sizeInBits;
};

class SpirvType {
public:
  enum Kind {
    TK_Void,
    TK_Bool,
    TK_Integer,
    TK_Float,
    TK_Vector,
    TK_Matrix,
    TK_Image,
    TK_Sampler,
    TK_SampledImage,
    TK_Array,
    TK_RuntimeArray,
    TK_NodePayloadArrayAMD,
    TK_Struct,
    TK_Pointer,
    TK_ForwardPointer,
    TK_Function,
    TK_AccelerationStructureNV,
    TK_RayQueryKHR,
    TK_SpirvIntrinsicType,
    // Order matters: all the following are hybrid types
    TK_HybridStruct,
    TK_HybridPointer,
    TK_HybridSampledImage,
    TK_HybridFunction,
  };

  virtual ~SpirvType() = default;

  Kind getKind() const { return kind; }
  llvm::StringRef getName() const { return debugName; }

  static bool isTexture(const SpirvType *);
  static bool isRWTexture(const SpirvType *);
  static bool isSampler(const SpirvType *);
  static bool isBuffer(const SpirvType *);
  static bool isRWBuffer(const SpirvType *);
  static bool isSubpassInput(const SpirvType *);
  static bool isSubpassInputMS(const SpirvType *);
  static bool isResourceType(const SpirvType *);
  template <class T, unsigned int Bitwidth = 0>
  static bool isOrContainsType(const SpirvType *);

protected:
  SpirvType(Kind k, llvm::StringRef name = "") : kind(k), debugName(name) {}

private:
  const Kind kind;
  std::string debugName;
};

class VoidType : public SpirvType {
public:
  VoidType() : SpirvType(TK_Void) {}

  static bool classof(const SpirvType *t) { return t->getKind() == TK_Void; }
};

class ScalarType : public SpirvType {
public:
  static bool classof(const SpirvType *t);

protected:
  ScalarType(Kind k) : SpirvType(k) {}
};

class NumericalType : public ScalarType {
public:
  static bool classof(const SpirvType *t) {
    return t->getKind() == TK_Integer || t->getKind() == TK_Float;
  }

  uint32_t getBitwidth() const { return bitwidth; }

protected:
  NumericalType(Kind k, uint32_t width) : ScalarType(k), bitwidth(width) {}

  uint32_t bitwidth;
};

class BoolType : public ScalarType {
public:
  BoolType() : ScalarType(TK_Bool) {}

  static bool classof(const SpirvType *t) { return t->getKind() == TK_Bool; }
};

class IntegerType : public NumericalType {
public:
  IntegerType(uint32_t numBits, bool sign)
      : NumericalType(TK_Integer, numBits), isSigned(sign) {}

  static bool classof(const SpirvType *t) { return t->getKind() == TK_Integer; }

  bool isSignedInt() const { return isSigned; }

private:
  bool isSigned;
};

class FloatType : public NumericalType {
public:
  FloatType(uint32_t numBits) : NumericalType(TK_Float, numBits) {}

  static bool classof(const SpirvType *t) { return t->getKind() == TK_Float; }
};

class VectorType : public SpirvType {
public:
  VectorType(const ScalarType *elemType, uint32_t elemCount)
      : SpirvType(TK_Vector), elementType(elemType), elementCount(elemCount) {}

  static bool classof(const SpirvType *t) { return t->getKind() == TK_Vector; }

  const SpirvType *getElementType() const { return elementType; }
  uint32_t getElementCount() const { return elementCount; }

private:
  const ScalarType *elementType;
  uint32_t elementCount;
};

class MatrixType : public SpirvType {
public:
  MatrixType(const VectorType *vecType, uint32_t vecCount);

  static bool classof(const SpirvType *t) { return t->getKind() == TK_Matrix; }

  bool operator==(const MatrixType &that) const;

  const SpirvType *getVecType() const { return vectorType; }
  const SpirvType *getElementType() const {
    return vectorType->getElementType();
  }
  uint32_t getVecCount() const { return vectorCount; }
  uint32_t numCols() const { return vectorCount; }
  uint32_t numRows() const { return vectorType->getElementCount(); }

private:
  const VectorType *vectorType;
  uint32_t vectorCount;
};

class ImageType : public SpirvType {
public:
  enum class WithSampler : uint32_t {
    Unknown = 0,
    Yes = 1,
    No = 2,
  };
  enum class WithDepth : uint32_t {
    No = 0,
    Yes = 1,
    Unknown = 2,
  };

  ImageType(const NumericalType *sampledType, spv::Dim, WithDepth depth,
            bool isArrayed, bool isMultiSampled, WithSampler sampled,
            spv::ImageFormat);

  static bool classof(const SpirvType *t) { return t->getKind() == TK_Image; }

  bool operator==(const ImageType &that) const;

  const SpirvType *getSampledType() const { return sampledType; }
  spv::Dim getDimension() const { return dimension; }
  WithDepth getDepth() const { return imageDepth; }
  bool isArrayedImage() const { return isArrayed; }
  bool isMSImage() const { return isMultiSampled; }
  WithSampler withSampler() const { return isSampled; }
  spv::ImageFormat getImageFormat() const { return imageFormat; }

private:
  std::string getImageName(spv::Dim, bool arrayed);

private:
  const NumericalType *sampledType;
  spv::Dim dimension;
  WithDepth imageDepth;
  bool isArrayed;
  bool isMultiSampled;
  WithSampler isSampled;
  spv::ImageFormat imageFormat;
};

class SamplerType : public SpirvType {
public:
  SamplerType() : SpirvType(TK_Sampler, "type.sampler") {}

  static bool classof(const SpirvType *t) { return t->getKind() == TK_Sampler; }
};

class SampledImageType : public SpirvType {
public:
  SampledImageType(const ImageType *image)
      : SpirvType(TK_SampledImage, "type.sampled.image"), imageType(image) {}

  static bool classof(const SpirvType *t) {
    return t->getKind() == TK_SampledImage;
  }

  const SpirvType *getImageType() const { return imageType; }

private:
  const ImageType *imageType;
};

class ArrayType : public SpirvType {
public:
  ArrayType(const SpirvType *elemType, uint32_t elemCount,
            llvm::Optional<uint32_t> arrayStride)
      : SpirvType(TK_Array), elementType(elemType), elementCount(elemCount),
        stride(arrayStride) {}

  const SpirvType *getElementType() const { return elementType; }
  uint32_t getElementCount() const { return elementCount; }
  llvm::Optional<uint32_t> getStride() const { return stride; }

  static bool classof(const SpirvType *t) { return t->getKind() == TK_Array; }

  bool operator==(const ArrayType &that) const;

private:
  const SpirvType *elementType;
  uint32_t elementCount;
  // Two arrays types with different ArrayStride decorations, are in fact two
  // different array types. If no layout information is needed, use llvm::None.
  llvm::Optional<uint32_t> stride;
};

class RuntimeArrayType : public SpirvType {
public:
  RuntimeArrayType(const SpirvType *elemType,
                   llvm::Optional<uint32_t> arrayStride)
      : SpirvType(TK_RuntimeArray), elementType(elemType), stride(arrayStride) {
  }

  static bool classof(const SpirvType *t) {
    return t->getKind() == TK_RuntimeArray;
  }

  bool operator==(const RuntimeArrayType &that) const;

  const SpirvType *getElementType() const { return elementType; }
  llvm::Optional<uint32_t> getStride() const { return stride; }

private:
  const SpirvType *elementType;
  // Two runtime arrays with different ArrayStride decorations, are in fact two
  // different types. If no layout information is needed, use llvm::None.
  llvm::Optional<uint32_t> stride;
};

class NodePayloadArrayType : public SpirvType {
public:
  NodePayloadArrayType(const SpirvType *elemType, const ParmVarDecl *decl)
      : SpirvType(TK_NodePayloadArrayAMD), elementType(elemType),
        nodeDecl(decl) {}

  static bool classof(const SpirvType *t) {
    return t->getKind() == TK_NodePayloadArrayAMD;
  }

  bool operator==(const NodePayloadArrayType &that) const;

  const SpirvType *getElementType() const { return elementType; }
  const ParmVarDecl *getNodeDecl() const { return nodeDecl; }

private:
  const SpirvType *elementType;
  const ParmVarDecl *nodeDecl;
};

// The StructType is the lowered type that best represents what a structure type
// is in SPIR-V. Contains all necessary information for properly emitting a
// SPIR-V structure type.
class StructType : public SpirvType {
public:
  struct FieldInfo {
  public:
    FieldInfo(const SpirvType *type_, uint32_t fieldIndex_,
              llvm::StringRef name_ = "",
              llvm::Optional<uint32_t> offset_ = llvm::None,
              llvm::Optional<uint32_t> matrixStride_ = llvm::None,
              llvm::Optional<bool> isRowMajor_ = llvm::None,
              bool relaxedPrecision = false, bool precise = false,
              llvm::Optional<AttrVec> attributes = llvm::None)
        : type(type_), fieldIndex(fieldIndex_), name(name_), offset(offset_),
          sizeInBytes(llvm::None), matrixStride(matrixStride_),
          isRowMajor(isRowMajor_), isRelaxedPrecision(relaxedPrecision),
          isPrecise(precise), bitfield(llvm::None), attributes(attributes) {
      // A StructType may not contain any hybrid types.
      assert(!isa<HybridType>(type_));
    }

    bool operator==(const FieldInfo &that) const;

    // The field's type.
    const SpirvType *type;
    // The index of this field in the composite construct.
    // When the struct contains bitfields, StructType index and construct index
    // can diverge as we merge bitfields together.
    uint32_t fieldIndex;
    // The field's name.
    std::string name;
    // The integer offset in bytes for this field.
    llvm::Optional<uint32_t> offset;
    // The integer size in bytes for this field.
    llvm::Optional<uint32_t> sizeInBytes;
    // The matrix stride for this field (if applicable).
    llvm::Optional<uint32_t> matrixStride;
    // The majorness of this field (if applicable).
    llvm::Optional<bool> isRowMajor;
    // Whether this field is a RelaxedPrecision field.
    bool isRelaxedPrecision;
    // Whether this field is marked as 'precise'.
    bool isPrecise;
    // Information about the bitfield (if applicable).
    llvm::Optional<BitfieldInfo> bitfield;
    // Other attributes applied to the field.
    llvm::Optional<AttrVec> attributes;
  };

  StructType(
      llvm::ArrayRef<FieldInfo> fields, llvm::StringRef name, bool isReadOnly,
      StructInterfaceType interfaceType = StructInterfaceType::InternalStorage);

  static bool classof(const SpirvType *t) { return t->getKind() == TK_Struct; }

  llvm::ArrayRef<FieldInfo> getFields() const { return fields; }
  bool isReadOnly() const { return readOnly; }
  llvm::StringRef getStructName() const { return getName(); }
  StructInterfaceType getInterfaceType() const { return interfaceType; }

  bool operator==(const StructType &that) const;

private:
  // Reflection is heavily used in graphics pipelines. Reflection relies on
  // struct names and field names. That basically means we cannot ignore these
  // names when considering unification. Otherwise, reflection will be confused.

  llvm::SmallVector<FieldInfo, 8> fields;
  bool readOnly;
  // Indicates the interface type of this structure. If this structure is a
  // storage buffer shader-interface, it will be decorated with 'BufferBlock'.
  // If this structure is a uniform buffer shader-interface, it will be
  // decorated with 'Block'.
  StructInterfaceType interfaceType;
};

/// Represents a SPIR-V pointer type.
class SpirvPointerType : public SpirvType {
public:
  SpirvPointerType(const SpirvType *pointee, spv::StorageClass sc)
      : SpirvType(TK_Pointer), pointeeType(pointee), storageClass(sc) {}

  static bool classof(const SpirvType *t) { return t->getKind() == TK_Pointer; }

  const SpirvType *getPointeeType() const { return pointeeType; }
  spv::StorageClass getStorageClass() const { return storageClass; }

  bool operator==(const SpirvPointerType &that) const {
    return pointeeType == that.pointeeType && storageClass == that.storageClass;
  }

private:
  const SpirvType *pointeeType;
  spv::StorageClass storageClass;
};

/// Represents a SPIR-V forwarding pointer type.
class ForwardPointerType : public SpirvType {
public:
  ForwardPointerType(QualType pointee)
      : SpirvType(TK_ForwardPointer), pointeeType(pointee) {}

  static bool classof(const SpirvType *t) {
    return t->getKind() == TK_ForwardPointer;
  }

  const QualType getPointeeType() const { return pointeeType; }

  bool operator==(const ForwardPointerType &that) const {
    return pointeeType == that.pointeeType;
  }

private:
  const QualType pointeeType;
};

/// Represents a SPIR-V function type. None of the parameters nor the return
/// type is allowed to be a hybrid type.
class FunctionType : public SpirvType {
public:
  FunctionType(const SpirvType *ret, llvm::ArrayRef<const SpirvType *> param);

  static bool classof(const SpirvType *t) {
    return t->getKind() == TK_Function;
  }

  bool operator==(const FunctionType &that) const {
    return returnType == that.returnType && paramTypes == that.paramTypes;
  }

  // void setReturnType(const SpirvType *t) { returnType = t; }
  const SpirvType *getReturnType() const { return returnType; }
  llvm::ArrayRef<const SpirvType *> getParamTypes() const { return paramTypes; }

private:
  const SpirvType *returnType;
  llvm::SmallVector<const SpirvType *, 8> paramTypes;
};

/// Represents accleration structure type as defined in SPV_NV_ray_tracing.
class AccelerationStructureTypeNV : public SpirvType {
public:
  AccelerationStructureTypeNV()
      : SpirvType(TK_AccelerationStructureNV, "accelerationStructureNV") {}

  static bool classof(const SpirvType *t) {
    return t->getKind() == TK_AccelerationStructureNV;
  }
};

class RayQueryTypeKHR : public SpirvType {
public:
  RayQueryTypeKHR() : SpirvType(TK_RayQueryKHR, "rayQueryKHR") {}

  static bool classof(const SpirvType *t) {
    return t->getKind() == TK_RayQueryKHR;
  }
};

class SpirvInstruction;
struct SpvIntrinsicTypeOperand {
  SpvIntrinsicTypeOperand(const SpirvType *type_operand)
      : operand_as_type(type_operand), isTypeOperand(true) {}
  SpvIntrinsicTypeOperand(SpirvInstruction *inst_operand)
      : operand_as_inst(inst_operand), isTypeOperand(false) {}
  bool operator==(const SpvIntrinsicTypeOperand &that) const;
  union {
    const SpirvType *operand_as_type;
    SpirvInstruction *operand_as_inst;
  };
  const bool isTypeOperand;
};

class SpirvIntrinsicType : public SpirvType {
public:
  SpirvIntrinsicType(unsigned typeOp,
                     llvm::ArrayRef<SpvIntrinsicTypeOperand> inOps);

  static bool classof(const SpirvType *t) {
    return t->getKind() == TK_SpirvIntrinsicType;
  }
  unsigned getOpCode() const { return typeOpCode; }
  llvm::ArrayRef<SpvIntrinsicTypeOperand> getOperands() const {
    return operands;
  }

  bool operator==(const SpirvIntrinsicType &that) const {
    return typeOpCode == that.typeOpCode &&
           operands.size() == that.operands.size() &&
           std::equal(operands.begin(), operands.end(), that.operands.begin());
  }

private:
  unsigned typeOpCode;
  llvm::SmallVector<SpvIntrinsicTypeOperand, 3> operands;
};

class HybridType : public SpirvType {
public:
  static bool classof(const SpirvType *t) {
    return t->getKind() >= TK_HybridStruct && t->getKind() <= TK_HybridFunction;
  }

protected:
  HybridType(Kind k, llvm::StringRef name = "") : SpirvType(k, name) {}
};

/// **NOTE**: This type is created in order to facilitate transition of old
/// infrastructure to the new infrastructure. Using this type should be avoided
/// as much as possible.
///
/// This type uses a mix of SpirvType and QualType for the structure fields.
class HybridStructType : public HybridType {
public:
  struct FieldInfo {
  public:
    FieldInfo(QualType astType_, llvm::StringRef name_ = "",
              clang::VKOffsetAttr *offset = nullptr,
              hlsl::ConstantPacking *packOffset = nullptr,
              const hlsl::RegisterAssignment *regC = nullptr,
              bool precise = false,
              llvm::Optional<BitfieldInfo> bitfield = llvm::None,
              llvm::Optional<AttrVec> attributes = llvm::None)
        : astType(astType_), name(name_), vkOffsetAttr(offset),
          packOffsetAttr(packOffset), registerC(regC), isPrecise(precise),
          bitfield(std::move(bitfield)), attributes(std::move(attributes)) {}

    // The field's type.
    QualType astType;
    // The field's name.
    std::string name;
    // vk::offset attributes associated with this field.
    clang::VKOffsetAttr *vkOffsetAttr;
    // :packoffset() annotations associated with this field.
    hlsl::ConstantPacking *packOffsetAttr;
    // :register(c#) annotations associated with this field.
    const hlsl::RegisterAssignment *registerC;
    // Whether this field is marked as 'precise'.
    bool isPrecise;
    // Whether this field is a bitfield or not. If set to false, bitfield width
    // value is undefined.
    llvm::Optional<BitfieldInfo> bitfield;
    // Other attributes applied to the field.
    llvm::Optional<AttrVec> attributes;
  };

  HybridStructType(
      llvm::ArrayRef<FieldInfo> fields, llvm::StringRef name, bool isReadOnly,
      StructInterfaceType interfaceType = StructInterfaceType::InternalStorage);

  static bool classof(const SpirvType *t) {
    return t->getKind() == TK_HybridStruct;
  }

  llvm::ArrayRef<FieldInfo> getFields() const { return fields; }
  bool isReadOnly() const { return readOnly; }
  llvm::StringRef getStructName() const { return getName(); }
  StructInterfaceType getInterfaceType() const { return interfaceType; }

private:
  // Reflection is heavily used in graphics pipelines. Reflection relies on
  // struct names and field names. That basically means we cannot ignore these
  // names when considering unification. Otherwise, reflection will be confused.

  llvm::SmallVector<FieldInfo, 8> fields;
  bool readOnly;
  // Indicates the interface type of this structure. If this structure is a
  // storage buffer shader-interface, it will be decorated with 'BufferBlock'.
  // If this structure is a uniform buffer shader-interface, it will be
  // decorated with 'Block'.
  StructInterfaceType interfaceType;
};

class HybridPointerType : public HybridType {
public:
  HybridPointerType(QualType pointee, spv::StorageClass sc)
      : HybridType(TK_HybridPointer), pointeeType(pointee), storageClass(sc) {}

  static bool classof(const SpirvType *t) {
    return t->getKind() == TK_HybridPointer;
  }

  QualType getPointeeType() const { return pointeeType; }
  spv::StorageClass getStorageClass() const { return storageClass; }

  bool operator==(const HybridPointerType &that) const {
    return pointeeType == that.pointeeType && storageClass == that.storageClass;
  }

private:
  QualType pointeeType;
  spv::StorageClass storageClass;
};

class HybridSampledImageType : public HybridType {
public:
  HybridSampledImageType(QualType image)
      : HybridType(TK_HybridSampledImage), imageType(image) {}

  static bool classof(const SpirvType *t) {
    return t->getKind() == TK_HybridSampledImage;
  }

  QualType getImageType() const { return imageType; }

private:
  QualType imageType;
};

//
// Function Definition for templated functions
//

template <class T, unsigned int Bitwidth>
bool SpirvType::isOrContainsType(const SpirvType *type) {
  if (isa<T>(type)) {
    if (Bitwidth == 0)
      // No specific bitwidth was asked for.
      return true;
    else
      // We want to make sure it is a numberical type of a specific bitwidth.
      return isa<NumericalType>(type) &&
             llvm::cast<NumericalType>(type)->getBitwidth() == Bitwidth;
  }

  if (const auto *vecType = dyn_cast<VectorType>(type))
    return isOrContainsType<T, Bitwidth>(vecType->getElementType());
  if (const auto *matType = dyn_cast<MatrixType>(type))
    return isOrContainsType<T, Bitwidth>(matType->getElementType());
  if (const auto *arrType = dyn_cast<ArrayType>(type))
    return isOrContainsType<T, Bitwidth>(arrType->getElementType());
  if (const auto *pointerType = dyn_cast<SpirvPointerType>(type))
    return isOrContainsType<T, Bitwidth>(pointerType->getPointeeType());
  if (const auto *raType = dyn_cast<RuntimeArrayType>(type))
    return isOrContainsType<T, Bitwidth>(raType->getElementType());
  if (const auto *npaType = dyn_cast<NodePayloadArrayType>(type))
    return isOrContainsType<T, Bitwidth>(npaType->getElementType());
  if (const auto *imgType = dyn_cast<ImageType>(type))
    return isOrContainsType<T, Bitwidth>(imgType->getSampledType());
  if (const auto *sampledImageType = dyn_cast<SampledImageType>(type))
    return isOrContainsType<T, Bitwidth>(sampledImageType->getImageType());
  if (const auto *structType = dyn_cast<StructType>(type))
    for (auto &field : structType->getFields())
      if (isOrContainsType<T, Bitwidth>(field.type))
        return true;

  return false;
}

} // end namespace spirv
} // end namespace clang

#endif // LLVM_CLANG_SPIRV_SPIRVTYPE_H
