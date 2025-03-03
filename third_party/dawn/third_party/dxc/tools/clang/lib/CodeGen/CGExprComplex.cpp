//===--- CGExprComplex.cpp - Emit LLVM Code for Complex Exprs -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This contains code to emit Expr nodes with complex types as LLVM code.
//
//===----------------------------------------------------------------------===//

#include "CodeGenFunction.h"
#include "CodeGenModule.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/StmtVisitor.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Metadata.h"
#include <algorithm>
using namespace clang;
using namespace CodeGen;

//===----------------------------------------------------------------------===//
//                        Complex Expression Emitter
//===----------------------------------------------------------------------===//

typedef CodeGenFunction::ComplexPairTy ComplexPairTy;

/// Return the complex type that we are meant to emit.
static const ComplexType *getComplexType(QualType type) {
  type = type.getCanonicalType();
  if (const ComplexType *comp = dyn_cast<ComplexType>(type)) {
    return comp;
  } else {
    return cast<ComplexType>(cast<AtomicType>(type)->getValueType());
  }
}

namespace  {
class ComplexExprEmitter
  : public StmtVisitor<ComplexExprEmitter, ComplexPairTy> {
  CodeGenFunction &CGF;
  CGBuilderTy &Builder;
  bool IgnoreReal;
  bool IgnoreImag;
public:
  ComplexExprEmitter(CodeGenFunction &cgf, bool ir=false, bool ii=false)
    : CGF(cgf), Builder(CGF.Builder), IgnoreReal(ir), IgnoreImag(ii) {
  }


  //===--------------------------------------------------------------------===//
  //                               Utilities
  //===--------------------------------------------------------------------===//

  bool TestAndClearIgnoreReal() {
    bool I = IgnoreReal;
    IgnoreReal = false;
    return I;
  }
  bool TestAndClearIgnoreImag() {
    bool I = IgnoreImag;
    IgnoreImag = false;
    return I;
  }

  /// EmitLoadOfLValue - Given an expression with complex type that represents a
  /// value l-value, this method emits the address of the l-value, then loads
  /// and returns the result.
  ComplexPairTy EmitLoadOfLValue(const Expr *E) {
    return EmitLoadOfLValue(CGF.EmitLValue(E), E->getExprLoc());
  }

  ComplexPairTy EmitLoadOfLValue(LValue LV, SourceLocation Loc);

  /// EmitStoreOfComplex - Store the specified real/imag parts into the
  /// specified value pointer.
  void EmitStoreOfComplex(ComplexPairTy Val, LValue LV, bool isInit);

  /// EmitComplexToComplexCast - Emit a cast from complex value Val to DestType.
  ComplexPairTy EmitComplexToComplexCast(ComplexPairTy Val, QualType SrcType,
                                         QualType DestType);
  /// EmitComplexToComplexCast - Emit a cast from scalar value Val to DestType.
  ComplexPairTy EmitScalarToComplexCast(llvm::Value *Val, QualType SrcType,
                                        QualType DestType);

  //===--------------------------------------------------------------------===//
  //                            Visitor Methods
  //===--------------------------------------------------------------------===//

  ComplexPairTy Visit(Expr *E) {
    ApplyDebugLocation DL(CGF, E);
    return StmtVisitor<ComplexExprEmitter, ComplexPairTy>::Visit(E);
  }

  ComplexPairTy VisitStmt(Stmt *S) {
    S->dump(CGF.getContext().getSourceManager());
    llvm_unreachable("Stmt can't have complex result type!");
  }
  ComplexPairTy VisitExpr(Expr *S);
  ComplexPairTy VisitParenExpr(ParenExpr *PE) { return Visit(PE->getSubExpr());}
  ComplexPairTy VisitGenericSelectionExpr(GenericSelectionExpr *GE) {
    return Visit(GE->getResultExpr());
  }
  ComplexPairTy VisitImaginaryLiteral(const ImaginaryLiteral *IL);
  ComplexPairTy
  VisitSubstNonTypeTemplateParmExpr(SubstNonTypeTemplateParmExpr *PE) {
    return Visit(PE->getReplacement());
  }

  // l-values.
  ComplexPairTy VisitDeclRefExpr(DeclRefExpr *E) {
    if (CodeGenFunction::ConstantEmission result = CGF.tryEmitAsConstant(E)) {
      if (result.isReference())
        return EmitLoadOfLValue(result.getReferenceLValue(CGF, E),
                                E->getExprLoc());

      llvm::Constant *pair = result.getValue();
      return ComplexPairTy(pair->getAggregateElement(0U),
                           pair->getAggregateElement(1U));
    }
    return EmitLoadOfLValue(E);
  }
  ComplexPairTy VisitObjCIvarRefExpr(ObjCIvarRefExpr *E) {
    return EmitLoadOfLValue(E);
  }
  ComplexPairTy VisitObjCMessageExpr(ObjCMessageExpr *E) {
    return CGF.EmitObjCMessageExpr(E).getComplexVal();
  }
  ComplexPairTy VisitArraySubscriptExpr(Expr *E) { return EmitLoadOfLValue(E); }
  ComplexPairTy VisitMemberExpr(const Expr *E) { return EmitLoadOfLValue(E); }
  ComplexPairTy VisitOpaqueValueExpr(OpaqueValueExpr *E) {
    if (E->isGLValue())
      return EmitLoadOfLValue(CGF.getOpaqueLValueMapping(E), E->getExprLoc());
    return CGF.getOpaqueRValueMapping(E).getComplexVal();
  }

  ComplexPairTy VisitPseudoObjectExpr(PseudoObjectExpr *E) {
    return CGF.EmitPseudoObjectRValue(E).getComplexVal();
  }

  // FIXME: CompoundLiteralExpr

  ComplexPairTy EmitCast(CastKind CK, Expr *Op, QualType DestTy);
  ComplexPairTy VisitImplicitCastExpr(ImplicitCastExpr *E) {
    // Unlike for scalars, we don't have to worry about function->ptr demotion
    // here.
    return EmitCast(E->getCastKind(), E->getSubExpr(), E->getType());
  }
  ComplexPairTy VisitCastExpr(CastExpr *E) {
    return EmitCast(E->getCastKind(), E->getSubExpr(), E->getType());
  }
  ComplexPairTy VisitCallExpr(const CallExpr *E);
  ComplexPairTy VisitStmtExpr(const StmtExpr *E);

  // Operators.
  ComplexPairTy VisitPrePostIncDec(const UnaryOperator *E,
                                   bool isInc, bool isPre) {
    LValue LV = CGF.EmitLValue(E->getSubExpr());
    return CGF.EmitComplexPrePostIncDec(E, LV, isInc, isPre);
  }
  ComplexPairTy VisitUnaryPostDec(const UnaryOperator *E) {
    return VisitPrePostIncDec(E, false, false);
  }
  ComplexPairTy VisitUnaryPostInc(const UnaryOperator *E) {
    return VisitPrePostIncDec(E, true, false);
  }
  ComplexPairTy VisitUnaryPreDec(const UnaryOperator *E) {
    return VisitPrePostIncDec(E, false, true);
  }
  ComplexPairTy VisitUnaryPreInc(const UnaryOperator *E) {
    return VisitPrePostIncDec(E, true, true);
  }
  ComplexPairTy VisitUnaryDeref(const Expr *E) { return EmitLoadOfLValue(E); }
  ComplexPairTy VisitUnaryPlus     (const UnaryOperator *E) {
    TestAndClearIgnoreReal();
    TestAndClearIgnoreImag();
    return Visit(E->getSubExpr());
  }
  ComplexPairTy VisitUnaryMinus    (const UnaryOperator *E);
  ComplexPairTy VisitUnaryNot      (const UnaryOperator *E);
  // LNot,Real,Imag never return complex.
  ComplexPairTy VisitUnaryExtension(const UnaryOperator *E) {
    return Visit(E->getSubExpr());
  }
  ComplexPairTy VisitCXXDefaultArgExpr(CXXDefaultArgExpr *DAE) {
    return Visit(DAE->getExpr());
  }
  ComplexPairTy VisitCXXDefaultInitExpr(CXXDefaultInitExpr *DIE) {
    CodeGenFunction::CXXDefaultInitExprScope Scope(CGF);
    return Visit(DIE->getExpr());
  }
  ComplexPairTy VisitExprWithCleanups(ExprWithCleanups *E) {
    CGF.enterFullExpression(E);
    CodeGenFunction::RunCleanupsScope Scope(CGF);
    return Visit(E->getSubExpr());
  }
  ComplexPairTy VisitCXXScalarValueInitExpr(CXXScalarValueInitExpr *E) {
    assert(E->getType()->isAnyComplexType() && "Expected complex type!");
    QualType Elem = E->getType()->castAs<ComplexType>()->getElementType();
    llvm::Constant *Null = llvm::Constant::getNullValue(CGF.ConvertType(Elem));
    return ComplexPairTy(Null, Null);
  }
  ComplexPairTy VisitImplicitValueInitExpr(ImplicitValueInitExpr *E) {
    assert(E->getType()->isAnyComplexType() && "Expected complex type!");
    QualType Elem = E->getType()->castAs<ComplexType>()->getElementType();
    llvm::Constant *Null =
                       llvm::Constant::getNullValue(CGF.ConvertType(Elem));
    return ComplexPairTy(Null, Null);
  }

  struct BinOpInfo {
    ComplexPairTy LHS;
    ComplexPairTy RHS;
    QualType Ty;  // Computation Type.
  };

  BinOpInfo EmitBinOps(const BinaryOperator *E);
  LValue EmitCompoundAssignLValue(const CompoundAssignOperator *E,
                                  ComplexPairTy (ComplexExprEmitter::*Func)
                                  (const BinOpInfo &),
                                  RValue &Val);
  ComplexPairTy EmitCompoundAssign(const CompoundAssignOperator *E,
                                   ComplexPairTy (ComplexExprEmitter::*Func)
                                   (const BinOpInfo &));

  ComplexPairTy EmitBinAdd(const BinOpInfo &Op);
  ComplexPairTy EmitBinSub(const BinOpInfo &Op);
  ComplexPairTy EmitBinMul(const BinOpInfo &Op);
  ComplexPairTy EmitBinDiv(const BinOpInfo &Op);

  ComplexPairTy EmitComplexBinOpLibCall(StringRef LibCallName,
                                        const BinOpInfo &Op);

  ComplexPairTy VisitBinAdd(const BinaryOperator *E) {
    return EmitBinAdd(EmitBinOps(E));
  }
  ComplexPairTy VisitBinSub(const BinaryOperator *E) {
    return EmitBinSub(EmitBinOps(E));
  }
  ComplexPairTy VisitBinMul(const BinaryOperator *E) {
    return EmitBinMul(EmitBinOps(E));
  }
  ComplexPairTy VisitBinDiv(const BinaryOperator *E) {
    return EmitBinDiv(EmitBinOps(E));
  }

  // Compound assignments.
  ComplexPairTy VisitBinAddAssign(const CompoundAssignOperator *E) {
    return EmitCompoundAssign(E, &ComplexExprEmitter::EmitBinAdd);
  }
  ComplexPairTy VisitBinSubAssign(const CompoundAssignOperator *E) {
    return EmitCompoundAssign(E, &ComplexExprEmitter::EmitBinSub);
  }
  ComplexPairTy VisitBinMulAssign(const CompoundAssignOperator *E) {
    return EmitCompoundAssign(E, &ComplexExprEmitter::EmitBinMul);
  }
  ComplexPairTy VisitBinDivAssign(const CompoundAssignOperator *E) {
    return EmitCompoundAssign(E, &ComplexExprEmitter::EmitBinDiv);
  }

  // GCC rejects rem/and/or/xor for integer complex.
  // Logical and/or always return int, never complex.

  // No comparisons produce a complex result.

  LValue EmitBinAssignLValue(const BinaryOperator *E,
                             ComplexPairTy &Val);
  ComplexPairTy VisitBinAssign     (const BinaryOperator *E);
  ComplexPairTy VisitBinComma      (const BinaryOperator *E);


  ComplexPairTy
  VisitAbstractConditionalOperator(const AbstractConditionalOperator *CO);
  ComplexPairTy VisitChooseExpr(ChooseExpr *CE);

  ComplexPairTy VisitInitListExpr(InitListExpr *E);

  ComplexPairTy VisitCompoundLiteralExpr(CompoundLiteralExpr *E) {
    return EmitLoadOfLValue(E);
  }

  ComplexPairTy VisitVAArgExpr(VAArgExpr *E);

  ComplexPairTy VisitAtomicExpr(AtomicExpr *E) {
    return CGF.EmitAtomicExpr(E).getComplexVal();
  }
};
}  // end anonymous namespace.

//===----------------------------------------------------------------------===//
//                                Utilities
//===----------------------------------------------------------------------===//

/// EmitLoadOfLValue - Given an RValue reference for a complex, emit code to
/// load the real and imaginary pieces, returning them as Real/Imag.
ComplexPairTy ComplexExprEmitter::EmitLoadOfLValue(LValue lvalue,
                                                   SourceLocation loc) {
  assert(lvalue.isSimple() && "non-simple complex l-value?");
  if (lvalue.getType()->isAtomicType())
    return CGF.EmitAtomicLoad(lvalue, loc).getComplexVal();

  llvm::Value *SrcPtr = lvalue.getAddress();
  bool isVolatile = lvalue.isVolatileQualified();
  unsigned AlignR = lvalue.getAlignment().getQuantity();
  ASTContext &C = CGF.getContext();
  QualType ComplexTy = lvalue.getType();
  unsigned ComplexAlign = C.getTypeAlignInChars(ComplexTy).getQuantity();
  unsigned AlignI = std::min(AlignR, ComplexAlign);

  llvm::Value *Real=nullptr, *Imag=nullptr;

  if (!IgnoreReal || isVolatile) {
    llvm::Value *RealP = Builder.CreateStructGEP(nullptr, SrcPtr, 0,
                                                 SrcPtr->getName() + ".realp");
    Real = Builder.CreateAlignedLoad(RealP, AlignR, isVolatile,
                                     SrcPtr->getName() + ".real");
  }

  if (!IgnoreImag || isVolatile) {
    llvm::Value *ImagP = Builder.CreateStructGEP(nullptr, SrcPtr, 1,
                                                 SrcPtr->getName() + ".imagp");
    Imag = Builder.CreateAlignedLoad(ImagP, AlignI, isVolatile,
                                     SrcPtr->getName() + ".imag");
  }
  return ComplexPairTy(Real, Imag);
}

/// EmitStoreOfComplex - Store the specified real/imag parts into the
/// specified value pointer.
void ComplexExprEmitter::EmitStoreOfComplex(ComplexPairTy Val, LValue lvalue,
                                            bool isInit) {
  if (lvalue.getType()->isAtomicType() ||
      (!isInit && CGF.LValueIsSuitableForInlineAtomic(lvalue)))
    return CGF.EmitAtomicStore(RValue::getComplex(Val), lvalue, isInit);

  llvm::Value *Ptr = lvalue.getAddress();
  llvm::Value *RealPtr = Builder.CreateStructGEP(nullptr, Ptr, 0, "real");
  llvm::Value *ImagPtr = Builder.CreateStructGEP(nullptr, Ptr, 1, "imag");
  unsigned AlignR = lvalue.getAlignment().getQuantity();
  ASTContext &C = CGF.getContext();
  QualType ComplexTy = lvalue.getType();
  unsigned ComplexAlign = C.getTypeAlignInChars(ComplexTy).getQuantity();
  unsigned AlignI = std::min(AlignR, ComplexAlign);

  Builder.CreateAlignedStore(Val.first, RealPtr, AlignR,
                             lvalue.isVolatileQualified());
  Builder.CreateAlignedStore(Val.second, ImagPtr, AlignI,
                             lvalue.isVolatileQualified());
}



//===----------------------------------------------------------------------===//
//                            Visitor Methods
//===----------------------------------------------------------------------===//

ComplexPairTy ComplexExprEmitter::VisitExpr(Expr *E) {
  CGF.ErrorUnsupported(E, "complex expression");
  llvm::Type *EltTy =
    CGF.ConvertType(getComplexType(E->getType())->getElementType());
  llvm::Value *U = llvm::UndefValue::get(EltTy);
  return ComplexPairTy(U, U);
}

ComplexPairTy ComplexExprEmitter::
VisitImaginaryLiteral(const ImaginaryLiteral *IL) {
  llvm::Value *Imag = CGF.EmitScalarExpr(IL->getSubExpr());
  return ComplexPairTy(llvm::Constant::getNullValue(Imag->getType()), Imag);
}


ComplexPairTy ComplexExprEmitter::VisitCallExpr(const CallExpr *E) {
  if (E->getCallReturnType(CGF.getContext())->isReferenceType())
    return EmitLoadOfLValue(E);

  return CGF.EmitCallExpr(E).getComplexVal();
}

ComplexPairTy ComplexExprEmitter::VisitStmtExpr(const StmtExpr *E) {
  CodeGenFunction::StmtExprEvaluation eval(CGF);
  llvm::Value *RetAlloca = CGF.EmitCompoundStmt(*E->getSubStmt(), true);
  assert(RetAlloca && "Expected complex return value");
  return EmitLoadOfLValue(CGF.MakeAddrLValue(RetAlloca, E->getType()),
                          E->getExprLoc());
}

/// EmitComplexToComplexCast - Emit a cast from complex value Val to DestType.
ComplexPairTy ComplexExprEmitter::EmitComplexToComplexCast(ComplexPairTy Val,
                                                           QualType SrcType,
                                                           QualType DestType) {
  // Get the src/dest element type.
  SrcType = SrcType->castAs<ComplexType>()->getElementType();
  DestType = DestType->castAs<ComplexType>()->getElementType();

  // C99 6.3.1.6: When a value of complex type is converted to another
  // complex type, both the real and imaginary parts follow the conversion
  // rules for the corresponding real types.
  Val.first = CGF.EmitScalarConversion(Val.first, SrcType, DestType);
  Val.second = CGF.EmitScalarConversion(Val.second, SrcType, DestType);
  return Val;
}

ComplexPairTy ComplexExprEmitter::EmitScalarToComplexCast(llvm::Value *Val,
                                                          QualType SrcType,
                                                          QualType DestType) {
  // Convert the input element to the element type of the complex.
  DestType = DestType->castAs<ComplexType>()->getElementType();
  Val = CGF.EmitScalarConversion(Val, SrcType, DestType);

  // Return (realval, 0).
  return ComplexPairTy(Val, llvm::Constant::getNullValue(Val->getType()));
}

ComplexPairTy ComplexExprEmitter::EmitCast(CastKind CK, Expr *Op,
                                           QualType DestTy) {
  switch (CK) {
  case CK_Dependent: llvm_unreachable("dependent cast kind in IR gen!");

  // Atomic to non-atomic casts may be more than a no-op for some platforms and
  // for some types.
  case CK_AtomicToNonAtomic:
  case CK_NonAtomicToAtomic:
  case CK_NoOp:
  case CK_LValueToRValue:
  case CK_UserDefinedConversion:
    return Visit(Op);

  case CK_LValueBitCast: {
    LValue origLV = CGF.EmitLValue(Op);
    llvm::Value *V = origLV.getAddress();
    V = Builder.CreateBitCast(V,
                    CGF.ConvertType(CGF.getContext().getPointerType(DestTy)));
    return EmitLoadOfLValue(CGF.MakeAddrLValue(V, DestTy,
                                               origLV.getAlignment()),
                            Op->getExprLoc());
  }

  case CK_BitCast:
  case CK_BaseToDerived:
  case CK_DerivedToBase:
  case CK_UncheckedDerivedToBase:
  case CK_Dynamic:
  case CK_ToUnion:
  case CK_ArrayToPointerDecay:
  case CK_FunctionToPointerDecay:
  case CK_NullToPointer:
  case CK_NullToMemberPointer:
  case CK_BaseToDerivedMemberPointer:
  case CK_DerivedToBaseMemberPointer:
  case CK_MemberPointerToBoolean:
  case CK_ReinterpretMemberPointer:
  case CK_ConstructorConversion:
  case CK_IntegralToPointer:
  case CK_PointerToIntegral:
  case CK_PointerToBoolean:
  case CK_ToVoid:
  case CK_VectorSplat:
  case CK_IntegralCast:
  case CK_IntegralToBoolean:
  case CK_IntegralToFloating:
  case CK_FloatingToIntegral:
  case CK_FloatingToBoolean:
  case CK_FloatingCast:
  case CK_CPointerToObjCPointerCast:
  case CK_BlockPointerToObjCPointerCast:
  case CK_AnyPointerToBlockPointerCast:
  case CK_ObjCObjectLValueCast:
  case CK_FloatingComplexToReal:
  case CK_FloatingComplexToBoolean:
  case CK_IntegralComplexToReal:
  case CK_IntegralComplexToBoolean:
  case CK_ARCProduceObject:
  case CK_ARCConsumeObject:
  case CK_ARCReclaimReturnedObject:
  case CK_ARCExtendBlockObject:
  case CK_CopyAndAutoreleaseBlockObject:
  case CK_BuiltinFnToFnPtr:
  case CK_ZeroToOCLEvent:
  case CK_AddressSpaceConversion:
  // HLSL Change Start
  case CK_FlatConversion:
  case CK_HLSLVectorSplat:
  case CK_HLSLMatrixSplat:
  case CK_HLSLVectorToScalarCast:
  case CK_HLSLMatrixToScalarCast:
  case CK_HLSLVectorTruncationCast:
  case CK_HLSLMatrixTruncationCast:
  case CK_HLSLVectorToMatrixCast:
  case CK_HLSLMatrixToVectorCast:
  case CK_HLSLDerivedToBase:
  case CK_HLSLCC_IntegralCast:
  case CK_HLSLCC_IntegralToBoolean:
  case CK_HLSLCC_IntegralToFloating:
  case CK_HLSLCC_FloatingToIntegral:
  case CK_HLSLCC_FloatingToBoolean:
  case CK_HLSLCC_FloatingCast:
  // HLSL Change End
    llvm_unreachable("invalid cast kind for complex value");

  case CK_FloatingRealToComplex:
  case CK_IntegralRealToComplex:
    return EmitScalarToComplexCast(CGF.EmitScalarExpr(Op),
                                   Op->getType(), DestTy);

  case CK_FloatingComplexCast:
  case CK_FloatingComplexToIntegralComplex:
  case CK_IntegralComplexCast:
  case CK_IntegralComplexToFloatingComplex:
    return EmitComplexToComplexCast(Visit(Op), Op->getType(), DestTy);
  }

  llvm_unreachable("unknown cast resulting in complex value");
}

ComplexPairTy ComplexExprEmitter::VisitUnaryMinus(const UnaryOperator *E) {
  TestAndClearIgnoreReal();
  TestAndClearIgnoreImag();
  ComplexPairTy Op = Visit(E->getSubExpr());

  llvm::Value *ResR, *ResI;
  if (Op.first->getType()->isFloatingPointTy()) {
    ResR = Builder.CreateFNeg(Op.first,  "neg.r");
    ResI = Builder.CreateFNeg(Op.second, "neg.i");
  } else {
    ResR = Builder.CreateNeg(Op.first,  "neg.r");
    ResI = Builder.CreateNeg(Op.second, "neg.i");
  }
  return ComplexPairTy(ResR, ResI);
}

ComplexPairTy ComplexExprEmitter::VisitUnaryNot(const UnaryOperator *E) {
  TestAndClearIgnoreReal();
  TestAndClearIgnoreImag();
  // ~(a+ib) = a + i*-b
  ComplexPairTy Op = Visit(E->getSubExpr());
  llvm::Value *ResI;
  if (Op.second->getType()->isFloatingPointTy())
    ResI = Builder.CreateFNeg(Op.second, "conj.i");
  else
    ResI = Builder.CreateNeg(Op.second, "conj.i");

  return ComplexPairTy(Op.first, ResI);
}

ComplexPairTy ComplexExprEmitter::EmitBinAdd(const BinOpInfo &Op) {
  llvm::Value *ResR, *ResI;

  if (Op.LHS.first->getType()->isFloatingPointTy()) {
    ResR = Builder.CreateFAdd(Op.LHS.first,  Op.RHS.first,  "add.r");
    if (Op.LHS.second && Op.RHS.second)
      ResI = Builder.CreateFAdd(Op.LHS.second, Op.RHS.second, "add.i");
    else
      ResI = Op.LHS.second ? Op.LHS.second : Op.RHS.second;
    assert(ResI && "Only one operand may be real!");
  } else {
    ResR = Builder.CreateAdd(Op.LHS.first,  Op.RHS.first,  "add.r");
    assert(Op.LHS.second && Op.RHS.second &&
           "Both operands of integer complex operators must be complex!");
    ResI = Builder.CreateAdd(Op.LHS.second, Op.RHS.second, "add.i");
  }
  return ComplexPairTy(ResR, ResI);
}

ComplexPairTy ComplexExprEmitter::EmitBinSub(const BinOpInfo &Op) {
  llvm::Value *ResR, *ResI;
  if (Op.LHS.first->getType()->isFloatingPointTy()) {
    ResR = Builder.CreateFSub(Op.LHS.first, Op.RHS.first, "sub.r");
    if (Op.LHS.second && Op.RHS.second)
      ResI = Builder.CreateFSub(Op.LHS.second, Op.RHS.second, "sub.i");
    else
      ResI = Op.LHS.second ? Op.LHS.second
                           : Builder.CreateFNeg(Op.RHS.second, "sub.i");
    assert(ResI && "Only one operand may be real!");
  } else {
    ResR = Builder.CreateSub(Op.LHS.first, Op.RHS.first, "sub.r");
    assert(Op.LHS.second && Op.RHS.second &&
           "Both operands of integer complex operators must be complex!");
    ResI = Builder.CreateSub(Op.LHS.second, Op.RHS.second, "sub.i");
  }
  return ComplexPairTy(ResR, ResI);
}

/// \brief Emit a libcall for a binary operation on complex types.
ComplexPairTy ComplexExprEmitter::EmitComplexBinOpLibCall(StringRef LibCallName,
                                                          const BinOpInfo &Op) {
  CallArgList Args;
  Args.add(RValue::get(Op.LHS.first),
           Op.Ty->castAs<ComplexType>()->getElementType());
  Args.add(RValue::get(Op.LHS.second),
           Op.Ty->castAs<ComplexType>()->getElementType());
  Args.add(RValue::get(Op.RHS.first),
           Op.Ty->castAs<ComplexType>()->getElementType());
  Args.add(RValue::get(Op.RHS.second),
           Op.Ty->castAs<ComplexType>()->getElementType());

  // We *must* use the full CG function call building logic here because the
  // complex type has special ABI handling. We also should not forget about
  // special calling convention which may be used for compiler builtins.
  const CGFunctionInfo &FuncInfo =
    CGF.CGM.getTypes().arrangeFreeFunctionCall(
      Op.Ty, Args, FunctionType::ExtInfo(/* No CC here - will be added later */),
      RequiredArgs::All);
  llvm::FunctionType *FTy = CGF.CGM.getTypes().GetFunctionType(FuncInfo);
  llvm::Constant *Func = CGF.CGM.CreateBuiltinFunction(FTy, LibCallName);
  llvm::Instruction *Call;

  RValue Res = CGF.EmitCall(FuncInfo, Func, ReturnValueSlot(), Args,
                            nullptr, &Call);
  cast<llvm::CallInst>(Call)->setCallingConv(CGF.CGM.getBuiltinCC());
  cast<llvm::CallInst>(Call)->setDoesNotThrow();

  return Res.getComplexVal();
}

/// \brief Lookup the libcall name for a given floating point type complex
/// multiply.
static StringRef getComplexMultiplyLibCallName(llvm::Type *Ty) {
  switch (Ty->getTypeID()) {
  default:
    llvm_unreachable("Unsupported floating point type!");
  case llvm::Type::HalfTyID:
    return "__mulhc3";
  case llvm::Type::FloatTyID:
    return "__mulsc3";
  case llvm::Type::DoubleTyID:
    return "__muldc3";
  case llvm::Type::PPC_FP128TyID:
    return "__multc3";
  case llvm::Type::X86_FP80TyID:
    return "__mulxc3";
  case llvm::Type::FP128TyID:
    return "__multc3";
  }
}

// See C11 Annex G.5.1 for the semantics of multiplicative operators on complex
// typed values.
ComplexPairTy ComplexExprEmitter::EmitBinMul(const BinOpInfo &Op) {
  using llvm::Value;
  Value *ResR, *ResI;
  llvm::MDBuilder MDHelper(CGF.getLLVMContext());

  if (Op.LHS.first->getType()->isFloatingPointTy()) {
    // The general formulation is:
    // (a + ib) * (c + id) = (a * c - b * d) + i(a * d + b * c)
    //
    // But we can fold away components which would be zero due to a real
    // operand according to C11 Annex G.5.1p2.
    // FIXME: C11 also provides for imaginary types which would allow folding
    // still more of this within the type system.

    if (Op.LHS.second && Op.RHS.second) {
      // If both operands are complex, emit the core math directly, and then
      // test for NaNs. If we find NaNs in the result, we delegate to a libcall
      // to carefully re-compute the correct infinity representation if
      // possible. The expectation is that the presence of NaNs here is
      // *extremely* rare, and so the cost of the libcall is almost irrelevant.
      // This is good, because the libcall re-computes the core multiplication
      // exactly the same as we do here and re-tests for NaNs in order to be
      // a generic complex*complex libcall.

      // First compute the four products.
      Value *AC = Builder.CreateFMul(Op.LHS.first, Op.RHS.first, "mul_ac");
      Value *BD = Builder.CreateFMul(Op.LHS.second, Op.RHS.second, "mul_bd");
      Value *AD = Builder.CreateFMul(Op.LHS.first, Op.RHS.second, "mul_ad");
      Value *BC = Builder.CreateFMul(Op.LHS.second, Op.RHS.first, "mul_bc");

      // The real part is the difference of the first two, the imaginary part is
      // the sum of the second.
      ResR = Builder.CreateFSub(AC, BD, "mul_r");
      ResI = Builder.CreateFAdd(AD, BC, "mul_i");

      // Emit the test for the real part becoming NaN and create a branch to
      // handle it. We test for NaN by comparing the number to itself.
      Value *IsRNaN = Builder.CreateFCmpUNO(ResR, ResR, "isnan_cmp");
      llvm::BasicBlock *ContBB = CGF.createBasicBlock("complex_mul_cont");
      llvm::BasicBlock *INaNBB = CGF.createBasicBlock("complex_mul_imag_nan");
      llvm::Instruction *Branch = Builder.CreateCondBr(IsRNaN, INaNBB, ContBB);
      llvm::BasicBlock *OrigBB = Branch->getParent();

      // Give hint that we very much don't expect to see NaNs.
      // Value chosen to match UR_NONTAKEN_WEIGHT, see BranchProbabilityInfo.cpp
      llvm::MDNode *BrWeight = MDHelper.createBranchWeights(1, (1U << 20) - 1);
      Branch->setMetadata(llvm::LLVMContext::MD_prof, BrWeight);

      // Now test the imaginary part and create its branch.
      CGF.EmitBlock(INaNBB);
      Value *IsINaN = Builder.CreateFCmpUNO(ResI, ResI, "isnan_cmp");
      llvm::BasicBlock *LibCallBB = CGF.createBasicBlock("complex_mul_libcall");
      Branch = Builder.CreateCondBr(IsINaN, LibCallBB, ContBB);
      Branch->setMetadata(llvm::LLVMContext::MD_prof, BrWeight);

      // Now emit the libcall on this slowest of the slow paths.
      CGF.EmitBlock(LibCallBB);
      Value *LibCallR, *LibCallI;
      std::tie(LibCallR, LibCallI) = EmitComplexBinOpLibCall(
          getComplexMultiplyLibCallName(Op.LHS.first->getType()), Op);
      Builder.CreateBr(ContBB);

      // Finally continue execution by phi-ing together the different
      // computation paths.
      CGF.EmitBlock(ContBB);
      llvm::PHINode *RealPHI = Builder.CreatePHI(ResR->getType(), 3, "real_mul_phi");
      RealPHI->addIncoming(ResR, OrigBB);
      RealPHI->addIncoming(ResR, INaNBB);
      RealPHI->addIncoming(LibCallR, LibCallBB);
      llvm::PHINode *ImagPHI = Builder.CreatePHI(ResI->getType(), 3, "imag_mul_phi");
      ImagPHI->addIncoming(ResI, OrigBB);
      ImagPHI->addIncoming(ResI, INaNBB);
      ImagPHI->addIncoming(LibCallI, LibCallBB);
      return ComplexPairTy(RealPHI, ImagPHI);
    }
    assert((Op.LHS.second || Op.RHS.second) &&
           "At least one operand must be complex!");

    // If either of the operands is a real rather than a complex, the
    // imaginary component is ignored when computing the real component of the
    // result.
    ResR = Builder.CreateFMul(Op.LHS.first, Op.RHS.first, "mul.rl");

    ResI = Op.LHS.second
               ? Builder.CreateFMul(Op.LHS.second, Op.RHS.first, "mul.il")
               : Builder.CreateFMul(Op.LHS.first, Op.RHS.second, "mul.ir");
  } else {
    assert(Op.LHS.second && Op.RHS.second &&
           "Both operands of integer complex operators must be complex!");
    Value *ResRl = Builder.CreateMul(Op.LHS.first, Op.RHS.first, "mul.rl");
    Value *ResRr = Builder.CreateMul(Op.LHS.second, Op.RHS.second, "mul.rr");
    ResR = Builder.CreateSub(ResRl, ResRr, "mul.r");

    Value *ResIl = Builder.CreateMul(Op.LHS.second, Op.RHS.first, "mul.il");
    Value *ResIr = Builder.CreateMul(Op.LHS.first, Op.RHS.second, "mul.ir");
    ResI = Builder.CreateAdd(ResIl, ResIr, "mul.i");
  }
  return ComplexPairTy(ResR, ResI);
}

// See C11 Annex G.5.1 for the semantics of multiplicative operators on complex
// typed values.
ComplexPairTy ComplexExprEmitter::EmitBinDiv(const BinOpInfo &Op) {
  llvm::Value *LHSr = Op.LHS.first, *LHSi = Op.LHS.second;
  llvm::Value *RHSr = Op.RHS.first, *RHSi = Op.RHS.second;


  llvm::Value *DSTr, *DSTi;
  if (LHSr->getType()->isFloatingPointTy()) {
    // If we have a complex operand on the RHS, we delegate to a libcall to
    // handle all of the complexities and minimize underflow/overflow cases.
    //
    // FIXME: We would be able to avoid the libcall in many places if we
    // supported imaginary types in addition to complex types.
    if (RHSi) {
      BinOpInfo LibCallOp = Op;
      // If LHS was a real, supply a null imaginary part.
      if (!LHSi)
        LibCallOp.LHS.second = llvm::Constant::getNullValue(LHSr->getType());

      StringRef LibCallName;
      switch (LHSr->getType()->getTypeID()) {
      default:
        llvm_unreachable("Unsupported floating point type!");
      case llvm::Type::HalfTyID:
        return EmitComplexBinOpLibCall("__divhc3", LibCallOp);
      case llvm::Type::FloatTyID:
        return EmitComplexBinOpLibCall("__divsc3", LibCallOp);
      case llvm::Type::DoubleTyID:
        return EmitComplexBinOpLibCall("__divdc3", LibCallOp);
      case llvm::Type::PPC_FP128TyID:
        return EmitComplexBinOpLibCall("__divtc3", LibCallOp);
      case llvm::Type::X86_FP80TyID:
        return EmitComplexBinOpLibCall("__divxc3", LibCallOp);
      case llvm::Type::FP128TyID:
        return EmitComplexBinOpLibCall("__divtc3", LibCallOp);
      }
    }
    assert(LHSi && "Can have at most one non-complex operand!");

    DSTr = Builder.CreateFDiv(LHSr, RHSr);
    DSTi = Builder.CreateFDiv(LHSi, RHSr);
  } else {
    assert(Op.LHS.second && Op.RHS.second &&
           "Both operands of integer complex operators must be complex!");
    // (a+ib) / (c+id) = ((ac+bd)/(cc+dd)) + i((bc-ad)/(cc+dd))
    llvm::Value *Tmp1 = Builder.CreateMul(LHSr, RHSr); // a*c
    llvm::Value *Tmp2 = Builder.CreateMul(LHSi, RHSi); // b*d
    llvm::Value *Tmp3 = Builder.CreateAdd(Tmp1, Tmp2); // ac+bd

    llvm::Value *Tmp4 = Builder.CreateMul(RHSr, RHSr); // c*c
    llvm::Value *Tmp5 = Builder.CreateMul(RHSi, RHSi); // d*d
    llvm::Value *Tmp6 = Builder.CreateAdd(Tmp4, Tmp5); // cc+dd

    llvm::Value *Tmp7 = Builder.CreateMul(LHSi, RHSr); // b*c
    llvm::Value *Tmp8 = Builder.CreateMul(LHSr, RHSi); // a*d
    llvm::Value *Tmp9 = Builder.CreateSub(Tmp7, Tmp8); // bc-ad

    if (Op.Ty->castAs<ComplexType>()->getElementType()->isUnsignedIntegerType()) {
      DSTr = Builder.CreateUDiv(Tmp3, Tmp6);
      DSTi = Builder.CreateUDiv(Tmp9, Tmp6);
    } else {
      DSTr = Builder.CreateSDiv(Tmp3, Tmp6);
      DSTi = Builder.CreateSDiv(Tmp9, Tmp6);
    }
  }

  return ComplexPairTy(DSTr, DSTi);
}

ComplexExprEmitter::BinOpInfo
ComplexExprEmitter::EmitBinOps(const BinaryOperator *E) {
  TestAndClearIgnoreReal();
  TestAndClearIgnoreImag();
  BinOpInfo Ops;
  if (E->getLHS()->getType()->isRealFloatingType())
    Ops.LHS = ComplexPairTy(CGF.EmitScalarExpr(E->getLHS()), nullptr);
  else
    Ops.LHS = Visit(E->getLHS());
  if (E->getRHS()->getType()->isRealFloatingType())
    Ops.RHS = ComplexPairTy(CGF.EmitScalarExpr(E->getRHS()), nullptr);
  else
    Ops.RHS = Visit(E->getRHS());

  Ops.Ty = E->getType();
  return Ops;
}


LValue ComplexExprEmitter::
EmitCompoundAssignLValue(const CompoundAssignOperator *E,
          ComplexPairTy (ComplexExprEmitter::*Func)(const BinOpInfo&),
                         RValue &Val) {
  TestAndClearIgnoreReal();
  TestAndClearIgnoreImag();
  QualType LHSTy = E->getLHS()->getType();
  if (const AtomicType *AT = LHSTy->getAs<AtomicType>())
    LHSTy = AT->getValueType();

  BinOpInfo OpInfo;

  // Load the RHS and LHS operands.
  // __block variables need to have the rhs evaluated first, plus this should
  // improve codegen a little.
  OpInfo.Ty = E->getComputationResultType();
  QualType ComplexElementTy = cast<ComplexType>(OpInfo.Ty)->getElementType();

  // The RHS should have been converted to the computation type.
  if (E->getRHS()->getType()->isRealFloatingType()) {
    assert(
        CGF.getContext()
            .hasSameUnqualifiedType(ComplexElementTy, E->getRHS()->getType()));
    OpInfo.RHS = ComplexPairTy(CGF.EmitScalarExpr(E->getRHS()), nullptr);
  } else {
    assert(CGF.getContext()
               .hasSameUnqualifiedType(OpInfo.Ty, E->getRHS()->getType()));
    OpInfo.RHS = Visit(E->getRHS());
  }

  LValue LHS = CGF.EmitLValue(E->getLHS());

  // Load from the l-value and convert it.
  if (LHSTy->isAnyComplexType()) {
    ComplexPairTy LHSVal = EmitLoadOfLValue(LHS, E->getExprLoc());
    OpInfo.LHS = EmitComplexToComplexCast(LHSVal, LHSTy, OpInfo.Ty);
  } else {
    llvm::Value *LHSVal = CGF.EmitLoadOfScalar(LHS, E->getExprLoc());
    // For floating point real operands we can directly pass the scalar form
    // to the binary operator emission and potentially get more efficient code.
    if (LHSTy->isRealFloatingType()) {
      if (!CGF.getContext().hasSameUnqualifiedType(ComplexElementTy, LHSTy))
        LHSVal = CGF.EmitScalarConversion(LHSVal, LHSTy, ComplexElementTy);
      OpInfo.LHS = ComplexPairTy(LHSVal, nullptr);
    } else {
      OpInfo.LHS = EmitScalarToComplexCast(LHSVal, LHSTy, OpInfo.Ty);
    }
  }

  // Expand the binary operator.
  ComplexPairTy Result = (this->*Func)(OpInfo);

  // Truncate the result and store it into the LHS lvalue.
  if (LHSTy->isAnyComplexType()) {
    ComplexPairTy ResVal = EmitComplexToComplexCast(Result, OpInfo.Ty, LHSTy);
    EmitStoreOfComplex(ResVal, LHS, /*isInit*/ false);
    Val = RValue::getComplex(ResVal);
  } else {
    llvm::Value *ResVal =
        CGF.EmitComplexToScalarConversion(Result, OpInfo.Ty, LHSTy);
    CGF.EmitStoreOfScalar(ResVal, LHS, /*isInit*/ false);
    Val = RValue::get(ResVal);
  }

  return LHS;
}

// Compound assignments.
ComplexPairTy ComplexExprEmitter::
EmitCompoundAssign(const CompoundAssignOperator *E,
                   ComplexPairTy (ComplexExprEmitter::*Func)(const BinOpInfo&)){
  RValue Val;
  LValue LV = EmitCompoundAssignLValue(E, Func, Val);

  // The result of an assignment in C is the assigned r-value.
  if (!CGF.getLangOpts().CPlusPlus)
    return Val.getComplexVal();

  // If the lvalue is non-volatile, return the computed value of the assignment.
  if (!LV.isVolatileQualified())
    return Val.getComplexVal();

  return EmitLoadOfLValue(LV, E->getExprLoc());
}

LValue ComplexExprEmitter::EmitBinAssignLValue(const BinaryOperator *E,
                                               ComplexPairTy &Val) {
  assert(CGF.getContext().hasSameUnqualifiedType(E->getLHS()->getType(),
                                                 E->getRHS()->getType()) &&
         "Invalid assignment");
  TestAndClearIgnoreReal();
  TestAndClearIgnoreImag();

  // Emit the RHS.  __block variables need the RHS evaluated first.
  Val = Visit(E->getRHS());

  // Compute the address to store into.
  LValue LHS = CGF.EmitLValue(E->getLHS());

  // Store the result value into the LHS lvalue.
  EmitStoreOfComplex(Val, LHS, /*isInit*/ false);

  return LHS;
}

ComplexPairTy ComplexExprEmitter::VisitBinAssign(const BinaryOperator *E) {
  ComplexPairTy Val;
  LValue LV = EmitBinAssignLValue(E, Val);

  // The result of an assignment in C is the assigned r-value.
  if (!CGF.getLangOpts().CPlusPlus)
    return Val;

  // If the lvalue is non-volatile, return the computed value of the assignment.
  if (!LV.isVolatileQualified())
    return Val;

  return EmitLoadOfLValue(LV, E->getExprLoc());
}

ComplexPairTy ComplexExprEmitter::VisitBinComma(const BinaryOperator *E) {
  CGF.EmitIgnoredExpr(E->getLHS());
  return Visit(E->getRHS());
}

ComplexPairTy ComplexExprEmitter::
VisitAbstractConditionalOperator(const AbstractConditionalOperator *E) {
  TestAndClearIgnoreReal();
  TestAndClearIgnoreImag();
  llvm::BasicBlock *LHSBlock = CGF.createBasicBlock("cond.true");
  llvm::BasicBlock *RHSBlock = CGF.createBasicBlock("cond.false");
  llvm::BasicBlock *ContBlock = CGF.createBasicBlock("cond.end");

  // Bind the common expression if necessary.
  CodeGenFunction::OpaqueValueMapping binding(CGF, E);


  CodeGenFunction::ConditionalEvaluation eval(CGF);
  CGF.EmitBranchOnBoolExpr(E->getCond(), LHSBlock, RHSBlock,
                           CGF.getProfileCount(E));

  eval.begin(CGF);
  CGF.EmitBlock(LHSBlock);
  CGF.incrementProfileCounter(E);
  ComplexPairTy LHS = Visit(E->getTrueExpr());
  LHSBlock = Builder.GetInsertBlock();
  CGF.EmitBranch(ContBlock);
  eval.end(CGF);

  eval.begin(CGF);
  CGF.EmitBlock(RHSBlock);
  ComplexPairTy RHS = Visit(E->getFalseExpr());
  RHSBlock = Builder.GetInsertBlock();
  CGF.EmitBlock(ContBlock);
  eval.end(CGF);

  // Create a PHI node for the real part.
  llvm::PHINode *RealPN = Builder.CreatePHI(LHS.first->getType(), 2, "cond.r");
  RealPN->addIncoming(LHS.first, LHSBlock);
  RealPN->addIncoming(RHS.first, RHSBlock);

  // Create a PHI node for the imaginary part.
  llvm::PHINode *ImagPN = Builder.CreatePHI(LHS.first->getType(), 2, "cond.i");
  ImagPN->addIncoming(LHS.second, LHSBlock);
  ImagPN->addIncoming(RHS.second, RHSBlock);

  return ComplexPairTy(RealPN, ImagPN);
}

ComplexPairTy ComplexExprEmitter::VisitChooseExpr(ChooseExpr *E) {
  return Visit(E->getChosenSubExpr());
}

ComplexPairTy ComplexExprEmitter::VisitInitListExpr(InitListExpr *E) {
    bool Ignore = TestAndClearIgnoreReal();
    (void)Ignore;
    assert (Ignore == false && "init list ignored");
    Ignore = TestAndClearIgnoreImag();
    (void)Ignore;
    assert (Ignore == false && "init list ignored");

  if (E->getNumInits() == 2) {
    llvm::Value *Real = CGF.EmitScalarExpr(E->getInit(0));
    llvm::Value *Imag = CGF.EmitScalarExpr(E->getInit(1));
    return ComplexPairTy(Real, Imag);
  } else if (E->getNumInits() == 1) {
    return Visit(E->getInit(0));
  }

  // Empty init list intializes to null
  assert(E->getNumInits() == 0 && "Unexpected number of inits");
  QualType Ty = E->getType()->castAs<ComplexType>()->getElementType();
  llvm::Type* LTy = CGF.ConvertType(Ty);
  llvm::Value* zeroConstant = llvm::Constant::getNullValue(LTy);
  return ComplexPairTy(zeroConstant, zeroConstant);
}

ComplexPairTy ComplexExprEmitter::VisitVAArgExpr(VAArgExpr *E) {
  llvm::Value *ArgValue = CGF.EmitVAListRef(E->getSubExpr());
  llvm::Value *ArgPtr = CGF.EmitVAArg(ArgValue, E->getType());

  if (!ArgPtr) {
    CGF.ErrorUnsupported(E, "complex va_arg expression");
    llvm::Type *EltTy =
      CGF.ConvertType(E->getType()->castAs<ComplexType>()->getElementType());
    llvm::Value *U = llvm::UndefValue::get(EltTy);
    return ComplexPairTy(U, U);
  }

  return EmitLoadOfLValue(CGF.MakeNaturalAlignAddrLValue(ArgPtr, E->getType()),
                          E->getExprLoc());
}

//===----------------------------------------------------------------------===//
//                         Entry Point into this File
//===----------------------------------------------------------------------===//

/// EmitComplexExpr - Emit the computation of the specified expression of
/// complex type, ignoring the result.
ComplexPairTy CodeGenFunction::EmitComplexExpr(const Expr *E, bool IgnoreReal,
                                               bool IgnoreImag) {
  assert(E && getComplexType(E->getType()) &&
         "Invalid complex expression to emit");

  return ComplexExprEmitter(*this, IgnoreReal, IgnoreImag)
      .Visit(const_cast<Expr *>(E));
}

void CodeGenFunction::EmitComplexExprIntoLValue(const Expr *E, LValue dest,
                                                bool isInit) {
  assert(E && getComplexType(E->getType()) &&
         "Invalid complex expression to emit");
  ComplexExprEmitter Emitter(*this);
  ComplexPairTy Val = Emitter.Visit(const_cast<Expr*>(E));
  Emitter.EmitStoreOfComplex(Val, dest, isInit);
}

/// EmitStoreOfComplex - Store a complex number into the specified l-value.
void CodeGenFunction::EmitStoreOfComplex(ComplexPairTy V, LValue dest,
                                         bool isInit) {
  ComplexExprEmitter(*this).EmitStoreOfComplex(V, dest, isInit);
}

/// EmitLoadOfComplex - Load a complex number from the specified address.
ComplexPairTy CodeGenFunction::EmitLoadOfComplex(LValue src,
                                                 SourceLocation loc) {
  return ComplexExprEmitter(*this).EmitLoadOfLValue(src, loc);
}

LValue CodeGenFunction::EmitComplexAssignmentLValue(const BinaryOperator *E) {
  assert(E->getOpcode() == BO_Assign);
  ComplexPairTy Val; // ignored
  return ComplexExprEmitter(*this).EmitBinAssignLValue(E, Val);
}

typedef ComplexPairTy (ComplexExprEmitter::*CompoundFunc)(
    const ComplexExprEmitter::BinOpInfo &);

static CompoundFunc getComplexOp(BinaryOperatorKind Op) {
  switch (Op) {
  case BO_MulAssign: return &ComplexExprEmitter::EmitBinMul;
  case BO_DivAssign: return &ComplexExprEmitter::EmitBinDiv;
  case BO_SubAssign: return &ComplexExprEmitter::EmitBinSub;
  case BO_AddAssign: return &ComplexExprEmitter::EmitBinAdd;
  default:
    llvm_unreachable("unexpected complex compound assignment");
  }
}

LValue CodeGenFunction::
EmitComplexCompoundAssignmentLValue(const CompoundAssignOperator *E) {
  CompoundFunc Op = getComplexOp(E->getOpcode());
  RValue Val;
  return ComplexExprEmitter(*this).EmitCompoundAssignLValue(E, Op, Val);
}

LValue CodeGenFunction::
EmitScalarCompoundAssignWithComplex(const CompoundAssignOperator *E,
                                    llvm::Value *&Result) {
  CompoundFunc Op = getComplexOp(E->getOpcode());
  RValue Val;
  LValue Ret = ComplexExprEmitter(*this).EmitCompoundAssignLValue(E, Op, Val);
  Result = Val.getScalarVal();
  return Ret;
}
