//===- subzero/src/IceTargetLoweringMIPS32.h - MIPS32 lowering ---*- C++-*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the TargetLoweringMIPS32 class, which implements the
/// TargetLowering interface for the MIPS 32-bit architecture.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICETARGETLOWERINGMIPS32_H
#define SUBZERO_SRC_ICETARGETLOWERINGMIPS32_H

#include "IceAssemblerMIPS32.h"
#include "IceDefs.h"
#include "IceInstMIPS32.h"
#include "IceRegistersMIPS32.h"
#include "IceTargetLowering.h"

namespace Ice {
namespace MIPS32 {

class TargetMIPS32 : public TargetLowering {
  TargetMIPS32() = delete;
  TargetMIPS32(const TargetMIPS32 &) = delete;
  TargetMIPS32 &operator=(const TargetMIPS32 &) = delete;

public:
  ~TargetMIPS32() override = default;

  static void staticInit(GlobalContext *Ctx);
  static bool shouldBePooled(const Constant *C) {
    if (auto *ConstDouble = llvm::dyn_cast<ConstantDouble>(C)) {
      return !Utils::isPositiveZero(ConstDouble->getValue());
    }
    if (auto *ConstFloat = llvm::dyn_cast<ConstantFloat>(C)) {
      return !Utils::isPositiveZero(ConstFloat->getValue());
    }
    return false;
  }
  static ::Ice::Type getPointerType() { return ::Ice::IceType_i32; }
  static std::unique_ptr<::Ice::TargetLowering> create(Cfg *Func) {
    return makeUnique<TargetMIPS32>(Func);
  }

  std::unique_ptr<::Ice::Assembler> createAssembler() const override {
    return makeUnique<MIPS32::AssemblerMIPS32>();
  }

  void initNodeForLowering(CfgNode *Node) override {
    Computations.forgetProducers();
    Computations.recordProducers(Node);
    Computations.dump(Func);
  }

  void translateOm1() override;
  void translateO2() override;
  bool doBranchOpt(Inst *Instr, const CfgNode *NextNode) override;
  void setImplicitRet(Variable *Ret) { ImplicitRet = Ret; }
  Variable *getImplicitRet() const { return ImplicitRet; }
  SizeT getNumRegisters() const override { return RegMIPS32::Reg_NUM; }
  Variable *getPhysicalRegister(RegNumT RegNum,
                                Type Ty = IceType_void) override;
  const char *getRegName(RegNumT RegNum, Type Ty) const override;
  SmallBitVector getRegisterSet(RegSetMask Include,
                                RegSetMask Exclude) const override;
  const SmallBitVector &
  getRegistersForVariable(const Variable *Var) const override {
    RegClass RC = Var->getRegClass();
    assert(RC < RC_Target);
    return TypeToRegisterSet[RC];
  }
  const SmallBitVector &
  getAllRegistersForVariable(const Variable *Var) const override {
    RegClass RC = Var->getRegClass();
    assert(RC < RC_Target);
    return TypeToRegisterSetUnfiltered[RC];
  }
  const SmallBitVector &getAliasesForRegister(RegNumT Reg) const override {
    return RegisterAliases[Reg];
  }
  bool hasFramePointer() const override { return UsesFramePointer; }
  void setHasFramePointer() override { UsesFramePointer = true; }
  RegNumT getStackReg() const override { return RegMIPS32::Reg_SP; }
  RegNumT getFrameReg() const override { return RegMIPS32::Reg_FP; }
  RegNumT getFrameOrStackReg() const override {
    return UsesFramePointer ? getFrameReg() : getStackReg();
  }
  RegNumT getReservedTmpReg() const { return RegMIPS32::Reg_AT; }
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
    int32_t FixedAllocaOffset =
        Utils::applyAlignment(CurrentAllocaOffset, FixedAllocaAlignBytes);
    return FixedAllocaOffset - MaxOutArgsSizeBytes;
  }

  uint32_t maxOutArgsSizeBytes() const override { return MaxOutArgsSizeBytes; }

  uint32_t getFramePointerOffset(uint32_t CurrentOffset,
                                 uint32_t Size) const override {
    (void)Size;
    return CurrentOffset + MaxOutArgsSizeBytes;
  }

  bool shouldSplitToVariable64On32(Type Ty) const override {
    return Ty == IceType_i64;
  }

  bool shouldSplitToVariableVecOn32(Type Ty) const override {
    return isVectorType(Ty);
  }

  // TODO(ascull): what is the best size of MIPS?
  SizeT getMinJumpTableSize() const override { return 3; }
  void emitJumpTable(const Cfg *Func,
                     const InstJumpTable *JumpTable) const override;

  void emitVariable(const Variable *Var) const override;

  void emit(const ConstantInteger32 *C) const final {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Ctx->getStrEmit();
    Str << C->getValue();
  }
  void emit(const ConstantInteger64 *C) const final {
    (void)C;
    llvm::report_fatal_error("Not yet implemented");
  }
  void emit(const ConstantFloat *C) const final {
    (void)C;
    llvm::report_fatal_error("Not yet implemented");
  }
  void emit(const ConstantDouble *C) const final {
    (void)C;
    llvm::report_fatal_error("Not yet implemented");
  }
  void emit(const ConstantUndef *C) const final {
    (void)C;
    llvm::report_fatal_error("Not yet implemented");
  }
  void emit(const ConstantRelocatable *C) const final {
    (void)C;
    llvm::report_fatal_error("Not yet implemented");
  }

  // The following are helpers that insert lowered MIPS32 instructions with
  // minimal syntactic overhead, so that the lowering code can look as close to
  // assembly as practical.
  void _add(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Add>(Dest, Src0, Src1);
  }

  void _addu(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Addu>(Dest, Src0, Src1);
  }

  void _and(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32And>(Dest, Src0, Src1);
  }

  void _andi(Variable *Dest, Variable *Src, uint32_t Imm) {
    Context.insert<InstMIPS32Andi>(Dest, Src, Imm);
  }

  void _br(CfgNode *Target) { Context.insert<InstMIPS32Br>(Target); }

  void _br(CfgNode *Target, const InstMIPS32Label *Label) {
    Context.insert<InstMIPS32Br>(Target, Label);
  }

  void _br(CfgNode *TargetTrue, CfgNode *TargetFalse, Operand *Src0,
           Operand *Src1, CondMIPS32::Cond Condition) {
    Context.insert<InstMIPS32Br>(TargetTrue, TargetFalse, Src0, Src1,
                                 Condition);
  }

  void _br(CfgNode *TargetTrue, CfgNode *TargetFalse, Operand *Src0,
           CondMIPS32::Cond Condition) {
    Context.insert<InstMIPS32Br>(TargetTrue, TargetFalse, Src0, Condition);
  }

  void _br(CfgNode *TargetTrue, CfgNode *TargetFalse, Operand *Src0,
           Operand *Src1, const InstMIPS32Label *Label,
           CondMIPS32::Cond Condition) {
    Context.insert<InstMIPS32Br>(TargetTrue, TargetFalse, Src0, Src1, Label,
                                 Condition);
  }

  void _ret(Variable *RA, Variable *Src0 = nullptr) {
    Context.insert<InstMIPS32Ret>(RA, Src0);
  }

  void _abs_d(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Abs_d>(Dest, Src);
  }

  void _abs_s(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Abs_s>(Dest, Src);
  }

  void _addi(Variable *Dest, Variable *Src, uint32_t Imm) {
    Context.insert<InstMIPS32Addi>(Dest, Src, Imm);
  }

  void _add_d(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Add_d>(Dest, Src0, Src1);
  }

  void _add_s(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Add_s>(Dest, Src0, Src1);
  }

  void _addiu(Variable *Dest, Variable *Src, uint32_t Imm) {
    Context.insert<InstMIPS32Addiu>(Dest, Src, Imm);
  }

  void _addiu(Variable *Dest, Variable *Src0, Operand *Src1, RelocOp Reloc) {
    Context.insert<InstMIPS32Addiu>(Dest, Src0, Src1, Reloc);
  }

  void _c_eq_d(Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32C_eq_d>(Src0, Src1);
  }

  void _c_eq_s(Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32C_eq_s>(Src0, Src1);
  }

  void _c_ole_d(Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32C_ole_d>(Src0, Src1);
  }

  void _c_ole_s(Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32C_ole_s>(Src0, Src1);
  }

  void _c_olt_d(Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32C_olt_d>(Src0, Src1);
  }

  void _c_olt_s(Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32C_olt_s>(Src0, Src1);
  }

  void _c_ueq_d(Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32C_ueq_d>(Src0, Src1);
  }

  void _c_ueq_s(Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32C_ueq_s>(Src0, Src1);
  }

  void _c_ule_d(Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32C_ule_d>(Src0, Src1);
  }

  void _c_ule_s(Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32C_ule_s>(Src0, Src1);
  }

  void _c_ult_d(Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32C_ult_d>(Src0, Src1);
  }

  void _c_ult_s(Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32C_ult_s>(Src0, Src1);
  }

  void _c_un_d(Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32C_un_d>(Src0, Src1);
  }

  void _c_un_s(Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32C_un_s>(Src0, Src1);
  }

  void _clz(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Clz>(Dest, Src);
  }

  void _cvt_d_l(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Cvt_d_l>(Dest, Src);
  }

  void _cvt_d_s(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Cvt_d_s>(Dest, Src);
  }

  void _cvt_d_w(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Cvt_d_w>(Dest, Src);
  }

  void _cvt_s_d(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Cvt_s_d>(Dest, Src);
  }

  void _cvt_s_l(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Cvt_s_l>(Dest, Src);
  }

  void _cvt_s_w(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Cvt_s_w>(Dest, Src);
  }

  void _div(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Div>(Dest, Src0, Src1);
  }

  void _div_d(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Div_d>(Dest, Src0, Src1);
  }

  void _div_s(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Div_s>(Dest, Src0, Src1);
  }

  void _divu(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Divu>(Dest, Src0, Src1);
  }

  void _ldc1(Variable *Value, OperandMIPS32Mem *Mem, RelocOp Reloc = RO_No) {
    Context.insert<InstMIPS32Ldc1>(Value, Mem, Reloc);
  }

  void _ll(Variable *Value, OperandMIPS32Mem *Mem) {
    Context.insert<InstMIPS32Ll>(Value, Mem);
  }

  void _lw(Variable *Value, OperandMIPS32Mem *Mem) {
    Context.insert<InstMIPS32Lw>(Value, Mem);
  }

  void _lwc1(Variable *Value, OperandMIPS32Mem *Mem, RelocOp Reloc = RO_No) {
    Context.insert<InstMIPS32Lwc1>(Value, Mem, Reloc);
  }

  void _lui(Variable *Dest, Operand *Src, RelocOp Reloc = RO_No) {
    Context.insert<InstMIPS32Lui>(Dest, Src, Reloc);
  }

  void _mfc1(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Mfc1>(Dest, Src);
  }

  void _mfhi(Variable *Dest, Operand *Src) {
    Context.insert<InstMIPS32Mfhi>(Dest, Src);
  }

  void _mflo(Variable *Dest, Operand *Src) {
    Context.insert<InstMIPS32Mflo>(Dest, Src);
  }

  void _mov(Variable *Dest, Operand *Src0, Operand *Src1 = nullptr) {
    assert(Dest != nullptr);
    // Variable* Src0_ = llvm::dyn_cast<Variable>(Src0);
    if (llvm::isa<ConstantRelocatable>(Src0)) {
      Context.insert<InstMIPS32La>(Dest, Src0);
    } else {
      auto *Instr = Context.insert<InstMIPS32Mov>(Dest, Src0, Src1);
      if (Instr->getDestHi() != nullptr) {
        // If DestHi is available, then Dest must be a Variable64On32. We add a
        // fake-def for Instr.DestHi here.
        assert(llvm::isa<Variable64On32>(Dest));
        Context.insert<InstFakeDef>(Instr->getDestHi());
      }
    }
  }

  void _mov_redefined(Variable *Dest, Operand *Src0, Operand *Src1 = nullptr) {
    if (llvm::isa<ConstantRelocatable>(Src0)) {
      Context.insert<InstMIPS32La>(Dest, Src0);
    } else {
      auto *Instr = Context.insert<InstMIPS32Mov>(Dest, Src0, Src1);
      Instr->setDestRedefined();
      if (Instr->getDestHi() != nullptr) {
        // If Instr is multi-dest, then Dest must be a Variable64On32. We add a
        // fake-def for Instr.DestHi here.
        assert(llvm::isa<Variable64On32>(Dest));
        Context.insert<InstFakeDef>(Instr->getDestHi());
      }
    }
  }

  void _mov_fp64_to_i64(Variable *Dest, Operand *Src, Int64Part Int64HiLo) {
    assert(Dest != nullptr);
    Context.insert<InstMIPS32MovFP64ToI64>(Dest, Src, Int64HiLo);
  }

  void _mov_d(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Mov_d>(Dest, Src);
  }

  void _mov_s(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Mov_s>(Dest, Src);
  }

  void _movf(Variable *Dest, Variable *Src0, Operand *FCC) {
    Context.insert<InstMIPS32Movf>(Dest, Src0, FCC)->setDestRedefined();
  }

  void _movn(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Movn>(Dest, Src0, Src1)->setDestRedefined();
  }

  void _movn_d(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Movn_d>(Dest, Src0, Src1)->setDestRedefined();
  }

  void _movn_s(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Movn_s>(Dest, Src0, Src1)->setDestRedefined();
  }

  void _movt(Variable *Dest, Variable *Src0, Operand *FCC) {
    Context.insert<InstMIPS32Movt>(Dest, Src0, FCC)->setDestRedefined();
  }

  void _movz(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Movz>(Dest, Src0, Src1)->setDestRedefined();
  }

  void _movz_d(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Movz_d>(Dest, Src0, Src1)->setDestRedefined();
  }

  void _movz_s(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Movz_s>(Dest, Src0, Src1)->setDestRedefined();
  }

  void _mtc1(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Mtc1>(Dest, Src);
  }

  void _mthi(Variable *Dest, Operand *Src) {
    Context.insert<InstMIPS32Mthi>(Dest, Src);
  }

  void _mtlo(Variable *Dest, Operand *Src) {
    Context.insert<InstMIPS32Mtlo>(Dest, Src);
  }

  void _mul(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Mul>(Dest, Src0, Src1);
  }

  void _mul_d(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Mul_d>(Dest, Src0, Src1);
  }

  void _mul_s(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Mul_s>(Dest, Src0, Src1);
  }

  void _mult(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Mult>(Dest, Src0, Src1);
  }

  void _multu(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Multu>(Dest, Src0, Src1);
  }

  void _nop() { Context.insert<InstMIPS32Sll>(getZero(), getZero(), 0); }

  void _nor(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Nor>(Dest, Src0, Src1);
  }

  void _not(Variable *Dest, Variable *Src0) {
    Context.insert<InstMIPS32Nor>(Dest, Src0, getZero());
  }

  void _or(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Or>(Dest, Src0, Src1);
  }

  void _ori(Variable *Dest, Variable *Src, uint32_t Imm) {
    Context.insert<InstMIPS32Ori>(Dest, Src, Imm);
  }

  InstMIPS32Sc *_sc(Variable *Value, OperandMIPS32Mem *Mem) {
    return Context.insert<InstMIPS32Sc>(Value, Mem);
  }

  void _sdc1(Variable *Value, OperandMIPS32Mem *Mem) {
    Context.insert<InstMIPS32Sdc1>(Value, Mem);
  }

  void _sll(Variable *Dest, Variable *Src, uint32_t Imm) {
    Context.insert<InstMIPS32Sll>(Dest, Src, Imm);
  }

  void _sllv(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Sllv>(Dest, Src0, Src1);
  }

  void _slt(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Slt>(Dest, Src0, Src1);
  }

  void _slti(Variable *Dest, Variable *Src, uint32_t Imm) {
    Context.insert<InstMIPS32Slti>(Dest, Src, Imm);
  }

  void _sltiu(Variable *Dest, Variable *Src, uint32_t Imm) {
    Context.insert<InstMIPS32Sltiu>(Dest, Src, Imm);
  }

  void _sltu(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Sltu>(Dest, Src0, Src1);
  }

  void _sqrt_d(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Sqrt_d>(Dest, Src);
  }

  void _sqrt_s(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Sqrt_s>(Dest, Src);
  }

  void _sra(Variable *Dest, Variable *Src, uint32_t Imm) {
    Context.insert<InstMIPS32Sra>(Dest, Src, Imm);
  }

  void _srav(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Srav>(Dest, Src0, Src1);
  }

  void _srl(Variable *Dest, Variable *Src, uint32_t Imm) {
    Context.insert<InstMIPS32Srl>(Dest, Src, Imm);
  }

  void _srlv(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Srlv>(Dest, Src0, Src1);
  }

  void _sub(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Sub>(Dest, Src0, Src1);
  }

  void _sub_d(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Sub_d>(Dest, Src0, Src1);
  }

  void _sub_s(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Sub_s>(Dest, Src0, Src1);
  }

  void _subu(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Subu>(Dest, Src0, Src1);
  }

  void _sw(Variable *Value, OperandMIPS32Mem *Mem) {
    Context.insert<InstMIPS32Sw>(Value, Mem);
  }

  void _swc1(Variable *Value, OperandMIPS32Mem *Mem) {
    Context.insert<InstMIPS32Swc1>(Value, Mem);
  }

  void _sync() { Context.insert<InstMIPS32Sync>(); }

  void _teq(Variable *Src0, Variable *Src1, uint32_t TrapCode) {
    Context.insert<InstMIPS32Teq>(Src0, Src1, TrapCode);
  }

  void _trunc_l_d(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Trunc_l_d>(Dest, Src);
  }

  void _trunc_l_s(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Trunc_l_s>(Dest, Src);
  }

  void _trunc_w_d(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Trunc_w_d>(Dest, Src);
  }

  void _trunc_w_s(Variable *Dest, Variable *Src) {
    Context.insert<InstMIPS32Trunc_w_s>(Dest, Src);
  }

  void _xor(Variable *Dest, Variable *Src0, Variable *Src1) {
    Context.insert<InstMIPS32Xor>(Dest, Src0, Src1);
  }

  void _xori(Variable *Dest, Variable *Src, uint32_t Imm) {
    Context.insert<InstMIPS32Xori>(Dest, Src, Imm);
  }

  void lowerArguments() override;

  /// Make a pass through the SortedSpilledVariables and actually assign stack
  /// slots. SpillAreaPaddingBytes takes into account stack alignment padding.
  /// The SpillArea starts after that amount of padding. This matches the scheme
  /// in getVarStackSlotParams, where there may be a separate multi-block global
  /// var spill area and a local var spill area.
  void assignVarStackSlots(VarList &SortedSpilledVariables,
                           size_t SpillAreaPaddingBytes,
                           size_t SpillAreaSizeBytes,
                           size_t GlobalsAndSubsequentPaddingSize);

  /// Operand legalization helpers.  To deal with address mode constraints,
  /// the helpers will create a new Operand and emit instructions that
  /// guarantee that the Operand kind is one of those indicated by the
  /// LegalMask (a bitmask of allowed kinds).  If the input Operand is known
  /// to already meet the constraints, it may be simply returned as the result,
  /// without creating any new instructions or operands.
  enum OperandLegalization {
    Legal_None = 0,
    Legal_Reg = 1 << 0, // physical register, not stack location
    Legal_Imm = 1 << 1,
    Legal_Mem = 1 << 2,
    Legal_Rematerializable = 1 << 3,
    Legal_Default = ~Legal_None
  };
  typedef uint32_t LegalMask;
  Operand *legalize(Operand *From, LegalMask Allowed = Legal_Default,
                    RegNumT RegNum = RegNumT());

  Variable *legalizeToVar(Operand *From, RegNumT RegNum = RegNumT());

  Variable *legalizeToReg(Operand *From, RegNumT RegNum = RegNumT());

  Variable *makeReg(Type Ty, RegNumT RegNum = RegNumT());

  Variable *getZero() {
    auto *Zero = makeReg(IceType_i32, RegMIPS32::Reg_ZERO);
    Context.insert<InstFakeDef>(Zero);
    return Zero;
  }

  Variable *I32Reg(RegNumT RegNum = RegNumT()) {
    return makeReg(IceType_i32, RegNum);
  }

  Variable *F32Reg(RegNumT RegNum = RegNumT()) {
    return makeReg(IceType_f32, RegNum);
  }

  Variable *F64Reg(RegNumT RegNum = RegNumT()) {
    return makeReg(IceType_f64, RegNum);
  }

  static Type stackSlotType();
  Variable *copyToReg(Operand *Src, RegNumT RegNum = RegNumT());

  void unsetIfNonLeafFunc();

  // Iterates over the CFG and determines the maximum outgoing stack arguments
  // bytes. This information is later used during addProlog() to pre-allocate
  // the outargs area
  void findMaxStackOutArgsSize();

  void postLowerLegalization();

  void addProlog(CfgNode *Node) override;
  void addEpilog(CfgNode *Node) override;

  // Ensure that a 64-bit Variable has been split into 2 32-bit
  // Variables, creating them if necessary.  This is needed for all
  // I64 operations.
  void split64(Variable *Var);
  Operand *loOperand(Operand *Operand);
  Operand *hiOperand(Operand *Operand);
  Operand *getOperandAtIndex(Operand *Operand, Type BaseType, uint32_t Index);

  void finishArgumentLowering(Variable *Arg, bool PartialOnStack,
                              Variable *FramePtr, size_t BasicFrameOffset,
                              size_t *InArgsSizeBytes);

  Operand *legalizeUndef(Operand *From, RegNumT RegNum = RegNumT());

  /// Helper class that understands the Calling Convention and register
  /// assignments as per MIPS O32 abi.
  class CallingConv {
    CallingConv(const CallingConv &) = delete;
    CallingConv &operator=(const CallingConv &) = delete;

  public:
    CallingConv();
    ~CallingConv() = default;

    /// argInReg returns true if there is a Register available for the requested
    /// type, and false otherwise. If it returns true, Reg is set to the
    /// appropriate register number. Note that, when Ty == IceType_i64, Reg will
    /// be an I64 register pair.
    bool argInReg(Type Ty, uint32_t ArgNo, RegNumT *Reg);
    void discardReg(RegNumT Reg) { GPRegsUsed |= RegisterAliases[Reg]; }

  private:
    // argInGPR is used to find if any GPR register is available for argument of
    // type Ty
    bool argInGPR(Type Ty, RegNumT *Reg);
    /// argInVFP is to floating-point/vector types what argInGPR is for integer
    /// types.
    bool argInVFP(Type Ty, RegNumT *Reg);
    inline void discardNextGPRAndItsAliases(CfgVector<RegNumT> *Regs);
    inline void alignGPR(CfgVector<RegNumT> *Regs);
    void discardUnavailableGPRsAndTheirAliases(CfgVector<RegNumT> *Regs);
    SmallBitVector GPRegsUsed;
    CfgVector<RegNumT> GPRArgs;
    CfgVector<RegNumT> I64Args;

    void discardUnavailableVFPRegsAndTheirAliases(CfgVector<RegNumT> *Regs);
    SmallBitVector VFPRegsUsed;
    CfgVector<RegNumT> FP32Args;
    CfgVector<RegNumT> FP64Args;
    // UseFPRegs is a flag indicating if FP registers can be used
    bool UseFPRegs = false;
  };

protected:
  explicit TargetMIPS32(Cfg *Func);

  void postLower() override;

  void lowerAlloca(const InstAlloca *Instr) override;
  void lowerArithmetic(const InstArithmetic *Instr) override;
  void lowerInt64Arithmetic(const InstArithmetic *Instr, Variable *Dest,
                            Operand *Src0, Operand *Src1);
  void lowerAssign(const InstAssign *Instr) override;
  void lowerBr(const InstBr *Instr) override;
  void lowerBreakpoint(const InstBreakpoint *Instr) override;
  void lowerCall(const InstCall *Instr) override;
  void lowerCast(const InstCast *Instr) override;
  void lowerExtractElement(const InstExtractElement *Instr) override;
  void lowerFcmp(const InstFcmp *Instr) override;
  void lowerIcmp(const InstIcmp *Instr) override;
  void lower64Icmp(const InstIcmp *Instr);
  void createArithInst(Intrinsics::AtomicRMWOperation Operation, Variable *Dest,
                       Variable *Src0, Variable *Src1);
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
  void lowerOther(const Inst *Instr) override;
  void prelowerPhis() override;
  uint32_t getCallStackArgumentsSizeBytes(const InstCall *Instr) override;
  void genTargetHelperCallFor(Inst *Instr) override;
  void doAddressOptLoad() override;
  void doAddressOptStore() override;

  OperandMIPS32Mem *formMemoryOperand(Operand *Ptr, Type Ty);

  class PostLoweringLegalizer {
    PostLoweringLegalizer() = delete;
    PostLoweringLegalizer(const PostLoweringLegalizer &) = delete;
    PostLoweringLegalizer &operator=(const PostLoweringLegalizer &) = delete;

  public:
    explicit PostLoweringLegalizer(TargetMIPS32 *Target)
        : Target(Target), StackOrFrameReg(Target->getPhysicalRegister(
                              Target->getFrameOrStackReg())) {}

    /// Legalizes Mem. if Mem.Base is a rematerializable variable,
    /// Mem.Offset is fixed up.
    OperandMIPS32Mem *legalizeMemOperand(OperandMIPS32Mem *Mem);

    /// Legalizes Immediate if larger value overflows range of 16 bits
    Variable *legalizeImmediate(int32_t Imm);

    /// Legalizes Mov if its Source (or Destination) is a spilled Variable, or
    /// if its Source is a Rematerializable variable (this form is used in lieu
    /// of lea, which is not available in MIPS.)
    ///
    /// Moves to memory become store instructions, and moves from memory, loads.
    void legalizeMov(InstMIPS32Mov *Mov);
    void legalizeMovFp(InstMIPS32MovFP64ToI64 *MovInstr);

  private:
    /// Creates a new Base register centered around [Base, +/- Offset].
    Variable *newBaseRegister(Variable *Base, int32_t Offset,
                              RegNumT ScratchRegNum);

    TargetMIPS32 *const Target;
    Variable *const StackOrFrameReg;
  };

  bool UsesFramePointer = false;
  bool NeedsStackAlignment = false;
  bool MaybeLeafFunc = true;
  bool PrologEmitsFixedAllocas = false;
  bool VariableAllocaUsed = false;
  uint32_t MaxOutArgsSizeBytes = 0;
  uint32_t TotalStackSizeBytes = 0;
  uint32_t CurrentAllocaOffset = 0;
  uint32_t VariableAllocaAlignBytes = 0;
  static SmallBitVector TypeToRegisterSet[RCMIPS32_NUM];
  static SmallBitVector TypeToRegisterSetUnfiltered[RCMIPS32_NUM];
  static SmallBitVector RegisterAliases[RegMIPS32::Reg_NUM];
  SmallBitVector RegsUsed;
  VarList PhysicalRegisters[IceType_NUM];
  VarList PreservedGPRs;
  static constexpr uint32_t CHAR_BITS = 8;
  static constexpr uint32_t INT32_BITS = 32;
  size_t SpillAreaSizeBytes = 0;
  size_t FixedAllocaSizeBytes = 0;
  size_t FixedAllocaAlignBytes = 0;
  size_t PreservedRegsSizeBytes = 0;
  Variable *ImplicitRet = nullptr; /// Implicit return

private:
  ENABLE_MAKE_UNIQUE;

  OperandMIPS32Mem *formAddressingMode(Type Ty, Cfg *Func, const Inst *LdSt,
                                       Operand *Base);

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
};

class TargetDataMIPS32 final : public TargetDataLowering {
  TargetDataMIPS32() = delete;
  TargetDataMIPS32(const TargetDataMIPS32 &) = delete;
  TargetDataMIPS32 &operator=(const TargetDataMIPS32 &) = delete;

public:
  static std::unique_ptr<TargetDataLowering> create(GlobalContext *Ctx) {
    return std::unique_ptr<TargetDataLowering>(new TargetDataMIPS32(Ctx));
  }

  void lowerGlobals(const VariableDeclarationList &Vars,
                    const std::string &SectionSuffix) override;
  void lowerConstants() override;
  void lowerJumpTables() override;
  void emitTargetRODataSections() override;

protected:
  explicit TargetDataMIPS32(GlobalContext *Ctx);

private:
  ~TargetDataMIPS32() override = default;
};

class TargetHeaderMIPS32 final : public TargetHeaderLowering {
  TargetHeaderMIPS32() = delete;
  TargetHeaderMIPS32(const TargetHeaderMIPS32 &) = delete;
  TargetHeaderMIPS32 &operator=(const TargetHeaderMIPS32 &) = delete;

public:
  static std::unique_ptr<TargetHeaderLowering> create(GlobalContext *Ctx) {
    return std::unique_ptr<TargetHeaderLowering>(new TargetHeaderMIPS32(Ctx));
  }

  void lower() override;

protected:
  explicit TargetHeaderMIPS32(GlobalContext *Ctx);

private:
  ~TargetHeaderMIPS32() = default;
};

// This structure (with some minor modifications) is copied from
// llvm/lib/Target/Mips/MCTargetDesc/MipsABIFlagsSection.h file.
struct MipsABIFlagsSection {

  // Version of the MIPS.abiflags section
  enum AFL_VERSION {
    AFL_VERSION_V0 = 0 // Version 0
  };

  // The level of the ISA: 1-5, 32, 64.
  enum AFL_ISA_LEVEL {
    AFL_ISA_LEVEL_NONE = 0,
    AFL_ISA_LEVEL_MIPS32 = 32, // MIPS32
  };

  // The revision of ISA: 0 for MIPS V and below, 1-n otherwise.
  enum AFL_ISA_REV {
    AFL_ISA_REV_NONE = 0,
    AFL_ISA_REV_R1 = 1, // R1
  };

  // Values for the xxx_size bytes of an ABI flags structure.
  enum AFL_REG {
    AFL_REG_NONE = 0x00, // No registers.
    AFL_REG_32 = 0x01,   // 32-bit registers.
    AFL_REG_64 = 0x02,   // 64-bit registers.
    AFL_REG_128 = 0x03   // 128-bit registers.
  };

  // Values for the fp_abi word of an ABI flags structure.
  enum AFL_FP_ABI {
    AFL_FP_ANY = 0,
    AFL_FP_DOUBLE = 1,
    AFL_FP_XX = 5,
    AFL_FP_64 = 6,
    AFL_FP_64A = 7
  };

  // Values for the isa_ext word of an ABI flags structure.
  enum AFL_EXT {
    AFL_EXT_NONE = 0,
    AFL_EXT_XLR = 1,          // RMI Xlr instruction.
    AFL_EXT_OCTEON2 = 2,      // Cavium Networks Octeon2.
    AFL_EXT_OCTEONP = 3,      // Cavium Networks OcteonP.
    AFL_EXT_LOONGSON_3A = 4,  // Loongson 3A.
    AFL_EXT_OCTEON = 5,       // Cavium Networks Octeon.
    AFL_EXT_5900 = 6,         // MIPS R5900 instruction.
    AFL_EXT_4650 = 7,         // MIPS R4650 instruction.
    AFL_EXT_4010 = 8,         // LSI R4010 instruction.
    AFL_EXT_4100 = 9,         // NEC VR4100 instruction.
    AFL_EXT_3900 = 10,        // Toshiba R3900 instruction.
    AFL_EXT_10000 = 11,       // MIPS R10000 instruction.
    AFL_EXT_SB1 = 12,         // Broadcom SB-1 instruction.
    AFL_EXT_4111 = 13,        // NEC VR4111/VR4181 instruction.
    AFL_EXT_4120 = 14,        // NEC VR4120 instruction.
    AFL_EXT_5400 = 15,        // NEC VR5400 instruction.
    AFL_EXT_5500 = 16,        // NEC VR5500 instruction.
    AFL_EXT_LOONGSON_2E = 17, // ST Microelectronics Loongson 2E.
    AFL_EXT_LOONGSON_2F = 18  // ST Microelectronics Loongson 2F.
  };

  // Masks for the ases word of an ABI flags structure.
  enum AFL_ASE {
    AFL_ASE_NONE = 0x00000000,
    AFL_ASE_DSP = 0x00000001,       // DSP ASE.
    AFL_ASE_DSPR2 = 0x00000002,     // DSP R2 ASE.
    AFL_ASE_EVA = 0x00000004,       // Enhanced VA Scheme.
    AFL_ASE_MCU = 0x00000008,       // MCU (MicroController) ASE.
    AFL_ASE_MDMX = 0x00000010,      // MDMX ASE.
    AFL_ASE_MIPS3D = 0x00000020,    // MIPS-3D ASE.
    AFL_ASE_MT = 0x00000040,        // MT ASE.
    AFL_ASE_SMARTMIPS = 0x00000080, // SmartMIPS ASE.
    AFL_ASE_VIRT = 0x00000100,      // VZ ASE.
    AFL_ASE_MSA = 0x00000200,       // MSA ASE.
    AFL_ASE_MIPS16 = 0x00000400,    // MIPS16 ASE.
    AFL_ASE_MICROMIPS = 0x00000800, // MICROMIPS ASE.
    AFL_ASE_XPA = 0x00001000        // XPA ASE.
  };

  enum AFL_FLAGS1 { AFL_FLAGS1_NONE = 0, AFL_FLAGS1_ODDSPREG = 1 };

  enum AFL_FLAGS2 { AFL_FLAGS2_NONE = 0 };

  uint16_t Version = AFL_VERSION_V0;
  uint8_t ISALevel = AFL_ISA_LEVEL_MIPS32;
  uint8_t ISARevision = AFL_ISA_REV_R1;
  uint8_t GPRSize = AFL_REG_32;
  uint8_t CPR1Size = AFL_REG_32;
  uint8_t CPR2Size = AFL_REG_NONE;
  uint8_t FPABI = AFL_FP_DOUBLE;
  uint32_t Extension = AFL_EXT_NONE;
  uint32_t ASE = AFL_ASE_NONE;
  uint32_t Flags1 = AFL_FLAGS1_ODDSPREG;
  uint32_t Flags2 = AFL_FLAGS2_NONE;

  MipsABIFlagsSection() = default;
};

} // end of namespace MIPS32
} // end of namespace Ice

#endif // SUBZERO_SRC_ICETARGETLOWERINGMIPS32_H
