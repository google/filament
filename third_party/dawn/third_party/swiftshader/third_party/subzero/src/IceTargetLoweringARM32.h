//===- subzero/src/IceTargetLoweringARM32.h - ARM32 lowering ----*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the TargetLoweringARM32 class, which implements the
/// TargetLowering interface for the ARM 32-bit architecture.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICETARGETLOWERINGARM32_H
#define SUBZERO_SRC_ICETARGETLOWERINGARM32_H

#include "IceAssemblerARM32.h"
#include "IceDefs.h"
#include "IceInstARM32.h"
#include "IceRegistersARM32.h"
#include "IceTargetLowering.h"

#include <utility>

namespace Ice {
namespace ARM32 {

// Class encapsulating ARM cpu features / instruction set.
class TargetARM32Features {
  TargetARM32Features() = delete;
  TargetARM32Features(const TargetARM32Features &) = delete;
  TargetARM32Features &operator=(const TargetARM32Features &) = delete;

public:
  explicit TargetARM32Features(const ClFlags &Flags);

  enum ARM32InstructionSet {
    Begin,
    // Neon is the PNaCl baseline instruction set.
    Neon = Begin,
    HWDivArm, // HW divide in ARM mode (not just Thumb mode).
    End
  };

  bool hasFeature(ARM32InstructionSet I) const { return I <= InstructionSet; }

private:
  ARM32InstructionSet InstructionSet = ARM32InstructionSet::Begin;
};

// The target lowering logic for ARM32.
class TargetARM32 : public TargetLowering {
  TargetARM32() = delete;
  TargetARM32(const TargetARM32 &) = delete;
  TargetARM32 &operator=(const TargetARM32 &) = delete;

public:
  static void staticInit(GlobalContext *Ctx);

  static bool shouldBePooled(const Constant *C) {
    if (auto *ConstDouble = llvm::dyn_cast<ConstantDouble>(C)) {
      return !Utils::isPositiveZero(ConstDouble->getValue());
    }
    if (llvm::isa<ConstantFloat>(C))
      return true;
    return false;
  }

  static ::Ice::Type getPointerType() { return ::Ice::IceType_i32; }

  // TODO(jvoung): return a unique_ptr.
  static std::unique_ptr<::Ice::TargetLowering> create(Cfg *Func) {
    return makeUnique<TargetARM32>(Func);
  }

  std::unique_ptr<::Ice::Assembler> createAssembler() const override {
    return makeUnique<ARM32::AssemblerARM32>();
  }

  void initNodeForLowering(CfgNode *Node) override {
    Computations.forgetProducers();
    Computations.recordProducers(Node);
    Computations.dump(Func);
  }

  void translateOm1() override;
  void translateO2() override;
  bool doBranchOpt(Inst *I, const CfgNode *NextNode) override;

  SizeT getNumRegisters() const override { return RegARM32::Reg_NUM; }
  Variable *getPhysicalRegister(RegNumT RegNum,
                                Type Ty = IceType_void) override;
  const char *getRegName(RegNumT RegNum, Type Ty) const override;
  SmallBitVector getRegisterSet(RegSetMask Include,
                                RegSetMask Exclude) const override;
  const SmallBitVector &
  getRegistersForVariable(const Variable *Var) const override {
    RegClass RC = Var->getRegClass();
    switch (RC) {
    default:
      assert(RC < RC_Target);
      return TypeToRegisterSet[RC];
    case (RegClass)RegARM32::RCARM32_QtoS:
      return TypeToRegisterSet[RC];
    }
  }
  const SmallBitVector &
  getAllRegistersForVariable(const Variable *Var) const override {
    RegClass RC = Var->getRegClass();
    assert((RegARM32::RegClassARM32)RC < RegARM32::RCARM32_NUM);
    return TypeToRegisterSetUnfiltered[RC];
  }
  const SmallBitVector &getAliasesForRegister(RegNumT Reg) const override {
    return RegisterAliases[Reg];
  }
  bool hasFramePointer() const override { return UsesFramePointer; }
  void setHasFramePointer() override { UsesFramePointer = true; }
  RegNumT getStackReg() const override { return RegARM32::Reg_sp; }
  RegNumT getFrameReg() const override { return RegARM32::Reg_fp; }
  RegNumT getFrameOrStackReg() const override {
    return UsesFramePointer ? getFrameReg() : getStackReg();
  }
  RegNumT getReservedTmpReg() const { return RegARM32::Reg_ip; }

  size_t typeWidthInBytesOnStack(Type Ty) const override {
    // Round up to the next multiple of 4 bytes. In particular, i1, i8, and i16
    // are rounded up to 4 bytes.
    return (typeWidthInBytes(Ty) + 3) & ~3;
  }
  uint32_t getStackAlignment() const override;
  void reserveFixedAllocaArea(size_t Size, size_t Align) override {
    FixedAllocaSizeBytes = Size;
    assert(llvm::isPowerOf2_32(Align));
    FixedAllocaAlignBytes = Align;
    PrologEmitsFixedAllocas = true;
  }
  int32_t getFrameFixedAllocaOffset() const override {
    return FixedAllocaSizeBytes - (SpillAreaSizeBytes - MaxOutArgsSizeBytes);
  }
  uint32_t maxOutArgsSizeBytes() const override { return MaxOutArgsSizeBytes; }

  bool shouldSplitToVariable64On32(Type Ty) const override {
    return Ty == IceType_i64;
  }

  // TODO(ascull): what size is best for ARM?
  SizeT getMinJumpTableSize() const override { return 3; }
  void emitJumpTable(const Cfg *Func,
                     const InstJumpTable *JumpTable) const override;

  void emitVariable(const Variable *Var) const override;

  void emit(const ConstantUndef *C) const final;
  void emit(const ConstantInteger32 *C) const final;
  void emit(const ConstantInteger64 *C) const final;
  void emit(const ConstantFloat *C) const final;
  void emit(const ConstantDouble *C) const final;
  void emit(const ConstantRelocatable *C) const final;

  void lowerArguments() override;
  void addProlog(CfgNode *Node) override;
  void addEpilog(CfgNode *Node) override;

  Operand *loOperand(Operand *Operand);
  Operand *hiOperand(Operand *Operand);
  void finishArgumentLowering(Variable *Arg, Variable *FramePtr,
                              size_t BasicFrameOffset, size_t *InArgsSizeBytes);

  bool hasCPUFeature(TargetARM32Features::ARM32InstructionSet I) const {
    return CPUFeatures.hasFeature(I);
  }

  enum OperandLegalization {
    Legal_Reg = 1 << 0,  /// physical register, not stack location
    Legal_Flex = 1 << 1, /// A flexible operand2, which can hold rotated small
                         /// immediates, shifted registers, or modified fp imm.
    Legal_Mem = 1 << 2,  /// includes [r0, r1 lsl #2] as well as [sp, #12]
    Legal_Rematerializable = 1 << 3,
    Legal_Default = ~Legal_Rematerializable,
  };

  using LegalMask = uint32_t;
  Operand *legalizeUndef(Operand *From, RegNumT RegNum = RegNumT());
  Operand *legalize(Operand *From, LegalMask Allowed = Legal_Default,
                    RegNumT RegNum = RegNumT());
  Variable *legalizeToReg(Operand *From, RegNumT RegNum = RegNumT());

  OperandARM32ShAmtImm *shAmtImm(uint32_t ShAmtImm) const {
    assert(ShAmtImm < 32);
    return OperandARM32ShAmtImm::create(
        Func,
        llvm::cast<ConstantInteger32>(Ctx->getConstantInt32(ShAmtImm & 0x1F)));
  }

  GlobalContext *getCtx() const { return Ctx; }

protected:
  explicit TargetARM32(Cfg *Func);

  void postLower() override;

  enum SafeBoolChain {
    SBC_No,
    SBC_Yes,
  };

  void lowerAlloca(const InstAlloca *Instr) override;
  SafeBoolChain lowerInt1Arithmetic(const InstArithmetic *Instr);
  void lowerInt64Arithmetic(InstArithmetic::OpKind Op, Variable *Dest,
                            Operand *Src0, Operand *Src1);
  void lowerArithmetic(const InstArithmetic *Instr) override;
  void lowerAssign(const InstAssign *Instr) override;
  void lowerBr(const InstBr *Instr) override;
  void lowerCall(const InstCall *Instr) override;
  void lowerCast(const InstCast *Instr) override;
  void lowerExtractElement(const InstExtractElement *Instr) override;

  /// CondWhenTrue is a helper type returned by every method in the lowering
  /// that emits code to set the condition codes.
  class CondWhenTrue {
  public:
    explicit CondWhenTrue(CondARM32::Cond T0,
                          CondARM32::Cond T1 = CondARM32::kNone)
        : WhenTrue0(T0), WhenTrue1(T1) {
      assert(T1 == CondARM32::kNone || T0 != CondARM32::kNone);
      assert(T1 != T0 || T0 == CondARM32::kNone);
    }
    CondARM32::Cond WhenTrue0;
    CondARM32::Cond WhenTrue1;

    /// invert returns a new object with WhenTrue0 and WhenTrue1 inverted.
    CondWhenTrue invert() const {
      switch (WhenTrue0) {
      default:
        if (WhenTrue1 == CondARM32::kNone)
          return CondWhenTrue(InstARM32::getOppositeCondition(WhenTrue0));
        return CondWhenTrue(InstARM32::getOppositeCondition(WhenTrue0),
                            InstARM32::getOppositeCondition(WhenTrue1));
      case CondARM32::AL:
        return CondWhenTrue(CondARM32::kNone);
      case CondARM32::kNone:
        return CondWhenTrue(CondARM32::AL);
      }
    }
  };

  CondWhenTrue lowerFcmpCond(const InstFcmp *Instr);
  void lowerFcmp(const InstFcmp *Instr) override;
  CondWhenTrue lowerInt8AndInt16IcmpCond(InstIcmp::ICond Condition,
                                         Operand *Src0, Operand *Src1);
  CondWhenTrue lowerInt32IcmpCond(InstIcmp::ICond Condition, Operand *Src0,
                                  Operand *Src1);
  CondWhenTrue lowerInt64IcmpCond(InstIcmp::ICond Condition, Operand *Src0,
                                  Operand *Src1);
  CondWhenTrue lowerIcmpCond(InstIcmp::ICond Condition, Operand *Src0,
                             Operand *Src1);
  CondWhenTrue lowerIcmpCond(const InstIcmp *Instr);
  void lowerIcmp(const InstIcmp *Instr) override;
  /// Emits the basic sequence for lower-linked/store-exclusive loops:
  ///
  /// retry:
  ///        ldrex tmp, [Addr]
  ///        StoreValue = Operation(tmp)
  ///        strexCond success, StoreValue, [Addr]
  ///        cmpCond success, #0
  ///        bne retry
  ///
  /// Operation needs to return which value to strex in Addr, it must not change
  /// the flags if Cond is not AL, and must not emit any instructions that could
  /// end up writing to memory. Operation also needs to handle fake-defing for
  /// i64 handling.
  void
  lowerLoadLinkedStoreExclusive(Type Ty, Operand *Addr,
                                std::function<Variable *(Variable *)> Operation,
                                CondARM32::Cond Cond = CondARM32::AL);
  void lowerInt64AtomicRMW(Variable *Dest, uint32_t Operation, Operand *Ptr,
                           Operand *Val);
  void lowerAtomicRMW(Variable *Dest, uint32_t Operation, Operand *Ptr,
                      Operand *Val);
  void lowerBreakpoint(const InstBreakpoint *Instr) override;
  void lowerIntrinsic(const InstIntrinsic *Instr) override;
  void lowerInsertElement(const InstInsertElement *Instr) override;
  void lowerLoad(const InstLoad *Instr) override;
  void lowerPhi(const InstPhi *Instr) override;
  void lowerRet(const InstRet *Instr) override;
  void lowerSelect(const InstSelect *Instr) override;
  void lowerShuffleVector(const InstShuffleVector *Instr) override;
  void lowerStore(const InstStore *Instr) override;
  void lowerSwitch(const InstSwitch *Instr) override;
  void lowerUnreachable(const InstUnreachable *Instr) override;
  void prelowerPhis() override;
  uint32_t getCallStackArgumentsSizeBytes(const InstCall *Instr) override;
  void genTargetHelperCallFor(Inst *Instr) override;
  void doAddressOptLoad() override;
  void doAddressOptStore() override;

  OperandARM32Mem *formMemoryOperand(Operand *Ptr, Type Ty);

  Variable64On32 *makeI64RegPair();
  Variable *makeReg(Type Ty, RegNumT RegNum = RegNumT());
  static Type stackSlotType();
  Variable *copyToReg(Operand *Src, RegNumT RegNum = RegNumT());
  void alignRegisterPow2(Variable *Reg, uint32_t Align,
                         RegNumT TmpRegNum = RegNumT());

  /// Returns a vector in a register with the given constant entries.
  Variable *makeVectorOfZeros(Type Ty, RegNumT RegNum = RegNumT());

  // If a divide-by-zero check is needed, inserts a: test; branch .LSKIP; trap;
  // .LSKIP: <continuation>. If no check is needed nothing is inserted.
  void div0Check(Type Ty, Operand *SrcLo, Operand *SrcHi);
  using ExtInstr = void (TargetARM32::*)(Variable *, Variable *,
                                         CondARM32::Cond);
  using DivInstr = void (TargetARM32::*)(Variable *, Variable *, Variable *,
                                         CondARM32::Cond);
  void lowerIDivRem(Variable *Dest, Variable *T, Variable *Src0R, Operand *Src1,
                    ExtInstr ExtFunc, DivInstr DivFunc, bool IsRemainder);

  void lowerCLZ(Variable *Dest, Variable *ValLo, Variable *ValHi);

  // The following are helpers that insert lowered ARM32 instructions with
  // minimal syntactic overhead, so that the lowering code can look as close to
  // assembly as practical.
  void _add(Variable *Dest, Variable *Src0, Operand *Src1,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Add>(Dest, Src0, Src1, Pred);
  }
  void _adds(Variable *Dest, Variable *Src0, Operand *Src1,
             CondARM32::Cond Pred = CondARM32::AL) {
    constexpr bool SetFlags = true;
    Context.insert<InstARM32Add>(Dest, Src0, Src1, Pred, SetFlags);
    if (SetFlags) {
      Context.insert<InstFakeUse>(Dest);
    }
  }
  void _adc(Variable *Dest, Variable *Src0, Operand *Src1,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Adc>(Dest, Src0, Src1, Pred);
  }
  void _and(Variable *Dest, Variable *Src0, Operand *Src1,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32And>(Dest, Src0, Src1, Pred);
  }
  void _asr(Variable *Dest, Variable *Src0, Operand *Src1,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Asr>(Dest, Src0, Src1, Pred);
  }
  void _bic(Variable *Dest, Variable *Src0, Operand *Src1,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Bic>(Dest, Src0, Src1, Pred);
  }
  void _br(CfgNode *TargetTrue, CfgNode *TargetFalse,
           CondARM32::Cond Condition) {
    Context.insert<InstARM32Br>(TargetTrue, TargetFalse, Condition);
  }
  void _br(CfgNode *Target) { Context.insert<InstARM32Br>(Target); }
  void _br(CfgNode *Target, CondARM32::Cond Condition) {
    Context.insert<InstARM32Br>(Target, Condition);
  }
  void _br(InstARM32Label *Label, CondARM32::Cond Condition) {
    Context.insert<InstARM32Br>(Label, Condition);
  }
  void _cmn(Variable *Src0, Operand *Src1,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Cmn>(Src0, Src1, Pred);
  }
  void _cmp(Variable *Src0, Operand *Src1,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Cmp>(Src0, Src1, Pred);
  }
  void _clz(Variable *Dest, Variable *Src0,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Clz>(Dest, Src0, Pred);
  }
  void _dmb() { Context.insert<InstARM32Dmb>(); }
  void _eor(Variable *Dest, Variable *Src0, Operand *Src1,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Eor>(Dest, Src0, Src1, Pred);
  }
  /// _ldr, for all your memory to Variable data moves. It handles all types
  /// (integer, floating point, and vectors.) Addr needs to be valid for Dest's
  /// type (e.g., no immediates for vector loads, and no index registers for fp
  /// loads.)
  void _ldr(Variable *Dest, OperandARM32Mem *Addr,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Ldr>(Dest, Addr, Pred);
  }
  InstARM32Ldrex *_ldrex(Variable *Dest, OperandARM32Mem *Addr,
                         CondARM32::Cond Pred = CondARM32::AL) {
    auto *Ldrex = Context.insert<InstARM32Ldrex>(Dest, Addr, Pred);
    if (auto *Dest64 = llvm::dyn_cast<Variable64On32>(Dest)) {
      Context.insert<InstFakeDef>(Dest64->getLo(), Dest);
      Context.insert<InstFakeDef>(Dest64->getHi(), Dest);
    }
    return Ldrex;
  }
  void _lsl(Variable *Dest, Variable *Src0, Operand *Src1,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Lsl>(Dest, Src0, Src1, Pred);
  }
  void _lsls(Variable *Dest, Variable *Src0, Operand *Src1,
             CondARM32::Cond Pred = CondARM32::AL) {
    constexpr bool SetFlags = true;
    Context.insert<InstARM32Lsl>(Dest, Src0, Src1, Pred, SetFlags);
    if (SetFlags) {
      Context.insert<InstFakeUse>(Dest);
    }
  }
  void _lsr(Variable *Dest, Variable *Src0, Operand *Src1,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Lsr>(Dest, Src0, Src1, Pred);
  }
  void _mla(Variable *Dest, Variable *Src0, Variable *Src1, Variable *Acc,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Mla>(Dest, Src0, Src1, Acc, Pred);
  }
  void _mls(Variable *Dest, Variable *Src0, Variable *Src1, Variable *Acc,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Mls>(Dest, Src0, Src1, Acc, Pred);
  }
  /// _mov, for all your Variable to Variable data movement needs. It handles
  /// all types (integer, floating point, and vectors), as well as moves between
  /// Core and VFP registers. This is not a panacea: you must obey the (weird,
  /// confusing, non-uniform) rules for data moves in ARM.
  void _mov(Variable *Dest, Operand *Src0,
            CondARM32::Cond Pred = CondARM32::AL) {
    // _mov used to be unique in the sense that it would create a temporary
    // automagically if Dest was nullptr. It won't do that anymore, so we keep
    // an assert around just in case there is some untested code path where Dest
    // is nullptr.
    assert(Dest != nullptr);
    assert(!llvm::isa<OperandARM32Mem>(Src0));
    auto *Instr = Context.insert<InstARM32Mov>(Dest, Src0, Pred);

    if (Instr->isMultiDest()) {
      // If Instr is multi-dest, then Dest must be a Variable64On32. We add a
      // fake-def for Instr.DestHi here.
      assert(llvm::isa<Variable64On32>(Dest));
      Context.insert<InstFakeDef>(Instr->getDestHi());
    }
  }

  void _mov_redefined(Variable *Dest, Operand *Src0,
                      CondARM32::Cond Pred = CondARM32::AL) {
    auto *Instr = Context.insert<InstARM32Mov>(Dest, Src0, Pred);
    Instr->setDestRedefined();
    if (Instr->isMultiDest()) {
      // If Instr is multi-dest, then Dest must be a Variable64On32. We add a
      // fake-def for Instr.DestHi here.
      assert(llvm::isa<Variable64On32>(Dest));
      Context.insert<InstFakeDef>(Instr->getDestHi());
    }
  }

  void _nop() { Context.insert<InstARM32Nop>(); }

  // Generates a vmov instruction to extract the given index from a vector
  // register.
  void _extractelement(Variable *Dest, Variable *Src0, uint32_t Index,
                       CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Extract>(Dest, Src0, Index, Pred);
  }

  // Generates a vmov instruction to insert a value into the given index of a
  // vector register.
  void _insertelement(Variable *Dest, Variable *Src0, uint32_t Index,
                      CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Insert>(Dest, Src0, Index, Pred);
  }

  // --------------------------------------------------------------------------
  // Begin bool folding machinery.
  //
  // There are three types of boolean lowerings handled by this target:
  //
  // 1) Boolean expressions leading to a boolean Variable definition
  // ---------------------------------------------------------------
  //
  // Whenever a i1 Variable is live out (i.e., its live range extends beyond
  // the defining basic block) we do not fold the operation. We instead
  // materialize (i.e., compute) the variable normally, so that it can be used
  // when needed. We also materialize i1 values that are not single use to
  // avoid code duplication. These expressions are not short circuited.
  //
  // 2) Boolean expressions leading to a select
  // ------------------------------------------
  //
  // These include boolean chains leading to a select instruction, as well as
  // i1 Sexts. These boolean expressions are lowered to:
  //
  // mov T, <false value>
  // CC <- eval(Boolean Expression)
  // movCC T, <true value>
  //
  // For Sexts, <false value> is 0, and <true value> is -1.
  //
  // 3) Boolean expressions leading to a br i1
  // -----------------------------------------
  //
  // These are the boolean chains leading to a branch. These chains are
  // short-circuited, i.e.:
  //
  //   A = or i1 B, C
  //   br i1 A, label %T, label %F
  //
  // becomes
  //
  //   tst B
  //   jne %T
  //   tst B
  //   jne %T
  //   j %F
  //
  // and
  //
  //   A = and i1 B, C
  //   br i1 A, label %T, label %F
  //
  // becomes
  //
  //   tst B
  //   jeq %F
  //   tst B
  //   jeq %F
  //   j %T
  //
  // Arbitrarily long chains are short circuited, e.g
  //
  //   A = or  i1 B, C
  //   D = and i1 A, E
  //   F = and i1 G, H
  //   I = or i1 D, F
  //   br i1 I, label %True, label %False
  //
  // becomes
  //
  // Label[A]:
  //   tst B, 1
  //   bne Label[D]
  //   tst C, 1
  //   beq Label[I]
  // Label[D]:
  //   tst E, 1
  //   bne %True
  // Label[I]
  //   tst G, 1
  //   beq %False
  //   tst H, 1
  //   beq %False (bne %True)

  /// lowerInt1 materializes Boolean to a Variable.
  SafeBoolChain lowerInt1(Variable *Dest, Operand *Boolean);

  /// lowerInt1ForSelect generates the following instruction sequence:
  ///
  ///   mov T, FalseValue
  ///   CC <- eval(Boolean)
  ///   movCC T, TrueValue
  ///   mov Dest, T
  ///
  /// It is used for lowering select i1, as well as i1 Sext.
  void lowerInt1ForSelect(Variable *Dest, Operand *Boolean, Operand *TrueValue,
                          Operand *FalseValue);

  /// LowerInt1BranchTarget is used by lowerIntForBranch. It wraps a CfgNode, or
  /// an InstARM32Label (but never both) so that, during br i1 lowering, we can
  /// create auxiliary labels for short circuiting the condition evaluation.
  class LowerInt1BranchTarget {
  public:
    explicit LowerInt1BranchTarget(CfgNode *const Target)
        : NodeTarget(Target) {}
    explicit LowerInt1BranchTarget(InstARM32Label *const Target)
        : LabelTarget(Target) {}

    /// createForLabelOrDuplicate will return a new LowerInt1BranchTarget that
    /// is the exact copy of this if Label is nullptr; otherwise, the returned
    /// object will wrap Label instead.
    LowerInt1BranchTarget
    createForLabelOrDuplicate(InstARM32Label *Label) const {
      if (Label != nullptr)
        return LowerInt1BranchTarget(Label);
      if (NodeTarget)
        return LowerInt1BranchTarget(NodeTarget);
      return LowerInt1BranchTarget(LabelTarget);
    }

    CfgNode *const NodeTarget = nullptr;
    InstARM32Label *const LabelTarget = nullptr;
  };

  /// LowerInt1AllowShortCircuit is a helper type used by lowerInt1ForBranch for
  /// determining which type arithmetic is allowed to be short circuited. This
  /// is useful for lowering
  ///
  ///   t1 = and i1 A, B
  ///   t2 = and i1 t1, C
  ///   br i1 t2, label %False, label %True
  ///
  /// to
  ///
  ///   tst A, 1
  ///   beq %False
  ///   tst B, 1
  ///   beq %False
  ///   tst C, 1
  ///   bne %True
  ///   b %False
  ///
  /// Without this information, short circuiting would only allow to short
  /// circuit a single high level instruction. For example:
  ///
  ///   t1 = or i1 A, B
  ///   t2 = and i1 t1, C
  ///   br i1 t2, label %False, label %True
  ///
  /// cannot be lowered to
  ///
  ///   tst A, 1
  ///   bne %True
  ///   tst B, 1
  ///   bne %True
  ///   tst C, 1
  ///   beq %True
  ///   b %False
  ///
  /// It needs to be lowered to
  ///
  ///   tst A, 1
  ///   bne Aux
  ///   tst B, 1
  ///   beq %False
  /// Aux:
  ///   tst C, 1
  ///   bne %True
  ///   b %False
  ///
  /// TODO(jpp): evaluate if this kind of short circuiting hurts performance (it
  /// might.)
  enum LowerInt1AllowShortCircuit {
    SC_And = 1,
    SC_Or = 2,
    SC_All = SC_And | SC_Or,
  };

  /// ShortCircuitCondAndLabel wraps the condition codes that should be used
  /// after a lowerInt1ForBranch returns to branch to the
  /// TrueTarget/FalseTarget. If ShortCircuitLabel is not nullptr, then the
  /// called lowerInt1forBranch created an internal (i.e., short-circuit) label
  /// used for short circuiting.
  class ShortCircuitCondAndLabel {
  public:
    explicit ShortCircuitCondAndLabel(CondWhenTrue &&C,
                                      InstARM32Label *L = nullptr)
        : Cond(std::move(C)), ShortCircuitTarget(L) {}
    const CondWhenTrue Cond;
    InstARM32Label *const ShortCircuitTarget;

    CondWhenTrue assertNoLabelAndReturnCond() const {
      assert(ShortCircuitTarget == nullptr);
      return Cond;
    }
  };

  /// lowerInt1ForBranch expands Boolean, and returns the condition codes that
  /// are to be used for branching to the branch's TrueTarget. It may return a
  /// label that the expansion of Boolean used to short circuit the chain's
  /// evaluation.
  ShortCircuitCondAndLabel
  lowerInt1ForBranch(Operand *Boolean, const LowerInt1BranchTarget &TargetTrue,
                     const LowerInt1BranchTarget &TargetFalse,
                     uint32_t ShortCircuitable);

  // _br is a convenience wrapper that emits br instructions to Target.
  void _br(const LowerInt1BranchTarget &BrTarget,
           CondARM32::Cond Cond = CondARM32::AL) {
    assert((BrTarget.NodeTarget == nullptr) !=
           (BrTarget.LabelTarget == nullptr));
    if (BrTarget.NodeTarget != nullptr)
      _br(BrTarget.NodeTarget, Cond);
    else
      _br(BrTarget.LabelTarget, Cond);
  }

  // _br_short_circuit is used when lowering InstArithmetic::And and
  // InstArithmetic::Or and a short circuit branch is needed.
  void _br_short_circuit(const LowerInt1BranchTarget &Target,
                         const CondWhenTrue &Cond) {
    if (Cond.WhenTrue1 != CondARM32::kNone) {
      _br(Target, Cond.WhenTrue1);
    }
    if (Cond.WhenTrue0 != CondARM32::kNone) {
      _br(Target, Cond.WhenTrue0);
    }
  }
  // End of bool folding machinery
  // --------------------------------------------------------------------------

  /// The Operand can only be a 16-bit immediate or a ConstantRelocatable (with
  /// an upper16 relocation).
  void _movt(Variable *Dest, Operand *Src0,
             CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Movt>(Dest, Src0, Pred);
  }
  void _movw(Variable *Dest, Operand *Src0,
             CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Movw>(Dest, Src0, Pred);
  }
  void _mul(Variable *Dest, Variable *Src0, Variable *Src1,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Mul>(Dest, Src0, Src1, Pred);
  }
  void _mvn(Variable *Dest, Operand *Src0,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Mvn>(Dest, Src0, Pred);
  }
  void _orr(Variable *Dest, Variable *Src0, Operand *Src1,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Orr>(Dest, Src0, Src1, Pred);
  }
  void _orrs(Variable *Dest, Variable *Src0, Operand *Src1,
             CondARM32::Cond Pred = CondARM32::AL) {
    constexpr bool SetFlags = true;
    Context.insert<InstARM32Orr>(Dest, Src0, Src1, Pred, SetFlags);
    if (SetFlags) {
      Context.insert<InstFakeUse>(Dest);
    }
  }
  void _push(const VarList &Sources) { Context.insert<InstARM32Push>(Sources); }
  void _pop(const VarList &Dests) {
    Context.insert<InstARM32Pop>(Dests);
    // Mark dests as modified.
    for (Variable *Dest : Dests)
      Context.insert<InstFakeDef>(Dest);
  }
  void _rbit(Variable *Dest, Variable *Src0,
             CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Rbit>(Dest, Src0, Pred);
  }
  void _rev(Variable *Dest, Variable *Src0,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Rev>(Dest, Src0, Pred);
  }
  void _ret(Variable *LR, Variable *Src0 = nullptr) {
    Context.insert<InstARM32Ret>(LR, Src0);
  }
  void _rscs(Variable *Dest, Variable *Src0, Operand *Src1,
             CondARM32::Cond Pred = CondARM32::AL) {
    constexpr bool SetFlags = true;
    Context.insert<InstARM32Rsc>(Dest, Src0, Src1, Pred, SetFlags);
    if (SetFlags) {
      Context.insert<InstFakeUse>(Dest);
    }
  }
  void _rsc(Variable *Dest, Variable *Src0, Operand *Src1,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Rsc>(Dest, Src0, Src1, Pred);
  }
  void _rsbs(Variable *Dest, Variable *Src0, Operand *Src1,
             CondARM32::Cond Pred = CondARM32::AL) {
    constexpr bool SetFlags = true;
    Context.insert<InstARM32Rsb>(Dest, Src0, Src1, Pred, SetFlags);
    if (SetFlags) {
      Context.insert<InstFakeUse>(Dest);
    }
  }
  void _rsb(Variable *Dest, Variable *Src0, Operand *Src1,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Rsb>(Dest, Src0, Src1, Pred);
  }
  void _sbc(Variable *Dest, Variable *Src0, Operand *Src1,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Sbc>(Dest, Src0, Src1, Pred);
  }
  void _sbcs(Variable *Dest, Variable *Src0, Operand *Src1,
             CondARM32::Cond Pred = CondARM32::AL) {
    constexpr bool SetFlags = true;
    Context.insert<InstARM32Sbc>(Dest, Src0, Src1, Pred, SetFlags);
    if (SetFlags) {
      Context.insert<InstFakeUse>(Dest);
    }
  }
  void _sdiv(Variable *Dest, Variable *Src0, Variable *Src1,
             CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Sdiv>(Dest, Src0, Src1, Pred);
  }
  /// _str, for all your Variable to memory transfers. Addr has the same
  /// restrictions that it does in _ldr.
  void _str(Variable *Value, OperandARM32Mem *Addr,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Str>(Value, Addr, Pred);
  }
  InstARM32Strex *_strex(Variable *Dest, Variable *Value, OperandARM32Mem *Addr,
                         CondARM32::Cond Pred = CondARM32::AL) {
    if (auto *Value64 = llvm::dyn_cast<Variable64On32>(Value)) {
      Context.insert<InstFakeUse>(Value64->getLo());
      Context.insert<InstFakeUse>(Value64->getHi());
    }
    return Context.insert<InstARM32Strex>(Dest, Value, Addr, Pred);
  }
  void _sub(Variable *Dest, Variable *Src0, Operand *Src1,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Sub>(Dest, Src0, Src1, Pred);
  }
  void _subs(Variable *Dest, Variable *Src0, Operand *Src1,
             CondARM32::Cond Pred = CondARM32::AL) {
    constexpr bool SetFlags = true;
    Context.insert<InstARM32Sub>(Dest, Src0, Src1, Pred, SetFlags);
    if (SetFlags) {
      Context.insert<InstFakeUse>(Dest);
    }
  }
  void _sxt(Variable *Dest, Variable *Src0,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Sxt>(Dest, Src0, Pred);
  }
  void _tst(Variable *Src0, Operand *Src1,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Tst>(Src0, Src1, Pred);
  }
  void _trap() { Context.insert<InstARM32Trap>(); }
  void _udiv(Variable *Dest, Variable *Src0, Variable *Src1,
             CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Udiv>(Dest, Src0, Src1, Pred);
  }
  void _umull(Variable *DestLo, Variable *DestHi, Variable *Src0,
              Variable *Src1, CondARM32::Cond Pred = CondARM32::AL) {
    // umull requires DestLo and DestHi to be assigned to different GPRs. The
    // following lines create overlapping liveness ranges for both variables. If
    // either one of them is live, then they are both going to be live, and thus
    // assigned to different registers; if they are both dead, then DCE will
    // kick in and delete the following three instructions.
    Context.insert<InstFakeDef>(DestHi);
    Context.insert<InstARM32Umull>(DestLo, DestHi, Src0, Src1, Pred);
    Context.insert<InstFakeDef>(DestHi, DestLo)->setDestRedefined();
    Context.insert<InstFakeUse>(DestHi);
  }
  void _uxt(Variable *Dest, Variable *Src0,
            CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Uxt>(Dest, Src0, Pred);
  }
  void _vabs(Variable *Dest, Variable *Src,
             CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Vabs>(Dest, Src, Pred);
  }
  void _vadd(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstARM32Vadd>(Dest, Src0, Src1);
  }
  void _vand(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstARM32Vand>(Dest, Src0, Src1);
  }
  InstARM32Vbsl *_vbsl(Variable *Dest, Variable *Src0, Variable *Src1) {
    return Context.insert<InstARM32Vbsl>(Dest, Src0, Src1);
  }
  void _vceq(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstARM32Vceq>(Dest, Src0, Src1);
  }
  InstARM32Vcge *_vcge(Variable *Dest, Variable *Src0, Variable *Src1) {
    return Context.insert<InstARM32Vcge>(Dest, Src0, Src1);
  }
  InstARM32Vcgt *_vcgt(Variable *Dest, Variable *Src0, Variable *Src1) {
    return Context.insert<InstARM32Vcgt>(Dest, Src0, Src1);
  }
  void _vcvt(Variable *Dest, Variable *Src, InstARM32Vcvt::VcvtVariant Variant,
             CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Vcvt>(Dest, Src, Variant, Pred);
  }
  void _vdiv(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstARM32Vdiv>(Dest, Src0, Src1);
  }
  void _vcmp(Variable *Src0, Variable *Src1,
             CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Vcmp>(Src0, Src1, Pred);
  }
  void _vcmp(Variable *Src0, OperandARM32FlexFpZero *FpZero,
             CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Vcmp>(Src0, FpZero, Pred);
  }
  void _vdup(Variable *Dest, Variable *Src, int Idx) {
    Context.insert<InstARM32Vdup>(Dest, Src, Idx);
  }
  void _veor(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstARM32Veor>(Dest, Src0, Src1);
  }
  void _vldr1d(Variable *Dest, OperandARM32Mem *Addr,
               CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Vldr1d>(Dest, Addr, Pred);
  }
  void _vldr1q(Variable *Dest, OperandARM32Mem *Addr,
               CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Vldr1q>(Dest, Addr, Pred);
  }
  void _vmrs(CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Vmrs>(Pred);
  }
  void _vmla(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstARM32Vmla>(Dest, Src0, Src1);
  }
  void _vmlap(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstARM32Vmlap>(Dest, Src0, Src1);
  }
  void _vmls(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstARM32Vmls>(Dest, Src0, Src1);
  }
  void _vmovl(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstARM32Vmovl>(Dest, Src0, Src1);
  }
  void _vmovh(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstARM32Vmovh>(Dest, Src0, Src1);
  }
  void _vmovhl(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstARM32Vmovhl>(Dest, Src0, Src1);
  }
  void _vmovlh(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstARM32Vmovlh>(Dest, Src0, Src1);
  }
  void _vmul(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstARM32Vmul>(Dest, Src0, Src1);
  }
  void _vmulh(Variable *Dest, Variable *Src0, Variable *Src1, bool Unsigned) {
    Context.insert<InstARM32Vmulh>(Dest, Src0, Src1)
        ->setSignType(Unsigned ? InstARM32::FS_Unsigned : InstARM32::FS_Signed);
  }
  void _vmvn(Variable *Dest, Variable *Src0) {
    Context.insert<InstARM32Vmvn>(Dest, Src0, CondARM32::AL);
  }
  void _vneg(Variable *Dest, Variable *Src0) {
    Context.insert<InstARM32Vneg>(Dest, Src0, CondARM32::AL)
        ->setSignType(InstARM32::FS_Signed);
  }
  void _vorr(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstARM32Vorr>(Dest, Src0, Src1);
  }
  void _vqadd(Variable *Dest, Variable *Src0, Variable *Src1, bool Unsigned) {
    Context.insert<InstARM32Vqadd>(Dest, Src0, Src1)
        ->setSignType(Unsigned ? InstARM32::FS_Unsigned : InstARM32::FS_Signed);
  }
  void _vqmovn2(Variable *Dest, Variable *Src0, Variable *Src1, bool Unsigned,
                bool Saturating) {
    Context.insert<InstARM32Vqmovn2>(Dest, Src0, Src1)
        ->setSignType(Saturating ? (Unsigned ? InstARM32::FS_Unsigned
                                             : InstARM32::FS_Signed)
                                 : InstARM32::FS_None);
  }
  void _vqsub(Variable *Dest, Variable *Src0, Variable *Src1, bool Unsigned) {
    Context.insert<InstARM32Vqsub>(Dest, Src0, Src1)
        ->setSignType(Unsigned ? InstARM32::FS_Unsigned : InstARM32::FS_Signed);
  }
  InstARM32Vshl *_vshl(Variable *Dest, Variable *Src0, Variable *Src1) {
    return Context.insert<InstARM32Vshl>(Dest, Src0, Src1);
  }
  void _vshl(Variable *Dest, Variable *Src0, ConstantInteger32 *Src1) {
    Context.insert<InstARM32Vshl>(Dest, Src0, Src1)
        ->setSignType(InstARM32::FS_Unsigned);
  }
  InstARM32Vshr *_vshr(Variable *Dest, Variable *Src0,
                       ConstantInteger32 *Src1) {
    return Context.insert<InstARM32Vshr>(Dest, Src0, Src1);
  }
  void _vsqrt(Variable *Dest, Variable *Src,
              CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Vsqrt>(Dest, Src, Pred);
  }
  void _vstr1d(Variable *Value, OperandARM32Mem *Addr,
               CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Vstr1>(Value, Addr, Pred, 32);
  }
  void _vstr1q(Variable *Value, OperandARM32Mem *Addr,
               CondARM32::Cond Pred = CondARM32::AL) {
    Context.insert<InstARM32Vstr1>(Value, Addr, Pred, 64);
  }
  void _vsub(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstARM32Vsub>(Dest, Src0, Src1);
  }
  void _vzip(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstARM32Vzip>(Dest, Src0, Src1);
  }

  // Iterates over the CFG and determines the maximum outgoing stack arguments
  // bytes. This information is later used during addProlog() to pre-allocate
  // the outargs area.
  // TODO(jpp): This could live in the Parser, if we provided a Target-specific
  // method that the Parser could call.
  void findMaxStackOutArgsSize();

  /// Returns true if the given Offset can be represented in a Load/Store Mem
  /// Operand.
  bool isLegalMemOffset(Type Ty, int32_t Offset) const;

  void postLowerLegalization();

  /// Manages the Gotoff relocations created during the function lowering. A
  /// single Gotoff relocation is created for each global variable used by the
  /// function being lowered.
  /// @{
  // TODO(jpp): if the same global G is used in different functions, then this
  // method will emit one G(gotoff) relocation per function.
  GlobalString createGotoffRelocation(const ConstantRelocatable *CR);
  CfgUnorderedSet<GlobalString> KnownGotoffs;
  /// @}

  class PostLoweringLegalizer {
    PostLoweringLegalizer() = delete;
    PostLoweringLegalizer(const PostLoweringLegalizer &) = delete;
    PostLoweringLegalizer &operator=(const PostLoweringLegalizer &) = delete;

  public:
    explicit PostLoweringLegalizer(TargetARM32 *Target)
        : Target(Target), StackOrFrameReg(Target->getPhysicalRegister(
                              Target->getFrameOrStackReg())) {}

    void resetTempBaseIfClobberedBy(const Inst *Instr);

    // Ensures that the TempBase register held by the this legalizer (if any) is
    // assigned to IP.
    void assertNoTempOrAssignedToIP() const {
      assert(TempBaseReg == nullptr ||
             TempBaseReg->getRegNum() == Target->getReservedTmpReg());
    }

    // Legalizes Mem. if Mem.Base is a Reamaterializable variable, Mem.Offset is
    // fixed up.
    OperandARM32Mem *legalizeMemOperand(OperandARM32Mem *Mem,
                                        bool AllowOffsets = true);

    /// Legalizes Mov if its Source (or Destination) is a spilled Variable, or
    /// if its Source is a Rematerializable variable (this form is used in lieu
    /// of lea, which is not available in ARM.)
    ///
    /// Moves to memory become store instructions, and moves from memory, loads.
    void legalizeMov(InstARM32Mov *Mov);

  private:
    /// Creates a new Base register centered around [Base, +/- Offset].
    Variable *newBaseRegister(Variable *Base, int32_t Offset,
                              RegNumT ScratchRegNum);

    /// Creates a new, legal OperandARM32Mem for accessing Base + Offset.
    /// The returned mem operand is a legal operand for accessing memory that is
    /// of type Ty.
    ///
    /// If [Base, #Offset] is encodable, then the method returns a Mem operand
    /// expressing it. Otherwise,
    ///
    /// if [TempBaseReg, #Offset-TempBaseOffset] is a valid memory operand, the
    /// method will return that. Otherwise,
    ///
    /// a new base register ip=Base+Offset is created, and the method returns a
    /// memory operand expressing [ip, #0].
    OperandARM32Mem *createMemOperand(Type Ty, Variable *Base, int32_t Offset,
                                      bool AllowOffsets = true);
    TargetARM32 *const Target;
    Variable *const StackOrFrameReg;
    Variable *TempBaseReg = nullptr;
    int32_t TempBaseOffset = 0;
  };

  TargetARM32Features CPUFeatures;
  bool UsesFramePointer = false;
  bool NeedsStackAlignment = false;
  bool MaybeLeafFunc = true;
  size_t SpillAreaSizeBytes = 0;
  size_t FixedAllocaSizeBytes = 0;
  size_t FixedAllocaAlignBytes = 0;
  bool PrologEmitsFixedAllocas = false;
  uint32_t MaxOutArgsSizeBytes = 0;
  // TODO(jpp): std::array instead of array.
  static SmallBitVector TypeToRegisterSet[RegARM32::RCARM32_NUM];
  static SmallBitVector TypeToRegisterSetUnfiltered[RegARM32::RCARM32_NUM];
  static SmallBitVector RegisterAliases[RegARM32::Reg_NUM];
  SmallBitVector RegsUsed;
  VarList PhysicalRegisters[IceType_NUM];
  VarList PreservedGPRs;
  VarList PreservedSRegs;

  /// Helper class that understands the Calling Convention and register
  /// assignments. The first few integer type parameters can use r0-r3,
  /// regardless of their position relative to the floating-point/vector
  /// arguments in the argument list. Floating-point and vector arguments
  /// can use q0-q3 (aka d0-d7, s0-s15). For more information on the topic,
  /// see the ARM Architecture Procedure Calling Standards (AAPCS).
  ///
  /// Technically, arguments that can start with registers but extend beyond the
  /// available registers can be split between the registers and the stack.
  /// However, this is typically  for passing GPR structs by value, and PNaCl
  /// transforms expand this out.
  ///
  /// At (public) function entry, the stack must be 8-byte aligned.
  class CallingConv {
    CallingConv(const CallingConv &) = delete;
    CallingConv &operator=(const CallingConv &) = delete;

  public:
    CallingConv();
    ~CallingConv() = default;

    /// argInGPR returns true if there is a GPR available for the requested
    /// type, and false otherwise. If it returns true, Reg is set to the
    /// appropriate register number. Note that, when Ty == IceType_i64, Reg will
    /// be an I64 register pair.
    bool argInGPR(Type Ty, RegNumT *Reg);

    /// argInVFP is to floating-point/vector types what argInGPR is for integer
    /// types.
    bool argInVFP(Type Ty, RegNumT *Reg);

  private:
    void discardUnavailableGPRsAndTheirAliases(CfgVector<RegNumT> *Regs);
    SmallBitVector GPRegsUsed;
    CfgVector<RegNumT> GPRArgs;
    CfgVector<RegNumT> I64Args;

    void discardUnavailableVFPRegs(CfgVector<RegNumT> *Regs);
    SmallBitVector VFPRegsUsed;
    CfgVector<RegNumT> FP32Args;
    CfgVector<RegNumT> FP64Args;
    CfgVector<RegNumT> Vec128Args;
  };

private:
  ENABLE_MAKE_UNIQUE;

  OperandARM32Mem *formAddressingMode(Type Ty, Cfg *Func, const Inst *LdSt,
                                      Operand *Base);

  void postambleCtpop64(const InstCall *Instr);
  void preambleDivRem(const InstCall *Instr);
  CfgUnorderedMap<Operand *, void (TargetARM32::*)(const InstCall *Instr)>
      ARM32HelpersPreamble;
  CfgUnorderedMap<Operand *, void (TargetARM32::*)(const InstCall *Instr)>
      ARM32HelpersPostamble;

  class ComputationTracker {
  public:
    ComputationTracker() = default;
    ~ComputationTracker() = default;

    void forgetProducers() { KnownComputations.clear(); }
    void recordProducers(CfgNode *Node);

    const Inst *getProducerOf(const Operand *Opnd) const {
      auto *Var = llvm::dyn_cast<Variable>(Opnd);
      if (Var == nullptr) {
        return nullptr;
      }

      auto Iter = KnownComputations.find(Var->getIndex());
      if (Iter == KnownComputations.end()) {
        return nullptr;
      }

      return Iter->second.Instr;
    }

    void dump(const Cfg *Func) const {
      if (!BuildDefs::dump() || !Func->isVerbose(IceV_Folding))
        return;
      OstreamLocker L(Func->getContext());
      Ostream &Str = Func->getContext()->getStrDump();
      Str << "foldable producer:\n";
      for (const auto &Computation : KnownComputations) {
        Str << "    ";
        Computation.second.Instr->dump(Func);
        Str << "\n";
      }
      Str << "\n";
    }

  private:
    class ComputationEntry {
    public:
      ComputationEntry(Inst *I, Type Ty) : Instr(I), ComputationType(Ty) {}
      Inst *const Instr;
      // Boolean folding is disabled for variables whose live range is multi
      // block. We conservatively initialize IsLiveOut to true, and set it to
      // false once we find the end of the live range for the variable defined
      // by this instruction. If liveness analysis is not performed (e.g., in
      // Om1 mode) IsLiveOut will never be set to false, and folding will be
      // disabled.
      bool IsLiveOut = true;
      int32_t NumUses = 0;
      Type ComputationType;
    };

    // ComputationMap maps a Variable number to a payload identifying which
    // instruction defined it.
    using ComputationMap = CfgUnorderedMap<SizeT, ComputationEntry>;
    ComputationMap KnownComputations;
  };

  ComputationTracker Computations;

  // AllowTemporaryWithNoReg indicates if TargetARM32::makeReg() can be invoked
  // without specifying a physical register. This is needed for creating unbound
  // temporaries during Ice -> ARM lowering, but before register allocation.
  // This a safe-guard that no unbound temporaries are created during the
  // legalization post-passes.
  bool AllowTemporaryWithNoReg = true;
  // ForbidTemporaryWithoutReg is a RAII class that manages
  // AllowTemporaryWithNoReg.
  class ForbidTemporaryWithoutReg {
    ForbidTemporaryWithoutReg() = delete;
    ForbidTemporaryWithoutReg(const ForbidTemporaryWithoutReg &) = delete;
    ForbidTemporaryWithoutReg &
    operator=(const ForbidTemporaryWithoutReg &) = delete;

  public:
    explicit ForbidTemporaryWithoutReg(TargetARM32 *Target) : Target(Target) {
      Target->AllowTemporaryWithNoReg = false;
    }
    ~ForbidTemporaryWithoutReg() { Target->AllowTemporaryWithNoReg = true; }

  private:
    TargetARM32 *const Target;
  };
};

class TargetDataARM32 final : public TargetDataLowering {
  TargetDataARM32() = delete;
  TargetDataARM32(const TargetDataARM32 &) = delete;
  TargetDataARM32 &operator=(const TargetDataARM32 &) = delete;

public:
  static std::unique_ptr<TargetDataLowering> create(GlobalContext *Ctx) {
    return std::unique_ptr<TargetDataLowering>(new TargetDataARM32(Ctx));
  }

  void lowerGlobals(const VariableDeclarationList &Vars,
                    const std::string &SectionSuffix) override;
  void lowerConstants() override;
  void lowerJumpTables() override;

protected:
  explicit TargetDataARM32(GlobalContext *Ctx);

private:
  ~TargetDataARM32() override = default;
};

class TargetHeaderARM32 final : public TargetHeaderLowering {
  TargetHeaderARM32() = delete;
  TargetHeaderARM32(const TargetHeaderARM32 &) = delete;
  TargetHeaderARM32 &operator=(const TargetHeaderARM32 &) = delete;

public:
  static std::unique_ptr<TargetHeaderLowering> create(GlobalContext *Ctx) {
    return std::unique_ptr<TargetHeaderLowering>(new TargetHeaderARM32(Ctx));
  }

  void lower() override;

protected:
  explicit TargetHeaderARM32(GlobalContext *Ctx);

private:
  ~TargetHeaderARM32() = default;

  TargetARM32Features CPUFeatures;
};

} // end of namespace ARM32
} // end of namespace Ice

#endif // SUBZERO_SRC_ICETARGETLOWERINGARM32_H
