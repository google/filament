//===------- ConstEvaluator.cpp ----- Translate Constants -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//===----------------------------------------------------------------------===//
//
//  This file implements methods for translating AST expressions to SPIR-V
//  constants.
//
//===----------------------------------------------------------------------===//

#include "ConstEvaluator.h"

namespace clang {
namespace spirv {

/// Returns true iff the given expression is a literal integer that cannot be
/// represented in a 32-bit integer type or a literal float that cannot be
/// represented in a 32-bit float type without losing info. Returns false
/// otherwise.
bool isLiteralLargerThan32Bits(const Expr *expr) {
  if (const auto *intLiteral = dyn_cast<IntegerLiteral>(expr)) {
    const bool isSigned = expr->getType()->isSignedIntegerType();
    const llvm::APInt &value = intLiteral->getValue();
    return (isSigned && !value.isSignedIntN(32)) ||
           (!isSigned && !value.isIntN(32));
  }

  if (const auto *floatLiteral = dyn_cast<FloatingLiteral>(expr)) {
    llvm::APFloat value = floatLiteral->getValue();
    const auto &semantics = value.getSemantics();
    // regular 'half' and 'float' can be represented in 32 bits.
    if (&semantics == &llvm::APFloat::IEEEsingle ||
        &semantics == &llvm::APFloat::IEEEhalf)
      return true;

    // See if 'double' value can be represented in 32 bits without losing info.
    bool losesInfo = false;
    const auto convertStatus =
        value.convert(llvm::APFloat::IEEEsingle,
                      llvm::APFloat::rmNearestTiesToEven, &losesInfo);
    if (convertStatus != llvm::APFloat::opOK &&
        convertStatus != llvm::APFloat::opInexact)
      return true;
  }

  return false;
}

SpirvConstant *ConstEvaluator::translateAPValue(const APValue &value,
                                                const QualType targetType,
                                                bool isSpecConstantMode) {
  SpirvConstant *result = nullptr;

  if (targetType->isBooleanType()) {
    result = spvBuilder.getConstantBool(value.getInt().getBoolValue(),
                                        isSpecConstantMode);
  } else if (targetType->isIntegralOrEnumerationType()) {
    result = translateAPInt(value.getInt(), targetType, isSpecConstantMode);
  } else if (targetType->isFloatingType()) {
    result = translateAPFloat(value.getFloat(), targetType, isSpecConstantMode);
  } else if (hlsl::IsHLSLVecType(targetType)) {
    const QualType elemType = hlsl::GetHLSLVecElementType(targetType);
    const auto numElements = value.getVectorLength();
    // Special case for vectors of size 1. SPIR-V doesn't support this vector
    // size so we need to translate it to scalar values.
    if (numElements == 1) {
      result =
          translateAPValue(value.getVectorElt(0), elemType, isSpecConstantMode);
    } else {
      llvm::SmallVector<SpirvConstant *, 4> elements;
      for (uint32_t i = 0; i < numElements; ++i) {
        elements.push_back(translateAPValue(value.getVectorElt(i), elemType,
                                            isSpecConstantMode));
      }
      result = spvBuilder.getConstantComposite(targetType, elements);
    }
  }

  if (result)
    return result;

  emitError("APValue of type %0 unimplemented", {}) << value.getKind();
  return 0;
}

SpirvConstant *ConstEvaluator::translateAPInt(const llvm::APInt &intValue,
                                              QualType targetType,
                                              bool isSpecConstantMode) {
  return spvBuilder.getConstantInt(targetType, intValue, isSpecConstantMode);
}

SpirvConstant *ConstEvaluator::translateAPFloat(llvm::APFloat floatValue,
                                                QualType targetType,
                                                bool isSpecConstantMode) {
  return spvBuilder.getConstantFloat(targetType, floatValue,
                                     isSpecConstantMode);
}

SpirvConstant *ConstEvaluator::tryToEvaluateAsInt32(const llvm::APInt &intValue,
                                                    bool isSigned) {
  if (isSigned && intValue.isSignedIntN(32)) {
    return spvBuilder.getConstantInt(astContext.IntTy, intValue);
  }
  if (!isSigned && intValue.isIntN(32)) {
    return spvBuilder.getConstantInt(astContext.UnsignedIntTy, intValue);
  }

  // Couldn't evaluate as a 32-bit int without losing information.
  return nullptr;
}

SpirvConstant *
ConstEvaluator::tryToEvaluateAsFloat32(const llvm::APFloat &floatValue,
                                       bool isSpecConstantMode) {
  const auto &semantics = floatValue.getSemantics();
  // If the given value is already a 32-bit float, there is no need to convert.
  if (&semantics == &llvm::APFloat::IEEEsingle) {
    return spvBuilder.getConstantFloat(astContext.FloatTy, floatValue,
                                       isSpecConstantMode);
  }

  // Try to see if this literal float can be represented in 32-bit.
  // Since the convert function below may modify the fp value, we call it on a
  // temporary copy.
  llvm::APFloat eval = floatValue;
  bool losesInfo = false;
  const auto convertStatus =
      eval.convert(llvm::APFloat::IEEEsingle,
                   llvm::APFloat::rmNearestTiesToEven, &losesInfo);
  if (convertStatus == llvm::APFloat::opOK && !losesInfo)
    return spvBuilder.getConstantFloat(astContext.FloatTy,
                                       llvm::APFloat(eval.convertToFloat()));

  // Couldn't evaluate as a 32-bit float without losing information.
  return nullptr;
}

SpirvConstant *ConstEvaluator::tryToEvaluateAsConst(const Expr *expr,
                                                    bool isSpecConstantMode) {
  Expr::EvalResult evalResult;
  if (expr->EvaluateAsRValue(evalResult, astContext) &&
      !evalResult.HasSideEffects) {
    return translateAPValue(evalResult.Val, expr->getType(),
                            isSpecConstantMode);
  }

  return nullptr;
}

} // namespace spirv
} // namespace clang
