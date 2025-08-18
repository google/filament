//===-- SpirvType.cpp - SPIR-V Type Hierarchy -------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//===----------------------------------------------------------------------===//
//
//  This file implements the in-memory representation of SPIR-V types.
//
//===----------------------------------------------------------------------===//

#include "clang/SPIRV/SpirvType.h"
#include "clang/SPIRV/SpirvInstruction.h"
#include <sstream>

namespace clang {
namespace spirv {

bool ScalarType::classof(const SpirvType *t) {
  switch (t->getKind()) {
  case TK_Bool:
  case TK_Integer:
  case TK_Float:
    return true;
  default:
    break;
  }
  return false;
}

bool SpirvType::isTexture(const SpirvType *type) {
  if (const auto *imageType = dyn_cast<ImageType>(type)) {
    const auto dim = imageType->getDimension();
    const auto withSampler = imageType->withSampler();
    return (withSampler == ImageType::WithSampler::Yes) &&
           (dim == spv::Dim::Dim1D || dim == spv::Dim::Dim2D ||
            dim == spv::Dim::Dim3D || dim == spv::Dim::Cube);
  }
  return false;
}

bool SpirvType::isRWTexture(const SpirvType *type) {
  if (const auto *imageType = dyn_cast<ImageType>(type)) {
    const auto dim = imageType->getDimension();
    const auto withSampler = imageType->withSampler();
    return (withSampler == ImageType::WithSampler::No) &&
           (dim == spv::Dim::Dim1D || dim == spv::Dim::Dim2D ||
            dim == spv::Dim::Dim3D);
  }
  return false;
}

bool SpirvType::isSampler(const SpirvType *type) {
  return isa<SamplerType>(type);
}

bool SpirvType::isBuffer(const SpirvType *type) {
  if (const auto *imageType = dyn_cast<ImageType>(type)) {
    return imageType->getDimension() == spv::Dim::Buffer &&
           imageType->withSampler() == ImageType::WithSampler::Yes;
  }
  return false;
}

bool SpirvType::isRWBuffer(const SpirvType *type) {
  if (const auto *imageType = dyn_cast<ImageType>(type)) {
    return imageType->getDimension() == spv::Dim::Buffer &&
           imageType->withSampler() == ImageType::WithSampler::No;
  }
  return false;
}

bool SpirvType::isSubpassInput(const SpirvType *type) {
  if (const auto *imageType = dyn_cast<ImageType>(type)) {
    return imageType->getDimension() == spv::Dim::SubpassData &&
           imageType->isMSImage() == false;
  }
  return false;
}

bool SpirvType::isSubpassInputMS(const SpirvType *type) {
  if (const auto *imageType = dyn_cast<ImageType>(type)) {
    return imageType->getDimension() == spv::Dim::SubpassData &&
           imageType->isMSImage() == true;
  }
  return false;
}

bool SpirvType::isResourceType(const SpirvType *type) {
  if (isa<ImageType>(type) || isa<SamplerType>(type) ||
      isa<AccelerationStructureTypeNV>(type))
    return true;

  if (const auto *structType = dyn_cast<StructType>(type))
    return structType->getInterfaceType() !=
           StructInterfaceType::InternalStorage;

  if (const auto *pointerType = dyn_cast<SpirvPointerType>(type))
    return isResourceType(pointerType->getPointeeType());

  return false;
}

MatrixType::MatrixType(const VectorType *vecType, uint32_t vecCount)
    : SpirvType(TK_Matrix), vectorType(vecType), vectorCount(vecCount) {}

bool MatrixType::operator==(const MatrixType &that) const {
  return vectorType == that.vectorType && vectorCount == that.vectorCount;
}

ImageType::ImageType(const NumericalType *type, spv::Dim dim, WithDepth depth,
                     bool arrayed, bool ms, WithSampler sampled,
                     spv::ImageFormat format)
    : SpirvType(TK_Image, getImageName(dim, arrayed)), sampledType(type),
      dimension(dim), imageDepth(depth), isArrayed(arrayed), isMultiSampled(ms),
      isSampled(sampled), imageFormat(format) {}

std::string ImageType::getImageName(spv::Dim dim, bool arrayed) {
  const char *dimStr = "";
  switch (dim) {
  case spv::Dim::Dim1D:
    dimStr = "1d.";
    break;
  case spv::Dim::Dim2D:
    dimStr = "2d.";
    break;
  case spv::Dim::Dim3D:
    dimStr = "3d.";
    break;
  case spv::Dim::Cube:
    dimStr = "cube.";
    break;
  case spv::Dim::Rect:
    dimStr = "rect.";
    break;
  case spv::Dim::Buffer:
    dimStr = "buffer.";
    break;
  case spv::Dim::SubpassData:
    dimStr = "subpass.";
    break;
  default:
    break;
  }
  std::ostringstream name;
  name << "type." << dimStr << "image" << (arrayed ? ".array" : "");
  return name.str();
}

bool ImageType::operator==(const ImageType &that) const {
  return sampledType == that.sampledType && dimension == that.dimension &&
         imageDepth == that.imageDepth && isArrayed == that.isArrayed &&
         isMultiSampled == that.isMultiSampled && isSampled == that.isSampled &&
         imageFormat == that.imageFormat;
}

bool ArrayType::operator==(const ArrayType &that) const {
  return elementType == that.elementType && elementCount == that.elementCount &&
         stride.hasValue() == that.stride.hasValue() &&
         (!stride.hasValue() || stride.getValue() == that.stride.getValue());
}

bool RuntimeArrayType::operator==(const RuntimeArrayType &that) const {
  return elementType == that.elementType &&
         stride.hasValue() == that.stride.hasValue() &&
         (!stride.hasValue() || stride.getValue() == that.stride.getValue());
}

bool NodePayloadArrayType::operator==(const NodePayloadArrayType &that) const {
  return elementType == that.elementType && nodeDecl == that.nodeDecl;
}

bool SpvIntrinsicTypeOperand::operator==(
    const SpvIntrinsicTypeOperand &that) const {
  if (isTypeOperand != that.isTypeOperand)
    return false;

  if (isTypeOperand) {
    return operand_as_type == that.operand_as_type;
  } else {
    auto constantInst = dyn_cast<SpirvConstant>(operand_as_inst);
    assert(constantInst != nullptr);
    auto thatConstantInst = dyn_cast<SpirvConstant>(that.operand_as_inst);
    assert(thatConstantInst != nullptr);
    return *constantInst == *thatConstantInst;
  }
}

SpirvIntrinsicType::SpirvIntrinsicType(
    unsigned typeOp, llvm::ArrayRef<SpvIntrinsicTypeOperand> inOps)
    : SpirvType(TK_SpirvIntrinsicType, "spirvIntrinsicType"),
      typeOpCode(typeOp), operands(inOps.begin(), inOps.end()) {}

StructType::StructType(llvm::ArrayRef<StructType::FieldInfo> fieldsVec,
                       llvm::StringRef name, bool isReadOnly,
                       StructInterfaceType iface)
    : SpirvType(TK_Struct, name), fields(fieldsVec.begin(), fieldsVec.end()),
      readOnly(isReadOnly), interfaceType(iface) {}

bool StructType::FieldInfo::operator==(
    const StructType::FieldInfo &that) const {
  return type == that.type && offset.hasValue() == that.offset.hasValue() &&
         matrixStride.hasValue() == that.matrixStride.hasValue() &&
         isRowMajor.hasValue() == that.isRowMajor.hasValue() &&
         name == that.name &&
         // Either not have offset value, or have the same value
         (!offset.hasValue() || offset.getValue() == that.offset.getValue()) &&
         // Either not have matrix stride value, or have the same value
         (!matrixStride.hasValue() ||
          matrixStride.getValue() == that.matrixStride.getValue()) &&
         // Either not have row major value, or have the same value
         (!isRowMajor.hasValue() ||
          isRowMajor.getValue() == that.isRowMajor.getValue()) &&
         // Both should have the same precision
         isRelaxedPrecision == that.isRelaxedPrecision &&
         // Both fields should be precise or not precise
         isPrecise == that.isPrecise;
}

bool StructType::operator==(const StructType &that) const {
  return fields == that.fields && getName() == that.getName() &&
         readOnly == that.readOnly && interfaceType == that.interfaceType;
}

HybridStructType::HybridStructType(
    llvm::ArrayRef<HybridStructType::FieldInfo> fieldsVec, llvm::StringRef name,
    bool isReadOnly, StructInterfaceType iface)
    : HybridType(TK_HybridStruct, name),
      fields(fieldsVec.begin(), fieldsVec.end()), readOnly(isReadOnly),
      interfaceType(iface) {}

FunctionType::FunctionType(const SpirvType *ret,
                           llvm::ArrayRef<const SpirvType *> param)
    : SpirvType(TK_Function), returnType(ret),
      paramTypes(param.begin(), param.end()) {
  // Make sure
  assert(!isa<HybridType>(ret));
  for (auto *paramType : param) {
    (void)paramType;
    assert(!isa<HybridType>(paramType));
  }
}

} // namespace spirv
} // namespace clang
