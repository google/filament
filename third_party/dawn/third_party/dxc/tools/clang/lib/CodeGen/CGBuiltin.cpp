//===---- CGBuiltin.cpp - Emit LLVM Code for builtins ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This contains code to emit Builtin calls as LLVM code.
//
//===----------------------------------------------------------------------===//

#include "CodeGenFunction.h"
#include "CGCXXABI.h"
#include "CGObjCRuntime.h"
#include "CodeGenModule.h"
#include "TargetInfo.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/Basic/TargetBuiltins.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/CodeGen/CGFunctionInfo.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Intrinsics.h"
#include <sstream>

using namespace clang;
using namespace CodeGen;
using namespace llvm;

/// getBuiltinLibFunction - Given a builtin id for a function like
/// "__builtin_fabsf", return a Function* for "fabsf".
llvm::Value *CodeGenModule::getBuiltinLibFunction(const FunctionDecl *FD,
                                                  unsigned BuiltinID) {
  assert(Context.BuiltinInfo.isLibFunction(BuiltinID));

  // Get the name, skip over the __builtin_ prefix (if necessary).
  StringRef Name;
  GlobalDecl D(FD);

  // If the builtin has been declared explicitly with an assembler label,
  // use the mangled name. This differs from the plain label on platforms
  // that prefix labels.
  if (FD->hasAttr<AsmLabelAttr>())
    Name = getMangledName(D);
  else
    Name = Context.BuiltinInfo.GetName(BuiltinID) + 10;

  llvm::FunctionType *Ty =
    cast<llvm::FunctionType>(getTypes().ConvertType(FD->getType()));

  return GetOrCreateLLVMFunction(Name, Ty, D, /*ForVTable=*/false);
}

/// Emit the conversions required to turn the given value into an
/// integer of the given size.
static Value *EmitToInt(CodeGenFunction &CGF, llvm::Value *V,
                        QualType T, llvm::IntegerType *IntType) {
  V = CGF.EmitToMemory(V, T);

  if (V->getType()->isPointerTy())
    return CGF.Builder.CreatePtrToInt(V, IntType);

  assert(V->getType() == IntType);
  return V;
}

static Value *EmitFromInt(CodeGenFunction &CGF, llvm::Value *V,
                          QualType T, llvm::Type *ResultType) {
  V = CGF.EmitFromMemory(V, T);

  if (ResultType->isPointerTy())
    return CGF.Builder.CreateIntToPtr(V, ResultType);

  assert(V->getType() == ResultType);
  return V;
}

/// Utility to insert an atomic instruction based on Instrinsic::ID
/// and the expression node.
static Value *MakeBinaryAtomicValue(CodeGenFunction &CGF,
                                    llvm::AtomicRMWInst::BinOp Kind,
                                    const CallExpr *E) {
  QualType T = E->getType();
  assert(E->getArg(0)->getType()->isPointerType());
  assert(CGF.getContext().hasSameUnqualifiedType(T,
                                  E->getArg(0)->getType()->getPointeeType()));
  assert(CGF.getContext().hasSameUnqualifiedType(T, E->getArg(1)->getType()));

  llvm::Value *DestPtr = CGF.EmitScalarExpr(E->getArg(0));
  unsigned AddrSpace = DestPtr->getType()->getPointerAddressSpace();

  llvm::IntegerType *IntType =
    llvm::IntegerType::get(CGF.getLLVMContext(),
                           CGF.getContext().getTypeSize(T));
  llvm::Type *IntPtrType = IntType->getPointerTo(AddrSpace);

  llvm::Value *Args[2];
  Args[0] = CGF.Builder.CreateBitCast(DestPtr, IntPtrType);
  Args[1] = CGF.EmitScalarExpr(E->getArg(1));
  llvm::Type *ValueType = Args[1]->getType();
  Args[1] = EmitToInt(CGF, Args[1], T, IntType);

  llvm::Value *Result =
      CGF.Builder.CreateAtomicRMW(Kind, Args[0], Args[1],
                                  llvm::SequentiallyConsistent);
  return EmitFromInt(CGF, Result, T, ValueType);
}

static RValue EmitBinaryAtomic(CodeGenFunction &CGF,
                               llvm::AtomicRMWInst::BinOp Kind,
                               const CallExpr *E) {
  return RValue::get(MakeBinaryAtomicValue(CGF, Kind, E));
}

/// Utility to insert an atomic instruction based Instrinsic::ID and
/// the expression node, where the return value is the result of the
/// operation.
static RValue EmitBinaryAtomicPost(CodeGenFunction &CGF,
                                   llvm::AtomicRMWInst::BinOp Kind,
                                   const CallExpr *E,
                                   Instruction::BinaryOps Op,
                                   bool Invert = false) {
  QualType T = E->getType();
  assert(E->getArg(0)->getType()->isPointerType());
  assert(CGF.getContext().hasSameUnqualifiedType(T,
                                  E->getArg(0)->getType()->getPointeeType()));
  assert(CGF.getContext().hasSameUnqualifiedType(T, E->getArg(1)->getType()));

  llvm::Value *DestPtr = CGF.EmitScalarExpr(E->getArg(0));
  unsigned AddrSpace = DestPtr->getType()->getPointerAddressSpace();

  llvm::IntegerType *IntType =
    llvm::IntegerType::get(CGF.getLLVMContext(),
                           CGF.getContext().getTypeSize(T));
  llvm::Type *IntPtrType = IntType->getPointerTo(AddrSpace);

  llvm::Value *Args[2];
  Args[1] = CGF.EmitScalarExpr(E->getArg(1));
  llvm::Type *ValueType = Args[1]->getType();
  Args[1] = EmitToInt(CGF, Args[1], T, IntType);
  Args[0] = CGF.Builder.CreateBitCast(DestPtr, IntPtrType);

  llvm::Value *Result =
      CGF.Builder.CreateAtomicRMW(Kind, Args[0], Args[1],
                                  llvm::SequentiallyConsistent);
  Result = CGF.Builder.CreateBinOp(Op, Result, Args[1]);
  if (Invert)
    Result = CGF.Builder.CreateBinOp(llvm::Instruction::Xor, Result,
                                     llvm::ConstantInt::get(IntType, -1));
  Result = EmitFromInt(CGF, Result, T, ValueType);
  return RValue::get(Result);
}

/// @brief Utility to insert an atomic cmpxchg instruction.
///
/// @param CGF The current codegen function.
/// @param E   Builtin call expression to convert to cmpxchg.
///            arg0 - address to operate on
///            arg1 - value to compare with
///            arg2 - new value
/// @param ReturnBool Specifies whether to return success flag of
///                   cmpxchg result or the old value.
///
/// @returns result of cmpxchg, according to ReturnBool
static Value *MakeAtomicCmpXchgValue(CodeGenFunction &CGF, const CallExpr *E,
                                     bool ReturnBool) {
  QualType T = ReturnBool ? E->getArg(1)->getType() : E->getType();
  llvm::Value *DestPtr = CGF.EmitScalarExpr(E->getArg(0));
  unsigned AddrSpace = DestPtr->getType()->getPointerAddressSpace();

  llvm::IntegerType *IntType = llvm::IntegerType::get(
      CGF.getLLVMContext(), CGF.getContext().getTypeSize(T));
  llvm::Type *IntPtrType = IntType->getPointerTo(AddrSpace);

  Value *Args[3];
  Args[0] = CGF.Builder.CreateBitCast(DestPtr, IntPtrType);
  Args[1] = CGF.EmitScalarExpr(E->getArg(1));
  llvm::Type *ValueType = Args[1]->getType();
  Args[1] = EmitToInt(CGF, Args[1], T, IntType);
  Args[2] = EmitToInt(CGF, CGF.EmitScalarExpr(E->getArg(2)), T, IntType);

  Value *Pair = CGF.Builder.CreateAtomicCmpXchg(Args[0], Args[1], Args[2],
                                                llvm::SequentiallyConsistent,
                                                llvm::SequentiallyConsistent);
  if (ReturnBool)
    // Extract boolean success flag and zext it to int.
    return CGF.Builder.CreateZExt(CGF.Builder.CreateExtractValue(Pair, 1),
                                  CGF.ConvertType(E->getType()));
  else
    // Extract old value and emit it using the same type as compare value.
    return EmitFromInt(CGF, CGF.Builder.CreateExtractValue(Pair, 0), T,
                       ValueType);
}

/// EmitFAbs - Emit a call to @llvm.fabs().
static Value *EmitFAbs(CodeGenFunction &CGF, Value *V) {
  Value *F = CGF.CGM.getIntrinsic(Intrinsic::fabs, V->getType());
  llvm::CallInst *Call = CGF.Builder.CreateCall(F, V);
  Call->setDoesNotAccessMemory();
  return Call;
}

/// Emit the computation of the sign bit for a floating point value. Returns
/// the i1 sign bit value.
static Value *EmitSignBit(CodeGenFunction &CGF, Value *V) {
  LLVMContext &C = CGF.CGM.getLLVMContext();

  llvm::Type *Ty = V->getType();
  int Width = Ty->getPrimitiveSizeInBits();
  llvm::Type *IntTy = llvm::IntegerType::get(C, Width);
  V = CGF.Builder.CreateBitCast(V, IntTy);
  if (Ty->isPPC_FP128Ty()) {
    // The higher-order double comes first, and so we need to truncate the
    // pair to extract the overall sign. The order of the pair is the same
    // in both little- and big-Endian modes.
    Width >>= 1;
    IntTy = llvm::IntegerType::get(C, Width);
    V = CGF.Builder.CreateTrunc(V, IntTy);
  }
  Value *Zero = llvm::Constant::getNullValue(IntTy);
  return CGF.Builder.CreateICmpSLT(V, Zero);
}

static RValue emitLibraryCall(CodeGenFunction &CGF, const FunctionDecl *Fn,
                              const CallExpr *E, llvm::Value *calleeValue) {
  return CGF.EmitCall(E->getCallee()->getType(), calleeValue, E,
                      ReturnValueSlot(), Fn);
}

/// \brief Emit a call to llvm.{sadd,uadd,ssub,usub,smul,umul}.with.overflow.*
/// depending on IntrinsicID.
///
/// \arg CGF The current codegen function.
/// \arg IntrinsicID The ID for the Intrinsic we wish to generate.
/// \arg X The first argument to the llvm.*.with.overflow.*.
/// \arg Y The second argument to the llvm.*.with.overflow.*.
/// \arg Carry The carry returned by the llvm.*.with.overflow.*.
/// \returns The result (i.e. sum/product) returned by the intrinsic.
static llvm::Value *EmitOverflowIntrinsic(CodeGenFunction &CGF,
                                          const llvm::Intrinsic::ID IntrinsicID,
                                          llvm::Value *X, llvm::Value *Y,
                                          llvm::Value *&Carry) {
  // Make sure we have integers of the same width.
  assert(X->getType() == Y->getType() &&
         "Arguments must be the same type. (Did you forget to make sure both "
         "arguments have the same integer width?)");

  llvm::Value *Callee = CGF.CGM.getIntrinsic(IntrinsicID, X->getType());
  llvm::Value *Tmp = CGF.Builder.CreateCall(Callee, {X, Y});
  Carry = CGF.Builder.CreateExtractValue(Tmp, 1);
  return CGF.Builder.CreateExtractValue(Tmp, 0);
}

RValue CodeGenFunction::EmitBuiltinExpr(const FunctionDecl *FD,
                                        unsigned BuiltinID, const CallExpr *E,
                                        ReturnValueSlot ReturnValue) {
  // See if we can constant fold this builtin.  If so, don't emit it at all.
  Expr::EvalResult Result;
  if (E->EvaluateAsRValue(Result, CGM.getContext()) &&
      !Result.hasSideEffects()) {
    if (Result.Val.isInt())
      return RValue::get(llvm::ConstantInt::get(getLLVMContext(),
                                                Result.Val.getInt()));
    if (Result.Val.isFloat())
      return RValue::get(llvm::ConstantFP::get(getLLVMContext(),
                                               Result.Val.getFloat()));
  }

  switch (BuiltinID) {
  default: break;  // Handle intrinsics and libm functions below.
  case Builtin::BI__builtin___CFStringMakeConstantString:
  case Builtin::BI__builtin___NSStringMakeConstantString:
    return RValue::get(CGM.EmitConstantExpr(E, E->getType(), nullptr));
  case Builtin::BI__builtin_stdarg_start:
  case Builtin::BI__builtin_va_start:
  case Builtin::BI__va_start:
  case Builtin::BI__builtin_va_end: {
    Value *ArgValue = (BuiltinID == Builtin::BI__va_start)
                          ? EmitScalarExpr(E->getArg(0))
                          : EmitVAListRef(E->getArg(0));
    llvm::Type *DestType = Int8PtrTy;
    if (ArgValue->getType() != DestType)
      ArgValue = Builder.CreateBitCast(ArgValue, DestType,
                                       ArgValue->getName().data());

    Intrinsic::ID inst = (BuiltinID == Builtin::BI__builtin_va_end) ?
      Intrinsic::vaend : Intrinsic::vastart;
    return RValue::get(Builder.CreateCall(CGM.getIntrinsic(inst), ArgValue));
  }
  case Builtin::BI__builtin_va_copy: {
    Value *DstPtr = EmitVAListRef(E->getArg(0));
    Value *SrcPtr = EmitVAListRef(E->getArg(1));

    llvm::Type *Type = Int8PtrTy;

    DstPtr = Builder.CreateBitCast(DstPtr, Type);
    SrcPtr = Builder.CreateBitCast(SrcPtr, Type);
    return RValue::get(Builder.CreateCall(CGM.getIntrinsic(Intrinsic::vacopy),
                                          {DstPtr, SrcPtr}));
  }
  case Builtin::BI__builtin_abs:
  case Builtin::BI__builtin_labs:
  case Builtin::BI__builtin_llabs: {
    Value *ArgValue = EmitScalarExpr(E->getArg(0));

    Value *NegOp = Builder.CreateNeg(ArgValue, "neg");
    Value *CmpResult =
    Builder.CreateICmpSGE(ArgValue,
                          llvm::Constant::getNullValue(ArgValue->getType()),
                                                            "abscond");
    Value *Result =
      Builder.CreateSelect(CmpResult, ArgValue, NegOp, "abs");

    return RValue::get(Result);
  }
  case Builtin::BI__builtin_fabs:
  case Builtin::BI__builtin_fabsf:
  case Builtin::BI__builtin_fabsl: {
    Value *Arg1 = EmitScalarExpr(E->getArg(0));
    Value *Result = EmitFAbs(*this, Arg1);
    return RValue::get(Result);
  }
  case Builtin::BI__builtin_fmod:
  case Builtin::BI__builtin_fmodf:
  case Builtin::BI__builtin_fmodl: {
    Value *Arg1 = EmitScalarExpr(E->getArg(0));
    Value *Arg2 = EmitScalarExpr(E->getArg(1));
    Value *Result = Builder.CreateFRem(Arg1, Arg2, "fmod");
    return RValue::get(Result);
  }

  case Builtin::BI__builtin_conj:
  case Builtin::BI__builtin_conjf:
  case Builtin::BI__builtin_conjl: {
    ComplexPairTy ComplexVal = EmitComplexExpr(E->getArg(0));
    Value *Real = ComplexVal.first;
    Value *Imag = ComplexVal.second;
    Value *Zero =
      Imag->getType()->isFPOrFPVectorTy()
        ? llvm::ConstantFP::getZeroValueForNegation(Imag->getType())
        : llvm::Constant::getNullValue(Imag->getType());

    Imag = Builder.CreateFSub(Zero, Imag, "sub");
    return RValue::getComplex(std::make_pair(Real, Imag));
  }
  case Builtin::BI__builtin_creal:
  case Builtin::BI__builtin_crealf:
  case Builtin::BI__builtin_creall:
  case Builtin::BIcreal:
  case Builtin::BIcrealf:
  case Builtin::BIcreall: {
    ComplexPairTy ComplexVal = EmitComplexExpr(E->getArg(0));
    return RValue::get(ComplexVal.first);
  }

  case Builtin::BI__builtin_cimag:
  case Builtin::BI__builtin_cimagf:
  case Builtin::BI__builtin_cimagl:
  case Builtin::BIcimag:
  case Builtin::BIcimagf:
  case Builtin::BIcimagl: {
    ComplexPairTy ComplexVal = EmitComplexExpr(E->getArg(0));
    return RValue::get(ComplexVal.second);
  }

  case Builtin::BI__builtin_ctzs:
  case Builtin::BI__builtin_ctz:
  case Builtin::BI__builtin_ctzl:
  case Builtin::BI__builtin_ctzll: {
    Value *ArgValue = EmitScalarExpr(E->getArg(0));

    llvm::Type *ArgType = ArgValue->getType();
    Value *F = CGM.getIntrinsic(Intrinsic::cttz, ArgType);

    llvm::Type *ResultType = ConvertType(E->getType());
    Value *ZeroUndef = Builder.getInt1(getTarget().isCLZForZeroUndef());
    Value *Result = Builder.CreateCall(F, {ArgValue, ZeroUndef});
    if (Result->getType() != ResultType)
      Result = Builder.CreateIntCast(Result, ResultType, /*isSigned*/true,
                                     "cast");
    return RValue::get(Result);
  }
  case Builtin::BI__builtin_clzs:
  case Builtin::BI__builtin_clz:
  case Builtin::BI__builtin_clzl:
  case Builtin::BI__builtin_clzll: {
    Value *ArgValue = EmitScalarExpr(E->getArg(0));

    llvm::Type *ArgType = ArgValue->getType();
    Value *F = CGM.getIntrinsic(Intrinsic::ctlz, ArgType);

    llvm::Type *ResultType = ConvertType(E->getType());
    Value *ZeroUndef = Builder.getInt1(getTarget().isCLZForZeroUndef());
    Value *Result = Builder.CreateCall(F, {ArgValue, ZeroUndef});
    if (Result->getType() != ResultType)
      Result = Builder.CreateIntCast(Result, ResultType, /*isSigned*/true,
                                     "cast");
    return RValue::get(Result);
  }
  case Builtin::BI__builtin_ffs:
  case Builtin::BI__builtin_ffsl:
  case Builtin::BI__builtin_ffsll: {
    // ffs(x) -> x ? cttz(x) + 1 : 0
    Value *ArgValue = EmitScalarExpr(E->getArg(0));

    llvm::Type *ArgType = ArgValue->getType();
    Value *F = CGM.getIntrinsic(Intrinsic::cttz, ArgType);

    llvm::Type *ResultType = ConvertType(E->getType());
    Value *Tmp =
        Builder.CreateAdd(Builder.CreateCall(F, {ArgValue, Builder.getTrue()}),
                          llvm::ConstantInt::get(ArgType, 1));
    Value *Zero = llvm::Constant::getNullValue(ArgType);
    Value *IsZero = Builder.CreateICmpEQ(ArgValue, Zero, "iszero");
    Value *Result = Builder.CreateSelect(IsZero, Zero, Tmp, "ffs");
    if (Result->getType() != ResultType)
      Result = Builder.CreateIntCast(Result, ResultType, /*isSigned*/true,
                                     "cast");
    return RValue::get(Result);
  }
  case Builtin::BI__builtin_parity:
  case Builtin::BI__builtin_parityl:
  case Builtin::BI__builtin_parityll: {
    // parity(x) -> ctpop(x) & 1
    Value *ArgValue = EmitScalarExpr(E->getArg(0));

    llvm::Type *ArgType = ArgValue->getType();
    Value *F = CGM.getIntrinsic(Intrinsic::ctpop, ArgType);

    llvm::Type *ResultType = ConvertType(E->getType());
    Value *Tmp = Builder.CreateCall(F, ArgValue);
    Value *Result = Builder.CreateAnd(Tmp, llvm::ConstantInt::get(ArgType, 1));
    if (Result->getType() != ResultType)
      Result = Builder.CreateIntCast(Result, ResultType, /*isSigned*/true,
                                     "cast");
    return RValue::get(Result);
  }
  case Builtin::BI__builtin_popcount:
  case Builtin::BI__builtin_popcountl:
  case Builtin::BI__builtin_popcountll: {
    Value *ArgValue = EmitScalarExpr(E->getArg(0));

    llvm::Type *ArgType = ArgValue->getType();
    Value *F = CGM.getIntrinsic(Intrinsic::ctpop, ArgType);

    llvm::Type *ResultType = ConvertType(E->getType());
    Value *Result = Builder.CreateCall(F, ArgValue);
    if (Result->getType() != ResultType)
      Result = Builder.CreateIntCast(Result, ResultType, /*isSigned*/true,
                                     "cast");
    return RValue::get(Result);
  }
  case Builtin::BI__builtin_expect: {
    Value *ArgValue = EmitScalarExpr(E->getArg(0));
    llvm::Type *ArgType = ArgValue->getType();

    Value *ExpectedValue = EmitScalarExpr(E->getArg(1));
    // Don't generate llvm.expect on -O0 as the backend won't use it for
    // anything.
    // Note, we still IRGen ExpectedValue because it could have side-effects.
    if (CGM.getCodeGenOpts().OptimizationLevel == 0)
      return RValue::get(ArgValue);

    Value *FnExpect = CGM.getIntrinsic(Intrinsic::expect, ArgType);
    Value *Result =
        Builder.CreateCall(FnExpect, {ArgValue, ExpectedValue}, "expval");
    return RValue::get(Result);
  }
  case Builtin::BI__builtin_assume_aligned: {
    Value *PtrValue = EmitScalarExpr(E->getArg(0));
    Value *OffsetValue =
      (E->getNumArgs() > 2) ? EmitScalarExpr(E->getArg(2)) : nullptr;

    Value *AlignmentValue = EmitScalarExpr(E->getArg(1));
    ConstantInt *AlignmentCI = cast<ConstantInt>(AlignmentValue);
    unsigned Alignment = (unsigned) AlignmentCI->getZExtValue();

    EmitAlignmentAssumption(PtrValue, Alignment, OffsetValue);
    return RValue::get(PtrValue);
  }
  case Builtin::BI__assume:
  case Builtin::BI__builtin_assume: {
    if (E->getArg(0)->HasSideEffects(getContext()))
      return RValue::get(nullptr);

    Value *ArgValue = EmitScalarExpr(E->getArg(0));
    Value *FnAssume = CGM.getIntrinsic(Intrinsic::assume);
    return RValue::get(Builder.CreateCall(FnAssume, ArgValue));
  }
  case Builtin::BI__builtin_bswap16:
  case Builtin::BI__builtin_bswap32:
  case Builtin::BI__builtin_bswap64: {
    Value *ArgValue = EmitScalarExpr(E->getArg(0));
    llvm::Type *ArgType = ArgValue->getType();
    Value *F = CGM.getIntrinsic(Intrinsic::bswap, ArgType);
    return RValue::get(Builder.CreateCall(F, ArgValue));
  }
  case Builtin::BI__builtin_object_size: {
    // We rely on constant folding to deal with expressions with side effects.
    assert(!E->getArg(0)->HasSideEffects(getContext()) &&
           "should have been constant folded");

    // We pass this builtin onto the optimizer so that it can
    // figure out the object size in more complex cases.
    llvm::Type *ResType = ConvertType(E->getType());

    // LLVM only supports 0 and 2, make sure that we pass along that
    // as a boolean.
    Value *Ty = EmitScalarExpr(E->getArg(1));
    ConstantInt *CI = dyn_cast<ConstantInt>(Ty);
    assert(CI);
    uint64_t val = CI->getZExtValue();
    CI = ConstantInt::get(Builder.getInt1Ty(), (val & 0x2) >> 1);
    // FIXME: Get right address space.
    llvm::Type *Tys[] = { ResType, Builder.getInt8PtrTy(0) };
    Value *F = CGM.getIntrinsic(Intrinsic::objectsize, Tys);
    return RValue::get(
        Builder.CreateCall(F, {EmitScalarExpr(E->getArg(0)), CI}));
  }
  case Builtin::BI__builtin_prefetch: {
    Value *Locality, *RW, *Address = EmitScalarExpr(E->getArg(0));
    // FIXME: Technically these constants should of type 'int', yes?
    RW = (E->getNumArgs() > 1) ? EmitScalarExpr(E->getArg(1)) :
      llvm::ConstantInt::get(Int32Ty, 0);
    Locality = (E->getNumArgs() > 2) ? EmitScalarExpr(E->getArg(2)) :
      llvm::ConstantInt::get(Int32Ty, 3);
    Value *Data = llvm::ConstantInt::get(Int32Ty, 1);
    Value *F = CGM.getIntrinsic(Intrinsic::prefetch);
    return RValue::get(Builder.CreateCall(F, {Address, RW, Locality, Data}));
  }
  case Builtin::BI__builtin_readcyclecounter: {
    Value *F = CGM.getIntrinsic(Intrinsic::readcyclecounter);
    return RValue::get(Builder.CreateCall(F));
  }
  case Builtin::BI__builtin___clear_cache: {
    Value *Begin = EmitScalarExpr(E->getArg(0));
    Value *End = EmitScalarExpr(E->getArg(1));
    Value *F = CGM.getIntrinsic(Intrinsic::clear_cache);
    return RValue::get(Builder.CreateCall(F, {Begin, End}));
  }
  case Builtin::BI__builtin_trap:
    return RValue::get(EmitTrapCall(Intrinsic::trap));
  case Builtin::BI__debugbreak:
    return RValue::get(EmitTrapCall(Intrinsic::debugtrap));
  case Builtin::BI__builtin_unreachable: {
    if (SanOpts.has(SanitizerKind::Unreachable)) {
      SanitizerScope SanScope(this);
      EmitCheck(std::make_pair(static_cast<llvm::Value *>(Builder.getFalse()),
                               SanitizerKind::Unreachable),
                "builtin_unreachable", EmitCheckSourceLocation(E->getExprLoc()),
                None);
    } else
      Builder.CreateUnreachable();

    // We do need to preserve an insertion point.
    EmitBlock(createBasicBlock("unreachable.cont"));

    return RValue::get(nullptr);
  }

  case Builtin::BI__builtin_powi:
  case Builtin::BI__builtin_powif:
  case Builtin::BI__builtin_powil: {
    Value *Base = EmitScalarExpr(E->getArg(0));
    Value *Exponent = EmitScalarExpr(E->getArg(1));
    llvm::Type *ArgType = Base->getType();
    Value *F = CGM.getIntrinsic(Intrinsic::powi, ArgType);
    return RValue::get(Builder.CreateCall(F, {Base, Exponent}));
  }

  case Builtin::BI__builtin_isgreater:
  case Builtin::BI__builtin_isgreaterequal:
  case Builtin::BI__builtin_isless:
  case Builtin::BI__builtin_islessequal:
  case Builtin::BI__builtin_islessgreater:
  case Builtin::BI__builtin_isunordered: {
    // Ordered comparisons: we know the arguments to these are matching scalar
    // floating point values.
    Value *LHS = EmitScalarExpr(E->getArg(0));
    Value *RHS = EmitScalarExpr(E->getArg(1));

    switch (BuiltinID) {
    default: llvm_unreachable("Unknown ordered comparison");
    case Builtin::BI__builtin_isgreater:
      LHS = Builder.CreateFCmpOGT(LHS, RHS, "cmp");
      break;
    case Builtin::BI__builtin_isgreaterequal:
      LHS = Builder.CreateFCmpOGE(LHS, RHS, "cmp");
      break;
    case Builtin::BI__builtin_isless:
      LHS = Builder.CreateFCmpOLT(LHS, RHS, "cmp");
      break;
    case Builtin::BI__builtin_islessequal:
      LHS = Builder.CreateFCmpOLE(LHS, RHS, "cmp");
      break;
    case Builtin::BI__builtin_islessgreater:
      LHS = Builder.CreateFCmpONE(LHS, RHS, "cmp");
      break;
    case Builtin::BI__builtin_isunordered:
      LHS = Builder.CreateFCmpUNO(LHS, RHS, "cmp");
      break;
    }
    // ZExt bool to int type.
    return RValue::get(Builder.CreateZExt(LHS, ConvertType(E->getType())));
  }
  case Builtin::BI__builtin_isnan: {
    Value *V = EmitScalarExpr(E->getArg(0));
    V = Builder.CreateFCmpUNO(V, V, "cmp");
    return RValue::get(Builder.CreateZExt(V, ConvertType(E->getType())));
  }

  case Builtin::BI__builtin_isinf: {
    // isinf(x) --> fabs(x) == infinity
    Value *V = EmitScalarExpr(E->getArg(0));
    V = EmitFAbs(*this, V);

    V = Builder.CreateFCmpOEQ(V, ConstantFP::getInfinity(V->getType()),"isinf");
    return RValue::get(Builder.CreateZExt(V, ConvertType(E->getType())));
  }

  case Builtin::BI__builtin_isinf_sign: {
    // isinf_sign(x) -> fabs(x) == infinity ? (signbit(x) ? -1 : 1) : 0
    Value *Arg = EmitScalarExpr(E->getArg(0));
    Value *AbsArg = EmitFAbs(*this, Arg);
    Value *IsInf = Builder.CreateFCmpOEQ(
        AbsArg, ConstantFP::getInfinity(Arg->getType()), "isinf");
    Value *IsNeg = EmitSignBit(*this, Arg);

    llvm::Type *IntTy = ConvertType(E->getType());
    Value *Zero = Constant::getNullValue(IntTy);
    Value *One = ConstantInt::get(IntTy, 1);
    Value *NegativeOne = ConstantInt::get(IntTy, -1);
    Value *SignResult = Builder.CreateSelect(IsNeg, NegativeOne, One);
    Value *Result = Builder.CreateSelect(IsInf, SignResult, Zero);
    return RValue::get(Result);
  }

  case Builtin::BI__builtin_isnormal: {
    // isnormal(x) --> x == x && fabsf(x) < infinity && fabsf(x) >= float_min
    Value *V = EmitScalarExpr(E->getArg(0));
    Value *Eq = Builder.CreateFCmpOEQ(V, V, "iseq");

    Value *Abs = EmitFAbs(*this, V);
    Value *IsLessThanInf =
      Builder.CreateFCmpULT(Abs, ConstantFP::getInfinity(V->getType()),"isinf");
    APFloat Smallest = APFloat::getSmallestNormalized(
                   getContext().getFloatTypeSemantics(E->getArg(0)->getType()));
    Value *IsNormal =
      Builder.CreateFCmpUGE(Abs, ConstantFP::get(V->getContext(), Smallest),
                            "isnormal");
    V = Builder.CreateAnd(Eq, IsLessThanInf, "and");
    V = Builder.CreateAnd(V, IsNormal, "and");
    return RValue::get(Builder.CreateZExt(V, ConvertType(E->getType())));
  }

  case Builtin::BI__builtin_isfinite: {
    // isfinite(x) --> x == x && fabs(x) != infinity;
    Value *V = EmitScalarExpr(E->getArg(0));
    Value *Eq = Builder.CreateFCmpOEQ(V, V, "iseq");

    Value *Abs = EmitFAbs(*this, V);
    Value *IsNotInf =
      Builder.CreateFCmpUNE(Abs, ConstantFP::getInfinity(V->getType()),"isinf");

    V = Builder.CreateAnd(Eq, IsNotInf, "and");
    return RValue::get(Builder.CreateZExt(V, ConvertType(E->getType())));
  }

  case Builtin::BI__builtin_fpclassify: {
    Value *V = EmitScalarExpr(E->getArg(5));
    llvm::Type *Ty = ConvertType(E->getArg(5)->getType());

    // Create Result
    BasicBlock *Begin = Builder.GetInsertBlock();
    BasicBlock *End = createBasicBlock("fpclassify_end", this->CurFn);
    Builder.SetInsertPoint(End);
    PHINode *Result =
      Builder.CreatePHI(ConvertType(E->getArg(0)->getType()), 4,
                        "fpclassify_result");

    // if (V==0) return FP_ZERO
    Builder.SetInsertPoint(Begin);
    Value *IsZero = Builder.CreateFCmpOEQ(V, Constant::getNullValue(Ty),
                                          "iszero");
    Value *ZeroLiteral = EmitScalarExpr(E->getArg(4));
    BasicBlock *NotZero = createBasicBlock("fpclassify_not_zero", this->CurFn);
    Builder.CreateCondBr(IsZero, End, NotZero);
    Result->addIncoming(ZeroLiteral, Begin);

    // if (V != V) return FP_NAN
    Builder.SetInsertPoint(NotZero);
    Value *IsNan = Builder.CreateFCmpUNO(V, V, "cmp");
    Value *NanLiteral = EmitScalarExpr(E->getArg(0));
    BasicBlock *NotNan = createBasicBlock("fpclassify_not_nan", this->CurFn);
    Builder.CreateCondBr(IsNan, End, NotNan);
    Result->addIncoming(NanLiteral, NotZero);

    // if (fabs(V) == infinity) return FP_INFINITY
    Builder.SetInsertPoint(NotNan);
    Value *VAbs = EmitFAbs(*this, V);
    Value *IsInf =
      Builder.CreateFCmpOEQ(VAbs, ConstantFP::getInfinity(V->getType()),
                            "isinf");
    Value *InfLiteral = EmitScalarExpr(E->getArg(1));
    BasicBlock *NotInf = createBasicBlock("fpclassify_not_inf", this->CurFn);
    Builder.CreateCondBr(IsInf, End, NotInf);
    Result->addIncoming(InfLiteral, NotNan);

    // if (fabs(V) >= MIN_NORMAL) return FP_NORMAL else FP_SUBNORMAL
    Builder.SetInsertPoint(NotInf);
    APFloat Smallest = APFloat::getSmallestNormalized(
        getContext().getFloatTypeSemantics(E->getArg(5)->getType()));
    Value *IsNormal =
      Builder.CreateFCmpUGE(VAbs, ConstantFP::get(V->getContext(), Smallest),
                            "isnormal");
    Value *NormalResult =
      Builder.CreateSelect(IsNormal, EmitScalarExpr(E->getArg(2)),
                           EmitScalarExpr(E->getArg(3)));
    Builder.CreateBr(End);
    Result->addIncoming(NormalResult, NotInf);

    // return Result
    Builder.SetInsertPoint(End);
    return RValue::get(Result);
  }

  case Builtin::BIalloca:
  case Builtin::BI_alloca:
  case Builtin::BI__builtin_alloca: {
    Value *Size = EmitScalarExpr(E->getArg(0));
    return RValue::get(Builder.CreateAlloca(Builder.getInt8Ty(), Size));
  }
  case Builtin::BIbzero:
  case Builtin::BI__builtin_bzero: {
    std::pair<llvm::Value*, unsigned> Dest =
        EmitPointerWithAlignment(E->getArg(0));
    Value *SizeVal = EmitScalarExpr(E->getArg(1));
    EmitNonNullArgCheck(RValue::get(Dest.first), E->getArg(0)->getType(),
                        E->getArg(0)->getExprLoc(), FD, 0);
    Builder.CreateMemSet(Dest.first, Builder.getInt8(0), SizeVal,
                         Dest.second, false);
    return RValue::get(Dest.first);
  }
  case Builtin::BImemcpy:
  case Builtin::BI__builtin_memcpy: {
    std::pair<llvm::Value*, unsigned> Dest =
        EmitPointerWithAlignment(E->getArg(0));
    std::pair<llvm::Value*, unsigned> Src =
        EmitPointerWithAlignment(E->getArg(1));
    Value *SizeVal = EmitScalarExpr(E->getArg(2));
    unsigned Align = std::min(Dest.second, Src.second);
    EmitNonNullArgCheck(RValue::get(Dest.first), E->getArg(0)->getType(),
                        E->getArg(0)->getExprLoc(), FD, 0);
    EmitNonNullArgCheck(RValue::get(Src.first), E->getArg(1)->getType(),
                        E->getArg(1)->getExprLoc(), FD, 1);
    Builder.CreateMemCpy(Dest.first, Src.first, SizeVal, Align, false);
    return RValue::get(Dest.first);
  }

  case Builtin::BI__builtin___memcpy_chk: {
    // fold __builtin_memcpy_chk(x, y, cst1, cst2) to memcpy iff cst1<=cst2.
    llvm::APSInt Size, DstSize;
    if (!E->getArg(2)->EvaluateAsInt(Size, CGM.getContext()) ||
        !E->getArg(3)->EvaluateAsInt(DstSize, CGM.getContext()))
      break;
    if (Size.ugt(DstSize))
      break;
    std::pair<llvm::Value*, unsigned> Dest =
        EmitPointerWithAlignment(E->getArg(0));
    std::pair<llvm::Value*, unsigned> Src =
        EmitPointerWithAlignment(E->getArg(1));
    Value *SizeVal = llvm::ConstantInt::get(Builder.getContext(), Size);
    unsigned Align = std::min(Dest.second, Src.second);
    Builder.CreateMemCpy(Dest.first, Src.first, SizeVal, Align, false);
    return RValue::get(Dest.first);
  }

  case Builtin::BI__builtin_objc_memmove_collectable: {
    Value *Address = EmitScalarExpr(E->getArg(0));
    Value *SrcAddr = EmitScalarExpr(E->getArg(1));
    Value *SizeVal = EmitScalarExpr(E->getArg(2));
    CGM.getObjCRuntime().EmitGCMemmoveCollectable(*this,
                                                  Address, SrcAddr, SizeVal);
    return RValue::get(Address);
  }

  case Builtin::BI__builtin___memmove_chk: {
    // fold __builtin_memmove_chk(x, y, cst1, cst2) to memmove iff cst1<=cst2.
    llvm::APSInt Size, DstSize;
    if (!E->getArg(2)->EvaluateAsInt(Size, CGM.getContext()) ||
        !E->getArg(3)->EvaluateAsInt(DstSize, CGM.getContext()))
      break;
    if (Size.ugt(DstSize))
      break;
    std::pair<llvm::Value*, unsigned> Dest =
        EmitPointerWithAlignment(E->getArg(0));
    std::pair<llvm::Value*, unsigned> Src =
        EmitPointerWithAlignment(E->getArg(1));
    Value *SizeVal = llvm::ConstantInt::get(Builder.getContext(), Size);
    unsigned Align = std::min(Dest.second, Src.second);
    Builder.CreateMemMove(Dest.first, Src.first, SizeVal, Align, false);
    return RValue::get(Dest.first);
  }

  case Builtin::BImemmove:
  case Builtin::BI__builtin_memmove: {
    std::pair<llvm::Value*, unsigned> Dest =
        EmitPointerWithAlignment(E->getArg(0));
    std::pair<llvm::Value*, unsigned> Src =
        EmitPointerWithAlignment(E->getArg(1));
    Value *SizeVal = EmitScalarExpr(E->getArg(2));
    unsigned Align = std::min(Dest.second, Src.second);
    EmitNonNullArgCheck(RValue::get(Dest.first), E->getArg(0)->getType(),
                        E->getArg(0)->getExprLoc(), FD, 0);
    EmitNonNullArgCheck(RValue::get(Src.first), E->getArg(1)->getType(),
                        E->getArg(1)->getExprLoc(), FD, 1);
    Builder.CreateMemMove(Dest.first, Src.first, SizeVal, Align, false);
    return RValue::get(Dest.first);
  }
  case Builtin::BImemset:
  case Builtin::BI__builtin_memset: {
    std::pair<llvm::Value*, unsigned> Dest =
        EmitPointerWithAlignment(E->getArg(0));
    Value *ByteVal = Builder.CreateTrunc(EmitScalarExpr(E->getArg(1)),
                                         Builder.getInt8Ty());
    Value *SizeVal = EmitScalarExpr(E->getArg(2));
    EmitNonNullArgCheck(RValue::get(Dest.first), E->getArg(0)->getType(),
                        E->getArg(0)->getExprLoc(), FD, 0);
    Builder.CreateMemSet(Dest.first, ByteVal, SizeVal, Dest.second, false);
    return RValue::get(Dest.first);
  }
  case Builtin::BI__builtin___memset_chk: {
    // fold __builtin_memset_chk(x, y, cst1, cst2) to memset iff cst1<=cst2.
    llvm::APSInt Size, DstSize;
    if (!E->getArg(2)->EvaluateAsInt(Size, CGM.getContext()) ||
        !E->getArg(3)->EvaluateAsInt(DstSize, CGM.getContext()))
      break;
    if (Size.ugt(DstSize))
      break;
    std::pair<llvm::Value*, unsigned> Dest =
        EmitPointerWithAlignment(E->getArg(0));
    Value *ByteVal = Builder.CreateTrunc(EmitScalarExpr(E->getArg(1)),
                                         Builder.getInt8Ty());
    Value *SizeVal = llvm::ConstantInt::get(Builder.getContext(), Size);
    Builder.CreateMemSet(Dest.first, ByteVal, SizeVal, Dest.second, false);
    return RValue::get(Dest.first);
  }
  case Builtin::BI__builtin_dwarf_cfa: {
    // The offset in bytes from the first argument to the CFA.
    //
    // Why on earth is this in the frontend?  Is there any reason at
    // all that the backend can't reasonably determine this while
    // lowering llvm.eh.dwarf.cfa()?
    //
    // TODO: If there's a satisfactory reason, add a target hook for
    // this instead of hard-coding 0, which is correct for most targets.
    int32_t Offset = 0;

    Value *F = CGM.getIntrinsic(Intrinsic::eh_dwarf_cfa);
    return RValue::get(Builder.CreateCall(F,
                                      llvm::ConstantInt::get(Int32Ty, Offset)));
  }
  case Builtin::BI__builtin_return_address: {
    Value *Depth =
        CGM.EmitConstantExpr(E->getArg(0), getContext().UnsignedIntTy, this);
    Value *F = CGM.getIntrinsic(Intrinsic::returnaddress);
    return RValue::get(Builder.CreateCall(F, Depth));
  }
  case Builtin::BI__builtin_frame_address: {
    Value *Depth =
        CGM.EmitConstantExpr(E->getArg(0), getContext().UnsignedIntTy, this);
    Value *F = CGM.getIntrinsic(Intrinsic::frameaddress);
    return RValue::get(Builder.CreateCall(F, Depth));
  }
  case Builtin::BI__builtin_extract_return_addr: {
    Value *Address = EmitScalarExpr(E->getArg(0));
    Value *Result = getTargetHooks().decodeReturnAddress(*this, Address);
    return RValue::get(Result);
  }
  case Builtin::BI__builtin_frob_return_addr: {
    Value *Address = EmitScalarExpr(E->getArg(0));
    Value *Result = getTargetHooks().encodeReturnAddress(*this, Address);
    return RValue::get(Result);
  }
  case Builtin::BI__builtin_dwarf_sp_column: {
    llvm::IntegerType *Ty
      = cast<llvm::IntegerType>(ConvertType(E->getType()));
    int Column = getTargetHooks().getDwarfEHStackPointer(CGM);
    if (Column == -1) {
      CGM.ErrorUnsupported(E, "__builtin_dwarf_sp_column");
      return RValue::get(llvm::UndefValue::get(Ty));
    }
    return RValue::get(llvm::ConstantInt::get(Ty, Column, true));
  }
  case Builtin::BI__builtin_init_dwarf_reg_size_table: {
    Value *Address = EmitScalarExpr(E->getArg(0));
    if (getTargetHooks().initDwarfEHRegSizeTable(*this, Address))
      CGM.ErrorUnsupported(E, "__builtin_init_dwarf_reg_size_table");
    return RValue::get(llvm::UndefValue::get(ConvertType(E->getType())));
  }
  case Builtin::BI__builtin_eh_return: {
    Value *Int = EmitScalarExpr(E->getArg(0));
    Value *Ptr = EmitScalarExpr(E->getArg(1));

    llvm::IntegerType *IntTy = cast<llvm::IntegerType>(Int->getType());
    assert((IntTy->getBitWidth() == 32 || IntTy->getBitWidth() == 64) &&
           "LLVM's __builtin_eh_return only supports 32- and 64-bit variants");
    Value *F = CGM.getIntrinsic(IntTy->getBitWidth() == 32
                                  ? Intrinsic::eh_return_i32
                                  : Intrinsic::eh_return_i64);
    Builder.CreateCall(F, {Int, Ptr});
    Builder.CreateUnreachable();

    // We do need to preserve an insertion point.
    EmitBlock(createBasicBlock("builtin_eh_return.cont"));

    return RValue::get(nullptr);
  }
  case Builtin::BI__builtin_unwind_init: {
    Value *F = CGM.getIntrinsic(Intrinsic::eh_unwind_init);
    return RValue::get(Builder.CreateCall(F));
  }
  case Builtin::BI__builtin_extend_pointer: {
    // Extends a pointer to the size of an _Unwind_Word, which is
    // uint64_t on all platforms.  Generally this gets poked into a
    // register and eventually used as an address, so if the
    // addressing registers are wider than pointers and the platform
    // doesn't implicitly ignore high-order bits when doing
    // addressing, we need to make sure we zext / sext based on
    // the platform's expectations.
    //
    // See: http://gcc.gnu.org/ml/gcc-bugs/2002-02/msg00237.html

    // Cast the pointer to intptr_t.
    Value *Ptr = EmitScalarExpr(E->getArg(0));
    Value *Result = Builder.CreatePtrToInt(Ptr, IntPtrTy, "extend.cast");

    // If that's 64 bits, we're done.
    if (IntPtrTy->getBitWidth() == 64)
      return RValue::get(Result);

    // Otherwise, ask the codegen data what to do.
    if (getTargetHooks().extendPointerWithSExt())
      return RValue::get(Builder.CreateSExt(Result, Int64Ty, "extend.sext"));
    else
      return RValue::get(Builder.CreateZExt(Result, Int64Ty, "extend.zext"));
  }
  case Builtin::BI__builtin_setjmp: {
    // Buffer is a void**.
    Value *Buf = EmitScalarExpr(E->getArg(0));

    // Store the frame pointer to the setjmp buffer.
    Value *FrameAddr =
      Builder.CreateCall(CGM.getIntrinsic(Intrinsic::frameaddress),
                         ConstantInt::get(Int32Ty, 0));
    Builder.CreateStore(FrameAddr, Buf);

    // Store the stack pointer to the setjmp buffer.
    Value *StackAddr =
        Builder.CreateCall(CGM.getIntrinsic(Intrinsic::stacksave));
    Value *StackSaveSlot =
      Builder.CreateGEP(Buf, ConstantInt::get(Int32Ty, 2));
    Builder.CreateStore(StackAddr, StackSaveSlot);

    // Call LLVM's EH setjmp, which is lightweight.
    Value *F = CGM.getIntrinsic(Intrinsic::eh_sjlj_setjmp);
    Buf = Builder.CreateBitCast(Buf, Int8PtrTy);
    return RValue::get(Builder.CreateCall(F, Buf));
  }
  case Builtin::BI__builtin_longjmp: {
    Value *Buf = EmitScalarExpr(E->getArg(0));
    Buf = Builder.CreateBitCast(Buf, Int8PtrTy);

    // Call LLVM's EH longjmp, which is lightweight.
    Builder.CreateCall(CGM.getIntrinsic(Intrinsic::eh_sjlj_longjmp), Buf);

    // longjmp doesn't return; mark this as unreachable.
    Builder.CreateUnreachable();

    // We do need to preserve an insertion point.
    EmitBlock(createBasicBlock("longjmp.cont"));

    return RValue::get(nullptr);
  }
  case Builtin::BI__sync_fetch_and_add:
  case Builtin::BI__sync_fetch_and_sub:
  case Builtin::BI__sync_fetch_and_or:
  case Builtin::BI__sync_fetch_and_and:
  case Builtin::BI__sync_fetch_and_xor:
  case Builtin::BI__sync_fetch_and_nand:
  case Builtin::BI__sync_add_and_fetch:
  case Builtin::BI__sync_sub_and_fetch:
  case Builtin::BI__sync_and_and_fetch:
  case Builtin::BI__sync_or_and_fetch:
  case Builtin::BI__sync_xor_and_fetch:
  case Builtin::BI__sync_nand_and_fetch:
  case Builtin::BI__sync_val_compare_and_swap:
  case Builtin::BI__sync_bool_compare_and_swap:
  case Builtin::BI__sync_lock_test_and_set:
  case Builtin::BI__sync_lock_release:
  case Builtin::BI__sync_swap:
    llvm_unreachable("Shouldn't make it through sema");
  case Builtin::BI__sync_fetch_and_add_1:
  case Builtin::BI__sync_fetch_and_add_2:
  case Builtin::BI__sync_fetch_and_add_4:
  case Builtin::BI__sync_fetch_and_add_8:
  case Builtin::BI__sync_fetch_and_add_16:
    return EmitBinaryAtomic(*this, llvm::AtomicRMWInst::Add, E);
  case Builtin::BI__sync_fetch_and_sub_1:
  case Builtin::BI__sync_fetch_and_sub_2:
  case Builtin::BI__sync_fetch_and_sub_4:
  case Builtin::BI__sync_fetch_and_sub_8:
  case Builtin::BI__sync_fetch_and_sub_16:
    return EmitBinaryAtomic(*this, llvm::AtomicRMWInst::Sub, E);
  case Builtin::BI__sync_fetch_and_or_1:
  case Builtin::BI__sync_fetch_and_or_2:
  case Builtin::BI__sync_fetch_and_or_4:
  case Builtin::BI__sync_fetch_and_or_8:
  case Builtin::BI__sync_fetch_and_or_16:
    return EmitBinaryAtomic(*this, llvm::AtomicRMWInst::Or, E);
  case Builtin::BI__sync_fetch_and_and_1:
  case Builtin::BI__sync_fetch_and_and_2:
  case Builtin::BI__sync_fetch_and_and_4:
  case Builtin::BI__sync_fetch_and_and_8:
  case Builtin::BI__sync_fetch_and_and_16:
    return EmitBinaryAtomic(*this, llvm::AtomicRMWInst::And, E);
  case Builtin::BI__sync_fetch_and_xor_1:
  case Builtin::BI__sync_fetch_and_xor_2:
  case Builtin::BI__sync_fetch_and_xor_4:
  case Builtin::BI__sync_fetch_and_xor_8:
  case Builtin::BI__sync_fetch_and_xor_16:
    return EmitBinaryAtomic(*this, llvm::AtomicRMWInst::Xor, E);
  case Builtin::BI__sync_fetch_and_nand_1:
  case Builtin::BI__sync_fetch_and_nand_2:
  case Builtin::BI__sync_fetch_and_nand_4:
  case Builtin::BI__sync_fetch_and_nand_8:
  case Builtin::BI__sync_fetch_and_nand_16:
    return EmitBinaryAtomic(*this, llvm::AtomicRMWInst::Nand, E);

  // Clang extensions: not overloaded yet.
  case Builtin::BI__sync_fetch_and_min:
    return EmitBinaryAtomic(*this, llvm::AtomicRMWInst::Min, E);
  case Builtin::BI__sync_fetch_and_max:
    return EmitBinaryAtomic(*this, llvm::AtomicRMWInst::Max, E);
  case Builtin::BI__sync_fetch_and_umin:
    return EmitBinaryAtomic(*this, llvm::AtomicRMWInst::UMin, E);
  case Builtin::BI__sync_fetch_and_umax:
    return EmitBinaryAtomic(*this, llvm::AtomicRMWInst::UMax, E);

  case Builtin::BI__sync_add_and_fetch_1:
  case Builtin::BI__sync_add_and_fetch_2:
  case Builtin::BI__sync_add_and_fetch_4:
  case Builtin::BI__sync_add_and_fetch_8:
  case Builtin::BI__sync_add_and_fetch_16:
    return EmitBinaryAtomicPost(*this, llvm::AtomicRMWInst::Add, E,
                                llvm::Instruction::Add);
  case Builtin::BI__sync_sub_and_fetch_1:
  case Builtin::BI__sync_sub_and_fetch_2:
  case Builtin::BI__sync_sub_and_fetch_4:
  case Builtin::BI__sync_sub_and_fetch_8:
  case Builtin::BI__sync_sub_and_fetch_16:
    return EmitBinaryAtomicPost(*this, llvm::AtomicRMWInst::Sub, E,
                                llvm::Instruction::Sub);
  case Builtin::BI__sync_and_and_fetch_1:
  case Builtin::BI__sync_and_and_fetch_2:
  case Builtin::BI__sync_and_and_fetch_4:
  case Builtin::BI__sync_and_and_fetch_8:
  case Builtin::BI__sync_and_and_fetch_16:
    return EmitBinaryAtomicPost(*this, llvm::AtomicRMWInst::And, E,
                                llvm::Instruction::And);
  case Builtin::BI__sync_or_and_fetch_1:
  case Builtin::BI__sync_or_and_fetch_2:
  case Builtin::BI__sync_or_and_fetch_4:
  case Builtin::BI__sync_or_and_fetch_8:
  case Builtin::BI__sync_or_and_fetch_16:
    return EmitBinaryAtomicPost(*this, llvm::AtomicRMWInst::Or, E,
                                llvm::Instruction::Or);
  case Builtin::BI__sync_xor_and_fetch_1:
  case Builtin::BI__sync_xor_and_fetch_2:
  case Builtin::BI__sync_xor_and_fetch_4:
  case Builtin::BI__sync_xor_and_fetch_8:
  case Builtin::BI__sync_xor_and_fetch_16:
    return EmitBinaryAtomicPost(*this, llvm::AtomicRMWInst::Xor, E,
                                llvm::Instruction::Xor);
  case Builtin::BI__sync_nand_and_fetch_1:
  case Builtin::BI__sync_nand_and_fetch_2:
  case Builtin::BI__sync_nand_and_fetch_4:
  case Builtin::BI__sync_nand_and_fetch_8:
  case Builtin::BI__sync_nand_and_fetch_16:
    return EmitBinaryAtomicPost(*this, llvm::AtomicRMWInst::Nand, E,
                                llvm::Instruction::And, true);

  case Builtin::BI__sync_val_compare_and_swap_1:
  case Builtin::BI__sync_val_compare_and_swap_2:
  case Builtin::BI__sync_val_compare_and_swap_4:
  case Builtin::BI__sync_val_compare_and_swap_8:
  case Builtin::BI__sync_val_compare_and_swap_16:
    return RValue::get(MakeAtomicCmpXchgValue(*this, E, false));

  case Builtin::BI__sync_bool_compare_and_swap_1:
  case Builtin::BI__sync_bool_compare_and_swap_2:
  case Builtin::BI__sync_bool_compare_and_swap_4:
  case Builtin::BI__sync_bool_compare_and_swap_8:
  case Builtin::BI__sync_bool_compare_and_swap_16:
    return RValue::get(MakeAtomicCmpXchgValue(*this, E, true));

  case Builtin::BI__sync_swap_1:
  case Builtin::BI__sync_swap_2:
  case Builtin::BI__sync_swap_4:
  case Builtin::BI__sync_swap_8:
  case Builtin::BI__sync_swap_16:
    return EmitBinaryAtomic(*this, llvm::AtomicRMWInst::Xchg, E);

  case Builtin::BI__sync_lock_test_and_set_1:
  case Builtin::BI__sync_lock_test_and_set_2:
  case Builtin::BI__sync_lock_test_and_set_4:
  case Builtin::BI__sync_lock_test_and_set_8:
  case Builtin::BI__sync_lock_test_and_set_16:
    return EmitBinaryAtomic(*this, llvm::AtomicRMWInst::Xchg, E);

  case Builtin::BI__sync_lock_release_1:
  case Builtin::BI__sync_lock_release_2:
  case Builtin::BI__sync_lock_release_4:
  case Builtin::BI__sync_lock_release_8:
  case Builtin::BI__sync_lock_release_16: {
    Value *Ptr = EmitScalarExpr(E->getArg(0));
    QualType ElTy = E->getArg(0)->getType()->getPointeeType();
    CharUnits StoreSize = getContext().getTypeSizeInChars(ElTy);
    llvm::Type *ITy = llvm::IntegerType::get(getLLVMContext(),
                                             StoreSize.getQuantity() * 8);
    Ptr = Builder.CreateBitCast(Ptr, ITy->getPointerTo());
    llvm::StoreInst *Store =
      Builder.CreateStore(llvm::Constant::getNullValue(ITy), Ptr);
    Store->setAlignment(StoreSize.getQuantity());
    Store->setAtomic(llvm::Release);
    return RValue::get(nullptr);
  }

  case Builtin::BI__sync_synchronize: {
    // We assume this is supposed to correspond to a C++0x-style
    // sequentially-consistent fence (i.e. this is only usable for
    // synchonization, not device I/O or anything like that). This intrinsic
    // is really badly designed in the sense that in theory, there isn't
    // any way to safely use it... but in practice, it mostly works
    // to use it with non-atomic loads and stores to get acquire/release
    // semantics.
    Builder.CreateFence(llvm::SequentiallyConsistent);
    return RValue::get(nullptr);
  }

  case Builtin::BI__c11_atomic_is_lock_free:
  case Builtin::BI__atomic_is_lock_free: {
    // Call "bool __atomic_is_lock_free(size_t size, void *ptr)". For the
    // __c11 builtin, ptr is 0 (indicating a properly-aligned object), since
    // _Atomic(T) is always properly-aligned.
    const char *LibCallName = "__atomic_is_lock_free";
    CallArgList Args;
    Args.add(RValue::get(EmitScalarExpr(E->getArg(0))),
             getContext().getSizeType());
    if (BuiltinID == Builtin::BI__atomic_is_lock_free)
      Args.add(RValue::get(EmitScalarExpr(E->getArg(1))),
               getContext().VoidPtrTy);
    else
      Args.add(RValue::get(llvm::Constant::getNullValue(VoidPtrTy)),
               getContext().VoidPtrTy);
    const CGFunctionInfo &FuncInfo =
        CGM.getTypes().arrangeFreeFunctionCall(E->getType(), Args,
                                               FunctionType::ExtInfo(),
                                               RequiredArgs::All);
    llvm::FunctionType *FTy = CGM.getTypes().GetFunctionType(FuncInfo);
    llvm::Constant *Func = CGM.CreateRuntimeFunction(FTy, LibCallName);
    return EmitCall(FuncInfo, Func, ReturnValueSlot(), Args);
  }

  case Builtin::BI__atomic_test_and_set: {
    // Look at the argument type to determine whether this is a volatile
    // operation. The parameter type is always volatile.
    QualType PtrTy = E->getArg(0)->IgnoreImpCasts()->getType();
    bool Volatile =
        PtrTy->castAs<PointerType>()->getPointeeType().isVolatileQualified();

    Value *Ptr = EmitScalarExpr(E->getArg(0));
    unsigned AddrSpace = Ptr->getType()->getPointerAddressSpace();
    Ptr = Builder.CreateBitCast(Ptr, Int8Ty->getPointerTo(AddrSpace));
    Value *NewVal = Builder.getInt8(1);
    Value *Order = EmitScalarExpr(E->getArg(1));
    if (isa<llvm::ConstantInt>(Order)) {
      int ord = cast<llvm::ConstantInt>(Order)->getZExtValue();
      AtomicRMWInst *Result = nullptr;
      switch (ord) {
      case 0:  // memory_order_relaxed
      default: // invalid order
        Result = Builder.CreateAtomicRMW(llvm::AtomicRMWInst::Xchg,
                                         Ptr, NewVal,
                                         llvm::Monotonic);
        break;
      case 1:  // memory_order_consume
      case 2:  // memory_order_acquire
        Result = Builder.CreateAtomicRMW(llvm::AtomicRMWInst::Xchg,
                                         Ptr, NewVal,
                                         llvm::Acquire);
        break;
      case 3:  // memory_order_release
        Result = Builder.CreateAtomicRMW(llvm::AtomicRMWInst::Xchg,
                                         Ptr, NewVal,
                                         llvm::Release);
        break;
      case 4:  // memory_order_acq_rel
        Result = Builder.CreateAtomicRMW(llvm::AtomicRMWInst::Xchg,
                                         Ptr, NewVal,
                                         llvm::AcquireRelease);
        break;
      case 5:  // memory_order_seq_cst
        Result = Builder.CreateAtomicRMW(llvm::AtomicRMWInst::Xchg,
                                         Ptr, NewVal,
                                         llvm::SequentiallyConsistent);
        break;
      }
      Result->setVolatile(Volatile);
      return RValue::get(Builder.CreateIsNotNull(Result, "tobool"));
    }

    llvm::BasicBlock *ContBB = createBasicBlock("atomic.continue", CurFn);

    llvm::BasicBlock *BBs[5] = {
      createBasicBlock("monotonic", CurFn),
      createBasicBlock("acquire", CurFn),
      createBasicBlock("release", CurFn),
      createBasicBlock("acqrel", CurFn),
      createBasicBlock("seqcst", CurFn)
    };
    llvm::AtomicOrdering Orders[5] = {
      llvm::Monotonic, llvm::Acquire, llvm::Release,
      llvm::AcquireRelease, llvm::SequentiallyConsistent
    };

    Order = Builder.CreateIntCast(Order, Builder.getInt32Ty(), false);
    llvm::SwitchInst *SI = Builder.CreateSwitch(Order, BBs[0]);

    Builder.SetInsertPoint(ContBB);
    PHINode *Result = Builder.CreatePHI(Int8Ty, 5, "was_set");

    for (unsigned i = 0; i < 5; ++i) {
      Builder.SetInsertPoint(BBs[i]);
      AtomicRMWInst *RMW = Builder.CreateAtomicRMW(llvm::AtomicRMWInst::Xchg,
                                                   Ptr, NewVal, Orders[i]);
      RMW->setVolatile(Volatile);
      Result->addIncoming(RMW, BBs[i]);
      Builder.CreateBr(ContBB);
    }

    SI->addCase(Builder.getInt32(0), BBs[0]);
    SI->addCase(Builder.getInt32(1), BBs[1]);
    SI->addCase(Builder.getInt32(2), BBs[1]);
    SI->addCase(Builder.getInt32(3), BBs[2]);
    SI->addCase(Builder.getInt32(4), BBs[3]);
    SI->addCase(Builder.getInt32(5), BBs[4]);

    Builder.SetInsertPoint(ContBB);
    return RValue::get(Builder.CreateIsNotNull(Result, "tobool"));
  }

  case Builtin::BI__atomic_clear: {
    QualType PtrTy = E->getArg(0)->IgnoreImpCasts()->getType();
    bool Volatile =
        PtrTy->castAs<PointerType>()->getPointeeType().isVolatileQualified();

    Value *Ptr = EmitScalarExpr(E->getArg(0));
    unsigned AddrSpace = Ptr->getType()->getPointerAddressSpace();
    Ptr = Builder.CreateBitCast(Ptr, Int8Ty->getPointerTo(AddrSpace));
    Value *NewVal = Builder.getInt8(0);
    Value *Order = EmitScalarExpr(E->getArg(1));
    if (isa<llvm::ConstantInt>(Order)) {
      int ord = cast<llvm::ConstantInt>(Order)->getZExtValue();
      StoreInst *Store = Builder.CreateStore(NewVal, Ptr, Volatile);
      Store->setAlignment(1);
      switch (ord) {
      case 0:  // memory_order_relaxed
      default: // invalid order
        Store->setOrdering(llvm::Monotonic);
        break;
      case 3:  // memory_order_release
        Store->setOrdering(llvm::Release);
        break;
      case 5:  // memory_order_seq_cst
        Store->setOrdering(llvm::SequentiallyConsistent);
        break;
      }
      return RValue::get(nullptr);
    }

    llvm::BasicBlock *ContBB = createBasicBlock("atomic.continue", CurFn);

    llvm::BasicBlock *BBs[3] = {
      createBasicBlock("monotonic", CurFn),
      createBasicBlock("release", CurFn),
      createBasicBlock("seqcst", CurFn)
    };
    llvm::AtomicOrdering Orders[3] = {
      llvm::Monotonic, llvm::Release, llvm::SequentiallyConsistent
    };

    Order = Builder.CreateIntCast(Order, Builder.getInt32Ty(), false);
    llvm::SwitchInst *SI = Builder.CreateSwitch(Order, BBs[0]);

    for (unsigned i = 0; i < 3; ++i) {
      Builder.SetInsertPoint(BBs[i]);
      StoreInst *Store = Builder.CreateStore(NewVal, Ptr, Volatile);
      Store->setAlignment(1);
      Store->setOrdering(Orders[i]);
      Builder.CreateBr(ContBB);
    }

    SI->addCase(Builder.getInt32(0), BBs[0]);
    SI->addCase(Builder.getInt32(3), BBs[1]);
    SI->addCase(Builder.getInt32(5), BBs[2]);

    Builder.SetInsertPoint(ContBB);
    return RValue::get(nullptr);
  }

  case Builtin::BI__atomic_thread_fence:
  case Builtin::BI__atomic_signal_fence:
  case Builtin::BI__c11_atomic_thread_fence:
  case Builtin::BI__c11_atomic_signal_fence: {
    llvm::SynchronizationScope Scope;
    if (BuiltinID == Builtin::BI__atomic_signal_fence ||
        BuiltinID == Builtin::BI__c11_atomic_signal_fence)
      Scope = llvm::SingleThread;
    else
      Scope = llvm::CrossThread;
    Value *Order = EmitScalarExpr(E->getArg(0));
    if (isa<llvm::ConstantInt>(Order)) {
      int ord = cast<llvm::ConstantInt>(Order)->getZExtValue();
      switch (ord) {
      case 0:  // memory_order_relaxed
      default: // invalid order
        break;
      case 1:  // memory_order_consume
      case 2:  // memory_order_acquire
        Builder.CreateFence(llvm::Acquire, Scope);
        break;
      case 3:  // memory_order_release
        Builder.CreateFence(llvm::Release, Scope);
        break;
      case 4:  // memory_order_acq_rel
        Builder.CreateFence(llvm::AcquireRelease, Scope);
        break;
      case 5:  // memory_order_seq_cst
        Builder.CreateFence(llvm::SequentiallyConsistent, Scope);
        break;
      }
      return RValue::get(nullptr);
    }

    llvm::BasicBlock *AcquireBB, *ReleaseBB, *AcqRelBB, *SeqCstBB;
    AcquireBB = createBasicBlock("acquire", CurFn);
    ReleaseBB = createBasicBlock("release", CurFn);
    AcqRelBB = createBasicBlock("acqrel", CurFn);
    SeqCstBB = createBasicBlock("seqcst", CurFn);
    llvm::BasicBlock *ContBB = createBasicBlock("atomic.continue", CurFn);

    Order = Builder.CreateIntCast(Order, Builder.getInt32Ty(), false);
    llvm::SwitchInst *SI = Builder.CreateSwitch(Order, ContBB);

    Builder.SetInsertPoint(AcquireBB);
    Builder.CreateFence(llvm::Acquire, Scope);
    Builder.CreateBr(ContBB);
    SI->addCase(Builder.getInt32(1), AcquireBB);
    SI->addCase(Builder.getInt32(2), AcquireBB);

    Builder.SetInsertPoint(ReleaseBB);
    Builder.CreateFence(llvm::Release, Scope);
    Builder.CreateBr(ContBB);
    SI->addCase(Builder.getInt32(3), ReleaseBB);

    Builder.SetInsertPoint(AcqRelBB);
    Builder.CreateFence(llvm::AcquireRelease, Scope);
    Builder.CreateBr(ContBB);
    SI->addCase(Builder.getInt32(4), AcqRelBB);

    Builder.SetInsertPoint(SeqCstBB);
    Builder.CreateFence(llvm::SequentiallyConsistent, Scope);
    Builder.CreateBr(ContBB);
    SI->addCase(Builder.getInt32(5), SeqCstBB);

    Builder.SetInsertPoint(ContBB);
    return RValue::get(nullptr);
  }

    // Library functions with special handling.
  case Builtin::BIsqrt:
  case Builtin::BIsqrtf:
  case Builtin::BIsqrtl: {
    // Transform a call to sqrt* into a @llvm.sqrt.* intrinsic call, but only
    // in finite- or unsafe-math mode (the intrinsic has different semantics
    // for handling negative numbers compared to the library function, so
    // -fmath-errno=0 is not enough).
    if (!FD->hasAttr<ConstAttr>())
      break;
    if (!(CGM.getCodeGenOpts().UnsafeFPMath ||
          CGM.getCodeGenOpts().NoNaNsFPMath))
      break;
    Value *Arg0 = EmitScalarExpr(E->getArg(0));
    llvm::Type *ArgType = Arg0->getType();
    Value *F = CGM.getIntrinsic(Intrinsic::sqrt, ArgType);
    return RValue::get(Builder.CreateCall(F, Arg0));
  }

  case Builtin::BI__builtin_pow:
  case Builtin::BI__builtin_powf:
  case Builtin::BI__builtin_powl:
  case Builtin::BIpow:
  case Builtin::BIpowf:
  case Builtin::BIpowl: {
    // Transform a call to pow* into a @llvm.pow.* intrinsic call.
    if (!FD->hasAttr<ConstAttr>())
      break;
    Value *Base = EmitScalarExpr(E->getArg(0));
    Value *Exponent = EmitScalarExpr(E->getArg(1));
    llvm::Type *ArgType = Base->getType();
    Value *F = CGM.getIntrinsic(Intrinsic::pow, ArgType);
    return RValue::get(Builder.CreateCall(F, {Base, Exponent}));
  }

  case Builtin::BIfma:
  case Builtin::BIfmaf:
  case Builtin::BIfmal:
  case Builtin::BI__builtin_fma:
  case Builtin::BI__builtin_fmaf:
  case Builtin::BI__builtin_fmal: {
    // Rewrite fma to intrinsic.
    Value *FirstArg = EmitScalarExpr(E->getArg(0));
    llvm::Type *ArgType = FirstArg->getType();
    Value *F = CGM.getIntrinsic(Intrinsic::fma, ArgType);
    return RValue::get(
        Builder.CreateCall(F, {FirstArg, EmitScalarExpr(E->getArg(1)),
                               EmitScalarExpr(E->getArg(2))}));
  }

  case Builtin::BI__builtin_signbit:
  case Builtin::BI__builtin_signbitf:
  case Builtin::BI__builtin_signbitl: {
    return RValue::get(
        Builder.CreateZExt(EmitSignBit(*this, EmitScalarExpr(E->getArg(0))),
                           ConvertType(E->getType())));
  }
  case Builtin::BI__builtin_annotation: {
    llvm::Value *AnnVal = EmitScalarExpr(E->getArg(0));
    llvm::Value *F = CGM.getIntrinsic(llvm::Intrinsic::annotation,
                                      AnnVal->getType());

    // Get the annotation string, go through casts. Sema requires this to be a
    // non-wide string literal, potentially casted, so the cast<> is safe.
    const Expr *AnnotationStrExpr = E->getArg(1)->IgnoreParenCasts();
    StringRef Str = cast<StringLiteral>(AnnotationStrExpr)->getString();
    return RValue::get(EmitAnnotationCall(F, AnnVal, Str, E->getExprLoc()));
  }
  case Builtin::BI__builtin_addcb:
  case Builtin::BI__builtin_addcs:
  case Builtin::BI__builtin_addc:
  case Builtin::BI__builtin_addcl:
  case Builtin::BI__builtin_addcll:
  case Builtin::BI__builtin_subcb:
  case Builtin::BI__builtin_subcs:
  case Builtin::BI__builtin_subc:
  case Builtin::BI__builtin_subcl:
  case Builtin::BI__builtin_subcll: {

    // We translate all of these builtins from expressions of the form:
    //   int x = ..., y = ..., carryin = ..., carryout, result;
    //   result = __builtin_addc(x, y, carryin, &carryout);
    //
    // to LLVM IR of the form:
    //
    //   %tmp1 = call {i32, i1} @llvm.uadd.with.overflow.i32(i32 %x, i32 %y)
    //   %tmpsum1 = extractvalue {i32, i1} %tmp1, 0
    //   %carry1 = extractvalue {i32, i1} %tmp1, 1
    //   %tmp2 = call {i32, i1} @llvm.uadd.with.overflow.i32(i32 %tmpsum1,
    //                                                       i32 %carryin)
    //   %result = extractvalue {i32, i1} %tmp2, 0
    //   %carry2 = extractvalue {i32, i1} %tmp2, 1
    //   %tmp3 = or i1 %carry1, %carry2
    //   %tmp4 = zext i1 %tmp3 to i32
    //   store i32 %tmp4, i32* %carryout

    // Scalarize our inputs.
    llvm::Value *X = EmitScalarExpr(E->getArg(0));
    llvm::Value *Y = EmitScalarExpr(E->getArg(1));
    llvm::Value *Carryin = EmitScalarExpr(E->getArg(2));
    std::pair<llvm::Value*, unsigned> CarryOutPtr =
      EmitPointerWithAlignment(E->getArg(3));

    // Decide if we are lowering to a uadd.with.overflow or usub.with.overflow.
    llvm::Intrinsic::ID IntrinsicId;
    switch (BuiltinID) {
    default: llvm_unreachable("Unknown multiprecision builtin id.");
    case Builtin::BI__builtin_addcb:
    case Builtin::BI__builtin_addcs:
    case Builtin::BI__builtin_addc:
    case Builtin::BI__builtin_addcl:
    case Builtin::BI__builtin_addcll:
      IntrinsicId = llvm::Intrinsic::uadd_with_overflow;
      break;
    case Builtin::BI__builtin_subcb:
    case Builtin::BI__builtin_subcs:
    case Builtin::BI__builtin_subc:
    case Builtin::BI__builtin_subcl:
    case Builtin::BI__builtin_subcll:
      IntrinsicId = llvm::Intrinsic::usub_with_overflow;
      break;
    }

    // Construct our resulting LLVM IR expression.
    llvm::Value *Carry1;
    llvm::Value *Sum1 = EmitOverflowIntrinsic(*this, IntrinsicId,
                                              X, Y, Carry1);
    llvm::Value *Carry2;
    llvm::Value *Sum2 = EmitOverflowIntrinsic(*this, IntrinsicId,
                                              Sum1, Carryin, Carry2);
    llvm::Value *CarryOut = Builder.CreateZExt(Builder.CreateOr(Carry1, Carry2),
                                               X->getType());
    llvm::StoreInst *CarryOutStore = Builder.CreateStore(CarryOut,
                                                         CarryOutPtr.first);
    CarryOutStore->setAlignment(CarryOutPtr.second);
    return RValue::get(Sum2);
  }
  case Builtin::BI__builtin_uadd_overflow:
  case Builtin::BI__builtin_uaddl_overflow:
  case Builtin::BI__builtin_uaddll_overflow:
  case Builtin::BI__builtin_usub_overflow:
  case Builtin::BI__builtin_usubl_overflow:
  case Builtin::BI__builtin_usubll_overflow:
  case Builtin::BI__builtin_umul_overflow:
  case Builtin::BI__builtin_umull_overflow:
  case Builtin::BI__builtin_umulll_overflow:
  case Builtin::BI__builtin_sadd_overflow:
  case Builtin::BI__builtin_saddl_overflow:
  case Builtin::BI__builtin_saddll_overflow:
  case Builtin::BI__builtin_ssub_overflow:
  case Builtin::BI__builtin_ssubl_overflow:
  case Builtin::BI__builtin_ssubll_overflow:
  case Builtin::BI__builtin_smul_overflow:
  case Builtin::BI__builtin_smull_overflow:
  case Builtin::BI__builtin_smulll_overflow: {

    // We translate all of these builtins directly to the relevant llvm IR node.

    // Scalarize our inputs.
    llvm::Value *X = EmitScalarExpr(E->getArg(0));
    llvm::Value *Y = EmitScalarExpr(E->getArg(1));
    std::pair<llvm::Value *, unsigned> SumOutPtr =
      EmitPointerWithAlignment(E->getArg(2));

    // Decide which of the overflow intrinsics we are lowering to:
    llvm::Intrinsic::ID IntrinsicId;
    switch (BuiltinID) {
    default: llvm_unreachable("Unknown security overflow builtin id.");
    case Builtin::BI__builtin_uadd_overflow:
    case Builtin::BI__builtin_uaddl_overflow:
    case Builtin::BI__builtin_uaddll_overflow:
      IntrinsicId = llvm::Intrinsic::uadd_with_overflow;
      break;
    case Builtin::BI__builtin_usub_overflow:
    case Builtin::BI__builtin_usubl_overflow:
    case Builtin::BI__builtin_usubll_overflow:
      IntrinsicId = llvm::Intrinsic::usub_with_overflow;
      break;
    case Builtin::BI__builtin_umul_overflow:
    case Builtin::BI__builtin_umull_overflow:
    case Builtin::BI__builtin_umulll_overflow:
      IntrinsicId = llvm::Intrinsic::umul_with_overflow;
      break;
    case Builtin::BI__builtin_sadd_overflow:
    case Builtin::BI__builtin_saddl_overflow:
    case Builtin::BI__builtin_saddll_overflow:
      IntrinsicId = llvm::Intrinsic::sadd_with_overflow;
      break;
    case Builtin::BI__builtin_ssub_overflow:
    case Builtin::BI__builtin_ssubl_overflow:
    case Builtin::BI__builtin_ssubll_overflow:
      IntrinsicId = llvm::Intrinsic::ssub_with_overflow;
      break;
    case Builtin::BI__builtin_smul_overflow:
    case Builtin::BI__builtin_smull_overflow:
    case Builtin::BI__builtin_smulll_overflow:
      IntrinsicId = llvm::Intrinsic::smul_with_overflow;
      break;
    }

    
    llvm::Value *Carry;
    llvm::Value *Sum = EmitOverflowIntrinsic(*this, IntrinsicId, X, Y, Carry);
    llvm::StoreInst *SumOutStore = Builder.CreateStore(Sum, SumOutPtr.first);
    SumOutStore->setAlignment(SumOutPtr.second);

    return RValue::get(Carry);
  }
  case Builtin::BI__builtin_addressof:
    return RValue::get(EmitLValue(E->getArg(0)).getAddress());
  case Builtin::BI__builtin_operator_new:
    return EmitBuiltinNewDeleteCall(FD->getType()->castAs<FunctionProtoType>(),
                                    E->getArg(0), false);
  case Builtin::BI__builtin_operator_delete:
    return EmitBuiltinNewDeleteCall(FD->getType()->castAs<FunctionProtoType>(),
                                    E->getArg(0), true);
  case Builtin::BI__noop:
    // __noop always evaluates to an integer literal zero.
    return RValue::get(ConstantInt::get(IntTy, 0));
  case Builtin::BI__builtin_call_with_static_chain: {
    const CallExpr *Call = cast<CallExpr>(E->getArg(0));
    const Expr *Chain = E->getArg(1);
    return EmitCall(Call->getCallee()->getType(),
                    EmitScalarExpr(Call->getCallee()), Call, ReturnValue,
                    Call->getCalleeDecl(), EmitScalarExpr(Chain));
  }
  case Builtin::BI_InterlockedExchange:
  case Builtin::BI_InterlockedExchangePointer:
    return EmitBinaryAtomic(*this, llvm::AtomicRMWInst::Xchg, E);
  case Builtin::BI_InterlockedCompareExchangePointer: {
    llvm::Type *RTy;
    llvm::IntegerType *IntType =
      IntegerType::get(getLLVMContext(),
                       getContext().getTypeSize(E->getType()));
    llvm::Type *IntPtrType = IntType->getPointerTo();

    llvm::Value *Destination =
      Builder.CreateBitCast(EmitScalarExpr(E->getArg(0)), IntPtrType);

    llvm::Value *Exchange = EmitScalarExpr(E->getArg(1));
    RTy = Exchange->getType();
    Exchange = Builder.CreatePtrToInt(Exchange, IntType);

    llvm::Value *Comparand =
      Builder.CreatePtrToInt(EmitScalarExpr(E->getArg(2)), IntType);

    auto Result = Builder.CreateAtomicCmpXchg(Destination, Comparand, Exchange,
                                              SequentiallyConsistent,
                                              SequentiallyConsistent);
    Result->setVolatile(true);

    return RValue::get(Builder.CreateIntToPtr(Builder.CreateExtractValue(Result,
                                                                         0),
                                              RTy));
  }
  case Builtin::BI_InterlockedCompareExchange: {
    AtomicCmpXchgInst *CXI = Builder.CreateAtomicCmpXchg(
        EmitScalarExpr(E->getArg(0)),
        EmitScalarExpr(E->getArg(2)),
        EmitScalarExpr(E->getArg(1)),
        SequentiallyConsistent,
        SequentiallyConsistent);
      CXI->setVolatile(true);
      return RValue::get(Builder.CreateExtractValue(CXI, 0));
  }
  case Builtin::BI_InterlockedIncrement: {
    AtomicRMWInst *RMWI = Builder.CreateAtomicRMW(
      AtomicRMWInst::Add,
      EmitScalarExpr(E->getArg(0)),
      ConstantInt::get(Int32Ty, 1),
      llvm::SequentiallyConsistent);
    RMWI->setVolatile(true);
    return RValue::get(Builder.CreateAdd(RMWI, ConstantInt::get(Int32Ty, 1)));
  }
  case Builtin::BI_InterlockedDecrement: {
    AtomicRMWInst *RMWI = Builder.CreateAtomicRMW(
      AtomicRMWInst::Sub,
      EmitScalarExpr(E->getArg(0)),
      ConstantInt::get(Int32Ty, 1),
      llvm::SequentiallyConsistent);
    RMWI->setVolatile(true);
    return RValue::get(Builder.CreateSub(RMWI, ConstantInt::get(Int32Ty, 1)));
  }
  case Builtin::BI_InterlockedExchangeAdd: {
    AtomicRMWInst *RMWI = Builder.CreateAtomicRMW(
      AtomicRMWInst::Add,
      EmitScalarExpr(E->getArg(0)),
      EmitScalarExpr(E->getArg(1)),
      llvm::SequentiallyConsistent);
    RMWI->setVolatile(true);
    return RValue::get(RMWI);
  }
  case Builtin::BI__readfsdword: {
    Value *IntToPtr =
      Builder.CreateIntToPtr(EmitScalarExpr(E->getArg(0)),
                             llvm::PointerType::get(CGM.Int32Ty, 257));
    LoadInst *Load =
        Builder.CreateAlignedLoad(IntToPtr, /*Align=*/4, /*isVolatile=*/true);
    return RValue::get(Load);
  }

#if 0 // HLSL Change Start - no support for exception handling
  case Builtin::BI__exception_code:
  case Builtin::BI_exception_code:
    return RValue::get(EmitSEHExceptionCode());
  case Builtin::BI__exception_info:
  case Builtin::BI_exception_info:
    return RValue::get(EmitSEHExceptionInfo());
  case Builtin::BI__abnormal_termination:
  case Builtin::BI_abnormal_termination:
    return RValue::get(EmitSEHAbnormalTermination());
  case Builtin::BI_setjmpex: {
    if (getTarget().getTriple().isOSMSVCRT()) {
      llvm::Type *ArgTypes[] = {Int8PtrTy, Int8PtrTy};
      llvm::AttributeSet ReturnsTwiceAttr =
          AttributeSet::get(getLLVMContext(), llvm::AttributeSet::FunctionIndex,
                            llvm::Attribute::ReturnsTwice);
      llvm::Constant *SetJmpEx = CGM.CreateRuntimeFunction(
          llvm::FunctionType::get(IntTy, ArgTypes, /*isVarArg=*/false),
          "_setjmpex", ReturnsTwiceAttr);
      llvm::Value *Buf = Builder.CreateBitOrPointerCast(
          EmitScalarExpr(E->getArg(0)), Int8PtrTy);
      llvm::Value *FrameAddr =
          Builder.CreateCall(CGM.getIntrinsic(Intrinsic::frameaddress),
                             ConstantInt::get(Int32Ty, 0));
      llvm::Value *Args[] = {Buf, FrameAddr};
      llvm::CallSite CS = EmitRuntimeCallOrInvoke(SetJmpEx, Args);
      CS.setAttributes(ReturnsTwiceAttr);
      return RValue::get(CS.getInstruction());
    }
    break;
  }
  case Builtin::BI_setjmp: {
    if (getTarget().getTriple().isOSMSVCRT()) {
      llvm::AttributeSet ReturnsTwiceAttr =
          AttributeSet::get(getLLVMContext(), llvm::AttributeSet::FunctionIndex,
                            llvm::Attribute::ReturnsTwice);
      llvm::Value *Buf = Builder.CreateBitOrPointerCast(
          EmitScalarExpr(E->getArg(0)), Int8PtrTy);
      llvm::CallSite CS;
      if (getTarget().getTriple().getArch() == llvm::Triple::x86) {
        llvm::Type *ArgTypes[] = {Int8PtrTy, IntTy};
        llvm::Constant *SetJmp3 = CGM.CreateRuntimeFunction(
            llvm::FunctionType::get(IntTy, ArgTypes, /*isVarArg=*/true),
            "_setjmp3", ReturnsTwiceAttr);
        llvm::Value *Count = ConstantInt::get(IntTy, 0);
        llvm::Value *Args[] = {Buf, Count};
        CS = EmitRuntimeCallOrInvoke(SetJmp3, Args);
      } else {
        llvm::Type *ArgTypes[] = {Int8PtrTy, Int8PtrTy};
        llvm::Constant *SetJmp = CGM.CreateRuntimeFunction(
            llvm::FunctionType::get(IntTy, ArgTypes, /*isVarArg=*/false),
            "_setjmp", ReturnsTwiceAttr);
        llvm::Value *FrameAddr =
            Builder.CreateCall(CGM.getIntrinsic(Intrinsic::frameaddress),
                               ConstantInt::get(Int32Ty, 0));
        llvm::Value *Args[] = {Buf, FrameAddr};
        CS = EmitRuntimeCallOrInvoke(SetJmp, Args);
      }
      CS.setAttributes(ReturnsTwiceAttr);
      return RValue::get(CS.getInstruction());
    }
    break;
  }

  case Builtin::BI__GetExceptionInfo: {
    if (llvm::GlobalVariable *GV =
            CGM.getCXXABI().getThrowInfo(FD->getParamDecl(0)->getType()))
      return RValue::get(llvm::ConstantExpr::getBitCast(GV, CGM.Int8PtrTy));
    break;
  }
#endif // HLSL Change Ends - no support for exception handling
  }

  // If this is an alias for a lib function (e.g. __builtin_sin), emit
  // the call using the normal call path, but using the unmangled
  // version of the function name.
  if (getContext().BuiltinInfo.isLibFunction(BuiltinID))
    return emitLibraryCall(*this, FD, E,
                           CGM.getBuiltinLibFunction(FD, BuiltinID));

  // If this is a predefined lib function (e.g. malloc), emit the call
  // using exactly the normal call path.
  if (getContext().BuiltinInfo.isPredefinedLibFunction(BuiltinID))
    return emitLibraryCall(*this, FD, E, EmitScalarExpr(E->getCallee()));

  // See if we have a target specific intrinsic.
  const char *Name = getContext().BuiltinInfo.GetName(BuiltinID);
  Intrinsic::ID IntrinsicID = Intrinsic::not_intrinsic;
  if (const char *Prefix =
          llvm::Triple::getArchTypePrefix(getTarget().getTriple().getArch())) {
    IntrinsicID = Intrinsic::getIntrinsicForGCCBuiltin(Prefix, Name);
    // NOTE we dont need to perform a compatibility flag check here since the
    // intrinsics are declared in Builtins*.def via LANGBUILTIN which filter the
    // MS builtins via ALL_MS_LANGUAGES and are filtered earlier.
    if (IntrinsicID == Intrinsic::not_intrinsic)
      IntrinsicID = Intrinsic::getIntrinsicForMSBuiltin(Prefix, Name);
  }

  if (IntrinsicID != Intrinsic::not_intrinsic) {
    SmallVector<Value*, 16> Args;

    // Find out if any arguments are required to be integer constant
    // expressions.
    unsigned ICEArguments = 0;
    ASTContext::GetBuiltinTypeError Error;
    getContext().GetBuiltinType(BuiltinID, Error, &ICEArguments);
    assert(Error == ASTContext::GE_None && "Should not codegen an error");

    Function *F = CGM.getIntrinsic(IntrinsicID);
    llvm::FunctionType *FTy = F->getFunctionType();

    for (unsigned i = 0, e = E->getNumArgs(); i != e; ++i) {
      Value *ArgValue;
      // If this is a normal argument, just emit it as a scalar.
      if ((ICEArguments & (1 << i)) == 0) {
        ArgValue = EmitScalarExpr(E->getArg(i));
      } else {
        // If this is required to be a constant, constant fold it so that we
        // know that the generated intrinsic gets a ConstantInt.
        llvm::APSInt Result;
        bool IsConst = E->getArg(i)->isIntegerConstantExpr(Result,getContext());
        assert(IsConst && "Constant arg isn't actually constant?");
        (void)IsConst;
        ArgValue = llvm::ConstantInt::get(getLLVMContext(), Result);
      }

      // If the intrinsic arg type is different from the builtin arg type
      // we need to do a bit cast.
      llvm::Type *PTy = FTy->getParamType(i);
      if (PTy != ArgValue->getType()) {
        assert(PTy->canLosslesslyBitCastTo(FTy->getParamType(i)) &&
               "Must be able to losslessly bit cast to param");
        ArgValue = Builder.CreateBitCast(ArgValue, PTy);
      }

      Args.push_back(ArgValue);
    }

    Value *V = Builder.CreateCall(F, Args);
    QualType BuiltinRetType = E->getType();

    llvm::Type *RetTy = VoidTy;
    if (!BuiltinRetType->isVoidType())
      RetTy = ConvertType(BuiltinRetType);

    if (RetTy != V->getType()) {
      assert(V->getType()->canLosslesslyBitCastTo(RetTy) &&
             "Must be able to losslessly bit cast result type");
      V = Builder.CreateBitCast(V, RetTy);
    }

    return RValue::get(V);
  }

  // See if we have a target specific builtin that needs to be lowered.
  if (Value *V = EmitTargetBuiltinExpr(BuiltinID, E))
    return RValue::get(V);

  ErrorUnsupported(E, "builtin function");

  // Unknown builtin, for now just dump it out and return undef.
  return GetUndefRValue(E->getType());
}

Value *CodeGenFunction::EmitTargetBuiltinExpr(unsigned BuiltinID,
                                              const CallExpr *E) {
#if 1 // HLSL Change Starts
  return nullptr;
#else
  switch (getTarget().getTriple().getArch()) {
  case llvm::Triple::arm:
  case llvm::Triple::armeb:
  case llvm::Triple::thumb:
  case llvm::Triple::thumbeb:
    return EmitARMBuiltinExpr(BuiltinID, E);
  case llvm::Triple::aarch64:
  case llvm::Triple::aarch64_be:
    return EmitAArch64BuiltinExpr(BuiltinID, E);
  case llvm::Triple::x86:
  case llvm::Triple::x86_64:
    return EmitX86BuiltinExpr(BuiltinID, E);
  case llvm::Triple::ppc:
  case llvm::Triple::ppc64:
  case llvm::Triple::ppc64le:
    return EmitPPCBuiltinExpr(BuiltinID, E);
  case llvm::Triple::r600:
  case llvm::Triple::amdgcn:
    return EmitAMDGPUBuiltinExpr(BuiltinID, E);
  case llvm::Triple::systemz:
    return EmitSystemZBuiltinExpr(BuiltinID, E);
  case llvm::Triple::nvptx:
  case llvm::Triple::nvptx64:
    return EmitNVPTXBuiltinExpr(BuiltinID, E);
  default:
    return nullptr;
  }
#endif // HLSL Change Ends
}

#if 0 // HLSL Change Starts

static llvm::VectorType *GetNeonType(CodeGenFunction *CGF,
                                     NeonTypeFlags TypeFlags,
                                     bool V1Ty=false) {
  int IsQuad = TypeFlags.isQuad();
  switch (TypeFlags.getEltType()) {
  case NeonTypeFlags::Int8:
  case NeonTypeFlags::Poly8:
    return llvm::VectorType::get(CGF->Int8Ty, V1Ty ? 1 : (8 << IsQuad));
  case NeonTypeFlags::Int16:
  case NeonTypeFlags::Poly16:
  case NeonTypeFlags::Float16:
    return llvm::VectorType::get(CGF->Int16Ty, V1Ty ? 1 : (4 << IsQuad));
  case NeonTypeFlags::Int32:
    return llvm::VectorType::get(CGF->Int32Ty, V1Ty ? 1 : (2 << IsQuad));
  case NeonTypeFlags::Int64:
  case NeonTypeFlags::Poly64:
    return llvm::VectorType::get(CGF->Int64Ty, V1Ty ? 1 : (1 << IsQuad));
  case NeonTypeFlags::Poly128:
    // FIXME: i128 and f128 doesn't get fully support in Clang and llvm.
    // There is a lot of i128 and f128 API missing.
    // so we use v16i8 to represent poly128 and get pattern matched.
    return llvm::VectorType::get(CGF->Int8Ty, 16);
  case NeonTypeFlags::Float32:
    return llvm::VectorType::get(CGF->FloatTy, V1Ty ? 1 : (2 << IsQuad));
  case NeonTypeFlags::Float64:
    return llvm::VectorType::get(CGF->DoubleTy, V1Ty ? 1 : (1 << IsQuad));
  }
  llvm_unreachable("Unknown vector element type!");
}

Value *CodeGenFunction::EmitNeonSplat(Value *V, Constant *C) {
  unsigned nElts = cast<llvm::VectorType>(V->getType())->getNumElements();
  Value* SV = llvm::ConstantVector::getSplat(nElts, C);
  return Builder.CreateShuffleVector(V, V, SV, "lane");
}

Value *CodeGenFunction::EmitNeonCall(Function *F, SmallVectorImpl<Value*> &Ops,
                                     const char *name,
                                     unsigned shift, bool rightshift) {
  unsigned j = 0;
  for (Function::const_arg_iterator ai = F->arg_begin(), ae = F->arg_end();
       ai != ae; ++ai, ++j)
    if (shift > 0 && shift == j)
      Ops[j] = EmitNeonShiftVector(Ops[j], ai->getType(), rightshift);
    else
      Ops[j] = Builder.CreateBitCast(Ops[j], ai->getType(), name);

  return Builder.CreateCall(F, Ops, name);
}

Value *CodeGenFunction::EmitNeonShiftVector(Value *V, llvm::Type *Ty,
                                            bool neg) {
  int SV = cast<ConstantInt>(V)->getSExtValue();

  llvm::VectorType *VTy = cast<llvm::VectorType>(Ty);
  llvm::Constant *C = ConstantInt::get(VTy->getElementType(), neg ? -SV : SV);
  return llvm::ConstantVector::getSplat(VTy->getNumElements(), C);
}

// \brief Right-shift a vector by a constant.
Value *CodeGenFunction::EmitNeonRShiftImm(Value *Vec, Value *Shift,
                                          llvm::Type *Ty, bool usgn,
                                          const char *name) {
  llvm::VectorType *VTy = cast<llvm::VectorType>(Ty);

  int ShiftAmt = cast<ConstantInt>(Shift)->getSExtValue();
  int EltSize = VTy->getScalarSizeInBits();

  Vec = Builder.CreateBitCast(Vec, Ty);

  // lshr/ashr are undefined when the shift amount is equal to the vector
  // element size.
  if (ShiftAmt == EltSize) {
    if (usgn) {
      // Right-shifting an unsigned value by its size yields 0.
      llvm::Constant *Zero = ConstantInt::get(VTy->getElementType(), 0);
      return llvm::ConstantVector::getSplat(VTy->getNumElements(), Zero);
    } else {
      // Right-shifting a signed value by its size is equivalent
      // to a shift of size-1.
      --ShiftAmt;
      Shift = ConstantInt::get(VTy->getElementType(), ShiftAmt);
    }
  }

  Shift = EmitNeonShiftVector(Shift, Ty, false);
  if (usgn)
    return Builder.CreateLShr(Vec, Shift, name);
  else
    return Builder.CreateAShr(Vec, Shift, name);
}

#endif // HLSL Change Ends

/// GetPointeeAlignment - Given an expression with a pointer type, find the
/// alignment of the type referenced by the pointer.  Skip over implicit
/// casts.
std::pair<llvm::Value*, unsigned>
CodeGenFunction::EmitPointerWithAlignment(const Expr *Addr) {
  assert(Addr->getType()->isPointerType());
  Addr = Addr->IgnoreParens();
  if (const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(Addr)) {
    if ((ICE->getCastKind() == CK_BitCast || ICE->getCastKind() == CK_NoOp) &&
        ICE->getSubExpr()->getType()->isPointerType()) {
      std::pair<llvm::Value*, unsigned> Ptr =
          EmitPointerWithAlignment(ICE->getSubExpr());
      Ptr.first = Builder.CreateBitCast(Ptr.first,
                                        ConvertType(Addr->getType()));
      return Ptr;
    } else if (ICE->getCastKind() == CK_ArrayToPointerDecay) {
      LValue LV = EmitLValue(ICE->getSubExpr());
      unsigned Align = LV.getAlignment().getQuantity();
      if (!Align) {
        // FIXME: Once LValues are fixed to always set alignment,
        // zap this code.
        QualType PtTy = ICE->getSubExpr()->getType();
        if (!PtTy->isIncompleteType())
          Align = getContext().getTypeAlignInChars(PtTy).getQuantity();
        else
          Align = 1;
      }
      return std::make_pair(LV.getAddress(), Align);
    }
  }
  if (const UnaryOperator *UO = dyn_cast<UnaryOperator>(Addr)) {
    if (UO->getOpcode() == UO_AddrOf) {
      LValue LV = EmitLValue(UO->getSubExpr());
      unsigned Align = LV.getAlignment().getQuantity();
      if (!Align) {
        // FIXME: Once LValues are fixed to always set alignment,
        // zap this code.
        QualType PtTy = UO->getSubExpr()->getType();
        if (!PtTy->isIncompleteType())
          Align = getContext().getTypeAlignInChars(PtTy).getQuantity();
        else
          Align = 1;
      }
      return std::make_pair(LV.getAddress(), Align);
    }
  }

  unsigned Align = 1;
  QualType PtTy = Addr->getType()->getPointeeType();
  if (!PtTy->isIncompleteType())
    Align = getContext().getTypeAlignInChars(PtTy).getQuantity();

  return std::make_pair(EmitScalarExpr(Addr), Align);
}

#if 0 // HLSL Change Starts

enum {
  AddRetType = (1 << 0),
  Add1ArgType = (1 << 1),
  Add2ArgTypes = (1 << 2),

  VectorizeRetType = (1 << 3),
  VectorizeArgTypes = (1 << 4),

  InventFloatType = (1 << 5),
  UnsignedAlts = (1 << 6),

  Use64BitVectors = (1 << 7),
  Use128BitVectors = (1 << 8),

  Vectorize1ArgType = Add1ArgType | VectorizeArgTypes,
  VectorRet = AddRetType | VectorizeRetType,
  VectorRetGetArgs01 =
      AddRetType | Add2ArgTypes | VectorizeRetType | VectorizeArgTypes,
  FpCmpzModifiers =
      AddRetType | VectorizeRetType | Add1ArgType | InventFloatType
};

 struct NeonIntrinsicInfo {
  unsigned BuiltinID;
  unsigned LLVMIntrinsic;
  unsigned AltLLVMIntrinsic;
  const char *NameHint;
  unsigned TypeModifier;

  bool operator<(unsigned RHSBuiltinID) const {
    return BuiltinID < RHSBuiltinID;
  }
};

#define NEONMAP0(NameBase) \
  { NEON::BI__builtin_neon_ ## NameBase, 0, 0, #NameBase, 0 }

#define NEONMAP1(NameBase, LLVMIntrinsic, TypeModifier) \
  { NEON:: BI__builtin_neon_ ## NameBase, \
      Intrinsic::LLVMIntrinsic, 0, #NameBase, TypeModifier }

#define NEONMAP2(NameBase, LLVMIntrinsic, AltLLVMIntrinsic, TypeModifier) \
  { NEON:: BI__builtin_neon_ ## NameBase, \
      Intrinsic::LLVMIntrinsic, Intrinsic::AltLLVMIntrinsic, \
      #NameBase, TypeModifier }

static NeonIntrinsicInfo ARMSIMDIntrinsicMap [] = {
  NEONMAP2(vabd_v, arm_neon_vabdu, arm_neon_vabds, Add1ArgType | UnsignedAlts),
  NEONMAP2(vabdq_v, arm_neon_vabdu, arm_neon_vabds, Add1ArgType | UnsignedAlts),
  NEONMAP1(vabs_v, arm_neon_vabs, 0),
  NEONMAP1(vabsq_v, arm_neon_vabs, 0),
  NEONMAP0(vaddhn_v),
  NEONMAP1(vaesdq_v, arm_neon_aesd, 0),
  NEONMAP1(vaeseq_v, arm_neon_aese, 0),
  NEONMAP1(vaesimcq_v, arm_neon_aesimc, 0),
  NEONMAP1(vaesmcq_v, arm_neon_aesmc, 0),
  NEONMAP1(vbsl_v, arm_neon_vbsl, AddRetType),
  NEONMAP1(vbslq_v, arm_neon_vbsl, AddRetType),
  NEONMAP1(vcage_v, arm_neon_vacge, 0),
  NEONMAP1(vcageq_v, arm_neon_vacge, 0),
  NEONMAP1(vcagt_v, arm_neon_vacgt, 0),
  NEONMAP1(vcagtq_v, arm_neon_vacgt, 0),
  NEONMAP1(vcale_v, arm_neon_vacge, 0),
  NEONMAP1(vcaleq_v, arm_neon_vacge, 0),
  NEONMAP1(vcalt_v, arm_neon_vacgt, 0),
  NEONMAP1(vcaltq_v, arm_neon_vacgt, 0),
  NEONMAP1(vcls_v, arm_neon_vcls, Add1ArgType),
  NEONMAP1(vclsq_v, arm_neon_vcls, Add1ArgType),
  NEONMAP1(vclz_v, ctlz, Add1ArgType),
  NEONMAP1(vclzq_v, ctlz, Add1ArgType),
  NEONMAP1(vcnt_v, ctpop, Add1ArgType),
  NEONMAP1(vcntq_v, ctpop, Add1ArgType),
  NEONMAP1(vcvt_f16_v, arm_neon_vcvtfp2hf, 0),
  NEONMAP1(vcvt_f32_f16, arm_neon_vcvthf2fp, 0),
  NEONMAP0(vcvt_f32_v),
  NEONMAP2(vcvt_n_f32_v, arm_neon_vcvtfxu2fp, arm_neon_vcvtfxs2fp, 0),
  NEONMAP1(vcvt_n_s32_v, arm_neon_vcvtfp2fxs, 0),
  NEONMAP1(vcvt_n_s64_v, arm_neon_vcvtfp2fxs, 0),
  NEONMAP1(vcvt_n_u32_v, arm_neon_vcvtfp2fxu, 0),
  NEONMAP1(vcvt_n_u64_v, arm_neon_vcvtfp2fxu, 0),
  NEONMAP0(vcvt_s32_v),
  NEONMAP0(vcvt_s64_v),
  NEONMAP0(vcvt_u32_v),
  NEONMAP0(vcvt_u64_v),
  NEONMAP1(vcvta_s32_v, arm_neon_vcvtas, 0),
  NEONMAP1(vcvta_s64_v, arm_neon_vcvtas, 0),
  NEONMAP1(vcvta_u32_v, arm_neon_vcvtau, 0),
  NEONMAP1(vcvta_u64_v, arm_neon_vcvtau, 0),
  NEONMAP1(vcvtaq_s32_v, arm_neon_vcvtas, 0),
  NEONMAP1(vcvtaq_s64_v, arm_neon_vcvtas, 0),
  NEONMAP1(vcvtaq_u32_v, arm_neon_vcvtau, 0),
  NEONMAP1(vcvtaq_u64_v, arm_neon_vcvtau, 0),
  NEONMAP1(vcvtm_s32_v, arm_neon_vcvtms, 0),
  NEONMAP1(vcvtm_s64_v, arm_neon_vcvtms, 0),
  NEONMAP1(vcvtm_u32_v, arm_neon_vcvtmu, 0),
  NEONMAP1(vcvtm_u64_v, arm_neon_vcvtmu, 0),
  NEONMAP1(vcvtmq_s32_v, arm_neon_vcvtms, 0),
  NEONMAP1(vcvtmq_s64_v, arm_neon_vcvtms, 0),
  NEONMAP1(vcvtmq_u32_v, arm_neon_vcvtmu, 0),
  NEONMAP1(vcvtmq_u64_v, arm_neon_vcvtmu, 0),
  NEONMAP1(vcvtn_s32_v, arm_neon_vcvtns, 0),
  NEONMAP1(vcvtn_s64_v, arm_neon_vcvtns, 0),
  NEONMAP1(vcvtn_u32_v, arm_neon_vcvtnu, 0),
  NEONMAP1(vcvtn_u64_v, arm_neon_vcvtnu, 0),
  NEONMAP1(vcvtnq_s32_v, arm_neon_vcvtns, 0),
  NEONMAP1(vcvtnq_s64_v, arm_neon_vcvtns, 0),
  NEONMAP1(vcvtnq_u32_v, arm_neon_vcvtnu, 0),
  NEONMAP1(vcvtnq_u64_v, arm_neon_vcvtnu, 0),
  NEONMAP1(vcvtp_s32_v, arm_neon_vcvtps, 0),
  NEONMAP1(vcvtp_s64_v, arm_neon_vcvtps, 0),
  NEONMAP1(vcvtp_u32_v, arm_neon_vcvtpu, 0),
  NEONMAP1(vcvtp_u64_v, arm_neon_vcvtpu, 0),
  NEONMAP1(vcvtpq_s32_v, arm_neon_vcvtps, 0),
  NEONMAP1(vcvtpq_s64_v, arm_neon_vcvtps, 0),
  NEONMAP1(vcvtpq_u32_v, arm_neon_vcvtpu, 0),
  NEONMAP1(vcvtpq_u64_v, arm_neon_vcvtpu, 0),
  NEONMAP0(vcvtq_f32_v),
  NEONMAP2(vcvtq_n_f32_v, arm_neon_vcvtfxu2fp, arm_neon_vcvtfxs2fp, 0),
  NEONMAP1(vcvtq_n_s32_v, arm_neon_vcvtfp2fxs, 0),
  NEONMAP1(vcvtq_n_s64_v, arm_neon_vcvtfp2fxs, 0),
  NEONMAP1(vcvtq_n_u32_v, arm_neon_vcvtfp2fxu, 0),
  NEONMAP1(vcvtq_n_u64_v, arm_neon_vcvtfp2fxu, 0),
  NEONMAP0(vcvtq_s32_v),
  NEONMAP0(vcvtq_s64_v),
  NEONMAP0(vcvtq_u32_v),
  NEONMAP0(vcvtq_u64_v),
  NEONMAP0(vext_v),
  NEONMAP0(vextq_v),
  NEONMAP0(vfma_v),
  NEONMAP0(vfmaq_v),
  NEONMAP2(vhadd_v, arm_neon_vhaddu, arm_neon_vhadds, Add1ArgType | UnsignedAlts),
  NEONMAP2(vhaddq_v, arm_neon_vhaddu, arm_neon_vhadds, Add1ArgType | UnsignedAlts),
  NEONMAP2(vhsub_v, arm_neon_vhsubu, arm_neon_vhsubs, Add1ArgType | UnsignedAlts),
  NEONMAP2(vhsubq_v, arm_neon_vhsubu, arm_neon_vhsubs, Add1ArgType | UnsignedAlts),
  NEONMAP0(vld1_dup_v),
  NEONMAP1(vld1_v, arm_neon_vld1, 0),
  NEONMAP0(vld1q_dup_v),
  NEONMAP1(vld1q_v, arm_neon_vld1, 0),
  NEONMAP1(vld2_lane_v, arm_neon_vld2lane, 0),
  NEONMAP1(vld2_v, arm_neon_vld2, 0),
  NEONMAP1(vld2q_lane_v, arm_neon_vld2lane, 0),
  NEONMAP1(vld2q_v, arm_neon_vld2, 0),
  NEONMAP1(vld3_lane_v, arm_neon_vld3lane, 0),
  NEONMAP1(vld3_v, arm_neon_vld3, 0),
  NEONMAP1(vld3q_lane_v, arm_neon_vld3lane, 0),
  NEONMAP1(vld3q_v, arm_neon_vld3, 0),
  NEONMAP1(vld4_lane_v, arm_neon_vld4lane, 0),
  NEONMAP1(vld4_v, arm_neon_vld4, 0),
  NEONMAP1(vld4q_lane_v, arm_neon_vld4lane, 0),
  NEONMAP1(vld4q_v, arm_neon_vld4, 0),
  NEONMAP2(vmax_v, arm_neon_vmaxu, arm_neon_vmaxs, Add1ArgType | UnsignedAlts),
  NEONMAP1(vmaxnm_v, arm_neon_vmaxnm, Add1ArgType),
  NEONMAP1(vmaxnmq_v, arm_neon_vmaxnm, Add1ArgType),
  NEONMAP2(vmaxq_v, arm_neon_vmaxu, arm_neon_vmaxs, Add1ArgType | UnsignedAlts),
  NEONMAP2(vmin_v, arm_neon_vminu, arm_neon_vmins, Add1ArgType | UnsignedAlts),
  NEONMAP1(vminnm_v, arm_neon_vminnm, Add1ArgType),
  NEONMAP1(vminnmq_v, arm_neon_vminnm, Add1ArgType),
  NEONMAP2(vminq_v, arm_neon_vminu, arm_neon_vmins, Add1ArgType | UnsignedAlts),
  NEONMAP0(vmovl_v),
  NEONMAP0(vmovn_v),
  NEONMAP1(vmul_v, arm_neon_vmulp, Add1ArgType),
  NEONMAP0(vmull_v),
  NEONMAP1(vmulq_v, arm_neon_vmulp, Add1ArgType),
  NEONMAP2(vpadal_v, arm_neon_vpadalu, arm_neon_vpadals, UnsignedAlts),
  NEONMAP2(vpadalq_v, arm_neon_vpadalu, arm_neon_vpadals, UnsignedAlts),
  NEONMAP1(vpadd_v, arm_neon_vpadd, Add1ArgType),
  NEONMAP2(vpaddl_v, arm_neon_vpaddlu, arm_neon_vpaddls, UnsignedAlts),
  NEONMAP2(vpaddlq_v, arm_neon_vpaddlu, arm_neon_vpaddls, UnsignedAlts),
  NEONMAP1(vpaddq_v, arm_neon_vpadd, Add1ArgType),
  NEONMAP2(vpmax_v, arm_neon_vpmaxu, arm_neon_vpmaxs, Add1ArgType | UnsignedAlts),
  NEONMAP2(vpmin_v, arm_neon_vpminu, arm_neon_vpmins, Add1ArgType | UnsignedAlts),
  NEONMAP1(vqabs_v, arm_neon_vqabs, Add1ArgType),
  NEONMAP1(vqabsq_v, arm_neon_vqabs, Add1ArgType),
  NEONMAP2(vqadd_v, arm_neon_vqaddu, arm_neon_vqadds, Add1ArgType | UnsignedAlts),
  NEONMAP2(vqaddq_v, arm_neon_vqaddu, arm_neon_vqadds, Add1ArgType | UnsignedAlts),
  NEONMAP2(vqdmlal_v, arm_neon_vqdmull, arm_neon_vqadds, 0),
  NEONMAP2(vqdmlsl_v, arm_neon_vqdmull, arm_neon_vqsubs, 0),
  NEONMAP1(vqdmulh_v, arm_neon_vqdmulh, Add1ArgType),
  NEONMAP1(vqdmulhq_v, arm_neon_vqdmulh, Add1ArgType),
  NEONMAP1(vqdmull_v, arm_neon_vqdmull, Add1ArgType),
  NEONMAP2(vqmovn_v, arm_neon_vqmovnu, arm_neon_vqmovns, Add1ArgType | UnsignedAlts),
  NEONMAP1(vqmovun_v, arm_neon_vqmovnsu, Add1ArgType),
  NEONMAP1(vqneg_v, arm_neon_vqneg, Add1ArgType),
  NEONMAP1(vqnegq_v, arm_neon_vqneg, Add1ArgType),
  NEONMAP1(vqrdmulh_v, arm_neon_vqrdmulh, Add1ArgType),
  NEONMAP1(vqrdmulhq_v, arm_neon_vqrdmulh, Add1ArgType),
  NEONMAP2(vqrshl_v, arm_neon_vqrshiftu, arm_neon_vqrshifts, Add1ArgType | UnsignedAlts),
  NEONMAP2(vqrshlq_v, arm_neon_vqrshiftu, arm_neon_vqrshifts, Add1ArgType | UnsignedAlts),
  NEONMAP2(vqshl_n_v, arm_neon_vqshiftu, arm_neon_vqshifts, UnsignedAlts),
  NEONMAP2(vqshl_v, arm_neon_vqshiftu, arm_neon_vqshifts, Add1ArgType | UnsignedAlts),
  NEONMAP2(vqshlq_n_v, arm_neon_vqshiftu, arm_neon_vqshifts, UnsignedAlts),
  NEONMAP2(vqshlq_v, arm_neon_vqshiftu, arm_neon_vqshifts, Add1ArgType | UnsignedAlts),
  NEONMAP1(vqshlu_n_v, arm_neon_vqshiftsu, 0),
  NEONMAP1(vqshluq_n_v, arm_neon_vqshiftsu, 0),
  NEONMAP2(vqsub_v, arm_neon_vqsubu, arm_neon_vqsubs, Add1ArgType | UnsignedAlts),
  NEONMAP2(vqsubq_v, arm_neon_vqsubu, arm_neon_vqsubs, Add1ArgType | UnsignedAlts),
  NEONMAP1(vraddhn_v, arm_neon_vraddhn, Add1ArgType),
  NEONMAP2(vrecpe_v, arm_neon_vrecpe, arm_neon_vrecpe, 0),
  NEONMAP2(vrecpeq_v, arm_neon_vrecpe, arm_neon_vrecpe, 0),
  NEONMAP1(vrecps_v, arm_neon_vrecps, Add1ArgType),
  NEONMAP1(vrecpsq_v, arm_neon_vrecps, Add1ArgType),
  NEONMAP2(vrhadd_v, arm_neon_vrhaddu, arm_neon_vrhadds, Add1ArgType | UnsignedAlts),
  NEONMAP2(vrhaddq_v, arm_neon_vrhaddu, arm_neon_vrhadds, Add1ArgType | UnsignedAlts),
  NEONMAP1(vrnd_v, arm_neon_vrintz, Add1ArgType),
  NEONMAP1(vrnda_v, arm_neon_vrinta, Add1ArgType),
  NEONMAP1(vrndaq_v, arm_neon_vrinta, Add1ArgType),
  NEONMAP1(vrndm_v, arm_neon_vrintm, Add1ArgType),
  NEONMAP1(vrndmq_v, arm_neon_vrintm, Add1ArgType),
  NEONMAP1(vrndn_v, arm_neon_vrintn, Add1ArgType),
  NEONMAP1(vrndnq_v, arm_neon_vrintn, Add1ArgType),
  NEONMAP1(vrndp_v, arm_neon_vrintp, Add1ArgType),
  NEONMAP1(vrndpq_v, arm_neon_vrintp, Add1ArgType),
  NEONMAP1(vrndq_v, arm_neon_vrintz, Add1ArgType),
  NEONMAP1(vrndx_v, arm_neon_vrintx, Add1ArgType),
  NEONMAP1(vrndxq_v, arm_neon_vrintx, Add1ArgType),
  NEONMAP2(vrshl_v, arm_neon_vrshiftu, arm_neon_vrshifts, Add1ArgType | UnsignedAlts),
  NEONMAP2(vrshlq_v, arm_neon_vrshiftu, arm_neon_vrshifts, Add1ArgType | UnsignedAlts),
  NEONMAP2(vrshr_n_v, arm_neon_vrshiftu, arm_neon_vrshifts, UnsignedAlts),
  NEONMAP2(vrshrq_n_v, arm_neon_vrshiftu, arm_neon_vrshifts, UnsignedAlts),
  NEONMAP2(vrsqrte_v, arm_neon_vrsqrte, arm_neon_vrsqrte, 0),
  NEONMAP2(vrsqrteq_v, arm_neon_vrsqrte, arm_neon_vrsqrte, 0),
  NEONMAP1(vrsqrts_v, arm_neon_vrsqrts, Add1ArgType),
  NEONMAP1(vrsqrtsq_v, arm_neon_vrsqrts, Add1ArgType),
  NEONMAP1(vrsubhn_v, arm_neon_vrsubhn, Add1ArgType),
  NEONMAP1(vsha1su0q_v, arm_neon_sha1su0, 0),
  NEONMAP1(vsha1su1q_v, arm_neon_sha1su1, 0),
  NEONMAP1(vsha256h2q_v, arm_neon_sha256h2, 0),
  NEONMAP1(vsha256hq_v, arm_neon_sha256h, 0),
  NEONMAP1(vsha256su0q_v, arm_neon_sha256su0, 0),
  NEONMAP1(vsha256su1q_v, arm_neon_sha256su1, 0),
  NEONMAP0(vshl_n_v),
  NEONMAP2(vshl_v, arm_neon_vshiftu, arm_neon_vshifts, Add1ArgType | UnsignedAlts),
  NEONMAP0(vshll_n_v),
  NEONMAP0(vshlq_n_v),
  NEONMAP2(vshlq_v, arm_neon_vshiftu, arm_neon_vshifts, Add1ArgType | UnsignedAlts),
  NEONMAP0(vshr_n_v),
  NEONMAP0(vshrn_n_v),
  NEONMAP0(vshrq_n_v),
  NEONMAP1(vst1_v, arm_neon_vst1, 0),
  NEONMAP1(vst1q_v, arm_neon_vst1, 0),
  NEONMAP1(vst2_lane_v, arm_neon_vst2lane, 0),
  NEONMAP1(vst2_v, arm_neon_vst2, 0),
  NEONMAP1(vst2q_lane_v, arm_neon_vst2lane, 0),
  NEONMAP1(vst2q_v, arm_neon_vst2, 0),
  NEONMAP1(vst3_lane_v, arm_neon_vst3lane, 0),
  NEONMAP1(vst3_v, arm_neon_vst3, 0),
  NEONMAP1(vst3q_lane_v, arm_neon_vst3lane, 0),
  NEONMAP1(vst3q_v, arm_neon_vst3, 0),
  NEONMAP1(vst4_lane_v, arm_neon_vst4lane, 0),
  NEONMAP1(vst4_v, arm_neon_vst4, 0),
  NEONMAP1(vst4q_lane_v, arm_neon_vst4lane, 0),
  NEONMAP1(vst4q_v, arm_neon_vst4, 0),
  NEONMAP0(vsubhn_v),
  NEONMAP0(vtrn_v),
  NEONMAP0(vtrnq_v),
  NEONMAP0(vtst_v),
  NEONMAP0(vtstq_v),
  NEONMAP0(vuzp_v),
  NEONMAP0(vuzpq_v),
  NEONMAP0(vzip_v),
  NEONMAP0(vzipq_v)
};

static NeonIntrinsicInfo AArch64SIMDIntrinsicMap[] = {
  NEONMAP1(vabs_v, aarch64_neon_abs, 0),
  NEONMAP1(vabsq_v, aarch64_neon_abs, 0),
  NEONMAP0(vaddhn_v),
  NEONMAP1(vaesdq_v, aarch64_crypto_aesd, 0),
  NEONMAP1(vaeseq_v, aarch64_crypto_aese, 0),
  NEONMAP1(vaesimcq_v, aarch64_crypto_aesimc, 0),
  NEONMAP1(vaesmcq_v, aarch64_crypto_aesmc, 0),
  NEONMAP1(vcage_v, aarch64_neon_facge, 0),
  NEONMAP1(vcageq_v, aarch64_neon_facge, 0),
  NEONMAP1(vcagt_v, aarch64_neon_facgt, 0),
  NEONMAP1(vcagtq_v, aarch64_neon_facgt, 0),
  NEONMAP1(vcale_v, aarch64_neon_facge, 0),
  NEONMAP1(vcaleq_v, aarch64_neon_facge, 0),
  NEONMAP1(vcalt_v, aarch64_neon_facgt, 0),
  NEONMAP1(vcaltq_v, aarch64_neon_facgt, 0),
  NEONMAP1(vcls_v, aarch64_neon_cls, Add1ArgType),
  NEONMAP1(vclsq_v, aarch64_neon_cls, Add1ArgType),
  NEONMAP1(vclz_v, ctlz, Add1ArgType),
  NEONMAP1(vclzq_v, ctlz, Add1ArgType),
  NEONMAP1(vcnt_v, ctpop, Add1ArgType),
  NEONMAP1(vcntq_v, ctpop, Add1ArgType),
  NEONMAP1(vcvt_f16_v, aarch64_neon_vcvtfp2hf, 0),
  NEONMAP1(vcvt_f32_f16, aarch64_neon_vcvthf2fp, 0),
  NEONMAP0(vcvt_f32_v),
  NEONMAP2(vcvt_n_f32_v, aarch64_neon_vcvtfxu2fp, aarch64_neon_vcvtfxs2fp, 0),
  NEONMAP2(vcvt_n_f64_v, aarch64_neon_vcvtfxu2fp, aarch64_neon_vcvtfxs2fp, 0),
  NEONMAP1(vcvt_n_s32_v, aarch64_neon_vcvtfp2fxs, 0),
  NEONMAP1(vcvt_n_s64_v, aarch64_neon_vcvtfp2fxs, 0),
  NEONMAP1(vcvt_n_u32_v, aarch64_neon_vcvtfp2fxu, 0),
  NEONMAP1(vcvt_n_u64_v, aarch64_neon_vcvtfp2fxu, 0),
  NEONMAP0(vcvtq_f32_v),
  NEONMAP2(vcvtq_n_f32_v, aarch64_neon_vcvtfxu2fp, aarch64_neon_vcvtfxs2fp, 0),
  NEONMAP2(vcvtq_n_f64_v, aarch64_neon_vcvtfxu2fp, aarch64_neon_vcvtfxs2fp, 0),
  NEONMAP1(vcvtq_n_s32_v, aarch64_neon_vcvtfp2fxs, 0),
  NEONMAP1(vcvtq_n_s64_v, aarch64_neon_vcvtfp2fxs, 0),
  NEONMAP1(vcvtq_n_u32_v, aarch64_neon_vcvtfp2fxu, 0),
  NEONMAP1(vcvtq_n_u64_v, aarch64_neon_vcvtfp2fxu, 0),
  NEONMAP1(vcvtx_f32_v, aarch64_neon_fcvtxn, AddRetType | Add1ArgType),
  NEONMAP0(vext_v),
  NEONMAP0(vextq_v),
  NEONMAP0(vfma_v),
  NEONMAP0(vfmaq_v),
  NEONMAP2(vhadd_v, aarch64_neon_uhadd, aarch64_neon_shadd, Add1ArgType | UnsignedAlts),
  NEONMAP2(vhaddq_v, aarch64_neon_uhadd, aarch64_neon_shadd, Add1ArgType | UnsignedAlts),
  NEONMAP2(vhsub_v, aarch64_neon_uhsub, aarch64_neon_shsub, Add1ArgType | UnsignedAlts),
  NEONMAP2(vhsubq_v, aarch64_neon_uhsub, aarch64_neon_shsub, Add1ArgType | UnsignedAlts),
  NEONMAP0(vmovl_v),
  NEONMAP0(vmovn_v),
  NEONMAP1(vmul_v, aarch64_neon_pmul, Add1ArgType),
  NEONMAP1(vmulq_v, aarch64_neon_pmul, Add1ArgType),
  NEONMAP1(vpadd_v, aarch64_neon_addp, Add1ArgType),
  NEONMAP2(vpaddl_v, aarch64_neon_uaddlp, aarch64_neon_saddlp, UnsignedAlts),
  NEONMAP2(vpaddlq_v, aarch64_neon_uaddlp, aarch64_neon_saddlp, UnsignedAlts),
  NEONMAP1(vpaddq_v, aarch64_neon_addp, Add1ArgType),
  NEONMAP1(vqabs_v, aarch64_neon_sqabs, Add1ArgType),
  NEONMAP1(vqabsq_v, aarch64_neon_sqabs, Add1ArgType),
  NEONMAP2(vqadd_v, aarch64_neon_uqadd, aarch64_neon_sqadd, Add1ArgType | UnsignedAlts),
  NEONMAP2(vqaddq_v, aarch64_neon_uqadd, aarch64_neon_sqadd, Add1ArgType | UnsignedAlts),
  NEONMAP2(vqdmlal_v, aarch64_neon_sqdmull, aarch64_neon_sqadd, 0),
  NEONMAP2(vqdmlsl_v, aarch64_neon_sqdmull, aarch64_neon_sqsub, 0),
  NEONMAP1(vqdmulh_v, aarch64_neon_sqdmulh, Add1ArgType),
  NEONMAP1(vqdmulhq_v, aarch64_neon_sqdmulh, Add1ArgType),
  NEONMAP1(vqdmull_v, aarch64_neon_sqdmull, Add1ArgType),
  NEONMAP2(vqmovn_v, aarch64_neon_uqxtn, aarch64_neon_sqxtn, Add1ArgType | UnsignedAlts),
  NEONMAP1(vqmovun_v, aarch64_neon_sqxtun, Add1ArgType),
  NEONMAP1(vqneg_v, aarch64_neon_sqneg, Add1ArgType),
  NEONMAP1(vqnegq_v, aarch64_neon_sqneg, Add1ArgType),
  NEONMAP1(vqrdmulh_v, aarch64_neon_sqrdmulh, Add1ArgType),
  NEONMAP1(vqrdmulhq_v, aarch64_neon_sqrdmulh, Add1ArgType),
  NEONMAP2(vqrshl_v, aarch64_neon_uqrshl, aarch64_neon_sqrshl, Add1ArgType | UnsignedAlts),
  NEONMAP2(vqrshlq_v, aarch64_neon_uqrshl, aarch64_neon_sqrshl, Add1ArgType | UnsignedAlts),
  NEONMAP2(vqshl_n_v, aarch64_neon_uqshl, aarch64_neon_sqshl, UnsignedAlts),
  NEONMAP2(vqshl_v, aarch64_neon_uqshl, aarch64_neon_sqshl, Add1ArgType | UnsignedAlts),
  NEONMAP2(vqshlq_n_v, aarch64_neon_uqshl, aarch64_neon_sqshl,UnsignedAlts),
  NEONMAP2(vqshlq_v, aarch64_neon_uqshl, aarch64_neon_sqshl, Add1ArgType | UnsignedAlts),
  NEONMAP1(vqshlu_n_v, aarch64_neon_sqshlu, 0),
  NEONMAP1(vqshluq_n_v, aarch64_neon_sqshlu, 0),
  NEONMAP2(vqsub_v, aarch64_neon_uqsub, aarch64_neon_sqsub, Add1ArgType | UnsignedAlts),
  NEONMAP2(vqsubq_v, aarch64_neon_uqsub, aarch64_neon_sqsub, Add1ArgType | UnsignedAlts),
  NEONMAP1(vraddhn_v, aarch64_neon_raddhn, Add1ArgType),
  NEONMAP2(vrecpe_v, aarch64_neon_frecpe, aarch64_neon_urecpe, 0),
  NEONMAP2(vrecpeq_v, aarch64_neon_frecpe, aarch64_neon_urecpe, 0),
  NEONMAP1(vrecps_v, aarch64_neon_frecps, Add1ArgType),
  NEONMAP1(vrecpsq_v, aarch64_neon_frecps, Add1ArgType),
  NEONMAP2(vrhadd_v, aarch64_neon_urhadd, aarch64_neon_srhadd, Add1ArgType | UnsignedAlts),
  NEONMAP2(vrhaddq_v, aarch64_neon_urhadd, aarch64_neon_srhadd, Add1ArgType | UnsignedAlts),
  NEONMAP2(vrshl_v, aarch64_neon_urshl, aarch64_neon_srshl, Add1ArgType | UnsignedAlts),
  NEONMAP2(vrshlq_v, aarch64_neon_urshl, aarch64_neon_srshl, Add1ArgType | UnsignedAlts),
  NEONMAP2(vrshr_n_v, aarch64_neon_urshl, aarch64_neon_srshl, UnsignedAlts),
  NEONMAP2(vrshrq_n_v, aarch64_neon_urshl, aarch64_neon_srshl, UnsignedAlts),
  NEONMAP2(vrsqrte_v, aarch64_neon_frsqrte, aarch64_neon_ursqrte, 0),
  NEONMAP2(vrsqrteq_v, aarch64_neon_frsqrte, aarch64_neon_ursqrte, 0),
  NEONMAP1(vrsqrts_v, aarch64_neon_frsqrts, Add1ArgType),
  NEONMAP1(vrsqrtsq_v, aarch64_neon_frsqrts, Add1ArgType),
  NEONMAP1(vrsubhn_v, aarch64_neon_rsubhn, Add1ArgType),
  NEONMAP1(vsha1su0q_v, aarch64_crypto_sha1su0, 0),
  NEONMAP1(vsha1su1q_v, aarch64_crypto_sha1su1, 0),
  NEONMAP1(vsha256h2q_v, aarch64_crypto_sha256h2, 0),
  NEONMAP1(vsha256hq_v, aarch64_crypto_sha256h, 0),
  NEONMAP1(vsha256su0q_v, aarch64_crypto_sha256su0, 0),
  NEONMAP1(vsha256su1q_v, aarch64_crypto_sha256su1, 0),
  NEONMAP0(vshl_n_v),
  NEONMAP2(vshl_v, aarch64_neon_ushl, aarch64_neon_sshl, Add1ArgType | UnsignedAlts),
  NEONMAP0(vshll_n_v),
  NEONMAP0(vshlq_n_v),
  NEONMAP2(vshlq_v, aarch64_neon_ushl, aarch64_neon_sshl, Add1ArgType | UnsignedAlts),
  NEONMAP0(vshr_n_v),
  NEONMAP0(vshrn_n_v),
  NEONMAP0(vshrq_n_v),
  NEONMAP0(vsubhn_v),
  NEONMAP0(vtst_v),
  NEONMAP0(vtstq_v),
};

static NeonIntrinsicInfo AArch64SISDIntrinsicMap[] = {
  NEONMAP1(vabdd_f64, aarch64_sisd_fabd, Add1ArgType),
  NEONMAP1(vabds_f32, aarch64_sisd_fabd, Add1ArgType),
  NEONMAP1(vabsd_s64, aarch64_neon_abs, Add1ArgType),
  NEONMAP1(vaddlv_s32, aarch64_neon_saddlv, AddRetType | Add1ArgType),
  NEONMAP1(vaddlv_u32, aarch64_neon_uaddlv, AddRetType | Add1ArgType),
  NEONMAP1(vaddlvq_s32, aarch64_neon_saddlv, AddRetType | Add1ArgType),
  NEONMAP1(vaddlvq_u32, aarch64_neon_uaddlv, AddRetType | Add1ArgType),
  NEONMAP1(vaddv_f32, aarch64_neon_faddv, AddRetType | Add1ArgType),
  NEONMAP1(vaddv_s32, aarch64_neon_saddv, AddRetType | Add1ArgType),
  NEONMAP1(vaddv_u32, aarch64_neon_uaddv, AddRetType | Add1ArgType),
  NEONMAP1(vaddvq_f32, aarch64_neon_faddv, AddRetType | Add1ArgType),
  NEONMAP1(vaddvq_f64, aarch64_neon_faddv, AddRetType | Add1ArgType),
  NEONMAP1(vaddvq_s32, aarch64_neon_saddv, AddRetType | Add1ArgType),
  NEONMAP1(vaddvq_s64, aarch64_neon_saddv, AddRetType | Add1ArgType),
  NEONMAP1(vaddvq_u32, aarch64_neon_uaddv, AddRetType | Add1ArgType),
  NEONMAP1(vaddvq_u64, aarch64_neon_uaddv, AddRetType | Add1ArgType),
  NEONMAP1(vcaged_f64, aarch64_neon_facge, AddRetType | Add1ArgType),
  NEONMAP1(vcages_f32, aarch64_neon_facge, AddRetType | Add1ArgType),
  NEONMAP1(vcagtd_f64, aarch64_neon_facgt, AddRetType | Add1ArgType),
  NEONMAP1(vcagts_f32, aarch64_neon_facgt, AddRetType | Add1ArgType),
  NEONMAP1(vcaled_f64, aarch64_neon_facge, AddRetType | Add1ArgType),
  NEONMAP1(vcales_f32, aarch64_neon_facge, AddRetType | Add1ArgType),
  NEONMAP1(vcaltd_f64, aarch64_neon_facgt, AddRetType | Add1ArgType),
  NEONMAP1(vcalts_f32, aarch64_neon_facgt, AddRetType | Add1ArgType),
  NEONMAP1(vcvtad_s64_f64, aarch64_neon_fcvtas, AddRetType | Add1ArgType),
  NEONMAP1(vcvtad_u64_f64, aarch64_neon_fcvtau, AddRetType | Add1ArgType),
  NEONMAP1(vcvtas_s32_f32, aarch64_neon_fcvtas, AddRetType | Add1ArgType),
  NEONMAP1(vcvtas_u32_f32, aarch64_neon_fcvtau, AddRetType | Add1ArgType),
  NEONMAP1(vcvtd_n_f64_s64, aarch64_neon_vcvtfxs2fp, AddRetType | Add1ArgType),
  NEONMAP1(vcvtd_n_f64_u64, aarch64_neon_vcvtfxu2fp, AddRetType | Add1ArgType),
  NEONMAP1(vcvtd_n_s64_f64, aarch64_neon_vcvtfp2fxs, AddRetType | Add1ArgType),
  NEONMAP1(vcvtd_n_u64_f64, aarch64_neon_vcvtfp2fxu, AddRetType | Add1ArgType),
  NEONMAP1(vcvtmd_s64_f64, aarch64_neon_fcvtms, AddRetType | Add1ArgType),
  NEONMAP1(vcvtmd_u64_f64, aarch64_neon_fcvtmu, AddRetType | Add1ArgType),
  NEONMAP1(vcvtms_s32_f32, aarch64_neon_fcvtms, AddRetType | Add1ArgType),
  NEONMAP1(vcvtms_u32_f32, aarch64_neon_fcvtmu, AddRetType | Add1ArgType),
  NEONMAP1(vcvtnd_s64_f64, aarch64_neon_fcvtns, AddRetType | Add1ArgType),
  NEONMAP1(vcvtnd_u64_f64, aarch64_neon_fcvtnu, AddRetType | Add1ArgType),
  NEONMAP1(vcvtns_s32_f32, aarch64_neon_fcvtns, AddRetType | Add1ArgType),
  NEONMAP1(vcvtns_u32_f32, aarch64_neon_fcvtnu, AddRetType | Add1ArgType),
  NEONMAP1(vcvtpd_s64_f64, aarch64_neon_fcvtps, AddRetType | Add1ArgType),
  NEONMAP1(vcvtpd_u64_f64, aarch64_neon_fcvtpu, AddRetType | Add1ArgType),
  NEONMAP1(vcvtps_s32_f32, aarch64_neon_fcvtps, AddRetType | Add1ArgType),
  NEONMAP1(vcvtps_u32_f32, aarch64_neon_fcvtpu, AddRetType | Add1ArgType),
  NEONMAP1(vcvts_n_f32_s32, aarch64_neon_vcvtfxs2fp, AddRetType | Add1ArgType),
  NEONMAP1(vcvts_n_f32_u32, aarch64_neon_vcvtfxu2fp, AddRetType | Add1ArgType),
  NEONMAP1(vcvts_n_s32_f32, aarch64_neon_vcvtfp2fxs, AddRetType | Add1ArgType),
  NEONMAP1(vcvts_n_u32_f32, aarch64_neon_vcvtfp2fxu, AddRetType | Add1ArgType),
  NEONMAP1(vcvtxd_f32_f64, aarch64_sisd_fcvtxn, 0),
  NEONMAP1(vmaxnmv_f32, aarch64_neon_fmaxnmv, AddRetType | Add1ArgType),
  NEONMAP1(vmaxnmvq_f32, aarch64_neon_fmaxnmv, AddRetType | Add1ArgType),
  NEONMAP1(vmaxnmvq_f64, aarch64_neon_fmaxnmv, AddRetType | Add1ArgType),
  NEONMAP1(vmaxv_f32, aarch64_neon_fmaxv, AddRetType | Add1ArgType),
  NEONMAP1(vmaxv_s32, aarch64_neon_smaxv, AddRetType | Add1ArgType),
  NEONMAP1(vmaxv_u32, aarch64_neon_umaxv, AddRetType | Add1ArgType),
  NEONMAP1(vmaxvq_f32, aarch64_neon_fmaxv, AddRetType | Add1ArgType),
  NEONMAP1(vmaxvq_f64, aarch64_neon_fmaxv, AddRetType | Add1ArgType),
  NEONMAP1(vmaxvq_s32, aarch64_neon_smaxv, AddRetType | Add1ArgType),
  NEONMAP1(vmaxvq_u32, aarch64_neon_umaxv, AddRetType | Add1ArgType),
  NEONMAP1(vminnmv_f32, aarch64_neon_fminnmv, AddRetType | Add1ArgType),
  NEONMAP1(vminnmvq_f32, aarch64_neon_fminnmv, AddRetType | Add1ArgType),
  NEONMAP1(vminnmvq_f64, aarch64_neon_fminnmv, AddRetType | Add1ArgType),
  NEONMAP1(vminv_f32, aarch64_neon_fminv, AddRetType | Add1ArgType),
  NEONMAP1(vminv_s32, aarch64_neon_sminv, AddRetType | Add1ArgType),
  NEONMAP1(vminv_u32, aarch64_neon_uminv, AddRetType | Add1ArgType),
  NEONMAP1(vminvq_f32, aarch64_neon_fminv, AddRetType | Add1ArgType),
  NEONMAP1(vminvq_f64, aarch64_neon_fminv, AddRetType | Add1ArgType),
  NEONMAP1(vminvq_s32, aarch64_neon_sminv, AddRetType | Add1ArgType),
  NEONMAP1(vminvq_u32, aarch64_neon_uminv, AddRetType | Add1ArgType),
  NEONMAP1(vmull_p64, aarch64_neon_pmull64, 0),
  NEONMAP1(vmulxd_f64, aarch64_neon_fmulx, Add1ArgType),
  NEONMAP1(vmulxs_f32, aarch64_neon_fmulx, Add1ArgType),
  NEONMAP1(vpaddd_s64, aarch64_neon_uaddv, AddRetType | Add1ArgType),
  NEONMAP1(vpaddd_u64, aarch64_neon_uaddv, AddRetType | Add1ArgType),
  NEONMAP1(vpmaxnmqd_f64, aarch64_neon_fmaxnmv, AddRetType | Add1ArgType),
  NEONMAP1(vpmaxnms_f32, aarch64_neon_fmaxnmv, AddRetType | Add1ArgType),
  NEONMAP1(vpmaxqd_f64, aarch64_neon_fmaxv, AddRetType | Add1ArgType),
  NEONMAP1(vpmaxs_f32, aarch64_neon_fmaxv, AddRetType | Add1ArgType),
  NEONMAP1(vpminnmqd_f64, aarch64_neon_fminnmv, AddRetType | Add1ArgType),
  NEONMAP1(vpminnms_f32, aarch64_neon_fminnmv, AddRetType | Add1ArgType),
  NEONMAP1(vpminqd_f64, aarch64_neon_fminv, AddRetType | Add1ArgType),
  NEONMAP1(vpmins_f32, aarch64_neon_fminv, AddRetType | Add1ArgType),
  NEONMAP1(vqabsb_s8, aarch64_neon_sqabs, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqabsd_s64, aarch64_neon_sqabs, Add1ArgType),
  NEONMAP1(vqabsh_s16, aarch64_neon_sqabs, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqabss_s32, aarch64_neon_sqabs, Add1ArgType),
  NEONMAP1(vqaddb_s8, aarch64_neon_sqadd, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqaddb_u8, aarch64_neon_uqadd, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqaddd_s64, aarch64_neon_sqadd, Add1ArgType),
  NEONMAP1(vqaddd_u64, aarch64_neon_uqadd, Add1ArgType),
  NEONMAP1(vqaddh_s16, aarch64_neon_sqadd, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqaddh_u16, aarch64_neon_uqadd, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqadds_s32, aarch64_neon_sqadd, Add1ArgType),
  NEONMAP1(vqadds_u32, aarch64_neon_uqadd, Add1ArgType),
  NEONMAP1(vqdmulhh_s16, aarch64_neon_sqdmulh, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqdmulhs_s32, aarch64_neon_sqdmulh, Add1ArgType),
  NEONMAP1(vqdmullh_s16, aarch64_neon_sqdmull, VectorRet | Use128BitVectors),
  NEONMAP1(vqdmulls_s32, aarch64_neon_sqdmulls_scalar, 0),
  NEONMAP1(vqmovnd_s64, aarch64_neon_scalar_sqxtn, AddRetType | Add1ArgType),
  NEONMAP1(vqmovnd_u64, aarch64_neon_scalar_uqxtn, AddRetType | Add1ArgType),
  NEONMAP1(vqmovnh_s16, aarch64_neon_sqxtn, VectorRet | Use64BitVectors),
  NEONMAP1(vqmovnh_u16, aarch64_neon_uqxtn, VectorRet | Use64BitVectors),
  NEONMAP1(vqmovns_s32, aarch64_neon_sqxtn, VectorRet | Use64BitVectors),
  NEONMAP1(vqmovns_u32, aarch64_neon_uqxtn, VectorRet | Use64BitVectors),
  NEONMAP1(vqmovund_s64, aarch64_neon_scalar_sqxtun, AddRetType | Add1ArgType),
  NEONMAP1(vqmovunh_s16, aarch64_neon_sqxtun, VectorRet | Use64BitVectors),
  NEONMAP1(vqmovuns_s32, aarch64_neon_sqxtun, VectorRet | Use64BitVectors),
  NEONMAP1(vqnegb_s8, aarch64_neon_sqneg, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqnegd_s64, aarch64_neon_sqneg, Add1ArgType),
  NEONMAP1(vqnegh_s16, aarch64_neon_sqneg, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqnegs_s32, aarch64_neon_sqneg, Add1ArgType),
  NEONMAP1(vqrdmulhh_s16, aarch64_neon_sqrdmulh, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqrdmulhs_s32, aarch64_neon_sqrdmulh, Add1ArgType),
  NEONMAP1(vqrshlb_s8, aarch64_neon_sqrshl, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqrshlb_u8, aarch64_neon_uqrshl, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqrshld_s64, aarch64_neon_sqrshl, Add1ArgType),
  NEONMAP1(vqrshld_u64, aarch64_neon_uqrshl, Add1ArgType),
  NEONMAP1(vqrshlh_s16, aarch64_neon_sqrshl, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqrshlh_u16, aarch64_neon_uqrshl, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqrshls_s32, aarch64_neon_sqrshl, Add1ArgType),
  NEONMAP1(vqrshls_u32, aarch64_neon_uqrshl, Add1ArgType),
  NEONMAP1(vqrshrnd_n_s64, aarch64_neon_sqrshrn, AddRetType),
  NEONMAP1(vqrshrnd_n_u64, aarch64_neon_uqrshrn, AddRetType),
  NEONMAP1(vqrshrnh_n_s16, aarch64_neon_sqrshrn, VectorRet | Use64BitVectors),
  NEONMAP1(vqrshrnh_n_u16, aarch64_neon_uqrshrn, VectorRet | Use64BitVectors),
  NEONMAP1(vqrshrns_n_s32, aarch64_neon_sqrshrn, VectorRet | Use64BitVectors),
  NEONMAP1(vqrshrns_n_u32, aarch64_neon_uqrshrn, VectorRet | Use64BitVectors),
  NEONMAP1(vqrshrund_n_s64, aarch64_neon_sqrshrun, AddRetType),
  NEONMAP1(vqrshrunh_n_s16, aarch64_neon_sqrshrun, VectorRet | Use64BitVectors),
  NEONMAP1(vqrshruns_n_s32, aarch64_neon_sqrshrun, VectorRet | Use64BitVectors),
  NEONMAP1(vqshlb_n_s8, aarch64_neon_sqshl, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqshlb_n_u8, aarch64_neon_uqshl, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqshlb_s8, aarch64_neon_sqshl, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqshlb_u8, aarch64_neon_uqshl, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqshld_s64, aarch64_neon_sqshl, Add1ArgType),
  NEONMAP1(vqshld_u64, aarch64_neon_uqshl, Add1ArgType),
  NEONMAP1(vqshlh_n_s16, aarch64_neon_sqshl, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqshlh_n_u16, aarch64_neon_uqshl, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqshlh_s16, aarch64_neon_sqshl, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqshlh_u16, aarch64_neon_uqshl, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqshls_n_s32, aarch64_neon_sqshl, Add1ArgType),
  NEONMAP1(vqshls_n_u32, aarch64_neon_uqshl, Add1ArgType),
  NEONMAP1(vqshls_s32, aarch64_neon_sqshl, Add1ArgType),
  NEONMAP1(vqshls_u32, aarch64_neon_uqshl, Add1ArgType),
  NEONMAP1(vqshlub_n_s8, aarch64_neon_sqshlu, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqshluh_n_s16, aarch64_neon_sqshlu, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqshlus_n_s32, aarch64_neon_sqshlu, Add1ArgType),
  NEONMAP1(vqshrnd_n_s64, aarch64_neon_sqshrn, AddRetType),
  NEONMAP1(vqshrnd_n_u64, aarch64_neon_uqshrn, AddRetType),
  NEONMAP1(vqshrnh_n_s16, aarch64_neon_sqshrn, VectorRet | Use64BitVectors),
  NEONMAP1(vqshrnh_n_u16, aarch64_neon_uqshrn, VectorRet | Use64BitVectors),
  NEONMAP1(vqshrns_n_s32, aarch64_neon_sqshrn, VectorRet | Use64BitVectors),
  NEONMAP1(vqshrns_n_u32, aarch64_neon_uqshrn, VectorRet | Use64BitVectors),
  NEONMAP1(vqshrund_n_s64, aarch64_neon_sqshrun, AddRetType),
  NEONMAP1(vqshrunh_n_s16, aarch64_neon_sqshrun, VectorRet | Use64BitVectors),
  NEONMAP1(vqshruns_n_s32, aarch64_neon_sqshrun, VectorRet | Use64BitVectors),
  NEONMAP1(vqsubb_s8, aarch64_neon_sqsub, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqsubb_u8, aarch64_neon_uqsub, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqsubd_s64, aarch64_neon_sqsub, Add1ArgType),
  NEONMAP1(vqsubd_u64, aarch64_neon_uqsub, Add1ArgType),
  NEONMAP1(vqsubh_s16, aarch64_neon_sqsub, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqsubh_u16, aarch64_neon_uqsub, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vqsubs_s32, aarch64_neon_sqsub, Add1ArgType),
  NEONMAP1(vqsubs_u32, aarch64_neon_uqsub, Add1ArgType),
  NEONMAP1(vrecped_f64, aarch64_neon_frecpe, Add1ArgType),
  NEONMAP1(vrecpes_f32, aarch64_neon_frecpe, Add1ArgType),
  NEONMAP1(vrecpxd_f64, aarch64_neon_frecpx, Add1ArgType),
  NEONMAP1(vrecpxs_f32, aarch64_neon_frecpx, Add1ArgType),
  NEONMAP1(vrshld_s64, aarch64_neon_srshl, Add1ArgType),
  NEONMAP1(vrshld_u64, aarch64_neon_urshl, Add1ArgType),
  NEONMAP1(vrsqrted_f64, aarch64_neon_frsqrte, Add1ArgType),
  NEONMAP1(vrsqrtes_f32, aarch64_neon_frsqrte, Add1ArgType),
  NEONMAP1(vrsqrtsd_f64, aarch64_neon_frsqrts, Add1ArgType),
  NEONMAP1(vrsqrtss_f32, aarch64_neon_frsqrts, Add1ArgType),
  NEONMAP1(vsha1cq_u32, aarch64_crypto_sha1c, 0),
  NEONMAP1(vsha1h_u32, aarch64_crypto_sha1h, 0),
  NEONMAP1(vsha1mq_u32, aarch64_crypto_sha1m, 0),
  NEONMAP1(vsha1pq_u32, aarch64_crypto_sha1p, 0),
  NEONMAP1(vshld_s64, aarch64_neon_sshl, Add1ArgType),
  NEONMAP1(vshld_u64, aarch64_neon_ushl, Add1ArgType),
  NEONMAP1(vslid_n_s64, aarch64_neon_vsli, Vectorize1ArgType),
  NEONMAP1(vslid_n_u64, aarch64_neon_vsli, Vectorize1ArgType),
  NEONMAP1(vsqaddb_u8, aarch64_neon_usqadd, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vsqaddd_u64, aarch64_neon_usqadd, Add1ArgType),
  NEONMAP1(vsqaddh_u16, aarch64_neon_usqadd, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vsqadds_u32, aarch64_neon_usqadd, Add1ArgType),
  NEONMAP1(vsrid_n_s64, aarch64_neon_vsri, Vectorize1ArgType),
  NEONMAP1(vsrid_n_u64, aarch64_neon_vsri, Vectorize1ArgType),
  NEONMAP1(vuqaddb_s8, aarch64_neon_suqadd, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vuqaddd_s64, aarch64_neon_suqadd, Add1ArgType),
  NEONMAP1(vuqaddh_s16, aarch64_neon_suqadd, Vectorize1ArgType | Use64BitVectors),
  NEONMAP1(vuqadds_s32, aarch64_neon_suqadd, Add1ArgType),
};

#undef NEONMAP0
#undef NEONMAP1
#undef NEONMAP2

static bool NEONSIMDIntrinsicsProvenSorted = false;

static bool AArch64SIMDIntrinsicsProvenSorted = false;
static bool AArch64SISDIntrinsicsProvenSorted = false;


static const NeonIntrinsicInfo *
findNeonIntrinsicInMap(ArrayRef<NeonIntrinsicInfo> IntrinsicMap,
                       unsigned BuiltinID, bool &MapProvenSorted) {

#ifndef NDEBUG
  if (!MapProvenSorted) {
    // FIXME: use std::is_sorted once C++11 is allowed
    for (unsigned i = 0; i < IntrinsicMap.size() - 1; ++i)
      assert(IntrinsicMap[i].BuiltinID <= IntrinsicMap[i + 1].BuiltinID);
    MapProvenSorted = true;
  }
#endif

  const NeonIntrinsicInfo *Builtin =
      std::lower_bound(IntrinsicMap.begin(), IntrinsicMap.end(), BuiltinID);

  if (Builtin != IntrinsicMap.end() && Builtin->BuiltinID == BuiltinID)
    return Builtin;

  return nullptr;
}

Function *CodeGenFunction::LookupNeonLLVMIntrinsic(unsigned IntrinsicID,
                                                   unsigned Modifier,
                                                   llvm::Type *ArgType,
                                                   const CallExpr *E) {
  int VectorSize = 0;
  if (Modifier & Use64BitVectors)
    VectorSize = 64;
  else if (Modifier & Use128BitVectors)
    VectorSize = 128;

  // Return type.
  SmallVector<llvm::Type *, 3> Tys;
  if (Modifier & AddRetType) {
    llvm::Type *Ty = ConvertType(E->getCallReturnType(getContext()));
    if (Modifier & VectorizeRetType)
      Ty = llvm::VectorType::get(
          Ty, VectorSize ? VectorSize / Ty->getPrimitiveSizeInBits() : 1);

    Tys.push_back(Ty);
  }

  // Arguments.
  if (Modifier & VectorizeArgTypes) {
    int Elts = VectorSize ? VectorSize / ArgType->getPrimitiveSizeInBits() : 1;
    ArgType = llvm::VectorType::get(ArgType, Elts);
  }

  if (Modifier & (Add1ArgType | Add2ArgTypes))
    Tys.push_back(ArgType);

  if (Modifier & Add2ArgTypes)
    Tys.push_back(ArgType);

  if (Modifier & InventFloatType)
    Tys.push_back(FloatTy);

  return CGM.getIntrinsic(IntrinsicID, Tys);
}

static Value *EmitCommonNeonSISDBuiltinExpr(CodeGenFunction &CGF,
                                            const NeonIntrinsicInfo &SISDInfo,
                                            SmallVectorImpl<Value *> &Ops,
                                            const CallExpr *E) {
  unsigned BuiltinID = SISDInfo.BuiltinID;
  unsigned int Int = SISDInfo.LLVMIntrinsic;
  unsigned Modifier = SISDInfo.TypeModifier;
  const char *s = SISDInfo.NameHint;

  switch (BuiltinID) {
  case NEON::BI__builtin_neon_vcled_s64:
  case NEON::BI__builtin_neon_vcled_u64:
  case NEON::BI__builtin_neon_vcles_f32:
  case NEON::BI__builtin_neon_vcled_f64:
  case NEON::BI__builtin_neon_vcltd_s64:
  case NEON::BI__builtin_neon_vcltd_u64:
  case NEON::BI__builtin_neon_vclts_f32:
  case NEON::BI__builtin_neon_vcltd_f64:
  case NEON::BI__builtin_neon_vcales_f32:
  case NEON::BI__builtin_neon_vcaled_f64:
  case NEON::BI__builtin_neon_vcalts_f32:
  case NEON::BI__builtin_neon_vcaltd_f64:
    // Only one direction of comparisons actually exist, cmle is actually a cmge
    // with swapped operands. The table gives us the right intrinsic but we
    // still need to do the swap.
    std::swap(Ops[0], Ops[1]);
    break;
  }

  assert(Int && "Generic code assumes a valid intrinsic");

  // Determine the type(s) of this overloaded AArch64 intrinsic.
  const Expr *Arg = E->getArg(0);
  llvm::Type *ArgTy = CGF.ConvertType(Arg->getType());
  Function *F = CGF.LookupNeonLLVMIntrinsic(Int, Modifier, ArgTy, E);

  int j = 0;
  ConstantInt *C0 = ConstantInt::get(CGF.SizeTy, 0);
  for (Function::const_arg_iterator ai = F->arg_begin(), ae = F->arg_end();
       ai != ae; ++ai, ++j) {
    llvm::Type *ArgTy = ai->getType();
    if (Ops[j]->getType()->getPrimitiveSizeInBits() ==
             ArgTy->getPrimitiveSizeInBits())
      continue;

    assert(ArgTy->isVectorTy() && !Ops[j]->getType()->isVectorTy());
    // The constant argument to an _n_ intrinsic always has Int32Ty, so truncate
    // it before inserting.
    Ops[j] =
        CGF.Builder.CreateTruncOrBitCast(Ops[j], ArgTy->getVectorElementType());
    Ops[j] =
        CGF.Builder.CreateInsertElement(UndefValue::get(ArgTy), Ops[j], C0);
  }

  Value *Result = CGF.EmitNeonCall(F, Ops, s);
  llvm::Type *ResultType = CGF.ConvertType(E->getType());
  if (ResultType->getPrimitiveSizeInBits() <
      Result->getType()->getPrimitiveSizeInBits())
    return CGF.Builder.CreateExtractElement(Result, C0);

  return CGF.Builder.CreateBitCast(Result, ResultType, s);
}

Value *CodeGenFunction::EmitCommonNeonBuiltinExpr(
    unsigned BuiltinID, unsigned LLVMIntrinsic, unsigned AltLLVMIntrinsic,
    const char *NameHint, unsigned Modifier, const CallExpr *E,
    SmallVectorImpl<llvm::Value *> &Ops, llvm::Value *Align) {
  // Get the last argument, which specifies the vector type.
  llvm::APSInt NeonTypeConst;
  const Expr *Arg = E->getArg(E->getNumArgs() - 1);
  if (!Arg->isIntegerConstantExpr(NeonTypeConst, getContext()))
    return nullptr;

  // Determine the type of this overloaded NEON intrinsic.
  NeonTypeFlags Type(NeonTypeConst.getZExtValue());
  bool Usgn = Type.isUnsigned();
  bool Quad = Type.isQuad();

  llvm::VectorType *VTy = GetNeonType(this, Type);
  llvm::Type *Ty = VTy;
  if (!Ty)
    return nullptr;

  unsigned Int = LLVMIntrinsic;
  if ((Modifier & UnsignedAlts) && !Usgn)
    Int = AltLLVMIntrinsic;

  switch (BuiltinID) {
  default: break;
  case NEON::BI__builtin_neon_vabs_v:
  case NEON::BI__builtin_neon_vabsq_v:
    if (VTy->getElementType()->isFloatingPointTy())
      return EmitNeonCall(CGM.getIntrinsic(Intrinsic::fabs, Ty), Ops, "vabs");
    return EmitNeonCall(CGM.getIntrinsic(LLVMIntrinsic, Ty), Ops, "vabs");
  case NEON::BI__builtin_neon_vaddhn_v: {
    llvm::VectorType *SrcTy =
        llvm::VectorType::getExtendedElementVectorType(VTy);

    // %sum = add <4 x i32> %lhs, %rhs
    Ops[0] = Builder.CreateBitCast(Ops[0], SrcTy);
    Ops[1] = Builder.CreateBitCast(Ops[1], SrcTy);
    Ops[0] = Builder.CreateAdd(Ops[0], Ops[1], "vaddhn");

    // %high = lshr <4 x i32> %sum, <i32 16, i32 16, i32 16, i32 16>
    Constant *ShiftAmt = ConstantInt::get(SrcTy->getElementType(),
                                       SrcTy->getScalarSizeInBits() / 2);
    ShiftAmt = ConstantVector::getSplat(VTy->getNumElements(), ShiftAmt);
    Ops[0] = Builder.CreateLShr(Ops[0], ShiftAmt, "vaddhn");

    // %res = trunc <4 x i32> %high to <4 x i16>
    return Builder.CreateTrunc(Ops[0], VTy, "vaddhn");
  }
  case NEON::BI__builtin_neon_vcale_v:
  case NEON::BI__builtin_neon_vcaleq_v:
  case NEON::BI__builtin_neon_vcalt_v:
  case NEON::BI__builtin_neon_vcaltq_v:
    std::swap(Ops[0], Ops[1]);
  case NEON::BI__builtin_neon_vcage_v:
  case NEON::BI__builtin_neon_vcageq_v:
  case NEON::BI__builtin_neon_vcagt_v:
  case NEON::BI__builtin_neon_vcagtq_v: {
    llvm::Type *VecFlt = llvm::VectorType::get(
        VTy->getScalarSizeInBits() == 32 ? FloatTy : DoubleTy,
        VTy->getNumElements());
    llvm::Type *Tys[] = { VTy, VecFlt };
    Function *F = CGM.getIntrinsic(LLVMIntrinsic, Tys);
    return EmitNeonCall(F, Ops, NameHint);
  }
  case NEON::BI__builtin_neon_vclz_v:
  case NEON::BI__builtin_neon_vclzq_v:
    // We generate target-independent intrinsic, which needs a second argument
    // for whether or not clz of zero is undefined; on ARM it isn't.
    Ops.push_back(Builder.getInt1(getTarget().isCLZForZeroUndef()));
    break;
  case NEON::BI__builtin_neon_vcvt_f32_v:
  case NEON::BI__builtin_neon_vcvtq_f32_v:
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    Ty = GetNeonType(this, NeonTypeFlags(NeonTypeFlags::Float32, false, Quad));
    return Usgn ? Builder.CreateUIToFP(Ops[0], Ty, "vcvt")
                : Builder.CreateSIToFP(Ops[0], Ty, "vcvt");
  case NEON::BI__builtin_neon_vcvt_n_f32_v:
  case NEON::BI__builtin_neon_vcvt_n_f64_v:
  case NEON::BI__builtin_neon_vcvtq_n_f32_v:
  case NEON::BI__builtin_neon_vcvtq_n_f64_v: {
    bool Double =
      (cast<llvm::IntegerType>(VTy->getElementType())->getBitWidth() == 64);
    llvm::Type *FloatTy =
        GetNeonType(this, NeonTypeFlags(Double ? NeonTypeFlags::Float64
                                               : NeonTypeFlags::Float32,
                                        false, Quad));
    llvm::Type *Tys[2] = { FloatTy, Ty };
    Int = Usgn ? LLVMIntrinsic : AltLLVMIntrinsic;
    Function *F = CGM.getIntrinsic(Int, Tys);
    return EmitNeonCall(F, Ops, "vcvt_n");
  }
  case NEON::BI__builtin_neon_vcvt_n_s32_v:
  case NEON::BI__builtin_neon_vcvt_n_u32_v:
  case NEON::BI__builtin_neon_vcvt_n_s64_v:
  case NEON::BI__builtin_neon_vcvt_n_u64_v:
  case NEON::BI__builtin_neon_vcvtq_n_s32_v:
  case NEON::BI__builtin_neon_vcvtq_n_u32_v:
  case NEON::BI__builtin_neon_vcvtq_n_s64_v:
  case NEON::BI__builtin_neon_vcvtq_n_u64_v: {
    bool Double =
      (cast<llvm::IntegerType>(VTy->getElementType())->getBitWidth() == 64);
    llvm::Type *FloatTy =
        GetNeonType(this, NeonTypeFlags(Double ? NeonTypeFlags::Float64
                                               : NeonTypeFlags::Float32,
                                        false, Quad));
    llvm::Type *Tys[2] = { Ty, FloatTy };
    Function *F = CGM.getIntrinsic(LLVMIntrinsic, Tys);
    return EmitNeonCall(F, Ops, "vcvt_n");
  }
  case NEON::BI__builtin_neon_vcvt_s32_v:
  case NEON::BI__builtin_neon_vcvt_u32_v:
  case NEON::BI__builtin_neon_vcvt_s64_v:
  case NEON::BI__builtin_neon_vcvt_u64_v:
  case NEON::BI__builtin_neon_vcvtq_s32_v:
  case NEON::BI__builtin_neon_vcvtq_u32_v:
  case NEON::BI__builtin_neon_vcvtq_s64_v:
  case NEON::BI__builtin_neon_vcvtq_u64_v: {
    bool Double =
      (cast<llvm::IntegerType>(VTy->getElementType())->getBitWidth() == 64);
    llvm::Type *FloatTy =
        GetNeonType(this, NeonTypeFlags(Double ? NeonTypeFlags::Float64
                                               : NeonTypeFlags::Float32,
                                        false, Quad));
    Ops[0] = Builder.CreateBitCast(Ops[0], FloatTy);
    return Usgn ? Builder.CreateFPToUI(Ops[0], Ty, "vcvt")
                : Builder.CreateFPToSI(Ops[0], Ty, "vcvt");
  }
  case NEON::BI__builtin_neon_vcvta_s32_v:
  case NEON::BI__builtin_neon_vcvta_s64_v:
  case NEON::BI__builtin_neon_vcvta_u32_v:
  case NEON::BI__builtin_neon_vcvta_u64_v:
  case NEON::BI__builtin_neon_vcvtaq_s32_v:
  case NEON::BI__builtin_neon_vcvtaq_s64_v:
  case NEON::BI__builtin_neon_vcvtaq_u32_v:
  case NEON::BI__builtin_neon_vcvtaq_u64_v:
  case NEON::BI__builtin_neon_vcvtn_s32_v:
  case NEON::BI__builtin_neon_vcvtn_s64_v:
  case NEON::BI__builtin_neon_vcvtn_u32_v:
  case NEON::BI__builtin_neon_vcvtn_u64_v:
  case NEON::BI__builtin_neon_vcvtnq_s32_v:
  case NEON::BI__builtin_neon_vcvtnq_s64_v:
  case NEON::BI__builtin_neon_vcvtnq_u32_v:
  case NEON::BI__builtin_neon_vcvtnq_u64_v:
  case NEON::BI__builtin_neon_vcvtp_s32_v:
  case NEON::BI__builtin_neon_vcvtp_s64_v:
  case NEON::BI__builtin_neon_vcvtp_u32_v:
  case NEON::BI__builtin_neon_vcvtp_u64_v:
  case NEON::BI__builtin_neon_vcvtpq_s32_v:
  case NEON::BI__builtin_neon_vcvtpq_s64_v:
  case NEON::BI__builtin_neon_vcvtpq_u32_v:
  case NEON::BI__builtin_neon_vcvtpq_u64_v:
  case NEON::BI__builtin_neon_vcvtm_s32_v:
  case NEON::BI__builtin_neon_vcvtm_s64_v:
  case NEON::BI__builtin_neon_vcvtm_u32_v:
  case NEON::BI__builtin_neon_vcvtm_u64_v:
  case NEON::BI__builtin_neon_vcvtmq_s32_v:
  case NEON::BI__builtin_neon_vcvtmq_s64_v:
  case NEON::BI__builtin_neon_vcvtmq_u32_v:
  case NEON::BI__builtin_neon_vcvtmq_u64_v: {
    bool Double =
      (cast<llvm::IntegerType>(VTy->getElementType())->getBitWidth() == 64);
    llvm::Type *InTy =
      GetNeonType(this,
                  NeonTypeFlags(Double ? NeonTypeFlags::Float64
                                : NeonTypeFlags::Float32, false, Quad));
    llvm::Type *Tys[2] = { Ty, InTy };
    return EmitNeonCall(CGM.getIntrinsic(LLVMIntrinsic, Tys), Ops, NameHint);
  }
  case NEON::BI__builtin_neon_vext_v:
  case NEON::BI__builtin_neon_vextq_v: {
    int CV = cast<ConstantInt>(Ops[2])->getSExtValue();
    SmallVector<Constant*, 16> Indices;
    for (unsigned i = 0, e = VTy->getNumElements(); i != e; ++i)
      Indices.push_back(ConstantInt::get(Int32Ty, i+CV));

    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Value *SV = llvm::ConstantVector::get(Indices);
    return Builder.CreateShuffleVector(Ops[0], Ops[1], SV, "vext");
  }
  case NEON::BI__builtin_neon_vfma_v:
  case NEON::BI__builtin_neon_vfmaq_v: {
    Value *F = CGM.getIntrinsic(Intrinsic::fma, Ty);
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Ops[2] = Builder.CreateBitCast(Ops[2], Ty);

    // NEON intrinsic puts accumulator first, unlike the LLVM fma.
    return Builder.CreateCall(F, {Ops[1], Ops[2], Ops[0]});
  }
  case NEON::BI__builtin_neon_vld1_v:
  case NEON::BI__builtin_neon_vld1q_v:
    Ops.push_back(Align);
    return EmitNeonCall(CGM.getIntrinsic(LLVMIntrinsic, Ty), Ops, "vld1");
  case NEON::BI__builtin_neon_vld2_v:
  case NEON::BI__builtin_neon_vld2q_v:
  case NEON::BI__builtin_neon_vld3_v:
  case NEON::BI__builtin_neon_vld3q_v:
  case NEON::BI__builtin_neon_vld4_v:
  case NEON::BI__builtin_neon_vld4q_v: {
    Function *F = CGM.getIntrinsic(LLVMIntrinsic, Ty);
    Ops[1] = Builder.CreateCall(F, {Ops[1], Align}, NameHint);
    Ty = llvm::PointerType::getUnqual(Ops[1]->getType());
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    return Builder.CreateStore(Ops[1], Ops[0]);
  }
  case NEON::BI__builtin_neon_vld1_dup_v:
  case NEON::BI__builtin_neon_vld1q_dup_v: {
    Value *V = UndefValue::get(Ty);
    Ty = llvm::PointerType::getUnqual(VTy->getElementType());
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    LoadInst *Ld = Builder.CreateLoad(Ops[0]);
    Ld->setAlignment(cast<ConstantInt>(Align)->getZExtValue());
    llvm::Constant *CI = ConstantInt::get(SizeTy, 0);
    Ops[0] = Builder.CreateInsertElement(V, Ld, CI);
    return EmitNeonSplat(Ops[0], CI);
  }
  case NEON::BI__builtin_neon_vld2_lane_v:
  case NEON::BI__builtin_neon_vld2q_lane_v:
  case NEON::BI__builtin_neon_vld3_lane_v:
  case NEON::BI__builtin_neon_vld3q_lane_v:
  case NEON::BI__builtin_neon_vld4_lane_v:
  case NEON::BI__builtin_neon_vld4q_lane_v: {
    Function *F = CGM.getIntrinsic(LLVMIntrinsic, Ty);
    for (unsigned I = 2; I < Ops.size() - 1; ++I)
      Ops[I] = Builder.CreateBitCast(Ops[I], Ty);
    Ops.push_back(Align);
    Ops[1] = Builder.CreateCall(F, makeArrayRef(Ops).slice(1), NameHint);
    Ty = llvm::PointerType::getUnqual(Ops[1]->getType());
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    return Builder.CreateStore(Ops[1], Ops[0]);
  }
  case NEON::BI__builtin_neon_vmovl_v: {
    llvm::Type *DTy =llvm::VectorType::getTruncatedElementVectorType(VTy);
    Ops[0] = Builder.CreateBitCast(Ops[0], DTy);
    if (Usgn)
      return Builder.CreateZExt(Ops[0], Ty, "vmovl");
    return Builder.CreateSExt(Ops[0], Ty, "vmovl");
  }
  case NEON::BI__builtin_neon_vmovn_v: {
    llvm::Type *QTy = llvm::VectorType::getExtendedElementVectorType(VTy);
    Ops[0] = Builder.CreateBitCast(Ops[0], QTy);
    return Builder.CreateTrunc(Ops[0], Ty, "vmovn");
  }
  case NEON::BI__builtin_neon_vmull_v:
    // FIXME: the integer vmull operations could be emitted in terms of pure
    // LLVM IR (2 exts followed by a mul). Unfortunately LLVM has a habit of
    // hoisting the exts outside loops. Until global ISel comes along that can
    // see through such movement this leads to bad CodeGen. So we need an
    // intrinsic for now.
    Int = Usgn ? Intrinsic::arm_neon_vmullu : Intrinsic::arm_neon_vmulls;
    Int = Type.isPoly() ? (unsigned)Intrinsic::arm_neon_vmullp : Int;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vmull");
  case NEON::BI__builtin_neon_vpadal_v:
  case NEON::BI__builtin_neon_vpadalq_v: {
    // The source operand type has twice as many elements of half the size.
    unsigned EltBits = VTy->getElementType()->getPrimitiveSizeInBits();
    llvm::Type *EltTy =
      llvm::IntegerType::get(getLLVMContext(), EltBits / 2);
    llvm::Type *NarrowTy =
      llvm::VectorType::get(EltTy, VTy->getNumElements() * 2);
    llvm::Type *Tys[2] = { Ty, NarrowTy };
    return EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, NameHint);
  }
  case NEON::BI__builtin_neon_vpaddl_v:
  case NEON::BI__builtin_neon_vpaddlq_v: {
    // The source operand type has twice as many elements of half the size.
    unsigned EltBits = VTy->getElementType()->getPrimitiveSizeInBits();
    llvm::Type *EltTy = llvm::IntegerType::get(getLLVMContext(), EltBits / 2);
    llvm::Type *NarrowTy =
      llvm::VectorType::get(EltTy, VTy->getNumElements() * 2);
    llvm::Type *Tys[2] = { Ty, NarrowTy };
    return EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vpaddl");
  }
  case NEON::BI__builtin_neon_vqdmlal_v:
  case NEON::BI__builtin_neon_vqdmlsl_v: {
    SmallVector<Value *, 2> MulOps(Ops.begin() + 1, Ops.end());
    Value *Mul = EmitNeonCall(CGM.getIntrinsic(LLVMIntrinsic, Ty),
                              MulOps, "vqdmlal");

    SmallVector<Value *, 2> AccumOps;
    AccumOps.push_back(Ops[0]);
    AccumOps.push_back(Mul);
    return EmitNeonCall(CGM.getIntrinsic(AltLLVMIntrinsic, Ty),
                        AccumOps, NameHint);
  }
  case NEON::BI__builtin_neon_vqshl_n_v:
  case NEON::BI__builtin_neon_vqshlq_n_v:
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vqshl_n",
                        1, false);
  case NEON::BI__builtin_neon_vqshlu_n_v:
  case NEON::BI__builtin_neon_vqshluq_n_v:
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vqshlu_n",
                        1, false);
  case NEON::BI__builtin_neon_vrecpe_v:
  case NEON::BI__builtin_neon_vrecpeq_v:
  case NEON::BI__builtin_neon_vrsqrte_v:
  case NEON::BI__builtin_neon_vrsqrteq_v:
    Int = Ty->isFPOrFPVectorTy() ? LLVMIntrinsic : AltLLVMIntrinsic;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, NameHint);

  case NEON::BI__builtin_neon_vrshr_n_v:
  case NEON::BI__builtin_neon_vrshrq_n_v:
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vrshr_n",
                        1, true);
  case NEON::BI__builtin_neon_vshl_n_v:
  case NEON::BI__builtin_neon_vshlq_n_v:
    Ops[1] = EmitNeonShiftVector(Ops[1], Ty, false);
    return Builder.CreateShl(Builder.CreateBitCast(Ops[0],Ty), Ops[1],
                             "vshl_n");
  case NEON::BI__builtin_neon_vshll_n_v: {
    llvm::Type *SrcTy = llvm::VectorType::getTruncatedElementVectorType(VTy);
    Ops[0] = Builder.CreateBitCast(Ops[0], SrcTy);
    if (Usgn)
      Ops[0] = Builder.CreateZExt(Ops[0], VTy);
    else
      Ops[0] = Builder.CreateSExt(Ops[0], VTy);
    Ops[1] = EmitNeonShiftVector(Ops[1], VTy, false);
    return Builder.CreateShl(Ops[0], Ops[1], "vshll_n");
  }
  case NEON::BI__builtin_neon_vshrn_n_v: {
    llvm::Type *SrcTy = llvm::VectorType::getExtendedElementVectorType(VTy);
    Ops[0] = Builder.CreateBitCast(Ops[0], SrcTy);
    Ops[1] = EmitNeonShiftVector(Ops[1], SrcTy, false);
    if (Usgn)
      Ops[0] = Builder.CreateLShr(Ops[0], Ops[1]);
    else
      Ops[0] = Builder.CreateAShr(Ops[0], Ops[1]);
    return Builder.CreateTrunc(Ops[0], Ty, "vshrn_n");
  }
  case NEON::BI__builtin_neon_vshr_n_v:
  case NEON::BI__builtin_neon_vshrq_n_v:
    return EmitNeonRShiftImm(Ops[0], Ops[1], Ty, Usgn, "vshr_n");
  case NEON::BI__builtin_neon_vst1_v:
  case NEON::BI__builtin_neon_vst1q_v:
  case NEON::BI__builtin_neon_vst2_v:
  case NEON::BI__builtin_neon_vst2q_v:
  case NEON::BI__builtin_neon_vst3_v:
  case NEON::BI__builtin_neon_vst3q_v:
  case NEON::BI__builtin_neon_vst4_v:
  case NEON::BI__builtin_neon_vst4q_v:
  case NEON::BI__builtin_neon_vst2_lane_v:
  case NEON::BI__builtin_neon_vst2q_lane_v:
  case NEON::BI__builtin_neon_vst3_lane_v:
  case NEON::BI__builtin_neon_vst3q_lane_v:
  case NEON::BI__builtin_neon_vst4_lane_v:
  case NEON::BI__builtin_neon_vst4q_lane_v:
    Ops.push_back(Align);
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "");
  case NEON::BI__builtin_neon_vsubhn_v: {
    llvm::VectorType *SrcTy =
        llvm::VectorType::getExtendedElementVectorType(VTy);

    // %sum = add <4 x i32> %lhs, %rhs
    Ops[0] = Builder.CreateBitCast(Ops[0], SrcTy);
    Ops[1] = Builder.CreateBitCast(Ops[1], SrcTy);
    Ops[0] = Builder.CreateSub(Ops[0], Ops[1], "vsubhn");

    // %high = lshr <4 x i32> %sum, <i32 16, i32 16, i32 16, i32 16>
    Constant *ShiftAmt = ConstantInt::get(SrcTy->getElementType(),
                                       SrcTy->getScalarSizeInBits() / 2);
    ShiftAmt = ConstantVector::getSplat(VTy->getNumElements(), ShiftAmt);
    Ops[0] = Builder.CreateLShr(Ops[0], ShiftAmt, "vsubhn");

    // %res = trunc <4 x i32> %high to <4 x i16>
    return Builder.CreateTrunc(Ops[0], VTy, "vsubhn");
  }
  case NEON::BI__builtin_neon_vtrn_v:
  case NEON::BI__builtin_neon_vtrnq_v: {
    Ops[0] = Builder.CreateBitCast(Ops[0], llvm::PointerType::getUnqual(Ty));
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Ops[2] = Builder.CreateBitCast(Ops[2], Ty);
    Value *SV = nullptr;

    for (unsigned vi = 0; vi != 2; ++vi) {
      SmallVector<Constant*, 16> Indices;
      for (unsigned i = 0, e = VTy->getNumElements(); i != e; i += 2) {
        Indices.push_back(Builder.getInt32(i+vi));
        Indices.push_back(Builder.getInt32(i+e+vi));
      }
      Value *Addr = Builder.CreateConstInBoundsGEP1_32(Ty, Ops[0], vi);
      SV = llvm::ConstantVector::get(Indices);
      SV = Builder.CreateShuffleVector(Ops[1], Ops[2], SV, "vtrn");
      SV = Builder.CreateStore(SV, Addr);
    }
    return SV;
  }
  case NEON::BI__builtin_neon_vtst_v:
  case NEON::BI__builtin_neon_vtstq_v: {
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Ops[0] = Builder.CreateAnd(Ops[0], Ops[1]);
    Ops[0] = Builder.CreateICmp(ICmpInst::ICMP_NE, Ops[0],
                                ConstantAggregateZero::get(Ty));
    return Builder.CreateSExt(Ops[0], Ty, "vtst");
  }
  case NEON::BI__builtin_neon_vuzp_v:
  case NEON::BI__builtin_neon_vuzpq_v: {
    Ops[0] = Builder.CreateBitCast(Ops[0], llvm::PointerType::getUnqual(Ty));
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Ops[2] = Builder.CreateBitCast(Ops[2], Ty);
    Value *SV = nullptr;

    for (unsigned vi = 0; vi != 2; ++vi) {
      SmallVector<Constant*, 16> Indices;
      for (unsigned i = 0, e = VTy->getNumElements(); i != e; ++i)
        Indices.push_back(ConstantInt::get(Int32Ty, 2*i+vi));

      Value *Addr = Builder.CreateConstInBoundsGEP1_32(Ty, Ops[0], vi);
      SV = llvm::ConstantVector::get(Indices);
      SV = Builder.CreateShuffleVector(Ops[1], Ops[2], SV, "vuzp");
      SV = Builder.CreateStore(SV, Addr);
    }
    return SV;
  }
  case NEON::BI__builtin_neon_vzip_v:
  case NEON::BI__builtin_neon_vzipq_v: {
    Ops[0] = Builder.CreateBitCast(Ops[0], llvm::PointerType::getUnqual(Ty));
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Ops[2] = Builder.CreateBitCast(Ops[2], Ty);
    Value *SV = nullptr;

    for (unsigned vi = 0; vi != 2; ++vi) {
      SmallVector<Constant*, 16> Indices;
      for (unsigned i = 0, e = VTy->getNumElements(); i != e; i += 2) {
        Indices.push_back(ConstantInt::get(Int32Ty, (i + vi*e) >> 1));
        Indices.push_back(ConstantInt::get(Int32Ty, ((i + vi*e) >> 1)+e));
      }
      Value *Addr = Builder.CreateConstInBoundsGEP1_32(Ty, Ops[0], vi);
      SV = llvm::ConstantVector::get(Indices);
      SV = Builder.CreateShuffleVector(Ops[1], Ops[2], SV, "vzip");
      SV = Builder.CreateStore(SV, Addr);
    }
    return SV;
  }
  }

  assert(Int && "Expected valid intrinsic number");

  // Determine the type(s) of this overloaded AArch64 intrinsic.
  Function *F = LookupNeonLLVMIntrinsic(Int, Modifier, Ty, E);

  Value *Result = EmitNeonCall(F, Ops, NameHint);
  llvm::Type *ResultType = ConvertType(E->getType());
  // AArch64 intrinsic one-element vector type cast to
  // scalar type expected by the builtin
  return Builder.CreateBitCast(Result, ResultType, NameHint);
}

Value *CodeGenFunction::EmitAArch64CompareBuiltinExpr(
    Value *Op, llvm::Type *Ty, const CmpInst::Predicate Fp,
    const CmpInst::Predicate Ip, const Twine &Name) {
  llvm::Type *OTy = Op->getType();

  // FIXME: this is utterly horrific. We should not be looking at previous
  // codegen context to find out what needs doing. Unfortunately TableGen
  // currently gives us exactly the same calls for vceqz_f32 and vceqz_s32
  // (etc).
  if (BitCastInst *BI = dyn_cast<BitCastInst>(Op))
    OTy = BI->getOperand(0)->getType();

  Op = Builder.CreateBitCast(Op, OTy);
  if (OTy->getScalarType()->isFloatingPointTy()) {
    Op = Builder.CreateFCmp(Fp, Op, Constant::getNullValue(OTy));
  } else {
    Op = Builder.CreateICmp(Ip, Op, Constant::getNullValue(OTy));
  }
  return Builder.CreateSExt(Op, Ty, Name);
}

static Value *packTBLDVectorList(CodeGenFunction &CGF, ArrayRef<Value *> Ops,
                                 Value *ExtOp, Value *IndexOp,
                                 llvm::Type *ResTy, unsigned IntID,
                                 const char *Name) {
  SmallVector<Value *, 2> TblOps;
  if (ExtOp)
    TblOps.push_back(ExtOp);

  // Build a vector containing sequential number like (0, 1, 2, ..., 15)  
  SmallVector<Constant*, 16> Indices;
  llvm::VectorType *TblTy = cast<llvm::VectorType>(Ops[0]->getType());
  for (unsigned i = 0, e = TblTy->getNumElements(); i != e; ++i) {
    Indices.push_back(ConstantInt::get(CGF.Int32Ty, 2*i));
    Indices.push_back(ConstantInt::get(CGF.Int32Ty, 2*i+1));
  }
  Value *SV = llvm::ConstantVector::get(Indices);

  int PairPos = 0, End = Ops.size() - 1;
  while (PairPos < End) {
    TblOps.push_back(CGF.Builder.CreateShuffleVector(Ops[PairPos],
                                                     Ops[PairPos+1], SV, Name));
    PairPos += 2;
  }

  // If there's an odd number of 64-bit lookup table, fill the high 64-bit
  // of the 128-bit lookup table with zero.
  if (PairPos == End) {
    Value *ZeroTbl = ConstantAggregateZero::get(TblTy);
    TblOps.push_back(CGF.Builder.CreateShuffleVector(Ops[PairPos],
                                                     ZeroTbl, SV, Name));
  }

  Function *TblF;
  TblOps.push_back(IndexOp);
  TblF = CGF.CGM.getIntrinsic(IntID, ResTy);
  
  return CGF.EmitNeonCall(TblF, TblOps, Name);
}

Value *CodeGenFunction::GetValueForARMHint(unsigned BuiltinID) {
  switch (BuiltinID) {
  default:
    return nullptr;
  case ARM::BI__builtin_arm_nop:
    return Builder.CreateCall(CGM.getIntrinsic(Intrinsic::arm_hint),
                              llvm::ConstantInt::get(Int32Ty, 0));
  case ARM::BI__builtin_arm_yield:
  case ARM::BI__yield:
    return Builder.CreateCall(CGM.getIntrinsic(Intrinsic::arm_hint),
                              llvm::ConstantInt::get(Int32Ty, 1));
  case ARM::BI__builtin_arm_wfe:
  case ARM::BI__wfe:
    return Builder.CreateCall(CGM.getIntrinsic(Intrinsic::arm_hint),
                              llvm::ConstantInt::get(Int32Ty, 2));
  case ARM::BI__builtin_arm_wfi:
  case ARM::BI__wfi:
    return Builder.CreateCall(CGM.getIntrinsic(Intrinsic::arm_hint),
                              llvm::ConstantInt::get(Int32Ty, 3));
  case ARM::BI__builtin_arm_sev:
  case ARM::BI__sev:
    return Builder.CreateCall(CGM.getIntrinsic(Intrinsic::arm_hint),
                              llvm::ConstantInt::get(Int32Ty, 4));
  case ARM::BI__builtin_arm_sevl:
  case ARM::BI__sevl:
    return Builder.CreateCall(CGM.getIntrinsic(Intrinsic::arm_hint),
                              llvm::ConstantInt::get(Int32Ty, 5));
  }
}

// Generates the IR for the read/write special register builtin,
// ValueType is the type of the value that is to be written or read,
// RegisterType is the type of the register being written to or read from.
static Value *EmitSpecialRegisterBuiltin(CodeGenFunction &CGF,
                                         const CallExpr *E,
                                         llvm::Type *RegisterType,
                                         llvm::Type *ValueType, bool IsRead) {
  // write and register intrinsics only support 32 and 64 bit operations.
  assert((RegisterType->isIntegerTy(32) || RegisterType->isIntegerTy(64))
          && "Unsupported size for register.");

  CodeGen::CGBuilderTy &Builder = CGF.Builder;
  CodeGen::CodeGenModule &CGM = CGF.CGM;
  LLVMContext &Context = CGM.getLLVMContext();

  const Expr *SysRegStrExpr = E->getArg(0)->IgnoreParenCasts();
  StringRef SysReg = cast<StringLiteral>(SysRegStrExpr)->getString();

  llvm::Metadata *Ops[] = { llvm::MDString::get(Context, SysReg) };
  llvm::MDNode *RegName = llvm::MDNode::get(Context, Ops);
  llvm::Value *Metadata = llvm::MetadataAsValue::get(Context, RegName);

  llvm::Type *Types[] = { RegisterType };

  bool MixedTypes = RegisterType->isIntegerTy(64) && ValueType->isIntegerTy(32);
  assert(!(RegisterType->isIntegerTy(32) && ValueType->isIntegerTy(64))
            && "Can't fit 64-bit value in 32-bit register");

  if (IsRead) {
    llvm::Value *F = CGM.getIntrinsic(llvm::Intrinsic::read_register, Types);
    llvm::Value *Call = Builder.CreateCall(F, Metadata);

    if (MixedTypes)
      // Read into 64 bit register and then truncate result to 32 bit.
      return Builder.CreateTrunc(Call, ValueType);

    if (ValueType->isPointerTy())
      // Have i32/i64 result (Call) but want to return a VoidPtrTy (i8*).
      return Builder.CreateIntToPtr(Call, ValueType);

    return Call;
  }

  llvm::Value *F = CGM.getIntrinsic(llvm::Intrinsic::write_register, Types);
  llvm::Value *ArgValue = CGF.EmitScalarExpr(E->getArg(1));
  if (MixedTypes) {
    // Extend 32 bit write value to 64 bit to pass to write.
    ArgValue = Builder.CreateZExt(ArgValue, RegisterType);
    return Builder.CreateCall(F, { Metadata, ArgValue });
  }

  if (ValueType->isPointerTy()) {
    // Have VoidPtrTy ArgValue but want to return an i32/i64.
    ArgValue = Builder.CreatePtrToInt(ArgValue, RegisterType);
    return Builder.CreateCall(F, { Metadata, ArgValue });
  }

  return Builder.CreateCall(F, { Metadata, ArgValue });
}

/// Return true if BuiltinID is an overloaded Neon intrinsic with an extra
/// argument that specifies the vector type.
static bool HasExtraNeonArgument(unsigned BuiltinID) {
  switch (BuiltinID) {
  default: break;
  case NEON::BI__builtin_neon_vget_lane_i8:
  case NEON::BI__builtin_neon_vget_lane_i16:
  case NEON::BI__builtin_neon_vget_lane_i32:
  case NEON::BI__builtin_neon_vget_lane_i64:
  case NEON::BI__builtin_neon_vget_lane_f32:
  case NEON::BI__builtin_neon_vgetq_lane_i8:
  case NEON::BI__builtin_neon_vgetq_lane_i16:
  case NEON::BI__builtin_neon_vgetq_lane_i32:
  case NEON::BI__builtin_neon_vgetq_lane_i64:
  case NEON::BI__builtin_neon_vgetq_lane_f32:
  case NEON::BI__builtin_neon_vset_lane_i8:
  case NEON::BI__builtin_neon_vset_lane_i16:
  case NEON::BI__builtin_neon_vset_lane_i32:
  case NEON::BI__builtin_neon_vset_lane_i64:
  case NEON::BI__builtin_neon_vset_lane_f32:
  case NEON::BI__builtin_neon_vsetq_lane_i8:
  case NEON::BI__builtin_neon_vsetq_lane_i16:
  case NEON::BI__builtin_neon_vsetq_lane_i32:
  case NEON::BI__builtin_neon_vsetq_lane_i64:
  case NEON::BI__builtin_neon_vsetq_lane_f32:
  case NEON::BI__builtin_neon_vsha1h_u32:
  case NEON::BI__builtin_neon_vsha1cq_u32:
  case NEON::BI__builtin_neon_vsha1pq_u32:
  case NEON::BI__builtin_neon_vsha1mq_u32:
  case ARM::BI_MoveToCoprocessor:
  case ARM::BI_MoveToCoprocessor2:
    return false;
  }
  return true;
}

Value *CodeGenFunction::EmitARMBuiltinExpr(unsigned BuiltinID,
                                           const CallExpr *E) {
  if (auto Hint = GetValueForARMHint(BuiltinID))
    return Hint;

  if (BuiltinID == ARM::BI__emit) {
    bool IsThumb = getTarget().getTriple().getArch() == llvm::Triple::thumb;
    llvm::FunctionType *FTy =
        llvm::FunctionType::get(VoidTy, /*Variadic=*/false);

    APSInt Value;
    if (!E->getArg(0)->EvaluateAsInt(Value, CGM.getContext()))
      llvm_unreachable("Sema will ensure that the parameter is constant");

    uint64_t ZExtValue = Value.zextOrTrunc(IsThumb ? 16 : 32).getZExtValue();

    llvm::InlineAsm *Emit =
        IsThumb ? InlineAsm::get(FTy, ".inst.n 0x" + utohexstr(ZExtValue), "",
                                 /*SideEffects=*/true)
                : InlineAsm::get(FTy, ".inst 0x" + utohexstr(ZExtValue), "",
                                 /*SideEffects=*/true);

    return Builder.CreateCall(Emit);
  }

  if (BuiltinID == ARM::BI__builtin_arm_dbg) {
    Value *Option = EmitScalarExpr(E->getArg(0));
    return Builder.CreateCall(CGM.getIntrinsic(Intrinsic::arm_dbg), Option);
  }

  if (BuiltinID == ARM::BI__builtin_arm_prefetch) {
    Value *Address = EmitScalarExpr(E->getArg(0));
    Value *RW      = EmitScalarExpr(E->getArg(1));
    Value *IsData  = EmitScalarExpr(E->getArg(2));

    // Locality is not supported on ARM target
    Value *Locality = llvm::ConstantInt::get(Int32Ty, 3);

    Value *F = CGM.getIntrinsic(Intrinsic::prefetch);
    return Builder.CreateCall(F, {Address, RW, Locality, IsData});
  }

  if (BuiltinID == ARM::BI__builtin_arm_rbit) {
    return Builder.CreateCall(CGM.getIntrinsic(Intrinsic::arm_rbit),
                                               EmitScalarExpr(E->getArg(0)),
                              "rbit");
  }

  if (BuiltinID == ARM::BI__clear_cache) {
    assert(E->getNumArgs() == 2 && "__clear_cache takes 2 arguments");
    const FunctionDecl *FD = E->getDirectCallee();
    SmallVector<Value*, 2> Ops;
    for (unsigned i = 0; i < 2; i++)
      Ops.push_back(EmitScalarExpr(E->getArg(i)));
    llvm::Type *Ty = CGM.getTypes().ConvertType(FD->getType());
    llvm::FunctionType *FTy = cast<llvm::FunctionType>(Ty);
    StringRef Name = FD->getName();
    return EmitNounwindRuntimeCall(CGM.CreateRuntimeFunction(FTy, Name), Ops);
  }

  if (BuiltinID == ARM::BI__builtin_arm_ldrexd ||
      ((BuiltinID == ARM::BI__builtin_arm_ldrex ||
        BuiltinID == ARM::BI__builtin_arm_ldaex) &&
       getContext().getTypeSize(E->getType()) == 64) ||
      BuiltinID == ARM::BI__ldrexd) {
    Function *F;

    switch (BuiltinID) {
    default: llvm_unreachable("unexpected builtin");
    case ARM::BI__builtin_arm_ldaex:
      F = CGM.getIntrinsic(Intrinsic::arm_ldaexd);
      break;
    case ARM::BI__builtin_arm_ldrexd:
    case ARM::BI__builtin_arm_ldrex:
    case ARM::BI__ldrexd:
      F = CGM.getIntrinsic(Intrinsic::arm_ldrexd);
      break;
    }

    Value *LdPtr = EmitScalarExpr(E->getArg(0));
    Value *Val = Builder.CreateCall(F, Builder.CreateBitCast(LdPtr, Int8PtrTy),
                                    "ldrexd");

    Value *Val0 = Builder.CreateExtractValue(Val, 1);
    Value *Val1 = Builder.CreateExtractValue(Val, 0);
    Val0 = Builder.CreateZExt(Val0, Int64Ty);
    Val1 = Builder.CreateZExt(Val1, Int64Ty);

    Value *ShiftCst = llvm::ConstantInt::get(Int64Ty, 32);
    Val = Builder.CreateShl(Val0, ShiftCst, "shl", true /* nuw */);
    Val = Builder.CreateOr(Val, Val1);
    return Builder.CreateBitCast(Val, ConvertType(E->getType()));
  }

  if (BuiltinID == ARM::BI__builtin_arm_ldrex ||
      BuiltinID == ARM::BI__builtin_arm_ldaex) {
    Value *LoadAddr = EmitScalarExpr(E->getArg(0));

    QualType Ty = E->getType();
    llvm::Type *RealResTy = ConvertType(Ty);
    llvm::Type *IntResTy = llvm::IntegerType::get(getLLVMContext(),
                                                  getContext().getTypeSize(Ty));
    LoadAddr = Builder.CreateBitCast(LoadAddr, IntResTy->getPointerTo());

    Function *F = CGM.getIntrinsic(BuiltinID == ARM::BI__builtin_arm_ldaex
                                       ? Intrinsic::arm_ldaex
                                       : Intrinsic::arm_ldrex,
                                   LoadAddr->getType());
    Value *Val = Builder.CreateCall(F, LoadAddr, "ldrex");

    if (RealResTy->isPointerTy())
      return Builder.CreateIntToPtr(Val, RealResTy);
    else {
      Val = Builder.CreateTruncOrBitCast(Val, IntResTy);
      return Builder.CreateBitCast(Val, RealResTy);
    }
  }

  if (BuiltinID == ARM::BI__builtin_arm_strexd ||
      ((BuiltinID == ARM::BI__builtin_arm_stlex ||
        BuiltinID == ARM::BI__builtin_arm_strex) &&
       getContext().getTypeSize(E->getArg(0)->getType()) == 64)) {
    Function *F = CGM.getIntrinsic(BuiltinID == ARM::BI__builtin_arm_stlex
                                       ? Intrinsic::arm_stlexd
                                       : Intrinsic::arm_strexd);
    llvm::Type *STy = llvm::StructType::get(Int32Ty, Int32Ty, nullptr);

    Value *Tmp = CreateMemTemp(E->getArg(0)->getType());
    Value *Val = EmitScalarExpr(E->getArg(0));
    Builder.CreateStore(Val, Tmp);

    Value *LdPtr = Builder.CreateBitCast(Tmp,llvm::PointerType::getUnqual(STy));
    Val = Builder.CreateLoad(LdPtr);

    Value *Arg0 = Builder.CreateExtractValue(Val, 0);
    Value *Arg1 = Builder.CreateExtractValue(Val, 1);
    Value *StPtr = Builder.CreateBitCast(EmitScalarExpr(E->getArg(1)), Int8PtrTy);
    return Builder.CreateCall(F, {Arg0, Arg1, StPtr}, "strexd");
  }

  if (BuiltinID == ARM::BI__builtin_arm_strex ||
      BuiltinID == ARM::BI__builtin_arm_stlex) {
    Value *StoreVal = EmitScalarExpr(E->getArg(0));
    Value *StoreAddr = EmitScalarExpr(E->getArg(1));

    QualType Ty = E->getArg(0)->getType();
    llvm::Type *StoreTy = llvm::IntegerType::get(getLLVMContext(),
                                                 getContext().getTypeSize(Ty));
    StoreAddr = Builder.CreateBitCast(StoreAddr, StoreTy->getPointerTo());

    if (StoreVal->getType()->isPointerTy())
      StoreVal = Builder.CreatePtrToInt(StoreVal, Int32Ty);
    else {
      StoreVal = Builder.CreateBitCast(StoreVal, StoreTy);
      StoreVal = Builder.CreateZExtOrBitCast(StoreVal, Int32Ty);
    }

    Function *F = CGM.getIntrinsic(BuiltinID == ARM::BI__builtin_arm_stlex
                                       ? Intrinsic::arm_stlex
                                       : Intrinsic::arm_strex,
                                   StoreAddr->getType());
    return Builder.CreateCall(F, {StoreVal, StoreAddr}, "strex");
  }

  if (BuiltinID == ARM::BI__builtin_arm_clrex) {
    Function *F = CGM.getIntrinsic(Intrinsic::arm_clrex);
    return Builder.CreateCall(F);
  }

  // CRC32
  Intrinsic::ID CRCIntrinsicID = Intrinsic::not_intrinsic;
  switch (BuiltinID) {
  case ARM::BI__builtin_arm_crc32b:
    CRCIntrinsicID = Intrinsic::arm_crc32b; break;
  case ARM::BI__builtin_arm_crc32cb:
    CRCIntrinsicID = Intrinsic::arm_crc32cb; break;
  case ARM::BI__builtin_arm_crc32h:
    CRCIntrinsicID = Intrinsic::arm_crc32h; break;
  case ARM::BI__builtin_arm_crc32ch:
    CRCIntrinsicID = Intrinsic::arm_crc32ch; break;
  case ARM::BI__builtin_arm_crc32w:
  case ARM::BI__builtin_arm_crc32d:
    CRCIntrinsicID = Intrinsic::arm_crc32w; break;
  case ARM::BI__builtin_arm_crc32cw:
  case ARM::BI__builtin_arm_crc32cd:
    CRCIntrinsicID = Intrinsic::arm_crc32cw; break;
  }

  if (CRCIntrinsicID != Intrinsic::not_intrinsic) {
    Value *Arg0 = EmitScalarExpr(E->getArg(0));
    Value *Arg1 = EmitScalarExpr(E->getArg(1));

    // crc32{c,}d intrinsics are implemnted as two calls to crc32{c,}w
    // intrinsics, hence we need different codegen for these cases.
    if (BuiltinID == ARM::BI__builtin_arm_crc32d ||
        BuiltinID == ARM::BI__builtin_arm_crc32cd) {
      Value *C1 = llvm::ConstantInt::get(Int64Ty, 32);
      Value *Arg1a = Builder.CreateTruncOrBitCast(Arg1, Int32Ty);
      Value *Arg1b = Builder.CreateLShr(Arg1, C1);
      Arg1b = Builder.CreateTruncOrBitCast(Arg1b, Int32Ty);

      Function *F = CGM.getIntrinsic(CRCIntrinsicID);
      Value *Res = Builder.CreateCall(F, {Arg0, Arg1a});
      return Builder.CreateCall(F, {Res, Arg1b});
    } else {
      Arg1 = Builder.CreateZExtOrBitCast(Arg1, Int32Ty);

      Function *F = CGM.getIntrinsic(CRCIntrinsicID);
      return Builder.CreateCall(F, {Arg0, Arg1});
    }
  }

  if (BuiltinID == ARM::BI__builtin_arm_rsr ||
      BuiltinID == ARM::BI__builtin_arm_rsr64 ||
      BuiltinID == ARM::BI__builtin_arm_rsrp ||
      BuiltinID == ARM::BI__builtin_arm_wsr ||
      BuiltinID == ARM::BI__builtin_arm_wsr64 ||
      BuiltinID == ARM::BI__builtin_arm_wsrp) {

    bool IsRead = BuiltinID == ARM::BI__builtin_arm_rsr ||
                  BuiltinID == ARM::BI__builtin_arm_rsr64 ||
                  BuiltinID == ARM::BI__builtin_arm_rsrp;

    bool IsPointerBuiltin = BuiltinID == ARM::BI__builtin_arm_rsrp ||
                            BuiltinID == ARM::BI__builtin_arm_wsrp;

    bool Is64Bit = BuiltinID == ARM::BI__builtin_arm_rsr64 ||
                   BuiltinID == ARM::BI__builtin_arm_wsr64;

    llvm::Type *ValueType;
    llvm::Type *RegisterType;
    if (IsPointerBuiltin) {
      ValueType = VoidPtrTy;
      RegisterType = Int32Ty;
    } else if (Is64Bit) {
      ValueType = RegisterType = Int64Ty;
    } else {
      ValueType = RegisterType = Int32Ty;
    }

    return EmitSpecialRegisterBuiltin(*this, E, RegisterType, ValueType, IsRead);
  }

  // Find out if any arguments are required to be integer constant
  // expressions.
  unsigned ICEArguments = 0;
  ASTContext::GetBuiltinTypeError Error;
  getContext().GetBuiltinType(BuiltinID, Error, &ICEArguments);
  assert(Error == ASTContext::GE_None && "Should not codegen an error");

  SmallVector<Value*, 4> Ops;
  llvm::Value *Align = nullptr;
  bool HasExtraArg = HasExtraNeonArgument(BuiltinID);
  unsigned NumArgs = E->getNumArgs() - (HasExtraArg ? 1 : 0);
  for (unsigned i = 0, e = NumArgs; i != e; i++) {
    if (i == 0) {
      switch (BuiltinID) {
      case NEON::BI__builtin_neon_vld1_v:
      case NEON::BI__builtin_neon_vld1q_v:
      case NEON::BI__builtin_neon_vld1q_lane_v:
      case NEON::BI__builtin_neon_vld1_lane_v:
      case NEON::BI__builtin_neon_vld1_dup_v:
      case NEON::BI__builtin_neon_vld1q_dup_v:
      case NEON::BI__builtin_neon_vst1_v:
      case NEON::BI__builtin_neon_vst1q_v:
      case NEON::BI__builtin_neon_vst1q_lane_v:
      case NEON::BI__builtin_neon_vst1_lane_v:
      case NEON::BI__builtin_neon_vst2_v:
      case NEON::BI__builtin_neon_vst2q_v:
      case NEON::BI__builtin_neon_vst2_lane_v:
      case NEON::BI__builtin_neon_vst2q_lane_v:
      case NEON::BI__builtin_neon_vst3_v:
      case NEON::BI__builtin_neon_vst3q_v:
      case NEON::BI__builtin_neon_vst3_lane_v:
      case NEON::BI__builtin_neon_vst3q_lane_v:
      case NEON::BI__builtin_neon_vst4_v:
      case NEON::BI__builtin_neon_vst4q_v:
      case NEON::BI__builtin_neon_vst4_lane_v:
      case NEON::BI__builtin_neon_vst4q_lane_v:
        // Get the alignment for the argument in addition to the value;
        // we'll use it later.
        std::pair<llvm::Value*, unsigned> Src =
            EmitPointerWithAlignment(E->getArg(0));
        Ops.push_back(Src.first);
        Align = Builder.getInt32(Src.second);
        continue;
      }
    }
    if (i == 1) {
      switch (BuiltinID) {
      case NEON::BI__builtin_neon_vld2_v:
      case NEON::BI__builtin_neon_vld2q_v:
      case NEON::BI__builtin_neon_vld3_v:
      case NEON::BI__builtin_neon_vld3q_v:
      case NEON::BI__builtin_neon_vld4_v:
      case NEON::BI__builtin_neon_vld4q_v:
      case NEON::BI__builtin_neon_vld2_lane_v:
      case NEON::BI__builtin_neon_vld2q_lane_v:
      case NEON::BI__builtin_neon_vld3_lane_v:
      case NEON::BI__builtin_neon_vld3q_lane_v:
      case NEON::BI__builtin_neon_vld4_lane_v:
      case NEON::BI__builtin_neon_vld4q_lane_v:
      case NEON::BI__builtin_neon_vld2_dup_v:
      case NEON::BI__builtin_neon_vld3_dup_v:
      case NEON::BI__builtin_neon_vld4_dup_v:
        // Get the alignment for the argument in addition to the value;
        // we'll use it later.
        std::pair<llvm::Value*, unsigned> Src =
            EmitPointerWithAlignment(E->getArg(1));
        Ops.push_back(Src.first);
        Align = Builder.getInt32(Src.second);
        continue;
      }
    }

    if ((ICEArguments & (1 << i)) == 0) {
      Ops.push_back(EmitScalarExpr(E->getArg(i)));
    } else {
      // If this is required to be a constant, constant fold it so that we know
      // that the generated intrinsic gets a ConstantInt.
      llvm::APSInt Result;
      bool IsConst = E->getArg(i)->isIntegerConstantExpr(Result, getContext());
      assert(IsConst && "Constant arg isn't actually constant?"); (void)IsConst;
      Ops.push_back(llvm::ConstantInt::get(getLLVMContext(), Result));
    }
  }

  switch (BuiltinID) {
  default: break;

  case NEON::BI__builtin_neon_vget_lane_i8:
  case NEON::BI__builtin_neon_vget_lane_i16:
  case NEON::BI__builtin_neon_vget_lane_i32:
  case NEON::BI__builtin_neon_vget_lane_i64:
  case NEON::BI__builtin_neon_vget_lane_f32:
  case NEON::BI__builtin_neon_vgetq_lane_i8:
  case NEON::BI__builtin_neon_vgetq_lane_i16:
  case NEON::BI__builtin_neon_vgetq_lane_i32:
  case NEON::BI__builtin_neon_vgetq_lane_i64:
  case NEON::BI__builtin_neon_vgetq_lane_f32:
    return Builder.CreateExtractElement(Ops[0], Ops[1], "vget_lane");

  case NEON::BI__builtin_neon_vset_lane_i8:
  case NEON::BI__builtin_neon_vset_lane_i16:
  case NEON::BI__builtin_neon_vset_lane_i32:
  case NEON::BI__builtin_neon_vset_lane_i64:
  case NEON::BI__builtin_neon_vset_lane_f32:
  case NEON::BI__builtin_neon_vsetq_lane_i8:
  case NEON::BI__builtin_neon_vsetq_lane_i16:
  case NEON::BI__builtin_neon_vsetq_lane_i32:
  case NEON::BI__builtin_neon_vsetq_lane_i64:
  case NEON::BI__builtin_neon_vsetq_lane_f32:
    return Builder.CreateInsertElement(Ops[1], Ops[0], Ops[2], "vset_lane");

  case NEON::BI__builtin_neon_vsha1h_u32:
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::arm_neon_sha1h), Ops,
                        "vsha1h");
  case NEON::BI__builtin_neon_vsha1cq_u32:
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::arm_neon_sha1c), Ops,
                        "vsha1h");
  case NEON::BI__builtin_neon_vsha1pq_u32:
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::arm_neon_sha1p), Ops,
                        "vsha1h");
  case NEON::BI__builtin_neon_vsha1mq_u32:
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::arm_neon_sha1m), Ops,
                        "vsha1h");

  // The ARM _MoveToCoprocessor builtins put the input register value as
  // the first argument, but the LLVM intrinsic expects it as the third one.
  case ARM::BI_MoveToCoprocessor:
  case ARM::BI_MoveToCoprocessor2: {
    Function *F = CGM.getIntrinsic(BuiltinID == ARM::BI_MoveToCoprocessor ? 
                                   Intrinsic::arm_mcr : Intrinsic::arm_mcr2);
    return Builder.CreateCall(F, {Ops[1], Ops[2], Ops[0],
                                  Ops[3], Ops[4], Ops[5]});
  }
  }

  // Get the last argument, which specifies the vector type.
  assert(HasExtraArg);
  llvm::APSInt Result;
  const Expr *Arg = E->getArg(E->getNumArgs()-1);
  if (!Arg->isIntegerConstantExpr(Result, getContext()))
    return nullptr;

  if (BuiltinID == ARM::BI__builtin_arm_vcvtr_f ||
      BuiltinID == ARM::BI__builtin_arm_vcvtr_d) {
    // Determine the overloaded type of this builtin.
    llvm::Type *Ty;
    if (BuiltinID == ARM::BI__builtin_arm_vcvtr_f)
      Ty = FloatTy;
    else
      Ty = DoubleTy;

    // Determine whether this is an unsigned conversion or not.
    bool usgn = Result.getZExtValue() == 1;
    unsigned Int = usgn ? Intrinsic::arm_vcvtru : Intrinsic::arm_vcvtr;

    // Call the appropriate intrinsic.
    Function *F = CGM.getIntrinsic(Int, Ty);
    return Builder.CreateCall(F, Ops, "vcvtr");
  }

  // Determine the type of this overloaded NEON intrinsic.
  NeonTypeFlags Type(Result.getZExtValue());
  bool usgn = Type.isUnsigned();
  bool rightShift = false;

  llvm::VectorType *VTy = GetNeonType(this, Type);
  llvm::Type *Ty = VTy;
  if (!Ty)
    return nullptr;

  // Many NEON builtins have identical semantics and uses in ARM and
  // AArch64. Emit these in a single function.
  auto IntrinsicMap = makeArrayRef(ARMSIMDIntrinsicMap);
  const NeonIntrinsicInfo *Builtin = findNeonIntrinsicInMap(
      IntrinsicMap, BuiltinID, NEONSIMDIntrinsicsProvenSorted);
  if (Builtin)
    return EmitCommonNeonBuiltinExpr(
        Builtin->BuiltinID, Builtin->LLVMIntrinsic, Builtin->AltLLVMIntrinsic,
        Builtin->NameHint, Builtin->TypeModifier, E, Ops, Align);

  unsigned Int;
  switch (BuiltinID) {
  default: return nullptr;
  case NEON::BI__builtin_neon_vld1q_lane_v:
    // Handle 64-bit integer elements as a special case.  Use shuffles of
    // one-element vectors to avoid poor code for i64 in the backend.
    if (VTy->getElementType()->isIntegerTy(64)) {
      // Extract the other lane.
      Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
      int Lane = cast<ConstantInt>(Ops[2])->getZExtValue();
      Value *SV = llvm::ConstantVector::get(ConstantInt::get(Int32Ty, 1-Lane));
      Ops[1] = Builder.CreateShuffleVector(Ops[1], Ops[1], SV);
      // Load the value as a one-element vector.
      Ty = llvm::VectorType::get(VTy->getElementType(), 1);
      Function *F = CGM.getIntrinsic(Intrinsic::arm_neon_vld1, Ty);
      Value *Ld = Builder.CreateCall(F, {Ops[0], Align});
      // Combine them.
      SmallVector<Constant*, 2> Indices;
      Indices.push_back(ConstantInt::get(Int32Ty, 1-Lane));
      Indices.push_back(ConstantInt::get(Int32Ty, Lane));
      SV = llvm::ConstantVector::get(Indices);
      return Builder.CreateShuffleVector(Ops[1], Ld, SV, "vld1q_lane");
    }
    // fall through
  case NEON::BI__builtin_neon_vld1_lane_v: {
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Ty = llvm::PointerType::getUnqual(VTy->getElementType());
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    LoadInst *Ld = Builder.CreateLoad(Ops[0]);
    Ld->setAlignment(cast<ConstantInt>(Align)->getZExtValue());
    return Builder.CreateInsertElement(Ops[1], Ld, Ops[2], "vld1_lane");
  }
  case NEON::BI__builtin_neon_vld2_dup_v:
  case NEON::BI__builtin_neon_vld3_dup_v:
  case NEON::BI__builtin_neon_vld4_dup_v: {
    // Handle 64-bit elements as a special-case.  There is no "dup" needed.
    if (VTy->getElementType()->getPrimitiveSizeInBits() == 64) {
      switch (BuiltinID) {
      case NEON::BI__builtin_neon_vld2_dup_v:
        Int = Intrinsic::arm_neon_vld2;
        break;
      case NEON::BI__builtin_neon_vld3_dup_v:
        Int = Intrinsic::arm_neon_vld3;
        break;
      case NEON::BI__builtin_neon_vld4_dup_v:
        Int = Intrinsic::arm_neon_vld4;
        break;
      default: llvm_unreachable("unknown vld_dup intrinsic?");
      }
      Function *F = CGM.getIntrinsic(Int, Ty);
      Ops[1] = Builder.CreateCall(F, {Ops[1], Align}, "vld_dup");
      Ty = llvm::PointerType::getUnqual(Ops[1]->getType());
      Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
      return Builder.CreateStore(Ops[1], Ops[0]);
    }
    switch (BuiltinID) {
    case NEON::BI__builtin_neon_vld2_dup_v:
      Int = Intrinsic::arm_neon_vld2lane;
      break;
    case NEON::BI__builtin_neon_vld3_dup_v:
      Int = Intrinsic::arm_neon_vld3lane;
      break;
    case NEON::BI__builtin_neon_vld4_dup_v:
      Int = Intrinsic::arm_neon_vld4lane;
      break;
    default: llvm_unreachable("unknown vld_dup intrinsic?");
    }
    Function *F = CGM.getIntrinsic(Int, Ty);
    llvm::StructType *STy = cast<llvm::StructType>(F->getReturnType());

    SmallVector<Value*, 6> Args;
    Args.push_back(Ops[1]);
    Args.append(STy->getNumElements(), UndefValue::get(Ty));

    llvm::Constant *CI = ConstantInt::get(Int32Ty, 0);
    Args.push_back(CI);
    Args.push_back(Align);

    Ops[1] = Builder.CreateCall(F, Args, "vld_dup");
    // splat lane 0 to all elts in each vector of the result.
    for (unsigned i = 0, e = STy->getNumElements(); i != e; ++i) {
      Value *Val = Builder.CreateExtractValue(Ops[1], i);
      Value *Elt = Builder.CreateBitCast(Val, Ty);
      Elt = EmitNeonSplat(Elt, CI);
      Elt = Builder.CreateBitCast(Elt, Val->getType());
      Ops[1] = Builder.CreateInsertValue(Ops[1], Elt, i);
    }
    Ty = llvm::PointerType::getUnqual(Ops[1]->getType());
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    return Builder.CreateStore(Ops[1], Ops[0]);
  }
  case NEON::BI__builtin_neon_vqrshrn_n_v:
    Int =
      usgn ? Intrinsic::arm_neon_vqrshiftnu : Intrinsic::arm_neon_vqrshiftns;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vqrshrn_n",
                        1, true);
  case NEON::BI__builtin_neon_vqrshrun_n_v:
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::arm_neon_vqrshiftnsu, Ty),
                        Ops, "vqrshrun_n", 1, true);
  case NEON::BI__builtin_neon_vqshrn_n_v:
    Int = usgn ? Intrinsic::arm_neon_vqshiftnu : Intrinsic::arm_neon_vqshiftns;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vqshrn_n",
                        1, true);
  case NEON::BI__builtin_neon_vqshrun_n_v:
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::arm_neon_vqshiftnsu, Ty),
                        Ops, "vqshrun_n", 1, true);
  case NEON::BI__builtin_neon_vrecpe_v:
  case NEON::BI__builtin_neon_vrecpeq_v:
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::arm_neon_vrecpe, Ty),
                        Ops, "vrecpe");
  case NEON::BI__builtin_neon_vrshrn_n_v:
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::arm_neon_vrshiftn, Ty),
                        Ops, "vrshrn_n", 1, true);
  case NEON::BI__builtin_neon_vrsra_n_v:
  case NEON::BI__builtin_neon_vrsraq_n_v:
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Ops[2] = EmitNeonShiftVector(Ops[2], Ty, true);
    Int = usgn ? Intrinsic::arm_neon_vrshiftu : Intrinsic::arm_neon_vrshifts;
    Ops[1] = Builder.CreateCall(CGM.getIntrinsic(Int, Ty), {Ops[1], Ops[2]});
    return Builder.CreateAdd(Ops[0], Ops[1], "vrsra_n");
  case NEON::BI__builtin_neon_vsri_n_v:
  case NEON::BI__builtin_neon_vsriq_n_v:
    rightShift = true;
  case NEON::BI__builtin_neon_vsli_n_v:
  case NEON::BI__builtin_neon_vsliq_n_v:
    Ops[2] = EmitNeonShiftVector(Ops[2], Ty, rightShift);
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::arm_neon_vshiftins, Ty),
                        Ops, "vsli_n");
  case NEON::BI__builtin_neon_vsra_n_v:
  case NEON::BI__builtin_neon_vsraq_n_v:
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    Ops[1] = EmitNeonRShiftImm(Ops[1], Ops[2], Ty, usgn, "vsra_n");
    return Builder.CreateAdd(Ops[0], Ops[1]);
  case NEON::BI__builtin_neon_vst1q_lane_v:
    // Handle 64-bit integer elements as a special case.  Use a shuffle to get
    // a one-element vector and avoid poor code for i64 in the backend.
    if (VTy->getElementType()->isIntegerTy(64)) {
      Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
      Value *SV = llvm::ConstantVector::get(cast<llvm::Constant>(Ops[2]));
      Ops[1] = Builder.CreateShuffleVector(Ops[1], Ops[1], SV);
      Ops[2] = Align;
      return Builder.CreateCall(CGM.getIntrinsic(Intrinsic::arm_neon_vst1,
                                                 Ops[1]->getType()), Ops);
    }
    // fall through
  case NEON::BI__builtin_neon_vst1_lane_v: {
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Ops[1] = Builder.CreateExtractElement(Ops[1], Ops[2]);
    Ty = llvm::PointerType::getUnqual(Ops[1]->getType());
    StoreInst *St = Builder.CreateStore(Ops[1],
                                        Builder.CreateBitCast(Ops[0], Ty));
    St->setAlignment(cast<ConstantInt>(Align)->getZExtValue());
    return St;
  }
  case NEON::BI__builtin_neon_vtbl1_v:
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::arm_neon_vtbl1),
                        Ops, "vtbl1");
  case NEON::BI__builtin_neon_vtbl2_v:
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::arm_neon_vtbl2),
                        Ops, "vtbl2");
  case NEON::BI__builtin_neon_vtbl3_v:
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::arm_neon_vtbl3),
                        Ops, "vtbl3");
  case NEON::BI__builtin_neon_vtbl4_v:
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::arm_neon_vtbl4),
                        Ops, "vtbl4");
  case NEON::BI__builtin_neon_vtbx1_v:
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::arm_neon_vtbx1),
                        Ops, "vtbx1");
  case NEON::BI__builtin_neon_vtbx2_v:
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::arm_neon_vtbx2),
                        Ops, "vtbx2");
  case NEON::BI__builtin_neon_vtbx3_v:
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::arm_neon_vtbx3),
                        Ops, "vtbx3");
  case NEON::BI__builtin_neon_vtbx4_v:
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::arm_neon_vtbx4),
                        Ops, "vtbx4");
  }
}

static Value *EmitAArch64TblBuiltinExpr(CodeGenFunction &CGF, unsigned BuiltinID,
                                      const CallExpr *E,
                                      SmallVectorImpl<Value *> &Ops) {
  unsigned int Int = 0;
  const char *s = nullptr;

  switch (BuiltinID) {
  default:
    return nullptr;
  case NEON::BI__builtin_neon_vtbl1_v:
  case NEON::BI__builtin_neon_vqtbl1_v:
  case NEON::BI__builtin_neon_vqtbl1q_v:
  case NEON::BI__builtin_neon_vtbl2_v:
  case NEON::BI__builtin_neon_vqtbl2_v:
  case NEON::BI__builtin_neon_vqtbl2q_v:
  case NEON::BI__builtin_neon_vtbl3_v:
  case NEON::BI__builtin_neon_vqtbl3_v:
  case NEON::BI__builtin_neon_vqtbl3q_v:
  case NEON::BI__builtin_neon_vtbl4_v:
  case NEON::BI__builtin_neon_vqtbl4_v:
  case NEON::BI__builtin_neon_vqtbl4q_v:
    break;
  case NEON::BI__builtin_neon_vtbx1_v:
  case NEON::BI__builtin_neon_vqtbx1_v:
  case NEON::BI__builtin_neon_vqtbx1q_v:
  case NEON::BI__builtin_neon_vtbx2_v:
  case NEON::BI__builtin_neon_vqtbx2_v:
  case NEON::BI__builtin_neon_vqtbx2q_v:
  case NEON::BI__builtin_neon_vtbx3_v:
  case NEON::BI__builtin_neon_vqtbx3_v:
  case NEON::BI__builtin_neon_vqtbx3q_v:
  case NEON::BI__builtin_neon_vtbx4_v:
  case NEON::BI__builtin_neon_vqtbx4_v:
  case NEON::BI__builtin_neon_vqtbx4q_v:
    break;
  }

  assert(E->getNumArgs() >= 3);

  // Get the last argument, which specifies the vector type.
  llvm::APSInt Result;
  const Expr *Arg = E->getArg(E->getNumArgs() - 1);
  if (!Arg->isIntegerConstantExpr(Result, CGF.getContext()))
    return nullptr;

  // Determine the type of this overloaded NEON intrinsic.
  NeonTypeFlags Type(Result.getZExtValue());
  llvm::VectorType *VTy = GetNeonType(&CGF, Type);
  llvm::Type *Ty = VTy;
  if (!Ty)
    return nullptr;

  unsigned nElts = VTy->getNumElements();

  CodeGen::CGBuilderTy &Builder = CGF.Builder;

  // AArch64 scalar builtins are not overloaded, they do not have an extra
  // argument that specifies the vector type, need to handle each case.
  SmallVector<Value *, 2> TblOps;
  switch (BuiltinID) {
  case NEON::BI__builtin_neon_vtbl1_v: {
    TblOps.push_back(Ops[0]);
    return packTBLDVectorList(CGF, TblOps, nullptr, Ops[1], Ty,
                              Intrinsic::aarch64_neon_tbl1, "vtbl1");
  }
  case NEON::BI__builtin_neon_vtbl2_v: {
    TblOps.push_back(Ops[0]);
    TblOps.push_back(Ops[1]);
    return packTBLDVectorList(CGF, TblOps, nullptr, Ops[2], Ty,
                              Intrinsic::aarch64_neon_tbl1, "vtbl1");
  }
  case NEON::BI__builtin_neon_vtbl3_v: {
    TblOps.push_back(Ops[0]);
    TblOps.push_back(Ops[1]);
    TblOps.push_back(Ops[2]);
    return packTBLDVectorList(CGF, TblOps, nullptr, Ops[3], Ty,
                              Intrinsic::aarch64_neon_tbl2, "vtbl2");
  }
  case NEON::BI__builtin_neon_vtbl4_v: {
    TblOps.push_back(Ops[0]);
    TblOps.push_back(Ops[1]);
    TblOps.push_back(Ops[2]);
    TblOps.push_back(Ops[3]);
    return packTBLDVectorList(CGF, TblOps, nullptr, Ops[4], Ty,
                              Intrinsic::aarch64_neon_tbl2, "vtbl2");
  }
  case NEON::BI__builtin_neon_vtbx1_v: {
    TblOps.push_back(Ops[1]);
    Value *TblRes = packTBLDVectorList(CGF, TblOps, nullptr, Ops[2], Ty,
                                       Intrinsic::aarch64_neon_tbl1, "vtbl1");

    llvm::Constant *Eight = ConstantInt::get(VTy->getElementType(), 8);
    Value* EightV = llvm::ConstantVector::getSplat(nElts, Eight);
    Value *CmpRes = Builder.CreateICmp(ICmpInst::ICMP_UGE, Ops[2], EightV);
    CmpRes = Builder.CreateSExt(CmpRes, Ty);

    Value *EltsFromInput = Builder.CreateAnd(CmpRes, Ops[0]);
    Value *EltsFromTbl = Builder.CreateAnd(Builder.CreateNot(CmpRes), TblRes);
    return Builder.CreateOr(EltsFromInput, EltsFromTbl, "vtbx");
  }
  case NEON::BI__builtin_neon_vtbx2_v: {
    TblOps.push_back(Ops[1]);
    TblOps.push_back(Ops[2]);
    return packTBLDVectorList(CGF, TblOps, Ops[0], Ops[3], Ty,
                              Intrinsic::aarch64_neon_tbx1, "vtbx1");
  }
  case NEON::BI__builtin_neon_vtbx3_v: {
    TblOps.push_back(Ops[1]);
    TblOps.push_back(Ops[2]);
    TblOps.push_back(Ops[3]);
    Value *TblRes = packTBLDVectorList(CGF, TblOps, nullptr, Ops[4], Ty,
                                       Intrinsic::aarch64_neon_tbl2, "vtbl2");

    llvm::Constant *TwentyFour = ConstantInt::get(VTy->getElementType(), 24);
    Value* TwentyFourV = llvm::ConstantVector::getSplat(nElts, TwentyFour);
    Value *CmpRes = Builder.CreateICmp(ICmpInst::ICMP_UGE, Ops[4],
                                           TwentyFourV);
    CmpRes = Builder.CreateSExt(CmpRes, Ty);

    Value *EltsFromInput = Builder.CreateAnd(CmpRes, Ops[0]);
    Value *EltsFromTbl = Builder.CreateAnd(Builder.CreateNot(CmpRes), TblRes);
    return Builder.CreateOr(EltsFromInput, EltsFromTbl, "vtbx");
  }
  case NEON::BI__builtin_neon_vtbx4_v: {
    TblOps.push_back(Ops[1]);
    TblOps.push_back(Ops[2]);
    TblOps.push_back(Ops[3]);
    TblOps.push_back(Ops[4]);
    return packTBLDVectorList(CGF, TblOps, Ops[0], Ops[5], Ty,
                              Intrinsic::aarch64_neon_tbx2, "vtbx2");
  }
  case NEON::BI__builtin_neon_vqtbl1_v:
  case NEON::BI__builtin_neon_vqtbl1q_v:
    Int = Intrinsic::aarch64_neon_tbl1; s = "vtbl1"; break;
  case NEON::BI__builtin_neon_vqtbl2_v:
  case NEON::BI__builtin_neon_vqtbl2q_v: {
    Int = Intrinsic::aarch64_neon_tbl2; s = "vtbl2"; break;
  case NEON::BI__builtin_neon_vqtbl3_v:
  case NEON::BI__builtin_neon_vqtbl3q_v:
    Int = Intrinsic::aarch64_neon_tbl3; s = "vtbl3"; break;
  case NEON::BI__builtin_neon_vqtbl4_v:
  case NEON::BI__builtin_neon_vqtbl4q_v:
    Int = Intrinsic::aarch64_neon_tbl4; s = "vtbl4"; break;
  case NEON::BI__builtin_neon_vqtbx1_v:
  case NEON::BI__builtin_neon_vqtbx1q_v:
    Int = Intrinsic::aarch64_neon_tbx1; s = "vtbx1"; break;
  case NEON::BI__builtin_neon_vqtbx2_v:
  case NEON::BI__builtin_neon_vqtbx2q_v:
    Int = Intrinsic::aarch64_neon_tbx2; s = "vtbx2"; break;
  case NEON::BI__builtin_neon_vqtbx3_v:
  case NEON::BI__builtin_neon_vqtbx3q_v:
    Int = Intrinsic::aarch64_neon_tbx3; s = "vtbx3"; break;
  case NEON::BI__builtin_neon_vqtbx4_v:
  case NEON::BI__builtin_neon_vqtbx4q_v:
    Int = Intrinsic::aarch64_neon_tbx4; s = "vtbx4"; break;
  }
  }

  if (!Int)
    return nullptr;

  Function *F = CGF.CGM.getIntrinsic(Int, Ty);
  return CGF.EmitNeonCall(F, Ops, s);
}

Value *CodeGenFunction::vectorWrapScalar16(Value *Op) {
  llvm::Type *VTy = llvm::VectorType::get(Int16Ty, 4);
  Op = Builder.CreateBitCast(Op, Int16Ty);
  Value *V = UndefValue::get(VTy);
  llvm::Constant *CI = ConstantInt::get(SizeTy, 0);
  Op = Builder.CreateInsertElement(V, Op, CI);
  return Op;
}

Value *CodeGenFunction::vectorWrapScalar8(Value *Op) {
  llvm::Type *VTy = llvm::VectorType::get(Int8Ty, 8);
  Op = Builder.CreateBitCast(Op, Int8Ty);
  Value *V = UndefValue::get(VTy);
  llvm::Constant *CI = ConstantInt::get(SizeTy, 0);
  Op = Builder.CreateInsertElement(V, Op, CI);
  return Op;
}

Value *CodeGenFunction::EmitAArch64BuiltinExpr(unsigned BuiltinID,
                                               const CallExpr *E) {
  unsigned HintID = static_cast<unsigned>(-1);
  switch (BuiltinID) {
  default: break;
  case AArch64::BI__builtin_arm_nop:
    HintID = 0;
    break;
  case AArch64::BI__builtin_arm_yield:
    HintID = 1;
    break;
  case AArch64::BI__builtin_arm_wfe:
    HintID = 2;
    break;
  case AArch64::BI__builtin_arm_wfi:
    HintID = 3;
    break;
  case AArch64::BI__builtin_arm_sev:
    HintID = 4;
    break;
  case AArch64::BI__builtin_arm_sevl:
    HintID = 5;
    break;
  }

  if (HintID != static_cast<unsigned>(-1)) {
    Function *F = CGM.getIntrinsic(Intrinsic::aarch64_hint);
    return Builder.CreateCall(F, llvm::ConstantInt::get(Int32Ty, HintID));
  }

  if (BuiltinID == AArch64::BI__builtin_arm_prefetch) {
    Value *Address         = EmitScalarExpr(E->getArg(0));
    Value *RW              = EmitScalarExpr(E->getArg(1));
    Value *CacheLevel      = EmitScalarExpr(E->getArg(2));
    Value *RetentionPolicy = EmitScalarExpr(E->getArg(3));
    Value *IsData          = EmitScalarExpr(E->getArg(4));

    Value *Locality = nullptr;
    if (cast<llvm::ConstantInt>(RetentionPolicy)->isZero()) {
      // Temporal fetch, needs to convert cache level to locality.
      Locality = llvm::ConstantInt::get(Int32Ty,
        -cast<llvm::ConstantInt>(CacheLevel)->getValue() + 3);
    } else {
      // Streaming fetch.
      Locality = llvm::ConstantInt::get(Int32Ty, 0);
    }

    // FIXME: We need AArch64 specific LLVM intrinsic if we want to specify
    // PLDL3STRM or PLDL2STRM.
    Value *F = CGM.getIntrinsic(Intrinsic::prefetch);
    return Builder.CreateCall(F, {Address, RW, Locality, IsData});
  }

  if (BuiltinID == AArch64::BI__builtin_arm_rbit) {
    assert((getContext().getTypeSize(E->getType()) == 32) &&
           "rbit of unusual size!");
    llvm::Value *Arg = EmitScalarExpr(E->getArg(0));
    return Builder.CreateCall(
        CGM.getIntrinsic(Intrinsic::aarch64_rbit, Arg->getType()), Arg, "rbit");
  }
  if (BuiltinID == AArch64::BI__builtin_arm_rbit64) {
    assert((getContext().getTypeSize(E->getType()) == 64) &&
           "rbit of unusual size!");
    llvm::Value *Arg = EmitScalarExpr(E->getArg(0));
    return Builder.CreateCall(
        CGM.getIntrinsic(Intrinsic::aarch64_rbit, Arg->getType()), Arg, "rbit");
  }

  if (BuiltinID == AArch64::BI__clear_cache) {
    assert(E->getNumArgs() == 2 && "__clear_cache takes 2 arguments");
    const FunctionDecl *FD = E->getDirectCallee();
    SmallVector<Value*, 2> Ops;
    for (unsigned i = 0; i < 2; i++)
      Ops.push_back(EmitScalarExpr(E->getArg(i)));
    llvm::Type *Ty = CGM.getTypes().ConvertType(FD->getType());
    llvm::FunctionType *FTy = cast<llvm::FunctionType>(Ty);
    StringRef Name = FD->getName();
    return EmitNounwindRuntimeCall(CGM.CreateRuntimeFunction(FTy, Name), Ops);
  }

  if ((BuiltinID == AArch64::BI__builtin_arm_ldrex ||
      BuiltinID == AArch64::BI__builtin_arm_ldaex) &&
      getContext().getTypeSize(E->getType()) == 128) {
    Function *F = CGM.getIntrinsic(BuiltinID == AArch64::BI__builtin_arm_ldaex
                                       ? Intrinsic::aarch64_ldaxp
                                       : Intrinsic::aarch64_ldxp);

    Value *LdPtr = EmitScalarExpr(E->getArg(0));
    Value *Val = Builder.CreateCall(F, Builder.CreateBitCast(LdPtr, Int8PtrTy),
                                    "ldxp");

    Value *Val0 = Builder.CreateExtractValue(Val, 1);
    Value *Val1 = Builder.CreateExtractValue(Val, 0);
    llvm::Type *Int128Ty = llvm::IntegerType::get(getLLVMContext(), 128);
    Val0 = Builder.CreateZExt(Val0, Int128Ty);
    Val1 = Builder.CreateZExt(Val1, Int128Ty);

    Value *ShiftCst = llvm::ConstantInt::get(Int128Ty, 64);
    Val = Builder.CreateShl(Val0, ShiftCst, "shl", true /* nuw */);
    Val = Builder.CreateOr(Val, Val1);
    return Builder.CreateBitCast(Val, ConvertType(E->getType()));
  } else if (BuiltinID == AArch64::BI__builtin_arm_ldrex ||
             BuiltinID == AArch64::BI__builtin_arm_ldaex) {
    Value *LoadAddr = EmitScalarExpr(E->getArg(0));

    QualType Ty = E->getType();
    llvm::Type *RealResTy = ConvertType(Ty);
    llvm::Type *IntResTy = llvm::IntegerType::get(getLLVMContext(),
                                                  getContext().getTypeSize(Ty));
    LoadAddr = Builder.CreateBitCast(LoadAddr, IntResTy->getPointerTo());

    Function *F = CGM.getIntrinsic(BuiltinID == AArch64::BI__builtin_arm_ldaex
                                       ? Intrinsic::aarch64_ldaxr
                                       : Intrinsic::aarch64_ldxr,
                                   LoadAddr->getType());
    Value *Val = Builder.CreateCall(F, LoadAddr, "ldxr");

    if (RealResTy->isPointerTy())
      return Builder.CreateIntToPtr(Val, RealResTy);

    Val = Builder.CreateTruncOrBitCast(Val, IntResTy);
    return Builder.CreateBitCast(Val, RealResTy);
  }

  if ((BuiltinID == AArch64::BI__builtin_arm_strex ||
       BuiltinID == AArch64::BI__builtin_arm_stlex) &&
      getContext().getTypeSize(E->getArg(0)->getType()) == 128) {
    Function *F = CGM.getIntrinsic(BuiltinID == AArch64::BI__builtin_arm_stlex
                                       ? Intrinsic::aarch64_stlxp
                                       : Intrinsic::aarch64_stxp);
    llvm::Type *STy = llvm::StructType::get(Int64Ty, Int64Ty, nullptr);

    Value *One = llvm::ConstantInt::get(Int32Ty, 1);
    Value *Tmp = Builder.CreateAlloca(ConvertType(E->getArg(0)->getType()),
                                      One);
    Value *Val = EmitScalarExpr(E->getArg(0));
    Builder.CreateStore(Val, Tmp);

    Value *LdPtr = Builder.CreateBitCast(Tmp,llvm::PointerType::getUnqual(STy));
    Val = Builder.CreateLoad(LdPtr);

    Value *Arg0 = Builder.CreateExtractValue(Val, 0);
    Value *Arg1 = Builder.CreateExtractValue(Val, 1);
    Value *StPtr = Builder.CreateBitCast(EmitScalarExpr(E->getArg(1)),
                                         Int8PtrTy);
    return Builder.CreateCall(F, {Arg0, Arg1, StPtr}, "stxp");
  }

  if (BuiltinID == AArch64::BI__builtin_arm_strex ||
      BuiltinID == AArch64::BI__builtin_arm_stlex) {
    Value *StoreVal = EmitScalarExpr(E->getArg(0));
    Value *StoreAddr = EmitScalarExpr(E->getArg(1));

    QualType Ty = E->getArg(0)->getType();
    llvm::Type *StoreTy = llvm::IntegerType::get(getLLVMContext(),
                                                 getContext().getTypeSize(Ty));
    StoreAddr = Builder.CreateBitCast(StoreAddr, StoreTy->getPointerTo());

    if (StoreVal->getType()->isPointerTy())
      StoreVal = Builder.CreatePtrToInt(StoreVal, Int64Ty);
    else {
      StoreVal = Builder.CreateBitCast(StoreVal, StoreTy);
      StoreVal = Builder.CreateZExtOrBitCast(StoreVal, Int64Ty);
    }

    Function *F = CGM.getIntrinsic(BuiltinID == AArch64::BI__builtin_arm_stlex
                                       ? Intrinsic::aarch64_stlxr
                                       : Intrinsic::aarch64_stxr,
                                   StoreAddr->getType());
    return Builder.CreateCall(F, {StoreVal, StoreAddr}, "stxr");
  }

  if (BuiltinID == AArch64::BI__builtin_arm_clrex) {
    Function *F = CGM.getIntrinsic(Intrinsic::aarch64_clrex);
    return Builder.CreateCall(F);
  }

  // CRC32
  Intrinsic::ID CRCIntrinsicID = Intrinsic::not_intrinsic;
  switch (BuiltinID) {
  case AArch64::BI__builtin_arm_crc32b:
    CRCIntrinsicID = Intrinsic::aarch64_crc32b; break;
  case AArch64::BI__builtin_arm_crc32cb:
    CRCIntrinsicID = Intrinsic::aarch64_crc32cb; break;
  case AArch64::BI__builtin_arm_crc32h:
    CRCIntrinsicID = Intrinsic::aarch64_crc32h; break;
  case AArch64::BI__builtin_arm_crc32ch:
    CRCIntrinsicID = Intrinsic::aarch64_crc32ch; break;
  case AArch64::BI__builtin_arm_crc32w:
    CRCIntrinsicID = Intrinsic::aarch64_crc32w; break;
  case AArch64::BI__builtin_arm_crc32cw:
    CRCIntrinsicID = Intrinsic::aarch64_crc32cw; break;
  case AArch64::BI__builtin_arm_crc32d:
    CRCIntrinsicID = Intrinsic::aarch64_crc32x; break;
  case AArch64::BI__builtin_arm_crc32cd:
    CRCIntrinsicID = Intrinsic::aarch64_crc32cx; break;
  }

  if (CRCIntrinsicID != Intrinsic::not_intrinsic) {
    Value *Arg0 = EmitScalarExpr(E->getArg(0));
    Value *Arg1 = EmitScalarExpr(E->getArg(1));
    Function *F = CGM.getIntrinsic(CRCIntrinsicID);

    llvm::Type *DataTy = F->getFunctionType()->getParamType(1);
    Arg1 = Builder.CreateZExtOrBitCast(Arg1, DataTy);

    return Builder.CreateCall(F, {Arg0, Arg1});
  }

  if (BuiltinID == AArch64::BI__builtin_arm_rsr ||
      BuiltinID == AArch64::BI__builtin_arm_rsr64 ||
      BuiltinID == AArch64::BI__builtin_arm_rsrp ||
      BuiltinID == AArch64::BI__builtin_arm_wsr ||
      BuiltinID == AArch64::BI__builtin_arm_wsr64 ||
      BuiltinID == AArch64::BI__builtin_arm_wsrp) {

    bool IsRead = BuiltinID == AArch64::BI__builtin_arm_rsr ||
                  BuiltinID == AArch64::BI__builtin_arm_rsr64 ||
                  BuiltinID == AArch64::BI__builtin_arm_rsrp;

    bool IsPointerBuiltin = BuiltinID == AArch64::BI__builtin_arm_rsrp ||
                            BuiltinID == AArch64::BI__builtin_arm_wsrp;

    bool Is64Bit = BuiltinID != AArch64::BI__builtin_arm_rsr &&
                   BuiltinID != AArch64::BI__builtin_arm_wsr;

    llvm::Type *ValueType;
    llvm::Type *RegisterType = Int64Ty;
    if (IsPointerBuiltin) {
      ValueType = VoidPtrTy;
    } else if (Is64Bit) {
      ValueType = Int64Ty;
    } else {
      ValueType = Int32Ty;
    }

    return EmitSpecialRegisterBuiltin(*this, E, RegisterType, ValueType, IsRead);
  }

  // Find out if any arguments are required to be integer constant
  // expressions.
  unsigned ICEArguments = 0;
  ASTContext::GetBuiltinTypeError Error;
  getContext().GetBuiltinType(BuiltinID, Error, &ICEArguments);
  assert(Error == ASTContext::GE_None && "Should not codegen an error");

  llvm::SmallVector<Value*, 4> Ops;
  for (unsigned i = 0, e = E->getNumArgs() - 1; i != e; i++) {
    if ((ICEArguments & (1 << i)) == 0) {
      Ops.push_back(EmitScalarExpr(E->getArg(i)));
    } else {
      // If this is required to be a constant, constant fold it so that we know
      // that the generated intrinsic gets a ConstantInt.
      llvm::APSInt Result;
      bool IsConst = E->getArg(i)->isIntegerConstantExpr(Result, getContext());
      assert(IsConst && "Constant arg isn't actually constant?");
      (void)IsConst;
      Ops.push_back(llvm::ConstantInt::get(getLLVMContext(), Result));
    }
  }

  auto SISDMap = makeArrayRef(AArch64SISDIntrinsicMap);
  const NeonIntrinsicInfo *Builtin = findNeonIntrinsicInMap(
      SISDMap, BuiltinID, AArch64SISDIntrinsicsProvenSorted);

  if (Builtin) {
    Ops.push_back(EmitScalarExpr(E->getArg(E->getNumArgs() - 1)));
    Value *Result = EmitCommonNeonSISDBuiltinExpr(*this, *Builtin, Ops, E);
    assert(Result && "SISD intrinsic should have been handled");
    return Result;
  }

  llvm::APSInt Result;
  const Expr *Arg = E->getArg(E->getNumArgs()-1);
  NeonTypeFlags Type(0);
  if (Arg->isIntegerConstantExpr(Result, getContext()))
    // Determine the type of this overloaded NEON intrinsic.
    Type = NeonTypeFlags(Result.getZExtValue());

  bool usgn = Type.isUnsigned();
  bool quad = Type.isQuad();

  // Handle non-overloaded intrinsics first.
  switch (BuiltinID) {
  default: break;
  case NEON::BI__builtin_neon_vldrq_p128: {
    llvm::Type *Int128PTy = llvm::Type::getIntNPtrTy(getLLVMContext(), 128);
    Value *Ptr = Builder.CreateBitCast(EmitScalarExpr(E->getArg(0)), Int128PTy);
    return Builder.CreateLoad(Ptr);
  }
  case NEON::BI__builtin_neon_vstrq_p128: {
    llvm::Type *Int128PTy = llvm::Type::getIntNPtrTy(getLLVMContext(), 128);
    Value *Ptr = Builder.CreateBitCast(Ops[0], Int128PTy);
    return Builder.CreateStore(EmitScalarExpr(E->getArg(1)), Ptr);
  }
  case NEON::BI__builtin_neon_vcvts_u32_f32:
  case NEON::BI__builtin_neon_vcvtd_u64_f64:
    usgn = true;
    // FALL THROUGH
  case NEON::BI__builtin_neon_vcvts_s32_f32:
  case NEON::BI__builtin_neon_vcvtd_s64_f64: {
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    bool Is64 = Ops[0]->getType()->getPrimitiveSizeInBits() == 64;
    llvm::Type *InTy = Is64 ? Int64Ty : Int32Ty;
    llvm::Type *FTy = Is64 ? DoubleTy : FloatTy;
    Ops[0] = Builder.CreateBitCast(Ops[0], FTy);
    if (usgn)
      return Builder.CreateFPToUI(Ops[0], InTy);
    return Builder.CreateFPToSI(Ops[0], InTy);
  }
  case NEON::BI__builtin_neon_vcvts_f32_u32:
  case NEON::BI__builtin_neon_vcvtd_f64_u64:
    usgn = true;
    // FALL THROUGH
  case NEON::BI__builtin_neon_vcvts_f32_s32:
  case NEON::BI__builtin_neon_vcvtd_f64_s64: {
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    bool Is64 = Ops[0]->getType()->getPrimitiveSizeInBits() == 64;
    llvm::Type *InTy = Is64 ? Int64Ty : Int32Ty;
    llvm::Type *FTy = Is64 ? DoubleTy : FloatTy;
    Ops[0] = Builder.CreateBitCast(Ops[0], InTy);
    if (usgn)
      return Builder.CreateUIToFP(Ops[0], FTy);
    return Builder.CreateSIToFP(Ops[0], FTy);
  }
  case NEON::BI__builtin_neon_vpaddd_s64: {
    llvm::Type *Ty =
      llvm::VectorType::get(llvm::Type::getInt64Ty(getLLVMContext()), 2);
    Value *Vec = EmitScalarExpr(E->getArg(0));
    // The vector is v2f64, so make sure it's bitcast to that.
    Vec = Builder.CreateBitCast(Vec, Ty, "v2i64");
    llvm::Value *Idx0 = llvm::ConstantInt::get(SizeTy, 0);
    llvm::Value *Idx1 = llvm::ConstantInt::get(SizeTy, 1);
    Value *Op0 = Builder.CreateExtractElement(Vec, Idx0, "lane0");
    Value *Op1 = Builder.CreateExtractElement(Vec, Idx1, "lane1");
    // Pairwise addition of a v2f64 into a scalar f64.
    return Builder.CreateAdd(Op0, Op1, "vpaddd");
  }
  case NEON::BI__builtin_neon_vpaddd_f64: {
    llvm::Type *Ty =
      llvm::VectorType::get(llvm::Type::getDoubleTy(getLLVMContext()), 2);
    Value *Vec = EmitScalarExpr(E->getArg(0));
    // The vector is v2f64, so make sure it's bitcast to that.
    Vec = Builder.CreateBitCast(Vec, Ty, "v2f64");
    llvm::Value *Idx0 = llvm::ConstantInt::get(SizeTy, 0);
    llvm::Value *Idx1 = llvm::ConstantInt::get(SizeTy, 1);
    Value *Op0 = Builder.CreateExtractElement(Vec, Idx0, "lane0");
    Value *Op1 = Builder.CreateExtractElement(Vec, Idx1, "lane1");
    // Pairwise addition of a v2f64 into a scalar f64.
    return Builder.CreateFAdd(Op0, Op1, "vpaddd");
  }
  case NEON::BI__builtin_neon_vpadds_f32: {
    llvm::Type *Ty =
      llvm::VectorType::get(llvm::Type::getFloatTy(getLLVMContext()), 2);
    Value *Vec = EmitScalarExpr(E->getArg(0));
    // The vector is v2f32, so make sure it's bitcast to that.
    Vec = Builder.CreateBitCast(Vec, Ty, "v2f32");
    llvm::Value *Idx0 = llvm::ConstantInt::get(SizeTy, 0);
    llvm::Value *Idx1 = llvm::ConstantInt::get(SizeTy, 1);
    Value *Op0 = Builder.CreateExtractElement(Vec, Idx0, "lane0");
    Value *Op1 = Builder.CreateExtractElement(Vec, Idx1, "lane1");
    // Pairwise addition of a v2f32 into a scalar f32.
    return Builder.CreateFAdd(Op0, Op1, "vpaddd");
  }
  case NEON::BI__builtin_neon_vceqzd_s64:
  case NEON::BI__builtin_neon_vceqzd_f64:
  case NEON::BI__builtin_neon_vceqzs_f32:
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    return EmitAArch64CompareBuiltinExpr(
        Ops[0], ConvertType(E->getCallReturnType(getContext())),
        ICmpInst::FCMP_OEQ, ICmpInst::ICMP_EQ, "vceqz");
  case NEON::BI__builtin_neon_vcgezd_s64:
  case NEON::BI__builtin_neon_vcgezd_f64:
  case NEON::BI__builtin_neon_vcgezs_f32:
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    return EmitAArch64CompareBuiltinExpr(
        Ops[0], ConvertType(E->getCallReturnType(getContext())),
        ICmpInst::FCMP_OGE, ICmpInst::ICMP_SGE, "vcgez");
  case NEON::BI__builtin_neon_vclezd_s64:
  case NEON::BI__builtin_neon_vclezd_f64:
  case NEON::BI__builtin_neon_vclezs_f32:
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    return EmitAArch64CompareBuiltinExpr(
        Ops[0], ConvertType(E->getCallReturnType(getContext())),
        ICmpInst::FCMP_OLE, ICmpInst::ICMP_SLE, "vclez");
  case NEON::BI__builtin_neon_vcgtzd_s64:
  case NEON::BI__builtin_neon_vcgtzd_f64:
  case NEON::BI__builtin_neon_vcgtzs_f32:
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    return EmitAArch64CompareBuiltinExpr(
        Ops[0], ConvertType(E->getCallReturnType(getContext())),
        ICmpInst::FCMP_OGT, ICmpInst::ICMP_SGT, "vcgtz");
  case NEON::BI__builtin_neon_vcltzd_s64:
  case NEON::BI__builtin_neon_vcltzd_f64:
  case NEON::BI__builtin_neon_vcltzs_f32:
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    return EmitAArch64CompareBuiltinExpr(
        Ops[0], ConvertType(E->getCallReturnType(getContext())),
        ICmpInst::FCMP_OLT, ICmpInst::ICMP_SLT, "vcltz");

  case NEON::BI__builtin_neon_vceqzd_u64: {
    llvm::Type *Ty = llvm::Type::getInt64Ty(getLLVMContext());
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    Ops[0] = Builder.CreateICmp(llvm::ICmpInst::ICMP_EQ, Ops[0],
                                llvm::Constant::getNullValue(Ty));
    return Builder.CreateSExt(Ops[0], Ty, "vceqzd");
  }
  case NEON::BI__builtin_neon_vceqd_f64:
  case NEON::BI__builtin_neon_vcled_f64:
  case NEON::BI__builtin_neon_vcltd_f64:
  case NEON::BI__builtin_neon_vcged_f64:
  case NEON::BI__builtin_neon_vcgtd_f64: {
    llvm::CmpInst::Predicate P;
    switch (BuiltinID) {
    default: llvm_unreachable("missing builtin ID in switch!");
    case NEON::BI__builtin_neon_vceqd_f64: P = llvm::FCmpInst::FCMP_OEQ; break;
    case NEON::BI__builtin_neon_vcled_f64: P = llvm::FCmpInst::FCMP_OLE; break;
    case NEON::BI__builtin_neon_vcltd_f64: P = llvm::FCmpInst::FCMP_OLT; break;
    case NEON::BI__builtin_neon_vcged_f64: P = llvm::FCmpInst::FCMP_OGE; break;
    case NEON::BI__builtin_neon_vcgtd_f64: P = llvm::FCmpInst::FCMP_OGT; break;
    }
    Ops.push_back(EmitScalarExpr(E->getArg(1)));
    Ops[0] = Builder.CreateBitCast(Ops[0], DoubleTy);
    Ops[1] = Builder.CreateBitCast(Ops[1], DoubleTy);
    Ops[0] = Builder.CreateFCmp(P, Ops[0], Ops[1]);
    return Builder.CreateSExt(Ops[0], Int64Ty, "vcmpd");
  }
  case NEON::BI__builtin_neon_vceqs_f32:
  case NEON::BI__builtin_neon_vcles_f32:
  case NEON::BI__builtin_neon_vclts_f32:
  case NEON::BI__builtin_neon_vcges_f32:
  case NEON::BI__builtin_neon_vcgts_f32: {
    llvm::CmpInst::Predicate P;
    switch (BuiltinID) {
    default: llvm_unreachable("missing builtin ID in switch!");
    case NEON::BI__builtin_neon_vceqs_f32: P = llvm::FCmpInst::FCMP_OEQ; break;
    case NEON::BI__builtin_neon_vcles_f32: P = llvm::FCmpInst::FCMP_OLE; break;
    case NEON::BI__builtin_neon_vclts_f32: P = llvm::FCmpInst::FCMP_OLT; break;
    case NEON::BI__builtin_neon_vcges_f32: P = llvm::FCmpInst::FCMP_OGE; break;
    case NEON::BI__builtin_neon_vcgts_f32: P = llvm::FCmpInst::FCMP_OGT; break;
    }
    Ops.push_back(EmitScalarExpr(E->getArg(1)));
    Ops[0] = Builder.CreateBitCast(Ops[0], FloatTy);
    Ops[1] = Builder.CreateBitCast(Ops[1], FloatTy);
    Ops[0] = Builder.CreateFCmp(P, Ops[0], Ops[1]);
    return Builder.CreateSExt(Ops[0], Int32Ty, "vcmpd");
  }
  case NEON::BI__builtin_neon_vceqd_s64:
  case NEON::BI__builtin_neon_vceqd_u64:
  case NEON::BI__builtin_neon_vcgtd_s64:
  case NEON::BI__builtin_neon_vcgtd_u64:
  case NEON::BI__builtin_neon_vcltd_s64:
  case NEON::BI__builtin_neon_vcltd_u64:
  case NEON::BI__builtin_neon_vcged_u64:
  case NEON::BI__builtin_neon_vcged_s64:
  case NEON::BI__builtin_neon_vcled_u64:
  case NEON::BI__builtin_neon_vcled_s64: {
    llvm::CmpInst::Predicate P;
    switch (BuiltinID) {
    default: llvm_unreachable("missing builtin ID in switch!");
    case NEON::BI__builtin_neon_vceqd_s64:
    case NEON::BI__builtin_neon_vceqd_u64:P = llvm::ICmpInst::ICMP_EQ;break;
    case NEON::BI__builtin_neon_vcgtd_s64:P = llvm::ICmpInst::ICMP_SGT;break;
    case NEON::BI__builtin_neon_vcgtd_u64:P = llvm::ICmpInst::ICMP_UGT;break;
    case NEON::BI__builtin_neon_vcltd_s64:P = llvm::ICmpInst::ICMP_SLT;break;
    case NEON::BI__builtin_neon_vcltd_u64:P = llvm::ICmpInst::ICMP_ULT;break;
    case NEON::BI__builtin_neon_vcged_u64:P = llvm::ICmpInst::ICMP_UGE;break;
    case NEON::BI__builtin_neon_vcged_s64:P = llvm::ICmpInst::ICMP_SGE;break;
    case NEON::BI__builtin_neon_vcled_u64:P = llvm::ICmpInst::ICMP_ULE;break;
    case NEON::BI__builtin_neon_vcled_s64:P = llvm::ICmpInst::ICMP_SLE;break;
    }
    Ops.push_back(EmitScalarExpr(E->getArg(1)));
    Ops[0] = Builder.CreateBitCast(Ops[0], Int64Ty);
    Ops[1] = Builder.CreateBitCast(Ops[1], Int64Ty);
    Ops[0] = Builder.CreateICmp(P, Ops[0], Ops[1]);
    return Builder.CreateSExt(Ops[0], Int64Ty, "vceqd");
  }
  case NEON::BI__builtin_neon_vtstd_s64:
  case NEON::BI__builtin_neon_vtstd_u64: {
    llvm::Type *Ty = llvm::Type::getInt64Ty(getLLVMContext());
    Ops.push_back(EmitScalarExpr(E->getArg(1)));
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Ops[0] = Builder.CreateAnd(Ops[0], Ops[1]);
    Ops[0] = Builder.CreateICmp(ICmpInst::ICMP_NE, Ops[0],
                                llvm::Constant::getNullValue(Ty));
    return Builder.CreateSExt(Ops[0], Ty, "vtstd");
  }
  case NEON::BI__builtin_neon_vset_lane_i8:
  case NEON::BI__builtin_neon_vset_lane_i16:
  case NEON::BI__builtin_neon_vset_lane_i32:
  case NEON::BI__builtin_neon_vset_lane_i64:
  case NEON::BI__builtin_neon_vset_lane_f32:
  case NEON::BI__builtin_neon_vsetq_lane_i8:
  case NEON::BI__builtin_neon_vsetq_lane_i16:
  case NEON::BI__builtin_neon_vsetq_lane_i32:
  case NEON::BI__builtin_neon_vsetq_lane_i64:
  case NEON::BI__builtin_neon_vsetq_lane_f32:
    Ops.push_back(EmitScalarExpr(E->getArg(2)));
    return Builder.CreateInsertElement(Ops[1], Ops[0], Ops[2], "vset_lane");
  case NEON::BI__builtin_neon_vset_lane_f64:
    // The vector type needs a cast for the v1f64 variant.
    Ops[1] = Builder.CreateBitCast(Ops[1],
                                   llvm::VectorType::get(DoubleTy, 1));
    Ops.push_back(EmitScalarExpr(E->getArg(2)));
    return Builder.CreateInsertElement(Ops[1], Ops[0], Ops[2], "vset_lane");
  case NEON::BI__builtin_neon_vsetq_lane_f64:
    // The vector type needs a cast for the v2f64 variant.
    Ops[1] = Builder.CreateBitCast(Ops[1],
        llvm::VectorType::get(llvm::Type::getDoubleTy(getLLVMContext()), 2));
    Ops.push_back(EmitScalarExpr(E->getArg(2)));
    return Builder.CreateInsertElement(Ops[1], Ops[0], Ops[2], "vset_lane");

  case NEON::BI__builtin_neon_vget_lane_i8:
  case NEON::BI__builtin_neon_vdupb_lane_i8:
    Ops[0] = Builder.CreateBitCast(Ops[0],
        llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 8), 8));
    return Builder.CreateExtractElement(Ops[0], EmitScalarExpr(E->getArg(1)),
                                        "vget_lane");
  case NEON::BI__builtin_neon_vgetq_lane_i8:
  case NEON::BI__builtin_neon_vdupb_laneq_i8:
    Ops[0] = Builder.CreateBitCast(Ops[0],
        llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 8), 16));
    return Builder.CreateExtractElement(Ops[0], EmitScalarExpr(E->getArg(1)),
                                        "vgetq_lane");
  case NEON::BI__builtin_neon_vget_lane_i16:
  case NEON::BI__builtin_neon_vduph_lane_i16:
    Ops[0] = Builder.CreateBitCast(Ops[0],
        llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 16), 4));
    return Builder.CreateExtractElement(Ops[0], EmitScalarExpr(E->getArg(1)),
                                        "vget_lane");
  case NEON::BI__builtin_neon_vgetq_lane_i16:
  case NEON::BI__builtin_neon_vduph_laneq_i16:
    Ops[0] = Builder.CreateBitCast(Ops[0],
        llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 16), 8));
    return Builder.CreateExtractElement(Ops[0], EmitScalarExpr(E->getArg(1)),
                                        "vgetq_lane");
  case NEON::BI__builtin_neon_vget_lane_i32:
  case NEON::BI__builtin_neon_vdups_lane_i32:
    Ops[0] = Builder.CreateBitCast(
        Ops[0],
        llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 32), 2));
    return Builder.CreateExtractElement(Ops[0], EmitScalarExpr(E->getArg(1)),
                                        "vget_lane");
  case NEON::BI__builtin_neon_vdups_lane_f32:
    Ops[0] = Builder.CreateBitCast(Ops[0],
        llvm::VectorType::get(llvm::Type::getFloatTy(getLLVMContext()), 2));
    return Builder.CreateExtractElement(Ops[0], EmitScalarExpr(E->getArg(1)),
                                        "vdups_lane");
  case NEON::BI__builtin_neon_vgetq_lane_i32:
  case NEON::BI__builtin_neon_vdups_laneq_i32:
    Ops[0] = Builder.CreateBitCast(Ops[0],
        llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 32), 4));
    return Builder.CreateExtractElement(Ops[0], EmitScalarExpr(E->getArg(1)),
                                        "vgetq_lane");
  case NEON::BI__builtin_neon_vget_lane_i64:
  case NEON::BI__builtin_neon_vdupd_lane_i64:
    Ops[0] = Builder.CreateBitCast(Ops[0],
        llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 64), 1));
    return Builder.CreateExtractElement(Ops[0], EmitScalarExpr(E->getArg(1)),
                                        "vget_lane");
  case NEON::BI__builtin_neon_vdupd_lane_f64:
    Ops[0] = Builder.CreateBitCast(Ops[0],
        llvm::VectorType::get(llvm::Type::getDoubleTy(getLLVMContext()), 1));
    return Builder.CreateExtractElement(Ops[0], EmitScalarExpr(E->getArg(1)),
                                        "vdupd_lane");
  case NEON::BI__builtin_neon_vgetq_lane_i64:
  case NEON::BI__builtin_neon_vdupd_laneq_i64:
    Ops[0] = Builder.CreateBitCast(Ops[0],
        llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 64), 2));
    return Builder.CreateExtractElement(Ops[0], EmitScalarExpr(E->getArg(1)),
                                        "vgetq_lane");
  case NEON::BI__builtin_neon_vget_lane_f32:
    Ops[0] = Builder.CreateBitCast(Ops[0],
        llvm::VectorType::get(llvm::Type::getFloatTy(getLLVMContext()), 2));
    return Builder.CreateExtractElement(Ops[0], EmitScalarExpr(E->getArg(1)),
                                        "vget_lane");
  case NEON::BI__builtin_neon_vget_lane_f64:
    Ops[0] = Builder.CreateBitCast(Ops[0],
        llvm::VectorType::get(llvm::Type::getDoubleTy(getLLVMContext()), 1));
    return Builder.CreateExtractElement(Ops[0], EmitScalarExpr(E->getArg(1)),
                                        "vget_lane");
  case NEON::BI__builtin_neon_vgetq_lane_f32:
  case NEON::BI__builtin_neon_vdups_laneq_f32:
    Ops[0] = Builder.CreateBitCast(Ops[0],
        llvm::VectorType::get(llvm::Type::getFloatTy(getLLVMContext()), 4));
    return Builder.CreateExtractElement(Ops[0], EmitScalarExpr(E->getArg(1)),
                                        "vgetq_lane");
  case NEON::BI__builtin_neon_vgetq_lane_f64:
  case NEON::BI__builtin_neon_vdupd_laneq_f64:
    Ops[0] = Builder.CreateBitCast(Ops[0],
        llvm::VectorType::get(llvm::Type::getDoubleTy(getLLVMContext()), 2));
    return Builder.CreateExtractElement(Ops[0], EmitScalarExpr(E->getArg(1)),
                                        "vgetq_lane");
  case NEON::BI__builtin_neon_vaddd_s64:
  case NEON::BI__builtin_neon_vaddd_u64:
    return Builder.CreateAdd(Ops[0], EmitScalarExpr(E->getArg(1)), "vaddd");
  case NEON::BI__builtin_neon_vsubd_s64:
  case NEON::BI__builtin_neon_vsubd_u64:
    return Builder.CreateSub(Ops[0], EmitScalarExpr(E->getArg(1)), "vsubd");
  case NEON::BI__builtin_neon_vqdmlalh_s16:
  case NEON::BI__builtin_neon_vqdmlslh_s16: {
    SmallVector<Value *, 2> ProductOps;
    ProductOps.push_back(vectorWrapScalar16(Ops[1]));
    ProductOps.push_back(vectorWrapScalar16(EmitScalarExpr(E->getArg(2))));
    llvm::Type *VTy = llvm::VectorType::get(Int32Ty, 4);
    Ops[1] = EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_sqdmull, VTy),
                          ProductOps, "vqdmlXl");
    Constant *CI = ConstantInt::get(SizeTy, 0);
    Ops[1] = Builder.CreateExtractElement(Ops[1], CI, "lane0");

    unsigned AccumInt = BuiltinID == NEON::BI__builtin_neon_vqdmlalh_s16
                                        ? Intrinsic::aarch64_neon_sqadd
                                        : Intrinsic::aarch64_neon_sqsub;
    return EmitNeonCall(CGM.getIntrinsic(AccumInt, Int32Ty), Ops, "vqdmlXl");
  }
  case NEON::BI__builtin_neon_vqshlud_n_s64: {
    Ops.push_back(EmitScalarExpr(E->getArg(1)));
    Ops[1] = Builder.CreateZExt(Ops[1], Int64Ty);
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_sqshlu, Int64Ty),
                        Ops, "vqshlu_n");
  }
  case NEON::BI__builtin_neon_vqshld_n_u64:
  case NEON::BI__builtin_neon_vqshld_n_s64: {
    unsigned Int = BuiltinID == NEON::BI__builtin_neon_vqshld_n_u64
                                   ? Intrinsic::aarch64_neon_uqshl
                                   : Intrinsic::aarch64_neon_sqshl;
    Ops.push_back(EmitScalarExpr(E->getArg(1)));
    Ops[1] = Builder.CreateZExt(Ops[1], Int64Ty);
    return EmitNeonCall(CGM.getIntrinsic(Int, Int64Ty), Ops, "vqshl_n");
  }
  case NEON::BI__builtin_neon_vrshrd_n_u64:
  case NEON::BI__builtin_neon_vrshrd_n_s64: {
    unsigned Int = BuiltinID == NEON::BI__builtin_neon_vrshrd_n_u64
                                   ? Intrinsic::aarch64_neon_urshl
                                   : Intrinsic::aarch64_neon_srshl;
    Ops.push_back(EmitScalarExpr(E->getArg(1)));
    int SV = cast<ConstantInt>(Ops[1])->getSExtValue();
    Ops[1] = ConstantInt::get(Int64Ty, -SV);
    return EmitNeonCall(CGM.getIntrinsic(Int, Int64Ty), Ops, "vrshr_n");
  }
  case NEON::BI__builtin_neon_vrsrad_n_u64:
  case NEON::BI__builtin_neon_vrsrad_n_s64: {
    unsigned Int = BuiltinID == NEON::BI__builtin_neon_vrsrad_n_u64
                                   ? Intrinsic::aarch64_neon_urshl
                                   : Intrinsic::aarch64_neon_srshl;
    Ops[1] = Builder.CreateBitCast(Ops[1], Int64Ty);
    Ops.push_back(Builder.CreateNeg(EmitScalarExpr(E->getArg(2))));
    Ops[1] = Builder.CreateCall(CGM.getIntrinsic(Int, Int64Ty),
                                {Ops[1], Builder.CreateSExt(Ops[2], Int64Ty)});
    return Builder.CreateAdd(Ops[0], Builder.CreateBitCast(Ops[1], Int64Ty));
  }
  case NEON::BI__builtin_neon_vshld_n_s64:
  case NEON::BI__builtin_neon_vshld_n_u64: {
    llvm::ConstantInt *Amt = cast<ConstantInt>(EmitScalarExpr(E->getArg(1)));
    return Builder.CreateShl(
        Ops[0], ConstantInt::get(Int64Ty, Amt->getZExtValue()), "shld_n");
  }
  case NEON::BI__builtin_neon_vshrd_n_s64: {
    llvm::ConstantInt *Amt = cast<ConstantInt>(EmitScalarExpr(E->getArg(1)));
    return Builder.CreateAShr(
        Ops[0], ConstantInt::get(Int64Ty, std::min(static_cast<uint64_t>(63),
                                                   Amt->getZExtValue())),
        "shrd_n");
  }
  case NEON::BI__builtin_neon_vshrd_n_u64: {
    llvm::ConstantInt *Amt = cast<ConstantInt>(EmitScalarExpr(E->getArg(1)));
    uint64_t ShiftAmt = Amt->getZExtValue();
    // Right-shifting an unsigned value by its size yields 0.
    if (ShiftAmt == 64)
      return ConstantInt::get(Int64Ty, 0);
    return Builder.CreateLShr(Ops[0], ConstantInt::get(Int64Ty, ShiftAmt),
                              "shrd_n");
  }
  case NEON::BI__builtin_neon_vsrad_n_s64: {
    llvm::ConstantInt *Amt = cast<ConstantInt>(EmitScalarExpr(E->getArg(2)));
    Ops[1] = Builder.CreateAShr(
        Ops[1], ConstantInt::get(Int64Ty, std::min(static_cast<uint64_t>(63),
                                                   Amt->getZExtValue())),
        "shrd_n");
    return Builder.CreateAdd(Ops[0], Ops[1]);
  }
  case NEON::BI__builtin_neon_vsrad_n_u64: {
    llvm::ConstantInt *Amt = cast<ConstantInt>(EmitScalarExpr(E->getArg(2)));
    uint64_t ShiftAmt = Amt->getZExtValue();
    // Right-shifting an unsigned value by its size yields 0.
    // As Op + 0 = Op, return Ops[0] directly.
    if (ShiftAmt == 64)
      return Ops[0];
    Ops[1] = Builder.CreateLShr(Ops[1], ConstantInt::get(Int64Ty, ShiftAmt),
                                "shrd_n");
    return Builder.CreateAdd(Ops[0], Ops[1]);
  }
  case NEON::BI__builtin_neon_vqdmlalh_lane_s16:
  case NEON::BI__builtin_neon_vqdmlalh_laneq_s16:
  case NEON::BI__builtin_neon_vqdmlslh_lane_s16:
  case NEON::BI__builtin_neon_vqdmlslh_laneq_s16: {
    Ops[2] = Builder.CreateExtractElement(Ops[2], EmitScalarExpr(E->getArg(3)),
                                          "lane");
    SmallVector<Value *, 2> ProductOps;
    ProductOps.push_back(vectorWrapScalar16(Ops[1]));
    ProductOps.push_back(vectorWrapScalar16(Ops[2]));
    llvm::Type *VTy = llvm::VectorType::get(Int32Ty, 4);
    Ops[1] = EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_sqdmull, VTy),
                          ProductOps, "vqdmlXl");
    Constant *CI = ConstantInt::get(SizeTy, 0);
    Ops[1] = Builder.CreateExtractElement(Ops[1], CI, "lane0");
    Ops.pop_back();

    unsigned AccInt = (BuiltinID == NEON::BI__builtin_neon_vqdmlalh_lane_s16 ||
                       BuiltinID == NEON::BI__builtin_neon_vqdmlalh_laneq_s16)
                          ? Intrinsic::aarch64_neon_sqadd
                          : Intrinsic::aarch64_neon_sqsub;
    return EmitNeonCall(CGM.getIntrinsic(AccInt, Int32Ty), Ops, "vqdmlXl");
  }
  case NEON::BI__builtin_neon_vqdmlals_s32:
  case NEON::BI__builtin_neon_vqdmlsls_s32: {
    SmallVector<Value *, 2> ProductOps;
    ProductOps.push_back(Ops[1]);
    ProductOps.push_back(EmitScalarExpr(E->getArg(2)));
    Ops[1] =
        EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_sqdmulls_scalar),
                     ProductOps, "vqdmlXl");

    unsigned AccumInt = BuiltinID == NEON::BI__builtin_neon_vqdmlals_s32
                                        ? Intrinsic::aarch64_neon_sqadd
                                        : Intrinsic::aarch64_neon_sqsub;
    return EmitNeonCall(CGM.getIntrinsic(AccumInt, Int64Ty), Ops, "vqdmlXl");
  }
  case NEON::BI__builtin_neon_vqdmlals_lane_s32:
  case NEON::BI__builtin_neon_vqdmlals_laneq_s32:
  case NEON::BI__builtin_neon_vqdmlsls_lane_s32:
  case NEON::BI__builtin_neon_vqdmlsls_laneq_s32: {
    Ops[2] = Builder.CreateExtractElement(Ops[2], EmitScalarExpr(E->getArg(3)),
                                          "lane");
    SmallVector<Value *, 2> ProductOps;
    ProductOps.push_back(Ops[1]);
    ProductOps.push_back(Ops[2]);
    Ops[1] =
        EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_sqdmulls_scalar),
                     ProductOps, "vqdmlXl");
    Ops.pop_back();

    unsigned AccInt = (BuiltinID == NEON::BI__builtin_neon_vqdmlals_lane_s32 ||
                       BuiltinID == NEON::BI__builtin_neon_vqdmlals_laneq_s32)
                          ? Intrinsic::aarch64_neon_sqadd
                          : Intrinsic::aarch64_neon_sqsub;
    return EmitNeonCall(CGM.getIntrinsic(AccInt, Int64Ty), Ops, "vqdmlXl");
  }
  }

  llvm::VectorType *VTy = GetNeonType(this, Type);
  llvm::Type *Ty = VTy;
  if (!Ty)
    return nullptr;

  // Not all intrinsics handled by the common case work for AArch64 yet, so only
  // defer to common code if it's been added to our special map.
  Builtin = findNeonIntrinsicInMap(AArch64SIMDIntrinsicMap, BuiltinID,
                                   AArch64SIMDIntrinsicsProvenSorted);

  if (Builtin)
    return EmitCommonNeonBuiltinExpr(
        Builtin->BuiltinID, Builtin->LLVMIntrinsic, Builtin->AltLLVMIntrinsic,
        Builtin->NameHint, Builtin->TypeModifier, E, Ops, nullptr);

  if (Value *V = EmitAArch64TblBuiltinExpr(*this, BuiltinID, E, Ops))
    return V;

  unsigned Int;
  switch (BuiltinID) {
  default: return nullptr;
  case NEON::BI__builtin_neon_vbsl_v:
  case NEON::BI__builtin_neon_vbslq_v: {
    llvm::Type *BitTy = llvm::VectorType::getInteger(VTy);
    Ops[0] = Builder.CreateBitCast(Ops[0], BitTy, "vbsl");
    Ops[1] = Builder.CreateBitCast(Ops[1], BitTy, "vbsl");
    Ops[2] = Builder.CreateBitCast(Ops[2], BitTy, "vbsl");

    Ops[1] = Builder.CreateAnd(Ops[0], Ops[1], "vbsl");
    Ops[2] = Builder.CreateAnd(Builder.CreateNot(Ops[0]), Ops[2], "vbsl");
    Ops[0] = Builder.CreateOr(Ops[1], Ops[2], "vbsl");
    return Builder.CreateBitCast(Ops[0], Ty);
  }
  case NEON::BI__builtin_neon_vfma_lane_v:
  case NEON::BI__builtin_neon_vfmaq_lane_v: { // Only used for FP types
    // The ARM builtins (and instructions) have the addend as the first
    // operand, but the 'fma' intrinsics have it last. Swap it around here.
    Value *Addend = Ops[0];
    Value *Multiplicand = Ops[1];
    Value *LaneSource = Ops[2];
    Ops[0] = Multiplicand;
    Ops[1] = LaneSource;
    Ops[2] = Addend;

    // Now adjust things to handle the lane access.
    llvm::Type *SourceTy = BuiltinID == NEON::BI__builtin_neon_vfmaq_lane_v ?
      llvm::VectorType::get(VTy->getElementType(), VTy->getNumElements() / 2) :
      VTy;
    llvm::Constant *cst = cast<Constant>(Ops[3]);
    Value *SV = llvm::ConstantVector::getSplat(VTy->getNumElements(), cst);
    Ops[1] = Builder.CreateBitCast(Ops[1], SourceTy);
    Ops[1] = Builder.CreateShuffleVector(Ops[1], Ops[1], SV, "lane");

    Ops.pop_back();
    Int = Intrinsic::fma;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "fmla");
  }
  case NEON::BI__builtin_neon_vfma_laneq_v: {
    llvm::VectorType *VTy = cast<llvm::VectorType>(Ty);
    // v1f64 fma should be mapped to Neon scalar f64 fma
    if (VTy && VTy->getElementType() == DoubleTy) {
      Ops[0] = Builder.CreateBitCast(Ops[0], DoubleTy);
      Ops[1] = Builder.CreateBitCast(Ops[1], DoubleTy);
      llvm::Type *VTy = GetNeonType(this,
        NeonTypeFlags(NeonTypeFlags::Float64, false, true));
      Ops[2] = Builder.CreateBitCast(Ops[2], VTy);
      Ops[2] = Builder.CreateExtractElement(Ops[2], Ops[3], "extract");
      Value *F = CGM.getIntrinsic(Intrinsic::fma, DoubleTy);
      Value *Result = Builder.CreateCall(F, {Ops[1], Ops[2], Ops[0]});
      return Builder.CreateBitCast(Result, Ty);
    }
    Value *F = CGM.getIntrinsic(Intrinsic::fma, Ty);
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);

    llvm::Type *STy = llvm::VectorType::get(VTy->getElementType(),
                                            VTy->getNumElements() * 2);
    Ops[2] = Builder.CreateBitCast(Ops[2], STy);
    Value* SV = llvm::ConstantVector::getSplat(VTy->getNumElements(),
                                               cast<ConstantInt>(Ops[3]));
    Ops[2] = Builder.CreateShuffleVector(Ops[2], Ops[2], SV, "lane");

    return Builder.CreateCall(F, {Ops[2], Ops[1], Ops[0]});
  }
  case NEON::BI__builtin_neon_vfmaq_laneq_v: {
    Value *F = CGM.getIntrinsic(Intrinsic::fma, Ty);
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);

    Ops[2] = Builder.CreateBitCast(Ops[2], Ty);
    Ops[2] = EmitNeonSplat(Ops[2], cast<ConstantInt>(Ops[3]));
    return Builder.CreateCall(F, {Ops[2], Ops[1], Ops[0]});
  }
  case NEON::BI__builtin_neon_vfmas_lane_f32:
  case NEON::BI__builtin_neon_vfmas_laneq_f32:
  case NEON::BI__builtin_neon_vfmad_lane_f64:
  case NEON::BI__builtin_neon_vfmad_laneq_f64: {
    Ops.push_back(EmitScalarExpr(E->getArg(3)));
    llvm::Type *Ty = ConvertType(E->getCallReturnType(getContext()));
    Value *F = CGM.getIntrinsic(Intrinsic::fma, Ty);
    Ops[2] = Builder.CreateExtractElement(Ops[2], Ops[3], "extract");
    return Builder.CreateCall(F, {Ops[1], Ops[2], Ops[0]});
  }
  case NEON::BI__builtin_neon_vfms_v:
  case NEON::BI__builtin_neon_vfmsq_v: {  // Only used for FP types
    // FIXME: probably remove when we no longer support aarch64_simd.h
    // (arm_neon.h delegates to vfma).

    // The ARM builtins (and instructions) have the addend as the first
    // operand, but the 'fma' intrinsics have it last. Swap it around here.
    Value *Subtrahend = Ops[0];
    Value *Multiplicand = Ops[2];
    Ops[0] = Multiplicand;
    Ops[2] = Subtrahend;
    Ops[1] = Builder.CreateBitCast(Ops[1], VTy);
    Ops[1] = Builder.CreateFNeg(Ops[1]);
    Int = Intrinsic::fma;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "fmls");
  }
  case NEON::BI__builtin_neon_vmull_v:
    // FIXME: improve sharing scheme to cope with 3 alternative LLVM intrinsics.
    Int = usgn ? Intrinsic::aarch64_neon_umull : Intrinsic::aarch64_neon_smull;
    if (Type.isPoly()) Int = Intrinsic::aarch64_neon_pmull;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vmull");
  case NEON::BI__builtin_neon_vmax_v:
  case NEON::BI__builtin_neon_vmaxq_v:
    // FIXME: improve sharing scheme to cope with 3 alternative LLVM intrinsics.
    Int = usgn ? Intrinsic::aarch64_neon_umax : Intrinsic::aarch64_neon_smax;
    if (Ty->isFPOrFPVectorTy()) Int = Intrinsic::aarch64_neon_fmax;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vmax");
  case NEON::BI__builtin_neon_vmin_v:
  case NEON::BI__builtin_neon_vminq_v:
    // FIXME: improve sharing scheme to cope with 3 alternative LLVM intrinsics.
    Int = usgn ? Intrinsic::aarch64_neon_umin : Intrinsic::aarch64_neon_smin;
    if (Ty->isFPOrFPVectorTy()) Int = Intrinsic::aarch64_neon_fmin;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vmin");
  case NEON::BI__builtin_neon_vabd_v:
  case NEON::BI__builtin_neon_vabdq_v:
    // FIXME: improve sharing scheme to cope with 3 alternative LLVM intrinsics.
    Int = usgn ? Intrinsic::aarch64_neon_uabd : Intrinsic::aarch64_neon_sabd;
    if (Ty->isFPOrFPVectorTy()) Int = Intrinsic::aarch64_neon_fabd;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vabd");
  case NEON::BI__builtin_neon_vpadal_v:
  case NEON::BI__builtin_neon_vpadalq_v: {
    unsigned ArgElts = VTy->getNumElements();
    llvm::IntegerType *EltTy = cast<IntegerType>(VTy->getElementType());
    unsigned BitWidth = EltTy->getBitWidth();
    llvm::Type *ArgTy = llvm::VectorType::get(
        llvm::IntegerType::get(getLLVMContext(), BitWidth/2), 2*ArgElts);
    llvm::Type* Tys[2] = { VTy, ArgTy };
    Int = usgn ? Intrinsic::aarch64_neon_uaddlp : Intrinsic::aarch64_neon_saddlp;
    SmallVector<llvm::Value*, 1> TmpOps;
    TmpOps.push_back(Ops[1]);
    Function *F = CGM.getIntrinsic(Int, Tys);
    llvm::Value *tmp = EmitNeonCall(F, TmpOps, "vpadal");
    llvm::Value *addend = Builder.CreateBitCast(Ops[0], tmp->getType());
    return Builder.CreateAdd(tmp, addend);
  }
  case NEON::BI__builtin_neon_vpmin_v:
  case NEON::BI__builtin_neon_vpminq_v:
    // FIXME: improve sharing scheme to cope with 3 alternative LLVM intrinsics.
    Int = usgn ? Intrinsic::aarch64_neon_uminp : Intrinsic::aarch64_neon_sminp;
    if (Ty->isFPOrFPVectorTy()) Int = Intrinsic::aarch64_neon_fminp;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vpmin");
  case NEON::BI__builtin_neon_vpmax_v:
  case NEON::BI__builtin_neon_vpmaxq_v:
    // FIXME: improve sharing scheme to cope with 3 alternative LLVM intrinsics.
    Int = usgn ? Intrinsic::aarch64_neon_umaxp : Intrinsic::aarch64_neon_smaxp;
    if (Ty->isFPOrFPVectorTy()) Int = Intrinsic::aarch64_neon_fmaxp;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vpmax");
  case NEON::BI__builtin_neon_vminnm_v:
  case NEON::BI__builtin_neon_vminnmq_v:
    Int = Intrinsic::aarch64_neon_fminnm;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vminnm");
  case NEON::BI__builtin_neon_vmaxnm_v:
  case NEON::BI__builtin_neon_vmaxnmq_v:
    Int = Intrinsic::aarch64_neon_fmaxnm;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vmaxnm");
  case NEON::BI__builtin_neon_vrecpss_f32: {
    llvm::Type *f32Type = llvm::Type::getFloatTy(getLLVMContext());
    Ops.push_back(EmitScalarExpr(E->getArg(1)));
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_frecps, f32Type),
                        Ops, "vrecps");
  }
  case NEON::BI__builtin_neon_vrecpsd_f64: {
    llvm::Type *f64Type = llvm::Type::getDoubleTy(getLLVMContext());
    Ops.push_back(EmitScalarExpr(E->getArg(1)));
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_frecps, f64Type),
                        Ops, "vrecps");
  }
  case NEON::BI__builtin_neon_vqshrun_n_v:
    Int = Intrinsic::aarch64_neon_sqshrun;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vqshrun_n");
  case NEON::BI__builtin_neon_vqrshrun_n_v:
    Int = Intrinsic::aarch64_neon_sqrshrun;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vqrshrun_n");
  case NEON::BI__builtin_neon_vqshrn_n_v:
    Int = usgn ? Intrinsic::aarch64_neon_uqshrn : Intrinsic::aarch64_neon_sqshrn;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vqshrn_n");
  case NEON::BI__builtin_neon_vrshrn_n_v:
    Int = Intrinsic::aarch64_neon_rshrn;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vrshrn_n");
  case NEON::BI__builtin_neon_vqrshrn_n_v:
    Int = usgn ? Intrinsic::aarch64_neon_uqrshrn : Intrinsic::aarch64_neon_sqrshrn;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vqrshrn_n");
  case NEON::BI__builtin_neon_vrnda_v:
  case NEON::BI__builtin_neon_vrndaq_v: {
    Int = Intrinsic::round;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vrnda");
  }
  case NEON::BI__builtin_neon_vrndi_v:
  case NEON::BI__builtin_neon_vrndiq_v: {
    Int = Intrinsic::nearbyint;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vrndi");
  }
  case NEON::BI__builtin_neon_vrndm_v:
  case NEON::BI__builtin_neon_vrndmq_v: {
    Int = Intrinsic::floor;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vrndm");
  }
  case NEON::BI__builtin_neon_vrndn_v:
  case NEON::BI__builtin_neon_vrndnq_v: {
    Int = Intrinsic::aarch64_neon_frintn;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vrndn");
  }
  case NEON::BI__builtin_neon_vrndp_v:
  case NEON::BI__builtin_neon_vrndpq_v: {
    Int = Intrinsic::ceil;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vrndp");
  }
  case NEON::BI__builtin_neon_vrndx_v:
  case NEON::BI__builtin_neon_vrndxq_v: {
    Int = Intrinsic::rint;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vrndx");
  }
  case NEON::BI__builtin_neon_vrnd_v:
  case NEON::BI__builtin_neon_vrndq_v: {
    Int = Intrinsic::trunc;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vrndz");
  }
  case NEON::BI__builtin_neon_vceqz_v:
  case NEON::BI__builtin_neon_vceqzq_v:
    return EmitAArch64CompareBuiltinExpr(Ops[0], Ty, ICmpInst::FCMP_OEQ,
                                         ICmpInst::ICMP_EQ, "vceqz");
  case NEON::BI__builtin_neon_vcgez_v:
  case NEON::BI__builtin_neon_vcgezq_v:
    return EmitAArch64CompareBuiltinExpr(Ops[0], Ty, ICmpInst::FCMP_OGE,
                                         ICmpInst::ICMP_SGE, "vcgez");
  case NEON::BI__builtin_neon_vclez_v:
  case NEON::BI__builtin_neon_vclezq_v:
    return EmitAArch64CompareBuiltinExpr(Ops[0], Ty, ICmpInst::FCMP_OLE,
                                         ICmpInst::ICMP_SLE, "vclez");
  case NEON::BI__builtin_neon_vcgtz_v:
  case NEON::BI__builtin_neon_vcgtzq_v:
    return EmitAArch64CompareBuiltinExpr(Ops[0], Ty, ICmpInst::FCMP_OGT,
                                         ICmpInst::ICMP_SGT, "vcgtz");
  case NEON::BI__builtin_neon_vcltz_v:
  case NEON::BI__builtin_neon_vcltzq_v:
    return EmitAArch64CompareBuiltinExpr(Ops[0], Ty, ICmpInst::FCMP_OLT,
                                         ICmpInst::ICMP_SLT, "vcltz");
  case NEON::BI__builtin_neon_vcvt_f64_v:
  case NEON::BI__builtin_neon_vcvtq_f64_v:
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    Ty = GetNeonType(this, NeonTypeFlags(NeonTypeFlags::Float64, false, quad));
    return usgn ? Builder.CreateUIToFP(Ops[0], Ty, "vcvt")
                : Builder.CreateSIToFP(Ops[0], Ty, "vcvt");
  case NEON::BI__builtin_neon_vcvt_f64_f32: {
    assert(Type.getEltType() == NeonTypeFlags::Float64 && quad &&
           "unexpected vcvt_f64_f32 builtin");
    NeonTypeFlags SrcFlag = NeonTypeFlags(NeonTypeFlags::Float32, false, false);
    Ops[0] = Builder.CreateBitCast(Ops[0], GetNeonType(this, SrcFlag));

    return Builder.CreateFPExt(Ops[0], Ty, "vcvt");
  }
  case NEON::BI__builtin_neon_vcvt_f32_f64: {
    assert(Type.getEltType() == NeonTypeFlags::Float32 &&
           "unexpected vcvt_f32_f64 builtin");
    NeonTypeFlags SrcFlag = NeonTypeFlags(NeonTypeFlags::Float64, false, true);
    Ops[0] = Builder.CreateBitCast(Ops[0], GetNeonType(this, SrcFlag));

    return Builder.CreateFPTrunc(Ops[0], Ty, "vcvt");
  }
  case NEON::BI__builtin_neon_vcvt_s32_v:
  case NEON::BI__builtin_neon_vcvt_u32_v:
  case NEON::BI__builtin_neon_vcvt_s64_v:
  case NEON::BI__builtin_neon_vcvt_u64_v:
  case NEON::BI__builtin_neon_vcvtq_s32_v:
  case NEON::BI__builtin_neon_vcvtq_u32_v:
  case NEON::BI__builtin_neon_vcvtq_s64_v:
  case NEON::BI__builtin_neon_vcvtq_u64_v: {
    bool Double =
      (cast<llvm::IntegerType>(VTy->getElementType())->getBitWidth() == 64);
    llvm::Type *InTy =
      GetNeonType(this,
                  NeonTypeFlags(Double ? NeonTypeFlags::Float64
                                : NeonTypeFlags::Float32, false, quad));
    Ops[0] = Builder.CreateBitCast(Ops[0], InTy);
    if (usgn)
      return Builder.CreateFPToUI(Ops[0], Ty);
    return Builder.CreateFPToSI(Ops[0], Ty);
  }
  case NEON::BI__builtin_neon_vcvta_s32_v:
  case NEON::BI__builtin_neon_vcvtaq_s32_v:
  case NEON::BI__builtin_neon_vcvta_u32_v:
  case NEON::BI__builtin_neon_vcvtaq_u32_v:
  case NEON::BI__builtin_neon_vcvta_s64_v:
  case NEON::BI__builtin_neon_vcvtaq_s64_v:
  case NEON::BI__builtin_neon_vcvta_u64_v:
  case NEON::BI__builtin_neon_vcvtaq_u64_v: {
    Int = usgn ? Intrinsic::aarch64_neon_fcvtau : Intrinsic::aarch64_neon_fcvtas;
    bool Double =
      (cast<llvm::IntegerType>(VTy->getElementType())->getBitWidth() == 64);
    llvm::Type *InTy =
      GetNeonType(this,
                  NeonTypeFlags(Double ? NeonTypeFlags::Float64
                                : NeonTypeFlags::Float32, false, quad));
    llvm::Type *Tys[2] = { Ty, InTy };
    return EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vcvta");
  }
  case NEON::BI__builtin_neon_vcvtm_s32_v:
  case NEON::BI__builtin_neon_vcvtmq_s32_v:
  case NEON::BI__builtin_neon_vcvtm_u32_v:
  case NEON::BI__builtin_neon_vcvtmq_u32_v:
  case NEON::BI__builtin_neon_vcvtm_s64_v:
  case NEON::BI__builtin_neon_vcvtmq_s64_v:
  case NEON::BI__builtin_neon_vcvtm_u64_v:
  case NEON::BI__builtin_neon_vcvtmq_u64_v: {
    Int = usgn ? Intrinsic::aarch64_neon_fcvtmu : Intrinsic::aarch64_neon_fcvtms;
    bool Double =
      (cast<llvm::IntegerType>(VTy->getElementType())->getBitWidth() == 64);
    llvm::Type *InTy =
      GetNeonType(this,
                  NeonTypeFlags(Double ? NeonTypeFlags::Float64
                                : NeonTypeFlags::Float32, false, quad));
    llvm::Type *Tys[2] = { Ty, InTy };
    return EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vcvtm");
  }
  case NEON::BI__builtin_neon_vcvtn_s32_v:
  case NEON::BI__builtin_neon_vcvtnq_s32_v:
  case NEON::BI__builtin_neon_vcvtn_u32_v:
  case NEON::BI__builtin_neon_vcvtnq_u32_v:
  case NEON::BI__builtin_neon_vcvtn_s64_v:
  case NEON::BI__builtin_neon_vcvtnq_s64_v:
  case NEON::BI__builtin_neon_vcvtn_u64_v:
  case NEON::BI__builtin_neon_vcvtnq_u64_v: {
    Int = usgn ? Intrinsic::aarch64_neon_fcvtnu : Intrinsic::aarch64_neon_fcvtns;
    bool Double =
      (cast<llvm::IntegerType>(VTy->getElementType())->getBitWidth() == 64);
    llvm::Type *InTy =
      GetNeonType(this,
                  NeonTypeFlags(Double ? NeonTypeFlags::Float64
                                : NeonTypeFlags::Float32, false, quad));
    llvm::Type *Tys[2] = { Ty, InTy };
    return EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vcvtn");
  }
  case NEON::BI__builtin_neon_vcvtp_s32_v:
  case NEON::BI__builtin_neon_vcvtpq_s32_v:
  case NEON::BI__builtin_neon_vcvtp_u32_v:
  case NEON::BI__builtin_neon_vcvtpq_u32_v:
  case NEON::BI__builtin_neon_vcvtp_s64_v:
  case NEON::BI__builtin_neon_vcvtpq_s64_v:
  case NEON::BI__builtin_neon_vcvtp_u64_v:
  case NEON::BI__builtin_neon_vcvtpq_u64_v: {
    Int = usgn ? Intrinsic::aarch64_neon_fcvtpu : Intrinsic::aarch64_neon_fcvtps;
    bool Double =
      (cast<llvm::IntegerType>(VTy->getElementType())->getBitWidth() == 64);
    llvm::Type *InTy =
      GetNeonType(this,
                  NeonTypeFlags(Double ? NeonTypeFlags::Float64
                                : NeonTypeFlags::Float32, false, quad));
    llvm::Type *Tys[2] = { Ty, InTy };
    return EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vcvtp");
  }
  case NEON::BI__builtin_neon_vmulx_v:
  case NEON::BI__builtin_neon_vmulxq_v: {
    Int = Intrinsic::aarch64_neon_fmulx;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vmulx");
  }
  case NEON::BI__builtin_neon_vmul_lane_v:
  case NEON::BI__builtin_neon_vmul_laneq_v: {
    // v1f64 vmul_lane should be mapped to Neon scalar mul lane
    bool Quad = false;
    if (BuiltinID == NEON::BI__builtin_neon_vmul_laneq_v)
      Quad = true;
    Ops[0] = Builder.CreateBitCast(Ops[0], DoubleTy);
    llvm::Type *VTy = GetNeonType(this,
      NeonTypeFlags(NeonTypeFlags::Float64, false, Quad));
    Ops[1] = Builder.CreateBitCast(Ops[1], VTy);
    Ops[1] = Builder.CreateExtractElement(Ops[1], Ops[2], "extract");
    Value *Result = Builder.CreateFMul(Ops[0], Ops[1]);
    return Builder.CreateBitCast(Result, Ty);
  }
  case NEON::BI__builtin_neon_vnegd_s64:
    return Builder.CreateNeg(EmitScalarExpr(E->getArg(0)), "vnegd");
  case NEON::BI__builtin_neon_vpmaxnm_v:
  case NEON::BI__builtin_neon_vpmaxnmq_v: {
    Int = Intrinsic::aarch64_neon_fmaxnmp;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vpmaxnm");
  }
  case NEON::BI__builtin_neon_vpminnm_v:
  case NEON::BI__builtin_neon_vpminnmq_v: {
    Int = Intrinsic::aarch64_neon_fminnmp;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vpminnm");
  }
  case NEON::BI__builtin_neon_vsqrt_v:
  case NEON::BI__builtin_neon_vsqrtq_v: {
    Int = Intrinsic::sqrt;
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vsqrt");
  }
  case NEON::BI__builtin_neon_vrbit_v:
  case NEON::BI__builtin_neon_vrbitq_v: {
    Int = Intrinsic::aarch64_neon_rbit;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vrbit");
  }
  case NEON::BI__builtin_neon_vaddv_u8:
    // FIXME: These are handled by the AArch64 scalar code.
    usgn = true;
    // FALLTHROUGH
  case NEON::BI__builtin_neon_vaddv_s8: {
    Int = usgn ? Intrinsic::aarch64_neon_uaddv : Intrinsic::aarch64_neon_saddv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 8), 8);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vaddv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 8));
  }
  case NEON::BI__builtin_neon_vaddv_u16:
    usgn = true;
    // FALLTHROUGH
  case NEON::BI__builtin_neon_vaddv_s16: {
    Int = usgn ? Intrinsic::aarch64_neon_uaddv : Intrinsic::aarch64_neon_saddv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 16), 4);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vaddv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 16));
  }
  case NEON::BI__builtin_neon_vaddvq_u8:
    usgn = true;
    // FALLTHROUGH
  case NEON::BI__builtin_neon_vaddvq_s8: {
    Int = usgn ? Intrinsic::aarch64_neon_uaddv : Intrinsic::aarch64_neon_saddv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 8), 16);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vaddv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 8));
  }
  case NEON::BI__builtin_neon_vaddvq_u16:
    usgn = true;
    // FALLTHROUGH
  case NEON::BI__builtin_neon_vaddvq_s16: {
    Int = usgn ? Intrinsic::aarch64_neon_uaddv : Intrinsic::aarch64_neon_saddv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 16), 8);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vaddv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 16));
  }
  case NEON::BI__builtin_neon_vmaxv_u8: {
    Int = Intrinsic::aarch64_neon_umaxv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 8), 8);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vmaxv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 8));
  }
  case NEON::BI__builtin_neon_vmaxv_u16: {
    Int = Intrinsic::aarch64_neon_umaxv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 16), 4);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vmaxv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 16));
  }
  case NEON::BI__builtin_neon_vmaxvq_u8: {
    Int = Intrinsic::aarch64_neon_umaxv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 8), 16);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vmaxv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 8));
  }
  case NEON::BI__builtin_neon_vmaxvq_u16: {
    Int = Intrinsic::aarch64_neon_umaxv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 16), 8);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vmaxv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 16));
  }
  case NEON::BI__builtin_neon_vmaxv_s8: {
    Int = Intrinsic::aarch64_neon_smaxv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 8), 8);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vmaxv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 8));
  }
  case NEON::BI__builtin_neon_vmaxv_s16: {
    Int = Intrinsic::aarch64_neon_smaxv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 16), 4);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vmaxv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 16));
  }
  case NEON::BI__builtin_neon_vmaxvq_s8: {
    Int = Intrinsic::aarch64_neon_smaxv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 8), 16);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vmaxv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 8));
  }
  case NEON::BI__builtin_neon_vmaxvq_s16: {
    Int = Intrinsic::aarch64_neon_smaxv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 16), 8);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vmaxv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 16));
  }
  case NEON::BI__builtin_neon_vminv_u8: {
    Int = Intrinsic::aarch64_neon_uminv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 8), 8);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vminv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 8));
  }
  case NEON::BI__builtin_neon_vminv_u16: {
    Int = Intrinsic::aarch64_neon_uminv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 16), 4);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vminv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 16));
  }
  case NEON::BI__builtin_neon_vminvq_u8: {
    Int = Intrinsic::aarch64_neon_uminv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 8), 16);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vminv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 8));
  }
  case NEON::BI__builtin_neon_vminvq_u16: {
    Int = Intrinsic::aarch64_neon_uminv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 16), 8);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vminv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 16));
  }
  case NEON::BI__builtin_neon_vminv_s8: {
    Int = Intrinsic::aarch64_neon_sminv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 8), 8);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vminv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 8));
  }
  case NEON::BI__builtin_neon_vminv_s16: {
    Int = Intrinsic::aarch64_neon_sminv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 16), 4);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vminv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 16));
  }
  case NEON::BI__builtin_neon_vminvq_s8: {
    Int = Intrinsic::aarch64_neon_sminv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 8), 16);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vminv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 8));
  }
  case NEON::BI__builtin_neon_vminvq_s16: {
    Int = Intrinsic::aarch64_neon_sminv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 16), 8);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vminv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 16));
  }
  case NEON::BI__builtin_neon_vmul_n_f64: {
    Ops[0] = Builder.CreateBitCast(Ops[0], DoubleTy);
    Value *RHS = Builder.CreateBitCast(EmitScalarExpr(E->getArg(1)), DoubleTy);
    return Builder.CreateFMul(Ops[0], RHS);
  }
  case NEON::BI__builtin_neon_vaddlv_u8: {
    Int = Intrinsic::aarch64_neon_uaddlv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 8), 8);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vaddlv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 16));
  }
  case NEON::BI__builtin_neon_vaddlv_u16: {
    Int = Intrinsic::aarch64_neon_uaddlv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 16), 4);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    return EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vaddlv");
  }
  case NEON::BI__builtin_neon_vaddlvq_u8: {
    Int = Intrinsic::aarch64_neon_uaddlv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 8), 16);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vaddlv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 16));
  }
  case NEON::BI__builtin_neon_vaddlvq_u16: {
    Int = Intrinsic::aarch64_neon_uaddlv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 16), 8);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    return EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vaddlv");
  }
  case NEON::BI__builtin_neon_vaddlv_s8: {
    Int = Intrinsic::aarch64_neon_saddlv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 8), 8);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vaddlv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 16));
  }
  case NEON::BI__builtin_neon_vaddlv_s16: {
    Int = Intrinsic::aarch64_neon_saddlv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 16), 4);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    return EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vaddlv");
  }
  case NEON::BI__builtin_neon_vaddlvq_s8: {
    Int = Intrinsic::aarch64_neon_saddlv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 8), 16);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    Ops[0] = EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vaddlv");
    return Builder.CreateTrunc(Ops[0],
             llvm::IntegerType::get(getLLVMContext(), 16));
  }
  case NEON::BI__builtin_neon_vaddlvq_s16: {
    Int = Intrinsic::aarch64_neon_saddlv;
    Ty = llvm::IntegerType::get(getLLVMContext(), 32);
    VTy =
      llvm::VectorType::get(llvm::IntegerType::get(getLLVMContext(), 16), 8);
    llvm::Type *Tys[2] = { Ty, VTy };
    Ops.push_back(EmitScalarExpr(E->getArg(0)));
    return EmitNeonCall(CGM.getIntrinsic(Int, Tys), Ops, "vaddlv");
  }
  case NEON::BI__builtin_neon_vsri_n_v:
  case NEON::BI__builtin_neon_vsriq_n_v: {
    Int = Intrinsic::aarch64_neon_vsri;
    llvm::Function *Intrin = CGM.getIntrinsic(Int, Ty);
    return EmitNeonCall(Intrin, Ops, "vsri_n");
  }
  case NEON::BI__builtin_neon_vsli_n_v:
  case NEON::BI__builtin_neon_vsliq_n_v: {
    Int = Intrinsic::aarch64_neon_vsli;
    llvm::Function *Intrin = CGM.getIntrinsic(Int, Ty);
    return EmitNeonCall(Intrin, Ops, "vsli_n");
  }
  case NEON::BI__builtin_neon_vsra_n_v:
  case NEON::BI__builtin_neon_vsraq_n_v:
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    Ops[1] = EmitNeonRShiftImm(Ops[1], Ops[2], Ty, usgn, "vsra_n");
    return Builder.CreateAdd(Ops[0], Ops[1]);
  case NEON::BI__builtin_neon_vrsra_n_v:
  case NEON::BI__builtin_neon_vrsraq_n_v: {
    Int = usgn ? Intrinsic::aarch64_neon_urshl : Intrinsic::aarch64_neon_srshl;
    SmallVector<llvm::Value*,2> TmpOps;
    TmpOps.push_back(Ops[1]);
    TmpOps.push_back(Ops[2]);
    Function* F = CGM.getIntrinsic(Int, Ty);
    llvm::Value *tmp = EmitNeonCall(F, TmpOps, "vrshr_n", 1, true);
    Ops[0] = Builder.CreateBitCast(Ops[0], VTy);
    return Builder.CreateAdd(Ops[0], tmp);
  }
    // FIXME: Sharing loads & stores with 32-bit is complicated by the absence
    // of an Align parameter here.
  case NEON::BI__builtin_neon_vld1_x2_v:
  case NEON::BI__builtin_neon_vld1q_x2_v:
  case NEON::BI__builtin_neon_vld1_x3_v:
  case NEON::BI__builtin_neon_vld1q_x3_v:
  case NEON::BI__builtin_neon_vld1_x4_v:
  case NEON::BI__builtin_neon_vld1q_x4_v: {
    llvm::Type *PTy = llvm::PointerType::getUnqual(VTy->getVectorElementType());
    Ops[1] = Builder.CreateBitCast(Ops[1], PTy);
    llvm::Type *Tys[2] = { VTy, PTy };
    unsigned Int;
    switch (BuiltinID) {
    case NEON::BI__builtin_neon_vld1_x2_v:
    case NEON::BI__builtin_neon_vld1q_x2_v:
      Int = Intrinsic::aarch64_neon_ld1x2;
      break;
    case NEON::BI__builtin_neon_vld1_x3_v:
    case NEON::BI__builtin_neon_vld1q_x3_v:
      Int = Intrinsic::aarch64_neon_ld1x3;
      break;
    case NEON::BI__builtin_neon_vld1_x4_v:
    case NEON::BI__builtin_neon_vld1q_x4_v:
      Int = Intrinsic::aarch64_neon_ld1x4;
      break;
    }
    Function *F = CGM.getIntrinsic(Int, Tys);
    Ops[1] = Builder.CreateCall(F, Ops[1], "vld1xN");
    Ty = llvm::PointerType::getUnqual(Ops[1]->getType());
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    return Builder.CreateStore(Ops[1], Ops[0]);
  }
  case NEON::BI__builtin_neon_vst1_x2_v:
  case NEON::BI__builtin_neon_vst1q_x2_v:
  case NEON::BI__builtin_neon_vst1_x3_v:
  case NEON::BI__builtin_neon_vst1q_x3_v:
  case NEON::BI__builtin_neon_vst1_x4_v:
  case NEON::BI__builtin_neon_vst1q_x4_v: {
    llvm::Type *PTy = llvm::PointerType::getUnqual(VTy->getVectorElementType());
    llvm::Type *Tys[2] = { VTy, PTy };
    unsigned Int;
    switch (BuiltinID) {
    case NEON::BI__builtin_neon_vst1_x2_v:
    case NEON::BI__builtin_neon_vst1q_x2_v:
      Int = Intrinsic::aarch64_neon_st1x2;
      break;
    case NEON::BI__builtin_neon_vst1_x3_v:
    case NEON::BI__builtin_neon_vst1q_x3_v:
      Int = Intrinsic::aarch64_neon_st1x3;
      break;
    case NEON::BI__builtin_neon_vst1_x4_v:
    case NEON::BI__builtin_neon_vst1q_x4_v:
      Int = Intrinsic::aarch64_neon_st1x4;
      break;
    }
    SmallVector<Value *, 4> IntOps(Ops.begin()+1, Ops.end());
    IntOps.push_back(Ops[0]);
    return EmitNeonCall(CGM.getIntrinsic(Int, Tys), IntOps, "");
  }
  case NEON::BI__builtin_neon_vld1_v:
  case NEON::BI__builtin_neon_vld1q_v:
    Ops[0] = Builder.CreateBitCast(Ops[0], llvm::PointerType::getUnqual(VTy));
    return Builder.CreateLoad(Ops[0]);
  case NEON::BI__builtin_neon_vst1_v:
  case NEON::BI__builtin_neon_vst1q_v:
    Ops[0] = Builder.CreateBitCast(Ops[0], llvm::PointerType::getUnqual(VTy));
    Ops[1] = Builder.CreateBitCast(Ops[1], VTy);
    return Builder.CreateStore(Ops[1], Ops[0]);
  case NEON::BI__builtin_neon_vld1_lane_v:
  case NEON::BI__builtin_neon_vld1q_lane_v:
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Ty = llvm::PointerType::getUnqual(VTy->getElementType());
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    Ops[0] = Builder.CreateLoad(Ops[0]);
    return Builder.CreateInsertElement(Ops[1], Ops[0], Ops[2], "vld1_lane");
  case NEON::BI__builtin_neon_vld1_dup_v:
  case NEON::BI__builtin_neon_vld1q_dup_v: {
    Value *V = UndefValue::get(Ty);
    Ty = llvm::PointerType::getUnqual(VTy->getElementType());
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    Ops[0] = Builder.CreateLoad(Ops[0]);
    llvm::Constant *CI = ConstantInt::get(Int32Ty, 0);
    Ops[0] = Builder.CreateInsertElement(V, Ops[0], CI);
    return EmitNeonSplat(Ops[0], CI);
  }
  case NEON::BI__builtin_neon_vst1_lane_v:
  case NEON::BI__builtin_neon_vst1q_lane_v:
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Ops[1] = Builder.CreateExtractElement(Ops[1], Ops[2]);
    Ty = llvm::PointerType::getUnqual(Ops[1]->getType());
    return Builder.CreateStore(Ops[1], Builder.CreateBitCast(Ops[0], Ty));
  case NEON::BI__builtin_neon_vld2_v:
  case NEON::BI__builtin_neon_vld2q_v: {
    llvm::Type *PTy = llvm::PointerType::getUnqual(VTy);
    Ops[1] = Builder.CreateBitCast(Ops[1], PTy);
    llvm::Type *Tys[2] = { VTy, PTy };
    Function *F = CGM.getIntrinsic(Intrinsic::aarch64_neon_ld2, Tys);
    Ops[1] = Builder.CreateCall(F, Ops[1], "vld2");
    Ops[0] = Builder.CreateBitCast(Ops[0],
                llvm::PointerType::getUnqual(Ops[1]->getType()));
    return Builder.CreateStore(Ops[1], Ops[0]);
  }
  case NEON::BI__builtin_neon_vld3_v:
  case NEON::BI__builtin_neon_vld3q_v: {
    llvm::Type *PTy = llvm::PointerType::getUnqual(VTy);
    Ops[1] = Builder.CreateBitCast(Ops[1], PTy);
    llvm::Type *Tys[2] = { VTy, PTy };
    Function *F = CGM.getIntrinsic(Intrinsic::aarch64_neon_ld3, Tys);
    Ops[1] = Builder.CreateCall(F, Ops[1], "vld3");
    Ops[0] = Builder.CreateBitCast(Ops[0],
                llvm::PointerType::getUnqual(Ops[1]->getType()));
    return Builder.CreateStore(Ops[1], Ops[0]);
  }
  case NEON::BI__builtin_neon_vld4_v:
  case NEON::BI__builtin_neon_vld4q_v: {
    llvm::Type *PTy = llvm::PointerType::getUnqual(VTy);
    Ops[1] = Builder.CreateBitCast(Ops[1], PTy);
    llvm::Type *Tys[2] = { VTy, PTy };
    Function *F = CGM.getIntrinsic(Intrinsic::aarch64_neon_ld4, Tys);
    Ops[1] = Builder.CreateCall(F, Ops[1], "vld4");
    Ops[0] = Builder.CreateBitCast(Ops[0],
                llvm::PointerType::getUnqual(Ops[1]->getType()));
    return Builder.CreateStore(Ops[1], Ops[0]);
  }
  case NEON::BI__builtin_neon_vld2_dup_v:
  case NEON::BI__builtin_neon_vld2q_dup_v: {
    llvm::Type *PTy =
      llvm::PointerType::getUnqual(VTy->getElementType());
    Ops[1] = Builder.CreateBitCast(Ops[1], PTy);
    llvm::Type *Tys[2] = { VTy, PTy };
    Function *F = CGM.getIntrinsic(Intrinsic::aarch64_neon_ld2r, Tys);
    Ops[1] = Builder.CreateCall(F, Ops[1], "vld2");
    Ops[0] = Builder.CreateBitCast(Ops[0],
                llvm::PointerType::getUnqual(Ops[1]->getType()));
    return Builder.CreateStore(Ops[1], Ops[0]);
  }
  case NEON::BI__builtin_neon_vld3_dup_v:
  case NEON::BI__builtin_neon_vld3q_dup_v: {
    llvm::Type *PTy =
      llvm::PointerType::getUnqual(VTy->getElementType());
    Ops[1] = Builder.CreateBitCast(Ops[1], PTy);
    llvm::Type *Tys[2] = { VTy, PTy };
    Function *F = CGM.getIntrinsic(Intrinsic::aarch64_neon_ld3r, Tys);
    Ops[1] = Builder.CreateCall(F, Ops[1], "vld3");
    Ops[0] = Builder.CreateBitCast(Ops[0],
                llvm::PointerType::getUnqual(Ops[1]->getType()));
    return Builder.CreateStore(Ops[1], Ops[0]);
  }
  case NEON::BI__builtin_neon_vld4_dup_v:
  case NEON::BI__builtin_neon_vld4q_dup_v: {
    llvm::Type *PTy =
      llvm::PointerType::getUnqual(VTy->getElementType());
    Ops[1] = Builder.CreateBitCast(Ops[1], PTy);
    llvm::Type *Tys[2] = { VTy, PTy };
    Function *F = CGM.getIntrinsic(Intrinsic::aarch64_neon_ld4r, Tys);
    Ops[1] = Builder.CreateCall(F, Ops[1], "vld4");
    Ops[0] = Builder.CreateBitCast(Ops[0],
                llvm::PointerType::getUnqual(Ops[1]->getType()));
    return Builder.CreateStore(Ops[1], Ops[0]);
  }
  case NEON::BI__builtin_neon_vld2_lane_v:
  case NEON::BI__builtin_neon_vld2q_lane_v: {
    llvm::Type *Tys[2] = { VTy, Ops[1]->getType() };
    Function *F = CGM.getIntrinsic(Intrinsic::aarch64_neon_ld2lane, Tys);
    Ops.push_back(Ops[1]);
    Ops.erase(Ops.begin()+1);
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Ops[2] = Builder.CreateBitCast(Ops[2], Ty);
    Ops[3] = Builder.CreateZExt(Ops[3],
                llvm::IntegerType::get(getLLVMContext(), 64));
    Ops[1] = Builder.CreateCall(F, makeArrayRef(Ops).slice(1), "vld2_lane");
    Ty = llvm::PointerType::getUnqual(Ops[1]->getType());
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    return Builder.CreateStore(Ops[1], Ops[0]);
  }
  case NEON::BI__builtin_neon_vld3_lane_v:
  case NEON::BI__builtin_neon_vld3q_lane_v: {
    llvm::Type *Tys[2] = { VTy, Ops[1]->getType() };
    Function *F = CGM.getIntrinsic(Intrinsic::aarch64_neon_ld3lane, Tys);
    Ops.push_back(Ops[1]);
    Ops.erase(Ops.begin()+1);
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Ops[2] = Builder.CreateBitCast(Ops[2], Ty);
    Ops[3] = Builder.CreateBitCast(Ops[3], Ty);
    Ops[4] = Builder.CreateZExt(Ops[4],
                llvm::IntegerType::get(getLLVMContext(), 64));
    Ops[1] = Builder.CreateCall(F, makeArrayRef(Ops).slice(1), "vld3_lane");
    Ty = llvm::PointerType::getUnqual(Ops[1]->getType());
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    return Builder.CreateStore(Ops[1], Ops[0]);
  }
  case NEON::BI__builtin_neon_vld4_lane_v:
  case NEON::BI__builtin_neon_vld4q_lane_v: {
    llvm::Type *Tys[2] = { VTy, Ops[1]->getType() };
    Function *F = CGM.getIntrinsic(Intrinsic::aarch64_neon_ld4lane, Tys);
    Ops.push_back(Ops[1]);
    Ops.erase(Ops.begin()+1);
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Ops[2] = Builder.CreateBitCast(Ops[2], Ty);
    Ops[3] = Builder.CreateBitCast(Ops[3], Ty);
    Ops[4] = Builder.CreateBitCast(Ops[4], Ty);
    Ops[5] = Builder.CreateZExt(Ops[5],
                llvm::IntegerType::get(getLLVMContext(), 64));
    Ops[1] = Builder.CreateCall(F, makeArrayRef(Ops).slice(1), "vld4_lane");
    Ty = llvm::PointerType::getUnqual(Ops[1]->getType());
    Ops[0] = Builder.CreateBitCast(Ops[0], Ty);
    return Builder.CreateStore(Ops[1], Ops[0]);
  }
  case NEON::BI__builtin_neon_vst2_v:
  case NEON::BI__builtin_neon_vst2q_v: {
    Ops.push_back(Ops[0]);
    Ops.erase(Ops.begin());
    llvm::Type *Tys[2] = { VTy, Ops[2]->getType() };
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_st2, Tys),
                        Ops, "");
  }
  case NEON::BI__builtin_neon_vst2_lane_v:
  case NEON::BI__builtin_neon_vst2q_lane_v: {
    Ops.push_back(Ops[0]);
    Ops.erase(Ops.begin());
    Ops[2] = Builder.CreateZExt(Ops[2],
                llvm::IntegerType::get(getLLVMContext(), 64));
    llvm::Type *Tys[2] = { VTy, Ops[3]->getType() };
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_st2lane, Tys),
                        Ops, "");
  }
  case NEON::BI__builtin_neon_vst3_v:
  case NEON::BI__builtin_neon_vst3q_v: {
    Ops.push_back(Ops[0]);
    Ops.erase(Ops.begin());
    llvm::Type *Tys[2] = { VTy, Ops[3]->getType() };
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_st3, Tys),
                        Ops, "");
  }
  case NEON::BI__builtin_neon_vst3_lane_v:
  case NEON::BI__builtin_neon_vst3q_lane_v: {
    Ops.push_back(Ops[0]);
    Ops.erase(Ops.begin());
    Ops[3] = Builder.CreateZExt(Ops[3],
                llvm::IntegerType::get(getLLVMContext(), 64));
    llvm::Type *Tys[2] = { VTy, Ops[4]->getType() };
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_st3lane, Tys),
                        Ops, "");
  }
  case NEON::BI__builtin_neon_vst4_v:
  case NEON::BI__builtin_neon_vst4q_v: {
    Ops.push_back(Ops[0]);
    Ops.erase(Ops.begin());
    llvm::Type *Tys[2] = { VTy, Ops[4]->getType() };
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_st4, Tys),
                        Ops, "");
  }
  case NEON::BI__builtin_neon_vst4_lane_v:
  case NEON::BI__builtin_neon_vst4q_lane_v: {
    Ops.push_back(Ops[0]);
    Ops.erase(Ops.begin());
    Ops[4] = Builder.CreateZExt(Ops[4],
                llvm::IntegerType::get(getLLVMContext(), 64));
    llvm::Type *Tys[2] = { VTy, Ops[5]->getType() };
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_st4lane, Tys),
                        Ops, "");
  }
  case NEON::BI__builtin_neon_vtrn_v:
  case NEON::BI__builtin_neon_vtrnq_v: {
    Ops[0] = Builder.CreateBitCast(Ops[0], llvm::PointerType::getUnqual(Ty));
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Ops[2] = Builder.CreateBitCast(Ops[2], Ty);
    Value *SV = nullptr;

    for (unsigned vi = 0; vi != 2; ++vi) {
      SmallVector<Constant*, 16> Indices;
      for (unsigned i = 0, e = VTy->getNumElements(); i != e; i += 2) {
        Indices.push_back(ConstantInt::get(Int32Ty, i+vi));
        Indices.push_back(ConstantInt::get(Int32Ty, i+e+vi));
      }
      Value *Addr = Builder.CreateConstInBoundsGEP1_32(Ty, Ops[0], vi);
      SV = llvm::ConstantVector::get(Indices);
      SV = Builder.CreateShuffleVector(Ops[1], Ops[2], SV, "vtrn");
      SV = Builder.CreateStore(SV, Addr);
    }
    return SV;
  }
  case NEON::BI__builtin_neon_vuzp_v:
  case NEON::BI__builtin_neon_vuzpq_v: {
    Ops[0] = Builder.CreateBitCast(Ops[0], llvm::PointerType::getUnqual(Ty));
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Ops[2] = Builder.CreateBitCast(Ops[2], Ty);
    Value *SV = nullptr;

    for (unsigned vi = 0; vi != 2; ++vi) {
      SmallVector<Constant*, 16> Indices;
      for (unsigned i = 0, e = VTy->getNumElements(); i != e; ++i)
        Indices.push_back(ConstantInt::get(Int32Ty, 2*i+vi));

      Value *Addr = Builder.CreateConstInBoundsGEP1_32(Ty, Ops[0], vi);
      SV = llvm::ConstantVector::get(Indices);
      SV = Builder.CreateShuffleVector(Ops[1], Ops[2], SV, "vuzp");
      SV = Builder.CreateStore(SV, Addr);
    }
    return SV;
  }
  case NEON::BI__builtin_neon_vzip_v:
  case NEON::BI__builtin_neon_vzipq_v: {
    Ops[0] = Builder.CreateBitCast(Ops[0], llvm::PointerType::getUnqual(Ty));
    Ops[1] = Builder.CreateBitCast(Ops[1], Ty);
    Ops[2] = Builder.CreateBitCast(Ops[2], Ty);
    Value *SV = nullptr;

    for (unsigned vi = 0; vi != 2; ++vi) {
      SmallVector<Constant*, 16> Indices;
      for (unsigned i = 0, e = VTy->getNumElements(); i != e; i += 2) {
        Indices.push_back(ConstantInt::get(Int32Ty, (i + vi*e) >> 1));
        Indices.push_back(ConstantInt::get(Int32Ty, ((i + vi*e) >> 1)+e));
      }
      Value *Addr = Builder.CreateConstInBoundsGEP1_32(Ty, Ops[0], vi);
      SV = llvm::ConstantVector::get(Indices);
      SV = Builder.CreateShuffleVector(Ops[1], Ops[2], SV, "vzip");
      SV = Builder.CreateStore(SV, Addr);
    }
    return SV;
  }
  case NEON::BI__builtin_neon_vqtbl1q_v: {
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_tbl1, Ty),
                        Ops, "vtbl1");
  }
  case NEON::BI__builtin_neon_vqtbl2q_v: {
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_tbl2, Ty),
                        Ops, "vtbl2");
  }
  case NEON::BI__builtin_neon_vqtbl3q_v: {
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_tbl3, Ty),
                        Ops, "vtbl3");
  }
  case NEON::BI__builtin_neon_vqtbl4q_v: {
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_tbl4, Ty),
                        Ops, "vtbl4");
  }
  case NEON::BI__builtin_neon_vqtbx1q_v: {
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_tbx1, Ty),
                        Ops, "vtbx1");
  }
  case NEON::BI__builtin_neon_vqtbx2q_v: {
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_tbx2, Ty),
                        Ops, "vtbx2");
  }
  case NEON::BI__builtin_neon_vqtbx3q_v: {
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_tbx3, Ty),
                        Ops, "vtbx3");
  }
  case NEON::BI__builtin_neon_vqtbx4q_v: {
    return EmitNeonCall(CGM.getIntrinsic(Intrinsic::aarch64_neon_tbx4, Ty),
                        Ops, "vtbx4");
  }
  case NEON::BI__builtin_neon_vsqadd_v:
  case NEON::BI__builtin_neon_vsqaddq_v: {
    Int = Intrinsic::aarch64_neon_usqadd;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vsqadd");
  }
  case NEON::BI__builtin_neon_vuqadd_v:
  case NEON::BI__builtin_neon_vuqaddq_v: {
    Int = Intrinsic::aarch64_neon_suqadd;
    return EmitNeonCall(CGM.getIntrinsic(Int, Ty), Ops, "vuqadd");
  }
  }
}

llvm::Value *CodeGenFunction::
BuildVector(ArrayRef<llvm::Value*> Ops) {
  assert((Ops.size() & (Ops.size() - 1)) == 0 &&
         "Not a power-of-two sized vector!");
  bool AllConstants = true;
  for (unsigned i = 0, e = Ops.size(); i != e && AllConstants; ++i)
    AllConstants &= isa<Constant>(Ops[i]);

  // If this is a constant vector, create a ConstantVector.
  if (AllConstants) {
    SmallVector<llvm::Constant*, 16> CstOps;
    for (unsigned i = 0, e = Ops.size(); i != e; ++i)
      CstOps.push_back(cast<Constant>(Ops[i]));
    return llvm::ConstantVector::get(CstOps);
  }

  // Otherwise, insertelement the values to build the vector.
  Value *Result =
    llvm::UndefValue::get(llvm::VectorType::get(Ops[0]->getType(), Ops.size()));

  for (unsigned i = 0, e = Ops.size(); i != e; ++i)
    Result = Builder.CreateInsertElement(Result, Ops[i], Builder.getInt32(i));

  return Result;
}

Value *CodeGenFunction::EmitX86BuiltinExpr(unsigned BuiltinID,
                                           const CallExpr *E) {
  SmallVector<Value*, 4> Ops;

  // Find out if any arguments are required to be integer constant expressions.
  unsigned ICEArguments = 0;
  ASTContext::GetBuiltinTypeError Error;
  getContext().GetBuiltinType(BuiltinID, Error, &ICEArguments);
  assert(Error == ASTContext::GE_None && "Should not codegen an error");

  for (unsigned i = 0, e = E->getNumArgs(); i != e; i++) {
    // If this is a normal argument, just emit it as a scalar.
    if ((ICEArguments & (1 << i)) == 0) {
      Ops.push_back(EmitScalarExpr(E->getArg(i)));
      continue;
    }

    // If this is required to be a constant, constant fold it so that we know
    // that the generated intrinsic gets a ConstantInt.
    llvm::APSInt Result;
    bool IsConst = E->getArg(i)->isIntegerConstantExpr(Result, getContext());
    assert(IsConst && "Constant arg isn't actually constant?"); (void)IsConst;
    Ops.push_back(llvm::ConstantInt::get(getLLVMContext(), Result));
  }

  switch (BuiltinID) {
  default: return nullptr;
  case X86::BI__builtin_cpu_supports: {
    const Expr *FeatureExpr = E->getArg(0)->IgnoreParenCasts();
    StringRef FeatureStr = cast<StringLiteral>(FeatureExpr)->getString();

    // TODO: When/if this becomes more than x86 specific then use a TargetInfo
    // based mapping.
    // Processor features and mapping to processor feature value.
    enum X86Features {
      CMOV = 0,
      MMX,
      POPCNT,
      SSE,
      SSE2,
      SSE3,
      SSSE3,
      SSE4_1,
      SSE4_2,
      AVX,
      AVX2,
      SSE4_A,
      FMA4,
      XOP,
      FMA,
      AVX512F,
      BMI,
      BMI2,
      MAX
    };

    X86Features Feature = StringSwitch<X86Features>(FeatureStr)
                              .Case("cmov", X86Features::CMOV)
                              .Case("mmx", X86Features::MMX)
                              .Case("popcnt", X86Features::POPCNT)
                              .Case("sse", X86Features::SSE)
                              .Case("sse2", X86Features::SSE2)
                              .Case("sse3", X86Features::SSE3)
                              .Case("sse4.1", X86Features::SSE4_1)
                              .Case("sse4.2", X86Features::SSE4_2)
                              .Case("avx", X86Features::AVX)
                              .Case("avx2", X86Features::AVX2)
                              .Case("sse4a", X86Features::SSE4_A)
                              .Case("fma4", X86Features::FMA4)
                              .Case("xop", X86Features::XOP)
                              .Case("fma", X86Features::FMA)
                              .Case("avx512f", X86Features::AVX512F)
                              .Case("bmi", X86Features::BMI)
                              .Case("bmi2", X86Features::BMI2)
                              .Default(X86Features::MAX);
    assert(Feature != X86Features::MAX && "Invalid feature!");

    // Matching the struct layout from the compiler-rt/libgcc structure that is
    // filled in:
    // unsigned int __cpu_vendor;
    // unsigned int __cpu_type;
    // unsigned int __cpu_subtype;
    // unsigned int __cpu_features[1];
    llvm::Type *STy = llvm::StructType::get(
        Int32Ty, Int32Ty, Int32Ty, llvm::ArrayType::get(Int32Ty, 1), nullptr);

    // Grab the global __cpu_model.
    llvm::Constant *CpuModel = CGM.CreateRuntimeVariable(STy, "__cpu_model");

    // Grab the first (0th) element from the field __cpu_features off of the
    // global in the struct STy.
    Value *Idxs[] = {
      ConstantInt::get(Int32Ty, 0),
      ConstantInt::get(Int32Ty, 3),
      ConstantInt::get(Int32Ty, 0)
    };
    Value *CpuFeatures = Builder.CreateGEP(STy, CpuModel, Idxs);
    Value *Features = Builder.CreateLoad(CpuFeatures);

    // Check the value of the bit corresponding to the feature requested.
    Value *Bitset = Builder.CreateAnd(
        Features, llvm::ConstantInt::get(Int32Ty, 1 << Feature));
    return Builder.CreateICmpNE(Bitset, llvm::ConstantInt::get(Int32Ty, 0));
  }
  case X86::BI_mm_prefetch: {
    Value *Address = EmitScalarExpr(E->getArg(0));
    Value *RW = ConstantInt::get(Int32Ty, 0);
    Value *Locality = EmitScalarExpr(E->getArg(1));
    Value *Data = ConstantInt::get(Int32Ty, 1);
    Value *F = CGM.getIntrinsic(Intrinsic::prefetch);
    return Builder.CreateCall(F, {Address, RW, Locality, Data});
  }
  case X86::BI__builtin_ia32_vec_init_v8qi:
  case X86::BI__builtin_ia32_vec_init_v4hi:
  case X86::BI__builtin_ia32_vec_init_v2si:
    return Builder.CreateBitCast(BuildVector(Ops),
                                 llvm::Type::getX86_MMXTy(getLLVMContext()));
  case X86::BI__builtin_ia32_vec_ext_v2si:
    return Builder.CreateExtractElement(Ops[0],
                                  llvm::ConstantInt::get(Ops[1]->getType(), 0));
  case X86::BI__builtin_ia32_ldmxcsr: {
    Value *Tmp = CreateMemTemp(E->getArg(0)->getType());
    Builder.CreateStore(Ops[0], Tmp);
    return Builder.CreateCall(CGM.getIntrinsic(Intrinsic::x86_sse_ldmxcsr),
                              Builder.CreateBitCast(Tmp, Int8PtrTy));
  }
  case X86::BI__builtin_ia32_stmxcsr: {
    Value *Tmp = CreateMemTemp(E->getType());
    Builder.CreateCall(CGM.getIntrinsic(Intrinsic::x86_sse_stmxcsr),
                       Builder.CreateBitCast(Tmp, Int8PtrTy));
    return Builder.CreateLoad(Tmp, "stmxcsr");
  }
  case X86::BI__builtin_ia32_storehps:
  case X86::BI__builtin_ia32_storelps: {
    llvm::Type *PtrTy = llvm::PointerType::getUnqual(Int64Ty);
    llvm::Type *VecTy = llvm::VectorType::get(Int64Ty, 2);

    // cast val v2i64
    Ops[1] = Builder.CreateBitCast(Ops[1], VecTy, "cast");

    // extract (0, 1)
    unsigned Index = BuiltinID == X86::BI__builtin_ia32_storelps ? 0 : 1;
    llvm::Value *Idx = llvm::ConstantInt::get(SizeTy, Index);
    Ops[1] = Builder.CreateExtractElement(Ops[1], Idx, "extract");

    // cast pointer to i64 & store
    Ops[0] = Builder.CreateBitCast(Ops[0], PtrTy);
    return Builder.CreateStore(Ops[1], Ops[0]);
  }
  case X86::BI__builtin_ia32_palignr128:
  case X86::BI__builtin_ia32_palignr256: {
    unsigned ShiftVal = cast<llvm::ConstantInt>(Ops[2])->getZExtValue();

    unsigned NumElts =
      cast<llvm::VectorType>(Ops[0]->getType())->getNumElements();
    assert(NumElts % 16 == 0);
    unsigned NumLanes = NumElts / 16;
    unsigned NumLaneElts = NumElts / NumLanes;

    // If palignr is shifting the pair of vectors more than the size of two
    // lanes, emit zero.
    if (ShiftVal >= (2 * NumLaneElts))
      return llvm::Constant::getNullValue(ConvertType(E->getType()));

    // If palignr is shifting the pair of input vectors more than one lane,
    // but less than two lanes, convert to shifting in zeroes.
    if (ShiftVal > NumLaneElts) {
      ShiftVal -= NumLaneElts;
      Ops[1] = Ops[0];
      Ops[0] = llvm::Constant::getNullValue(Ops[0]->getType());
    }

    SmallVector<llvm::Constant*, 32> Indices;
    // 256-bit palignr operates on 128-bit lanes so we need to handle that
    for (unsigned l = 0; l != NumElts; l += NumLaneElts) {
      for (unsigned i = 0; i != NumLaneElts; ++i) {
        unsigned Idx = ShiftVal + i;
        if (Idx >= NumLaneElts)
          Idx += NumElts - NumLaneElts; // End of lane, switch operand.
        Indices.push_back(llvm::ConstantInt::get(Int32Ty, Idx + l));
      }
    }

    Value* SV = llvm::ConstantVector::get(Indices);
    return Builder.CreateShuffleVector(Ops[1], Ops[0], SV, "palignr");
  }
  case X86::BI__builtin_ia32_pslldqi256: {
    // Shift value is in bits so divide by 8.
    unsigned shiftVal = cast<llvm::ConstantInt>(Ops[1])->getZExtValue() >> 3;

    // If pslldq is shifting the vector more than 15 bytes, emit zero.
    if (shiftVal >= 16)
      return llvm::Constant::getNullValue(ConvertType(E->getType()));

    SmallVector<llvm::Constant*, 32> Indices;
    // 256-bit pslldq operates on 128-bit lanes so we need to handle that
    for (unsigned l = 0; l != 32; l += 16) {
      for (unsigned i = 0; i != 16; ++i) {
        unsigned Idx = 32 + i - shiftVal;
        if (Idx < 32) Idx -= 16; // end of lane, switch operand.
        Indices.push_back(llvm::ConstantInt::get(Int32Ty, Idx + l));
      }
    }

    llvm::Type *VecTy = llvm::VectorType::get(Int8Ty, 32);
    Ops[0] = Builder.CreateBitCast(Ops[0], VecTy, "cast");
    Value *Zero = llvm::Constant::getNullValue(VecTy);

    Value *SV = llvm::ConstantVector::get(Indices);
    SV = Builder.CreateShuffleVector(Zero, Ops[0], SV, "pslldq");
    llvm::Type *ResultType = ConvertType(E->getType());
    return Builder.CreateBitCast(SV, ResultType, "cast");
  }
  case X86::BI__builtin_ia32_psrldqi256: {
    // Shift value is in bits so divide by 8.
    unsigned shiftVal = cast<llvm::ConstantInt>(Ops[1])->getZExtValue() >> 3;

    // If psrldq is shifting the vector more than 15 bytes, emit zero.
    if (shiftVal >= 16)
      return llvm::Constant::getNullValue(ConvertType(E->getType()));

    SmallVector<llvm::Constant*, 32> Indices;
    // 256-bit psrldq operates on 128-bit lanes so we need to handle that
    for (unsigned l = 0; l != 32; l += 16) {
      for (unsigned i = 0; i != 16; ++i) {
        unsigned Idx = i + shiftVal;
        if (Idx >= 16) Idx += 16; // end of lane, switch operand.
        Indices.push_back(llvm::ConstantInt::get(Int32Ty, Idx + l));
      }
    }

    llvm::Type *VecTy = llvm::VectorType::get(Int8Ty, 32);
    Ops[0] = Builder.CreateBitCast(Ops[0], VecTy, "cast");
    Value *Zero = llvm::Constant::getNullValue(VecTy);

    Value *SV = llvm::ConstantVector::get(Indices);
    SV = Builder.CreateShuffleVector(Ops[0], Zero, SV, "psrldq");
    llvm::Type *ResultType = ConvertType(E->getType());
    return Builder.CreateBitCast(SV, ResultType, "cast");
  }
  case X86::BI__builtin_ia32_movntps:
  case X86::BI__builtin_ia32_movntps256:
  case X86::BI__builtin_ia32_movntpd:
  case X86::BI__builtin_ia32_movntpd256:
  case X86::BI__builtin_ia32_movntdq:
  case X86::BI__builtin_ia32_movntdq256:
  case X86::BI__builtin_ia32_movnti:
  case X86::BI__builtin_ia32_movnti64: {
    llvm::MDNode *Node = llvm::MDNode::get(
        getLLVMContext(), llvm::ConstantAsMetadata::get(Builder.getInt32(1)));

    // Convert the type of the pointer to a pointer to the stored type.
    Value *BC = Builder.CreateBitCast(Ops[0],
                                llvm::PointerType::getUnqual(Ops[1]->getType()),
                                      "cast");
    StoreInst *SI = Builder.CreateStore(Ops[1], BC);
    SI->setMetadata(CGM.getModule().getMDKindID("nontemporal"), Node);

    // If the operand is an integer, we can't assume alignment. Otherwise,
    // assume natural alignment.
    QualType ArgTy = E->getArg(1)->getType();
    unsigned Align;
    if (ArgTy->isIntegerType())
      Align = 1;
    else
      Align = getContext().getTypeSizeInChars(ArgTy).getQuantity();
    SI->setAlignment(Align);
    return SI;
  }
  // 3DNow!
  case X86::BI__builtin_ia32_pswapdsf:
  case X86::BI__builtin_ia32_pswapdsi: {
    llvm::Type *MMXTy = llvm::Type::getX86_MMXTy(getLLVMContext());
    Ops[0] = Builder.CreateBitCast(Ops[0], MMXTy, "cast");
    llvm::Function *F = CGM.getIntrinsic(Intrinsic::x86_3dnowa_pswapd);
    return Builder.CreateCall(F, Ops, "pswapd");
  }
  case X86::BI__builtin_ia32_rdrand16_step:
  case X86::BI__builtin_ia32_rdrand32_step:
  case X86::BI__builtin_ia32_rdrand64_step:
  case X86::BI__builtin_ia32_rdseed16_step:
  case X86::BI__builtin_ia32_rdseed32_step:
  case X86::BI__builtin_ia32_rdseed64_step: {
    Intrinsic::ID ID;
    switch (BuiltinID) {
    default: llvm_unreachable("Unsupported intrinsic!");
    case X86::BI__builtin_ia32_rdrand16_step:
      ID = Intrinsic::x86_rdrand_16;
      break;
    case X86::BI__builtin_ia32_rdrand32_step:
      ID = Intrinsic::x86_rdrand_32;
      break;
    case X86::BI__builtin_ia32_rdrand64_step:
      ID = Intrinsic::x86_rdrand_64;
      break;
    case X86::BI__builtin_ia32_rdseed16_step:
      ID = Intrinsic::x86_rdseed_16;
      break;
    case X86::BI__builtin_ia32_rdseed32_step:
      ID = Intrinsic::x86_rdseed_32;
      break;
    case X86::BI__builtin_ia32_rdseed64_step:
      ID = Intrinsic::x86_rdseed_64;
      break;
    }

    Value *Call = Builder.CreateCall(CGM.getIntrinsic(ID));
    Builder.CreateStore(Builder.CreateExtractValue(Call, 0), Ops[0]);
    return Builder.CreateExtractValue(Call, 1);
  }
  // SSE comparison intrisics
  case X86::BI__builtin_ia32_cmpeqps:
  case X86::BI__builtin_ia32_cmpltps:
  case X86::BI__builtin_ia32_cmpleps:
  case X86::BI__builtin_ia32_cmpunordps:
  case X86::BI__builtin_ia32_cmpneqps:
  case X86::BI__builtin_ia32_cmpnltps:
  case X86::BI__builtin_ia32_cmpnleps:
  case X86::BI__builtin_ia32_cmpordps:
  case X86::BI__builtin_ia32_cmpeqss:
  case X86::BI__builtin_ia32_cmpltss:
  case X86::BI__builtin_ia32_cmpless:
  case X86::BI__builtin_ia32_cmpunordss:
  case X86::BI__builtin_ia32_cmpneqss:
  case X86::BI__builtin_ia32_cmpnltss:
  case X86::BI__builtin_ia32_cmpnless:
  case X86::BI__builtin_ia32_cmpordss:
  case X86::BI__builtin_ia32_cmpeqpd:
  case X86::BI__builtin_ia32_cmpltpd:
  case X86::BI__builtin_ia32_cmplepd:
  case X86::BI__builtin_ia32_cmpunordpd:
  case X86::BI__builtin_ia32_cmpneqpd:
  case X86::BI__builtin_ia32_cmpnltpd:
  case X86::BI__builtin_ia32_cmpnlepd:
  case X86::BI__builtin_ia32_cmpordpd:
  case X86::BI__builtin_ia32_cmpeqsd:
  case X86::BI__builtin_ia32_cmpltsd:
  case X86::BI__builtin_ia32_cmplesd:
  case X86::BI__builtin_ia32_cmpunordsd:
  case X86::BI__builtin_ia32_cmpneqsd:
  case X86::BI__builtin_ia32_cmpnltsd:
  case X86::BI__builtin_ia32_cmpnlesd:
  case X86::BI__builtin_ia32_cmpordsd:
    // These exist so that the builtin that takes an immediate can be bounds
    // checked by clang to avoid passing bad immediates to the backend. Since
    // AVX has a larger immediate than SSE we would need separate builtins to
    // do the different bounds checking. Rather than create a clang specific
    // SSE only builtin, this implements eight separate builtins to match gcc
    // implementation.

    // Choose the immediate.
    unsigned Imm;
    switch (BuiltinID) {
    default: llvm_unreachable("Unsupported intrinsic!");
    case X86::BI__builtin_ia32_cmpeqps:
    case X86::BI__builtin_ia32_cmpeqss:
    case X86::BI__builtin_ia32_cmpeqpd:
    case X86::BI__builtin_ia32_cmpeqsd:
      Imm = 0;
      break;
    case X86::BI__builtin_ia32_cmpltps:
    case X86::BI__builtin_ia32_cmpltss:
    case X86::BI__builtin_ia32_cmpltpd:
    case X86::BI__builtin_ia32_cmpltsd:
      Imm = 1;
      break;
    case X86::BI__builtin_ia32_cmpleps:
    case X86::BI__builtin_ia32_cmpless:
    case X86::BI__builtin_ia32_cmplepd:
    case X86::BI__builtin_ia32_cmplesd:
      Imm = 2;
      break;
    case X86::BI__builtin_ia32_cmpunordps:
    case X86::BI__builtin_ia32_cmpunordss:
    case X86::BI__builtin_ia32_cmpunordpd:
    case X86::BI__builtin_ia32_cmpunordsd:
      Imm = 3;
      break;
    case X86::BI__builtin_ia32_cmpneqps:
    case X86::BI__builtin_ia32_cmpneqss:
    case X86::BI__builtin_ia32_cmpneqpd:
    case X86::BI__builtin_ia32_cmpneqsd:
      Imm = 4;
      break;
    case X86::BI__builtin_ia32_cmpnltps:
    case X86::BI__builtin_ia32_cmpnltss:
    case X86::BI__builtin_ia32_cmpnltpd:
    case X86::BI__builtin_ia32_cmpnltsd:
      Imm = 5;
      break;
    case X86::BI__builtin_ia32_cmpnleps:
    case X86::BI__builtin_ia32_cmpnless:
    case X86::BI__builtin_ia32_cmpnlepd:
    case X86::BI__builtin_ia32_cmpnlesd:
      Imm = 6;
      break;
    case X86::BI__builtin_ia32_cmpordps:
    case X86::BI__builtin_ia32_cmpordss:
    case X86::BI__builtin_ia32_cmpordpd:
    case X86::BI__builtin_ia32_cmpordsd:
      Imm = 7;
      break;
    }

    // Choose the intrinsic ID.
    const char *name;
    Intrinsic::ID ID;
    switch (BuiltinID) {
    default: llvm_unreachable("Unsupported intrinsic!");
    case X86::BI__builtin_ia32_cmpeqps:
    case X86::BI__builtin_ia32_cmpltps:
    case X86::BI__builtin_ia32_cmpleps:
    case X86::BI__builtin_ia32_cmpunordps:
    case X86::BI__builtin_ia32_cmpneqps:
    case X86::BI__builtin_ia32_cmpnltps:
    case X86::BI__builtin_ia32_cmpnleps:
    case X86::BI__builtin_ia32_cmpordps:
      name = "cmpps";
      ID = Intrinsic::x86_sse_cmp_ps;
      break;
    case X86::BI__builtin_ia32_cmpeqss:
    case X86::BI__builtin_ia32_cmpltss:
    case X86::BI__builtin_ia32_cmpless:
    case X86::BI__builtin_ia32_cmpunordss:
    case X86::BI__builtin_ia32_cmpneqss:
    case X86::BI__builtin_ia32_cmpnltss:
    case X86::BI__builtin_ia32_cmpnless:
    case X86::BI__builtin_ia32_cmpordss:
      name = "cmpss";
      ID = Intrinsic::x86_sse_cmp_ss;
      break;
    case X86::BI__builtin_ia32_cmpeqpd:
    case X86::BI__builtin_ia32_cmpltpd:
    case X86::BI__builtin_ia32_cmplepd:
    case X86::BI__builtin_ia32_cmpunordpd:
    case X86::BI__builtin_ia32_cmpneqpd:
    case X86::BI__builtin_ia32_cmpnltpd:
    case X86::BI__builtin_ia32_cmpnlepd:
    case X86::BI__builtin_ia32_cmpordpd:
      name = "cmppd";
      ID = Intrinsic::x86_sse2_cmp_pd;
      break;
    case X86::BI__builtin_ia32_cmpeqsd:
    case X86::BI__builtin_ia32_cmpltsd:
    case X86::BI__builtin_ia32_cmplesd:
    case X86::BI__builtin_ia32_cmpunordsd:
    case X86::BI__builtin_ia32_cmpneqsd:
    case X86::BI__builtin_ia32_cmpnltsd:
    case X86::BI__builtin_ia32_cmpnlesd:
    case X86::BI__builtin_ia32_cmpordsd:
      name = "cmpsd";
      ID = Intrinsic::x86_sse2_cmp_sd;
      break;
    }

    Ops.push_back(llvm::ConstantInt::get(Int8Ty, Imm));
    llvm::Function *F = CGM.getIntrinsic(ID);
    return Builder.CreateCall(F, Ops, name);
  }
}


Value *CodeGenFunction::EmitPPCBuiltinExpr(unsigned BuiltinID,
                                           const CallExpr *E) {
  SmallVector<Value*, 4> Ops;

  for (unsigned i = 0, e = E->getNumArgs(); i != e; i++)
    Ops.push_back(EmitScalarExpr(E->getArg(i)));

  Intrinsic::ID ID = Intrinsic::not_intrinsic;

  switch (BuiltinID) {
  default: return nullptr;

  // vec_ld, vec_lvsl, vec_lvsr
  case PPC::BI__builtin_altivec_lvx:
  case PPC::BI__builtin_altivec_lvxl:
  case PPC::BI__builtin_altivec_lvebx:
  case PPC::BI__builtin_altivec_lvehx:
  case PPC::BI__builtin_altivec_lvewx:
  case PPC::BI__builtin_altivec_lvsl:
  case PPC::BI__builtin_altivec_lvsr:
  case PPC::BI__builtin_vsx_lxvd2x:
  case PPC::BI__builtin_vsx_lxvw4x:
  {
    Ops[1] = Builder.CreateBitCast(Ops[1], Int8PtrTy);

    Ops[0] = Builder.CreateGEP(Ops[1], Ops[0]);
    Ops.pop_back();

    switch (BuiltinID) {
    default: llvm_unreachable("Unsupported ld/lvsl/lvsr intrinsic!");
    case PPC::BI__builtin_altivec_lvx:
      ID = Intrinsic::ppc_altivec_lvx;
      break;
    case PPC::BI__builtin_altivec_lvxl:
      ID = Intrinsic::ppc_altivec_lvxl;
      break;
    case PPC::BI__builtin_altivec_lvebx:
      ID = Intrinsic::ppc_altivec_lvebx;
      break;
    case PPC::BI__builtin_altivec_lvehx:
      ID = Intrinsic::ppc_altivec_lvehx;
      break;
    case PPC::BI__builtin_altivec_lvewx:
      ID = Intrinsic::ppc_altivec_lvewx;
      break;
    case PPC::BI__builtin_altivec_lvsl:
      ID = Intrinsic::ppc_altivec_lvsl;
      break;
    case PPC::BI__builtin_altivec_lvsr:
      ID = Intrinsic::ppc_altivec_lvsr;
      break;
    case PPC::BI__builtin_vsx_lxvd2x:
      ID = Intrinsic::ppc_vsx_lxvd2x;
      break;
    case PPC::BI__builtin_vsx_lxvw4x:
      ID = Intrinsic::ppc_vsx_lxvw4x;
      break;
    }
    llvm::Function *F = CGM.getIntrinsic(ID);
    return Builder.CreateCall(F, Ops, "");
  }

  // vec_st
  case PPC::BI__builtin_altivec_stvx:
  case PPC::BI__builtin_altivec_stvxl:
  case PPC::BI__builtin_altivec_stvebx:
  case PPC::BI__builtin_altivec_stvehx:
  case PPC::BI__builtin_altivec_stvewx:
  case PPC::BI__builtin_vsx_stxvd2x:
  case PPC::BI__builtin_vsx_stxvw4x:
  {
    Ops[2] = Builder.CreateBitCast(Ops[2], Int8PtrTy);
    Ops[1] = Builder.CreateGEP(Ops[2], Ops[1]);
    Ops.pop_back();

    switch (BuiltinID) {
    default: llvm_unreachable("Unsupported st intrinsic!");
    case PPC::BI__builtin_altivec_stvx:
      ID = Intrinsic::ppc_altivec_stvx;
      break;
    case PPC::BI__builtin_altivec_stvxl:
      ID = Intrinsic::ppc_altivec_stvxl;
      break;
    case PPC::BI__builtin_altivec_stvebx:
      ID = Intrinsic::ppc_altivec_stvebx;
      break;
    case PPC::BI__builtin_altivec_stvehx:
      ID = Intrinsic::ppc_altivec_stvehx;
      break;
    case PPC::BI__builtin_altivec_stvewx:
      ID = Intrinsic::ppc_altivec_stvewx;
      break;
    case PPC::BI__builtin_vsx_stxvd2x:
      ID = Intrinsic::ppc_vsx_stxvd2x;
      break;
    case PPC::BI__builtin_vsx_stxvw4x:
      ID = Intrinsic::ppc_vsx_stxvw4x;
      break;
    }
    llvm::Function *F = CGM.getIntrinsic(ID);
    return Builder.CreateCall(F, Ops, "");
  }
  // Square root
  case PPC::BI__builtin_vsx_xvsqrtsp:
  case PPC::BI__builtin_vsx_xvsqrtdp: {
    llvm::Type *ResultType = ConvertType(E->getType());
    Value *X = EmitScalarExpr(E->getArg(0));
    ID = Intrinsic::sqrt;
    llvm::Function *F = CGM.getIntrinsic(ID, ResultType);
    return Builder.CreateCall(F, X);
  }
  // Count leading zeros
  case PPC::BI__builtin_altivec_vclzb:
  case PPC::BI__builtin_altivec_vclzh:
  case PPC::BI__builtin_altivec_vclzw:
  case PPC::BI__builtin_altivec_vclzd: {
    llvm::Type *ResultType = ConvertType(E->getType());
    Value *X = EmitScalarExpr(E->getArg(0));
    Value *Undef = ConstantInt::get(Builder.getInt1Ty(), false);
    Function *F = CGM.getIntrinsic(Intrinsic::ctlz, ResultType);
    return Builder.CreateCall(F, {X, Undef});
  }
  // Copy sign
  case PPC::BI__builtin_vsx_xvcpsgnsp:
  case PPC::BI__builtin_vsx_xvcpsgndp: {
    llvm::Type *ResultType = ConvertType(E->getType());
    Value *X = EmitScalarExpr(E->getArg(0));
    Value *Y = EmitScalarExpr(E->getArg(1));
    ID = Intrinsic::copysign;
    llvm::Function *F = CGM.getIntrinsic(ID, ResultType);
    return Builder.CreateCall(F, {X, Y});
  }
  // Rounding/truncation
  case PPC::BI__builtin_vsx_xvrspip:
  case PPC::BI__builtin_vsx_xvrdpip:
  case PPC::BI__builtin_vsx_xvrdpim:
  case PPC::BI__builtin_vsx_xvrspim:
  case PPC::BI__builtin_vsx_xvrdpi:
  case PPC::BI__builtin_vsx_xvrspi:
  case PPC::BI__builtin_vsx_xvrdpic:
  case PPC::BI__builtin_vsx_xvrspic:
  case PPC::BI__builtin_vsx_xvrdpiz:
  case PPC::BI__builtin_vsx_xvrspiz: {
    llvm::Type *ResultType = ConvertType(E->getType());
    Value *X = EmitScalarExpr(E->getArg(0));
    if (BuiltinID == PPC::BI__builtin_vsx_xvrdpim ||
        BuiltinID == PPC::BI__builtin_vsx_xvrspim)
      ID = Intrinsic::floor;
    else if (BuiltinID == PPC::BI__builtin_vsx_xvrdpi ||
             BuiltinID == PPC::BI__builtin_vsx_xvrspi)
      ID = Intrinsic::round;
    else if (BuiltinID == PPC::BI__builtin_vsx_xvrdpic ||
             BuiltinID == PPC::BI__builtin_vsx_xvrspic)
      ID = Intrinsic::nearbyint;
    else if (BuiltinID == PPC::BI__builtin_vsx_xvrdpip ||
             BuiltinID == PPC::BI__builtin_vsx_xvrspip)
      ID = Intrinsic::ceil;
    else if (BuiltinID == PPC::BI__builtin_vsx_xvrdpiz ||
             BuiltinID == PPC::BI__builtin_vsx_xvrspiz)
      ID = Intrinsic::trunc;
    llvm::Function *F = CGM.getIntrinsic(ID, ResultType);
    return Builder.CreateCall(F, X);
  }
  // FMA variations
  case PPC::BI__builtin_vsx_xvmaddadp:
  case PPC::BI__builtin_vsx_xvmaddasp:
  case PPC::BI__builtin_vsx_xvnmaddadp:
  case PPC::BI__builtin_vsx_xvnmaddasp:
  case PPC::BI__builtin_vsx_xvmsubadp:
  case PPC::BI__builtin_vsx_xvmsubasp:
  case PPC::BI__builtin_vsx_xvnmsubadp:
  case PPC::BI__builtin_vsx_xvnmsubasp: {
    llvm::Type *ResultType = ConvertType(E->getType());
    Value *X = EmitScalarExpr(E->getArg(0));
    Value *Y = EmitScalarExpr(E->getArg(1));
    Value *Z = EmitScalarExpr(E->getArg(2));
    Value *Zero = llvm::ConstantFP::getZeroValueForNegation(ResultType);
    llvm::Function *F = CGM.getIntrinsic(Intrinsic::fma, ResultType);
    switch (BuiltinID) {
      case PPC::BI__builtin_vsx_xvmaddadp:
      case PPC::BI__builtin_vsx_xvmaddasp:
        return Builder.CreateCall(F, {X, Y, Z});
      case PPC::BI__builtin_vsx_xvnmaddadp:
      case PPC::BI__builtin_vsx_xvnmaddasp:
        return Builder.CreateFSub(Zero,
                                  Builder.CreateCall(F, {X, Y, Z}), "sub");
      case PPC::BI__builtin_vsx_xvmsubadp:
      case PPC::BI__builtin_vsx_xvmsubasp:
        return Builder.CreateCall(F,
                                  {X, Y, Builder.CreateFSub(Zero, Z, "sub")});
      case PPC::BI__builtin_vsx_xvnmsubadp:
      case PPC::BI__builtin_vsx_xvnmsubasp:
        Value *FsubRes =
          Builder.CreateCall(F, {X, Y, Builder.CreateFSub(Zero, Z, "sub")});
        return Builder.CreateFSub(Zero, FsubRes, "sub");
    }
    llvm_unreachable("Unknown FMA operation");
    return nullptr; // Suppress no-return warning
  }
  }
}

// Emit an intrinsic that has 1 float or double.
static Value *emitUnaryFPBuiltin(CodeGenFunction &CGF,
                                 const CallExpr *E,
                                 unsigned IntrinsicID) {
  llvm::Value *Src0 = CGF.EmitScalarExpr(E->getArg(0));

  Value *F = CGF.CGM.getIntrinsic(IntrinsicID, Src0->getType());
  return CGF.Builder.CreateCall(F, Src0);
}

// Emit an intrinsic that has 3 float or double operands.
static Value *emitTernaryFPBuiltin(CodeGenFunction &CGF,
                                   const CallExpr *E,
                                   unsigned IntrinsicID) {
  llvm::Value *Src0 = CGF.EmitScalarExpr(E->getArg(0));
  llvm::Value *Src1 = CGF.EmitScalarExpr(E->getArg(1));
  llvm::Value *Src2 = CGF.EmitScalarExpr(E->getArg(2));

  Value *F = CGF.CGM.getIntrinsic(IntrinsicID, Src0->getType());
  return CGF.Builder.CreateCall(F, {Src0, Src1, Src2});
}

// Emit an intrinsic that has 1 float or double operand, and 1 integer.
static Value *emitFPIntBuiltin(CodeGenFunction &CGF,
                               const CallExpr *E,
                               unsigned IntrinsicID) {
  llvm::Value *Src0 = CGF.EmitScalarExpr(E->getArg(0));
  llvm::Value *Src1 = CGF.EmitScalarExpr(E->getArg(1));

  Value *F = CGF.CGM.getIntrinsic(IntrinsicID, Src0->getType());
  return CGF.Builder.CreateCall(F, {Src0, Src1});
}

Value *CodeGenFunction::EmitAMDGPUBuiltinExpr(unsigned BuiltinID,
                                              const CallExpr *E) {
  switch (BuiltinID) {
  case AMDGPU::BI__builtin_amdgpu_div_scale:
  case AMDGPU::BI__builtin_amdgpu_div_scalef: {
    // Translate from the intrinsics's struct return to the builtin's out
    // argument.

    std::pair<llvm::Value *, unsigned> FlagOutPtr
      = EmitPointerWithAlignment(E->getArg(3));

    llvm::Value *X = EmitScalarExpr(E->getArg(0));
    llvm::Value *Y = EmitScalarExpr(E->getArg(1));
    llvm::Value *Z = EmitScalarExpr(E->getArg(2));

    llvm::Value *Callee = CGM.getIntrinsic(Intrinsic::AMDGPU_div_scale,
                                           X->getType());

    llvm::Value *Tmp = Builder.CreateCall(Callee, {X, Y, Z});

    llvm::Value *Result = Builder.CreateExtractValue(Tmp, 0);
    llvm::Value *Flag = Builder.CreateExtractValue(Tmp, 1);

    llvm::Type *RealFlagType
      = FlagOutPtr.first->getType()->getPointerElementType();

    llvm::Value *FlagExt = Builder.CreateZExt(Flag, RealFlagType);
    llvm::StoreInst *FlagStore = Builder.CreateStore(FlagExt, FlagOutPtr.first);
    FlagStore->setAlignment(FlagOutPtr.second);
    return Result;
  }
  case AMDGPU::BI__builtin_amdgpu_div_fmas:
  case AMDGPU::BI__builtin_amdgpu_div_fmasf: {
    llvm::Value *Src0 = EmitScalarExpr(E->getArg(0));
    llvm::Value *Src1 = EmitScalarExpr(E->getArg(1));
    llvm::Value *Src2 = EmitScalarExpr(E->getArg(2));
    llvm::Value *Src3 = EmitScalarExpr(E->getArg(3));

    llvm::Value *F = CGM.getIntrinsic(Intrinsic::AMDGPU_div_fmas,
                                      Src0->getType());
    llvm::Value *Src3ToBool = Builder.CreateIsNotNull(Src3);
    return Builder.CreateCall(F, {Src0, Src1, Src2, Src3ToBool});
  }
  case AMDGPU::BI__builtin_amdgpu_div_fixup:
  case AMDGPU::BI__builtin_amdgpu_div_fixupf:
    return emitTernaryFPBuiltin(*this, E, Intrinsic::AMDGPU_div_fixup);
  case AMDGPU::BI__builtin_amdgpu_trig_preop:
  case AMDGPU::BI__builtin_amdgpu_trig_preopf:
    return emitFPIntBuiltin(*this, E, Intrinsic::AMDGPU_trig_preop);
  case AMDGPU::BI__builtin_amdgpu_rcp:
  case AMDGPU::BI__builtin_amdgpu_rcpf:
    return emitUnaryFPBuiltin(*this, E, Intrinsic::AMDGPU_rcp);
  case AMDGPU::BI__builtin_amdgpu_rsq:
  case AMDGPU::BI__builtin_amdgpu_rsqf:
    return emitUnaryFPBuiltin(*this, E, Intrinsic::AMDGPU_rsq);
  case AMDGPU::BI__builtin_amdgpu_rsq_clamped:
  case AMDGPU::BI__builtin_amdgpu_rsq_clampedf:
    return emitUnaryFPBuiltin(*this, E, Intrinsic::AMDGPU_rsq_clamped);
  case AMDGPU::BI__builtin_amdgpu_ldexp:
  case AMDGPU::BI__builtin_amdgpu_ldexpf:
    return emitFPIntBuiltin(*this, E, Intrinsic::AMDGPU_ldexp);
  case AMDGPU::BI__builtin_amdgpu_class:
  case AMDGPU::BI__builtin_amdgpu_classf:
    return emitFPIntBuiltin(*this, E, Intrinsic::AMDGPU_class);
   default:
    return nullptr;
  }
}

/// Handle a SystemZ function in which the final argument is a pointer
/// to an int that receives the post-instruction CC value.  At the LLVM level
/// this is represented as a function that returns a {result, cc} pair.
static Value *EmitSystemZIntrinsicWithCC(CodeGenFunction &CGF,
                                         unsigned IntrinsicID,
                                         const CallExpr *E) {
  unsigned NumArgs = E->getNumArgs() - 1;
  SmallVector<Value *, 8> Args(NumArgs);
  for (unsigned I = 0; I < NumArgs; ++I)
    Args[I] = CGF.EmitScalarExpr(E->getArg(I));
  Value *CCPtr = CGF.EmitScalarExpr(E->getArg(NumArgs));
  Value *F = CGF.CGM.getIntrinsic(IntrinsicID);
  Value *Call = CGF.Builder.CreateCall(F, Args);
  Value *CC = CGF.Builder.CreateExtractValue(Call, 1);
  CGF.Builder.CreateStore(CC, CCPtr);
  return CGF.Builder.CreateExtractValue(Call, 0);
}

Value *CodeGenFunction::EmitSystemZBuiltinExpr(unsigned BuiltinID,
                                               const CallExpr *E) {
  switch (BuiltinID) {
  case SystemZ::BI__builtin_tbegin: {
    Value *TDB = EmitScalarExpr(E->getArg(0));
    Value *Control = llvm::ConstantInt::get(Int32Ty, 0xff0c);
    Value *F = CGM.getIntrinsic(Intrinsic::s390_tbegin);
    return Builder.CreateCall(F, {TDB, Control});
  }
  case SystemZ::BI__builtin_tbegin_nofloat: {
    Value *TDB = EmitScalarExpr(E->getArg(0));
    Value *Control = llvm::ConstantInt::get(Int32Ty, 0xff0c);
    Value *F = CGM.getIntrinsic(Intrinsic::s390_tbegin_nofloat);
    return Builder.CreateCall(F, {TDB, Control});
  }
  case SystemZ::BI__builtin_tbeginc: {
    Value *TDB = llvm::ConstantPointerNull::get(Int8PtrTy);
    Value *Control = llvm::ConstantInt::get(Int32Ty, 0xff08);
    Value *F = CGM.getIntrinsic(Intrinsic::s390_tbeginc);
    return Builder.CreateCall(F, {TDB, Control});
  }
  case SystemZ::BI__builtin_tabort: {
    Value *Data = EmitScalarExpr(E->getArg(0));
    Value *F = CGM.getIntrinsic(Intrinsic::s390_tabort);
    return Builder.CreateCall(F, Builder.CreateSExt(Data, Int64Ty, "tabort"));
  }
  case SystemZ::BI__builtin_non_tx_store: {
    Value *Address = EmitScalarExpr(E->getArg(0));
    Value *Data = EmitScalarExpr(E->getArg(1));
    Value *F = CGM.getIntrinsic(Intrinsic::s390_ntstg);
    return Builder.CreateCall(F, {Data, Address});
  }

  // Vector builtins.  Note that most vector builtins are mapped automatically
  // to target-specific LLVM intrinsics.  The ones handled specially here can
  // be represented via standard LLVM IR, which is preferable to enable common
  // LLVM optimizations.

  case SystemZ::BI__builtin_s390_vpopctb:
  case SystemZ::BI__builtin_s390_vpopcth:
  case SystemZ::BI__builtin_s390_vpopctf:
  case SystemZ::BI__builtin_s390_vpopctg: {
    llvm::Type *ResultType = ConvertType(E->getType());
    Value *X = EmitScalarExpr(E->getArg(0));
    Function *F = CGM.getIntrinsic(Intrinsic::ctpop, ResultType);
    return Builder.CreateCall(F, X);
  }

  case SystemZ::BI__builtin_s390_vclzb:
  case SystemZ::BI__builtin_s390_vclzh:
  case SystemZ::BI__builtin_s390_vclzf:
  case SystemZ::BI__builtin_s390_vclzg: {
    llvm::Type *ResultType = ConvertType(E->getType());
    Value *X = EmitScalarExpr(E->getArg(0));
    Value *Undef = ConstantInt::get(Builder.getInt1Ty(), false);
    Function *F = CGM.getIntrinsic(Intrinsic::ctlz, ResultType);
    return Builder.CreateCall(F, {X, Undef});
  }

  case SystemZ::BI__builtin_s390_vctzb:
  case SystemZ::BI__builtin_s390_vctzh:
  case SystemZ::BI__builtin_s390_vctzf:
  case SystemZ::BI__builtin_s390_vctzg: {
    llvm::Type *ResultType = ConvertType(E->getType());
    Value *X = EmitScalarExpr(E->getArg(0));
    Value *Undef = ConstantInt::get(Builder.getInt1Ty(), false);
    Function *F = CGM.getIntrinsic(Intrinsic::cttz, ResultType);
    return Builder.CreateCall(F, {X, Undef});
  }

  case SystemZ::BI__builtin_s390_vfsqdb: {
    llvm::Type *ResultType = ConvertType(E->getType());
    Value *X = EmitScalarExpr(E->getArg(0));
    Function *F = CGM.getIntrinsic(Intrinsic::sqrt, ResultType);
    return Builder.CreateCall(F, X);
  }
  case SystemZ::BI__builtin_s390_vfmadb: {
    llvm::Type *ResultType = ConvertType(E->getType());
    Value *X = EmitScalarExpr(E->getArg(0));
    Value *Y = EmitScalarExpr(E->getArg(1));
    Value *Z = EmitScalarExpr(E->getArg(2));
    Function *F = CGM.getIntrinsic(Intrinsic::fma, ResultType);
    return Builder.CreateCall(F, {X, Y, Z});
  }
  case SystemZ::BI__builtin_s390_vfmsdb: {
    llvm::Type *ResultType = ConvertType(E->getType());
    Value *X = EmitScalarExpr(E->getArg(0));
    Value *Y = EmitScalarExpr(E->getArg(1));
    Value *Z = EmitScalarExpr(E->getArg(2));
    Value *Zero = llvm::ConstantFP::getZeroValueForNegation(ResultType);
    Function *F = CGM.getIntrinsic(Intrinsic::fma, ResultType);
    return Builder.CreateCall(F, {X, Y, Builder.CreateFSub(Zero, Z, "sub")});
  }
  case SystemZ::BI__builtin_s390_vflpdb: {
    llvm::Type *ResultType = ConvertType(E->getType());
    Value *X = EmitScalarExpr(E->getArg(0));
    Function *F = CGM.getIntrinsic(Intrinsic::fabs, ResultType);
    return Builder.CreateCall(F, X);
  }
  case SystemZ::BI__builtin_s390_vflndb: {
    llvm::Type *ResultType = ConvertType(E->getType());
    Value *X = EmitScalarExpr(E->getArg(0));
    Value *Zero = llvm::ConstantFP::getZeroValueForNegation(ResultType);
    Function *F = CGM.getIntrinsic(Intrinsic::fabs, ResultType);
    return Builder.CreateFSub(Zero, Builder.CreateCall(F, X), "sub");
  }
  case SystemZ::BI__builtin_s390_vfidb: {
    llvm::Type *ResultType = ConvertType(E->getType());
    Value *X = EmitScalarExpr(E->getArg(0));
    // Constant-fold the M4 and M5 mask arguments.
    llvm::APSInt M4, M5;
    bool IsConstM4 = E->getArg(1)->isIntegerConstantExpr(M4, getContext());
    bool IsConstM5 = E->getArg(2)->isIntegerConstantExpr(M5, getContext());
    assert(IsConstM4 && IsConstM5 && "Constant arg isn't actually constant?");
    (void)IsConstM4; (void)IsConstM5;
    // Check whether this instance of vfidb can be represented via a LLVM
    // standard intrinsic.  We only support some combinations of M4 and M5.
    Intrinsic::ID ID = Intrinsic::not_intrinsic;
    switch (M4.getZExtValue()) {
    default: break;
    case 0:  // IEEE-inexact exception allowed
      switch (M5.getZExtValue()) {
      default: break;
      case 0: ID = Intrinsic::rint; break;
      }
      break;
    case 4:  // IEEE-inexact exception suppressed
      switch (M5.getZExtValue()) {
      default: break;
      case 0: ID = Intrinsic::nearbyint; break;
      case 1: ID = Intrinsic::round; break;
      case 5: ID = Intrinsic::trunc; break;
      case 6: ID = Intrinsic::ceil; break;
      case 7: ID = Intrinsic::floor; break;
      }
      break;
    }
    if (ID != Intrinsic::not_intrinsic) {
      Function *F = CGM.getIntrinsic(ID, ResultType);
      return Builder.CreateCall(F, X);
    }
    Function *F = CGM.getIntrinsic(Intrinsic::s390_vfidb);
    Value *M4Value = llvm::ConstantInt::get(getLLVMContext(), M4);
    Value *M5Value = llvm::ConstantInt::get(getLLVMContext(), M5);
    return Builder.CreateCall(F, {X, M4Value, M5Value});
  }

  // Vector intrisincs that output the post-instruction CC value.

#define INTRINSIC_WITH_CC(NAME) \
    case SystemZ::BI__builtin_##NAME: \
      return EmitSystemZIntrinsicWithCC(*this, Intrinsic::NAME, E)

  INTRINSIC_WITH_CC(s390_vpkshs);
  INTRINSIC_WITH_CC(s390_vpksfs);
  INTRINSIC_WITH_CC(s390_vpksgs);

  INTRINSIC_WITH_CC(s390_vpklshs);
  INTRINSIC_WITH_CC(s390_vpklsfs);
  INTRINSIC_WITH_CC(s390_vpklsgs);

  INTRINSIC_WITH_CC(s390_vceqbs);
  INTRINSIC_WITH_CC(s390_vceqhs);
  INTRINSIC_WITH_CC(s390_vceqfs);
  INTRINSIC_WITH_CC(s390_vceqgs);

  INTRINSIC_WITH_CC(s390_vchbs);
  INTRINSIC_WITH_CC(s390_vchhs);
  INTRINSIC_WITH_CC(s390_vchfs);
  INTRINSIC_WITH_CC(s390_vchgs);

  INTRINSIC_WITH_CC(s390_vchlbs);
  INTRINSIC_WITH_CC(s390_vchlhs);
  INTRINSIC_WITH_CC(s390_vchlfs);
  INTRINSIC_WITH_CC(s390_vchlgs);

  INTRINSIC_WITH_CC(s390_vfaebs);
  INTRINSIC_WITH_CC(s390_vfaehs);
  INTRINSIC_WITH_CC(s390_vfaefs);

  INTRINSIC_WITH_CC(s390_vfaezbs);
  INTRINSIC_WITH_CC(s390_vfaezhs);
  INTRINSIC_WITH_CC(s390_vfaezfs);

  INTRINSIC_WITH_CC(s390_vfeebs);
  INTRINSIC_WITH_CC(s390_vfeehs);
  INTRINSIC_WITH_CC(s390_vfeefs);

  INTRINSIC_WITH_CC(s390_vfeezbs);
  INTRINSIC_WITH_CC(s390_vfeezhs);
  INTRINSIC_WITH_CC(s390_vfeezfs);

  INTRINSIC_WITH_CC(s390_vfenebs);
  INTRINSIC_WITH_CC(s390_vfenehs);
  INTRINSIC_WITH_CC(s390_vfenefs);

  INTRINSIC_WITH_CC(s390_vfenezbs);
  INTRINSIC_WITH_CC(s390_vfenezhs);
  INTRINSIC_WITH_CC(s390_vfenezfs);

  INTRINSIC_WITH_CC(s390_vistrbs);
  INTRINSIC_WITH_CC(s390_vistrhs);
  INTRINSIC_WITH_CC(s390_vistrfs);

  INTRINSIC_WITH_CC(s390_vstrcbs);
  INTRINSIC_WITH_CC(s390_vstrchs);
  INTRINSIC_WITH_CC(s390_vstrcfs);

  INTRINSIC_WITH_CC(s390_vstrczbs);
  INTRINSIC_WITH_CC(s390_vstrczhs);
  INTRINSIC_WITH_CC(s390_vstrczfs);

  INTRINSIC_WITH_CC(s390_vfcedbs);
  INTRINSIC_WITH_CC(s390_vfchdbs);
  INTRINSIC_WITH_CC(s390_vfchedbs);

  INTRINSIC_WITH_CC(s390_vftcidb);

#undef INTRINSIC_WITH_CC

  default:
    return nullptr;
  }
}

Value *CodeGenFunction::EmitNVPTXBuiltinExpr(unsigned BuiltinID,
                                             const CallExpr *E) {
  switch (BuiltinID) {
  case NVPTX::BI__nvvm_atom_add_gen_i:
  case NVPTX::BI__nvvm_atom_add_gen_l:
  case NVPTX::BI__nvvm_atom_add_gen_ll:
    return MakeBinaryAtomicValue(*this, llvm::AtomicRMWInst::Add, E);

  case NVPTX::BI__nvvm_atom_sub_gen_i:
  case NVPTX::BI__nvvm_atom_sub_gen_l:
  case NVPTX::BI__nvvm_atom_sub_gen_ll:
    return MakeBinaryAtomicValue(*this, llvm::AtomicRMWInst::Sub, E);

  case NVPTX::BI__nvvm_atom_and_gen_i:
  case NVPTX::BI__nvvm_atom_and_gen_l:
  case NVPTX::BI__nvvm_atom_and_gen_ll:
    return MakeBinaryAtomicValue(*this, llvm::AtomicRMWInst::And, E);

  case NVPTX::BI__nvvm_atom_or_gen_i:
  case NVPTX::BI__nvvm_atom_or_gen_l:
  case NVPTX::BI__nvvm_atom_or_gen_ll:
    return MakeBinaryAtomicValue(*this, llvm::AtomicRMWInst::Or, E);

  case NVPTX::BI__nvvm_atom_xor_gen_i:
  case NVPTX::BI__nvvm_atom_xor_gen_l:
  case NVPTX::BI__nvvm_atom_xor_gen_ll:
    return MakeBinaryAtomicValue(*this, llvm::AtomicRMWInst::Xor, E);

  case NVPTX::BI__nvvm_atom_xchg_gen_i:
  case NVPTX::BI__nvvm_atom_xchg_gen_l:
  case NVPTX::BI__nvvm_atom_xchg_gen_ll:
    return MakeBinaryAtomicValue(*this, llvm::AtomicRMWInst::Xchg, E);

  case NVPTX::BI__nvvm_atom_max_gen_i:
  case NVPTX::BI__nvvm_atom_max_gen_l:
  case NVPTX::BI__nvvm_atom_max_gen_ll:
  case NVPTX::BI__nvvm_atom_max_gen_ui:
  case NVPTX::BI__nvvm_atom_max_gen_ul:
  case NVPTX::BI__nvvm_atom_max_gen_ull:
    return MakeBinaryAtomicValue(*this, llvm::AtomicRMWInst::Max, E);

  case NVPTX::BI__nvvm_atom_min_gen_i:
  case NVPTX::BI__nvvm_atom_min_gen_l:
  case NVPTX::BI__nvvm_atom_min_gen_ll:
  case NVPTX::BI__nvvm_atom_min_gen_ui:
  case NVPTX::BI__nvvm_atom_min_gen_ul:
  case NVPTX::BI__nvvm_atom_min_gen_ull:
    return MakeBinaryAtomicValue(*this, llvm::AtomicRMWInst::Min, E);

  case NVPTX::BI__nvvm_atom_cas_gen_i:
  case NVPTX::BI__nvvm_atom_cas_gen_l:
  case NVPTX::BI__nvvm_atom_cas_gen_ll:
    return MakeAtomicCmpXchgValue(*this, E, true);

  case NVPTX::BI__nvvm_atom_add_gen_f: {
    Value *Ptr = EmitScalarExpr(E->getArg(0));
    Value *Val = EmitScalarExpr(E->getArg(1));
    // atomicrmw only deals with integer arguments so we need to use
    // LLVM's nvvm_atomic_load_add_f32 intrinsic for that.
    Value *FnALAF32 =
        CGM.getIntrinsic(Intrinsic::nvvm_atomic_load_add_f32, Ptr->getType());
    return Builder.CreateCall(FnALAF32, {Ptr, Val});
  }

  default:
    return nullptr;
  }
}

#endif // HLSL Change Ends
