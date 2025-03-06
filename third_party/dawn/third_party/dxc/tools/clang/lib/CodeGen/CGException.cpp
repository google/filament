//===--- CGException.cpp - Emit LLVM Code for C++ exceptions --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This contains code dealing with C++ exception related code generation.
//
//===----------------------------------------------------------------------===//

#include "CodeGenFunction.h"
#include "CGCXXABI.h"
#include "CGCleanup.h"
#include "CGObjCRuntime.h"
#include "TargetInfo.h"
#include "clang/AST/Mangle.h"
#include "clang/AST/StmtCXX.h"
#include "clang/AST/StmtObjC.h"
#include "clang/AST/StmtVisitor.h"
#include "clang/Basic/TargetBuiltins.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/SaveAndRestore.h"

using namespace clang;
using namespace CodeGen;

// HLSL Change Starts
// No exeption codegen support, so simply skip all of this compilation.
// Here are enough stubs to link the current targets.
#if 1
// No GCC compat requirements, no notion of personality for HLSL.
void CodeGenModule::SimplifyPersonality() { }
// No exception support.
void CodeGenFunction::EmitStartEHSpec(const Decl *) { }
void CodeGenFunction::EmitEndEHSpec(const Decl *) { }
void CodeGenFunction::EmitCXXThrowExpr(const CXXThrowExpr *, bool) {
  llvm_unreachable("HLSL does not generate throw expressions");
}
llvm::Constant *CodeGenModule::getTerminateFn() {
  llvm_unreachable("HLSL does not terminate");
  return nullptr;
}
llvm::Value *CodeGenFunction::getExceptionFromSlot() {
  llvm_unreachable("HLSL does not support exceptions");
  return nullptr;
}
llvm::Value *CodeGenFunction::getSelectorFromSlot() {
  llvm_unreachable("HLSL does not support exceptions");
  return nullptr;
}
#else
// HLSL Change Ends

static llvm::Constant *getFreeExceptionFn(CodeGenModule &CGM) {
  // void __cxa_free_exception(void *thrown_exception);

  llvm::FunctionType *FTy =
    llvm::FunctionType::get(CGM.VoidTy, CGM.Int8PtrTy, /*IsVarArgs=*/false);

  return CGM.CreateRuntimeFunction(FTy, "__cxa_free_exception");
}

static llvm::Constant *getUnexpectedFn(CodeGenModule &CGM) {
  // void __cxa_call_unexpected(void *thrown_exception);

  llvm::FunctionType *FTy =
    llvm::FunctionType::get(CGM.VoidTy, CGM.Int8PtrTy, /*IsVarArgs=*/false);

  return CGM.CreateRuntimeFunction(FTy, "__cxa_call_unexpected");
}

llvm::Constant *CodeGenModule::getTerminateFn() {
  // void __terminate();

  llvm::FunctionType *FTy =
    llvm::FunctionType::get(VoidTy, /*IsVarArgs=*/false);

  StringRef name;

  // In C++, use std::terminate().
  if (getLangOpts().CPlusPlus &&
      getTarget().getCXXABI().isItaniumFamily()) {
    name = "_ZSt9terminatev";
  } else if (getLangOpts().CPlusPlus &&
             getTarget().getCXXABI().isMicrosoft()) {
    if (getLangOpts().isCompatibleWithMSVC(LangOptions::MSVC2015))
      name = "__std_terminate";
    else
      name = "\01?terminate@@YAXXZ";
  } else if (getLangOpts().ObjC1 &&
             getLangOpts().ObjCRuntime.hasTerminate())
    name = "objc_terminate";
  else
    name = "abort";
  return CreateRuntimeFunction(FTy, name);
}

static llvm::Constant *getCatchallRethrowFn(CodeGenModule &CGM,
                                            StringRef Name) {
  llvm::FunctionType *FTy =
    llvm::FunctionType::get(CGM.VoidTy, CGM.Int8PtrTy, /*IsVarArgs=*/false);

  return CGM.CreateRuntimeFunction(FTy, Name);
}

namespace {
  /// The exceptions personality for a function.
  struct EHPersonality {
    const char *PersonalityFn;

    // If this is non-null, this personality requires a non-standard
    // function for rethrowing an exception after a catchall cleanup.
    // This function must have prototype void(void*).
    const char *CatchallRethrowFn;

    static const EHPersonality &get(CodeGenModule &CGM,
                                    const FunctionDecl *FD);
    static const EHPersonality &get(CodeGenFunction &CGF) {
      return get(CGF.CGM, dyn_cast_or_null<FunctionDecl>(CGF.CurCodeDecl));
    }

    static const EHPersonality GNU_C;
    static const EHPersonality GNU_C_SJLJ;
    static const EHPersonality GNU_C_SEH;
    static const EHPersonality GNU_ObjC;
    static const EHPersonality GNUstep_ObjC;
    static const EHPersonality GNU_ObjCXX;
    static const EHPersonality NeXT_ObjC;
    static const EHPersonality GNU_CPlusPlus;
    static const EHPersonality GNU_CPlusPlus_SJLJ;
    static const EHPersonality GNU_CPlusPlus_SEH;
    static const EHPersonality MSVC_except_handler;
    static const EHPersonality MSVC_C_specific_handler;
    static const EHPersonality MSVC_CxxFrameHandler3;
  };
}

const EHPersonality EHPersonality::GNU_C = { "__gcc_personality_v0", nullptr };
const EHPersonality
EHPersonality::GNU_C_SJLJ = { "__gcc_personality_sj0", nullptr };
const EHPersonality
EHPersonality::GNU_C_SEH = { "__gcc_personality_seh0", nullptr };
const EHPersonality
EHPersonality::NeXT_ObjC = { "__objc_personality_v0", nullptr };
const EHPersonality
EHPersonality::GNU_CPlusPlus = { "__gxx_personality_v0", nullptr };
const EHPersonality
EHPersonality::GNU_CPlusPlus_SJLJ = { "__gxx_personality_sj0", nullptr };
const EHPersonality
EHPersonality::GNU_CPlusPlus_SEH = { "__gxx_personality_seh0", nullptr };
const EHPersonality
EHPersonality::GNU_ObjC = {"__gnu_objc_personality_v0", "objc_exception_throw"};
const EHPersonality
EHPersonality::GNU_ObjCXX = { "__gnustep_objcxx_personality_v0", nullptr };
const EHPersonality
EHPersonality::GNUstep_ObjC = { "__gnustep_objc_personality_v0", nullptr };
const EHPersonality
EHPersonality::MSVC_except_handler = { "_except_handler3", nullptr };
const EHPersonality
EHPersonality::MSVC_C_specific_handler = { "__C_specific_handler", nullptr };
const EHPersonality
EHPersonality::MSVC_CxxFrameHandler3 = { "__CxxFrameHandler3", nullptr };

/// On Win64, use libgcc's SEH personality function. We fall back to dwarf on
/// other platforms, unless the user asked for SjLj exceptions.
static bool useLibGCCSEHPersonality(const llvm::Triple &T) {
  return T.isOSWindows() && T.getArch() == llvm::Triple::x86_64;
}

static const EHPersonality &getCPersonality(const llvm::Triple &T,
                                            const LangOptions &L) {
  if (L.SjLjExceptions)
    return EHPersonality::GNU_C_SJLJ;
  else if (useLibGCCSEHPersonality(T))
    return EHPersonality::GNU_C_SEH;
  return EHPersonality::GNU_C;
}

static const EHPersonality &getObjCPersonality(const llvm::Triple &T,
                                               const LangOptions &L) {
  switch (L.ObjCRuntime.getKind()) {
  case ObjCRuntime::FragileMacOSX:
    return getCPersonality(T, L);
  case ObjCRuntime::MacOSX:
  case ObjCRuntime::iOS:
    return EHPersonality::NeXT_ObjC;
  case ObjCRuntime::GNUstep:
    if (L.ObjCRuntime.getVersion() >= VersionTuple(1, 7))
      return EHPersonality::GNUstep_ObjC;
    // fallthrough
  case ObjCRuntime::GCC:
  case ObjCRuntime::ObjFW:
    return EHPersonality::GNU_ObjC;
  }
  llvm_unreachable("bad runtime kind");
}

static const EHPersonality &getCXXPersonality(const llvm::Triple &T,
                                              const LangOptions &L) {
  if (L.SjLjExceptions)
    return EHPersonality::GNU_CPlusPlus_SJLJ;
  else if (useLibGCCSEHPersonality(T))
    return EHPersonality::GNU_CPlusPlus_SEH;
  return EHPersonality::GNU_CPlusPlus;
}

/// Determines the personality function to use when both C++
/// and Objective-C exceptions are being caught.
static const EHPersonality &getObjCXXPersonality(const llvm::Triple &T,
                                                 const LangOptions &L) {
  switch (L.ObjCRuntime.getKind()) {
  // The ObjC personality defers to the C++ personality for non-ObjC
  // handlers.  Unlike the C++ case, we use the same personality
  // function on targets using (backend-driven) SJLJ EH.
  case ObjCRuntime::MacOSX:
  case ObjCRuntime::iOS:
    return EHPersonality::NeXT_ObjC;

  // In the fragile ABI, just use C++ exception handling and hope
  // they're not doing crazy exception mixing.
  case ObjCRuntime::FragileMacOSX:
    return getCXXPersonality(T, L);

  // The GCC runtime's personality function inherently doesn't support
  // mixed EH.  Use the C++ personality just to avoid returning null.
  case ObjCRuntime::GCC:
  case ObjCRuntime::ObjFW: // XXX: this will change soon
    return EHPersonality::GNU_ObjC;
  case ObjCRuntime::GNUstep:
    return EHPersonality::GNU_ObjCXX;
  }
  llvm_unreachable("bad runtime kind");
}

static const EHPersonality &getSEHPersonalityMSVC(const llvm::Triple &T) {
  if (T.getArch() == llvm::Triple::x86)
    return EHPersonality::MSVC_except_handler;
  return EHPersonality::MSVC_C_specific_handler;
}

const EHPersonality &EHPersonality::get(CodeGenModule &CGM,
                                        const FunctionDecl *FD) {
  const llvm::Triple &T = CGM.getTarget().getTriple();
  const LangOptions &L = CGM.getLangOpts();

  // Try to pick a personality function that is compatible with MSVC if we're
  // not compiling Obj-C. Obj-C users better have an Obj-C runtime that supports
  // the GCC-style personality function.
  if (T.isWindowsMSVCEnvironment() && !L.ObjC1) {
    if (L.SjLjExceptions)
      return EHPersonality::GNU_CPlusPlus_SJLJ;
    else if (FD && FD->usesSEHTry())
      return getSEHPersonalityMSVC(T);
    else
      return EHPersonality::MSVC_CxxFrameHandler3;
  }

  if (L.CPlusPlus && L.ObjC1)
    return getObjCXXPersonality(T, L);
  else if (L.CPlusPlus)
    return getCXXPersonality(T, L);
  else if (L.ObjC1)
    return getObjCPersonality(T, L);
  else
    return getCPersonality(T, L);
}

static llvm::Constant *getPersonalityFn(CodeGenModule &CGM,
                                        const EHPersonality &Personality) {
  llvm::Constant *Fn =
    CGM.CreateRuntimeFunction(llvm::FunctionType::get(CGM.Int32Ty, true),
                              Personality.PersonalityFn);
  return Fn;
}

static llvm::Constant *getOpaquePersonalityFn(CodeGenModule &CGM,
                                        const EHPersonality &Personality) {
  llvm::Constant *Fn = getPersonalityFn(CGM, Personality);
  return llvm::ConstantExpr::getBitCast(Fn, CGM.Int8PtrTy);
}

/// Check whether a personality function could reasonably be swapped
/// for a C++ personality function.
static bool PersonalityHasOnlyCXXUses(llvm::Constant *Fn) {
  for (llvm::User *U : Fn->users()) {
    // Conditionally white-list bitcasts.
    if (llvm::ConstantExpr *CE = dyn_cast<llvm::ConstantExpr>(U)) {
      if (CE->getOpcode() != llvm::Instruction::BitCast) return false;
      if (!PersonalityHasOnlyCXXUses(CE))
        return false;
      continue;
    }

    // Otherwise, it has to be a landingpad instruction.
    llvm::LandingPadInst *LPI = dyn_cast<llvm::LandingPadInst>(U);
    if (!LPI) return false;

    for (unsigned I = 0, E = LPI->getNumClauses(); I != E; ++I) {
      // Look for something that would've been returned by the ObjC
      // runtime's GetEHType() method.
      llvm::Value *Val = LPI->getClause(I)->stripPointerCasts();
      if (LPI->isCatch(I)) {
        // Check if the catch value has the ObjC prefix.
        if (llvm::GlobalVariable *GV = dyn_cast<llvm::GlobalVariable>(Val))
          // ObjC EH selector entries are always global variables with
          // names starting like this.
          if (GV->getName().startswith("OBJC_EHTYPE"))
            return false;
      } else {
        // Check if any of the filter values have the ObjC prefix.
        llvm::Constant *CVal = cast<llvm::Constant>(Val);
        for (llvm::User::op_iterator
               II = CVal->op_begin(), IE = CVal->op_end(); II != IE; ++II) {
          if (llvm::GlobalVariable *GV =
              cast<llvm::GlobalVariable>((*II)->stripPointerCasts()))
            // ObjC EH selector entries are always global variables with
            // names starting like this.
            if (GV->getName().startswith("OBJC_EHTYPE"))
              return false;
        }
      }
    }
  }

  return true;
}

/// Try to use the C++ personality function in ObjC++.  Not doing this
/// can cause some incompatibilities with gcc, which is more
/// aggressive about only using the ObjC++ personality in a function
/// when it really needs it.
void CodeGenModule::SimplifyPersonality() {
  // If we're not in ObjC++ -fexceptions, there's nothing to do.
  if (!LangOpts.CPlusPlus || !LangOpts.ObjC1 || !LangOpts.Exceptions)
    return;

  // Both the problem this endeavors to fix and the way the logic
  // above works is specific to the NeXT runtime.
  if (!LangOpts.ObjCRuntime.isNeXTFamily())
    return;

  const EHPersonality &ObjCXX = EHPersonality::get(*this, /*FD=*/nullptr);
  const EHPersonality &CXX =
      getCXXPersonality(getTarget().getTriple(), LangOpts);
  if (&ObjCXX == &CXX)
    return;

  assert(std::strcmp(ObjCXX.PersonalityFn, CXX.PersonalityFn) != 0 &&
         "Different EHPersonalities using the same personality function.");

  llvm::Function *Fn = getModule().getFunction(ObjCXX.PersonalityFn);

  // Nothing to do if it's unused.
  if (!Fn || Fn->use_empty()) return;
  
  // Can't do the optimization if it has non-C++ uses.
  if (!PersonalityHasOnlyCXXUses(Fn)) return;

  // Create the C++ personality function and kill off the old
  // function.
  llvm::Constant *CXXFn = getPersonalityFn(*this, CXX);

  // This can happen if the user is screwing with us.
  if (Fn->getType() != CXXFn->getType()) return;

  Fn->replaceAllUsesWith(CXXFn);
  Fn->eraseFromParent();
}

/// Returns the value to inject into a selector to indicate the
/// presence of a catch-all.
static llvm::Constant *getCatchAllValue(CodeGenFunction &CGF) {
  // Possibly we should use @llvm.eh.catch.all.value here.
  return llvm::ConstantPointerNull::get(CGF.Int8PtrTy);
}

namespace {
  /// A cleanup to free the exception object if its initialization
  /// throws.
  struct FreeException : EHScopeStack::Cleanup {
    llvm::Value *exn;
    FreeException(llvm::Value *exn) : exn(exn) {}
    void Emit(CodeGenFunction &CGF, Flags flags) override {
      CGF.EmitNounwindRuntimeCall(getFreeExceptionFn(CGF.CGM), exn);
    }
  };
}

// Emits an exception expression into the given location.  This
// differs from EmitAnyExprToMem only in that, if a final copy-ctor
// call is required, an exception within that copy ctor causes
// std::terminate to be invoked.
void CodeGenFunction::EmitAnyExprToExn(const Expr *e, llvm::Value *addr) {
  // Make sure the exception object is cleaned up if there's an
  // exception during initialization.
  pushFullExprCleanup<FreeException>(EHCleanup, addr);
  EHScopeStack::stable_iterator cleanup = EHStack.stable_begin();

  // __cxa_allocate_exception returns a void*;  we need to cast this
  // to the appropriate type for the object.
  llvm::Type *ty = ConvertTypeForMem(e->getType())->getPointerTo();
  llvm::Value *typedAddr = Builder.CreateBitCast(addr, ty);

  // FIXME: this isn't quite right!  If there's a final unelided call
  // to a copy constructor, then according to [except.terminate]p1 we
  // must call std::terminate() if that constructor throws, because
  // technically that copy occurs after the exception expression is
  // evaluated but before the exception is caught.  But the best way
  // to handle that is to teach EmitAggExpr to do the final copy
  // differently if it can't be elided.
  EmitAnyExprToMem(e, typedAddr, e->getType().getQualifiers(),
                   /*IsInit*/ true);

  // Deactivate the cleanup block.
  DeactivateCleanupBlock(cleanup, cast<llvm::Instruction>(typedAddr));
}

llvm::Value *CodeGenFunction::getExceptionSlot() {
  if (!ExceptionSlot)
    ExceptionSlot = CreateTempAlloca(Int8PtrTy, "exn.slot");
  return ExceptionSlot;
}

llvm::Value *CodeGenFunction::getEHSelectorSlot() {
  if (!EHSelectorSlot)
    EHSelectorSlot = CreateTempAlloca(Int32Ty, "ehselector.slot");
  return EHSelectorSlot;
}

llvm::Value *CodeGenFunction::getExceptionFromSlot() {
  return Builder.CreateLoad(getExceptionSlot(), "exn");
}

llvm::Value *CodeGenFunction::getSelectorFromSlot() {
  return Builder.CreateLoad(getEHSelectorSlot(), "sel");
}

void CodeGenFunction::EmitCXXThrowExpr(const CXXThrowExpr *E,
                                       bool KeepInsertionPoint) {
  if (const Expr *SubExpr = E->getSubExpr()) {
    QualType ThrowType = SubExpr->getType();
    if (ThrowType->isObjCObjectPointerType()) {
      const Stmt *ThrowStmt = E->getSubExpr();
      const ObjCAtThrowStmt S(E->getExprLoc(), const_cast<Stmt *>(ThrowStmt));
      CGM.getObjCRuntime().EmitThrowStmt(*this, S, false);
    } else {
      CGM.getCXXABI().emitThrow(*this, E);
    }
  } else {
    CGM.getCXXABI().emitRethrow(*this, /*isNoReturn=*/true);
  }

  // throw is an expression, and the expression emitters expect us
  // to leave ourselves at a valid insertion point.
  if (KeepInsertionPoint)
    EmitBlock(createBasicBlock("throw.cont"));
}

void CodeGenFunction::EmitStartEHSpec(const Decl *D) {
  if (!CGM.getLangOpts().CXXExceptions)
    return;
  
  const FunctionDecl* FD = dyn_cast_or_null<FunctionDecl>(D);
  if (!FD) {
    // Check if CapturedDecl is nothrow and create terminate scope for it.
    if (const CapturedDecl* CD = dyn_cast_or_null<CapturedDecl>(D)) {
      if (CD->isNothrow())
        EHStack.pushTerminate();
    }
    return;
  }
  const FunctionProtoType *Proto = FD->getType()->getAs<FunctionProtoType>();
  if (!Proto)
    return;

  ExceptionSpecificationType EST = Proto->getExceptionSpecType();
  if (isNoexceptExceptionSpec(EST)) {
    if (Proto->getNoexceptSpec(getContext()) == FunctionProtoType::NR_Nothrow) {
      // noexcept functions are simple terminate scopes.
      EHStack.pushTerminate();
    }
  } else if (EST == EST_Dynamic || EST == EST_DynamicNone) {
    // TODO: Revisit exception specifications for the MS ABI.  There is a way to
    // encode these in an object file but MSVC doesn't do anything with it.
    if (getTarget().getCXXABI().isMicrosoft())
      return;
    unsigned NumExceptions = Proto->getNumExceptions();
    EHFilterScope *Filter = EHStack.pushFilter(NumExceptions);

    for (unsigned I = 0; I != NumExceptions; ++I) {
      QualType Ty = Proto->getExceptionType(I);
      QualType ExceptType = Ty.getNonReferenceType().getUnqualifiedType();
      llvm::Value *EHType = CGM.GetAddrOfRTTIDescriptor(ExceptType,
                                                        /*ForEH=*/true);
      Filter->setFilter(I, EHType);
    }
  }
}

/// Emit the dispatch block for a filter scope if necessary.
static void emitFilterDispatchBlock(CodeGenFunction &CGF,
                                    EHFilterScope &filterScope) {
  llvm::BasicBlock *dispatchBlock = filterScope.getCachedEHDispatchBlock();
  if (!dispatchBlock) return;
  if (dispatchBlock->use_empty()) {
    delete dispatchBlock;
    return;
  }

  CGF.EmitBlockAfterUses(dispatchBlock);

  // If this isn't a catch-all filter, we need to check whether we got
  // here because the filter triggered.
  if (filterScope.getNumFilters()) {
    // Load the selector value.
    llvm::Value *selector = CGF.getSelectorFromSlot();
    llvm::BasicBlock *unexpectedBB = CGF.createBasicBlock("ehspec.unexpected");

    llvm::Value *zero = CGF.Builder.getInt32(0);
    llvm::Value *failsFilter =
        CGF.Builder.CreateICmpSLT(selector, zero, "ehspec.fails");
    CGF.Builder.CreateCondBr(failsFilter, unexpectedBB,
                             CGF.getEHResumeBlock(false));

    CGF.EmitBlock(unexpectedBB);
  }

  // Call __cxa_call_unexpected.  This doesn't need to be an invoke
  // because __cxa_call_unexpected magically filters exceptions
  // according to the last landing pad the exception was thrown
  // into.  Seriously.
  llvm::Value *exn = CGF.getExceptionFromSlot();
  CGF.EmitRuntimeCall(getUnexpectedFn(CGF.CGM), exn)
    ->setDoesNotReturn();
  CGF.Builder.CreateUnreachable();
}

void CodeGenFunction::EmitEndEHSpec(const Decl *D) {
  if (!CGM.getLangOpts().CXXExceptions)
    return;
  
  const FunctionDecl* FD = dyn_cast_or_null<FunctionDecl>(D);
  if (!FD) {
    // Check if CapturedDecl is nothrow and pop terminate scope for it.
    if (const CapturedDecl* CD = dyn_cast_or_null<CapturedDecl>(D)) {
      if (CD->isNothrow())
        EHStack.popTerminate();
    }
    return;
  }
  const FunctionProtoType *Proto = FD->getType()->getAs<FunctionProtoType>();
  if (!Proto)
    return;

  ExceptionSpecificationType EST = Proto->getExceptionSpecType();
  if (isNoexceptExceptionSpec(EST)) {
    if (Proto->getNoexceptSpec(getContext()) == FunctionProtoType::NR_Nothrow) {
      EHStack.popTerminate();
    }
  } else if (EST == EST_Dynamic || EST == EST_DynamicNone) {
    // TODO: Revisit exception specifications for the MS ABI.  There is a way to
    // encode these in an object file but MSVC doesn't do anything with it.
    if (getTarget().getCXXABI().isMicrosoft())
      return;
    EHFilterScope &filterScope = cast<EHFilterScope>(*EHStack.begin());
    emitFilterDispatchBlock(*this, filterScope);
    EHStack.popFilter();
  }
}

void CodeGenFunction::EmitCXXTryStmt(const CXXTryStmt &S) {
  EnterCXXTryStmt(S);
  EmitStmt(S.getTryBlock());
  ExitCXXTryStmt(S);
}

void CodeGenFunction::EnterCXXTryStmt(const CXXTryStmt &S, bool IsFnTryBlock) {
  unsigned NumHandlers = S.getNumHandlers();
  EHCatchScope *CatchScope = EHStack.pushCatch(NumHandlers);

  for (unsigned I = 0; I != NumHandlers; ++I) {
    const CXXCatchStmt *C = S.getHandler(I);

    llvm::BasicBlock *Handler = createBasicBlock("catch");
    if (C->getExceptionDecl()) {
      // FIXME: Dropping the reference type on the type into makes it
      // impossible to correctly implement catch-by-reference
      // semantics for pointers.  Unfortunately, this is what all
      // existing compilers do, and it's not clear that the standard
      // personality routine is capable of doing this right.  See C++ DR 388:
      //   http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_active.html#388
      Qualifiers CaughtTypeQuals;
      QualType CaughtType = CGM.getContext().getUnqualifiedArrayType(
          C->getCaughtType().getNonReferenceType(), CaughtTypeQuals);

      llvm::Constant *TypeInfo = nullptr;
      if (CaughtType->isObjCObjectPointerType())
        TypeInfo = CGM.getObjCRuntime().GetEHType(CaughtType);
      else
        TypeInfo =
            CGM.getAddrOfCXXCatchHandlerType(CaughtType, C->getCaughtType());
      CatchScope->setHandler(I, TypeInfo, Handler);
    } else {
      // No exception decl indicates '...', a catch-all.
      CatchScope->setCatchAllHandler(I, Handler);
    }
  }
}

llvm::BasicBlock *
CodeGenFunction::getEHDispatchBlock(EHScopeStack::stable_iterator si) {
  // The dispatch block for the end of the scope chain is a block that
  // just resumes unwinding.
  if (si == EHStack.stable_end())
    return getEHResumeBlock(true);

  // Otherwise, we should look at the actual scope.
  EHScope &scope = *EHStack.find(si);

  llvm::BasicBlock *dispatchBlock = scope.getCachedEHDispatchBlock();
  if (!dispatchBlock) {
    switch (scope.getKind()) {
    case EHScope::Catch: {
      // Apply a special case to a single catch-all.
      EHCatchScope &catchScope = cast<EHCatchScope>(scope);
      if (catchScope.getNumHandlers() == 1 &&
          catchScope.getHandler(0).isCatchAll()) {
        dispatchBlock = catchScope.getHandler(0).Block;

      // Otherwise, make a dispatch block.
      } else {
        dispatchBlock = createBasicBlock("catch.dispatch");
      }
      break;
    }

    case EHScope::Cleanup:
      dispatchBlock = createBasicBlock("ehcleanup");
      break;

    case EHScope::Filter:
      dispatchBlock = createBasicBlock("filter.dispatch");
      break;

    case EHScope::Terminate:
      dispatchBlock = getTerminateHandler();
      break;
    }
    scope.setCachedEHDispatchBlock(dispatchBlock);
  }
  return dispatchBlock;
}

/// Check whether this is a non-EH scope, i.e. a scope which doesn't
/// affect exception handling.  Currently, the only non-EH scopes are
/// normal-only cleanup scopes.
static bool isNonEHScope(const EHScope &S) {
  switch (S.getKind()) {
  case EHScope::Cleanup:
    return !cast<EHCleanupScope>(S).isEHCleanup();
  case EHScope::Filter:
  case EHScope::Catch:
  case EHScope::Terminate:
    return false;
  }

  llvm_unreachable("Invalid EHScope Kind!");
}

llvm::BasicBlock *CodeGenFunction::getInvokeDestImpl() {
  assert(EHStack.requiresLandingPad());
  assert(!EHStack.empty());

  // If exceptions are disabled, there are usually no landingpads. However, when
  // SEH is enabled, functions using SEH still get landingpads.
  const LangOptions &LO = CGM.getLangOpts();
  if (!LO.Exceptions) {
    if (!LO.Borland && !LO.MicrosoftExt)
      return nullptr;
    if (!currentFunctionUsesSEHTry())
      return nullptr;
  }

  // Check the innermost scope for a cached landing pad.  If this is
  // a non-EH cleanup, we'll check enclosing scopes in EmitLandingPad.
  llvm::BasicBlock *LP = EHStack.begin()->getCachedLandingPad();
  if (LP) return LP;

  // Build the landing pad for this scope.
  LP = EmitLandingPad();
  assert(LP);

  // Cache the landing pad on the innermost scope.  If this is a
  // non-EH scope, cache the landing pad on the enclosing scope, too.
  for (EHScopeStack::iterator ir = EHStack.begin(); true; ++ir) {
    ir->setCachedLandingPad(LP);
    if (!isNonEHScope(*ir)) break;
  }

  return LP;
}

llvm::BasicBlock *CodeGenFunction::EmitLandingPad() {
  assert(EHStack.requiresLandingPad());

  EHScope &innermostEHScope = *EHStack.find(EHStack.getInnermostEHScope());
  switch (innermostEHScope.getKind()) {
  case EHScope::Terminate:
    return getTerminateLandingPad();

  case EHScope::Catch:
  case EHScope::Cleanup:
  case EHScope::Filter:
    if (llvm::BasicBlock *lpad = innermostEHScope.getCachedLandingPad())
      return lpad;
  }

  // Save the current IR generation state.
  CGBuilderTy::InsertPoint savedIP = Builder.saveAndClearIP();
  auto DL = ApplyDebugLocation::CreateDefaultArtificial(*this, CurEHLocation);

  const EHPersonality &personality = EHPersonality::get(*this);

  if (!CurFn->hasPersonalityFn())
    CurFn->setPersonalityFn(getOpaquePersonalityFn(CGM, personality));

  // Create and configure the landing pad.
  llvm::BasicBlock *lpad = createBasicBlock("lpad");
  EmitBlock(lpad);

  llvm::LandingPadInst *LPadInst = Builder.CreateLandingPad(
      llvm::StructType::get(Int8PtrTy, Int32Ty, nullptr), 0);

  llvm::Value *LPadExn = Builder.CreateExtractValue(LPadInst, 0);
  Builder.CreateStore(LPadExn, getExceptionSlot());
  llvm::Value *LPadSel = Builder.CreateExtractValue(LPadInst, 1);
  Builder.CreateStore(LPadSel, getEHSelectorSlot());

  // Save the exception pointer.  It's safe to use a single exception
  // pointer per function because EH cleanups can never have nested
  // try/catches.
  // Build the landingpad instruction.

  // Accumulate all the handlers in scope.
  bool hasCatchAll = false;
  bool hasCleanup = false;
  bool hasFilter = false;
  SmallVector<llvm::Value*, 4> filterTypes;
  llvm::SmallPtrSet<llvm::Value*, 4> catchTypes;
  for (EHScopeStack::iterator I = EHStack.begin(), E = EHStack.end(); I != E;
       ++I) {

    switch (I->getKind()) {
    case EHScope::Cleanup:
      // If we have a cleanup, remember that.
      hasCleanup = (hasCleanup || cast<EHCleanupScope>(*I).isEHCleanup());
      continue;

    case EHScope::Filter: {
      assert(I.next() == EHStack.end() && "EH filter is not end of EH stack");
      assert(!hasCatchAll && "EH filter reached after catch-all");

      // Filter scopes get added to the landingpad in weird ways.
      EHFilterScope &filter = cast<EHFilterScope>(*I);
      hasFilter = true;

      // Add all the filter values.
      for (unsigned i = 0, e = filter.getNumFilters(); i != e; ++i)
        filterTypes.push_back(filter.getFilter(i));
      goto done;
    }

    case EHScope::Terminate:
      // Terminate scopes are basically catch-alls.
      assert(!hasCatchAll);
      hasCatchAll = true;
      goto done;

    case EHScope::Catch:
      break;
    }

    EHCatchScope &catchScope = cast<EHCatchScope>(*I);
    for (unsigned hi = 0, he = catchScope.getNumHandlers(); hi != he; ++hi) {
      EHCatchScope::Handler handler = catchScope.getHandler(hi);

      // If this is a catch-all, register that and abort.
      if (!handler.Type) {
        assert(!hasCatchAll);
        hasCatchAll = true;
        goto done;
      }

      // Check whether we already have a handler for this type.
      if (catchTypes.insert(handler.Type).second)
        // If not, add it directly to the landingpad.
        LPadInst->addClause(handler.Type);
    }
  }

 done:
  // If we have a catch-all, add null to the landingpad.
  assert(!(hasCatchAll && hasFilter));
  if (hasCatchAll) {
    LPadInst->addClause(getCatchAllValue(*this));

  // If we have an EH filter, we need to add those handlers in the
  // right place in the landingpad, which is to say, at the end.
  } else if (hasFilter) {
    // Create a filter expression: a constant array indicating which filter
    // types there are. The personality routine only lands here if the filter
    // doesn't match.
    SmallVector<llvm::Constant*, 8> Filters;
    llvm::ArrayType *AType =
      llvm::ArrayType::get(!filterTypes.empty() ?
                             filterTypes[0]->getType() : Int8PtrTy,
                           filterTypes.size());

    for (unsigned i = 0, e = filterTypes.size(); i != e; ++i)
      Filters.push_back(cast<llvm::Constant>(filterTypes[i]));
    llvm::Constant *FilterArray = llvm::ConstantArray::get(AType, Filters);
    LPadInst->addClause(FilterArray);

    // Also check whether we need a cleanup.
    if (hasCleanup)
      LPadInst->setCleanup(true);

  // Otherwise, signal that we at least have cleanups.
  } else if (hasCleanup) {
    LPadInst->setCleanup(true);
  }

  assert((LPadInst->getNumClauses() > 0 || LPadInst->isCleanup()) &&
         "landingpad instruction has no clauses!");

  // Tell the backend how to generate the landing pad.
  Builder.CreateBr(getEHDispatchBlock(EHStack.getInnermostEHScope()));

  // Restore the old IR generation state.
  Builder.restoreIP(savedIP);

  return lpad;
}

/// Emit the structure of the dispatch block for the given catch scope.
/// It is an invariant that the dispatch block already exists.
static void emitCatchDispatchBlock(CodeGenFunction &CGF,
                                   EHCatchScope &catchScope) {
  llvm::BasicBlock *dispatchBlock = catchScope.getCachedEHDispatchBlock();
  assert(dispatchBlock);

  // If there's only a single catch-all, getEHDispatchBlock returned
  // that catch-all as the dispatch block.
  if (catchScope.getNumHandlers() == 1 &&
      catchScope.getHandler(0).isCatchAll()) {
    assert(dispatchBlock == catchScope.getHandler(0).Block);
    return;
  }

  CGBuilderTy::InsertPoint savedIP = CGF.Builder.saveIP();
  CGF.EmitBlockAfterUses(dispatchBlock);

  // Select the right handler.
  llvm::Value *llvm_eh_typeid_for =
    CGF.CGM.getIntrinsic(llvm::Intrinsic::eh_typeid_for);

  // Load the selector value.
  llvm::Value *selector = CGF.getSelectorFromSlot();

  // Test against each of the exception types we claim to catch.
  for (unsigned i = 0, e = catchScope.getNumHandlers(); ; ++i) {
    assert(i < e && "ran off end of handlers!");
    const EHCatchScope::Handler &handler = catchScope.getHandler(i);

    llvm::Value *typeValue = handler.Type;
    assert(typeValue && "fell into catch-all case!");
    typeValue = CGF.Builder.CreateBitCast(typeValue, CGF.Int8PtrTy);

    // Figure out the next block.
    bool nextIsEnd;
    llvm::BasicBlock *nextBlock;

    // If this is the last handler, we're at the end, and the next
    // block is the block for the enclosing EH scope.
    if (i + 1 == e) {
      nextBlock = CGF.getEHDispatchBlock(catchScope.getEnclosingEHScope());
      nextIsEnd = true;

    // If the next handler is a catch-all, we're at the end, and the
    // next block is that handler.
    } else if (catchScope.getHandler(i+1).isCatchAll()) {
      nextBlock = catchScope.getHandler(i+1).Block;
      nextIsEnd = true;

    // Otherwise, we're not at the end and we need a new block.
    } else {
      nextBlock = CGF.createBasicBlock("catch.fallthrough");
      nextIsEnd = false;
    }

    // Figure out the catch type's index in the LSDA's type table.
    llvm::CallInst *typeIndex =
      CGF.Builder.CreateCall(llvm_eh_typeid_for, typeValue);
    typeIndex->setDoesNotThrow();

    llvm::Value *matchesTypeIndex =
      CGF.Builder.CreateICmpEQ(selector, typeIndex, "matches");
    CGF.Builder.CreateCondBr(matchesTypeIndex, handler.Block, nextBlock);

    // If the next handler is a catch-all, we're completely done.
    if (nextIsEnd) {
      CGF.Builder.restoreIP(savedIP);
      return;
    }
    // Otherwise we need to emit and continue at that block.
    CGF.EmitBlock(nextBlock);
  }
}

void CodeGenFunction::popCatchScope() {
  EHCatchScope &catchScope = cast<EHCatchScope>(*EHStack.begin());
  if (catchScope.hasEHBranches())
    emitCatchDispatchBlock(*this, catchScope);
  EHStack.popCatch();
}

void CodeGenFunction::ExitCXXTryStmt(const CXXTryStmt &S, bool IsFnTryBlock) {
  unsigned NumHandlers = S.getNumHandlers();
  EHCatchScope &CatchScope = cast<EHCatchScope>(*EHStack.begin());
  assert(CatchScope.getNumHandlers() == NumHandlers);

  // If the catch was not required, bail out now.
  if (!CatchScope.hasEHBranches()) {
    CatchScope.clearHandlerBlocks();
    EHStack.popCatch();
    return;
  }

  // Emit the structure of the EH dispatch for this catch.
  emitCatchDispatchBlock(*this, CatchScope);

  // Copy the handler blocks off before we pop the EH stack.  Emitting
  // the handlers might scribble on this memory.
  SmallVector<EHCatchScope::Handler, 8> Handlers(NumHandlers);
  memcpy(Handlers.data(), CatchScope.begin(),
         NumHandlers * sizeof(EHCatchScope::Handler));

  EHStack.popCatch();

  // The fall-through block.
  llvm::BasicBlock *ContBB = createBasicBlock("try.cont");

  // We just emitted the body of the try; jump to the continue block.
  if (HaveInsertPoint())
    Builder.CreateBr(ContBB);

  // Determine if we need an implicit rethrow for all these catch handlers;
  // see the comment below.
  bool doImplicitRethrow = false;
  if (IsFnTryBlock)
    doImplicitRethrow = isa<CXXDestructorDecl>(CurCodeDecl) ||
                        isa<CXXConstructorDecl>(CurCodeDecl);

  // Perversely, we emit the handlers backwards precisely because we
  // want them to appear in source order.  In all of these cases, the
  // catch block will have exactly one predecessor, which will be a
  // particular block in the catch dispatch.  However, in the case of
  // a catch-all, one of the dispatch blocks will branch to two
  // different handlers, and EmitBlockAfterUses will cause the second
  // handler to be moved before the first.
  for (unsigned I = NumHandlers; I != 0; --I) {
    llvm::BasicBlock *CatchBlock = Handlers[I-1].Block;
    EmitBlockAfterUses(CatchBlock);

    // Catch the exception if this isn't a catch-all.
    const CXXCatchStmt *C = S.getHandler(I-1);

    // Enter a cleanup scope, including the catch variable and the
    // end-catch.
    RunCleanupsScope CatchScope(*this);

    // Initialize the catch variable and set up the cleanups.
    CGM.getCXXABI().emitBeginCatch(*this, C);

    // Emit the PGO counter increment.
    incrementProfileCounter(C);

    // Perform the body of the catch.
    EmitStmt(C->getHandlerBlock());

    // [except.handle]p11:
    //   The currently handled exception is rethrown if control
    //   reaches the end of a handler of the function-try-block of a
    //   constructor or destructor.

    // It is important that we only do this on fallthrough and not on
    // return.  Note that it's illegal to put a return in a
    // constructor function-try-block's catch handler (p14), so this
    // really only applies to destructors.
    if (doImplicitRethrow && HaveInsertPoint()) {
      CGM.getCXXABI().emitRethrow(*this, /*isNoReturn*/false);
      Builder.CreateUnreachable();
      Builder.ClearInsertionPoint();
    }

    // Fall out through the catch cleanups.
    CatchScope.ForceCleanup();

    // Branch out of the try.
    if (HaveInsertPoint())
      Builder.CreateBr(ContBB);
  }

  EmitBlock(ContBB);
  incrementProfileCounter(&S);
}

namespace {
  struct CallEndCatchForFinally : EHScopeStack::Cleanup {
    llvm::Value *ForEHVar;
    llvm::Value *EndCatchFn;
    CallEndCatchForFinally(llvm::Value *ForEHVar, llvm::Value *EndCatchFn)
      : ForEHVar(ForEHVar), EndCatchFn(EndCatchFn) {}

    void Emit(CodeGenFunction &CGF, Flags flags) override {
      llvm::BasicBlock *EndCatchBB = CGF.createBasicBlock("finally.endcatch");
      llvm::BasicBlock *CleanupContBB =
        CGF.createBasicBlock("finally.cleanup.cont");

      llvm::Value *ShouldEndCatch =
        CGF.Builder.CreateLoad(ForEHVar, "finally.endcatch");
      CGF.Builder.CreateCondBr(ShouldEndCatch, EndCatchBB, CleanupContBB);
      CGF.EmitBlock(EndCatchBB);
      CGF.EmitRuntimeCallOrInvoke(EndCatchFn); // catch-all, so might throw
      CGF.EmitBlock(CleanupContBB);
    }
  };

  struct PerformFinally : EHScopeStack::Cleanup {
    const Stmt *Body;
    llvm::Value *ForEHVar;
    llvm::Value *EndCatchFn;
    llvm::Value *RethrowFn;
    llvm::Value *SavedExnVar;

    PerformFinally(const Stmt *Body, llvm::Value *ForEHVar,
                   llvm::Value *EndCatchFn,
                   llvm::Value *RethrowFn, llvm::Value *SavedExnVar)
      : Body(Body), ForEHVar(ForEHVar), EndCatchFn(EndCatchFn),
        RethrowFn(RethrowFn), SavedExnVar(SavedExnVar) {}

    void Emit(CodeGenFunction &CGF, Flags flags) override {
      // Enter a cleanup to call the end-catch function if one was provided.
      if (EndCatchFn)
        CGF.EHStack.pushCleanup<CallEndCatchForFinally>(NormalAndEHCleanup,
                                                        ForEHVar, EndCatchFn);

      // Save the current cleanup destination in case there are
      // cleanups in the finally block.
      llvm::Value *SavedCleanupDest =
        CGF.Builder.CreateLoad(CGF.getNormalCleanupDestSlot(),
                               "cleanup.dest.saved");

      // Emit the finally block.
      CGF.EmitStmt(Body);

      // If the end of the finally is reachable, check whether this was
      // for EH.  If so, rethrow.
      if (CGF.HaveInsertPoint()) {
        llvm::BasicBlock *RethrowBB = CGF.createBasicBlock("finally.rethrow");
        llvm::BasicBlock *ContBB = CGF.createBasicBlock("finally.cont");

        llvm::Value *ShouldRethrow =
          CGF.Builder.CreateLoad(ForEHVar, "finally.shouldthrow");
        CGF.Builder.CreateCondBr(ShouldRethrow, RethrowBB, ContBB);

        CGF.EmitBlock(RethrowBB);
        if (SavedExnVar) {
          CGF.EmitRuntimeCallOrInvoke(RethrowFn,
                                      CGF.Builder.CreateLoad(SavedExnVar));
        } else {
          CGF.EmitRuntimeCallOrInvoke(RethrowFn);
        }
        CGF.Builder.CreateUnreachable();

        CGF.EmitBlock(ContBB);

        // Restore the cleanup destination.
        CGF.Builder.CreateStore(SavedCleanupDest,
                                CGF.getNormalCleanupDestSlot());
      }

      // Leave the end-catch cleanup.  As an optimization, pretend that
      // the fallthrough path was inaccessible; we've dynamically proven
      // that we're not in the EH case along that path.
      if (EndCatchFn) {
        CGBuilderTy::InsertPoint SavedIP = CGF.Builder.saveAndClearIP();
        CGF.PopCleanupBlock();
        CGF.Builder.restoreIP(SavedIP);
      }
    
      // Now make sure we actually have an insertion point or the
      // cleanup gods will hate us.
      CGF.EnsureInsertPoint();
    }
  };
}

/// Enters a finally block for an implementation using zero-cost
/// exceptions.  This is mostly general, but hard-codes some
/// language/ABI-specific behavior in the catch-all sections.
void CodeGenFunction::FinallyInfo::enter(CodeGenFunction &CGF,
                                         const Stmt *body,
                                         llvm::Constant *beginCatchFn,
                                         llvm::Constant *endCatchFn,
                                         llvm::Constant *rethrowFn) {
  assert((beginCatchFn != nullptr) == (endCatchFn != nullptr) &&
         "begin/end catch functions not paired");
  assert(rethrowFn && "rethrow function is required");

  BeginCatchFn = beginCatchFn;

  // The rethrow function has one of the following two types:
  //   void (*)()
  //   void (*)(void*)
  // In the latter case we need to pass it the exception object.
  // But we can't use the exception slot because the @finally might
  // have a landing pad (which would overwrite the exception slot).
  llvm::FunctionType *rethrowFnTy =
    cast<llvm::FunctionType>(
      cast<llvm::PointerType>(rethrowFn->getType())->getElementType());
  SavedExnVar = nullptr;
  if (rethrowFnTy->getNumParams())
    SavedExnVar = CGF.CreateTempAlloca(CGF.Int8PtrTy, "finally.exn");

  // A finally block is a statement which must be executed on any edge
  // out of a given scope.  Unlike a cleanup, the finally block may
  // contain arbitrary control flow leading out of itself.  In
  // addition, finally blocks should always be executed, even if there
  // are no catch handlers higher on the stack.  Therefore, we
  // surround the protected scope with a combination of a normal
  // cleanup (to catch attempts to break out of the block via normal
  // control flow) and an EH catch-all (semantically "outside" any try
  // statement to which the finally block might have been attached).
  // The finally block itself is generated in the context of a cleanup
  // which conditionally leaves the catch-all.

  // Jump destination for performing the finally block on an exception
  // edge.  We'll never actually reach this block, so unreachable is
  // fine.
  RethrowDest = CGF.getJumpDestInCurrentScope(CGF.getUnreachableBlock());

  // Whether the finally block is being executed for EH purposes.
  ForEHVar = CGF.CreateTempAlloca(CGF.Builder.getInt1Ty(), "finally.for-eh");
  CGF.Builder.CreateStore(CGF.Builder.getFalse(), ForEHVar);

  // Enter a normal cleanup which will perform the @finally block.
  CGF.EHStack.pushCleanup<PerformFinally>(NormalCleanup, body,
                                          ForEHVar, endCatchFn,
                                          rethrowFn, SavedExnVar);

  // Enter a catch-all scope.
  llvm::BasicBlock *catchBB = CGF.createBasicBlock("finally.catchall");
  EHCatchScope *catchScope = CGF.EHStack.pushCatch(1);
  catchScope->setCatchAllHandler(0, catchBB);
}

void CodeGenFunction::FinallyInfo::exit(CodeGenFunction &CGF) {
  // Leave the finally catch-all.
  EHCatchScope &catchScope = cast<EHCatchScope>(*CGF.EHStack.begin());
  llvm::BasicBlock *catchBB = catchScope.getHandler(0).Block;

  CGF.popCatchScope();

  // If there are any references to the catch-all block, emit it.
  if (catchBB->use_empty()) {
    delete catchBB;
  } else {
    CGBuilderTy::InsertPoint savedIP = CGF.Builder.saveAndClearIP();
    CGF.EmitBlock(catchBB);

    llvm::Value *exn = nullptr;

    // If there's a begin-catch function, call it.
    if (BeginCatchFn) {
      exn = CGF.getExceptionFromSlot();
      CGF.EmitNounwindRuntimeCall(BeginCatchFn, exn);
    }

    // If we need to remember the exception pointer to rethrow later, do so.
    if (SavedExnVar) {
      if (!exn) exn = CGF.getExceptionFromSlot();
      CGF.Builder.CreateStore(exn, SavedExnVar);
    }

    // Tell the cleanups in the finally block that we're do this for EH.
    CGF.Builder.CreateStore(CGF.Builder.getTrue(), ForEHVar);

    // Thread a jump through the finally cleanup.
    CGF.EmitBranchThroughCleanup(RethrowDest);

    CGF.Builder.restoreIP(savedIP);
  }

  // Finally, leave the @finally cleanup.
  CGF.PopCleanupBlock();
}

llvm::BasicBlock *CodeGenFunction::getTerminateLandingPad() {
  if (TerminateLandingPad)
    return TerminateLandingPad;

  CGBuilderTy::InsertPoint SavedIP = Builder.saveAndClearIP();

  // This will get inserted at the end of the function.
  TerminateLandingPad = createBasicBlock("terminate.lpad");
  Builder.SetInsertPoint(TerminateLandingPad);

  // Tell the backend that this is a landing pad.
  const EHPersonality &Personality = EHPersonality::get(*this);

  if (!CurFn->hasPersonalityFn())
    CurFn->setPersonalityFn(getOpaquePersonalityFn(CGM, Personality));

  llvm::LandingPadInst *LPadInst = Builder.CreateLandingPad(
      llvm::StructType::get(Int8PtrTy, Int32Ty, nullptr), 0);
  LPadInst->addClause(getCatchAllValue(*this));

  llvm::Value *Exn = 0;
  if (getLangOpts().CPlusPlus)
    Exn = Builder.CreateExtractValue(LPadInst, 0);
  llvm::CallInst *terminateCall =
      CGM.getCXXABI().emitTerminateForUnexpectedException(*this, Exn);
  terminateCall->setDoesNotReturn();
  Builder.CreateUnreachable();

  // Restore the saved insertion state.
  Builder.restoreIP(SavedIP);

  return TerminateLandingPad;
}

llvm::BasicBlock *CodeGenFunction::getTerminateHandler() {
  if (TerminateHandler)
    return TerminateHandler;

  CGBuilderTy::InsertPoint SavedIP = Builder.saveAndClearIP();

  // Set up the terminate handler.  This block is inserted at the very
  // end of the function by FinishFunction.
  TerminateHandler = createBasicBlock("terminate.handler");
  Builder.SetInsertPoint(TerminateHandler);
  llvm::Value *Exn = 0;
  if (getLangOpts().CPlusPlus)
    Exn = getExceptionFromSlot();
  llvm::CallInst *terminateCall =
      CGM.getCXXABI().emitTerminateForUnexpectedException(*this, Exn);
  terminateCall->setDoesNotReturn();
  Builder.CreateUnreachable();

  // Restore the saved insertion state.
  Builder.restoreIP(SavedIP);

  return TerminateHandler;
}

llvm::BasicBlock *CodeGenFunction::getEHResumeBlock(bool isCleanup) {
  if (EHResumeBlock) return EHResumeBlock;

  CGBuilderTy::InsertPoint SavedIP = Builder.saveIP();

  // We emit a jump to a notional label at the outermost unwind state.
  EHResumeBlock = createBasicBlock("eh.resume");
  Builder.SetInsertPoint(EHResumeBlock);

  const EHPersonality &Personality = EHPersonality::get(*this);

  // This can always be a call because we necessarily didn't find
  // anything on the EH stack which needs our help.
  const char *RethrowName = Personality.CatchallRethrowFn;
  if (RethrowName != nullptr && !isCleanup) {
    EmitRuntimeCall(getCatchallRethrowFn(CGM, RethrowName),
                    getExceptionFromSlot())->setDoesNotReturn();
    Builder.CreateUnreachable();
    Builder.restoreIP(SavedIP);
    return EHResumeBlock;
  }

  // Recreate the landingpad's return value for the 'resume' instruction.
  llvm::Value *Exn = getExceptionFromSlot();
  llvm::Value *Sel = getSelectorFromSlot();

  llvm::Type *LPadType = llvm::StructType::get(Exn->getType(),
                                               Sel->getType(), nullptr);
  llvm::Value *LPadVal = llvm::UndefValue::get(LPadType);
  LPadVal = Builder.CreateInsertValue(LPadVal, Exn, 0, "lpad.val");
  LPadVal = Builder.CreateInsertValue(LPadVal, Sel, 1, "lpad.val");

  Builder.CreateResume(LPadVal);
  Builder.restoreIP(SavedIP);
  return EHResumeBlock;
}

void CodeGenFunction::EmitSEHTryStmt(const SEHTryStmt &S) {
  EnterSEHTryStmt(S);
  {
    JumpDest TryExit = getJumpDestInCurrentScope("__try.__leave");

    SEHTryEpilogueStack.push_back(&TryExit);
    EmitStmt(S.getTryBlock());
    SEHTryEpilogueStack.pop_back();

    if (!TryExit.getBlock()->use_empty())
      EmitBlock(TryExit.getBlock(), /*IsFinished=*/true);
    else
      delete TryExit.getBlock();
  }
  ExitSEHTryStmt(S);
}

namespace {
struct PerformSEHFinally : EHScopeStack::Cleanup {
  llvm::Function *OutlinedFinally;
  PerformSEHFinally(llvm::Function *OutlinedFinally)
      : OutlinedFinally(OutlinedFinally) {}

  void Emit(CodeGenFunction &CGF, Flags F) override {
    ASTContext &Context = CGF.getContext();
    CodeGenModule &CGM = CGF.CGM;

    CallArgList Args;

    // Compute the two argument values.
    QualType ArgTys[2] = {Context.UnsignedCharTy, Context.VoidPtrTy};
    llvm::Value *LocalAddrFn = CGM.getIntrinsic(llvm::Intrinsic::localaddress);
    llvm::Value *FP = CGF.Builder.CreateCall(LocalAddrFn);
    llvm::Value *IsForEH =
        llvm::ConstantInt::get(CGF.ConvertType(ArgTys[0]), F.isForEHCleanup());
    Args.add(RValue::get(IsForEH), ArgTys[0]);
    Args.add(RValue::get(FP), ArgTys[1]);

    // Arrange a two-arg function info and type.
    FunctionProtoType::ExtProtoInfo EPI;
    const auto *FPT = cast<FunctionProtoType>(
        Context.getFunctionType(Context.VoidTy, ArgTys, EPI));
    const CGFunctionInfo &FnInfo =
        CGM.getTypes().arrangeFreeFunctionCall(Args, FPT,
                                               /*chainCall=*/false);

    CGF.EmitCall(FnInfo, OutlinedFinally, ReturnValueSlot(), Args);
  }
};
}

namespace {
/// Find all local variable captures in the statement.
struct CaptureFinder : ConstStmtVisitor<CaptureFinder> {
  CodeGenFunction &ParentCGF;
  const VarDecl *ParentThis;
  SmallVector<const VarDecl *, 4> Captures;
  llvm::Value *SEHCodeSlot = nullptr;
  CaptureFinder(CodeGenFunction &ParentCGF, const VarDecl *ParentThis)
      : ParentCGF(ParentCGF), ParentThis(ParentThis) {}

  // Return true if we need to do any capturing work.
  bool foundCaptures() {
    return !Captures.empty() || SEHCodeSlot;
  }

  void Visit(const Stmt *S) {
    // See if this is a capture, then recurse.
    ConstStmtVisitor<CaptureFinder>::Visit(S);
    for (const Stmt *Child : S->children())
      if (Child)
        Visit(Child);
  }

  void VisitDeclRefExpr(const DeclRefExpr *E) {
    // If this is already a capture, just make sure we capture 'this'.
    if (E->refersToEnclosingVariableOrCapture()) {
      Captures.push_back(ParentThis);
      return;
    }

    const auto *D = dyn_cast<VarDecl>(E->getDecl());
    if (D && D->isLocalVarDeclOrParm() && D->hasLocalStorage())
      Captures.push_back(D);
  }

  void VisitCXXThisExpr(const CXXThisExpr *E) {
    Captures.push_back(ParentThis);
  }

  void VisitCallExpr(const CallExpr *E) {
    // We only need to add parent frame allocations for these builtins in x86.
    if (ParentCGF.getTarget().getTriple().getArch() != llvm::Triple::x86)
      return;

    unsigned ID = E->getBuiltinCallee();
    switch (ID) {
    case Builtin::BI__exception_code:
    case Builtin::BI_exception_code:
      // This is the simple case where we are the outermost finally. All we
      // have to do here is make sure we escape this and recover it in the
      // outlined handler.
      if (!SEHCodeSlot)
        SEHCodeSlot = ParentCGF.SEHCodeSlotStack.back();
      break;
    }
  }
};
}

llvm::Value *CodeGenFunction::recoverAddrOfEscapedLocal(
    CodeGenFunction &ParentCGF, llvm::Value *ParentVar, llvm::Value *ParentFP) {
  llvm::CallInst *RecoverCall = nullptr;
  CGBuilderTy Builder(AllocaInsertPt);
  if (auto *ParentAlloca = dyn_cast<llvm::AllocaInst>(ParentVar)) {
    // Mark the variable escaped if nobody else referenced it and compute the
    // localescape index.
    auto InsertPair = ParentCGF.EscapedLocals.insert(
        std::make_pair(ParentAlloca, ParentCGF.EscapedLocals.size()));
    int FrameEscapeIdx = InsertPair.first->second;
    // call i8* @llvm.localrecover(i8* bitcast(@parentFn), i8* %fp, i32 N)
    llvm::Function *FrameRecoverFn = llvm::Intrinsic::getDeclaration(
        &CGM.getModule(), llvm::Intrinsic::localrecover);
    llvm::Constant *ParentI8Fn =
        llvm::ConstantExpr::getBitCast(ParentCGF.CurFn, Int8PtrTy);
    RecoverCall = Builder.CreateCall(
        FrameRecoverFn, {ParentI8Fn, ParentFP,
                         llvm::ConstantInt::get(Int32Ty, FrameEscapeIdx)});

  } else {
    // If the parent didn't have an alloca, we're doing some nested outlining.
    // Just clone the existing localrecover call, but tweak the FP argument to
    // use our FP value. All other arguments are constants.
    auto *ParentRecover =
        cast<llvm::IntrinsicInst>(ParentVar->stripPointerCasts());
    assert(ParentRecover->getIntrinsicID() == llvm::Intrinsic::localrecover &&
           "expected alloca or localrecover in parent LocalDeclMap");
    RecoverCall = cast<llvm::CallInst>(ParentRecover->clone());
    RecoverCall->setArgOperand(1, ParentFP);
    RecoverCall->insertBefore(AllocaInsertPt);
  }

  // Bitcast the variable, rename it, and insert it in the local decl map.
  llvm::Value *ChildVar =
      Builder.CreateBitCast(RecoverCall, ParentVar->getType());
  ChildVar->setName(ParentVar->getName());
  return ChildVar;
}

void CodeGenFunction::EmitCapturedLocals(CodeGenFunction &ParentCGF,
                                         const Stmt *OutlinedStmt,
                                         bool IsFilter) {
  // Find all captures in the Stmt.
  CaptureFinder Finder(ParentCGF, ParentCGF.CXXABIThisDecl);
  Finder.Visit(OutlinedStmt);

  // We can exit early on x86_64 when there are no captures. We just have to
  // save the exception code in filters so that __exception_code() works.
  if (!Finder.foundCaptures() &&
      CGM.getTarget().getTriple().getArch() != llvm::Triple::x86) {
    if (IsFilter)
      EmitSEHExceptionCodeSave(ParentCGF, nullptr, nullptr);
    return;
  }

  llvm::Value *EntryEBP = nullptr;
  llvm::Value *ParentFP;
  if (IsFilter && CGM.getTarget().getTriple().getArch() == llvm::Triple::x86) {
    // 32-bit SEH filters need to be careful about FP recovery.  The end of the
    // EH registration is passed in as the EBP physical register.  We can
    // recover that with llvm.frameaddress(1), and adjust that to recover the
    // parent's true frame pointer.
    CGBuilderTy Builder(AllocaInsertPt);
    EntryEBP = Builder.CreateCall(
        CGM.getIntrinsic(llvm::Intrinsic::frameaddress), {Builder.getInt32(1)});
    llvm::Function *RecoverFPIntrin =
        CGM.getIntrinsic(llvm::Intrinsic::x86_seh_recoverfp);
    llvm::Constant *ParentI8Fn =
        llvm::ConstantExpr::getBitCast(ParentCGF.CurFn, Int8PtrTy);
    ParentFP = Builder.CreateCall(RecoverFPIntrin, {ParentI8Fn, EntryEBP});
  } else {
    // Otherwise, for x64 and 32-bit finally functions, the parent FP is the
    // second parameter.
    auto AI = CurFn->arg_begin();
    ++AI;
    ParentFP = AI;
  }

  // Create llvm.localrecover calls for all captures.
  for (const VarDecl *VD : Finder.Captures) {
    if (isa<ImplicitParamDecl>(VD)) {
      CGM.ErrorUnsupported(VD, "'this' captured by SEH");
      CXXThisValue = llvm::UndefValue::get(ConvertTypeForMem(VD->getType()));
      continue;
    }
    if (VD->getType()->isVariablyModifiedType()) {
      CGM.ErrorUnsupported(VD, "VLA captured by SEH");
      continue;
    }
    assert((isa<ImplicitParamDecl>(VD) || VD->isLocalVarDeclOrParm()) &&
           "captured non-local variable");

    // If this decl hasn't been declared yet, it will be declared in the
    // OutlinedStmt.
    auto I = ParentCGF.LocalDeclMap.find(VD);
    if (I == ParentCGF.LocalDeclMap.end())
      continue;
    llvm::Value *ParentVar = I->second;

    LocalDeclMap[VD] =
        recoverAddrOfEscapedLocal(ParentCGF, ParentVar, ParentFP);
  }

  if (Finder.SEHCodeSlot) {
    SEHCodeSlotStack.push_back(
        recoverAddrOfEscapedLocal(ParentCGF, Finder.SEHCodeSlot, ParentFP));
  }

  if (IsFilter)
    EmitSEHExceptionCodeSave(ParentCGF, ParentFP, EntryEBP);
}

/// Arrange a function prototype that can be called by Windows exception
/// handling personalities. On Win64, the prototype looks like:
/// RetTy func(void *EHPtrs, void *ParentFP);
void CodeGenFunction::startOutlinedSEHHelper(CodeGenFunction &ParentCGF,
                                             bool IsFilter,
                                             const Stmt *OutlinedStmt) {
  SourceLocation StartLoc = OutlinedStmt->getLocStart();

  // Get the mangled function name.
  SmallString<128> Name;
  {
    llvm::raw_svector_ostream OS(Name);
    const Decl *ParentCodeDecl = ParentCGF.CurCodeDecl;
    const NamedDecl *Parent = dyn_cast_or_null<NamedDecl>(ParentCodeDecl);
    assert(Parent && "FIXME: handle unnamed decls (lambdas, blocks) with SEH");
    MangleContext &Mangler = CGM.getCXXABI().getMangleContext();
    if (IsFilter)
      Mangler.mangleSEHFilterExpression(Parent, OS);
    else
      Mangler.mangleSEHFinallyBlock(Parent, OS);
  }

  FunctionArgList Args;
  if (CGM.getTarget().getTriple().getArch() != llvm::Triple::x86 || !IsFilter) {
    // All SEH finally functions take two parameters. Win64 filters take two
    // parameters. Win32 filters take no parameters.
    if (IsFilter) {
      Args.push_back(ImplicitParamDecl::Create(
          getContext(), nullptr, StartLoc,
          &getContext().Idents.get("exception_pointers"),
          getContext().VoidPtrTy));
    } else {
      Args.push_back(ImplicitParamDecl::Create(
          getContext(), nullptr, StartLoc,
          &getContext().Idents.get("abnormal_termination"),
          getContext().UnsignedCharTy));
    }
    Args.push_back(ImplicitParamDecl::Create(
        getContext(), nullptr, StartLoc,
        &getContext().Idents.get("frame_pointer"), getContext().VoidPtrTy));
  }

  QualType RetTy = IsFilter ? getContext().LongTy : getContext().VoidTy;

  llvm::Function *ParentFn = ParentCGF.CurFn;
  const CGFunctionInfo &FnInfo = CGM.getTypes().arrangeFreeFunctionDeclaration(
      RetTy, Args, FunctionType::ExtInfo(), /*isVariadic=*/false);

  llvm::FunctionType *FnTy = CGM.getTypes().GetFunctionType(FnInfo);
  llvm::Function *Fn = llvm::Function::Create(
      FnTy, llvm::GlobalValue::InternalLinkage, Name.str(), &CGM.getModule());
  // The filter is either in the same comdat as the function, or it's internal.
  if (llvm::Comdat *C = ParentFn->getComdat()) {
    Fn->setComdat(C);
  } else if (ParentFn->hasWeakLinkage() || ParentFn->hasLinkOnceLinkage()) {
    llvm::Comdat *C = CGM.getModule().getOrInsertComdat(ParentFn->getName());
    ParentFn->setComdat(C);
    Fn->setComdat(C);
  } else {
    Fn->setLinkage(llvm::GlobalValue::InternalLinkage);
  }

  IsOutlinedSEHHelper = true;

  StartFunction(GlobalDecl(), RetTy, Fn, FnInfo, Args,
                OutlinedStmt->getLocStart(), OutlinedStmt->getLocStart());

  CGM.SetLLVMFunctionAttributes(nullptr, FnInfo, CurFn);
  EmitCapturedLocals(ParentCGF, OutlinedStmt, IsFilter);
}

/// Create a stub filter function that will ultimately hold the code of the
/// filter expression. The EH preparation passes in LLVM will outline the code
/// from the main function body into this stub.
llvm::Function *
CodeGenFunction::GenerateSEHFilterFunction(CodeGenFunction &ParentCGF,
                                           const SEHExceptStmt &Except) {
  const Expr *FilterExpr = Except.getFilterExpr();
  startOutlinedSEHHelper(ParentCGF, true, FilterExpr);

  // Emit the original filter expression, convert to i32, and return.
  llvm::Value *R = EmitScalarExpr(FilterExpr);
  R = Builder.CreateIntCast(R, ConvertType(getContext().LongTy),
                            FilterExpr->getType()->isSignedIntegerType());
  Builder.CreateStore(R, ReturnValue);

  FinishFunction(FilterExpr->getLocEnd());

  return CurFn;
}

llvm::Function *
CodeGenFunction::GenerateSEHFinallyFunction(CodeGenFunction &ParentCGF,
                                            const SEHFinallyStmt &Finally) {
  const Stmt *FinallyBlock = Finally.getBlock();
  startOutlinedSEHHelper(ParentCGF, false, FinallyBlock);

  // Mark finally block calls as nounwind and noinline to make LLVM's job a
  // little easier.
  // FIXME: Remove these restrictions in the future.
  CurFn->addFnAttr(llvm::Attribute::NoUnwind);
  CurFn->addFnAttr(llvm::Attribute::NoInline);

  // Emit the original filter expression, convert to i32, and return.
  EmitStmt(FinallyBlock);

  FinishFunction(FinallyBlock->getLocEnd());

  return CurFn;
}

void CodeGenFunction::EmitSEHExceptionCodeSave(CodeGenFunction &ParentCGF,
                                               llvm::Value *ParentFP,
                                               llvm::Value *EntryEBP) {
  // Get the pointer to the EXCEPTION_POINTERS struct. This is returned by the
  // __exception_info intrinsic.
  if (CGM.getTarget().getTriple().getArch() != llvm::Triple::x86) {
    // On Win64, the info is passed as the first parameter to the filter.
    auto AI = CurFn->arg_begin();
    SEHInfo = AI;
    SEHCodeSlotStack.push_back(
        CreateMemTemp(getContext().IntTy, "__exception_code"));
  } else {
    // On Win32, the EBP on entry to the filter points to the end of an
    // exception registration object. It contains 6 32-bit fields, and the info
    // pointer is stored in the second field. So, GEP 20 bytes backwards and
    // load the pointer.
    SEHInfo = Builder.CreateConstInBoundsGEP1_32(Int8Ty, EntryEBP, -20);
    SEHInfo = Builder.CreateBitCast(SEHInfo, Int8PtrTy->getPointerTo());
    SEHInfo = Builder.CreateLoad(Int8PtrTy, SEHInfo);
    SEHCodeSlotStack.push_back(recoverAddrOfEscapedLocal(
        ParentCGF, ParentCGF.SEHCodeSlotStack.back(), ParentFP));
  }

  // Save the exception code in the exception slot to unify exception access in
  // the filter function and the landing pad.
  // struct EXCEPTION_POINTERS {
  //   EXCEPTION_RECORD *ExceptionRecord;
  //   CONTEXT *ContextRecord;
  // };
  // int exceptioncode = exception_pointers->ExceptionRecord->ExceptionCode;
  llvm::Type *RecordTy = CGM.Int32Ty->getPointerTo();
  llvm::Type *PtrsTy = llvm::StructType::get(RecordTy, CGM.VoidPtrTy, nullptr);
  llvm::Value *Ptrs = Builder.CreateBitCast(SEHInfo, PtrsTy->getPointerTo());
  llvm::Value *Rec = Builder.CreateStructGEP(PtrsTy, Ptrs, 0);
  Rec = Builder.CreateLoad(Rec);
  llvm::Value *Code = Builder.CreateLoad(Rec);
  assert(!SEHCodeSlotStack.empty() && "emitting EH code outside of __except");
  Builder.CreateStore(Code, SEHCodeSlotStack.back());
}

llvm::Value *CodeGenFunction::EmitSEHExceptionInfo() {
  // Sema should diagnose calling this builtin outside of a filter context, but
  // don't crash if we screw up.
  if (!SEHInfo)
    return llvm::UndefValue::get(Int8PtrTy);
  assert(SEHInfo->getType() == Int8PtrTy);
  return SEHInfo;
}

llvm::Value *CodeGenFunction::EmitSEHExceptionCode() {
  assert(!SEHCodeSlotStack.empty() && "emitting EH code outside of __except");
  return Builder.CreateLoad(Int32Ty, SEHCodeSlotStack.back());
}

llvm::Value *CodeGenFunction::EmitSEHAbnormalTermination() {
  // Abnormal termination is just the first parameter to the outlined finally
  // helper.
  auto AI = CurFn->arg_begin();
  return Builder.CreateZExt(&*AI, Int32Ty);
}

void CodeGenFunction::EnterSEHTryStmt(const SEHTryStmt &S) {
  CodeGenFunction HelperCGF(CGM, /*suppressNewContext=*/true);
  if (const SEHFinallyStmt *Finally = S.getFinallyHandler()) {
    // Outline the finally block.
    llvm::Function *FinallyFunc =
        HelperCGF.GenerateSEHFinallyFunction(*this, *Finally);

    // Push a cleanup for __finally blocks.
    EHStack.pushCleanup<PerformSEHFinally>(NormalAndEHCleanup, FinallyFunc);
    return;
  }

  // Otherwise, we must have an __except block.
  const SEHExceptStmt *Except = S.getExceptHandler();
  assert(Except);
  EHCatchScope *CatchScope = EHStack.pushCatch(1);
  SEHCodeSlotStack.push_back(
      CreateMemTemp(getContext().IntTy, "__exception_code"));

  // If the filter is known to evaluate to 1, then we can use the clause
  // "catch i8* null". We can't do this on x86 because the filter has to save
  // the exception code.
  llvm::Constant *C =
      CGM.EmitConstantExpr(Except->getFilterExpr(), getContext().IntTy, this);
  if (CGM.getTarget().getTriple().getArch() != llvm::Triple::x86 && C &&
      C->isOneValue()) {
    CatchScope->setCatchAllHandler(0, createBasicBlock("__except"));
    return;
  }

  // In general, we have to emit an outlined filter function. Use the function
  // in place of the RTTI typeinfo global that C++ EH uses.
  llvm::Function *FilterFunc =
      HelperCGF.GenerateSEHFilterFunction(*this, *Except);
  llvm::Constant *OpaqueFunc =
      llvm::ConstantExpr::getBitCast(FilterFunc, Int8PtrTy);
  CatchScope->setHandler(0, OpaqueFunc, createBasicBlock("__except"));
}

void CodeGenFunction::ExitSEHTryStmt(const SEHTryStmt &S) {
  // Just pop the cleanup if it's a __finally block.
  if (S.getFinallyHandler()) {
    PopCleanupBlock();
    return;
  }

  // Otherwise, we must have an __except block.
  const SEHExceptStmt *Except = S.getExceptHandler();
  assert(Except && "__try must have __finally xor __except");
  EHCatchScope &CatchScope = cast<EHCatchScope>(*EHStack.begin());

  // Don't emit the __except block if the __try block lacked invokes.
  // TODO: Model unwind edges from instructions, either with iload / istore or
  // a try body function.
  if (!CatchScope.hasEHBranches()) {
    CatchScope.clearHandlerBlocks();
    EHStack.popCatch();
    SEHCodeSlotStack.pop_back();
    return;
  }

  // The fall-through block.
  llvm::BasicBlock *ContBB = createBasicBlock("__try.cont");

  // We just emitted the body of the __try; jump to the continue block.
  if (HaveInsertPoint())
    Builder.CreateBr(ContBB);

  // Check if our filter function returned true.
  emitCatchDispatchBlock(*this, CatchScope);

  // Grab the block before we pop the handler.
  llvm::BasicBlock *ExceptBB = CatchScope.getHandler(0).Block;
  EHStack.popCatch();

  EmitBlockAfterUses(ExceptBB);

  // On Win64, the exception pointer is the exception code. Copy it to the slot.
  if (CGM.getTarget().getTriple().getArch() != llvm::Triple::x86) {
    llvm::Value *Code =
        Builder.CreatePtrToInt(getExceptionFromSlot(), IntPtrTy);
    Code = Builder.CreateTrunc(Code, Int32Ty);
    Builder.CreateStore(Code, SEHCodeSlotStack.back());
  }

  // Emit the __except body.
  EmitStmt(Except->getBlock());

  // End the lifetime of the exception code.
  SEHCodeSlotStack.pop_back();

  if (HaveInsertPoint())
    Builder.CreateBr(ContBB);

  EmitBlock(ContBB);
}

void CodeGenFunction::EmitSEHLeaveStmt(const SEHLeaveStmt &S) {
  // If this code is reachable then emit a stop point (if generating
  // debug info). We have to do this ourselves because we are on the
  // "simple" statement path.
  if (HaveInsertPoint())
    EmitStopPoint(&S);

  // This must be a __leave from a __finally block, which we warn on and is UB.
  // Just emit unreachable.
  if (!isSEHTryScope()) {
    Builder.CreateUnreachable();
    Builder.ClearInsertionPoint();
    return;
  }

  EmitBranchThroughCleanup(*SEHTryEpilogueStack.back());
}

#endif // HLSL Change
