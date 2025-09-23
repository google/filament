//===--- DeclResultIdMapper.cpp - DeclResultIdMapper impl --------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "DeclResultIdMapper.h"

#include <algorithm>
#include <optional>
#include <sstream>

#include "dxc/DXIL/DxilConstants.h"
#include "dxc/DXIL/DxilTypeSystem.h"
#include "dxc/Support/SPIRVOptions.h"
#include "clang/AST/Expr.h"
#include "clang/AST/HlslTypes.h"
#include "clang/SPIRV/AstTypeProbe.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/Casting.h"

#include "AlignmentSizeCalculator.h"
#include "SignaturePackingUtil.h"
#include "SpirvEmitter.h"

namespace clang {
namespace spirv {

namespace {

// Returns true if the image format is compatible with the sampled type. This is
// determined according to the same at
// https://docs.vulkan.org/spec/latest/appendices/spirvenv.html#spirvenv-format-type-matching.
bool areFormatAndTypeCompatible(spv::ImageFormat format, QualType sampledType) {
  if (format == spv::ImageFormat::Unknown) {
    return true;
  }

  if (hlsl::IsHLSLVecType(sampledType)) {
    // For vectors, we need to check if the element type is compatible. We do
    // not check the number of elements because it is possible that the number
    // of elements in the sampled type is different. I could not find in the
    // spec what should happen in that case.
    sampledType = hlsl::GetHLSLVecElementType(sampledType);
  }

  const Type *desugaredType = sampledType->getUnqualifiedDesugaredType();
  const BuiltinType *builtinType = dyn_cast<BuiltinType>(desugaredType);
  if (!builtinType) {
    return false;
  }

  switch (format) {
  case spv::ImageFormat::Rgba32f:
  case spv::ImageFormat::Rg32f:
  case spv::ImageFormat::R32f:
  case spv::ImageFormat::Rgba16f:
  case spv::ImageFormat::Rg16f:
  case spv::ImageFormat::R16f:
  case spv::ImageFormat::Rgba16:
  case spv::ImageFormat::Rg16:
  case spv::ImageFormat::R16:
  case spv::ImageFormat::Rgba16Snorm:
  case spv::ImageFormat::Rg16Snorm:
  case spv::ImageFormat::R16Snorm:
  case spv::ImageFormat::Rgb10A2:
  case spv::ImageFormat::R11fG11fB10f:
  case spv::ImageFormat::Rgba8:
  case spv::ImageFormat::Rg8:
  case spv::ImageFormat::R8:
  case spv::ImageFormat::Rgba8Snorm:
  case spv::ImageFormat::Rg8Snorm:
  case spv::ImageFormat::R8Snorm:
    // 32-bit float
    return builtinType->getKind() == BuiltinType::Float;
  case spv::ImageFormat::Rgba32i:
  case spv::ImageFormat::Rg32i:
  case spv::ImageFormat::R32i:
  case spv::ImageFormat::Rgba16i:
  case spv::ImageFormat::Rg16i:
  case spv::ImageFormat::R16i:
  case spv::ImageFormat::Rgba8i:
  case spv::ImageFormat::Rg8i:
  case spv::ImageFormat::R8i:
    // signed 32-bit int
    return builtinType->getKind() == BuiltinType::Int;
  case spv::ImageFormat::Rgba32ui:
  case spv::ImageFormat::Rg32ui:
  case spv::ImageFormat::R32ui:
  case spv::ImageFormat::Rgba16ui:
  case spv::ImageFormat::Rg16ui:
  case spv::ImageFormat::R16ui:
  case spv::ImageFormat::Rgb10a2ui:
  case spv::ImageFormat::Rgba8ui:
  case spv::ImageFormat::Rg8ui:
  case spv::ImageFormat::R8ui:
    // unsigned 32-bit int
    return builtinType->getKind() == BuiltinType::UInt;
  case spv::ImageFormat::R64i:
    // signed 64-bit int
    return builtinType->getKind() == BuiltinType::LongLong;
  case spv::ImageFormat::R64ui:
    // unsigned 64-bit int
    return builtinType->getKind() == BuiltinType::ULongLong;
  }

  return true;
}

uint32_t getVkBindingAttrSet(const VKBindingAttr *attr, uint32_t defaultSet) {
  // If the [[vk::binding(x)]] attribute is provided without the descriptor set,
  // we should use the default descriptor set.
  if (attr->getSet() == INT_MIN) {
    return defaultSet;
  }
  return attr->getSet();
}

/// Returns the :packoffset() annotation on the given decl. Returns nullptr if
/// the decl does not have one.
hlsl::ConstantPacking *getPackOffset(const clang::NamedDecl *decl) {
  for (auto *annotation : decl->getUnusualAnnotations())
    if (auto *packing = llvm::dyn_cast<hlsl::ConstantPacking>(annotation))
      return packing;
  return nullptr;
}

/// Returns the number of binding numbers that are used up by the given type.
/// An array of size N consumes N*M binding numbers where M is the number of
/// binding numbers used by each array element.
/// The number of binding numbers used by a structure is the sum of binding
/// numbers used by its members.
uint32_t getNumBindingsUsedByResourceType(QualType type) {
  // For custom-generated types that have SpirvType but no QualType.
  if (type.isNull())
    return 1;

  // For every array dimension, the number of bindings needed should be
  // multiplied by the array size. For example: an array of two Textures should
  // use 2 binding slots.
  uint32_t arrayFactor = 1;
  while (auto constArrayType = dyn_cast<ConstantArrayType>(type)) {
    arrayFactor *=
        static_cast<uint32_t>(constArrayType->getSize().getZExtValue());
    type = constArrayType->getElementType();
  }

  // Once we remove the arrayness, we expect the given type to be either a
  // resource OR a structure that only contains resources.
  assert(isResourceType(type) || isResourceOnlyStructure(type));

  // In the case of a resource, each resource takes 1 binding slot, so in total
  // it consumes: 1 * arrayFactor.
  if (isResourceType(type))
    return arrayFactor;

  // In the case of a struct of resources, we need to sum up the number of
  // bindings for the struct members. So in total it consumes:
  //  sum(bindings of struct members) * arrayFactor.
  if (isResourceOnlyStructure(type)) {
    uint32_t sumOfMemberBindings = 0;
    const auto *structDecl = type->getAs<RecordType>()->getDecl();
    assert(structDecl);
    for (const auto *field : structDecl->fields())
      sumOfMemberBindings += getNumBindingsUsedByResourceType(field->getType());

    return sumOfMemberBindings * arrayFactor;
  }

  llvm_unreachable(
      "getNumBindingsUsedByResourceType was called with unknown resource type");
}

QualType getUintTypeWithSourceComponents(const ASTContext &astContext,
                                         QualType sourceType) {
  if (isScalarType(sourceType)) {
    return astContext.UnsignedIntTy;
  }
  uint32_t elemCount = 0;
  if (isVectorType(sourceType, nullptr, &elemCount)) {
    return astContext.getExtVectorType(astContext.UnsignedIntTy, elemCount);
  }
  llvm_unreachable("only scalar and vector types are supported in "
                   "getUintTypeWithSourceComponents");
}

LocationAndComponent getLocationAndComponentCount(const ASTContext &astContext,
                                                  QualType type) {
  // See Vulkan spec 14.1.4. Location Assignment for the complete set of rules.

  const auto canonicalType = type.getCanonicalType();
  if (canonicalType != type)
    return getLocationAndComponentCount(astContext, canonicalType);

  // Inputs and outputs of the following types consume a single interface
  // location:
  // * 16-bit scalar and vector types, and
  // * 32-bit scalar and vector types, and
  // * 64-bit scalar and 2-component vector types.

  // 64-bit three- and four- component vectors consume two consecutive
  // locations.

  // Primitive types
  if (isScalarType(type)) {
    const auto *builtinType = type->getAs<BuiltinType>();
    if (builtinType != nullptr) {
      switch (builtinType->getKind()) {
      case BuiltinType::Double:
      case BuiltinType::LongLong:
      case BuiltinType::ULongLong:
        return {1, 2, true};
      default:
        return {1, 1, false};
      }
    }
    return {1, 1, false};
  }

  // Vector types
  {
    QualType elemType = {};
    uint32_t elemCount = {};
    if (isVectorType(type, &elemType, &elemCount)) {
      const auto *builtinType = elemType->getAs<BuiltinType>();
      switch (builtinType->getKind()) {
      case BuiltinType::Double:
      case BuiltinType::LongLong:
      case BuiltinType::ULongLong: {
        if (elemCount >= 3)
          return {2, 4, true};
        return {1, 2 * elemCount, true};
      }
      default:
        // Filter switch only interested in types occupying 2 locations.
        break;
      }
      return {1, elemCount, false};
    }
  }

  // If the declared input or output is an n * m 16- , 32- or 64- bit matrix,
  // it will be assigned multiple locations starting with the location
  // specified. The number of locations assigned for each matrix will be the
  // same as for an n-element array of m-component vectors.

  // Matrix types
  {
    QualType elemType = {};
    uint32_t rowCount = 0, colCount = 0;
    if (isMxNMatrix(type, &elemType, &rowCount, &colCount)) {
      auto locComponentCount = getLocationAndComponentCount(
          astContext, astContext.getExtVectorType(elemType, colCount));
      return {locComponentCount.location * rowCount,
              locComponentCount.component,
              locComponentCount.componentAlignment};
    }
  }

  // Typedefs
  if (const auto *typedefType = type->getAs<TypedefType>())
    return getLocationAndComponentCount(astContext, typedefType->desugar());

  // Reference types
  if (const auto *refType = type->getAs<ReferenceType>())
    return getLocationAndComponentCount(astContext, refType->getPointeeType());

  // Pointer types
  if (const auto *ptrType = type->getAs<PointerType>())
    return getLocationAndComponentCount(astContext, ptrType->getPointeeType());

  // If a declared input or output is an array of size n and each element takes
  // m locations, it will be assigned m * n consecutive locations starting with
  // the location specified.

  // Array types
  if (const auto *arrayType = astContext.getAsConstantArrayType(type)) {
    auto locComponentCount =
        getLocationAndComponentCount(astContext, arrayType->getElementType());
    uint32_t arrayLength =
        static_cast<uint32_t>(arrayType->getSize().getZExtValue());
    return {locComponentCount.location * arrayLength,
            locComponentCount.component, locComponentCount.componentAlignment};
  }

  // Struct type
  if (type->getAs<RecordType>()) {
    assert(false && "all structs should already be flattened");
    return {0, 0, false};
  }

  llvm_unreachable(
      "calculating number of occupied locations for type unimplemented");
  return {0, 0, false};
}

bool shouldSkipInStructLayout(const Decl *decl) {
  // Ignore implicit generated struct declarations/constructors/destructors
  if (decl->isImplicit())
    return true;
  // Ignore embedded type decls
  if (isa<TypeDecl>(decl))
    return true;
  // Ignore embeded function decls
  if (isa<FunctionDecl>(decl))
    return true;
  // Ignore empty decls
  if (isa<EmptyDecl>(decl))
    return true;

  // For the $Globals cbuffer, we only care about externally-visible
  // non-resource-type variables. The rest should be filtered out.

  const auto *declContext = decl->getDeclContext();

  // $Globals' "struct" is the TranslationUnit, so we should ignore resources
  // in the TranslationUnit "struct" and its child namespaces.
  if (declContext->isTranslationUnit() || declContext->isNamespace()) {

    if (decl->hasAttr<VKConstantIdAttr>()) {
      return true;
    }

    if (decl->hasAttr<VKPushConstantAttr>()) {
      return true;
    }

    // External visibility
    if (const auto *declDecl = dyn_cast<DeclaratorDecl>(decl))
      if (!declDecl->hasExternalFormalLinkage())
        return true;

    // cbuffer/tbuffer
    if (isa<HLSLBufferDecl>(decl))
      return true;

    // 'groupshared' variables should not be placed in $Globals cbuffer.
    if (decl->hasAttr<HLSLGroupSharedAttr>())
      return true;

    // Other resource types
    if (const auto *valueDecl = dyn_cast<ValueDecl>(decl)) {
      const auto declType = valueDecl->getType();
      if (isResourceType(declType) || isResourceOnlyStructure(declType))
        return true;
    }
  }

  return false;
}

void collectDeclsInField(const Decl *field,
                         llvm::SmallVector<const Decl *, 4> *decls) {

  // Case of nested namespaces.
  if (const auto *nsDecl = dyn_cast<NamespaceDecl>(field)) {
    for (const auto *decl : nsDecl->decls()) {
      collectDeclsInField(decl, decls);
    }
  }

  if (shouldSkipInStructLayout(field))
    return;

  if (!isa<DeclaratorDecl>(field)) {
    return;
  }

  decls->push_back(field);
}

llvm::SmallVector<const Decl *, 4>
collectDeclsInDeclContext(const DeclContext *declContext) {
  llvm::SmallVector<const Decl *, 4> decls;
  for (const auto *field : declContext->decls()) {
    collectDeclsInField(field, &decls);
  }
  return decls;
}

/// \brief Returns true if the given decl is a boolean stage I/O variable.
/// Returns false if the type is not boolean, or the decl is a built-in stage
/// variable.
bool isBooleanStageIOVar(const NamedDecl *decl, QualType type,
                         const hlsl::DXIL::SemanticKind semanticKind,
                         const hlsl::SigPoint::Kind sigPointKind) {
  // [[vk::builtin(...)]] makes the decl a built-in stage variable.
  // IsFrontFace (if used as PSIn) is the only known boolean built-in stage
  // variable.
  bool isBooleanBuiltin = false;

  if ((decl->getAttr<VKBuiltInAttr>() != nullptr))
    isBooleanBuiltin = true;
  else if (semanticKind == hlsl::Semantic::Kind::IsFrontFace &&
           sigPointKind == hlsl::SigPoint::Kind::PSIn) {
    isBooleanBuiltin = true;
  } else if (semanticKind == hlsl::Semantic::Kind::CullPrimitive) {
    isBooleanBuiltin = true;
  }

  // TODO: support boolean matrix stage I/O variable if needed.
  QualType elemType = {};
  const bool isBooleanType =
      ((isScalarType(type, &elemType) || isVectorType(type, &elemType)) &&
       elemType->isBooleanType());

  return isBooleanType && !isBooleanBuiltin;
}

/// \brief Returns the stage variable's register assignment for the given Decl.
const hlsl::RegisterAssignment *getResourceBinding(const NamedDecl *decl) {
  for (auto *annotation : decl->getUnusualAnnotations()) {
    if (auto *reg = dyn_cast<hlsl::RegisterAssignment>(annotation)) {
      return reg;
    }
  }
  return nullptr;
}

/// \brief Returns the stage variable's 'register(c#) assignment for the given
/// Decl. Return nullptr if the given variable does not have such assignment.
const hlsl::RegisterAssignment *getRegisterCAssignment(const NamedDecl *decl) {
  const auto *regAssignment = getResourceBinding(decl);
  if (regAssignment)
    return regAssignment->RegisterType == 'c' ? regAssignment : nullptr;
  return nullptr;
}

/// \brief Returns true if the given declaration has a primitive type qualifier.
/// Returns false otherwise.
inline bool hasGSPrimitiveTypeQualifier(const Decl *decl) {
  return decl->hasAttr<HLSLTriangleAttr>() ||
         decl->hasAttr<HLSLTriangleAdjAttr>() ||
         decl->hasAttr<HLSLPointAttr>() || decl->hasAttr<HLSLLineAttr>() ||
         decl->hasAttr<HLSLLineAdjAttr>();
}

/// \brief Deduces the parameter qualifier for the given decl.
hlsl::DxilParamInputQual deduceParamQual(const DeclaratorDecl *decl,
                                         bool asInput) {
  const auto type = decl->getType();

  if (hlsl::IsHLSLInputPatchType(type))
    return hlsl::DxilParamInputQual::InputPatch;
  if (hlsl::IsHLSLOutputPatchType(type))
    return hlsl::DxilParamInputQual::OutputPatch;
  // TODO: Add support for multiple output streams.
  if (hlsl::IsHLSLStreamOutputType(type))
    return hlsl::DxilParamInputQual::OutStream0;

  // The inputs to the geometry shader that have a primitive type qualifier
  // must use 'InputPrimitive'.
  if (hasGSPrimitiveTypeQualifier(decl))
    return hlsl::DxilParamInputQual::InputPrimitive;

  if (decl->hasAttr<HLSLIndicesAttr>())
    return hlsl::DxilParamInputQual::OutIndices;
  if (decl->hasAttr<HLSLVerticesAttr>())
    return hlsl::DxilParamInputQual::OutVertices;
  if (decl->hasAttr<HLSLPrimitivesAttr>())
    return hlsl::DxilParamInputQual::OutPrimitives;
  if (decl->hasAttr<HLSLPayloadAttr>())
    return hlsl::DxilParamInputQual::InPayload;

  if (hlsl::IsHLSLNodeType(type)) {
    return hlsl::DxilParamInputQual::NodeIO;
  }

  return asInput ? hlsl::DxilParamInputQual::In : hlsl::DxilParamInputQual::Out;
}

/// \brief Deduces the HLSL SigPoint for the given decl appearing in the given
/// shader model.
const hlsl::SigPoint *deduceSigPoint(const DeclaratorDecl *decl, bool asInput,
                                     const hlsl::ShaderModel::Kind kind,
                                     bool forPCF) {
  if (kind == hlsl::ShaderModel::Kind::Node) {
    return hlsl::SigPoint::GetSigPoint(hlsl::SigPoint::Kind::CSIn);
  }
  return hlsl::SigPoint::GetSigPoint(hlsl::SigPointFromInputQual(
      deduceParamQual(decl, asInput), kind, forPCF));
}

/// Returns the type of the given decl. If the given decl is a FunctionDecl,
/// returns its result type.
inline QualType getTypeOrFnRetType(const DeclaratorDecl *decl) {
  if (const auto *funcDecl = dyn_cast<FunctionDecl>(decl)) {
    return funcDecl->getReturnType();
  }
  return decl->getType();
}

/// Returns the number of base classes if this type is a derived class/struct.
/// Returns zero otherwise.
inline uint32_t getNumBaseClasses(QualType type) {
  if (const auto *cxxDecl = type->getAsCXXRecordDecl())
    return cxxDecl->getNumBases();
  return 0;
}

/// Returns the appropriate storage class for an extern variable of the given
/// type.
spv::StorageClass getStorageClassForExternVar(QualType type,
                                              bool hasGroupsharedAttr) {
  // For CS groupshared variables
  if (hasGroupsharedAttr)
    return spv::StorageClass::Workgroup;

  if (isAKindOfStructuredOrByteBuffer(type) || isConstantTextureBuffer(type))
    return spv::StorageClass::Uniform;

  return spv::StorageClass::UniformConstant;
}

/// Returns the appropriate layout rule for an extern variable of the given
/// type.
SpirvLayoutRule getLayoutRuleForExternVar(QualType type,
                                          const SpirvCodeGenOptions &opts) {
  if (isAKindOfStructuredOrByteBuffer(type))
    return opts.sBufferLayoutRule;
  if (isConstantBuffer(type))
    return opts.cBufferLayoutRule;
  if (isTextureBuffer(type))
    return opts.tBufferLayoutRule;
  return SpirvLayoutRule::Void;
}

std::optional<spv::ImageFormat>
getSpvImageFormat(const VKImageFormatAttr *imageFormatAttr) {
  if (imageFormatAttr == nullptr)
    return std::nullopt;

  switch (imageFormatAttr->getImageFormat()) {
  case VKImageFormatAttr::unknown:
    return spv::ImageFormat::Unknown;
  case VKImageFormatAttr::rgba32f:
    return spv::ImageFormat::Rgba32f;
  case VKImageFormatAttr::rgba16f:
    return spv::ImageFormat::Rgba16f;
  case VKImageFormatAttr::r32f:
    return spv::ImageFormat::R32f;
  case VKImageFormatAttr::rgba8:
    return spv::ImageFormat::Rgba8;
  case VKImageFormatAttr::rgba8snorm:
    return spv::ImageFormat::Rgba8Snorm;
  case VKImageFormatAttr::rg32f:
    return spv::ImageFormat::Rg32f;
  case VKImageFormatAttr::rg16f:
    return spv::ImageFormat::Rg16f;
  case VKImageFormatAttr::r11g11b10f:
    return spv::ImageFormat::R11fG11fB10f;
  case VKImageFormatAttr::r16f:
    return spv::ImageFormat::R16f;
  case VKImageFormatAttr::rgba16:
    return spv::ImageFormat::Rgba16;
  case VKImageFormatAttr::rgb10a2:
    return spv::ImageFormat::Rgb10A2;
  case VKImageFormatAttr::rg16:
    return spv::ImageFormat::Rg16;
  case VKImageFormatAttr::rg8:
    return spv::ImageFormat::Rg8;
  case VKImageFormatAttr::r16:
    return spv::ImageFormat::R16;
  case VKImageFormatAttr::r8:
    return spv::ImageFormat::R8;
  case VKImageFormatAttr::rgba16snorm:
    return spv::ImageFormat::Rgba16Snorm;
  case VKImageFormatAttr::rg16snorm:
    return spv::ImageFormat::Rg16Snorm;
  case VKImageFormatAttr::rg8snorm:
    return spv::ImageFormat::Rg8Snorm;
  case VKImageFormatAttr::r16snorm:
    return spv::ImageFormat::R16Snorm;
  case VKImageFormatAttr::r8snorm:
    return spv::ImageFormat::R8Snorm;
  case VKImageFormatAttr::rgba32i:
    return spv::ImageFormat::Rgba32i;
  case VKImageFormatAttr::rgba16i:
    return spv::ImageFormat::Rgba16i;
  case VKImageFormatAttr::rgba8i:
    return spv::ImageFormat::Rgba8i;
  case VKImageFormatAttr::r32i:
    return spv::ImageFormat::R32i;
  case VKImageFormatAttr::rg32i:
    return spv::ImageFormat::Rg32i;
  case VKImageFormatAttr::rg16i:
    return spv::ImageFormat::Rg16i;
  case VKImageFormatAttr::rg8i:
    return spv::ImageFormat::Rg8i;
  case VKImageFormatAttr::r16i:
    return spv::ImageFormat::R16i;
  case VKImageFormatAttr::r8i:
    return spv::ImageFormat::R8i;
  case VKImageFormatAttr::rgba32ui:
    return spv::ImageFormat::Rgba32ui;
  case VKImageFormatAttr::rgba16ui:
    return spv::ImageFormat::Rgba16ui;
  case VKImageFormatAttr::rgba8ui:
    return spv::ImageFormat::Rgba8ui;
  case VKImageFormatAttr::r32ui:
    return spv::ImageFormat::R32ui;
  case VKImageFormatAttr::rgb10a2ui:
    return spv::ImageFormat::Rgb10a2ui;
  case VKImageFormatAttr::rg32ui:
    return spv::ImageFormat::Rg32ui;
  case VKImageFormatAttr::rg16ui:
    return spv::ImageFormat::Rg16ui;
  case VKImageFormatAttr::rg8ui:
    return spv::ImageFormat::Rg8ui;
  case VKImageFormatAttr::r16ui:
    return spv::ImageFormat::R16ui;
  case VKImageFormatAttr::r8ui:
    return spv::ImageFormat::R8ui;
  case VKImageFormatAttr::r64ui:
    return spv::ImageFormat::R64ui;
  case VKImageFormatAttr::r64i:
    return spv::ImageFormat::R64i;
  }
  return spv::ImageFormat::Unknown;
}

// Inserts seen semantics for entryPoint to seenSemanticsForEntryPoints. Returns
// whether it does not already exist in seenSemanticsForEntryPoints.
bool insertSeenSemanticsForEntryPointIfNotExist(
    llvm::SmallDenseMap<SpirvFunction *, llvm::StringSet<>>
        *seenSemanticsForEntryPoints,
    SpirvFunction *entryPoint, const std::string &semantics) {
  auto seenSemanticsForEntryPointsItr =
      seenSemanticsForEntryPoints->find(entryPoint);
  if (seenSemanticsForEntryPointsItr == seenSemanticsForEntryPoints->end()) {
    bool insertResult = false;
    std::tie(seenSemanticsForEntryPointsItr, insertResult) =
        seenSemanticsForEntryPoints->insert(
            std::make_pair(entryPoint, llvm::StringSet<>()));
    assert(insertResult);
    seenSemanticsForEntryPointsItr->second.insert(semantics);
    return true;
  }

  auto &seenSemantics = seenSemanticsForEntryPointsItr->second;
  if (seenSemantics.count(semantics)) {
    return false;
  }
  seenSemantics.insert(semantics);
  return true;
}

// Returns whether the type is translated to a 32-bit floating point type,
// depending on whether SPIR-V codegen options are configured to use 16-bit
// types when possible.
bool is32BitFloatingPointType(BuiltinType::Kind kind, bool use16Bit) {
  // Always translated into 32-bit floating point types.
  if (kind == BuiltinType::Float || kind == BuiltinType::LitFloat)
    return true;

  // Translated into 32-bit floating point types when run without
  // -enable-16bit-types.
  if (kind == BuiltinType::Half || kind == BuiltinType::HalfFloat ||
      kind == BuiltinType::Min10Float || kind == BuiltinType::Min16Float)
    return !use16Bit;

  return false;
}

// Returns whether the type is a 4-component 32-bit float or a composite type
// recursively including only such a vector e.g., float4, float4[1], struct S {
// float4 foo[1]; }.
bool containOnlyVecWithFourFloats(QualType type, bool use16Bit) {
  if (type->isReferenceType())
    type = type->getPointeeType();

  if (is1xNMatrix(type, nullptr, nullptr))
    return false;

  uint32_t elemCount = 0;
  if (type->isConstantArrayType()) {
    const ConstantArrayType *arrayType =
        (const ConstantArrayType *)type->getAsArrayTypeUnsafe();
    elemCount = hlsl::GetArraySize(type);
    return elemCount == 1 &&
           containOnlyVecWithFourFloats(arrayType->getElementType(), use16Bit);
  }

  if (const auto *structType = type->getAs<RecordType>()) {
    uint32_t fieldCount = 0;
    for (const auto *field : structType->getDecl()->fields()) {
      if (fieldCount != 0)
        return false;
      if (!containOnlyVecWithFourFloats(field->getType(), use16Bit))
        return false;
      ++fieldCount;
    }
    return fieldCount == 1;
  }

  QualType elemType = {};
  if (isVectorType(type, &elemType, &elemCount)) {
    if (const auto *builtinType = elemType->getAs<BuiltinType>()) {
      return elemCount == 4 &&
             is32BitFloatingPointType(builtinType->getKind(), use16Bit);
    }
    return false;
  }
  return false;
}

} // anonymous namespace

std::string StageVar::getSemanticStr() const {
  // A special case for zero index, which is equivalent to no index.
  // Use what is in the source code.
  // TODO: this looks like a hack to make the current tests happy.
  // Should consider remove it and fix all tests.
  if (semanticInfo.index == 0)
    return semanticInfo.str;

  std::ostringstream ss;
  ss << semanticInfo.name.str() << semanticInfo.index;
  return ss.str();
}

SpirvInstruction *CounterIdAliasPair::getAliasAddress() const {
  assert(isAlias);
  return counterVar;
}

SpirvInstruction *
CounterIdAliasPair::getCounterVariable(SpirvBuilder &builder,
                                       SpirvContext &spvContext) const {
  if (isAlias) {
    const auto *counterType = spvContext.getACSBufferCounterType();
    const auto *counterVarType =
        spvContext.getPointerType(counterType, spv::StorageClass::Uniform);
    return builder.createLoad(counterVarType, counterVar,
                              /* SourceLocation */ {});
  }
  return counterVar;
}

const CounterIdAliasPair *
CounterVarFields::get(const llvm::SmallVectorImpl<uint32_t> &indices) const {
  for (const auto &field : fields)
    if (field.indices == indices)
      return &field.counterVar;
  return nullptr;
}

bool CounterVarFields::assign(const CounterVarFields &srcFields,
                              SpirvBuilder &builder,
                              SpirvContext &context) const {
  for (const auto &field : fields) {
    const auto *srcField = srcFields.get(field.indices);
    if (!srcField)
      return false;

    field.counterVar.assign(srcField->getCounterVariable(builder, context),
                            builder);
  }

  return true;
}

bool CounterVarFields::assign(const CounterVarFields &srcFields,
                              const llvm::SmallVector<uint32_t, 4> &dstPrefix,
                              const llvm::SmallVector<uint32_t, 4> &srcPrefix,
                              SpirvBuilder &builder,
                              SpirvContext &context) const {
  if (dstPrefix.empty() && srcPrefix.empty())
    return assign(srcFields, builder, context);

  llvm::SmallVector<uint32_t, 4> srcIndices = srcPrefix;

  // If whole has the given prefix, appends all elements after the prefix in
  // whole to srcIndices.
  const auto applyDiff =
      [&srcIndices](const llvm::SmallVector<uint32_t, 4> &whole,
                    const llvm::SmallVector<uint32_t, 4> &prefix) -> bool {
    uint32_t i = 0;
    for (; i < prefix.size(); ++i)
      if (whole[i] != prefix[i]) {
        break;
      }
    if (i == prefix.size()) {
      for (; i < whole.size(); ++i)
        srcIndices.push_back(whole[i]);
      return true;
    }
    return false;
  };

  for (const auto &field : fields)
    if (applyDiff(field.indices, dstPrefix)) {
      const auto *srcField = srcFields.get(srcIndices);
      if (!srcField)
        return false;

      field.counterVar.assign(srcField->getCounterVariable(builder, context),
                              builder);
      for (uint32_t i = srcPrefix.size(); i < srcIndices.size(); ++i)
        srcIndices.pop_back();
    }

  return true;
}

SemanticInfo DeclResultIdMapper::getStageVarSemantic(const NamedDecl *decl) {
  for (auto *annotation : decl->getUnusualAnnotations()) {
    if (auto *sema = dyn_cast<hlsl::SemanticDecl>(annotation)) {
      llvm::StringRef semanticStr = sema->SemanticName;
      llvm::StringRef semanticName;
      uint32_t index = 0;
      hlsl::Semantic::DecomposeNameAndIndex(semanticStr, &semanticName, &index);
      const auto *semantic = hlsl::Semantic::GetByName(semanticName);
      return {semanticStr, semantic, semanticName, index, sema->Loc};
    }
  }
  return {};
}

bool DeclResultIdMapper::createStageOutputVar(const DeclaratorDecl *decl,
                                              SpirvInstruction *storedValue,
                                              bool forPCF) {
  QualType type = getTypeOrFnRetType(decl);
  uint32_t arraySize = 0;

  // Output stream types (PointStream, LineStream, TriangleStream) are
  // translated as their underlying struct types.
  if (hlsl::IsHLSLStreamOutputType(type))
    type = hlsl::GetHLSLResourceResultType(type);

  if (decl->hasAttr<HLSLIndicesAttr>() || decl->hasAttr<HLSLVerticesAttr>() ||
      decl->hasAttr<HLSLPrimitivesAttr>()) {
    const auto *typeDecl = astContext.getAsConstantArrayType(type);
    type = typeDecl->getElementType();
    arraySize = static_cast<uint32_t>(typeDecl->getSize().getZExtValue());
    if (decl->hasAttr<HLSLIndicesAttr>()) {
      // create SPIR-V builtin array PrimitiveIndicesNV of type
      // "uint [MaxPrimitiveCount * verticesPerPrim]"
      uint32_t verticesPerPrim = 1;
      if (!isVectorType(type, nullptr, &verticesPerPrim)) {
        assert(isScalarType(type));
      }

      spv::BuiltIn builtinID = spv::BuiltIn::Max;
      if (featureManager.isExtensionEnabled(Extension::EXT_mesh_shader)) {
        // For EXT_mesh_shader, set builtin type as
        // PrimitivePoint/Line/TriangleIndicesEXT based on the vertices per
        // primitive
        switch (verticesPerPrim) {
        case 1:
          builtinID = spv::BuiltIn::PrimitivePointIndicesEXT;
          break;
        case 2:
          builtinID = spv::BuiltIn::PrimitiveLineIndicesEXT;
          break;
        case 3:
          builtinID = spv::BuiltIn::PrimitiveTriangleIndicesEXT;
          break;
        default:
          break;
        }
        QualType arrayType = astContext.getConstantArrayType(
            type, llvm::APInt(32, arraySize), clang::ArrayType::Normal, 0);

        msOutIndicesBuiltin =
            getBuiltinVar(builtinID, arrayType, decl->getLocation());
      } else {
        // For NV_mesh_shader, the built type is PrimitiveIndicesNV
        builtinID = spv::BuiltIn::PrimitiveIndicesNV;

        arraySize = arraySize * verticesPerPrim;
        QualType arrayType = astContext.getConstantArrayType(
            astContext.UnsignedIntTy, llvm::APInt(32, arraySize),
            clang::ArrayType::Normal, 0);

        msOutIndicesBuiltin =
            getBuiltinVar(builtinID, arrayType, decl->getLocation());
      }

      return true;
    }
  }

  const auto *sigPoint = deduceSigPoint(
      decl, /*asInput=*/false, spvContext.getCurrentShaderModelKind(), forPCF);

  // HS output variables are created using the other overload. For the rest,
  // none of them should be created as arrays.
  assert(sigPoint->GetKind() != hlsl::DXIL::SigPointKind::HSCPOut);

  SemanticInfo inheritSemantic = {};

  // If storedValue is 0, it means this parameter in the original source code is
  // not used at all. Avoid writing back.
  //
  // Write back of stage output variables in GS is manually controlled by
  // .Append() intrinsic method, implemented in writeBackOutputStream(). So
  // ignoreValue should be set to true for GS.
  const bool noWriteBack =
      storedValue == nullptr || spvContext.isGS() || spvContext.isMS();

  StageVarDataBundle stageVarData = {
      decl, &inheritSemantic, false,     sigPoint,
      type, arraySize,        "out.var", llvm::None};
  return createStageVars(stageVarData, /*asInput=*/false, &storedValue,
                         noWriteBack);
}

bool DeclResultIdMapper::createStageOutputVar(const DeclaratorDecl *decl,
                                              uint32_t arraySize,
                                              SpirvInstruction *invocationId,
                                              SpirvInstruction *storedValue) {
  assert(spvContext.isHS());

  QualType type = getTypeOrFnRetType(decl);

  const auto *sigPoint =
      hlsl::SigPoint::GetSigPoint(hlsl::DXIL::SigPointKind::HSCPOut);

  SemanticInfo inheritSemantic = {};

  StageVarDataBundle stageVarData = {
      decl, &inheritSemantic, false,     sigPoint,
      type, arraySize,        "out.var", invocationId};
  return createStageVars(stageVarData, /*asInput=*/false, &storedValue,
                         /*noWriteBack=*/false);
}

bool DeclResultIdMapper::createStageInputVar(const ParmVarDecl *paramDecl,
                                             SpirvInstruction **loadedValue,
                                             bool forPCF) {
  uint32_t arraySize = 0;
  QualType type = paramDecl->getType();

  // Deprive the outermost arrayness for HS/DS/GS and use arraySize
  // to convey that information
  if (hlsl::IsHLSLInputPatchType(type)) {
    arraySize = hlsl::GetHLSLInputPatchCount(type);
    type = hlsl::GetHLSLInputPatchElementType(type);
  } else if (hlsl::IsHLSLOutputPatchType(type)) {
    arraySize = hlsl::GetHLSLOutputPatchCount(type);
    type = hlsl::GetHLSLOutputPatchElementType(type);
  }
  if (hasGSPrimitiveTypeQualifier(paramDecl)) {
    const auto *typeDecl = astContext.getAsConstantArrayType(type);
    arraySize = static_cast<uint32_t>(typeDecl->getSize().getZExtValue());
    type = typeDecl->getElementType();
  }

  const auto *sigPoint =
      deduceSigPoint(paramDecl, /*asInput=*/true,
                     spvContext.getCurrentShaderModelKind(), forPCF);

  SemanticInfo inheritSemantic = {};

  if (paramDecl->hasAttr<HLSLPayloadAttr>()) {
    spv::StorageClass sc =
        (featureManager.isExtensionEnabled(Extension::EXT_mesh_shader))
            ? spv::StorageClass::TaskPayloadWorkgroupEXT
            : getStorageClassForSigPoint(sigPoint);
    return createPayloadStageVars(sigPoint, sc, paramDecl, /*asInput=*/true,
                                  type, "in.var", loadedValue);
  } else {
    StageVarDataBundle stageVarData = {
        paramDecl,
        &inheritSemantic,
        paramDecl->hasAttr<HLSLNoInterpolationAttr>(),
        sigPoint,
        type,
        arraySize,
        "in.var",
        llvm::None};
    return createStageVars(stageVarData, /*asInput=*/true, loadedValue,
                           /*noWriteBack=*/false);
  }
}

const DeclResultIdMapper::DeclSpirvInfo *
DeclResultIdMapper::getDeclSpirvInfo(const ValueDecl *decl) const {
  auto it = astDecls.find(decl);
  if (it != astDecls.end())
    return &it->second;

  return nullptr;
}

SpirvInstruction *DeclResultIdMapper::getDeclEvalInfo(const ValueDecl *decl,
                                                      SourceLocation loc,
                                                      SourceRange range) {
  if (auto *builtinAttr = decl->getAttr<VKExtBuiltinInputAttr>()) {
    return getBuiltinVar(spv::BuiltIn(builtinAttr->getBuiltInID()),
                         decl->getType(), spv::StorageClass::Input, loc);
  } else if (auto *builtinAttr = decl->getAttr<VKExtBuiltinOutputAttr>()) {
    return getBuiltinVar(spv::BuiltIn(builtinAttr->getBuiltInID()),
                         decl->getType(), spv::StorageClass::Output, loc);
  }

  const DeclSpirvInfo *info = getDeclSpirvInfo(decl);

  // If DeclSpirvInfo is not found for this decl, it might be because it is an
  // implicit VarDecl. All implicit VarDecls are lazily created in order to
  // avoid creating large number of unused variables/constants/enums.
  if (!info) {
    tryToCreateImplicitConstVar(decl);
    info = getDeclSpirvInfo(decl);
  }

  if (info) {
    if (info->indexInCTBuffer >= 0) {
      // If this is a VarDecl inside a HLSLBufferDecl, we need to do an extra
      // OpAccessChain to get the pointer to the variable since we created
      // a single variable for the whole buffer object.

      // Should only have VarDecls in a HLSLBufferDecl.
      QualType valueType = cast<VarDecl>(decl)->getType();
      return spvBuilder.createAccessChain(
          valueType, info->instr,
          {spvBuilder.getConstantInt(
              astContext.IntTy, llvm::APInt(32, info->indexInCTBuffer, true))},
          loc, range);
    } else if (auto *type = info->instr->getResultType()) {
      const auto *ptrTy = dyn_cast<HybridPointerType>(type);

      // If it is a local variable or function parameter with a bindless
      // array of an opaque type, we have to load it because we pass a
      // pointer of a global variable that has the bindless opaque array.
      if (ptrTy != nullptr && isBindlessOpaqueArray(decl->getType())) {
        auto *load = spvBuilder.createLoad(ptrTy, info->instr, loc, range);
        load->setRValue(false);
        return load;
      } else {
        return *info;
      }
    } else {
      return *info;
    }
  }

  emitFatalError("found unregistered decl %0", decl->getLocation())
      << decl->getName();
  emitNote("please file a bug report on "
           "https://github.com/Microsoft/DirectXShaderCompiler/issues with "
           "source code if possible",
           {});
  return 0;
}

SpirvFunctionParameter *
DeclResultIdMapper::createFnParam(const ParmVarDecl *param,
                                  uint32_t dbgArgNumber) {
  const auto type = getTypeOrFnRetType(param);
  const auto loc = param->getLocation();
  const auto range = param->getSourceRange();
  const auto name = param->getName();
  SpirvFunctionParameter *fnParamInstr = spvBuilder.addFnParam(
      type, param->hasAttr<HLSLPreciseAttr>(),
      param->hasAttr<HLSLNoInterpolationAttr>(), loc, param->getName());
  bool isAlias = false;
  (void)getTypeAndCreateCounterForPotentialAliasVar(param, &isAlias);
  fnParamInstr->setContainsAliasComponent(isAlias);

  assert(astDecls[param].instr == nullptr);
  registerVariableForDecl(param, fnParamInstr);

  if (spirvOptions.debugInfoRich) {
    // Add DebugLocalVariable information
    const auto &sm = astContext.getSourceManager();
    const uint32_t line = sm.getPresumedLineNumber(loc);
    const uint32_t column = sm.getPresumedColumnNumber(loc);
    const auto *info = theEmitter.getOrCreateRichDebugInfo(loc);
    // TODO: replace this with FlagIsLocal enum.
    uint32_t flags = 1 << 2;
    auto *debugLocalVar = spvBuilder.createDebugLocalVariable(
        type, name, info->source, line, column, info->scopeStack.back(), flags,
        dbgArgNumber);
    spvBuilder.createDebugDeclare(debugLocalVar, fnParamInstr, loc, range);
  }

  return fnParamInstr;
}

void DeclResultIdMapper::createCounterVarForDecl(const DeclaratorDecl *decl) {
  const QualType declType = getTypeOrFnRetType(decl);

  if (!counterVars.count(decl) && isRWAppendConsumeSBuffer(declType)) {
    createCounterVar(decl, /*declId=*/0, /*isAlias=*/true);
  } else if (!fieldCounterVars.count(decl) && declType->isStructureType() &&
             // Exclude other resource types which are represented as structs
             !hlsl::IsHLSLResourceType(declType)) {
    createFieldCounterVars(decl);
  }
}

SpirvVariable *
DeclResultIdMapper::createFnVar(const VarDecl *var,
                                llvm::Optional<SpirvInstruction *> init) {
  if (astDecls[var].instr != nullptr)
    return cast<SpirvVariable>(astDecls[var].instr);

  const auto type = getTypeOrFnRetType(var);
  const auto loc = var->getLocation();
  const auto name = var->getName();
  const bool isPrecise = var->hasAttr<HLSLPreciseAttr>();
  const bool isNointerp = var->hasAttr<HLSLNoInterpolationAttr>();
  SpirvVariable *varInstr =
      spvBuilder.addFnVar(type, loc, name, isPrecise, isNointerp,
                          init.hasValue() ? init.getValue() : nullptr);

  bool isAlias = false;
  (void)getTypeAndCreateCounterForPotentialAliasVar(var, &isAlias);
  varInstr->setContainsAliasComponent(isAlias);
  registerVariableForDecl(var, varInstr);
  return varInstr;
}

SpirvDebugGlobalVariable *DeclResultIdMapper::createDebugGlobalVariable(
    SpirvVariable *var, const QualType &type, const SourceLocation &loc,
    const StringRef &name) {
  if (spirvOptions.debugInfoRich) {
    // Add DebugGlobalVariable information
    const auto &sm = astContext.getSourceManager();
    const uint32_t line = sm.getPresumedLineNumber(loc);
    const uint32_t column = sm.getPresumedColumnNumber(loc);
    const auto *info = theEmitter.getOrCreateRichDebugInfo(loc);
    // TODO: replace this with FlagIsDefinition enum.
    uint32_t flags = 1 << 3;
    // TODO: update linkageName correctly.
    auto *dbgGlobalVar = spvBuilder.createDebugGlobalVariable(
        type, name, info->source, line, column, info->scopeStack.back(),
        /* linkageName */ name, var, flags);
    dbgGlobalVar->setDebugSpirvType(var->getResultType());
    dbgGlobalVar->setLayoutRule(var->getLayoutRule());
    return dbgGlobalVar;
  }
  return nullptr;
}

SpirvVariable *
DeclResultIdMapper::createFileVar(const VarDecl *var,
                                  llvm::Optional<SpirvInstruction *> init) {
  // In the case of template specialization, the same VarDecl node in the AST
  // may be traversed more than once.
  if (astDecls[var].instr != nullptr) {
    return cast<SpirvVariable>(astDecls[var].instr);
  }

  const auto type = getTypeOrFnRetType(var);
  const auto loc = var->getLocation();
  const auto name = var->getName();
  SpirvVariable *varInstr = spvBuilder.addModuleVar(
      type, spv::StorageClass::Private, var->hasAttr<HLSLPreciseAttr>(),
      var->hasAttr<HLSLNoInterpolationAttr>(), name, init, loc);

  bool isAlias = false;
  (void)getTypeAndCreateCounterForPotentialAliasVar(var, &isAlias);
  varInstr->setContainsAliasComponent(isAlias);
  registerVariableForDecl(var, varInstr);

  createDebugGlobalVariable(varInstr, type, loc, name);

  return varInstr;
}

SpirvVariable *DeclResultIdMapper::createResourceHeap(const VarDecl *var,
                                                      QualType ResourceType) {
  QualType ResourceArrayType = astContext.getIncompleteArrayType(
      ResourceType, clang::ArrayType::Normal, 0);
  return createExternVar(var, ResourceArrayType);
}

SpirvVariable *DeclResultIdMapper::createExternVar(const VarDecl *var) {
  return createExternVar(var, var->getType());
}

SpirvVariable *DeclResultIdMapper::createExternVar(const VarDecl *var,
                                                   QualType type) {
  const bool isGroupShared = var->hasAttr<HLSLGroupSharedAttr>();
  const bool isACSBuffer =
      isAppendStructuredBuffer(type) || isConsumeStructuredBuffer(type);
  const bool isRWSBuffer = isRWStructuredBuffer(type);
  const auto storageClass = getStorageClassForExternVar(type, isGroupShared);
  const auto rule = getLayoutRuleForExternVar(type, spirvOptions);
  const auto loc = var->getLocation();

  if (!isGroupShared && !isResourceType(type) &&
      !isResourceOnlyStructure(type)) {

    // We currently cannot support global structures that contain both resources
    // and non-resources. That would require significant work in manipulating
    // structure field decls, manipulating QualTypes, as well as inserting
    // non-resources into the Globals cbuffer which changes offset decorations
    // for it.
    if (isStructureContainingMixOfResourcesAndNonResources(type)) {
      emitError("global structures containing both resources and non-resources "
                "are not supported",
                loc);
      return nullptr;
    }

    // This is a stand-alone externally-visiable non-resource-type variable.
    // They should be grouped into the $Globals cbuffer. We create that cbuffer
    // and record all variables inside it upon seeing the first such variable.
    if (astDecls.count(var) == 0)
      createGlobalsCBuffer(var);

    auto *varInstr = astDecls[var].instr;
    return varInstr ? cast<SpirvVariable>(varInstr) : nullptr;
  }

  if (isResourceOnlyStructure(type)) {
    // We currently do not support global structures that contain buffers.
    // Supporting global structures that contain buffers has two complications:
    //
    // 1- Buffers have the Uniform storage class, whereas Textures/Samplers have
    // UniformConstant storage class. As a result, if a struct contains both
    // textures and buffers, it is not clear what storage class should be used
    // for the struct. Also legalization cannot deduce the proper storage class
    // for struct members based on the structure's storage class.
    //
    // 2- Any kind of structured buffer has associated counters. The current DXC
    // code is not written in a way to place associated counters inside a
    // structure. Changing this behavior is non-trivial. There's also
    // significant work to be done both in DXC (to properly generate binding
    // numbers for the resource and its associated counters at correct offsets)
    // and in spirv-opt (to flatten such strcutures and modify the binding
    // numbers accordingly).
    if (isStructureContainingAnyKindOfBuffer(type)) {
      emitError("global structures containing buffers are not supported", loc);
      return nullptr;
    }

    needsFlatteningCompositeResources = true;
  }

  const auto name = var->getName();
  SpirvVariable *varInstr = spvBuilder.addModuleVar(
      type, storageClass, var->hasAttr<HLSLPreciseAttr>(),
      var->hasAttr<HLSLNoInterpolationAttr>(), name, llvm::None, loc);
  varInstr->setLayoutRule(rule);

  // If this variable has [[vk::combinedImageSampler]] and/or
  // [[vk::image_format("..")]] attributes, we have to keep the information in
  // the SpirvContext and use it when we lower the QualType to SpirvType.
  VkImageFeatures vkImgFeatures = {
      var->getAttr<VKCombinedImageSamplerAttr>() != nullptr,
      getSpvImageFormat(var->getAttr<VKImageFormatAttr>())};
  if (vkImgFeatures.format) {
    // Legalization is needed to propagate the correct image type for
    // instructions in addition to cases where the resource is assigned to
    // another variable or function parameter
    needsLegalization = true;
  }
  if (vkImgFeatures.isCombinedImageSampler || vkImgFeatures.format) {
    spvContext.registerVkImageFeaturesForSpvVariable(varInstr, vkImgFeatures);
  }

  if (const auto *recordType = type->getAs<RecordType>()) {
    StringRef typeName = recordType->getDecl()->getName();
    if (typeName.startswith("FeedbackTexture")) {
      emitError("Texture resource type '%0' is not supported with -spirv", loc)
          << typeName;
      return nullptr;
    }
  }

  if (hlsl::IsHLSLResourceType(type)) {
    if (!areFormatAndTypeCompatible(
            vkImgFeatures.format.value_or(spv::ImageFormat::Unknown),
            hlsl::GetHLSLResourceResultType(type))) {
      emitError("The image format and the sampled type are not compatible.\n"
                "For the table of compatible types, see "
                "https://docs.vulkan.org/spec/latest/appendices/"
                "spirvenv.html#spirvenv-format-type-matching.",
                loc);
      return nullptr;
    }
  }

  registerVariableForDecl(var, createDeclSpirvInfo(varInstr));

  createDebugGlobalVariable(varInstr, type, loc, name);

  // Variables in Workgroup do not need descriptor decorations.
  if (storageClass == spv::StorageClass::Workgroup)
    return varInstr;

  const auto *bindingAttr = var->getAttr<VKBindingAttr>();
  resourceVars.emplace_back(varInstr, var, loc, getResourceBinding(var),
                            bindingAttr, var->getAttr<VKCounterBindingAttr>());

  if (const auto *inputAttachment = var->getAttr<VKInputAttachmentIndexAttr>())
    spvBuilder.decorateInputAttachmentIndex(varInstr,
                                            inputAttachment->getIndex(), loc);

  if (isACSBuffer) {
    // For {Append|Consume}StructuredBuffer, we need to always create another
    // variable for its associated counter.
    createCounterVar(var, varInstr, /*isAlias=*/false);
  } else if (isRWSBuffer) {
    declRWSBuffers[var] = varInstr;
  }

  return varInstr;
}

SpirvInstruction *DeclResultIdMapper::createResultId(const VarDecl *var) {
  assert(isExtResultIdType(var->getType()));

  // Without initialization, we cannot generate the result id.
  if (!var->hasInit()) {
    emitError("Found uninitialized variable for result id.",
              var->getLocation());
    return nullptr;
  }

  SpirvInstruction *init = theEmitter.doExpr(var->getInit());
  registerVariableForDecl(var, createDeclSpirvInfo(init));
  return init;
}

SpirvInstruction *
DeclResultIdMapper::createOrUpdateStringVar(const VarDecl *var) {
  assert(hlsl::IsStringType(var->getType()) ||
         hlsl::IsStringLiteralType(var->getType()));

  // If the string variable is not initialized to a string literal, we cannot
  // generate an OpString for it.
  if (!var->hasInit()) {
    emitError("Found uninitialized string variable.", var->getLocation());
    return nullptr;
  }

  const StringLiteral *stringLiteral =
      dyn_cast<StringLiteral>(var->getInit()->IgnoreParenCasts());
  SpirvString *init = spvBuilder.getString(stringLiteral->getString());
  registerVariableForDecl(var, createDeclSpirvInfo(init));
  return init;
}

SpirvVariable *DeclResultIdMapper::createStructOrStructArrayVarOfExplicitLayout(
    const DeclContext *decl, llvm::ArrayRef<int> arraySize,
    const ContextUsageKind usageKind, llvm::StringRef typeName,
    llvm::StringRef varName) {
  // cbuffers are translated into OpTypeStruct with Block decoration.
  // tbuffers are translated into OpTypeStruct with BufferBlock decoration.
  // Push constants are translated into OpTypeStruct with Block decoration.
  //
  // Both cbuffers and tbuffers have the SPIR-V Uniform storage class.
  // Push constants have the SPIR-V PushConstant storage class.

  const bool forCBuffer = usageKind == ContextUsageKind::CBuffer;
  const bool forTBuffer = usageKind == ContextUsageKind::TBuffer;
  const bool forGlobals = usageKind == ContextUsageKind::Globals;
  const bool forPC = usageKind == ContextUsageKind::PushConstant;
  const bool forShaderRecordNV =
      usageKind == ContextUsageKind::ShaderRecordBufferNV;
  const bool forShaderRecordEXT =
      usageKind == ContextUsageKind::ShaderRecordBufferKHR;

  const auto &declGroup = collectDeclsInDeclContext(decl);

  // Collect the type and name for each field
  llvm::SmallVector<HybridStructType::FieldInfo, 4> fields;
  for (const auto *subDecl : declGroup) {
    // The field can only be FieldDecl (for normal structs) or VarDecl (for
    // HLSLBufferDecls).
    assert(isa<VarDecl>(subDecl) || isa<FieldDecl>(subDecl));
    const auto *declDecl = cast<DeclaratorDecl>(subDecl);
    auto varType = declDecl->getType();
    if (const auto *fieldVar = dyn_cast<VarDecl>(subDecl)) {

      // Static variables are not part of the struct from a layout perspective.
      // Thus, they should not be listed in the struct fields.
      if (fieldVar->getStorageClass() == StorageClass::SC_Static) {
        continue;
      }

      if (isResourceType(varType)) {
        createExternVar(fieldVar);
        continue;
      }
    }

    // In case 'register(c#)' annotation is placed on a global variable.
    const hlsl::RegisterAssignment *registerC =
        forGlobals ? getRegisterCAssignment(declDecl) : nullptr;

    llvm::Optional<BitfieldInfo> bitfieldInfo;
    {
      const FieldDecl *Field = dyn_cast<FieldDecl>(subDecl);
      if (Field && Field->isBitField()) {
        bitfieldInfo = BitfieldInfo();
        bitfieldInfo->sizeInBits =
            Field->getBitWidthValue(Field->getASTContext());
      }
    }

    // All fields are qualified with const. It will affect the debug name.
    // We don't need it here.
    varType.removeLocalConst();
    HybridStructType::FieldInfo info(
        varType, declDecl->getName(),
        /*vkoffset*/ declDecl->getAttr<VKOffsetAttr>(),
        /*packoffset*/ getPackOffset(declDecl),
        /*RegisterAssignment*/ registerC,
        /*isPrecise*/ declDecl->hasAttr<HLSLPreciseAttr>(),
        /*bitfield*/ bitfieldInfo);
    fields.push_back(info);
  }

  // Get the type for the whole struct
  // tbuffer/TextureBuffers are non-writable SSBOs.
  const SpirvType *resultType = spvContext.getHybridStructType(
      fields, typeName, /*isReadOnly*/ forTBuffer,
      forTBuffer ? StructInterfaceType::StorageBuffer
                 : StructInterfaceType::UniformBuffer);

  for (int size : arraySize) {
    if (size != -1) {
      resultType = spvContext.getArrayType(resultType, size,
                                           /*ArrayStride*/ llvm::None);
    } else {
      resultType = spvContext.getRuntimeArrayType(resultType,
                                                  /*ArrayStride*/ llvm::None);
    }
  }

  const auto sc = forPC               ? spv::StorageClass::PushConstant
                  : forShaderRecordNV ? spv::StorageClass::ShaderRecordBufferNV
                  : forShaderRecordEXT
                      ? spv::StorageClass::ShaderRecordBufferKHR
                      : spv::StorageClass::Uniform;

  // Create the variable for the whole struct / struct array.
  // The fields may be 'precise', but the structure itself is not.
  SpirvVariable *var = spvBuilder.addModuleVar(
      resultType, sc, /*isPrecise*/ false, /*isNoInterp*/ false, varName);

  const SpirvLayoutRule layoutRule =
      (forCBuffer || forGlobals)
          ? spirvOptions.cBufferLayoutRule
          : (forTBuffer ? spirvOptions.tBufferLayoutRule
                        : spirvOptions.sBufferLayoutRule);

  var->setHlslUserType(forCBuffer ? "cbuffer" : forTBuffer ? "tbuffer" : "");
  var->setLayoutRule(layoutRule);
  return var;
}

SpirvVariable *DeclResultIdMapper::createStructOrStructArrayVarOfExplicitLayout(
    const DeclContext *decl, int arraySize, const ContextUsageKind usageKind,
    llvm::StringRef typeName, llvm::StringRef varName) {
  llvm::SmallVector<int, 1> arraySizes;
  if (arraySize > 0)
    arraySizes.push_back(arraySize);

  return createStructOrStructArrayVarOfExplicitLayout(
      decl, arraySizes, usageKind, typeName, varName);
}

void DeclResultIdMapper::createEnumConstant(const EnumConstantDecl *decl) {
  const auto *valueDecl = dyn_cast<ValueDecl>(decl);
  const auto enumConstant =
      spvBuilder.getConstantInt(astContext.IntTy, decl->getInitVal());
  SpirvVariable *varInstr = spvBuilder.addModuleVar(
      astContext.IntTy, spv::StorageClass::Private, /*isPrecise*/ false, false,
      decl->getName(), enumConstant, decl->getLocation());
  astDecls[valueDecl] = createDeclSpirvInfo(varInstr);
}

SpirvVariable *DeclResultIdMapper::createCTBuffer(const HLSLBufferDecl *decl) {
  // This function handles creation of cbuffer or tbuffer.
  const auto usageKind =
      decl->isCBuffer() ? ContextUsageKind::CBuffer : ContextUsageKind::TBuffer;
  const std::string structName = "type." + decl->getName().str();
  // The front-end does not allow arrays of cbuffer/tbuffer.
  SpirvVariable *bufferVar = createStructOrStructArrayVarOfExplicitLayout(
      decl, /*arraySize*/ 0, usageKind, structName, decl->getName());

  // We still register all VarDecls seperately here. All the VarDecls are
  // mapped to the <result-id> of the buffer object, which means when querying
  // querying the <result-id> for a certain VarDecl, we need to do an extra
  // OpAccessChain.
  int index = 0;
  for (const auto *subDecl : decl->decls()) {
    if (shouldSkipInStructLayout(subDecl))
      continue;

    // If subDecl is a variable with resource type, we already added a separate
    // OpVariable for it in createStructOrStructArrayVarOfExplicitLayout().
    const auto *varDecl = cast<VarDecl>(subDecl);
    if (isResourceType(varDecl->getType()))
      continue;

    registerVariableForDecl(varDecl, createDeclSpirvInfo(bufferVar, index++));
  }
  // If it does not contains a member with non-resource type, we do not want to
  // set a dedicated binding number.
  if (index != 0) {
    resourceVars.emplace_back(
        bufferVar, decl, decl->getLocation(), getResourceBinding(decl),
        decl->getAttr<VKBindingAttr>(), decl->getAttr<VKCounterBindingAttr>());
  }

  if (!spirvOptions.debugInfoRich) {
    return bufferVar;
  }

  auto *dbgGlobalVar = createDebugGlobalVariable(
      bufferVar, QualType(), decl->getLocation(), decl->getName());
  assert(dbgGlobalVar);
  (void)dbgGlobalVar; // For NDEBUG builds.

  auto *resultType = bufferVar->getResultType();
  // Depending on the requested layout (DX or VK), constant buffers is either a
  // struct containing every constant fields, or a pointer to the type. This is
  // caused by the workaround we implemented to support FXC/DX layout. See #3672
  // for more details.
  assert(isa<SpirvPointerType>(resultType) ||
         isa<HybridStructType>(resultType));
  if (auto *ptr = dyn_cast<SpirvPointerType>(resultType))
    resultType = ptr->getPointeeType();
  // Debug type lowering requires the HLSLBufferDecl. Updating the type<>decl
  // mapping.
  spvContext.registerStructDeclForSpirvType(resultType, decl);

  return bufferVar;
}

SpirvVariable *DeclResultIdMapper::createPushConstant(const VarDecl *decl) {
  // The front-end errors out if non-struct type push constant is used.
  const QualType type = decl->getType();
  const auto *recordType = type->getAs<RecordType>();

  SpirvVariable *var = nullptr;

  if (isConstantBuffer(type)) {
    // Constant buffers already have Block decoration. The variable will need
    // the PushConstant storage class.

    // Create the variable for the whole struct / struct array.
    // The fields may be 'precise', but the structure itself is not.
    var = spvBuilder.addModuleVar(type, spv::StorageClass::PushConstant,
                                  /*isPrecise*/ false,
                                  /*isNoInterp*/ false, decl->getName());

    const SpirvLayoutRule layoutRule = spirvOptions.sBufferLayoutRule;

    var->setHlslUserType("");
    var->setLayoutRule(layoutRule);
  } else {
    assert(recordType);
    const std::string structName =
        "type.PushConstant." + recordType->getDecl()->getName().str();
    var = createStructOrStructArrayVarOfExplicitLayout(
        recordType->getDecl(), /*arraySize*/ 0, ContextUsageKind::PushConstant,
        structName, decl->getName());
  }

  // Register the VarDecl
  registerVariableForDecl(decl, createDeclSpirvInfo(var));

  // Do not push this variable into resourceVars since it does not need
  // descriptor set.

  return var;
}

SpirvVariable *
DeclResultIdMapper::createShaderRecordBuffer(const VarDecl *decl,
                                             ContextUsageKind kind) {
  const QualType type = decl->getType();
  const auto *recordType =
      hlsl::GetHLSLResourceResultType(type)->getAs<RecordType>();
  assert(recordType);

  assert(kind == ContextUsageKind::ShaderRecordBufferKHR ||
         kind == ContextUsageKind::ShaderRecordBufferNV);

  SpirvVariable *var = nullptr;
  if (isConstantBuffer(type)) {
    // Constant buffers already have Block decoration. The variable will need
    // the appropriate storage class.

    const auto sc = kind == ContextUsageKind::ShaderRecordBufferNV
                        ? spv::StorageClass::ShaderRecordBufferNV
                        : spv::StorageClass::ShaderRecordBufferKHR;

    // Create the variable for the whole struct / struct array.
    // The fields may be 'precise', but the structure itself is not.
    var = spvBuilder.addModuleVar(type, sc,
                                  /*isPrecise*/ false,
                                  /*isNoInterp*/ false, decl->getName());

    const SpirvLayoutRule layoutRule = spirvOptions.sBufferLayoutRule;

    var->setHlslUserType("");
    var->setLayoutRule(layoutRule);
  } else {
    const auto typeName = kind == ContextUsageKind::ShaderRecordBufferKHR
                              ? "type.ShaderRecordBufferKHR."
                              : "type.ShaderRecordBufferNV.";

    const std::string structName =
        typeName + recordType->getDecl()->getName().str();
    var = createStructOrStructArrayVarOfExplicitLayout(
        recordType->getDecl(), /*arraySize*/ 0, kind, structName,
        decl->getName());
  }

  // Register the VarDecl
  registerVariableForDecl(decl, createDeclSpirvInfo(var));

  // Do not push this variable into resourceVars since it does not need
  // descriptor set.

  return var;
}

SpirvVariable *
DeclResultIdMapper::createShaderRecordBuffer(const HLSLBufferDecl *decl,
                                             ContextUsageKind kind) {
  assert(kind == ContextUsageKind::ShaderRecordBufferKHR ||
         kind == ContextUsageKind::ShaderRecordBufferNV);

  const auto typeName = kind == ContextUsageKind::ShaderRecordBufferKHR
                            ? "type.ShaderRecordBufferKHR."
                            : "type.ShaderRecordBufferNV.";

  const std::string structName = typeName + decl->getName().str();
  // The front-end does not allow arrays of cbuffer/tbuffer.
  SpirvVariable *bufferVar = createStructOrStructArrayVarOfExplicitLayout(
      decl, /*arraySize*/ 0, kind, structName, decl->getName());

  // We still register all VarDecls seperately here. All the VarDecls are
  // mapped to the <result-id> of the buffer object, which means when
  // querying the <result-id> for a certain VarDecl, we need to do an extra
  // OpAccessChain.
  int index = 0;
  for (const auto *subDecl : decl->decls()) {
    if (shouldSkipInStructLayout(subDecl))
      continue;

    // If subDecl is a variable with resource type, we already added a separate
    // OpVariable for it in createStructOrStructArrayVarOfExplicitLayout().
    const auto *varDecl = cast<VarDecl>(subDecl);
    if (isResourceType(varDecl->getType()))
      continue;

    registerVariableForDecl(varDecl, createDeclSpirvInfo(bufferVar, index++));
  }
  return bufferVar;
}

void DeclResultIdMapper::recordsSpirvTypeAlias(const Decl *decl) {
  auto *typedefDecl = dyn_cast<TypedefNameDecl>(decl);
  if (!typedefDecl)
    return;

  if (!typedefDecl->hasAttr<VKCapabilityExtAttr>() &&
      !typedefDecl->hasAttr<VKExtensionExtAttr>())
    return;

  typeAliasesWithAttributes.push_back(typedefDecl);
}

void DeclResultIdMapper::createGlobalsCBuffer(const VarDecl *var) {
  if (astDecls.count(var) != 0)
    return;

  const auto *context = var->getTranslationUnitDecl();
  SpirvVariable *globals = createStructOrStructArrayVarOfExplicitLayout(
      context, /*arraySize*/ 0, ContextUsageKind::Globals, "type.$Globals",
      "$Globals");

  uint32_t index = 0;
  for (const auto *decl : collectDeclsInDeclContext(context)) {
    if (const auto *varDecl = dyn_cast<VarDecl>(decl)) {
      if (!spirvOptions.noWarnIgnoredFeatures) {
        if (const auto *init = varDecl->getInit())
          emitWarning(
              "variable '%0' will be placed in $Globals so initializer ignored",
              init->getExprLoc())
              << var->getName() << init->getSourceRange();
      }
      if (const auto *attr = varDecl->getAttr<VKBindingAttr>()) {
        emitError("variable '%0' will be placed in $Globals so cannot have "
                  "vk::binding attribute",
                  attr->getLocation())
            << var->getName();
        return;
      }

      // If subDecl is a variable with resource type, we already added a
      // separate OpVariable for it in
      // createStructOrStructArrayVarOfExplicitLayout().
      if (isResourceType(varDecl->getType()))
        continue;

      registerVariableForDecl(varDecl, createDeclSpirvInfo(globals, index++));
    }
  }

  // If it does not contains a member with non-resource type, we do not want to
  // set a dedicated binding number.
  if (index != 0) {
    resourceVars.emplace_back(globals, /*decl*/ nullptr, SourceLocation(),
                              nullptr, nullptr, nullptr, /*isCounterVar*/ false,
                              /*isGlobalsCBuffer*/ true);
  }
}

SpirvFunction *DeclResultIdMapper::getOrRegisterFn(const FunctionDecl *fn) {
  // Return it if it's already been created.
  auto it = astFunctionDecls.find(fn);
  if (it != astFunctionDecls.end()) {
    return it->second;
  }

  bool isAlias = false;
  (void)getTypeAndCreateCounterForPotentialAliasVar(fn, &isAlias);

  const bool isPrecise = fn->hasAttr<HLSLPreciseAttr>();
  const bool isNoInline = fn->hasAttr<NoInlineAttr>();
  // Note: we do not need to worry about function parameter types at this point
  // as this is used when function declarations are seen. When function
  // definition is seen, the parameter types will be set properly and take into
  // account whether the function is a member function of a class/struct (in
  // which case a 'this' parameter is added at the beginnig).
  SpirvFunction *spirvFunction = spvBuilder.createSpirvFunction(
      fn->getReturnType(), fn->getLocation(),
      getFunctionOrOperatorName(fn, true), isPrecise, isNoInline);

  if (fn->getAttr<HLSLExportAttr>()) {
    spvBuilder.decorateLinkage(nullptr, spirvFunction, fn->getName(),
                               spv::LinkageType::Export, fn->getLocation());
  }

  // No need to dereference to get the pointer. Function returns that are
  // stand-alone aliases are already pointers to values. All other cases should
  // be normal rvalues.
  if (!isAlias || !isAKindOfStructuredOrByteBuffer(fn->getReturnType()))
    spirvFunction->setRValue();

  spirvFunction->setConstainsAliasComponent(isAlias);

  astFunctionDecls[fn] = spirvFunction;
  return spirvFunction;
}

const CounterIdAliasPair *DeclResultIdMapper::getCounterIdAliasPair(
    const DeclaratorDecl *decl, const llvm::SmallVector<uint32_t, 4> *indices) {
  if (!decl)
    return nullptr;

  if (indices) {
    // Indices are provided. Walk through the fields of the decl.
    const auto counter = fieldCounterVars.find(decl);
    if (counter != fieldCounterVars.end())
      return counter->second.get(*indices);
  } else {
    // No indices. Check the stand-alone entities. If not found,
    // likely a deferred RWStructuredBuffer counter, so try
    // creating it now.
    auto counter = counterVars.find(decl);
    if (counter == counterVars.end()) {
      auto declInstr = declRWSBuffers[decl];
      if (declInstr) {
        createCounterVar(decl, declInstr, /*isAlias*/ false);
        counter = counterVars.find(decl);
      }
    }
    if (counter != counterVars.end())
      return &counter->second;
  }

  return nullptr;
}

const CounterIdAliasPair *
DeclResultIdMapper::createOrGetCounterIdAliasPair(const DeclaratorDecl *decl) {
  auto counterPair = getCounterIdAliasPair(decl);
  if (counterPair)
    return counterPair;
  if (!decl)
    return nullptr;
  // If deferred RWStructuredBuffer, try creating the counter now
  auto declInstr = declRWSBuffers[decl];
  if (declInstr) {
    createCounterVar(decl, declInstr, /*isAlias*/ false);
    auto counter = counterVars.find(decl);
    assert(counter != counterVars.end() && "counter not found");
    return &counter->second;
  }
  return nullptr;
}

const CounterVarFields *
DeclResultIdMapper::getCounterVarFields(const DeclaratorDecl *decl) {
  if (!decl)
    return nullptr;

  const auto found = fieldCounterVars.find(decl);
  if (found != fieldCounterVars.end())
    return &found->second;

  return nullptr;
}

void DeclResultIdMapper::registerSpecConstant(const VarDecl *decl,
                                              SpirvInstruction *specConstant) {
  specConstant->setRValue();
  registerVariableForDecl(decl, createDeclSpirvInfo(specConstant));
}

void DeclResultIdMapper::createCounterVar(
    const DeclaratorDecl *decl, SpirvInstruction *declInstr, bool isAlias,
    const llvm::SmallVector<uint32_t, 4> *indices) {
  std::string counterName = "counter.var." + decl->getName().str();
  if (indices) {
    // Append field indices to the name
    for (const auto index : *indices)
      counterName += "." + std::to_string(index);
  }

  const SpirvType *counterType = spvContext.getACSBufferCounterType();
  llvm::Optional<uint32_t> noArrayStride;
  QualType declType = decl->getType();
  if (declType->isArrayType()) {
    // Vulkan does not support multi-dimentional arrays of resource, so we
    // assume the array is a single dimensional array.
    assert(!declType->getArrayElementTypeNoTypeQual()->isArrayType());

    if (const auto *constArrayType =
            astContext.getAsConstantArrayType(declType)) {
      counterType = spvContext.getArrayType(
          counterType, constArrayType->getSize().getZExtValue(), noArrayStride);
    } else {
      assert(declType->isIncompleteArrayType());
      counterType = spvContext.getRuntimeArrayType(counterType, noArrayStride);
    }
  } else if (isResourceDescriptorHeap(decl->getType()) ||
             isSamplerDescriptorHeap(decl->getType())) {
    counterType = spvContext.getRuntimeArrayType(counterType, noArrayStride);
  }

  // {RW|Append|Consume}StructuredBuffer are all in Uniform storage class.
  // Alias counter variables should be created into the Private storage class.
  const spv::StorageClass sc =
      isAlias ? spv::StorageClass::Private : spv::StorageClass::Uniform;

  if (isAlias) {
    // Apply an extra level of pointer for alias counter variable
    counterType =
        spvContext.getPointerType(counterType, spv::StorageClass::Uniform);
  }

  SpirvVariable *counterInstr = spvBuilder.addModuleVar(
      counterType, sc, /*isPrecise*/ false, false, declInstr, counterName);

  if (!isAlias) {
    // Non-alias counter variables should be put in to resourceVars so that
    // descriptors can be allocated for them.
    resourceVars.emplace_back(counterInstr, decl, decl->getLocation(),
                              getResourceBinding(decl),
                              decl->getAttr<VKBindingAttr>(),
                              decl->getAttr<VKCounterBindingAttr>(), true);
    assert(declInstr);
    spvBuilder.decorateCounterBuffer(declInstr, counterInstr,
                                     decl->getLocation());
  }

  if (indices)
    fieldCounterVars[decl].append(*indices, counterInstr);
  else
    counterVars[decl] = {counterInstr, isAlias};
}

void DeclResultIdMapper::createFieldCounterVars(
    const DeclaratorDecl *rootDecl, const DeclaratorDecl *decl,
    llvm::SmallVector<uint32_t, 4> *indices) {
  const QualType type = getTypeOrFnRetType(decl);
  const auto *recordType = type->getAs<RecordType>();
  assert(recordType);
  const auto *recordDecl = recordType->getDecl();

  for (const auto *field : recordDecl->fields()) {
    // Build up the index chain
    indices->push_back(getNumBaseClasses(type) + field->getFieldIndex());

    const QualType fieldType = field->getType();
    if (isRWAppendConsumeSBuffer(fieldType))
      createCounterVar(rootDecl, /*declId=*/0, /*isAlias=*/true, indices);
    else if (fieldType->isStructureType() &&
             !hlsl::IsHLSLResourceType(fieldType))
      // Go recursively into all nested structs
      createFieldCounterVars(rootDecl, field, indices);

    indices->pop_back();
  }
}

std::vector<SpirvVariable *>
DeclResultIdMapper::collectStageVars(SpirvFunction *entryPoint) const {
  std::vector<SpirvVariable *> vars;

  for (auto var : glPerVertex.getStageInVars())
    vars.push_back(var);
  for (auto var : glPerVertex.getStageOutVars())
    vars.push_back(var);

  for (const auto &var : stageVars) {
    // We must collect stage variables that are included in entryPoint and stage
    // variables that are not included in any specific entryPoint i.e.,
    // var.getEntryPoint() is nullptr. Note that stage variables without any
    // specific entry point are common stage variables among all entry points.
    if (var.getEntryPoint() && var.getEntryPoint() != entryPoint)
      continue;
    auto *instr = var.getSpirvInstr();
    if (instr->getStorageClass() == spv::StorageClass::Private)
      continue;
    vars.push_back(instr);
  }

  return vars;
}

namespace {
/// A class for managing stage input/output locations to avoid duplicate uses of
/// the same location.
class LocationSet {
public:
  /// Maximum number of indices supported
  const static uint32_t kMaxIndex = 2;

  // Creates an empty set.
  LocationSet() {
    for (uint32_t i = 0; i < kMaxIndex; ++i) {
      // Default size. 64 should cover most cases without having to resize.
      usedLocations[i].resize(64);
      nextAvailableLocation[i] = 0;
    }
  }

  /// Marks a given location as used.
  void useLocation(uint32_t loc, uint32_t index = 0) {
    assert(index < kMaxIndex);

    auto &set = usedLocations[index];
    if (loc >= set.size()) {
      set.resize(std::max<size_t>(loc + 1, set.size() * 2));
    }
    set.set(loc);
    nextAvailableLocation[index] =
        std::max(loc + 1, nextAvailableLocation[index]);
  }

  // Find the first range of size |count| of unused locations,
  // and marks them as used.
  // Returns the first index of this range.
  int useNextNLocations(uint32_t count, uint32_t index = 0) {
    auto res = findUnusedRange(index, count);
    auto &locations = usedLocations[index];

    // Simple case: no hole large enough left, resizing.
    if (res == std::nullopt) {
      const uint32_t spaceLeft =
          locations.size() - nextAvailableLocation[index];
      assert(spaceLeft < count && "There is a bug.");

      const uint32_t requiredAlloc = count - spaceLeft;
      locations.resize(locations.size() + requiredAlloc);
      res = nextAvailableLocation[index];
    }

    for (uint32_t i = res.value(); i < res.value() + count; i++) {
      locations.set(i);
    }

    nextAvailableLocation[index] =
        std::max(res.value() + count, nextAvailableLocation[index]);
    return res.value();
  }

  /// Returns true if the given location number is already used.
  bool isLocationUsed(uint32_t loc, uint32_t index = 0) {
    assert(index < kMaxIndex);
    if (loc >= usedLocations[index].size())
      return false;
    return usedLocations[index][loc];
  }

private:
  // Find the first unused range of size |size| in the given set.
  // If the set contains such range, returns the first usable index.
  // Otherwise, nullopt is returned.
  std::optional<uint32_t> findUnusedRange(uint32_t index, uint32_t size) {
    if (size == 0) {
      return 0;
    }

    assert(index < kMaxIndex);
    const auto &locations = usedLocations[index];

    uint32_t required_size = size;
    uint32_t start = 0;
    for (uint32_t i = 0; i < locations.size() && required_size > 0; i++) {
      if (!locations[i]) {
        --required_size;
        continue;
      }

      required_size = size;
      start = i + 1;
    }

    return required_size == 0 ? std::optional(start) : std::nullopt;
  }

  // The sets to remember used locations. A set bit means the location is used.
  /// All previously used locations
  llvm::SmallBitVector usedLocations[kMaxIndex];

  // The position of the last bit set in the usedLocation vector.
  uint32_t nextAvailableLocation[kMaxIndex];
};

} // namespace

/// A class for managing resource bindings to avoid duplicate uses of the same
/// set and binding number.
class DeclResultIdMapper::BindingSet {
public:
  /// Uses the given set and binding number. Returns false if the binding number
  /// was already occupied in the set, and returns true otherwise.
  bool useBinding(uint32_t binding, uint32_t set) {
    bool inserted = false;
    std::tie(std::ignore, inserted) = usedBindings[set].insert(binding);
    return inserted;
  }

  /// Uses the next available binding number in |set|. If more than one binding
  /// number is to be occupied, it finds the next available chunk that can fit
  /// |numBindingsToUse| in the |set|.
  uint32_t useNextBinding(uint32_t set, uint32_t numBindingsToUse = 1,
                          uint32_t bindingShift = 0) {
    uint32_t bindingNoStart =
        getNextBindingChunk(set, numBindingsToUse, bindingShift);
    auto &binding = usedBindings[set];
    for (uint32_t i = 0; i < numBindingsToUse; ++i)
      binding.insert(bindingNoStart + i);
    return bindingNoStart;
  }

  /// Returns the first available binding number in the |set| for which |n|
  /// consecutive binding numbers are unused starting at |bindingShift|.
  uint32_t getNextBindingChunk(uint32_t set, uint32_t n,
                               uint32_t bindingShift) {
    auto &existingBindings = usedBindings[set];

    // There were no bindings in this set. Can start at binding zero.
    if (existingBindings.empty())
      return bindingShift;

    // Check whether the chunk of |n| binding numbers can be fitted at the
    // very beginning of the list (start at binding 0 in the current set).
    uint32_t curBinding = *existingBindings.begin();
    if (curBinding >= (n + bindingShift))
      return bindingShift;

    auto iter = std::next(existingBindings.begin());
    while (iter != existingBindings.end()) {
      // There exists a next binding number that is used. Check to see if the
      // gap between current binding number and next binding number is large
      // enough to accommodate |n|.
      uint32_t nextBinding = *iter;
      if ((bindingShift > 0) && (curBinding < (bindingShift - 1)))
        curBinding = bindingShift - 1;

      if (curBinding < nextBinding && n <= nextBinding - curBinding - 1)
        return curBinding + 1;

      curBinding = nextBinding;

      // Peek at the next binding that has already been used (if any).
      ++iter;
    }

    // |curBinding| was the last binding that was used in this set. The next
    // chunk of |n| bindings can start at |curBinding|+1.
    return std::max(curBinding + 1, bindingShift);
  }

private:
  ///< set number -> set of used binding number
  llvm::DenseMap<uint32_t, std::set<uint32_t>> usedBindings;
};

bool DeclResultIdMapper::checkSemanticDuplication(bool forInput) {
  // Mapping from entry points to the corresponding set of semantics.
  llvm::SmallDenseMap<SpirvFunction *, llvm::StringSet<>>
      seenSemanticsForEntryPoints;
  bool success = true;
  for (const auto &var : stageVars) {
    auto s = var.getSemanticStr();

    if (s.empty()) {
      // We translate WaveGetLaneCount(), WaveGetLaneIndex() and 'payload' param
      // block declaration into builtin variables. Those variables are inserted
      // into the normal stage IO processing pipeline, but with the semantics as
      // empty strings.
      assert(var.isSpirvBuitin());
      continue;
    }

    if (forInput && var.getSigPoint()->IsInput()) {
      bool insertionSuccess = insertSeenSemanticsForEntryPointIfNotExist(
          &seenSemanticsForEntryPoints, var.getEntryPoint(), s);
      if (!insertionSuccess) {
        emitError("input semantic '%0' used more than once",
                  var.getSemanticInfo().loc)
            << s;
        success = false;
      }
    } else if (!forInput && var.getSigPoint()->IsOutput()) {
      bool insertionSuccess = insertSeenSemanticsForEntryPointIfNotExist(
          &seenSemanticsForEntryPoints, var.getEntryPoint(), s);
      if (!insertionSuccess) {
        emitError("output semantic '%0' used more than once",
                  var.getSemanticInfo().loc)
            << s;
        success = false;
      }
    }
  }

  return success;
}

bool DeclResultIdMapper::isDuplicatedStageVarLocation(
    llvm::DenseSet<StageVariableLocationInfo, StageVariableLocationInfo>
        *stageVariableLocationInfo,
    const StageVar &var, uint32_t location, uint32_t index) {
  if (!stageVariableLocationInfo
           ->insert({var.getEntryPoint(),
                     var.getSpirvInstr()->getStorageClass(), location, index})
           .second) {
    emitError("Multiple stage variables have a duplicated pair of "
              "location and index at %0 / %1",
              var.getSpirvInstr()->getSourceLocation())
        << location << index;
    return false;
  }
  return true;
}

bool DeclResultIdMapper::assignLocations(
    const std::vector<const StageVar *> &vars,
    llvm::function_ref<uint32_t(uint32_t)> nextLocs,
    llvm::DenseSet<StageVariableLocationInfo, StageVariableLocationInfo>
        *stageVariableLocationInfo) {
  for (const auto *var : vars) {
    if (hlsl::IsHLSLNodeType(var->getAstType()))
      continue;
    auto locCount = var->getLocationCount();
    uint32_t location = nextLocs(locCount);
    spvBuilder.decorateLocation(var->getSpirvInstr(), location);

    if (!isDuplicatedStageVarLocation(stageVariableLocationInfo, *var, location,
                                      0)) {
      return false;
    }
  }
  return true;
}

bool DeclResultIdMapper::finalizeStageIOLocationsForASingleEntryPoint(
    bool forInput, ArrayRef<StageVar> functionStageVars) {
  // Returns false if the given StageVar is an input/output variable without
  // explicit location assignment. Otherwise, returns true.
  const auto locAssigned = [forInput, this](const StageVar &v) {
    if (forInput == isInputStorageClass(v)) {
      // No need to assign location for builtins. Treat as assigned.
      return v.isSpirvBuitin() || v.hasLocOrBuiltinDecorateAttr() ||
             v.getLocationAttr() != nullptr;
    }
    // For the ones we don't care, treat as assigned.
    return true;
  };

  /// Set of locations of assigned stage variables used to correctly report
  /// duplicated stage variable locations.
  llvm::DenseSet<StageVariableLocationInfo, StageVariableLocationInfo>
      stageVariableLocationInfo;

  // If we have explicit location specified for all input/output variables,
  // use them instead assign by ourselves.
  if (std::all_of(functionStageVars.begin(), functionStageVars.end(),
                  locAssigned)) {
    LocationSet locSet;
    bool noError = true;

    for (const auto &var : functionStageVars) {
      // Skip builtins & those stage variables we are not handling for this call
      if (var.isSpirvBuitin() || var.hasLocOrBuiltinDecorateAttr() ||
          forInput != isInputStorageClass(var)) {
        continue;
      }

      const auto *attr = var.getLocationAttr();
      const auto loc = attr->getNumber();
      const auto locCount = var.getLocationCount();
      const auto attrLoc = attr->getLocation(); // Attr source code location
      const auto idx = var.getIndexAttr() ? var.getIndexAttr()->getNumber() : 0;

      // Make sure the same location is not assigned more than once
      for (uint32_t l = loc; l < loc + locCount; ++l) {
        if (locSet.isLocationUsed(l, idx)) {
          emitError("stage %select{output|input}0 location #%1 already "
                    "consumed by semantic '%2'",
                    attrLoc)
              << forInput << l << functionStageVars[idx].getSemanticStr();
          noError = false;
        }

        locSet.useLocation(l, idx);
      }

      spvBuilder.decorateLocation(var.getSpirvInstr(), loc);
      if (var.getIndexAttr())
        spvBuilder.decorateIndex(var.getSpirvInstr(), idx,
                                 var.getSemanticInfo().loc);

      if (!isDuplicatedStageVarLocation(&stageVariableLocationInfo, var, loc,
                                        idx)) {
        return false;
      }
    }

    return noError;
  }

  std::vector<const StageVar *> vars;
  LocationSet locSet;

  for (const auto &var : functionStageVars) {
    if (var.isSpirvBuitin() || var.hasLocOrBuiltinDecorateAttr() ||
        forInput != isInputStorageClass(var)) {
      continue;
    }

    if (var.getLocationAttr()) {
      // We have checked that not all of the stage variables have explicit
      // location assignment.
      emitError("partial explicit stage %select{output|input}0 location "
                "assignment via vk::location(X) unsupported",
                {})
          << forInput;
      return false;
    }

    const auto &semaInfo = var.getSemanticInfo();

    // We should special rules for SV_Target: the location number comes from the
    // semantic string index.
    if (semaInfo.isTarget()) {
      spvBuilder.decorateLocation(var.getSpirvInstr(), semaInfo.index);
      locSet.useLocation(semaInfo.index);

      if (!isDuplicatedStageVarLocation(&stageVariableLocationInfo, var,
                                        semaInfo.index, 0)) {
        return false;
      }
    } else {
      vars.push_back(&var);
    }
  }

  if (vars.empty())
    return true;

  auto nextLocs = [&locSet](uint32_t locCount) {
    return locSet.useNextNLocations(locCount);
  };

  // If alphabetical ordering was requested, sort by semantic string.
  if (spirvOptions.stageIoOrder == "alpha") {
    // Sort stage input/output variables alphabetically
    std::stable_sort(vars.begin(), vars.end(),
                     [](const StageVar *a, const StageVar *b) {
                       return a->getSemanticStr() < b->getSemanticStr();
                     });
    return assignLocations(vars, nextLocs, &stageVariableLocationInfo);
  }

  // Pack signature if it is enabled. Vertext shader input and pixel
  // shader output are special. We have to preserve the given signature.
  auto sigPointKind = vars[0]->getSigPoint()->GetKind();
  if (spirvOptions.signaturePacking &&
      sigPointKind != hlsl::SigPoint::Kind::VSIn &&
      sigPointKind != hlsl::SigPoint::Kind::PSOut) {
    return packSignature(spvBuilder, vars, nextLocs, forInput);
  }

  // Since HS includes 2 sets of outputs (patch-constant output and
  // OutputPatch), running into location mismatches between HS and DS is very
  // likely. In order to avoid location mismatches between HS and DS, use
  // alphabetical ordering.
  if ((!forInput && spvContext.isHS()) || (forInput && spvContext.isDS())) {
    // Sort stage input/output variables alphabetically
    std::stable_sort(vars.begin(), vars.end(),
                     [](const StageVar *a, const StageVar *b) {
                       return a->getSemanticStr() < b->getSemanticStr();
                     });
  }
  return assignLocations(vars, nextLocs, &stageVariableLocationInfo);
}

llvm::DenseMap<const SpirvFunction *, SmallVector<StageVar, 8>>
DeclResultIdMapper::getStageVarsPerFunction() {
  llvm::DenseMap<const SpirvFunction *, SmallVector<StageVar, 8>> result;
  for (const auto &var : stageVars) {
    result[var.getEntryPoint()].push_back(var);
  }
  return result;
}

bool DeclResultIdMapper::finalizeStageIOLocations(bool forInput) {
  if (!checkSemanticDuplication(forInput))
    return false;

  auto stageVarPerFunction = getStageVarsPerFunction();
  for (const auto &functionStageVars : stageVarPerFunction) {
    if (!finalizeStageIOLocationsForASingleEntryPoint(
            forInput, functionStageVars.getSecond())) {
      return false;
    }
  }
  return true;
}

namespace {
/// A class for maintaining the binding number shift requested for descriptor
/// sets.
class BindingShiftMapper {
public:
  explicit BindingShiftMapper(const llvm::SmallVectorImpl<int32_t> &shifts)
      : masterShift(0) {
    assert(shifts.size() % 2 == 0);
    if (shifts.size() == 2 && shifts[1] == -1) {
      masterShift = shifts[0];
    } else {
      for (uint32_t i = 0; i < shifts.size(); i += 2)
        perSetShift[shifts[i + 1]] = shifts[i];
    }
  }

  /// Returns the shift amount for the given set.
  int32_t getShiftForSet(int32_t set) const {
    const auto found = perSetShift.find(set);
    if (found != perSetShift.end())
      return found->second;
    return masterShift;
  }

private:
  uint32_t masterShift; /// Shift amount applies to all sets.
  llvm::DenseMap<int32_t, int32_t> perSetShift;
};

/// A class for maintaining the mapping from source code register attributes to
/// descriptor set and number settings.
class RegisterBindingMapper {
public:
  /// Takes in the relation between register attributes and descriptor settings.
  /// Each relation is represented by four strings:
  ///   <register-type-number> <space> <descriptor-binding> <set>
  bool takeInRelation(const std::vector<std::string> &relation,
                      std::string *error) {
    assert(relation.size() % 4 == 0);
    mapping.clear();

    for (uint32_t i = 0; i < relation.size(); i += 4) {
      int32_t spaceNo = -1, setNo = -1, bindNo = -1;
      if (StringRef(relation[i + 1]).getAsInteger(10, spaceNo) || spaceNo < 0) {
        *error = "space number: " + relation[i + 1];
        return false;
      }
      if (StringRef(relation[i + 2]).getAsInteger(10, bindNo) || bindNo < 0) {
        *error = "binding number: " + relation[i + 2];
        return false;
      }
      if (StringRef(relation[i + 3]).getAsInteger(10, setNo) || setNo < 0) {
        *error = "set number: " + relation[i + 3];
        return false;
      }
      mapping[relation[i + 1] + relation[i]] = std::make_pair(setNo, bindNo);
    }
    return true;
  }

  /// Returns true and set the correct set and binding number if we can find a
  /// descriptor setting for the given register. False otherwise.
  bool getSetBinding(const hlsl::RegisterAssignment *regAttr,
                     uint32_t defaultSpace, int *setNo, int *bindNo) const {
    std::ostringstream iss;
    iss << regAttr->RegisterSpace.getValueOr(defaultSpace)
        << regAttr->RegisterType << regAttr->RegisterNumber;

    auto found = mapping.find(iss.str());
    if (found != mapping.end()) {
      *setNo = found->second.first;
      *bindNo = found->second.second;
      return true;
    }

    return false;
  }

private:
  llvm::StringMap<std::pair<int, int>> mapping;
};
} // namespace

bool DeclResultIdMapper::decorateResourceBindings() {
  // For normal resource, we support 4 approaches of setting binding numbers:
  // - m1: [[vk::binding(...)]]
  // - m2: :register(xX, spaceY)
  // - m3: None
  // - m4: :register(spaceY)
  //
  // For associated counters, we support 2 approaches:
  // - c1: [[vk::counter_binding(...)]
  // - c2: None
  //
  // In combination, we need to handle 12 cases:
  // - 4 cases for nomral resoures (m1, m2, m3, m4)
  // - 8 cases for associated counters (mX * cY)
  //
  // In the following order:
  // - m1, mX * c1
  // - m2
  // - m3, m4, mX * c2

  // The "-auto-binding-space" command line option can be used to specify a
  // certain space as default. UINT_MAX means the user has not provided this
  // option. If not provided, the SPIR-V backend uses space "0" as default.
  auto defaultSpaceOpt =
      theEmitter.getCompilerInstance().getCodeGenOpts().HLSLDefaultSpace;
  uint32_t defaultSpace = (defaultSpaceOpt == UINT_MAX) ? 0 : defaultSpaceOpt;

  const bool bindGlobals = !spirvOptions.bindGlobals.empty();
  int32_t globalsBindNo = -1, globalsSetNo = -1;
  if (bindGlobals) {
    assert(spirvOptions.bindGlobals.size() == 2);
    if (StringRef(spirvOptions.bindGlobals[0])
            .getAsInteger(10, globalsBindNo) ||
        globalsBindNo < 0) {
      emitError("invalid -fvk-bind-globals binding number: %0", {})
          << spirvOptions.bindGlobals[0];
      return false;
    }
    if (StringRef(spirvOptions.bindGlobals[1]).getAsInteger(10, globalsSetNo) ||
        globalsSetNo < 0) {
      emitError("invalid -fvk-bind-globals set number: %0", {})
          << spirvOptions.bindGlobals[1];
      return false;
    }
  }

  // Special handling of -fvk-bind-register, which requires
  // * All resources are annoated with :register() in the source code
  // * -fvk-bind-register is specified for every resource
  if (!spirvOptions.bindRegister.empty()) {
    RegisterBindingMapper bindingMapper;
    std::string error;

    if (!bindingMapper.takeInRelation(spirvOptions.bindRegister, &error)) {
      emitError("invalid -fvk-bind-register %0", {}) << error;
      return false;
    }

    for (const auto &var : resourceVars)
      if (const auto *regAttr = var.getRegister()) {
        if (var.isCounter()) {
          emitError("-fvk-bind-register for RW/Append/Consume StructuredBuffer "
                    "unimplemented",
                    var.getSourceLocation());
        } else {
          int setNo = 0, bindNo = 0;
          if (!bindingMapper.getSetBinding(regAttr, defaultSpace, &setNo,
                                           &bindNo)) {
            emitError("missing -fvk-bind-register for resource",
                      var.getSourceLocation());
            return false;
          }
          spvBuilder.decorateDSetBinding(var.getSpirvInstr(), setNo, bindNo);
        }
      } else if (var.isGlobalsBuffer()) {
        if (!bindGlobals) {
          emitError("-fvk-bind-register requires Globals buffer to be bound "
                    "with -fvk-bind-globals",
                    var.getSourceLocation());
          return false;
        }

        spvBuilder.decorateDSetBinding(var.getSpirvInstr(), globalsSetNo,
                                       globalsBindNo);
      } else {
        emitError(
            "-fvk-bind-register requires register annotations on all resources",
            var.getSourceLocation());
        return false;
      }

    return true;
  }

  BindingSet bindingSet;

  // If some bindings are reserved for heaps, mark those are used.
  if (spirvOptions.resourceHeapBinding)
    bindingSet.useBinding(spirvOptions.resourceHeapBinding->binding,
                          spirvOptions.resourceHeapBinding->set);
  if (spirvOptions.samplerHeapBinding)
    bindingSet.useBinding(spirvOptions.samplerHeapBinding->binding,
                          spirvOptions.samplerHeapBinding->set);
  if (spirvOptions.counterHeapBinding)
    bindingSet.useBinding(spirvOptions.counterHeapBinding->binding,
                          spirvOptions.counterHeapBinding->set);

  // Decorates the given varId of the given category with set number
  // setNo, binding number bindingNo. Ignores overlaps.
  const auto tryToDecorate = [this, &bindingSet](const ResourceVar &var,
                                                 const uint32_t setNo,
                                                 const uint32_t bindingNo) {
    // By default we use one binding number per resource, and an array of
    // resources also gets only one binding number. However, for array of
    // resources (e.g. array of textures), DX uses one binding number per array
    // element. We can match this behavior via a command line option.
    uint32_t numBindingsToUse = 1;
    if (spirvOptions.flattenResourceArrays || needsFlatteningCompositeResources)
      numBindingsToUse = getNumBindingsUsedByResourceType(
          var.getSpirvInstr()->getAstResultType());

    for (uint32_t i = 0; i < numBindingsToUse; ++i) {
      bool success = bindingSet.useBinding(bindingNo + i, setNo);
      // We will not emit an error if we find a set/binding overlap because it
      // is possible that the optimizer optimizes away a resource which resolves
      // the overlap.
      (void)success;
    }

    // No need to decorate multiple binding numbers for arrays. It will be done
    // by legalization/optimization.
    spvBuilder.decorateDSetBinding(var.getSpirvInstr(), setNo, bindingNo);
  };

  for (const auto &var : resourceVars) {
    if (var.isCounter()) {
      if (const auto *vkCBinding = var.getCounterBinding()) {
        // Process mX * c1
        uint32_t set = defaultSpace;
        if (const auto *vkBinding = var.getBinding())
          set = getVkBindingAttrSet(vkBinding, defaultSpace);
        else if (const auto *reg = var.getRegister())
          set = reg->RegisterSpace.getValueOr(defaultSpace);

        tryToDecorate(var, set, vkCBinding->getBinding());
      }
    } else {
      if (const auto *vkBinding = var.getBinding()) {
        // Process m1
        tryToDecorate(var, getVkBindingAttrSet(vkBinding, defaultSpace),
                      vkBinding->getBinding());
      }
    }
  }

  BindingShiftMapper bShiftMapper(spirvOptions.bShift);
  BindingShiftMapper tShiftMapper(spirvOptions.tShift);
  BindingShiftMapper sShiftMapper(spirvOptions.sShift);
  BindingShiftMapper uShiftMapper(spirvOptions.uShift);

  // Process m2
  for (const auto &var : resourceVars)
    if (!var.isCounter() && !var.getBinding())
      if (const auto *reg = var.getRegister()) {
        // Skip space-only register() annotations
        if (reg->isSpaceOnly())
          continue;

        const uint32_t set = reg->RegisterSpace.getValueOr(defaultSpace);
        uint32_t binding = reg->RegisterNumber;
        switch (reg->RegisterType) {
        case 'b':
          binding += bShiftMapper.getShiftForSet(set);
          break;
        case 't':
          binding += tShiftMapper.getShiftForSet(set);
          break;
        case 's':
          // For combined texture and sampler resources, always use the t shift
          // value and ignore the s shift value.
          if (const auto *decl = var.getDeclaration()) {
            if (decl->getAttr<VKCombinedImageSamplerAttr>() != nullptr) {
              binding += tShiftMapper.getShiftForSet(set);
              break;
            }
          }
          binding += sShiftMapper.getShiftForSet(set);
          break;
        case 'u':
          binding += uShiftMapper.getShiftForSet(set);
          break;
        case 'c':
          // For setting packing offset. Does not affect binding.
          break;
        default:
          llvm_unreachable("unknown register type found");
        }

        tryToDecorate(var, set, binding);
      }

  for (const auto &var : resourceVars) {
    // By default we use one binding number per resource, and an array of
    // resources also gets only one binding number. However, for array of
    // resources (e.g. array of textures), DX uses one binding number per array
    // element. We can match this behavior via a command line option.
    uint32_t numBindingsToUse = 1;
    if (spirvOptions.flattenResourceArrays || needsFlatteningCompositeResources)
      numBindingsToUse = getNumBindingsUsedByResourceType(
          var.getSpirvInstr()->getAstResultType());

    BindingShiftMapper *bindingShiftMapper = nullptr;
    if (spirvOptions.autoShiftBindings) {
      char registerType = '\0';
      if (getImplicitRegisterType(var, &registerType)) {
        switch (registerType) {
        case 'b':
          bindingShiftMapper = &bShiftMapper;
          break;
        case 't':
          bindingShiftMapper = &tShiftMapper;
          break;
        case 's':
          bindingShiftMapper = &sShiftMapper;
          break;
        case 'u':
          bindingShiftMapper = &uShiftMapper;
          break;
        default:
          llvm_unreachable("unknown register type found");
        }
      }
    }

    if (var.getDeclaration()) {
      const VarDecl *decl = dyn_cast<VarDecl>(var.getDeclaration());
      if (decl && (isResourceDescriptorHeap(decl->getType()) ||
                   isSamplerDescriptorHeap(decl->getType())))
        continue;
    }

    if (var.isCounter()) {

      if (!var.getCounterBinding()) {
        // Process mX * c2
        uint32_t set = defaultSpace;
        if (const auto *vkBinding = var.getBinding())
          set = getVkBindingAttrSet(vkBinding, defaultSpace);
        else if (const auto *reg = var.getRegister())
          set = reg->RegisterSpace.getValueOr(defaultSpace);

        uint32_t bindingShift = 0;
        if (bindingShiftMapper)
          bindingShift = bindingShiftMapper->getShiftForSet(set);
        spvBuilder.decorateDSetBinding(
            var.getSpirvInstr(), set,
            bindingSet.useNextBinding(set, numBindingsToUse, bindingShift));
      }
    } else if (!var.getBinding()) {
      const auto *reg = var.getRegister();
      if (reg && reg->isSpaceOnly()) {
        const uint32_t set = reg->RegisterSpace.getValueOr(defaultSpace);
        uint32_t bindingShift = 0;
        if (bindingShiftMapper)
          bindingShift = bindingShiftMapper->getShiftForSet(set);
        spvBuilder.decorateDSetBinding(
            var.getSpirvInstr(), set,
            bindingSet.useNextBinding(set, numBindingsToUse, bindingShift));
      } else if (!reg) {
        // Process m3 (no 'vk::binding' and no ':register' assignment)

        // There is a special case for the $Globals cbuffer. The $Globals buffer
        // doesn't have either 'vk::binding' or ':register', but the user may
        // ask for a specific binding for it via command line options.
        if (bindGlobals && var.isGlobalsBuffer()) {
          uint32_t bindingShift = 0;
          if (bindingShiftMapper)
            bindingShift = bindingShiftMapper->getShiftForSet(globalsSetNo);
          spvBuilder.decorateDSetBinding(var.getSpirvInstr(), globalsSetNo,
                                         globalsBindNo + bindingShift);
        }
        // The normal case
        else {
          uint32_t bindingShift = 0;
          if (bindingShiftMapper)
            bindingShift = bindingShiftMapper->getShiftForSet(defaultSpace);
          spvBuilder.decorateDSetBinding(
              var.getSpirvInstr(), defaultSpace,
              bindingSet.useNextBinding(defaultSpace, numBindingsToUse,
                                        bindingShift));
        }
      }
    }
  }

  decorateResourceHeapsBindings(bindingSet);
  return true;
}

SpirvCodeGenOptions::BindingInfo DeclResultIdMapper::getBindingInfo(
    BindingSet &bindingSet,
    const std::optional<SpirvCodeGenOptions::BindingInfo> &userProvidedInfo) {
  if (userProvidedInfo.has_value()) {
    return *userProvidedInfo;
  }
  return {bindingSet.useNextBinding(0), /* set= */ 0};
}

void DeclResultIdMapper::decorateResourceHeapsBindings(BindingSet &bindingSet) {
  bool hasResource = false;
  bool hasSamplers = false;
  bool hasCounters = false;

  // Determine which type of heap resource is used to lazily allocation
  // bindings.
  for (const auto &var : resourceVars) {
    if (!var.getDeclaration())
      continue;
    const VarDecl *decl = dyn_cast<VarDecl>(var.getDeclaration());
    if (!decl)
      continue;

    const bool isResourceHeap = isResourceDescriptorHeap(decl->getType());
    const bool isSamplerHeap = isSamplerDescriptorHeap(decl->getType());

    assert(!(var.isCounter() && isSamplerHeap));

    hasResource |= isResourceHeap;
    hasSamplers |= isSamplerHeap;
    hasCounters |= isResourceHeap && var.isCounter();
  }

  // Allocate bindings only for used resources. The order of this allocation is
  // important:
  //  - First resource heaps, then sampler heaps, and finally counter heaps.
  SpirvCodeGenOptions::BindingInfo resourceBinding = {/* binding= */ 0,
                                                      /* set= */ 0};
  SpirvCodeGenOptions::BindingInfo samplersBinding = {/* binding= */ 0,
                                                      /* set= */ 0};
  SpirvCodeGenOptions::BindingInfo countersBinding = {/* binding= */ 0,
                                                      /* set= */ 0};
  if (hasResource)
    resourceBinding =
        getBindingInfo(bindingSet, spirvOptions.resourceHeapBinding);
  if (hasSamplers)
    samplersBinding =
        getBindingInfo(bindingSet, spirvOptions.samplerHeapBinding);
  if (hasCounters)
    countersBinding =
        getBindingInfo(bindingSet, spirvOptions.counterHeapBinding);

  for (const auto &var : resourceVars) {
    if (!var.getDeclaration())
      continue;
    const VarDecl *decl = dyn_cast<VarDecl>(var.getDeclaration());
    if (!decl)
      continue;

    const bool isResourceHeap = isResourceDescriptorHeap(decl->getType());
    const bool isSamplerHeap = isSamplerDescriptorHeap(decl->getType());
    if (!isSamplerHeap && !isResourceHeap)
      continue;
    const SpirvCodeGenOptions::BindingInfo &info =
        isSamplerHeap ? samplersBinding
                      : (var.isCounter() ? countersBinding : resourceBinding);
    spvBuilder.decorateDSetBinding(var.getSpirvInstr(), info.set, info.binding);
  }
}

bool DeclResultIdMapper::decorateResourceCoherent() {
  for (const auto &var : resourceVars) {
    if (const auto *decl = var.getDeclaration()) {
      if (decl->getAttr<HLSLGloballyCoherentAttr>()) {
        spvBuilder.decorateCoherent(var.getSpirvInstr(),
                                    var.getSourceLocation());
      }
    }
  }

  return true;
}

bool DeclResultIdMapper::createStructOutputVar(
    const StageVarDataBundle &stageVarData, SpirvInstruction *value,
    bool noWriteBack) {
  // If we have base classes, we need to handle them first.
  if (const auto *cxxDecl = stageVarData.type->getAsCXXRecordDecl()) {
    uint32_t baseIndex = 0;
    for (auto base : cxxDecl->bases()) {
      SpirvInstruction *subValue = nullptr;
      if (!noWriteBack)
        subValue = spvBuilder.createCompositeExtract(
            base.getType(), value, {baseIndex++},
            stageVarData.decl->getLocation());

      StageVarDataBundle memberVarData = stageVarData;
      memberVarData.decl = base.getType()->getAsCXXRecordDecl();
      memberVarData.type = base.getType();
      if (!createStageVars(memberVarData, false, &subValue, noWriteBack))
        return false;
    }
  }

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
  const auto *structDecl = stageVarData.type->getAs<RecordType>()->getDecl();
  for (const auto *field : structDecl->fields()) {
    const auto fieldType = field->getType();
    SpirvInstruction *subValue = nullptr;
    if (!noWriteBack) {
      subValue = spvBuilder.createCompositeExtract(
          fieldType, value,
          {getNumBaseClasses(stageVarData.type) + field->getFieldIndex()},
          stageVarData.decl->getLocation());
      if (field->hasAttr<HLSLNoInterpolationAttr>() ||
          structDecl->hasAttr<HLSLNoInterpolationAttr>())
        subValue->setNoninterpolated();
    }

    StageVarDataBundle memberVarData = stageVarData;
    memberVarData.decl = field;
    memberVarData.type = field->getType();
    memberVarData.asNoInterp |= field->hasAttr<HLSLNoInterpolationAttr>();
    if (!createStageVars(memberVarData, false, &subValue, noWriteBack))
      return false;
  }
  return true;
}

SpirvInstruction *
DeclResultIdMapper::createStructInputVar(const StageVarDataBundle &stageVarData,
                                         bool noWriteBack) {
  // If this decl translates into multiple stage input variables, we need to
  // load their values into a composite.
  llvm::SmallVector<SpirvInstruction *, 4> subValues;

  // If we have base classes, we need to handle them first.
  if (const auto *cxxDecl = stageVarData.type->getAsCXXRecordDecl()) {
    for (auto base : cxxDecl->bases()) {
      SpirvInstruction *subValue = nullptr;
      StageVarDataBundle memberVarData = stageVarData;
      memberVarData.decl = base.getType()->getAsCXXRecordDecl();
      memberVarData.type = base.getType();
      if (!createStageVars(memberVarData, true, &subValue, noWriteBack))
        return nullptr;
      subValues.push_back(subValue);
    }
  }

  const auto *structDecl = stageVarData.type->getAs<RecordType>()->getDecl();
  for (const auto *field : structDecl->fields()) {
    SpirvInstruction *subValue = nullptr;
    StageVarDataBundle memberVarData = stageVarData;
    memberVarData.decl = field;
    memberVarData.type = field->getType();
    memberVarData.asNoInterp |= field->hasAttr<HLSLNoInterpolationAttr>();
    if (!createStageVars(memberVarData, true, &subValue, noWriteBack))
      return nullptr;
    subValues.push_back(subValue);
  }

  if (stageVarData.arraySize == 0) {
    SpirvInstruction *value = spvBuilder.createCompositeConstruct(
        stageVarData.type, subValues, stageVarData.decl->getLocation());
    for (auto *subInstr : subValues)
      spvBuilder.addPerVertexStgInputFuncVarEntry(subInstr, value);
    return value;
  }

  // Handle the extra level of arrayness.

  // We need to return an array of structs. But we get arrays of fields
  // from visiting all fields. So now we need to extract all the elements
  // at the same index of each field arrays and compose a new struct out
  // of them.
  const auto structType = stageVarData.type;
  const auto arrayType = astContext.getConstantArrayType(
      structType, llvm::APInt(32, stageVarData.arraySize),
      clang::ArrayType::Normal, 0);

  llvm::SmallVector<SpirvInstruction *, 16> arrayElements;

  for (uint32_t arrayIndex = 0; arrayIndex < stageVarData.arraySize;
       ++arrayIndex) {
    llvm::SmallVector<SpirvInstruction *, 8> fields;

    // If we have base classes, we need to handle them first.
    if (const auto *cxxDecl = stageVarData.type->getAsCXXRecordDecl()) {
      uint32_t baseIndex = 0;
      for (auto base : cxxDecl->bases()) {
        const auto baseType = base.getType();
        fields.push_back(spvBuilder.createCompositeExtract(
            baseType, subValues[baseIndex++], {arrayIndex},
            stageVarData.decl->getLocation()));
      }
    }

    // Extract the element at index arrayIndex from each field
    for (const auto *field : structDecl->fields()) {
      const auto fieldType = field->getType();
      fields.push_back(spvBuilder.createCompositeExtract(
          fieldType,
          subValues[getNumBaseClasses(stageVarData.type) +
                    field->getFieldIndex()],
          {arrayIndex}, stageVarData.decl->getLocation()));
    }
    // Compose a new struct out of them
    arrayElements.push_back(spvBuilder.createCompositeConstruct(
        structType, fields, stageVarData.decl->getLocation()));
  }

  return spvBuilder.createCompositeConstruct(arrayType, arrayElements,
                                             stageVarData.decl->getLocation());
}

void DeclResultIdMapper::storeToShaderOutputVariable(
    SpirvVariable *varInstr, SpirvInstruction *value,
    const StageVarDataBundle &stageVarData) {
  SpirvInstruction *ptr = varInstr;

  // Since boolean output stage variables are represented as unsigned
  // integers, we must cast the value to uint before storing.
  if (isBooleanStageIOVar(stageVarData.decl, stageVarData.type,
                          stageVarData.semantic->getKind(),
                          stageVarData.sigPoint->GetKind())) {
    QualType finalType = varInstr->getAstResultType();
    if (stageVarData.arraySize != 0) {
      // We assume that we will only have to write to a single value of the
      // array, so we have to cast to the element type of the array, and not the
      // array type.
      assert(stageVarData.invocationId.hasValue());
      finalType = finalType->getAsArrayTypeUnsafe()->getElementType();
    }
    value = theEmitter.castToType(value, stageVarData.type, finalType,
                                  stageVarData.decl->getLocation());
  }

  // Special handling of SV_TessFactor HS patch constant output.
  // TessLevelOuter is always an array of size 4 in SPIR-V, but
  // SV_TessFactor could be an array of size 2, 3, or 4 in HLSL. Only the
  // relevant indexes must be written to.
  if (stageVarData.semantic->getKind() == hlsl::Semantic::Kind::TessFactor &&
      hlsl::GetArraySize(stageVarData.type) != 4) {
    const auto tessFactorSize = hlsl::GetArraySize(stageVarData.type);
    for (uint32_t i = 0; i < tessFactorSize; ++i) {
      ptr = spvBuilder.createAccessChain(
          astContext.FloatTy, varInstr,
          {spvBuilder.getConstantInt(astContext.UnsignedIntTy,
                                     llvm::APInt(32, i))},
          stageVarData.decl->getLocation());
      spvBuilder.createStore(
          ptr,
          spvBuilder.createCompositeExtract(astContext.FloatTy, value, {i},
                                            stageVarData.decl->getLocation()),
          stageVarData.decl->getLocation());
    }
  }
  // Special handling of SV_InsideTessFactor HS patch constant output.
  // TessLevelInner is always an array of size 2 in SPIR-V, but
  // SV_InsideTessFactor could be an array of size 1 (scalar) or size 2 in
  // HLSL. If SV_InsideTessFactor is a scalar, only write to index 0 of
  // TessLevelInner.
  else if (stageVarData.semantic->getKind() ==
               hlsl::Semantic::Kind::InsideTessFactor &&
           // Some developers use float[1] instead of a scalar float.
           (!stageVarData.type->isArrayType() ||
            hlsl::GetArraySize(stageVarData.type) == 1)) {
    ptr = spvBuilder.createAccessChain(
        astContext.FloatTy, varInstr,
        spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, 0)),
        stageVarData.decl->getLocation());
    if (stageVarData.type->isArrayType()) // float[1]
      value = spvBuilder.createCompositeExtract(
          astContext.FloatTy, value, {0}, stageVarData.decl->getLocation());
    spvBuilder.createStore(ptr, value, stageVarData.decl->getLocation());
  }
  // Special handling of SV_Coverage, which is an unit value. We need to
  // write it to the first element in the SampleMask builtin.
  else if (stageVarData.semantic->getKind() == hlsl::Semantic::Kind::Coverage) {
    ptr = spvBuilder.createAccessChain(
        stageVarData.type, varInstr,
        spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, 0)),
        stageVarData.decl->getLocation());
    ptr->setStorageClass(spv::StorageClass::Output);
    spvBuilder.createStore(ptr, value, stageVarData.decl->getLocation());
  }
  // Special handling of HS ouput, for which we write to only one
  // element in the per-vertex data array: the one indexed by
  // SV_ControlPointID.
  else if (stageVarData.invocationId.hasValue() &&
           stageVarData.invocationId.getValue() != nullptr) {
    // Remove the arrayness to get the element type.
    assert(isa<ConstantArrayType>(varInstr->getAstResultType()));
    const auto elementType =
        astContext.getAsArrayType(varInstr->getAstResultType())
            ->getElementType();
    auto index = stageVarData.invocationId.getValue();
    ptr = spvBuilder.createAccessChain(elementType, varInstr, index,
                                       stageVarData.decl->getLocation());
    ptr->setStorageClass(spv::StorageClass::Output);
    spvBuilder.createStore(ptr, value, stageVarData.decl->getLocation());
  }
  // For all normal cases
  else {
    spvBuilder.createStore(ptr, value, stageVarData.decl->getLocation());
  }
}

SpirvInstruction *DeclResultIdMapper::loadShaderInputVariable(
    SpirvVariable *varInstr, const StageVarDataBundle &stageVarData) {
  SpirvInstruction *load = spvBuilder.createLoad(
      varInstr->getAstResultType(), varInstr, stageVarData.decl->getLocation());
  // Fix ups for corner cases

  // Special handling of SV_TessFactor DS patch constant input.
  // TessLevelOuter is always an array of size 4 in SPIR-V, but
  // SV_TessFactor could be an array of size 2, 3, or 4 in HLSL. Only the
  // relevant indexes must be loaded.
  if (stageVarData.semantic->getKind() == hlsl::Semantic::Kind::TessFactor &&
      hlsl::GetArraySize(stageVarData.type) != 4) {
    llvm::SmallVector<SpirvInstruction *, 4> components;
    const auto tessFactorSize = hlsl::GetArraySize(stageVarData.type);
    const auto arrType = astContext.getConstantArrayType(
        astContext.FloatTy, llvm::APInt(32, tessFactorSize),
        clang::ArrayType::Normal, 0);
    for (uint32_t i = 0; i < tessFactorSize; ++i)
      components.push_back(spvBuilder.createCompositeExtract(
          astContext.FloatTy, load, {i}, stageVarData.decl->getLocation()));
    load = spvBuilder.createCompositeConstruct(
        arrType, components, stageVarData.decl->getLocation());
  }
  // Special handling of SV_InsideTessFactor DS patch constant input.
  // TessLevelInner is always an array of size 2 in SPIR-V, but
  // SV_InsideTessFactor could be an array of size 1 (scalar) or size 2 in
  // HLSL. If SV_InsideTessFactor is a scalar, only extract index 0 of
  // TessLevelInner.
  else if (stageVarData.semantic->getKind() ==
               hlsl::Semantic::Kind::InsideTessFactor &&
           // Some developers use float[1] instead of a scalar float.
           (!stageVarData.type->isArrayType() ||
            hlsl::GetArraySize(stageVarData.type) == 1)) {
    load = spvBuilder.createCompositeExtract(astContext.FloatTy, load, {0},
                                             stageVarData.decl->getLocation());
    if (stageVarData.type->isArrayType()) { // float[1]
      const auto arrType = astContext.getConstantArrayType(
          astContext.FloatTy, llvm::APInt(32, 1), clang::ArrayType::Normal, 0);
      load = spvBuilder.createCompositeConstruct(
          arrType, {load}, stageVarData.decl->getLocation());
    }
  }
  // SV_DomainLocation can refer to a float2 or a float3, whereas TessCoord
  // is always a float3. To ensure SPIR-V validity, a float3 stage variable
  // is created, and we must extract a float2 from it before passing it to
  // the main function.
  else if (stageVarData.semantic->getKind() ==
               hlsl::Semantic::Kind::DomainLocation &&
           hlsl::GetHLSLVecSize(stageVarData.type) != 3) {
    const auto domainLocSize = hlsl::GetHLSLVecSize(stageVarData.type);
    load = spvBuilder.createVectorShuffle(
        astContext.getExtVectorType(astContext.FloatTy, domainLocSize), load,
        load, {0, 1}, stageVarData.decl->getLocation());
  }
  // Special handling of SV_Coverage, which is an uint value. We need to
  // read SampleMask and extract its first element.
  else if (stageVarData.semantic->getKind() == hlsl::Semantic::Kind::Coverage) {
    load = spvBuilder.createCompositeExtract(stageVarData.type, load, {0},
                                             stageVarData.decl->getLocation());
  }
  // Special handling of SV_InnerCoverage, which is an uint value. We need
  // to read FullyCoveredEXT, which is a boolean value, and convert it to an
  // uint value. According to D3D12 "Conservative Rasterization" doc: "The
  // Pixel Shader has a 32-bit scalar integer System Generate Value
  // available: InnerCoverage. This is a bit-field that has bit 0 from the
  // LSB set to 1 for a given conservatively rasterized pixel, only when
  // that pixel is guaranteed to be entirely inside the current primitive.
  // All other input register bits must be set to 0 when bit 0 is not set,
  // but are undefined when bit 0 is set to 1 (essentially, this bit-field
  // represents a Boolean value where false must be exactly 0, but true can
  // be any odd (i.e. bit 0 set) non-zero value)."
  else if (stageVarData.semantic->getKind() ==
           hlsl::Semantic::Kind::InnerCoverage) {
    const auto constOne =
        spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, 1));
    const auto constZero =
        spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, 0));
    load = spvBuilder.createSelect(astContext.UnsignedIntTy, load, constOne,
                                   constZero, stageVarData.decl->getLocation());
  }
  // Special handling of SV_Barycentrics, which is a float3, but the
  // The 3 values are NOT guaranteed to add up to floating-point 1.0
  // exactly. Calculate the third element here.
  else if (stageVarData.semantic->getKind() ==
           hlsl::Semantic::Kind::Barycentrics) {
    const auto x = spvBuilder.createCompositeExtract(
        astContext.FloatTy, load, {0}, stageVarData.decl->getLocation());
    const auto y = spvBuilder.createCompositeExtract(
        astContext.FloatTy, load, {1}, stageVarData.decl->getLocation());
    const auto xy =
        spvBuilder.createBinaryOp(spv::Op::OpFAdd, astContext.FloatTy, x, y,
                                  stageVarData.decl->getLocation());
    const auto z = spvBuilder.createBinaryOp(
        spv::Op::OpFSub, astContext.FloatTy,
        spvBuilder.getConstantFloat(astContext.FloatTy, llvm::APFloat(1.0f)),
        xy, stageVarData.decl->getLocation());
    load = spvBuilder.createCompositeConstruct(
        astContext.getExtVectorType(astContext.FloatTy, 3), {x, y, z},
        stageVarData.decl->getLocation());
  }
  // Special handling of SV_DispatchThreadID and SV_GroupThreadID, which may
  // be a uint or uint2, but the underlying stage input variable is a uint3.
  // The last component(s) should be discarded in needed.
  else if ((stageVarData.semantic->getKind() ==
                hlsl::Semantic::Kind::DispatchThreadID ||
            stageVarData.semantic->getKind() ==
                hlsl::Semantic::Kind::GroupThreadID ||
            stageVarData.semantic->getKind() ==
                hlsl::Semantic::Kind::GroupID) &&
           (!hlsl::IsHLSLVecType(stageVarData.type) ||
            hlsl::GetHLSLVecSize(stageVarData.type) != 3)) {
    const auto srcVecElemType =
        hlsl::IsHLSLVecType(stageVarData.type)
            ? hlsl::GetHLSLVecElementType(stageVarData.type)
            : stageVarData.type;
    const auto vecSize = hlsl::IsHLSLVecType(stageVarData.type)
                             ? hlsl::GetHLSLVecSize(stageVarData.type)
                             : 1;
    if (vecSize == 1)
      load = spvBuilder.createCompositeExtract(
          srcVecElemType, load, {0}, stageVarData.decl->getLocation());
    else if (vecSize == 2)
      load = spvBuilder.createVectorShuffle(
          astContext.getExtVectorType(srcVecElemType, 2), load, load, {0, 1},
          stageVarData.decl->getLocation());
  }

  // Reciprocate SV_Position.w if requested
  if (stageVarData.semantic->getKind() == hlsl::Semantic::Kind::Position)
    load = invertWIfRequested(load, stageVarData.decl->getLocation());

  // Since boolean stage input variables are represented as unsigned
  // integers, after loading them, we should cast them to boolean.
  if (isBooleanStageIOVar(stageVarData.decl, stageVarData.type,
                          stageVarData.semantic->getKind(),
                          stageVarData.sigPoint->GetKind())) {

    if (stageVarData.arraySize == 0) {
      load = theEmitter.castToType(load, varInstr->getAstResultType(),
                                   stageVarData.type,
                                   stageVarData.decl->getLocation());
    } else {
      llvm::SmallVector<SpirvInstruction *, 8> fields;
      SourceLocation loc = stageVarData.decl->getLocation();
      QualType originalScalarType = varInstr->getAstResultType()
                                        ->castAsArrayTypeUnsafe()
                                        ->getElementType();
      for (uint32_t idx = 0; idx < stageVarData.arraySize; ++idx) {
        SpirvInstruction *field = spvBuilder.createCompositeExtract(
            originalScalarType, load, {idx}, loc);
        field = theEmitter.castToType(field, field->getAstResultType(),
                                      stageVarData.type, loc);
        fields.push_back(field);
      }

      QualType finalType = astContext.getConstantArrayType(
          stageVarData.type, llvm::APInt(32, stageVarData.arraySize),
          clang::ArrayType::Normal, 0);
      load = spvBuilder.createCompositeConstruct(finalType, fields, loc);
    }
  }
  return load;
}

bool DeclResultIdMapper::validateShaderStageVar(
    const StageVarDataBundle &stageVarData) {
  if (!validateVKAttributes(stageVarData.decl))
    return false;

  if (!isValidSemanticInShaderModel(stageVarData)) {
    emitError("invalid usage of semantic '%0' in shader profile %1",
              stageVarData.decl->getLocation())
        << stageVarData.semantic->str
        << hlsl::ShaderModel::GetKindName(
               spvContext.getCurrentShaderModelKind());
    return false;
  }

  if (!validateVKBuiltins(stageVarData))
    return false;

  if (!validateShaderStageVarType(stageVarData))
    return false;
  return true;
}

bool DeclResultIdMapper::validateVKAttributes(const NamedDecl *decl) {
  bool success = true;
  if (const auto *idxAttr = decl->getAttr<VKIndexAttr>()) {
    if (!spvContext.isPS()) {
      emitError("vk::index only allowed in pixel shader",
                idxAttr->getLocation());
      success = false;
    }

    const auto *locAttr = decl->getAttr<VKLocationAttr>();

    if (!locAttr) {
      emitError("vk::index should be used together with vk::location for "
                "dual-source blending",
                idxAttr->getLocation());
      success = false;
    } else {
      const auto locNumber = locAttr->getNumber();
      if (locNumber != 0) {
        emitError("dual-source blending should use vk::location 0",
                  locAttr->getLocation());
        success = false;
      }
    }

    const auto idxNumber = idxAttr->getNumber();
    if (idxNumber != 0 && idxNumber != 1) {
      emitError("dual-source blending only accepts 0 or 1 as vk::index",
                idxAttr->getLocation());
      success = false;
    }
  }

  return success;
}

bool DeclResultIdMapper::validateVKBuiltins(
    const StageVarDataBundle &stageVarData) {
  bool success = true;

  if (const auto *builtinAttr = stageVarData.decl->getAttr<VKBuiltInAttr>()) {
    // The front end parsing only allows vk::builtin to be attached to a
    // function/parameter/variable; all of them are DeclaratorDecls.
    const auto declType =
        getTypeOrFnRetType(cast<DeclaratorDecl>(stageVarData.decl));
    const auto loc = builtinAttr->getLocation();

    if (stageVarData.decl->hasAttr<VKLocationAttr>()) {
      emitError("cannot use vk::builtin and vk::location together", loc);
      success = false;
    }

    const llvm::StringRef builtin = builtinAttr->getBuiltIn();

    if (builtin == "HelperInvocation") {
      if (!declType->isBooleanType()) {
        emitError("HelperInvocation builtin must be of boolean type", loc);
        success = false;
      }

      if (stageVarData.sigPoint->GetKind() != hlsl::SigPoint::Kind::PSIn) {
        emitError(
            "HelperInvocation builtin can only be used as pixel shader input",
            loc);
        success = false;
      }
    } else if (builtin == "PointSize") {
      if (!declType->isFloatingType()) {
        emitError("PointSize builtin must be of float type", loc);
        success = false;
      }

      switch (stageVarData.sigPoint->GetKind()) {
      case hlsl::SigPoint::Kind::VSOut:
      case hlsl::SigPoint::Kind::HSCPIn:
      case hlsl::SigPoint::Kind::HSCPOut:
      case hlsl::SigPoint::Kind::DSCPIn:
      case hlsl::SigPoint::Kind::DSOut:
      case hlsl::SigPoint::Kind::GSVIn:
      case hlsl::SigPoint::Kind::GSOut:
      case hlsl::SigPoint::Kind::PSIn:
      case hlsl::SigPoint::Kind::MSOut:
        break;
      default:
        emitError("PointSize builtin cannot be used as %0", loc)
            << stageVarData.sigPoint->GetName();
        success = false;
      }
    } else if (builtin == "BaseVertex" || builtin == "BaseInstance" ||
               builtin == "DrawIndex") {
      if (!declType->isSpecificBuiltinType(BuiltinType::Kind::Int) &&
          !declType->isSpecificBuiltinType(BuiltinType::Kind::UInt)) {
        emitError("%0 builtin must be of 32-bit scalar integer type", loc)
            << builtin;
        success = false;
      }

      switch (stageVarData.sigPoint->GetKind()) {
      case hlsl::SigPoint::Kind::VSIn:
        break;
      case hlsl::SigPoint::Kind::MSIn:
      case hlsl::SigPoint::Kind::ASIn:
        if (builtin != "DrawIndex") {
          emitError("%0 builtin cannot be used as %1", loc)
              << builtin << stageVarData.sigPoint->GetName();
          success = false;
        }
        break;
      default:
        emitError("%0 builtin cannot be used as %1", loc)
            << builtin << stageVarData.sigPoint->GetName();
        success = false;
      }
    } else if (builtin == "DeviceIndex") {
      if (getStorageClassForSigPoint(stageVarData.sigPoint) !=
          spv::StorageClass::Input) {
        emitError("%0 builtin can only be used as shader input", loc)
            << builtin;
        success = false;
      }
      if (!declType->isSpecificBuiltinType(BuiltinType::Kind::Int) &&
          !declType->isSpecificBuiltinType(BuiltinType::Kind::UInt)) {
        emitError("%0 builtin must be of 32-bit scalar integer type", loc)
            << builtin;
        success = false;
      }
    } else if (builtin == "ViewportMaskNV") {
      if (stageVarData.sigPoint->GetKind() != hlsl::SigPoint::Kind::MSPOut) {
        emitError("%0 builtin can only be used as 'primitives' output in MS",
                  loc)
            << builtin;
        success = false;
      }
      if (!declType->isArrayType() ||
          !declType->getArrayElementTypeNoTypeQual()->isSpecificBuiltinType(
              BuiltinType::Kind::Int)) {
        emitError("%0 builtin must be of type array of integers", loc)
            << builtin;
        success = false;
      }
    }
  }

  return success;
}

bool DeclResultIdMapper::validateShaderStageVarType(
    const StageVarDataBundle &stageVarData) {

  switch (stageVarData.semantic->getKind()) {
  case hlsl::Semantic::Kind::InnerCoverage:
    if (!stageVarData.type->isSpecificBuiltinType(BuiltinType::UInt)) {
      emitError("SV_InnerCoverage must be of uint type.",
                stageVarData.decl->getLocation());
      return false;
    }
    break;
  default:
    break;
  }
  return true;
}

bool DeclResultIdMapper::isValidSemanticInShaderModel(
    const StageVarDataBundle &stageVarData) {
  // Error out when the given semantic is invalid in this shader model
  if (hlsl::SigPoint::GetInterpretation(
          stageVarData.semantic->getKind(), stageVarData.sigPoint->GetKind(),
          spvContext.getMajorVersion(), spvContext.getMinorVersion()) ==
      hlsl::DXIL::SemanticInterpretationKind::NA) {
    // Special handle MSIn/ASIn allowing VK-only builtin "DrawIndex".
    switch (stageVarData.sigPoint->GetKind()) {
    case hlsl::SigPoint::Kind::MSIn:
    case hlsl::SigPoint::Kind::ASIn:
      if (const auto *builtinAttr =
              stageVarData.decl->getAttr<VKBuiltInAttr>()) {
        const llvm::StringRef builtin = builtinAttr->getBuiltIn();
        if (builtin == "DrawIndex") {
          break;
        }
      }
      LLVM_FALLTHROUGH;
    default:
      return false;
    }
  }
  return true;
}

SpirvVariable *DeclResultIdMapper::getInstanceIdFromIndexAndBase(
    SpirvVariable *instanceIndexVar, SpirvVariable *baseInstanceVar) {
  QualType type = instanceIndexVar->getAstResultType();
  auto *instanceIdVar = spvBuilder.addFnVar(
      type, instanceIndexVar->getSourceLocation(), "SV_InstanceID");
  auto *instanceIndexValue = spvBuilder.createLoad(
      type, instanceIndexVar, instanceIndexVar->getSourceLocation());
  auto *baseInstanceValue = spvBuilder.createLoad(
      type, baseInstanceVar, instanceIndexVar->getSourceLocation());
  auto *instanceIdValue = spvBuilder.createBinaryOp(
      spv::Op::OpISub, type, instanceIndexValue, baseInstanceValue,
      instanceIndexVar->getSourceLocation());
  spvBuilder.createStore(instanceIdVar, instanceIdValue,
                         instanceIndexVar->getSourceLocation());
  return instanceIdVar;
}

SpirvVariable *
DeclResultIdMapper::getVertexIdFromIndexAndBase(SpirvVariable *vertexIndexVar,
                                                SpirvVariable *baseVertexVar) {
  QualType type = vertexIndexVar->getAstResultType();
  auto *vertexIdVar = spvBuilder.addFnVar(
      type, vertexIndexVar->getSourceLocation(), "SV_VertexID");
  auto *vertexIndexValue = spvBuilder.createLoad(
      type, vertexIndexVar, vertexIndexVar->getSourceLocation());
  auto *baseVertexValue = spvBuilder.createLoad(
      type, baseVertexVar, vertexIndexVar->getSourceLocation());
  auto *vertexIdValue = spvBuilder.createBinaryOp(
      spv::Op::OpISub, type, vertexIndexValue, baseVertexValue,
      vertexIndexVar->getSourceLocation());
  spvBuilder.createStore(vertexIdVar, vertexIdValue,
                         vertexIndexVar->getSourceLocation());
  return vertexIdVar;
}

SpirvVariable *
DeclResultIdMapper::getBaseInstanceVariable(const hlsl::SigPoint *sigPoint,
                                            QualType type) {
  assert(type->isSpecificBuiltinType(BuiltinType::Kind::Int) ||
         type->isSpecificBuiltinType(BuiltinType::Kind::UInt));
  auto *baseInstanceVar = spvBuilder.addStageBuiltinVar(
      type, spv::StorageClass::Input, spv::BuiltIn::BaseInstance, false, {});
  StageVar var(sigPoint, {}, nullptr, type,
               getLocationAndComponentCount(astContext, type));
  var.setSpirvInstr(baseInstanceVar);
  var.setIsSpirvBuiltin();
  stageVars.push_back(var);
  return baseInstanceVar;
}

SpirvVariable *
DeclResultIdMapper::getBaseVertexVariable(const hlsl::SigPoint *sigPoint,
                                          QualType type) {
  assert(type->isSpecificBuiltinType(BuiltinType::Kind::Int) ||
         type->isSpecificBuiltinType(BuiltinType::Kind::UInt));
  auto *baseVertexVar = spvBuilder.addStageBuiltinVar(
      type, spv::StorageClass::Input, spv::BuiltIn::BaseVertex, false, {});
  StageVar var(sigPoint, {}, nullptr, type,
               getLocationAndComponentCount(astContext, type));
  var.setSpirvInstr(baseVertexVar);
  var.setIsSpirvBuiltin();
  stageVars.push_back(var);
  return baseVertexVar;
}

SpirvVariable *DeclResultIdMapper::createSpirvInterfaceVariable(
    const StageVarDataBundle &stageVarData) {
  // The evalType will be the type of the interface variable in SPIR-V.
  // The type of the variable used in the body of the function will still be
  // `stageVarData.type`.
  QualType evalType = getTypeForSpirvStageVariable(stageVarData);

  const auto *builtinAttr = stageVarData.decl->getAttr<VKBuiltInAttr>();
  StageVar stageVar(
      stageVarData.sigPoint, *stageVarData.semantic, builtinAttr, evalType,
      // For HS/DS/GS, we have already stripped the outmost arrayness on type.
      hlsl::IsHLSLNodeInputType(stageVarData.type)
          ? LocationAndComponent({0, 0, false})
          : getLocationAndComponentCount(astContext, stageVarData.type));
  const auto name =
      stageVarData.namePrefix.str() + "." + stageVar.getSemanticStr();
  SpirvVariable *varInstr = createSpirvStageVar(
      &stageVar, stageVarData.decl, name, stageVarData.semantic->loc);

  if (!varInstr)
    return nullptr;

  if (stageVarData.asNoInterp)
    varInstr->setNoninterpolated();

  stageVar.setSpirvInstr(varInstr);
  stageVar.setLocationAttr(stageVarData.decl->getAttr<VKLocationAttr>());
  stageVar.setIndexAttr(stageVarData.decl->getAttr<VKIndexAttr>());
  if (stageVar.getStorageClass() == spv::StorageClass::Input ||
      stageVar.getStorageClass() == spv::StorageClass::Output) {
    stageVar.setEntryPoint(entryFunction);
  }
  decorateStageVarWithIntrinsicAttrs(stageVarData.decl, &stageVar, varInstr);
  stageVars.push_back(stageVar);

  // Emit OpDecorate* instructions to link this stage variable with the HLSL
  // semantic it is created for
  spvBuilder.decorateHlslSemantic(varInstr, stageVar.getSemanticStr());

  // TODO: the following may not be correct?
  if (stageVarData.sigPoint->GetSignatureKind() ==
      hlsl::DXIL::SignatureKind::PatchConstOrPrim) {
    if (stageVarData.sigPoint->GetKind() == hlsl::SigPoint::Kind::MSPOut) {
      // Decorate with PerPrimitiveNV for per-primitive out variables.
      spvBuilder.decoratePerPrimitiveNV(varInstr,
                                        varInstr->getSourceLocation());
    } else if (stageVar.getSemanticInfo().getKind() !=
               hlsl::Semantic::Kind::DomainLocation) {
      spvBuilder.decoratePatch(varInstr, varInstr->getSourceLocation());
    }
  }

  // Decorate with interpolation modes for pixel shader input variables, vertex
  // shader output variables, or mesh shader output variables.
  if ((spvContext.isPS() && stageVarData.sigPoint->IsInput()) ||
      (spvContext.isVS() && stageVarData.sigPoint->IsOutput()) ||
      (spvContext.isMS() && stageVarData.sigPoint->IsOutput()))
    decorateInterpolationMode(stageVarData.decl, stageVarData.type, varInstr,
                              *stageVarData.semantic);

  // Special case: The DX12 SV_InstanceID always counts from 0, even if the
  // StartInstanceLocation parameter is non-zero. gl_InstanceIndex, however,
  // starts from firstInstance. Thus it doesn't emulate actual DX12 shader
  // behavior. To make it equivalent, SPIR-V codegen should emit:
  // SV_InstanceID = gl_InstanceIndex - gl_BaseInstance
  // As a result, we have to manually create a second stage variable for this
  // specific case.
  //
  // According to the Vulkan spec on builtin variables:
  // www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#interfaces-builtin-variables
  //
  // InstanceIndex:
  //   Decorating a variable in a vertex shader with the InstanceIndex
  //   built-in decoration will make that variable contain the index of the
  //   instance that is being processed by the current vertex shader
  //   invocation. InstanceIndex begins at the firstInstance.
  // BaseInstance
  //   Decorating a variable with the BaseInstance built-in will make that
  //   variable contain the integer value corresponding to the first instance
  //   that was passed to the command that invoked the current vertex shader
  //   invocation. BaseInstance is the firstInstance parameter to a direct
  //   drawing command or the firstInstance member of a structure consumed by
  //   an indirect drawing command.
  if (spirvOptions.supportNonzeroBaseInstance &&
      stageVarData.semantic->getKind() == hlsl::Semantic::Kind::InstanceID &&
      stageVarData.sigPoint->GetKind() == hlsl::SigPoint::Kind::VSIn) {
    // The above call to createSpirvStageVar creates the gl_InstanceIndex.
    // We should now manually create the gl_BaseInstance variable and do the
    // subtraction.
    auto *baseInstanceVar =
        getBaseInstanceVariable(stageVarData.sigPoint, stageVarData.type);

    // SPIR-V code for 'SV_InstanceID = gl_InstanceIndex - gl_BaseInstance'
    varInstr = getInstanceIdFromIndexAndBase(varInstr, baseInstanceVar);
  }

  if (spirvOptions.supportNonzeroBaseVertex &&
      stageVarData.semantic->getKind() == hlsl::Semantic::Kind::VertexID &&
      stageVarData.sigPoint->GetKind() == hlsl::SigPoint::Kind::VSIn) {

    auto *baseVertexVar =
        getBaseVertexVariable(stageVarData.sigPoint, stageVarData.type);

    // SPIR-V code for 'SV_VertexID = gl_VertexIndex - gl_BaseVertex'
    varInstr = getVertexIdFromIndexAndBase(varInstr, baseVertexVar);
  }

  // We have semantics attached to this decl, which means it must be a
  // function/parameter/variable. All are DeclaratorDecls.
  stageVarInstructions[cast<DeclaratorDecl>(stageVarData.decl)] = varInstr;

  return varInstr;
}

QualType DeclResultIdMapper::getTypeForSpirvStageVariable(
    const StageVarDataBundle &stageVarData) {
  QualType evalType = stageVarData.type;
  switch (stageVarData.semantic->getKind()) {
  case hlsl::Semantic::Kind::DomainLocation:
    // SV_DomainLocation can refer to a float2, whereas TessCoord is a float3.
    // To ensure SPIR-V validity, we must create a float3 and  extract a
    // float2 from it before passing it to the main function.
    evalType = astContext.getExtVectorType(astContext.FloatTy, 3);
    break;
  case hlsl::Semantic::Kind::TessFactor:
    // SV_TessFactor is an array of size 2 for isoline patch, array of size 3
    // for tri patch, and array of size 4 for quad patch, but it must always
    // be an array of size 4 in SPIR-V for Vulkan.
    evalType = astContext.getConstantArrayType(
        astContext.FloatTy, llvm::APInt(32, 4), clang::ArrayType::Normal, 0);
    break;
  case hlsl::Semantic::Kind::InsideTessFactor:
    // SV_InsideTessFactor is a single float for tri patch, and an array of
    // size 2 for a quad patch, but it must always be an array of size 2 in
    // SPIR-V for Vulkan.
    evalType = astContext.getConstantArrayType(
        astContext.FloatTy, llvm::APInt(32, 2), clang::ArrayType::Normal, 0);
    break;
  case hlsl::Semantic::Kind::Coverage:
    // SV_Coverage is an uint value, but the SPIR-V builtin it corresponds to,
    // SampleMask, must be an array of integers.
    evalType = astContext.getConstantArrayType(astContext.UnsignedIntTy,
                                               llvm::APInt(32, 1),
                                               clang::ArrayType::Normal, 0);
    break;
  case hlsl::Semantic::Kind::InnerCoverage:
    // SV_InnerCoverage is an uint value, but the corresponding SPIR-V builtin,
    // FullyCoveredEXT, must be an boolean value.
    evalType = astContext.BoolTy;
    break;
  case hlsl::Semantic::Kind::Barycentrics:
    evalType = astContext.getExtVectorType(astContext.FloatTy, 3);
    break;
  case hlsl::Semantic::Kind::DispatchThreadID:
  case hlsl::Semantic::Kind::GroupThreadID:
  case hlsl::Semantic::Kind::GroupID:
    // SV_DispatchThreadID, SV_GroupThreadID, and SV_GroupID are allowed to be
    // uint, uint2, or uint3, but the corresponding SPIR-V builtins
    // (GlobalInvocationId, LocalInvocationId, WorkgroupId) must be a uint3.
    // Keep the original integer signedness
    evalType = astContext.getExtVectorType(
        hlsl::IsHLSLVecType(stageVarData.type)
            ? hlsl::GetHLSLVecElementType(stageVarData.type)
            : stageVarData.type,
        3);
    break;
  default:
    // Other semantic kinds can keep the original type.
    break;
  }

  // Boolean stage I/O variables must be represented as unsigned integers.
  // Boolean built-in variables are represented as bool.
  if (isBooleanStageIOVar(stageVarData.decl, stageVarData.type,
                          stageVarData.semantic->getKind(),
                          stageVarData.sigPoint->GetKind())) {
    evalType = getUintTypeWithSourceComponents(astContext, stageVarData.type);
  }

  // Handle the extra arrayness
  if (stageVarData.arraySize != 0) {
    evalType = astContext.getConstantArrayType(
        evalType, llvm::APInt(32, stageVarData.arraySize),
        clang::ArrayType::Normal, 0);
  }

  return evalType;
}

bool DeclResultIdMapper::createStageVars(StageVarDataBundle &stageVarData,
                                         bool asInput, SpirvInstruction **value,
                                         bool noWriteBack) {
  assert(value);
  // invocationId should only be used for handling HS per-vertex output.
  if (stageVarData.invocationId.hasValue()) {
    assert(spvContext.isHS() && stageVarData.arraySize != 0 && !asInput);
  }

  assert(stageVarData.semantic);

  if (stageVarData.type->isVoidType()) {
    // No stage variables will be created for void type.
    return true;
  }

  // We have several cases regarding HLSL semantics to handle here:
  // * If the current decl inherits a semantic from some enclosing entity,
  //   use the inherited semantic no matter whether there is a semantic
  //   attached to the current decl.
  // * If there is no semantic to inherit,
  //   * If the current decl is a struct,
  //     * If the current decl has a semantic, all its members inherit this
  //       decl's semantic, with the index sequentially increasing;
  //     * If the current decl does not have a semantic, all its members
  //       should have semantics attached;
  //   * If the current decl is not a struct, it should have semantic attached.

  auto thisSemantic = getStageVarSemantic(stageVarData.decl);

  // Which semantic we should use for this decl
  // Enclosing semantics override internal ones
  if (stageVarData.semantic->isValid()) {
    if (thisSemantic.isValid()) {
      emitWarning(
          "internal semantic '%0' overridden by enclosing semantic '%1'",
          thisSemantic.loc)
          << thisSemantic.str << stageVarData.semantic->str;
    }
  } else {
    stageVarData.semantic = &thisSemantic;
  }

  if (hlsl::IsHLSLNodeType(stageVarData.type)) {
    // Hijack the notion of semantic to use createSpirvInterfaceVariable
    StringRef str = stageVarData.decl->getName();
    stageVarData.semantic->str = stageVarData.semantic->name = str;
    stageVarData.semantic->semantic = hlsl::Semantic::GetArbitrary();
    SpirvVariable *varInstr = createSpirvInterfaceVariable(stageVarData);
    if (!varInstr) {
      return false;
    }

    *value = hlsl::IsHLSLNodeInputType(stageVarData.type)
                 ? varInstr
                 : loadShaderInputVariable(varInstr, stageVarData);
    return true;
  }

  if (stageVarData.semantic->isValid() &&
      // Structs with attached semantics will be handled later.
      !stageVarData.type->isStructureType()) {
    // Found semantic attached directly to this Decl. This means we need to
    // map this decl to a single stage variable.

    const auto semanticKind = stageVarData.semantic->getKind();
    const auto sigPointKind = stageVarData.sigPoint->GetKind();

    if (!validateShaderStageVar(stageVarData)) {
      return false;
    }

    // Special handling of certain mappings between HLSL semantics and
    // SPIR-V builtins:
    // * SV_CullDistance/SV_ClipDistance are outsourced to GlPerVertex.
    if (glPerVertex.tryToAccess(
            sigPointKind, semanticKind, stageVarData.semantic->index,
            stageVarData.invocationId, value, noWriteBack,
            /*vecComponent=*/nullptr, stageVarData.decl->getLocation()))
      return true;

    SpirvVariable *varInstr = createSpirvInterfaceVariable(stageVarData);
    if (!varInstr) {
      return false;
    }

    // Mark that we have used one index for this semantic
    ++stageVarData.semantic->index;

    if (asInput) {
      *value = loadShaderInputVariable(varInstr, stageVarData);
      if ((stageVarData.decl->hasAttr<HLSLNoInterpolationAttr>() ||
           stageVarData.asNoInterp) &&
          sigPointKind == hlsl::SigPoint::Kind::PSIn)
        spvBuilder.addPerVertexStgInputFuncVarEntry(varInstr, *value);

    } else {
      if (noWriteBack)
        return true;
      // Negate SV_Position.y if requested
      if (semanticKind == hlsl::Semantic::Kind::Position)
        *value = theEmitter.invertYIfRequested(*value, thisSemantic.loc);
      storeToShaderOutputVariable(varInstr, *value, stageVarData);
    }

    return true;
  }

  // If the decl itself doesn't have semantic string attached and there is no
  // one to inherit, it should be a struct having all its fields with semantic
  // strings.
  if (!stageVarData.semantic->isValid() &&
      !stageVarData.type->isStructureType()) {
    emitError("semantic string missing for shader %select{output|input}0 "
              "variable '%1'",
              stageVarData.decl->getLocation())
        << asInput << stageVarData.decl->getName();
    return false;
  }

  if (asInput) {
    *value = createStructInputVar(stageVarData, noWriteBack);
    return (*value) != nullptr;
  } else {
    return createStructOutputVar(stageVarData, *value, noWriteBack);
  }
}

bool DeclResultIdMapper::createPayloadStageVars(
    const hlsl::SigPoint *sigPoint, spv::StorageClass sc, const NamedDecl *decl,
    bool asInput, QualType type, const llvm::StringRef namePrefix,
    SpirvInstruction **value, uint32_t payloadMemOffset) {
  assert(spvContext.isMS() || spvContext.isAS());
  assert(value);

  if (type->isVoidType()) {
    // No stage variables will be created for void type.
    return true;
  }

  const auto loc = decl->getLocation();

  // Most struct type stage vars must be flattened, but for EXT_mesh_shaders the
  // mesh payload struct should be decorated with TaskPayloadWorkgroupEXT and
  // used directly as the OpEntryPoint variable.
  if (!type->isStructureType() ||
      featureManager.isExtensionEnabled(Extension::EXT_mesh_shader)) {

    SpirvVariable *varInstr = nullptr;

    // Check whether a mesh payload module variable has already been added, as
    // is the case for the groupshared payload variable parameter of
    // DispatchMesh. In this case, change the storage class from Workgroup to
    // TaskPayloadWorkgroupEXT.
    if (featureManager.isExtensionEnabled(Extension::EXT_mesh_shader)) {
      for (SpirvVariable *moduleVar : spvBuilder.getModule()->getVariables()) {
        if (moduleVar->getAstResultType() == type) {
          moduleVar->setStorageClass(
              spv::StorageClass::TaskPayloadWorkgroupEXT);
          varInstr = moduleVar;
        }
      }
    }

    // If necessary, create new stage variable for mesh payload.
    if (!varInstr) {
      LocationAndComponent locationAndComponentCount =
          type->isStructureType()
              ? LocationAndComponent({0, 0, false})
              : getLocationAndComponentCount(astContext, type);
      StageVar stageVar(sigPoint, /*semaInfo=*/{}, /*builtinAttr=*/nullptr,
                        type, locationAndComponentCount);
      const auto name = namePrefix.str() + "." + decl->getNameAsString();
      varInstr = spvBuilder.addStageIOVar(type, sc, name, /*isPrecise=*/false,
                                          /*isNointerp=*/false, loc);

      if (!varInstr)
        return false;

      // Even though these as user defined IO stage variables, set them as
      // SPIR-V builtins in order to bypass any semantic string checks and
      // location assignment.
      stageVar.setIsSpirvBuiltin();
      stageVar.setSpirvInstr(varInstr);
      if (stageVar.getStorageClass() == spv::StorageClass::Input ||
          stageVar.getStorageClass() == spv::StorageClass::Output) {
        stageVar.setEntryPoint(entryFunction);
      }
      stageVars.push_back(stageVar);

      if (!featureManager.isExtensionEnabled(Extension::EXT_mesh_shader)) {
        // Decorate with PerTaskNV for mesh/amplification shader payload
        // variables.
        spvBuilder.decoratePerTaskNV(varInstr, payloadMemOffset,
                                     varInstr->getSourceLocation());
      }
    }

    if (asInput) {
      *value = spvBuilder.createLoad(type, varInstr, loc);
    } else {
      spvBuilder.createStore(varInstr, *value, loc);
    }
    return true;
  }

  // This decl translates into multiple stage input/output payload variables
  // and we need to load/store these individual member variables.
  const auto *structDecl = type->getAs<RecordType>()->getDecl();
  llvm::SmallVector<SpirvInstruction *, 4> subValues;
  AlignmentSizeCalculator alignmentCalc(astContext, spirvOptions);
  uint32_t nextMemberOffset = 0;

  for (const auto *field : structDecl->fields()) {
    const auto fieldType = field->getType();
    SpirvInstruction *subValue = nullptr;
    uint32_t memberAlignment = 0, memberSize = 0, stride = 0;

    // The next avaiable offset after laying out the previous members.
    std::tie(memberAlignment, memberSize) = alignmentCalc.getAlignmentAndSize(
        field->getType(), spirvOptions.ampPayloadLayoutRule,
        /*isRowMajor*/ llvm::None, &stride);
    alignmentCalc.alignUsingHLSLRelaxedLayout(
        field->getType(), memberSize, memberAlignment, &nextMemberOffset);

    // The vk::offset attribute takes precedence over all.
    if (field->getAttr<VKOffsetAttr>()) {
      nextMemberOffset = field->getAttr<VKOffsetAttr>()->getOffset();
    }

    // Each payload member must have an Offset Decoration.
    payloadMemOffset = nextMemberOffset;
    nextMemberOffset += memberSize;

    if (!asInput) {
      subValue = spvBuilder.createCompositeExtract(
          fieldType, *value, {getNumBaseClasses(type) + field->getFieldIndex()},
          loc);
    }

    if (!createPayloadStageVars(sigPoint, sc, field, asInput, field->getType(),
                                namePrefix, &subValue, payloadMemOffset))
      return false;

    if (asInput) {
      subValues.push_back(subValue);
    }
  }
  if (asInput) {
    *value = spvBuilder.createCompositeConstruct(type, subValues, loc);
  }
  return true;
}

bool DeclResultIdMapper::writeBackOutputStream(const NamedDecl *decl,
                                               QualType type,
                                               SpirvInstruction *value,
                                               SourceRange range) {
  assert(spvContext.isGS()); // Only for GS use

  if (hlsl::IsHLSLStreamOutputType(type))
    type = hlsl::GetHLSLResourceResultType(type);
  if (hasGSPrimitiveTypeQualifier(decl))
    type = astContext.getAsConstantArrayType(type)->getElementType();

  auto semanticInfo = getStageVarSemantic(decl);
  const auto loc = decl->getLocation();

  if (semanticInfo.isValid()) {
    // Found semantic attached directly to this Decl. Write the value for this
    // Decl to the corresponding stage output variable.

    // Handle SV_ClipDistance, and SV_CullDistance
    if (glPerVertex.tryToAccess(
            hlsl::DXIL::SigPointKind::GSOut, semanticInfo.semantic->GetKind(),
            semanticInfo.index, llvm::None, &value,
            /*noWriteBack=*/false, /*vecComponent=*/nullptr, loc, range))
      return true;

    // Query the <result-id> for the stage output variable generated out
    // of this decl.
    // We have semantic string attached to this decl; therefore, it must be a
    // DeclaratorDecl.
    const auto found = stageVarInstructions.find(cast<DeclaratorDecl>(decl));

    // We should have recorded its stage output variable previously.
    assert(found != stageVarInstructions.end());

    // Negate SV_Position.y if requested
    if (semanticInfo.semantic->GetKind() == hlsl::Semantic::Kind::Position)
      value = theEmitter.invertYIfRequested(value, loc, range);

    // Boolean stage output variables are represented as unsigned integers.
    if (isBooleanStageIOVar(decl, type, semanticInfo.semantic->GetKind(),
                            hlsl::SigPoint::Kind::GSOut)) {
      QualType uintType = getUintTypeWithSourceComponents(astContext, type);
      value = theEmitter.castToType(value, type, uintType, loc, range);
    }

    spvBuilder.createStore(found->second, value, loc, range);
    return true;
  }

  // If the decl itself doesn't have semantic string attached, it should be
  // a struct having all its fields with semantic strings.
  if (!type->isStructureType()) {
    emitError("semantic string missing for shader output variable '%0'", loc)
        << decl->getName();
    return false;
  }

  // If we have base classes, we need to handle them first.
  if (const auto *cxxDecl = type->getAsCXXRecordDecl()) {
    uint32_t baseIndex = 0;
    for (auto base : cxxDecl->bases()) {
      auto *subValue = spvBuilder.createCompositeExtract(
          base.getType(), value, {baseIndex++}, loc, range);

      if (!writeBackOutputStream(base.getType()->getAsCXXRecordDecl(),
                                 base.getType(), subValue, range))
        return false;
    }
  }

  const auto *structDecl = type->getAs<RecordType>()->getDecl();

  // Write out each field
  for (const auto *field : structDecl->fields()) {
    const auto fieldType = field->getType();
    auto *subValue = spvBuilder.createCompositeExtract(
        fieldType, value, {getNumBaseClasses(type) + field->getFieldIndex()},
        loc, range);

    if (!writeBackOutputStream(field, field->getType(), subValue, range))
      return false;
  }

  return true;
}

SpirvInstruction *
DeclResultIdMapper::invertWIfRequested(SpirvInstruction *position,
                                       SourceLocation loc) {
  // Reciprocate SV_Position.w if requested
  if (spirvOptions.invertW && spvContext.isPS()) {
    const auto oldW = spvBuilder.createCompositeExtract(astContext.FloatTy,
                                                        position, {3}, loc);
    const auto newW = spvBuilder.createBinaryOp(
        spv::Op::OpFDiv, astContext.FloatTy,
        spvBuilder.getConstantFloat(astContext.FloatTy, llvm::APFloat(1.0f)),
        oldW, loc);
    position = spvBuilder.createCompositeInsert(
        astContext.getExtVectorType(astContext.FloatTy, 4), position, {3}, newW,
        loc);
  }
  return position;
}

void DeclResultIdMapper::decorateInterpolationMode(
    const NamedDecl *decl, QualType type, SpirvVariable *varInstr,
    const SemanticInfo semanticInfo) {
  if (varInstr->getStorageClass() != spv::StorageClass::Input &&
      varInstr->getStorageClass() != spv::StorageClass::Output) {
    return;
  }
  const bool isBaryCoord =
      (semanticInfo.getKind() == hlsl::Semantic::Kind::Barycentrics);
  uint32_t semanticIndex = semanticInfo.index;

  if (isBaryCoord) {
    // BaryCentrics inputs cannot have attrib 'nointerpolation'.
    if (decl->getAttr<HLSLNoInterpolationAttr>()) {
      emitError(
          "SV_BaryCentrics inputs cannot have attribute 'nointerpolation'.",
          decl->getLocation());
    }
    // SV_BaryCentrics could only have two index and apply to different inputs.
    // The index should be 0 or 1, each index should be mapped to different
    // interpolation type.
    if (semanticIndex > 1) {
      emitError("The index SV_BaryCentrics semantics could only be 1 or 0.",
                decl->getLocation());
    } else if (noPerspBaryCentricsIndex < 2 && perspBaryCentricsIndex < 2) {
      emitError(
          "Cannot have more than 2 inputs with SV_BaryCentrics semantics.",
          decl->getLocation());
    } else if (decl->getAttr<HLSLNoPerspectiveAttr>()) {
      if (noPerspBaryCentricsIndex == 2 &&
          perspBaryCentricsIndex != semanticIndex) {
        noPerspBaryCentricsIndex = semanticIndex;
      } else {
        emitError("Cannot have more than 1 noperspective inputs with "
                  "SV_BaryCentrics semantics.",
                  decl->getLocation());
      }
    } else {
      if (perspBaryCentricsIndex == 2 &&
          noPerspBaryCentricsIndex != semanticIndex) {
        perspBaryCentricsIndex = semanticIndex;
      } else {
        emitError("Cannot have more than 1 perspective-correct inputs with "
                  "SV_BaryCentrics semantics.",
                  decl->getLocation());
      }
    }
  }

  const auto loc = decl->getLocation();
  if (isUintOrVecMatOfUintType(type) || isSintOrVecMatOfSintType(type) ||
      isBoolOrVecMatOfBoolType(type)) {
    // TODO: Probably we can call hlsl::ValidateSignatureElement() for the
    // following check.
    if (decl->getAttr<HLSLLinearAttr>() || decl->getAttr<HLSLCentroidAttr>() ||
        decl->getAttr<HLSLNoPerspectiveAttr>() ||
        decl->getAttr<HLSLSampleAttr>()) {
      emitError("only nointerpolation mode allowed for integer input "
                "parameters in pixel shader or integer output in vertex shader",
                decl->getLocation());
    } else {
      spvBuilder.decorateFlat(varInstr, loc);
    }
  } else {
    // Do nothing for HLSLLinearAttr since its the default
    // Attributes can be used together. So cannot use else if.
    if (decl->getAttr<HLSLCentroidAttr>())
      spvBuilder.decorateCentroid(varInstr, loc);
    if (decl->getAttr<HLSLNoInterpolationAttr>() && !isBaryCoord)
      spvBuilder.decorateFlat(varInstr, loc);
    if (decl->getAttr<HLSLNoPerspectiveAttr>() && !isBaryCoord)
      spvBuilder.decorateNoPerspective(varInstr, loc);
    if (decl->getAttr<HLSLSampleAttr>()) {
      spvBuilder.decorateSample(varInstr, loc);
    }
  }
}

SpirvVariable *DeclResultIdMapper::getBuiltinVar(spv::BuiltIn builtIn,
                                                 QualType type,
                                                 spv::StorageClass sc,
                                                 SourceLocation loc) {
  // Guarantee uniqueness
  uint32_t spvBuiltinId = static_cast<uint32_t>(builtIn);
  const auto builtInVar = builtinToVarMap.find(spvBuiltinId);
  if (builtInVar != builtinToVarMap.end()) {
    return builtInVar->second;
  }
  switch (builtIn) {
  case spv::BuiltIn::HelperInvocation:
  case spv::BuiltIn::SubgroupSize:
  case spv::BuiltIn::SubgroupLocalInvocationId:
    needsLegalization = true;
    break;
  }

  // Create a dummy StageVar for this builtin variable
  auto var = spvBuilder.addStageBuiltinVar(type, sc, builtIn,
                                           /*isPrecise*/ false, loc);

  if (spvContext.isPS() && sc == spv::StorageClass::Input) {
    if (isUintOrVecMatOfUintType(type) || isSintOrVecMatOfSintType(type) ||
        isBoolOrVecMatOfBoolType(type)) {
      spvBuilder.decorateFlat(var, loc);
    }
  }

  const hlsl::SigPoint *sigPoint =
      hlsl::SigPoint::GetSigPoint(hlsl::SigPointFromInputQual(
          hlsl::DxilParamInputQual::In, spvContext.getCurrentShaderModelKind(),
          /*isPatchConstant=*/false));

  StageVar stageVar(sigPoint, /*semaInfo=*/{}, /*builtinAttr=*/nullptr, type,
                    /*locAndComponentCount=*/{0, 0, false});

  stageVar.setIsSpirvBuiltin();
  stageVar.setSpirvInstr(var);
  stageVars.push_back(stageVar);

  // Store in map for re-use
  builtinToVarMap[spvBuiltinId] = var;
  return var;
}

SpirvVariable *DeclResultIdMapper::getBuiltinVar(spv::BuiltIn builtIn,
                                                 QualType type,
                                                 SourceLocation loc) {
  spv::StorageClass sc = spv::StorageClass::Max;

  // Valid builtins supported
  switch (builtIn) {
  case spv::BuiltIn::HelperInvocation:
  case spv::BuiltIn::SubgroupSize:
  case spv::BuiltIn::SubgroupLocalInvocationId:
  case spv::BuiltIn::HitTNV:
  case spv::BuiltIn::RayTmaxNV:
  case spv::BuiltIn::RayTminNV:
  case spv::BuiltIn::HitKindNV:
  case spv::BuiltIn::IncomingRayFlagsNV:
  case spv::BuiltIn::InstanceCustomIndexNV:
  case spv::BuiltIn::RayGeometryIndexKHR:
  case spv::BuiltIn::PrimitiveId:
  case spv::BuiltIn::InstanceId:
  case spv::BuiltIn::WorldRayDirectionNV:
  case spv::BuiltIn::WorldRayOriginNV:
  case spv::BuiltIn::ObjectRayDirectionNV:
  case spv::BuiltIn::ObjectRayOriginNV:
  case spv::BuiltIn::ObjectToWorldNV:
  case spv::BuiltIn::WorldToObjectNV:
  case spv::BuiltIn::LaunchIdNV:
  case spv::BuiltIn::LaunchSizeNV:
  case spv::BuiltIn::GlobalInvocationId:
  case spv::BuiltIn::WorkgroupId:
  case spv::BuiltIn::LocalInvocationIndex:
  case spv::BuiltIn::RemainingRecursionLevelsAMDX:
  case spv::BuiltIn::ShaderIndexAMDX:
    sc = spv::StorageClass::Input;
    break;
  case spv::BuiltIn::TaskCountNV:
  case spv::BuiltIn::PrimitiveCountNV:
  case spv::BuiltIn::PrimitiveIndicesNV:
  case spv::BuiltIn::PrimitivePointIndicesEXT:
  case spv::BuiltIn::PrimitiveLineIndicesEXT:
  case spv::BuiltIn::PrimitiveTriangleIndicesEXT:
  case spv::BuiltIn::CullPrimitiveEXT:
    sc = spv::StorageClass::Output;
    break;
  default:
    assert(false && "cannot infer storage class for SPIR-V builtin");
    break;
  }

  return getBuiltinVar(builtIn, type, sc, loc);
}

SpirvFunction *
DeclResultIdMapper::getRayTracingStageVarEntryFunction(SpirvVariable *var) {
  return rayTracingStageVarToEntryPoints[var];
}

SpirvVariable *DeclResultIdMapper::createSpirvStageVar(
    StageVar *stageVar, const NamedDecl *decl, const llvm::StringRef name,
    SourceLocation srcLoc) {
  using spv::BuiltIn;

  const auto sigPoint = stageVar->getSigPoint();
  const auto semanticKind = stageVar->getSemanticInfo().getKind();
  const auto sigPointKind = sigPoint->GetKind();
  const auto type = stageVar->getAstType();
  const auto isPrecise = decl->hasAttr<HLSLPreciseAttr>();
  auto isNointerp = decl->hasAttr<HLSLNoInterpolationAttr>();
  spv::StorageClass sc = hlsl::IsHLSLNodeInputType(stageVar->getAstType())
                             ? spv::StorageClass::NodePayloadAMDX
                             : getStorageClassForSigPoint(sigPoint);
  if (sc == spv::StorageClass::Max)
    return 0;
  stageVar->setStorageClass(sc);

  // [[vk::builtin(...)]] takes precedence.
  if (const auto *builtinAttr = stageVar->getBuiltInAttr()) {
    const auto spvBuiltIn =
        llvm::StringSwitch<BuiltIn>(builtinAttr->getBuiltIn())
            .Case("PointSize", BuiltIn::PointSize)
            .Case("HelperInvocation", BuiltIn::HelperInvocation)
            .Case("BaseVertex", BuiltIn::BaseVertex)
            .Case("BaseInstance", BuiltIn::BaseInstance)
            .Case("DrawIndex", BuiltIn::DrawIndex)
            .Case("DeviceIndex", BuiltIn::DeviceIndex)
            .Case("ViewportMaskNV", BuiltIn::ViewportMaskNV)
            .Default(BuiltIn::Max);

    assert(spvBuiltIn != BuiltIn::Max); // The frontend should guarantee this.
    if (spvBuiltIn == BuiltIn::HelperInvocation &&
        !featureManager.isTargetEnvVulkan1p3OrAbove()) {
      // If [[vk::HelperInvocation]] is used for Vulkan 1.2 or less, we enable
      // SPV_EXT_demote_to_helper_invocation extension to use
      // OpIsHelperInvocationEXT instruction.
      featureManager.allowExtension("SPV_EXT_demote_to_helper_invocation");
      return spvBuilder.addVarForHelperInvocation(type, isPrecise, srcLoc);
    }
    return spvBuilder.addStageBuiltinVar(type, sc, spvBuiltIn, isPrecise,
                                         srcLoc);
  }

  // The following translation assumes that semantic validity in the current
  // shader model is already checked, so it only covers valid SigPoints for
  // each semantic.
  switch (semanticKind) {
  // According to DXIL spec, the Position SV can be used by all SigPoints
  // other than PCIn, HSIn, GSIn, PSOut, CSIn, MSIn, MSPOut, ASIn.
  // According to Vulkan spec, the Position BuiltIn can only be used
  // by VSOut, HS/DS/GS In/Out, MSOut.
  case hlsl::Semantic::Kind::Position: {
    if (sigPointKind == hlsl::SigPoint::Kind::VSOut &&
        !containOnlyVecWithFourFloats(
            type, theEmitter.getSpirvOptions().enable16BitTypes)) {
      emitError("SV_Position must be a 4-component 32-bit float vector or a "
                "composite which recursively contains only such a vector",
                srcLoc);
    }

    switch (sigPointKind) {
    case hlsl::SigPoint::Kind::VSIn:
    case hlsl::SigPoint::Kind::PCOut:
    case hlsl::SigPoint::Kind::DSIn:
      return spvBuilder.addStageIOVar(type, sc, name.str(), isPrecise,
                                      isNointerp, srcLoc);
    case hlsl::SigPoint::Kind::VSOut:
    case hlsl::SigPoint::Kind::HSCPIn:
    case hlsl::SigPoint::Kind::HSCPOut:
    case hlsl::SigPoint::Kind::DSCPIn:
    case hlsl::SigPoint::Kind::DSOut:
    case hlsl::SigPoint::Kind::GSVIn:
    case hlsl::SigPoint::Kind::GSOut:
    case hlsl::SigPoint::Kind::MSOut:
      stageVar->setIsSpirvBuiltin();
      return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::Position,
                                           isPrecise, srcLoc);
    case hlsl::SigPoint::Kind::PSIn:
      stageVar->setIsSpirvBuiltin();
      return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::FragCoord,
                                           isPrecise, srcLoc);
    default:
      llvm_unreachable("invalid usage of SV_Position sneaked in");
    }
  }
  // According to DXIL spec, the VertexID SV can only be used by VSIn.
  // According to Vulkan spec, the VertexIndex BuiltIn can only be used by
  // VSIn.
  case hlsl::Semantic::Kind::VertexID: {
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::VertexIndex,
                                         isPrecise, srcLoc);
  }
  // According to DXIL spec, the InstanceID SV can be used by VSIn, VSOut,
  // HSCPIn, HSCPOut, DSCPIn, DSOut, GSVIn, GSOut, PSIn.
  // According to Vulkan spec, the InstanceIndex BuitIn can only be used by
  // VSIn.
  case hlsl::Semantic::Kind::InstanceID: {
    switch (sigPointKind) {
    case hlsl::SigPoint::Kind::VSIn:
      stageVar->setIsSpirvBuiltin();
      return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::InstanceIndex,
                                           isPrecise, srcLoc);
    case hlsl::SigPoint::Kind::VSOut:
    case hlsl::SigPoint::Kind::HSCPIn:
    case hlsl::SigPoint::Kind::HSCPOut:
    case hlsl::SigPoint::Kind::DSCPIn:
    case hlsl::SigPoint::Kind::DSOut:
    case hlsl::SigPoint::Kind::GSVIn:
    case hlsl::SigPoint::Kind::GSOut:
    case hlsl::SigPoint::Kind::PSIn:
      return spvBuilder.addStageIOVar(type, sc, name.str(), isPrecise,
                                      isNointerp, srcLoc);
    default:
      llvm_unreachable("invalid usage of SV_InstanceID sneaked in");
    }
  }
  // According to DXIL spec, the StartVertexLocation SV can only be used by
  // VSIn. According to Vulkan spec, the BaseVertex BuiltIn can only be used by
  // VSIn.
  case hlsl::Semantic::Kind::StartVertexLocation: {
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::BaseVertex,
                                         isPrecise, srcLoc);
  }
  // According to DXIL spec, the StartInstanceLocation SV can only be used by
  // VSIn. According to Vulkan spec, the BaseInstance BuiltIn can only be used
  // by VSIn.
  case hlsl::Semantic::Kind::StartInstanceLocation: {
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::BaseInstance,
                                         isPrecise, srcLoc);
  }
  // According to DXIL spec, the Depth{|GreaterEqual|LessEqual} SV can only be
  // used by PSOut.
  // According to Vulkan spec, the FragDepth BuiltIn can only be used by PSOut.
  case hlsl::Semantic::Kind::Depth:
  case hlsl::Semantic::Kind::DepthGreaterEqual:
  case hlsl::Semantic::Kind::DepthLessEqual: {
    stageVar->setIsSpirvBuiltin();
    // Vulkan requires the DepthReplacing execution mode to write to FragDepth.
    spvBuilder.addExecutionMode(entryFunction,
                                spv::ExecutionMode::DepthReplacing, {}, srcLoc);
    if (semanticKind == hlsl::Semantic::Kind::DepthGreaterEqual)
      spvBuilder.addExecutionMode(entryFunction,
                                  spv::ExecutionMode::DepthGreater, {}, srcLoc);
    else if (semanticKind == hlsl::Semantic::Kind::DepthLessEqual)
      spvBuilder.addExecutionMode(entryFunction, spv::ExecutionMode::DepthLess,
                                  {}, srcLoc);
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::FragDepth,
                                         isPrecise, srcLoc);
  }
  // According to DXIL spec, the ClipDistance/CullDistance SV can be used by all
  // SigPoints other than PCIn, HSIn, GSIn, PSOut, CSIn, MSIn, MSPOut, ASIn.
  // According to Vulkan spec, the ClipDistance/CullDistance
  // BuiltIn can only be used by VSOut, HS/DS/GS In/Out, MSOut.
  case hlsl::Semantic::Kind::ClipDistance:
  case hlsl::Semantic::Kind::CullDistance: {
    switch (sigPointKind) {
    case hlsl::SigPoint::Kind::VSIn:
    case hlsl::SigPoint::Kind::PCOut:
    case hlsl::SigPoint::Kind::DSIn:
      return spvBuilder.addStageIOVar(type, sc, name.str(), isPrecise,
                                      isNointerp, srcLoc);
    case hlsl::SigPoint::Kind::VSOut:
    case hlsl::SigPoint::Kind::HSCPIn:
    case hlsl::SigPoint::Kind::HSCPOut:
    case hlsl::SigPoint::Kind::DSCPIn:
    case hlsl::SigPoint::Kind::DSOut:
    case hlsl::SigPoint::Kind::GSVIn:
    case hlsl::SigPoint::Kind::GSOut:
    case hlsl::SigPoint::Kind::PSIn:
    case hlsl::SigPoint::Kind::MSOut:
      llvm_unreachable("should be handled in gl_PerVertex struct");
    default:
      llvm_unreachable(
          "invalid usage of SV_ClipDistance/SV_CullDistance sneaked in");
    }
  }
  // According to DXIL spec, the IsFrontFace SV can only be used by GSOut and
  // PSIn.
  // According to Vulkan spec, the FrontFacing BuitIn can only be used in PSIn.
  case hlsl::Semantic::Kind::IsFrontFace: {
    switch (sigPointKind) {
    case hlsl::SigPoint::Kind::GSOut:
      return spvBuilder.addStageIOVar(type, sc, name.str(), isPrecise,
                                      isNointerp, srcLoc);
    case hlsl::SigPoint::Kind::PSIn:
      stageVar->setIsSpirvBuiltin();
      return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::FrontFacing,
                                           isPrecise, srcLoc);
    default:
      llvm_unreachable("invalid usage of SV_IsFrontFace sneaked in");
    }
  }
  // According to DXIL spec, the Target SV can only be used by PSOut.
  // There is no corresponding builtin decoration in SPIR-V. So generate normal
  // Vulkan stage input/output variables.
  case hlsl::Semantic::Kind::Target:
  // An arbitrary semantic is defined by users. Generate normal Vulkan stage
  // input/output variables.
  case hlsl::Semantic::Kind::Arbitrary: {
    return spvBuilder.addStageIOVar(type, sc, name.str(), isPrecise, isNointerp,
                                    srcLoc);
    // TODO: patch constant function in hull shader
  }
  // According to DXIL spec, the DispatchThreadID SV can only be used by CSIn.
  // According to Vulkan spec, the GlobalInvocationId can only be used in CSIn.
  case hlsl::Semantic::Kind::DispatchThreadID: {
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::GlobalInvocationId,
                                         isPrecise, srcLoc);
  }
  // According to DXIL spec, the GroupID SV can only be used by CSIn.
  // According to Vulkan spec, the WorkgroupId can only be used in CSIn.
  case hlsl::Semantic::Kind::GroupID: {
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::WorkgroupId,
                                         isPrecise, srcLoc);
  }
  // According to DXIL spec, the GroupThreadID SV can only be used by CSIn.
  // According to Vulkan spec, the LocalInvocationId can only be used in CSIn.
  case hlsl::Semantic::Kind::GroupThreadID: {
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::LocalInvocationId,
                                         isPrecise, srcLoc);
  }
  // According to DXIL spec, the GroupIndex SV can only be used by CSIn.
  // According to Vulkan spec, the LocalInvocationIndex can only be used in
  // CSIn.
  case hlsl::Semantic::Kind::GroupIndex: {
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(
        type, sc, BuiltIn::LocalInvocationIndex, isPrecise, srcLoc);
  }
  // According to DXIL spec, the OutputControlID SV can only be used by HSIn.
  // According to Vulkan spec, the InvocationId BuiltIn can only be used in
  // HS/GS In.
  case hlsl::Semantic::Kind::OutputControlPointID: {
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::InvocationId,
                                         isPrecise, srcLoc);
  }
  // According to DXIL spec, the PrimitiveID SV can only be used by PCIn, HSIn,
  // DSIn, GSIn, GSOut, PSIn, and MSPOut.
  // According to Vulkan spec, the PrimitiveId BuiltIn can only be used in
  // HS/DS/PS In, GS In/Out, MSPOut.
  case hlsl::Semantic::Kind::PrimitiveID: {
    // Translate to PrimitiveId BuiltIn for all valid SigPoints.
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::PrimitiveId,
                                         isPrecise, srcLoc);
  }
  // According to DXIL spec, the TessFactor SV can only be used by PCOut and
  // DSIn.
  // According to Vulkan spec, the TessLevelOuter BuiltIn can only be used in
  // PCOut and DSIn.
  case hlsl::Semantic::Kind::TessFactor: {
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::TessLevelOuter,
                                         isPrecise, srcLoc);
  }
  // According to DXIL spec, the InsideTessFactor SV can only be used by PCOut
  // and DSIn.
  // According to Vulkan spec, the TessLevelInner BuiltIn can only be used in
  // PCOut and DSIn.
  case hlsl::Semantic::Kind::InsideTessFactor: {
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::TessLevelInner,
                                         isPrecise, srcLoc);
  }
  // According to DXIL spec, the DomainLocation SV can only be used by DSIn.
  // According to Vulkan spec, the TessCoord BuiltIn can only be used in DSIn.
  case hlsl::Semantic::Kind::DomainLocation: {
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::TessCoord,
                                         isPrecise, srcLoc);
  }
  // According to DXIL spec, the GSInstanceID SV can only be used by GSIn.
  // According to Vulkan spec, the InvocationId BuiltIn can only be used in
  // HS/GS In.
  case hlsl::Semantic::Kind::GSInstanceID: {
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::InvocationId,
                                         isPrecise, srcLoc);
  }
  // According to DXIL spec, the SampleIndex SV can only be used by PSIn.
  // According to Vulkan spec, the SampleId BuiltIn can only be used in PSIn.
  case hlsl::Semantic::Kind::SampleIndex: {
    setInterlockExecutionMode(spv::ExecutionMode::SampleInterlockOrderedEXT);
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::SampleId, isPrecise,
                                         srcLoc);
  }
  // According to DXIL spec, the StencilRef SV can only be used by PSOut.
  case hlsl::Semantic::Kind::StencilRef: {
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::FragStencilRefEXT,
                                         isPrecise, srcLoc);
  }
  // According to DXIL spec, the Barycentrics SV can only be used by PSIn.
  case hlsl::Semantic::Kind::Barycentrics: {
    stageVar->setIsSpirvBuiltin();

    // Selecting the correct builtin according to interpolation mode
    auto bi = BuiltIn::Max;
    if (decl->hasAttr<HLSLNoPerspectiveAttr>()) {
      bi = BuiltIn::BaryCoordNoPerspKHR;
    } else {
      bi = BuiltIn::BaryCoordKHR;
    }

    return spvBuilder.addStageBuiltinVar(type, sc, bi, isPrecise, srcLoc);
  }
  // According to DXIL spec, the RenderTargetArrayIndex SV can only be used by
  // VSIn, VSOut, HSCPIn, HSCPOut, DSIn, DSOut, GSVIn, GSOut, PSIn, MSPOut.
  // According to Vulkan spec, the Layer BuiltIn can only be used in GSOut
  // PSIn, and MSPOut.
  case hlsl::Semantic::Kind::RenderTargetArrayIndex: {
    switch (sigPointKind) {
    case hlsl::SigPoint::Kind::VSIn:
    case hlsl::SigPoint::Kind::HSCPIn:
    case hlsl::SigPoint::Kind::HSCPOut:
    case hlsl::SigPoint::Kind::PCOut:
    case hlsl::SigPoint::Kind::DSIn:
    case hlsl::SigPoint::Kind::DSCPIn:
    case hlsl::SigPoint::Kind::GSVIn:
      return spvBuilder.addStageIOVar(type, sc, name.str(), isPrecise,
                                      isNointerp, srcLoc);
    case hlsl::SigPoint::Kind::VSOut:
    case hlsl::SigPoint::Kind::DSOut:
      stageVar->setIsSpirvBuiltin();
      return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::Layer, isPrecise,
                                           srcLoc);
    case hlsl::SigPoint::Kind::GSOut:
    case hlsl::SigPoint::Kind::PSIn:
    case hlsl::SigPoint::Kind::MSPOut:
      stageVar->setIsSpirvBuiltin();
      return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::Layer, isPrecise,
                                           srcLoc);
    default:
      llvm_unreachable("invalid usage of SV_RenderTargetArrayIndex sneaked in");
    }
  }
  // According to DXIL spec, the ViewportArrayIndex SV can only be used by
  // VSIn, VSOut, HSCPIn, HSCPOut, DSIn, DSOut, GSVIn, GSOut, PSIn, MSPOut.
  // According to Vulkan spec, the ViewportIndex BuiltIn can only be used in
  // GSOut, PSIn, and MSPOut.
  case hlsl::Semantic::Kind::ViewPortArrayIndex: {
    switch (sigPointKind) {
    case hlsl::SigPoint::Kind::VSIn:
    case hlsl::SigPoint::Kind::HSCPIn:
    case hlsl::SigPoint::Kind::HSCPOut:
    case hlsl::SigPoint::Kind::PCOut:
    case hlsl::SigPoint::Kind::DSIn:
    case hlsl::SigPoint::Kind::DSCPIn:
    case hlsl::SigPoint::Kind::GSVIn:
      return spvBuilder.addStageIOVar(type, sc, name.str(), isPrecise,
                                      isNointerp, srcLoc);
    case hlsl::SigPoint::Kind::VSOut:
    case hlsl::SigPoint::Kind::DSOut:
      stageVar->setIsSpirvBuiltin();
      return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::ViewportIndex,
                                           isPrecise, srcLoc);
    case hlsl::SigPoint::Kind::GSOut:
    case hlsl::SigPoint::Kind::PSIn:
    case hlsl::SigPoint::Kind::MSPOut:
      stageVar->setIsSpirvBuiltin();
      return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::ViewportIndex,
                                           isPrecise, srcLoc);
    default:
      llvm_unreachable("invalid usage of SV_ViewportArrayIndex sneaked in");
    }
  }
  // According to DXIL spec, the Coverage SV can only be used by PSIn and PSOut.
  // According to Vulkan spec, the SampleMask BuiltIn can only be used in
  // PSIn and PSOut.
  case hlsl::Semantic::Kind::Coverage: {
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::SampleMask,
                                         isPrecise, srcLoc);
  }
  // According to DXIL spec, the ViewID SV can only be used by VSIn, PCIn,
  // HSIn, DSIn, GSIn, PSIn.
  // According to Vulkan spec, the ViewIndex BuiltIn can only be used in
  // VS/HS/DS/GS/PS input.
  case hlsl::Semantic::Kind::ViewID: {
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::ViewIndex,
                                         isPrecise, srcLoc);
  }
  // According to DXIL spec, the InnerCoverage SV can only be used as PSIn.
  // According to Vulkan spec, the FullyCoveredEXT BuiltIn can only be used as
  // PSIn.
  case hlsl::Semantic::Kind::InnerCoverage: {
    stageVar->setIsSpirvBuiltin();
    return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::FullyCoveredEXT,
                                         isPrecise, srcLoc);
  }
  // According to DXIL spec, the ShadingRate SV can only be used by GSOut,
  // VSOut, or PSIn. According to Vulkan spec, the FragSizeEXT BuiltIn can only
  // be used as VSOut, GSOut, MSOut or PSIn.
  case hlsl::Semantic::Kind::ShadingRate: {
    setInterlockExecutionMode(
        spv::ExecutionMode::ShadingRateInterlockOrderedEXT);
    switch (sigPointKind) {
    case hlsl::SigPoint::Kind::PSIn:
      stageVar->setIsSpirvBuiltin();
      return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::ShadingRateKHR,
                                           isPrecise, srcLoc);
    case hlsl::SigPoint::Kind::VSOut:
    case hlsl::SigPoint::Kind::GSOut:
    case hlsl::SigPoint::Kind::MSOut:
    case hlsl::SigPoint::Kind::MSPOut:
      stageVar->setIsSpirvBuiltin();
      return spvBuilder.addStageBuiltinVar(
          type, sc, BuiltIn::PrimitiveShadingRateKHR, isPrecise, srcLoc);
    default:
      emitError("semantic ShadingRate must be used only for PSIn, VSOut, "
                "GSOut, MSOut",
                srcLoc);
      break;
    }
    break;
  }
  // According to DXIL spec, the ShadingRate SV can only be used by
  // MSPOut or PSIn.
  // According to Vulkan spec, the CullPrimitiveEXT BuiltIn can only
  // be used as MSOut.
  case hlsl::Semantic::Kind::CullPrimitive: {
    switch (sigPointKind) {
    case hlsl::SigPoint::Kind::PSIn:
      stageVar->setIsSpirvBuiltin();
      return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::CullPrimitiveEXT,
                                           isPrecise, srcLoc);
    case hlsl::SigPoint::Kind::MSPOut:
      stageVar->setIsSpirvBuiltin();
      return spvBuilder.addStageBuiltinVar(type, sc, BuiltIn::CullPrimitiveEXT,
                                           isPrecise, srcLoc);
    default:
      emitError("semantic CullPrimitive must be used only for PSIn, MSPOut",
                srcLoc);
      break;
    }
    break;
  }
  default:
    emitError("semantic %0 unimplemented", srcLoc)
        << stageVar->getSemanticStr();
    break;
  }

  return 0;
}

spv::StorageClass
DeclResultIdMapper::getStorageClassForSigPoint(const hlsl::SigPoint *sigPoint) {
  // This translation is done based on the HLSL reference (see docs/dxil.rst).
  const auto sigPointKind = sigPoint->GetKind();
  const auto signatureKind = sigPoint->GetSignatureKind();
  spv::StorageClass sc = spv::StorageClass::Max;
  switch (signatureKind) {
  case hlsl::DXIL::SignatureKind::Input:
    sc = spv::StorageClass::Input;
    break;
  case hlsl::DXIL::SignatureKind::Output:
    sc = spv::StorageClass::Output;
    break;
  case hlsl::DXIL::SignatureKind::Invalid: {
    // There are some special cases in HLSL (See docs/dxil.rst):
    // SignatureKind is "invalid" for PCIn, HSIn, GSIn, and CSIn.
    switch (sigPointKind) {
    case hlsl::DXIL::SigPointKind::PCIn:
    case hlsl::DXIL::SigPointKind::HSIn:
    case hlsl::DXIL::SigPointKind::GSIn:
    case hlsl::DXIL::SigPointKind::CSIn:
    case hlsl::DXIL::SigPointKind::MSIn:
    case hlsl::DXIL::SigPointKind::ASIn:
      sc = spv::StorageClass::Input;
      break;
    default:
      llvm_unreachable("Found invalid SigPoint kind for semantic");
    }
    break;
  }
  case hlsl::DXIL::SignatureKind::PatchConstOrPrim: {
    // There are some special cases in HLSL (See docs/dxil.rst):
    // SignatureKind is "PatchConstOrPrim" for PCOut, MSPOut and DSIn.
    switch (sigPointKind) {
    case hlsl::DXIL::SigPointKind::PCOut:
    case hlsl::DXIL::SigPointKind::MSPOut:
      // Patch Constant Output (Output of Hull which is passed to Domain).
      // Mesh Shader per-primitive output attributes.
      sc = spv::StorageClass::Output;
      break;
    case hlsl::DXIL::SigPointKind::DSIn:
      // Domain Shader regular input - Patch Constant data plus system values.
      sc = spv::StorageClass::Input;
      break;
    default:
      llvm_unreachable("Found invalid SigPoint kind for semantic");
    }
    break;
  }
  default:
    llvm_unreachable("Found invalid SigPoint kind for semantic");
  }
  return sc;
}

QualType DeclResultIdMapper::getTypeAndCreateCounterForPotentialAliasVar(
    const DeclaratorDecl *decl, bool *shouldBeAlias) {
  if (const auto *varDecl = dyn_cast<VarDecl>(decl)) {
    // This method is only intended to be used to create SPIR-V variables in the
    // Function or Private storage class.
    assert(!SpirvEmitter::isExternalVar(varDecl));
  }

  const QualType type = getTypeOrFnRetType(decl);
  // Whether we should generate this decl as an alias variable.
  bool genAlias = false;

  // For ConstantBuffers, TextureBuffers, StructuredBuffers, ByteAddressBuffers
  if (isConstantTextureBuffer(type) ||
      isOrContainsAKindOfStructuredOrByteBuffer(type)) {
    genAlias = true;
  }

  // Return via parameter whether alias was generated.
  if (shouldBeAlias)
    *shouldBeAlias = genAlias;

  if (genAlias) {
    needsLegalization = true;
    createCounterVarForDecl(decl);
  }

  return type;
}

bool DeclResultIdMapper::getImplicitRegisterType(const ResourceVar &var,
                                                 char *registerTypeOut) const {
  assert(registerTypeOut);

  if (var.getSpirvInstr()) {
    if (var.getSpirvInstr()->hasAstResultType()) {
      QualType type = var.getSpirvInstr()->getAstResultType();
      // Strip outer arrayness first
      while (type->isArrayType())
        type = type->getAsArrayTypeUnsafe()->getElementType();

      // t - for shader resource views (SRV)
      if (isTexture(type) || isNonWritableStructuredBuffer(type) ||
          isByteAddressBuffer(type) || isBuffer(type)) {
        *registerTypeOut = 't';
        return true;
      }
      // s - for samplers
      else if (isSampler(type)) {
        *registerTypeOut = 's';
        return true;
      }
      // u - for unordered access views (UAV)
      else if (isRWByteAddressBuffer(type) || isRWAppendConsumeSBuffer(type) ||
               isRWBuffer(type) || isRWTexture(type)) {
        *registerTypeOut = 'u';
        return true;
      }

      // b - for constant buffer
      // views (CBV)
      else if (isConstantBuffer(type)) {
        *registerTypeOut = 'b';
        return true;
      }
    } else {
      llvm::StringRef hlslUserType = var.getSpirvInstr()->getHlslUserType();
      // b - for constant buffer views (CBV)
      if (var.isGlobalsBuffer() || hlslUserType == "cbuffer" ||
          hlslUserType == "ConstantBuffer") {
        *registerTypeOut = 'b';
        return true;
      }
      if (hlslUserType == "tbuffer") {
        *registerTypeOut = 't';
        return true;
      }
    }
  }

  *registerTypeOut = '\0';
  return false;
}

SpirvVariable *
DeclResultIdMapper::createRayTracingNVStageVar(spv::StorageClass sc,
                                               const VarDecl *decl) {
  return createRayTracingNVStageVar(sc, decl->getType(), decl->getName().str(),
                                    decl->hasAttr<HLSLPreciseAttr>(),
                                    decl->hasAttr<HLSLNoInterpolationAttr>());
}

SpirvVariable *DeclResultIdMapper::createRayTracingNVStageVar(
    spv::StorageClass sc, QualType type, std::string name, bool isPrecise,
    bool isNointerp) {
  SpirvVariable *retVal = nullptr;

  // Raytracing interface variables are special since they do not participate
  // in any interface matching and hence do not create StageVar and
  // track them under StageVars vector

  switch (sc) {
  case spv::StorageClass::IncomingRayPayloadNV:
  case spv::StorageClass::IncomingCallableDataNV:
  case spv::StorageClass::HitAttributeNV:
  case spv::StorageClass::RayPayloadNV:
  case spv::StorageClass::CallableDataNV:
    retVal = spvBuilder.addModuleVar(type, sc, isPrecise, isNointerp, name);
    break;

  default:
    assert(false && "Unsupported SPIR-V storage class for raytracing");
  }

  rayTracingStageVarToEntryPoints[retVal] = entryFunction;

  return retVal;
}

void DeclResultIdMapper::tryToCreateImplicitConstVar(const ValueDecl *decl) {
  const VarDecl *varDecl = dyn_cast<VarDecl>(decl);
  if (!varDecl || !varDecl->isImplicit())
    return;

  APValue *val = varDecl->evaluateValue();
  if (!val)
    return;

  SpirvInstruction *constVal =
      spvBuilder.getConstantInt(astContext.UnsignedIntTy, val->getInt());
  constVal->setRValue(true);
  registerVariableForDecl(varDecl, constVal);
}

void DeclResultIdMapper::decorateWithIntrinsicAttrs(
    const NamedDecl *decl, SpirvVariable *varInst,
    llvm::function_ref<void(VKDecorateExtAttr *)> extraFunctionForDecoAttr) {
  if (!decl->hasAttrs())
    return;

  // TODO: Handle member field in a struct and function parameter.
  for (auto &attr : decl->getAttrs()) {
    if (auto decoAttr = dyn_cast<VKDecorateExtAttr>(attr)) {
      spvBuilder.decorateWithLiterals(
          varInst, decoAttr->getDecorate(),
          {decoAttr->literals_begin(), decoAttr->literals_end()},
          varInst->getSourceLocation());
      extraFunctionForDecoAttr(decoAttr);
      continue;
    }
    if (auto decoAttr = dyn_cast<VKDecorateIdExtAttr>(attr)) {
      llvm::SmallVector<SpirvInstruction *, 2> args;
      for (Expr *arg : decoAttr->arguments()) {
        args.push_back(theEmitter.doExpr(arg));
      }
      spvBuilder.decorateWithIds(varInst, decoAttr->getDecorate(), args,
                                 varInst->getSourceLocation());
      continue;
    }
    if (auto decoAttr = dyn_cast<VKDecorateStringExtAttr>(attr)) {
      llvm::SmallVector<llvm::StringRef, 2> args(decoAttr->arguments_begin(),
                                                 decoAttr->arguments_end());
      spvBuilder.decorateWithStrings(varInst, decoAttr->getDecorate(), args,
                                     varInst->getSourceLocation());
      continue;
    }
  }
}

void DeclResultIdMapper::decorateStageVarWithIntrinsicAttrs(
    const NamedDecl *decl, StageVar *stageVar, SpirvVariable *varInst) {
  auto checkBuiltInLocationDecoration =
      [stageVar](const VKDecorateExtAttr *decoAttr) {
        auto decorate = static_cast<spv::Decoration>(decoAttr->getDecorate());
        if (decorate == spv::Decoration::BuiltIn ||
            decorate == spv::Decoration::Location) {
          // This information will be used to avoid
          // assigning multiple location decorations
          // in finalizeStageIOLocations()
          stageVar->setIsLocOrBuiltinDecorateAttr();
        }
      };
  decorateWithIntrinsicAttrs(decl, varInst, checkBuiltInLocationDecoration);
}

void DeclResultIdMapper::setInterlockExecutionMode(spv::ExecutionMode mode) {
  interlockExecutionMode = mode;
}

spv::ExecutionMode DeclResultIdMapper::getInterlockExecutionMode() {
  return interlockExecutionMode.getValueOr(
      spv::ExecutionMode::PixelInterlockOrderedEXT);
}

void DeclResultIdMapper::registerVariableForDecl(const VarDecl *var,
                                                 SpirvInstruction *varInstr) {
  DeclSpirvInfo spirvInfo;
  spirvInfo.instr = varInstr;
  spirvInfo.indexInCTBuffer = -1;
  registerVariableForDecl(var, spirvInfo);
}

void DeclResultIdMapper::registerVariableForDecl(const VarDecl *var,
                                                 DeclSpirvInfo spirvInfo) {
  for (const auto *v : var->redecls()) {
    astDecls[v] = spirvInfo;
  }
}

void DeclResultIdMapper::copyHullOutStageVarsToOutputPatch(
    SpirvInstruction *hullMainOutputPatch, const ParmVarDecl *outputPatchDecl,
    QualType outputControlPointType, uint32_t numOutputControlPoints) {
  for (uint32_t outputCtrlPoint = 0; outputCtrlPoint < numOutputControlPoints;
       ++outputCtrlPoint) {
    SpirvConstant *index = spvBuilder.getConstantInt(
        astContext.UnsignedIntTy, llvm::APInt(32, outputCtrlPoint));
    auto *tempLocation = spvBuilder.createAccessChain(
        outputControlPointType, hullMainOutputPatch, {index}, /*loc=*/{});
    storeOutStageVarsToStorage(cast<DeclaratorDecl>(outputPatchDecl), index,
                               outputControlPointType, tempLocation);
  }
}

void DeclResultIdMapper::storeOutStageVarsToStorage(
    const DeclaratorDecl *outputPatchDecl, SpirvConstant *ctrlPointID,
    QualType outputControlPointType, SpirvInstruction *ptr) {
  if (!outputControlPointType->isStructureType()) {
    const auto found = stageVarInstructions.find(outputPatchDecl);
    if (found == stageVarInstructions.end()) {
      emitError("Shader output variable '%0' was not created", {})
          << outputPatchDecl->getName();
    }
    auto *ptrToOutputStageVar = spvBuilder.createAccessChain(
        outputControlPointType, found->second, {ctrlPointID}, /*loc=*/{});
    auto *load =
        spvBuilder.createLoad(outputControlPointType, ptrToOutputStageVar,
                              /*loc=*/{});
    spvBuilder.createStore(ptr, load, /*loc=*/{});
    return;
  }

  const auto *recordType = outputControlPointType->getAs<RecordType>();
  assert(recordType != nullptr);
  const auto *structDecl = recordType->getDecl();
  assert(structDecl != nullptr);

  uint32_t index = 0;
  for (const auto *field : structDecl->fields()) {
    SpirvConstant *indexInst = spvBuilder.getConstantInt(
        astContext.UnsignedIntTy, llvm::APInt(32, index));
    auto *tempLocation = spvBuilder.createAccessChain(field->getType(), ptr,
                                                      {indexInst}, /*loc=*/{});
    storeOutStageVarsToStorage(cast<DeclaratorDecl>(field), ctrlPointID,
                               field->getType(), tempLocation);
    ++index;
  }
}

void DeclResultIdMapper::registerCapabilitiesAndExtensionsForType(
    const TypedefType *type) {
  for (const auto *decl : typeAliasesWithAttributes) {
    if (type == decl->getTypeForDecl()) {
      for (auto *attribute : decl->specific_attrs<VKExtensionExtAttr>()) {
        clang::StringRef extensionName = attribute->getName();
        spvBuilder.requireExtension(extensionName, decl->getLocation());
      }
      for (auto *attribute : decl->specific_attrs<VKCapabilityExtAttr>()) {
        spv::Capability cap = spv::Capability(attribute->getCapability());
        spvBuilder.requireCapability(cap, decl->getLocation());
      }
    }
  }
}

} // end namespace spirv
} // end namespace clang
