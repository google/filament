//===--- GlPerVertex.cpp - GlPerVertex implementation ------------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "GlPerVertex.h"

#include <algorithm>

#include "clang/AST/Attr.h"
#include "clang/AST/HlslTypes.h"
#include "clang/SPIRV/AstTypeProbe.h"

namespace clang {
namespace spirv {

namespace {
constexpr uint32_t gClipDistanceIndex = 0;
constexpr uint32_t gCullDistanceIndex = 1;

/// \brief Returns true if the given decl has a semantic string attached and
/// writes the info to *semanticStr, *semantic, and *semanticIndex.
// TODO: duplication! Same as the one in DeclResultIdMapper.cpp
bool getStageVarSemantic(const NamedDecl *decl, llvm::StringRef *semanticStr,
                         const hlsl::Semantic **semantic,
                         uint32_t *semanticIndex) {
  for (auto *annotation : decl->getUnusualAnnotations()) {
    if (auto *sema = dyn_cast<hlsl::SemanticDecl>(annotation)) {
      *semanticStr = sema->SemanticName;
      llvm::StringRef semanticName;
      hlsl::Semantic::DecomposeNameAndIndex(*semanticStr, &semanticName,
                                            semanticIndex);
      *semantic = hlsl::Semantic::GetByName(semanticName);
      return true;
    }
  }
  return false;
}

/// Returns the type of the given decl. If the given decl is a FunctionDecl,
/// returns its result type.
inline QualType getTypeOrFnRetType(const DeclaratorDecl *decl) {
  if (const auto *funcDecl = dyn_cast<FunctionDecl>(decl)) {
    return funcDecl->getReturnType();
  }
  return decl->getType();
}

/// Returns true if the given declaration has a primitive type qualifier.
/// Returns false otherwise.
inline bool hasGSPrimitiveTypeQualifier(const NamedDecl *decl) {
  return decl->hasAttr<HLSLTriangleAttr>() ||
         decl->hasAttr<HLSLTriangleAdjAttr>() ||
         decl->hasAttr<HLSLPointAttr>() || decl->hasAttr<HLSLLineAttr>() ||
         decl->hasAttr<HLSLLineAdjAttr>();
}
} // anonymous namespace

GlPerVertex::GlPerVertex(ASTContext &context, SpirvContext &spirvContext,
                         SpirvBuilder &spirvBuilder)
    : astContext(context), spvContext(spirvContext), spvBuilder(spirvBuilder),
      inClipVar(nullptr), inCullVar(nullptr), outClipVar(nullptr),
      outCullVar(nullptr), inClipPrecise(false), outClipPrecise(false),
      inCullPrecise(false), outCullPrecise(false), inArraySize(0),
      outArraySize(0), inClipArraySize(1), outClipArraySize(1),
      inCullArraySize(1), outCullArraySize(1), inSemanticStrs(2, ""),
      outSemanticStrs(2, "") {}

void GlPerVertex::generateVars(uint32_t inArrayLen, uint32_t outArrayLen) {
  inArraySize = inArrayLen;
  outArraySize = outArrayLen;

  if (!inClipType.empty())
    inClipVar = createClipCullDistanceVar(/*asInput=*/true, /*isClip=*/true,
                                          inClipArraySize, inClipPrecise);
  if (!inCullType.empty())
    inCullVar = createClipCullDistanceVar(/*asInput=*/true, /*isClip=*/false,
                                          inCullArraySize, inCullPrecise);
  if (!outClipType.empty())
    outClipVar = createClipCullDistanceVar(/*asInput=*/false, /*isClip=*/true,
                                           outClipArraySize, outClipPrecise);
  if (!outCullType.empty())
    outCullVar = createClipCullDistanceVar(/*asInput=*/false, /*isClip=*/false,
                                           outCullArraySize, outCullPrecise);
}

llvm::SmallVector<SpirvVariable *, 2> GlPerVertex::getStageInVars() const {
  llvm::SmallVector<SpirvVariable *, 2> vars;

  if (inClipVar)
    vars.push_back(inClipVar);
  if (inCullVar)
    vars.push_back(inCullVar);

  return vars;
}

llvm::SmallVector<SpirvVariable *, 2> GlPerVertex::getStageOutVars() const {
  llvm::SmallVector<SpirvVariable *, 2> vars;

  if (outClipVar)
    vars.push_back(outClipVar);
  if (outCullVar)
    vars.push_back(outCullVar);

  return vars;
}

bool GlPerVertex::recordGlPerVertexDeclFacts(const DeclaratorDecl *decl,
                                             bool asInput) {
  const QualType type = getTypeOrFnRetType(decl);

  if (type->isVoidType())
    return true;

  // Indices or payload mesh shader param objects don't contain any
  // builtin variables or semantic strings. So early return.
  if (decl->hasAttr<HLSLIndicesAttr>() || decl->hasAttr<HLSLPayloadAttr>()) {
    return true;
  }

  return doGlPerVertexFacts(decl, type, asInput);
}

bool GlPerVertex::containOnlyFloatType(QualType type) const {
  QualType elemType;
  if (isScalarType(type, &elemType)) {
    if (const auto *builtinType = elemType->getAs<BuiltinType>())
      return builtinType->getKind() == BuiltinType::Float;
    return false;
  } else if (isVectorType(type, &elemType, nullptr)) {
    return containOnlyFloatType(elemType);
  } else if (const auto *arrayType = astContext.getAsConstantArrayType(type)) {
    return containOnlyFloatType(arrayType->getElementType());
  }
  return false;
}

uint32_t GlPerVertex::getNumberOfScalarComponentsInScalarVectorArray(
    QualType type) const {
  uint32_t count = 0;
  if (isScalarType(type)) {
    return 1;
  } else if (isVectorType(type, nullptr, &count)) {
    return count;
  } else if (type->isConstantArrayType()) {
    const auto *arrayType = astContext.getAsConstantArrayType(type);
    count = static_cast<uint32_t>(arrayType->getSize().getZExtValue());
    return count * getNumberOfScalarComponentsInScalarVectorArray(
                       arrayType->getElementType());
  }
  return 0;
}

SpirvInstruction *GlPerVertex::createScalarClipCullDistanceLoad(
    SpirvInstruction *ptr, QualType asType, uint32_t offset, SourceLocation loc,
    llvm::Optional<uint32_t> arrayIndex) const {
  if (!isScalarType(asType))
    return nullptr;

  // The ClipDistance/CullDistance is always an float array. We are accessing
  // it using pointers, which should be of pointer to float type.
  const QualType f32Type = astContext.FloatTy;

  llvm::SmallVector<SpirvInstruction *, 2> spirvConstants;
  if (arrayIndex.hasValue()) {
    spirvConstants.push_back(spvBuilder.getConstantInt(
        astContext.UnsignedIntTy, llvm::APInt(32, arrayIndex.getValue())));
  }
  spirvConstants.push_back(spvBuilder.getConstantInt(astContext.UnsignedIntTy,
                                                     llvm::APInt(32, offset)));
  return spvBuilder.createLoad(
      f32Type, spvBuilder.createAccessChain(f32Type, ptr, spirvConstants, loc),
      loc);
}

SpirvInstruction *GlPerVertex::createScalarOrVectorClipCullDistanceLoad(
    SpirvInstruction *ptr, QualType asType, uint32_t offset, SourceLocation loc,
    llvm::Optional<uint32_t> arrayIndex) const {
  if (isScalarType(asType))
    return createScalarClipCullDistanceLoad(ptr, asType, offset, loc,
                                            arrayIndex);

  QualType elemType = {};
  uint32_t count = 0;
  if (!isVectorType(asType, &elemType, &count))
    return nullptr;

  // The target SV_ClipDistance/SV_CullDistance variable is of vector
  // type, then we need to construct a vector out of float array elements.
  llvm::SmallVector<SpirvInstruction *, 4> elements;
  for (uint32_t i = 0; i < count; ++i) {
    elements.push_back(createScalarClipCullDistanceLoad(
        ptr, elemType, offset + i, loc, arrayIndex));
  }
  return spvBuilder.createCompositeConstruct(
      astContext.getExtVectorType(astContext.FloatTy, count), elements, loc);
}

SpirvInstruction *GlPerVertex::createClipCullDistanceLoad(
    SpirvInstruction *ptr, QualType asType, uint32_t offset, SourceLocation loc,
    llvm::Optional<uint32_t> arrayIndex) const {
  if (asType->isConstantArrayType()) {
    const auto *arrayType = astContext.getAsConstantArrayType(asType);
    uint32_t count = static_cast<uint32_t>(arrayType->getSize().getZExtValue());
    QualType elemType = arrayType->getElementType();
    uint32_t numberOfScalarsInElement =
        getNumberOfScalarComponentsInScalarVectorArray(elemType);
    if (numberOfScalarsInElement == 0)
      return nullptr;

    llvm::SmallVector<SpirvInstruction *, 4> elements;
    for (uint32_t i = 0; i < count; ++i) {
      elements.push_back(createScalarOrVectorClipCullDistanceLoad(
          ptr, elemType, offset + i * numberOfScalarsInElement, loc,
          arrayIndex));
    }
    return spvBuilder.createCompositeConstruct(asType, elements, loc);
  }

  return createScalarOrVectorClipCullDistanceLoad(ptr, asType, offset, loc,
                                                  arrayIndex);
}

bool GlPerVertex::createScalarClipCullDistanceStore(
    SpirvInstruction *ptr, SpirvInstruction *value, QualType valueType,
    SpirvInstruction *offset, SourceLocation loc,
    llvm::ArrayRef<uint32_t> valueIndices,
    llvm::Optional<SpirvInstruction *> arrayIndex) const {
  if (!isScalarType(valueType))
    return false;

  llvm::SmallVector<SpirvInstruction *, 2> ptrIndices;
  if (arrayIndex.hasValue()) {
    ptrIndices.push_back(arrayIndex.getValue());
  }
  ptrIndices.push_back(offset);
  ptr = spvBuilder.createAccessChain(astContext.FloatTy, ptr, ptrIndices, loc);
  if (!valueIndices.empty()) {
    value = spvBuilder.createCompositeExtract(astContext.FloatTy, value,
                                              valueIndices, loc);
  }
  spvBuilder.createStore(ptr, value, loc);
  return true;
}

bool GlPerVertex::createScalarOrVectorClipCullDistanceStore(
    SpirvInstruction *ptr, SpirvInstruction *value, QualType valueType,
    SpirvInstruction *offset, SourceLocation loc,
    llvm::Optional<uint32_t> valueOffset,
    llvm::Optional<SpirvInstruction *> arrayIndex) const {
  llvm::SmallVector<uint32_t, 2> valueIndices;
  if (valueOffset.hasValue())
    valueIndices.push_back(valueOffset.getValue());
  if (isScalarType(valueType)) {
    return createScalarClipCullDistanceStore(ptr, value, valueType, offset, loc,
                                             valueIndices, arrayIndex);
  }

  QualType elemType = {};
  uint32_t count = 0;
  if (!isVectorType(valueType, &elemType, &count))
    return false;

  // The target SV_ClipDistance/SV_CullDistance variable is of vector
  // type, then we need to construct a vector out of float array elements.
  for (uint32_t i = 0; i < count; ++i) {
    auto *constant =
        spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, i));
    auto *elemOffset = spvBuilder.createBinaryOp(
        spv::Op::OpIAdd, astContext.UnsignedIntTy, offset, constant, loc);
    valueIndices.push_back(i);
    createScalarClipCullDistanceStore(ptr, value, elemType, elemOffset, loc,
                                      valueIndices, arrayIndex);
    valueIndices.pop_back();
  }
  return true;
}

bool GlPerVertex::createClipCullDistanceStore(
    SpirvInstruction *ptr, SpirvInstruction *value, QualType valueType,
    SpirvInstruction *offset, SourceLocation loc,
    llvm::Optional<SpirvInstruction *> arrayIndex) const {
  if (valueType->isConstantArrayType()) {
    const auto *arrayType = astContext.getAsConstantArrayType(valueType);
    uint32_t count = static_cast<uint32_t>(arrayType->getSize().getZExtValue());
    QualType elemType = arrayType->getElementType();
    uint32_t numberOfScalarsInElement =
        getNumberOfScalarComponentsInScalarVectorArray(elemType);
    if (numberOfScalarsInElement == 0)
      return false;

    for (uint32_t i = 0; i < count; ++i) {
      auto *constant = spvBuilder.getConstantInt(
          astContext.UnsignedIntTy,
          llvm::APInt(32, i * numberOfScalarsInElement));
      auto *elemOffset = spvBuilder.createBinaryOp(
          spv::Op::OpIAdd, astContext.UnsignedIntTy, offset, constant, loc);
      createScalarOrVectorClipCullDistanceStore(
          ptr, value, elemType, elemOffset, loc, llvm::Optional<uint32_t>(i),
          arrayIndex);
    }
    return true;
  }

  return createScalarOrVectorClipCullDistanceStore(
      ptr, value, valueType, offset, loc, llvm::None, arrayIndex);
}

bool GlPerVertex::setClipCullDistanceType(SemanticIndexToTypeMap *typeMap,
                                          uint32_t semanticIndex,
                                          QualType clipCullDistanceType) const {
  if (getNumberOfScalarComponentsInScalarVectorArray(clipCullDistanceType) ==
      0) {
    return false;
  }
  (*typeMap)[semanticIndex] = clipCullDistanceType;
  return true;
}

bool GlPerVertex::doGlPerVertexFacts(const NamedDecl *decl, QualType baseType,
                                     bool asInput) {
  if (hlsl::IsHLSLNodeType(baseType)) {
    return true;
  }

  llvm::StringRef semanticStr;
  const hlsl::Semantic *semantic = {};
  uint32_t semanticIndex = {};
  bool isPrecise = decl->hasAttr<HLSLPreciseAttr>();

  if (!getStageVarSemantic(decl, &semanticStr, &semantic, &semanticIndex)) {
    if (baseType->isStructureType()) {
      const auto *recordType = baseType->getAs<RecordType>();
      const auto *recordDecl = recordType->getAsCXXRecordDecl();
      // Go through each field to see if there is any usage of
      // SV_ClipDistance/SV_CullDistance.
      for (const auto *field : recordDecl->fields()) {
        if (!doGlPerVertexFacts(field, field->getType(), asInput))
          return false;
      }

      // We should also recursively go through each inherited class.
      for (const auto &base : recordDecl->bases()) {
        const auto *baseDecl = base.getType()->getAsCXXRecordDecl();
        if (!doGlPerVertexFacts(baseDecl, base.getType(), asInput))
          return false;
      }
      return true;
    }

    // For these HS/DS/GS specific data types, semantic strings are attached
    // to the underlying struct's fields.
    if (hlsl::IsHLSLInputPatchType(baseType)) {
      return doGlPerVertexFacts(
          decl, hlsl::GetHLSLInputPatchElementType(baseType), asInput);
    }
    if (hlsl::IsHLSLOutputPatchType(baseType) ||
        hlsl::IsHLSLStreamOutputType(baseType)) {
      return doGlPerVertexFacts(
          decl, hlsl::GetHLSLOutputPatchElementType(baseType), asInput);
    }
    if (hasGSPrimitiveTypeQualifier(decl) ||
        decl->hasAttr<HLSLVerticesAttr>() ||
        decl->hasAttr<HLSLPrimitivesAttr>()) {
      // GS inputs and MS output attribute have an additional arrayness that we
      // should remove to check the underlying type instead.
      baseType = astContext.getAsConstantArrayType(baseType)->getElementType();
      return doGlPerVertexFacts(decl, baseType, asInput);
    }

    emitError("semantic string missing for shader %select{output|input}0 "
              "variable '%1'",
              decl->getLocation())
        << asInput << decl->getName();
    return false;
  }

  // Semantic string is attached to this decl directly

  // Select the corresponding data member to update
  SemanticIndexToTypeMap *typeMap = nullptr;
  uint32_t *blockArraySize = asInput ? &inArraySize : &outArraySize;
  bool isCull = false;
  auto *semanticStrs = asInput ? &inSemanticStrs : &outSemanticStrs;
  uint32_t index = kSemanticStrCount;

  switch (semantic->GetKind()) {
  case hlsl::Semantic::Kind::ClipDistance:
    typeMap = asInput ? &inClipType : &outClipType;
    index = gClipDistanceIndex;
    break;
  case hlsl::Semantic::Kind::CullDistance:
    typeMap = asInput ? &inCullType : &outCullType;
    isCull = true;
    index = gCullDistanceIndex;
    break;
  default:
    // Only Cull or Clip apply.
    break;
  }

  if (isCull) {
    if (asInput)
      inCullPrecise = isPrecise;
    else
      outCullPrecise = isPrecise;
  } else {
    if (asInput)
      inClipPrecise = isPrecise;
    else
      outClipPrecise = isPrecise;
  }

  // Remember the semantic strings provided by the developer so that we can
  // emit OpDecorate* instructions properly for them
  if (index < kSemanticStrCount) {
    if ((*semanticStrs)[index].empty())
      (*semanticStrs)[index] = semanticStr;
    // We can have multiple ClipDistance/CullDistance semantics mapping to the
    // same variable. For those cases, it is not appropriate to use any one of
    // them as the semantic. Use the standard one without index.
    else if (index == gClipDistanceIndex)
      (*semanticStrs)[index] = "SV_ClipDistance";
    else if (index == gCullDistanceIndex)
      (*semanticStrs)[index] = "SV_CullDistance";
  }

  if (index > gCullDistanceIndex) {
    // Annotated with something other than SV_ClipDistance or SV_CullDistance.
    // We don't care about such cases.
    return true;
  }

  // Parameters marked as inout has reference type.
  if (baseType->isReferenceType())
    baseType = baseType->getPointeeType();

  // Clip/cull distance must be made up only with floats.
  if (!containOnlyFloatType(baseType)) {
    emitError("elements for %select{SV_ClipDistance|SV_CullDistance}0 "
              "variable '%1' must be scalar, vector, or array with float type",
              decl->getLocStart())
        << isCull << decl->getName();
    return false;
  }

  if (baseType->isConstantArrayType()) {
    const auto *arrayType = astContext.getAsConstantArrayType(baseType);

    // TODO: handle extra large array size?
    if (*blockArraySize ==
        static_cast<uint32_t>(arrayType->getSize().getZExtValue())) {
      if (setClipCullDistanceType(typeMap, semanticIndex,
                                  arrayType->getElementType())) {
        return true;
      }

      emitError("elements for %select{SV_ClipDistance|SV_CullDistance}0 "
                "variable '%1' must be scalar, vector, or array with float "
                "type",
                decl->getLocStart())
          << isCull << decl->getName();
      return false;
    }
  }

  if (setClipCullDistanceType(typeMap, semanticIndex, baseType)) {
    return true;
  }

  emitError("type for %select{SV_ClipDistance|SV_CullDistance}0 "
            "variable '%1' must be a scalar, vector, or array with float type",
            decl->getLocStart())
      << isCull << decl->getName();
  return false;
}

void GlPerVertex::calculateClipCullDistanceArraySize() {
  // Updates the offset map and array size for the given input/output
  // SV_ClipDistance/SV_CullDistance.
  const auto updateSizeAndOffset =
      [this](const SemanticIndexToTypeMap &typeMap,
             SemanticIndexToArrayOffsetMap *offsetMap, uint32_t *totalSize) {
        // If no usage of SV_ClipDistance/SV_CullDistance was recorded,just
        // return. This will keep the size defaulted to 1.
        if (typeMap.empty())
          return;

        *totalSize = 0;

        // Collect all indices and sort them
        llvm::SmallVector<uint32_t, 8> indices;
        for (const auto &kv : typeMap)
          indices.push_back(kv.first);
        std::sort(indices.begin(), indices.end(), std::less<uint32_t>());

        for (uint32_t index : indices) {
          const auto type = typeMap.find(index)->second;
          uint32_t count = getNumberOfScalarComponentsInScalarVectorArray(type);
          if (count == 0) {
            llvm_unreachable("SV_ClipDistance/SV_CullDistance has unexpected "
                             "type or size");
          }
          (*offsetMap)[index] = *totalSize;
          *totalSize += count;
        }
      };

  updateSizeAndOffset(inClipType, &inClipOffset, &inClipArraySize);
  updateSizeAndOffset(inCullType, &inCullOffset, &inCullArraySize);
  updateSizeAndOffset(outClipType, &outClipOffset, &outClipArraySize);
  updateSizeAndOffset(outCullType, &outCullOffset, &outCullArraySize);
}

SpirvVariable *GlPerVertex::createClipCullDistanceVar(bool asInput, bool isClip,
                                                      uint32_t arraySize,
                                                      bool isPrecise) {
  QualType type = astContext.getConstantArrayType(astContext.FloatTy,
                                                  llvm::APInt(32, arraySize),
                                                  clang::ArrayType::Normal, 0);

  if (asInput && inArraySize != 0) {
    type = astContext.getConstantArrayType(type, llvm::APInt(32, inArraySize),
                                           clang::ArrayType::Normal, 0);
  } else if (!asInput && outArraySize != 0) {
    type = astContext.getConstantArrayType(type, llvm::APInt(32, outArraySize),
                                           clang::ArrayType::Normal, 0);
  }

  spv::StorageClass sc =
      asInput ? spv::StorageClass::Input : spv::StorageClass::Output;

  SpirvVariable *var = spvBuilder.addStageBuiltinVar(
      type, sc,
      isClip ? spv::BuiltIn::ClipDistance : spv::BuiltIn::CullDistance,
      isPrecise, /*SourceLocation*/ {});

  const auto index = isClip ? gClipDistanceIndex : gCullDistanceIndex;
  spvBuilder.decorateHlslSemantic(var, asInput ? inSemanticStrs[index]
                                               : outSemanticStrs[index]);
  return var;
}

bool GlPerVertex::tryToAccess(hlsl::SigPoint::Kind sigPointKind,
                              hlsl::Semantic::Kind semanticKind,
                              uint32_t semanticIndex,
                              llvm::Optional<SpirvInstruction *> invocationId,
                              SpirvInstruction **value, bool noWriteBack,
                              SpirvInstruction *vecComponent,
                              SourceLocation loc, SourceRange range) {
  assert(value);
  // invocationId should only be used for HSPCOut or MSOut.
  assert(invocationId.hasValue()
             ? (sigPointKind == hlsl::SigPoint::Kind::HSCPOut ||
                sigPointKind == hlsl::SigPoint::Kind::MSOut)
             : true);

  switch (semanticKind) {
  case hlsl::Semantic::Kind::ClipDistance:
  case hlsl::Semantic::Kind::CullDistance:
    // gl_PerVertex only cares about these builtins.
    break;
  default:
    return false; // Fall back to the normal path
  }

  switch (sigPointKind) {
  case hlsl::SigPoint::Kind::PSIn:
  case hlsl::SigPoint::Kind::HSCPIn:
  case hlsl::SigPoint::Kind::DSCPIn:
  case hlsl::SigPoint::Kind::GSVIn:
    return readField(semanticKind, semanticIndex, value, loc, range);

  case hlsl::SigPoint::Kind::GSOut:
  case hlsl::SigPoint::Kind::VSOut:
  case hlsl::SigPoint::Kind::HSCPOut:
  case hlsl::SigPoint::Kind::DSOut:
  case hlsl::SigPoint::Kind::MSOut:
    if (noWriteBack)
      return true;

    return writeField(semanticKind, semanticIndex, invocationId, value,
                      vecComponent, loc, range);
  default:
    // Only interfaces that involve gl_PerVertex are needed.
    break;
  }

  return false;
}

SpirvInstruction *
GlPerVertex::readClipCullArrayAsType(bool isClip, uint32_t offset,
                                     QualType asType, SourceLocation loc,
                                     SourceRange range) const {
  SpirvVariable *clipCullVar = isClip ? inClipVar : inCullVar;
  uint32_t count = getNumberOfScalarComponentsInScalarVectorArray(asType);
  if (count == 0) {
    llvm_unreachable("SV_ClipDistance/SV_CullDistance has unexpected type "
                     "or size");
  }

  if (inArraySize == 0) {
    return createClipCullDistanceLoad(clipCullVar, asType, offset, loc);
  }

  // The input builtin block is an array of block, which means we need to
  // return an array of ClipDistance/CullDistance values from an array of
  // struct.

  llvm::SmallVector<SpirvInstruction *, 8> arrayElements;
  QualType arrayType = {};
  for (uint32_t i = 0; i < inArraySize; ++i) {
    arrayElements.push_back(createClipCullDistanceLoad(
        clipCullVar, asType, offset, loc, llvm::Optional<uint32_t>(i)));
  }
  arrayType = astContext.getConstantArrayType(
      asType, llvm::APInt(32, inArraySize), clang::ArrayType::Normal, 0);
  return spvBuilder.createCompositeConstruct(arrayType, arrayElements, loc);
}

bool GlPerVertex::readField(hlsl::Semantic::Kind semanticKind,
                            uint32_t semanticIndex, SpirvInstruction **value,
                            SourceLocation loc, SourceRange range) {
  assert(value);
  switch (semanticKind) {
  case hlsl::Semantic::Kind::ClipDistance: {
    const auto offsetIter = inClipOffset.find(semanticIndex);
    const auto typeIter = inClipType.find(semanticIndex);
    // We should have recorded all these semantics before.
    assert(offsetIter != inClipOffset.end());
    assert(typeIter != inClipType.end());
    *value = readClipCullArrayAsType(/*isClip=*/true, offsetIter->second,
                                     typeIter->second, loc, range);
    return true;
  }
  case hlsl::Semantic::Kind::CullDistance: {
    const auto offsetIter = inCullOffset.find(semanticIndex);
    const auto typeIter = inCullType.find(semanticIndex);
    // We should have recorded all these semantics before.
    assert(offsetIter != inCullOffset.end());
    assert(typeIter != inCullType.end());
    *value = readClipCullArrayAsType(/*isClip=*/false, offsetIter->second,
                                     typeIter->second, loc, range);
    return true;
  }
  default:
    // Only Cull or Clip apply.
    break;
  }
  return false;
}

void GlPerVertex::writeClipCullArrayFromType(
    llvm::Optional<SpirvInstruction *> invocationId, bool isClip,
    SpirvInstruction *offset, QualType fromType, SpirvInstruction *fromValue,
    SourceLocation loc, SourceRange range) const {
  auto *clipCullVar = isClip ? outClipVar : outCullVar;

  if (outArraySize == 0) {
    // The output builtin does not have extra arrayness. Only need one index
    // to locate the array segment for this SV_ClipDistance/SV_CullDistance
    // variable: the start offset within the float array.
    if (createClipCullDistanceStore(clipCullVar, fromValue, fromType, offset,
                                    loc)) {
      return;
    }

    llvm_unreachable("SV_ClipDistance/SV_CullDistance has unexpected type "
                     "or size");
    return;
  }

  // Writing to an array only happens in HSCPOut or MSOut.
  if (!spvContext.isHS() && !spvContext.isMS()) {
    llvm_unreachable("Writing to clip/cull distance in hull/mesh shader is "
                     "not allowed");
  }

  // And we are only writing to the array element with InvocationId as index.
  assert(invocationId.hasValue());

  // The output builtin block is an array of block, which means we need to
  // write an array of ClipDistance/CullDistance values into an array of
  // struct.

  SpirvInstruction *arrayIndex = invocationId.getValue();
  if (createClipCullDistanceStore(
          clipCullVar, fromValue, fromType, offset, loc,
          llvm::Optional<SpirvInstruction *>(arrayIndex))) {
    return;
  }

  llvm_unreachable("SV_ClipDistance/SV_CullDistance has unexpected type or "
                   "size");
}

bool GlPerVertex::writeField(hlsl::Semantic::Kind semanticKind,
                             uint32_t semanticIndex,
                             llvm::Optional<SpirvInstruction *> invocationId,
                             SpirvInstruction **value,
                             SpirvInstruction *vecComponent, SourceLocation loc,
                             SourceRange range) {
  // Similar to the writing logic in DeclResultIdMapper::createStageVars():
  //
  // Unlike reading, which may require us to read stand-alone builtins and
  // stage input variables and compose an array of structs out of them,
  // it happens that we don't need to write an array of structs in a bunch
  // for all shader stages:
  //
  // * VS: output is a single struct, without extra arrayness
  // * HS: output is an array of structs, with extra arrayness,
  //       but we only write to the struct at the InvocationID index
  // * DS: output is a single struct, without extra arrayness
  // * GS: output is controlled by OpEmitVertex, one vertex per time
  // * MS: output is an array of structs, with extra arrayness
  //
  // The interesting shader stage is HS. We need the InvocationID to write
  // out the value to the correct array element.
  SpirvInstruction *offset = nullptr;
  QualType type = {};
  bool isClip = false;
  switch (semanticKind) {
  case hlsl::Semantic::Kind::ClipDistance: {
    const auto offsetIter = outClipOffset.find(semanticIndex);
    const auto typeIter = outClipType.find(semanticIndex);
    // We should have recorded all these semantics before.
    assert(offsetIter != outClipOffset.end());
    assert(typeIter != outClipType.end());
    offset = spvBuilder.getConstantInt(astContext.UnsignedIntTy,
                                       llvm::APInt(32, offsetIter->second));
    type = typeIter->second;
    isClip = true;
    break;
  }
  case hlsl::Semantic::Kind::CullDistance: {
    const auto offsetIter = outCullOffset.find(semanticIndex);
    const auto typeIter = outCullType.find(semanticIndex);
    // We should have recorded all these semantics before.
    assert(offsetIter != outCullOffset.end());
    assert(typeIter != outCullType.end());
    offset = spvBuilder.getConstantInt(astContext.UnsignedIntTy,
                                       llvm::APInt(32, offsetIter->second));
    type = typeIter->second;
    break;
  }
  default:
    // Only Cull or Clip apply.
    return false;
  }
  if (vecComponent) {
    QualType elemType = {};
    if (!isVectorType(type, &elemType)) {
      assert(false && "expected vector type");
    }
    type = elemType;
    offset =
        spvBuilder.createBinaryOp(spv::Op::OpIAdd, astContext.UnsignedIntTy,
                                  vecComponent, offset, loc, range);
  }
  writeClipCullArrayFromType(invocationId, isClip, offset, type, *value, loc,
                             range);
  return true;
}

} // end namespace spirv
} // end namespace clang
