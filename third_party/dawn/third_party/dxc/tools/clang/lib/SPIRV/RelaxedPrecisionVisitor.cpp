//===--- RelaxedPrecisionVisitor.cpp - RelaxedPrecision Visitor --*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "RelaxedPrecisionVisitor.h"
#include "clang/SPIRV/AstTypeProbe.h"
#include "clang/SPIRV/SpirvBuilder.h"

namespace clang {
namespace spirv {

bool RelaxedPrecisionVisitor::visit(SpirvFunction *fn, Phase phase) {
  assert(fn);
  if (phase == Visitor::Phase::Init)
    if (isRelaxedPrecisionType(fn->getAstReturnType(), spvOptions))
      fn->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvVectorShuffle *inst) {
  // The result of vector shuffle must have RelaxedPrecision if the chosen
  // elements come from a vector that is RelaxedPrecision.
  auto *vec1 = inst->getVec1();
  auto *vec2 = inst->getVec2();
  const auto vec1Type = vec1->getAstResultType();
  const auto vec2Type = vec2->getAstResultType();
  const bool isVec1Relaxed = isRelaxedPrecisionType(vec1Type, spvOptions);
  const bool isVec2Relaxed = isRelaxedPrecisionType(vec2Type, spvOptions);
  uint32_t vec1Size;
  uint32_t vec2Size;
  (void)isVectorType(vec1Type, nullptr, &vec1Size);
  (void)isVectorType(vec2Type, nullptr, &vec2Size);
  bool vec1ElemUsed = false;
  bool vec2ElemUsed = false;
  for (auto component : inst->getComponents()) {
    if (component < vec1Size)
      vec1ElemUsed = true;
    else
      vec2ElemUsed = true;
  }
  const bool onlyVec1Used = vec1ElemUsed && !vec2ElemUsed;
  const bool onlyVec2Used = vec2ElemUsed && !vec1ElemUsed;
  if ((onlyVec1Used && isVec1Relaxed) || (onlyVec2Used && isVec2Relaxed) ||
      (vec1ElemUsed && vec2ElemUsed && isVec1Relaxed && isVec2Relaxed))
    inst->setRelaxedPrecision();

  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvUnaryOp *inst) {
  // For conversion operations, check the result QualType. For example: if we
  // are converting from min12int to int, the result should no longet get
  // RelaxedPrecision.
  switch (inst->getopcode()) {
  case spv::Op::OpBitcast:
  case spv::Op::OpFConvert:
  case spv::Op::OpSConvert:
  case spv::Op::OpUConvert: {
    if (isRelaxedPrecisionType(inst->getAstResultType(), spvOptions)) {
      inst->setRelaxedPrecision();
    }
    return true;
  }
  default:
    break;
  }

  // If the argument of the unary operation is RelaxedPrecision and the unary
  // operation is operating on numerical values, the result is also
  // RelaxedPrecision.
  if (inst->getOperand()->isRelaxedPrecision() &&
      isScalarOrNonStructAggregateOfNumericalTypes(
          inst->getOperand()->getAstResultType()))
    inst->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvBinaryOp *inst) {
  // If either argument of the binary operation is RelaxedPrecision, and the
  // binary operation is operating on numerical values, the result is also
  // RelaxedPrecision.
  if (inst->getOperand1()->isRelaxedPrecision() &&
      isScalarOrNonStructAggregateOfNumericalTypes(
          inst->getOperand1()->getAstResultType()) &&
      inst->getOperand2()->isRelaxedPrecision() &&
      isScalarOrNonStructAggregateOfNumericalTypes(
          inst->getOperand2()->getAstResultType()))
    inst->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvSpecConstantUnaryOp *inst) {
  // If the argument of the unary operation is RelaxedPrecision and the unary
  // operation is operating on numerical values, the result is also
  // RelaxedPrecision.
  if (inst->getOperand()->isRelaxedPrecision() &&
      isScalarOrNonStructAggregateOfNumericalTypes(
          inst->getOperand()->getAstResultType()))
    inst->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvSpecConstantBinaryOp *inst) {
  // If either argument of the binary operation is RelaxedPrecision, and the
  // binary operation is operating on numerical values, the result is also
  // RelaxedPrecision.
  if (inst->getOperand1()->isRelaxedPrecision() &&
      isScalarOrNonStructAggregateOfNumericalTypes(
          inst->getOperand1()->getAstResultType()) &&
      inst->getOperand2()->isRelaxedPrecision() &&
      isScalarOrNonStructAggregateOfNumericalTypes(
          inst->getOperand2()->getAstResultType()))
    inst->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvLoad *inst) {
  // If loading from a RelaxedPrecision variable, the result is also decorated
  // with RelaxedPrecision.
  if (isRelaxedPrecisionType(inst->getAstResultType(), spvOptions))
    inst->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvStore *inst) { return true; }

bool RelaxedPrecisionVisitor::visit(SpirvSelect *inst) {
  if (inst->getTrueObject()->isRelaxedPrecision() &&
      inst->getFalseObject()->isRelaxedPrecision())
    inst->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvFunctionCall *inst) {
  // If the return type of the function is RelaxedPrecision, we can decorate the
  // result-id of the OpFunctionCall.
  if (isRelaxedPrecisionType(inst->getAstResultType(), spvOptions))
    inst->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvExtInst *inst) {
  // If all operands to numeric instructions in GLSL extended instruction set is
  // RelaxedPrecision, the result of the opration is also RelaxedPrecision.
  if (inst->getInstructionSet()->getExtendedInstSetName() == "GLSL.std.450") {
    const auto &operands = inst->getOperands();
    if (std::all_of(operands.begin(), operands.end(), [](SpirvInstruction *op) {
          return op->isRelaxedPrecision();
        })) {
      inst->setRelaxedPrecision();
    }
  }
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvCompositeInsert *inst) {
  // If inserting a RelaxedPrecision object into a composite, check the type of
  // the resulting composite. For example: if you are inserting a
  // RelaxedPrecision object as a member into a structure, the resulting
  // structure type is not RelaxedPrecision. But, if you are inserting a
  // RelaxedPrecision object into a vector of RelaxedPrecision integers, the
  // resulting composite *is* RelaxedPrecision.
  // In short, it simply depends on the composite type.
  if (isRelaxedPrecisionType(inst->getAstResultType(), spvOptions))
    inst->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvCompositeExtract *inst) {
  // If extracting a RelaxedPrecision object from a composite, check the type of
  // the extracted object. For example: if extracting different members of a
  // structure, depending on the member, you may or may not want to apply the
  // RelaxedPrecision decoration.
  // In short, it simply depends on the type of what you have extracted.
  if (isRelaxedPrecisionType(inst->getAstResultType(), spvOptions))
    inst->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvCompositeConstruct *inst) {
  // When constructing a composite, only look at the type of the resulting
  // composite.
  if (isRelaxedPrecisionType(inst->getAstResultType(), spvOptions))
    inst->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvConstantBoolean *) {
  // Booleans do not have precision!
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvConstantInteger *inst) {
  if (isRelaxedPrecisionType(inst->getAstResultType(), spvOptions))
    inst->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvConstantFloat *inst) {
  if (isRelaxedPrecisionType(inst->getAstResultType(), spvOptions))
    inst->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvConstantComposite *inst) {
  if (isRelaxedPrecisionType(inst->getAstResultType(), spvOptions))
    inst->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvBitFieldExtract *inst) {
  if (inst->getBase()->isRelaxedPrecision())
    inst->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvBitFieldInsert *inst) {
  if (inst->getBase()->isRelaxedPrecision())
    inst->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvAtomic *inst) {
  // If the original pointer is RelaxedPrecision or operating on a value that is
  // RelaxedPrecision, result is RelaxedPrecision.
  if (inst->getPointer()->isRelaxedPrecision()) {
    if (!inst->hasValue() ||
        (inst->hasValue() && inst->getValue()->isRelaxedPrecision()))
      inst->setRelaxedPrecision();
  }
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvAccessChain *) {
  // The access chain operation itself is irrelevant regarding precision.
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvFunctionParameter *inst) {
  if (isRelaxedPrecisionType(inst->getAstResultType(), spvOptions))
    inst->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvVariable *inst) {
  if (isRelaxedPrecisionType(inst->getAstResultType(), spvOptions))
    inst->setRelaxedPrecision();
  return true;
}

bool RelaxedPrecisionVisitor::visit(SpirvImageOp *inst) {
  // Since OpImageWrite does not have result type, it must not be decorated with
  // the RelaxedPrecision.
  if (inst->getopcode() == spv::Op::OpImageWrite)
    return true;

  // If the operation result type or the underlying image type is relaxed
  // precision, the instruction can be considered relaxed precision.
  if (isRelaxedPrecisionType(inst->getAstResultType(), spvOptions) ||
      isRelaxedPrecisionType(inst->getImage()->getAstResultType(), spvOptions))
    inst->setRelaxedPrecision();
  return true;
}

} // end namespace spirv
} // namespace clang
