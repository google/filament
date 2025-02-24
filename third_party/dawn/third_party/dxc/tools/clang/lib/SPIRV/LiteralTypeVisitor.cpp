//===--- LiteralTypeVisitor.cpp - Literal Type Visitor -----------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "LiteralTypeVisitor.h"
#include "LowerTypeVisitor.h"
#include "clang/SPIRV/AstTypeProbe.h"
#include "clang/SPIRV/SpirvFunction.h"

namespace clang {
namespace spirv {

bool LiteralTypeVisitor::visit(SpirvFunction *fn, Phase phase) {
  assert(fn);

  // Before going through the function instructions
  if (phase == Visitor::Phase::Init) {
    curFnAstReturnType = fn->getAstReturnType();
  }

  return true;
}

bool LiteralTypeVisitor::isLiteralLargerThan32Bits(
    SpirvConstantInteger *constant) {
  assert(constant->hasAstResultType());
  QualType type = constant->getAstResultType();
  const bool isSigned = type->isSignedIntegerType();
  const llvm::APInt &value = constant->getValue();
  return (isSigned && !value.isSignedIntN(32)) ||
         (!isSigned && !value.isIntN(32));
}

bool LiteralTypeVisitor::canDeduceTypeFromLitType(QualType litType,
                                                  QualType newType) {
  if (litType == QualType() || newType == QualType() || litType == newType)
    return false;

  // The 'inout' and 'out' function arguments are of a reference type.
  // For example: 'uint &'.
  // We should first remove such reference from QualType (if any).
  if (const auto *refType = litType->getAs<ReferenceType>())
    litType = refType->getPointeeType();
  if (const auto *refType = newType->getAs<ReferenceType>())
    newType = refType->getPointeeType();

  if (!isLitTypeOrVecOfLitType(litType))
    return false;
  if (isLitTypeOrVecOfLitType(newType))
    return false;

  if (litType->isFloatingType() && newType->isFloatingType())
    return true;
  if ((litType->isIntegerType() && !litType->isBooleanType()) &&
      (newType->isIntegerType() && !newType->isBooleanType()))
    return true;

  {
    QualType elemType1 = {};
    uint32_t elemCount1 = 0;
    QualType elemType2 = {};
    uint32_t elemCount2 = 0;
    if (isVectorType(litType, &elemType1, &elemCount1) &&
        isVectorType(newType, &elemType2, &elemCount2))
      return elemCount1 == elemCount2 &&
             canDeduceTypeFromLitType(elemType1, elemType2);
  }

  return false;
}

void LiteralTypeVisitor::tryToUpdateInstLitType(SpirvInstruction *inst,
                                                QualType newType) {
  if (!inst)
    return;

  // We may only update LitInt to Int type and LitFloat to Float type.
  if (!canDeduceTypeFromLitType(inst->getAstResultType(), newType))
    return;

  // Since LiteralTypeVisitor is run before lowering the types, we can simply
  // update the AST result-type of the instruction to the new type. In the case
  // of the instruction being a constant instruction, since we do not have
  // unique constants at this point, changing the QualType of the constant
  // instruction is safe.
  inst->setAstResultType(newType);
}

bool LiteralTypeVisitor::visitInstruction(SpirvInstruction *instr) {
  // Instructions that don't have custom visitors cannot help with deducing the
  // real type from the literal type.
  return true;
}

bool LiteralTypeVisitor::visit(SpirvVariable *var) {
  tryToUpdateInstLitType(var->getInitializer(), var->getAstResultType());
  return true;
}

bool LiteralTypeVisitor::visit(SpirvAtomic *inst) {
  const auto resultType = inst->getAstResultType();
  tryToUpdateInstLitType(inst->getValue(), resultType);
  tryToUpdateInstLitType(inst->getComparator(), resultType);
  return true;
}

bool LiteralTypeVisitor::visit(SpirvUnaryOp *inst) {
  const auto opcode = inst->getopcode();
  const auto resultType = inst->getAstResultType();
  auto *arg = inst->getOperand();
  const auto argType = arg->getAstResultType();

  if (!isLitTypeOrVecOfLitType(argType)) {
    return true;
  }
  if (isLitTypeOrVecOfLitType(resultType)) {
    return true;
  }

  switch (opcode) {
  case spv::Op::OpUConvert:
  case spv::Op::OpSConvert:
  case spv::Op::OpFConvert:
    // The result type gives us no information about the operand type. Do not do
    // anything.
    return true;
  case spv::Op::OpConvertFToU:
  case spv::Op::OpConvertFToS:
  case spv::Op::OpConvertSToF:
  case spv::Op::OpConvertUToF:
  case spv::Op::OpNot:
  case spv::Op::OpBitcast:
  case spv::Op::OpSNegate: {
    // The cases can change the type, but not the bitwidth. We can use the
    // result type's bitwidth and the operand's type.
    const uint32_t resultTypeBitwidth = getElementSpirvBitwidth(
        astContext, resultType, spvOptions.enable16BitTypes);
    const QualType newType =
        getTypeWithCustomBitwidth(astContext, argType, resultTypeBitwidth);
    tryToUpdateInstLitType(arg, newType);
    return true;
  }
  default:
    // In all other cases, try to set the operand type to the result type.
    tryToUpdateInstLitType(arg, resultType);
    return true;
  }
}

bool LiteralTypeVisitor::visit(SpirvBinaryOp *inst) {
  const auto resultType = inst->getAstResultType();
  const auto op = inst->getopcode();
  auto *operand1 = inst->getOperand1();
  auto *operand2 = inst->getOperand2();

  switch (op) {
  case spv::Op::OpShiftRightLogical:
  case spv::Op::OpShiftRightArithmetic:
  case spv::Op::OpShiftLeftLogical: {
    // Base (arg1) should have the same type as result type
    tryToUpdateInstLitType(inst->getOperand1(), resultType);
    // The shift amount (arg2) cannot be a 64-bit type for a 32-bit base!
    tryToUpdateInstLitType(inst->getOperand2(), resultType);
    return true;
  }
  // The following operations have a boolean return type, so we cannot deduce
  // anything about the operand type from the result type. However, the two
  // operands in these operations must have the same bitwidth.
  case spv::Op::OpIEqual:
  case spv::Op::OpINotEqual:
  case spv::Op::OpUGreaterThan:
  case spv::Op::OpSGreaterThan:
  case spv::Op::OpUGreaterThanEqual:
  case spv::Op::OpSGreaterThanEqual:
  case spv::Op::OpULessThan:
  case spv::Op::OpSLessThan:
  case spv::Op::OpULessThanEqual:
  case spv::Op::OpSLessThanEqual:
  case spv::Op::OpFOrdEqual:
  case spv::Op::OpFUnordEqual:
  case spv::Op::OpFOrdNotEqual:
  case spv::Op::OpFUnordNotEqual:
  case spv::Op::OpFOrdLessThan:
  case spv::Op::OpFUnordLessThan:
  case spv::Op::OpFOrdGreaterThan:
  case spv::Op::OpFUnordGreaterThan:
  case spv::Op::OpFOrdLessThanEqual:
  case spv::Op::OpFUnordLessThanEqual:
  case spv::Op::OpFOrdGreaterThanEqual:
  case spv::Op::OpFUnordGreaterThanEqual: {
    if (operand1->hasAstResultType() && operand2->hasAstResultType()) {
      const auto operand1Type = operand1->getAstResultType();
      const auto operand2Type = operand2->getAstResultType();
      bool isLitOp1 = isLitTypeOrVecOfLitType(operand1Type);
      bool isLitOp2 = isLitTypeOrVecOfLitType(operand2Type);

      if (isLitOp1 && !isLitOp2) {
        const uint32_t operand2Bitwidth = getElementSpirvBitwidth(
            astContext, operand2Type, spvOptions.enable16BitTypes);
        const QualType newType = getTypeWithCustomBitwidth(
            astContext, operand1Type, operand2Bitwidth);
        tryToUpdateInstLitType(operand1, newType);
        return true;
      }
      if (isLitOp2 && !isLitOp1) {
        const uint32_t operand1Bitwidth = getElementSpirvBitwidth(
            astContext, operand1Type, spvOptions.enable16BitTypes);
        const QualType newType = getTypeWithCustomBitwidth(
            astContext, operand2Type, operand1Bitwidth);
        tryToUpdateInstLitType(operand2, newType);
        return true;
      }
    }
    break;
  }
  // The result type of dot product is scalar but operands should be vector of
  // the same type.
  case spv::Op::OpDot: {
    tryToUpdateInstLitType(inst->getOperand1(),
                           inst->getOperand2()->getAstResultType());
    tryToUpdateInstLitType(inst->getOperand2(),
                           inst->getOperand1()->getAstResultType());
    return true;
  }

  case spv::Op::OpVectorTimesScalar: {
    QualType elemType;
    if (isVectorType(operand1->getAstResultType(), &elemType) &&
        elemType->isFloatingType()) {
      tryToUpdateInstLitType(inst->getOperand2(), elemType);
    }
    return true;
  }

  default:
    break;
  }

  // General attempt to deduce operand types from the result type.
  tryToUpdateInstLitType(operand1, resultType);
  tryToUpdateInstLitType(operand2, resultType);
  return true;
}

bool LiteralTypeVisitor::visit(SpirvBitFieldInsert *inst) {
  const auto resultType = inst->getAstResultType();
  tryToUpdateInstLitType(inst->getBase(), resultType);
  tryToUpdateInstLitType(inst->getInsert(), resultType);
  return true;
}

bool LiteralTypeVisitor::visit(SpirvBitFieldExtract *inst) {
  const auto resultType = inst->getAstResultType();
  tryToUpdateInstLitType(inst->getBase(), resultType);
  return true;
}

bool LiteralTypeVisitor::visit(SpirvSelect *inst) {
  const auto resultType = inst->getAstResultType();
  tryToUpdateInstLitType(inst->getTrueObject(), resultType);
  tryToUpdateInstLitType(inst->getFalseObject(), resultType);
  return true;
}

bool LiteralTypeVisitor::visit(SpirvVectorShuffle *inst) {
  const auto resultType = inst->getAstResultType();
  if (inst->hasAstResultType() && !isLitTypeOrVecOfLitType(resultType)) {
    auto *vec1 = inst->getVec1();
    auto *vec2 = inst->getVec1();
    assert(vec1 && vec2);
    QualType resultElemType = {};
    uint32_t resultElemCount = 0;
    QualType vec1ElemType = {};
    uint32_t vec1ElemCount = 0;
    QualType vec2ElemType = {};
    uint32_t vec2ElemCount = 0;
    (void)isVectorType(resultType, &resultElemType, &resultElemCount);
    (void)isVectorType(vec1->getAstResultType(), &vec1ElemType, &vec1ElemCount);
    (void)isVectorType(vec2->getAstResultType(), &vec2ElemType, &vec2ElemCount);
    if (isLitTypeOrVecOfLitType(vec1ElemType)) {
      tryToUpdateInstLitType(
          vec1, astContext.getExtVectorType(resultElemType, vec1ElemCount));
    }
    if (isLitTypeOrVecOfLitType(vec2ElemType)) {
      tryToUpdateInstLitType(
          vec2, astContext.getExtVectorType(resultElemType, vec2ElemCount));
    }
  }
  return true;
}

bool LiteralTypeVisitor::visit(SpirvGroupNonUniformOp *inst) {
  for (auto *operand : inst->getOperands())
    tryToUpdateInstLitType(operand, inst->getAstResultType());
  return true;
}

bool LiteralTypeVisitor::visit(SpirvLoad *inst) {
  auto *pointer = inst->getPointer();
  if (!pointer->hasAstResultType())
    return true;

  QualType pointerType = pointer->getAstResultType();
  if (!isLitTypeOrVecOfLitType(pointerType))
    return true;

  assert(inst->hasAstResultType());
  QualType resultType = inst->getAstResultType();

  if (!canDeduceTypeFromLitType(pointerType, resultType))
    return true;

  QualType newPointerType = astContext.getPointerType(resultType);
  pointer->setAstResultType(newPointerType);
  return true;
}

bool LiteralTypeVisitor::visit(SpirvStore *inst) {
  auto *object = inst->getObject();
  auto *pointer = inst->getPointer();
  if (pointer->hasAstResultType()) {
    QualType type = pointer->getAstResultType();
    if (const auto *ptrType = type->getAs<PointerType>())
      type = ptrType->getPointeeType();
    tryToUpdateInstLitType(object, type);
  } else if (pointer->hasResultType()) {
    if (auto *ptrType = dyn_cast<HybridPointerType>(pointer->getResultType())) {
      QualType type = ptrType->getPointeeType();
      tryToUpdateInstLitType(object, type);
    }
  }
  return true;
}

bool LiteralTypeVisitor::visit(SpirvConstantComposite *inst) {
  const auto resultType = inst->getAstResultType();
  llvm::SmallVector<SpirvInstruction *, 4> constituents(
      inst->getConstituents().begin(), inst->getConstituents().end());
  updateTypeForCompositeMembers(resultType, constituents);
  return true;
}

bool LiteralTypeVisitor::visit(SpirvCompositeConstruct *inst) {
  const auto resultType = inst->getAstResultType();
  updateTypeForCompositeMembers(resultType, inst->getConstituents());
  return true;
}

bool LiteralTypeVisitor::visit(SpirvCompositeExtract *inst) {
  const auto resultType = inst->getAstResultType();
  auto *base = inst->getComposite();
  const auto baseType = base->getAstResultType();
  if (isLitTypeOrVecOfLitType(baseType) &&
      !isLitTypeOrVecOfLitType(resultType)) {
    const uint32_t resultTypeBitwidth = getElementSpirvBitwidth(
        astContext, resultType, spvOptions.enable16BitTypes);
    const QualType newType =
        getTypeWithCustomBitwidth(astContext, baseType, resultTypeBitwidth);
    tryToUpdateInstLitType(base, newType);
  }

  return true;
}

bool LiteralTypeVisitor::updateTypeForCompositeMembers(
    QualType compositeType, llvm::ArrayRef<SpirvInstruction *> constituents) {

  if (compositeType == QualType())
    return true;

  // The constituents are the top level objects that create the result type.
  // The result type may be one of the following:
  // Vector, Array, Matrix, Struct

  // TODO: This method is currently not recursive. We can use recursion if
  // absolutely necessary.

  { // Vector case
    QualType elemType = {};
    if (isVectorType(compositeType, &elemType)) {
      for (auto *constituent : constituents)
        tryToUpdateInstLitType(constituent, elemType);
      return true;
    }
  }

  { // Array case
    if (const auto *arrType = dyn_cast<ConstantArrayType>(compositeType)) {
      for (auto *constituent : constituents)
        tryToUpdateInstLitType(constituent, arrType->getElementType());
      return true;
    }
  }

  { // Matrix case
    QualType elemType = {};
    if (isMxNMatrix(compositeType, &elemType)) {
      for (auto *constituent : constituents) {
        // Each constituent is a matrix column (a vector)
        uint32_t colSize = 0;
        if (isVectorType(constituent->getAstResultType(), nullptr, &colSize)) {
          QualType newType = astContext.getExtVectorType(elemType, colSize);
          tryToUpdateInstLitType(constituent, newType);
        }
      }
      return true;
    }
  }

  { // Struct case
    if (const auto *structType = compositeType->getAs<RecordType>()) {
      const auto *decl = structType->getDecl();
      size_t i = 0;
      for (const auto *field : decl->fields()) {
        // If the field is a bitfield, it might be squashed later when building
        // the SPIR-V type depending on context. This means indices starting
        // from this bitfield are not guaranteed, and we shouldn't touch them.
        if (field->isBitField())
          break;
        tryToUpdateInstLitType(constituents[i], field->getType());
        ++i;
      }
      return true;
    }
  }

  return true;
}

bool LiteralTypeVisitor::visit(SpirvAccessChain *inst) {
  for (auto *index : inst->getIndexes()) {
    if (auto *constInt = dyn_cast<SpirvConstantInteger>(index)) {
      if (!isLiteralLargerThan32Bits(constInt)) {
        tryToUpdateInstLitType(
            constInt, constInt->getAstResultType()->isSignedIntegerType()
                          ? astContext.IntTy
                          : astContext.UnsignedIntTy);
      }
    } else {
      tryToUpdateInstLitType(index,
                             index->getAstResultType()->isSignedIntegerType()
                                 ? astContext.IntTy
                                 : astContext.UnsignedIntTy);
    }
  }
  return true;
}

bool LiteralTypeVisitor::visit(SpirvExtInst *inst) {
  // Result type of the instruction can provide a hint about its operands. e.g.
  // OpExtInst %float %glsl_set Pow %double_2 %double_12
  // should be evaluated as:
  // OpExtInst %float %glsl_set Pow %float_2 %float_12
  const auto resultType = inst->getAstResultType();
  for (auto *operand : inst->getOperands())
    tryToUpdateInstLitType(operand, resultType);
  return true;
}

bool LiteralTypeVisitor::visit(SpirvReturn *inst) {
  if (inst->hasReturnValue()) {
    tryToUpdateInstLitType(inst->getReturnValue(), curFnAstReturnType);
  }
  return true;
}

bool LiteralTypeVisitor::visit(SpirvCompositeInsert *inst) {
  const auto resultType = inst->getAstResultType();
  tryToUpdateInstLitType(inst->getComposite(), resultType);
  tryToUpdateInstLitType(inst->getObject(),
                         getElementType(astContext, resultType));
  return true;
}

bool LiteralTypeVisitor::visit(SpirvImageOp *inst) {
  if (inst->isImageWrite() && inst->hasAstResultType()) {
    const auto sampledType =
        hlsl::GetHLSLResourceResultType(inst->getAstResultType());
    tryToUpdateInstLitType(inst->getTexelToWrite(), sampledType);
  }
  return true;
}

bool LiteralTypeVisitor::visit(SpirvSwitch *inst) {
  if (auto *constInt = dyn_cast<SpirvConstantInteger>(inst->getSelector())) {
    if (isLiteralLargerThan32Bits(constInt)) {
      const bool isSigned = constInt->getAstResultType()->isSignedIntegerType();
      constInt->setAstResultType(isSigned ? astContext.LongLongTy
                                          : astContext.UnsignedLongLongTy);
    }
  }
  return true;
}

} // end namespace spirv
} // end namespace clang
