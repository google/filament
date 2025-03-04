//===----- CGCall.h - Encapsulate calling convention details ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// These classes wrap the information about a call or function
// definition used to handle ABI compliancy.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_CODEGEN_CGCALL_H
#define LLVM_CLANG_LIB_CODEGEN_CGCALL_H

#include "CGValue.h"
#include "EHScopeStack.h"
#include "clang/AST/CanonicalType.h"
#include "clang/AST/Type.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/IR/Value.h"

// FIXME: Restructure so we don't have to expose so much stuff.
#include "ABIInfo.h"

namespace llvm {
  class AttributeSet;
  class Function;
  class Type;
  class Value;
}

namespace clang {
  class ASTContext;
  class Decl;
  class FunctionDecl;
  class ObjCMethodDecl;
  class VarDecl;

namespace CodeGen {
  typedef SmallVector<llvm::AttributeSet, 8> AttributeListType;

  struct CallArg {
    RValue RV;
    QualType Ty;
    bool NeedsCopy;
    CallArg(RValue rv, QualType ty, bool needscopy)
    : RV(rv), Ty(ty), NeedsCopy(needscopy)
    { }
  };

  /// CallArgList - Type for representing both the value and type of
  /// arguments in a call.
  class CallArgList :
    public SmallVector<CallArg, 16> {
  public:
    CallArgList() : StackBase(nullptr), StackBaseMem(nullptr) {}

    struct Writeback {
      /// The original argument.  Note that the argument l-value
      /// is potentially null.
      LValue Source;

      /// The temporary alloca.
      llvm::Value *Temporary;

      /// A value to "use" after the writeback, or null.
      llvm::Value *ToUse;
    };

    struct CallArgCleanup {
      EHScopeStack::stable_iterator Cleanup;

      /// The "is active" insertion point.  This instruction is temporary and
      /// will be removed after insertion.
      llvm::Instruction *IsActiveIP;
    };

    void add(RValue rvalue, QualType type, bool needscopy = false) {
      push_back(CallArg(rvalue, type, needscopy));
    }

    void addFrom(const CallArgList &other) {
      insert(end(), other.begin(), other.end());
#if 0 // HLSL Change - no ObjC support
      Writebacks.insert(Writebacks.end(),
                        other.Writebacks.begin(), other.Writebacks.end());
#else
      assert(!other.hasWritebacks() && "writeback is unreachable in HLSL");
#endif // HLSL Change - no ObjC support
    }

    void addWriteback(LValue srcLV, llvm::Value *temporary,
                      llvm::Value *toUse) {
#if 0 // HLSL Change - no ObjC support
      Writeback writeback;
      writeback.Source = srcLV;
      writeback.Temporary = temporary;
      writeback.ToUse = toUse;
      Writebacks.push_back(writeback);
#else
      llvm_unreachable("addWriteback is unreachable in HLSL");
#endif // HLSL Change - no ObjC support
    }

    bool hasWritebacks() const { return !Writebacks.empty(); }

    typedef llvm::iterator_range<SmallVectorImpl<Writeback>::const_iterator>
      writeback_const_range;

    writeback_const_range writebacks() const {
      return writeback_const_range(Writebacks.begin(), Writebacks.end());
    }

    void addArgCleanupDeactivation(EHScopeStack::stable_iterator Cleanup,
                                   llvm::Instruction *IsActiveIP) {
      CallArgCleanup ArgCleanup;
      ArgCleanup.Cleanup = Cleanup;
      ArgCleanup.IsActiveIP = IsActiveIP;
      CleanupsToDeactivate.push_back(ArgCleanup);
    }

    ArrayRef<CallArgCleanup> getCleanupsToDeactivate() const {
      return CleanupsToDeactivate;
    }

    void allocateArgumentMemory(CodeGenFunction &CGF);
    llvm::Instruction *getStackBase() const { return StackBase; }
    void freeArgumentMemory(CodeGenFunction &CGF) const;

    /// \brief Returns if we're using an inalloca struct to pass arguments in
    /// memory.
    bool isUsingInAlloca() const { return StackBase; }

  private:
    SmallVector<Writeback, 1> Writebacks;

    /// Deactivate these cleanups immediately before making the call.  This
    /// is used to cleanup objects that are owned by the callee once the call
    /// occurs.
    SmallVector<CallArgCleanup, 1> CleanupsToDeactivate;

    /// The stacksave call.  It dominates all of the argument evaluation.
    llvm::CallInst *StackBase;

    /// The alloca holding the stackbase.  We need it to maintain SSA form.
    llvm::AllocaInst *StackBaseMem;

    /// The iterator pointing to the stack restore cleanup.  We manually run and
    /// deactivate this cleanup after the call in the unexceptional case because
    /// it doesn't run in the normal order.
    EHScopeStack::stable_iterator StackCleanup;
  };

  /// FunctionArgList - Type for representing both the decl and type
  /// of parameters to a function. The decl must be either a
  /// ParmVarDecl or ImplicitParamDecl.
  class FunctionArgList : public SmallVector<const VarDecl*, 16> {
  };

  /// ReturnValueSlot - Contains the address where the return value of a 
  /// function can be stored, and whether the address is volatile or not.
  class ReturnValueSlot {
    llvm::PointerIntPair<llvm::Value *, 2, unsigned int> Value;

    // Return value slot flags
    enum Flags {
      IS_VOLATILE = 0x1,
      IS_UNUSED = 0x2,
    };

  public:
    ReturnValueSlot() {}
    ReturnValueSlot(llvm::Value *Value, bool IsVolatile, bool IsUnused = false)
      : Value(Value,
              (IsVolatile ? IS_VOLATILE : 0) | (IsUnused ? IS_UNUSED : 0)) {}

    bool isNull() const { return !getValue(); }

    bool isVolatile() const { return Value.getInt() & IS_VOLATILE; }
    llvm::Value *getValue() const { return Value.getPointer(); }
    bool isUnused() const { return Value.getInt() & IS_UNUSED; }
  };
  
}  // end namespace CodeGen
}  // end namespace clang

#endif
