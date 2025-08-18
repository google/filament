//===--- LowerTypeVisitor.cpp - AST type to SPIR-V type impl -----*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "LowerTypeVisitor.h"

#include "ConstEvaluator.h"
#include "clang/AST/Attr.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/HlslTypes.h"
#include "clang/SPIRV/AstTypeProbe.h"
#include "clang/SPIRV/SpirvFunction.h"

namespace clang {
namespace spirv {

namespace {
/// Returns the :packoffset() annotation on the given decl. Returns nullptr if
/// the decl does not have one.
hlsl::ConstantPacking *getPackOffset(const clang::NamedDecl *decl) {
  for (auto *annotation : decl->getUnusualAnnotations())
    if (auto *packing = llvm::dyn_cast<hlsl::ConstantPacking>(annotation))
      return packing;
  return nullptr;
}

/// Rounds the given value up to the given power of 2.
inline uint32_t roundToPow2(uint32_t val, uint32_t pow2) {
  assert(pow2 != 0);
  return (val + pow2 - 1) & ~(pow2 - 1);
}

} // end anonymous namespace

static void setDefaultFieldSize(const AlignmentSizeCalculator &alignmentCalc,
                                const SpirvLayoutRule rule,
                                const HybridStructType::FieldInfo *currentField,
                                StructType::FieldInfo *field) {

  const auto &fieldType = currentField->astType;
  uint32_t memberAlignment = 0, memberSize = 0, stride = 0;
  std::tie(memberAlignment, memberSize) = alignmentCalc.getAlignmentAndSize(
      fieldType, rule, /*isRowMajor*/ llvm::None, &stride);
  field->sizeInBytes = memberSize;
  return;
}

// Correctly determine a field offset/size/padding depending on its neighbors
// and other rules.
static void
setDefaultFieldOffset(const AlignmentSizeCalculator &alignmentCalc,
                      const SpirvLayoutRule rule,
                      const uint32_t previousFieldEnd,
                      const HybridStructType::FieldInfo *currentField,
                      StructType::FieldInfo *field) {

  const auto &fieldType = currentField->astType;
  uint32_t memberAlignment = 0, memberSize = 0, stride = 0;
  std::tie(memberAlignment, memberSize) = alignmentCalc.getAlignmentAndSize(
      fieldType, rule, /*isRowMajor*/ llvm::None, &stride);

  const uint32_t baseOffset = previousFieldEnd;
  // The next avaiable location after laying out the previous members
  if (rule != SpirvLayoutRule::RelaxedGLSLStd140 &&
      rule != SpirvLayoutRule::RelaxedGLSLStd430 &&
      rule != SpirvLayoutRule::FxcCTBuffer) {
    field->offset = roundToPow2(baseOffset, memberAlignment);
    return;
  }

  uint32_t newOffset = previousFieldEnd;
  alignmentCalc.alignUsingHLSLRelaxedLayout(fieldType, memberSize,
                                            memberAlignment, &newOffset);
  field->offset = newOffset;
}

bool LowerTypeVisitor::visit(SpirvFunction *fn, Phase phase) {
  if (phase == Visitor::Phase::Done) {
    // Lower the function return type.
    const SpirvType *spirvReturnType =
        lowerType(fn->getAstReturnType(), SpirvLayoutRule::Void,
                  /*isRowMajor*/ llvm::None,
                  /*SourceLocation*/ {});
    fn->setReturnType(spirvReturnType);

    // Lower the function parameter types.
    auto params = fn->getParameters();
    llvm::SmallVector<const SpirvType *, 4> spirvParamTypes;
    for (auto *param : params) {
      spirvParamTypes.push_back(param->getResultType());
    }
    fn->setFunctionType(
        spvContext.getFunctionType(spirvReturnType, spirvParamTypes));
  }
  return true;
}

bool LowerTypeVisitor::visitInstruction(SpirvInstruction *instr) {
  if (spvContext.hasLoweredType(instr))
    return true;

  const QualType astType = instr->getAstResultType();
  const SpirvType *hybridType = instr->getResultType();

  // Lower QualType to SpirvType
  if (astType != QualType({})) {
    const SpirvType *spirvType =
        lowerType(astType, instr->getLayoutRule(), /*isRowMajor*/ llvm::None,
                  instr->getSourceLocation());
    instr->setResultType(spirvType);
  }
  // Lower Hybrid type to SpirvType
  else if (hybridType) {
    const SpirvType *spirvType = lowerType(hybridType, instr->getLayoutRule(),
                                           instr->getSourceLocation());
    instr->setResultType(spirvType);
  }

  // Lower QualType of DebugLocalVariable or DebugGlobalVariable to SpirvType.
  // Since debug local/global variable must have a debug type, SpirvEmitter sets
  // its QualType. Here we lower it to SpirvType and DebugTypeVisitor will lower
  // the SpirvType to debug type.
  if (auto *debugInstruction = dyn_cast<SpirvDebugInstruction>(instr)) {
    const QualType debugQualType = debugInstruction->getDebugQualType();
    if (!debugQualType.isNull()) {
      assert(isa<SpirvDebugLocalVariable>(debugInstruction) ||
             isa<SpirvDebugGlobalVariable>(debugInstruction));
      const SpirvType *spirvType =
          lowerType(debugQualType, instr->getLayoutRule(),
                    /*isRowMajor*/ llvm::None, instr->getSourceLocation());
      debugInstruction->setDebugSpirvType(spirvType);
    } else if (const auto *debugSpirvType =
                   debugInstruction->getDebugSpirvType()) {
      // When it does not have a QualType, either the type is already lowered,
      // or it's an HybridStructType we should lower.
      assert(isa<SpirvDebugGlobalVariable>(debugInstruction));
      if (isa<HybridType>(debugSpirvType)) {
        const SpirvType *loweredSpirvType = lowerType(
            debugSpirvType, instr->getLayoutRule(), instr->getSourceLocation());
        debugInstruction->setDebugSpirvType(loweredSpirvType);
      } else {
        debugInstruction->setDebugSpirvType(debugSpirvType);
      }
    }
  }

  // Instruction-specific type updates

  const auto *resultType = instr->getResultType();
  switch (instr->getopcode()) {
  case spv::Op::OpSampledImage: {
    // Wrap the image type in sampled image type if necessary.
    if (!isa<SampledImageType>(resultType)) {
      assert(isa<ImageType>(resultType));
      instr->setResultType(
          spvContext.getSampledImageType(cast<ImageType>(resultType)));
    }
    break;
  }
  // Variables and function parameters must have a pointer type.
  case spv::Op::OpFunctionParameter:
  case spv::Op::OpVariable: {
    if (auto *var = dyn_cast<SpirvVariable>(instr)) {
      if (var->hasBinding() && var->getHlslUserType().empty()) {
        var->setHlslUserType(getHlslResourceTypeName(var->getAstResultType()));
      }

      auto vkImgFeatures = spvContext.getVkImageFeaturesForSpirvVariable(var);
      if (vkImgFeatures.format) {
        if (const auto *imageType = dyn_cast<ImageType>(resultType)) {
          resultType =
              spvContext.getImageType(imageType, *vkImgFeatures.format);
          instr->setResultType(resultType);
        } else if (const auto *arrayType = dyn_cast<ArrayType>(resultType)) {
          if (const auto *imageType =
                  dyn_cast<ImageType>(arrayType->getElementType())) {
            auto newImgType = spvContext.getImageType(
                imageType,
                vkImgFeatures.format.value_or(spv::ImageFormat::Unknown));
            resultType = spvContext.getArrayType(newImgType,
                                                 arrayType->getElementCount(),
                                                 arrayType->getStride());
            instr->setResultType(resultType);
          }
        } else if (const auto *runtimeArrayType =
                       dyn_cast<RuntimeArrayType>(resultType)) {
          if (const auto *imageType =
                  dyn_cast<ImageType>(runtimeArrayType->getElementType())) {
            auto newImgType = spvContext.getImageType(
                imageType,
                vkImgFeatures.format.value_or(spv::ImageFormat::Unknown));
            resultType = spvContext.getRuntimeArrayType(
                newImgType, runtimeArrayType->getStride());
            instr->setResultType(resultType);
          }
        }
      }
    }
    const SpirvType *pointerType =
        spvContext.getPointerType(resultType, instr->getStorageClass());
    instr->setResultType(pointerType);
    break;
  }
  // Access chains must have a pointer type. The storage class for the pointer
  // is the same as the storage class of the access base.
  case spv::Op::OpAccessChain: {
    if (auto *acInst = dyn_cast<SpirvAccessChain>(instr)) {
      const auto *pointerType = spvContext.getPointerType(
          resultType, acInst->getBase()->getStorageClass());
      instr->setResultType(pointerType);
    }
    break;
  }
  // OpImageTexelPointer's result type must be a pointer with image storage
  // class.
  case spv::Op::OpImageTexelPointer: {
    const SpirvType *pointerType =
        spvContext.getPointerType(resultType, spv::StorageClass::Image);
    instr->setResultType(pointerType);
    break;
  }
  // Sparse image operations return a sparse residency struct.
  case spv::Op::OpImageSparseSampleImplicitLod:
  case spv::Op::OpImageSparseSampleExplicitLod:
  case spv::Op::OpImageSparseSampleDrefImplicitLod:
  case spv::Op::OpImageSparseSampleDrefExplicitLod:
  case spv::Op::OpImageSparseFetch:
  case spv::Op::OpImageSparseGather:
  case spv::Op::OpImageSparseDrefGather:
  case spv::Op::OpImageSparseRead: {
    const auto *uintType = spvContext.getUIntType(32);
    const auto *sparseResidencyStruct = spvContext.getStructType(
        {StructType::FieldInfo(uintType, /* fieldIndex*/ 0, "Residency.Code"),
         StructType::FieldInfo(resultType, /* fieldIndex*/ 1, "Result.Type")},
        "SparseResidencyStruct");
    instr->setResultType(sparseResidencyStruct);
    break;
  }
  case spv::Op::OpSwitch: {
    SpirvSwitch *spirvSwitch = cast<SpirvSwitch>(instr);
    // OpSwitch target literals must have the same type as the selector. Now
    // that the selector's AST type has been lowered, update the literals if
    // necessary.
    const SpirvType *selectorType = spirvSwitch->getSelector()->getResultType();
    // Selectors must have a type of OpTypeInt.
    assert(selectorType->getKind() == SpirvType::TK_Integer);
    uint32_t bitwidth = cast<IntegerType>(selectorType)->getBitwidth();
    for (auto &target : spirvSwitch->getTargets()) {
      if (target.first.getBitWidth() != bitwidth) {
        target.first = target.first.sextOrTrunc(bitwidth);
      }
    }
    break;
  }
  default:
    break;
  }

  // The instruction does not have a result-type, so nothing to do.
  return true;
}

std::vector<const HybridStructType::FieldInfo *> LowerTypeVisitor::sortFields(
    llvm::ArrayRef<HybridStructType::FieldInfo> fields) {
  std::vector<const HybridStructType::FieldInfo *> output;
  output.resize(fields.size());

  auto back_inserter = output.rbegin();
  std::map<uint32_t, const HybridStructType::FieldInfo *> fixed_fields;
  for (auto it = fields.rbegin(); it < fields.rend(); it++) {
    if (it->registerC) {
      auto insertionResult =
          fixed_fields.insert({it->registerC->RegisterNumber, &*it});
      if (!insertionResult.second) {
        emitError(
            "field \"%0\" at register(c%1) overlaps with previous members",
            it->registerC->Loc)
            << it->name << it->registerC->RegisterNumber;
      }
    } else {
      *back_inserter = &*it;
      back_inserter++;
    }
  }

  auto front_inserter = output.begin();
  for (const auto &item : fixed_fields) {
    *front_inserter = item.second;
    front_inserter++;
  }
  return output;
}

const SpirvType *LowerTypeVisitor::lowerType(const SpirvType *type,
                                             SpirvLayoutRule rule,
                                             SourceLocation loc) {
  if (const auto *hybridPointer = dyn_cast<HybridPointerType>(type)) {
    const QualType pointeeType = hybridPointer->getPointeeType();
    const SpirvType *pointeeSpirvType =
        lowerType(pointeeType, rule, /*isRowMajor*/ llvm::None, loc);
    return spvContext.getPointerType(pointeeSpirvType,
                                     hybridPointer->getStorageClass());
  } else if (const auto *hybridSampledImage =
                 dyn_cast<HybridSampledImageType>(type)) {
    const QualType imageAstType = hybridSampledImage->getImageType();
    const SpirvType *imageSpirvType =
        lowerType(imageAstType, rule, /*isRowMajor*/ llvm::None, loc);
    assert(isa<ImageType>(imageSpirvType));
    return spvContext.getSampledImageType(cast<ImageType>(imageSpirvType));
  } else if (const auto *hybridStruct = dyn_cast<HybridStructType>(type)) {
    // lower all fields of the struct.
    auto loweredFields =
        populateLayoutInformation(hybridStruct->getFields(), rule);
    const StructType *structType = spvContext.getStructType(
        loweredFields, hybridStruct->getStructName(),
        hybridStruct->isReadOnly(), hybridStruct->getInterfaceType());
    if (const auto *decl = spvContext.getStructDeclForSpirvType(type))
      spvContext.registerStructDeclForSpirvType(structType, decl);
    return structType;
  }
  // Void, bool, int, float cannot be further lowered.
  // Matrices cannot contain hybrid types. Only matrices of scalars are valid.
  // sampledType in image types can only be numberical type.
  // Sampler types cannot be further lowered.
  // SampledImage types cannot be further lowered.
  // FunctionType is not allowed to contain hybrid parameters or return type.
  // StructType is not allowed to contain any hybrid types.
  else if (isa<VoidType>(type) || isa<ScalarType>(type) ||
           isa<MatrixType>(type) || isa<ImageType>(type) ||
           isa<SamplerType>(type) || isa<SampledImageType>(type) ||
           isa<FunctionType>(type) || isa<StructType>(type)) {
    return type;
  }
  // Vectors could contain a hybrid type
  else if (const auto *vecType = dyn_cast<VectorType>(type)) {
    const auto *loweredElemType =
        lowerType(vecType->getElementType(), rule, loc);
    // If vector didn't contain any hybrid types, return itself.
    if (vecType->getElementType() == loweredElemType)
      return vecType;
    return spvContext.getVectorType(loweredElemType,
                                    vecType->getElementCount());
  }
  // Arrays could contain a hybrid type
  else if (const auto *arrType = dyn_cast<ArrayType>(type)) {
    const auto *loweredElemType =
        lowerType(arrType->getElementType(), rule, loc);
    // If array didn't contain any hybrid types, return itself.
    if (arrType->getElementType() == loweredElemType)
      return arrType;

    return spvContext.getArrayType(loweredElemType, arrType->getElementCount(),
                                   arrType->getStride());
  }
  // Runtime arrays could contain a hybrid type
  else if (const auto *raType = dyn_cast<RuntimeArrayType>(type)) {
    const auto *loweredElemType =
        lowerType(raType->getElementType(), rule, loc);
    // If runtime array didn't contain any hybrid types, return itself.
    if (raType->getElementType() == loweredElemType)
      return raType;
    return spvContext.getRuntimeArrayType(loweredElemType, raType->getStride());
  }
  // Node payload arrays could contain a hybrid type
  else if (const auto *npaType = dyn_cast<NodePayloadArrayType>(type)) {
    const auto *loweredElemType =
        lowerType(npaType->getElementType(), rule, loc);
    // If runtime array didn't contain any hybrid types, return itself.
    if (npaType->getElementType() == loweredElemType)
      return npaType;
    return spvContext.getNodePayloadArrayType(loweredElemType,
                                              npaType->getNodeDecl());
  }
  // Pointer types could point to a hybrid type.
  else if (const auto *ptrType = dyn_cast<SpirvPointerType>(type)) {
    const auto *loweredPointee =
        lowerType(ptrType->getPointeeType(), rule, loc);
    // If the pointer type didn't point to any hybrid type, return itself.
    if (ptrType->getPointeeType() == loweredPointee)
      return ptrType;

    return spvContext.getPointerType(loweredPointee,
                                     ptrType->getStorageClass());
  }

  llvm_unreachable("lowering of hybrid type not implemented");
}

const SpirvType *LowerTypeVisitor::lowerType(QualType type,
                                             SpirvLayoutRule rule,
                                             llvm::Optional<bool> isRowMajor,
                                             SourceLocation srcLoc) {
  const auto desugaredType = desugarType(type, &isRowMajor);

  if (desugaredType != type) {
    const auto *spvType = lowerType(desugaredType, rule, isRowMajor, srcLoc);
    return spvType;
  }

  { // Primitive types
    QualType ty = {};
    if (isScalarType(type, &ty)) {
      if (const auto *builtinType = ty->getAs<BuiltinType>()) {
        const bool use16Bit = getCodeGenOptions().enable16BitTypes;

        // Cases sorted roughly according to frequency in source code
        switch (builtinType->getKind()) {
          // 32-bit types
        case BuiltinType::Float:
          // The HalfFloat AST type is just an alias for the Float AST type
          // and is always 32-bit. The HLSL half keyword is translated to
          // HalfFloat if -enable-16bit-types is false.
        case BuiltinType::HalfFloat:
          return spvContext.getFloatType(32);
        case BuiltinType::Int:
          return spvContext.getSIntType(32);
        case BuiltinType::UInt:
        case BuiltinType::ULong:
        // The 'int8_t4_packed' and 'uint8_t4_packed' types are in fact 32-bit
        // unsigned integers.
        case BuiltinType::Int8_4Packed:
        case BuiltinType::UInt8_4Packed:
          return spvContext.getUIntType(32);

          // void and bool
        case BuiltinType::Void:
          return spvContext.getVoidType();
        case BuiltinType::Bool:
          // According to the SPIR-V spec, there is no physical size or bit
          // pattern defined for boolean type. Therefore an unsigned integer
          // is used to represent booleans when layout is required.
          if (rule == SpirvLayoutRule::Void)
            return spvContext.getBoolType();
          else
            return spvContext.getUIntType(32);

          // 64-bit types
        case BuiltinType::Double:
          return spvContext.getFloatType(64);
        case BuiltinType::LongLong:
          return spvContext.getSIntType(64);
        case BuiltinType::ULongLong:
          return spvContext.getUIntType(64);

          // 16-bit types
          // The Half AST type is always 16-bit. The HLSL half keyword is
          // translated to Half if -enable-16bit-types is true.
        case BuiltinType::Half:
          return spvContext.getFloatType(16);
        case BuiltinType::Short: // int16_t
          return spvContext.getSIntType(16);
        case BuiltinType::UShort: // uint16_t
          return spvContext.getUIntType(16);

        // 8-bit integer types
        case BuiltinType::UChar:
        case BuiltinType::Char_U:
          return spvContext.getUIntType(8);
        case BuiltinType::SChar:
        case BuiltinType::Char_S:
          return spvContext.getSIntType(8);

          // Relaxed precision types
        case BuiltinType::Min10Float:
        case BuiltinType::Min16Float:
          return spvContext.getFloatType(use16Bit ? 16 : 32);
        case BuiltinType::Min12Int:
        case BuiltinType::Min16Int:
          return spvContext.getSIntType(use16Bit ? 16 : 32);
        case BuiltinType::Min16UInt:
          return spvContext.getUIntType(use16Bit ? 16 : 32);

        // All literal types should have been lowered to concrete types before
        // LowerTypeVisitor is invoked. However, if there are unused literals,
        // they will still have 'literal' type when we get to this point. Use
        // 32-bit width by default for these cases.
        // Example:
        // void main() { 1.0; 1; }
        case BuiltinType::LitInt:
          return type->isSignedIntegerType() ? spvContext.getSIntType(32)
                                             : spvContext.getUIntType(32);
        case BuiltinType::LitFloat: {
          return spvContext.getFloatType(32);

        default:
          emitError("primitive type %0 unimplemented", srcLoc)
              << builtinType->getTypeClassName();
          return spvContext.getVoidType();
        }
        }
      }
    }
  }

  // AST vector/matrix types are TypedefType of TemplateSpecializationType. We
  // handle them via HLSL type inspection functions.

  // When the memory layout rule is FxcCTBuffer, typeNxM matrix with M > 1 and
  // N == 1 consists of M vectors where each vector has a single element. Since
  // SPIR-V does not have a vector with single element, we have to use an
  // OpTypeArray with ArrayStride 16 instead of OpTypeVector. We have the same
  // rule for column_major typeNxM and row_major typeMxN.
  if (rule == SpirvLayoutRule::FxcCTBuffer && hlsl::IsHLSLMatType(type)) {
    uint32_t rowCount = 0, colCount = 0;
    hlsl::GetHLSLMatRowColCount(type, rowCount, colCount);
    if (!alignmentCalc.useRowMajor(isRowMajor, type))
      std::swap(rowCount, colCount);
    if (rowCount == 1) {
      useArrayForMat1xN = true;
      auto elemType = hlsl::GetHLSLMatElementType(type);
      uint32_t stride = 0;
      alignmentCalc.getAlignmentAndSize(type, rule, isRowMajor, &stride);
      return spvContext.getArrayType(
          lowerType(elemType, rule, isRowMajor, srcLoc), colCount, stride);
    }
  }

  { // Vector types
    QualType elemType = {};
    uint32_t elemCount = {};
    if (isVectorType(type, &elemType, &elemCount))
      return spvContext.getVectorType(
          lowerType(elemType, rule, isRowMajor, srcLoc), elemCount);
  }

  { // Matrix types
    QualType elemType = {};
    uint32_t rowCount = 0, colCount = 0;
    if (isMxNMatrix(type, &elemType, &rowCount, &colCount)) {
      const auto *vecType = spvContext.getVectorType(
          lowerType(elemType, rule, isRowMajor, srcLoc), colCount);

      // Non-float matrices are represented as an array of vectors.
      if (!elemType->isFloatingType()) {
        llvm::Optional<uint32_t> arrayStride = llvm::None;
        // If there is a layout rule, we need array stride information.
        if (rule != SpirvLayoutRule::Void) {
          uint32_t stride = 0;
          alignmentCalc.getAlignmentAndSize(type, rule, isRowMajor, &stride);
          arrayStride = stride;
        }

        // This return type is ArrayType.
        return spvContext.getArrayType(vecType, rowCount, arrayStride);
      }

      return spvContext.getMatrixType(vecType, rowCount);
    }
  }

  // Struct type
  if (const auto *structType = type->getAs<RecordType>()) {
    const auto *decl = structType->getDecl();

    // HLSL resource types are also represented as RecordType in the AST.
    // (ClassTemplateSpecializationDecl is a subclass of CXXRecordDecl, which
    // is then a subclass of RecordDecl.) So we need to check them before
    // checking the general struct type.
    if (const auto *spvType =
            lowerResourceType(type, rule, isRowMajor, srcLoc)) {
      if (!isa<SpirvPointerType>(spvType)) {
        spvContext.registerStructDeclForSpirvType(spvType, decl);
      }
      return spvType;
    }

    auto loweredFields = lowerStructFields(decl, rule);

    const auto *spvStructType =
        spvContext.getStructType(loweredFields, decl->getName());
    spvContext.registerStructDeclForSpirvType(spvStructType, decl);
    return spvStructType;
  }

  // Array type
  if (const auto *arrayType = astContext.getAsArrayType(type)) {
    const auto elemType = arrayType->getElementType();

    // If layout rule is void, it means these resource types are used for
    // declaring local resources. This should be lowered to a pointer to the
    // array.
    //
    // The pointer points to the Uniform storage class, and the element type
    // should have the corresponding layout.
    bool isLocalStructuredOrByteBuffer =
        isAKindOfStructuredOrByteBuffer(elemType) &&
        rule == SpirvLayoutRule::Void;

    SpirvLayoutRule elementLayoutRule =
        (isLocalStructuredOrByteBuffer ? getCodeGenOptions().sBufferLayoutRule
                                       : rule);
    const SpirvType *loweredElemType =
        lowerType(elemType, elementLayoutRule, isRowMajor, srcLoc);

    llvm::Optional<uint32_t> arrayStride = llvm::None;
    if (rule != SpirvLayoutRule::Void &&
        // We won't have stride information for structured/byte buffers since
        // they contain runtime arrays.
        !isAKindOfStructuredOrByteBuffer(elemType) &&
        !isConstantTextureBuffer(elemType)) {
      uint32_t stride = 0;
      alignmentCalc.getAlignmentAndSize(type, rule, isRowMajor, &stride);
      arrayStride = stride;
    }

    const SpirvType *spirvArrayType = nullptr;
    if (const auto *caType = astContext.getAsConstantArrayType(type)) {
      const auto size = static_cast<uint32_t>(caType->getSize().getZExtValue());
      spirvArrayType =
          spvContext.getArrayType(loweredElemType, size, arrayStride);
    } else {
      assert(type->isIncompleteArrayType());
      spirvArrayType =
          spvContext.getRuntimeArrayType(loweredElemType, arrayStride);
    }

    if (isLocalStructuredOrByteBuffer) {
      return spvContext.getPointerType(spirvArrayType,
                                       spv::StorageClass::Uniform);
    }

    return spirvArrayType;
  }

  // Reference types
  if (const auto *refType = type->getAs<ReferenceType>()) {
    // Note: Pointer/reference types are disallowed in HLSL source code.
    // Although developers cannot use them directly, they are generated into
    // the AST by out/inout parameter modifiers in function signatures.
    // We already pass function arguments via pointers to tempoary local
    // variables. So it should be fine to drop the pointer type and treat it
    // as the underlying pointee type here.
    return lowerType(refType->getPointeeType(), rule, isRowMajor, srcLoc);
  }

  // Pointer types
  if (const auto *ptrType = type->getAs<PointerType>()) {
    // The this object in a struct member function is of pointer type.
    return lowerType(ptrType->getPointeeType(), rule, isRowMajor, srcLoc);
  }

  // Enum types
  if (isEnumType(type)) {
    return spvContext.getSIntType(32);
  }

  // Templated types.
  if (const auto *spec = type->getAs<TemplateSpecializationType>()) {
    return lowerType(spec->desugar(), rule, isRowMajor, srcLoc);
  }
  if (const auto *spec = type->getAs<SubstTemplateTypeParmType>()) {
    return lowerType(spec->desugar(), rule, isRowMajor, srcLoc);
  }

  emitError("lower type %0 unimplemented", srcLoc) << type->getTypeClassName();
  type->dump();
  return 0;
}

QualType LowerTypeVisitor::createASTTypeFromTemplateName(TemplateName name) {
  auto *decl = name.getAsTemplateDecl();
  if (decl == nullptr) {
    return QualType();
  }

  auto *classTemplateDecl = dyn_cast<ClassTemplateDecl>(decl);
  if (classTemplateDecl == nullptr) {
    return QualType();
  }

  TemplateParameterList *parameters =
      classTemplateDecl->getTemplateParameters();
  if (parameters->size() != 1) {
    return QualType();
  }

  auto *parmDecl = dyn_cast<TemplateTypeParmDecl>(parameters->getParam(0));
  if (parmDecl == nullptr) {
    return QualType();
  }

  if (!parmDecl->hasDefaultArgument()) {
    return QualType();
  }

  TemplateArgument *arg =
      new (context) TemplateArgument(parmDecl->getDefaultArgument());

  auto *specialized = ClassTemplateSpecializationDecl::Create(
      astContext, TagDecl::TagKind::TTK_Class,
      classTemplateDecl->getDeclContext(), classTemplateDecl->getLocStart(),
      classTemplateDecl->getLocStart(), classTemplateDecl, /* Args */ arg,
      /* NumArgs */ 1,
      /* PrevDecl */ nullptr);
  QualType type = astContext.getTypeDeclType(specialized);

  return type;
}

bool LowerTypeVisitor::getVkIntegralConstantValue(QualType type,
                                                  SpirvConstant *&result,
                                                  SourceLocation srcLoc) {
  auto *recordType = type->getAs<RecordType>();
  if (!recordType)
    return false;
  if (!isTypeInVkNamespace(recordType))
    return false;

  if (recordType->getDecl()->getName() == "Literal") {
    auto *specDecl =
        dyn_cast<ClassTemplateSpecializationDecl>(recordType->getDecl());
    assert(specDecl);

    const TemplateArgumentList &args = specDecl->getTemplateArgs();
    QualType constant = args[0].getAsType();
    bool val = getVkIntegralConstantValue(constant, result, srcLoc);

    if (val) {
      result->setLiteral(true);
    } else {
      emitError("The template argument to vk::Literal must be a "
                "vk::integral_constant",
                srcLoc);
    }
    return true;
  }

  if (recordType->getDecl()->getName() != "integral_constant")
    return false;

  auto *specDecl =
      dyn_cast<ClassTemplateSpecializationDecl>(recordType->getDecl());
  assert(specDecl);

  const TemplateArgumentList &args = specDecl->getTemplateArgs();

  QualType constantType = args[0].getAsType();
  llvm::APSInt value = args[1].getAsIntegral();
  result = ConstEvaluator(astContext, spvBuilder)
               .translateAPValue(APValue(value), constantType, false);
  return true;
}

const SpirvType *LowerTypeVisitor::lowerInlineSpirvType(
    llvm::StringRef name, unsigned int opcode,
    const ClassTemplateSpecializationDecl *specDecl, SpirvLayoutRule rule,
    llvm::Optional<bool> isRowMajor, SourceLocation srcLoc) {
  assert(specDecl);

  SmallVector<SpvIntrinsicTypeOperand, 4> operands;

  // Lower each operand argument

  size_t operandsIndex = 1;
  if (name == "SpirvType")
    operandsIndex = 3;

  auto args = specDecl->getTemplateArgs()[operandsIndex].getPackAsArray();

  if (operandsIndex == 1 && args.size() == 2 &&
      static_cast<spv::Op>(opcode) == spv::Op::OpTypePointer) {
    const SpirvType *result =
        getSpirvPointerFromInlineSpirvType(args, rule, isRowMajor, srcLoc);
    if (result) {
      return result;
    }
  }

  for (TemplateArgument arg : args) {
    switch (arg.getKind()) {
    case TemplateArgument::ArgKind::Type: {
      QualType typeArg = arg.getAsType();

      SpirvConstant *constant = nullptr;
      if (getVkIntegralConstantValue(typeArg, constant, srcLoc)) {
        if (constant) {
          visitInstruction(constant);
          operands.emplace_back(constant);
        }
      } else {
        operands.emplace_back(lowerType(typeArg, rule, isRowMajor, srcLoc));
      }
      break;
    }
    case TemplateArgument::ArgKind::Template: {
      // Handle HLSL template types that allow the omission of < and >; for
      // example, Texture2D
      TemplateName templateName = arg.getAsTemplate();
      QualType typeArg = createASTTypeFromTemplateName(templateName);
      assert(!typeArg.isNull() &&
             "Could not create HLSL type from template name");

      operands.emplace_back(lowerType(typeArg, rule, isRowMajor, srcLoc));
      break;
    }
    default:
      emitError("template argument kind %0 unimplemented", srcLoc)
          << arg.getKind();
    }
  }
  return spvContext.getOrCreateSpirvIntrinsicType(opcode, operands);
}

const SpirvType *LowerTypeVisitor::lowerVkTypeInVkNamespace(
    QualType type, llvm::StringRef name, SpirvLayoutRule rule,
    llvm::Optional<bool> isRowMajor, SourceLocation srcLoc) {
  if (name == "SpirvType" || name == "SpirvOpaqueType") {
    auto opcode = hlsl::GetHLSLResourceTemplateUInt(type);
    auto *specDecl = dyn_cast<ClassTemplateSpecializationDecl>(
        type->getAs<RecordType>()->getDecl());

    return lowerInlineSpirvType(name, opcode, specDecl, rule, isRowMajor,
                                srcLoc);
  }
  if (name == "ext_type") {
    auto typeId = hlsl::GetHLSLResourceTemplateUInt(type);
    return spvContext.getCreatedSpirvIntrinsicType(typeId);
  }
  if (name == "ext_result_id") {
    QualType realType = hlsl::GetHLSLResourceTemplateParamType(type);
    return lowerType(realType, rule, llvm::None, srcLoc);
  }
  if (name == "BufferPointer") {
    const size_t visitedTypeStackSize = visitedTypeStack.size();
    (void)visitedTypeStackSize; // suppress unused warning (used only in assert)

    for (QualType t : visitedTypeStack) {
      if (t == type) {
        return spvContext.getForwardPointerType(type);
      }
    }

    QualType realType = hlsl::GetHLSLResourceTemplateParamType(type);
    if (rule == SpirvLayoutRule::Void) {
      rule = spvOptions.sBufferLayoutRule;
    }
    visitedTypeStack.push_back(type);

    const SpirvType *spirvType = lowerType(realType, rule, llvm::None, srcLoc);
    const auto *pointerType = spvContext.getPointerType(
        spirvType, spv::StorageClass::PhysicalStorageBuffer);
    spvContext.registerForwardReference(type, pointerType);

    assert(visitedTypeStack.back() == type);
    visitedTypeStack.pop_back();
    assert(visitedTypeStack.size() == visitedTypeStackSize);
    return pointerType;
  }
  emitError("unknown type %0 in vk namespace", srcLoc) << type;
  return nullptr;
}

const SpirvType *
LowerTypeVisitor::lowerResourceType(QualType type, SpirvLayoutRule rule,
                                    llvm::Optional<bool> isRowMajor,
                                    SourceLocation srcLoc) {
  // Resource types are either represented like C struct or C++ class in the
  // AST. Samplers are represented like C struct, so isStructureType() will
  // return true for it; textures are represented like C++ class, so
  // isClassType() will return true for it.

  assert(type->isStructureOrClassType());

  const auto *recordType = type->getAs<RecordType>();
  assert(recordType);
  const llvm::StringRef name = recordType->getDecl()->getName();

  if (isTypeInVkNamespace(recordType)) {
    return lowerVkTypeInVkNamespace(type, name, rule, isRowMajor, srcLoc);
  }

  // TODO: avoid string comparison once hlsl::IsHLSLResouceType() does that.

  { // Texture types
    spv::Dim dim = {};
    bool isArray = {};
    if ((dim = spv::Dim::Dim1D, isArray = false, name == "Texture1D") ||
        (dim = spv::Dim::Dim2D, isArray = false, name == "Texture2D") ||
        (dim = spv::Dim::Dim3D, isArray = false, name == "Texture3D") ||
        (dim = spv::Dim::Cube, isArray = false, name == "TextureCube") ||
        (dim = spv::Dim::Dim1D, isArray = true, name == "Texture1DArray") ||
        (dim = spv::Dim::Dim2D, isArray = true, name == "Texture2DArray") ||
        (dim = spv::Dim::Dim2D, isArray = false, name == "Texture2DMS") ||
        (dim = spv::Dim::Dim2D, isArray = true, name == "Texture2DMSArray") ||
        // There is no Texture3DArray
        (dim = spv::Dim::Cube, isArray = true, name == "TextureCubeArray")) {
      const bool isMS = (name == "Texture2DMS" || name == "Texture2DMSArray");
      const auto sampledType = hlsl::GetHLSLResourceResultType(type);
      auto loweredType =
          lowerType(getElementType(astContext, sampledType), rule,
                    /*isRowMajor*/ llvm::None, srcLoc);
      // Treat bool textures as uint for compatibility with OpTypeImage.
      if (loweredType == spvContext.getBoolType()) {
        loweredType = spvContext.getUIntType(32);
      }
      return spvContext.getImageType(
          loweredType, dim, ImageType::WithDepth::Unknown, isArray, isMS,
          ImageType::WithSampler::Yes, spv::ImageFormat::Unknown);
    }

    // There is no RWTexture3DArray
    if ((dim = spv::Dim::Dim1D, isArray = false,
         name == "RWTexture1D" || name == "RasterizerOrderedTexture1D") ||
        (dim = spv::Dim::Dim2D, isArray = false,
         name == "RWTexture2D" || name == "RasterizerOrderedTexture2D") ||
        (dim = spv::Dim::Dim3D, isArray = false,
         name == "RWTexture3D" || name == "RasterizerOrderedTexture3D") ||
        (dim = spv::Dim::Dim1D, isArray = true,
         name == "RWTexture1DArray" ||
             name == "RasterizerOrderedTexture1DArray") ||
        (dim = spv::Dim::Dim2D, isArray = true,
         name == "RWTexture2DArray" ||
             name == "RasterizerOrderedTexture2DArray")) {
      const auto sampledType = hlsl::GetHLSLResourceResultType(type);
      const auto format =
          translateSampledTypeToImageFormat(sampledType, srcLoc);
      return spvContext.getImageType(
          lowerType(getElementType(astContext, sampledType), rule,
                    /*isRowMajor*/ llvm::None, srcLoc),
          dim, ImageType::WithDepth::Unknown, isArray,
          /*isMultiSampled=*/false, /*sampled=*/ImageType::WithSampler::No,
          format);
    }
  }

  // Sampler types
  if (name == "SamplerState" || name == "SamplerComparisonState") {
    return spvContext.getSamplerType();
  }

  if (name == "RaytracingAccelerationStructure") {
    return spvContext.getAccelerationStructureTypeNV();
  }

  if (name == "RayQuery")
    return spvContext.getRayQueryTypeKHR();

  if (name == "StructuredBuffer" || name == "RWStructuredBuffer" ||
      name == "RasterizerOrderedStructuredBuffer" ||
      name == "AppendStructuredBuffer" || name == "ConsumeStructuredBuffer") {
    // StructureBuffer<S> will be translated into an OpTypeStruct with one
    // field, which is an OpTypeRuntimeArray of OpTypeStruct (S).

    // If layout rule is void, it means these resource types are used for
    // declaring local resources, which should be created as alias variables.
    // The aliased-to variable should surely be in the Uniform storage class,
    // which has layout decorations.
    bool asAlias = false;
    if (rule == SpirvLayoutRule::Void) {
      asAlias = true;
      rule = getCodeGenOptions().sBufferLayoutRule;
    }

    // Get the underlying resource type.
    const auto s = hlsl::GetHLSLResourceResultType(type);

    // If the underlying type is a matrix, check majorness.
    llvm::Optional<bool> isRowMajor = llvm::None;
    if (isMxNMatrix(s))
      isRowMajor = isRowMajorMatrix(spvOptions, type);

    // Lower the underlying type.
    const auto *structType = lowerType(s, rule, isRowMajor, srcLoc);

    // Calculate memory alignment for the resource.
    uint32_t arrayStride = 0;
    QualType sArray = astContext.getConstantArrayType(
        s, llvm::APInt(32, 1), clang::ArrayType::Normal, 0);
    alignmentCalc.getAlignmentAndSize(sArray, rule, isRowMajor, &arrayStride);

    // We have a runtime array of structures. So:
    // The stride of the runtime array is the size of the struct.
    const auto *raType =
        spvContext.getRuntimeArrayType(structType, arrayStride);
    const bool isReadOnly = (name == "StructuredBuffer");

    // Attach matrix stride decorations if this is a *StructuredBuffer<matrix>.
    llvm::Optional<uint32_t> matrixStride = llvm::None;
    if (isMxNMatrix(s)) {
      uint32_t stride = 0;
      alignmentCalc.getAlignmentAndSize(s, rule, isRowMajor, &stride);
      matrixStride = stride;
    }

    const std::string typeName = "type." + name.str() + "." + getAstTypeName(s);
    const auto *valType = spvContext.getStructType(
        {StructType::FieldInfo(raType, /* fieldIndex*/ 0, /*name*/ "",
                               /*offset*/ 0, matrixStride, isRowMajor)},
        typeName, isReadOnly, StructInterfaceType::StorageBuffer);

    if (asAlias) {
      // All structured buffers are in the Uniform storage class.
      return spvContext.getPointerType(valType, spv::StorageClass::Uniform);
    }

    return valType;
  }

  if (name == "ConstantBuffer" || name == "TextureBuffer") {
    // ConstantBuffer<T> and TextureBuffer<T> are lowered as T

    const bool forTBuffer = name == "TextureBuffer";

    if (rule == SpirvLayoutRule::Void) {
      rule = forTBuffer ? getCodeGenOptions().tBufferLayoutRule
                        : getCodeGenOptions().cBufferLayoutRule;
    }

    const auto *bufferType = type->getAs<RecordType>();
    assert(bufferType);
    const auto *bufferDecl = bufferType->getDecl();

    // Get the underlying resource type.
    const auto underlyingType = hlsl::GetHLSLResourceResultType(type);

    const auto *underlyingStructType = underlyingType->getAs<RecordType>();
    assert(underlyingStructType &&
           "T in ConstantBuffer<T> or TextureBuffer<T> must be a struct type");

    const auto *underlyingStructDecl = underlyingStructType->getDecl();

    auto loweredFields = lowerStructFields(underlyingStructDecl, rule);

    const std::string structName = "type." + bufferDecl->getName().str() + "." +
                                   underlyingStructDecl->getName().str();

    const auto *spvStructType = spvContext.getStructType(
        loweredFields, structName, /*isReadOnly*/ forTBuffer,
        forTBuffer ? StructInterfaceType::StorageBuffer
                   : StructInterfaceType::UniformBuffer);

    spvContext.registerStructDeclForSpirvType(spvStructType, bufferDecl);
    return spvStructType;
  }

  // ByteAddressBuffer and RWByteAddressBuffer types.
  if (name == "ByteAddressBuffer" || name == "RWByteAddressBuffer" ||
      name == "RasterizerOrderedByteAddressBuffer") {
    const auto *bufferType = spvContext.getByteAddressBufferType(
        /*isRW*/ name != "ByteAddressBuffer");
    if (rule == SpirvLayoutRule::Void) {
      // All byte address buffers are in the Uniform storage class.
      return spvContext.getPointerType(bufferType, spv::StorageClass::Uniform);
    }
    return bufferType;
  }

  // Buffer and RWBuffer types
  if (name == "Buffer" || name == "RWBuffer" ||
      name == "RasterizerOrderedBuffer") {
    const auto sampledType = hlsl::GetHLSLResourceResultType(type);
    const auto format = translateSampledTypeToImageFormat(sampledType, srcLoc);
    return spvContext.getImageType(
        lowerType(getElementType(astContext, sampledType), rule,
                  /*isRowMajor*/ llvm::None, srcLoc),
        spv::Dim::Buffer, ImageType::WithDepth::Unknown,
        /*isArrayed=*/false, /*isMultiSampled=*/false,
        /*sampled*/ name == "Buffer" ? ImageType::WithSampler::Yes
                                     : ImageType::WithSampler::No,
        format);
  }

  // InputPatch
  if (name == "InputPatch") {
    const auto elemType = hlsl::GetHLSLInputPatchElementType(type);
    const auto elemCount = hlsl::GetHLSLInputPatchCount(type);
    return spvContext.getArrayType(
        lowerType(elemType, rule, /*isRowMajor*/ llvm::None, srcLoc), elemCount,
        /*ArrayStride*/ llvm::None);
  }
  // OutputPatch
  if (name == "OutputPatch") {
    const auto elemType = hlsl::GetHLSLOutputPatchElementType(type);
    const auto elemCount = hlsl::GetHLSLOutputPatchCount(type);
    return spvContext.getArrayType(
        lowerType(elemType, rule, /*isRowMajor*/ llvm::None, srcLoc), elemCount,
        /*ArrayStride*/ llvm::None);
  }
  // Output stream objects (TriangleStream, LineStream, and PointStream)
  if (name == "TriangleStream" || name == "LineStream" ||
      name == "PointStream") {
    return lowerType(hlsl::GetHLSLResourceResultType(type), rule,
                     /*isRowMajor*/ llvm::None, srcLoc);
  }

  if (name == "SubpassInput" || name == "SubpassInputMS") {
    const auto sampledType = hlsl::GetHLSLResourceResultType(type);
    return spvContext.getImageType(
        lowerType(getElementType(astContext, sampledType), rule,
                  /*isRowMajor*/ llvm::None, srcLoc),
        spv::Dim::SubpassData, ImageType::WithDepth::Unknown,
        /*isArrayed=*/false,
        /*isMultipleSampled=*/name == "SubpassInputMS",
        ImageType::WithSampler::No, spv::ImageFormat::Unknown);
  }

  return nullptr;
}

llvm::SmallVector<StructType::FieldInfo, 4>
LowerTypeVisitor::lowerStructFields(const RecordDecl *decl,
                                    SpirvLayoutRule rule) {
  assert(decl);

  // Collect all fields' information.
  llvm::SmallVector<HybridStructType::FieldInfo, 8> fields;

  // If this struct is derived from some other struct, place an implicit
  // field at the very beginning for the base struct.
  if (const auto *cxxDecl = dyn_cast<CXXRecordDecl>(decl)) {
    for (const auto &base : cxxDecl->bases()) {
      fields.push_back(HybridStructType::FieldInfo(base.getType()));
    }
  }

  // Create fields for all members of this struct
  for (const auto *field : decl->fields()) {
    llvm::Optional<BitfieldInfo> bitfieldInfo;
    if (field->isBitField()) {
      bitfieldInfo = BitfieldInfo();
      bitfieldInfo->sizeInBits =
          field->getBitWidthValue(field->getASTContext());
    }

    llvm::Optional<AttrVec> attributes;
    if (field->hasAttrs()) {
      attributes.emplace();
      for (auto attr : field->getAttrs()) {
        if (auto capAttr = dyn_cast<VKCapabilityExtAttr>(attr)) {
          spvBuilder.requireCapability(
              static_cast<spv::Capability>(capAttr->getCapability()),
              capAttr->getLocation());
        } else if (auto extAttr = dyn_cast<VKExtensionExtAttr>(attr)) {
          spvBuilder.requireExtension(extAttr->getName(),
                                      extAttr->getLocation());
        } else {
          attributes->push_back(attr);
        }
      }
    }

    fields.push_back(HybridStructType::FieldInfo(
        field->getType(), field->getName(),
        /*vkoffset*/ field->getAttr<VKOffsetAttr>(),
        /*packoffset*/ getPackOffset(field),
        /*RegisterAssignment*/ nullptr,
        /*isPrecise*/ field->hasAttr<HLSLPreciseAttr>(),
        /*bitfield*/ bitfieldInfo,
        /* attributes */ attributes));
  }

  return populateLayoutInformation(fields, rule);
}

spv::ImageFormat
LowerTypeVisitor::translateSampledTypeToImageFormat(QualType sampledType,
                                                    SourceLocation srcLoc) {

  if (spvOptions.useUnknownImageFormat)
    return spv::ImageFormat::Unknown;

  uint32_t elemCount = 1;
  QualType ty = {};
  if (!isScalarType(sampledType, &ty) &&
      !isVectorType(sampledType, &ty, &elemCount) &&
      !canFitIntoOneRegister(astContext, sampledType, &ty, &elemCount)) {
    return spv::ImageFormat::Unknown;
  }

  const auto *builtinType = ty->getAs<BuiltinType>();
  if (builtinType == nullptr) {
    return spv::ImageFormat::Unknown;
  }

  switch (builtinType->getKind()) {
  case BuiltinType::Int:
    return elemCount == 1   ? spv::ImageFormat::R32i
           : elemCount == 2 ? spv::ImageFormat::Rg32i
           : elemCount == 4 ? spv::ImageFormat::Rgba32i
                            : spv::ImageFormat::Unknown;
  case BuiltinType::Min12Int:
  case BuiltinType::Min16Int:
    return elemCount == 1   ? spv::ImageFormat::R16i
           : elemCount == 2 ? spv::ImageFormat::Rg16i
           : elemCount == 4 ? spv::ImageFormat::Rgba16i
                            : spv::ImageFormat::Unknown;
  case BuiltinType::UInt:
    return elemCount == 1   ? spv::ImageFormat::R32ui
           : elemCount == 2 ? spv::ImageFormat::Rg32ui
           : elemCount == 4 ? spv::ImageFormat::Rgba32ui
                            : spv::ImageFormat::Unknown;
  case BuiltinType::Min16UInt:
    return elemCount == 1   ? spv::ImageFormat::R16ui
           : elemCount == 2 ? spv::ImageFormat::Rg16ui
           : elemCount == 4 ? spv::ImageFormat::Rgba16ui
                            : spv::ImageFormat::Unknown;
  case BuiltinType::Float:
    return elemCount == 1   ? spv::ImageFormat::R32f
           : elemCount == 2 ? spv::ImageFormat::Rg32f
           : elemCount == 4 ? spv::ImageFormat::Rgba32f
                            : spv::ImageFormat::Unknown;
  case BuiltinType::HalfFloat:
  case BuiltinType::Min10Float:
  case BuiltinType::Min16Float:
    return elemCount == 1   ? spv::ImageFormat::R16f
           : elemCount == 2 ? spv::ImageFormat::Rg16f
           : elemCount == 4 ? spv::ImageFormat::Rgba16f
                            : spv::ImageFormat::Unknown;
  case BuiltinType::LongLong:
    return elemCount == 1 ? spv::ImageFormat::R64i : spv::ImageFormat::Unknown;
  case BuiltinType::ULongLong:
    return elemCount == 1 ? spv::ImageFormat::R64ui : spv::ImageFormat::Unknown;
  default:
    // Other sampled types unimplemented or irrelevant.
    break;
  }

  return spv::ImageFormat::Unknown;
}

StructType::FieldInfo
LowerTypeVisitor::lowerField(const HybridStructType::FieldInfo *field,
                             SpirvLayoutRule rule, const uint32_t fieldIndex) {
  auto fieldType = field->astType;
  // Lower the field type fist. This call will populate proper matrix
  // majorness information.
  StructType::FieldInfo loweredField(
      lowerType(fieldType, rule, /*isRowMajor*/ llvm::None, {}), fieldIndex,
      field->name);

  // Set RelaxedPrecision information for the lowered field.
  if (isRelaxedPrecisionType(fieldType, spvOptions)) {
    loweredField.isRelaxedPrecision = true;
  }
  if (field->isPrecise) {
    loweredField.isPrecise = true;
  }
  loweredField.bitfield = field->bitfield;
  loweredField.attributes = field->attributes;

  // We only need layout information for structures with non-void layout rule.
  if (rule == SpirvLayoutRule::Void) {
    return loweredField;
  }

  // Each structure-type member that is a matrix or array-of-matrices must be
  // decorated with
  // * A MatrixStride decoration, and
  // * one of the RowMajor or ColMajor Decorations.
  if (const auto *arrayType = astContext.getAsConstantArrayType(fieldType)) {
    // We have an array of matrices as a field, we need to decorate
    // MatrixStride on the field. So skip possible arrays here.
    fieldType = arrayType->getElementType();
  }

  // Non-floating point matrices are represented as arrays of vectors, and
  // therefore ColMajor and RowMajor decorations should not be applied to
  // them.
  QualType elemType = {};
  if (isMxNMatrix(fieldType, &elemType) && elemType->isFloatingType()) {
    uint32_t stride = 0;
    alignmentCalc.getAlignmentAndSize(fieldType, rule,
                                      /*isRowMajor*/ llvm::None, &stride);
    loweredField.matrixStride = stride;
    loweredField.isRowMajor = isRowMajorMatrix(spvOptions, fieldType);
  }
  return loweredField;
}

llvm::SmallVector<StructType::FieldInfo, 4>
LowerTypeVisitor::populateLayoutInformation(
    llvm::ArrayRef<HybridStructType::FieldInfo> fields, SpirvLayoutRule rule) {

  auto fieldVisitor = [this,
                       &rule](const StructType::FieldInfo *previousField,
                              const HybridStructType::FieldInfo *currentField,
                              const uint32_t nextFieldIndex) {
    StructType::FieldInfo loweredField =
        lowerField(currentField, rule, nextFieldIndex);
    setDefaultFieldSize(alignmentCalc, rule, currentField, &loweredField);

    // We only need size information for structures with non-void layout &
    // non-bitfield fields.
    if (rule == SpirvLayoutRule::Void && !currentField->bitfield.hasValue())
      return loweredField;

    // We only need layout information for structures with non-void layout rule.
    if (rule != SpirvLayoutRule::Void) {
      const uint32_t previousFieldEnd =
          previousField ? previousField->offset.getValue() +
                              previousField->sizeInBytes.getValue()
                        : 0;
      setDefaultFieldOffset(alignmentCalc, rule, previousFieldEnd, currentField,
                            &loweredField);

      // The vk::offset attribute takes precedence over all.
      if (currentField->vkOffsetAttr) {
        loweredField.offset = currentField->vkOffsetAttr->getOffset();
        return loweredField;
      }

      // The :packoffset() annotation takes precedence over normal layout
      // calculation.
      if (currentField->packOffsetAttr) {
        const uint32_t offset =
            currentField->packOffsetAttr->Subcomponent * 16 +
            currentField->packOffsetAttr->ComponentOffset * 4;
        // Do minimal check to make sure the offset specified by packoffset does
        // not cause overlap.
        if (offset < previousFieldEnd) {
          emitError("packoffset caused overlap with previous members",
                    currentField->packOffsetAttr->Loc);
        }

        loweredField.offset = offset;
        return loweredField;
      }

      // The :register(c#) annotation takes precedence over normal layout
      // calculation.
      if (currentField->registerC) {
        const uint32_t offset = 16 * currentField->registerC->RegisterNumber;
        // Do minimal check to make sure the offset specified by :register(c#)
        // does not cause overlap.
        if (offset < previousFieldEnd) {
          emitError(
              "found offset overlap when processing register(c%0) assignment",
              currentField->registerC->Loc)
              << currentField->registerC->RegisterNumber;
        }

        loweredField.offset = offset;
        return loweredField;
      }
    }

    if (!currentField->bitfield.hasValue())
      return loweredField;

    // Previous field is a full type, cannot merge.
    if (!previousField || !previousField->bitfield.hasValue())
      return loweredField;

    // Bitfields can only be merged if they have the exact base type.
    // (SPIR-V cannot handle mixed-types bitfields).
    if (previousField->type != loweredField.type)
      return loweredField;

    const uint32_t basetypeSize = previousField->sizeInBytes.getValue() * 8;
    const auto &previousBitfield = previousField->bitfield.getValue();
    const uint32_t nextAvailableBit =
        previousBitfield.offsetInBits + previousBitfield.sizeInBits;
    if (nextAvailableBit + currentField->bitfield->sizeInBits > basetypeSize)
      return loweredField;

    loweredField.bitfield->offsetInBits = nextAvailableBit;
    loweredField.offset = previousField->offset;
    loweredField.fieldIndex = previousField->fieldIndex;
    return loweredField;
  };

  // First, check to see if any of the structure members had 'register(c#)'
  // location semantics. If so, members that do not have the 'register(c#)'
  // assignment should be allocated after the *highest explicit address*.
  // Example:
  // float x : register(c10);   // Offset = 160 (10 * 16)
  // float y;                   // Offset = 164 (160 + 4)
  // float z: register(c1);     // Offset = 16  (1  * 16)
  //
  // This step is only here to simplify the struct layout generation.
  std::vector<const HybridStructType::FieldInfo *> sortedFields =
      sortFields(fields);

  // The resulting vector of fields with proper layout information.
  // Second, build each field, and determine their actual offset in the
  // structure (explicit layout, bitfield merging, etc).
  llvm::SmallVector<StructType::FieldInfo, 4> loweredFields;
  llvm::DenseMap<const HybridStructType::FieldInfo *, uint32_t> fieldToIndexMap;

  llvm::SmallVector<StructType::FieldInfo, 4> result;

  // This stores the index of the field in the actual SPIR-V construct.
  // When bitfields are merged, this index will be the same for merged fields.
  uint32_t fieldIndexInConstruct = 0;
  for (size_t i = 0, iPrevious = -1; i < sortedFields.size(); iPrevious = i++) {
    const size_t fieldIndexForMap = loweredFields.size();

    // Can happen if sortFields runs over fields with the same register(c#)
    if (!sortedFields[i]) {
      return result;
    }

    loweredFields.emplace_back(fieldVisitor(
        (iPrevious < loweredFields.size() ? &loweredFields[iPrevious]
                                          : nullptr),
        sortedFields[i], fieldIndexInConstruct));
    if (!(iPrevious < loweredFields.size()) ||
        loweredFields[iPrevious].fieldIndex !=
            loweredFields.back().fieldIndex) {
      fieldIndexInConstruct++;
    }
    fieldToIndexMap[sortedFields[i]] = fieldIndexForMap;
  }

  // Re-order the sorted fields back to their original order.
  for (const auto &field : fields)
    result.push_back(loweredFields[fieldToIndexMap[&field]]);
  return result;
}

const SpirvType *LowerTypeVisitor::getSpirvPointerFromInlineSpirvType(
    ArrayRef<TemplateArgument> args, SpirvLayoutRule rule,
    Optional<bool> isRowMajor, SourceLocation location) {

  assert(args.size() == 2 && "OpTypePointer requires exactly 2 arguments.");
  QualType scLiteralType = args[0].getAsType();
  SpirvConstant *constant = nullptr;
  if (!getVkIntegralConstantValue(scLiteralType, constant, location) ||
      !constant) {
    return nullptr;
  }
  if (!constant->isLiteral())
    return nullptr;

  auto *intConstant = dyn_cast<SpirvConstantInteger>(constant);
  if (!intConstant) {
    return nullptr;
  }

  visitInstruction(constant);
  spv::StorageClass storageClass =
      static_cast<spv::StorageClass>(intConstant->getValue().getLimitedValue());

  QualType pointeeType;
  if (args[1].getKind() == TemplateArgument::ArgKind::Type) {
    pointeeType = args[1].getAsType();
  } else {
    TemplateName templateName = args[1].getAsTemplate();
    pointeeType = createASTTypeFromTemplateName(templateName);
  }

  const SpirvType *pointeeSpirvType =
      lowerType(pointeeType, rule, isRowMajor, location);
  return spvContext.getPointerType(pointeeSpirvType, storageClass);
}

} // namespace spirv
} // namespace clang
