//===--- CGAtomic.cpp - Emit LLVM IR for atomic operations ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the code for emitting atomic operations.
//
//===----------------------------------------------------------------------===//

#include "CodeGenFunction.h"
#include "CGCall.h"
#include "CGRecordLayout.h"
#include "CodeGenModule.h"
#include "clang/AST/ASTContext.h"
#include "clang/CodeGen/CGFunctionInfo.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Operator.h"

using namespace clang;
using namespace CodeGen;

namespace {
  class AtomicInfo {
    CodeGenFunction &CGF;
    QualType AtomicTy;
    QualType ValueTy;
    uint64_t AtomicSizeInBits;
    uint64_t ValueSizeInBits;
    CharUnits AtomicAlign;
    CharUnits ValueAlign;
    CharUnits LValueAlign;
    TypeEvaluationKind EvaluationKind;
    bool UseLibcall;
    LValue LVal;
    CGBitFieldInfo BFI;
  public:
    AtomicInfo(CodeGenFunction &CGF, LValue &lvalue)
        : CGF(CGF), AtomicSizeInBits(0), ValueSizeInBits(0),
          EvaluationKind(TEK_Scalar), UseLibcall(true) {
      assert(!lvalue.isGlobalReg());
      ASTContext &C = CGF.getContext();
      if (lvalue.isSimple()) {
        AtomicTy = lvalue.getType();
        if (auto *ATy = AtomicTy->getAs<AtomicType>())
          ValueTy = ATy->getValueType();
        else
          ValueTy = AtomicTy;
        EvaluationKind = CGF.getEvaluationKind(ValueTy);

        uint64_t ValueAlignInBits;
        uint64_t AtomicAlignInBits;
        TypeInfo ValueTI = C.getTypeInfo(ValueTy);
        ValueSizeInBits = ValueTI.Width;
        ValueAlignInBits = ValueTI.Align;

        TypeInfo AtomicTI = C.getTypeInfo(AtomicTy);
        AtomicSizeInBits = AtomicTI.Width;
        AtomicAlignInBits = AtomicTI.Align;

        assert(ValueSizeInBits <= AtomicSizeInBits);
        assert(ValueAlignInBits <= AtomicAlignInBits);

        AtomicAlign = C.toCharUnitsFromBits(AtomicAlignInBits);
        ValueAlign = C.toCharUnitsFromBits(ValueAlignInBits);
        if (lvalue.getAlignment().isZero())
          lvalue.setAlignment(AtomicAlign);

        LVal = lvalue;
      } else if (lvalue.isBitField()) {
        ValueTy = lvalue.getType();
        ValueSizeInBits = C.getTypeSize(ValueTy);
        auto &OrigBFI = lvalue.getBitFieldInfo();
        auto Offset = OrigBFI.Offset % C.toBits(lvalue.getAlignment());
        AtomicSizeInBits = C.toBits(
            C.toCharUnitsFromBits(Offset + OrigBFI.Size + C.getCharWidth() - 1)
                .RoundUpToAlignment(lvalue.getAlignment()));
        auto VoidPtrAddr = CGF.EmitCastToVoidPtr(lvalue.getBitFieldAddr());
        auto OffsetInChars =
            (C.toCharUnitsFromBits(OrigBFI.Offset) / lvalue.getAlignment()) *
            lvalue.getAlignment();
        VoidPtrAddr = CGF.Builder.CreateConstGEP1_64(
            VoidPtrAddr, OffsetInChars.getQuantity());
        auto Addr = CGF.Builder.CreatePointerBitCastOrAddrSpaceCast(
            VoidPtrAddr,
            CGF.Builder.getIntNTy(AtomicSizeInBits)->getPointerTo(),
            "atomic_bitfield_base");
        BFI = OrigBFI;
        BFI.Offset = Offset;
        BFI.StorageSize = AtomicSizeInBits;
        BFI.StorageOffset += OffsetInChars;
        LVal = LValue::MakeBitfield(Addr, BFI, lvalue.getType(),
                                    lvalue.getAlignment());
        LVal.setTBAAInfo(lvalue.getTBAAInfo());
        AtomicTy = C.getIntTypeForBitwidth(AtomicSizeInBits, OrigBFI.IsSigned);
        if (AtomicTy.isNull()) {
          llvm::APInt Size(
              /*numBits=*/32,
              C.toCharUnitsFromBits(AtomicSizeInBits).getQuantity());
          AtomicTy = C.getConstantArrayType(C.CharTy, Size, ArrayType::Normal,
                                            /*IndexTypeQuals=*/0);
        }
        AtomicAlign = ValueAlign = lvalue.getAlignment();
      } else if (lvalue.isVectorElt()) {
        ValueTy = lvalue.getType()->getAs<VectorType>()->getElementType();
        ValueSizeInBits = C.getTypeSize(ValueTy);
        AtomicTy = lvalue.getType();
        AtomicSizeInBits = C.getTypeSize(AtomicTy);
        AtomicAlign = ValueAlign = lvalue.getAlignment();
        LVal = lvalue;
      } else {
        assert(lvalue.isExtVectorElt());
        ValueTy = lvalue.getType();
        ValueSizeInBits = C.getTypeSize(ValueTy);
        AtomicTy = ValueTy = CGF.getContext().getExtVectorType(
            lvalue.getType(), lvalue.getExtVectorAddr()
                                  ->getType()
                                  ->getPointerElementType()
                                  ->getVectorNumElements());
        AtomicSizeInBits = C.getTypeSize(AtomicTy);
        AtomicAlign = ValueAlign = lvalue.getAlignment();
        LVal = lvalue;
      }
      UseLibcall = !C.getTargetInfo().hasBuiltinAtomic(
          AtomicSizeInBits, C.toBits(lvalue.getAlignment()));
    }

    QualType getAtomicType() const { return AtomicTy; }
    QualType getValueType() const { return ValueTy; }
    CharUnits getAtomicAlignment() const { return AtomicAlign; }
    CharUnits getValueAlignment() const { return ValueAlign; }
    uint64_t getAtomicSizeInBits() const { return AtomicSizeInBits; }
    uint64_t getValueSizeInBits() const { return ValueSizeInBits; }
    TypeEvaluationKind getEvaluationKind() const { return EvaluationKind; }
    bool shouldUseLibcall() const { return UseLibcall; }
    const LValue &getAtomicLValue() const { return LVal; }
    llvm::Value *getAtomicAddress() const {
      if (LVal.isSimple())
        return LVal.getAddress();
      else if (LVal.isBitField())
        return LVal.getBitFieldAddr();
      else if (LVal.isVectorElt())
        return LVal.getVectorAddr();
      assert(LVal.isExtVectorElt());
      return LVal.getExtVectorAddr();
    }

    /// Is the atomic size larger than the underlying value type?
    ///
    /// Note that the absence of padding does not mean that atomic
    /// objects are completely interchangeable with non-atomic
    /// objects: we might have promoted the alignment of a type
    /// without making it bigger.
    bool hasPadding() const {
      return (ValueSizeInBits != AtomicSizeInBits);
    }

    bool emitMemSetZeroIfNecessary() const;

    llvm::Value *getAtomicSizeValue() const {
      CharUnits size = CGF.getContext().toCharUnitsFromBits(AtomicSizeInBits);
      return CGF.CGM.getSize(size);
    }

    /// Cast the given pointer to an integer pointer suitable for
    /// atomic operations.
    llvm::Value *emitCastToAtomicIntPointer(llvm::Value *addr) const;

    /// Turn an atomic-layout object into an r-value.
    RValue convertTempToRValue(llvm::Value *addr, AggValueSlot resultSlot,
                               SourceLocation loc, bool AsValue) const;

    /// \brief Converts a rvalue to integer value.
    llvm::Value *convertRValueToInt(RValue RVal) const;

    RValue ConvertIntToValueOrAtomic(llvm::Value *IntVal,
                                     AggValueSlot ResultSlot,
                                     SourceLocation Loc, bool AsValue) const;

    /// Copy an atomic r-value into atomic-layout memory.
    void emitCopyIntoMemory(RValue rvalue) const;

    /// Project an l-value down to the value field.
    LValue projectValue() const {
      assert(LVal.isSimple());
      llvm::Value *addr = getAtomicAddress();
      if (hasPadding())
        addr = CGF.Builder.CreateStructGEP(nullptr, addr, 0);

      return LValue::MakeAddr(addr, getValueType(), LVal.getAlignment(),
                              CGF.getContext(), LVal.getTBAAInfo());
    }

    /// \brief Emits atomic load.
    /// \returns Loaded value.
    RValue EmitAtomicLoad(AggValueSlot ResultSlot, SourceLocation Loc,
                          bool AsValue, llvm::AtomicOrdering AO,
                          bool IsVolatile);

    /// \brief Emits atomic compare-and-exchange sequence.
    /// \param Expected Expected value.
    /// \param Desired Desired value.
    /// \param Success Atomic ordering for success operation.
    /// \param Failure Atomic ordering for failed operation.
    /// \param IsWeak true if atomic operation is weak, false otherwise.
    /// \returns Pair of values: previous value from storage (value type) and
    /// boolean flag (i1 type) with true if success and false otherwise.
    std::pair<RValue, llvm::Value *> EmitAtomicCompareExchange(
        RValue Expected, RValue Desired,
        llvm::AtomicOrdering Success = llvm::SequentiallyConsistent,
        llvm::AtomicOrdering Failure = llvm::SequentiallyConsistent,
        bool IsWeak = false);

    /// \brief Emits atomic update.
    /// \param AO Atomic ordering.
    /// \param UpdateOp Update operation for the current lvalue.
    void EmitAtomicUpdate(llvm::AtomicOrdering AO,
                          const llvm::function_ref<RValue(RValue)> &UpdateOp,
                          bool IsVolatile);
    /// \brief Emits atomic update.
    /// \param AO Atomic ordering.
    void EmitAtomicUpdate(llvm::AtomicOrdering AO, RValue UpdateRVal,
                          bool IsVolatile);

    /// Materialize an atomic r-value in atomic-layout memory.
    llvm::Value *materializeRValue(RValue rvalue) const;

    /// \brief Translates LLVM atomic ordering to GNU atomic ordering for
    /// libcalls.
    static AtomicExpr::AtomicOrderingKind
    translateAtomicOrdering(const llvm::AtomicOrdering AO);

  private:
    bool requiresMemSetZero(llvm::Type *type) const;

    /// \brief Creates temp alloca for intermediate operations on atomic value.
    llvm::Value *CreateTempAlloca() const;

    /// \brief Emits atomic load as a libcall.
    void EmitAtomicLoadLibcall(llvm::Value *AddForLoaded,
                               llvm::AtomicOrdering AO, bool IsVolatile);
    /// \brief Emits atomic load as LLVM instruction.
    llvm::Value *EmitAtomicLoadOp(llvm::AtomicOrdering AO, bool IsVolatile);
    /// \brief Emits atomic compare-and-exchange op as a libcall.
    llvm::Value *EmitAtomicCompareExchangeLibcall(
        llvm::Value *ExpectedAddr, llvm::Value *DesiredAddr,
        llvm::AtomicOrdering Success = llvm::SequentiallyConsistent,
        llvm::AtomicOrdering Failure = llvm::SequentiallyConsistent);
    /// \brief Emits atomic compare-and-exchange op as LLVM instruction.
    std::pair<llvm::Value *, llvm::Value *> EmitAtomicCompareExchangeOp(
        llvm::Value *ExpectedVal, llvm::Value *DesiredVal,
        llvm::AtomicOrdering Success = llvm::SequentiallyConsistent,
        llvm::AtomicOrdering Failure = llvm::SequentiallyConsistent,
        bool IsWeak = false);
    /// \brief Emit atomic update as libcalls.
    void
    EmitAtomicUpdateLibcall(llvm::AtomicOrdering AO,
                            const llvm::function_ref<RValue(RValue)> &UpdateOp,
                            bool IsVolatile);
    /// \brief Emit atomic update as LLVM instructions.
    void EmitAtomicUpdateOp(llvm::AtomicOrdering AO,
                            const llvm::function_ref<RValue(RValue)> &UpdateOp,
                            bool IsVolatile);
    /// \brief Emit atomic update as libcalls.
    void EmitAtomicUpdateLibcall(llvm::AtomicOrdering AO, RValue UpdateRVal,
                                 bool IsVolatile);
    /// \brief Emit atomic update as LLVM instructions.
    void EmitAtomicUpdateOp(llvm::AtomicOrdering AO, RValue UpdateRal,
                            bool IsVolatile);
  };
}

AtomicExpr::AtomicOrderingKind
AtomicInfo::translateAtomicOrdering(const llvm::AtomicOrdering AO) {
  switch (AO) {
  case llvm::Unordered:
  case llvm::NotAtomic:
  case llvm::Monotonic:
    return AtomicExpr::AO_ABI_memory_order_relaxed;
  case llvm::Acquire:
    return AtomicExpr::AO_ABI_memory_order_acquire;
  case llvm::Release:
    return AtomicExpr::AO_ABI_memory_order_release;
  case llvm::AcquireRelease:
    return AtomicExpr::AO_ABI_memory_order_acq_rel;
  case llvm::SequentiallyConsistent:
    return AtomicExpr::AO_ABI_memory_order_seq_cst;
  }
  llvm_unreachable("Unhandled AtomicOrdering");
}

llvm::Value *AtomicInfo::CreateTempAlloca() const {
  auto *TempAlloca = CGF.CreateMemTemp(
      (LVal.isBitField() && ValueSizeInBits > AtomicSizeInBits) ? ValueTy
                                                                : AtomicTy,
      "atomic-temp");
  TempAlloca->setAlignment(getAtomicAlignment().getQuantity());
  // Cast to pointer to value type for bitfields.
  if (LVal.isBitField())
    return CGF.Builder.CreatePointerBitCastOrAddrSpaceCast(
        TempAlloca, getAtomicAddress()->getType());
  return TempAlloca;
}

static RValue emitAtomicLibcall(CodeGenFunction &CGF,
                                StringRef fnName,
                                QualType resultType,
                                CallArgList &args) {
  const CGFunctionInfo &fnInfo =
    CGF.CGM.getTypes().arrangeFreeFunctionCall(resultType, args,
            FunctionType::ExtInfo(), RequiredArgs::All);
  llvm::FunctionType *fnTy = CGF.CGM.getTypes().GetFunctionType(fnInfo);
  llvm::Constant *fn = CGF.CGM.CreateRuntimeFunction(fnTy, fnName);
  return CGF.EmitCall(fnInfo, fn, ReturnValueSlot(), args);
}

/// Does a store of the given IR type modify the full expected width?
static bool isFullSizeType(CodeGenModule &CGM, llvm::Type *type,
                           uint64_t expectedSize) {
  return (CGM.getDataLayout().getTypeStoreSize(type) * 8 == expectedSize);
}

/// Does the atomic type require memsetting to zero before initialization?
///
/// The IR type is provided as a way of making certain queries faster.
bool AtomicInfo::requiresMemSetZero(llvm::Type *type) const {
  // If the atomic type has size padding, we definitely need a memset.
  if (hasPadding()) return true;

  // Otherwise, do some simple heuristics to try to avoid it:
  switch (getEvaluationKind()) {
  // For scalars and complexes, check whether the store size of the
  // type uses the full size.
  case TEK_Scalar:
    return !isFullSizeType(CGF.CGM, type, AtomicSizeInBits);
  case TEK_Complex:
    return !isFullSizeType(CGF.CGM, type->getStructElementType(0),
                           AtomicSizeInBits / 2);

  // Padding in structs has an undefined bit pattern.  User beware.
  case TEK_Aggregate:
    return false;
  }
  llvm_unreachable("bad evaluation kind");
}

bool AtomicInfo::emitMemSetZeroIfNecessary() const {
  assert(LVal.isSimple());
  llvm::Value *addr = LVal.getAddress();
  if (!requiresMemSetZero(addr->getType()->getPointerElementType()))
    return false;

  CGF.Builder.CreateMemSet(
      addr, llvm::ConstantInt::get(CGF.Int8Ty, 0),
      CGF.getContext().toCharUnitsFromBits(AtomicSizeInBits).getQuantity(),
      LVal.getAlignment().getQuantity());
  return true;
}

static void emitAtomicCmpXchg(CodeGenFunction &CGF, AtomicExpr *E, bool IsWeak,
                              llvm::Value *Dest, llvm::Value *Ptr,
                              llvm::Value *Val1, llvm::Value *Val2,
                              uint64_t Size, unsigned Align,
                              llvm::AtomicOrdering SuccessOrder,
                              llvm::AtomicOrdering FailureOrder) {
  // Note that cmpxchg doesn't support weak cmpxchg, at least at the moment.
  llvm::LoadInst *Expected = CGF.Builder.CreateLoad(Val1);
  Expected->setAlignment(Align);
  llvm::LoadInst *Desired = CGF.Builder.CreateLoad(Val2);
  Desired->setAlignment(Align);

  llvm::AtomicCmpXchgInst *Pair = CGF.Builder.CreateAtomicCmpXchg(
      Ptr, Expected, Desired, SuccessOrder, FailureOrder);
  Pair->setVolatile(E->isVolatile());
  Pair->setWeak(IsWeak);

  // Cmp holds the result of the compare-exchange operation: true on success,
  // false on failure.
  llvm::Value *Old = CGF.Builder.CreateExtractValue(Pair, 0);
  llvm::Value *Cmp = CGF.Builder.CreateExtractValue(Pair, 1);

  // This basic block is used to hold the store instruction if the operation
  // failed.
  llvm::BasicBlock *StoreExpectedBB =
      CGF.createBasicBlock("cmpxchg.store_expected", CGF.CurFn);

  // This basic block is the exit point of the operation, we should end up
  // here regardless of whether or not the operation succeeded.
  llvm::BasicBlock *ContinueBB =
      CGF.createBasicBlock("cmpxchg.continue", CGF.CurFn);

  // Update Expected if Expected isn't equal to Old, otherwise branch to the
  // exit point.
  CGF.Builder.CreateCondBr(Cmp, ContinueBB, StoreExpectedBB);

  CGF.Builder.SetInsertPoint(StoreExpectedBB);
  // Update the memory at Expected with Old's value.
  llvm::StoreInst *StoreExpected = CGF.Builder.CreateStore(Old, Val1);
  StoreExpected->setAlignment(Align);
  // Finally, branch to the exit point.
  CGF.Builder.CreateBr(ContinueBB);

  CGF.Builder.SetInsertPoint(ContinueBB);
  // Update the memory at Dest with Cmp's value.
  CGF.EmitStoreOfScalar(Cmp, CGF.MakeAddrLValue(Dest, E->getType()));
  return;
}

/// Given an ordering required on success, emit all possible cmpxchg
/// instructions to cope with the provided (but possibly only dynamically known)
/// FailureOrder.
static void emitAtomicCmpXchgFailureSet(CodeGenFunction &CGF, AtomicExpr *E,
                                        bool IsWeak, llvm::Value *Dest,
                                        llvm::Value *Ptr, llvm::Value *Val1,
                                        llvm::Value *Val2,
                                        llvm::Value *FailureOrderVal,
                                        uint64_t Size, unsigned Align,
                                        llvm::AtomicOrdering SuccessOrder) {
  llvm::AtomicOrdering FailureOrder;
  if (llvm::ConstantInt *FO = dyn_cast<llvm::ConstantInt>(FailureOrderVal)) {
    switch (FO->getSExtValue()) {
    default:
      FailureOrder = llvm::Monotonic;
      break;
    case AtomicExpr::AO_ABI_memory_order_consume:
    case AtomicExpr::AO_ABI_memory_order_acquire:
      FailureOrder = llvm::Acquire;
      break;
    case AtomicExpr::AO_ABI_memory_order_seq_cst:
      FailureOrder = llvm::SequentiallyConsistent;
      break;
    }
    if (FailureOrder >= SuccessOrder) {
      // Don't assert on undefined behaviour.
      FailureOrder =
        llvm::AtomicCmpXchgInst::getStrongestFailureOrdering(SuccessOrder);
    }
    emitAtomicCmpXchg(CGF, E, IsWeak, Dest, Ptr, Val1, Val2, Size, Align,
                      SuccessOrder, FailureOrder);
    return;
  }

  // Create all the relevant BB's
  llvm::BasicBlock *MonotonicBB = nullptr, *AcquireBB = nullptr,
                   *SeqCstBB = nullptr;
  MonotonicBB = CGF.createBasicBlock("monotonic_fail", CGF.CurFn);
  if (SuccessOrder != llvm::Monotonic && SuccessOrder != llvm::Release)
    AcquireBB = CGF.createBasicBlock("acquire_fail", CGF.CurFn);
  if (SuccessOrder == llvm::SequentiallyConsistent)
    SeqCstBB = CGF.createBasicBlock("seqcst_fail", CGF.CurFn);

  llvm::BasicBlock *ContBB = CGF.createBasicBlock("atomic.continue", CGF.CurFn);

  llvm::SwitchInst *SI = CGF.Builder.CreateSwitch(FailureOrderVal, MonotonicBB);

  // Emit all the different atomics

  // MonotonicBB is arbitrarily chosen as the default case; in practice, this
  // doesn't matter unless someone is crazy enough to use something that
  // doesn't fold to a constant for the ordering.
  CGF.Builder.SetInsertPoint(MonotonicBB);
  emitAtomicCmpXchg(CGF, E, IsWeak, Dest, Ptr, Val1, Val2,
                    Size, Align, SuccessOrder, llvm::Monotonic);
  CGF.Builder.CreateBr(ContBB);

  if (AcquireBB) {
    CGF.Builder.SetInsertPoint(AcquireBB);
    emitAtomicCmpXchg(CGF, E, IsWeak, Dest, Ptr, Val1, Val2,
                      Size, Align, SuccessOrder, llvm::Acquire);
    CGF.Builder.CreateBr(ContBB);
    SI->addCase(CGF.Builder.getInt32(AtomicExpr::AO_ABI_memory_order_consume),
                AcquireBB);
    SI->addCase(CGF.Builder.getInt32(AtomicExpr::AO_ABI_memory_order_acquire),
                AcquireBB);
  }
  if (SeqCstBB) {
    CGF.Builder.SetInsertPoint(SeqCstBB);
    emitAtomicCmpXchg(CGF, E, IsWeak, Dest, Ptr, Val1, Val2,
                      Size, Align, SuccessOrder, llvm::SequentiallyConsistent);
    CGF.Builder.CreateBr(ContBB);
    SI->addCase(CGF.Builder.getInt32(AtomicExpr::AO_ABI_memory_order_seq_cst),
                SeqCstBB);
  }

  CGF.Builder.SetInsertPoint(ContBB);
}

static void EmitAtomicOp(CodeGenFunction &CGF, AtomicExpr *E, llvm::Value *Dest,
                         llvm::Value *Ptr, llvm::Value *Val1, llvm::Value *Val2,
                         llvm::Value *IsWeak, llvm::Value *FailureOrder,
                         uint64_t Size, unsigned Align,
                         llvm::AtomicOrdering Order) {
  llvm::AtomicRMWInst::BinOp Op = llvm::AtomicRMWInst::Add;
  llvm::Instruction::BinaryOps PostOp = (llvm::Instruction::BinaryOps)0;

  switch (E->getOp()) {
  case AtomicExpr::AO__c11_atomic_init:
    llvm_unreachable("Already handled!");

  case AtomicExpr::AO__c11_atomic_compare_exchange_strong:
    emitAtomicCmpXchgFailureSet(CGF, E, false, Dest, Ptr, Val1, Val2,
                                FailureOrder, Size, Align, Order);
    return;
  case AtomicExpr::AO__c11_atomic_compare_exchange_weak:
    emitAtomicCmpXchgFailureSet(CGF, E, true, Dest, Ptr, Val1, Val2,
                                FailureOrder, Size, Align, Order);
    return;
  case AtomicExpr::AO__atomic_compare_exchange:
  case AtomicExpr::AO__atomic_compare_exchange_n: {
    if (llvm::ConstantInt *IsWeakC = dyn_cast<llvm::ConstantInt>(IsWeak)) {
      emitAtomicCmpXchgFailureSet(CGF, E, IsWeakC->getZExtValue(), Dest, Ptr,
                                  Val1, Val2, FailureOrder, Size, Align, Order);
    } else {
      // Create all the relevant BB's
      llvm::BasicBlock *StrongBB =
          CGF.createBasicBlock("cmpxchg.strong", CGF.CurFn);
      llvm::BasicBlock *WeakBB = CGF.createBasicBlock("cmxchg.weak", CGF.CurFn);
      llvm::BasicBlock *ContBB =
          CGF.createBasicBlock("cmpxchg.continue", CGF.CurFn);

      llvm::SwitchInst *SI = CGF.Builder.CreateSwitch(IsWeak, WeakBB);
      SI->addCase(CGF.Builder.getInt1(false), StrongBB);

      CGF.Builder.SetInsertPoint(StrongBB);
      emitAtomicCmpXchgFailureSet(CGF, E, false, Dest, Ptr, Val1, Val2,
                                  FailureOrder, Size, Align, Order);
      CGF.Builder.CreateBr(ContBB);

      CGF.Builder.SetInsertPoint(WeakBB);
      emitAtomicCmpXchgFailureSet(CGF, E, true, Dest, Ptr, Val1, Val2,
                                  FailureOrder, Size, Align, Order);
      CGF.Builder.CreateBr(ContBB);

      CGF.Builder.SetInsertPoint(ContBB);
    }
    return;
  }
  case AtomicExpr::AO__c11_atomic_load:
  case AtomicExpr::AO__atomic_load_n:
  case AtomicExpr::AO__atomic_load: {
    llvm::LoadInst *Load = CGF.Builder.CreateLoad(Ptr);
    Load->setAtomic(Order);
    Load->setAlignment(Size);
    Load->setVolatile(E->isVolatile());
    llvm::StoreInst *StoreDest = CGF.Builder.CreateStore(Load, Dest);
    StoreDest->setAlignment(Align);
    return;
  }

  case AtomicExpr::AO__c11_atomic_store:
  case AtomicExpr::AO__atomic_store:
  case AtomicExpr::AO__atomic_store_n: {
    assert(!Dest && "Store does not return a value");
    llvm::LoadInst *LoadVal1 = CGF.Builder.CreateLoad(Val1);
    LoadVal1->setAlignment(Align);
    llvm::StoreInst *Store = CGF.Builder.CreateStore(LoadVal1, Ptr);
    Store->setAtomic(Order);
    Store->setAlignment(Size);
    Store->setVolatile(E->isVolatile());
    return;
  }

  case AtomicExpr::AO__c11_atomic_exchange:
  case AtomicExpr::AO__atomic_exchange_n:
  case AtomicExpr::AO__atomic_exchange:
    Op = llvm::AtomicRMWInst::Xchg;
    break;

  case AtomicExpr::AO__atomic_add_fetch:
    PostOp = llvm::Instruction::Add;
    LLVM_FALLTHROUGH; // HLSL Change
  case AtomicExpr::AO__c11_atomic_fetch_add:
  case AtomicExpr::AO__atomic_fetch_add:
    Op = llvm::AtomicRMWInst::Add;
    break;

  case AtomicExpr::AO__atomic_sub_fetch:
    PostOp = llvm::Instruction::Sub;
    LLVM_FALLTHROUGH; // HLSL Change
  case AtomicExpr::AO__c11_atomic_fetch_sub:
  case AtomicExpr::AO__atomic_fetch_sub:
    Op = llvm::AtomicRMWInst::Sub;
    break;

  case AtomicExpr::AO__atomic_and_fetch:
    PostOp = llvm::Instruction::And;
    LLVM_FALLTHROUGH; // HLSL Change
  case AtomicExpr::AO__c11_atomic_fetch_and:
  case AtomicExpr::AO__atomic_fetch_and:
    Op = llvm::AtomicRMWInst::And;
    break;

  case AtomicExpr::AO__atomic_or_fetch:
    PostOp = llvm::Instruction::Or;
    LLVM_FALLTHROUGH; // HLSL Change
  case AtomicExpr::AO__c11_atomic_fetch_or:
  case AtomicExpr::AO__atomic_fetch_or:
    Op = llvm::AtomicRMWInst::Or;
    break;

  case AtomicExpr::AO__atomic_xor_fetch:
    PostOp = llvm::Instruction::Xor;
    LLVM_FALLTHROUGH; // HLSL Change
  case AtomicExpr::AO__c11_atomic_fetch_xor:
  case AtomicExpr::AO__atomic_fetch_xor:
    Op = llvm::AtomicRMWInst::Xor;
    break;

  case AtomicExpr::AO__atomic_nand_fetch:
    PostOp = llvm::Instruction::And;
    LLVM_FALLTHROUGH; // HLSL Change
  case AtomicExpr::AO__atomic_fetch_nand:
    Op = llvm::AtomicRMWInst::Nand;
    break;
  }

  llvm::LoadInst *LoadVal1 = CGF.Builder.CreateLoad(Val1);
  LoadVal1->setAlignment(Align);
  llvm::AtomicRMWInst *RMWI =
      CGF.Builder.CreateAtomicRMW(Op, Ptr, LoadVal1, Order);
  RMWI->setVolatile(E->isVolatile());

  // For __atomic_*_fetch operations, perform the operation again to
  // determine the value which was written.
  llvm::Value *Result = RMWI;
  if (PostOp)
    Result = CGF.Builder.CreateBinOp(PostOp, RMWI, LoadVal1);
  if (E->getOp() == AtomicExpr::AO__atomic_nand_fetch)
    Result = CGF.Builder.CreateNot(Result);
  llvm::StoreInst *StoreDest = CGF.Builder.CreateStore(Result, Dest);
  StoreDest->setAlignment(Align);
}

// This function emits any expression (scalar, complex, or aggregate)
// into a temporary alloca.
static llvm::Value *
EmitValToTemp(CodeGenFunction &CGF, Expr *E) {
  llvm::Value *DeclPtr = CGF.CreateMemTemp(E->getType(), ".atomictmp");
  CGF.EmitAnyExprToMem(E, DeclPtr, E->getType().getQualifiers(),
                       /*Init*/ true);
  return DeclPtr;
}

static void
AddDirectArgument(CodeGenFunction &CGF, CallArgList &Args,
                  bool UseOptimizedLibcall, llvm::Value *Val, QualType ValTy,
                  SourceLocation Loc, CharUnits SizeInChars) {
  if (UseOptimizedLibcall) {
    // Load value and pass it to the function directly.
    unsigned Align = CGF.getContext().getTypeAlignInChars(ValTy).getQuantity();
    int64_t SizeInBits = CGF.getContext().toBits(SizeInChars);
    ValTy =
        CGF.getContext().getIntTypeForBitwidth(SizeInBits, /*Signed=*/false);
    llvm::Type *IPtrTy = llvm::IntegerType::get(CGF.getLLVMContext(),
                                                SizeInBits)->getPointerTo();
    Val = CGF.EmitLoadOfScalar(CGF.Builder.CreateBitCast(Val, IPtrTy), false,
                               Align, CGF.getContext().getPointerType(ValTy),
                               Loc);
    // Coerce the value into an appropriately sized integer type.
    Args.add(RValue::get(Val), ValTy);
  } else {
    // Non-optimized functions always take a reference.
    Args.add(RValue::get(CGF.EmitCastToVoidPtr(Val)),
                         CGF.getContext().VoidPtrTy);
  }
}

RValue CodeGenFunction::EmitAtomicExpr(AtomicExpr *E, llvm::Value *Dest) {
  QualType AtomicTy = E->getPtr()->getType()->getPointeeType();
  QualType MemTy = AtomicTy;
  if (const AtomicType *AT = AtomicTy->getAs<AtomicType>())
    MemTy = AT->getValueType();
  CharUnits sizeChars = getContext().getTypeSizeInChars(AtomicTy);
  uint64_t Size = sizeChars.getQuantity();
  CharUnits alignChars = getContext().getTypeAlignInChars(AtomicTy);
  unsigned Align = alignChars.getQuantity();
  unsigned MaxInlineWidthInBits =
    getTarget().getMaxAtomicInlineWidth();
  bool UseLibcall = (Size != Align ||
                     getContext().toBits(sizeChars) > MaxInlineWidthInBits);

  llvm::Value *IsWeak = nullptr, *OrderFail = nullptr, *Val1 = nullptr,
              *Val2 = nullptr;
  llvm::Value *Ptr = EmitScalarExpr(E->getPtr());

  if (E->getOp() == AtomicExpr::AO__c11_atomic_init) {
    assert(!Dest && "Init does not return a value");
    LValue lvalue = LValue::MakeAddr(Ptr, AtomicTy, alignChars, getContext());
    EmitAtomicInit(E->getVal1(), lvalue);
    return RValue::get(nullptr);
  }

  llvm::Value *Order = EmitScalarExpr(E->getOrder());

  switch (E->getOp()) {
  case AtomicExpr::AO__c11_atomic_init:
    llvm_unreachable("Already handled!");

  case AtomicExpr::AO__c11_atomic_load:
  case AtomicExpr::AO__atomic_load_n:
    break;

  case AtomicExpr::AO__atomic_load:
    Dest = EmitScalarExpr(E->getVal1());
    break;

  case AtomicExpr::AO__atomic_store:
    Val1 = EmitScalarExpr(E->getVal1());
    break;

  case AtomicExpr::AO__atomic_exchange:
    Val1 = EmitScalarExpr(E->getVal1());
    Dest = EmitScalarExpr(E->getVal2());
    break;

  case AtomicExpr::AO__c11_atomic_compare_exchange_strong:
  case AtomicExpr::AO__c11_atomic_compare_exchange_weak:
  case AtomicExpr::AO__atomic_compare_exchange_n:
  case AtomicExpr::AO__atomic_compare_exchange:
    Val1 = EmitScalarExpr(E->getVal1());
    if (E->getOp() == AtomicExpr::AO__atomic_compare_exchange)
      Val2 = EmitScalarExpr(E->getVal2());
    else
      Val2 = EmitValToTemp(*this, E->getVal2());
    OrderFail = EmitScalarExpr(E->getOrderFail());
    if (E->getNumSubExprs() == 6)
      IsWeak = EmitScalarExpr(E->getWeak());
    break;

  case AtomicExpr::AO__c11_atomic_fetch_add:
  case AtomicExpr::AO__c11_atomic_fetch_sub:
    if (MemTy->isPointerType()) {
      // For pointer arithmetic, we're required to do a bit of math:
      // adding 1 to an int* is not the same as adding 1 to a uintptr_t.
      // ... but only for the C11 builtins. The GNU builtins expect the
      // user to multiply by sizeof(T).
      QualType Val1Ty = E->getVal1()->getType();
      llvm::Value *Val1Scalar = EmitScalarExpr(E->getVal1());
      CharUnits PointeeIncAmt =
          getContext().getTypeSizeInChars(MemTy->getPointeeType());
      Val1Scalar = Builder.CreateMul(Val1Scalar, CGM.getSize(PointeeIncAmt));
      Val1 = CreateMemTemp(Val1Ty, ".atomictmp");
      EmitStoreOfScalar(Val1Scalar, MakeAddrLValue(Val1, Val1Ty));
      break;
    }
    LLVM_FALLTHROUGH; // HLSL Change
  case AtomicExpr::AO__atomic_fetch_add:
  case AtomicExpr::AO__atomic_fetch_sub:
  case AtomicExpr::AO__atomic_add_fetch:
  case AtomicExpr::AO__atomic_sub_fetch:
  case AtomicExpr::AO__c11_atomic_store:
  case AtomicExpr::AO__c11_atomic_exchange:
  case AtomicExpr::AO__atomic_store_n:
  case AtomicExpr::AO__atomic_exchange_n:
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
    Val1 = EmitValToTemp(*this, E->getVal1());
    break;
  }

  QualType RValTy = E->getType().getUnqualifiedType();

  auto GetDest = [&] {
    if (!RValTy->isVoidType() && !Dest) {
      Dest = CreateMemTemp(RValTy, ".atomicdst");
    }
    return Dest;
  };

  // Use a library call.  See: http://gcc.gnu.org/wiki/Atomic/GCCMM/LIbrary .
  if (UseLibcall) {
    bool UseOptimizedLibcall = false;
    switch (E->getOp()) {
    case AtomicExpr::AO__c11_atomic_fetch_add:
    case AtomicExpr::AO__atomic_fetch_add:
    case AtomicExpr::AO__c11_atomic_fetch_and:
    case AtomicExpr::AO__atomic_fetch_and:
    case AtomicExpr::AO__c11_atomic_fetch_or:
    case AtomicExpr::AO__atomic_fetch_or:
    case AtomicExpr::AO__c11_atomic_fetch_sub:
    case AtomicExpr::AO__atomic_fetch_sub:
    case AtomicExpr::AO__c11_atomic_fetch_xor:
    case AtomicExpr::AO__atomic_fetch_xor:
      // For these, only library calls for certain sizes exist.
      UseOptimizedLibcall = true;
      break;
    default:
      // Only use optimized library calls for sizes for which they exist.
      if (Size == 1 || Size == 2 || Size == 4 || Size == 8)
        UseOptimizedLibcall = true;
      break;
    }

    CallArgList Args;
    if (!UseOptimizedLibcall) {
      // For non-optimized library calls, the size is the first parameter
      Args.add(RValue::get(llvm::ConstantInt::get(SizeTy, Size)),
               getContext().getSizeType());
    }
    // Atomic address is the first or second parameter
    Args.add(RValue::get(EmitCastToVoidPtr(Ptr)), getContext().VoidPtrTy);

    std::string LibCallName;
    QualType LoweredMemTy =
      MemTy->isPointerType() ? getContext().getIntPtrType() : MemTy;
    QualType RetTy;
    bool HaveRetTy = false;
    switch (E->getOp()) {
    // There is only one libcall for compare an exchange, because there is no
    // optimisation benefit possible from a libcall version of a weak compare
    // and exchange.
    // bool __atomic_compare_exchange(size_t size, void *mem, void *expected,
    //                                void *desired, int success, int failure)
    // bool __atomic_compare_exchange_N(T *mem, T *expected, T desired,
    //                                  int success, int failure)
    case AtomicExpr::AO__c11_atomic_compare_exchange_weak:
    case AtomicExpr::AO__c11_atomic_compare_exchange_strong:
    case AtomicExpr::AO__atomic_compare_exchange:
    case AtomicExpr::AO__atomic_compare_exchange_n:
      LibCallName = "__atomic_compare_exchange";
      RetTy = getContext().BoolTy;
      HaveRetTy = true;
      Args.add(RValue::get(EmitCastToVoidPtr(Val1)), getContext().VoidPtrTy);
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val2, MemTy,
                        E->getExprLoc(), sizeChars);
      Args.add(RValue::get(Order), getContext().IntTy);
      Order = OrderFail;
      break;
    // void __atomic_exchange(size_t size, void *mem, void *val, void *return,
    //                        int order)
    // T __atomic_exchange_N(T *mem, T val, int order)
    case AtomicExpr::AO__c11_atomic_exchange:
    case AtomicExpr::AO__atomic_exchange_n:
    case AtomicExpr::AO__atomic_exchange:
      LibCallName = "__atomic_exchange";
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val1, MemTy,
                        E->getExprLoc(), sizeChars);
      break;
    // void __atomic_store(size_t size, void *mem, void *val, int order)
    // void __atomic_store_N(T *mem, T val, int order)
    case AtomicExpr::AO__c11_atomic_store:
    case AtomicExpr::AO__atomic_store:
    case AtomicExpr::AO__atomic_store_n:
      LibCallName = "__atomic_store";
      RetTy = getContext().VoidTy;
      HaveRetTy = true;
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val1, MemTy,
                        E->getExprLoc(), sizeChars);
      break;
    // void __atomic_load(size_t size, void *mem, void *return, int order)
    // T __atomic_load_N(T *mem, int order)
    case AtomicExpr::AO__c11_atomic_load:
    case AtomicExpr::AO__atomic_load:
    case AtomicExpr::AO__atomic_load_n:
      LibCallName = "__atomic_load";
      break;
    // T __atomic_fetch_add_N(T *mem, T val, int order)
    case AtomicExpr::AO__c11_atomic_fetch_add:
    case AtomicExpr::AO__atomic_fetch_add:
      LibCallName = "__atomic_fetch_add";
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val1, LoweredMemTy,
                        E->getExprLoc(), sizeChars);
      break;
    // T __atomic_fetch_and_N(T *mem, T val, int order)
    case AtomicExpr::AO__c11_atomic_fetch_and:
    case AtomicExpr::AO__atomic_fetch_and:
      LibCallName = "__atomic_fetch_and";
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val1, MemTy,
                        E->getExprLoc(), sizeChars);
      break;
    // T __atomic_fetch_or_N(T *mem, T val, int order)
    case AtomicExpr::AO__c11_atomic_fetch_or:
    case AtomicExpr::AO__atomic_fetch_or:
      LibCallName = "__atomic_fetch_or";
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val1, MemTy,
                        E->getExprLoc(), sizeChars);
      break;
    // T __atomic_fetch_sub_N(T *mem, T val, int order)
    case AtomicExpr::AO__c11_atomic_fetch_sub:
    case AtomicExpr::AO__atomic_fetch_sub:
      LibCallName = "__atomic_fetch_sub";
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val1, LoweredMemTy,
                        E->getExprLoc(), sizeChars);
      break;
    // T __atomic_fetch_xor_N(T *mem, T val, int order)
    case AtomicExpr::AO__c11_atomic_fetch_xor:
    case AtomicExpr::AO__atomic_fetch_xor:
      LibCallName = "__atomic_fetch_xor";
      AddDirectArgument(*this, Args, UseOptimizedLibcall, Val1, MemTy,
                        E->getExprLoc(), sizeChars);
      break;
    default: return EmitUnsupportedRValue(E, "atomic library call");
    }

    // Optimized functions have the size in their name.
    if (UseOptimizedLibcall)
      LibCallName += "_" + llvm::utostr(Size);
    // By default, assume we return a value of the atomic type.
    if (!HaveRetTy) {
      if (UseOptimizedLibcall) {
        // Value is returned directly.
        // The function returns an appropriately sized integer type.
        RetTy = getContext().getIntTypeForBitwidth(
            getContext().toBits(sizeChars), /*Signed=*/false);
      } else {
        // Value is returned through parameter before the order.
        RetTy = getContext().VoidTy;
        Args.add(RValue::get(EmitCastToVoidPtr(Dest)), getContext().VoidPtrTy);
      }
    }
    // order is always the last parameter
    Args.add(RValue::get(Order),
             getContext().IntTy);

    RValue Res = emitAtomicLibcall(*this, LibCallName, RetTy, Args);
    // The value is returned directly from the libcall.
    if (HaveRetTy && !RetTy->isVoidType())
      return Res;
    // The value is returned via an explicit out param.
    if (RetTy->isVoidType())
      return RValue::get(nullptr);
    // The value is returned directly for optimized libcalls but the caller is
    // expected an out-param.
    if (UseOptimizedLibcall) {
      llvm::Value *ResVal = Res.getScalarVal();
      llvm::StoreInst *StoreDest = Builder.CreateStore(
          ResVal,
          Builder.CreateBitCast(GetDest(), ResVal->getType()->getPointerTo()));
      StoreDest->setAlignment(Align);
    }
    return convertTempToRValue(Dest, RValTy, E->getExprLoc());
  }

  bool IsStore = E->getOp() == AtomicExpr::AO__c11_atomic_store ||
                 E->getOp() == AtomicExpr::AO__atomic_store ||
                 E->getOp() == AtomicExpr::AO__atomic_store_n;
  bool IsLoad = E->getOp() == AtomicExpr::AO__c11_atomic_load ||
                E->getOp() == AtomicExpr::AO__atomic_load ||
                E->getOp() == AtomicExpr::AO__atomic_load_n;

  llvm::Type *ITy =
      llvm::IntegerType::get(getLLVMContext(), Size * 8);
  llvm::Value *OrigDest = GetDest();
  Ptr = Builder.CreateBitCast(
      Ptr, ITy->getPointerTo(Ptr->getType()->getPointerAddressSpace()));
  if (Val1) Val1 = Builder.CreateBitCast(Val1, ITy->getPointerTo());
  if (Val2) Val2 = Builder.CreateBitCast(Val2, ITy->getPointerTo());
  if (Dest && !E->isCmpXChg())
    Dest = Builder.CreateBitCast(Dest, ITy->getPointerTo());

  if (isa<llvm::ConstantInt>(Order)) {
    int ord = cast<llvm::ConstantInt>(Order)->getZExtValue();
    switch (ord) {
    case AtomicExpr::AO_ABI_memory_order_relaxed:
      EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail,
                   Size, Align, llvm::Monotonic);
      break;
    case AtomicExpr::AO_ABI_memory_order_consume:
    case AtomicExpr::AO_ABI_memory_order_acquire:
      if (IsStore)
        break; // Avoid crashing on code with undefined behavior
      EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail,
                   Size, Align, llvm::Acquire);
      break;
    case AtomicExpr::AO_ABI_memory_order_release:
      if (IsLoad)
        break; // Avoid crashing on code with undefined behavior
      EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail,
                   Size, Align, llvm::Release);
      break;
    case AtomicExpr::AO_ABI_memory_order_acq_rel:
      if (IsLoad || IsStore)
        break; // Avoid crashing on code with undefined behavior
      EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail,
                   Size, Align, llvm::AcquireRelease);
      break;
    case AtomicExpr::AO_ABI_memory_order_seq_cst:
      EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail,
                   Size, Align, llvm::SequentiallyConsistent);
      break;
    default: // invalid order
      // We should not ever get here normally, but it's hard to
      // enforce that in general.
      break;
    }
    if (RValTy->isVoidType())
      return RValue::get(nullptr);
    return convertTempToRValue(OrigDest, RValTy, E->getExprLoc());
  }

  // Long case, when Order isn't obviously constant.

  // Create all the relevant BB's
  llvm::BasicBlock *MonotonicBB = nullptr, *AcquireBB = nullptr,
                   *ReleaseBB = nullptr, *AcqRelBB = nullptr,
                   *SeqCstBB = nullptr;
  MonotonicBB = createBasicBlock("monotonic", CurFn);
  if (!IsStore)
    AcquireBB = createBasicBlock("acquire", CurFn);
  if (!IsLoad)
    ReleaseBB = createBasicBlock("release", CurFn);
  if (!IsLoad && !IsStore)
    AcqRelBB = createBasicBlock("acqrel", CurFn);
  SeqCstBB = createBasicBlock("seqcst", CurFn);
  llvm::BasicBlock *ContBB = createBasicBlock("atomic.continue", CurFn);

  // Create the switch for the split
  // MonotonicBB is arbitrarily chosen as the default case; in practice, this
  // doesn't matter unless someone is crazy enough to use something that
  // doesn't fold to a constant for the ordering.
  Order = Builder.CreateIntCast(Order, Builder.getInt32Ty(), false);
  llvm::SwitchInst *SI = Builder.CreateSwitch(Order, MonotonicBB);

  // Emit all the different atomics
  Builder.SetInsertPoint(MonotonicBB);
  EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail,
               Size, Align, llvm::Monotonic);
  Builder.CreateBr(ContBB);
  if (!IsStore) {
    Builder.SetInsertPoint(AcquireBB);
    EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail,
                 Size, Align, llvm::Acquire);
    Builder.CreateBr(ContBB);
    SI->addCase(Builder.getInt32(AtomicExpr::AO_ABI_memory_order_consume),
                AcquireBB);
    SI->addCase(Builder.getInt32(AtomicExpr::AO_ABI_memory_order_acquire),
                AcquireBB);
  }
  if (!IsLoad) {
    Builder.SetInsertPoint(ReleaseBB);
    EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail,
                 Size, Align, llvm::Release);
    Builder.CreateBr(ContBB);
    SI->addCase(Builder.getInt32(AtomicExpr::AO_ABI_memory_order_release),
                ReleaseBB);
  }
  if (!IsLoad && !IsStore) {
    Builder.SetInsertPoint(AcqRelBB);
    EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail,
                 Size, Align, llvm::AcquireRelease);
    Builder.CreateBr(ContBB);
    SI->addCase(Builder.getInt32(AtomicExpr::AO_ABI_memory_order_acq_rel),
                AcqRelBB);
  }
  Builder.SetInsertPoint(SeqCstBB);
  EmitAtomicOp(*this, E, Dest, Ptr, Val1, Val2, IsWeak, OrderFail,
               Size, Align, llvm::SequentiallyConsistent);
  Builder.CreateBr(ContBB);
  SI->addCase(Builder.getInt32(AtomicExpr::AO_ABI_memory_order_seq_cst),
              SeqCstBB);

  // Cleanup and return
  Builder.SetInsertPoint(ContBB);
  if (RValTy->isVoidType())
    return RValue::get(nullptr);
  return convertTempToRValue(OrigDest, RValTy, E->getExprLoc());
}

llvm::Value *AtomicInfo::emitCastToAtomicIntPointer(llvm::Value *addr) const {
  unsigned addrspace =
    cast<llvm::PointerType>(addr->getType())->getAddressSpace();
  llvm::IntegerType *ty =
    llvm::IntegerType::get(CGF.getLLVMContext(), AtomicSizeInBits);
  return CGF.Builder.CreateBitCast(addr, ty->getPointerTo(addrspace));
}

RValue AtomicInfo::convertTempToRValue(llvm::Value *addr,
                                       AggValueSlot resultSlot,
                                       SourceLocation loc, bool AsValue) const {
  if (LVal.isSimple()) {
    if (EvaluationKind == TEK_Aggregate)
      return resultSlot.asRValue();

    // Drill into the padding structure if we have one.
    if (hasPadding())
      addr = CGF.Builder.CreateStructGEP(nullptr, addr, 0);

    // Otherwise, just convert the temporary to an r-value using the
    // normal conversion routine.
    return CGF.convertTempToRValue(addr, getValueType(), loc);
  }
  if (!AsValue)
    // Get RValue from temp memory as atomic for non-simple lvalues
    return RValue::get(
        CGF.Builder.CreateAlignedLoad(addr, AtomicAlign.getQuantity()));
  if (LVal.isBitField())
    return CGF.EmitLoadOfBitfieldLValue(LValue::MakeBitfield(
        addr, LVal.getBitFieldInfo(), LVal.getType(), LVal.getAlignment()));
  if (LVal.isVectorElt())
    return CGF.EmitLoadOfLValue(LValue::MakeVectorElt(addr, LVal.getVectorIdx(),
                                                      LVal.getType(),
                                                      LVal.getAlignment()),
                                loc);
  assert(LVal.isExtVectorElt());
  return CGF.EmitLoadOfExtVectorElementLValue(LValue::MakeExtVectorElt(
      addr, LVal.getExtVectorElts(), LVal.getType(), LVal.getAlignment()));
}

RValue AtomicInfo::ConvertIntToValueOrAtomic(llvm::Value *IntVal,
                                             AggValueSlot ResultSlot,
                                             SourceLocation Loc,
                                             bool AsValue) const {
  // Try not to in some easy cases.
  assert(IntVal->getType()->isIntegerTy() && "Expected integer value");
  if (getEvaluationKind() == TEK_Scalar &&
      (((!LVal.isBitField() ||
         LVal.getBitFieldInfo().Size == ValueSizeInBits) &&
        !hasPadding()) ||
       !AsValue)) {
    auto *ValTy = AsValue
                      ? CGF.ConvertTypeForMem(ValueTy)
                      : getAtomicAddress()->getType()->getPointerElementType();
    if (ValTy->isIntegerTy()) {
      assert(IntVal->getType() == ValTy && "Different integer types.");
      return RValue::get(CGF.EmitFromMemory(IntVal, ValueTy));
    } else if (ValTy->isPointerTy())
      return RValue::get(CGF.Builder.CreateIntToPtr(IntVal, ValTy));
    else if (llvm::CastInst::isBitCastable(IntVal->getType(), ValTy))
      return RValue::get(CGF.Builder.CreateBitCast(IntVal, ValTy));
  }

  // Create a temporary.  This needs to be big enough to hold the
  // atomic integer.
  llvm::Value *Temp;
  bool TempIsVolatile = false;
  CharUnits TempAlignment;
  if (AsValue && getEvaluationKind() == TEK_Aggregate) {
    assert(!ResultSlot.isIgnored());
    Temp = ResultSlot.getAddr();
    TempAlignment = getValueAlignment();
    TempIsVolatile = ResultSlot.isVolatile();
  } else {
    Temp = CreateTempAlloca();
    TempAlignment = getAtomicAlignment();
  }

  // Slam the integer into the temporary.
  llvm::Value *CastTemp = emitCastToAtomicIntPointer(Temp);
  CGF.Builder.CreateAlignedStore(IntVal, CastTemp, TempAlignment.getQuantity())
      ->setVolatile(TempIsVolatile);

  return convertTempToRValue(Temp, ResultSlot, Loc, AsValue);
}

void AtomicInfo::EmitAtomicLoadLibcall(llvm::Value *AddForLoaded,
                                       llvm::AtomicOrdering AO, bool) {
  // void __atomic_load(size_t size, void *mem, void *return, int order);
  CallArgList Args;
  Args.add(RValue::get(getAtomicSizeValue()), CGF.getContext().getSizeType());
  Args.add(RValue::get(CGF.EmitCastToVoidPtr(getAtomicAddress())),
           CGF.getContext().VoidPtrTy);
  Args.add(RValue::get(CGF.EmitCastToVoidPtr(AddForLoaded)),
           CGF.getContext().VoidPtrTy);
  Args.add(RValue::get(
               llvm::ConstantInt::get(CGF.IntTy, translateAtomicOrdering(AO))),
           CGF.getContext().IntTy);
  emitAtomicLibcall(CGF, "__atomic_load", CGF.getContext().VoidTy, Args);
}

llvm::Value *AtomicInfo::EmitAtomicLoadOp(llvm::AtomicOrdering AO,
                                          bool IsVolatile) {
  // Okay, we're doing this natively.
  llvm::Value *Addr = emitCastToAtomicIntPointer(getAtomicAddress());
  llvm::LoadInst *Load = CGF.Builder.CreateLoad(Addr, "atomic-load");
  Load->setAtomic(AO);

  // Other decoration.
  Load->setAlignment(getAtomicAlignment().getQuantity());
  if (IsVolatile)
    Load->setVolatile(true);
  if (LVal.getTBAAInfo())
    CGF.CGM.DecorateInstruction(Load, LVal.getTBAAInfo());
  return Load;
}

/// An LValue is a candidate for having its loads and stores be made atomic if
/// we are operating under /volatile:ms *and* the LValue itself is volatile and
/// performing such an operation can be performed without a libcall.
bool CodeGenFunction::LValueIsSuitableForInlineAtomic(LValue LV) {
  AtomicInfo AI(*this, LV);
  bool IsVolatile = LV.isVolatile() || hasVolatileMember(LV.getType());
  // An atomic is inline if we don't need to use a libcall.
  bool AtomicIsInline = !AI.shouldUseLibcall();
  return CGM.getCodeGenOpts().MSVolatile && IsVolatile && AtomicIsInline;
}

/// An type is a candidate for having its loads and stores be made atomic if
/// we are operating under /volatile:ms *and* we know the access is volatile and
/// performing such an operation can be performed without a libcall.
bool CodeGenFunction::typeIsSuitableForInlineAtomic(QualType Ty,
                                                    bool IsVolatile) const {
  // An atomic is inline if we don't need to use a libcall (e.g. it is builtin).
  bool AtomicIsInline = getContext().getTargetInfo().hasBuiltinAtomic(
      getContext().getTypeSize(Ty), getContext().getTypeAlign(Ty));
  return CGM.getCodeGenOpts().MSVolatile && IsVolatile && AtomicIsInline;
}

RValue CodeGenFunction::EmitAtomicLoad(LValue LV, SourceLocation SL,
                                       AggValueSlot Slot) {
  llvm::AtomicOrdering AO;
  bool IsVolatile = LV.isVolatileQualified();
  if (LV.getType()->isAtomicType()) {
    AO = llvm::SequentiallyConsistent;
  } else {
    AO = llvm::Acquire;
    IsVolatile = true;
  }
  return EmitAtomicLoad(LV, SL, AO, IsVolatile, Slot);
}

RValue AtomicInfo::EmitAtomicLoad(AggValueSlot ResultSlot, SourceLocation Loc,
                                  bool AsValue, llvm::AtomicOrdering AO,
                                  bool IsVolatile) {
  // Check whether we should use a library call.
  if (shouldUseLibcall()) {
    llvm::Value *TempAddr;
    if (LVal.isSimple() && !ResultSlot.isIgnored()) {
      assert(getEvaluationKind() == TEK_Aggregate);
      TempAddr = ResultSlot.getAddr();
    } else
      TempAddr = CreateTempAlloca();

    EmitAtomicLoadLibcall(TempAddr, AO, IsVolatile);

    // Okay, turn that back into the original value or whole atomic (for
    // non-simple lvalues) type.
    return convertTempToRValue(TempAddr, ResultSlot, Loc, AsValue);
  }

  // Okay, we're doing this natively.
  auto *Load = EmitAtomicLoadOp(AO, IsVolatile);

  // If we're ignoring an aggregate return, don't do anything.
  if (getEvaluationKind() == TEK_Aggregate && ResultSlot.isIgnored())
    return RValue::getAggregate(nullptr, false);

  // Okay, turn that back into the original value or atomic (for non-simple
  // lvalues) type.
  return ConvertIntToValueOrAtomic(Load, ResultSlot, Loc, AsValue);
}

/// Emit a load from an l-value of atomic type.  Note that the r-value
/// we produce is an r-value of the atomic *value* type.
RValue CodeGenFunction::EmitAtomicLoad(LValue src, SourceLocation loc,
                                       llvm::AtomicOrdering AO, bool IsVolatile,
                                       AggValueSlot resultSlot) {
  AtomicInfo Atomics(*this, src);
  return Atomics.EmitAtomicLoad(resultSlot, loc, /*AsValue=*/true, AO,
                                IsVolatile);
}

/// Copy an r-value into memory as part of storing to an atomic type.
/// This needs to create a bit-pattern suitable for atomic operations.
void AtomicInfo::emitCopyIntoMemory(RValue rvalue) const {
  assert(LVal.isSimple());
  // If we have an r-value, the rvalue should be of the atomic type,
  // which means that the caller is responsible for having zeroed
  // any padding.  Just do an aggregate copy of that type.
  if (rvalue.isAggregate()) {
    CGF.EmitAggregateCopy(getAtomicAddress(),
                          rvalue.getAggregateAddr(),
                          getAtomicType(),
                          (rvalue.isVolatileQualified()
                           || LVal.isVolatileQualified()),
                          LVal.getAlignment());
    return;
  }

  // Okay, otherwise we're copying stuff.

  // Zero out the buffer if necessary.
  emitMemSetZeroIfNecessary();

  // Drill past the padding if present.
  LValue TempLVal = projectValue();

  // Okay, store the rvalue in.
  if (rvalue.isScalar()) {
    CGF.EmitStoreOfScalar(rvalue.getScalarVal(), TempLVal, /*init*/ true);
  } else {
    CGF.EmitStoreOfComplex(rvalue.getComplexVal(), TempLVal, /*init*/ true);
  }
}


/// Materialize an r-value into memory for the purposes of storing it
/// to an atomic type.
llvm::Value *AtomicInfo::materializeRValue(RValue rvalue) const {
  // Aggregate r-values are already in memory, and EmitAtomicStore
  // requires them to be values of the atomic type.
  if (rvalue.isAggregate())
    return rvalue.getAggregateAddr();

  // Otherwise, make a temporary and materialize into it.
  LValue TempLV = CGF.MakeAddrLValue(CreateTempAlloca(), getAtomicType(),
                                     getAtomicAlignment());
  AtomicInfo Atomics(CGF, TempLV);
  Atomics.emitCopyIntoMemory(rvalue);
  return TempLV.getAddress();
}

llvm::Value *AtomicInfo::convertRValueToInt(RValue RVal) const {
  // If we've got a scalar value of the right size, try to avoid going
  // through memory.
  if (RVal.isScalar() && (!hasPadding() || !LVal.isSimple())) {
    llvm::Value *Value = RVal.getScalarVal();
    if (isa<llvm::IntegerType>(Value->getType()))
      return CGF.EmitToMemory(Value, ValueTy);
    else {
      llvm::IntegerType *InputIntTy = llvm::IntegerType::get(
          CGF.getLLVMContext(),
          LVal.isSimple() ? getValueSizeInBits() : getAtomicSizeInBits());
      if (isa<llvm::PointerType>(Value->getType()))
        return CGF.Builder.CreatePtrToInt(Value, InputIntTy);
      else if (llvm::BitCastInst::isBitCastable(Value->getType(), InputIntTy))
        return CGF.Builder.CreateBitCast(Value, InputIntTy);
    }
  }
  // Otherwise, we need to go through memory.
  // Put the r-value in memory.
  llvm::Value *Addr = materializeRValue(RVal);

  // Cast the temporary to the atomic int type and pull a value out.
  Addr = emitCastToAtomicIntPointer(Addr);
  return CGF.Builder.CreateAlignedLoad(Addr,
                                       getAtomicAlignment().getQuantity());
}

std::pair<llvm::Value *, llvm::Value *> AtomicInfo::EmitAtomicCompareExchangeOp(
    llvm::Value *ExpectedVal, llvm::Value *DesiredVal,
    llvm::AtomicOrdering Success, llvm::AtomicOrdering Failure, bool IsWeak) {
  // Do the atomic store.
  auto *Addr = emitCastToAtomicIntPointer(getAtomicAddress());
  auto *Inst = CGF.Builder.CreateAtomicCmpXchg(Addr, ExpectedVal, DesiredVal,
                                               Success, Failure);
  // Other decoration.
  Inst->setVolatile(LVal.isVolatileQualified());
  Inst->setWeak(IsWeak);

  // Okay, turn that back into the original value type.
  auto *PreviousVal = CGF.Builder.CreateExtractValue(Inst, /*Idxs=*/0);
  auto *SuccessFailureVal = CGF.Builder.CreateExtractValue(Inst, /*Idxs=*/1);
  return std::make_pair(PreviousVal, SuccessFailureVal);
}

llvm::Value *
AtomicInfo::EmitAtomicCompareExchangeLibcall(llvm::Value *ExpectedAddr,
                                             llvm::Value *DesiredAddr,
                                             llvm::AtomicOrdering Success,
                                             llvm::AtomicOrdering Failure) {
  // bool __atomic_compare_exchange(size_t size, void *obj, void *expected,
  // void *desired, int success, int failure);
  CallArgList Args;
  Args.add(RValue::get(getAtomicSizeValue()), CGF.getContext().getSizeType());
  Args.add(RValue::get(CGF.EmitCastToVoidPtr(getAtomicAddress())),
           CGF.getContext().VoidPtrTy);
  Args.add(RValue::get(CGF.EmitCastToVoidPtr(ExpectedAddr)),
           CGF.getContext().VoidPtrTy);
  Args.add(RValue::get(CGF.EmitCastToVoidPtr(DesiredAddr)),
           CGF.getContext().VoidPtrTy);
  Args.add(RValue::get(llvm::ConstantInt::get(
               CGF.IntTy, translateAtomicOrdering(Success))),
           CGF.getContext().IntTy);
  Args.add(RValue::get(llvm::ConstantInt::get(
               CGF.IntTy, translateAtomicOrdering(Failure))),
           CGF.getContext().IntTy);
  auto SuccessFailureRVal = emitAtomicLibcall(CGF, "__atomic_compare_exchange",
                                              CGF.getContext().BoolTy, Args);

  return SuccessFailureRVal.getScalarVal();
}

std::pair<RValue, llvm::Value *> AtomicInfo::EmitAtomicCompareExchange(
    RValue Expected, RValue Desired, llvm::AtomicOrdering Success,
    llvm::AtomicOrdering Failure, bool IsWeak) {
  if (Failure >= Success)
    // Don't assert on undefined behavior.
    Failure = llvm::AtomicCmpXchgInst::getStrongestFailureOrdering(Success);

  // Check whether we should use a library call.
  if (shouldUseLibcall()) {
    // Produce a source address.
    auto *ExpectedAddr = materializeRValue(Expected);
    auto *DesiredAddr = materializeRValue(Desired);
    auto *Res = EmitAtomicCompareExchangeLibcall(ExpectedAddr, DesiredAddr,
                                                 Success, Failure);
    return std::make_pair(
        convertTempToRValue(ExpectedAddr, AggValueSlot::ignored(),
                            SourceLocation(), /*AsValue=*/false),
        Res);
  }

  // If we've got a scalar value of the right size, try to avoid going
  // through memory.
  auto *ExpectedVal = convertRValueToInt(Expected);
  auto *DesiredVal = convertRValueToInt(Desired);
  auto Res = EmitAtomicCompareExchangeOp(ExpectedVal, DesiredVal, Success,
                                         Failure, IsWeak);
  return std::make_pair(
      ConvertIntToValueOrAtomic(Res.first, AggValueSlot::ignored(),
                                SourceLocation(), /*AsValue=*/false),
      Res.second);
}

static void
EmitAtomicUpdateValue(CodeGenFunction &CGF, AtomicInfo &Atomics, RValue OldRVal,
                      const llvm::function_ref<RValue(RValue)> &UpdateOp,
                      llvm::Value *DesiredAddr) {
  llvm::Value *Ptr = nullptr;
  LValue UpdateLVal;
  RValue UpRVal;
  LValue AtomicLVal = Atomics.getAtomicLValue();
  LValue DesiredLVal;
  if (AtomicLVal.isSimple()) {
    UpRVal = OldRVal;
    DesiredLVal =
        LValue::MakeAddr(DesiredAddr, AtomicLVal.getType(),
                         AtomicLVal.getAlignment(), CGF.CGM.getContext());
  } else {
    // Build new lvalue for temp address
    Ptr = Atomics.materializeRValue(OldRVal);
    if (AtomicLVal.isBitField()) {
      UpdateLVal =
          LValue::MakeBitfield(Ptr, AtomicLVal.getBitFieldInfo(),
                               AtomicLVal.getType(), AtomicLVal.getAlignment());
      DesiredLVal =
          LValue::MakeBitfield(DesiredAddr, AtomicLVal.getBitFieldInfo(),
                               AtomicLVal.getType(), AtomicLVal.getAlignment());
    } else if (AtomicLVal.isVectorElt()) {
      UpdateLVal = LValue::MakeVectorElt(Ptr, AtomicLVal.getVectorIdx(),
                                         AtomicLVal.getType(),
                                         AtomicLVal.getAlignment());
      DesiredLVal = LValue::MakeVectorElt(
          DesiredAddr, AtomicLVal.getVectorIdx(), AtomicLVal.getType(),
          AtomicLVal.getAlignment());
    } else {
      assert(AtomicLVal.isExtVectorElt());
      UpdateLVal = LValue::MakeExtVectorElt(Ptr, AtomicLVal.getExtVectorElts(),
                                            AtomicLVal.getType(),
                                            AtomicLVal.getAlignment());
      DesiredLVal = LValue::MakeExtVectorElt(
          DesiredAddr, AtomicLVal.getExtVectorElts(), AtomicLVal.getType(),
          AtomicLVal.getAlignment());
    }
    UpdateLVal.setTBAAInfo(AtomicLVal.getTBAAInfo());
    DesiredLVal.setTBAAInfo(AtomicLVal.getTBAAInfo());
    UpRVal = CGF.EmitLoadOfLValue(UpdateLVal, SourceLocation());
  }
  // Store new value in the corresponding memory area
  RValue NewRVal = UpdateOp(UpRVal);
  if (NewRVal.isScalar()) {
    CGF.EmitStoreThroughLValue(NewRVal, DesiredLVal);
  } else {
    assert(NewRVal.isComplex());
    CGF.EmitStoreOfComplex(NewRVal.getComplexVal(), DesiredLVal,
                           /*isInit=*/false);
  }
}

void AtomicInfo::EmitAtomicUpdateLibcall(
    llvm::AtomicOrdering AO, const llvm::function_ref<RValue(RValue)> &UpdateOp,
    bool IsVolatile) {
  auto Failure = llvm::AtomicCmpXchgInst::getStrongestFailureOrdering(AO);

  llvm::Value *ExpectedAddr = CreateTempAlloca();

  EmitAtomicLoadLibcall(ExpectedAddr, AO, IsVolatile);
  auto *ContBB = CGF.createBasicBlock("atomic_cont");
  auto *ExitBB = CGF.createBasicBlock("atomic_exit");
  CGF.EmitBlock(ContBB);
  auto *DesiredAddr = CreateTempAlloca();
  if ((LVal.isBitField() && BFI.Size != ValueSizeInBits) ||
      requiresMemSetZero(
          getAtomicAddress()->getType()->getPointerElementType())) {
    auto *OldVal = CGF.Builder.CreateAlignedLoad(
        ExpectedAddr, getAtomicAlignment().getQuantity());
    CGF.Builder.CreateAlignedStore(OldVal, DesiredAddr,
                                   getAtomicAlignment().getQuantity());
  }
  auto OldRVal = convertTempToRValue(ExpectedAddr, AggValueSlot::ignored(),
                                    SourceLocation(), /*AsValue=*/false);
  EmitAtomicUpdateValue(CGF, *this, OldRVal, UpdateOp, DesiredAddr);
  auto *Res =
      EmitAtomicCompareExchangeLibcall(ExpectedAddr, DesiredAddr, AO, Failure);
  CGF.Builder.CreateCondBr(Res, ExitBB, ContBB);
  CGF.EmitBlock(ExitBB, /*IsFinished=*/true);
}

void AtomicInfo::EmitAtomicUpdateOp(
    llvm::AtomicOrdering AO, const llvm::function_ref<RValue(RValue)> &UpdateOp,
    bool IsVolatile) {
  auto Failure = llvm::AtomicCmpXchgInst::getStrongestFailureOrdering(AO);

  // Do the atomic load.
  auto *OldVal = EmitAtomicLoadOp(AO, IsVolatile);
  // For non-simple lvalues perform compare-and-swap procedure.
  auto *ContBB = CGF.createBasicBlock("atomic_cont");
  auto *ExitBB = CGF.createBasicBlock("atomic_exit");
  auto *CurBB = CGF.Builder.GetInsertBlock();
  CGF.EmitBlock(ContBB);
  llvm::PHINode *PHI = CGF.Builder.CreatePHI(OldVal->getType(),
                                             /*NumReservedValues=*/2);
  PHI->addIncoming(OldVal, CurBB);
  auto *NewAtomicAddr = CreateTempAlloca();
  auto *NewAtomicIntAddr = emitCastToAtomicIntPointer(NewAtomicAddr);
  if ((LVal.isBitField() && BFI.Size != ValueSizeInBits) ||
      requiresMemSetZero(
          getAtomicAddress()->getType()->getPointerElementType())) {
    CGF.Builder.CreateAlignedStore(PHI, NewAtomicIntAddr,
                                   getAtomicAlignment().getQuantity());
  }
  auto OldRVal = ConvertIntToValueOrAtomic(PHI, AggValueSlot::ignored(),
                                           SourceLocation(), /*AsValue=*/false);
  EmitAtomicUpdateValue(CGF, *this, OldRVal, UpdateOp, NewAtomicAddr);
  auto *DesiredVal = CGF.Builder.CreateAlignedLoad(
      NewAtomicIntAddr, getAtomicAlignment().getQuantity());
  // Try to write new value using cmpxchg operation
  auto Res = EmitAtomicCompareExchangeOp(PHI, DesiredVal, AO, Failure);
  PHI->addIncoming(Res.first, CGF.Builder.GetInsertBlock());
  CGF.Builder.CreateCondBr(Res.second, ExitBB, ContBB);
  CGF.EmitBlock(ExitBB, /*IsFinished=*/true);
}

static void EmitAtomicUpdateValue(CodeGenFunction &CGF, AtomicInfo &Atomics,
                                  RValue UpdateRVal, llvm::Value *DesiredAddr) {
  LValue AtomicLVal = Atomics.getAtomicLValue();
  LValue DesiredLVal;
  // Build new lvalue for temp address
  if (AtomicLVal.isBitField()) {
    DesiredLVal =
        LValue::MakeBitfield(DesiredAddr, AtomicLVal.getBitFieldInfo(),
                             AtomicLVal.getType(), AtomicLVal.getAlignment());
  } else if (AtomicLVal.isVectorElt()) {
    DesiredLVal =
        LValue::MakeVectorElt(DesiredAddr, AtomicLVal.getVectorIdx(),
                              AtomicLVal.getType(), AtomicLVal.getAlignment());
  } else {
    assert(AtomicLVal.isExtVectorElt());
    DesiredLVal = LValue::MakeExtVectorElt(
        DesiredAddr, AtomicLVal.getExtVectorElts(), AtomicLVal.getType(),
        AtomicLVal.getAlignment());
  }
  DesiredLVal.setTBAAInfo(AtomicLVal.getTBAAInfo());
  // Store new value in the corresponding memory area
  assert(UpdateRVal.isScalar());
  CGF.EmitStoreThroughLValue(UpdateRVal, DesiredLVal);
}

void AtomicInfo::EmitAtomicUpdateLibcall(llvm::AtomicOrdering AO,
                                         RValue UpdateRVal, bool IsVolatile) {
  auto Failure = llvm::AtomicCmpXchgInst::getStrongestFailureOrdering(AO);

  llvm::Value *ExpectedAddr = CreateTempAlloca();

  EmitAtomicLoadLibcall(ExpectedAddr, AO, IsVolatile);
  auto *ContBB = CGF.createBasicBlock("atomic_cont");
  auto *ExitBB = CGF.createBasicBlock("atomic_exit");
  CGF.EmitBlock(ContBB);
  auto *DesiredAddr = CreateTempAlloca();
  if ((LVal.isBitField() && BFI.Size != ValueSizeInBits) ||
      requiresMemSetZero(
          getAtomicAddress()->getType()->getPointerElementType())) {
    auto *OldVal = CGF.Builder.CreateAlignedLoad(
        ExpectedAddr, getAtomicAlignment().getQuantity());
    CGF.Builder.CreateAlignedStore(OldVal, DesiredAddr,
                                   getAtomicAlignment().getQuantity());
  }
  EmitAtomicUpdateValue(CGF, *this, UpdateRVal, DesiredAddr);
  auto *Res =
      EmitAtomicCompareExchangeLibcall(ExpectedAddr, DesiredAddr, AO, Failure);
  CGF.Builder.CreateCondBr(Res, ExitBB, ContBB);
  CGF.EmitBlock(ExitBB, /*IsFinished=*/true);
}

void AtomicInfo::EmitAtomicUpdateOp(llvm::AtomicOrdering AO, RValue UpdateRVal,
                                    bool IsVolatile) {
  auto Failure = llvm::AtomicCmpXchgInst::getStrongestFailureOrdering(AO);

  // Do the atomic load.
  auto *OldVal = EmitAtomicLoadOp(AO, IsVolatile);
  // For non-simple lvalues perform compare-and-swap procedure.
  auto *ContBB = CGF.createBasicBlock("atomic_cont");
  auto *ExitBB = CGF.createBasicBlock("atomic_exit");
  auto *CurBB = CGF.Builder.GetInsertBlock();
  CGF.EmitBlock(ContBB);
  llvm::PHINode *PHI = CGF.Builder.CreatePHI(OldVal->getType(),
                                             /*NumReservedValues=*/2);
  PHI->addIncoming(OldVal, CurBB);
  auto *NewAtomicAddr = CreateTempAlloca();
  auto *NewAtomicIntAddr = emitCastToAtomicIntPointer(NewAtomicAddr);
  if ((LVal.isBitField() && BFI.Size != ValueSizeInBits) ||
      requiresMemSetZero(
          getAtomicAddress()->getType()->getPointerElementType())) {
    CGF.Builder.CreateAlignedStore(PHI, NewAtomicIntAddr,
                                   getAtomicAlignment().getQuantity());
  }
  EmitAtomicUpdateValue(CGF, *this, UpdateRVal, NewAtomicAddr);
  auto *DesiredVal = CGF.Builder.CreateAlignedLoad(
      NewAtomicIntAddr, getAtomicAlignment().getQuantity());
  // Try to write new value using cmpxchg operation
  auto Res = EmitAtomicCompareExchangeOp(PHI, DesiredVal, AO, Failure);
  PHI->addIncoming(Res.first, CGF.Builder.GetInsertBlock());
  CGF.Builder.CreateCondBr(Res.second, ExitBB, ContBB);
  CGF.EmitBlock(ExitBB, /*IsFinished=*/true);
}

void AtomicInfo::EmitAtomicUpdate(
    llvm::AtomicOrdering AO, const llvm::function_ref<RValue(RValue)> &UpdateOp,
    bool IsVolatile) {
  if (shouldUseLibcall()) {
    EmitAtomicUpdateLibcall(AO, UpdateOp, IsVolatile);
  } else {
    EmitAtomicUpdateOp(AO, UpdateOp, IsVolatile);
  }
}

void AtomicInfo::EmitAtomicUpdate(llvm::AtomicOrdering AO, RValue UpdateRVal,
                                  bool IsVolatile) {
  if (shouldUseLibcall()) {
    EmitAtomicUpdateLibcall(AO, UpdateRVal, IsVolatile);
  } else {
    EmitAtomicUpdateOp(AO, UpdateRVal, IsVolatile);
  }
}

void CodeGenFunction::EmitAtomicStore(RValue rvalue, LValue lvalue,
                                      bool isInit) {
  bool IsVolatile = lvalue.isVolatileQualified();
  llvm::AtomicOrdering AO;
  if (lvalue.getType()->isAtomicType()) {
    AO = llvm::SequentiallyConsistent;
  } else {
    AO = llvm::Release;
    IsVolatile = true;
  }
  return EmitAtomicStore(rvalue, lvalue, AO, IsVolatile, isInit);
}

/// Emit a store to an l-value of atomic type.
///
/// Note that the r-value is expected to be an r-value *of the atomic
/// type*; this means that for aggregate r-values, it should include
/// storage for any padding that was necessary.
void CodeGenFunction::EmitAtomicStore(RValue rvalue, LValue dest,
                                      llvm::AtomicOrdering AO, bool IsVolatile,
                                      bool isInit) {
  // If this is an aggregate r-value, it should agree in type except
  // maybe for address-space qualification.
  assert(!rvalue.isAggregate() ||
         rvalue.getAggregateAddr()->getType()->getPointerElementType()
           == dest.getAddress()->getType()->getPointerElementType());

  AtomicInfo atomics(*this, dest);
  LValue LVal = atomics.getAtomicLValue();

  // If this is an initialization, just put the value there normally.
  if (LVal.isSimple()) {
    if (isInit) {
      atomics.emitCopyIntoMemory(rvalue);
      return;
    }

    // Check whether we should use a library call.
    if (atomics.shouldUseLibcall()) {
      // Produce a source address.
      llvm::Value *srcAddr = atomics.materializeRValue(rvalue);

      // void __atomic_store(size_t size, void *mem, void *val, int order)
      CallArgList args;
      args.add(RValue::get(atomics.getAtomicSizeValue()),
               getContext().getSizeType());
      args.add(RValue::get(EmitCastToVoidPtr(atomics.getAtomicAddress())),
               getContext().VoidPtrTy);
      args.add(RValue::get(EmitCastToVoidPtr(srcAddr)), getContext().VoidPtrTy);
      args.add(RValue::get(llvm::ConstantInt::get(
                   IntTy, AtomicInfo::translateAtomicOrdering(AO))),
               getContext().IntTy);
      emitAtomicLibcall(*this, "__atomic_store", getContext().VoidTy, args);
      return;
    }

    // Okay, we're doing this natively.
    llvm::Value *intValue = atomics.convertRValueToInt(rvalue);

    // Do the atomic store.
    llvm::Value *addr =
        atomics.emitCastToAtomicIntPointer(atomics.getAtomicAddress());
    intValue = Builder.CreateIntCast(
        intValue, addr->getType()->getPointerElementType(), /*isSigned=*/false);
    llvm::StoreInst *store = Builder.CreateStore(intValue, addr);

    // Initializations don't need to be atomic.
    if (!isInit)
      store->setAtomic(AO);

    // Other decoration.
    store->setAlignment(dest.getAlignment().getQuantity());
    if (IsVolatile)
      store->setVolatile(true);
    if (dest.getTBAAInfo())
      CGM.DecorateInstruction(store, dest.getTBAAInfo());
    return;
  }

  // Emit simple atomic update operation.
  atomics.EmitAtomicUpdate(AO, rvalue, IsVolatile);
}

/// Emit a compare-and-exchange op for atomic type.
///
std::pair<RValue, llvm::Value *> CodeGenFunction::EmitAtomicCompareExchange(
    LValue Obj, RValue Expected, RValue Desired, SourceLocation Loc,
    llvm::AtomicOrdering Success, llvm::AtomicOrdering Failure, bool IsWeak,
    AggValueSlot Slot) {
  // If this is an aggregate r-value, it should agree in type except
  // maybe for address-space qualification.
  assert(!Expected.isAggregate() ||
         Expected.getAggregateAddr()->getType()->getPointerElementType() ==
             Obj.getAddress()->getType()->getPointerElementType());
  assert(!Desired.isAggregate() ||
         Desired.getAggregateAddr()->getType()->getPointerElementType() ==
             Obj.getAddress()->getType()->getPointerElementType());
  AtomicInfo Atomics(*this, Obj);

  return Atomics.EmitAtomicCompareExchange(Expected, Desired, Success, Failure,
                                           IsWeak);
}

void CodeGenFunction::EmitAtomicUpdate(
    LValue LVal, llvm::AtomicOrdering AO,
    const llvm::function_ref<RValue(RValue)> &UpdateOp, bool IsVolatile) {
  AtomicInfo Atomics(*this, LVal);
  Atomics.EmitAtomicUpdate(AO, UpdateOp, IsVolatile);
}

void CodeGenFunction::EmitAtomicInit(Expr *init, LValue dest) {
  AtomicInfo atomics(*this, dest);

  switch (atomics.getEvaluationKind()) {
  case TEK_Scalar: {
    llvm::Value *value = EmitScalarExpr(init);
    atomics.emitCopyIntoMemory(RValue::get(value));
    return;
  }

  case TEK_Complex: {
    ComplexPairTy value = EmitComplexExpr(init);
    atomics.emitCopyIntoMemory(RValue::getComplex(value));
    return;
  }

  case TEK_Aggregate: {
    // Fix up the destination if the initializer isn't an expression
    // of atomic type.
    bool Zeroed = false;
    if (!init->getType()->isAtomicType()) {
      Zeroed = atomics.emitMemSetZeroIfNecessary();
      dest = atomics.projectValue();
    }

    // Evaluate the expression directly into the destination.
    AggValueSlot slot = AggValueSlot::forLValue(dest,
                                        AggValueSlot::IsNotDestructed,
                                        AggValueSlot::DoesNotNeedGCBarriers,
                                        AggValueSlot::IsNotAliased,
                                        Zeroed ? AggValueSlot::IsZeroed :
                                                 AggValueSlot::IsNotZeroed);

    EmitAggExpr(init, slot);
    return;
  }
  }
  llvm_unreachable("bad evaluation kind");
}
