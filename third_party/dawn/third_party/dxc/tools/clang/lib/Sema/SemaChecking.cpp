//===--- SemaChecking.cpp - Extra Semantic Checking -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements extra semantic analysis beyond what is enforced
//  by the C type system.
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/SemaInternal.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/CharUnits.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/EvaluatedExprVisitor.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/ExprObjC.h"
#include "clang/AST/StmtCXX.h"
#include "clang/AST/StmtObjC.h"
#include "clang/Analysis/Analyses/FormatString.h"
#include "clang/Basic/CharInfo.h"
#include "clang/Basic/TargetBuiltins.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/Lexer.h" // TODO: Extract static functions to fix layering.
#include "clang/Sema/Initialization.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/ScopeInfo.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaHLSL.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/raw_ostream.h"
#include <limits>
using namespace clang;
using namespace sema;

SourceLocation Sema::getLocationOfStringLiteralByte(const StringLiteral *SL,
                                                    unsigned ByteNo) const {
  return SL->getLocationOfByte(ByteNo, getSourceManager(), LangOpts,
                               Context.getTargetInfo());
}

/// Checks that a call expression's argument count is the desired number.
/// This is useful when doing custom type-checking.  Returns true on error.
static bool checkArgCount(Sema &S, CallExpr *call, unsigned desiredArgCount) {
  unsigned argCount = call->getNumArgs();
  if (argCount == desiredArgCount) return false;

  if (argCount < desiredArgCount)
    return S.Diag(call->getLocEnd(), diag::err_typecheck_call_too_few_args)
        << 0 /*function call*/ << desiredArgCount << argCount
        << call->getSourceRange();

  // Highlight all the excess arguments.
  SourceRange range(call->getArg(desiredArgCount)->getLocStart(),
                    call->getArg(argCount - 1)->getLocEnd());
    
  return S.Diag(range.getBegin(), diag::err_typecheck_call_too_many_args)
    << 0 /*function call*/ << desiredArgCount << argCount
    << call->getArg(1)->getSourceRange();
}

/// Check that the first argument to __builtin_annotation is an integer
/// and the second argument is a non-wide string literal.
static bool SemaBuiltinAnnotation(Sema &S, CallExpr *TheCall) {
  if (checkArgCount(S, TheCall, 2))
    return true;

  // First argument should be an integer.
  Expr *ValArg = TheCall->getArg(0);
  QualType Ty = ValArg->getType();
  if (!Ty->isIntegerType()) {
    S.Diag(ValArg->getLocStart(), diag::err_builtin_annotation_first_arg)
      << ValArg->getSourceRange();
    return true;
  }

  // Second argument should be a constant string.
  Expr *StrArg = TheCall->getArg(1)->IgnoreParenCasts();
  StringLiteral *Literal = dyn_cast<StringLiteral>(StrArg);
  if (!Literal || !Literal->isAscii()) {
    S.Diag(StrArg->getLocStart(), diag::err_builtin_annotation_second_arg)
      << StrArg->getSourceRange();
    return true;
  }

  TheCall->setType(Ty);
  return false;
}

/// Check that the argument to __builtin_addressof is a glvalue, and set the
/// result type to the corresponding pointer type.
static bool SemaBuiltinAddressof(Sema &S, CallExpr *TheCall) {
  if (checkArgCount(S, TheCall, 1))
    return true;

  ExprResult Arg(TheCall->getArg(0));
  QualType ResultType = S.CheckAddressOfOperand(Arg, TheCall->getLocStart());
  if (ResultType.isNull())
    return true;

  TheCall->setArg(0, Arg.get());
  TheCall->setType(ResultType);
  return false;
}

static void SemaBuiltinMemChkCall(Sema &S, FunctionDecl *FDecl,
		                  CallExpr *TheCall, unsigned SizeIdx,
                                  unsigned DstSizeIdx) {
  if (TheCall->getNumArgs() <= SizeIdx ||
      TheCall->getNumArgs() <= DstSizeIdx)
    return;

  const Expr *SizeArg = TheCall->getArg(SizeIdx);
  const Expr *DstSizeArg = TheCall->getArg(DstSizeIdx);

  llvm::APSInt Size, DstSize;

  // find out if both sizes are known at compile time
  if (!SizeArg->EvaluateAsInt(Size, S.Context) ||
      !DstSizeArg->EvaluateAsInt(DstSize, S.Context))
    return;

  if (Size.ule(DstSize))
    return;

  // confirmed overflow so generate the diagnostic.
  IdentifierInfo *FnName = FDecl->getIdentifier();
  SourceLocation SL = TheCall->getLocStart();
  SourceRange SR = TheCall->getSourceRange();

  S.Diag(SL, diag::warn_memcpy_chk_overflow) << SR << FnName;
}

static bool SemaBuiltinCallWithStaticChain(Sema &S, CallExpr *BuiltinCall) {
  if (checkArgCount(S, BuiltinCall, 2))
    return true;

  SourceLocation BuiltinLoc = BuiltinCall->getLocStart();
  Expr *Builtin = BuiltinCall->getCallee()->IgnoreImpCasts();
  Expr *Call = BuiltinCall->getArg(0);
  Expr *Chain = BuiltinCall->getArg(1);

  if (Call->getStmtClass() != Stmt::CallExprClass) {
    S.Diag(BuiltinLoc, diag::err_first_argument_to_cwsc_not_call)
        << Call->getSourceRange();
    return true;
  }

  auto CE = cast<CallExpr>(Call);
  if (CE->getCallee()->getType()->isBlockPointerType()) {
    S.Diag(BuiltinLoc, diag::err_first_argument_to_cwsc_block_call)
        << Call->getSourceRange();
    return true;
  }

  const Decl *TargetDecl = CE->getCalleeDecl();
  if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(TargetDecl))
    if (FD->getBuiltinID()) {
      S.Diag(BuiltinLoc, diag::err_first_argument_to_cwsc_builtin_call)
          << Call->getSourceRange();
      return true;
    }

  if (isa<CXXPseudoDestructorExpr>(CE->getCallee()->IgnoreParens())) {
    S.Diag(BuiltinLoc, diag::err_first_argument_to_cwsc_pdtor_call)
        << Call->getSourceRange();
    return true;
  }

  ExprResult ChainResult = S.UsualUnaryConversions(Chain);
  if (ChainResult.isInvalid())
    return true;
  if (!ChainResult.get()->getType()->isPointerType()) {
    S.Diag(BuiltinLoc, diag::err_second_argument_to_cwsc_not_pointer)
        << Chain->getSourceRange();
    return true;
  }

  QualType ReturnTy = CE->getCallReturnType(S.Context);
  QualType ArgTys[2] = { ReturnTy, ChainResult.get()->getType() };
  QualType BuiltinTy = S.Context.getFunctionType(
      ReturnTy, ArgTys, FunctionProtoType::ExtProtoInfo(),
      None); // HLSL Change - builtins are all in parameters
  QualType BuiltinPtrTy = S.Context.getPointerType(BuiltinTy);

  Builtin =
      S.ImpCastExprToType(Builtin, BuiltinPtrTy, CK_BuiltinFnToFnPtr).get();

  BuiltinCall->setType(CE->getType());
  BuiltinCall->setValueKind(CE->getValueKind());
  BuiltinCall->setObjectKind(CE->getObjectKind());
  BuiltinCall->setCallee(Builtin);
  BuiltinCall->setArg(1, ChainResult.get());

  return false;
}

static bool SemaBuiltinSEHScopeCheck(Sema &SemaRef, CallExpr *TheCall,
                                     Scope::ScopeFlags NeededScopeFlags,
                                     unsigned DiagID) {
  // Scopes aren't available during instantiation. Fortunately, builtin
  // functions cannot be template args so they cannot be formed through template
  // instantiation. Therefore checking once during the parse is sufficient.
  if (!SemaRef.ActiveTemplateInstantiations.empty())
    return false;

  Scope *S = SemaRef.getCurScope();
  while (S && !S->isSEHExceptScope())
    S = S->getParent();
  if (!S || !(S->getFlags() & NeededScopeFlags)) {
    auto *DRE = cast<DeclRefExpr>(TheCall->getCallee()->IgnoreParenCasts());
    SemaRef.Diag(TheCall->getExprLoc(), DiagID)
        << DRE->getDecl()->getIdentifier();
    return true;
  }

  return false;
}

ExprResult
Sema::CheckBuiltinFunctionCall(FunctionDecl *FDecl, unsigned BuiltinID,
                               CallExpr *TheCall) {
  ExprResult TheCallResult(TheCall);

  // Find out if any arguments are required to be integer constant expressions.
  unsigned ICEArguments = 0;
  ASTContext::GetBuiltinTypeError Error;
  Context.GetBuiltinType(BuiltinID, Error, &ICEArguments);
  if (Error != ASTContext::GE_None)
    ICEArguments = 0;  // Don't diagnose previously diagnosed errors.
  
  // If any arguments are required to be ICE's, check and diagnose.
  for (unsigned ArgNo = 0; ICEArguments != 0; ++ArgNo) {
    // Skip arguments not required to be ICE's.
    if ((ICEArguments & (1 << ArgNo)) == 0) continue;
    
    llvm::APSInt Result;
    if (SemaBuiltinConstantArg(TheCall, ArgNo, Result))
      return true;
    ICEArguments &= ~(1 << ArgNo);
  }
  
  switch (BuiltinID) {
  case Builtin::BI__builtin___CFStringMakeConstantString:
    assert(TheCall->getNumArgs() == 1 &&
           "Wrong # arguments to builtin CFStringMakeConstantString");
    if (CheckObjCString(TheCall->getArg(0)))
      return ExprError();
    break;
  case Builtin::BI__builtin_stdarg_start:
  case Builtin::BI__builtin_va_start:
    if (SemaBuiltinVAStart(TheCall))
      return ExprError();
    break;
  case Builtin::BI__va_start: {
    switch (Context.getTargetInfo().getTriple().getArch()) {
    case llvm::Triple::arm:
    case llvm::Triple::thumb:
      if (SemaBuiltinVAStartARM(TheCall))
        return ExprError();
      break;
    default:
      if (SemaBuiltinVAStart(TheCall))
        return ExprError();
      break;
    }
    break;
  }
  case Builtin::BI__builtin_isgreater:
  case Builtin::BI__builtin_isgreaterequal:
  case Builtin::BI__builtin_isless:
  case Builtin::BI__builtin_islessequal:
  case Builtin::BI__builtin_islessgreater:
  case Builtin::BI__builtin_isunordered:
    if (SemaBuiltinUnorderedCompare(TheCall))
      return ExprError();
    break;
  case Builtin::BI__builtin_fpclassify:
    if (SemaBuiltinFPClassification(TheCall, 6))
      return ExprError();
    break;
  case Builtin::BI__builtin_isfinite:
  case Builtin::BI__builtin_isinf:
  case Builtin::BI__builtin_isinf_sign:
  case Builtin::BI__builtin_isnan:
  case Builtin::BI__builtin_isnormal:
    if (SemaBuiltinFPClassification(TheCall, 1))
      return ExprError();
    break;
  case Builtin::BI__builtin_shufflevector:
    return SemaBuiltinShuffleVector(TheCall);
    // TheCall will be freed by the smart pointer here, but that's fine, since
    // SemaBuiltinShuffleVector guts it, but then doesn't release it.
  case Builtin::BI__builtin_prefetch:
    if (SemaBuiltinPrefetch(TheCall))
      return ExprError();
    break;
  case Builtin::BI__assume:
  case Builtin::BI__builtin_assume:
    if (SemaBuiltinAssume(TheCall))
      return ExprError();
    break;
  case Builtin::BI__builtin_assume_aligned:
    if (SemaBuiltinAssumeAligned(TheCall))
      return ExprError();
    break;
  case Builtin::BI__builtin_object_size:
    if (SemaBuiltinConstantArgRange(TheCall, 1, 0, 3))
      return ExprError();
    break;
  case Builtin::BI__builtin_longjmp:
    if (SemaBuiltinLongjmp(TheCall))
      return ExprError();
    break;
  case Builtin::BI__builtin_setjmp:
    if (SemaBuiltinSetjmp(TheCall))
      return ExprError();
    break;
  case Builtin::BI_setjmp:
  case Builtin::BI_setjmpex:
    if (checkArgCount(*this, TheCall, 1))
      return true;
    break;

  case Builtin::BI__builtin_classify_type:
    if (checkArgCount(*this, TheCall, 1)) return true;
    TheCall->setType(Context.IntTy);
    break;
  case Builtin::BI__builtin_constant_p:
    if (checkArgCount(*this, TheCall, 1)) return true;
    TheCall->setType(Context.IntTy);
    break;
  case Builtin::BI__sync_fetch_and_add:
  case Builtin::BI__sync_fetch_and_add_1:
  case Builtin::BI__sync_fetch_and_add_2:
  case Builtin::BI__sync_fetch_and_add_4:
  case Builtin::BI__sync_fetch_and_add_8:
  case Builtin::BI__sync_fetch_and_add_16:
  case Builtin::BI__sync_fetch_and_sub:
  case Builtin::BI__sync_fetch_and_sub_1:
  case Builtin::BI__sync_fetch_and_sub_2:
  case Builtin::BI__sync_fetch_and_sub_4:
  case Builtin::BI__sync_fetch_and_sub_8:
  case Builtin::BI__sync_fetch_and_sub_16:
  case Builtin::BI__sync_fetch_and_or:
  case Builtin::BI__sync_fetch_and_or_1:
  case Builtin::BI__sync_fetch_and_or_2:
  case Builtin::BI__sync_fetch_and_or_4:
  case Builtin::BI__sync_fetch_and_or_8:
  case Builtin::BI__sync_fetch_and_or_16:
  case Builtin::BI__sync_fetch_and_and:
  case Builtin::BI__sync_fetch_and_and_1:
  case Builtin::BI__sync_fetch_and_and_2:
  case Builtin::BI__sync_fetch_and_and_4:
  case Builtin::BI__sync_fetch_and_and_8:
  case Builtin::BI__sync_fetch_and_and_16:
  case Builtin::BI__sync_fetch_and_xor:
  case Builtin::BI__sync_fetch_and_xor_1:
  case Builtin::BI__sync_fetch_and_xor_2:
  case Builtin::BI__sync_fetch_and_xor_4:
  case Builtin::BI__sync_fetch_and_xor_8:
  case Builtin::BI__sync_fetch_and_xor_16:
  case Builtin::BI__sync_fetch_and_nand:
  case Builtin::BI__sync_fetch_and_nand_1:
  case Builtin::BI__sync_fetch_and_nand_2:
  case Builtin::BI__sync_fetch_and_nand_4:
  case Builtin::BI__sync_fetch_and_nand_8:
  case Builtin::BI__sync_fetch_and_nand_16:
  case Builtin::BI__sync_add_and_fetch:
  case Builtin::BI__sync_add_and_fetch_1:
  case Builtin::BI__sync_add_and_fetch_2:
  case Builtin::BI__sync_add_and_fetch_4:
  case Builtin::BI__sync_add_and_fetch_8:
  case Builtin::BI__sync_add_and_fetch_16:
  case Builtin::BI__sync_sub_and_fetch:
  case Builtin::BI__sync_sub_and_fetch_1:
  case Builtin::BI__sync_sub_and_fetch_2:
  case Builtin::BI__sync_sub_and_fetch_4:
  case Builtin::BI__sync_sub_and_fetch_8:
  case Builtin::BI__sync_sub_and_fetch_16:
  case Builtin::BI__sync_and_and_fetch:
  case Builtin::BI__sync_and_and_fetch_1:
  case Builtin::BI__sync_and_and_fetch_2:
  case Builtin::BI__sync_and_and_fetch_4:
  case Builtin::BI__sync_and_and_fetch_8:
  case Builtin::BI__sync_and_and_fetch_16:
  case Builtin::BI__sync_or_and_fetch:
  case Builtin::BI__sync_or_and_fetch_1:
  case Builtin::BI__sync_or_and_fetch_2:
  case Builtin::BI__sync_or_and_fetch_4:
  case Builtin::BI__sync_or_and_fetch_8:
  case Builtin::BI__sync_or_and_fetch_16:
  case Builtin::BI__sync_xor_and_fetch:
  case Builtin::BI__sync_xor_and_fetch_1:
  case Builtin::BI__sync_xor_and_fetch_2:
  case Builtin::BI__sync_xor_and_fetch_4:
  case Builtin::BI__sync_xor_and_fetch_8:
  case Builtin::BI__sync_xor_and_fetch_16:
  case Builtin::BI__sync_nand_and_fetch:
  case Builtin::BI__sync_nand_and_fetch_1:
  case Builtin::BI__sync_nand_and_fetch_2:
  case Builtin::BI__sync_nand_and_fetch_4:
  case Builtin::BI__sync_nand_and_fetch_8:
  case Builtin::BI__sync_nand_and_fetch_16:
  case Builtin::BI__sync_val_compare_and_swap:
  case Builtin::BI__sync_val_compare_and_swap_1:
  case Builtin::BI__sync_val_compare_and_swap_2:
  case Builtin::BI__sync_val_compare_and_swap_4:
  case Builtin::BI__sync_val_compare_and_swap_8:
  case Builtin::BI__sync_val_compare_and_swap_16:
  case Builtin::BI__sync_bool_compare_and_swap:
  case Builtin::BI__sync_bool_compare_and_swap_1:
  case Builtin::BI__sync_bool_compare_and_swap_2:
  case Builtin::BI__sync_bool_compare_and_swap_4:
  case Builtin::BI__sync_bool_compare_and_swap_8:
  case Builtin::BI__sync_bool_compare_and_swap_16:
  case Builtin::BI__sync_lock_test_and_set:
  case Builtin::BI__sync_lock_test_and_set_1:
  case Builtin::BI__sync_lock_test_and_set_2:
  case Builtin::BI__sync_lock_test_and_set_4:
  case Builtin::BI__sync_lock_test_and_set_8:
  case Builtin::BI__sync_lock_test_and_set_16:
  case Builtin::BI__sync_lock_release:
  case Builtin::BI__sync_lock_release_1:
  case Builtin::BI__sync_lock_release_2:
  case Builtin::BI__sync_lock_release_4:
  case Builtin::BI__sync_lock_release_8:
  case Builtin::BI__sync_lock_release_16:
  case Builtin::BI__sync_swap:
  case Builtin::BI__sync_swap_1:
  case Builtin::BI__sync_swap_2:
  case Builtin::BI__sync_swap_4:
  case Builtin::BI__sync_swap_8:
  case Builtin::BI__sync_swap_16:
    return SemaBuiltinAtomicOverloaded(TheCallResult);
#define BUILTIN(ID, TYPE, ATTRS)
#define ATOMIC_BUILTIN(ID, TYPE, ATTRS) \
  case Builtin::BI##ID: \
    return SemaAtomicOpsOverloaded(TheCallResult, AtomicExpr::AO##ID);
#include "clang/Basic/Builtins.def"
  case Builtin::BI__builtin_annotation:
    if (SemaBuiltinAnnotation(*this, TheCall))
      return ExprError();
    break;
  case Builtin::BI__builtin_addressof:
    if (SemaBuiltinAddressof(*this, TheCall))
      return ExprError();
    break;
  case Builtin::BI__builtin_operator_new:
  case Builtin::BI__builtin_operator_delete:
    if (!getLangOpts().CPlusPlus) {
      Diag(TheCall->getExprLoc(), diag::err_builtin_requires_language)
        << (BuiltinID == Builtin::BI__builtin_operator_new
                ? "__builtin_operator_new"
                : "__builtin_operator_delete")
        << "C++";
      return ExprError();
    }
    // CodeGen assumes it can find the global new and delete to call,
    // so ensure that they are declared.
    DeclareGlobalNewDelete();
    break;

  // check secure string manipulation functions where overflows
  // are detectable at compile time
  case Builtin::BI__builtin___memcpy_chk:
  case Builtin::BI__builtin___memmove_chk:
  case Builtin::BI__builtin___memset_chk:
  case Builtin::BI__builtin___strlcat_chk:
  case Builtin::BI__builtin___strlcpy_chk:
  case Builtin::BI__builtin___strncat_chk:
  case Builtin::BI__builtin___strncpy_chk:
  case Builtin::BI__builtin___stpncpy_chk:
    SemaBuiltinMemChkCall(*this, FDecl, TheCall, 2, 3);
    break;
  case Builtin::BI__builtin___memccpy_chk:
    SemaBuiltinMemChkCall(*this, FDecl, TheCall, 3, 4);
    break;
  case Builtin::BI__builtin___snprintf_chk:
  case Builtin::BI__builtin___vsnprintf_chk:
    SemaBuiltinMemChkCall(*this, FDecl, TheCall, 1, 3);
    break;

  case Builtin::BI__builtin_call_with_static_chain:
    if (SemaBuiltinCallWithStaticChain(*this, TheCall))
      return ExprError();
    break;

  case Builtin::BI__exception_code:
  case Builtin::BI_exception_code: {
    if (SemaBuiltinSEHScopeCheck(*this, TheCall, Scope::SEHExceptScope,
                                 diag::err_seh___except_block))
      return ExprError();
    break;
  }
  case Builtin::BI__exception_info:
  case Builtin::BI_exception_info: {
    if (SemaBuiltinSEHScopeCheck(*this, TheCall, Scope::SEHFilterScope,
                                 diag::err_seh___except_filter))
      return ExprError();
    break;
  }

  case Builtin::BI__GetExceptionInfo:
    if (checkArgCount(*this, TheCall, 1))
      return ExprError();

    if (CheckCXXThrowOperand(
            TheCall->getLocStart(),
            Context.getExceptionObjectType(FDecl->getParamDecl(0)->getType()),
            TheCall))
      return ExprError();

    TheCall->setType(Context.VoidPtrTy);
    break;
  }

  // Since the target specific builtins for each arch overlap, only check those
  // of the arch we are compiling for.
#if 0 // HLSL Change Starts
  if (BuiltinID >= Builtin::FirstTSBuiltin) {
    switch (Context.getTargetInfo().getTriple().getArch()) {
      case llvm::Triple::arm:
      case llvm::Triple::armeb:
      case llvm::Triple::thumb:
      case llvm::Triple::thumbeb:
        if (CheckARMBuiltinFunctionCall(BuiltinID, TheCall))
          return ExprError();
        break;
      case llvm::Triple::aarch64:
      case llvm::Triple::aarch64_be:
        if (CheckAArch64BuiltinFunctionCall(BuiltinID, TheCall))
          return ExprError();
        break;
      case llvm::Triple::mips:
      case llvm::Triple::mipsel:
      case llvm::Triple::mips64:
      case llvm::Triple::mips64el:
        if (CheckMipsBuiltinFunctionCall(BuiltinID, TheCall))
          return ExprError();
        break;
      case llvm::Triple::systemz:
        if (CheckSystemZBuiltinFunctionCall(BuiltinID, TheCall))
          return ExprError();
        break;
      case llvm::Triple::x86:
      case llvm::Triple::x86_64:
        if (CheckX86BuiltinFunctionCall(BuiltinID, TheCall))
          return ExprError();
        break;
      case llvm::Triple::ppc:
      case llvm::Triple::ppc64:
      case llvm::Triple::ppc64le:
        if (CheckPPCBuiltinFunctionCall(BuiltinID, TheCall))
          return ExprError();
        break;
      default:
        break;
    }
  }
#endif // HLSL Change Ends

  return TheCallResult;
}

#if 0 // HLSL Change Starts

// Get the valid immediate range for the specified NEON type code.
static unsigned RFT(unsigned t, bool shift = false, bool ForceQuad = false) {
  NeonTypeFlags Type(t);
  int IsQuad = ForceQuad ? true : Type.isQuad();
  switch (Type.getEltType()) {
  case NeonTypeFlags::Int8:
  case NeonTypeFlags::Poly8:
    return shift ? 7 : (8 << IsQuad) - 1;
  case NeonTypeFlags::Int16:
  case NeonTypeFlags::Poly16:
    return shift ? 15 : (4 << IsQuad) - 1;
  case NeonTypeFlags::Int32:
    return shift ? 31 : (2 << IsQuad) - 1;
  case NeonTypeFlags::Int64:
  case NeonTypeFlags::Poly64:
    return shift ? 63 : (1 << IsQuad) - 1;
  case NeonTypeFlags::Poly128:
    return shift ? 127 : (1 << IsQuad) - 1;
  case NeonTypeFlags::Float16:
    assert(!shift && "cannot shift float types!");
    return (4 << IsQuad) - 1;
  case NeonTypeFlags::Float32:
    assert(!shift && "cannot shift float types!");
    return (2 << IsQuad) - 1;
  case NeonTypeFlags::Float64:
    assert(!shift && "cannot shift float types!");
    return (1 << IsQuad) - 1;
  }
  llvm_unreachable("Invalid NeonTypeFlag!");
}

/// getNeonEltType - Return the QualType corresponding to the elements of
/// the vector type specified by the NeonTypeFlags.  This is used to check
/// the pointer arguments for Neon load/store intrinsics.
static QualType getNeonEltType(NeonTypeFlags Flags, ASTContext &Context,
                               bool IsPolyUnsigned, bool IsInt64Long) {
  switch (Flags.getEltType()) {
  case NeonTypeFlags::Int8:
    return Flags.isUnsigned() ? Context.UnsignedCharTy : Context.SignedCharTy;
  case NeonTypeFlags::Int16:
    return Flags.isUnsigned() ? Context.UnsignedShortTy : Context.ShortTy;
  case NeonTypeFlags::Int32:
    return Flags.isUnsigned() ? Context.UnsignedIntTy : Context.IntTy;
  case NeonTypeFlags::Int64:
    if (IsInt64Long)
      return Flags.isUnsigned() ? Context.UnsignedLongTy : Context.LongTy;
    else
      return Flags.isUnsigned() ? Context.UnsignedLongLongTy
                                : Context.LongLongTy;
  case NeonTypeFlags::Poly8:
    return IsPolyUnsigned ? Context.UnsignedCharTy : Context.SignedCharTy;
  case NeonTypeFlags::Poly16:
    return IsPolyUnsigned ? Context.UnsignedShortTy : Context.ShortTy;
  case NeonTypeFlags::Poly64:
    if (IsInt64Long)
      return Context.UnsignedLongTy;
    else
      return Context.UnsignedLongLongTy;
  case NeonTypeFlags::Poly128:
    break;
  case NeonTypeFlags::Float16:
    return Context.HalfTy;
  case NeonTypeFlags::Float32:
    return Context.FloatTy;
  case NeonTypeFlags::Float64:
    return Context.DoubleTy;
  }
  llvm_unreachable("Invalid NeonTypeFlag!");
}

bool Sema::CheckNeonBuiltinFunctionCall(unsigned BuiltinID, CallExpr *TheCall) {
  llvm::APSInt Result;
  uint64_t mask = 0;
  unsigned TV = 0;
  int PtrArgNum = -1;
  bool HasConstPtr = false;
  switch (BuiltinID) {
#define GET_NEON_OVERLOAD_CHECK
#include "clang/Basic/arm_neon.inc"
#undef GET_NEON_OVERLOAD_CHECK
  }

  // For NEON intrinsics which are overloaded on vector element type, validate
  // the immediate which specifies which variant to emit.
  unsigned ImmArg = TheCall->getNumArgs()-1;
  if (mask) {
    if (SemaBuiltinConstantArg(TheCall, ImmArg, Result))
      return true;

    TV = Result.getLimitedValue(64);
    if ((TV > 63) || (mask & (1ULL << TV)) == 0)
      return Diag(TheCall->getLocStart(), diag::err_invalid_neon_type_code)
        << TheCall->getArg(ImmArg)->getSourceRange();
  }

  if (PtrArgNum >= 0) {
    // Check that pointer arguments have the specified type.
    Expr *Arg = TheCall->getArg(PtrArgNum);
    if (ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(Arg))
      Arg = ICE->getSubExpr();
    ExprResult RHS = DefaultFunctionArrayLvalueConversion(Arg);
    QualType RHSTy = RHS.get()->getType();

    llvm::Triple::ArchType Arch = Context.getTargetInfo().getTriple().getArch();
    bool IsPolyUnsigned = Arch == llvm::Triple::aarch64;
    bool IsInt64Long =
        Context.getTargetInfo().getInt64Type() == TargetInfo::SignedLong;
    QualType EltTy =
        getNeonEltType(NeonTypeFlags(TV), Context, IsPolyUnsigned, IsInt64Long);
    if (HasConstPtr)
      EltTy = EltTy.withConst();
    QualType LHSTy = Context.getPointerType(EltTy);
    AssignConvertType ConvTy;
    ConvTy = CheckSingleAssignmentConstraints(LHSTy, RHS);
    if (RHS.isInvalid())
      return true;
    if (DiagnoseAssignmentResult(ConvTy, Arg->getLocStart(), LHSTy, RHSTy,
                                 RHS.get(), AA_Assigning))
      return true;
  }

  // For NEON intrinsics which take an immediate value as part of the
  // instruction, range check them here.
  unsigned i = 0, l = 0, u = 0;
  switch (BuiltinID) {
  default:
    return false;
#define GET_NEON_IMMEDIATE_CHECK
#include "clang/Basic/arm_neon.inc"
#undef GET_NEON_IMMEDIATE_CHECK
  }

  return SemaBuiltinConstantArgRange(TheCall, i, l, u + l);
}

bool Sema::CheckARMBuiltinExclusiveCall(unsigned BuiltinID, CallExpr *TheCall,
                                        unsigned MaxWidth) {
  assert((BuiltinID == ARM::BI__builtin_arm_ldrex ||
          BuiltinID == ARM::BI__builtin_arm_ldaex ||
          BuiltinID == ARM::BI__builtin_arm_strex ||
          BuiltinID == ARM::BI__builtin_arm_stlex ||
          BuiltinID == AArch64::BI__builtin_arm_ldrex ||
          BuiltinID == AArch64::BI__builtin_arm_ldaex ||
          BuiltinID == AArch64::BI__builtin_arm_strex ||
          BuiltinID == AArch64::BI__builtin_arm_stlex) &&
         "unexpected ARM builtin");
  bool IsLdrex = BuiltinID == ARM::BI__builtin_arm_ldrex ||
                 BuiltinID == ARM::BI__builtin_arm_ldaex ||
                 BuiltinID == AArch64::BI__builtin_arm_ldrex ||
                 BuiltinID == AArch64::BI__builtin_arm_ldaex;

  DeclRefExpr *DRE =cast<DeclRefExpr>(TheCall->getCallee()->IgnoreParenCasts());

  // Ensure that we have the proper number of arguments.
  if (checkArgCount(*this, TheCall, IsLdrex ? 1 : 2))
    return true;

  // Inspect the pointer argument of the atomic builtin.  This should always be
  // a pointer type, whose element is an integral scalar or pointer type.
  // Because it is a pointer type, we don't have to worry about any implicit
  // casts here.
  Expr *PointerArg = TheCall->getArg(IsLdrex ? 0 : 1);
  ExprResult PointerArgRes = DefaultFunctionArrayLvalueConversion(PointerArg);
  if (PointerArgRes.isInvalid())
    return true;
  PointerArg = PointerArgRes.get();

  const PointerType *pointerType = PointerArg->getType()->getAs<PointerType>();
  if (!pointerType) {
    Diag(DRE->getLocStart(), diag::err_atomic_builtin_must_be_pointer)
      << PointerArg->getType() << PointerArg->getSourceRange();
    return true;
  }

  // ldrex takes a "const volatile T*" and strex takes a "volatile T*". Our next
  // task is to insert the appropriate casts into the AST. First work out just
  // what the appropriate type is.
  QualType ValType = pointerType->getPointeeType();
  QualType AddrType = ValType.getUnqualifiedType().withVolatile();
  if (IsLdrex)
    AddrType.addConst();

  // Issue a warning if the cast is dodgy.
  CastKind CastNeeded = CK_NoOp;
  if (!AddrType.isAtLeastAsQualifiedAs(ValType)) {
    CastNeeded = CK_BitCast;
    Diag(DRE->getLocStart(), diag::ext_typecheck_convert_discards_qualifiers)
      << PointerArg->getType()
      << Context.getPointerType(AddrType)
      << AA_Passing << PointerArg->getSourceRange();
  }

  // Finally, do the cast and replace the argument with the corrected version.
  AddrType = Context.getPointerType(AddrType);
  PointerArgRes = ImpCastExprToType(PointerArg, AddrType, CastNeeded);
  if (PointerArgRes.isInvalid())
    return true;
  PointerArg = PointerArgRes.get();

  TheCall->setArg(IsLdrex ? 0 : 1, PointerArg);

  // In general, we allow ints, floats and pointers to be loaded and stored.
  if (!ValType->isIntegerType() && !ValType->isAnyPointerType() &&
      !ValType->isBlockPointerType() && !ValType->isFloatingType()) {
    Diag(DRE->getLocStart(), diag::err_atomic_builtin_must_be_pointer_intfltptr)
      << PointerArg->getType() << PointerArg->getSourceRange();
    return true;
  }

  // But ARM doesn't have instructions to deal with 128-bit versions.
  if (Context.getTypeSize(ValType) > MaxWidth) {
    assert(MaxWidth == 64 && "Diagnostic unexpectedly inaccurate");
    Diag(DRE->getLocStart(), diag::err_atomic_exclusive_builtin_pointer_size)
      << PointerArg->getType() << PointerArg->getSourceRange();
    return true;
  }

  switch (ValType.getObjCLifetime()) {
  case Qualifiers::OCL_None:
  case Qualifiers::OCL_ExplicitNone:
    // okay
    break;

  case Qualifiers::OCL_Weak:
  case Qualifiers::OCL_Strong:
  case Qualifiers::OCL_Autoreleasing:
    Diag(DRE->getLocStart(), diag::err_arc_atomic_ownership)
      << ValType << PointerArg->getSourceRange();
    return true;
  }


  if (IsLdrex) {
    TheCall->setType(ValType);
    return false;
  }

  // Initialize the argument to be stored.
  ExprResult ValArg = TheCall->getArg(0);
  InitializedEntity Entity = InitializedEntity::InitializeParameter(
      Context, ValType, /*consume*/ false);
  ValArg = PerformCopyInitialization(Entity, SourceLocation(), ValArg);
  if (ValArg.isInvalid())
    return true;
  TheCall->setArg(0, ValArg.get());

  // __builtin_arm_strex always returns an int. It's marked as such in the .def,
  // but the custom checker bypasses all default analysis.
  TheCall->setType(Context.IntTy);
  return false;
}

bool Sema::CheckARMBuiltinFunctionCall(unsigned BuiltinID, CallExpr *TheCall) {
  llvm::APSInt Result;

  if (BuiltinID == ARM::BI__builtin_arm_ldrex ||
      BuiltinID == ARM::BI__builtin_arm_ldaex ||
      BuiltinID == ARM::BI__builtin_arm_strex ||
      BuiltinID == ARM::BI__builtin_arm_stlex) {
    return CheckARMBuiltinExclusiveCall(BuiltinID, TheCall, 64);
  }

  if (BuiltinID == ARM::BI__builtin_arm_prefetch) {
    return SemaBuiltinConstantArgRange(TheCall, 1, 0, 1) ||
      SemaBuiltinConstantArgRange(TheCall, 2, 0, 1);
  }

  if (BuiltinID == ARM::BI__builtin_arm_rsr64 ||
      BuiltinID == ARM::BI__builtin_arm_wsr64)
    return SemaBuiltinARMSpecialReg(BuiltinID, TheCall, 0, 3, false);

  if (BuiltinID == ARM::BI__builtin_arm_rsr ||
      BuiltinID == ARM::BI__builtin_arm_rsrp ||
      BuiltinID == ARM::BI__builtin_arm_wsr ||
      BuiltinID == ARM::BI__builtin_arm_wsrp)
    return SemaBuiltinARMSpecialReg(BuiltinID, TheCall, 0, 5, true);

  if (CheckNeonBuiltinFunctionCall(BuiltinID, TheCall))
    return true;

  // For intrinsics which take an immediate value as part of the instruction,
  // range check them here.
  unsigned i = 0, l = 0, u = 0;
  switch (BuiltinID) {
  default: return false;
  case ARM::BI__builtin_arm_ssat: i = 1; l = 1; u = 31; break;
  case ARM::BI__builtin_arm_usat: i = 1; u = 31; break;
  case ARM::BI__builtin_arm_vcvtr_f:
  case ARM::BI__builtin_arm_vcvtr_d: i = 1; u = 1; break;
  case ARM::BI__builtin_arm_dmb:
  case ARM::BI__builtin_arm_dsb:
  case ARM::BI__builtin_arm_isb:
  case ARM::BI__builtin_arm_dbg: l = 0; u = 15; break;
  }

  // FIXME: VFP Intrinsics should error if VFP not present.
  return SemaBuiltinConstantArgRange(TheCall, i, l, u + l);
}

bool Sema::CheckAArch64BuiltinFunctionCall(unsigned BuiltinID,
                                         CallExpr *TheCall) {
  llvm::APSInt Result;

  if (BuiltinID == AArch64::BI__builtin_arm_ldrex ||
      BuiltinID == AArch64::BI__builtin_arm_ldaex ||
      BuiltinID == AArch64::BI__builtin_arm_strex ||
      BuiltinID == AArch64::BI__builtin_arm_stlex) {
    return CheckARMBuiltinExclusiveCall(BuiltinID, TheCall, 128);
  }

  if (BuiltinID == AArch64::BI__builtin_arm_prefetch) {
    return SemaBuiltinConstantArgRange(TheCall, 1, 0, 1) ||
      SemaBuiltinConstantArgRange(TheCall, 2, 0, 2) ||
      SemaBuiltinConstantArgRange(TheCall, 3, 0, 1) ||
      SemaBuiltinConstantArgRange(TheCall, 4, 0, 1);
  }

  if (BuiltinID == AArch64::BI__builtin_arm_rsr64 ||
      BuiltinID == AArch64::BI__builtin_arm_wsr64)
    return SemaBuiltinARMSpecialReg(BuiltinID, TheCall, 0, 5, false);

  if (BuiltinID == AArch64::BI__builtin_arm_rsr ||
      BuiltinID == AArch64::BI__builtin_arm_rsrp ||
      BuiltinID == AArch64::BI__builtin_arm_wsr ||
      BuiltinID == AArch64::BI__builtin_arm_wsrp)
    return SemaBuiltinARMSpecialReg(BuiltinID, TheCall, 0, 5, true);

  if (CheckNeonBuiltinFunctionCall(BuiltinID, TheCall))
    return true;

  // For intrinsics which take an immediate value as part of the instruction,
  // range check them here.
  unsigned i = 0, l = 0, u = 0;
  switch (BuiltinID) {
  default: return false;
  case AArch64::BI__builtin_arm_dmb:
  case AArch64::BI__builtin_arm_dsb:
  case AArch64::BI__builtin_arm_isb: l = 0; u = 15; break;
  }

  return SemaBuiltinConstantArgRange(TheCall, i, l, u + l);
}

bool Sema::CheckMipsBuiltinFunctionCall(unsigned BuiltinID, CallExpr *TheCall) {
  unsigned i = 0, l = 0, u = 0;
  switch (BuiltinID) {
  default: return false;
  case Mips::BI__builtin_mips_wrdsp: i = 1; l = 0; u = 63; break;
  case Mips::BI__builtin_mips_rddsp: i = 0; l = 0; u = 63; break;
  case Mips::BI__builtin_mips_append: i = 2; l = 0; u = 31; break;
  case Mips::BI__builtin_mips_balign: i = 2; l = 0; u = 3; break;
  case Mips::BI__builtin_mips_precr_sra_ph_w: i = 2; l = 0; u = 31; break;
  case Mips::BI__builtin_mips_precr_sra_r_ph_w: i = 2; l = 0; u = 31; break;
  case Mips::BI__builtin_mips_prepend: i = 2; l = 0; u = 31; break;
  }

  return SemaBuiltinConstantArgRange(TheCall, i, l, u);
}

bool Sema::CheckPPCBuiltinFunctionCall(unsigned BuiltinID, CallExpr *TheCall) {
  unsigned i = 0, l = 0, u = 0;
  bool Is64BitBltin = BuiltinID == PPC::BI__builtin_divde ||
                      BuiltinID == PPC::BI__builtin_divdeu ||
                      BuiltinID == PPC::BI__builtin_bpermd;
  bool IsTarget64Bit = Context.getTargetInfo()
                              .getTypeWidth(Context
                                            .getTargetInfo()
                                            .getIntPtrType()) == 64;
  bool IsBltinExtDiv = BuiltinID == PPC::BI__builtin_divwe ||
                       BuiltinID == PPC::BI__builtin_divweu ||
                       BuiltinID == PPC::BI__builtin_divde ||
                       BuiltinID == PPC::BI__builtin_divdeu;

  if (Is64BitBltin && !IsTarget64Bit)
      return Diag(TheCall->getLocStart(), diag::err_64_bit_builtin_32_bit_tgt)
             << TheCall->getSourceRange();

  if ((IsBltinExtDiv && !Context.getTargetInfo().hasFeature("extdiv")) ||
      (BuiltinID == PPC::BI__builtin_bpermd &&
       !Context.getTargetInfo().hasFeature("bpermd")))
    return Diag(TheCall->getLocStart(), diag::err_ppc_builtin_only_on_pwr7)
           << TheCall->getSourceRange();

  switch (BuiltinID) {
  default: return false;
  case PPC::BI__builtin_altivec_crypto_vshasigmaw:
  case PPC::BI__builtin_altivec_crypto_vshasigmad:
    return SemaBuiltinConstantArgRange(TheCall, 1, 0, 1) ||
           SemaBuiltinConstantArgRange(TheCall, 2, 0, 15);
  case PPC::BI__builtin_tbegin:
  case PPC::BI__builtin_tend: i = 0; l = 0; u = 1; break;
  case PPC::BI__builtin_tsr: i = 0; l = 0; u = 7; break;
  case PPC::BI__builtin_tabortwc:
  case PPC::BI__builtin_tabortdc: i = 0; l = 0; u = 31; break;
  case PPC::BI__builtin_tabortwci:
  case PPC::BI__builtin_tabortdci:
    return SemaBuiltinConstantArgRange(TheCall, 0, 0, 31) ||
           SemaBuiltinConstantArgRange(TheCall, 2, 0, 31);
  }
  return SemaBuiltinConstantArgRange(TheCall, i, l, u);
}

bool Sema::CheckSystemZBuiltinFunctionCall(unsigned BuiltinID,
                                           CallExpr *TheCall) {
  if (BuiltinID == SystemZ::BI__builtin_tabort) {
    Expr *Arg = TheCall->getArg(0);
    llvm::APSInt AbortCode(32);
    if (Arg->isIntegerConstantExpr(AbortCode, Context) &&
        AbortCode.getSExtValue() >= 0 && AbortCode.getSExtValue() < 256)
      return Diag(Arg->getLocStart(), diag::err_systemz_invalid_tabort_code)
             << Arg->getSourceRange();
  }

  // For intrinsics which take an immediate value as part of the instruction,
  // range check them here.
  unsigned i = 0, l = 0, u = 0;
  switch (BuiltinID) {
  default: return false;
  case SystemZ::BI__builtin_s390_lcbb: i = 1; l = 0; u = 15; break;
  case SystemZ::BI__builtin_s390_verimb:
  case SystemZ::BI__builtin_s390_verimh:
  case SystemZ::BI__builtin_s390_verimf:
  case SystemZ::BI__builtin_s390_verimg: i = 3; l = 0; u = 255; break;
  case SystemZ::BI__builtin_s390_vfaeb:
  case SystemZ::BI__builtin_s390_vfaeh:
  case SystemZ::BI__builtin_s390_vfaef:
  case SystemZ::BI__builtin_s390_vfaebs:
  case SystemZ::BI__builtin_s390_vfaehs:
  case SystemZ::BI__builtin_s390_vfaefs:
  case SystemZ::BI__builtin_s390_vfaezb:
  case SystemZ::BI__builtin_s390_vfaezh:
  case SystemZ::BI__builtin_s390_vfaezf:
  case SystemZ::BI__builtin_s390_vfaezbs:
  case SystemZ::BI__builtin_s390_vfaezhs:
  case SystemZ::BI__builtin_s390_vfaezfs: i = 2; l = 0; u = 15; break;
  case SystemZ::BI__builtin_s390_vfidb:
    return SemaBuiltinConstantArgRange(TheCall, 1, 0, 15) ||
           SemaBuiltinConstantArgRange(TheCall, 2, 0, 15);
  case SystemZ::BI__builtin_s390_vftcidb: i = 1; l = 0; u = 4095; break;
  case SystemZ::BI__builtin_s390_vlbb: i = 1; l = 0; u = 15; break;
  case SystemZ::BI__builtin_s390_vpdi: i = 2; l = 0; u = 15; break;
  case SystemZ::BI__builtin_s390_vsldb: i = 2; l = 0; u = 15; break;
  case SystemZ::BI__builtin_s390_vstrcb:
  case SystemZ::BI__builtin_s390_vstrch:
  case SystemZ::BI__builtin_s390_vstrcf:
  case SystemZ::BI__builtin_s390_vstrczb:
  case SystemZ::BI__builtin_s390_vstrczh:
  case SystemZ::BI__builtin_s390_vstrczf:
  case SystemZ::BI__builtin_s390_vstrcbs:
  case SystemZ::BI__builtin_s390_vstrchs:
  case SystemZ::BI__builtin_s390_vstrcfs:
  case SystemZ::BI__builtin_s390_vstrczbs:
  case SystemZ::BI__builtin_s390_vstrczhs:
  case SystemZ::BI__builtin_s390_vstrczfs: i = 3; l = 0; u = 15; break;
  }
  return SemaBuiltinConstantArgRange(TheCall, i, l, u);
}

bool Sema::CheckX86BuiltinFunctionCall(unsigned BuiltinID, CallExpr *TheCall) {
  unsigned i = 0, l = 0, u = 0;
  switch (BuiltinID) {
  default: return false;
  case X86::BI__builtin_cpu_supports:
    return SemaBuiltinCpuSupports(TheCall);
  case X86::BI_mm_prefetch: i = 1; l = 0; u = 3; break;
  case X86::BI__builtin_ia32_sha1rnds4: i = 2, l = 0; u = 3; break;
  case X86::BI__builtin_ia32_vpermil2pd:
  case X86::BI__builtin_ia32_vpermil2pd256:
  case X86::BI__builtin_ia32_vpermil2ps:
  case X86::BI__builtin_ia32_vpermil2ps256: i = 3, l = 0; u = 3; break;
  case X86::BI__builtin_ia32_cmpb128_mask:
  case X86::BI__builtin_ia32_cmpw128_mask:
  case X86::BI__builtin_ia32_cmpd128_mask:
  case X86::BI__builtin_ia32_cmpq128_mask:
  case X86::BI__builtin_ia32_cmpb256_mask:
  case X86::BI__builtin_ia32_cmpw256_mask:
  case X86::BI__builtin_ia32_cmpd256_mask:
  case X86::BI__builtin_ia32_cmpq256_mask:
  case X86::BI__builtin_ia32_cmpb512_mask:
  case X86::BI__builtin_ia32_cmpw512_mask:
  case X86::BI__builtin_ia32_cmpd512_mask:
  case X86::BI__builtin_ia32_cmpq512_mask:
  case X86::BI__builtin_ia32_ucmpb128_mask:
  case X86::BI__builtin_ia32_ucmpw128_mask:
  case X86::BI__builtin_ia32_ucmpd128_mask:
  case X86::BI__builtin_ia32_ucmpq128_mask:
  case X86::BI__builtin_ia32_ucmpb256_mask:
  case X86::BI__builtin_ia32_ucmpw256_mask:
  case X86::BI__builtin_ia32_ucmpd256_mask:
  case X86::BI__builtin_ia32_ucmpq256_mask:
  case X86::BI__builtin_ia32_ucmpb512_mask:
  case X86::BI__builtin_ia32_ucmpw512_mask:
  case X86::BI__builtin_ia32_ucmpd512_mask:
  case X86::BI__builtin_ia32_ucmpq512_mask: i = 2; l = 0; u = 7; break;
  case X86::BI__builtin_ia32_roundps:
  case X86::BI__builtin_ia32_roundpd:
  case X86::BI__builtin_ia32_roundps256:
  case X86::BI__builtin_ia32_roundpd256: i = 1, l = 0; u = 15; break;
  case X86::BI__builtin_ia32_roundss:
  case X86::BI__builtin_ia32_roundsd: i = 2, l = 0; u = 15; break;
  case X86::BI__builtin_ia32_cmpps:
  case X86::BI__builtin_ia32_cmpss:
  case X86::BI__builtin_ia32_cmppd:
  case X86::BI__builtin_ia32_cmpsd:
  case X86::BI__builtin_ia32_cmpps256:
  case X86::BI__builtin_ia32_cmppd256:
  case X86::BI__builtin_ia32_cmpps512_mask:
  case X86::BI__builtin_ia32_cmppd512_mask: i = 2; l = 0; u = 31; break;
  case X86::BI__builtin_ia32_vpcomub:
  case X86::BI__builtin_ia32_vpcomuw:
  case X86::BI__builtin_ia32_vpcomud:
  case X86::BI__builtin_ia32_vpcomuq:
  case X86::BI__builtin_ia32_vpcomb:
  case X86::BI__builtin_ia32_vpcomw:
  case X86::BI__builtin_ia32_vpcomd:
  case X86::BI__builtin_ia32_vpcomq: i = 2; l = 0; u = 7; break;
  }
  return SemaBuiltinConstantArgRange(TheCall, i, l, u);
}

#endif // HLSL Change Ends

/// Given a FunctionDecl's FormatAttr, attempts to populate the FomatStringInfo
/// parameter with the FormatAttr's correct format_idx and firstDataArg.
/// Returns true when the format fits the function and the FormatStringInfo has
/// been populated.
bool Sema::getFormatStringInfo(const FormatAttr *Format, bool IsCXXMember,
                               FormatStringInfo *FSI) {
  FSI->HasVAListArg = Format->getFirstArg() == 0;
  FSI->FormatIdx = Format->getFormatIdx() - 1;
  FSI->FirstDataArg = FSI->HasVAListArg ? 0 : Format->getFirstArg() - 1;

  // The way the format attribute works in GCC, the implicit this argument
  // of member functions is counted. However, it doesn't appear in our own
  // lists, so decrement format_idx in that case.
  if (IsCXXMember) {
    if(FSI->FormatIdx == 0)
      return false;
    --FSI->FormatIdx;
    if (FSI->FirstDataArg != 0)
      --FSI->FirstDataArg;
  }
  return true;
}

/// Checks if a the given expression evaluates to null.
///
/// \brief Returns true if the value evaluates to null.
static bool CheckNonNullExpr(Sema &S,
                             const Expr *Expr) {
  // If the expression has non-null type, it doesn't evaluate to null.
  if (auto nullability
        = Expr->IgnoreImplicit()->getType()->getNullability(S.Context)) {
    if (*nullability == NullabilityKind::NonNull)
      return false;
  }

  // As a special case, transparent unions initialized with zero are
  // considered null for the purposes of the nonnull attribute.
  if (const RecordType *UT = Expr->getType()->getAsUnionType()) {
    if (UT->getDecl()->hasAttr<TransparentUnionAttr>())
      if (const CompoundLiteralExpr *CLE =
          dyn_cast<CompoundLiteralExpr>(Expr))
        if (const InitListExpr *ILE =
            dyn_cast<InitListExpr>(CLE->getInitializer()))
          Expr = ILE->getInit(0);
  }

  bool Result;
  return (!Expr->isValueDependent() &&
          Expr->EvaluateAsBooleanCondition(Result, S.Context) &&
          !Result);
}

static void CheckNonNullArgument(Sema &S,
                                 const Expr *ArgExpr,
                                 SourceLocation CallSiteLoc) {
  if (CheckNonNullExpr(S, ArgExpr))
    S.Diag(CallSiteLoc, diag::warn_null_arg) << ArgExpr->getSourceRange();
}

bool Sema::GetFormatNSStringIdx(const FormatAttr *Format, unsigned &Idx) {
#if 0 // HLSL Change Starts
  FormatStringInfo FSI;
  if ((GetFormatStringType(Format) == FST_NSString) &&
      getFormatStringInfo(Format, false, &FSI)) {
    Idx = FSI.FormatIdx;
    return true;
  }
#endif // HLSL Change Ends
  return false;
}

#if 0 // HLSL Change Starts
/// \brief Diagnose use of %s directive in an NSString which is being passed
/// as formatting string to formatting method.
static void
DiagnoseCStringFormatDirectiveInCFAPI(Sema &S,
                                        const NamedDecl *FDecl,
                                        Expr **Args,
                                        unsigned NumArgs) {
  unsigned Idx = 0;
  bool Format = false;
  ObjCStringFormatFamily SFFamily = FDecl->getObjCFStringFormattingFamily();
  if (SFFamily == ObjCStringFormatFamily::SFF_CFString) {
    Idx = 2;
    Format = true;
  }
  else
    for (const auto *I : FDecl->specific_attrs<FormatAttr>()) {
      if (S.GetFormatNSStringIdx(I, Idx)) {
        Format = true;
        break;
      }
    }
  if (!Format || NumArgs <= Idx)
    return;
  const Expr *FormatExpr = Args[Idx];
  if (const CStyleCastExpr *CSCE = dyn_cast<CStyleCastExpr>(FormatExpr))
    FormatExpr = CSCE->getSubExpr();
  const StringLiteral *FormatString;
  if (const ObjCStringLiteral *OSL =
      dyn_cast<ObjCStringLiteral>(FormatExpr->IgnoreParenImpCasts()))
    FormatString = OSL->getString();
  else
    FormatString = dyn_cast<StringLiteral>(FormatExpr->IgnoreParenImpCasts());
  if (!FormatString)
    return;
  if (S.FormatStringHasSArg(FormatString)) {
    S.Diag(FormatExpr->getExprLoc(), diag::warn_objc_cdirective_format_string)
      << "%s" << 1 << 1;
    S.Diag(FDecl->getLocation(), diag::note_entity_declared_at)
      << FDecl->getDeclName();
  }
}
#endif // HLSL Change Ends

/// Determine whether the given type has a non-null nullability annotation.
static bool isNonNullType(ASTContext &ctx, QualType type) {
  if (auto nullability = type->getNullability(ctx))
    return *nullability == NullabilityKind::NonNull;
     
  return false;
}

static void CheckNonNullArguments(Sema &S,
                                  const NamedDecl *FDecl,
                                  const FunctionProtoType *Proto,
                                  ArrayRef<const Expr *> Args,
                                  SourceLocation CallSiteLoc) {
  assert((FDecl || Proto) && "Need a function declaration or prototype");

  // Check the attributes attached to the method/function itself.
  llvm::SmallBitVector NonNullArgs;
  if (FDecl) {
    // Handle the nonnull attribute on the function/method declaration itself.
    for (const auto *NonNull : FDecl->specific_attrs<NonNullAttr>()) {
      if (!NonNull->args_size()) {
        // Easy case: all pointer arguments are nonnull.
        for (const auto *Arg : Args)
          if (S.isValidPointerAttrType(Arg->getType()))
            CheckNonNullArgument(S, Arg, CallSiteLoc);
        return;
      }

      for (unsigned Val : NonNull->args()) {
        if (Val >= Args.size())
          continue;
        if (NonNullArgs.empty())
          NonNullArgs.resize(Args.size());
        NonNullArgs.set(Val);
      }
    }
  }

  if (FDecl && (isa<FunctionDecl>(FDecl) || isa<ObjCMethodDecl>(FDecl))) {
    // Handle the nonnull attribute on the parameters of the
    // function/method.
    ArrayRef<ParmVarDecl*> parms;
    if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(FDecl))
      parms = FD->parameters();
    else
      parms = cast<ObjCMethodDecl>(FDecl)->parameters();
    
    unsigned ParamIndex = 0;
    for (ArrayRef<ParmVarDecl*>::iterator I = parms.begin(), E = parms.end();
         I != E; ++I, ++ParamIndex) {
      const ParmVarDecl *PVD = *I;
      if (PVD->hasAttr<NonNullAttr>() || 
          isNonNullType(S.Context, PVD->getType())) {
        if (NonNullArgs.empty())
          NonNullArgs.resize(Args.size());

        NonNullArgs.set(ParamIndex);
      }
    }
  } else {
    // If we have a non-function, non-method declaration but no
    // function prototype, try to dig out the function prototype.
    if (!Proto) {
      if (const ValueDecl *VD = dyn_cast<ValueDecl>(FDecl)) {
        QualType type = VD->getType().getNonReferenceType();
        if (auto pointerType = type->getAs<PointerType>())
          type = pointerType->getPointeeType();
        else if (auto blockType = type->getAs<BlockPointerType>())
          type = blockType->getPointeeType();
        // FIXME: data member pointers?

        // Dig out the function prototype, if there is one.
        Proto = type->getAs<FunctionProtoType>();
      } 
    }

    // Fill in non-null argument information from the nullability
    // information on the parameter types (if we have them).
    if (Proto) {
      unsigned Index = 0;
      for (auto paramType : Proto->getParamTypes()) {
        if (isNonNullType(S.Context, paramType)) {
          if (NonNullArgs.empty())
            NonNullArgs.resize(Args.size());
          
          NonNullArgs.set(Index);
        }
        
        ++Index;
      }
    }
  }

  // Check for non-null arguments.
  for (unsigned ArgIndex = 0, ArgIndexEnd = NonNullArgs.size(); 
       ArgIndex != ArgIndexEnd; ++ArgIndex) {
    if (NonNullArgs[ArgIndex])
      CheckNonNullArgument(S, Args[ArgIndex], CallSiteLoc);
  }
}

/// Handles the checks for format strings, non-POD arguments to vararg
/// functions, and NULL arguments passed to non-NULL parameters.
void Sema::checkCall(NamedDecl *FDecl, const FunctionProtoType *Proto,
                     ArrayRef<const Expr *> Args, bool IsMemberFunction,
                     SourceLocation Loc, SourceRange Range,
                     VariadicCallType CallType) {
  // FIXME: We should check as much as we can in the template definition.
  if (CurContext->isDependentContext())
    return;

#if 1 // HLSL Change - no format string support
  (void)(IsMemberFunction); (void)(CallType); (void)(Range);
#else
  // Printf and scanf checking.
  llvm::SmallBitVector CheckedVarArgs;
  if (FDecl) {
    for (const auto *I : FDecl->specific_attrs<FormatAttr>()) {
      // Only create vector if there are format attributes.
      CheckedVarArgs.resize(Args.size());

      CheckFormatArguments(I, Args, IsMemberFunction, CallType, Loc, Range,
                           CheckedVarArgs);
    }
  }

  // Refuse POD arguments that weren't caught by the format string
  // checks above.
  if (CallType != VariadicDoesNotApply) {
    unsigned NumParams = Proto ? Proto->getNumParams()
                       : FDecl && isa<FunctionDecl>(FDecl)
                           ? cast<FunctionDecl>(FDecl)->getNumParams()
                       : FDecl && isa<ObjCMethodDecl>(FDecl)
                           ? cast<ObjCMethodDecl>(FDecl)->param_size()
                       : 0;

    for (unsigned ArgIdx = NumParams; ArgIdx < Args.size(); ++ArgIdx) {
      // Args[ArgIdx] can be null in malformed code.
      if (const Expr *Arg = Args[ArgIdx]) {
        if (CheckedVarArgs.empty() || !CheckedVarArgs[ArgIdx])
          checkVariadicArgument(Arg, CallType);
      }
    }
  }
#endif // HLSL Change - no format string support
  if (FDecl || Proto) {
    CheckNonNullArguments(*this, FDecl, Proto, Args, Loc);

    // Type safety checking.
    if (FDecl) {
      for (const auto *I : FDecl->specific_attrs<ArgumentWithTypeTagAttr>())
        CheckArgumentWithTypeTag(I, Args.data());
    }
  }
}

/// CheckConstructorCall - Check a constructor call for correctness and safety
/// properties not enforced by the C type system.
void Sema::CheckConstructorCall(FunctionDecl *FDecl,
                                ArrayRef<const Expr *> Args,
                                const FunctionProtoType *Proto,
                                SourceLocation Loc) {
  VariadicCallType CallType =
    Proto->isVariadic() ? VariadicConstructor : VariadicDoesNotApply;
  checkCall(FDecl, Proto, Args, /*IsMemberFunction=*/true, Loc, SourceRange(), 
            CallType);
}

/// CheckFunctionCall - Check a direct function call for various correctness
/// and safety properties not strictly enforced by the C type system.
bool Sema::CheckFunctionCall(FunctionDecl *FDecl, CallExpr *TheCall,
                             const FunctionProtoType *Proto) {
  bool IsMemberOperatorCall = isa<CXXOperatorCallExpr>(TheCall) &&
                              isa<CXXMethodDecl>(FDecl);
  bool IsMemberFunction = isa<CXXMemberCallExpr>(TheCall) ||
                          IsMemberOperatorCall;
  VariadicCallType CallType = getVariadicCallType(FDecl, Proto,
                                                  TheCall->getCallee());
  Expr** Args = TheCall->getArgs();
  unsigned NumArgs = TheCall->getNumArgs();
  if (IsMemberOperatorCall) {
    // If this is a call to a member operator, hide the first argument
    // from checkCall.
    // FIXME: Our choice of AST representation here is less than ideal.
    ++Args;
    --NumArgs;
  }
  checkCall(FDecl, Proto, llvm::makeArrayRef(Args, NumArgs), 
            IsMemberFunction, TheCall->getRParenLoc(),
            TheCall->getCallee()->getSourceRange(), CallType);

  IdentifierInfo *FnInfo = FDecl->getIdentifier();
  // None of the checks below are needed for functions that don't have
  // simple names (e.g., C++ conversion functions).
  if (!FnInfo)
    return false;

  CheckAbsoluteValueFunction(TheCall, FDecl, FnInfo);
#if 0 // HLSL Change Starts
  if (getLangOpts().ObjC1)
    DiagnoseCStringFormatDirectiveInCFAPI(*this, FDecl, Args, NumArgs);

  unsigned CMId = FDecl->getMemoryFunctionKind();
  if (CMId == 0)
    return false;

  // Handle memory setting and copying functions.
  if (CMId == Builtin::BIstrlcpy || CMId == Builtin::BIstrlcat)
    CheckStrlcpycatArguments(TheCall, FnInfo);
  else if (CMId == Builtin::BIstrncat)
    CheckStrncatArguments(TheCall, FnInfo);
  else
    CheckMemaccessArguments(TheCall, CMId, FnInfo);
#endif // HLSL Change Ends

  CheckHLSLFunctionCall(FDecl, TheCall, Proto); // HLSL Change

  return false;
}

bool Sema::CheckObjCMethodCall(ObjCMethodDecl *Method, SourceLocation lbrac, 
                               ArrayRef<const Expr *> Args) {
#if 0 // HLSL Change Starts
  VariadicCallType CallType =
      Method->isVariadic() ? VariadicMethod : VariadicDoesNotApply;

  checkCall(Method, nullptr, Args,
            /*IsMemberFunction=*/false, lbrac, Method->getSourceRange(), 
            CallType);
#endif // HLSL Change Ends
  return false;
}

bool Sema::CheckPointerCall(NamedDecl *NDecl, CallExpr *TheCall,
                            const FunctionProtoType *Proto) {
#if 0 // HLSL Change Starts
  QualType Ty;
  if (const auto *V = dyn_cast<VarDecl>(NDecl))
    Ty = V->getType().getNonReferenceType();
  else if (const auto *F = dyn_cast<FieldDecl>(NDecl))
    Ty = F->getType().getNonReferenceType();
  else
    return false;

  if (!Ty->isBlockPointerType() && !Ty->isFunctionPointerType() &&
      !Ty->isFunctionProtoType())
    return false;

  VariadicCallType CallType;
  if (!Proto || !Proto->isVariadic()) {
    CallType = VariadicDoesNotApply;
  } else if (Ty->isBlockPointerType()) {
    CallType = VariadicBlock;
  } else { // Ty->isFunctionPointerType()
    CallType = VariadicFunction;
  }

  checkCall(NDecl, Proto,
            llvm::makeArrayRef(TheCall->getArgs(), TheCall->getNumArgs()),
            /*IsMemberFunction=*/false, TheCall->getRParenLoc(),
            TheCall->getCallee()->getSourceRange(), CallType);
#endif // HLSL Change Ends
  return false;
}

/// Checks function calls when a FunctionDecl or a NamedDecl is not available,
/// such as function pointers returned from functions.
bool Sema::CheckOtherCall(CallExpr *TheCall, const FunctionProtoType *Proto) {
  VariadicCallType CallType = getVariadicCallType(/*FDecl=*/nullptr, Proto,
                                                  TheCall->getCallee());
  checkCall(/*FDecl=*/nullptr, Proto,
            llvm::makeArrayRef(TheCall->getArgs(), TheCall->getNumArgs()),
            /*IsMemberFunction=*/false, TheCall->getRParenLoc(),
            TheCall->getCallee()->getSourceRange(), CallType);

  return false;
}

static bool isValidOrderingForOp(int64_t Ordering, AtomicExpr::AtomicOp Op) {
  if (Ordering < AtomicExpr::AO_ABI_memory_order_relaxed ||
      Ordering > AtomicExpr::AO_ABI_memory_order_seq_cst)
    return false;

  switch (Op) {
  case AtomicExpr::AO__c11_atomic_init:
    llvm_unreachable("There is no ordering argument for an init");

  case AtomicExpr::AO__c11_atomic_load:
  case AtomicExpr::AO__atomic_load_n:
  case AtomicExpr::AO__atomic_load:
    return Ordering != AtomicExpr::AO_ABI_memory_order_release &&
           Ordering != AtomicExpr::AO_ABI_memory_order_acq_rel;

  case AtomicExpr::AO__c11_atomic_store:
  case AtomicExpr::AO__atomic_store:
  case AtomicExpr::AO__atomic_store_n:
    return Ordering != AtomicExpr::AO_ABI_memory_order_consume &&
           Ordering != AtomicExpr::AO_ABI_memory_order_acquire &&
           Ordering != AtomicExpr::AO_ABI_memory_order_acq_rel;

  default:
    return true;
  }
}

ExprResult Sema::SemaAtomicOpsOverloaded(ExprResult TheCallResult,
                                         AtomicExpr::AtomicOp Op) {
  CallExpr *TheCall = cast<CallExpr>(TheCallResult.get());
  DeclRefExpr *DRE =cast<DeclRefExpr>(TheCall->getCallee()->IgnoreParenCasts());

  // All these operations take one of the following forms:
  enum {
    // C    __c11_atomic_init(A *, C)
    Init,
    // C    __c11_atomic_load(A *, int)
    Load,
    // void __atomic_load(A *, CP, int)
    Copy,
    // C    __c11_atomic_add(A *, M, int)
    Arithmetic,
    // C    __atomic_exchange_n(A *, CP, int)
    Xchg,
    // void __atomic_exchange(A *, C *, CP, int)
    GNUXchg,
    // bool __c11_atomic_compare_exchange_strong(A *, C *, CP, int, int)
    C11CmpXchg,
    // bool __atomic_compare_exchange(A *, C *, CP, bool, int, int)
    GNUCmpXchg
  } Form = Init;
  const unsigned NumArgs[] = { 2, 2, 3, 3, 3, 4, 5, 6 };
  const unsigned NumVals[] = { 1, 0, 1, 1, 1, 2, 2, 3 };
  // where:
  //   C is an appropriate type,
  //   A is volatile _Atomic(C) for __c11 builtins and is C for GNU builtins,
  //   CP is C for __c11 builtins and GNU _n builtins and is C * otherwise,
  //   M is C if C is an integer, and ptrdiff_t if C is a pointer, and
  //   the int parameters are for orderings.

  static_assert(AtomicExpr::AO__c11_atomic_init == 0 &&
                    AtomicExpr::AO__c11_atomic_fetch_xor + 1 ==
                        AtomicExpr::AO__atomic_load,
                "need to update code for modified C11 atomics");
  bool IsC11 = Op >= AtomicExpr::AO__c11_atomic_init &&
               Op <= AtomicExpr::AO__c11_atomic_fetch_xor;
  bool IsN = Op == AtomicExpr::AO__atomic_load_n ||
             Op == AtomicExpr::AO__atomic_store_n ||
             Op == AtomicExpr::AO__atomic_exchange_n ||
             Op == AtomicExpr::AO__atomic_compare_exchange_n;
  bool IsAddSub = false;

  switch (Op) {
  case AtomicExpr::AO__c11_atomic_init:
    Form = Init;
    break;

  case AtomicExpr::AO__c11_atomic_load:
  case AtomicExpr::AO__atomic_load_n:
    Form = Load;
    break;

  case AtomicExpr::AO__c11_atomic_store:
  case AtomicExpr::AO__atomic_load:
  case AtomicExpr::AO__atomic_store:
  case AtomicExpr::AO__atomic_store_n:
    Form = Copy;
    break;

  case AtomicExpr::AO__c11_atomic_fetch_add:
  case AtomicExpr::AO__c11_atomic_fetch_sub:
  case AtomicExpr::AO__atomic_fetch_add:
  case AtomicExpr::AO__atomic_fetch_sub:
  case AtomicExpr::AO__atomic_add_fetch:
  case AtomicExpr::AO__atomic_sub_fetch:
    IsAddSub = true;
    LLVM_FALLTHROUGH; // HLSL Change
  case AtomicExpr::AO__c11_atomic_fetch_and:
  case AtomicExpr::AO__c11_atomic_fetch_or:
  case AtomicExpr::AO__c11_atomic_fetch_xor:
  case AtomicExpr::AO__atomic_fetch_and:
  case AtomicExpr::AO__atomic_fetch_or:
  case AtomicExpr::AO__atomic_fetch_xor:
  case AtomicExpr::AO__atomic_fetch_nand:
  case AtomicExpr::AO__atomic_and_fetch:
  case AtomicExpr::AO__atomic_or_fetch:
  case AtomicExpr::AO__atomic_xor_fetch:
  case AtomicExpr::AO__atomic_nand_fetch:
    Form = Arithmetic;
    break;

  case AtomicExpr::AO__c11_atomic_exchange:
  case AtomicExpr::AO__atomic_exchange_n:
    Form = Xchg;
    break;

  case AtomicExpr::AO__atomic_exchange:
    Form = GNUXchg;
    break;

  case AtomicExpr::AO__c11_atomic_compare_exchange_strong:
  case AtomicExpr::AO__c11_atomic_compare_exchange_weak:
    Form = C11CmpXchg;
    break;

  case AtomicExpr::AO__atomic_compare_exchange:
  case AtomicExpr::AO__atomic_compare_exchange_n:
    Form = GNUCmpXchg;
    break;
  }

  // Check we have the right number of arguments.
  if (TheCall->getNumArgs() < NumArgs[Form]) {
    Diag(TheCall->getLocEnd(), diag::err_typecheck_call_too_few_args)
      << 0 << NumArgs[Form] << TheCall->getNumArgs()
      << TheCall->getCallee()->getSourceRange();
    return ExprError();
  } else if (TheCall->getNumArgs() > NumArgs[Form]) {
    Diag(TheCall->getArg(NumArgs[Form])->getLocStart(),
         diag::err_typecheck_call_too_many_args)
      << 0 << NumArgs[Form] << TheCall->getNumArgs()
      << TheCall->getCallee()->getSourceRange();
    return ExprError();
  }

  // Inspect the first argument of the atomic operation.
  Expr *Ptr = TheCall->getArg(0);
  Ptr = DefaultFunctionArrayLvalueConversion(Ptr).get();
  const PointerType *pointerType = Ptr->getType()->getAs<PointerType>();
  if (!pointerType) {
    Diag(DRE->getLocStart(), diag::err_atomic_builtin_must_be_pointer)
      << Ptr->getType() << Ptr->getSourceRange();
    return ExprError();
  }

  // For a __c11 builtin, this should be a pointer to an _Atomic type.
  QualType AtomTy = pointerType->getPointeeType(); // 'A'
  QualType ValType = AtomTy; // 'C'
  if (IsC11) {
    if (!AtomTy->isAtomicType()) {
      Diag(DRE->getLocStart(), diag::err_atomic_op_needs_atomic)
        << Ptr->getType() << Ptr->getSourceRange();
      return ExprError();
    }
    if (AtomTy.isConstQualified()) {
      Diag(DRE->getLocStart(), diag::err_atomic_op_needs_non_const_atomic)
        << Ptr->getType() << Ptr->getSourceRange();
      return ExprError();
    }
    ValType = AtomTy->getAs<AtomicType>()->getValueType();
  }

  // For an arithmetic operation, the implied arithmetic must be well-formed.
  if (Form == Arithmetic) {
    // gcc does not enforce these rules for GNU atomics, but we do so for sanity.
    if (IsAddSub && !ValType->isIntegerType() && !ValType->isPointerType()) {
      Diag(DRE->getLocStart(), diag::err_atomic_op_needs_atomic_int_or_ptr)
        << IsC11 << Ptr->getType() << Ptr->getSourceRange();
      return ExprError();
    }
    if (!IsAddSub && !ValType->isIntegerType()) {
      Diag(DRE->getLocStart(), diag::err_atomic_op_bitwise_needs_atomic_int)
        << IsC11 << Ptr->getType() << Ptr->getSourceRange();
      return ExprError();
    }
    if (IsC11 && ValType->isPointerType() &&
        RequireCompleteType(Ptr->getLocStart(), ValType->getPointeeType(),
                            diag::err_incomplete_type)) {
      return ExprError();
    }
  } else if (IsN && !ValType->isIntegerType() && !ValType->isPointerType()) {
    // For __atomic_*_n operations, the value type must be a scalar integral or
    // pointer type which is 1, 2, 4, 8 or 16 bytes in length.
    Diag(DRE->getLocStart(), diag::err_atomic_op_needs_atomic_int_or_ptr)
      << IsC11 << Ptr->getType() << Ptr->getSourceRange();
    return ExprError();
  }

  if (!IsC11 && !AtomTy.isTriviallyCopyableType(Context) &&
      !AtomTy->isScalarType()) {
    // For GNU atomics, require a trivially-copyable type. This is not part of
    // the GNU atomics specification, but we enforce it for sanity.
    Diag(DRE->getLocStart(), diag::err_atomic_op_needs_trivial_copy)
      << Ptr->getType() << Ptr->getSourceRange();
    return ExprError();
  }

  // FIXME: For any builtin other than a load, the ValType must not be
  // const-qualified.

  switch (ValType.getObjCLifetime()) {
  case Qualifiers::OCL_None:
  case Qualifiers::OCL_ExplicitNone:
    // okay
    break;

  case Qualifiers::OCL_Weak:
  case Qualifiers::OCL_Strong:
  case Qualifiers::OCL_Autoreleasing:
    // FIXME: Can this happen? By this point, ValType should be known
    // to be trivially copyable.
    Diag(DRE->getLocStart(), diag::err_arc_atomic_ownership)
      << ValType << Ptr->getSourceRange();
    return ExprError();
  }

  // atomic_fetch_or takes a pointer to a volatile 'A'.  We shouldn't let the
  // volatile-ness of the pointee-type inject itself into the result or the
  // other operands.
  ValType.removeLocalVolatile();
  QualType ResultType = ValType;
  if (Form == Copy || Form == GNUXchg || Form == Init)
    ResultType = Context.VoidTy;
  else if (Form == C11CmpXchg || Form == GNUCmpXchg)
    ResultType = Context.BoolTy;

  // The type of a parameter passed 'by value'. In the GNU atomics, such
  // arguments are actually passed as pointers.
  QualType ByValType = ValType; // 'CP'
  if (!IsC11 && !IsN)
    ByValType = Ptr->getType();

  // The first argument --- the pointer --- has a fixed type; we
  // deduce the types of the rest of the arguments accordingly.  Walk
  // the remaining arguments, converting them to the deduced value type.
  for (unsigned i = 1; i != NumArgs[Form]; ++i) {
    QualType Ty;
    if (i < NumVals[Form] + 1) {
      switch (i) {
      case 1:
        // The second argument is the non-atomic operand. For arithmetic, this
        // is always passed by value, and for a compare_exchange it is always
        // passed by address. For the rest, GNU uses by-address and C11 uses
        // by-value.
        assert(Form != Load);
        if (Form == Init || (Form == Arithmetic && ValType->isIntegerType()))
          Ty = ValType;
        else if (Form == Copy || Form == Xchg)
          Ty = ByValType;
        else if (Form == Arithmetic)
          Ty = Context.getPointerDiffType();
        else
          Ty = Context.getPointerType(ValType.getUnqualifiedType());
        break;
      case 2:
        // The third argument to compare_exchange / GNU exchange is a
        // (pointer to a) desired value.
        Ty = ByValType;
        break;
      case 3:
        // The fourth argument to GNU compare_exchange is a 'weak' flag.
        Ty = Context.BoolTy;
        break;
      }
    } else {
      // The order(s) are always converted to int.
      Ty = Context.IntTy;
    }

    InitializedEntity Entity =
        InitializedEntity::InitializeParameter(Context, Ty, false);
    ExprResult Arg = TheCall->getArg(i);
    Arg = PerformCopyInitialization(Entity, SourceLocation(), Arg);
    if (Arg.isInvalid())
      return true;
    TheCall->setArg(i, Arg.get());
  }

  // Permute the arguments into a 'consistent' order.
  SmallVector<Expr*, 5> SubExprs;
  SubExprs.push_back(Ptr);
  switch (Form) {
  case Init:
    // Note, AtomicExpr::getVal1() has a special case for this atomic.
    SubExprs.push_back(TheCall->getArg(1)); // Val1
    break;
  case Load:
    SubExprs.push_back(TheCall->getArg(1)); // Order
    break;
  case Copy:
  case Arithmetic:
  case Xchg:
    SubExprs.push_back(TheCall->getArg(2)); // Order
    SubExprs.push_back(TheCall->getArg(1)); // Val1
    break;
  case GNUXchg:
    // Note, AtomicExpr::getVal2() has a special case for this atomic.
    SubExprs.push_back(TheCall->getArg(3)); // Order
    SubExprs.push_back(TheCall->getArg(1)); // Val1
    SubExprs.push_back(TheCall->getArg(2)); // Val2
    break;
  case C11CmpXchg:
    SubExprs.push_back(TheCall->getArg(3)); // Order
    SubExprs.push_back(TheCall->getArg(1)); // Val1
    SubExprs.push_back(TheCall->getArg(4)); // OrderFail
    SubExprs.push_back(TheCall->getArg(2)); // Val2
    break;
  case GNUCmpXchg:
    SubExprs.push_back(TheCall->getArg(4)); // Order
    SubExprs.push_back(TheCall->getArg(1)); // Val1
    SubExprs.push_back(TheCall->getArg(5)); // OrderFail
    SubExprs.push_back(TheCall->getArg(2)); // Val2
    SubExprs.push_back(TheCall->getArg(3)); // Weak
    break;
  }

  if (SubExprs.size() >= 2 && Form != Init) {
    llvm::APSInt Result(32);
    if (SubExprs[1]->isIntegerConstantExpr(Result, Context) &&
        !isValidOrderingForOp(Result.getSExtValue(), Op))
      Diag(SubExprs[1]->getLocStart(),
           diag::warn_atomic_op_has_invalid_memory_order)
          << SubExprs[1]->getSourceRange();
  }

  AtomicExpr *AE = new (Context) AtomicExpr(TheCall->getCallee()->getLocStart(),
                                            SubExprs, ResultType, Op,
                                            TheCall->getRParenLoc());
  
  if ((Op == AtomicExpr::AO__c11_atomic_load ||
       (Op == AtomicExpr::AO__c11_atomic_store)) &&
      Context.AtomicUsesUnsupportedLibcall(AE))
    Diag(AE->getLocStart(), diag::err_atomic_load_store_uses_lib) <<
    ((Op == AtomicExpr::AO__c11_atomic_load) ? 0 : 1);

  return AE;
}


/// checkBuiltinArgument - Given a call to a builtin function, perform
/// normal type-checking on the given argument, updating the call in
/// place.  This is useful when a builtin function requires custom
/// type-checking for some of its arguments but not necessarily all of
/// them.
///
/// Returns true on error.
static bool checkBuiltinArgument(Sema &S, CallExpr *E, unsigned ArgIndex) {
  FunctionDecl *Fn = E->getDirectCallee();
  assert(Fn && "builtin call without direct callee!");

  ParmVarDecl *Param = Fn->getParamDecl(ArgIndex);
  InitializedEntity Entity =
    InitializedEntity::InitializeParameter(S.Context, Param);

  ExprResult Arg = E->getArg(0);
  Arg = S.PerformCopyInitialization(Entity, SourceLocation(), Arg);
  if (Arg.isInvalid())
    return true;

  E->setArg(ArgIndex, Arg.get());
  return false;
}

/// SemaBuiltinAtomicOverloaded - We have a call to a function like
/// __sync_fetch_and_add, which is an overloaded function based on the pointer
/// type of its first argument.  The main ActOnCallExpr routines have already
/// promoted the types of arguments because all of these calls are prototyped as
/// void(...).
///
/// This function goes through and does final semantic checking for these
/// builtins,
ExprResult
Sema::SemaBuiltinAtomicOverloaded(ExprResult TheCallResult) {
  CallExpr *TheCall = (CallExpr *)TheCallResult.get();
  DeclRefExpr *DRE =cast<DeclRefExpr>(TheCall->getCallee()->IgnoreParenCasts());
  FunctionDecl *FDecl = cast<FunctionDecl>(DRE->getDecl());

  // Ensure that we have at least one argument to do type inference from.
  if (TheCall->getNumArgs() < 1) {
    Diag(TheCall->getLocEnd(), diag::err_typecheck_call_too_few_args_at_least)
      << 0 << 1 << TheCall->getNumArgs()
      << TheCall->getCallee()->getSourceRange();
    return ExprError();
  }

  // Inspect the first argument of the atomic builtin.  This should always be
  // a pointer type, whose element is an integral scalar or pointer type.
  // Because it is a pointer type, we don't have to worry about any implicit
  // casts here.
  // FIXME: We don't allow floating point scalars as input.
  Expr *FirstArg = TheCall->getArg(0);
  ExprResult FirstArgResult = DefaultFunctionArrayLvalueConversion(FirstArg);
  if (FirstArgResult.isInvalid())
    return ExprError();
  FirstArg = FirstArgResult.get();
  TheCall->setArg(0, FirstArg);

  const PointerType *pointerType = FirstArg->getType()->getAs<PointerType>();
  if (!pointerType) {
    Diag(DRE->getLocStart(), diag::err_atomic_builtin_must_be_pointer)
      << FirstArg->getType() << FirstArg->getSourceRange();
    return ExprError();
  }

  QualType ValType = pointerType->getPointeeType();
  if (!ValType->isIntegerType() && !ValType->isAnyPointerType() &&
      !ValType->isBlockPointerType()) {
    Diag(DRE->getLocStart(), diag::err_atomic_builtin_must_be_pointer_intptr)
      << FirstArg->getType() << FirstArg->getSourceRange();
    return ExprError();
  }

  switch (ValType.getObjCLifetime()) {
  case Qualifiers::OCL_None:
  case Qualifiers::OCL_ExplicitNone:
    // okay
    break;

  case Qualifiers::OCL_Weak:
  case Qualifiers::OCL_Strong:
  case Qualifiers::OCL_Autoreleasing:
    Diag(DRE->getLocStart(), diag::err_arc_atomic_ownership)
      << ValType << FirstArg->getSourceRange();
    return ExprError();
  }

  // Strip any qualifiers off ValType.
  ValType = ValType.getUnqualifiedType();

  // The majority of builtins return a value, but a few have special return
  // types, so allow them to override appropriately below.
  QualType ResultType = ValType;

  // We need to figure out which concrete builtin this maps onto.  For example,
  // __sync_fetch_and_add with a 2 byte object turns into
  // __sync_fetch_and_add_2.
#define BUILTIN_ROW(x) \
  { Builtin::BI##x##_1, Builtin::BI##x##_2, Builtin::BI##x##_4, \
    Builtin::BI##x##_8, Builtin::BI##x##_16 }

  static const unsigned BuiltinIndices[][5] = {
    BUILTIN_ROW(__sync_fetch_and_add),
    BUILTIN_ROW(__sync_fetch_and_sub),
    BUILTIN_ROW(__sync_fetch_and_or),
    BUILTIN_ROW(__sync_fetch_and_and),
    BUILTIN_ROW(__sync_fetch_and_xor),
    BUILTIN_ROW(__sync_fetch_and_nand),

    BUILTIN_ROW(__sync_add_and_fetch),
    BUILTIN_ROW(__sync_sub_and_fetch),
    BUILTIN_ROW(__sync_and_and_fetch),
    BUILTIN_ROW(__sync_or_and_fetch),
    BUILTIN_ROW(__sync_xor_and_fetch),
    BUILTIN_ROW(__sync_nand_and_fetch),

    BUILTIN_ROW(__sync_val_compare_and_swap),
    BUILTIN_ROW(__sync_bool_compare_and_swap),
    BUILTIN_ROW(__sync_lock_test_and_set),
    BUILTIN_ROW(__sync_lock_release),
    BUILTIN_ROW(__sync_swap)
  };
#undef BUILTIN_ROW

  // Determine the index of the size.
  unsigned SizeIndex;
  switch (Context.getTypeSizeInChars(ValType).getQuantity()) {
  case 1: SizeIndex = 0; break;
  case 2: SizeIndex = 1; break;
  case 4: SizeIndex = 2; break;
  case 8: SizeIndex = 3; break;
  case 16: SizeIndex = 4; break;
  default:
    Diag(DRE->getLocStart(), diag::err_atomic_builtin_pointer_size)
      << FirstArg->getType() << FirstArg->getSourceRange();
    return ExprError();
  }

  // Each of these builtins has one pointer argument, followed by some number of
  // values (0, 1 or 2) followed by a potentially empty varags list of stuff
  // that we ignore.  Find out which row of BuiltinIndices to read from as well
  // as the number of fixed args.
  unsigned BuiltinID = FDecl->getBuiltinID();
  unsigned BuiltinIndex, NumFixed = 1;
  bool WarnAboutSemanticsChange = false;
  switch (BuiltinID) {
  default: llvm_unreachable("Unknown overloaded atomic builtin!");
  case Builtin::BI__sync_fetch_and_add: 
  case Builtin::BI__sync_fetch_and_add_1:
  case Builtin::BI__sync_fetch_and_add_2:
  case Builtin::BI__sync_fetch_and_add_4:
  case Builtin::BI__sync_fetch_and_add_8:
  case Builtin::BI__sync_fetch_and_add_16:
    BuiltinIndex = 0; 
    break;
      
  case Builtin::BI__sync_fetch_and_sub: 
  case Builtin::BI__sync_fetch_and_sub_1:
  case Builtin::BI__sync_fetch_and_sub_2:
  case Builtin::BI__sync_fetch_and_sub_4:
  case Builtin::BI__sync_fetch_and_sub_8:
  case Builtin::BI__sync_fetch_and_sub_16:
    BuiltinIndex = 1; 
    break;
      
  case Builtin::BI__sync_fetch_and_or:  
  case Builtin::BI__sync_fetch_and_or_1:
  case Builtin::BI__sync_fetch_and_or_2:
  case Builtin::BI__sync_fetch_and_or_4:
  case Builtin::BI__sync_fetch_and_or_8:
  case Builtin::BI__sync_fetch_and_or_16:
    BuiltinIndex = 2; 
    break;
      
  case Builtin::BI__sync_fetch_and_and: 
  case Builtin::BI__sync_fetch_and_and_1:
  case Builtin::BI__sync_fetch_and_and_2:
  case Builtin::BI__sync_fetch_and_and_4:
  case Builtin::BI__sync_fetch_and_and_8:
  case Builtin::BI__sync_fetch_and_and_16:
    BuiltinIndex = 3; 
    break;

  case Builtin::BI__sync_fetch_and_xor: 
  case Builtin::BI__sync_fetch_and_xor_1:
  case Builtin::BI__sync_fetch_and_xor_2:
  case Builtin::BI__sync_fetch_and_xor_4:
  case Builtin::BI__sync_fetch_and_xor_8:
  case Builtin::BI__sync_fetch_and_xor_16:
    BuiltinIndex = 4; 
    break;

  case Builtin::BI__sync_fetch_and_nand: 
  case Builtin::BI__sync_fetch_and_nand_1:
  case Builtin::BI__sync_fetch_and_nand_2:
  case Builtin::BI__sync_fetch_and_nand_4:
  case Builtin::BI__sync_fetch_and_nand_8:
  case Builtin::BI__sync_fetch_and_nand_16:
    BuiltinIndex = 5;
    WarnAboutSemanticsChange = true;
    break;

  case Builtin::BI__sync_add_and_fetch: 
  case Builtin::BI__sync_add_and_fetch_1:
  case Builtin::BI__sync_add_and_fetch_2:
  case Builtin::BI__sync_add_and_fetch_4:
  case Builtin::BI__sync_add_and_fetch_8:
  case Builtin::BI__sync_add_and_fetch_16:
    BuiltinIndex = 6; 
    break;
      
  case Builtin::BI__sync_sub_and_fetch: 
  case Builtin::BI__sync_sub_and_fetch_1:
  case Builtin::BI__sync_sub_and_fetch_2:
  case Builtin::BI__sync_sub_and_fetch_4:
  case Builtin::BI__sync_sub_and_fetch_8:
  case Builtin::BI__sync_sub_and_fetch_16:
    BuiltinIndex = 7; 
    break;
      
  case Builtin::BI__sync_and_and_fetch: 
  case Builtin::BI__sync_and_and_fetch_1:
  case Builtin::BI__sync_and_and_fetch_2:
  case Builtin::BI__sync_and_and_fetch_4:
  case Builtin::BI__sync_and_and_fetch_8:
  case Builtin::BI__sync_and_and_fetch_16:
    BuiltinIndex = 8; 
    break;
      
  case Builtin::BI__sync_or_and_fetch:  
  case Builtin::BI__sync_or_and_fetch_1:
  case Builtin::BI__sync_or_and_fetch_2:
  case Builtin::BI__sync_or_and_fetch_4:
  case Builtin::BI__sync_or_and_fetch_8:
  case Builtin::BI__sync_or_and_fetch_16:
    BuiltinIndex = 9; 
    break;
      
  case Builtin::BI__sync_xor_and_fetch: 
  case Builtin::BI__sync_xor_and_fetch_1:
  case Builtin::BI__sync_xor_and_fetch_2:
  case Builtin::BI__sync_xor_and_fetch_4:
  case Builtin::BI__sync_xor_and_fetch_8:
  case Builtin::BI__sync_xor_and_fetch_16:
    BuiltinIndex = 10;
    break;

  case Builtin::BI__sync_nand_and_fetch: 
  case Builtin::BI__sync_nand_and_fetch_1:
  case Builtin::BI__sync_nand_and_fetch_2:
  case Builtin::BI__sync_nand_and_fetch_4:
  case Builtin::BI__sync_nand_and_fetch_8:
  case Builtin::BI__sync_nand_and_fetch_16:
    BuiltinIndex = 11;
    WarnAboutSemanticsChange = true;
    break;

  case Builtin::BI__sync_val_compare_and_swap:
  case Builtin::BI__sync_val_compare_and_swap_1:
  case Builtin::BI__sync_val_compare_and_swap_2:
  case Builtin::BI__sync_val_compare_and_swap_4:
  case Builtin::BI__sync_val_compare_and_swap_8:
  case Builtin::BI__sync_val_compare_and_swap_16:
    BuiltinIndex = 12;
    NumFixed = 2;
    break;
      
  case Builtin::BI__sync_bool_compare_and_swap:
  case Builtin::BI__sync_bool_compare_and_swap_1:
  case Builtin::BI__sync_bool_compare_and_swap_2:
  case Builtin::BI__sync_bool_compare_and_swap_4:
  case Builtin::BI__sync_bool_compare_and_swap_8:
  case Builtin::BI__sync_bool_compare_and_swap_16:
    BuiltinIndex = 13;
    NumFixed = 2;
    ResultType = Context.BoolTy;
    break;
      
  case Builtin::BI__sync_lock_test_and_set: 
  case Builtin::BI__sync_lock_test_and_set_1:
  case Builtin::BI__sync_lock_test_and_set_2:
  case Builtin::BI__sync_lock_test_and_set_4:
  case Builtin::BI__sync_lock_test_and_set_8:
  case Builtin::BI__sync_lock_test_and_set_16:
    BuiltinIndex = 14; 
    break;
      
  case Builtin::BI__sync_lock_release:
  case Builtin::BI__sync_lock_release_1:
  case Builtin::BI__sync_lock_release_2:
  case Builtin::BI__sync_lock_release_4:
  case Builtin::BI__sync_lock_release_8:
  case Builtin::BI__sync_lock_release_16:
    BuiltinIndex = 15;
    NumFixed = 0;
    ResultType = Context.VoidTy;
    break;
      
  case Builtin::BI__sync_swap: 
  case Builtin::BI__sync_swap_1:
  case Builtin::BI__sync_swap_2:
  case Builtin::BI__sync_swap_4:
  case Builtin::BI__sync_swap_8:
  case Builtin::BI__sync_swap_16:
    BuiltinIndex = 16; 
    break;
  }

  // Now that we know how many fixed arguments we expect, first check that we
  // have at least that many.
  if (TheCall->getNumArgs() < 1+NumFixed) {
    Diag(TheCall->getLocEnd(), diag::err_typecheck_call_too_few_args_at_least)
      << 0 << 1+NumFixed << TheCall->getNumArgs()
      << TheCall->getCallee()->getSourceRange();
    return ExprError();
  }

  if (WarnAboutSemanticsChange) {
    Diag(TheCall->getLocEnd(), diag::warn_sync_fetch_and_nand_semantics_change)
      << TheCall->getCallee()->getSourceRange();
  }

  // Get the decl for the concrete builtin from this, we can tell what the
  // concrete integer type we should convert to is.
  unsigned NewBuiltinID = BuiltinIndices[BuiltinIndex][SizeIndex];
  const char *NewBuiltinName = Context.BuiltinInfo.GetName(NewBuiltinID);
  FunctionDecl *NewBuiltinDecl;
  if (NewBuiltinID == BuiltinID)
    NewBuiltinDecl = FDecl;
  else {
    // Perform builtin lookup to avoid redeclaring it.
    DeclarationName DN(&Context.Idents.get(NewBuiltinName));
    LookupResult Res(*this, DN, DRE->getLocStart(), LookupOrdinaryName);
    LookupName(Res, TUScope, /*AllowBuiltinCreation=*/true);
    assert(Res.getFoundDecl());
    NewBuiltinDecl = dyn_cast<FunctionDecl>(Res.getFoundDecl());
    if (!NewBuiltinDecl)
      return ExprError();
  }

  // The first argument --- the pointer --- has a fixed type; we
  // deduce the types of the rest of the arguments accordingly.  Walk
  // the remaining arguments, converting them to the deduced value type.
  for (unsigned i = 0; i != NumFixed; ++i) {
    ExprResult Arg = TheCall->getArg(i+1);

    // GCC does an implicit conversion to the pointer or integer ValType.  This
    // can fail in some cases (1i -> int**), check for this error case now.
    // Initialize the argument.
    InitializedEntity Entity = InitializedEntity::InitializeParameter(Context,
                                                   ValType, /*consume*/ false);
    Arg = PerformCopyInitialization(Entity, SourceLocation(), Arg);
    if (Arg.isInvalid())
      return ExprError();

    // Okay, we have something that *can* be converted to the right type.  Check
    // to see if there is a potentially weird extension going on here.  This can
    // happen when you do an atomic operation on something like an char* and
    // pass in 42.  The 42 gets converted to char.  This is even more strange
    // for things like 45.123 -> char, etc.
    // FIXME: Do this check.
    TheCall->setArg(i+1, Arg.get());
  }

  ASTContext& Context = this->getASTContext();

  // Create a new DeclRefExpr to refer to the new decl.
  DeclRefExpr* NewDRE = DeclRefExpr::Create(
      Context,
      DRE->getQualifierLoc(),
      SourceLocation(),
      NewBuiltinDecl,
      /*enclosing*/ false,
      DRE->getLocation(),
      Context.BuiltinFnTy,
      DRE->getValueKind());

  // Set the callee in the CallExpr.
  // FIXME: This loses syntactic information.
  QualType CalleePtrTy = Context.getPointerType(NewBuiltinDecl->getType());
  ExprResult PromotedCall = ImpCastExprToType(NewDRE, CalleePtrTy,
                                              CK_BuiltinFnToFnPtr);
  TheCall->setCallee(PromotedCall.get());

  // Change the result type of the call to match the original value type. This
  // is arbitrary, but the codegen for these builtins ins design to handle it
  // gracefully.
  TheCall->setType(ResultType);

  return TheCallResult;
}

/// CheckObjCString - Checks that the argument to the builtin
/// CFString constructor is correct
/// Note: It might also make sense to do the UTF-16 conversion here (would
/// simplify the backend).
bool Sema::CheckObjCString(Expr *Arg) {
#if 0 // HLSL Change Starts
  Arg = Arg->IgnoreParenCasts();
  StringLiteral *Literal = dyn_cast<StringLiteral>(Arg);

  if (!Literal || !Literal->isAscii()) {
    Diag(Arg->getLocStart(), diag::err_cfstring_literal_not_string_constant)
      << Arg->getSourceRange();
    return true;
  }

  if (Literal->containsNonAsciiOrNull()) {
    StringRef String = Literal->getString();
    unsigned NumBytes = String.size();
    SmallVector<UTF16, 128> ToBuf(NumBytes);
    const UTF8 *FromPtr = (const UTF8 *)String.data();
    UTF16 *ToPtr = &ToBuf[0];
    
    ConversionResult Result = ConvertUTF8toUTF16(&FromPtr, FromPtr + NumBytes,
                                                 &ToPtr, ToPtr + NumBytes,
                                                 strictConversion);
    // Check for conversion failure.
    if (Result != conversionOK)
      Diag(Arg->getLocStart(),
           diag::warn_cfstring_truncated) << Arg->getSourceRange();
  }
#endif // HLSL Change Ends
  return false;
}

/// SemaBuiltinVAStart - Check the arguments to __builtin_va_start for validity.
/// Emit an error and return true on failure, return false on success.
bool Sema::SemaBuiltinVAStart(CallExpr *TheCall) {
  Expr *Fn = TheCall->getCallee();
  if (TheCall->getNumArgs() > 2) {
    Diag(TheCall->getArg(2)->getLocStart(),
         diag::err_typecheck_call_too_many_args)
      << 0 /*function call*/ << 2 << TheCall->getNumArgs()
      << Fn->getSourceRange()
      << SourceRange(TheCall->getArg(2)->getLocStart(),
                     (*(TheCall->arg_end()-1))->getLocEnd());
    return true;
  }

  if (TheCall->getNumArgs() < 2) {
    return Diag(TheCall->getLocEnd(),
      diag::err_typecheck_call_too_few_args_at_least)
      << 0 /*function call*/ << 2 << TheCall->getNumArgs();
  }

  // Type-check the first argument normally.
  if (checkBuiltinArgument(*this, TheCall, 0))
    return true;

  // Determine whether the current function is variadic or not.
  BlockScopeInfo *CurBlock = getCurBlock();
  bool isVariadic;
  if (CurBlock)
    isVariadic = CurBlock->TheDecl->isVariadic();
  else if (FunctionDecl *FD = getCurFunctionDecl())
    isVariadic = FD->isVariadic();
  else
    isVariadic = getCurMethodDecl()->isVariadic();

  if (!isVariadic) {
    Diag(Fn->getLocStart(), diag::err_va_start_used_in_non_variadic_function);
    return true;
  }

  // Verify that the second argument to the builtin is the last argument of the
  // current function or method.
  bool SecondArgIsLastNamedArgument = false;
  const Expr *Arg = TheCall->getArg(1)->IgnoreParenCasts();

  // These are valid if SecondArgIsLastNamedArgument is false after the next
  // block.
  QualType Type;
  SourceLocation ParamLoc;

  if (const DeclRefExpr *DR = dyn_cast<DeclRefExpr>(Arg)) {
    if (const ParmVarDecl *PV = dyn_cast<ParmVarDecl>(DR->getDecl())) {
      // FIXME: This isn't correct for methods (results in bogus warning).
      // Get the last formal in the current function.
      const ParmVarDecl *LastArg;
      if (CurBlock)
        LastArg = *(CurBlock->TheDecl->param_end()-1);
      else if (FunctionDecl *FD = getCurFunctionDecl())
        LastArg = *(FD->param_end()-1);
      else
        LastArg = *(getCurMethodDecl()->param_end()-1);
      SecondArgIsLastNamedArgument = PV == LastArg;

      Type = PV->getType();
      ParamLoc = PV->getLocation();
    }
  }

  if (!SecondArgIsLastNamedArgument)
    Diag(TheCall->getArg(1)->getLocStart(),
         diag::warn_second_parameter_of_va_start_not_last_named_argument);
  else if (Type->isReferenceType()) {
    Diag(Arg->getLocStart(),
         diag::warn_va_start_of_reference_type_is_undefined);
    Diag(ParamLoc, diag::note_parameter_type) << Type;
  }

  TheCall->setType(Context.VoidTy);
  return false;
}

bool Sema::SemaBuiltinVAStartARM(CallExpr *Call) {
  // void __va_start(va_list *ap, const char *named_addr, size_t slot_size,
  //                 const char *named_addr);

  Expr *Func = Call->getCallee();

  if (Call->getNumArgs() < 3)
    return Diag(Call->getLocEnd(),
                diag::err_typecheck_call_too_few_args_at_least)
           << 0 /*function call*/ << 3 << Call->getNumArgs();

  // Determine whether the current function is variadic or not.
  bool IsVariadic;
  if (BlockScopeInfo *CurBlock = getCurBlock())
    IsVariadic = CurBlock->TheDecl->isVariadic();
  else if (FunctionDecl *FD = getCurFunctionDecl())
    IsVariadic = FD->isVariadic();
  else if (ObjCMethodDecl *MD = getCurMethodDecl())
    IsVariadic = MD->isVariadic();
  else
    llvm_unreachable("unexpected statement type");

  if (!IsVariadic) {
    Diag(Func->getLocStart(), diag::err_va_start_used_in_non_variadic_function);
    return true;
  }

  // Type-check the first argument normally.
  if (checkBuiltinArgument(*this, Call, 0))
    return true;

  const struct {
    unsigned ArgNo;
    QualType Type;
  } ArgumentTypes[] = {
    { 1, Context.getPointerType(Context.CharTy.withConst()) },
    { 2, Context.getSizeType() },
  };

  for (const auto &AT : ArgumentTypes) {
    const Expr *Arg = Call->getArg(AT.ArgNo)->IgnoreParens();
    if (Arg->getType().getCanonicalType() == AT.Type.getCanonicalType())
      continue;
    Diag(Arg->getLocStart(), diag::err_typecheck_convert_incompatible)
      << Arg->getType() << AT.Type << 1 /* different class */
      << 0 /* qualifier difference */ << 3 /* parameter mismatch */
      << AT.ArgNo + 1 << Arg->getType() << AT.Type;
  }

  return false;
}

/// SemaBuiltinUnorderedCompare - Handle functions like __builtin_isgreater and
/// friends.  This is declared to take (...), so we have to check everything.
bool Sema::SemaBuiltinUnorderedCompare(CallExpr *TheCall) {
  if (TheCall->getNumArgs() < 2)
    return Diag(TheCall->getLocEnd(), diag::err_typecheck_call_too_few_args)
      << 0 << 2 << TheCall->getNumArgs()/*function call*/;
  if (TheCall->getNumArgs() > 2)
    return Diag(TheCall->getArg(2)->getLocStart(),
                diag::err_typecheck_call_too_many_args)
      << 0 /*function call*/ << 2 << TheCall->getNumArgs()
      << SourceRange(TheCall->getArg(2)->getLocStart(),
                     (*(TheCall->arg_end()-1))->getLocEnd());

  ExprResult OrigArg0 = TheCall->getArg(0);
  ExprResult OrigArg1 = TheCall->getArg(1);

  // Do standard promotions between the two arguments, returning their common
  // type.
  QualType Res = UsualArithmeticConversions(OrigArg0, OrigArg1, false);
  if (OrigArg0.isInvalid() || OrigArg1.isInvalid())
    return true;

  // Make sure any conversions are pushed back into the call; this is
  // type safe since unordered compare builtins are declared as "_Bool
  // foo(...)".
  TheCall->setArg(0, OrigArg0.get());
  TheCall->setArg(1, OrigArg1.get());

  if (OrigArg0.get()->isTypeDependent() || OrigArg1.get()->isTypeDependent())
    return false;

  // If the common type isn't a real floating type, then the arguments were
  // invalid for this operation.
  if (Res.isNull() || !Res->isRealFloatingType())
    return Diag(OrigArg0.get()->getLocStart(),
                diag::err_typecheck_call_invalid_ordered_compare)
      << OrigArg0.get()->getType() << OrigArg1.get()->getType()
      << SourceRange(OrigArg0.get()->getLocStart(), OrigArg1.get()->getLocEnd());

  return false;
}

/// SemaBuiltinSemaBuiltinFPClassification - Handle functions like
/// __builtin_isnan and friends.  This is declared to take (...), so we have
/// to check everything. We expect the last argument to be a floating point
/// value.
bool Sema::SemaBuiltinFPClassification(CallExpr *TheCall, unsigned NumArgs) {
  if (TheCall->getNumArgs() < NumArgs)
    return Diag(TheCall->getLocEnd(), diag::err_typecheck_call_too_few_args)
      << 0 << NumArgs << TheCall->getNumArgs()/*function call*/;
  if (TheCall->getNumArgs() > NumArgs)
    return Diag(TheCall->getArg(NumArgs)->getLocStart(),
                diag::err_typecheck_call_too_many_args)
      << 0 /*function call*/ << NumArgs << TheCall->getNumArgs()
      << SourceRange(TheCall->getArg(NumArgs)->getLocStart(),
                     (*(TheCall->arg_end()-1))->getLocEnd());

  Expr *OrigArg = TheCall->getArg(NumArgs-1);

  if (OrigArg->isTypeDependent())
    return false;

  // This operation requires a non-_Complex floating-point number.
  if (!OrigArg->getType()->isRealFloatingType())
    return Diag(OrigArg->getLocStart(),
                diag::err_typecheck_call_invalid_unary_fp)
      << OrigArg->getType() << OrigArg->getSourceRange();

  // If this is an implicit conversion from float -> double, remove it.
  if (ImplicitCastExpr *Cast = dyn_cast<ImplicitCastExpr>(OrigArg)) {
    Expr *CastArg = Cast->getSubExpr();
    if (CastArg->getType()->isSpecificBuiltinType(BuiltinType::Float)) {
      assert(Cast->getType()->isSpecificBuiltinType(BuiltinType::Double) &&
             "promotion from float to double is the only expected cast here");
      Cast->setSubExpr(nullptr);
      TheCall->setArg(NumArgs-1, CastArg);
    }
  }
  
  return false;
}

/// SemaBuiltinShuffleVector - Handle __builtin_shufflevector.
// This is declared to take (...), so we have to check everything.
ExprResult Sema::SemaBuiltinShuffleVector(CallExpr *TheCall) {
  if (TheCall->getNumArgs() < 2)
    return ExprError(Diag(TheCall->getLocEnd(),
                          diag::err_typecheck_call_too_few_args_at_least)
                     << 0 /*function call*/ << 2 << TheCall->getNumArgs()
                     << TheCall->getSourceRange());

  // Determine which of the following types of shufflevector we're checking:
  // 1) unary, vector mask: (lhs, mask)
  // 2) binary, vector mask: (lhs, rhs, mask)
  // 3) binary, scalar mask: (lhs, rhs, index, ..., index)
  QualType resType = TheCall->getArg(0)->getType();
  unsigned numElements = 0;

  if (!TheCall->getArg(0)->isTypeDependent() &&
      !TheCall->getArg(1)->isTypeDependent()) {
    QualType LHSType = TheCall->getArg(0)->getType();
    QualType RHSType = TheCall->getArg(1)->getType();

    if (!LHSType->isVectorType() || !RHSType->isVectorType())
      return ExprError(Diag(TheCall->getLocStart(),
                            diag::err_shufflevector_non_vector)
                       << SourceRange(TheCall->getArg(0)->getLocStart(),
                                      TheCall->getArg(1)->getLocEnd()));

    numElements = LHSType->getAs<VectorType>()->getNumElements();
    unsigned numResElements = TheCall->getNumArgs() - 2;

    // Check to see if we have a call with 2 vector arguments, the unary shuffle
    // with mask.  If so, verify that RHS is an integer vector type with the
    // same number of elts as lhs.
    if (TheCall->getNumArgs() == 2) {
      if (!RHSType->hasIntegerRepresentation() ||
          RHSType->getAs<VectorType>()->getNumElements() != numElements)
        return ExprError(Diag(TheCall->getLocStart(),
                              diag::err_shufflevector_incompatible_vector)
                         << SourceRange(TheCall->getArg(1)->getLocStart(),
                                        TheCall->getArg(1)->getLocEnd()));
    } else if (!Context.hasSameUnqualifiedType(LHSType, RHSType)) {
      return ExprError(Diag(TheCall->getLocStart(),
                            diag::err_shufflevector_incompatible_vector)
                       << SourceRange(TheCall->getArg(0)->getLocStart(),
                                      TheCall->getArg(1)->getLocEnd()));
    } else if (numElements != numResElements) {
      QualType eltType = LHSType->getAs<VectorType>()->getElementType();
      resType = Context.getVectorType(eltType, numResElements,
                                      VectorType::GenericVector);
    }
  }

  for (unsigned i = 2; i < TheCall->getNumArgs(); i++) {
    if (TheCall->getArg(i)->isTypeDependent() ||
        TheCall->getArg(i)->isValueDependent())
      continue;

    llvm::APSInt Result(32);
    if (!TheCall->getArg(i)->isIntegerConstantExpr(Result, Context))
      return ExprError(Diag(TheCall->getLocStart(),
                            diag::err_shufflevector_nonconstant_argument)
                       << TheCall->getArg(i)->getSourceRange());

    // Allow -1 which will be translated to undef in the IR.
    if (Result.isSigned() && Result.isAllOnesValue())
      continue;

    if (Result.getActiveBits() > 64 || Result.getZExtValue() >= numElements*2)
      return ExprError(Diag(TheCall->getLocStart(),
                            diag::err_shufflevector_argument_too_large)
                       << TheCall->getArg(i)->getSourceRange());
  }

  SmallVector<Expr*, 32> exprs;

  for (unsigned i = 0, e = TheCall->getNumArgs(); i != e; i++) {
    exprs.push_back(TheCall->getArg(i));
    TheCall->setArg(i, nullptr);
  }

  return new (Context) ShuffleVectorExpr(Context, exprs, resType,
                                         TheCall->getCallee()->getLocStart(),
                                         TheCall->getRParenLoc());
}

/// SemaConvertVectorExpr - Handle __builtin_convertvector
ExprResult Sema::SemaConvertVectorExpr(Expr *E, TypeSourceInfo *TInfo,
                                       SourceLocation BuiltinLoc,
                                       SourceLocation RParenLoc) {
  ExprValueKind VK = VK_RValue;
  ExprObjectKind OK = OK_Ordinary;
  QualType DstTy = TInfo->getType();
  QualType SrcTy = E->getType();

  if (!SrcTy->isVectorType() && !SrcTy->isDependentType())
    return ExprError(Diag(BuiltinLoc,
                          diag::err_convertvector_non_vector)
                     << E->getSourceRange());
  if (!DstTy->isVectorType() && !DstTy->isDependentType())
    return ExprError(Diag(BuiltinLoc,
                          diag::err_convertvector_non_vector_type));

  if (!SrcTy->isDependentType() && !DstTy->isDependentType()) {
    unsigned SrcElts = SrcTy->getAs<VectorType>()->getNumElements();
    unsigned DstElts = DstTy->getAs<VectorType>()->getNumElements();
    if (SrcElts != DstElts)
      return ExprError(Diag(BuiltinLoc,
                            diag::err_convertvector_incompatible_vector)
                       << E->getSourceRange());
  }

  return new (Context)
      ConvertVectorExpr(E, TInfo, DstTy, VK, OK, BuiltinLoc, RParenLoc);
}

/// SemaBuiltinPrefetch - Handle __builtin_prefetch.
// This is declared to take (const void*, ...) and can take two
// optional constant int args.
bool Sema::SemaBuiltinPrefetch(CallExpr *TheCall) {
  unsigned NumArgs = TheCall->getNumArgs();

  if (NumArgs > 3)
    return Diag(TheCall->getLocEnd(),
             diag::err_typecheck_call_too_many_args_at_most)
             << 0 /*function call*/ << 3 << NumArgs
             << TheCall->getSourceRange();

  // Argument 0 is checked for us and the remaining arguments must be
  // constant integers.
  for (unsigned i = 1; i != NumArgs; ++i)
    if (SemaBuiltinConstantArgRange(TheCall, i, 0, i == 1 ? 1 : 3))
      return true;

  return false;
}

/// SemaBuiltinAssume - Handle __assume (MS Extension).
// __assume does not evaluate its arguments, and should warn if its argument
// has side effects.
bool Sema::SemaBuiltinAssume(CallExpr *TheCall) {
  Expr *Arg = TheCall->getArg(0);
  if (Arg->isInstantiationDependent()) return false;

  if (Arg->HasSideEffects(Context))
    Diag(Arg->getLocStart(), diag::warn_assume_side_effects)
      << Arg->getSourceRange()
      << cast<FunctionDecl>(TheCall->getCalleeDecl())->getIdentifier();

  return false;
}

/// Handle __builtin_assume_aligned. This is declared
/// as (const void*, size_t, ...) and can take one optional constant int arg.
bool Sema::SemaBuiltinAssumeAligned(CallExpr *TheCall) {
  unsigned NumArgs = TheCall->getNumArgs();

  if (NumArgs > 3)
    return Diag(TheCall->getLocEnd(),
             diag::err_typecheck_call_too_many_args_at_most)
             << 0 /*function call*/ << 3 << NumArgs
             << TheCall->getSourceRange();

  // The alignment must be a constant integer.
  Expr *Arg = TheCall->getArg(1);

  // We can't check the value of a dependent argument.
  if (!Arg->isTypeDependent() && !Arg->isValueDependent()) {
    llvm::APSInt Result;
    if (SemaBuiltinConstantArg(TheCall, 1, Result))
      return true;

    if (!Result.isPowerOf2())
      return Diag(TheCall->getLocStart(),
                  diag::err_alignment_not_power_of_two)
           << Arg->getSourceRange();
  }

  if (NumArgs > 2) {
    ExprResult Arg(TheCall->getArg(2));
    InitializedEntity Entity = InitializedEntity::InitializeParameter(Context,
      Context.getSizeType(), false);
    Arg = PerformCopyInitialization(Entity, SourceLocation(), Arg);
    if (Arg.isInvalid()) return true;
    TheCall->setArg(2, Arg.get());
  }

  return false;
}

/// SemaBuiltinConstantArg - Handle a check if argument ArgNum of CallExpr
/// TheCall is a constant expression.
bool Sema::SemaBuiltinConstantArg(CallExpr *TheCall, int ArgNum,
                                  llvm::APSInt &Result) {
  Expr *Arg = TheCall->getArg(ArgNum);
  DeclRefExpr *DRE =cast<DeclRefExpr>(TheCall->getCallee()->IgnoreParenCasts());
  FunctionDecl *FDecl = cast<FunctionDecl>(DRE->getDecl());
  
  if (Arg->isTypeDependent() || Arg->isValueDependent()) return false;
  
  if (!Arg->isIntegerConstantExpr(Result, Context))
    return Diag(TheCall->getLocStart(), diag::err_constant_integer_arg_type)
                << FDecl->getDeclName() <<  Arg->getSourceRange();
  
  return false;
}

/// SemaBuiltinConstantArgRange - Handle a check if argument ArgNum of CallExpr
/// TheCall is a constant expression in the range [Low, High].
bool Sema::SemaBuiltinConstantArgRange(CallExpr *TheCall, int ArgNum,
                                       int Low, int High) {
  llvm::APSInt Result;

  // We can't check the value of a dependent argument.
  Expr *Arg = TheCall->getArg(ArgNum);
  if (Arg->isTypeDependent() || Arg->isValueDependent())
    return false;

  // Check constant-ness first.
  if (SemaBuiltinConstantArg(TheCall, ArgNum, Result))
    return true;

  if (Result.getSExtValue() < Low || Result.getSExtValue() > High)
    return Diag(TheCall->getLocStart(), diag::err_argument_invalid_range)
      << Low << High << Arg->getSourceRange();

  return false;
}

/// SemaBuiltinARMSpecialReg - Handle a check if argument ArgNum of CallExpr
/// TheCall is an ARM/AArch64 special register string literal.
bool Sema::SemaBuiltinARMSpecialReg(unsigned BuiltinID, CallExpr *TheCall,
                                    int ArgNum, unsigned ExpectedFieldNum,
                                    bool AllowName) {
#if 1 // HLSL Change - remove unsupported builtins
  return false;
#else
  bool IsARMBuiltin = BuiltinID == ARM::BI__builtin_arm_rsr64 ||
                      BuiltinID == ARM::BI__builtin_arm_wsr64 ||
                      BuiltinID == ARM::BI__builtin_arm_rsr ||
                      BuiltinID == ARM::BI__builtin_arm_rsrp ||
                      BuiltinID == ARM::BI__builtin_arm_wsr ||
                      BuiltinID == ARM::BI__builtin_arm_wsrp;
  bool IsAArch64Builtin = BuiltinID == AArch64::BI__builtin_arm_rsr64 ||
                          BuiltinID == AArch64::BI__builtin_arm_wsr64 ||
                          BuiltinID == AArch64::BI__builtin_arm_rsr ||
                          BuiltinID == AArch64::BI__builtin_arm_rsrp ||
                          BuiltinID == AArch64::BI__builtin_arm_wsr ||
                          BuiltinID == AArch64::BI__builtin_arm_wsrp;
  assert((IsARMBuiltin || IsAArch64Builtin) && "Unexpected ARM builtin.");

  // We can't check the value of a dependent argument.
  Expr *Arg = TheCall->getArg(ArgNum);
  if (Arg->isTypeDependent() || Arg->isValueDependent())
    return false;

  // Check if the argument is a string literal.
  if (!isa<StringLiteral>(Arg->IgnoreParenImpCasts()))
    return Diag(TheCall->getLocStart(), diag::err_expr_not_string_literal)
           << Arg->getSourceRange();

  // Check the type of special register given.
  StringRef Reg = cast<StringLiteral>(Arg->IgnoreParenImpCasts())->getString();
  SmallVector<StringRef, 6> Fields;
  Reg.split(Fields, ":");

  if (Fields.size() != ExpectedFieldNum && !(AllowName && Fields.size() == 1))
    return Diag(TheCall->getLocStart(), diag::err_arm_invalid_specialreg)
           << Arg->getSourceRange();

  // If the string is the name of a register then we cannot check that it is
  // valid here but if the string is of one the forms described in ACLE then we
  // can check that the supplied fields are integers and within the valid
  // ranges.
  if (Fields.size() > 1) {
    bool FiveFields = Fields.size() == 5;

    bool ValidString = true;
    if (IsARMBuiltin) {
      ValidString &= Fields[0].startswith_lower("cp") ||
                     Fields[0].startswith_lower("p");
      if (ValidString)
        Fields[0] =
          Fields[0].drop_front(Fields[0].startswith_lower("cp") ? 2 : 1);

      ValidString &= Fields[2].startswith_lower("c");
      if (ValidString)
        Fields[2] = Fields[2].drop_front(1);

      if (FiveFields) {
        ValidString &= Fields[3].startswith_lower("c");
        if (ValidString)
          Fields[3] = Fields[3].drop_front(1);
      }
    }

    SmallVector<int, 5> Ranges;
    if (FiveFields)
      Ranges.append({IsAArch64Builtin ? 1 : 15, 7, 7, 15, 15});
    else
      Ranges.append({15, 7, 15});

    for (unsigned i=0; i<Fields.size(); ++i) {
      int IntField;
      ValidString &= !Fields[i].getAsInteger(10, IntField);
      ValidString &= (IntField >= 0 && IntField <= Ranges[i]);
    }

    if (!ValidString)
      return Diag(TheCall->getLocStart(), diag::err_arm_invalid_specialreg)
             << Arg->getSourceRange();

  } else if (IsAArch64Builtin && Fields.size() == 1) {
    // If the register name is one of those that appear in the condition below
    // and the special register builtin being used is one of the write builtins,
    // then we require that the argument provided for writing to the register
    // is an integer constant expression. This is because it will be lowered to
    // an MSR (immediate) instruction, so we need to know the immediate at
    // compile time.
    if (TheCall->getNumArgs() != 2)
      return false;

    std::string RegLower = Reg.lower();
    if (RegLower != "spsel" && RegLower != "daifset" && RegLower != "daifclr" &&
        RegLower != "pan" && RegLower != "uao")
      return false;

    return SemaBuiltinConstantArgRange(TheCall, 1, 0, 15);
  }

  return false;
#endif // HLSL Change Ends
}

/// SemaBuiltinCpuSupports - Handle __builtin_cpu_supports(char *).
/// This checks that the target supports __builtin_cpu_supports and
/// that the string argument is constant and valid.
bool Sema::SemaBuiltinCpuSupports(CallExpr *TheCall) {
  Expr *Arg = TheCall->getArg(0);

  // Check if the argument is a string literal.
  if (!isa<StringLiteral>(Arg->IgnoreParenImpCasts()))
    return Diag(TheCall->getLocStart(), diag::err_expr_not_string_literal)
           << Arg->getSourceRange();

  // Check the contents of the string.
  StringRef Feature =
      cast<StringLiteral>(Arg->IgnoreParenImpCasts())->getString();
  if (!Context.getTargetInfo().validateCpuSupports(Feature))
    return Diag(TheCall->getLocStart(), diag::err_invalid_cpu_supports)
           << Arg->getSourceRange();
  return false;
}

/// SemaBuiltinLongjmp - Handle __builtin_longjmp(void *env[5], int val).
/// This checks that the target supports __builtin_longjmp and
/// that val is a constant 1.
bool Sema::SemaBuiltinLongjmp(CallExpr *TheCall) {
  if (!Context.getTargetInfo().hasSjLjLowering())
    return Diag(TheCall->getLocStart(), diag::err_builtin_longjmp_unsupported)
             << SourceRange(TheCall->getLocStart(), TheCall->getLocEnd());

  Expr *Arg = TheCall->getArg(1);
  llvm::APSInt Result;

  // TODO: This is less than ideal. Overload this to take a value.
  if (SemaBuiltinConstantArg(TheCall, 1, Result))
    return true;
  
  if (Result != 1)
    return Diag(TheCall->getLocStart(), diag::err_builtin_longjmp_invalid_val)
             << SourceRange(Arg->getLocStart(), Arg->getLocEnd());

  return false;
}


/// SemaBuiltinSetjmp - Handle __builtin_setjmp(void *env[5]).
/// This checks that the target supports __builtin_setjmp.
bool Sema::SemaBuiltinSetjmp(CallExpr *TheCall) {
  if (!Context.getTargetInfo().hasSjLjLowering())
    return Diag(TheCall->getLocStart(), diag::err_builtin_setjmp_unsupported)
             << SourceRange(TheCall->getLocStart(), TheCall->getLocEnd());
  return false;
}

#if 1 // HLSL Change Starts - no support for format strings
bool Sema::hasCStrMethod(const Expr *) {
  return false;
}
#else

namespace {
enum StringLiteralCheckType {
  SLCT_NotALiteral,
  SLCT_UncheckedLiteral,
  SLCT_CheckedLiteral
};
}

// Determine if an expression is a string literal or constant string.
// If this function returns false on the arguments to a function expecting a
// format string, we will usually need to emit a warning.
// True string literals are then checked by CheckFormatString.
static StringLiteralCheckType
checkFormatStringExpr(Sema &S, const Expr *E, ArrayRef<const Expr *> Args,
                      bool HasVAListArg, unsigned format_idx,
                      unsigned firstDataArg, Sema::FormatStringType Type,
                      Sema::VariadicCallType CallType, bool InFunctionCall,
                      llvm::SmallBitVector &CheckedVarArgs) {
 tryAgain:
  if (E->isTypeDependent() || E->isValueDependent())
    return SLCT_NotALiteral;

  E = E->IgnoreParenCasts();

  if (E->isNullPointerConstant(S.Context, Expr::NPC_ValueDependentIsNotNull))
    // Technically -Wformat-nonliteral does not warn about this case.
    // The behavior of printf and friends in this case is implementation
    // dependent.  Ideally if the format string cannot be null then
    // it should have a 'nonnull' attribute in the function prototype.
    return SLCT_UncheckedLiteral;

  switch (E->getStmtClass()) {
  case Stmt::BinaryConditionalOperatorClass:
  case Stmt::ConditionalOperatorClass: {
    // The expression is a literal if both sub-expressions were, and it was
    // completely checked only if both sub-expressions were checked.
    const AbstractConditionalOperator *C =
        cast<AbstractConditionalOperator>(E);
    StringLiteralCheckType Left =
        checkFormatStringExpr(S, C->getTrueExpr(), Args,
                              HasVAListArg, format_idx, firstDataArg,
                              Type, CallType, InFunctionCall, CheckedVarArgs);
    if (Left == SLCT_NotALiteral)
      return SLCT_NotALiteral;
    StringLiteralCheckType Right =
        checkFormatStringExpr(S, C->getFalseExpr(), Args,
                              HasVAListArg, format_idx, firstDataArg,
                              Type, CallType, InFunctionCall, CheckedVarArgs);
    return Left < Right ? Left : Right;
  }

  case Stmt::ImplicitCastExprClass: {
    E = cast<ImplicitCastExpr>(E)->getSubExpr();
    goto tryAgain;
  }

  case Stmt::OpaqueValueExprClass:
    if (const Expr *src = cast<OpaqueValueExpr>(E)->getSourceExpr()) {
      E = src;
      goto tryAgain;
    }
    return SLCT_NotALiteral;

  case Stmt::PredefinedExprClass:
    // While __func__, etc., are technically not string literals, they
    // cannot contain format specifiers and thus are not a security
    // liability.
    return SLCT_UncheckedLiteral;
      
  case Stmt::DeclRefExprClass: {
    const DeclRefExpr *DR = cast<DeclRefExpr>(E);

    // As an exception, do not flag errors for variables binding to
    // const string literals.
    if (const VarDecl *VD = dyn_cast<VarDecl>(DR->getDecl())) {
      bool isConstant = false;
      QualType T = DR->getType();

      if (const ArrayType *AT = S.Context.getAsArrayType(T)) {
        isConstant = AT->getElementType().isConstant(S.Context);
      } else if (const PointerType *PT = T->getAs<PointerType>()) {
        isConstant = T.isConstant(S.Context) &&
                     PT->getPointeeType().isConstant(S.Context);
      } else if (T->isObjCObjectPointerType()) {
        // In ObjC, there is usually no "const ObjectPointer" type,
        // so don't check if the pointee type is constant.
        isConstant = T.isConstant(S.Context);
      }

      if (isConstant) {
        if (const Expr *Init = VD->getAnyInitializer()) {
          // Look through initializers like const char c[] = { "foo" }
          if (const InitListExpr *InitList = dyn_cast<InitListExpr>(Init)) {
            if (InitList->isStringLiteralInit())
              Init = InitList->getInit(0)->IgnoreParenImpCasts();
          }
          return checkFormatStringExpr(S, Init, Args,
                                       HasVAListArg, format_idx,
                                       firstDataArg, Type, CallType,
                                       /*InFunctionCall*/false, CheckedVarArgs);
        }
      }

      // For vprintf* functions (i.e., HasVAListArg==true), we add a
      // special check to see if the format string is a function parameter
      // of the function calling the printf function.  If the function
      // has an attribute indicating it is a printf-like function, then we
      // should suppress warnings concerning non-literals being used in a call
      // to a vprintf function.  For example:
      //
      // void
      // logmessage(char const *fmt __attribute__ (format (printf, 1, 2)), ...){
      //      va_list ap;
      //      va_start(ap, fmt);
      //      vprintf(fmt, ap);  // Do NOT emit a warning about "fmt".
      //      ...
      // }
      if (HasVAListArg) {
        if (const ParmVarDecl *PV = dyn_cast<ParmVarDecl>(VD)) {
          if (const NamedDecl *ND = dyn_cast<NamedDecl>(PV->getDeclContext())) {
            int PVIndex = PV->getFunctionScopeIndex() + 1;
            for (const auto *PVFormat : ND->specific_attrs<FormatAttr>()) {
              // adjust for implicit parameter
              if (const CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(ND))
                if (MD->isInstance())
                  ++PVIndex;
              // We also check if the formats are compatible.
              // We can't pass a 'scanf' string to a 'printf' function.
              if (PVIndex == PVFormat->getFormatIdx() &&
                  Type == S.GetFormatStringType(PVFormat))
                return SLCT_UncheckedLiteral;
            }
          }
        }
      }
    }

    return SLCT_NotALiteral;
  }

  case Stmt::CallExprClass:
  case Stmt::CXXMemberCallExprClass: {
    const CallExpr *CE = cast<CallExpr>(E);
    if (const NamedDecl *ND = dyn_cast_or_null<NamedDecl>(CE->getCalleeDecl())) {
      if (const FormatArgAttr *FA = ND->getAttr<FormatArgAttr>()) {
        unsigned ArgIndex = FA->getFormatIdx();
        if (const CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(ND))
          if (MD->isInstance())
            --ArgIndex;
        const Expr *Arg = CE->getArg(ArgIndex - 1);

        return checkFormatStringExpr(S, Arg, Args,
                                     HasVAListArg, format_idx, firstDataArg,
                                     Type, CallType, InFunctionCall,
                                     CheckedVarArgs);
      } else if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(ND)) {
        unsigned BuiltinID = FD->getBuiltinID();
        if (BuiltinID == Builtin::BI__builtin___CFStringMakeConstantString ||
            BuiltinID == Builtin::BI__builtin___NSStringMakeConstantString) {
          const Expr *Arg = CE->getArg(0);
          return checkFormatStringExpr(S, Arg, Args,
                                       HasVAListArg, format_idx,
                                       firstDataArg, Type, CallType,
                                       InFunctionCall, CheckedVarArgs);
        }
      }
    }

    return SLCT_NotALiteral;
  }
  case Stmt::ObjCStringLiteralClass:
  case Stmt::StringLiteralClass: {
    const StringLiteral *StrE = nullptr;

    if (const ObjCStringLiteral *ObjCFExpr = dyn_cast<ObjCStringLiteral>(E))
      StrE = ObjCFExpr->getString();
    else
      StrE = cast<StringLiteral>(E);

    if (StrE) {
      S.CheckFormatString(StrE, E, Args, HasVAListArg, format_idx, firstDataArg,
                          Type, InFunctionCall, CallType, CheckedVarArgs);
      return SLCT_CheckedLiteral;
    }

    return SLCT_NotALiteral;
  }

  default:
    return SLCT_NotALiteral;
  }
}

Sema::FormatStringType Sema::GetFormatStringType(const FormatAttr *Format) {
  return llvm::StringSwitch<FormatStringType>(Format->getType()->getName())
  .Case("scanf", FST_Scanf)
  .Cases("printf", "printf0", FST_Printf)
  .Cases("NSString", "CFString", FST_NSString)
  .Case("strftime", FST_Strftime)
  .Case("strfmon", FST_Strfmon)
  .Cases("kprintf", "cmn_err", "vcmn_err", "zcmn_err", FST_Kprintf)
  .Case("freebsd_kprintf", FST_FreeBSDKPrintf)
  .Case("os_trace", FST_OSTrace)
  .Default(FST_Unknown);
}

/// CheckFormatArguments - Check calls to printf and scanf (and similar
/// functions) for correct use of format strings.
/// Returns true if a format string has been fully checked.
bool Sema::CheckFormatArguments(const FormatAttr *Format,
                                ArrayRef<const Expr *> Args,
                                bool IsCXXMember,
                                VariadicCallType CallType,
                                SourceLocation Loc, SourceRange Range,
                                llvm::SmallBitVector &CheckedVarArgs) {
  FormatStringInfo FSI;
  if (getFormatStringInfo(Format, IsCXXMember, &FSI))
    return CheckFormatArguments(Args, FSI.HasVAListArg, FSI.FormatIdx,
                                FSI.FirstDataArg, GetFormatStringType(Format),
                                CallType, Loc, Range, CheckedVarArgs);
  return false;
}

bool Sema::CheckFormatArguments(ArrayRef<const Expr *> Args,
                                bool HasVAListArg, unsigned format_idx,
                                unsigned firstDataArg, FormatStringType Type,
                                VariadicCallType CallType,
                                SourceLocation Loc, SourceRange Range,
                                llvm::SmallBitVector &CheckedVarArgs) {
  // CHECK: printf/scanf-like function is called with no format string.
  if (format_idx >= Args.size()) {
    Diag(Loc, diag::warn_missing_format_string) << Range;
    return false;
  }

  const Expr *OrigFormatExpr = Args[format_idx]->IgnoreParenCasts();

  // CHECK: format string is not a string literal.
  //
  // Dynamically generated format strings are difficult to
  // automatically vet at compile time.  Requiring that format strings
  // are string literals: (1) permits the checking of format strings by
  // the compiler and thereby (2) can practically remove the source of
  // many format string exploits.

  // Format string can be either ObjC string (e.g. @"%d") or
  // C string (e.g. "%d")
  // ObjC string uses the same format specifiers as C string, so we can use
  // the same format string checking logic for both ObjC and C strings.
  StringLiteralCheckType CT =
      checkFormatStringExpr(*this, OrigFormatExpr, Args, HasVAListArg,
                            format_idx, firstDataArg, Type, CallType,
                            /*IsFunctionCall*/true, CheckedVarArgs);
  if (CT != SLCT_NotALiteral)
    // Literal format string found, check done!
    return CT == SLCT_CheckedLiteral;

  // Strftime is particular as it always uses a single 'time' argument,
  // so it is safe to pass a non-literal string.
  if (Type == FST_Strftime)
    return false;

  // Do not emit diag when the string param is a macro expansion and the
  // format is either NSString or CFString. This is a hack to prevent
  // diag when using the NSLocalizedString and CFCopyLocalizedString macros
  // which are usually used in place of NS and CF string literals.
  if (Type == FST_NSString &&
      SourceMgr.isInSystemMacro(Args[format_idx]->getLocStart()))
    return false;

  // If there are no arguments specified, warn with -Wformat-security, otherwise
  // warn only with -Wformat-nonliteral.
  if (Args.size() == firstDataArg)
    Diag(Args[format_idx]->getLocStart(),
         diag::warn_format_nonliteral_noargs)
      << OrigFormatExpr->getSourceRange();
  else
    Diag(Args[format_idx]->getLocStart(),
         diag::warn_format_nonliteral)
           << OrigFormatExpr->getSourceRange();
  return false;
}

namespace {
class CheckFormatHandler : public analyze_format_string::FormatStringHandler {
protected:
  Sema &S;
  const StringLiteral *FExpr;
  const Expr *OrigFormatExpr;
  const unsigned FirstDataArg;
  const unsigned NumDataArgs;
  const char *Beg; // Start of format string.
  const bool HasVAListArg;
  ArrayRef<const Expr *> Args;
  unsigned FormatIdx;
  llvm::SmallBitVector CoveredArgs;
  bool usesPositionalArgs;
  bool atFirstArg;
  bool inFunctionCall;
  Sema::VariadicCallType CallType;
  llvm::SmallBitVector &CheckedVarArgs;
public:
  CheckFormatHandler(Sema &s, const StringLiteral *fexpr,
                     const Expr *origFormatExpr, unsigned firstDataArg,
                     unsigned numDataArgs, const char *beg, bool hasVAListArg,
                     ArrayRef<const Expr *> Args,
                     unsigned formatIdx, bool inFunctionCall,
                     Sema::VariadicCallType callType,
                     llvm::SmallBitVector &CheckedVarArgs)
    : S(s), FExpr(fexpr), OrigFormatExpr(origFormatExpr),
      FirstDataArg(firstDataArg), NumDataArgs(numDataArgs),
      Beg(beg), HasVAListArg(hasVAListArg),
      Args(Args), FormatIdx(formatIdx),
      usesPositionalArgs(false), atFirstArg(true),
      inFunctionCall(inFunctionCall), CallType(callType),
      CheckedVarArgs(CheckedVarArgs) {
    CoveredArgs.resize(numDataArgs);
    CoveredArgs.reset();
  }

  void DoneProcessing();

  void HandleIncompleteSpecifier(const char *startSpecifier,
                                 unsigned specifierLen) override;

  void HandleInvalidLengthModifier(
                           const analyze_format_string::FormatSpecifier &FS,
                           const analyze_format_string::ConversionSpecifier &CS,
                           const char *startSpecifier, unsigned specifierLen,
                           unsigned DiagID);

  void HandleNonStandardLengthModifier(
                    const analyze_format_string::FormatSpecifier &FS,
                    const char *startSpecifier, unsigned specifierLen);

  void HandleNonStandardConversionSpecifier(
                    const analyze_format_string::ConversionSpecifier &CS,
                    const char *startSpecifier, unsigned specifierLen);

  void HandlePosition(const char *startPos, unsigned posLen) override;

  void HandleInvalidPosition(const char *startSpecifier,
                             unsigned specifierLen,
                             analyze_format_string::PositionContext p) override;

  void HandleZeroPosition(const char *startPos, unsigned posLen) override;

  void HandleNullChar(const char *nullCharacter) override;

  template <typename Range>
  static void EmitFormatDiagnostic(Sema &S, bool inFunctionCall,
                                   const Expr *ArgumentExpr,
                                   PartialDiagnostic PDiag,
                                   SourceLocation StringLoc,
                                   bool IsStringLocation, Range StringRange,
                                   ArrayRef<FixItHint> Fixit = None);

protected:
  bool HandleInvalidConversionSpecifier(unsigned argIndex, SourceLocation Loc,
                                        const char *startSpec,
                                        unsigned specifierLen,
                                        const char *csStart, unsigned csLen);

  void HandlePositionalNonpositionalArgs(SourceLocation Loc,
                                         const char *startSpec,
                                         unsigned specifierLen);
  
  SourceRange getFormatStringRange();
  CharSourceRange getSpecifierRange(const char *startSpecifier,
                                    unsigned specifierLen);
  SourceLocation getLocationOfByte(const char *x);

  const Expr *getDataArg(unsigned i) const;
  
  bool CheckNumArgs(const analyze_format_string::FormatSpecifier &FS,
                    const analyze_format_string::ConversionSpecifier &CS,
                    const char *startSpecifier, unsigned specifierLen,
                    unsigned argIndex);

  template <typename Range>
  void EmitFormatDiagnostic(PartialDiagnostic PDiag, SourceLocation StringLoc,
                            bool IsStringLocation, Range StringRange,
                            ArrayRef<FixItHint> Fixit = None);
};
}

SourceRange CheckFormatHandler::getFormatStringRange() {
  return OrigFormatExpr->getSourceRange();
}

CharSourceRange CheckFormatHandler::
getSpecifierRange(const char *startSpecifier, unsigned specifierLen) {
  SourceLocation Start = getLocationOfByte(startSpecifier);
  SourceLocation End   = getLocationOfByte(startSpecifier + specifierLen - 1);

  // Advance the end SourceLocation by one due to half-open ranges.
  End = End.getLocWithOffset(1);

  return CharSourceRange::getCharRange(Start, End);
}

SourceLocation CheckFormatHandler::getLocationOfByte(const char *x) {
  return S.getLocationOfStringLiteralByte(FExpr, x - Beg);
}

void CheckFormatHandler::HandleIncompleteSpecifier(const char *startSpecifier,
                                                   unsigned specifierLen){
  EmitFormatDiagnostic(S.PDiag(diag::warn_printf_incomplete_specifier),
                       getLocationOfByte(startSpecifier),
                       /*IsStringLocation*/true,
                       getSpecifierRange(startSpecifier, specifierLen));
}

void CheckFormatHandler::HandleInvalidLengthModifier(
    const analyze_format_string::FormatSpecifier &FS,
    const analyze_format_string::ConversionSpecifier &CS,
    const char *startSpecifier, unsigned specifierLen, unsigned DiagID) {
  using namespace analyze_format_string;

  const LengthModifier &LM = FS.getLengthModifier();
  CharSourceRange LMRange = getSpecifierRange(LM.getStart(), LM.getLength());

  // See if we know how to fix this length modifier.
  Optional<LengthModifier> FixedLM = FS.getCorrectedLengthModifier();
  if (FixedLM) {
    EmitFormatDiagnostic(S.PDiag(DiagID) << LM.toString() << CS.toString(),
                         getLocationOfByte(LM.getStart()),
                         /*IsStringLocation*/true,
                         getSpecifierRange(startSpecifier, specifierLen));

    S.Diag(getLocationOfByte(LM.getStart()), diag::note_format_fix_specifier)
      << FixedLM->toString()
      << FixItHint::CreateReplacement(LMRange, FixedLM->toString());

  } else {
    FixItHint Hint;
    if (DiagID == diag::warn_format_nonsensical_length)
      Hint = FixItHint::CreateRemoval(LMRange);

    EmitFormatDiagnostic(S.PDiag(DiagID) << LM.toString() << CS.toString(),
                         getLocationOfByte(LM.getStart()),
                         /*IsStringLocation*/true,
                         getSpecifierRange(startSpecifier, specifierLen),
                         Hint);
  }
}

void CheckFormatHandler::HandleNonStandardLengthModifier(
    const analyze_format_string::FormatSpecifier &FS,
    const char *startSpecifier, unsigned specifierLen) {
  using namespace analyze_format_string;

  const LengthModifier &LM = FS.getLengthModifier();
  CharSourceRange LMRange = getSpecifierRange(LM.getStart(), LM.getLength());

  // See if we know how to fix this length modifier.
  Optional<LengthModifier> FixedLM = FS.getCorrectedLengthModifier();
  if (FixedLM) {
    EmitFormatDiagnostic(S.PDiag(diag::warn_format_non_standard)
                           << LM.toString() << 0,
                         getLocationOfByte(LM.getStart()),
                         /*IsStringLocation*/true,
                         getSpecifierRange(startSpecifier, specifierLen));

    S.Diag(getLocationOfByte(LM.getStart()), diag::note_format_fix_specifier)
      << FixedLM->toString()
      << FixItHint::CreateReplacement(LMRange, FixedLM->toString());

  } else {
    EmitFormatDiagnostic(S.PDiag(diag::warn_format_non_standard)
                           << LM.toString() << 0,
                         getLocationOfByte(LM.getStart()),
                         /*IsStringLocation*/true,
                         getSpecifierRange(startSpecifier, specifierLen));
  }
}

void CheckFormatHandler::HandleNonStandardConversionSpecifier(
    const analyze_format_string::ConversionSpecifier &CS,
    const char *startSpecifier, unsigned specifierLen) {
  using namespace analyze_format_string;

  // See if we know how to fix this conversion specifier.
  Optional<ConversionSpecifier> FixedCS = CS.getStandardSpecifier();
  if (FixedCS) {
    EmitFormatDiagnostic(S.PDiag(diag::warn_format_non_standard)
                          << CS.toString() << /*conversion specifier*/1,
                         getLocationOfByte(CS.getStart()),
                         /*IsStringLocation*/true,
                         getSpecifierRange(startSpecifier, specifierLen));

    CharSourceRange CSRange = getSpecifierRange(CS.getStart(), CS.getLength());
    S.Diag(getLocationOfByte(CS.getStart()), diag::note_format_fix_specifier)
      << FixedCS->toString()
      << FixItHint::CreateReplacement(CSRange, FixedCS->toString());
  } else {
    EmitFormatDiagnostic(S.PDiag(diag::warn_format_non_standard)
                          << CS.toString() << /*conversion specifier*/1,
                         getLocationOfByte(CS.getStart()),
                         /*IsStringLocation*/true,
                         getSpecifierRange(startSpecifier, specifierLen));
  }
}

void CheckFormatHandler::HandlePosition(const char *startPos,
                                        unsigned posLen) {
  EmitFormatDiagnostic(S.PDiag(diag::warn_format_non_standard_positional_arg),
                               getLocationOfByte(startPos),
                               /*IsStringLocation*/true,
                               getSpecifierRange(startPos, posLen));
}

void
CheckFormatHandler::HandleInvalidPosition(const char *startPos, unsigned posLen,
                                     analyze_format_string::PositionContext p) {
  EmitFormatDiagnostic(S.PDiag(diag::warn_format_invalid_positional_specifier)
                         << (unsigned) p,
                       getLocationOfByte(startPos), /*IsStringLocation*/true,
                       getSpecifierRange(startPos, posLen));
}

void CheckFormatHandler::HandleZeroPosition(const char *startPos,
                                            unsigned posLen) {
  EmitFormatDiagnostic(S.PDiag(diag::warn_format_zero_positional_specifier),
                               getLocationOfByte(startPos),
                               /*IsStringLocation*/true,
                               getSpecifierRange(startPos, posLen));
}

void CheckFormatHandler::HandleNullChar(const char *nullCharacter) {
  if (!isa<ObjCStringLiteral>(OrigFormatExpr)) {
    // The presence of a null character is likely an error.
    EmitFormatDiagnostic(
      S.PDiag(diag::warn_printf_format_string_contains_null_char),
      getLocationOfByte(nullCharacter), /*IsStringLocation*/true,
      getFormatStringRange());
  }
}

// Note that this may return NULL if there was an error parsing or building
// one of the argument expressions.
const Expr *CheckFormatHandler::getDataArg(unsigned i) const {
  return Args[FirstDataArg + i];
}

void CheckFormatHandler::DoneProcessing() {
    // Does the number of data arguments exceed the number of
    // format conversions in the format string?
  if (!HasVAListArg) {
      // Find any arguments that weren't covered.
    CoveredArgs.flip();
    signed notCoveredArg = CoveredArgs.find_first();
    if (notCoveredArg >= 0) {
      assert((unsigned)notCoveredArg < NumDataArgs);
      if (const Expr *E = getDataArg((unsigned) notCoveredArg)) {
        SourceLocation Loc = E->getLocStart();
        if (!S.getSourceManager().isInSystemMacro(Loc)) {
          EmitFormatDiagnostic(S.PDiag(diag::warn_printf_data_arg_not_used),
                               Loc, /*IsStringLocation*/false,
                               getFormatStringRange());
        }
      }
    }
  }
}

bool
CheckFormatHandler::HandleInvalidConversionSpecifier(unsigned argIndex,
                                                     SourceLocation Loc,
                                                     const char *startSpec,
                                                     unsigned specifierLen,
                                                     const char *csStart,
                                                     unsigned csLen) {
  
  bool keepGoing = true;
  if (argIndex < NumDataArgs) {
    // Consider the argument coverered, even though the specifier doesn't
    // make sense.
    CoveredArgs.set(argIndex);
  }
  else {
    // If argIndex exceeds the number of data arguments we
    // don't issue a warning because that is just a cascade of warnings (and
    // they may have intended '%%' anyway). We don't want to continue processing
    // the format string after this point, however, as we will like just get
    // gibberish when trying to match arguments.
    keepGoing = false;
  }
  
  EmitFormatDiagnostic(S.PDiag(diag::warn_format_invalid_conversion)
                         << StringRef(csStart, csLen),
                       Loc, /*IsStringLocation*/true,
                       getSpecifierRange(startSpec, specifierLen));
  
  return keepGoing;
}

void
CheckFormatHandler::HandlePositionalNonpositionalArgs(SourceLocation Loc,
                                                      const char *startSpec,
                                                      unsigned specifierLen) {
  EmitFormatDiagnostic(
    S.PDiag(diag::warn_format_mix_positional_nonpositional_args),
    Loc, /*isStringLoc*/true, getSpecifierRange(startSpec, specifierLen));
}

bool
CheckFormatHandler::CheckNumArgs(
  const analyze_format_string::FormatSpecifier &FS,
  const analyze_format_string::ConversionSpecifier &CS,
  const char *startSpecifier, unsigned specifierLen, unsigned argIndex) {

  if (argIndex >= NumDataArgs) {
    PartialDiagnostic PDiag = FS.usesPositionalArg()
      ? (S.PDiag(diag::warn_printf_positional_arg_exceeds_data_args)
           << (argIndex+1) << NumDataArgs)
      : S.PDiag(diag::warn_printf_insufficient_data_args);
    EmitFormatDiagnostic(
      PDiag, getLocationOfByte(CS.getStart()), /*IsStringLocation*/true,
      getSpecifierRange(startSpecifier, specifierLen));
    return false;
  }
  return true;
}

template<typename Range>
void CheckFormatHandler::EmitFormatDiagnostic(PartialDiagnostic PDiag,
                                              SourceLocation Loc,
                                              bool IsStringLocation,
                                              Range StringRange,
                                              ArrayRef<FixItHint> FixIt) {
  EmitFormatDiagnostic(S, inFunctionCall, Args[FormatIdx], PDiag,
                       Loc, IsStringLocation, StringRange, FixIt);
}

/// \brief If the format string is not within the funcion call, emit a note
/// so that the function call and string are in diagnostic messages.
///
/// \param InFunctionCall if true, the format string is within the function
/// call and only one diagnostic message will be produced.  Otherwise, an
/// extra note will be emitted pointing to location of the format string.
///
/// \param ArgumentExpr the expression that is passed as the format string
/// argument in the function call.  Used for getting locations when two
/// diagnostics are emitted.
///
/// \param PDiag the callee should already have provided any strings for the
/// diagnostic message.  This function only adds locations and fixits
/// to diagnostics.
///
/// \param Loc primary location for diagnostic.  If two diagnostics are
/// required, one will be at Loc and a new SourceLocation will be created for
/// the other one.
///
/// \param IsStringLocation if true, Loc points to the format string should be
/// used for the note.  Otherwise, Loc points to the argument list and will
/// be used with PDiag.
///
/// \param StringRange some or all of the string to highlight.  This is
/// templated so it can accept either a CharSourceRange or a SourceRange.
///
/// \param FixIt optional fix it hint for the format string.
template<typename Range>
void CheckFormatHandler::EmitFormatDiagnostic(Sema &S, bool InFunctionCall,
                                              const Expr *ArgumentExpr,
                                              PartialDiagnostic PDiag,
                                              SourceLocation Loc,
                                              bool IsStringLocation,
                                              Range StringRange,
                                              ArrayRef<FixItHint> FixIt) {
  if (InFunctionCall) {
    const Sema::SemaDiagnosticBuilder &D = S.Diag(Loc, PDiag);
    D << StringRange;
    D << FixIt;
  } else {
    S.Diag(IsStringLocation ? ArgumentExpr->getExprLoc() : Loc, PDiag)
      << ArgumentExpr->getSourceRange();

    const Sema::SemaDiagnosticBuilder &Note =
      S.Diag(IsStringLocation ? Loc : StringRange.getBegin(),
             diag::note_format_string_defined);

    Note << StringRange;
    Note << FixIt;
  }
}

//===--- CHECK: Printf format string checking ------------------------------===//

namespace {
class CheckPrintfHandler : public CheckFormatHandler {
  bool ObjCContext;
public:
  CheckPrintfHandler(Sema &s, const StringLiteral *fexpr,
                     const Expr *origFormatExpr, unsigned firstDataArg,
                     unsigned numDataArgs, bool isObjC,
                     const char *beg, bool hasVAListArg,
                     ArrayRef<const Expr *> Args,
                     unsigned formatIdx, bool inFunctionCall,
                     Sema::VariadicCallType CallType,
                     llvm::SmallBitVector &CheckedVarArgs)
    : CheckFormatHandler(s, fexpr, origFormatExpr, firstDataArg,
                         numDataArgs, beg, hasVAListArg, Args,
                         formatIdx, inFunctionCall, CallType, CheckedVarArgs),
      ObjCContext(isObjC)
  {}


  bool HandleInvalidPrintfConversionSpecifier(
                                      const analyze_printf::PrintfSpecifier &FS,
                                      const char *startSpecifier,
                                      unsigned specifierLen) override;

  bool HandlePrintfSpecifier(const analyze_printf::PrintfSpecifier &FS,
                             const char *startSpecifier,
                             unsigned specifierLen) override;
  bool checkFormatExpr(const analyze_printf::PrintfSpecifier &FS,
                       const char *StartSpecifier,
                       unsigned SpecifierLen,
                       const Expr *E);

  bool HandleAmount(const analyze_format_string::OptionalAmount &Amt, unsigned k,
                    const char *startSpecifier, unsigned specifierLen);
  void HandleInvalidAmount(const analyze_printf::PrintfSpecifier &FS,
                           const analyze_printf::OptionalAmount &Amt,
                           unsigned type,
                           const char *startSpecifier, unsigned specifierLen);
  void HandleFlag(const analyze_printf::PrintfSpecifier &FS,
                  const analyze_printf::OptionalFlag &flag,
                  const char *startSpecifier, unsigned specifierLen);
  void HandleIgnoredFlag(const analyze_printf::PrintfSpecifier &FS,
                         const analyze_printf::OptionalFlag &ignoredFlag,
                         const analyze_printf::OptionalFlag &flag,
                         const char *startSpecifier, unsigned specifierLen);
  bool checkForCStrMembers(const analyze_printf::ArgType &AT,
                           const Expr *E);
                           
  void HandleEmptyObjCModifierFlag(const char *startFlag,
                                   unsigned flagLen) override;

  void HandleInvalidObjCModifierFlag(const char *startFlag,
                                            unsigned flagLen) override;

  void HandleObjCFlagsWithNonObjCConversion(const char *flagsStart,
                                           const char *flagsEnd,
                                           const char *conversionPosition) 
                                             override;
};
}

bool CheckPrintfHandler::HandleInvalidPrintfConversionSpecifier(
                                      const analyze_printf::PrintfSpecifier &FS,
                                      const char *startSpecifier,
                                      unsigned specifierLen) {
  const analyze_printf::PrintfConversionSpecifier &CS =
    FS.getConversionSpecifier();
  
  return HandleInvalidConversionSpecifier(FS.getArgIndex(),
                                          getLocationOfByte(CS.getStart()),
                                          startSpecifier, specifierLen,
                                          CS.getStart(), CS.getLength());
}

bool CheckPrintfHandler::HandleAmount(
                               const analyze_format_string::OptionalAmount &Amt,
                               unsigned k, const char *startSpecifier,
                               unsigned specifierLen) {

  if (Amt.hasDataArgument()) {
    if (!HasVAListArg) {
      unsigned argIndex = Amt.getArgIndex();
      if (argIndex >= NumDataArgs) {
        EmitFormatDiagnostic(S.PDiag(diag::warn_printf_asterisk_missing_arg)
                               << k,
                             getLocationOfByte(Amt.getStart()),
                             /*IsStringLocation*/true,
                             getSpecifierRange(startSpecifier, specifierLen));
        // Don't do any more checking.  We will just emit
        // spurious errors.
        return false;
      }

      // Type check the data argument.  It should be an 'int'.
      // Although not in conformance with C99, we also allow the argument to be
      // an 'unsigned int' as that is a reasonably safe case.  GCC also
      // doesn't emit a warning for that case.
      CoveredArgs.set(argIndex);
      const Expr *Arg = getDataArg(argIndex);
      if (!Arg)
        return false;

      QualType T = Arg->getType();

      const analyze_printf::ArgType &AT = Amt.getArgType(S.Context);
      assert(AT.isValid());

      if (!AT.matchesType(S.Context, T)) {
        EmitFormatDiagnostic(S.PDiag(diag::warn_printf_asterisk_wrong_type)
                               << k << AT.getRepresentativeTypeName(S.Context)
                               << T << Arg->getSourceRange(),
                             getLocationOfByte(Amt.getStart()),
                             /*IsStringLocation*/true,
                             getSpecifierRange(startSpecifier, specifierLen));
        // Don't do any more checking.  We will just emit
        // spurious errors.
        return false;
      }
    }
  }
  return true;
}

void CheckPrintfHandler::HandleInvalidAmount(
                                      const analyze_printf::PrintfSpecifier &FS,
                                      const analyze_printf::OptionalAmount &Amt,
                                      unsigned type,
                                      const char *startSpecifier,
                                      unsigned specifierLen) {
  const analyze_printf::PrintfConversionSpecifier &CS =
    FS.getConversionSpecifier();

  FixItHint fixit =
    Amt.getHowSpecified() == analyze_printf::OptionalAmount::Constant
      ? FixItHint::CreateRemoval(getSpecifierRange(Amt.getStart(),
                                 Amt.getConstantLength()))
      : FixItHint();

  EmitFormatDiagnostic(S.PDiag(diag::warn_printf_nonsensical_optional_amount)
                         << type << CS.toString(),
                       getLocationOfByte(Amt.getStart()),
                       /*IsStringLocation*/true,
                       getSpecifierRange(startSpecifier, specifierLen),
                       fixit);
}

void CheckPrintfHandler::HandleFlag(const analyze_printf::PrintfSpecifier &FS,
                                    const analyze_printf::OptionalFlag &flag,
                                    const char *startSpecifier,
                                    unsigned specifierLen) {
  // Warn about pointless flag with a fixit removal.
  const analyze_printf::PrintfConversionSpecifier &CS =
    FS.getConversionSpecifier();
  EmitFormatDiagnostic(S.PDiag(diag::warn_printf_nonsensical_flag)
                         << flag.toString() << CS.toString(),
                       getLocationOfByte(flag.getPosition()),
                       /*IsStringLocation*/true,
                       getSpecifierRange(startSpecifier, specifierLen),
                       FixItHint::CreateRemoval(
                         getSpecifierRange(flag.getPosition(), 1)));
}

void CheckPrintfHandler::HandleIgnoredFlag(
                                const analyze_printf::PrintfSpecifier &FS,
                                const analyze_printf::OptionalFlag &ignoredFlag,
                                const analyze_printf::OptionalFlag &flag,
                                const char *startSpecifier,
                                unsigned specifierLen) {
  // Warn about ignored flag with a fixit removal.
  EmitFormatDiagnostic(S.PDiag(diag::warn_printf_ignored_flag)
                         << ignoredFlag.toString() << flag.toString(),
                       getLocationOfByte(ignoredFlag.getPosition()),
                       /*IsStringLocation*/true,
                       getSpecifierRange(startSpecifier, specifierLen),
                       FixItHint::CreateRemoval(
                         getSpecifierRange(ignoredFlag.getPosition(), 1)));
}

//  void EmitFormatDiagnostic(PartialDiagnostic PDiag, SourceLocation StringLoc,
//                            bool IsStringLocation, Range StringRange,
//                            ArrayRef<FixItHint> Fixit = None);
                            
void CheckPrintfHandler::HandleEmptyObjCModifierFlag(const char *startFlag,
                                                     unsigned flagLen) {
  // Warn about an empty flag.
  EmitFormatDiagnostic(S.PDiag(diag::warn_printf_empty_objc_flag),
                       getLocationOfByte(startFlag),
                       /*IsStringLocation*/true,
                       getSpecifierRange(startFlag, flagLen));
}

void CheckPrintfHandler::HandleInvalidObjCModifierFlag(const char *startFlag,
                                                       unsigned flagLen) {
  // Warn about an invalid flag.
  auto Range = getSpecifierRange(startFlag, flagLen);
  StringRef flag(startFlag, flagLen);
  EmitFormatDiagnostic(S.PDiag(diag::warn_printf_invalid_objc_flag) << flag,
                      getLocationOfByte(startFlag),
                      /*IsStringLocation*/true,
                      Range, FixItHint::CreateRemoval(Range));
}

void CheckPrintfHandler::HandleObjCFlagsWithNonObjCConversion(
    const char *flagsStart, const char *flagsEnd, const char *conversionPosition) {
    // Warn about using '[...]' without a '@' conversion.
    auto Range = getSpecifierRange(flagsStart, flagsEnd - flagsStart + 1);
    auto diag = diag::warn_printf_ObjCflags_without_ObjCConversion;
    EmitFormatDiagnostic(S.PDiag(diag) << StringRef(conversionPosition, 1),
                         getLocationOfByte(conversionPosition),
                         /*IsStringLocation*/true,
                         Range, FixItHint::CreateRemoval(Range));
}

// Determines if the specified is a C++ class or struct containing
// a member with the specified name and kind (e.g. a CXXMethodDecl named
// "c_str()").
template<typename MemberKind>
static llvm::SmallPtrSet<MemberKind*, 1>
CXXRecordMembersNamed(StringRef Name, Sema &S, QualType Ty) {
  const RecordType *RT = Ty->getAs<RecordType>();
  llvm::SmallPtrSet<MemberKind*, 1> Results;

  if (!RT)
    return Results;
  const CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(RT->getDecl());
  if (!RD || !RD->getDefinition())
    return Results;

  LookupResult R(S, &S.Context.Idents.get(Name), SourceLocation(),
                 Sema::LookupMemberName);
  R.suppressDiagnostics();

  // We just need to include all members of the right kind turned up by the
  // filter, at this point.
  if (S.LookupQualifiedName(R, RT->getDecl()))
    for (LookupResult::iterator I = R.begin(), E = R.end(); I != E; ++I) {
      NamedDecl *decl = (*I)->getUnderlyingDecl();
      if (MemberKind *FK = dyn_cast<MemberKind>(decl))
        Results.insert(FK);
    }
  return Results;
}

/// Check if we could call '.c_str()' on an object.
///
/// FIXME: This returns the wrong results in some cases (if cv-qualifiers don't
/// allow the call, or if it would be ambiguous).
bool Sema::hasCStrMethod(const Expr *E) {
  typedef llvm::SmallPtrSet<CXXMethodDecl*, 1> MethodSet;
  MethodSet Results =
      CXXRecordMembersNamed<CXXMethodDecl>("c_str", *this, E->getType());
  for (MethodSet::iterator MI = Results.begin(), ME = Results.end();
       MI != ME; ++MI)
    if ((*MI)->getMinRequiredArguments() == 0)
      return true;
  return false;
}

// Check if a (w)string was passed when a (w)char* was needed, and offer a
// better diagnostic if so. AT is assumed to be valid.
// Returns true when a c_str() conversion method is found.
bool CheckPrintfHandler::checkForCStrMembers(
    const analyze_printf::ArgType &AT, const Expr *E) {
  typedef llvm::SmallPtrSet<CXXMethodDecl*, 1> MethodSet;

  MethodSet Results =
      CXXRecordMembersNamed<CXXMethodDecl>("c_str", S, E->getType());

  for (MethodSet::iterator MI = Results.begin(), ME = Results.end();
       MI != ME; ++MI) {
    const CXXMethodDecl *Method = *MI;
    if (Method->getMinRequiredArguments() == 0 &&
        AT.matchesType(S.Context, Method->getReturnType())) {
      // FIXME: Suggest parens if the expression needs them.
      SourceLocation EndLoc = S.getLocForEndOfToken(E->getLocEnd());
      S.Diag(E->getLocStart(), diag::note_printf_c_str)
          << "c_str()"
          << FixItHint::CreateInsertion(EndLoc, ".c_str()");
      return true;
    }
  }

  return false;
}

bool
CheckPrintfHandler::HandlePrintfSpecifier(const analyze_printf::PrintfSpecifier
                                            &FS,
                                          const char *startSpecifier,
                                          unsigned specifierLen) {

  using namespace analyze_format_string;
  using namespace analyze_printf;  
  const PrintfConversionSpecifier &CS = FS.getConversionSpecifier();

  if (FS.consumesDataArgument()) {
    if (atFirstArg) {
        atFirstArg = false;
        usesPositionalArgs = FS.usesPositionalArg();
    }
    else if (usesPositionalArgs != FS.usesPositionalArg()) {
      HandlePositionalNonpositionalArgs(getLocationOfByte(CS.getStart()),
                                        startSpecifier, specifierLen);
      return false;
    }
  }

  // First check if the field width, precision, and conversion specifier
  // have matching data arguments.
  if (!HandleAmount(FS.getFieldWidth(), /* field width */ 0,
                    startSpecifier, specifierLen)) {
    return false;
  }

  if (!HandleAmount(FS.getPrecision(), /* precision */ 1,
                    startSpecifier, specifierLen)) {
    return false;
  }

  if (!CS.consumesDataArgument()) {
    // FIXME: Technically specifying a precision or field width here
    // makes no sense.  Worth issuing a warning at some point.
    return true;
  }

  // Consume the argument.
  unsigned argIndex = FS.getArgIndex();
  if (argIndex < NumDataArgs) {
    // The check to see if the argIndex is valid will come later.
    // We set the bit here because we may exit early from this
    // function if we encounter some other error.
    CoveredArgs.set(argIndex);
  }

  // FreeBSD kernel extensions.
  if (CS.getKind() == ConversionSpecifier::FreeBSDbArg ||
      CS.getKind() == ConversionSpecifier::FreeBSDDArg) {
    // We need at least two arguments.
    if (!CheckNumArgs(FS, CS, startSpecifier, specifierLen, argIndex + 1))
      return false;

    // Claim the second argument.
    CoveredArgs.set(argIndex + 1);

    // Type check the first argument (int for %b, pointer for %D)
    const Expr *Ex = getDataArg(argIndex);
    const analyze_printf::ArgType &AT =
      (CS.getKind() == ConversionSpecifier::FreeBSDbArg) ?
        ArgType(S.Context.IntTy) : ArgType::CPointerTy;
    if (AT.isValid() && !AT.matchesType(S.Context, Ex->getType()))
      EmitFormatDiagnostic(
        S.PDiag(diag::warn_format_conversion_argument_type_mismatch)
        << AT.getRepresentativeTypeName(S.Context) << Ex->getType()
        << false << Ex->getSourceRange(),
        Ex->getLocStart(), /*IsStringLocation*/false,
        getSpecifierRange(startSpecifier, specifierLen));

    // Type check the second argument (char * for both %b and %D)
    Ex = getDataArg(argIndex + 1);
    const analyze_printf::ArgType &AT2 = ArgType::CStrTy;
    if (AT2.isValid() && !AT2.matchesType(S.Context, Ex->getType()))
      EmitFormatDiagnostic(
        S.PDiag(diag::warn_format_conversion_argument_type_mismatch)
        << AT2.getRepresentativeTypeName(S.Context) << Ex->getType()
        << false << Ex->getSourceRange(),
        Ex->getLocStart(), /*IsStringLocation*/false,
        getSpecifierRange(startSpecifier, specifierLen));

     return true;
  }

  // Check for using an Objective-C specific conversion specifier
  // in a non-ObjC literal.
  if (!ObjCContext && CS.isObjCArg()) {
    return HandleInvalidPrintfConversionSpecifier(FS, startSpecifier,
                                                  specifierLen);
  }

  // Check for invalid use of field width
  if (!FS.hasValidFieldWidth()) {
    HandleInvalidAmount(FS, FS.getFieldWidth(), /* field width */ 0,
        startSpecifier, specifierLen);
  }

  // Check for invalid use of precision
  if (!FS.hasValidPrecision()) {
    HandleInvalidAmount(FS, FS.getPrecision(), /* precision */ 1,
        startSpecifier, specifierLen);
  }

  // Check each flag does not conflict with any other component.
  if (!FS.hasValidThousandsGroupingPrefix())
    HandleFlag(FS, FS.hasThousandsGrouping(), startSpecifier, specifierLen);
  if (!FS.hasValidLeadingZeros())
    HandleFlag(FS, FS.hasLeadingZeros(), startSpecifier, specifierLen);
  if (!FS.hasValidPlusPrefix())
    HandleFlag(FS, FS.hasPlusPrefix(), startSpecifier, specifierLen);
  if (!FS.hasValidSpacePrefix())
    HandleFlag(FS, FS.hasSpacePrefix(), startSpecifier, specifierLen);
  if (!FS.hasValidAlternativeForm())
    HandleFlag(FS, FS.hasAlternativeForm(), startSpecifier, specifierLen);
  if (!FS.hasValidLeftJustified())
    HandleFlag(FS, FS.isLeftJustified(), startSpecifier, specifierLen);

  // Check that flags are not ignored by another flag
  if (FS.hasSpacePrefix() && FS.hasPlusPrefix()) // ' ' ignored by '+'
    HandleIgnoredFlag(FS, FS.hasSpacePrefix(), FS.hasPlusPrefix(),
        startSpecifier, specifierLen);
  if (FS.hasLeadingZeros() && FS.isLeftJustified()) // '0' ignored by '-'
    HandleIgnoredFlag(FS, FS.hasLeadingZeros(), FS.isLeftJustified(),
            startSpecifier, specifierLen);

  // Check the length modifier is valid with the given conversion specifier.
  if (!FS.hasValidLengthModifier(S.getASTContext().getTargetInfo()))
    HandleInvalidLengthModifier(FS, CS, startSpecifier, specifierLen,
                                diag::warn_format_nonsensical_length);
  else if (!FS.hasStandardLengthModifier())
    HandleNonStandardLengthModifier(FS, startSpecifier, specifierLen);
  else if (!FS.hasStandardLengthConversionCombination())
    HandleInvalidLengthModifier(FS, CS, startSpecifier, specifierLen,
                                diag::warn_format_non_standard_conversion_spec);

  if (!FS.hasStandardConversionSpecifier(S.getLangOpts()))
    HandleNonStandardConversionSpecifier(CS, startSpecifier, specifierLen);

  // The remaining checks depend on the data arguments.
  if (HasVAListArg)
    return true;

  if (!CheckNumArgs(FS, CS, startSpecifier, specifierLen, argIndex))
    return false;

  const Expr *Arg = getDataArg(argIndex);
  if (!Arg)
    return true;

  return checkFormatExpr(FS, startSpecifier, specifierLen, Arg);
}

static bool requiresParensToAddCast(const Expr *E) {
  // FIXME: We should have a general way to reason about operator
  // precedence and whether parens are actually needed here.
  // Take care of a few common cases where they aren't.
  const Expr *Inside = E->IgnoreImpCasts();
  if (const PseudoObjectExpr *POE = dyn_cast<PseudoObjectExpr>(Inside))
    Inside = POE->getSyntacticForm()->IgnoreImpCasts();

  switch (Inside->getStmtClass()) {
  case Stmt::ArraySubscriptExprClass:
  case Stmt::CallExprClass:
  case Stmt::CharacterLiteralClass:
  case Stmt::CXXBoolLiteralExprClass:
  case Stmt::DeclRefExprClass:
  case Stmt::FloatingLiteralClass:
  case Stmt::IntegerLiteralClass:
  case Stmt::MemberExprClass:
  case Stmt::ObjCArrayLiteralClass:
  case Stmt::ObjCBoolLiteralExprClass:
  case Stmt::ObjCBoxedExprClass:
  case Stmt::ObjCDictionaryLiteralClass:
  case Stmt::ObjCEncodeExprClass:
  case Stmt::ObjCIvarRefExprClass:
  case Stmt::ObjCMessageExprClass:
  case Stmt::ObjCPropertyRefExprClass:
  case Stmt::ObjCStringLiteralClass:
  case Stmt::ObjCSubscriptRefExprClass:
  case Stmt::ParenExprClass:
  case Stmt::StringLiteralClass:
  case Stmt::UnaryOperatorClass:
    return false;
  default:
    return true;
  }
}

static std::pair<QualType, StringRef>
shouldNotPrintDirectly(const ASTContext &Context,
                       QualType IntendedTy,
                       const Expr *E) {
  // Use a 'while' to peel off layers of typedefs.
  QualType TyTy = IntendedTy;
  while (const TypedefType *UserTy = TyTy->getAs<TypedefType>()) {
    StringRef Name = UserTy->getDecl()->getName();
    QualType CastTy = llvm::StringSwitch<QualType>(Name)
      .Case("NSInteger", Context.LongTy)
      .Case("NSUInteger", Context.UnsignedLongTy)
      .Case("SInt32", Context.IntTy)
      .Case("UInt32", Context.UnsignedIntTy)
      .Default(QualType());

    if (!CastTy.isNull())
      return std::make_pair(CastTy, Name);

    TyTy = UserTy->desugar();
  }

  // Strip parens if necessary.
  if (const ParenExpr *PE = dyn_cast<ParenExpr>(E))
    return shouldNotPrintDirectly(Context,
                                  PE->getSubExpr()->getType(),
                                  PE->getSubExpr());

  // If this is a conditional expression, then its result type is constructed
  // via usual arithmetic conversions and thus there might be no necessary
  // typedef sugar there.  Recurse to operands to check for NSInteger &
  // Co. usage condition.
  if (const ConditionalOperator *CO = dyn_cast<ConditionalOperator>(E)) {
    QualType TrueTy, FalseTy;
    StringRef TrueName, FalseName;

    std::tie(TrueTy, TrueName) =
      shouldNotPrintDirectly(Context,
                             CO->getTrueExpr()->getType(),
                             CO->getTrueExpr());
    std::tie(FalseTy, FalseName) =
      shouldNotPrintDirectly(Context,
                             CO->getFalseExpr()->getType(),
                             CO->getFalseExpr());

    if (TrueTy == FalseTy)
      return std::make_pair(TrueTy, TrueName);
    else if (TrueTy.isNull())
      return std::make_pair(FalseTy, FalseName);
    else if (FalseTy.isNull())
      return std::make_pair(TrueTy, TrueName);
  }

  return std::make_pair(QualType(), StringRef());
}

bool
CheckPrintfHandler::checkFormatExpr(const analyze_printf::PrintfSpecifier &FS,
                                    const char *StartSpecifier,
                                    unsigned SpecifierLen,
                                    const Expr *E) {
  using namespace analyze_format_string;
  using namespace analyze_printf;
  // Now type check the data expression that matches the
  // format specifier.
  const analyze_printf::ArgType &AT = FS.getArgType(S.Context,
                                                    ObjCContext);
  if (!AT.isValid())
    return true;

  QualType ExprTy = E->getType();
  while (const TypeOfExprType *TET = dyn_cast<TypeOfExprType>(ExprTy)) {
    ExprTy = TET->getUnderlyingExpr()->getType();
  }

  analyze_printf::ArgType::MatchKind match = AT.matchesType(S.Context, ExprTy);

  if (match == analyze_printf::ArgType::Match) {
    return true;
  }

  // Look through argument promotions for our error message's reported type.
  // This includes the integral and floating promotions, but excludes array
  // and function pointer decay; seeing that an argument intended to be a
  // string has type 'char [6]' is probably more confusing than 'char *'.
  if (const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(E)) {
    if (ICE->getCastKind() == CK_IntegralCast ||
        ICE->getCastKind() == CK_FloatingCast) {
      E = ICE->getSubExpr();
      ExprTy = E->getType();

      // Check if we didn't match because of an implicit cast from a 'char'
      // or 'short' to an 'int'.  This is done because printf is a varargs
      // function.
      if (ICE->getType() == S.Context.IntTy ||
          ICE->getType() == S.Context.UnsignedIntTy) {
        // All further checking is done on the subexpression.
        if (AT.matchesType(S.Context, ExprTy))
          return true;
      }
    }
  } else if (const CharacterLiteral *CL = dyn_cast<CharacterLiteral>(E)) {
    // Special case for 'a', which has type 'int' in C.
    // Note, however, that we do /not/ want to treat multibyte constants like
    // 'MooV' as characters! This form is deprecated but still exists.
    if (ExprTy == S.Context.IntTy)
      if (llvm::isUIntN(S.Context.getCharWidth(), CL->getValue()))
        ExprTy = S.Context.CharTy;
  }

  // Look through enums to their underlying type.
  bool IsEnum = false;
  if (auto EnumTy = ExprTy->getAs<EnumType>()) {
    ExprTy = EnumTy->getDecl()->getIntegerType();
    IsEnum = true;
  }

  // %C in an Objective-C context prints a unichar, not a wchar_t.
  // If the argument is an integer of some kind, believe the %C and suggest
  // a cast instead of changing the conversion specifier.
  QualType IntendedTy = ExprTy;
  if (ObjCContext &&
      FS.getConversionSpecifier().getKind() == ConversionSpecifier::CArg) {
    if (ExprTy->isIntegralOrUnscopedEnumerationType() &&
        !ExprTy->isCharType()) {
      // 'unichar' is defined as a typedef of unsigned short, but we should
      // prefer using the typedef if it is visible.
      IntendedTy = S.Context.UnsignedShortTy;

      // While we are here, check if the value is an IntegerLiteral that happens
      // to be within the valid range.
      if (const IntegerLiteral *IL = dyn_cast<IntegerLiteral>(E)) {
        const llvm::APInt &V = IL->getValue();
        if (V.getActiveBits() <= S.Context.getTypeSize(IntendedTy))
          return true;
      }

      LookupResult Result(S, &S.Context.Idents.get("unichar"), E->getLocStart(),
                          Sema::LookupOrdinaryName);
      if (S.LookupName(Result, S.getCurScope())) {
        NamedDecl *ND = Result.getFoundDecl();
        if (TypedefNameDecl *TD = dyn_cast<TypedefNameDecl>(ND))
          if (TD->getUnderlyingType() == IntendedTy)
            IntendedTy = S.Context.getTypedefType(TD);
      }
    }
  }

  // Special-case some of Darwin's platform-independence types by suggesting
  // casts to primitive types that are known to be large enough.
  bool ShouldNotPrintDirectly = false; StringRef CastTyName;
  if (S.Context.getTargetInfo().getTriple().isOSDarwin()) {
    QualType CastTy;
    std::tie(CastTy, CastTyName) = shouldNotPrintDirectly(S.Context, IntendedTy, E);
    if (!CastTy.isNull()) {
      IntendedTy = CastTy;
      ShouldNotPrintDirectly = true;
    }
  }

  // We may be able to offer a FixItHint if it is a supported type.
  PrintfSpecifier fixedFS = FS;
  bool success = fixedFS.fixType(IntendedTy, S.getLangOpts(),
                                 S.Context, ObjCContext);

  if (success) {
    // Get the fix string from the fixed format specifier
    SmallString<16> buf;
    llvm::raw_svector_ostream os(buf);
    fixedFS.toString(os);

    CharSourceRange SpecRange = getSpecifierRange(StartSpecifier, SpecifierLen);

    if (IntendedTy == ExprTy && !ShouldNotPrintDirectly) {
      unsigned diag = diag::warn_format_conversion_argument_type_mismatch;
      if (match == analyze_format_string::ArgType::NoMatchPedantic) {
        diag = diag::warn_format_conversion_argument_type_mismatch_pedantic;
      }
      // In this case, the specifier is wrong and should be changed to match
      // the argument.
      EmitFormatDiagnostic(S.PDiag(diag)
                               << AT.getRepresentativeTypeName(S.Context)
                               << IntendedTy << IsEnum << E->getSourceRange(),
                           E->getLocStart(),
                           /*IsStringLocation*/ false, SpecRange,
                           FixItHint::CreateReplacement(SpecRange, os.str()));

    } else {
      // The canonical type for formatting this value is different from the
      // actual type of the expression. (This occurs, for example, with Darwin's
      // NSInteger on 32-bit platforms, where it is typedef'd as 'int', but
      // should be printed as 'long' for 64-bit compatibility.)
      // Rather than emitting a normal format/argument mismatch, we want to
      // add a cast to the recommended type (and correct the format string
      // if necessary).
      SmallString<16> CastBuf;
      llvm::raw_svector_ostream CastFix(CastBuf);
      CastFix << "(";
      IntendedTy.print(CastFix, S.Context.getPrintingPolicy());
      CastFix << ")";

      SmallVector<FixItHint,4> Hints;
      if (!AT.matchesType(S.Context, IntendedTy))
        Hints.push_back(FixItHint::CreateReplacement(SpecRange, os.str()));

      if (const CStyleCastExpr *CCast = dyn_cast<CStyleCastExpr>(E)) {
        // If there's already a cast present, just replace it.
        SourceRange CastRange(CCast->getLParenLoc(), CCast->getRParenLoc());
        Hints.push_back(FixItHint::CreateReplacement(CastRange, CastFix.str()));

      } else if (!requiresParensToAddCast(E)) {
        // If the expression has high enough precedence,
        // just write the C-style cast.
        Hints.push_back(FixItHint::CreateInsertion(E->getLocStart(),
                                                   CastFix.str()));
      } else {
        // Otherwise, add parens around the expression as well as the cast.
        CastFix << "(";
        Hints.push_back(FixItHint::CreateInsertion(E->getLocStart(),
                                                   CastFix.str()));

        SourceLocation After = S.getLocForEndOfToken(E->getLocEnd());
        Hints.push_back(FixItHint::CreateInsertion(After, ")"));
      }

      if (ShouldNotPrintDirectly) {
        // The expression has a type that should not be printed directly.
        // We extract the name from the typedef because we don't want to show
        // the underlying type in the diagnostic.
        StringRef Name;
        if (const TypedefType *TypedefTy = dyn_cast<TypedefType>(ExprTy))
          Name = TypedefTy->getDecl()->getName();
        else
          Name = CastTyName;
        EmitFormatDiagnostic(S.PDiag(diag::warn_format_argument_needs_cast)
                               << Name << IntendedTy << IsEnum
                               << E->getSourceRange(),
                             E->getLocStart(), /*IsStringLocation=*/false,
                             SpecRange, Hints);
      } else {
        // In this case, the expression could be printed using a different
        // specifier, but we've decided that the specifier is probably correct 
        // and we should cast instead. Just use the normal warning message.
        EmitFormatDiagnostic(
          S.PDiag(diag::warn_format_conversion_argument_type_mismatch)
            << AT.getRepresentativeTypeName(S.Context) << ExprTy << IsEnum
            << E->getSourceRange(),
          E->getLocStart(), /*IsStringLocation*/false,
          SpecRange, Hints);
      }
    }
  } else {
    const CharSourceRange &CSR = getSpecifierRange(StartSpecifier,
                                                   SpecifierLen);
    // Since the warning for passing non-POD types to variadic functions
    // was deferred until now, we emit a warning for non-POD
    // arguments here.
    switch (S.isValidVarArgType(ExprTy)) {
    case Sema::VAK_Valid:
    case Sema::VAK_ValidInCXX11: {
      unsigned diag = diag::warn_format_conversion_argument_type_mismatch;
      if (match == analyze_printf::ArgType::NoMatchPedantic) {
        diag = diag::warn_format_conversion_argument_type_mismatch_pedantic;
      }

      EmitFormatDiagnostic(
          S.PDiag(diag) << AT.getRepresentativeTypeName(S.Context) << ExprTy
                        << IsEnum << CSR << E->getSourceRange(),
          E->getLocStart(), /*IsStringLocation*/ false, CSR);
      break;
    }
    case Sema::VAK_Undefined:
    case Sema::VAK_MSVCUndefined:
      EmitFormatDiagnostic(
        S.PDiag(diag::warn_non_pod_vararg_with_format_string)
          << S.getLangOpts().CPlusPlus11
          << ExprTy
          << CallType
          << AT.getRepresentativeTypeName(S.Context)
          << CSR
          << E->getSourceRange(),
        E->getLocStart(), /*IsStringLocation*/false, CSR);
      checkForCStrMembers(AT, E);
      break;

    case Sema::VAK_Invalid:
      if (ExprTy->isObjCObjectType())
        EmitFormatDiagnostic(
          S.PDiag(diag::err_cannot_pass_objc_interface_to_vararg_format)
            << S.getLangOpts().CPlusPlus11
            << ExprTy
            << CallType
            << AT.getRepresentativeTypeName(S.Context)
            << CSR
            << E->getSourceRange(),
          E->getLocStart(), /*IsStringLocation*/false, CSR);
      else
        // FIXME: If this is an initializer list, suggest removing the braces
        // or inserting a cast to the target type.
        S.Diag(E->getLocStart(), diag::err_cannot_pass_to_vararg_format)
          << isa<InitListExpr>(E) << ExprTy << CallType
          << AT.getRepresentativeTypeName(S.Context)
          << E->getSourceRange();
      break;
    }

    assert(FirstDataArg + FS.getArgIndex() < CheckedVarArgs.size() &&
           "format string specifier index out of range");
    CheckedVarArgs[FirstDataArg + FS.getArgIndex()] = true;
  }

  return true;
}

//===--- CHECK: Scanf format string checking ------------------------------===//

namespace {  
class CheckScanfHandler : public CheckFormatHandler {
public:
  CheckScanfHandler(Sema &s, const StringLiteral *fexpr,
                    const Expr *origFormatExpr, unsigned firstDataArg,
                    unsigned numDataArgs, const char *beg, bool hasVAListArg,
                    ArrayRef<const Expr *> Args,
                    unsigned formatIdx, bool inFunctionCall,
                    Sema::VariadicCallType CallType,
                    llvm::SmallBitVector &CheckedVarArgs)
    : CheckFormatHandler(s, fexpr, origFormatExpr, firstDataArg,
                         numDataArgs, beg, hasVAListArg,
                         Args, formatIdx, inFunctionCall, CallType,
                         CheckedVarArgs)
  {}
  
  bool HandleScanfSpecifier(const analyze_scanf::ScanfSpecifier &FS,
                            const char *startSpecifier,
                            unsigned specifierLen) override;
  
  bool HandleInvalidScanfConversionSpecifier(
          const analyze_scanf::ScanfSpecifier &FS,
          const char *startSpecifier,
          unsigned specifierLen) override;

  void HandleIncompleteScanList(const char *start, const char *end) override;
};
}

void CheckScanfHandler::HandleIncompleteScanList(const char *start,
                                                 const char *end) {
  EmitFormatDiagnostic(S.PDiag(diag::warn_scanf_scanlist_incomplete),
                       getLocationOfByte(end), /*IsStringLocation*/true,
                       getSpecifierRange(start, end - start));
}

bool CheckScanfHandler::HandleInvalidScanfConversionSpecifier(
                                        const analyze_scanf::ScanfSpecifier &FS,
                                        const char *startSpecifier,
                                        unsigned specifierLen) {

  const analyze_scanf::ScanfConversionSpecifier &CS =
    FS.getConversionSpecifier();

  return HandleInvalidConversionSpecifier(FS.getArgIndex(),
                                          getLocationOfByte(CS.getStart()),
                                          startSpecifier, specifierLen,
                                          CS.getStart(), CS.getLength());
}

bool CheckScanfHandler::HandleScanfSpecifier(
                                       const analyze_scanf::ScanfSpecifier &FS,
                                       const char *startSpecifier,
                                       unsigned specifierLen) {
  
  using namespace analyze_scanf;
  using namespace analyze_format_string;  

  const ScanfConversionSpecifier &CS = FS.getConversionSpecifier();

  // Handle case where '%' and '*' don't consume an argument.  These shouldn't
  // be used to decide if we are using positional arguments consistently.
  if (FS.consumesDataArgument()) {
    if (atFirstArg) {
      atFirstArg = false;
      usesPositionalArgs = FS.usesPositionalArg();
    }
    else if (usesPositionalArgs != FS.usesPositionalArg()) {
      HandlePositionalNonpositionalArgs(getLocationOfByte(CS.getStart()),
                                        startSpecifier, specifierLen);
      return false;
    }
  }
  
  // Check if the field with is non-zero.
  const OptionalAmount &Amt = FS.getFieldWidth();
  if (Amt.getHowSpecified() == OptionalAmount::Constant) {
    if (Amt.getConstantAmount() == 0) {
      const CharSourceRange &R = getSpecifierRange(Amt.getStart(),
                                                   Amt.getConstantLength());
      EmitFormatDiagnostic(S.PDiag(diag::warn_scanf_nonzero_width),
                           getLocationOfByte(Amt.getStart()),
                           /*IsStringLocation*/true, R,
                           FixItHint::CreateRemoval(R));
    }
  }

  if (!FS.consumesDataArgument()) {
    // FIXME: Technically specifying a precision or field width here
    // makes no sense.  Worth issuing a warning at some point.
    return true;
  }

  // Consume the argument.
  unsigned argIndex = FS.getArgIndex();
  if (argIndex < NumDataArgs) {
      // The check to see if the argIndex is valid will come later.
      // We set the bit here because we may exit early from this
      // function if we encounter some other error.
    CoveredArgs.set(argIndex);
  }

  // Check the length modifier is valid with the given conversion specifier.
  if (!FS.hasValidLengthModifier(S.getASTContext().getTargetInfo()))
    HandleInvalidLengthModifier(FS, CS, startSpecifier, specifierLen,
                                diag::warn_format_nonsensical_length);
  else if (!FS.hasStandardLengthModifier())
    HandleNonStandardLengthModifier(FS, startSpecifier, specifierLen);
  else if (!FS.hasStandardLengthConversionCombination())
    HandleInvalidLengthModifier(FS, CS, startSpecifier, specifierLen,
                                diag::warn_format_non_standard_conversion_spec);

  if (!FS.hasStandardConversionSpecifier(S.getLangOpts()))
    HandleNonStandardConversionSpecifier(CS, startSpecifier, specifierLen);

  // The remaining checks depend on the data arguments.
  if (HasVAListArg)
    return true;

  if (!CheckNumArgs(FS, CS, startSpecifier, specifierLen, argIndex))
    return false;

  // Check that the argument type matches the format specifier.
  const Expr *Ex = getDataArg(argIndex);
  if (!Ex)
    return true;

  const analyze_format_string::ArgType &AT = FS.getArgType(S.Context);

  if (!AT.isValid()) {
    return true;
  }

  analyze_format_string::ArgType::MatchKind match =
      AT.matchesType(S.Context, Ex->getType());
  if (match == analyze_format_string::ArgType::Match) {
    return true;
  }

  ScanfSpecifier fixedFS = FS;
  bool success = fixedFS.fixType(Ex->getType(), Ex->IgnoreImpCasts()->getType(),
                                 S.getLangOpts(), S.Context);

  unsigned diag = diag::warn_format_conversion_argument_type_mismatch;
  if (match == analyze_format_string::ArgType::NoMatchPedantic) {
    diag = diag::warn_format_conversion_argument_type_mismatch_pedantic;
  }

  if (success) {
    // Get the fix string from the fixed format specifier.
    SmallString<128> buf;
    llvm::raw_svector_ostream os(buf);
    fixedFS.toString(os);

    EmitFormatDiagnostic(
        S.PDiag(diag) << AT.getRepresentativeTypeName(S.Context)
                      << Ex->getType() << false << Ex->getSourceRange(),
        Ex->getLocStart(),
        /*IsStringLocation*/ false,
        getSpecifierRange(startSpecifier, specifierLen),
        FixItHint::CreateReplacement(
            getSpecifierRange(startSpecifier, specifierLen), os.str()));
  } else {
    EmitFormatDiagnostic(S.PDiag(diag)
                             << AT.getRepresentativeTypeName(S.Context)
                             << Ex->getType() << false << Ex->getSourceRange(),
                         Ex->getLocStart(),
                         /*IsStringLocation*/ false,
                         getSpecifierRange(startSpecifier, specifierLen));
  }

  return true;
}

void Sema::CheckFormatString(const StringLiteral *FExpr,
                             const Expr *OrigFormatExpr,
                             ArrayRef<const Expr *> Args,
                             bool HasVAListArg, unsigned format_idx,
                             unsigned firstDataArg, FormatStringType Type,
                             bool inFunctionCall, VariadicCallType CallType,
                             llvm::SmallBitVector &CheckedVarArgs) {
  
  // CHECK: is the format string a wide literal?
  if (!FExpr->isAscii() && !FExpr->isUTF8()) {
    CheckFormatHandler::EmitFormatDiagnostic(
      *this, inFunctionCall, Args[format_idx],
      PDiag(diag::warn_format_string_is_wide_literal), FExpr->getLocStart(),
      /*IsStringLocation*/true, OrigFormatExpr->getSourceRange());
    return;
  }
  
  // Str - The format string.  NOTE: this is NOT null-terminated!
  StringRef StrRef = FExpr->getString();
  const char *Str = StrRef.data();
  // Account for cases where the string literal is truncated in a declaration.
  const ConstantArrayType *T = Context.getAsConstantArrayType(FExpr->getType());
  assert(T && "String literal not of constant array type!");
  size_t TypeSize = T->getSize().getZExtValue();
  size_t StrLen = std::min(std::max(TypeSize, size_t(1)) - 1, StrRef.size());
  const unsigned numDataArgs = Args.size() - firstDataArg;

  // Emit a warning if the string literal is truncated and does not contain an
  // embedded null character.
  if (TypeSize <= StrRef.size() &&
      StrRef.substr(0, TypeSize).find('\0') == StringRef::npos) {
    CheckFormatHandler::EmitFormatDiagnostic(
        *this, inFunctionCall, Args[format_idx],
        PDiag(diag::warn_printf_format_string_not_null_terminated),
        FExpr->getLocStart(),
        /*IsStringLocation=*/true, OrigFormatExpr->getSourceRange());
    return;
  }

  // CHECK: empty format string?
  if (StrLen == 0 && numDataArgs > 0) {
    CheckFormatHandler::EmitFormatDiagnostic(
      *this, inFunctionCall, Args[format_idx],
      PDiag(diag::warn_empty_format_string), FExpr->getLocStart(),
      /*IsStringLocation*/true, OrigFormatExpr->getSourceRange());
    return;
  }
  
  if (Type == FST_Printf || Type == FST_NSString ||
      Type == FST_FreeBSDKPrintf || Type == FST_OSTrace) {
    CheckPrintfHandler H(*this, FExpr, OrigFormatExpr, firstDataArg,
                         numDataArgs, (Type == FST_NSString || Type == FST_OSTrace),
                         Str, HasVAListArg, Args, format_idx,
                         inFunctionCall, CallType, CheckedVarArgs);
  
    if (!analyze_format_string::ParsePrintfString(H, Str, Str + StrLen,
                                                  getLangOpts(),
                                                  Context.getTargetInfo(),
                                                  Type == FST_FreeBSDKPrintf))
      H.DoneProcessing();
  } else if (Type == FST_Scanf) {
    CheckScanfHandler H(*this, FExpr, OrigFormatExpr, firstDataArg, numDataArgs,
                        Str, HasVAListArg, Args, format_idx,
                        inFunctionCall, CallType, CheckedVarArgs);
    
    if (!analyze_format_string::ParseScanfString(H, Str, Str + StrLen,
                                                 getLangOpts(),
                                                 Context.getTargetInfo()))
      H.DoneProcessing();
  } // TODO: handle other formats
}

bool Sema::FormatStringHasSArg(const StringLiteral *FExpr) {
  // Str - The format string.  NOTE: this is NOT null-terminated!
  StringRef StrRef = FExpr->getString();
  const char *Str = StrRef.data();
  // Account for cases where the string literal is truncated in a declaration.
  const ConstantArrayType *T = Context.getAsConstantArrayType(FExpr->getType());
  assert(T && "String literal not of constant array type!");
  size_t TypeSize = T->getSize().getZExtValue();
  size_t StrLen = std::min(std::max(TypeSize, size_t(1)) - 1, StrRef.size());
  return analyze_format_string::ParseFormatStringHasSArg(Str, Str + StrLen,
                                                         getLangOpts(),
                                                         Context.getTargetInfo());
}

#endif // HLSL Change - no support for format strings

//===--- CHECK: Warn on use of wrong absolute value function. -------------===//

// Returns the related absolute value function that is larger, of 0 if one
// does not exist.
static unsigned getLargerAbsoluteValueFunction(unsigned AbsFunction) {
  switch (AbsFunction) {
  default:
    return 0;

  case Builtin::BI__builtin_abs:
    return Builtin::BI__builtin_labs;
  case Builtin::BI__builtin_labs:
    return Builtin::BI__builtin_llabs;
  case Builtin::BI__builtin_llabs:
    return 0;

  case Builtin::BI__builtin_fabsf:
    return Builtin::BI__builtin_fabs;
  case Builtin::BI__builtin_fabs:
    return Builtin::BI__builtin_fabsl;
  case Builtin::BI__builtin_fabsl:
    return 0;

  case Builtin::BI__builtin_cabsf:
    return Builtin::BI__builtin_cabs;
  case Builtin::BI__builtin_cabs:
    return Builtin::BI__builtin_cabsl;
  case Builtin::BI__builtin_cabsl:
    return 0;

  case Builtin::BIabs:
    return Builtin::BIlabs;
  case Builtin::BIlabs:
    return Builtin::BIllabs;
  case Builtin::BIllabs:
    return 0;

  case Builtin::BIfabsf:
    return Builtin::BIfabs;
  case Builtin::BIfabs:
    return Builtin::BIfabsl;
  case Builtin::BIfabsl:
    return 0;

  case Builtin::BIcabsf:
   return Builtin::BIcabs;
  case Builtin::BIcabs:
    return Builtin::BIcabsl;
  case Builtin::BIcabsl:
    return 0;
  }
}

// Returns the argument type of the absolute value function.
static QualType getAbsoluteValueArgumentType(ASTContext &Context,
                                             unsigned AbsType) {
  if (AbsType == 0)
    return QualType();

  ASTContext::GetBuiltinTypeError Error = ASTContext::GE_None;
  QualType BuiltinType = Context.GetBuiltinType(AbsType, Error);
  if (Error != ASTContext::GE_None)
    return QualType();

  const FunctionProtoType *FT = BuiltinType->getAs<FunctionProtoType>();
  if (!FT)
    return QualType();

  if (FT->getNumParams() != 1)
    return QualType();

  return FT->getParamType(0);
}

// Returns the best absolute value function, or zero, based on type and
// current absolute value function.
static unsigned getBestAbsFunction(ASTContext &Context, QualType ArgType,
                                   unsigned AbsFunctionKind) {
  unsigned BestKind = 0;
  uint64_t ArgSize = Context.getTypeSize(ArgType);
  for (unsigned Kind = AbsFunctionKind; Kind != 0;
       Kind = getLargerAbsoluteValueFunction(Kind)) {
    QualType ParamType = getAbsoluteValueArgumentType(Context, Kind);
    if (Context.getTypeSize(ParamType) >= ArgSize) {
      if (BestKind == 0)
        BestKind = Kind;
      else if (Context.hasSameType(ParamType, ArgType)) {
        BestKind = Kind;
        break;
      }
    }
  }
  return BestKind;
}

enum AbsoluteValueKind {
  AVK_Integer,
  AVK_Floating,
  AVK_Complex
};

static AbsoluteValueKind getAbsoluteValueKind(QualType T) {
  if (T->isIntegralOrEnumerationType())
    return AVK_Integer;
  if (T->isRealFloatingType())
    return AVK_Floating;
  if (T->isAnyComplexType())
    return AVK_Complex;

  llvm_unreachable("Type not integer, floating, or complex");
}

// Changes the absolute value function to a different type.  Preserves whether
// the function is a builtin.
static unsigned changeAbsFunction(unsigned AbsKind,
                                  AbsoluteValueKind ValueKind) {
  switch (ValueKind) {
  case AVK_Integer:
    switch (AbsKind) {
    default:
      return 0;
    case Builtin::BI__builtin_fabsf:
    case Builtin::BI__builtin_fabs:
    case Builtin::BI__builtin_fabsl:
    case Builtin::BI__builtin_cabsf:
    case Builtin::BI__builtin_cabs:
    case Builtin::BI__builtin_cabsl:
      return Builtin::BI__builtin_abs;
    case Builtin::BIfabsf:
    case Builtin::BIfabs:
    case Builtin::BIfabsl:
    case Builtin::BIcabsf:
    case Builtin::BIcabs:
    case Builtin::BIcabsl:
      return Builtin::BIabs;
    }
  case AVK_Floating:
    switch (AbsKind) {
    default:
      return 0;
    case Builtin::BI__builtin_abs:
    case Builtin::BI__builtin_labs:
    case Builtin::BI__builtin_llabs:
    case Builtin::BI__builtin_cabsf:
    case Builtin::BI__builtin_cabs:
    case Builtin::BI__builtin_cabsl:
      return Builtin::BI__builtin_fabsf;
    case Builtin::BIabs:
    case Builtin::BIlabs:
    case Builtin::BIllabs:
    case Builtin::BIcabsf:
    case Builtin::BIcabs:
    case Builtin::BIcabsl:
      return Builtin::BIfabsf;
    }
  case AVK_Complex:
    switch (AbsKind) {
    default:
      return 0;
    case Builtin::BI__builtin_abs:
    case Builtin::BI__builtin_labs:
    case Builtin::BI__builtin_llabs:
    case Builtin::BI__builtin_fabsf:
    case Builtin::BI__builtin_fabs:
    case Builtin::BI__builtin_fabsl:
      return Builtin::BI__builtin_cabsf;
    case Builtin::BIabs:
    case Builtin::BIlabs:
    case Builtin::BIllabs:
    case Builtin::BIfabsf:
    case Builtin::BIfabs:
    case Builtin::BIfabsl:
      return Builtin::BIcabsf;
    }
  }
  llvm_unreachable("Unable to convert function");
}

static unsigned getAbsoluteValueFunctionKind(const FunctionDecl *FDecl) {
  const IdentifierInfo *FnInfo = FDecl->getIdentifier();
  if (!FnInfo)
    return 0;

  switch (FDecl->getBuiltinID()) {
  default:
    return 0;
  case Builtin::BI__builtin_abs:
  case Builtin::BI__builtin_fabs:
  case Builtin::BI__builtin_fabsf:
  case Builtin::BI__builtin_fabsl:
  case Builtin::BI__builtin_labs:
  case Builtin::BI__builtin_llabs:
  case Builtin::BI__builtin_cabs:
  case Builtin::BI__builtin_cabsf:
  case Builtin::BI__builtin_cabsl:
  case Builtin::BIabs:
  case Builtin::BIlabs:
  case Builtin::BIllabs:
  case Builtin::BIfabs:
  case Builtin::BIfabsf:
  case Builtin::BIfabsl:
  case Builtin::BIcabs:
  case Builtin::BIcabsf:
  case Builtin::BIcabsl:
    return FDecl->getBuiltinID();
  }
  llvm_unreachable("Unknown Builtin type");
}

// If the replacement is valid, emit a note with replacement function.
// Additionally, suggest including the proper header if not already included.
static void emitReplacement(Sema &S, SourceLocation Loc, SourceRange Range,
                            unsigned AbsKind, QualType ArgType) {
  bool EmitHeaderHint = true;
  const char *HeaderName = nullptr;
  const char *FunctionName = nullptr;
  if (S.getLangOpts().CPlusPlus && !ArgType->isAnyComplexType()) {
    FunctionName = "std::abs";
    if (ArgType->isIntegralOrEnumerationType()) {
      HeaderName = "cstdlib";
    } else if (ArgType->isRealFloatingType()) {
      HeaderName = "cmath";
    } else {
      llvm_unreachable("Invalid Type");
    }

    // Lookup all std::abs
    if (NamespaceDecl *Std = S.getStdNamespace()) {
      LookupResult R(S, &S.Context.Idents.get("abs"), Loc, Sema::LookupAnyName);
      R.suppressDiagnostics();
      S.LookupQualifiedName(R, Std);

      for (const auto *I : R) {
        const FunctionDecl *FDecl = nullptr;
        if (const UsingShadowDecl *UsingD = dyn_cast<UsingShadowDecl>(I)) {
          FDecl = dyn_cast<FunctionDecl>(UsingD->getTargetDecl());
        } else {
          FDecl = dyn_cast<FunctionDecl>(I);
        }
        if (!FDecl)
          continue;

        // Found std::abs(), check that they are the right ones.
        if (FDecl->getNumParams() != 1)
          continue;

        // Check that the parameter type can handle the argument.
        QualType ParamType = FDecl->getParamDecl(0)->getType();
        if (getAbsoluteValueKind(ArgType) == getAbsoluteValueKind(ParamType) &&
            S.Context.getTypeSize(ArgType) <=
                S.Context.getTypeSize(ParamType)) {
          // Found a function, don't need the header hint.
          EmitHeaderHint = false;
          break;
        }
      }
    }
  } else {
    FunctionName = S.Context.BuiltinInfo.GetName(AbsKind);
    HeaderName = S.Context.BuiltinInfo.getHeaderName(AbsKind);

    if (HeaderName) {
      DeclarationName DN(&S.Context.Idents.get(FunctionName));
      LookupResult R(S, DN, Loc, Sema::LookupAnyName);
      R.suppressDiagnostics();
      S.LookupName(R, S.getCurScope());

      if (R.isSingleResult()) {
        FunctionDecl *FD = dyn_cast<FunctionDecl>(R.getFoundDecl());
        if (FD && FD->getBuiltinID() == AbsKind) {
          EmitHeaderHint = false;
        } else {
          return;
        }
      } else if (!R.empty()) {
        return;
      }
    }
  }

  S.Diag(Loc, diag::note_replace_abs_function)
      << FunctionName << FixItHint::CreateReplacement(Range, FunctionName);

  if (!HeaderName)
    return;

  if (!EmitHeaderHint)
    return;

  S.Diag(Loc, diag::note_include_header_or_declare) << HeaderName
                                                    << FunctionName;
}

static bool IsFunctionStdAbs(const FunctionDecl *FDecl) {
  if (!FDecl)
    return false;

  if (!FDecl->getIdentifier() || !FDecl->getIdentifier()->isStr("abs"))
    return false;

  const NamespaceDecl *ND = dyn_cast<NamespaceDecl>(FDecl->getDeclContext());

  while (ND && ND->isInlineNamespace()) {
    ND = dyn_cast<NamespaceDecl>(ND->getDeclContext());
  }

  if (!ND || !ND->getIdentifier() || !ND->getIdentifier()->isStr("std"))
    return false;

  if (!isa<TranslationUnitDecl>(ND->getDeclContext()))
    return false;

  return true;
}

// Warn when using the wrong abs() function.
void Sema::CheckAbsoluteValueFunction(const CallExpr *Call,
                                      const FunctionDecl *FDecl,
                                      IdentifierInfo *FnInfo) {
  if (Call->getNumArgs() != 1)
    return;

  unsigned AbsKind = getAbsoluteValueFunctionKind(FDecl);
  bool IsStdAbs = IsFunctionStdAbs(FDecl);
  if (AbsKind == 0 && !IsStdAbs)
    return;

  QualType ArgType = Call->getArg(0)->IgnoreParenImpCasts()->getType();
  QualType ParamType = Call->getArg(0)->getType();

  // Unsigned types cannot be negative.  Suggest removing the absolute value
  // function call.
  if (ArgType->isUnsignedIntegerType()) {
    const char *FunctionName =
        IsStdAbs ? "std::abs" : Context.BuiltinInfo.GetName(AbsKind);
    Diag(Call->getExprLoc(), diag::warn_unsigned_abs) << ArgType << ParamType;
    Diag(Call->getExprLoc(), diag::note_remove_abs)
        << FunctionName
        << FixItHint::CreateRemoval(Call->getCallee()->getSourceRange());
    return;
  }

  // std::abs has overloads which prevent most of the absolute value problems
  // from occurring.
  if (IsStdAbs)
    return;

  AbsoluteValueKind ArgValueKind = getAbsoluteValueKind(ArgType);
  AbsoluteValueKind ParamValueKind = getAbsoluteValueKind(ParamType);

  // The argument and parameter are the same kind.  Check if they are the right
  // size.
  if (ArgValueKind == ParamValueKind) {
    if (Context.getTypeSize(ArgType) <= Context.getTypeSize(ParamType))
      return;

    unsigned NewAbsKind = getBestAbsFunction(Context, ArgType, AbsKind);
    Diag(Call->getExprLoc(), diag::warn_abs_too_small)
        << FDecl << ArgType << ParamType;

    if (NewAbsKind == 0)
      return;

    emitReplacement(*this, Call->getExprLoc(),
                    Call->getCallee()->getSourceRange(), NewAbsKind, ArgType);
    return;
  }

  // ArgValueKind != ParamValueKind
  // The wrong type of absolute value function was used.  Attempt to find the
  // proper one.
  unsigned NewAbsKind = changeAbsFunction(AbsKind, ArgValueKind);
  NewAbsKind = getBestAbsFunction(Context, ArgType, NewAbsKind);
  if (NewAbsKind == 0)
    return;

  Diag(Call->getExprLoc(), diag::warn_wrong_absolute_value_type)
      << FDecl << ParamValueKind << ArgValueKind;

  emitReplacement(*this, Call->getExprLoc(),
                  Call->getCallee()->getSourceRange(), NewAbsKind, ArgType);
  return;
}

//===--- CHECK: Standard memory functions ---------------------------------===//

/// \brief Takes the expression passed to the size_t parameter of functions
/// such as memcmp, strncat, etc and warns if it's a comparison.
///
/// This is to catch typos like `if (memcmp(&a, &b, sizeof(a) > 0))`.
static bool CheckMemorySizeofForComparison(Sema &S, const Expr *E,
                                           IdentifierInfo *FnName,
                                           SourceLocation FnLoc,
                                           SourceLocation RParenLoc) {
  const BinaryOperator *Size = dyn_cast<BinaryOperator>(E);
  if (!Size)
    return false;

  // if E is binop and op is >, <, >=, <=, ==, &&, ||:
  if (!Size->isComparisonOp() && !Size->isEqualityOp() && !Size->isLogicalOp())
    return false;

  SourceRange SizeRange = Size->getSourceRange();
  S.Diag(Size->getOperatorLoc(), diag::warn_memsize_comparison)
      << SizeRange << FnName;
  S.Diag(FnLoc, diag::note_memsize_comparison_paren)
      << FnName << FixItHint::CreateInsertion(
                       S.getLocForEndOfToken(Size->getLHS()->getLocEnd()), ")")
      << FixItHint::CreateRemoval(RParenLoc);
  S.Diag(SizeRange.getBegin(), diag::note_memsize_comparison_cast_silence)
      << FixItHint::CreateInsertion(SizeRange.getBegin(), "(size_t)(")
      << FixItHint::CreateInsertion(S.getLocForEndOfToken(SizeRange.getEnd()),
                                    ")");

  return true;
}

/// \brief Determine whether the given type is or contains a dynamic class type
/// (e.g., whether it has a vtable).
static const CXXRecordDecl *getContainedDynamicClass(QualType T,
                                                     bool &IsContained) {
  // Look through array types while ignoring qualifiers.
  const Type *Ty = T->getBaseElementTypeUnsafe();
  IsContained = false;

  const CXXRecordDecl *RD = Ty->getAsCXXRecordDecl();
  RD = RD ? RD->getDefinition() : nullptr;
  if (!RD)
    return nullptr;

  if (RD->isDynamicClass())
    return RD;

  // Check all the fields.  If any bases were dynamic, the class is dynamic.
  // It's impossible for a class to transitively contain itself by value, so
  // infinite recursion is impossible.
  for (auto *FD : RD->fields()) {
    bool SubContained;
    if (const CXXRecordDecl *ContainedRD =
            getContainedDynamicClass(FD->getType(), SubContained)) {
      IsContained = true;
      return ContainedRD;
    }
  }

  return nullptr;
}

/// \brief If E is a sizeof expression, returns its argument expression,
/// otherwise returns NULL.
static const Expr *getSizeOfExprArg(const Expr *E) {
  if (const UnaryExprOrTypeTraitExpr *SizeOf =
      dyn_cast<UnaryExprOrTypeTraitExpr>(E))
    if (SizeOf->getKind() == clang::UETT_SizeOf && !SizeOf->isArgumentType())
      return SizeOf->getArgumentExpr()->IgnoreParenImpCasts();

  return nullptr;
}

/// \brief If E is a sizeof expression, returns its argument type.
static QualType getSizeOfArgType(const Expr *E) {
  if (const UnaryExprOrTypeTraitExpr *SizeOf =
      dyn_cast<UnaryExprOrTypeTraitExpr>(E))
    if (SizeOf->getKind() == clang::UETT_SizeOf)
      return SizeOf->getTypeOfArgument();

  return QualType();
}

/// \brief Check for dangerous or invalid arguments to memset().
///
/// This issues warnings on known problematic, dangerous or unspecified
/// arguments to the standard 'memset', 'memcpy', 'memmove', and 'memcmp'
/// function calls.
///
/// \param Call The call expression to diagnose.
void Sema::CheckMemaccessArguments(const CallExpr *Call,
                                   unsigned BId,
                                   IdentifierInfo *FnName) {
  assert(BId != 0);

  // It is possible to have a non-standard definition of memset.  Validate
  // we have enough arguments, and if not, abort further checking.
  unsigned ExpectedNumArgs = (BId == Builtin::BIstrndup ? 2 : 3);
  if (Call->getNumArgs() < ExpectedNumArgs)
    return;

  unsigned LastArg = (BId == Builtin::BImemset ||
                      BId == Builtin::BIstrndup ? 1 : 2);
  unsigned LenArg = (BId == Builtin::BIstrndup ? 1 : 2);
  const Expr *LenExpr = Call->getArg(LenArg)->IgnoreParenImpCasts();

  if (CheckMemorySizeofForComparison(*this, LenExpr, FnName,
                                     Call->getLocStart(), Call->getRParenLoc()))
    return;

  // We have special checking when the length is a sizeof expression.
  QualType SizeOfArgTy = getSizeOfArgType(LenExpr);
  const Expr *SizeOfArg = getSizeOfExprArg(LenExpr);
  llvm::FoldingSetNodeID SizeOfArgID;

  for (unsigned ArgIdx = 0; ArgIdx != LastArg; ++ArgIdx) {
    const Expr *Dest = Call->getArg(ArgIdx)->IgnoreParenImpCasts();
    SourceRange ArgRange = Call->getArg(ArgIdx)->getSourceRange();

    QualType DestTy = Dest->getType();
    QualType PointeeTy;
    if (const PointerType *DestPtrTy = DestTy->getAs<PointerType>()) {
      PointeeTy = DestPtrTy->getPointeeType();

      // Never warn about void type pointers. This can be used to suppress
      // false positives.
      if (PointeeTy->isVoidType())
        continue;

      // Catch "memset(p, 0, sizeof(p))" -- needs to be sizeof(*p). Do this by
      // actually comparing the expressions for equality. Because computing the
      // expression IDs can be expensive, we only do this if the diagnostic is
      // enabled.
      if (SizeOfArg &&
          !Diags.isIgnored(diag::warn_sizeof_pointer_expr_memaccess,
                           SizeOfArg->getExprLoc())) {
        // We only compute IDs for expressions if the warning is enabled, and
        // cache the sizeof arg's ID.
        if (SizeOfArgID == llvm::FoldingSetNodeID())
          SizeOfArg->Profile(SizeOfArgID, Context, true);
        llvm::FoldingSetNodeID DestID;
        Dest->Profile(DestID, Context, true);
        if (DestID == SizeOfArgID) {
          // TODO: For strncpy() and friends, this could suggest sizeof(dst)
          //       over sizeof(src) as well.
          unsigned ActionIdx = 0; // Default is to suggest dereferencing.
          StringRef ReadableName = FnName->getName();

          if (const UnaryOperator *UnaryOp = dyn_cast<UnaryOperator>(Dest))
            if (UnaryOp->getOpcode() == UO_AddrOf)
              ActionIdx = 1; // If its an address-of operator, just remove it.
          if (!PointeeTy->isIncompleteType() &&
              (Context.getTypeSize(PointeeTy) == Context.getCharWidth()))
            ActionIdx = 2; // If the pointee's size is sizeof(char),
                           // suggest an explicit length.

          // If the function is defined as a builtin macro, do not show macro
          // expansion.
          SourceLocation SL = SizeOfArg->getExprLoc();
          SourceRange DSR = Dest->getSourceRange();
          SourceRange SSR = SizeOfArg->getSourceRange();
          SourceManager &SM = getSourceManager();

          if (SM.isMacroArgExpansion(SL)) {
            ReadableName = Lexer::getImmediateMacroName(SL, SM, LangOpts);
            SL = SM.getSpellingLoc(SL);
            DSR = SourceRange(SM.getSpellingLoc(DSR.getBegin()),
                             SM.getSpellingLoc(DSR.getEnd()));
            SSR = SourceRange(SM.getSpellingLoc(SSR.getBegin()),
                             SM.getSpellingLoc(SSR.getEnd()));
          }

          DiagRuntimeBehavior(SL, SizeOfArg,
                              PDiag(diag::warn_sizeof_pointer_expr_memaccess)
                                << ReadableName
                                << PointeeTy
                                << DestTy
                                << DSR
                                << SSR);
          DiagRuntimeBehavior(SL, SizeOfArg,
                         PDiag(diag::warn_sizeof_pointer_expr_memaccess_note)
                                << ActionIdx
                                << SSR);

          break;
        }
      }

      // Also check for cases where the sizeof argument is the exact same
      // type as the memory argument, and where it points to a user-defined
      // record type.
      if (SizeOfArgTy != QualType()) {
        if (PointeeTy->isRecordType() &&
            Context.typesAreCompatible(SizeOfArgTy, DestTy)) {
          DiagRuntimeBehavior(LenExpr->getExprLoc(), Dest,
                              PDiag(diag::warn_sizeof_pointer_type_memaccess)
                                << FnName << SizeOfArgTy << ArgIdx
                                << PointeeTy << Dest->getSourceRange()
                                << LenExpr->getSourceRange());
          break;
        }
      }
    } else if (DestTy->isArrayType()) {
      PointeeTy = DestTy;
    }

    if (PointeeTy == QualType())
      continue;

    // Always complain about dynamic classes.
    bool IsContained;
    if (const CXXRecordDecl *ContainedRD =
            getContainedDynamicClass(PointeeTy, IsContained)) {

      unsigned OperationType = 0;
      // "overwritten" if we're warning about the destination for any call
      // but memcmp; otherwise a verb appropriate to the call.
      if (ArgIdx != 0 || BId == Builtin::BImemcmp) {
        if (BId == Builtin::BImemcpy)
          OperationType = 1;
        else if(BId == Builtin::BImemmove)
          OperationType = 2;
        else if (BId == Builtin::BImemcmp)
          OperationType = 3;
      }
        
      DiagRuntimeBehavior(
        Dest->getExprLoc(), Dest,
        PDiag(diag::warn_dyn_class_memaccess)
          << (BId == Builtin::BImemcmp ? ArgIdx + 2 : ArgIdx)
          << FnName << IsContained << ContainedRD << OperationType
          << Call->getCallee()->getSourceRange());
    } else if (PointeeTy.hasNonTrivialObjCLifetime() &&
             BId != Builtin::BImemset)
      DiagRuntimeBehavior(
        Dest->getExprLoc(), Dest,
        PDiag(diag::warn_arc_object_memaccess)
          << ArgIdx << FnName << PointeeTy
          << Call->getCallee()->getSourceRange());
    else
      continue;

    DiagRuntimeBehavior(
      Dest->getExprLoc(), Dest,
      PDiag(diag::note_bad_memaccess_silence)
        << FixItHint::CreateInsertion(ArgRange.getBegin(), "(void*)"));
    break;
  }

}

// A little helper routine: ignore addition and subtraction of integer literals.
// This intentionally does not ignore all integer constant expressions because
// we don't want to remove sizeof().
static const Expr *ignoreLiteralAdditions(const Expr *Ex, ASTContext &Ctx) {
  Ex = Ex->IgnoreParenCasts();

  for (;;) {
    const BinaryOperator * BO = dyn_cast<BinaryOperator>(Ex);
    if (!BO || !BO->isAdditiveOp())
      break;

    const Expr *RHS = BO->getRHS()->IgnoreParenCasts();
    const Expr *LHS = BO->getLHS()->IgnoreParenCasts();
    
    if (isa<IntegerLiteral>(RHS))
      Ex = LHS;
    else if (isa<IntegerLiteral>(LHS))
      Ex = RHS;
    else
      break;
  }

  return Ex;
}

static bool isConstantSizeArrayWithMoreThanOneElement(QualType Ty,
                                                      ASTContext &Context) {
  // Only handle constant-sized or VLAs, but not flexible members.
  if (const ConstantArrayType *CAT = Context.getAsConstantArrayType(Ty)) {
    // Only issue the FIXIT for arrays of size > 1.
    if (CAT->getSize().getSExtValue() <= 1)
      return false;
  } else if (!Ty->isVariableArrayType()) {
    return false;
  }
  return true;
}

// Warn if the user has made the 'size' argument to strlcpy or strlcat
// be the size of the source, instead of the destination.
void Sema::CheckStrlcpycatArguments(const CallExpr *Call,
                                    IdentifierInfo *FnName) {

  // Don't crash if the user has the wrong number of arguments
  unsigned NumArgs = Call->getNumArgs();
  if ((NumArgs != 3) && (NumArgs != 4))
    return;

  const Expr *SrcArg = ignoreLiteralAdditions(Call->getArg(1), Context);
  const Expr *SizeArg = ignoreLiteralAdditions(Call->getArg(2), Context);
  const Expr *CompareWithSrc = nullptr;

  if (CheckMemorySizeofForComparison(*this, SizeArg, FnName,
                                     Call->getLocStart(), Call->getRParenLoc()))
    return;
  
  // Look for 'strlcpy(dst, x, sizeof(x))'
  if (const Expr *Ex = getSizeOfExprArg(SizeArg))
    CompareWithSrc = Ex;
  else {
    // Look for 'strlcpy(dst, x, strlen(x))'
    if (const CallExpr *SizeCall = dyn_cast<CallExpr>(SizeArg)) {
      if (SizeCall->getBuiltinCallee() == Builtin::BIstrlen &&
          SizeCall->getNumArgs() == 1)
        CompareWithSrc = ignoreLiteralAdditions(SizeCall->getArg(0), Context);
    }
  }

  if (!CompareWithSrc)
    return;

  // Determine if the argument to sizeof/strlen is equal to the source
  // argument.  In principle there's all kinds of things you could do
  // here, for instance creating an == expression and evaluating it with
  // EvaluateAsBooleanCondition, but this uses a more direct technique:
  const DeclRefExpr *SrcArgDRE = dyn_cast<DeclRefExpr>(SrcArg);
  if (!SrcArgDRE)
    return;
  
  const DeclRefExpr *CompareWithSrcDRE = dyn_cast<DeclRefExpr>(CompareWithSrc);
  if (!CompareWithSrcDRE || 
      SrcArgDRE->getDecl() != CompareWithSrcDRE->getDecl())
    return;
  
  const Expr *OriginalSizeArg = Call->getArg(2);
  Diag(CompareWithSrcDRE->getLocStart(), diag::warn_strlcpycat_wrong_size)
    << OriginalSizeArg->getSourceRange() << FnName;
  
  // Output a FIXIT hint if the destination is an array (rather than a
  // pointer to an array).  This could be enhanced to handle some
  // pointers if we know the actual size, like if DstArg is 'array+2'
  // we could say 'sizeof(array)-2'.
  const Expr *DstArg = Call->getArg(0)->IgnoreParenImpCasts();
  if (!isConstantSizeArrayWithMoreThanOneElement(DstArg->getType(), Context))
    return;

  SmallString<128> sizeString;
  llvm::raw_svector_ostream OS(sizeString);
  OS << "sizeof(";
  DstArg->printPretty(OS, nullptr, getPrintingPolicy());
  OS << ")";
  
  Diag(OriginalSizeArg->getLocStart(), diag::note_strlcpycat_wrong_size)
    << FixItHint::CreateReplacement(OriginalSizeArg->getSourceRange(),
                                    OS.str());
}

/// Check if two expressions refer to the same declaration.
static bool referToTheSameDecl(const Expr *E1, const Expr *E2) {
  if (const DeclRefExpr *D1 = dyn_cast_or_null<DeclRefExpr>(E1))
    if (const DeclRefExpr *D2 = dyn_cast_or_null<DeclRefExpr>(E2))
      return D1->getDecl() == D2->getDecl();
  return false;
}

static const Expr *getStrlenExprArg(const Expr *E) {
  if (const CallExpr *CE = dyn_cast<CallExpr>(E)) {
    const FunctionDecl *FD = CE->getDirectCallee();
    if (!FD || FD->getMemoryFunctionKind() != Builtin::BIstrlen)
      return nullptr;
    return CE->getArg(0)->IgnoreParenCasts();
  }
  return nullptr;
}

// Warn on anti-patterns as the 'size' argument to strncat.
// The correct size argument should look like following:
//   strncat(dst, src, sizeof(dst) - strlen(dest) - 1);
void Sema::CheckStrncatArguments(const CallExpr *CE,
                                 IdentifierInfo *FnName) {
  // Don't crash if the user has the wrong number of arguments.
  if (CE->getNumArgs() < 3)
    return;
  const Expr *DstArg = CE->getArg(0)->IgnoreParenCasts();
  const Expr *SrcArg = CE->getArg(1)->IgnoreParenCasts();
  const Expr *LenArg = CE->getArg(2)->IgnoreParenCasts();

  if (CheckMemorySizeofForComparison(*this, LenArg, FnName, CE->getLocStart(),
                                     CE->getRParenLoc()))
    return;

  // Identify common expressions, which are wrongly used as the size argument
  // to strncat and may lead to buffer overflows.
  unsigned PatternType = 0;
  if (const Expr *SizeOfArg = getSizeOfExprArg(LenArg)) {
    // - sizeof(dst)
    if (referToTheSameDecl(SizeOfArg, DstArg))
      PatternType = 1;
    // - sizeof(src)
    else if (referToTheSameDecl(SizeOfArg, SrcArg))
      PatternType = 2;
  } else if (const BinaryOperator *BE = dyn_cast<BinaryOperator>(LenArg)) {
    if (BE->getOpcode() == BO_Sub) {
      const Expr *L = BE->getLHS()->IgnoreParenCasts();
      const Expr *R = BE->getRHS()->IgnoreParenCasts();
      // - sizeof(dst) - strlen(dst)
      if (referToTheSameDecl(DstArg, getSizeOfExprArg(L)) &&
          referToTheSameDecl(DstArg, getStrlenExprArg(R)))
        PatternType = 1;
      // - sizeof(src) - (anything)
      else if (referToTheSameDecl(SrcArg, getSizeOfExprArg(L)))
        PatternType = 2;
    }
  }

  if (PatternType == 0)
    return;

  // Generate the diagnostic.
  SourceLocation SL = LenArg->getLocStart();
  SourceRange SR = LenArg->getSourceRange();
  SourceManager &SM = getSourceManager();

  // If the function is defined as a builtin macro, do not show macro expansion.
  if (SM.isMacroArgExpansion(SL)) {
    SL = SM.getSpellingLoc(SL);
    SR = SourceRange(SM.getSpellingLoc(SR.getBegin()),
                     SM.getSpellingLoc(SR.getEnd()));
  }

  // Check if the destination is an array (rather than a pointer to an array).
  QualType DstTy = DstArg->getType();
  bool isKnownSizeArray = isConstantSizeArrayWithMoreThanOneElement(DstTy,
                                                                    Context);
  if (!isKnownSizeArray) {
    if (PatternType == 1)
      Diag(SL, diag::warn_strncat_wrong_size) << SR;
    else
      Diag(SL, diag::warn_strncat_src_size) << SR;
    return;
  }

  if (PatternType == 1)
    Diag(SL, diag::warn_strncat_large_size) << SR;
  else
    Diag(SL, diag::warn_strncat_src_size) << SR;

  SmallString<128> sizeString;
  llvm::raw_svector_ostream OS(sizeString);
  OS << "sizeof(";
  DstArg->printPretty(OS, nullptr, getPrintingPolicy());
  OS << ") - ";
  OS << "strlen(";
  DstArg->printPretty(OS, nullptr, getPrintingPolicy());
  OS << ") - 1";

  Diag(SL, diag::note_strncat_wrong_size)
    << FixItHint::CreateReplacement(SR, OS.str());
}

//===--- CHECK: Return Address of Stack Variable --------------------------===//

static Expr *EvalVal(Expr *E, SmallVectorImpl<DeclRefExpr *> &refVars,
                     Decl *ParentDecl);
static Expr *EvalAddr(Expr* E, SmallVectorImpl<DeclRefExpr *> &refVars,
                      Decl *ParentDecl);

/// CheckReturnStackAddr - Check if a return statement returns the address
///   of a stack variable.
static void
CheckReturnStackAddr(Sema &S, Expr *RetValExp, QualType lhsType,
                     SourceLocation ReturnLoc) {

  Expr *stackE = nullptr;
  SmallVector<DeclRefExpr *, 8> refVars;

  // Perform checking for returned stack addresses, local blocks,
  // label addresses or references to temporaries.
  if (lhsType->isPointerType() ||
      (!S.getLangOpts().ObjCAutoRefCount && lhsType->isBlockPointerType())) {
    stackE = EvalAddr(RetValExp, refVars, /*ParentDecl=*/nullptr);
  } else if (lhsType->isReferenceType()) {
    stackE = EvalVal(RetValExp, refVars, /*ParentDecl=*/nullptr);
  }

  if (!stackE)
    return; // Nothing suspicious was found.

  SourceLocation diagLoc;
  SourceRange diagRange;
  if (refVars.empty()) {
    diagLoc = stackE->getLocStart();
    diagRange = stackE->getSourceRange();
  } else {
    // We followed through a reference variable. 'stackE' contains the
    // problematic expression but we will warn at the return statement pointing
    // at the reference variable. We will later display the "trail" of
    // reference variables using notes.
    diagLoc = refVars[0]->getLocStart();
    diagRange = refVars[0]->getSourceRange();
  }

  if (DeclRefExpr *DR = dyn_cast<DeclRefExpr>(stackE)) { //address of local var.
    S.Diag(diagLoc, lhsType->isReferenceType() ? diag::warn_ret_stack_ref
                                             : diag::warn_ret_stack_addr)
     << DR->getDecl()->getDeclName() << diagRange;
  } else if (isa<BlockExpr>(stackE)) { // local block.
    S.Diag(diagLoc, diag::err_ret_local_block) << diagRange;
  } else if (isa<AddrLabelExpr>(stackE)) { // address of label.
    S.Diag(diagLoc, diag::warn_ret_addr_label) << diagRange;
  } else { // local temporary.
    S.Diag(diagLoc, lhsType->isReferenceType() ? diag::warn_ret_local_temp_ref
                                               : diag::warn_ret_local_temp_addr)
     << diagRange;
  }

  // Display the "trail" of reference variables that we followed until we
  // found the problematic expression using notes.
  for (unsigned i = 0, e = refVars.size(); i != e; ++i) {
    VarDecl *VD = cast<VarDecl>(refVars[i]->getDecl());
    // If this var binds to another reference var, show the range of the next
    // var, otherwise the var binds to the problematic expression, in which case
    // show the range of the expression.
    SourceRange range = (i < e-1) ? refVars[i+1]->getSourceRange()
                                  : stackE->getSourceRange();
    S.Diag(VD->getLocation(), diag::note_ref_var_local_bind)
        << VD->getDeclName() << range;
  }
}

/// EvalAddr - EvalAddr and EvalVal are mutually recursive functions that
///  check if the expression in a return statement evaluates to an address
///  to a location on the stack, a local block, an address of a label, or a
///  reference to local temporary. The recursion is used to traverse the
///  AST of the return expression, with recursion backtracking when we
///  encounter a subexpression that (1) clearly does not lead to one of the
///  above problematic expressions (2) is something we cannot determine leads to
///  a problematic expression based on such local checking.
///
///  Both EvalAddr and EvalVal follow through reference variables to evaluate
///  the expression that they point to. Such variables are added to the
///  'refVars' vector so that we know what the reference variable "trail" was.
///
///  EvalAddr processes expressions that are pointers that are used as
///  references (and not L-values).  EvalVal handles all other values.
///  At the base case of the recursion is a check for the above problematic
///  expressions.
///
///  This implementation handles:
///
///   * pointer-to-pointer casts
///   * implicit conversions from array references to pointers
///   * taking the address of fields
///   * arbitrary interplay between "&" and "*" operators
///   * pointer arithmetic from an address of a stack variable
///   * taking the address of an array element where the array is on the stack
static Expr *EvalAddr(Expr *E, SmallVectorImpl<DeclRefExpr *> &refVars,
                      Decl *ParentDecl) {
  if (E->isTypeDependent())
    return nullptr;

  // We should only be called for evaluating pointer expressions.
  assert((E->getType()->isAnyPointerType() ||
          E->getType()->isBlockPointerType() ||
          E->getType()->isObjCQualifiedIdType()) &&
         "EvalAddr only works on pointers");

  E = E->IgnoreParens();

  // Our "symbolic interpreter" is just a dispatch off the currently
  // viewed AST node.  We then recursively traverse the AST by calling
  // EvalAddr and EvalVal appropriately.
  switch (E->getStmtClass()) {
  case Stmt::DeclRefExprClass: {
    DeclRefExpr *DR = cast<DeclRefExpr>(E);

    // If we leave the immediate function, the lifetime isn't about to end.
    if (DR->refersToEnclosingVariableOrCapture())
      return nullptr;

    if (VarDecl *V = dyn_cast<VarDecl>(DR->getDecl()))
      // If this is a reference variable, follow through to the expression that
      // it points to.
      if (V->hasLocalStorage() &&
          V->getType()->isReferenceType() && V->hasInit()) {
        // Add the reference variable to the "trail".
        refVars.push_back(DR);
        return EvalAddr(V->getInit(), refVars, ParentDecl);
      }

    return nullptr;
  }

  case Stmt::UnaryOperatorClass: {
    // The only unary operator that make sense to handle here
    // is AddrOf.  All others don't make sense as pointers.
    UnaryOperator *U = cast<UnaryOperator>(E);

    if (U->getOpcode() == UO_AddrOf)
      return EvalVal(U->getSubExpr(), refVars, ParentDecl);
    else
      return nullptr;
  }

  case Stmt::BinaryOperatorClass: {
    // Handle pointer arithmetic.  All other binary operators are not valid
    // in this context.
    BinaryOperator *B = cast<BinaryOperator>(E);
    BinaryOperatorKind op = B->getOpcode();

    if (op != BO_Add && op != BO_Sub)
      return nullptr;

    Expr *Base = B->getLHS();

    // Determine which argument is the real pointer base.  It could be
    // the RHS argument instead of the LHS.
    if (!Base->getType()->isPointerType()) Base = B->getRHS();

    assert (Base->getType()->isPointerType());
    return EvalAddr(Base, refVars, ParentDecl);
  }

  // For conditional operators we need to see if either the LHS or RHS are
  // valid DeclRefExpr*s.  If one of them is valid, we return it.
  case Stmt::ConditionalOperatorClass: {
    ConditionalOperator *C = cast<ConditionalOperator>(E);

    // Handle the GNU extension for missing LHS.
    // FIXME: That isn't a ConditionalOperator, so doesn't get here.
    if (Expr *LHSExpr = C->getLHS()) {
      // In C++, we can have a throw-expression, which has 'void' type.
      if (!LHSExpr->getType()->isVoidType())
        if (Expr *LHS = EvalAddr(LHSExpr, refVars, ParentDecl))
          return LHS;
    }

    // In C++, we can have a throw-expression, which has 'void' type.
    if (C->getRHS()->getType()->isVoidType())
      return nullptr;

    return EvalAddr(C->getRHS(), refVars, ParentDecl);
  }

  case Stmt::BlockExprClass:
    if (cast<BlockExpr>(E)->getBlockDecl()->hasCaptures())
      return E; // local block.
    return nullptr;

  case Stmt::AddrLabelExprClass:
    return E; // address of label.

  case Stmt::ExprWithCleanupsClass:
    return EvalAddr(cast<ExprWithCleanups>(E)->getSubExpr(), refVars,
                    ParentDecl);

  // For casts, we need to handle conversions from arrays to
  // pointer values, and pointer-to-pointer conversions.
  case Stmt::ImplicitCastExprClass:
  case Stmt::CStyleCastExprClass:
  case Stmt::CXXFunctionalCastExprClass:
  case Stmt::ObjCBridgedCastExprClass:
  case Stmt::CXXStaticCastExprClass:
  case Stmt::CXXDynamicCastExprClass:
  case Stmt::CXXConstCastExprClass:
  case Stmt::CXXReinterpretCastExprClass: {
    Expr* SubExpr = cast<CastExpr>(E)->getSubExpr();
    switch (cast<CastExpr>(E)->getCastKind()) {
    case CK_LValueToRValue:
    case CK_NoOp:
    case CK_BaseToDerived:
    case CK_DerivedToBase:
    case CK_UncheckedDerivedToBase:
    case CK_Dynamic:
    case CK_CPointerToObjCPointerCast:
    case CK_BlockPointerToObjCPointerCast:
    case CK_AnyPointerToBlockPointerCast:
      return EvalAddr(SubExpr, refVars, ParentDecl);

    case CK_ArrayToPointerDecay:
      return EvalVal(SubExpr, refVars, ParentDecl);

    case CK_BitCast:
      if (SubExpr->getType()->isAnyPointerType() ||
          SubExpr->getType()->isBlockPointerType() ||
          SubExpr->getType()->isObjCQualifiedIdType())
        return EvalAddr(SubExpr, refVars, ParentDecl);
      else
        return nullptr;

    default:
      return nullptr;
    }
  }

  case Stmt::MaterializeTemporaryExprClass:
    if (Expr *Result = EvalAddr(
                         cast<MaterializeTemporaryExpr>(E)->GetTemporaryExpr(),
                                refVars, ParentDecl))
      return Result;
      
    return E;
      
  // Everything else: we simply don't reason about them.
  default:
    return nullptr;
  }
}


///  EvalVal - This function is complements EvalAddr in the mutual recursion.
///   See the comments for EvalAddr for more details.
static Expr *EvalVal(Expr *E, SmallVectorImpl<DeclRefExpr *> &refVars,
                     Decl *ParentDecl) {
do {
  // We should only be called for evaluating non-pointer expressions, or
  // expressions with a pointer type that are not used as references but instead
  // are l-values (e.g., DeclRefExpr with a pointer type).

  // Our "symbolic interpreter" is just a dispatch off the currently
  // viewed AST node.  We then recursively traverse the AST by calling
  // EvalAddr and EvalVal appropriately.

  E = E->IgnoreParens();
  switch (E->getStmtClass()) {
  case Stmt::ImplicitCastExprClass: {
    ImplicitCastExpr *IE = cast<ImplicitCastExpr>(E);
    if (IE->getValueKind() == VK_LValue) {
      E = IE->getSubExpr();
      continue;
    }
    return nullptr;
  }

  case Stmt::ExprWithCleanupsClass:
    return EvalVal(cast<ExprWithCleanups>(E)->getSubExpr(), refVars,ParentDecl);

  case Stmt::DeclRefExprClass: {
    // When we hit a DeclRefExpr we are looking at code that refers to a
    // variable's name. If it's not a reference variable we check if it has
    // local storage within the function, and if so, return the expression.
    DeclRefExpr *DR = cast<DeclRefExpr>(E);

    // If we leave the immediate function, the lifetime isn't about to end.
    if (DR->refersToEnclosingVariableOrCapture())
      return nullptr;

    if (VarDecl *V = dyn_cast<VarDecl>(DR->getDecl())) {
      // Check if it refers to itself, e.g. "int& i = i;".
      if (V == ParentDecl)
        return DR;

      if (V->hasLocalStorage()) {
        if (!V->getType()->isReferenceType())
          return DR;

        // Reference variable, follow through to the expression that
        // it points to.
        if (V->hasInit()) {
          // Add the reference variable to the "trail".
          refVars.push_back(DR);
          return EvalVal(V->getInit(), refVars, V);
        }
      }
    }

    return nullptr;
  }

  case Stmt::UnaryOperatorClass: {
    // The only unary operator that make sense to handle here
    // is Deref.  All others don't resolve to a "name."  This includes
    // handling all sorts of rvalues passed to a unary operator.
    UnaryOperator *U = cast<UnaryOperator>(E);

    if (U->getOpcode() == UO_Deref)
      return EvalAddr(U->getSubExpr(), refVars, ParentDecl);

    return nullptr;
  }

  case Stmt::ArraySubscriptExprClass: {
    // Array subscripts are potential references to data on the stack.  We
    // retrieve the DeclRefExpr* for the array variable if it indeed
    // has local storage.
    return EvalAddr(cast<ArraySubscriptExpr>(E)->getBase(), refVars,ParentDecl);
  }

  case Stmt::ConditionalOperatorClass: {
    // For conditional operators we need to see if either the LHS or RHS are
    // non-NULL Expr's.  If one is non-NULL, we return it.
    ConditionalOperator *C = cast<ConditionalOperator>(E);

    // Handle the GNU extension for missing LHS.
    if (Expr *LHSExpr = C->getLHS()) {
      // In C++, we can have a throw-expression, which has 'void' type.
      if (!LHSExpr->getType()->isVoidType())
        if (Expr *LHS = EvalVal(LHSExpr, refVars, ParentDecl))
          return LHS;
    }

    // In C++, we can have a throw-expression, which has 'void' type.
    if (C->getRHS()->getType()->isVoidType())
      return nullptr;

    return EvalVal(C->getRHS(), refVars, ParentDecl);
  }

  // Accesses to members are potential references to data on the stack.
  case Stmt::MemberExprClass: {
    MemberExpr *M = cast<MemberExpr>(E);

    // Check for indirect access.  We only want direct field accesses.
    if (M->isArrow())
      return nullptr;

    // Check whether the member type is itself a reference, in which case
    // we're not going to refer to the member, but to what the member refers to.
    if (M->getMemberDecl()->getType()->isReferenceType())
      return nullptr;

    return EvalVal(M->getBase(), refVars, ParentDecl);
  }

  case Stmt::MaterializeTemporaryExprClass:
    if (Expr *Result = EvalVal(
                          cast<MaterializeTemporaryExpr>(E)->GetTemporaryExpr(),
                               refVars, ParentDecl))
      return Result;
      
    return E;

  default:
    // Check that we don't return or take the address of a reference to a
    // temporary. This is only useful in C++.
    if (!E->isTypeDependent() && E->isRValue())
      return E;

    // Everything else: we simply don't reason about them.
    return nullptr;
  }
} while (true);
}

void
Sema::CheckReturnValExpr(Expr *RetValExp, QualType lhsType,
                         SourceLocation ReturnLoc,
                         bool isObjCMethod,
                         const AttrVec *Attrs,
                         const FunctionDecl *FD) {
  CheckReturnStackAddr(*this, RetValExp, lhsType, ReturnLoc);

  // Check if the return value is null but should not be.
  if (((Attrs && hasSpecificAttr<ReturnsNonNullAttr>(*Attrs)) ||
       (!isObjCMethod && isNonNullType(Context, lhsType))) &&
      CheckNonNullExpr(*this, RetValExp))
    Diag(ReturnLoc, diag::warn_null_ret)
      << (isObjCMethod ? 1 : 0) << RetValExp->getSourceRange();

  // C++11 [basic.stc.dynamic.allocation]p4:
  //   If an allocation function declared with a non-throwing
  //   exception-specification fails to allocate storage, it shall return
  //   a null pointer. Any other allocation function that fails to allocate
  //   storage shall indicate failure only by throwing an exception [...]
  if (FD) {
    OverloadedOperatorKind Op = FD->getOverloadedOperator();
    if (Op == OO_New || Op == OO_Array_New) {
      const FunctionProtoType *Proto
        = FD->getType()->castAs<FunctionProtoType>();
      if (!Proto->isNothrow(Context, /*ResultIfDependent*/true) &&
          CheckNonNullExpr(*this, RetValExp))
        Diag(ReturnLoc, diag::warn_operator_new_returns_null)
          << FD << getLangOpts().CPlusPlus11;
    }
  }
}

//===--- CHECK: Floating-Point comparisons (-Wfloat-equal) ---------------===//

/// Check for comparisons of floating point operands using != and ==.
/// Issue a warning if these are no self-comparisons, as they are not likely
/// to do what the programmer intended.
void Sema::CheckFloatComparison(SourceLocation Loc, Expr* LHS, Expr *RHS) {
  Expr* LeftExprSansParen = LHS->IgnoreParenImpCasts();
  Expr* RightExprSansParen = RHS->IgnoreParenImpCasts();

  // Special case: check for x == x (which is OK).
  // Do not emit warnings for such cases.
  if (DeclRefExpr* DRL = dyn_cast<DeclRefExpr>(LeftExprSansParen))
    if (DeclRefExpr* DRR = dyn_cast<DeclRefExpr>(RightExprSansParen))
      if (DRL->getDecl() == DRR->getDecl())
        return;


  // Special case: check for comparisons against literals that can be exactly
  //  represented by APFloat.  In such cases, do not emit a warning.  This
  //  is a heuristic: often comparison against such literals are used to
  //  detect if a value in a variable has not changed.  This clearly can
  //  lead to false negatives.
  if (FloatingLiteral* FLL = dyn_cast<FloatingLiteral>(LeftExprSansParen)) {
    if (FLL->isExact())
      return;
  } else
    if (FloatingLiteral* FLR = dyn_cast<FloatingLiteral>(RightExprSansParen))
      if (FLR->isExact())
        return;

  // Check for comparisons with builtin types.
  if (CallExpr* CL = dyn_cast<CallExpr>(LeftExprSansParen))
    if (CL->getBuiltinCallee())
      return;

  if (CallExpr* CR = dyn_cast<CallExpr>(RightExprSansParen))
    if (CR->getBuiltinCallee())
      return;

  // Emit the diagnostic.
  Diag(Loc, diag::warn_floatingpoint_eq)
    << LHS->getSourceRange() << RHS->getSourceRange();
}

//===--- CHECK: Integer mixed-sign comparisons (-Wsign-compare) --------===//
//===--- CHECK: Lossy implicit conversions (-Wconversion) --------------===//

namespace {

/// Structure recording the 'active' range of an integer-valued
/// expression.
struct IntRange {
  /// The number of bits active in the int.
  unsigned Width;

  /// True if the int is known not to have negative values.
  bool NonNegative;

  IntRange(unsigned Width, bool NonNegative)
    : Width(Width), NonNegative(NonNegative)
  {}

  /// Returns the range of the bool type.
  static IntRange forBoolType() {
    return IntRange(1, true);
  }

  /// Returns the range of an opaque value of the given integral type.
  static IntRange forValueOfType(ASTContext &C, QualType T) {
    return forValueOfCanonicalType(C,
                          T->getCanonicalTypeInternal().getTypePtr());
  }

  /// Returns the range of an opaque value of a canonical integral type.
  static IntRange forValueOfCanonicalType(ASTContext &C, const Type *T) {
    assert(T->isCanonicalUnqualified());

    if (const VectorType *VT = dyn_cast<VectorType>(T))
      T = VT->getElementType().getTypePtr();
    if (const ComplexType *CT = dyn_cast<ComplexType>(T))
      T = CT->getElementType().getTypePtr();
    if (const AtomicType *AT = dyn_cast<AtomicType>(T))
      T = AT->getValueType().getTypePtr();

    // For enum types, use the known bit width of the enumerators.
    if (const EnumType *ET = dyn_cast<EnumType>(T)) {
      EnumDecl *Enum = ET->getDecl();
      if (!Enum->isCompleteDefinition())
        return IntRange(C.getIntWidth(QualType(T, 0)), false);

      unsigned NumPositive = Enum->getNumPositiveBits();
      unsigned NumNegative = Enum->getNumNegativeBits();

      if (NumNegative == 0)
        return IntRange(NumPositive, true/*NonNegative*/);
      else
        return IntRange(std::max(NumPositive + 1, NumNegative),
                        false/*NonNegative*/);
    }

    const BuiltinType *BT = cast<BuiltinType>(T);
    assert(BT->isInteger());

    return IntRange(C.getIntWidth(QualType(T, 0)), BT->isUnsignedInteger());
  }

  /// Returns the "target" range of a canonical integral type, i.e.
  /// the range of values expressible in the type.
  ///
  /// This matches forValueOfCanonicalType except that enums have the
  /// full range of their type, not the range of their enumerators.
  static IntRange forTargetOfCanonicalType(ASTContext &C, const Type *T) {
    assert(T->isCanonicalUnqualified());

    if (const VectorType *VT = dyn_cast<VectorType>(T))
      T = VT->getElementType().getTypePtr();
    if (const ComplexType *CT = dyn_cast<ComplexType>(T))
      T = CT->getElementType().getTypePtr();
    if (const AtomicType *AT = dyn_cast<AtomicType>(T))
      T = AT->getValueType().getTypePtr();
    if (const EnumType *ET = dyn_cast<EnumType>(T))
      T = C.getCanonicalType(ET->getDecl()->getIntegerType()).getTypePtr();

    const BuiltinType *BT = cast<BuiltinType>(T);
    assert(BT->isInteger());

    return IntRange(C.getIntWidth(QualType(T, 0)), BT->isUnsignedInteger());
  }

  /// Returns the supremum of two ranges: i.e. their conservative merge.
  static IntRange join(IntRange L, IntRange R) {
    return IntRange(std::max(L.Width, R.Width),
                    L.NonNegative && R.NonNegative);
  }

  /// Returns the infinum of two ranges: i.e. their aggressive merge.
  static IntRange meet(IntRange L, IntRange R) {
    return IntRange(std::min(L.Width, R.Width),
                    L.NonNegative || R.NonNegative);
  }
};

static IntRange GetValueRange(ASTContext &C, llvm::APSInt &value,
                              unsigned MaxWidth) {
  if (value.isSigned() && value.isNegative())
    return IntRange(value.getMinSignedBits(), false);

  if (value.getBitWidth() > MaxWidth)
    value = value.trunc(MaxWidth);

  // isNonNegative() just checks the sign bit without considering
  // signedness.
  return IntRange(value.getActiveBits(), true);
}

static IntRange GetValueRange(ASTContext &C, APValue &result, QualType Ty,
                              unsigned MaxWidth) {
  if (result.isInt())
    return GetValueRange(C, result.getInt(), MaxWidth);

  if (result.isVector()) {
    IntRange R = GetValueRange(C, result.getVectorElt(0), Ty, MaxWidth);
    for (unsigned i = 1, e = result.getVectorLength(); i != e; ++i) {
      IntRange El = GetValueRange(C, result.getVectorElt(i), Ty, MaxWidth);
      R = IntRange::join(R, El);
    }
    return R;
  }

  if (result.isComplexInt()) {
    IntRange R = GetValueRange(C, result.getComplexIntReal(), MaxWidth);
    IntRange I = GetValueRange(C, result.getComplexIntImag(), MaxWidth);
    return IntRange::join(R, I);
  }

  // This can happen with lossless casts to intptr_t of "based" lvalues.
  // Assume it might use arbitrary bits.
  // FIXME: The only reason we need to pass the type in here is to get
  // the sign right on this one case.  It would be nice if APValue
  // preserved this.
  assert(result.isLValue() || result.isAddrLabelDiff());
  return IntRange(MaxWidth, Ty->isUnsignedIntegerOrEnumerationType());
}

static QualType GetExprType(Expr *E) {
  QualType Ty = E->getType();
  if (const AtomicType *AtomicRHS = Ty->getAs<AtomicType>())
    Ty = AtomicRHS->getValueType();
  return Ty;
}

/// Pseudo-evaluate the given integer expression, estimating the
/// range of values it might take.
///
/// \param MaxWidth - the width to which the value will be truncated
static IntRange GetExprRange(ASTContext &C, Expr *E, unsigned MaxWidth) {
  E = E->IgnoreParens();

  // Try a full evaluation first.
  Expr::EvalResult result;
  if (E->EvaluateAsRValue(result, C))
    return GetValueRange(C, result.Val, GetExprType(E), MaxWidth);

  // I think we only want to look through implicit casts here; if the
  // user has an explicit widening cast, we should treat the value as
  // being of the new, wider type.
  if (ImplicitCastExpr *CE = dyn_cast<ImplicitCastExpr>(E)) {
    if (CE->getCastKind() == CK_NoOp || CE->getCastKind() == CK_LValueToRValue)
      return GetExprRange(C, CE->getSubExpr(), MaxWidth);

    IntRange OutputTypeRange = IntRange::forValueOfType(C, GetExprType(CE));

    bool isIntegerCast = (CE->getCastKind() == CK_IntegralCast);

    // Assume that non-integer casts can span the full range of the type.
    if (!isIntegerCast)
      return OutputTypeRange;

    IntRange SubRange
      = GetExprRange(C, CE->getSubExpr(),
                     std::min(MaxWidth, OutputTypeRange.Width));

    // Bail out if the subexpr's range is as wide as the cast type.
    if (SubRange.Width >= OutputTypeRange.Width)
      return OutputTypeRange;

    // Otherwise, we take the smaller width, and we're non-negative if
    // either the output type or the subexpr is.
    return IntRange(SubRange.Width,
                    SubRange.NonNegative || OutputTypeRange.NonNegative);
  }

  if (ConditionalOperator *CO = dyn_cast<ConditionalOperator>(E)) {
    // If we can fold the condition, just take that operand.
    bool CondResult;
    if (CO->getCond()->EvaluateAsBooleanCondition(CondResult, C))
      return GetExprRange(C, CondResult ? CO->getTrueExpr()
                                        : CO->getFalseExpr(),
                          MaxWidth);

    // Otherwise, conservatively merge.
    IntRange L = GetExprRange(C, CO->getTrueExpr(), MaxWidth);
    IntRange R = GetExprRange(C, CO->getFalseExpr(), MaxWidth);
    return IntRange::join(L, R);
  }

  if (BinaryOperator *BO = dyn_cast<BinaryOperator>(E)) {
    switch (BO->getOpcode()) {

    // Boolean-valued operations are single-bit and positive.
    case BO_LAnd:
    case BO_LOr:
    case BO_LT:
    case BO_GT:
    case BO_LE:
    case BO_GE:
    case BO_EQ:
    case BO_NE:
      return IntRange::forBoolType();

    // The type of the assignments is the type of the LHS, so the RHS
    // is not necessarily the same type.
    case BO_MulAssign:
    case BO_DivAssign:
    case BO_RemAssign:
    case BO_AddAssign:
    case BO_SubAssign:
    case BO_XorAssign:
    case BO_OrAssign:
      // TODO: bitfields?
      return IntRange::forValueOfType(C, GetExprType(E));

    // Simple assignments just pass through the RHS, which will have
    // been coerced to the LHS type.
    case BO_Assign:
      // TODO: bitfields?
      return GetExprRange(C, BO->getRHS(), MaxWidth);

    // Operations with opaque sources are black-listed.
    case BO_PtrMemD:
    case BO_PtrMemI:
      return IntRange::forValueOfType(C, GetExprType(E));

    // Bitwise-and uses the *infinum* of the two source ranges.
    case BO_And:
    case BO_AndAssign:
      return IntRange::meet(GetExprRange(C, BO->getLHS(), MaxWidth),
                            GetExprRange(C, BO->getRHS(), MaxWidth));

    // Left shift gets black-listed based on a judgement call.
    case BO_Shl:
      // ...except that we want to treat '1 << (blah)' as logically
      // positive.  It's an important idiom.
      if (IntegerLiteral *I
            = dyn_cast<IntegerLiteral>(BO->getLHS()->IgnoreParenCasts())) {
        if (I->getValue() == 1) {
          IntRange R = IntRange::forValueOfType(C, GetExprType(E));
          return IntRange(R.Width, /*NonNegative*/ true);
        }
      }
      LLVM_FALLTHROUGH; // HLSL Change

    case BO_ShlAssign:
      return IntRange::forValueOfType(C, GetExprType(E));

    // Right shift by a constant can narrow its left argument.
    case BO_Shr:
    case BO_ShrAssign: {
      IntRange L = GetExprRange(C, BO->getLHS(), MaxWidth);

      // If the shift amount is a positive constant, drop the width by
      // that much.
      llvm::APSInt shift;
      if (BO->getRHS()->isIntegerConstantExpr(shift, C) &&
          shift.isNonNegative()) {
        unsigned zext = shift.getZExtValue();
        if (zext >= L.Width)
          L.Width = (L.NonNegative ? 0 : 1);
        else
          L.Width -= zext;
      }

      return L;
    }

    // Comma acts as its right operand.
    case BO_Comma:
      return GetExprRange(C, BO->getRHS(), MaxWidth);

    // Black-list pointer subtractions.
    case BO_Sub:
      if (BO->getLHS()->getType()->isPointerType())
        return IntRange::forValueOfType(C, GetExprType(E));
      break;

    // The width of a division result is mostly determined by the size
    // of the LHS.
    case BO_Div: {
      // Don't 'pre-truncate' the operands.
      unsigned opWidth = C.getIntWidth(GetExprType(E));
      IntRange L = GetExprRange(C, BO->getLHS(), opWidth);

      // If the divisor is constant, use that.
      llvm::APSInt divisor;
      if (BO->getRHS()->isIntegerConstantExpr(divisor, C)) {
        unsigned log2 = divisor.logBase2(); // floor(log_2(divisor))
        if (log2 >= L.Width)
          L.Width = (L.NonNegative ? 0 : 1);
        else
          L.Width = std::min(L.Width - log2, MaxWidth);
        return L;
      }

      // Otherwise, just use the LHS's width.
      IntRange R = GetExprRange(C, BO->getRHS(), opWidth);
      return IntRange(L.Width, L.NonNegative && R.NonNegative);
    }

    // The result of a remainder can't be larger than the result of
    // either side.
    case BO_Rem: {
      // Don't 'pre-truncate' the operands.
      unsigned opWidth = C.getIntWidth(GetExprType(E));
      IntRange L = GetExprRange(C, BO->getLHS(), opWidth);
      IntRange R = GetExprRange(C, BO->getRHS(), opWidth);

      IntRange meet = IntRange::meet(L, R);
      meet.Width = std::min(meet.Width, MaxWidth);
      return meet;
    }

    // The default behavior is okay for these.
    case BO_Mul:
    case BO_Add:
    case BO_Xor:
    case BO_Or:
      break;
    }

    // The default case is to treat the operation as if it were closed
    // on the narrowest type that encompasses both operands.
    IntRange L = GetExprRange(C, BO->getLHS(), MaxWidth);
    IntRange R = GetExprRange(C, BO->getRHS(), MaxWidth);
    return IntRange::join(L, R);
  }

  if (UnaryOperator *UO = dyn_cast<UnaryOperator>(E)) {
    switch (UO->getOpcode()) {
    // Boolean-valued operations are white-listed.
    case UO_LNot:
      return IntRange::forBoolType();

    // Operations with opaque sources are black-listed.
    case UO_Deref:
    case UO_AddrOf: // should be impossible
      return IntRange::forValueOfType(C, GetExprType(E));

    default:
      return GetExprRange(C, UO->getSubExpr(), MaxWidth);
    }
  }

  if (OpaqueValueExpr *OVE = dyn_cast<OpaqueValueExpr>(E))
    return GetExprRange(C, OVE->getSourceExpr(), MaxWidth);

  if (FieldDecl *BitField = E->getSourceBitField())
    return IntRange(BitField->getBitWidthValue(C),
                    BitField->getType()->isUnsignedIntegerOrEnumerationType());

  return IntRange::forValueOfType(C, GetExprType(E));
}

static IntRange GetExprRange(ASTContext &C, Expr *E) {
  return GetExprRange(C, E, C.getIntWidth(GetExprType(E)));
}

/// Checks whether the given value, which currently has the given
/// source semantics, has the same value when coerced through the
/// target semantics.
static bool IsSameFloatAfterCast(const llvm::APFloat &value,
                                 const llvm::fltSemantics &Src,
                                 const llvm::fltSemantics &Tgt) {
  llvm::APFloat truncated = value;

  bool ignored;
  truncated.convert(Src, llvm::APFloat::rmNearestTiesToEven, &ignored);
  truncated.convert(Tgt, llvm::APFloat::rmNearestTiesToEven, &ignored);

  return truncated.bitwiseIsEqual(value);
}

/// Checks whether the given value, which currently has the given
/// source semantics, has the same value when coerced through the
/// target semantics.
///
/// The value might be a vector of floats (or a complex number).
static bool IsSameFloatAfterCast(const APValue &value,
                                 const llvm::fltSemantics &Src,
                                 const llvm::fltSemantics &Tgt) {
  if (value.isFloat())
    return IsSameFloatAfterCast(value.getFloat(), Src, Tgt);

  if (value.isVector()) {
    for (unsigned i = 0, e = value.getVectorLength(); i != e; ++i)
      if (!IsSameFloatAfterCast(value.getVectorElt(i), Src, Tgt))
        return false;
    return true;
  }

  assert(value.isComplexFloat());
  return (IsSameFloatAfterCast(value.getComplexFloatReal(), Src, Tgt) &&
          IsSameFloatAfterCast(value.getComplexFloatImag(), Src, Tgt));
}

static void AnalyzeImplicitConversions(Sema &S, Expr *E, SourceLocation CC);

static bool IsZero(Sema &S, Expr *E) {
  // Suppress cases where we are comparing against an enum constant.
  if (const DeclRefExpr *DR =
      dyn_cast<DeclRefExpr>(E->IgnoreParenImpCasts()))
    if (isa<EnumConstantDecl>(DR->getDecl()))
      return false;

  // Suppress cases where the '0' value is expanded from a macro.
  if (E->getLocStart().isMacroID())
    return false;

  llvm::APSInt Value;
  return E->isIntegerConstantExpr(Value, S.Context) && Value == 0;
}

static bool HasEnumType(Expr *E) {
  // Strip off implicit integral promotions.
  while (ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(E)) {
    if (ICE->getCastKind() != CK_IntegralCast &&
        ICE->getCastKind() != CK_NoOp)
      break;
    E = ICE->getSubExpr();
  }

  return E->getType()->isEnumeralType();
}

static void CheckTrivialUnsignedComparison(Sema &S, BinaryOperator *E) {
  // Disable warning in template instantiations.
  if (!S.ActiveTemplateInstantiations.empty())
    return;

  BinaryOperatorKind op = E->getOpcode();
  if (E->isValueDependent())
    return;

  if (op == BO_LT && IsZero(S, E->getRHS())) {
    S.Diag(E->getOperatorLoc(), diag::warn_lunsigned_always_true_comparison)
      << "< 0" << "false" << HasEnumType(E->getLHS())
      << E->getLHS()->getSourceRange() << E->getRHS()->getSourceRange();
  } else if (op == BO_GE && IsZero(S, E->getRHS())) {
    S.Diag(E->getOperatorLoc(), diag::warn_lunsigned_always_true_comparison)
      << ">= 0" << "true" << HasEnumType(E->getLHS())
      << E->getLHS()->getSourceRange() << E->getRHS()->getSourceRange();
  } else if (op == BO_GT && IsZero(S, E->getLHS())) {
    S.Diag(E->getOperatorLoc(), diag::warn_runsigned_always_true_comparison)
      << "0 >" << "false" << HasEnumType(E->getRHS())
      << E->getLHS()->getSourceRange() << E->getRHS()->getSourceRange();
  } else if (op == BO_LE && IsZero(S, E->getLHS())) {
    S.Diag(E->getOperatorLoc(), diag::warn_runsigned_always_true_comparison)
      << "0 <=" << "true" << HasEnumType(E->getRHS())
      << E->getLHS()->getSourceRange() << E->getRHS()->getSourceRange();
  }
}

static void DiagnoseOutOfRangeComparison(Sema &S, BinaryOperator *E,
                                         Expr *Constant, Expr *Other,
                                         llvm::APSInt Value,
                                         bool RhsConstant) {
  // Disable warning in template instantiations.
  if (!S.ActiveTemplateInstantiations.empty())
    return;

  // TODO: Investigate using GetExprRange() to get tighter bounds
  // on the bit ranges.
  QualType OtherT = Other->getType();
  if (const auto *AT = OtherT->getAs<AtomicType>())
    OtherT = AT->getValueType();
  IntRange OtherRange = IntRange::forValueOfType(S.Context, OtherT);
  unsigned OtherWidth = OtherRange.Width;

  bool OtherIsBooleanType = Other->isKnownToHaveBooleanValue();

  // 0 values are handled later by CheckTrivialUnsignedComparison().
  if ((Value == 0) && (!OtherIsBooleanType))
    return;

  BinaryOperatorKind op = E->getOpcode();
  bool IsTrue = true;

  // Used for diagnostic printout.
  enum {
    LiteralConstant = 0,
    CXXBoolLiteralTrue,
    CXXBoolLiteralFalse
  } LiteralOrBoolConstant = LiteralConstant;

  if (!OtherIsBooleanType) {
    QualType ConstantT = Constant->getType();
    QualType CommonT = E->getLHS()->getType();

    if (S.Context.hasSameUnqualifiedType(OtherT, ConstantT))
      return;
    assert((OtherT->isIntegerType() && ConstantT->isIntegerType()) &&
           "comparison with non-integer type");

    bool ConstantSigned = ConstantT->isSignedIntegerType();
    bool CommonSigned = CommonT->isSignedIntegerType();

    bool EqualityOnly = false;

    if (CommonSigned) {
      // The common type is signed, therefore no signed to unsigned conversion.
      if (!OtherRange.NonNegative) {
        // Check that the constant is representable in type OtherT.
        if (ConstantSigned) {
          if (OtherWidth >= Value.getMinSignedBits())
            return;
        } else { // !ConstantSigned
          if (OtherWidth >= Value.getActiveBits() + 1)
            return;
        }
      } else { // !OtherSigned
               // Check that the constant is representable in type OtherT.
        // Negative values are out of range.
        if (ConstantSigned) {
          if (Value.isNonNegative() && OtherWidth >= Value.getActiveBits())
            return;
        } else { // !ConstantSigned
          if (OtherWidth >= Value.getActiveBits())
            return;
        }
      }
    } else { // !CommonSigned
      if (OtherRange.NonNegative) {
        if (OtherWidth >= Value.getActiveBits())
          return;
      } else { // OtherSigned
        assert(!ConstantSigned &&
               "Two signed types converted to unsigned types.");
        // Check to see if the constant is representable in OtherT.
        if (OtherWidth > Value.getActiveBits())
          return;
        // Check to see if the constant is equivalent to a negative value
        // cast to CommonT.
        if (S.Context.getIntWidth(ConstantT) ==
                S.Context.getIntWidth(CommonT) &&
            Value.isNegative() && Value.getMinSignedBits() <= OtherWidth)
          return;
        // The constant value rests between values that OtherT can represent
        // after conversion.  Relational comparison still works, but equality
        // comparisons will be tautological.
        EqualityOnly = true;
      }
    }

    bool PositiveConstant = !ConstantSigned || Value.isNonNegative();

    if (op == BO_EQ || op == BO_NE) {
      IsTrue = op == BO_NE;
    } else if (EqualityOnly) {
      return;
    } else if (RhsConstant) {
      if (op == BO_GT || op == BO_GE)
        IsTrue = !PositiveConstant;
      else // op == BO_LT || op == BO_LE
        IsTrue = PositiveConstant;
    } else {
      if (op == BO_LT || op == BO_LE)
        IsTrue = !PositiveConstant;
      else // op == BO_GT || op == BO_GE
        IsTrue = PositiveConstant;
    }
  } else {
    // Other isKnownToHaveBooleanValue
    enum CompareBoolWithConstantResult { AFals, ATrue, Unkwn };
    enum ConstantValue { LT_Zero, Zero, One, GT_One, SizeOfConstVal };
    enum ConstantSide { Lhs, Rhs, SizeOfConstSides };

    static const struct LinkedConditions {
      CompareBoolWithConstantResult BO_LT_OP[SizeOfConstSides][SizeOfConstVal];
      CompareBoolWithConstantResult BO_GT_OP[SizeOfConstSides][SizeOfConstVal];
      CompareBoolWithConstantResult BO_LE_OP[SizeOfConstSides][SizeOfConstVal];
      CompareBoolWithConstantResult BO_GE_OP[SizeOfConstSides][SizeOfConstVal];
      CompareBoolWithConstantResult BO_EQ_OP[SizeOfConstSides][SizeOfConstVal];
      CompareBoolWithConstantResult BO_NE_OP[SizeOfConstSides][SizeOfConstVal];

    } TruthTable = {
        // Constant on LHS.              | Constant on RHS.              |
        // LT_Zero| Zero  | One   |GT_One| LT_Zero| Zero  | One   |GT_One|
        { { ATrue, Unkwn, AFals, AFals }, { AFals, AFals, Unkwn, ATrue } },
        { { AFals, AFals, Unkwn, ATrue }, { ATrue, Unkwn, AFals, AFals } },
        { { ATrue, ATrue, Unkwn, AFals }, { AFals, Unkwn, ATrue, ATrue } },
        { { AFals, Unkwn, ATrue, ATrue }, { ATrue, ATrue, Unkwn, AFals } },
        { { AFals, Unkwn, Unkwn, AFals }, { AFals, Unkwn, Unkwn, AFals } },
        { { ATrue, Unkwn, Unkwn, ATrue }, { ATrue, Unkwn, Unkwn, ATrue } }
      };

    bool ConstantIsBoolLiteral = isa<CXXBoolLiteralExpr>(Constant);

    enum ConstantValue ConstVal = Zero;
    if (Value.isUnsigned() || Value.isNonNegative()) {
      if (Value == 0) {
        LiteralOrBoolConstant =
            ConstantIsBoolLiteral ? CXXBoolLiteralFalse : LiteralConstant;
        ConstVal = Zero;
      } else if (Value == 1) {
        LiteralOrBoolConstant =
            ConstantIsBoolLiteral ? CXXBoolLiteralTrue : LiteralConstant;
        ConstVal = One;
      } else {
        LiteralOrBoolConstant = LiteralConstant;
        ConstVal = GT_One;
      }
    } else {
      ConstVal = LT_Zero;
    }

    CompareBoolWithConstantResult CmpRes;

    switch (op) {
    case BO_LT:
      CmpRes = TruthTable.BO_LT_OP[RhsConstant][ConstVal];
      break;
    case BO_GT:
      CmpRes = TruthTable.BO_GT_OP[RhsConstant][ConstVal];
      break;
    case BO_LE:
      CmpRes = TruthTable.BO_LE_OP[RhsConstant][ConstVal];
      break;
    case BO_GE:
      CmpRes = TruthTable.BO_GE_OP[RhsConstant][ConstVal];
      break;
    case BO_EQ:
      CmpRes = TruthTable.BO_EQ_OP[RhsConstant][ConstVal];
      break;
    case BO_NE:
      CmpRes = TruthTable.BO_NE_OP[RhsConstant][ConstVal];
      break;
    default:
      CmpRes = Unkwn;
      break;
    }

    if (CmpRes == AFals) {
      IsTrue = false;
    } else if (CmpRes == ATrue) {
      IsTrue = true;
    } else {
      return;
    }
  }

  // If this is a comparison to an enum constant, include that
  // constant in the diagnostic.
  const EnumConstantDecl *ED = nullptr;
  if (const DeclRefExpr *DR = dyn_cast<DeclRefExpr>(Constant))
    ED = dyn_cast<EnumConstantDecl>(DR->getDecl());

  SmallString<64> PrettySourceValue;
  llvm::raw_svector_ostream OS(PrettySourceValue);
  if (ED)
    OS << '\'' << *ED << "' (" << Value << ")";
  else
    OS << Value;

  S.DiagRuntimeBehavior(
    E->getOperatorLoc(), E,
    S.PDiag(diag::warn_out_of_range_compare)
        << OS.str() << LiteralOrBoolConstant
        << OtherT << (OtherIsBooleanType && !OtherT->isBooleanType()) << IsTrue
        << E->getLHS()->getSourceRange() << E->getRHS()->getSourceRange());
}

/// Analyze the operands of the given comparison.  Implements the
/// fallback case from AnalyzeComparison.
static void AnalyzeImpConvsInComparison(Sema &S, BinaryOperator *E) {
  AnalyzeImplicitConversions(S, E->getLHS(), E->getOperatorLoc());
  AnalyzeImplicitConversions(S, E->getRHS(), E->getOperatorLoc());
}

/// \brief Implements -Wsign-compare.
///
/// \param E the binary operator to check for warnings
static void AnalyzeComparison(Sema &S, BinaryOperator *E) {
  // The type the comparison is being performed in.
  QualType T = E->getLHS()->getType();

  // HLSL Change Starts
  // HLSL has a different set of standard conversions allowed. Until
  // the warning analysis is updated, simply skip this step.
  if (S.getLangOpts().HLSL) {
    return;
  }
  // HLSL Change Ends

  // Only analyze comparison operators where both sides have been converted to
  // the same type.
  if (!S.Context.hasSameUnqualifiedType(T, E->getRHS()->getType()))
    return AnalyzeImpConvsInComparison(S, E);

  // Don't analyze value-dependent comparisons directly.
  if (E->isValueDependent())
    return AnalyzeImpConvsInComparison(S, E);

  Expr *LHS = E->getLHS()->IgnoreParenImpCasts();
  Expr *RHS = E->getRHS()->IgnoreParenImpCasts();
  
  bool IsComparisonConstant = false;
  
  // Check whether an integer constant comparison results in a value
  // of 'true' or 'false'.
  if (T->isIntegralType(S.Context)) {
    llvm::APSInt RHSValue;
    bool IsRHSIntegralLiteral = 
      RHS->isIntegerConstantExpr(RHSValue, S.Context);
    llvm::APSInt LHSValue;
    bool IsLHSIntegralLiteral = 
      LHS->isIntegerConstantExpr(LHSValue, S.Context);
    if (IsRHSIntegralLiteral && !IsLHSIntegralLiteral)
        DiagnoseOutOfRangeComparison(S, E, RHS, LHS, RHSValue, true);
    else if (!IsRHSIntegralLiteral && IsLHSIntegralLiteral)
      DiagnoseOutOfRangeComparison(S, E, LHS, RHS, LHSValue, false);
    else
      IsComparisonConstant = 
        (IsRHSIntegralLiteral && IsLHSIntegralLiteral);
  } else if (!T->hasUnsignedIntegerRepresentation())
      IsComparisonConstant = E->isIntegerConstantExpr(S.Context);
  
  // We don't do anything special if this isn't an unsigned integral
  // comparison:  we're only interested in integral comparisons, and
  // signed comparisons only happen in cases we don't care to warn about.
  //
  // We also don't care about value-dependent expressions or expressions
  // whose result is a constant.
  if (!T->hasUnsignedIntegerRepresentation() || IsComparisonConstant)
    return AnalyzeImpConvsInComparison(S, E);
  
  // Check to see if one of the (unmodified) operands is of different
  // signedness.
  Expr *signedOperand, *unsignedOperand;
  if (LHS->getType()->hasSignedIntegerRepresentation()) {
    assert(!RHS->getType()->hasSignedIntegerRepresentation() &&
           "unsigned comparison between two signed integer expressions?");
    signedOperand = LHS;
    unsignedOperand = RHS;
  } else if (RHS->getType()->hasSignedIntegerRepresentation()) {
    signedOperand = RHS;
    unsignedOperand = LHS;
  } else {
    CheckTrivialUnsignedComparison(S, E);
    return AnalyzeImpConvsInComparison(S, E);
  }

  // Otherwise, calculate the effective range of the signed operand.
  IntRange signedRange = GetExprRange(S.Context, signedOperand);

  // Go ahead and analyze implicit conversions in the operands.  Note
  // that we skip the implicit conversions on both sides.
  AnalyzeImplicitConversions(S, LHS, E->getOperatorLoc());
  AnalyzeImplicitConversions(S, RHS, E->getOperatorLoc());

  // If the signed range is non-negative, -Wsign-compare won't fire,
  // but we should still check for comparisons which are always true
  // or false.
  if (signedRange.NonNegative)
    return CheckTrivialUnsignedComparison(S, E);

  // For (in)equality comparisons, if the unsigned operand is a
  // constant which cannot collide with a overflowed signed operand,
  // then reinterpreting the signed operand as unsigned will not
  // change the result of the comparison.
  if (E->isEqualityOp()) {
    unsigned comparisonWidth = S.Context.getIntWidth(T);
    IntRange unsignedRange = GetExprRange(S.Context, unsignedOperand);

    // We should never be unable to prove that the unsigned operand is
    // non-negative.
    assert(unsignedRange.NonNegative && "unsigned range includes negative?");

    if (unsignedRange.Width < comparisonWidth)
      return;
  }

  S.DiagRuntimeBehavior(E->getOperatorLoc(), E,
    S.PDiag(diag::warn_mixed_sign_comparison)
      << LHS->getType() << RHS->getType()
      << LHS->getSourceRange() << RHS->getSourceRange());
}

/// Analyzes an attempt to assign the given value to a bitfield.
///
/// Returns true if there was something fishy about the attempt.
static bool AnalyzeBitFieldAssignment(Sema &S, FieldDecl *Bitfield, Expr *Init,
                                      SourceLocation InitLoc) {
  assert(Bitfield->isBitField());
  if (Bitfield->isInvalidDecl())
    return false;

  // White-list bool bitfields.
  if (Bitfield->getType()->isBooleanType())
    return false;

  // Ignore value- or type-dependent expressions.
  if (Bitfield->getBitWidth()->isValueDependent() ||
      Bitfield->getBitWidth()->isTypeDependent() ||
      Init->isValueDependent() ||
      Init->isTypeDependent())
    return false;

  Expr *OriginalInit = Init->IgnoreParenImpCasts();

  llvm::APSInt Value;
  if (!OriginalInit->EvaluateAsInt(Value, S.Context, Expr::SE_AllowSideEffects))
    return false;

  unsigned OriginalWidth = Value.getBitWidth();
  unsigned FieldWidth = Bitfield->getBitWidthValue(S.Context);

  if (OriginalWidth <= FieldWidth)
    return false;

  // Compute the value which the bitfield will contain.
  llvm::APSInt TruncatedValue = Value.trunc(FieldWidth);
  TruncatedValue.setIsSigned(Bitfield->getType()->isSignedIntegerType());

  // Check whether the stored value is equal to the original value.
  TruncatedValue = TruncatedValue.extend(OriginalWidth);
  if (llvm::APSInt::isSameValue(Value, TruncatedValue))
    return false;

  // Special-case bitfields of width 1: booleans are naturally 0/1, and
  // therefore don't strictly fit into a signed bitfield of width 1.
  if (FieldWidth == 1 && Value == 1)
    return false;

  std::string PrettyValue = Value.toString(10);
  std::string PrettyTrunc = TruncatedValue.toString(10);

  S.Diag(InitLoc, diag::warn_impcast_bitfield_precision_constant)
    << PrettyValue << PrettyTrunc << OriginalInit->getType()
    << Init->getSourceRange();

  return true;
}

/// Analyze the given simple or compound assignment for warning-worthy
/// operations.
static void AnalyzeAssignment(Sema &S, BinaryOperator *E) {
  // Just recurse on the LHS.
  AnalyzeImplicitConversions(S, E->getLHS(), E->getOperatorLoc());

  S.DiagnoseGloballyCoherentMismatch(E->getRHS(), E->getLHS()->getType(),
                                     E->getOperatorLoc());

  // We want to recurse on the RHS as normal unless we're assigning to
  // a bitfield.
  if (FieldDecl *Bitfield = E->getLHS()->getSourceBitField()) {
    if (AnalyzeBitFieldAssignment(S, Bitfield, E->getRHS(),
                                  E->getOperatorLoc())) {
      // Recurse, ignoring any implicit conversions on the RHS.
      return AnalyzeImplicitConversions(S, E->getRHS()->IgnoreParenImpCasts(),
                                        E->getOperatorLoc());
    }
  }

  AnalyzeImplicitConversions(S, E->getRHS(), E->getOperatorLoc());
}

/// Diagnose an implicit cast;  purely a helper for CheckImplicitConversion.
static void DiagnoseImpCast(Sema &S, Expr *E, QualType SourceType, QualType T, 
                            SourceLocation CContext, unsigned diag,
                            bool pruneControlFlow = false) {
  if (pruneControlFlow) {
    S.DiagRuntimeBehavior(E->getExprLoc(), E,
                          S.PDiag(diag)
                            << SourceType << T << E->getSourceRange()
                            << SourceRange(CContext));
    return;
  }
  S.Diag(E->getExprLoc(), diag)
    << SourceType << T << E->getSourceRange() << SourceRange(CContext);
}

/// Diagnose an implicit cast;  purely a helper for CheckImplicitConversion.
static void DiagnoseImpCast(Sema &S, Expr *E, QualType T,
                            SourceLocation CContext, unsigned diag,
                            bool pruneControlFlow = false) {
  DiagnoseImpCast(S, E, E->getType(), T, CContext, diag, pruneControlFlow);
}

/// Diagnose an implicit cast from a literal expression. Does not warn when the
/// cast wouldn't lose information.
void DiagnoseFloatingLiteralImpCast(Sema &S, FloatingLiteral *FL, QualType T,
                                    SourceLocation CContext) {
  // Try to convert the literal exactly to an integer. If we can, don't warn.
  bool isExact = false;
  const llvm::APFloat &Value = FL->getValue();
  llvm::APSInt IntegerValue(S.Context.getIntWidth(T),
                            T->hasUnsignedIntegerRepresentation());
  if (Value.convertToInteger(IntegerValue,
                             llvm::APFloat::rmTowardZero, &isExact)
      == llvm::APFloat::opOK && isExact)
    return;

  // FIXME: Force the precision of the source value down so we don't print
  // digits which are usually useless (we don't really care here if we
  // truncate a digit by accident in edge cases).  Ideally, APFloat::toString
  // would automatically print the shortest representation, but it's a bit
  // tricky to implement.
  SmallString<16> PrettySourceValue;
  unsigned precision = llvm::APFloat::semanticsPrecision(Value.getSemantics());
  precision = (precision * 59 + 195) / 196;
  Value.toString(PrettySourceValue, precision);

  SmallString<16> PrettyTargetValue;
  if (T->isSpecificBuiltinType(BuiltinType::Bool))
    PrettyTargetValue = IntegerValue == 0 ? "false" : "true";
  else
    IntegerValue.toString(PrettyTargetValue);

  S.Diag(FL->getExprLoc(), diag::warn_impcast_literal_float_to_integer)
    << FL->getType() << T.getUnqualifiedType() << PrettySourceValue
    << PrettyTargetValue << FL->getSourceRange() << SourceRange(CContext);
}

std::string PrettyPrintInRange(const llvm::APSInt &Value, IntRange Range) {
  if (!Range.Width) return "0";

  llvm::APSInt ValueInRange = Value;
  ValueInRange.setIsSigned(!Range.NonNegative);
  ValueInRange = ValueInRange.trunc(Range.Width);
  return ValueInRange.toString(10);
}

static bool IsImplicitBoolFloatConversion(Sema &S, Expr *Ex, bool ToBool) {
  if (!isa<ImplicitCastExpr>(Ex))
    return false;

  Expr *InnerE = Ex->IgnoreParenImpCasts();
  const Type *Target = S.Context.getCanonicalType(Ex->getType()).getTypePtr();
  const Type *Source =
    S.Context.getCanonicalType(InnerE->getType()).getTypePtr();
  if (Target->isDependentType())
    return false;

  const BuiltinType *FloatCandidateBT =
    dyn_cast<BuiltinType>(ToBool ? Source : Target);
  const Type *BoolCandidateType = ToBool ? Target : Source;

  return (BoolCandidateType->isSpecificBuiltinType(BuiltinType::Bool) &&
          FloatCandidateBT && (FloatCandidateBT->isFloatingPoint()));
}

void CheckImplicitArgumentConversions(Sema &S, CallExpr *TheCall,
                                      SourceLocation CC) {
  // HLSL Change Begin
  if (FunctionDecl *FD = TheCall->getDirectCallee()) {
    CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(FD);
    unsigned ArgIdx = 0;
    unsigned ParmIdx = 0;
    if (MD && MD->isInstance())
      ++ParmIdx;
    for (; ArgIdx < TheCall->getNumArgs() && ParmIdx < FD->getNumParams();
         ++ArgIdx, ++ParmIdx) {
      ParmVarDecl *PD = FD->getParamDecl(ParmIdx);
      Expr *CurrA = TheCall->getArg(ArgIdx);
      S.DiagnoseGloballyCoherentMismatch(CurrA, PD->getType(), CC);
    }
  }
  // HLSL CHange End

  unsigned NumArgs = TheCall->getNumArgs();
  for (unsigned i = 0; i < NumArgs; ++i) {
    Expr *CurrA = TheCall->getArg(i);
    if (!IsImplicitBoolFloatConversion(S, CurrA, true))
      continue;

    bool IsSwapped = ((i > 0) &&
        IsImplicitBoolFloatConversion(S, TheCall->getArg(i - 1), false));
    IsSwapped |= ((i < (NumArgs - 1)) &&
        IsImplicitBoolFloatConversion(S, TheCall->getArg(i + 1), false));
    if (IsSwapped) {
      // Warn on this floating-point to bool conversion.
      DiagnoseImpCast(S, CurrA->IgnoreParenImpCasts(),
                      CurrA->getType(), CC,
                      diag::warn_impcast_floating_point_to_bool);
    }
  }
}

static void DiagnoseNullConversion(Sema &S, Expr *E, QualType T,
                                   SourceLocation CC) {
  if (S.Diags.isIgnored(diag::warn_impcast_null_pointer_to_integer,
                        E->getExprLoc()))
    return;

  // Check for NULL (GNUNull) or nullptr (CXX11_nullptr).
  const Expr::NullPointerConstantKind NullKind =
      E->isNullPointerConstant(S.Context, Expr::NPC_ValueDependentIsNotNull);
  if (NullKind != Expr::NPCK_GNUNull && NullKind != Expr::NPCK_CXX11_nullptr)
    return;

  // Return if target type is a safe conversion.
  if (T->isAnyPointerType() || T->isBlockPointerType() ||
      T->isMemberPointerType() || !T->isScalarType() || T->isNullPtrType())
    return;

  SourceLocation Loc = E->getSourceRange().getBegin();

  // __null is usually wrapped in a macro.  Go up a macro if that is the case.
  if (NullKind == Expr::NPCK_GNUNull) {
    if (Loc.isMacroID())
      Loc = S.SourceMgr.getImmediateExpansionRange(Loc).first;
  }

  // Only warn if the null and context location are in the same macro expansion.
  if (S.SourceMgr.getFileID(Loc) != S.SourceMgr.getFileID(CC))
    return;

  S.Diag(Loc, diag::warn_impcast_null_pointer_to_integer)
      << (NullKind == Expr::NPCK_CXX11_nullptr) << T << clang::SourceRange(CC)
      << FixItHint::CreateReplacement(Loc,
                                      S.getFixItZeroLiteralForType(T, Loc));
}

static void checkObjCArrayLiteral(Sema &S, QualType TargetType,
                                  ObjCArrayLiteral *ArrayLiteral);
static void checkObjCDictionaryLiteral(Sema &S, QualType TargetType,
                                       ObjCDictionaryLiteral *DictionaryLiteral);

/// Check a single element within a collection literal against the
/// target element type.
static void checkObjCCollectionLiteralElement(Sema &S,
                                              QualType TargetElementType,
                                              Expr *Element,
                                              unsigned ElementKind) {
  // Skip a bitcast to 'id' or qualified 'id'.
  if (auto ICE = dyn_cast<ImplicitCastExpr>(Element)) {
    if (ICE->getCastKind() == CK_BitCast &&
        ICE->getSubExpr()->getType()->getAs<ObjCObjectPointerType>())
      Element = ICE->getSubExpr();
  }

  QualType ElementType = Element->getType();
  ExprResult ElementResult(Element);
  if (ElementType->getAs<ObjCObjectPointerType>() &&
      S.CheckSingleAssignmentConstraints(TargetElementType,
                                         ElementResult,
                                         false, false)
        != Sema::Compatible) {
    S.Diag(Element->getLocStart(),
           diag::warn_objc_collection_literal_element)
      << ElementType << ElementKind << TargetElementType
      << Element->getSourceRange();
  }

  if (auto ArrayLiteral = dyn_cast<ObjCArrayLiteral>(Element))
    checkObjCArrayLiteral(S, TargetElementType, ArrayLiteral);
  else if (auto DictionaryLiteral = dyn_cast<ObjCDictionaryLiteral>(Element))
    checkObjCDictionaryLiteral(S, TargetElementType, DictionaryLiteral);
}

/// Check an Objective-C array literal being converted to the given
/// target type.
static void checkObjCArrayLiteral(Sema &S, QualType TargetType,
                                  ObjCArrayLiteral *ArrayLiteral) {
  if (!S.NSArrayDecl)
    return;

  const auto *TargetObjCPtr = TargetType->getAs<ObjCObjectPointerType>();
  if (!TargetObjCPtr)
    return;

  if (TargetObjCPtr->isUnspecialized() ||
      TargetObjCPtr->getInterfaceDecl()->getCanonicalDecl()
        != S.NSArrayDecl->getCanonicalDecl())
    return;

  auto TypeArgs = TargetObjCPtr->getTypeArgs();
  if (TypeArgs.size() != 1)
    return;

  QualType TargetElementType = TypeArgs[0];
  for (unsigned I = 0, N = ArrayLiteral->getNumElements(); I != N; ++I) {
    checkObjCCollectionLiteralElement(S, TargetElementType,
                                      ArrayLiteral->getElement(I),
                                      0);
  }
}

/// Check an Objective-C dictionary literal being converted to the given
/// target type.
static void checkObjCDictionaryLiteral(
              Sema &S, QualType TargetType,
              ObjCDictionaryLiteral *DictionaryLiteral) {
  if (!S.NSDictionaryDecl)
    return;

  const auto *TargetObjCPtr = TargetType->getAs<ObjCObjectPointerType>();
  if (!TargetObjCPtr)
    return;

  if (TargetObjCPtr->isUnspecialized() ||
      TargetObjCPtr->getInterfaceDecl()->getCanonicalDecl()
        != S.NSDictionaryDecl->getCanonicalDecl())
    return;

  auto TypeArgs = TargetObjCPtr->getTypeArgs();
  if (TypeArgs.size() != 2)
    return;

  QualType TargetKeyType = TypeArgs[0];
  QualType TargetObjectType = TypeArgs[1];
  for (unsigned I = 0, N = DictionaryLiteral->getNumElements(); I != N; ++I) {
    auto Element = DictionaryLiteral->getKeyValueElement(I);
    checkObjCCollectionLiteralElement(S, TargetKeyType, Element.Key, 1);
    checkObjCCollectionLiteralElement(S, TargetObjectType, Element.Value, 2);
  }
}

void CheckImplicitConversion(Sema &S, Expr *E, QualType T,
                             SourceLocation CC, bool *ICContext = nullptr) {
  if (E->isTypeDependent() || E->isValueDependent()) return;

  const Type *Source = S.Context.getCanonicalType(E->getType()).getTypePtr();
  const Type *Target = S.Context.getCanonicalType(T).getTypePtr();
  if (Source == Target) return;
  if (Target->isDependentType()) return;

  // If the conversion context location is invalid don't complain. We also
  // don't want to emit a warning if the issue occurs from the expansion of
  // a system macro. The problem is that 'getSpellingLoc()' is slow, so we
  // delay this check as long as possible. Once we detect we are in that
  // scenario, we just return.
  if (CC.isInvalid())
    return;

  // Diagnose implicit casts to bool.
  if (Target->isSpecificBuiltinType(BuiltinType::Bool)) {
    if (isa<StringLiteral>(E))
      // Warn on string literal to bool.  Checks for string literals in logical
      // and expressions, for instance, assert(0 && "error here"), are
      // prevented by a check in AnalyzeImplicitConversions().
      return DiagnoseImpCast(S, E, T, CC,
                             diag::warn_impcast_string_literal_to_bool);
    if (isa<ObjCStringLiteral>(E) || isa<ObjCArrayLiteral>(E) ||
        isa<ObjCDictionaryLiteral>(E) || isa<ObjCBoxedExpr>(E)) {
      // This covers the literal expressions that evaluate to Objective-C
      // objects.
      return DiagnoseImpCast(S, E, T, CC,
                             diag::warn_impcast_objective_c_literal_to_bool);
    }
    if (Source->isPointerType() || Source->canDecayToPointerType()) {
      // Warn on pointer to bool conversion that is always true.
      S.DiagnoseAlwaysNonNullPointer(E, Expr::NPCK_NotNull, /*IsEqual*/ false,
                                     SourceRange(CC));
    }
  }

  // Check implicit casts from Objective-C collection literals to specialized
  // collection types, e.g., NSArray<NSString *> *.
  if (auto *ArrayLiteral = dyn_cast<ObjCArrayLiteral>(E))
    checkObjCArrayLiteral(S, QualType(Target, 0), ArrayLiteral);
  else if (auto *DictionaryLiteral = dyn_cast<ObjCDictionaryLiteral>(E))
    checkObjCDictionaryLiteral(S, QualType(Target, 0), DictionaryLiteral);

  // Strip vector types.
  if (isa<VectorType>(Source)) {
    if (!isa<VectorType>(Target)) {
      if (S.SourceMgr.isInSystemMacro(CC))
        return;
      return DiagnoseImpCast(S, E, T, CC, diag::warn_impcast_vector_scalar);
    }
    
    // If the vector cast is cast between two vectors of the same size, it is
    // a bitcast, not a conversion.
    if (S.Context.getTypeSize(Source) == S.Context.getTypeSize(Target))
      return;

    Source = cast<VectorType>(Source)->getElementType().getTypePtr();
    Target = cast<VectorType>(Target)->getElementType().getTypePtr();
  }
  if (auto VecTy = dyn_cast<VectorType>(Target))
    Target = VecTy->getElementType().getTypePtr();

  // HLSL Change Begins: Vector/matrix implicit conversions already diagnosed
  if (hlsl::IsVectorType(&S, QualType(Source, 0)) || hlsl::IsMatrixType(&S, QualType(Source, 0)) ||
      hlsl::IsVectorType(&S, QualType(Target, 0)) || hlsl::IsMatrixType(&S, QualType(Target, 0))) {
    return;
  }
  // HLSL Change Ends

  // Strip complex types.
  if (isa<ComplexType>(Source)) {
    if (!isa<ComplexType>(Target)) {
      if (S.SourceMgr.isInSystemMacro(CC))
        return;

      return DiagnoseImpCast(S, E, T, CC, diag::warn_impcast_complex_scalar);
    }

    Source = cast<ComplexType>(Source)->getElementType().getTypePtr();
    Target = cast<ComplexType>(Target)->getElementType().getTypePtr();
  }

  const BuiltinType *SourceBT = dyn_cast<BuiltinType>(Source);
  const BuiltinType *TargetBT = dyn_cast<BuiltinType>(Target);

  // If the source is floating point...
  if (SourceBT && SourceBT->isFloatingPoint()) {
    // ...and the target is floating point...
    if (TargetBT && TargetBT->isFloatingPoint()) {
      // ...then warn if we're dropping FP rank.

      // HLSL Change Begin - Warn on both promotions and conversions.
      // Don't warn on Literal float.
      if (SourceBT->getKind() == BuiltinType::LitFloat)
        return;
      int Order = S.getASTContext().getFloatingTypeOrder(QualType(SourceBT, 0),
                                                         QualType(TargetBT, 0));

      // Builtin FP kinds are ordered by increasing FP rank.
      if (Order > 0) {
        // Don't warn about float constants that are precisely
        // representable in the target type.
        Expr::EvalResult result;
        if (E->EvaluateAsRValue(result, S.Context)) {
          // Value might be a float, a float vector, or a float complex.
          if (IsSameFloatAfterCast(result.Val,
                   S.Context.getFloatTypeSemantics(QualType(TargetBT, 0)),
                   S.Context.getFloatTypeSemantics(QualType(SourceBT, 0))))
            return;
        }

        if (S.SourceMgr.isInSystemMacro(CC))
          return;

        DiagnoseImpCast(S, E, T, CC, diag::warn_impcast_float_precision);
      } else if (Order < 0) {
        DiagnoseImpCast(S, E, T, CC, diag::warn_impcast_double_promotion);
      }
      // HLSL Change End
      return;
    }

    // If the target is integral, always warn.    
    if (TargetBT && TargetBT->isInteger()) {
      if (S.SourceMgr.isInSystemMacro(CC))
        return;
      
      Expr *InnerE = E->IgnoreParenImpCasts();
      // We also want to warn on, e.g., "int i = -1.234"
      if (UnaryOperator *UOp = dyn_cast<UnaryOperator>(InnerE))
        if (UOp->getOpcode() == UO_Minus || UOp->getOpcode() == UO_Plus)
          InnerE = UOp->getSubExpr()->IgnoreParenImpCasts();

      if (FloatingLiteral *FL = dyn_cast<FloatingLiteral>(InnerE)) {
        DiagnoseFloatingLiteralImpCast(S, FL, T, CC);
      } else {
        DiagnoseImpCast(S, E, T, CC, diag::warn_impcast_float_integer);
      }
    }

    // If the target is bool, warn if expr is a function or method call.
    if (Target->isSpecificBuiltinType(BuiltinType::Bool) &&
        isa<CallExpr>(E)) {
      // Check last argument of function call to see if it is an
      // implicit cast from a type matching the type the result
      // is being cast to.
      CallExpr *CEx = cast<CallExpr>(E);
      unsigned NumArgs = CEx->getNumArgs();
      if (NumArgs > 0) {
        Expr *LastA = CEx->getArg(NumArgs - 1);
        Expr *InnerE = LastA->IgnoreParenImpCasts();
        const Type *InnerType =
          S.Context.getCanonicalType(InnerE->getType()).getTypePtr();
        if (isa<ImplicitCastExpr>(LastA) && (InnerType == Target)) {
          // Warn on this floating-point to bool conversion
          DiagnoseImpCast(S, E, T, CC,
                          diag::warn_impcast_floating_point_to_bool);
        }
      }
    }
    return;
  }

  DiagnoseNullConversion(S, E, T, CC);

  if (!Source->isIntegerType() || !Target->isIntegerType())
    return;

  // TODO: remove this early return once the false positives for constant->bool
  // in templates, macros, etc, are reduced or removed.
  if (Target->isSpecificBuiltinType(BuiltinType::Bool))
    return;

  IntRange SourceRange = GetExprRange(S.Context, E);
  IntRange TargetRange = IntRange::forTargetOfCanonicalType(S.Context, Target);

  if (SourceRange.Width > TargetRange.Width) {
    // If the source is a constant, use a default-on diagnostic.
    // TODO: this should happen for bitfield stores, too.
    llvm::APSInt Value(32);
    if (E->isIntegerConstantExpr(Value, S.Context)) {
      if (S.SourceMgr.isInSystemMacro(CC))
        return;

      std::string PrettySourceValue = Value.toString(10);
      std::string PrettyTargetValue = PrettyPrintInRange(Value, TargetRange);

      S.DiagRuntimeBehavior(E->getExprLoc(), E,
        S.PDiag(diag::warn_impcast_integer_precision_constant)
            << PrettySourceValue << PrettyTargetValue
            << E->getType() << T << E->getSourceRange()
            << clang::SourceRange(CC));
      return;
    }

    // People want to build with -Wshorten-64-to-32 and not -Wconversion.
    if (S.SourceMgr.isInSystemMacro(CC))
      return;

    if (TargetRange.Width == 32 && S.Context.getIntWidth(E->getType()) == 64)
      return DiagnoseImpCast(S, E, T, CC, diag::warn_impcast_integer_64_32,
                             /* pruneControlFlow */ true);
    return DiagnoseImpCast(S, E, T, CC, diag::warn_impcast_integer_precision);
  }

  if ((TargetRange.NonNegative && !SourceRange.NonNegative) ||
      (!TargetRange.NonNegative && SourceRange.NonNegative &&
       SourceRange.Width == TargetRange.Width)) {
        
    if (S.SourceMgr.isInSystemMacro(CC))
      return;

    unsigned DiagID = diag::warn_impcast_integer_sign;

    // Traditionally, gcc has warned about this under -Wsign-compare.
    // We also want to warn about it in -Wconversion.
    // So if -Wconversion is off, use a completely identical diagnostic
    // in the sign-compare group.
    // The conditional-checking code will 
    if (ICContext) {
      DiagID = diag::warn_impcast_integer_sign_conditional;
      *ICContext = true;
    }

    return DiagnoseImpCast(S, E, T, CC, DiagID);
  }

  // Diagnose conversions between different enumeration types.
  // In C, we pretend that the type of an EnumConstantDecl is its enumeration
  // type, to give us better diagnostics.
  QualType SourceType = E->getType();
  if (!S.getLangOpts().CPlusPlus) {
    if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(E))
      if (EnumConstantDecl *ECD = dyn_cast<EnumConstantDecl>(DRE->getDecl())) {
        EnumDecl *Enum = cast<EnumDecl>(ECD->getDeclContext());
        SourceType = S.Context.getTypeDeclType(Enum);
        Source = S.Context.getCanonicalType(SourceType).getTypePtr();
      }
  }
  
  if (const EnumType *SourceEnum = Source->getAs<EnumType>())
    if (const EnumType *TargetEnum = Target->getAs<EnumType>())
      if (SourceEnum->getDecl()->hasNameForLinkage() &&
          TargetEnum->getDecl()->hasNameForLinkage() &&
          SourceEnum != TargetEnum) {
        if (S.SourceMgr.isInSystemMacro(CC))
          return;

        return DiagnoseImpCast(S, E, SourceType, T, CC, 
                               diag::warn_impcast_different_enum_types);
      }
  
  return;
}

void CheckConditionalOperator(Sema &S, ConditionalOperator *E,
                              SourceLocation CC, QualType T);

void CheckConditionalOperand(Sema &S, Expr *E, QualType T,
                             SourceLocation CC, bool &ICContext) {
  E = E->IgnoreParenImpCasts();

  if (isa<ConditionalOperator>(E))
    return CheckConditionalOperator(S, cast<ConditionalOperator>(E), CC, T);

  AnalyzeImplicitConversions(S, E, CC);
  if (E->getType() != T)
    return CheckImplicitConversion(S, E, T, CC, &ICContext);
  return;
}

void CheckConditionalOperator(Sema &S, ConditionalOperator *E,
                              SourceLocation CC, QualType T) {
  AnalyzeImplicitConversions(S, E->getCond(), E->getQuestionLoc());

  bool Suspicious = false;
  CheckConditionalOperand(S, E->getTrueExpr(), T, CC, Suspicious);
  CheckConditionalOperand(S, E->getFalseExpr(), T, CC, Suspicious);

  // If -Wconversion would have warned about either of the candidates
  // for a signedness conversion to the context type...
  if (!Suspicious) return;

  // ...but it's currently ignored...
  if (!S.Diags.isIgnored(diag::warn_impcast_integer_sign_conditional, CC))
    return;

  // ...then check whether it would have warned about either of the
  // candidates for a signedness conversion to the condition type.
  if (E->getType() == T) return;
 
  Suspicious = false;
  CheckImplicitConversion(S, E->getTrueExpr()->IgnoreParenImpCasts(),
                          E->getType(), CC, &Suspicious);
  if (!Suspicious)
    CheckImplicitConversion(S, E->getFalseExpr()->IgnoreParenImpCasts(),
                            E->getType(), CC, &Suspicious);
}

/// CheckBoolLikeConversion - Check conversion of given expression to boolean.
/// Input argument E is a logical expression.
static void CheckBoolLikeConversion(Sema &S, Expr *E, SourceLocation CC) {
  if (S.getLangOpts().Bool)
    return;
  CheckImplicitConversion(S, E->IgnoreParenImpCasts(), S.Context.BoolTy, CC);
}

/// AnalyzeImplicitConversions - Find and report any interesting
/// implicit conversions in the given expression.  There are a couple
/// of competing diagnostics here, -Wconversion and -Wsign-compare.
void AnalyzeImplicitConversions(Sema &S, Expr *OrigE, SourceLocation CC) {
  QualType T = OrigE->getType();
  Expr *E = OrigE->IgnoreParenImpCasts();

  if (E->isTypeDependent() || E->isValueDependent())
    return;
  
  // For conditional operators, we analyze the arguments as if they
  // were being fed directly into the output.
  if (isa<ConditionalOperator>(E)) {
    ConditionalOperator *CO = cast<ConditionalOperator>(E);
    CheckConditionalOperator(S, CO, CC, T);
    return;
  }

  // Check implicit argument conversions for function calls.
  if (CallExpr *Call = dyn_cast<CallExpr>(E))
    CheckImplicitArgumentConversions(S, Call, CC);

  // Go ahead and check any implicit conversions we might have skipped.
  // The non-canonical typecheck is just an optimization;
  // CheckImplicitConversion will filter out dead implicit conversions.
  if (E->getType() != T)
    CheckImplicitConversion(S, E, T, CC);

  // Now continue drilling into this expression.
  
  if (PseudoObjectExpr * POE = dyn_cast<PseudoObjectExpr>(E)) {
    if (POE->getResultExpr())
      E = POE->getResultExpr();
  }
  
  if (const OpaqueValueExpr *OVE = dyn_cast<OpaqueValueExpr>(E)) {
    if (OVE->getSourceExpr())
      AnalyzeImplicitConversions(S, OVE->getSourceExpr(), CC);
    return;
  }
  
  // Skip past explicit casts.
  if (isa<ExplicitCastExpr>(E)) {
    E = cast<ExplicitCastExpr>(E)->getSubExpr()->IgnoreParenImpCasts();
    return AnalyzeImplicitConversions(S, E, CC);
  }

  if (BinaryOperator *BO = dyn_cast<BinaryOperator>(E)) {
    // Do a somewhat different check with comparison operators.
    if (BO->isComparisonOp())
      return AnalyzeComparison(S, BO);

    // And with simple assignments.
    if (BO->getOpcode() == BO_Assign)
      return AnalyzeAssignment(S, BO);
  }

  // These break the otherwise-useful invariant below.  Fortunately,
  // we don't really need to recurse into them, because any internal
  // expressions should have been analyzed already when they were
  // built into statements.
  if (isa<StmtExpr>(E)) return;

  // Don't descend into unevaluated contexts.
  if (isa<UnaryExprOrTypeTraitExpr>(E)) return;

  // Now just recurse over the expression's children.
  CC = E->getExprLoc();
  BinaryOperator *BO = dyn_cast<BinaryOperator>(E);
  bool IsLogicalAndOperator = BO && BO->getOpcode() == BO_LAnd;
  for (Stmt *SubStmt : E->children()) {
    Expr *ChildExpr = dyn_cast_or_null<Expr>(SubStmt);
    if (!ChildExpr)
      continue;

    if (IsLogicalAndOperator &&
        isa<StringLiteral>(ChildExpr->IgnoreParenImpCasts()))
      // Ignore checking string literals that are in logical and operators.
      // This is a common pattern for asserts.
      continue;
    AnalyzeImplicitConversions(S, ChildExpr, CC);
  }

  if (BO && BO->isLogicalOp()) {
    Expr *SubExpr = BO->getLHS()->IgnoreParenImpCasts();
    if (!IsLogicalAndOperator || !isa<StringLiteral>(SubExpr))
      ::CheckBoolLikeConversion(S, SubExpr, BO->getExprLoc());

    SubExpr = BO->getRHS()->IgnoreParenImpCasts();
    if (!IsLogicalAndOperator || !isa<StringLiteral>(SubExpr))
      ::CheckBoolLikeConversion(S, SubExpr, BO->getExprLoc());
  }

  if (const UnaryOperator *U = dyn_cast<UnaryOperator>(E))
    if (U->getOpcode() == UO_LNot)
      ::CheckBoolLikeConversion(S, U->getSubExpr(), CC);
}

} // end anonymous namespace

enum {
  AddressOf,
  FunctionPointer,
  ArrayPointer
};

// Helper function for Sema::DiagnoseAlwaysNonNullPointer.
// Returns true when emitting a warning about taking the address of a reference.
static bool CheckForReference(Sema &SemaRef, const Expr *E,
                              PartialDiagnostic PD) {
  E = E->IgnoreParenImpCasts();

  const FunctionDecl *FD = nullptr;

  if (const DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(E)) {
    if (!DRE->getDecl()->getType()->isReferenceType())
      return false;
  } else if (const MemberExpr *M = dyn_cast<MemberExpr>(E)) {
    if (!M->getMemberDecl()->getType()->isReferenceType())
      return false;
  } else if (const CallExpr *Call = dyn_cast<CallExpr>(E)) {
    if (!Call->getCallReturnType(SemaRef.Context)->isReferenceType())
      return false;
    FD = Call->getDirectCallee();
  } else {
    return false;
  }

  SemaRef.Diag(E->getExprLoc(), PD);

  // If possible, point to location of function.
  if (FD) {
    SemaRef.Diag(FD->getLocation(), diag::note_reference_is_return_value) << FD;
  }

  return true;
}

// Returns true if the SourceLocation is expanded from any macro body.
// Returns false if the SourceLocation is invalid, is from not in a macro
// expansion, or is from expanded from a top-level macro argument.
static bool IsInAnyMacroBody(const SourceManager &SM, SourceLocation Loc) {
  if (Loc.isInvalid())
    return false;

  while (Loc.isMacroID()) {
    if (SM.isMacroBodyExpansion(Loc))
      return true;
    Loc = SM.getImmediateMacroCallerLoc(Loc);
  }

  return false;
}

/// \brief Diagnose pointers that are always non-null.
/// \param E the expression containing the pointer
/// \param NullKind NPCK_NotNull if E is a cast to bool, otherwise, E is
/// compared to a null pointer
/// \param IsEqual True when the comparison is equal to a null pointer
/// \param Range Extra SourceRange to highlight in the diagnostic
void Sema::DiagnoseAlwaysNonNullPointer(Expr *E,
                                        Expr::NullPointerConstantKind NullKind,
                                        bool IsEqual, SourceRange Range) {
  if (!E)
    return;

  // Don't warn inside macros.
  if (E->getExprLoc().isMacroID()) {
    const SourceManager &SM = getSourceManager();
    if (IsInAnyMacroBody(SM, E->getExprLoc()) ||
        IsInAnyMacroBody(SM, Range.getBegin()))
      return;
  }
  E = E->IgnoreImpCasts();

  const bool IsCompare = NullKind != Expr::NPCK_NotNull;

  if (isa<CXXThisExpr>(E)) {
    unsigned DiagID = IsCompare ? diag::warn_this_null_compare
                                : diag::warn_this_bool_conversion;
    Diag(E->getExprLoc(), DiagID) << E->getSourceRange() << Range << IsEqual;
    return;
  }

  bool IsAddressOf = false;

  if (UnaryOperator *UO = dyn_cast<UnaryOperator>(E)) {
    if (UO->getOpcode() != UO_AddrOf)
      return;
    IsAddressOf = true;
    E = UO->getSubExpr();
  }

  if (IsAddressOf) {
    unsigned DiagID = IsCompare
                          ? diag::warn_address_of_reference_null_compare
                          : diag::warn_address_of_reference_bool_conversion;
    PartialDiagnostic PD = PDiag(DiagID) << E->getSourceRange() << Range
                                         << IsEqual;
    if (CheckForReference(*this, E, PD)) {
      return;
    }
  }

  // Expect to find a single Decl.  Skip anything more complicated.
  ValueDecl *D = nullptr;
  if (DeclRefExpr *R = dyn_cast<DeclRefExpr>(E)) {
    D = R->getDecl();
  } else if (MemberExpr *M = dyn_cast<MemberExpr>(E)) {
    D = M->getMemberDecl();
  }

  // Weak Decls can be null.
  if (!D || D->isWeak())
    return;
  
  // Check for parameter decl with nonnull attribute
  if (const ParmVarDecl* PV = dyn_cast<ParmVarDecl>(D)) {
    if (getCurFunction() && !getCurFunction()->ModifiedNonNullParams.count(PV))
      if (const FunctionDecl* FD = dyn_cast<FunctionDecl>(PV->getDeclContext())) {
        unsigned NumArgs = FD->getNumParams();
        llvm::SmallBitVector AttrNonNull(NumArgs);
        for (const auto *NonNull : FD->specific_attrs<NonNullAttr>()) {
          if (!NonNull->args_size()) {
            AttrNonNull.set(0, NumArgs);
            break;
          }
          for (unsigned Val : NonNull->args()) {
            if (Val >= NumArgs)
              continue;
            AttrNonNull.set(Val);
          }
        }
        if (!AttrNonNull.empty())
          for (unsigned i = 0; i < NumArgs; ++i)
            if (FD->getParamDecl(i) == PV &&
                (AttrNonNull[i] || PV->hasAttr<NonNullAttr>())) {
              std::string Str;
              llvm::raw_string_ostream S(Str);
              E->printPretty(S, nullptr, getPrintingPolicy());
              unsigned DiagID = IsCompare ? diag::warn_nonnull_parameter_compare
                                          : diag::warn_cast_nonnull_to_bool;
              Diag(E->getExprLoc(), DiagID) << S.str() << E->getSourceRange()
                << Range << IsEqual;
              return;
            }
      }
    }
  
  QualType T = D->getType();
  const bool IsArray = T->isArrayType();
  const bool IsFunction = T->isFunctionType();

  // Address of function is used to silence the function warning.
  if (IsAddressOf && IsFunction) {
    return;
  }

  // Found nothing.
  if (!IsAddressOf && !IsFunction && !IsArray)
    return;

  // Pretty print the expression for the diagnostic.
  std::string Str;
  llvm::raw_string_ostream S(Str);
  E->printPretty(S, nullptr, getPrintingPolicy());

  unsigned DiagID = IsCompare ? diag::warn_null_pointer_compare
                              : diag::warn_impcast_pointer_to_bool;
  unsigned DiagType;
  if (IsAddressOf)
    DiagType = AddressOf;
  else if (IsFunction)
    DiagType = FunctionPointer;
  else if (IsArray)
    DiagType = ArrayPointer;
  else
    llvm_unreachable("Could not determine diagnostic.");
  Diag(E->getExprLoc(), DiagID) << DiagType << S.str() << E->getSourceRange()
                                << Range << IsEqual;

  if (!IsFunction)
    return;

  // Suggest '&' to silence the function warning.
  Diag(E->getExprLoc(), diag::note_function_warning_silence)
      << FixItHint::CreateInsertion(E->getLocStart(), "&");

  // Check to see if '()' fixit should be emitted.
  QualType ReturnType;
  UnresolvedSet<4> NonTemplateOverloads;
  tryExprAsCall(*E, ReturnType, NonTemplateOverloads);
  if (ReturnType.isNull())
    return;

  if (IsCompare) {
    // There are two cases here.  If there is null constant, the only suggest
    // for a pointer return type.  If the null is 0, then suggest if the return
    // type is a pointer or an integer type.
    if (!ReturnType->isPointerType()) {
      if (NullKind == Expr::NPCK_ZeroExpression ||
          NullKind == Expr::NPCK_ZeroLiteral) {
        if (!ReturnType->isIntegerType())
          return;
      } else {
        return;
      }
    }
  } else { // !IsCompare
    // For function to bool, only suggest if the function pointer has bool
    // return type.
    if (!ReturnType->isSpecificBuiltinType(BuiltinType::Bool))
      return;
  }
  Diag(E->getExprLoc(), diag::note_function_to_function_call)
      << FixItHint::CreateInsertion(getLocForEndOfToken(E->getLocEnd()), "()");
}


/// Diagnoses "dangerous" implicit conversions within the given
/// expression (which is a full expression).  Implements -Wconversion
/// and -Wsign-compare.
///
/// \param CC the "context" location of the implicit conversion, i.e.
///   the most location of the syntactic entity requiring the implicit
///   conversion
void Sema::CheckImplicitConversions(Expr *E, SourceLocation CC) {
  // Don't diagnose in unevaluated contexts.
  if (isUnevaluatedContext())
    return;

  // Don't diagnose for value- or type-dependent expressions.
  if (E->isTypeDependent() || E->isValueDependent())
    return;

  // Check for array bounds violations in cases where the check isn't triggered
  // elsewhere for other Expr types (like BinaryOperators), e.g. when an
  // ArraySubscriptExpr is on the RHS of a variable initialization.
  CheckArrayAccess(E);

  // This is not the right CC for (e.g.) a variable initialization.
  AnalyzeImplicitConversions(*this, E, CC);
}

/// CheckBoolLikeConversion - Check conversion of given expression to boolean.
/// Input argument E is a logical expression.
void Sema::CheckBoolLikeConversion(Expr *E, SourceLocation CC) {
  ::CheckBoolLikeConversion(*this, E, CC);
}

/// Diagnose when expression is an integer constant expression and its evaluation
/// results in integer overflow
void Sema::CheckForIntOverflow (Expr *E) {
  if (isa<BinaryOperator>(E->IgnoreParenCasts()))
    E->IgnoreParenCasts()->EvaluateForOverflow(Context);
}

namespace {
/// \brief Visitor for expressions which looks for unsequenced operations on the
/// same object.
class SequenceChecker : public EvaluatedExprVisitor<SequenceChecker> {
  typedef EvaluatedExprVisitor<SequenceChecker> Base;

  /// \brief A tree of sequenced regions within an expression. Two regions are
  /// unsequenced if one is an ancestor or a descendent of the other. When we
  /// finish processing an expression with sequencing, such as a comma
  /// expression, we fold its tree nodes into its parent, since they are
  /// unsequenced with respect to nodes we will visit later.
  class SequenceTree {
    struct Value {
      explicit Value(unsigned Parent) : Parent(Parent), Merged(false) {}
      unsigned Parent : 31;
      bool Merged : 1;
    };
    SmallVector<Value, 8> Values;

  public:
    /// \brief A region within an expression which may be sequenced with respect
    /// to some other region.
    class Seq {
      explicit Seq(unsigned N) : Index(N) {}
      unsigned Index;
      friend class SequenceTree;
    public:
      Seq() : Index(0) {}
    };

    SequenceTree() { Values.push_back(Value(0)); }
    Seq root() const { return Seq(0); }

    /// \brief Create a new sequence of operations, which is an unsequenced
    /// subset of \p Parent. This sequence of operations is sequenced with
    /// respect to other children of \p Parent.
    Seq allocate(Seq Parent) {
      Values.push_back(Value(Parent.Index));
      return Seq(Values.size() - 1);
    }

    /// \brief Merge a sequence of operations into its parent.
    void merge(Seq S) {
      Values[S.Index].Merged = true;
    }

    /// \brief Determine whether two operations are unsequenced. This operation
    /// is asymmetric: \p Cur should be the more recent sequence, and \p Old
    /// should have been merged into its parent as appropriate.
    bool isUnsequenced(Seq Cur, Seq Old) {
      unsigned C = representative(Cur.Index);
      unsigned Target = representative(Old.Index);
      while (C >= Target) {
        if (C == Target)
          return true;
        C = Values[C].Parent;
      }
      return false;
    }

  private:
    /// \brief Pick a representative for a sequence.
    unsigned representative(unsigned K) {
      if (Values[K].Merged)
        // Perform path compression as we go.
        return Values[K].Parent = representative(Values[K].Parent);
      return K;
    }
  };

  /// An object for which we can track unsequenced uses.
  typedef NamedDecl *Object;

  /// Different flavors of object usage which we track. We only track the
  /// least-sequenced usage of each kind.
  enum UsageKind {
    /// A read of an object. Multiple unsequenced reads are OK.
    UK_Use,
    /// A modification of an object which is sequenced before the value
    /// computation of the expression, such as ++n in C++.
    UK_ModAsValue,
    /// A modification of an object which is not sequenced before the value
    /// computation of the expression, such as n++.
    UK_ModAsSideEffect,

    UK_Count = UK_ModAsSideEffect + 1
  };

  struct Usage {
    Usage() : Use(nullptr), Seq() {}
    Expr *Use;
    SequenceTree::Seq Seq;
  };

  struct UsageInfo {
    UsageInfo() : Diagnosed(false) {}
    Usage Uses[UK_Count];
    /// Have we issued a diagnostic for this variable already?
    bool Diagnosed;
  };
  typedef llvm::SmallDenseMap<Object, UsageInfo, 16> UsageInfoMap;

  Sema &SemaRef;
  /// Sequenced regions within the expression.
  SequenceTree Tree;
  /// Declaration modifications and references which we have seen.
  UsageInfoMap UsageMap;
  /// The region we are currently within.
  SequenceTree::Seq Region;
  /// Filled in with declarations which were modified as a side-effect
  /// (that is, post-increment operations).
  SmallVectorImpl<std::pair<Object, Usage> > *ModAsSideEffect;
  /// Expressions to check later. We defer checking these to reduce
  /// stack usage.
  SmallVectorImpl<Expr *> &WorkList;

  /// RAII object wrapping the visitation of a sequenced subexpression of an
  /// expression. At the end of this process, the side-effects of the evaluation
  /// become sequenced with respect to the value computation of the result, so
  /// we downgrade any UK_ModAsSideEffect within the evaluation to
  /// UK_ModAsValue.
  struct SequencedSubexpression {
    SequencedSubexpression(SequenceChecker &Self)
      : Self(Self), OldModAsSideEffect(Self.ModAsSideEffect) {
      Self.ModAsSideEffect = &ModAsSideEffect;
    }
    ~SequencedSubexpression() {
      for (auto MI = ModAsSideEffect.rbegin(), ME = ModAsSideEffect.rend();
           MI != ME; ++MI) {
        UsageInfo &U = Self.UsageMap[MI->first];
        auto &SideEffectUsage = U.Uses[UK_ModAsSideEffect];
        Self.addUsage(U, MI->first, SideEffectUsage.Use, UK_ModAsValue);
        SideEffectUsage = MI->second;
      }
      Self.ModAsSideEffect = OldModAsSideEffect;
    }

    SequenceChecker &Self;
    SmallVector<std::pair<Object, Usage>, 4> ModAsSideEffect;
    SmallVectorImpl<std::pair<Object, Usage> > *OldModAsSideEffect;
  };

  /// RAII object wrapping the visitation of a subexpression which we might
  /// choose to evaluate as a constant. If any subexpression is evaluated and
  /// found to be non-constant, this allows us to suppress the evaluation of
  /// the outer expression.
  class EvaluationTracker {
  public:
    EvaluationTracker(SequenceChecker &Self)
        : Self(Self), Prev(Self.EvalTracker), EvalOK(true) {
      Self.EvalTracker = this;
    }
    ~EvaluationTracker() {
      Self.EvalTracker = Prev;
      if (Prev)
        Prev->EvalOK &= EvalOK;
    }

    bool evaluate(const Expr *E, bool &Result) {
      if (!EvalOK || E->isValueDependent())
        return false;
      EvalOK = E->EvaluateAsBooleanCondition(Result, Self.SemaRef.Context);
      return EvalOK;
    }

  private:
    SequenceChecker &Self;
    EvaluationTracker *Prev;
    bool EvalOK;
  } *EvalTracker;

  /// \brief Find the object which is produced by the specified expression,
  /// if any.
  Object getObject(Expr *E, bool Mod) const {
    E = E->IgnoreParenCasts();
    if (UnaryOperator *UO = dyn_cast<UnaryOperator>(E)) {
      if (Mod && (UO->getOpcode() == UO_PreInc || UO->getOpcode() == UO_PreDec))
        return getObject(UO->getSubExpr(), Mod);
    } else if (BinaryOperator *BO = dyn_cast<BinaryOperator>(E)) {
      if (BO->getOpcode() == BO_Comma)
        return getObject(BO->getRHS(), Mod);
      if (Mod && BO->isAssignmentOp())
        return getObject(BO->getLHS(), Mod);
    } else if (MemberExpr *ME = dyn_cast<MemberExpr>(E)) {
      // FIXME: Check for more interesting cases, like "x.n = ++x.n".
      if (isa<CXXThisExpr>(ME->getBase()->IgnoreParenCasts()))
        return ME->getMemberDecl();
    } else if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(E))
      // FIXME: If this is a reference, map through to its value.
      return DRE->getDecl();
    return nullptr;
  }

  /// \brief Note that an object was modified or used by an expression.
  void addUsage(UsageInfo &UI, Object O, Expr *Ref, UsageKind UK) {
    Usage &U = UI.Uses[UK];
    if (!U.Use || !Tree.isUnsequenced(Region, U.Seq)) {
      if (UK == UK_ModAsSideEffect && ModAsSideEffect)
        ModAsSideEffect->push_back(std::make_pair(O, U));
      U.Use = Ref;
      U.Seq = Region;
    }
  }
  /// \brief Check whether a modification or use conflicts with a prior usage.
  void checkUsage(Object O, UsageInfo &UI, Expr *Ref, UsageKind OtherKind,
                  bool IsModMod) {
    if (UI.Diagnosed)
      return;

    const Usage &U = UI.Uses[OtherKind];
    if (!U.Use || !Tree.isUnsequenced(Region, U.Seq))
      return;

    Expr *Mod = U.Use;
    Expr *ModOrUse = Ref;
    if (OtherKind == UK_Use)
      std::swap(Mod, ModOrUse);

    SemaRef.Diag(Mod->getExprLoc(),
                 IsModMod ? diag::warn_unsequenced_mod_mod
                          : diag::warn_unsequenced_mod_use)
      << O << SourceRange(ModOrUse->getExprLoc());
    UI.Diagnosed = true;
  }

  void notePreUse(Object O, Expr *Use) {
    UsageInfo &U = UsageMap[O];
    // Uses conflict with other modifications.
    checkUsage(O, U, Use, UK_ModAsValue, false);
  }
  void notePostUse(Object O, Expr *Use) {
    UsageInfo &U = UsageMap[O];
    checkUsage(O, U, Use, UK_ModAsSideEffect, false);
    addUsage(U, O, Use, UK_Use);
  }

  void notePreMod(Object O, Expr *Mod) {
    UsageInfo &U = UsageMap[O];
    // Modifications conflict with other modifications and with uses.
    checkUsage(O, U, Mod, UK_ModAsValue, true);
    checkUsage(O, U, Mod, UK_Use, false);
  }
  void notePostMod(Object O, Expr *Use, UsageKind UK) {
    UsageInfo &U = UsageMap[O];
    checkUsage(O, U, Use, UK_ModAsSideEffect, true);
    addUsage(U, O, Use, UK);
  }

public:
  SequenceChecker(Sema &S, Expr *E, SmallVectorImpl<Expr *> &WorkList)
      : Base(S.Context), SemaRef(S), Region(Tree.root()),
        ModAsSideEffect(nullptr), WorkList(WorkList), EvalTracker(nullptr) {
    Visit(E);
  }

  void VisitStmt(Stmt *S) {
    // Skip all statements which aren't expressions for now.
  }

  void VisitExpr(Expr *E) {
    // By default, just recurse to evaluated subexpressions.
    Base::VisitStmt(E);
  }

  void VisitCastExpr(CastExpr *E) {
    Object O = Object();
    if (E->getCastKind() == CK_LValueToRValue)
      O = getObject(E->getSubExpr(), false);

    if (O)
      notePreUse(O, E);
    VisitExpr(E);
    if (O)
      notePostUse(O, E);
  }

  void VisitBinComma(BinaryOperator *BO) {
    // C++11 [expr.comma]p1:
    //   Every value computation and side effect associated with the left
    //   expression is sequenced before every value computation and side
    //   effect associated with the right expression.
    SequenceTree::Seq LHS = Tree.allocate(Region);
    SequenceTree::Seq RHS = Tree.allocate(Region);
    SequenceTree::Seq OldRegion = Region;

    {
      SequencedSubexpression SeqLHS(*this);
      Region = LHS;
      Visit(BO->getLHS());
    }

    Region = RHS;
    Visit(BO->getRHS());

    Region = OldRegion;

    // Forget that LHS and RHS are sequenced. They are both unsequenced
    // with respect to other stuff.
    Tree.merge(LHS);
    Tree.merge(RHS);
  }

  void VisitBinAssign(BinaryOperator *BO) {
    // The modification is sequenced after the value computation of the LHS
    // and RHS, so check it before inspecting the operands and update the
    // map afterwards.
    Object O = getObject(BO->getLHS(), true);
    if (!O)
      return VisitExpr(BO);

    notePreMod(O, BO);

    // C++11 [expr.ass]p7:
    //   E1 op= E2 is equivalent to E1 = E1 op E2, except that E1 is evaluated
    //   only once.
    //
    // Therefore, for a compound assignment operator, O is considered used
    // everywhere except within the evaluation of E1 itself.
    if (isa<CompoundAssignOperator>(BO))
      notePreUse(O, BO);

    Visit(BO->getLHS());

    if (isa<CompoundAssignOperator>(BO))
      notePostUse(O, BO);

    Visit(BO->getRHS());

    // C++11 [expr.ass]p1:
    //   the assignment is sequenced [...] before the value computation of the
    //   assignment expression.
    // C11 6.5.16/3 has no such rule.
    notePostMod(O, BO, SemaRef.getLangOpts().CPlusPlus ? UK_ModAsValue
                                                       : UK_ModAsSideEffect);
  }
  void VisitCompoundAssignOperator(CompoundAssignOperator *CAO) {
    VisitBinAssign(CAO);
  }

  void VisitUnaryPreInc(UnaryOperator *UO) { VisitUnaryPreIncDec(UO); }
  void VisitUnaryPreDec(UnaryOperator *UO) { VisitUnaryPreIncDec(UO); }
  void VisitUnaryPreIncDec(UnaryOperator *UO) {
    Object O = getObject(UO->getSubExpr(), true);
    if (!O)
      return VisitExpr(UO);

    notePreMod(O, UO);
    Visit(UO->getSubExpr());
    // C++11 [expr.pre.incr]p1:
    //   the expression ++x is equivalent to x+=1
    notePostMod(O, UO, SemaRef.getLangOpts().CPlusPlus ? UK_ModAsValue
                                                       : UK_ModAsSideEffect);
  }

  void VisitUnaryPostInc(UnaryOperator *UO) { VisitUnaryPostIncDec(UO); }
  void VisitUnaryPostDec(UnaryOperator *UO) { VisitUnaryPostIncDec(UO); }
  void VisitUnaryPostIncDec(UnaryOperator *UO) {
    Object O = getObject(UO->getSubExpr(), true);
    if (!O)
      return VisitExpr(UO);

    notePreMod(O, UO);
    Visit(UO->getSubExpr());
    notePostMod(O, UO, UK_ModAsSideEffect);
  }

  /// Don't visit the RHS of '&&' or '||' if it might not be evaluated.
  void VisitBinLOr(BinaryOperator *BO) {
    // The side-effects of the LHS of an '&&' are sequenced before the
    // value computation of the RHS, and hence before the value computation
    // of the '&&' itself, unless the LHS evaluates to zero. We treat them
    // as if they were unconditionally sequenced.
    EvaluationTracker Eval(*this);
    {
      SequencedSubexpression Sequenced(*this);
      Visit(BO->getLHS());
    }

    bool Result;
    if (Eval.evaluate(BO->getLHS(), Result)) {
      if (!Result)
        Visit(BO->getRHS());
    } else {
      // Check for unsequenced operations in the RHS, treating it as an
      // entirely separate evaluation.
      //
      // FIXME: If there are operations in the RHS which are unsequenced
      // with respect to operations outside the RHS, and those operations
      // are unconditionally evaluated, diagnose them.
      WorkList.push_back(BO->getRHS());
    }
  }
  void VisitBinLAnd(BinaryOperator *BO) {
    EvaluationTracker Eval(*this);
    {
      SequencedSubexpression Sequenced(*this);
      Visit(BO->getLHS());
    }

    bool Result;
    if (Eval.evaluate(BO->getLHS(), Result)) {
      if (Result)
        Visit(BO->getRHS());
    } else {
      WorkList.push_back(BO->getRHS());
    }
  }

  // Only visit the condition, unless we can be sure which subexpression will
  // be chosen.
  void VisitAbstractConditionalOperator(AbstractConditionalOperator *CO) {
    EvaluationTracker Eval(*this);
    {
      SequencedSubexpression Sequenced(*this);
      Visit(CO->getCond());
    }

    bool Result;
    if (Eval.evaluate(CO->getCond(), Result))
      Visit(Result ? CO->getTrueExpr() : CO->getFalseExpr());
    else {
      WorkList.push_back(CO->getTrueExpr());
      WorkList.push_back(CO->getFalseExpr());
    }
  }

  void VisitCallExpr(CallExpr *CE) {
    // C++11 [intro.execution]p15:
    //   When calling a function [...], every value computation and side effect
    //   associated with any argument expression, or with the postfix expression
    //   designating the called function, is sequenced before execution of every
    //   expression or statement in the body of the function [and thus before
    //   the value computation of its result].
    SequencedSubexpression Sequenced(*this);
    Base::VisitCallExpr(CE);

    // FIXME: CXXNewExpr and CXXDeleteExpr implicitly call functions.
  }

  void VisitCXXConstructExpr(CXXConstructExpr *CCE) {
    // This is a call, so all subexpressions are sequenced before the result.
    SequencedSubexpression Sequenced(*this);

    if (!CCE->isListInitialization())
      return VisitExpr(CCE);

    // In C++11, list initializations are sequenced.
    SmallVector<SequenceTree::Seq, 32> Elts;
    SequenceTree::Seq Parent = Region;
    for (CXXConstructExpr::arg_iterator I = CCE->arg_begin(),
                                        E = CCE->arg_end();
         I != E; ++I) {
      Region = Tree.allocate(Parent);
      Elts.push_back(Region);
      Visit(*I);
    }

    // Forget that the initializers are sequenced.
    Region = Parent;
    for (unsigned I = 0; I < Elts.size(); ++I)
      Tree.merge(Elts[I]);
  }

  void VisitInitListExpr(InitListExpr *ILE) {
    if (!SemaRef.getLangOpts().CPlusPlus11)
      return VisitExpr(ILE);

    // In C++11, list initializations are sequenced.
    SmallVector<SequenceTree::Seq, 32> Elts;
    SequenceTree::Seq Parent = Region;
    for (unsigned I = 0; I < ILE->getNumInits(); ++I) {
      Expr *E = ILE->getInit(I);
      if (!E) continue;
      Region = Tree.allocate(Parent);
      Elts.push_back(Region);
      Visit(E);
    }

    // Forget that the initializers are sequenced.
    Region = Parent;
    for (unsigned I = 0; I < Elts.size(); ++I)
      Tree.merge(Elts[I]);
  }
};
}

void Sema::CheckUnsequencedOperations(Expr *E) {
  SmallVector<Expr *, 8> WorkList;
  WorkList.push_back(E);
  while (!WorkList.empty()) {
    Expr *Item = WorkList.pop_back_val();
    SequenceChecker(*this, Item, WorkList);
  }
}

void Sema::CheckCompletedExpr(Expr *E, SourceLocation CheckLoc,
                              bool IsConstexpr) {
  CheckImplicitConversions(E, CheckLoc);
  CheckUnsequencedOperations(E);
  if (!IsConstexpr && !E->isValueDependent())
    CheckForIntOverflow(E);
}

void Sema::CheckBitFieldInitialization(SourceLocation InitLoc,
                                       FieldDecl *BitField,
                                       Expr *Init) {
  (void) AnalyzeBitFieldAssignment(*this, BitField, Init, InitLoc);
}

static void diagnoseArrayStarInParamType(Sema &S, QualType PType,
                                         SourceLocation Loc) {
  if (!PType->isVariablyModifiedType())
    return;
  if (const auto *PointerTy = dyn_cast<PointerType>(PType)) {
    diagnoseArrayStarInParamType(S, PointerTy->getPointeeType(), Loc);
    return;
  }
  if (const auto *ReferenceTy = dyn_cast<ReferenceType>(PType)) {
    diagnoseArrayStarInParamType(S, ReferenceTy->getPointeeType(), Loc);
    return;
  }
  if (const auto *ParenTy = dyn_cast<ParenType>(PType)) {
    diagnoseArrayStarInParamType(S, ParenTy->getInnerType(), Loc);
    return;
  }

  const ArrayType *AT = S.Context.getAsArrayType(PType);
  if (!AT)
    return;

  if (AT->getSizeModifier() != ArrayType::Star) {
    diagnoseArrayStarInParamType(S, AT->getElementType(), Loc);
    return;
  }

  S.Diag(Loc, diag::err_array_star_in_function_definition);
}

/// CheckParmsForFunctionDef - Check that the parameters of the given
/// function are appropriate for the definition of a function. This
/// takes care of any checks that cannot be performed on the
/// declaration itself, e.g., that the types of each of the function
/// parameters are complete.
bool Sema::CheckParmsForFunctionDef(ParmVarDecl *const *P,
                                    ParmVarDecl *const *PEnd,
                                    bool CheckParameterNames) {
  bool HasInvalidParm = false;
  for (; P != PEnd; ++P) {
    ParmVarDecl *Param = *P;
    
    // C99 6.7.5.3p4: the parameters in a parameter type list in a
    // function declarator that is part of a function definition of
    // that function shall not have incomplete type.
    //
    // This is also C++ [dcl.fct]p6.
    if (!Param->isInvalidDecl() &&
        !(getLangOpts().HLSL && Param->getType()->isIncompleteArrayType()) &&  // HLSL Change: allow incomplete array type
        RequireCompleteType(Param->getLocation(), Param->getType(),
                            diag::err_typecheck_decl_incomplete_type)) {
      Param->setInvalidDecl();
      HasInvalidParm = true;
    }

    // C99 6.9.1p5: If the declarator includes a parameter type list, the
    // declaration of each parameter shall include an identifier.
    if (CheckParameterNames &&
        Param->getIdentifier() == nullptr &&
        !Param->isImplicit() &&
        !getLangOpts().CPlusPlus)
      Diag(Param->getLocation(), diag::err_parameter_name_omitted);

    // C99 6.7.5.3p12:
    //   If the function declarator is not part of a definition of that
    //   function, parameters may have incomplete type and may use the [*]
    //   notation in their sequences of declarator specifiers to specify
    //   variable length array types.
    QualType PType = Param->getOriginalType();
    // FIXME: This diagnostic should point the '[*]' if source-location
    // information is added for it.
    diagnoseArrayStarInParamType(*this, PType, Param->getLocation());

    // MSVC destroys objects passed by value in the callee.  Therefore a
    // function definition which takes such a parameter must be able to call the
    // object's destructor.  However, we don't perform any direct access check
    // on the dtor.
    if (getLangOpts().CPlusPlus && Context.getTargetInfo()
                                       .getCXXABI()
                                       .areArgsDestroyedLeftToRightInCallee()) {
      if (!Param->isInvalidDecl()) {
        if (const RecordType *RT = Param->getType()->getAs<RecordType>()) {
          CXXRecordDecl *ClassDecl = cast<CXXRecordDecl>(RT->getDecl());
          if (!ClassDecl->isInvalidDecl() &&
              !ClassDecl->hasIrrelevantDestructor() &&
              !ClassDecl->isDependentContext()) {
            CXXDestructorDecl *Destructor = LookupDestructor(ClassDecl);
            MarkFunctionReferenced(Param->getLocation(), Destructor);
            DiagnoseUseOfDecl(Destructor, Param->getLocation());
          }
        }
      }
    }
  }

  return HasInvalidParm;
}

/// CheckCastAlign - Implements -Wcast-align, which warns when a
/// pointer cast increases the alignment requirements.
void Sema::CheckCastAlign(Expr *Op, QualType T, SourceRange TRange) {
  // This is actually a lot of work to potentially be doing on every
  // cast; don't do it if we're ignoring -Wcast_align (as is the default).
  if (getDiagnostics().isIgnored(diag::warn_cast_align, TRange.getBegin()))
    return;

  // Ignore dependent types.
  if (T->isDependentType() || Op->getType()->isDependentType())
    return;

  // Require that the destination be a pointer type.
  const PointerType *DestPtr = T->getAs<PointerType>();
  if (!DestPtr) return;

  // If the destination has alignment 1, we're done.
  QualType DestPointee = DestPtr->getPointeeType();
  if (DestPointee->isIncompleteType()) return;
  CharUnits DestAlign = Context.getTypeAlignInChars(DestPointee);
  if (DestAlign.isOne()) return;

  // Require that the source be a pointer type.
  const PointerType *SrcPtr = Op->getType()->getAs<PointerType>();
  if (!SrcPtr) return;
  QualType SrcPointee = SrcPtr->getPointeeType();

  // Whitelist casts from cv void*.  We already implicitly
  // whitelisted casts to cv void*, since they have alignment 1.
  // Also whitelist casts involving incomplete types, which implicitly
  // includes 'void'.
  if (SrcPointee->isIncompleteType()) return;

  CharUnits SrcAlign = Context.getTypeAlignInChars(SrcPointee);
  if (SrcAlign >= DestAlign) return;

  Diag(TRange.getBegin(), diag::warn_cast_align)
    << Op->getType() << T
    << static_cast<unsigned>(SrcAlign.getQuantity())
    << static_cast<unsigned>(DestAlign.getQuantity())
    << TRange << Op->getSourceRange();
}

static const Type* getElementType(const Expr *BaseExpr) {
  const Type* EltType = BaseExpr->getType().getTypePtr();
  if (EltType->isAnyPointerType())
    return EltType->getPointeeType().getTypePtr();
  else if (EltType->isArrayType())
    return EltType->getBaseElementTypeUnsafe();
  return EltType;
}

/// \brief Check whether this array fits the idiom of a size-one tail padded
/// array member of a struct.
///
/// We avoid emitting out-of-bounds access warnings for such arrays as they are
/// commonly used to emulate flexible arrays in C89 code.
static bool IsTailPaddedMemberArray(Sema &S, llvm::APInt Size,
                                    const NamedDecl *ND) {
  if (Size != 1 || !ND) return false;

  const FieldDecl *FD = dyn_cast<FieldDecl>(ND);
  if (!FD) return false;

  // Don't consider sizes resulting from macro expansions or template argument
  // substitution to form C89 tail-padded arrays.

  TypeSourceInfo *TInfo = FD->getTypeSourceInfo();
  while (TInfo) {
    TypeLoc TL = TInfo->getTypeLoc();
    // Look through typedefs.
    if (TypedefTypeLoc TTL = TL.getAs<TypedefTypeLoc>()) {
      const TypedefNameDecl *TDL = TTL.getTypedefNameDecl();
      TInfo = TDL->getTypeSourceInfo();
      continue;
    }
    if (ConstantArrayTypeLoc CTL = TL.getAs<ConstantArrayTypeLoc>()) {
      const Expr *SizeExpr = dyn_cast<IntegerLiteral>(CTL.getSizeExpr());
      if (!SizeExpr || SizeExpr->getExprLoc().isMacroID())
        return false;
    }
    break;
  }

  const RecordDecl *RD = dyn_cast<RecordDecl>(FD->getDeclContext());
  if (!RD) return false;
  if (RD->isUnion()) return false;
  if (const CXXRecordDecl *CRD = dyn_cast<CXXRecordDecl>(RD)) {
    if (!CRD->isStandardLayout()) return false;
  }

  // See if this is the last field decl in the record.
  const Decl *D = FD;
  while ((D = D->getNextDeclInContext()))
    if (isa<FieldDecl>(D))
      return false;
  return true;
}


void Sema::CheckArrayAccess(const Expr *BaseExpr, const Expr *IndexExpr,
                            const ArraySubscriptExpr *ASE,
                            bool AllowOnePastEnd, bool IndexNegated) {
  IndexExpr = IndexExpr->IgnoreParenImpCasts();
  if (IndexExpr->isValueDependent())
    return;

  const Type *EffectiveType = getElementType(BaseExpr);
  BaseExpr = BaseExpr->IgnoreParenCasts();
  const ConstantArrayType *ArrayTy =
    Context.getAsConstantArrayType(BaseExpr->getType());
  if (!ArrayTy)
    return;

  llvm::APSInt index;
  if (!IndexExpr->EvaluateAsInt(index, Context))
    return;
  if (IndexNegated)
    index = -index;

  const NamedDecl *ND = nullptr;
  if (const DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(BaseExpr))
    ND = dyn_cast<NamedDecl>(DRE->getDecl());
  if (const MemberExpr *ME = dyn_cast<MemberExpr>(BaseExpr))
    ND = dyn_cast<NamedDecl>(ME->getMemberDecl());

  if (index.isUnsigned() || !index.isNegative()) {
    llvm::APInt size = ArrayTy->getSize();
    if (!size.isStrictlyPositive())
      return;

    const Type* BaseType = getElementType(BaseExpr);
    if (BaseType != EffectiveType) {
      // Make sure we're comparing apples to apples when comparing index to size
      uint64_t ptrarith_typesize = Context.getTypeSize(EffectiveType);
      uint64_t array_typesize = Context.getTypeSize(BaseType);
      // Handle ptrarith_typesize being zero, such as when casting to void*
      if (!ptrarith_typesize) ptrarith_typesize = 1;
      if (ptrarith_typesize != array_typesize) {
        // There's a cast to a different size type involved
        uint64_t ratio = array_typesize / ptrarith_typesize;
        // TODO: Be smarter about handling cases where array_typesize is not a
        // multiple of ptrarith_typesize
        if (ptrarith_typesize * ratio == array_typesize)
          size *= llvm::APInt(size.getBitWidth(), ratio);
      }
    }

    if (size.getBitWidth() > index.getBitWidth())
      index = index.zext(size.getBitWidth());
    else if (size.getBitWidth() < index.getBitWidth())
      size = size.zext(index.getBitWidth());

    // For array subscripting the index must be less than size, but for pointer
    // arithmetic also allow the index (offset) to be equal to size since
    // computing the next address after the end of the array is legal and
    // commonly done e.g. in C++ iterators and range-based for loops.
    if (AllowOnePastEnd ? index.ule(size) : index.ult(size))
      return;

    // Also don't warn for arrays of size 1 which are members of some
    // structure. These are often used to approximate flexible arrays in C89
    // code.
    if (IsTailPaddedMemberArray(*this, size, ND))
      return;

    // Suppress the warning if the subscript expression (as identified by the
    // ']' location) and the index expression are both from macro expansions
    // within a system header.
    if (ASE) {
      SourceLocation RBracketLoc = SourceMgr.getSpellingLoc(
          ASE->getRBracketLoc());
      if (SourceMgr.isInSystemHeader(RBracketLoc)) {
        SourceLocation IndexLoc = SourceMgr.getSpellingLoc(
            IndexExpr->getLocStart());
        if (SourceMgr.isWrittenInSameFile(RBracketLoc, IndexLoc))
          return;
      }
    }

    // HLSL Change Starts
    if (getLangOpts().HLSL && getLangOpts().HLSLVersion > hlsl::LangStd::v2016) {
      DiagRuntimeBehavior(BaseExpr->getLocStart(), BaseExpr,
        PDiag(diag::err_hlsl_array_element_index_out_of_bounds) << index.toString(10, true));
    }
    else {
    // HLSL Change Ends

    unsigned DiagID = diag::warn_ptr_arith_exceeds_bounds;
    if (ASE)
      DiagID = diag::warn_array_index_exceeds_bounds;

    DiagRuntimeBehavior(BaseExpr->getLocStart(), BaseExpr,
                        PDiag(DiagID) << index.toString(10, true)
                          << size.toString(10, true)
                          << (unsigned)size.getLimitedValue(~0U)
                          << IndexExpr->getSourceRange());
    } // HLSL Change
  } else {
    // HLSL Change Starts
    if (getLangOpts().HLSL && getLangOpts().HLSLVersion > hlsl::LangStd::v2016) {
      DiagRuntimeBehavior(BaseExpr->getLocStart(), BaseExpr,
        PDiag(diag::err_hlsl_array_element_index_out_of_bounds) << index.toString(10, true));
    }
    else {
    // HLSL Change Ends

    unsigned DiagID = diag::warn_array_index_precedes_bounds;
    if (!ASE) {
      DiagID = diag::warn_ptr_arith_precedes_bounds;
      if (index.isNegative()) index = -index;
    }

    DiagRuntimeBehavior(BaseExpr->getLocStart(), BaseExpr,
                        PDiag(DiagID) << index.toString(10, true)
                          << IndexExpr->getSourceRange());
    } // HLSL Change
  }

  if (!ND) {
    // Try harder to find a NamedDecl to point at in the note.
    while (const ArraySubscriptExpr *ASE =
           dyn_cast<ArraySubscriptExpr>(BaseExpr))
      BaseExpr = ASE->getBase()->IgnoreParenCasts();
    if (const DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(BaseExpr))
      ND = dyn_cast<NamedDecl>(DRE->getDecl());
    if (const MemberExpr *ME = dyn_cast<MemberExpr>(BaseExpr))
      ND = dyn_cast<NamedDecl>(ME->getMemberDecl());
  }

  if (ND)
    DiagRuntimeBehavior(ND->getLocStart(), BaseExpr,
                        PDiag(diag::note_array_index_out_of_bounds)
                          << ND->getDeclName());
}

void Sema::CheckArrayAccess(const Expr *expr) {
  int AllowOnePastEnd = 0;
  while (expr) {
    expr = expr->IgnoreParenImpCasts();
    switch (expr->getStmtClass()) {
      case Stmt::ArraySubscriptExprClass: {
        const ArraySubscriptExpr *ASE = cast<ArraySubscriptExpr>(expr);
        CheckArrayAccess(ASE->getBase(), ASE->getIdx(), ASE,
                         AllowOnePastEnd > 0);
        return;
      }
      case Stmt::UnaryOperatorClass: {
        // Only unwrap the * and & unary operators
        const UnaryOperator *UO = cast<UnaryOperator>(expr);
        expr = UO->getSubExpr();
        switch (UO->getOpcode()) {
          case UO_AddrOf:
            AllowOnePastEnd++;
            break;
          case UO_Deref:
            AllowOnePastEnd--;
            break;
          default:
            return;
        }
        break;
      }
      case Stmt::ConditionalOperatorClass: {
        const ConditionalOperator *cond = cast<ConditionalOperator>(expr);
        if (const Expr *lhs = cond->getLHS())
          CheckArrayAccess(lhs);
        if (const Expr *rhs = cond->getRHS())
          CheckArrayAccess(rhs);
        return;
      }
      // HLSL Change Starts : Access checking for HLSL vector and matrix array subscript
      case Stmt::CXXOperatorCallExprClass : {
        if (getLangOpts().HLSL) {
          const CXXOperatorCallExpr *OperatorCallExpr =
              cast<CXXOperatorCallExpr>(expr);
          if (OperatorCallExpr->getOperator() ==
              OverloadedOperatorKind::OO_Subscript) {
            CheckHLSLArrayAccess(expr);
          }
        }
        return;
      }
      // HLSL Change Ends
      default:
        return;
    }
  }
}

//===--- CHECK: Objective-C retain cycles ----------------------------------//

namespace {
  struct RetainCycleOwner {
    RetainCycleOwner() : Variable(nullptr), Indirect(false) {}
    VarDecl *Variable;
    SourceRange Range;
    SourceLocation Loc;
    bool Indirect;

    void setLocsFrom(Expr *e) {
      Loc = e->getExprLoc();
      Range = e->getSourceRange();
    }
  };
}

#if 0 // HLSL Change Starts
/// Consider whether capturing the given variable can possibly lead to
/// a retain cycle.
static bool considerVariable(VarDecl *var, Expr *ref, RetainCycleOwner &owner) {
  // In ARC, it's captured strongly iff the variable has __strong
  // lifetime.  In MRR, it's captured strongly if the variable is
  // __block and has an appropriate type.
  if (var->getType().getObjCLifetime() != Qualifiers::OCL_Strong)
    return false;

  owner.Variable = var;
  if (ref)
    owner.setLocsFrom(ref);
  return true;
}

static bool findRetainCycleOwner(Sema &S, Expr *e, RetainCycleOwner &owner) {
  while (true) {
    e = e->IgnoreParens();
    if (CastExpr *cast = dyn_cast<CastExpr>(e)) {
      switch (cast->getCastKind()) {
      case CK_BitCast:
      case CK_LValueBitCast:
      case CK_LValueToRValue:
      case CK_ARCReclaimReturnedObject:
        e = cast->getSubExpr();
        continue;

      default:
        return false;
      }
    }

    if (ObjCIvarRefExpr *ref = dyn_cast<ObjCIvarRefExpr>(e)) {
      ObjCIvarDecl *ivar = ref->getDecl();
      if (ivar->getType().getObjCLifetime() != Qualifiers::OCL_Strong)
        return false;

      // Try to find a retain cycle in the base.
      if (!findRetainCycleOwner(S, ref->getBase(), owner))
        return false;

      if (ref->isFreeIvar()) owner.setLocsFrom(ref);
      owner.Indirect = true;
      return true;
    }

    if (DeclRefExpr *ref = dyn_cast<DeclRefExpr>(e)) {
      VarDecl *var = dyn_cast<VarDecl>(ref->getDecl());
      if (!var) return false;
      return considerVariable(var, ref, owner);
    }

    if (MemberExpr *member = dyn_cast<MemberExpr>(e)) {
      if (member->isArrow()) return false;

      // Don't count this as an indirect ownership.
      e = member->getBase();
      continue;
    }

    if (PseudoObjectExpr *pseudo = dyn_cast<PseudoObjectExpr>(e)) {
      // Only pay attention to pseudo-objects on property references.
      ObjCPropertyRefExpr *pre
        = dyn_cast<ObjCPropertyRefExpr>(pseudo->getSyntacticForm()
                                              ->IgnoreParens());
      if (!pre) return false;
      if (pre->isImplicitProperty()) return false;
      ObjCPropertyDecl *property = pre->getExplicitProperty();
      if (!property->isRetaining() &&
          !(property->getPropertyIvarDecl() &&
            property->getPropertyIvarDecl()->getType()
              .getObjCLifetime() == Qualifiers::OCL_Strong))
          return false;

      owner.Indirect = true;
      if (pre->isSuperReceiver()) {
        owner.Variable = S.getCurMethodDecl()->getSelfDecl();
        if (!owner.Variable)
          return false;
        owner.Loc = pre->getLocation();
        owner.Range = pre->getSourceRange();
        return true;
      }
      e = const_cast<Expr*>(cast<OpaqueValueExpr>(pre->getBase())
                              ->getSourceExpr());
      continue;
    }

    // Array ivars?

    return false;
  }
}
#endif // HLSL Change Ends
namespace {
  struct FindCaptureVisitor : EvaluatedExprVisitor<FindCaptureVisitor> {
    FindCaptureVisitor(ASTContext &Context, VarDecl *variable)
      : EvaluatedExprVisitor<FindCaptureVisitor>(Context),
        Context(Context), Variable(variable), Capturer(nullptr),
        VarWillBeReased(false) {}
    ASTContext &Context;
    VarDecl *Variable;
    Expr *Capturer;
    bool VarWillBeReased;

    void VisitDeclRefExpr(DeclRefExpr *ref) {
      if (ref->getDecl() == Variable && !Capturer)
        Capturer = ref;
    }

    void VisitObjCIvarRefExpr(ObjCIvarRefExpr *ref) {
      if (Capturer) return;
      Visit(ref->getBase());
      if (Capturer && ref->isFreeIvar())
        Capturer = ref;
    }

    void VisitBlockExpr(BlockExpr *block) {
      // Look inside nested blocks 
      if (block->getBlockDecl()->capturesVariable(Variable))
        Visit(block->getBlockDecl()->getBody());
    }
    
    void VisitOpaqueValueExpr(OpaqueValueExpr *OVE) {
      if (Capturer) return;
      if (OVE->getSourceExpr())
        Visit(OVE->getSourceExpr());
    }
    void VisitBinaryOperator(BinaryOperator *BinOp) {
      if (!Variable || VarWillBeReased || BinOp->getOpcode() != BO_Assign)
        return;
      Expr *LHS = BinOp->getLHS();
      if (const DeclRefExpr *DRE = dyn_cast_or_null<DeclRefExpr>(LHS)) {
        if (DRE->getDecl() != Variable)
          return;
        if (Expr *RHS = BinOp->getRHS()) {
          RHS = RHS->IgnoreParenCasts();
          llvm::APSInt Value;
          VarWillBeReased =
            (RHS && RHS->isIntegerConstantExpr(Value, Context) && Value == 0);
        }
      }
    }
  };
}

#if 0 // HLSL Change Starts
/// Check whether the given argument is a block which captures a
/// variable.
static Expr *findCapturingExpr(Sema &S, Expr *e, RetainCycleOwner &owner) {
  assert(owner.Variable && owner.Loc.isValid());

  e = e->IgnoreParenCasts();

  // Look through [^{...} copy] and Block_copy(^{...}).
  if (ObjCMessageExpr *ME = dyn_cast<ObjCMessageExpr>(e)) {
    Selector Cmd = ME->getSelector();
    if (Cmd.isUnarySelector() && Cmd.getNameForSlot(0) == "copy") {
      e = ME->getInstanceReceiver();
      if (!e)
        return nullptr;
      e = e->IgnoreParenCasts();
    }
  } else if (CallExpr *CE = dyn_cast<CallExpr>(e)) {
    if (CE->getNumArgs() == 1) {
      FunctionDecl *Fn = dyn_cast_or_null<FunctionDecl>(CE->getCalleeDecl());
      if (Fn) {
        const IdentifierInfo *FnI = Fn->getIdentifier();
        if (FnI && FnI->isStr("_Block_copy")) {
          e = CE->getArg(0)->IgnoreParenCasts();
        }
      }
    }
  }
  
  BlockExpr *block = dyn_cast<BlockExpr>(e);
  if (!block || !block->getBlockDecl()->capturesVariable(owner.Variable))
    return nullptr;

  FindCaptureVisitor visitor(S.Context, owner.Variable);
  visitor.Visit(block->getBlockDecl()->getBody());
  return visitor.VarWillBeReased ? nullptr : visitor.Capturer;
}

static void diagnoseRetainCycle(Sema &S, Expr *capturer,
                                RetainCycleOwner &owner) {
  assert(capturer);
  assert(owner.Variable && owner.Loc.isValid());

  S.Diag(capturer->getExprLoc(), diag::warn_arc_retain_cycle)
    << owner.Variable << capturer->getSourceRange();
  S.Diag(owner.Loc, diag::note_arc_retain_cycle_owner)
    << owner.Indirect << owner.Range;
}

/// Check for a keyword selector that starts with the word 'add' or
/// 'set'.
static bool isSetterLikeSelector(Selector sel) {
  if (sel.isUnarySelector()) return false;

  StringRef str = sel.getNameForSlot(0);
  while (!str.empty() && str.front() == '_') str = str.substr(1);
  if (str.startswith("set"))
    str = str.substr(3);
  else if (str.startswith("add")) {
    // Specially whitelist 'addOperationWithBlock:'.
    if (sel.getNumArgs() == 1 && str.startswith("addOperationWithBlock"))
      return false;
    str = str.substr(3);
  }
  else
    return false;

  if (str.empty()) return true;
  return !isLowercase(str.front());
}

static Optional<int> GetNSMutableArrayArgumentIndex(Sema &S,
                                                    ObjCMessageExpr *Message) {
  bool IsMutableArray = S.NSAPIObj->isSubclassOfNSClass(
                                                Message->getReceiverInterface(),
                                                NSAPI::ClassId_NSMutableArray);
  if (!IsMutableArray) {
    return None;
  }

  Selector Sel = Message->getSelector();

  Optional<NSAPI::NSArrayMethodKind> MKOpt =
    S.NSAPIObj->getNSArrayMethodKind(Sel);
  if (!MKOpt) {
    return None;
  }

  NSAPI::NSArrayMethodKind MK = *MKOpt;

  switch (MK) {
    case NSAPI::NSMutableArr_addObject:
    case NSAPI::NSMutableArr_insertObjectAtIndex:
    case NSAPI::NSMutableArr_setObjectAtIndexedSubscript:
      return 0;
    case NSAPI::NSMutableArr_replaceObjectAtIndex:
      return 1;

    default:
      return None;
  }

  return None;
}

static
Optional<int> GetNSMutableDictionaryArgumentIndex(Sema &S,
                                                  ObjCMessageExpr *Message) {
  bool IsMutableDictionary = S.NSAPIObj->isSubclassOfNSClass(
                                            Message->getReceiverInterface(),
                                            NSAPI::ClassId_NSMutableDictionary);
  if (!IsMutableDictionary) {
    return None;
  }

  Selector Sel = Message->getSelector();

  Optional<NSAPI::NSDictionaryMethodKind> MKOpt =
    S.NSAPIObj->getNSDictionaryMethodKind(Sel);
  if (!MKOpt) {
    return None;
  }

  NSAPI::NSDictionaryMethodKind MK = *MKOpt;

  switch (MK) {
    case NSAPI::NSMutableDict_setObjectForKey:
    case NSAPI::NSMutableDict_setValueForKey:
    case NSAPI::NSMutableDict_setObjectForKeyedSubscript:
      return 0;

    default:
      return None;
  }

  return None;
}

static Optional<int> GetNSSetArgumentIndex(Sema &S, ObjCMessageExpr *Message) {
  bool IsMutableSet = S.NSAPIObj->isSubclassOfNSClass(
                                                Message->getReceiverInterface(),
                                                NSAPI::ClassId_NSMutableSet);

  bool IsMutableOrderedSet = S.NSAPIObj->isSubclassOfNSClass(
                                            Message->getReceiverInterface(),
                                            NSAPI::ClassId_NSMutableOrderedSet);
  if (!IsMutableSet && !IsMutableOrderedSet) {
    return None;
  }

  Selector Sel = Message->getSelector();

  Optional<NSAPI::NSSetMethodKind> MKOpt = S.NSAPIObj->getNSSetMethodKind(Sel);
  if (!MKOpt) {
    return None;
  }

  NSAPI::NSSetMethodKind MK = *MKOpt;

  switch (MK) {
    case NSAPI::NSMutableSet_addObject:
    case NSAPI::NSOrderedSet_setObjectAtIndex:
    case NSAPI::NSOrderedSet_setObjectAtIndexedSubscript:
    case NSAPI::NSOrderedSet_insertObjectAtIndex:
      return 0;
    case NSAPI::NSOrderedSet_replaceObjectAtIndexWithObject:
      return 1;
  }

  return None;
}

void Sema::CheckObjCCircularContainer(ObjCMessageExpr *Message) {
  if (!Message->isInstanceMessage()) {
    return;
  }

  Optional<int> ArgOpt;

  if (!(ArgOpt = GetNSMutableArrayArgumentIndex(*this, Message)) &&
      !(ArgOpt = GetNSMutableDictionaryArgumentIndex(*this, Message)) &&
      !(ArgOpt = GetNSSetArgumentIndex(*this, Message))) {
    return;
  }

  int ArgIndex = *ArgOpt;

  Expr *Arg = Message->getArg(ArgIndex)->IgnoreImpCasts();
  if (OpaqueValueExpr *OE = dyn_cast<OpaqueValueExpr>(Arg)) {
    Arg = OE->getSourceExpr()->IgnoreImpCasts();
  }

  if (Message->getReceiverKind() == ObjCMessageExpr::SuperInstance) {
    if (DeclRefExpr *ArgRE = dyn_cast<DeclRefExpr>(Arg)) {
      if (ArgRE->isObjCSelfExpr()) {
        Diag(Message->getSourceRange().getBegin(),
             diag::warn_objc_circular_container)
          << ArgRE->getDecl()->getName() << StringRef("super");
      }
    }
  } else {
    Expr *Receiver = Message->getInstanceReceiver()->IgnoreImpCasts();

    if (OpaqueValueExpr *OE = dyn_cast<OpaqueValueExpr>(Receiver)) {
      Receiver = OE->getSourceExpr()->IgnoreImpCasts();
    }

    if (DeclRefExpr *ReceiverRE = dyn_cast<DeclRefExpr>(Receiver)) {
      if (DeclRefExpr *ArgRE = dyn_cast<DeclRefExpr>(Arg)) {
        if (ReceiverRE->getDecl() == ArgRE->getDecl()) {
          ValueDecl *Decl = ReceiverRE->getDecl();
          Diag(Message->getSourceRange().getBegin(),
               diag::warn_objc_circular_container)
            << Decl->getName() << Decl->getName();
          if (!ArgRE->isObjCSelfExpr()) {
            Diag(Decl->getLocation(),
                 diag::note_objc_circular_container_declared_here)
              << Decl->getName();
          }
        }
      }
    } else if (ObjCIvarRefExpr *IvarRE = dyn_cast<ObjCIvarRefExpr>(Receiver)) {
      if (ObjCIvarRefExpr *IvarArgRE = dyn_cast<ObjCIvarRefExpr>(Arg)) {
        if (IvarRE->getDecl() == IvarArgRE->getDecl()) {
          ObjCIvarDecl *Decl = IvarRE->getDecl();
          Diag(Message->getSourceRange().getBegin(),
               diag::warn_objc_circular_container)
            << Decl->getName() << Decl->getName();
          Diag(Decl->getLocation(),
               diag::note_objc_circular_container_declared_here)
            << Decl->getName();
        }
      }
    }
  }

}

/// Check a message send to see if it's likely to cause a retain cycle.
void Sema::checkRetainCycles(ObjCMessageExpr *msg) {
  // Only check instance methods whose selector looks like a setter.
  if (!msg->isInstanceMessage() || !isSetterLikeSelector(msg->getSelector()))
    return;

  // Try to find a variable that the receiver is strongly owned by.
  RetainCycleOwner owner;
  if (msg->getReceiverKind() == ObjCMessageExpr::Instance) {
    if (!findRetainCycleOwner(*this, msg->getInstanceReceiver(), owner))
      return;
  } else {
    assert(msg->getReceiverKind() == ObjCMessageExpr::SuperInstance);
    owner.Variable = getCurMethodDecl()->getSelfDecl();
    owner.Loc = msg->getSuperLoc();
    owner.Range = msg->getSuperLoc();
  }

  // Check whether the receiver is captured by any of the arguments.
  for (unsigned i = 0, e = msg->getNumArgs(); i != e; ++i)
    if (Expr *capturer = findCapturingExpr(*this, msg->getArg(i), owner))
      return diagnoseRetainCycle(*this, capturer, owner);
}

/// Check a property assign to see if it's likely to cause a retain cycle.
void Sema::checkRetainCycles(Expr *receiver, Expr *argument) {
  RetainCycleOwner owner;
  if (!findRetainCycleOwner(*this, receiver, owner))
    return;

  if (Expr *capturer = findCapturingExpr(*this, argument, owner))
    diagnoseRetainCycle(*this, capturer, owner);
}

void Sema::checkRetainCycles(VarDecl *Var, Expr *Init) {
  RetainCycleOwner Owner;
  if (!considerVariable(Var, /*DeclRefExpr=*/nullptr, Owner))
    return;
  
  // Because we don't have an expression for the variable, we have to set the
  // location explicitly here.
  Owner.Loc = Var->getLocation();
  Owner.Range = Var->getSourceRange();
  
  if (Expr *Capturer = findCapturingExpr(*this, Init, Owner))
    diagnoseRetainCycle(*this, Capturer, Owner);
}

static bool checkUnsafeAssignLiteral(Sema &S, SourceLocation Loc,
                                     Expr *RHS, bool isProperty) {
  // Check if RHS is an Objective-C object literal, which also can get
  // immediately zapped in a weak reference.  Note that we explicitly
  // allow ObjCStringLiterals, since those are designed to never really die.
  RHS = RHS->IgnoreParenImpCasts();

  // This enum needs to match with the 'select' in
  // warn_objc_arc_literal_assign (off-by-1).
  Sema::ObjCLiteralKind Kind = S.CheckLiteralKind(RHS);
  if (Kind == Sema::LK_String || Kind == Sema::LK_None)
    return false;

  S.Diag(Loc, diag::warn_arc_literal_assign)
    << (unsigned) Kind
    << (isProperty ? 0 : 1)
    << RHS->getSourceRange();

  return true;
}

static bool checkUnsafeAssignObject(Sema &S, SourceLocation Loc,
                                    Qualifiers::ObjCLifetime LT,
                                    Expr *RHS, bool isProperty) {
  // Strip off any implicit cast added to get to the one ARC-specific.
  while (ImplicitCastExpr *cast = dyn_cast<ImplicitCastExpr>(RHS)) {
    if (cast->getCastKind() == CK_ARCConsumeObject) {
      S.Diag(Loc, diag::warn_arc_retained_assign)
        << (LT == Qualifiers::OCL_ExplicitNone)
        << (isProperty ? 0 : 1)
        << RHS->getSourceRange();
      return true;
    }
    RHS = cast->getSubExpr();
  }

  if (LT == Qualifiers::OCL_Weak &&
      checkUnsafeAssignLiteral(S, Loc, RHS, isProperty))
    return true;

  return false;
}

bool Sema::checkUnsafeAssigns(SourceLocation Loc,
                              QualType LHS, Expr *RHS) {
  Qualifiers::ObjCLifetime LT = LHS.getObjCLifetime();

  if (LT != Qualifiers::OCL_Weak && LT != Qualifiers::OCL_ExplicitNone)
    return false;

  if (checkUnsafeAssignObject(*this, Loc, LT, RHS, false))
    return true;

  return false;
}

void Sema::checkUnsafeExprAssigns(SourceLocation Loc,
                              Expr *LHS, Expr *RHS) {
  QualType LHSType;
  // PropertyRef on LHS type need be directly obtained from
  // its declaration as it has a PseudoType.
  ObjCPropertyRefExpr *PRE
    = dyn_cast<ObjCPropertyRefExpr>(LHS->IgnoreParens());
  if (PRE && !PRE->isImplicitProperty()) {
    const ObjCPropertyDecl *PD = PRE->getExplicitProperty();
    if (PD)
      LHSType = PD->getType();
  }
  
  if (LHSType.isNull())
    LHSType = LHS->getType();

  Qualifiers::ObjCLifetime LT = LHSType.getObjCLifetime();

  if (LT == Qualifiers::OCL_Weak) {
    if (!Diags.isIgnored(diag::warn_arc_repeated_use_of_weak, Loc))
      getCurFunction()->markSafeWeakUse(LHS);
  }

  if (checkUnsafeAssigns(Loc, LHSType, RHS))
    return;

  // FIXME. Check for other life times.
  if (LT != Qualifiers::OCL_None)
    return;
  
  if (PRE) {
    if (PRE->isImplicitProperty())
      return;
    const ObjCPropertyDecl *PD = PRE->getExplicitProperty();
    if (!PD)
      return;
    
    unsigned Attributes = PD->getPropertyAttributes();
    if (Attributes & ObjCPropertyDecl::OBJC_PR_assign) {
      // when 'assign' attribute was not explicitly specified
      // by user, ignore it and rely on property type itself
      // for lifetime info.
      unsigned AsWrittenAttr = PD->getPropertyAttributesAsWritten();
      if (!(AsWrittenAttr & ObjCPropertyDecl::OBJC_PR_assign) &&
          LHSType->isObjCRetainableType())
        return;
        
      while (ImplicitCastExpr *cast = dyn_cast<ImplicitCastExpr>(RHS)) {
        if (cast->getCastKind() == CK_ARCConsumeObject) {
          Diag(Loc, diag::warn_arc_retained_property_assign)
          << RHS->getSourceRange();
          return;
        }
        RHS = cast->getSubExpr();
      }
    }
    else if (Attributes & ObjCPropertyDecl::OBJC_PR_weak) {
      if (checkUnsafeAssignObject(*this, Loc, Qualifiers::OCL_Weak, RHS, true))
        return;
    }
  }
}
#endif // HLSL Change

//===--- CHECK: Empty statement body (-Wempty-body) ---------------------===//

namespace {
bool ShouldDiagnoseEmptyStmtBody(const SourceManager &SourceMgr,
                                 SourceLocation StmtLoc,
                                 const NullStmt *Body) {
  // Do not warn if the body is a macro that expands to nothing, e.g:
  //
  // #define CALL(x)
  // if (condition)
  //   CALL(0);
  //
  if (Body->hasLeadingEmptyMacro())
    return false;

  // Get line numbers of statement and body.
  bool StmtLineInvalid;
  unsigned StmtLine = SourceMgr.getPresumedLineNumber(StmtLoc,
                                                      &StmtLineInvalid);
  if (StmtLineInvalid)
    return false;

  bool BodyLineInvalid;
  unsigned BodyLine = SourceMgr.getSpellingLineNumber(Body->getSemiLoc(),
                                                      &BodyLineInvalid);
  if (BodyLineInvalid)
    return false;

  // Warn if null statement and body are on the same line.
  if (StmtLine != BodyLine)
    return false;

  return true;
}
} // Unnamed namespace

void Sema::DiagnoseEmptyStmtBody(SourceLocation StmtLoc,
                                 const Stmt *Body,
                                 unsigned DiagID) {
  // Since this is a syntactic check, don't emit diagnostic for template
  // instantiations, this just adds noise.
  if (CurrentInstantiationScope)
    return;

  // The body should be a null statement.
  const NullStmt *NBody = dyn_cast<NullStmt>(Body);
  if (!NBody)
    return;

  // Do the usual checks.
  if (!ShouldDiagnoseEmptyStmtBody(SourceMgr, StmtLoc, NBody))
    return;

  Diag(NBody->getSemiLoc(), DiagID);
  Diag(NBody->getSemiLoc(), diag::note_empty_body_on_separate_line);
}

void Sema::DiagnoseEmptyLoopBody(const Stmt *S,
                                 const Stmt *PossibleBody) {
  assert(!CurrentInstantiationScope); // Ensured by caller

  SourceLocation StmtLoc;
  const Stmt *Body;
  unsigned DiagID;
  if (const ForStmt *FS = dyn_cast<ForStmt>(S)) {
    StmtLoc = FS->getRParenLoc();
    Body = FS->getBody();
    DiagID = diag::warn_empty_for_body;
  } else if (const WhileStmt *WS = dyn_cast<WhileStmt>(S)) {
    StmtLoc = WS->getCond()->getSourceRange().getEnd();
    Body = WS->getBody();
    DiagID = diag::warn_empty_while_body;
  } else
    return; // Neither `for' nor `while'.

  // The body should be a null statement.
  const NullStmt *NBody = dyn_cast<NullStmt>(Body);
  if (!NBody)
    return;

  // Skip expensive checks if diagnostic is disabled.
  if (Diags.isIgnored(DiagID, NBody->getSemiLoc()))
    return;

  // Do the usual checks.
  if (!ShouldDiagnoseEmptyStmtBody(SourceMgr, StmtLoc, NBody))
    return;

  // `for(...);' and `while(...);' are popular idioms, so in order to keep
  // noise level low, emit diagnostics only if for/while is followed by a
  // CompoundStmt, e.g.:
  //    for (int i = 0; i < n; i++);
  //    {
  //      a(i);
  //    }
  // or if for/while is followed by a statement with more indentation
  // than for/while itself:
  //    for (int i = 0; i < n; i++);
  //      a(i);
  bool ProbableTypo = isa<CompoundStmt>(PossibleBody);
  if (!ProbableTypo) {
    bool BodyColInvalid;
    unsigned BodyCol = SourceMgr.getPresumedColumnNumber(
                             PossibleBody->getLocStart(),
                             &BodyColInvalid);
    if (BodyColInvalid)
      return;

    bool StmtColInvalid;
    unsigned StmtCol = SourceMgr.getPresumedColumnNumber(
                             S->getLocStart(),
                             &StmtColInvalid);
    if (StmtColInvalid)
      return;

    if (BodyCol > StmtCol)
      ProbableTypo = true;
  }

  if (ProbableTypo) {
    Diag(NBody->getSemiLoc(), DiagID);
    Diag(NBody->getSemiLoc(), diag::note_empty_body_on_separate_line);
  }
}

//===--- CHECK: Warn on self move with std::move. -------------------------===//

/// DiagnoseSelfMove - Emits a warning if a value is moved to itself.
void Sema::DiagnoseSelfMove(const Expr *LHSExpr, const Expr *RHSExpr,
                             SourceLocation OpLoc) {

  if (Diags.isIgnored(diag::warn_sizeof_pointer_expr_memaccess, OpLoc))
    return;

  if (!ActiveTemplateInstantiations.empty())
    return;

  // Strip parens and casts away.
  LHSExpr = LHSExpr->IgnoreParenImpCasts();
  RHSExpr = RHSExpr->IgnoreParenImpCasts();

  // Check for a call expression
  const CallExpr *CE = dyn_cast<CallExpr>(RHSExpr);
  if (!CE || CE->getNumArgs() != 1)
    return;

  // Check for a call to std::move
  const FunctionDecl *FD = CE->getDirectCallee();
  if (!FD || !FD->isInStdNamespace() || !FD->getIdentifier() ||
      !FD->getIdentifier()->isStr("move"))
    return;

  // Get argument from std::move
  RHSExpr = CE->getArg(0);

  const DeclRefExpr *LHSDeclRef = dyn_cast<DeclRefExpr>(LHSExpr);
  const DeclRefExpr *RHSDeclRef = dyn_cast<DeclRefExpr>(RHSExpr);

  // Two DeclRefExpr's, check that the decls are the same.
  if (LHSDeclRef && RHSDeclRef) {
    if (!LHSDeclRef->getDecl() || !RHSDeclRef->getDecl())
      return;
    if (LHSDeclRef->getDecl()->getCanonicalDecl() !=
        RHSDeclRef->getDecl()->getCanonicalDecl())
      return;

    Diag(OpLoc, diag::warn_self_move) << LHSExpr->getType()
                                        << LHSExpr->getSourceRange()
                                        << RHSExpr->getSourceRange();
    return;
  }

  // Member variables require a different approach to check for self moves.
  // MemberExpr's are the same if every nested MemberExpr refers to the same
  // Decl and that the base Expr's are DeclRefExpr's with the same Decl or
  // the base Expr's are CXXThisExpr's.
  const Expr *LHSBase = LHSExpr;
  const Expr *RHSBase = RHSExpr;
  const MemberExpr *LHSME = dyn_cast<MemberExpr>(LHSExpr);
  const MemberExpr *RHSME = dyn_cast<MemberExpr>(RHSExpr);
  if (!LHSME || !RHSME)
    return;

  while (LHSME && RHSME) {
    if (LHSME->getMemberDecl()->getCanonicalDecl() !=
        RHSME->getMemberDecl()->getCanonicalDecl())
      return;

    LHSBase = LHSME->getBase();
    RHSBase = RHSME->getBase();
    LHSME = dyn_cast<MemberExpr>(LHSBase);
    RHSME = dyn_cast<MemberExpr>(RHSBase);
  }

  LHSDeclRef = dyn_cast<DeclRefExpr>(LHSBase);
  RHSDeclRef = dyn_cast<DeclRefExpr>(RHSBase);
  if (LHSDeclRef && RHSDeclRef) {
    if (!LHSDeclRef->getDecl() || !RHSDeclRef->getDecl())
      return;
    if (LHSDeclRef->getDecl()->getCanonicalDecl() !=
        RHSDeclRef->getDecl()->getCanonicalDecl())
      return;

    Diag(OpLoc, diag::warn_self_move) << LHSExpr->getType()
                                        << LHSExpr->getSourceRange()
                                        << RHSExpr->getSourceRange();
    return;
  }

  if (isa<CXXThisExpr>(LHSBase) && isa<CXXThisExpr>(RHSBase))
    Diag(OpLoc, diag::warn_self_move) << LHSExpr->getType()
                                        << LHSExpr->getSourceRange()
                                        << RHSExpr->getSourceRange();
}

//===--- Layout compatibility ----------------------------------------------//

namespace {

bool isLayoutCompatible(ASTContext &C, QualType T1, QualType T2);

/// \brief Check if two enumeration types are layout-compatible.
bool isLayoutCompatible(ASTContext &C, EnumDecl *ED1, EnumDecl *ED2) {
  // C++11 [dcl.enum] p8:
  // Two enumeration types are layout-compatible if they have the same
  // underlying type.
  return ED1->isComplete() && ED2->isComplete() &&
         C.hasSameType(ED1->getIntegerType(), ED2->getIntegerType());
}

/// \brief Check if two fields are layout-compatible.
bool isLayoutCompatible(ASTContext &C, FieldDecl *Field1, FieldDecl *Field2) {
  if (!isLayoutCompatible(C, Field1->getType(), Field2->getType()))
    return false;

  if (Field1->isBitField() != Field2->isBitField())
    return false;

  if (Field1->isBitField()) {
    // Make sure that the bit-fields are the same length.
    unsigned Bits1 = Field1->getBitWidthValue(C);
    unsigned Bits2 = Field2->getBitWidthValue(C);

    if (Bits1 != Bits2)
      return false;
  }

  return true;
}

/// \brief Check if two standard-layout structs are layout-compatible.
/// (C++11 [class.mem] p17)
bool isLayoutCompatibleStruct(ASTContext &C,
                              RecordDecl *RD1,
                              RecordDecl *RD2) {
  // If both records are C++ classes, check that base classes match.
  if (const CXXRecordDecl *D1CXX = dyn_cast<CXXRecordDecl>(RD1)) {
    // If one of records is a CXXRecordDecl we are in C++ mode,
    // thus the other one is a CXXRecordDecl, too.
    const CXXRecordDecl *D2CXX = cast<CXXRecordDecl>(RD2);
    // Check number of base classes.
    if (D1CXX->getNumBases() != D2CXX->getNumBases())
      return false;

    // Check the base classes.
    for (CXXRecordDecl::base_class_const_iterator
               Base1 = D1CXX->bases_begin(),
           BaseEnd1 = D1CXX->bases_end(),
              Base2 = D2CXX->bases_begin();
         Base1 != BaseEnd1;
         ++Base1, ++Base2) {
      if (!isLayoutCompatible(C, Base1->getType(), Base2->getType()))
        return false;
    }
  } else if (const CXXRecordDecl *D2CXX = dyn_cast<CXXRecordDecl>(RD2)) {
    // If only RD2 is a C++ class, it should have zero base classes.
    if (D2CXX->getNumBases() > 0)
      return false;
  }

  // Check the fields.
  RecordDecl::field_iterator Field2 = RD2->field_begin(),
                             Field2End = RD2->field_end(),
                             Field1 = RD1->field_begin(),
                             Field1End = RD1->field_end();
  for ( ; Field1 != Field1End && Field2 != Field2End; ++Field1, ++Field2) {
    if (!isLayoutCompatible(C, *Field1, *Field2))
      return false;
  }
  if (Field1 != Field1End || Field2 != Field2End)
    return false;

  return true;
}

/// \brief Check if two standard-layout unions are layout-compatible.
/// (C++11 [class.mem] p18)
bool isLayoutCompatibleUnion(ASTContext &C,
                             RecordDecl *RD1,
                             RecordDecl *RD2) {
  llvm::SmallPtrSet<FieldDecl *, 8> UnmatchedFields;
  for (auto *Field2 : RD2->fields())
    UnmatchedFields.insert(Field2);

  for (auto *Field1 : RD1->fields()) {
    llvm::SmallPtrSet<FieldDecl *, 8>::iterator
        I = UnmatchedFields.begin(),
        E = UnmatchedFields.end();

    for ( ; I != E; ++I) {
      if (isLayoutCompatible(C, Field1, *I)) {
        bool Result = UnmatchedFields.erase(*I);
        (void) Result;
        assert(Result);
        break;
      }
    }
    if (I == E)
      return false;
  }

  return UnmatchedFields.empty();
}

bool isLayoutCompatible(ASTContext &C, RecordDecl *RD1, RecordDecl *RD2) {
  if (RD1->isUnion() != RD2->isUnion())
    return false;

  if (RD1->isUnion())
    return isLayoutCompatibleUnion(C, RD1, RD2);
  else
    return isLayoutCompatibleStruct(C, RD1, RD2);
}

/// \brief Check if two types are layout-compatible in C++11 sense.
bool isLayoutCompatible(ASTContext &C, QualType T1, QualType T2) {
  if (T1.isNull() || T2.isNull())
    return false;

  // C++11 [basic.types] p11:
  // If two types T1 and T2 are the same type, then T1 and T2 are
  // layout-compatible types.
  if (C.hasSameType(T1, T2))
    return true;

  T1 = T1.getCanonicalType().getUnqualifiedType();
  T2 = T2.getCanonicalType().getUnqualifiedType();

  const Type::TypeClass TC1 = T1->getTypeClass();
  const Type::TypeClass TC2 = T2->getTypeClass();

  if (TC1 != TC2)
    return false;

  if (TC1 == Type::Enum) {
    return isLayoutCompatible(C,
                              cast<EnumType>(T1)->getDecl(),
                              cast<EnumType>(T2)->getDecl());
  } else if (TC1 == Type::Record) {
    if (!T1->isStandardLayoutType() || !T2->isStandardLayoutType())
      return false;

    return isLayoutCompatible(C,
                              cast<RecordType>(T1)->getDecl(),
                              cast<RecordType>(T2)->getDecl());
  }

  return false;
}
}

//===--- CHECK: pointer_with_type_tag attribute: datatypes should match ----//

namespace {
/// \brief Given a type tag expression find the type tag itself.
///
/// \param TypeExpr Type tag expression, as it appears in user's code.
///
/// \param VD Declaration of an identifier that appears in a type tag.
///
/// \param MagicValue Type tag magic value.
bool FindTypeTagExpr(const Expr *TypeExpr, const ASTContext &Ctx,
                     const ValueDecl **VD, uint64_t *MagicValue) {
  while(true) {
    if (!TypeExpr)
      return false;

    TypeExpr = TypeExpr->IgnoreParenImpCasts()->IgnoreParenCasts();

    switch (TypeExpr->getStmtClass()) {
    case Stmt::UnaryOperatorClass: {
      const UnaryOperator *UO = cast<UnaryOperator>(TypeExpr);
      if (UO->getOpcode() == UO_AddrOf || UO->getOpcode() == UO_Deref) {
        TypeExpr = UO->getSubExpr();
        continue;
      }
      return false;
    }

    case Stmt::DeclRefExprClass: {
      const DeclRefExpr *DRE = cast<DeclRefExpr>(TypeExpr);
      *VD = DRE->getDecl();
      return true;
    }

    case Stmt::IntegerLiteralClass: {
      const IntegerLiteral *IL = cast<IntegerLiteral>(TypeExpr);
      llvm::APInt MagicValueAPInt = IL->getValue();
      if (MagicValueAPInt.getActiveBits() <= 64) {
        *MagicValue = MagicValueAPInt.getZExtValue();
        return true;
      } else
        return false;
    }

    case Stmt::BinaryConditionalOperatorClass:
    case Stmt::ConditionalOperatorClass: {
      const AbstractConditionalOperator *ACO =
          cast<AbstractConditionalOperator>(TypeExpr);
      bool Result;
      if (ACO->getCond()->EvaluateAsBooleanCondition(Result, Ctx)) {
        if (Result)
          TypeExpr = ACO->getTrueExpr();
        else
          TypeExpr = ACO->getFalseExpr();
        continue;
      }
      return false;
    }

    case Stmt::BinaryOperatorClass: {
      const BinaryOperator *BO = cast<BinaryOperator>(TypeExpr);
      if (BO->getOpcode() == BO_Comma) {
        TypeExpr = BO->getRHS();
        continue;
      }
      return false;
    }

    default:
      return false;
    }
  }
}

/// \brief Retrieve the C type corresponding to type tag TypeExpr.
///
/// \param TypeExpr Expression that specifies a type tag.
///
/// \param MagicValues Registered magic values.
///
/// \param FoundWrongKind Set to true if a type tag was found, but of a wrong
///        kind.
///
/// \param TypeInfo Information about the corresponding C type.
///
/// \returns true if the corresponding C type was found.
bool GetMatchingCType(
        const IdentifierInfo *ArgumentKind,
        const Expr *TypeExpr, const ASTContext &Ctx,
        const llvm::DenseMap<Sema::TypeTagMagicValue,
                             Sema::TypeTagData> *MagicValues,
        bool &FoundWrongKind,
        Sema::TypeTagData &TypeInfo) {
  FoundWrongKind = false;

  // Variable declaration that has type_tag_for_datatype attribute.
  const ValueDecl *VD = nullptr;

  uint64_t MagicValue;

  if (!FindTypeTagExpr(TypeExpr, Ctx, &VD, &MagicValue))
    return false;

  if (VD) {
    if (TypeTagForDatatypeAttr *I = VD->getAttr<TypeTagForDatatypeAttr>()) {
      if (I->getArgumentKind() != ArgumentKind) {
        FoundWrongKind = true;
        return false;
      }
      TypeInfo.Type = I->getMatchingCType();
      TypeInfo.LayoutCompatible = I->getLayoutCompatible();
      TypeInfo.MustBeNull = I->getMustBeNull();
      return true;
    }
    return false;
  }

  if (!MagicValues)
    return false;

  llvm::DenseMap<Sema::TypeTagMagicValue,
                 Sema::TypeTagData>::const_iterator I =
      MagicValues->find(std::make_pair(ArgumentKind, MagicValue));
  if (I == MagicValues->end())
    return false;

  TypeInfo = I->second;
  return true;
}
} // unnamed namespace

void Sema::RegisterTypeTagForDatatype(const IdentifierInfo *ArgumentKind,
                                      uint64_t MagicValue, QualType Type,
                                      bool LayoutCompatible,
                                      bool MustBeNull) {
  if (!TypeTagForDatatypeMagicValues)
    TypeTagForDatatypeMagicValues.reset(
        new llvm::DenseMap<TypeTagMagicValue, TypeTagData>);

  TypeTagMagicValue Magic(ArgumentKind, MagicValue);
  (*TypeTagForDatatypeMagicValues)[Magic] =
      TypeTagData(Type, LayoutCompatible, MustBeNull);
}

namespace {
bool IsSameCharType(QualType T1, QualType T2) {
  const BuiltinType *BT1 = T1->getAs<BuiltinType>();
  if (!BT1)
    return false;

  const BuiltinType *BT2 = T2->getAs<BuiltinType>();
  if (!BT2)
    return false;

  BuiltinType::Kind T1Kind = BT1->getKind();
  BuiltinType::Kind T2Kind = BT2->getKind();

  return (T1Kind == BuiltinType::SChar  && T2Kind == BuiltinType::Char_S) ||
         (T1Kind == BuiltinType::UChar  && T2Kind == BuiltinType::Char_U) ||
         (T1Kind == BuiltinType::Char_U && T2Kind == BuiltinType::UChar) ||
         (T1Kind == BuiltinType::Char_S && T2Kind == BuiltinType::SChar);
}
} // unnamed namespace

void Sema::CheckArgumentWithTypeTag(const ArgumentWithTypeTagAttr *Attr,
                                    const Expr * const *ExprArgs) {
  const IdentifierInfo *ArgumentKind = Attr->getArgumentKind();
  bool IsPointerAttr = Attr->getIsPointer();

  const Expr *TypeTagExpr = ExprArgs[Attr->getTypeTagIdx()];
  bool FoundWrongKind;
  TypeTagData TypeInfo;
  if (!GetMatchingCType(ArgumentKind, TypeTagExpr, Context,
                        TypeTagForDatatypeMagicValues.get(),
                        FoundWrongKind, TypeInfo)) {
    if (FoundWrongKind)
      Diag(TypeTagExpr->getExprLoc(),
           diag::warn_type_tag_for_datatype_wrong_kind)
        << TypeTagExpr->getSourceRange();
    return;
  }

  const Expr *ArgumentExpr = ExprArgs[Attr->getArgumentIdx()];
  if (IsPointerAttr) {
    // Skip implicit cast of pointer to `void *' (as a function argument).
    if (const ImplicitCastExpr *ICE = dyn_cast<ImplicitCastExpr>(ArgumentExpr))
      if (ICE->getType()->isVoidPointerType() &&
          ICE->getCastKind() == CK_BitCast)
        ArgumentExpr = ICE->getSubExpr();
  }
  QualType ArgumentType = ArgumentExpr->getType();

  // Passing a `void*' pointer shouldn't trigger a warning.
  if (IsPointerAttr && ArgumentType->isVoidPointerType())
    return;

  if (TypeInfo.MustBeNull) {
    // Type tag with matching void type requires a null pointer.
    if (!ArgumentExpr->isNullPointerConstant(Context,
                                             Expr::NPC_ValueDependentIsNotNull)) {
      Diag(ArgumentExpr->getExprLoc(),
           diag::warn_type_safety_null_pointer_required)
          << ArgumentKind->getName()
          << ArgumentExpr->getSourceRange()
          << TypeTagExpr->getSourceRange();
    }
    return;
  }

  QualType RequiredType = TypeInfo.Type;
  if (IsPointerAttr)
    RequiredType = Context.getPointerType(RequiredType);

  bool mismatch = false;
  if (!TypeInfo.LayoutCompatible) {
    mismatch = !Context.hasSameType(ArgumentType, RequiredType);

    // C++11 [basic.fundamental] p1:
    // Plain char, signed char, and unsigned char are three distinct types.
    //
    // But we treat plain `char' as equivalent to `signed char' or `unsigned
    // char' depending on the current char signedness mode.
    if (mismatch)
      if ((IsPointerAttr && IsSameCharType(ArgumentType->getPointeeType(),
                                           RequiredType->getPointeeType())) ||
          (!IsPointerAttr && IsSameCharType(ArgumentType, RequiredType)))
        mismatch = false;
  } else
    if (IsPointerAttr)
      mismatch = !isLayoutCompatible(Context,
                                     ArgumentType->getPointeeType(),
                                     RequiredType->getPointeeType());
    else
      mismatch = !isLayoutCompatible(Context, ArgumentType, RequiredType);

  if (mismatch)
    Diag(ArgumentExpr->getExprLoc(), diag::warn_type_safety_type_mismatch)
        << ArgumentType << ArgumentKind
        << TypeInfo.LayoutCompatible << RequiredType
        << ArgumentExpr->getSourceRange()
        << TypeTagExpr->getSourceRange();
}

