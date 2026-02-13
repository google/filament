//===--- SpirvContext.cpp - SPIR-V SpirvContext implementation-------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <tuple>

#include "clang/SPIRV/SpirvContext.h"
#include "clang/SPIRV/SpirvModule.h"

namespace clang {
namespace spirv {

SpirvContext::SpirvContext()
    : allocator(), voidType(nullptr), boolType(nullptr), sintTypes({}),
      uintTypes({}), floatTypes({}), samplerType(nullptr),
      curShaderModelKind(ShaderModelKind::Invalid), majorVersion(0),
      minorVersion(0), currentLexicalScope(nullptr) {
  voidType = new (this) VoidType;
  boolType = new (this) BoolType;
  samplerType = new (this) SamplerType;
  accelerationStructureTypeNV = new (this) AccelerationStructureTypeNV;
  rayQueryTypeKHR = new (this) RayQueryTypeKHR;
}

SpirvContext::~SpirvContext() {
  voidType->~VoidType();
  boolType->~BoolType();
  samplerType->~SamplerType();
  accelerationStructureTypeNV->~AccelerationStructureTypeNV();
  rayQueryTypeKHR->~RayQueryTypeKHR();

  for (auto *sintType : sintTypes)
    if (sintType) // sintTypes may contain nullptr
      sintType->~IntegerType();

  for (auto *uintType : uintTypes)
    if (uintType) // uintTypes may contain nullptr
      uintType->~IntegerType();

  for (auto *floatType : floatTypes)
    if (floatType) // floatTypes may contain nullptr
      floatType->~FloatType();

  for (auto &pair : vecTypes)
    for (auto *vecType : pair.second)
      if (vecType) // vecTypes may contain nullptr
        vecType->~VectorType();

  for (auto &pair : matTypes)
    for (auto *matType : pair.second)
      matType->~MatrixType();

  for (auto *arrType : arrayTypes)
    arrType->~ArrayType();

  for (auto *raType : runtimeArrayTypes)
    raType->~RuntimeArrayType();

  for (auto *npaType : nodePayloadArrayTypes)
    npaType->~NodePayloadArrayType();

  for (auto *fnType : functionTypes)
    fnType->~FunctionType();

  for (auto *structType : structTypes)
    structType->~StructType();

  for (auto *hybridStructType : hybridStructTypes)
    hybridStructType->~HybridStructType();

  for (auto pair : sampledImageTypes)
    pair.second->~SampledImageType();

  for (auto *hybridSampledImageType : hybridSampledImageTypes)
    hybridSampledImageType->~HybridSampledImageType();

  for (auto *imgType : imageTypes)
    imgType->~ImageType();

  for (auto &pair : pointerTypes)
    for (auto &scPtrTypePair : pair.second)
      scPtrTypePair.second->~SpirvPointerType();

  for (auto *hybridPtrType : hybridPointerTypes)
    hybridPtrType->~HybridPointerType();

  for (auto &typePair : debugTypes)
    typePair.second->releaseMemory();

  for (auto &typePair : typeTemplates)
    typePair.second->releaseMemory();

  for (auto &typePair : typeTemplateParams)
    typePair.second->releaseMemory();

  for (auto &pair : spirvIntrinsicTypesById) {
    assert(pair.second);
    pair.second->~SpirvIntrinsicType();
  }

  for (auto *spirvIntrinsicType : spirvIntrinsicTypes) {
    spirvIntrinsicType->~SpirvIntrinsicType();
  }
}

inline uint32_t log2ForBitwidth(uint32_t bitwidth) {
  assert(bitwidth >= 8 && bitwidth <= 64 && llvm::isPowerOf2_32(bitwidth));

  return llvm::Log2_32(bitwidth);
}

const IntegerType *SpirvContext::getSIntType(uint32_t bitwidth) {
  auto &type = sintTypes[log2ForBitwidth(bitwidth)];

  if (type == nullptr) {
    type = new (this) IntegerType(bitwidth, true);
  }
  return type;
}

const IntegerType *SpirvContext::getUIntType(uint32_t bitwidth) {
  auto &type = uintTypes[log2ForBitwidth(bitwidth)];

  if (type == nullptr) {
    type = new (this) IntegerType(bitwidth, false);
  }
  return type;
}

const FloatType *SpirvContext::getFloatType(uint32_t bitwidth) {
  auto &type = floatTypes[log2ForBitwidth(bitwidth)];

  if (type == nullptr) {
    type = new (this) FloatType(bitwidth);
  }
  return type;
}

const VectorType *SpirvContext::getVectorType(const SpirvType *elemType,
                                              uint32_t count) {
  // We are certain this should be a scalar type. Otherwise, cast causes an
  // assertion failure.
  const ScalarType *scalarType = cast<ScalarType>(elemType);
  assert(count == 2 || count == 3 || count == 4);

  auto found = vecTypes.find(scalarType);

  if (found != vecTypes.end()) {
    auto &type = found->second[count];
    if (type != nullptr)
      return type;
  } else {
    // Make sure to initialize since std::array is "an aggregate type with the
    // same semantics as a struct holding a C-style array T[N]".
    vecTypes[scalarType] = {};
  }

  return vecTypes[scalarType][count] = new (this) VectorType(scalarType, count);
}

const SpirvType *SpirvContext::getMatrixType(const SpirvType *elemType,
                                             uint32_t count) {
  // We are certain this should be a vector type. Otherwise, cast causes an
  // assertion failure.
  const VectorType *vecType = cast<VectorType>(elemType);
  assert(count == 2 || count == 3 || count == 4);

  // In the case of non-floating-point matrices, we represent them as array of
  // vectors.
  if (!isa<FloatType>(vecType->getElementType())) {
    return getArrayType(elemType, count, llvm::None);
  }

  auto foundVec = matTypes.find(vecType);

  if (foundVec != matTypes.end()) {
    const auto &matVector = foundVec->second;
    // Create a temporary object for finding in the vector.
    MatrixType type(vecType, count);

    for (const auto *cachedType : matVector)
      if (type == *cachedType)
        return cachedType;
  }

  const auto *ptr = new (this) MatrixType(vecType, count);

  matTypes[vecType].push_back(ptr);

  return ptr;
}

const ImageType *
SpirvContext::getImageType(const ImageType *imageTypeWithUnknownFormat,
                           spv::ImageFormat format) {
  return getImageType(imageTypeWithUnknownFormat->getSampledType(),
                      imageTypeWithUnknownFormat->getDimension(),
                      imageTypeWithUnknownFormat->getDepth(),
                      imageTypeWithUnknownFormat->isArrayedImage(),
                      imageTypeWithUnknownFormat->isMSImage(),
                      imageTypeWithUnknownFormat->withSampler(), format);
}

const ImageType *SpirvContext::getImageType(const SpirvType *sampledType,
                                            spv::Dim dim,
                                            ImageType::WithDepth depth,
                                            bool arrayed, bool ms,
                                            ImageType::WithSampler sampled,
                                            spv::ImageFormat format) {
  // We are certain this should be a numerical type. Otherwise, cast causes an
  // assertion failure.
  const NumericalType *elemType = cast<NumericalType>(sampledType);

  // Create a temporary object for finding in the set.
  ImageType type(elemType, dim, depth, arrayed, ms, sampled, format);
  auto found = imageTypes.find(&type);
  if (found != imageTypes.end())
    return *found;

  auto inserted = imageTypes.insert(
      new (this) ImageType(elemType, dim, depth, arrayed, ms, sampled, format));

  return *(inserted.first);
}

const SampledImageType *
SpirvContext::getSampledImageType(const ImageType *image) {
  auto found = sampledImageTypes.find(image);

  if (found != sampledImageTypes.end())
    return found->second;

  return sampledImageTypes[image] = new (this) SampledImageType(image);
}

const HybridSampledImageType *
SpirvContext::getSampledImageType(QualType image) {
  const HybridSampledImageType *result =
      new (this) HybridSampledImageType(image);
  hybridSampledImageTypes.push_back(result);
  return result;
}

const ArrayType *
SpirvContext::getArrayType(const SpirvType *elemType, uint32_t elemCount,
                           llvm::Optional<uint32_t> arrayStride) {
  ArrayType type(elemType, elemCount, arrayStride);

  auto found = arrayTypes.find(&type);
  if (found != arrayTypes.end())
    return *found;

  auto inserted =
      arrayTypes.insert(new (this) ArrayType(elemType, elemCount, arrayStride));
  // The return value is an (iterator, bool) pair. The boolean indicates whether
  // it was actually added as a new type.
  return *(inserted.first);
}

const RuntimeArrayType *
SpirvContext::getRuntimeArrayType(const SpirvType *elemType,
                                  llvm::Optional<uint32_t> arrayStride) {
  RuntimeArrayType type(elemType, arrayStride);
  auto found = runtimeArrayTypes.find(&type);
  if (found != runtimeArrayTypes.end())
    return *found;

  auto inserted = runtimeArrayTypes.insert(
      new (this) RuntimeArrayType(elemType, arrayStride));
  return *(inserted.first);
}

const NodePayloadArrayType *
SpirvContext::getNodePayloadArrayType(const SpirvType *elemType,
                                      const ParmVarDecl *nodeDecl) {
  NodePayloadArrayType type(elemType, nodeDecl);
  auto found = nodePayloadArrayTypes.find(&type);
  if (found != nodePayloadArrayTypes.end())
    return *found;

  auto inserted = nodePayloadArrayTypes.insert(
      new (this) NodePayloadArrayType(elemType, nodeDecl));
  return *(inserted.first);
}

const StructType *
SpirvContext::getStructType(llvm::ArrayRef<StructType::FieldInfo> fields,
                            llvm::StringRef name, bool isReadOnly,
                            StructInterfaceType interfaceType) {
  // We are creating a temporary struct type here for querying whether the
  // same type was already created. It is a little bit costly, but we can
  // avoid allocating directly from the bump pointer allocator, from which
  // then we are unable to reclaim until the allocator itself is destroyed.

  StructType type(fields, name, isReadOnly, interfaceType);

  auto found = std::find_if(
      structTypes.begin(), structTypes.end(),
      [&type](const StructType *cachedType) { return type == *cachedType; });

  if (found != structTypes.end())
    return *found;

  structTypes.push_back(
      new (this) StructType(fields, name, isReadOnly, interfaceType));

  return structTypes.back();
}

const HybridStructType *SpirvContext::getHybridStructType(
    llvm::ArrayRef<HybridStructType::FieldInfo> fields, llvm::StringRef name,
    bool isReadOnly, StructInterfaceType interfaceType) {
  const HybridStructType *result =
      new (this) HybridStructType(fields, name, isReadOnly, interfaceType);
  hybridStructTypes.push_back(result);
  return result;
}

const SpirvPointerType *SpirvContext::getPointerType(const SpirvType *pointee,
                                                     spv::StorageClass sc) {
  auto foundPointee = pointerTypes.find(pointee);

  if (foundPointee != pointerTypes.end()) {
    auto &pointeeMap = foundPointee->second;
    auto foundSC = pointeeMap.find(sc);

    if (foundSC != pointeeMap.end())
      return foundSC->second;
  }

  return pointerTypes[pointee][sc] = new (this) SpirvPointerType(pointee, sc);
}

const HybridPointerType *SpirvContext::getPointerType(QualType pointee,
                                                      spv::StorageClass sc) {
  const HybridPointerType *result = new (this) HybridPointerType(pointee, sc);
  hybridPointerTypes.push_back(result);
  return result;
}

const ForwardPointerType *
SpirvContext::getForwardPointerType(QualType pointee) {
  assert(hlsl::IsVKBufferPointerType(pointee));

  auto foundPointee = forwardPointerTypes.find(pointee);
  if (foundPointee != forwardPointerTypes.end()) {
    return foundPointee->second;
  }

  return forwardPointerTypes[pointee] = new (this) ForwardPointerType(pointee);
}

const SpirvPointerType *SpirvContext::getForwardReference(QualType type) {
  return forwardReferences[type];
}

void SpirvContext::registerForwardReference(
    QualType type, const SpirvPointerType *pointerType) {
  assert(pointerType->getStorageClass() ==
         spv::StorageClass::PhysicalStorageBuffer);
  forwardReferences[type] = pointerType;
}

FunctionType *
SpirvContext::getFunctionType(const SpirvType *ret,
                              llvm::ArrayRef<const SpirvType *> param) {
  // Create a temporary object for finding in the set.
  FunctionType type(ret, param);
  auto found = functionTypes.find(&type);
  if (found != functionTypes.end())
    return *found;

  auto inserted = functionTypes.insert(new (this) FunctionType(ret, param));
  return *inserted.first;
}

const StructType *SpirvContext::getByteAddressBufferType(bool isWritable) {
  // Create a uint RuntimeArray.
  const auto *raType =
      getRuntimeArrayType(getUIntType(32), /* ArrayStride */ 4);

  // Create a struct containing the runtime array as its only member.
  return getStructType({StructType::FieldInfo(raType, /*fieldIndex*/ 0,
                                              /*name*/ "", /*offset*/ 0)},
                       isWritable ? "type.RWByteAddressBuffer"
                                  : "type.ByteAddressBuffer",
                       !isWritable, StructInterfaceType::StorageBuffer);
}

const StructType *SpirvContext::getACSBufferCounterType() {
  // Create int32.
  const auto *int32Type = getSIntType(32);

  // Create a struct containing the integer counter as its only member.
  const StructType *type =
      getStructType({StructType::FieldInfo(int32Type, /*fieldIndex*/ 0,
                                           "counter", /*offset*/ 0)},
                    "type.ACSBuffer.counter",
                    /*isReadOnly*/ false, StructInterfaceType::StorageBuffer);

  return type;
}

SpirvDebugType *SpirvContext::getDebugTypeBasic(const SpirvType *spirvType,
                                                llvm::StringRef name,
                                                SpirvConstant *size,
                                                uint32_t encoding) {
  // Reuse existing debug type if possible.
  if (debugTypes.find(spirvType) != debugTypes.end())
    return debugTypes[spirvType];

  auto *debugType = new (this) SpirvDebugTypeBasic(name, size, encoding);
  debugTypes[spirvType] = debugType;
  return debugType;
}

SpirvDebugType *
SpirvContext::getDebugTypeMember(llvm::StringRef name, SpirvDebugType *type,
                                 SpirvDebugSource *source, uint32_t line,
                                 uint32_t column, SpirvDebugInstruction *parent,
                                 uint32_t flags, uint32_t offsetInBits,
                                 uint32_t sizeInBits, const APValue *value) {
  // NOTE: Do not search it in debugTypes because it would have the same
  // spirvType but has different parent i.e., type composite.

  SpirvDebugTypeMember *debugType =
      new (this) SpirvDebugTypeMember(name, type, source, line, column, parent,
                                      flags, offsetInBits, sizeInBits, value);
  return debugType;
}

SpirvDebugTypeComposite *SpirvContext::getDebugTypeComposite(
    const SpirvType *spirvType, llvm::StringRef name, SpirvDebugSource *source,
    uint32_t line, uint32_t column, SpirvDebugInstruction *parent,
    llvm::StringRef linkageName, uint32_t flags, uint32_t tag) {
  // Reuse existing debug type if possible.
  auto it = debugTypes.find(spirvType);
  if (it != debugTypes.end()) {
    assert(it->second != nullptr && isa<SpirvDebugTypeComposite>(it->second));
    return dyn_cast<SpirvDebugTypeComposite>(it->second);
  }

  auto *debugType = new (this) SpirvDebugTypeComposite(
      name, source, line, column, parent, linkageName, flags, tag);
  debugType->setDebugSpirvType(spirvType);
  debugTypes[spirvType] = debugType;
  return debugType;
}

SpirvDebugType *SpirvContext::getDebugType(const SpirvType *spirvType) {
  auto it = debugTypes.find(spirvType);
  if (it != debugTypes.end())
    return it->second;
  return nullptr;
}

SpirvDebugType *
SpirvContext::getDebugTypeArray(const SpirvType *spirvType,
                                SpirvDebugInstruction *elemType,
                                llvm::ArrayRef<uint32_t> elemCount) {
  // Reuse existing debug type if possible.
  if (debugTypes.find(spirvType) != debugTypes.end())
    return debugTypes[spirvType];

  auto *eTy = dyn_cast<SpirvDebugType>(elemType);
  assert(eTy && "Element type must be a SpirvDebugType.");
  auto *debugType = new (this) SpirvDebugTypeArray(eTy, elemCount);
  debugTypes[spirvType] = debugType;
  return debugType;
}

SpirvDebugType *
SpirvContext::getDebugTypeVector(const SpirvType *spirvType,
                                 SpirvDebugInstruction *elemType,
                                 uint32_t elemCount) {
  // Reuse existing debug type if possible.
  if (debugTypes.find(spirvType) != debugTypes.end())
    return debugTypes[spirvType];

  auto *eTy = dyn_cast<SpirvDebugType>(elemType);
  assert(eTy && "Element type must be a SpirvDebugType.");
  auto *debugType = new (this) SpirvDebugTypeVector(eTy, elemCount);
  debugTypes[spirvType] = debugType;
  return debugType;
}

SpirvDebugType *
SpirvContext::getDebugTypeMatrix(const SpirvType *spirvType,
                                 SpirvDebugInstruction *vectorType,
                                 uint32_t vectorCount) {
  // Reuse existing debug type if possible.
  if (debugTypes.find(spirvType) != debugTypes.end())
    return debugTypes[spirvType];

  auto *eTy = dyn_cast<SpirvDebugTypeVector>(vectorType);
  assert(eTy && "Element type must be a SpirvDebugTypeVector.");
  auto *debugType = new (this) SpirvDebugTypeMatrix(eTy, vectorCount);
  debugTypes[spirvType] = debugType;
  return debugType;
}

SpirvDebugType *
SpirvContext::getDebugTypeFunction(const SpirvType *spirvType, uint32_t flags,
                                   SpirvDebugType *ret,
                                   llvm::ArrayRef<SpirvDebugType *> params) {
  // Reuse existing debug type if possible.
  if (debugTypes.find(spirvType) != debugTypes.end())
    return debugTypes[spirvType];

  auto *debugType = new (this) SpirvDebugTypeFunction(flags, ret, params);
  debugTypes[spirvType] = debugType;
  return debugType;
}

SpirvDebugTypeTemplate *SpirvContext::createDebugTypeTemplate(
    const ClassTemplateSpecializationDecl *templateType,
    SpirvDebugInstruction *target,
    const llvm::SmallVector<SpirvDebugTypeTemplateParameter *, 2> &params) {
  auto *tempTy = getDebugTypeTemplate(templateType);
  if (tempTy != nullptr)
    return tempTy;
  tempTy = new (this) SpirvDebugTypeTemplate(target, params);
  typeTemplates[templateType] = tempTy;
  return tempTy;
}

SpirvDebugTypeTemplate *SpirvContext::getDebugTypeTemplate(
    const ClassTemplateSpecializationDecl *templateType) {
  auto it = typeTemplates.find(templateType);
  if (it != typeTemplates.end())
    return it->second;
  return nullptr;
}

SpirvDebugTypeTemplateParameter *SpirvContext::createDebugTypeTemplateParameter(
    const TemplateArgument *templateArg, llvm::StringRef name,
    SpirvDebugType *type, SpirvInstruction *value, SpirvDebugSource *source,
    uint32_t line, uint32_t column) {
  auto *param = getDebugTypeTemplateParameter(templateArg);
  if (param != nullptr)
    return param;
  param = new (this)
      SpirvDebugTypeTemplateParameter(name, type, value, source, line, column);
  typeTemplateParams[templateArg] = param;
  return param;
}

SpirvDebugTypeTemplateParameter *SpirvContext::getDebugTypeTemplateParameter(
    const TemplateArgument *templateArg) {
  auto it = typeTemplateParams.find(templateArg);
  if (it != typeTemplateParams.end())
    return it->second;
  return nullptr;
}

void SpirvContext::pushDebugLexicalScope(RichDebugInfo *info,
                                         SpirvDebugInstruction *scope) {
  assert((isa<SpirvDebugLexicalBlock>(scope) ||
          isa<SpirvDebugFunction>(scope) ||
          isa<SpirvDebugCompilationUnit>(scope) ||
          isa<SpirvDebugTypeComposite>(scope)) &&
         "Given scope is not a lexical scope");
  currentLexicalScope = scope;
  info->scopeStack.push_back(scope);
}

void SpirvContext::moveDebugTypesToModule(SpirvModule *module) {
  for (const auto &typePair : debugTypes) {
    module->addDebugInfo(typePair.second);

    if (auto *composite = dyn_cast<SpirvDebugTypeComposite>(typePair.second)) {
      for (auto *member : composite->getMembers()) {
        module->addDebugInfo(member);
      }
    }
  }
  for (const auto &typePair : typeTemplates) {
    module->addDebugInfo(typePair.second);
  }
  for (const auto &typePair : typeTemplateParams) {
    module->addDebugInfo(typePair.second);
  }

  debugTypes.clear();
  typeTemplates.clear();
  typeTemplateParams.clear();
}

const SpirvIntrinsicType *SpirvContext::getOrCreateSpirvIntrinsicType(
    unsigned typeId, unsigned typeOpCode,
    llvm::ArrayRef<SpvIntrinsicTypeOperand> operands) {
  if (spirvIntrinsicTypesById[typeId] == nullptr) {
    spirvIntrinsicTypesById[typeId] =
        new (this) SpirvIntrinsicType(typeOpCode, operands);
  }
  return spirvIntrinsicTypesById[typeId];
}

const SpirvIntrinsicType *SpirvContext::getOrCreateSpirvIntrinsicType(
    unsigned typeOpCode, llvm::ArrayRef<SpvIntrinsicTypeOperand> operands) {
  SpirvIntrinsicType type(typeOpCode, operands);

  auto found =
      std::find_if(spirvIntrinsicTypes.begin(), spirvIntrinsicTypes.end(),
                   [&type](const SpirvIntrinsicType *cachedType) {
                     return type == *cachedType;
                   });

  if (found != spirvIntrinsicTypes.end())
    return *found;

  spirvIntrinsicTypes.push_back(new (this)
                                    SpirvIntrinsicType(typeOpCode, operands));

  return spirvIntrinsicTypes.back();
}

SpirvIntrinsicType *
SpirvContext::getCreatedSpirvIntrinsicType(unsigned typeId) {
  if (spirvIntrinsicTypesById.find(typeId) == spirvIntrinsicTypesById.end()) {
    return nullptr;
  }
  return spirvIntrinsicTypesById[typeId];
}

} // end namespace spirv
} // end namespace clang
