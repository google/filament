/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Global Instruction Selector for the RISCV target                           *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef GET_GLOBALISEL_PREDICATE_BITSET
const unsigned MAX_SUBTARGET_PREDICATES = 6;
using PredicateBitset = llvm::PredicateBitsetImpl<MAX_SUBTARGET_PREDICATES>;
#endif // ifdef GET_GLOBALISEL_PREDICATE_BITSET

#ifdef GET_GLOBALISEL_TEMPORARIES_DECL
  mutable MatcherState State;
  typedef ComplexRendererFns(RISCVInstructionSelector::*ComplexMatcherMemFn)(MachineOperand &) const;
  typedef void(RISCVInstructionSelector::*CustomRendererFn)(MachineInstrBuilder &, const MachineInstr&, int) const;
  const ISelInfoTy<PredicateBitset, ComplexMatcherMemFn, CustomRendererFn> ISelInfo;
  static RISCVInstructionSelector::ComplexMatcherMemFn ComplexPredicateFns[];
  static RISCVInstructionSelector::CustomRendererFn CustomRenderers[];
  bool testImmPredicate_I64(unsigned PredicateID, int64_t Imm) const override;
  bool testImmPredicate_APInt(unsigned PredicateID, const APInt &Imm) const override;
  bool testImmPredicate_APFloat(unsigned PredicateID, const APFloat &Imm) const override;
  const int64_t *getMatchTable() const override;
  bool testMIPredicate_MI(unsigned PredicateID, const MachineInstr &MI) const override;
#endif // ifdef GET_GLOBALISEL_TEMPORARIES_DECL

#ifdef GET_GLOBALISEL_TEMPORARIES_INIT
, State(0),
ISelInfo(TypeObjects, NumTypeObjects, FeatureBitsets, ComplexPredicateFns, CustomRenderers)
#endif // ifdef GET_GLOBALISEL_TEMPORARIES_INIT

#ifdef GET_GLOBALISEL_IMPL
// Bits for subtarget features that participate in instruction matching.
enum SubtargetFeatureBits : uint8_t {
  Feature_HasStdExtMBit = 4,
  Feature_HasStdExtABit = 5,
  Feature_HasStdExtFBit = 0,
  Feature_HasStdExtDBit = 1,
  Feature_IsRV64Bit = 3,
  Feature_IsRV32Bit = 2,
};

PredicateBitset RISCVInstructionSelector::
computeAvailableModuleFeatures(const RISCVSubtarget *Subtarget) const {
  PredicateBitset Features;
  if (Subtarget->hasStdExtM())
    Features.set(Feature_HasStdExtMBit);
  if (Subtarget->hasStdExtA())
    Features.set(Feature_HasStdExtABit);
  if (Subtarget->hasStdExtF())
    Features.set(Feature_HasStdExtFBit);
  if (Subtarget->hasStdExtD())
    Features.set(Feature_HasStdExtDBit);
  if (Subtarget->is64Bit())
    Features.set(Feature_IsRV64Bit);
  if (!Subtarget->is64Bit())
    Features.set(Feature_IsRV32Bit);
  return Features;
}

void RISCVInstructionSelector::setupGeneratedPerFunctionState(MachineFunction &MF) {
  AvailableFunctionFeatures = computeAvailableFunctionFeatures((const RISCVSubtarget*)&MF.getSubtarget(), &MF);
}
PredicateBitset RISCVInstructionSelector::
computeAvailableFunctionFeatures(const RISCVSubtarget *Subtarget, const MachineFunction *MF) const {
  PredicateBitset Features;
  return Features;
}

// LLT Objects.
enum {
  GILLT_s32,
  GILLT_s64,
};
const static size_t NumTypeObjects = 2;
const static LLT TypeObjects[] = {
  LLT::scalar(32),
  LLT::scalar(64),
};

// Feature bitsets.
enum {
  GIFBS_Invalid,
  GIFBS_HasStdExtA,
  GIFBS_HasStdExtD,
  GIFBS_HasStdExtF,
  GIFBS_HasStdExtM,
  GIFBS_IsRV32,
  GIFBS_IsRV64,
  GIFBS_HasStdExtA_IsRV64,
  GIFBS_HasStdExtD_IsRV32,
  GIFBS_HasStdExtD_IsRV64,
};
const static PredicateBitset FeatureBitsets[] {
  {}, // GIFBS_Invalid
  {Feature_HasStdExtABit, },
  {Feature_HasStdExtDBit, },
  {Feature_HasStdExtFBit, },
  {Feature_HasStdExtMBit, },
  {Feature_IsRV32Bit, },
  {Feature_IsRV64Bit, },
  {Feature_HasStdExtABit, Feature_IsRV64Bit, },
  {Feature_HasStdExtDBit, Feature_IsRV32Bit, },
  {Feature_HasStdExtDBit, Feature_IsRV64Bit, },
};

// ComplexPattern predicates.
enum {
  GICP_Invalid,
};
// See constructor for table contents

// PatFrag predicates.
enum {
  GIPFP_I64_Predicate_c_lui_imm = GIPFP_I64_Invalid + 1,
  GIPFP_I64_Predicate_immbottomxlenset,
  GIPFP_I64_Predicate_immzero,
  GIPFP_I64_Predicate_simm10_lsb0000nonzero,
  GIPFP_I64_Predicate_simm12,
  GIPFP_I64_Predicate_simm12_lsb0,
  GIPFP_I64_Predicate_simm32,
  GIPFP_I64_Predicate_simm32hi20,
  GIPFP_I64_Predicate_simm6,
  GIPFP_I64_Predicate_simm6nonzero,
  GIPFP_I64_Predicate_simm9_lsb0,
  GIPFP_I64_Predicate_uimm10_lsb00nonzero,
  GIPFP_I64_Predicate_uimm5,
  GIPFP_I64_Predicate_uimm7_lsb00,
  GIPFP_I64_Predicate_uimm8_lsb00,
  GIPFP_I64_Predicate_uimm8_lsb000,
  GIPFP_I64_Predicate_uimm9_lsb000,
  GIPFP_I64_Predicate_uimmlog2xlen,
  GIPFP_I64_Predicate_uimmlog2xlennonzero,
};
bool RISCVInstructionSelector::testImmPredicate_I64(unsigned PredicateID, int64_t Imm) const {
  switch (PredicateID) {
  case GIPFP_I64_Predicate_c_lui_imm: {
    return (Imm != 0) &&
                                 (isUInt<5>(Imm) ||
                                  (Imm >= 0xfffe0 && Imm <= 0xfffff));
    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_immbottomxlenset: {

  if (Subtarget->is64Bit())
    return countTrailingOnes<uint64_t>(Imm) >= 6;
  return countTrailingOnes<uint64_t>(Imm) >= 5;

    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_immzero: {
    return (Imm == 0);
    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_simm10_lsb0000nonzero: {
    return (Imm != 0) && isShiftedInt<6, 4>(Imm);
    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_simm12: {
    return isInt<12>(Imm);
    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_simm12_lsb0: {
    return isShiftedInt<11, 1>(Imm);
    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_simm32: {
    return isInt<32>(Imm);
    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_simm32hi20: {
    return isShiftedInt<20, 12>(Imm);
    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_simm6: {
    return isInt<6>(Imm);
    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_simm6nonzero: {
    return (Imm != 0) && isInt<6>(Imm);
    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_simm9_lsb0: {
    return isShiftedInt<8, 1>(Imm);
    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_uimm10_lsb00nonzero: {
    return isShiftedUInt<8, 2>(Imm) && (Imm != 0);
    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_uimm5: {
    return isUInt<5>(Imm);
    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_uimm7_lsb00: {
    return isShiftedUInt<5, 2>(Imm);
    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_uimm8_lsb00: {
    return isShiftedUInt<6, 2>(Imm);
    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_uimm8_lsb000: {
    return isShiftedUInt<5, 3>(Imm);
    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_uimm9_lsb000: {
    return isShiftedUInt<6, 3>(Imm);
    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_uimmlog2xlen: {

  if (Subtarget->is64Bit())
    return isUInt<6>(Imm);
  return isUInt<5>(Imm);

    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  case GIPFP_I64_Predicate_uimmlog2xlennonzero: {

  if (Subtarget->is64Bit())
    return isUInt<6>(Imm) && (Imm != 0);
  return isUInt<5>(Imm) && (Imm != 0);

    llvm_unreachable("ImmediateCode should have returned");
    return false;
  }
  }
  llvm_unreachable("Unknown predicate");
  return false;
}
bool RISCVInstructionSelector::testImmPredicate_APFloat(unsigned PredicateID, const APFloat & Imm) const {
  llvm_unreachable("Unknown predicate");
  return false;
}
bool RISCVInstructionSelector::testImmPredicate_APInt(unsigned PredicateID, const APInt & Imm) const {
  llvm_unreachable("Unknown predicate");
  return false;
}
bool RISCVInstructionSelector::testMIPredicate_MI(unsigned PredicateID, const MachineInstr & MI) const {
  const MachineFunction &MF = *MI.getParent()->getParent();
  const MachineRegisterInfo &MRI = MF.getRegInfo();
  (void)MRI;
  llvm_unreachable("Unknown predicate");
  return false;
}

RISCVInstructionSelector::ComplexMatcherMemFn
RISCVInstructionSelector::ComplexPredicateFns[] = {
  nullptr, // GICP_Invalid
};

// Custom renderers.
enum {
  GICR_Invalid,
};
RISCVInstructionSelector::CustomRendererFn
RISCVInstructionSelector::CustomRenderers[] = {
  nullptr, // GICR_Invalid
};

bool RISCVInstructionSelector::selectImpl(MachineInstr &I, CodeGenCoverage &CoverageInfo) const {
  MachineFunction &MF = *I.getParent()->getParent();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const PredicateBitset AvailableFeatures = getAvailableFeatures();
  NewMIVector OutMIs;
  State.MIs.clear();
  State.MIs.push_back(&I);

  if (executeMatchTable(*this, OutMIs, State, ISelInfo, getMatchTable(), TII, MRI, TRI, RBI, AvailableFeatures, CoverageInfo)) {
    return true;
  }

  return false;
}

const int64_t *RISCVInstructionSelector::getMatchTable() const {
  constexpr static int64_t MatchTable0[] = {
    GIM_SwitchOpcode, /*MI*/0, /*[*/35, 168, /*)*//*default:*//*Label 44*/ 28266,
    /*TargetOpcode::G_ADD*//*Label 0*/ 138,
    /*TargetOpcode::G_SUB*//*Label 1*/ 325,
    /*TargetOpcode::G_MUL*//*Label 2*/ 406,
    /*TargetOpcode::G_SDIV*//*Label 3*/ 493,
    /*TargetOpcode::G_UDIV*//*Label 4*/ 580,
    /*TargetOpcode::G_SREM*//*Label 5*/ 667,
    /*TargetOpcode::G_UREM*//*Label 6*/ 754,
    /*TargetOpcode::G_AND*//*Label 7*/ 841,
    /*TargetOpcode::G_OR*//*Label 8*/ 1166,
    /*TargetOpcode::G_XOR*//*Label 9*/ 1353, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*TargetOpcode::G_BITCAST*//*Label 10*/ 1540, 0, 0,
    /*TargetOpcode::G_READCYCLECOUNTER*//*Label 11*/ 1827,
    /*TargetOpcode::G_LOAD*//*Label 12*/ 1918,
    /*TargetOpcode::G_SEXTLOAD*//*Label 13*/ 6036,
    /*TargetOpcode::G_ZEXTLOAD*//*Label 14*/ 7130, 0, 0, 0,
    /*TargetOpcode::G_STORE*//*Label 15*/ 8224, 0, 0, 0,
    /*TargetOpcode::G_ATOMICRMW_XCHG*//*Label 16*/ 9563,
    /*TargetOpcode::G_ATOMICRMW_ADD*//*Label 17*/ 10608,
    /*TargetOpcode::G_ATOMICRMW_SUB*//*Label 18*/ 11653,
    /*TargetOpcode::G_ATOMICRMW_AND*//*Label 19*/ 13808, 0,
    /*TargetOpcode::G_ATOMICRMW_OR*//*Label 20*/ 14853,
    /*TargetOpcode::G_ATOMICRMW_XOR*//*Label 21*/ 15898,
    /*TargetOpcode::G_ATOMICRMW_MAX*//*Label 22*/ 16943,
    /*TargetOpcode::G_ATOMICRMW_MIN*//*Label 23*/ 17988,
    /*TargetOpcode::G_ATOMICRMW_UMAX*//*Label 24*/ 19033,
    /*TargetOpcode::G_ATOMICRMW_UMIN*//*Label 25*/ 20078, 0, 0,
    /*TargetOpcode::G_FENCE*//*Label 26*/ 21123, 0, 0, 0, 0, 0, 0,
    /*TargetOpcode::G_CONSTANT*//*Label 27*/ 21392, 0, 0, 0, 0, 0, 0,
    /*TargetOpcode::G_SHL*//*Label 28*/ 21483,
    /*TargetOpcode::G_LSHR*//*Label 29*/ 21835,
    /*TargetOpcode::G_ASHR*//*Label 30*/ 22187,
    /*TargetOpcode::G_ICMP*//*Label 31*/ 22539,
    /*TargetOpcode::G_FCMP*//*Label 32*/ 24755, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*TargetOpcode::G_UMULH*//*Label 33*/ 25936,
    /*TargetOpcode::G_SMULH*//*Label 34*/ 26023, 0, 0, 0,
    /*TargetOpcode::G_FMA*//*Label 35*/ 26110, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*TargetOpcode::G_FPEXT*//*Label 36*/ 27245,
    /*TargetOpcode::G_FPTRUNC*//*Label 37*/ 27273,
    /*TargetOpcode::G_FPTOSI*//*Label 38*/ 27362,
    /*TargetOpcode::G_FPTOUI*//*Label 39*/ 27541,
    /*TargetOpcode::G_SITOFP*//*Label 40*/ 27720,
    /*TargetOpcode::G_UITOFP*//*Label 41*/ 27897, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*TargetOpcode::G_BR*//*Label 42*/ 28074, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*TargetOpcode::G_FSQRT*//*Label 43*/ 28087,
    // Label 0: @138
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 47*/ 324,
    /*GILLT_s32*//*Label 45*/ 146,
    /*GILLT_s64*//*Label 46*/ 258,
    // Label 45: @146
    GIM_Try, /*On fail goto*//*Label 48*/ 257,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 49*/ 197, // Rule ID 35 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)  =>  (ADDI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::ADDI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 35,
        GIR_Done,
      // Label 49: @197
      GIM_Try, /*On fail goto*//*Label 50*/ 230, // Rule ID 37 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)  =>  (ADDI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::ADDI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 37,
        GIR_Done,
      // Label 50: @230
      GIM_Try, /*On fail goto*//*Label 51*/ 243, // Rule ID 32 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (ADD:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::ADD,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 32,
        GIR_Done,
      // Label 51: @243
      GIM_Try, /*On fail goto*//*Label 52*/ 256, // Rule ID 34 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (ADD:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::ADD,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 34,
        GIR_Done,
      // Label 52: @256
      GIM_Reject,
    // Label 48: @257
    GIM_Reject,
    // Label 46: @258
    GIM_Try, /*On fail goto*//*Label 53*/ 323,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 54*/ 309, // Rule ID 36 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)  =>  (ADDI:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::ADDI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 36,
        GIR_Done,
      // Label 54: @309
      GIM_Try, /*On fail goto*//*Label 55*/ 322, // Rule ID 33 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)  =>  (ADD:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::ADD,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 33,
        GIR_Done,
      // Label 55: @322
      GIM_Reject,
    // Label 53: @323
    GIM_Reject,
    // Label 47: @324
    GIM_Reject,
    // Label 1: @325
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 58*/ 405,
    /*GILLT_s32*//*Label 56*/ 333,
    /*GILLT_s64*//*Label 57*/ 375,
    // Label 56: @333
    GIM_Try, /*On fail goto*//*Label 59*/ 374,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 60*/ 364, // Rule ID 38 //
        // (sub:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (SUB:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::SUB,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 38,
        GIR_Done,
      // Label 60: @364
      GIM_Try, /*On fail goto*//*Label 61*/ 373, // Rule ID 40 //
        // (sub:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (SUB:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::SUB,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 40,
        GIR_Done,
      // Label 61: @373
      GIM_Reject,
    // Label 59: @374
    GIM_Reject,
    // Label 57: @375
    GIM_Try, /*On fail goto*//*Label 62*/ 404, // Rule ID 39 //
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
      // (sub:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)  =>  (SUB:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::SUB,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 39,
      GIR_Done,
    // Label 62: @404
    GIM_Reject,
    // Label 58: @405
    GIM_Reject,
    // Label 2: @406
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 65*/ 492,
    /*GILLT_s32*//*Label 63*/ 414,
    /*GILLT_s64*//*Label 64*/ 460,
    // Label 63: @414
    GIM_Try, /*On fail goto*//*Label 66*/ 459,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 67*/ 447, // Rule ID 473 //
        GIM_CheckFeatures, GIFBS_HasStdExtM,
        // (mul:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (MUL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::MUL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 473,
        GIR_Done,
      // Label 67: @447
      GIM_Try, /*On fail goto*//*Label 68*/ 458, // Rule ID 475 //
        GIM_CheckFeatures, GIFBS_HasStdExtM,
        // (mul:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (MUL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::MUL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 475,
        GIR_Done,
      // Label 68: @458
      GIM_Reject,
    // Label 66: @459
    GIM_Reject,
    // Label 64: @460
    GIM_Try, /*On fail goto*//*Label 69*/ 491, // Rule ID 474 //
      GIM_CheckFeatures, GIFBS_HasStdExtM,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
      // (mul:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)  =>  (MUL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::MUL,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 474,
      GIR_Done,
    // Label 69: @491
    GIM_Reject,
    // Label 65: @492
    GIM_Reject,
    // Label 3: @493
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 72*/ 579,
    /*GILLT_s32*//*Label 70*/ 501,
    /*GILLT_s64*//*Label 71*/ 547,
    // Label 70: @501
    GIM_Try, /*On fail goto*//*Label 73*/ 546,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 74*/ 534, // Rule ID 482 //
        GIM_CheckFeatures, GIFBS_HasStdExtM,
        // (sdiv:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (DIV:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::DIV,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 482,
        GIR_Done,
      // Label 74: @534
      GIM_Try, /*On fail goto*//*Label 75*/ 545, // Rule ID 484 //
        GIM_CheckFeatures, GIFBS_HasStdExtM,
        // (sdiv:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (DIV:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::DIV,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 484,
        GIR_Done,
      // Label 75: @545
      GIM_Reject,
    // Label 73: @546
    GIM_Reject,
    // Label 71: @547
    GIM_Try, /*On fail goto*//*Label 76*/ 578, // Rule ID 483 //
      GIM_CheckFeatures, GIFBS_HasStdExtM,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
      // (sdiv:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)  =>  (DIV:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::DIV,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 483,
      GIR_Done,
    // Label 76: @578
    GIM_Reject,
    // Label 72: @579
    GIM_Reject,
    // Label 4: @580
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 79*/ 666,
    /*GILLT_s32*//*Label 77*/ 588,
    /*GILLT_s64*//*Label 78*/ 634,
    // Label 77: @588
    GIM_Try, /*On fail goto*//*Label 80*/ 633,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 81*/ 621, // Rule ID 485 //
        GIM_CheckFeatures, GIFBS_HasStdExtM,
        // (udiv:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (DIVU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::DIVU,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 485,
        GIR_Done,
      // Label 81: @621
      GIM_Try, /*On fail goto*//*Label 82*/ 632, // Rule ID 487 //
        GIM_CheckFeatures, GIFBS_HasStdExtM,
        // (udiv:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (DIVU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::DIVU,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 487,
        GIR_Done,
      // Label 82: @632
      GIM_Reject,
    // Label 80: @633
    GIM_Reject,
    // Label 78: @634
    GIM_Try, /*On fail goto*//*Label 83*/ 665, // Rule ID 486 //
      GIM_CheckFeatures, GIFBS_HasStdExtM,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
      // (udiv:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)  =>  (DIVU:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::DIVU,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 486,
      GIR_Done,
    // Label 83: @665
    GIM_Reject,
    // Label 79: @666
    GIM_Reject,
    // Label 5: @667
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 86*/ 753,
    /*GILLT_s32*//*Label 84*/ 675,
    /*GILLT_s64*//*Label 85*/ 721,
    // Label 84: @675
    GIM_Try, /*On fail goto*//*Label 87*/ 720,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 88*/ 708, // Rule ID 488 //
        GIM_CheckFeatures, GIFBS_HasStdExtM,
        // (srem:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (REM:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::REM,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 488,
        GIR_Done,
      // Label 88: @708
      GIM_Try, /*On fail goto*//*Label 89*/ 719, // Rule ID 490 //
        GIM_CheckFeatures, GIFBS_HasStdExtM,
        // (srem:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (REM:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::REM,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 490,
        GIR_Done,
      // Label 89: @719
      GIM_Reject,
    // Label 87: @720
    GIM_Reject,
    // Label 85: @721
    GIM_Try, /*On fail goto*//*Label 90*/ 752, // Rule ID 489 //
      GIM_CheckFeatures, GIFBS_HasStdExtM,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
      // (srem:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)  =>  (REM:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::REM,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 489,
      GIR_Done,
    // Label 90: @752
    GIM_Reject,
    // Label 86: @753
    GIM_Reject,
    // Label 6: @754
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 93*/ 840,
    /*GILLT_s32*//*Label 91*/ 762,
    /*GILLT_s64*//*Label 92*/ 808,
    // Label 91: @762
    GIM_Try, /*On fail goto*//*Label 94*/ 807,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 95*/ 795, // Rule ID 491 //
        GIM_CheckFeatures, GIFBS_HasStdExtM,
        // (urem:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (REMU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::REMU,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 491,
        GIR_Done,
      // Label 95: @795
      GIM_Try, /*On fail goto*//*Label 96*/ 806, // Rule ID 493 //
        GIM_CheckFeatures, GIFBS_HasStdExtM,
        // (urem:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (REMU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::REMU,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 493,
        GIR_Done,
      // Label 96: @806
      GIM_Reject,
    // Label 94: @807
    GIM_Reject,
    // Label 92: @808
    GIM_Try, /*On fail goto*//*Label 97*/ 839, // Rule ID 492 //
      GIM_CheckFeatures, GIFBS_HasStdExtM,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
      // (urem:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)  =>  (REMU:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::REMU,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 492,
      GIR_Done,
    // Label 97: @839
    GIM_Reject,
    // Label 93: @840
    GIM_Reject,
    // Label 7: @841
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 100*/ 1165,
    /*GILLT_s32*//*Label 98*/ 849,
    /*GILLT_s64*//*Label 99*/ 1053,
    // Label 98: @849
    GIM_Try, /*On fail goto*//*Label 101*/ 1052,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 102*/ 913, // Rule ID 361 //
        GIM_CheckFeatures, GIFBS_IsRV64,
        GIM_CheckConstantInt, /*MI*/0, /*Op*/2, 4294967295,
        // (and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 4294967295:{ *:[i32] })  =>  (SRLI:{ *:[i32] } (SLLI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 32:{ *:[i32] }), 32:{ *:[i32] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SLLI,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/1, /*Imm*/32,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SRLI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/32,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 361,
        GIR_Done,
      // Label 102: @913
      GIM_Try, /*On fail goto*//*Label 103*/ 959, // Rule ID 363 //
        GIM_CheckFeatures, GIFBS_IsRV64,
        GIM_CheckConstantInt, /*MI*/0, /*Op*/2, 4294967295,
        // (and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 4294967295:{ *:[i32] })  =>  (SRLI:{ *:[i32] } (SLLI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 32:{ *:[i32] }), 32:{ *:[i32] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SLLI,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/1, /*Imm*/32,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SRLI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/32,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 363,
        GIR_Done,
      // Label 103: @959
      GIM_Try, /*On fail goto*//*Label 104*/ 992, // Rule ID 50 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)  =>  (ANDI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::ANDI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 50,
        GIR_Done,
      // Label 104: @992
      GIM_Try, /*On fail goto*//*Label 105*/ 1025, // Rule ID 52 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)  =>  (ANDI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::ANDI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 52,
        GIR_Done,
      // Label 105: @1025
      GIM_Try, /*On fail goto*//*Label 106*/ 1038, // Rule ID 47 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (AND:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AND,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 47,
        GIR_Done,
      // Label 106: @1038
      GIM_Try, /*On fail goto*//*Label 107*/ 1051, // Rule ID 49 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (AND:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AND,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 49,
        GIR_Done,
      // Label 107: @1051
      GIM_Reject,
    // Label 101: @1052
    GIM_Reject,
    // Label 99: @1053
    GIM_Try, /*On fail goto*//*Label 108*/ 1164,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 109*/ 1117, // Rule ID 362 //
        GIM_CheckFeatures, GIFBS_IsRV64,
        GIM_CheckConstantInt, /*MI*/0, /*Op*/2, 4294967295,
        // (and:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 4294967295:{ *:[i64] })  =>  (SRLI:{ *:[i64] } (SLLI:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 32:{ *:[i64] }), 32:{ *:[i64] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SLLI,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/1, /*Imm*/32,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SRLI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/32,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 362,
        GIR_Done,
      // Label 109: @1117
      GIM_Try, /*On fail goto*//*Label 110*/ 1150, // Rule ID 51 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (and:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)  =>  (ANDI:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::ANDI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 51,
        GIR_Done,
      // Label 110: @1150
      GIM_Try, /*On fail goto*//*Label 111*/ 1163, // Rule ID 48 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (and:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)  =>  (AND:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AND,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 48,
        GIR_Done,
      // Label 111: @1163
      GIM_Reject,
    // Label 108: @1164
    GIM_Reject,
    // Label 100: @1165
    GIM_Reject,
    // Label 8: @1166
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 114*/ 1352,
    /*GILLT_s32*//*Label 112*/ 1174,
    /*GILLT_s64*//*Label 113*/ 1286,
    // Label 112: @1174
    GIM_Try, /*On fail goto*//*Label 115*/ 1285,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 116*/ 1225, // Rule ID 44 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)  =>  (ORI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::ORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 44,
        GIR_Done,
      // Label 116: @1225
      GIM_Try, /*On fail goto*//*Label 117*/ 1258, // Rule ID 46 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)  =>  (ORI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::ORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 46,
        GIR_Done,
      // Label 117: @1258
      GIM_Try, /*On fail goto*//*Label 118*/ 1271, // Rule ID 41 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (OR:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::OR,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 41,
        GIR_Done,
      // Label 118: @1271
      GIM_Try, /*On fail goto*//*Label 119*/ 1284, // Rule ID 43 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (OR:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::OR,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 43,
        GIR_Done,
      // Label 119: @1284
      GIM_Reject,
    // Label 115: @1285
    GIM_Reject,
    // Label 113: @1286
    GIM_Try, /*On fail goto*//*Label 120*/ 1351,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 121*/ 1337, // Rule ID 45 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (or:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)  =>  (ORI:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::ORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 45,
        GIR_Done,
      // Label 121: @1337
      GIM_Try, /*On fail goto*//*Label 122*/ 1350, // Rule ID 42 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (or:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)  =>  (OR:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::OR,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 42,
        GIR_Done,
      // Label 122: @1350
      GIM_Reject,
    // Label 120: @1351
    GIM_Reject,
    // Label 114: @1352
    GIM_Reject,
    // Label 9: @1353
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 125*/ 1539,
    /*GILLT_s32*//*Label 123*/ 1361,
    /*GILLT_s64*//*Label 124*/ 1473,
    // Label 123: @1361
    GIM_Try, /*On fail goto*//*Label 126*/ 1472,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 127*/ 1412, // Rule ID 56 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)  =>  (XORI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::XORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 56,
        GIR_Done,
      // Label 127: @1412
      GIM_Try, /*On fail goto*//*Label 128*/ 1445, // Rule ID 58 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)  =>  (XORI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::XORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 58,
        GIR_Done,
      // Label 128: @1445
      GIM_Try, /*On fail goto*//*Label 129*/ 1458, // Rule ID 53 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (XOR:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::XOR,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 53,
        GIR_Done,
      // Label 129: @1458
      GIM_Try, /*On fail goto*//*Label 130*/ 1471, // Rule ID 55 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (XOR:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::XOR,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 55,
        GIR_Done,
      // Label 130: @1471
      GIM_Reject,
    // Label 126: @1472
    GIM_Reject,
    // Label 124: @1473
    GIM_Try, /*On fail goto*//*Label 131*/ 1538,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 132*/ 1524, // Rule ID 57 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (xor:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)  =>  (XORI:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::XORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 57,
        GIR_Done,
      // Label 132: @1524
      GIM_Try, /*On fail goto*//*Label 133*/ 1537, // Rule ID 54 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (xor:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)  =>  (XOR:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::XOR,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 54,
        GIR_Done,
      // Label 133: @1537
      GIM_Reject,
    // Label 131: @1538
    GIM_Reject,
    // Label 125: @1539
    GIM_Reject,
    // Label 10: @1540
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 136*/ 1826,
    /*GILLT_s32*//*Label 134*/ 1548,
    /*GILLT_s64*//*Label 135*/ 1710,
    // Label 134: @1548
    GIM_Try, /*On fail goto*//*Label 137*/ 1571, // Rule ID 1067 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (bitconvert:{ *:[f32] } GPR:{ *:[i32] }:$rs1)  =>  (FMV_W_X:{ *:[f32] } GPR:{ *:[i32] }:$rs1)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FMV_W_X,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1067,
      GIR_Done,
    // Label 137: @1571
    GIM_Try, /*On fail goto*//*Label 138*/ 1594, // Rule ID 1068 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (bitconvert:{ *:[f32] } GPR:{ *:[i64] }:$rs1)  =>  (FMV_W_X:{ *:[f32] } GPR:{ *:[i64] }:$rs1)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FMV_W_X,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1068,
      GIR_Done,
    // Label 138: @1594
    GIM_Try, /*On fail goto*//*Label 139*/ 1617, // Rule ID 1069 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (bitconvert:{ *:[f32] } GPR:{ *:[i32] }:$rs1)  =>  (FMV_W_X:{ *:[f32] } GPR:{ *:[i32] }:$rs1)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FMV_W_X,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1069,
      GIR_Done,
    // Label 139: @1617
    GIM_Try, /*On fail goto*//*Label 140*/ 1640, // Rule ID 1070 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
      // (bitconvert:{ *:[i32] } FPR32:{ *:[f32] }:$rs1)  =>  (FMV_X_W:{ *:[i32] } FPR32:{ *:[f32] }:$rs1)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FMV_X_W,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1070,
      GIR_Done,
    // Label 140: @1640
    GIM_Try, /*On fail goto*//*Label 141*/ 1663, // Rule ID 1072 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
      // (bitconvert:{ *:[i32] } FPR32:{ *:[f32] }:$rs1)  =>  (FMV_X_W:{ *:[i32] } FPR32:{ *:[f32] }:$rs1)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FMV_X_W,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1072,
      GIR_Done,
    // Label 141: @1663
    GIM_Try, /*On fail goto*//*Label 142*/ 1686, // Rule ID 1318 //
      GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
      // (bitconvert:{ *:[i32] } FPR64:{ *:[f64] }:$rs1)  =>  (FMV_X_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs1)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FMV_X_D,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1318,
      GIR_Done,
    // Label 142: @1686
    GIM_Try, /*On fail goto*//*Label 143*/ 1709, // Rule ID 1320 //
      GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
      // (bitconvert:{ *:[i32] } FPR64:{ *:[f64] }:$rs1)  =>  (FMV_X_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs1)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FMV_X_D,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1320,
      GIR_Done,
    // Label 143: @1709
    GIM_Reject,
    // Label 135: @1710
    GIM_Try, /*On fail goto*//*Label 144*/ 1733, // Rule ID 1071 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
      // (bitconvert:{ *:[i64] } FPR32:{ *:[f32] }:$rs1)  =>  (FMV_X_W:{ *:[i64] } FPR32:{ *:[f32] }:$rs1)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FMV_X_W,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1071,
      GIR_Done,
    // Label 144: @1733
    GIM_Try, /*On fail goto*//*Label 145*/ 1756, // Rule ID 1315 //
      GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (bitconvert:{ *:[f64] } GPR:{ *:[i32] }:$rs1)  =>  (FMV_D_X:{ *:[f64] } GPR:{ *:[i32] }:$rs1)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FMV_D_X,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1315,
      GIR_Done,
    // Label 145: @1756
    GIM_Try, /*On fail goto*//*Label 146*/ 1779, // Rule ID 1316 //
      GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (bitconvert:{ *:[f64] } GPR:{ *:[i64] }:$rs1)  =>  (FMV_D_X:{ *:[f64] } GPR:{ *:[i64] }:$rs1)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FMV_D_X,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1316,
      GIR_Done,
    // Label 146: @1779
    GIM_Try, /*On fail goto*//*Label 147*/ 1802, // Rule ID 1317 //
      GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (bitconvert:{ *:[f64] } GPR:{ *:[i32] }:$rs1)  =>  (FMV_D_X:{ *:[f64] } GPR:{ *:[i32] }:$rs1)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FMV_D_X,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1317,
      GIR_Done,
    // Label 147: @1802
    GIM_Try, /*On fail goto*//*Label 148*/ 1825, // Rule ID 1319 //
      GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
      // (bitconvert:{ *:[i64] } FPR64:{ *:[f64] }:$rs1)  =>  (FMV_X_D:{ *:[i64] } FPR64:{ *:[f64] }:$rs1)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FMV_X_D,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1319,
      GIR_Done,
    // Label 148: @1825
    GIM_Reject,
    // Label 136: @1826
    GIM_Reject,
    // Label 11: @1827
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 151*/ 1917,
    /*GILLT_s32*//*Label 149*/ 1835,
    /*GILLT_s64*//*Label 150*/ 1889,
    // Label 149: @1835
    GIM_Try, /*On fail goto*//*Label 152*/ 1888,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 153*/ 1864, // Rule ID 468 //
        GIM_CheckFeatures, GIFBS_IsRV64,
        // (readcyclecounter:{ *:[i32] })  =>  (CSRRS:{ *:[i32] } 3072:{ *:[i32] }, X0:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::CSRRS,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddImm, /*InsnID*/0, /*Imm*/3072,
        GIR_AddRegister, /*InsnID*/0, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 468,
        GIR_Done,
      // Label 153: @1864
      GIM_Try, /*On fail goto*//*Label 154*/ 1887, // Rule ID 470 //
        GIM_CheckFeatures, GIFBS_IsRV64,
        // (readcyclecounter:{ *:[i32] })  =>  (CSRRS:{ *:[i32] } 3072:{ *:[i32] }, X0:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::CSRRS,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddImm, /*InsnID*/0, /*Imm*/3072,
        GIR_AddRegister, /*InsnID*/0, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 470,
        GIR_Done,
      // Label 154: @1887
      GIM_Reject,
    // Label 152: @1888
    GIM_Reject,
    // Label 150: @1889
    GIM_Try, /*On fail goto*//*Label 155*/ 1916, // Rule ID 469 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // (readcyclecounter:{ *:[i64] })  =>  (CSRRS:{ *:[i64] } 3072:{ *:[i64] }, X0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::CSRRS,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_AddImm, /*InsnID*/0, /*Imm*/3072,
      GIR_AddRegister, /*InsnID*/0, RISCV::X0, /*AddRegisterRegFlags*/0,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 469,
      GIR_Done,
    // Label 155: @1916
    GIM_Reject,
    // Label 151: @1917
    GIM_Reject,
    // Label 12: @1918
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 158*/ 6035,
    /*GILLT_s32*//*Label 156*/ 1926,
    /*GILLT_s64*//*Label 157*/ 4542,
    // Label 156: @1926
    GIM_Try, /*On fail goto*//*Label 159*/ 2003, // Rule ID 581 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (atomic_load:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_atomic_load_8>>  =>  (LB:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 581,
      GIR_Done,
    // Label 159: @2003
    GIM_Try, /*On fail goto*//*Label 160*/ 2080, // Rule ID 583 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (atomic_load:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_atomic_load_8>>  =>  (LB:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 583,
      GIR_Done,
    // Label 160: @2080
    GIM_Try, /*On fail goto*//*Label 161*/ 2157, // Rule ID 596 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (atomic_load:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_atomic_load_16>>  =>  (LH:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 596,
      GIR_Done,
    // Label 161: @2157
    GIM_Try, /*On fail goto*//*Label 162*/ 2234, // Rule ID 598 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (atomic_load:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_atomic_load_16>>  =>  (LH:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 598,
      GIR_Done,
    // Label 162: @2234
    GIM_Try, /*On fail goto*//*Label 163*/ 2311, // Rule ID 611 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (atomic_load:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_atomic_load_32>>  =>  (LW:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 611,
      GIR_Done,
    // Label 163: @2311
    GIM_Try, /*On fail goto*//*Label 164*/ 2388, // Rule ID 613 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (atomic_load:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_atomic_load_32>>  =>  (LW:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 613,
      GIR_Done,
    // Label 164: @2388
    GIM_Try, /*On fail goto*//*Label 165*/ 2465, // Rule ID 854 //
      GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (atomic_load:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_atomic_load_64>>  =>  (LD:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 854,
      GIR_Done,
    // Label 165: @2465
    GIM_Try, /*On fail goto*//*Label 166*/ 2542, // Rule ID 856 //
      GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (atomic_load:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_atomic_load_64>>  =>  (LD:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 856,
      GIR_Done,
    // Label 166: @2542
    GIM_Try, /*On fail goto*//*Label 167*/ 2619, // Rule ID 264 //
      GIM_CheckFeatures, GIFBS_IsRV32,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (LW:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 264,
      GIR_Done,
    // Label 167: @2619
    GIM_Try, /*On fail goto*//*Label 168*/ 2696, // Rule ID 266 //
      GIM_CheckFeatures, GIFBS_IsRV32,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (LW:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 266,
      GIR_Done,
    // Label 168: @2696
    GIM_Try, /*On fail goto*//*Label 169*/ 2773, // Rule ID 429 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (LD:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 429,
      GIR_Done,
    // Label 169: @2773
    GIM_Try, /*On fail goto*//*Label 170*/ 2850, // Rule ID 431 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (LD:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 431,
      GIR_Done,
    // Label 170: @2850
    GIM_Try, /*On fail goto*//*Label 171*/ 2927, // Rule ID 1136 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[f32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (FLW:{ *:[f32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FLW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1136,
      GIR_Done,
    // Label 171: @2927
    GIM_Try, /*On fail goto*//*Label 172*/ 3004, // Rule ID 1137 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[f32] } (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (FLW:{ *:[f32] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FLW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1137,
      GIR_Done,
    // Label 172: @3004
    GIM_Try, /*On fail goto*//*Label 173*/ 3081, // Rule ID 1138 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[f32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (FLW:{ *:[f32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FLW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1138,
      GIR_Done,
    // Label 173: @3081
    GIM_Try, /*On fail goto*//*Label 174*/ 3160, // Rule ID 219 //
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi8>>  =>  (LB:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 219,
      GIR_Done,
    // Label 174: @3160
    GIM_Try, /*On fail goto*//*Label 175*/ 3239, // Rule ID 221 //
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi8>>  =>  (LB:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 221,
      GIR_Done,
    // Label 175: @3239
    GIM_Try, /*On fail goto*//*Label 176*/ 3318, // Rule ID 249 //
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi16>>  =>  (LH:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 249,
      GIR_Done,
    // Label 176: @3318
    GIM_Try, /*On fail goto*//*Label 177*/ 3397, // Rule ID 251 //
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi16>>  =>  (LH:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 251,
      GIR_Done,
    // Label 177: @3397
    GIM_Try, /*On fail goto*//*Label 178*/ 3478, // Rule ID 399 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi32>>  =>  (LW:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 399,
      GIR_Done,
    // Label 178: @3478
    GIM_Try, /*On fail goto*//*Label 179*/ 3559, // Rule ID 401 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi32>>  =>  (LW:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 401,
      GIR_Done,
    // Label 179: @3559
    GIM_Try, /*On fail goto*//*Label 180*/ 3605, // Rule ID 575 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (atomic_load:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_atomic_load_8>>  =>  (LB:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 575,
      GIR_Done,
    // Label 180: @3605
    GIM_Try, /*On fail goto*//*Label 181*/ 3651, // Rule ID 577 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (atomic_load:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_atomic_load_8>>  =>  (LB:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 577,
      GIR_Done,
    // Label 181: @3651
    GIM_Try, /*On fail goto*//*Label 182*/ 3697, // Rule ID 590 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (atomic_load:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_atomic_load_16>>  =>  (LH:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 590,
      GIR_Done,
    // Label 182: @3697
    GIM_Try, /*On fail goto*//*Label 183*/ 3743, // Rule ID 592 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (atomic_load:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_atomic_load_16>>  =>  (LH:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 592,
      GIR_Done,
    // Label 183: @3743
    GIM_Try, /*On fail goto*//*Label 184*/ 3789, // Rule ID 605 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (atomic_load:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_atomic_load_32>>  =>  (LW:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 605,
      GIR_Done,
    // Label 184: @3789
    GIM_Try, /*On fail goto*//*Label 185*/ 3835, // Rule ID 607 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (atomic_load:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_atomic_load_32>>  =>  (LW:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 607,
      GIR_Done,
    // Label 185: @3835
    GIM_Try, /*On fail goto*//*Label 186*/ 3881, // Rule ID 848 //
      GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (atomic_load:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_atomic_load_64>>  =>  (LD:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 848,
      GIR_Done,
    // Label 186: @3881
    GIM_Try, /*On fail goto*//*Label 187*/ 3927, // Rule ID 850 //
      GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (atomic_load:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_atomic_load_64>>  =>  (LD:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 850,
      GIR_Done,
    // Label 187: @3927
    GIM_Try, /*On fail goto*//*Label 188*/ 3973, // Rule ID 258 //
      GIM_CheckFeatures, GIFBS_IsRV32,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (LW:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 258,
      GIR_Done,
    // Label 188: @3973
    GIM_Try, /*On fail goto*//*Label 189*/ 4019, // Rule ID 260 //
      GIM_CheckFeatures, GIFBS_IsRV32,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (LW:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 260,
      GIR_Done,
    // Label 189: @4019
    GIM_Try, /*On fail goto*//*Label 190*/ 4065, // Rule ID 423 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (LD:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 423,
      GIR_Done,
    // Label 190: @4065
    GIM_Try, /*On fail goto*//*Label 191*/ 4111, // Rule ID 425 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (LD:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 425,
      GIR_Done,
    // Label 191: @4111
    GIM_Try, /*On fail goto*//*Label 192*/ 4157, // Rule ID 1130 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[f32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (FLW:{ *:[f32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FLW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1130,
      GIR_Done,
    // Label 192: @4157
    GIM_Try, /*On fail goto*//*Label 193*/ 4203, // Rule ID 1131 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[f32] } GPR:{ *:[i64] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (FLW:{ *:[f32] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FLW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1131,
      GIR_Done,
    // Label 193: @4203
    GIM_Try, /*On fail goto*//*Label 194*/ 4249, // Rule ID 1132 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[f32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (FLW:{ *:[f32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FLW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1132,
      GIR_Done,
    // Label 194: @4249
    GIM_Try, /*On fail goto*//*Label 195*/ 4297, // Rule ID 213 //
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi8>>  =>  (LB:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 213,
      GIR_Done,
    // Label 195: @4297
    GIM_Try, /*On fail goto*//*Label 196*/ 4345, // Rule ID 215 //
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi8>>  =>  (LB:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 215,
      GIR_Done,
    // Label 196: @4345
    GIM_Try, /*On fail goto*//*Label 197*/ 4393, // Rule ID 243 //
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi16>>  =>  (LH:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 243,
      GIR_Done,
    // Label 197: @4393
    GIM_Try, /*On fail goto*//*Label 198*/ 4441, // Rule ID 245 //
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi16>>  =>  (LH:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 245,
      GIR_Done,
    // Label 198: @4441
    GIM_Try, /*On fail goto*//*Label 199*/ 4491, // Rule ID 393 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi32>>  =>  (LW:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 393,
      GIR_Done,
    // Label 199: @4491
    GIM_Try, /*On fail goto*//*Label 200*/ 4541, // Rule ID 395 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi32>>  =>  (LW:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 395,
      GIR_Done,
    // Label 200: @4541
    GIM_Reject,
    // Label 157: @4542
    GIM_Try, /*On fail goto*//*Label 201*/ 4619, // Rule ID 582 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (atomic_load:{ *:[i64] } (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_atomic_load_8>>  =>  (LB:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 582,
      GIR_Done,
    // Label 201: @4619
    GIM_Try, /*On fail goto*//*Label 202*/ 4696, // Rule ID 597 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (atomic_load:{ *:[i64] } (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_atomic_load_16>>  =>  (LH:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 597,
      GIR_Done,
    // Label 202: @4696
    GIM_Try, /*On fail goto*//*Label 203*/ 4773, // Rule ID 612 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (atomic_load:{ *:[i64] } (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_atomic_load_32>>  =>  (LW:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 612,
      GIR_Done,
    // Label 203: @4773
    GIM_Try, /*On fail goto*//*Label 204*/ 4850, // Rule ID 855 //
      GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (atomic_load:{ *:[i64] } (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_atomic_load_64>>  =>  (LD:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 855,
      GIR_Done,
    // Label 204: @4850
    GIM_Try, /*On fail goto*//*Label 205*/ 4927, // Rule ID 265 //
      GIM_CheckFeatures, GIFBS_IsRV32,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i64] } (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (LW:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 265,
      GIR_Done,
    // Label 205: @4927
    GIM_Try, /*On fail goto*//*Label 206*/ 5004, // Rule ID 430 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i64] } (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (LD:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 430,
      GIR_Done,
    // Label 206: @5004
    GIM_Try, /*On fail goto*//*Label 207*/ 5081, // Rule ID 1279 //
      GIM_CheckFeatures, GIFBS_HasStdExtD,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[f64] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (FLD:{ *:[f64] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FLD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1279,
      GIR_Done,
    // Label 207: @5081
    GIM_Try, /*On fail goto*//*Label 208*/ 5158, // Rule ID 1280 //
      GIM_CheckFeatures, GIFBS_HasStdExtD,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[f64] } (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (FLD:{ *:[f64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FLD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1280,
      GIR_Done,
    // Label 208: @5158
    GIM_Try, /*On fail goto*//*Label 209*/ 5235, // Rule ID 1281 //
      GIM_CheckFeatures, GIFBS_HasStdExtD,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[f64] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (FLD:{ *:[f64] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FLD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1281,
      GIR_Done,
    // Label 209: @5235
    GIM_Try, /*On fail goto*//*Label 210*/ 5314, // Rule ID 220 //
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i64] } (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi8>>  =>  (LB:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 220,
      GIR_Done,
    // Label 210: @5314
    GIM_Try, /*On fail goto*//*Label 211*/ 5393, // Rule ID 250 //
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i64] } (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi16>>  =>  (LH:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 250,
      GIR_Done,
    // Label 211: @5393
    GIM_Try, /*On fail goto*//*Label 212*/ 5474, // Rule ID 400 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i64] } (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi32>>  =>  (LW:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 400,
      GIR_Done,
    // Label 212: @5474
    GIM_Try, /*On fail goto*//*Label 213*/ 5520, // Rule ID 576 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (atomic_load:{ *:[i64] } GPR:{ *:[i64] }:$rs1)<<P:Predicate_atomic_load_8>>  =>  (LB:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 576,
      GIR_Done,
    // Label 213: @5520
    GIM_Try, /*On fail goto*//*Label 214*/ 5566, // Rule ID 591 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (atomic_load:{ *:[i64] } GPR:{ *:[i64] }:$rs1)<<P:Predicate_atomic_load_16>>  =>  (LH:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 591,
      GIR_Done,
    // Label 214: @5566
    GIM_Try, /*On fail goto*//*Label 215*/ 5612, // Rule ID 606 //
      GIM_CheckFeatures, GIFBS_HasStdExtA,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (atomic_load:{ *:[i64] } GPR:{ *:[i64] }:$rs1)<<P:Predicate_atomic_load_32>>  =>  (LW:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 606,
      GIR_Done,
    // Label 215: @5612
    GIM_Try, /*On fail goto*//*Label 216*/ 5658, // Rule ID 849 //
      GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
      GIM_CheckAtomicOrderingOrStrongerThan, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Unordered,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (atomic_load:{ *:[i64] } GPR:{ *:[i64] }:$rs1)<<P:Predicate_atomic_load_64>>  =>  (LD:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 849,
      GIR_Done,
    // Label 216: @5658
    GIM_Try, /*On fail goto*//*Label 217*/ 5704, // Rule ID 259 //
      GIM_CheckFeatures, GIFBS_IsRV32,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i64] } GPR:{ *:[i64] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (LW:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 259,
      GIR_Done,
    // Label 217: @5704
    GIM_Try, /*On fail goto*//*Label 218*/ 5750, // Rule ID 424 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i64] } GPR:{ *:[i64] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (LD:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 424,
      GIR_Done,
    // Label 218: @5750
    GIM_Try, /*On fail goto*//*Label 219*/ 5796, // Rule ID 1273 //
      GIM_CheckFeatures, GIFBS_HasStdExtD,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[f64] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (FLD:{ *:[f64] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FLD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1273,
      GIR_Done,
    // Label 219: @5796
    GIM_Try, /*On fail goto*//*Label 220*/ 5842, // Rule ID 1274 //
      GIM_CheckFeatures, GIFBS_HasStdExtD,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[f64] } GPR:{ *:[i64] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (FLD:{ *:[f64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FLD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1274,
      GIR_Done,
    // Label 220: @5842
    GIM_Try, /*On fail goto*//*Label 221*/ 5888, // Rule ID 1275 //
      GIM_CheckFeatures, GIFBS_HasStdExtD,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[f64] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_load>>  =>  (FLD:{ *:[f64] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FLD,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1275,
      GIR_Done,
    // Label 221: @5888
    GIM_Try, /*On fail goto*//*Label 222*/ 5936, // Rule ID 214 //
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i64] } GPR:{ *:[i64] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi8>>  =>  (LB:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 214,
      GIR_Done,
    // Label 222: @5936
    GIM_Try, /*On fail goto*//*Label 223*/ 5984, // Rule ID 244 //
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i64] } GPR:{ *:[i64] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi16>>  =>  (LH:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 244,
      GIR_Done,
    // Label 223: @5984
    GIM_Try, /*On fail goto*//*Label 224*/ 6034, // Rule ID 394 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeLessThanLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i64] } GPR:{ *:[i64] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_extload>><<P:Predicate_extloadi32>>  =>  (LW:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 394,
      GIR_Done,
    // Label 224: @6034
    GIM_Reject,
    // Label 158: @6035
    GIM_Reject,
    // Label 13: @6036
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 227*/ 7129,
    /*GILLT_s32*//*Label 225*/ 6044,
    /*GILLT_s64*//*Label 226*/ 6767,
    // Label 225: @6044
    GIM_Try, /*On fail goto*//*Label 228*/ 6119, // Rule ID 204 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi8>>  =>  (LB:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 204,
      GIR_Done,
    // Label 228: @6119
    GIM_Try, /*On fail goto*//*Label 229*/ 6194, // Rule ID 206 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi8>>  =>  (LB:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 206,
      GIR_Done,
    // Label 229: @6194
    GIM_Try, /*On fail goto*//*Label 230*/ 6269, // Rule ID 234 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi16>>  =>  (LH:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 234,
      GIR_Done,
    // Label 230: @6269
    GIM_Try, /*On fail goto*//*Label 231*/ 6344, // Rule ID 236 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi16>>  =>  (LH:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 236,
      GIR_Done,
    // Label 231: @6344
    GIM_Try, /*On fail goto*//*Label 232*/ 6421, // Rule ID 384 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi32>>  =>  (LW:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 384,
      GIR_Done,
    // Label 232: @6421
    GIM_Try, /*On fail goto*//*Label 233*/ 6498, // Rule ID 386 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi32>>  =>  (LW:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 386,
      GIR_Done,
    // Label 233: @6498
    GIM_Try, /*On fail goto*//*Label 234*/ 6542, // Rule ID 198 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi8>>  =>  (LB:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 198,
      GIR_Done,
    // Label 234: @6542
    GIM_Try, /*On fail goto*//*Label 235*/ 6586, // Rule ID 200 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi8>>  =>  (LB:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 200,
      GIR_Done,
    // Label 235: @6586
    GIM_Try, /*On fail goto*//*Label 236*/ 6630, // Rule ID 228 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi16>>  =>  (LH:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 228,
      GIR_Done,
    // Label 236: @6630
    GIM_Try, /*On fail goto*//*Label 237*/ 6674, // Rule ID 230 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi16>>  =>  (LH:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 230,
      GIR_Done,
    // Label 237: @6674
    GIM_Try, /*On fail goto*//*Label 238*/ 6720, // Rule ID 378 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi32>>  =>  (LW:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 378,
      GIR_Done,
    // Label 238: @6720
    GIM_Try, /*On fail goto*//*Label 239*/ 6766, // Rule ID 380 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi32>>  =>  (LW:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 380,
      GIR_Done,
    // Label 239: @6766
    GIM_Reject,
    // Label 226: @6767
    GIM_Try, /*On fail goto*//*Label 240*/ 6842, // Rule ID 205 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i64] } (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi8>>  =>  (LB:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 205,
      GIR_Done,
    // Label 240: @6842
    GIM_Try, /*On fail goto*//*Label 241*/ 6917, // Rule ID 235 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i64] } (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi16>>  =>  (LH:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 235,
      GIR_Done,
    // Label 241: @6917
    GIM_Try, /*On fail goto*//*Label 242*/ 6994, // Rule ID 385 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i64] } (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi32>>  =>  (LW:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 385,
      GIR_Done,
    // Label 242: @6994
    GIM_Try, /*On fail goto*//*Label 243*/ 7038, // Rule ID 199 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i64] } GPR:{ *:[i64] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi8>>  =>  (LB:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LB,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 199,
      GIR_Done,
    // Label 243: @7038
    GIM_Try, /*On fail goto*//*Label 244*/ 7082, // Rule ID 229 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i64] } GPR:{ *:[i64] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi16>>  =>  (LH:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LH,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 229,
      GIR_Done,
    // Label 244: @7082
    GIM_Try, /*On fail goto*//*Label 245*/ 7128, // Rule ID 379 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i64] } GPR:{ *:[i64] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_sextload>><<P:Predicate_sextloadi32>>  =>  (LW:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LW,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 379,
      GIR_Done,
    // Label 245: @7128
    GIM_Reject,
    // Label 227: @7129
    GIM_Reject,
    // Label 14: @7130
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 248*/ 8223,
    /*GILLT_s32*//*Label 246*/ 7138,
    /*GILLT_s64*//*Label 247*/ 7861,
    // Label 246: @7138
    GIM_Try, /*On fail goto*//*Label 249*/ 7213, // Rule ID 279 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi8>>  =>  (LBU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LBU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 279,
      GIR_Done,
    // Label 249: @7213
    GIM_Try, /*On fail goto*//*Label 250*/ 7288, // Rule ID 281 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi8>>  =>  (LBU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LBU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 281,
      GIR_Done,
    // Label 250: @7288
    GIM_Try, /*On fail goto*//*Label 251*/ 7363, // Rule ID 294 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi16>>  =>  (LHU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LHU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 294,
      GIR_Done,
    // Label 251: @7363
    GIM_Try, /*On fail goto*//*Label 252*/ 7438, // Rule ID 296 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi16>>  =>  (LHU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LHU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 296,
      GIR_Done,
    // Label 252: @7438
    GIM_Try, /*On fail goto*//*Label 253*/ 7515, // Rule ID 414 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi32>>  =>  (LWU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LWU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 414,
      GIR_Done,
    // Label 253: @7515
    GIM_Try, /*On fail goto*//*Label 254*/ 7592, // Rule ID 416 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i32] } (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi32>>  =>  (LWU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LWU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 416,
      GIR_Done,
    // Label 254: @7592
    GIM_Try, /*On fail goto*//*Label 255*/ 7636, // Rule ID 273 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi8>>  =>  (LBU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LBU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 273,
      GIR_Done,
    // Label 255: @7636
    GIM_Try, /*On fail goto*//*Label 256*/ 7680, // Rule ID 275 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi8>>  =>  (LBU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LBU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 275,
      GIR_Done,
    // Label 256: @7680
    GIM_Try, /*On fail goto*//*Label 257*/ 7724, // Rule ID 288 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi16>>  =>  (LHU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LHU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 288,
      GIR_Done,
    // Label 257: @7724
    GIM_Try, /*On fail goto*//*Label 258*/ 7768, // Rule ID 290 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi16>>  =>  (LHU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LHU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 290,
      GIR_Done,
    // Label 258: @7768
    GIM_Try, /*On fail goto*//*Label 259*/ 7814, // Rule ID 408 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi32>>  =>  (LWU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LWU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 408,
      GIR_Done,
    // Label 259: @7814
    GIM_Try, /*On fail goto*//*Label 260*/ 7860, // Rule ID 410 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i32] } GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi32>>  =>  (LWU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LWU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 410,
      GIR_Done,
    // Label 260: @7860
    GIM_Reject,
    // Label 247: @7861
    GIM_Try, /*On fail goto*//*Label 261*/ 7936, // Rule ID 280 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i64] } (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi8>>  =>  (LBU:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LBU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 280,
      GIR_Done,
    // Label 261: @7936
    GIM_Try, /*On fail goto*//*Label 262*/ 8011, // Rule ID 295 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i64] } (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi16>>  =>  (LHU:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LHU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 295,
      GIR_Done,
    // Label 262: @8011
    GIM_Try, /*On fail goto*//*Label 263*/ 8088, // Rule ID 415 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
      GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
      GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
      GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
      GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
      // MIs[2] Operand 1
      // No operand predicates
      GIM_CheckIsSafeToFold, /*InsnID*/1,
      GIM_CheckIsSafeToFold, /*InsnID*/2,
      // (ld:{ *:[i64] } (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi32>>  =>  (LWU:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LWU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 415,
      GIR_Done,
    // Label 263: @8088
    GIM_Try, /*On fail goto*//*Label 264*/ 8132, // Rule ID 274 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/1,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i64] } GPR:{ *:[i64] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi8>>  =>  (LBU:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LBU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 274,
      GIR_Done,
    // Label 264: @8132
    GIM_Try, /*On fail goto*//*Label 265*/ 8176, // Rule ID 289 //
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/2,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i64] } GPR:{ *:[i64] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi16>>  =>  (LHU:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LHU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 289,
      GIR_Done,
    // Label 265: @8176
    GIM_Try, /*On fail goto*//*Label 266*/ 8222, // Rule ID 409 //
      GIM_CheckFeatures, GIFBS_IsRV64,
      GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] rs1
      GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      // (ld:{ *:[i64] } GPR:{ *:[i64] }:$rs1)<<P:Predicate_unindexedload>><<P:Predicate_zextload>><<P:Predicate_zextloadi32>>  =>  (LWU:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::LWU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
      GIR_AddImm, /*InsnID*/0, /*Imm*/0,
      GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 409,
      GIR_Done,
    // Label 266: @8222
    GIM_Reject,
    // Label 248: @8223
    GIM_Reject,
    // Label 15: @8224
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 269*/ 9562,
    /*GILLT_s32*//*Label 267*/ 8232,
    /*GILLT_s64*//*Label 268*/ 9006,
    // Label 267: @8232
    GIM_Try, /*On fail goto*//*Label 270*/ 9005,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_Try, /*On fail goto*//*Label 271*/ 8311, // Rule ID 339 //
        GIM_CheckFeatures, GIFBS_IsRV32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] Operand 1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (st GPR:{ *:[i32] }:$rs2, (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (SW GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SW,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 339,
        GIR_Done,
      // Label 271: @8311
      GIM_Try, /*On fail goto*//*Label 272*/ 8381, // Rule ID 341 //
        GIM_CheckFeatures, GIFBS_IsRV32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] Operand 1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (st GPR:{ *:[i32] }:$rs2, (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (SW GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SW,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 341,
        GIR_Done,
      // Label 272: @8381
      GIM_Try, /*On fail goto*//*Label 273*/ 8451, // Rule ID 459 //
        GIM_CheckFeatures, GIFBS_IsRV64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] Operand 1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (st GPR:{ *:[i32] }:$rs2, (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (SD GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SD,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 459,
        GIR_Done,
      // Label 273: @8451
      GIM_Try, /*On fail goto*//*Label 274*/ 8521, // Rule ID 461 //
        GIM_CheckFeatures, GIFBS_IsRV64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] Operand 1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (st GPR:{ *:[i32] }:$rs2, (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (SD GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SD,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 461,
        GIR_Done,
      // Label 274: @8521
      GIM_Try, /*On fail goto*//*Label 275*/ 8591, // Rule ID 1151 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
        // MIs[0] Operand 1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (st FPR32:{ *:[f32] }:$rs2, (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (FSW FPR32:{ *:[f32] }:$rs2, GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSW,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1151,
        GIR_Done,
      // Label 275: @8591
      GIM_Try, /*On fail goto*//*Label 276*/ 8661, // Rule ID 1152 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
        // MIs[0] Operand 1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (st FPR32:{ *:[f32] }:$rs2, (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (FSW FPR32:{ *:[f32] }:$rs2, GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSW,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1152,
        GIR_Done,
      // Label 276: @8661
      GIM_Try, /*On fail goto*//*Label 277*/ 8731, // Rule ID 1153 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
        // MIs[0] Operand 1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (st FPR32:{ *:[f32] }:$rs2, (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (FSW FPR32:{ *:[f32] }:$rs2, GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSW,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1153,
        GIR_Done,
      // Label 277: @8731
      GIM_Try, /*On fail goto*//*Label 278*/ 8770, // Rule ID 333 //
        GIM_CheckFeatures, GIFBS_IsRV32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (st GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (SW GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SW,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 333,
        GIR_Done,
      // Label 278: @8770
      GIM_Try, /*On fail goto*//*Label 279*/ 8809, // Rule ID 335 //
        GIM_CheckFeatures, GIFBS_IsRV32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (st GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (SW GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SW,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 335,
        GIR_Done,
      // Label 279: @8809
      GIM_Try, /*On fail goto*//*Label 280*/ 8848, // Rule ID 453 //
        GIM_CheckFeatures, GIFBS_IsRV64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (st GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (SD GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SD,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 453,
        GIR_Done,
      // Label 280: @8848
      GIM_Try, /*On fail goto*//*Label 281*/ 8887, // Rule ID 455 //
        GIM_CheckFeatures, GIFBS_IsRV64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (st GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (SD GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SD,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 455,
        GIR_Done,
      // Label 281: @8887
      GIM_Try, /*On fail goto*//*Label 282*/ 8926, // Rule ID 1145 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (st FPR32:{ *:[f32] }:$rs2, GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (FSW FPR32:{ *:[f32] }:$rs2, GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSW,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1145,
        GIR_Done,
      // Label 282: @8926
      GIM_Try, /*On fail goto*//*Label 283*/ 8965, // Rule ID 1146 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (st FPR32:{ *:[f32] }:$rs2, GPR:{ *:[i64] }:$rs1)<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (FSW FPR32:{ *:[f32] }:$rs2, GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSW,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1146,
        GIR_Done,
      // Label 283: @8965
      GIM_Try, /*On fail goto*//*Label 284*/ 9004, // Rule ID 1147 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (st FPR32:{ *:[f32] }:$rs2, GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (FSW FPR32:{ *:[f32] }:$rs2, GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSW,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1147,
        GIR_Done,
      // Label 284: @9004
      GIM_Reject,
    // Label 270: @9005
    GIM_Reject,
    // Label 268: @9006
    GIM_Try, /*On fail goto*//*Label 285*/ 9561,
      GIM_CheckMemorySizeEqualToLLT, /*MI*/0, /*MMO*/0, /*OpIdx*/0,
      GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::NotAtomic,
      GIM_Try, /*On fail goto*//*Label 286*/ 9085, // Rule ID 340 //
        GIM_CheckFeatures, GIFBS_IsRV32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] Operand 1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (st GPR:{ *:[i64] }:$rs2, (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (SW GPR:{ *:[i64] }:$rs2, GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SW,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 340,
        GIR_Done,
      // Label 286: @9085
      GIM_Try, /*On fail goto*//*Label 287*/ 9155, // Rule ID 460 //
        GIM_CheckFeatures, GIFBS_IsRV64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] Operand 1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (st GPR:{ *:[i64] }:$rs2, (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (SD GPR:{ *:[i64] }:$rs2, GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SD,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 460,
        GIR_Done,
      // Label 287: @9155
      GIM_Try, /*On fail goto*//*Label 288*/ 9225, // Rule ID 1294 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
        // MIs[0] Operand 1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (st FPR64:{ *:[f64] }:$rs2, (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (FSD FPR64:{ *:[f64] }:$rs2, GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSD,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1294,
        GIR_Done,
      // Label 288: @9225
      GIM_Try, /*On fail goto*//*Label 289*/ 9295, // Rule ID 1295 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
        // MIs[0] Operand 1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (st FPR64:{ *:[f64] }:$rs2, (add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (FSD FPR64:{ *:[f64] }:$rs2, GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSD,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1295,
        GIR_Done,
      // Label 289: @9295
      GIM_Try, /*On fail goto*//*Label 290*/ 9365, // Rule ID 1296 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
        // MIs[0] Operand 1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_ADD,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (st FPR64:{ *:[f64] }:$rs2, (add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (FSD FPR64:{ *:[f64] }:$rs2, GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSD,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/2, // imm12
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, 1, 2, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1296,
        GIR_Done,
      // Label 290: @9365
      GIM_Try, /*On fail goto*//*Label 291*/ 9404, // Rule ID 334 //
        GIM_CheckFeatures, GIFBS_IsRV32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (st GPR:{ *:[i64] }:$rs2, GPR:{ *:[i64] }:$rs1)<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (SW GPR:{ *:[i64] }:$rs2, GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SW,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 334,
        GIR_Done,
      // Label 291: @9404
      GIM_Try, /*On fail goto*//*Label 292*/ 9443, // Rule ID 454 //
        GIM_CheckFeatures, GIFBS_IsRV64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (st GPR:{ *:[i64] }:$rs2, GPR:{ *:[i64] }:$rs1)<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (SD GPR:{ *:[i64] }:$rs2, GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SD,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 454,
        GIR_Done,
      // Label 292: @9443
      GIM_Try, /*On fail goto*//*Label 293*/ 9482, // Rule ID 1288 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (st FPR64:{ *:[f64] }:$rs2, GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (FSD FPR64:{ *:[f64] }:$rs2, GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSD,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1288,
        GIR_Done,
      // Label 293: @9482
      GIM_Try, /*On fail goto*//*Label 294*/ 9521, // Rule ID 1289 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (st FPR64:{ *:[f64] }:$rs2, GPR:{ *:[i64] }:$rs1)<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (FSD FPR64:{ *:[f64] }:$rs2, GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSD,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1289,
        GIR_Done,
      // Label 294: @9521
      GIM_Try, /*On fail goto*//*Label 295*/ 9560, // Rule ID 1290 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (st FPR64:{ *:[f64] }:$rs2, GPR:{ *:[i32] }:$rs1)<<P:Predicate_unindexedstore>><<P:Predicate_store>>  =>  (FSD FPR64:{ *:[f64] }:$rs2, GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSD,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1290,
        GIR_Done,
      // Label 295: @9560
      GIM_Reject,
    // Label 285: @9561
    GIM_Reject,
    // Label 269: @9562
    GIM_Reject,
    // Label 16: @9563
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 298*/ 10607,
    /*GILLT_s32*//*Label 296*/ 9571,
    /*GILLT_s64*//*Label 297*/ 10259,
    // Label 296: @9571
    GIM_Try, /*On fail goto*//*Label 299*/ 10258,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_Try, /*On fail goto*//*Label 300*/ 9611, // Rule ID 650 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_32>><<P:Predicate_atomic_swap_32_monotonic>>  =>  (AMOSWAP_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 650,
        GIR_Done,
      // Label 300: @9611
      GIM_Try, /*On fail goto*//*Label 301*/ 9645, // Rule ID 652 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_32>><<P:Predicate_atomic_swap_32_monotonic>>  =>  (AMOSWAP_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 652,
        GIR_Done,
      // Label 301: @9645
      GIM_Try, /*On fail goto*//*Label 302*/ 9679, // Rule ID 653 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_32>><<P:Predicate_atomic_swap_32_acquire>>  =>  (AMOSWAP_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 653,
        GIR_Done,
      // Label 302: @9679
      GIM_Try, /*On fail goto*//*Label 303*/ 9713, // Rule ID 655 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_32>><<P:Predicate_atomic_swap_32_acquire>>  =>  (AMOSWAP_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 655,
        GIR_Done,
      // Label 303: @9713
      GIM_Try, /*On fail goto*//*Label 304*/ 9747, // Rule ID 656 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_32>><<P:Predicate_atomic_swap_32_release>>  =>  (AMOSWAP_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 656,
        GIR_Done,
      // Label 304: @9747
      GIM_Try, /*On fail goto*//*Label 305*/ 9781, // Rule ID 658 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_32>><<P:Predicate_atomic_swap_32_release>>  =>  (AMOSWAP_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 658,
        GIR_Done,
      // Label 305: @9781
      GIM_Try, /*On fail goto*//*Label 306*/ 9815, // Rule ID 659 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_32>><<P:Predicate_atomic_swap_32_acq_rel>>  =>  (AMOSWAP_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 659,
        GIR_Done,
      // Label 306: @9815
      GIM_Try, /*On fail goto*//*Label 307*/ 9849, // Rule ID 661 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_32>><<P:Predicate_atomic_swap_32_acq_rel>>  =>  (AMOSWAP_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 661,
        GIR_Done,
      // Label 307: @9849
      GIM_Try, /*On fail goto*//*Label 308*/ 9883, // Rule ID 662 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_32>><<P:Predicate_atomic_swap_32_seq_cst>>  =>  (AMOSWAP_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 662,
        GIR_Done,
      // Label 308: @9883
      GIM_Try, /*On fail goto*//*Label 309*/ 9917, // Rule ID 664 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_32>><<P:Predicate_atomic_swap_32_seq_cst>>  =>  (AMOSWAP_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 664,
        GIR_Done,
      // Label 309: @9917
      GIM_Try, /*On fail goto*//*Label 310*/ 9951, // Rule ID 878 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_64>><<P:Predicate_atomic_swap_64_monotonic>>  =>  (AMOSWAP_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 878,
        GIR_Done,
      // Label 310: @9951
      GIM_Try, /*On fail goto*//*Label 311*/ 9985, // Rule ID 880 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_64>><<P:Predicate_atomic_swap_64_monotonic>>  =>  (AMOSWAP_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 880,
        GIR_Done,
      // Label 311: @9985
      GIM_Try, /*On fail goto*//*Label 312*/ 10019, // Rule ID 881 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_64>><<P:Predicate_atomic_swap_64_acquire>>  =>  (AMOSWAP_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 881,
        GIR_Done,
      // Label 312: @10019
      GIM_Try, /*On fail goto*//*Label 313*/ 10053, // Rule ID 883 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_64>><<P:Predicate_atomic_swap_64_acquire>>  =>  (AMOSWAP_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 883,
        GIR_Done,
      // Label 313: @10053
      GIM_Try, /*On fail goto*//*Label 314*/ 10087, // Rule ID 884 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_64>><<P:Predicate_atomic_swap_64_release>>  =>  (AMOSWAP_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 884,
        GIR_Done,
      // Label 314: @10087
      GIM_Try, /*On fail goto*//*Label 315*/ 10121, // Rule ID 886 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_64>><<P:Predicate_atomic_swap_64_release>>  =>  (AMOSWAP_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 886,
        GIR_Done,
      // Label 315: @10121
      GIM_Try, /*On fail goto*//*Label 316*/ 10155, // Rule ID 887 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_64>><<P:Predicate_atomic_swap_64_acq_rel>>  =>  (AMOSWAP_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 887,
        GIR_Done,
      // Label 316: @10155
      GIM_Try, /*On fail goto*//*Label 317*/ 10189, // Rule ID 889 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_64>><<P:Predicate_atomic_swap_64_acq_rel>>  =>  (AMOSWAP_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 889,
        GIR_Done,
      // Label 317: @10189
      GIM_Try, /*On fail goto*//*Label 318*/ 10223, // Rule ID 890 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_64>><<P:Predicate_atomic_swap_64_seq_cst>>  =>  (AMOSWAP_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 890,
        GIR_Done,
      // Label 318: @10223
      GIM_Try, /*On fail goto*//*Label 319*/ 10257, // Rule ID 892 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_swap_64>><<P:Predicate_atomic_swap_64_seq_cst>>  =>  (AMOSWAP_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 892,
        GIR_Done,
      // Label 319: @10257
      GIM_Reject,
    // Label 299: @10258
    GIM_Reject,
    // Label 297: @10259
    GIM_Try, /*On fail goto*//*Label 320*/ 10606,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_Try, /*On fail goto*//*Label 321*/ 10299, // Rule ID 651 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_swap_32>><<P:Predicate_atomic_swap_32_monotonic>>  =>  (AMOSWAP_W:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 651,
        GIR_Done,
      // Label 321: @10299
      GIM_Try, /*On fail goto*//*Label 322*/ 10333, // Rule ID 654 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_swap_32>><<P:Predicate_atomic_swap_32_acquire>>  =>  (AMOSWAP_W_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 654,
        GIR_Done,
      // Label 322: @10333
      GIM_Try, /*On fail goto*//*Label 323*/ 10367, // Rule ID 657 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_swap_32>><<P:Predicate_atomic_swap_32_release>>  =>  (AMOSWAP_W_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 657,
        GIR_Done,
      // Label 323: @10367
      GIM_Try, /*On fail goto*//*Label 324*/ 10401, // Rule ID 660 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_swap_32>><<P:Predicate_atomic_swap_32_acq_rel>>  =>  (AMOSWAP_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 660,
        GIR_Done,
      // Label 324: @10401
      GIM_Try, /*On fail goto*//*Label 325*/ 10435, // Rule ID 663 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_swap_32>><<P:Predicate_atomic_swap_32_seq_cst>>  =>  (AMOSWAP_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 663,
        GIR_Done,
      // Label 325: @10435
      GIM_Try, /*On fail goto*//*Label 326*/ 10469, // Rule ID 879 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_swap_64>><<P:Predicate_atomic_swap_64_monotonic>>  =>  (AMOSWAP_D:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 879,
        GIR_Done,
      // Label 326: @10469
      GIM_Try, /*On fail goto*//*Label 327*/ 10503, // Rule ID 882 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_swap_64>><<P:Predicate_atomic_swap_64_acquire>>  =>  (AMOSWAP_D_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 882,
        GIR_Done,
      // Label 327: @10503
      GIM_Try, /*On fail goto*//*Label 328*/ 10537, // Rule ID 885 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_swap_64>><<P:Predicate_atomic_swap_64_release>>  =>  (AMOSWAP_D_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 885,
        GIR_Done,
      // Label 328: @10537
      GIM_Try, /*On fail goto*//*Label 329*/ 10571, // Rule ID 888 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_swap_64>><<P:Predicate_atomic_swap_64_acq_rel>>  =>  (AMOSWAP_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 888,
        GIR_Done,
      // Label 329: @10571
      GIM_Try, /*On fail goto*//*Label 330*/ 10605, // Rule ID 891 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_swap:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_swap_64>><<P:Predicate_atomic_swap_64_seq_cst>>  =>  (AMOSWAP_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOSWAP_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 891,
        GIR_Done,
      // Label 330: @10605
      GIM_Reject,
    // Label 320: @10606
    GIM_Reject,
    // Label 298: @10607
    GIM_Reject,
    // Label 17: @10608
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 333*/ 11652,
    /*GILLT_s32*//*Label 331*/ 10616,
    /*GILLT_s64*//*Label 332*/ 11304,
    // Label 331: @10616
    GIM_Try, /*On fail goto*//*Label 334*/ 11303,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_Try, /*On fail goto*//*Label 335*/ 10656, // Rule ID 665 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_32>><<P:Predicate_atomic_load_add_32_monotonic>>  =>  (AMOADD_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 665,
        GIR_Done,
      // Label 335: @10656
      GIM_Try, /*On fail goto*//*Label 336*/ 10690, // Rule ID 667 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_32>><<P:Predicate_atomic_load_add_32_monotonic>>  =>  (AMOADD_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 667,
        GIR_Done,
      // Label 336: @10690
      GIM_Try, /*On fail goto*//*Label 337*/ 10724, // Rule ID 668 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_32>><<P:Predicate_atomic_load_add_32_acquire>>  =>  (AMOADD_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 668,
        GIR_Done,
      // Label 337: @10724
      GIM_Try, /*On fail goto*//*Label 338*/ 10758, // Rule ID 670 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_32>><<P:Predicate_atomic_load_add_32_acquire>>  =>  (AMOADD_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 670,
        GIR_Done,
      // Label 338: @10758
      GIM_Try, /*On fail goto*//*Label 339*/ 10792, // Rule ID 671 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_32>><<P:Predicate_atomic_load_add_32_release>>  =>  (AMOADD_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 671,
        GIR_Done,
      // Label 339: @10792
      GIM_Try, /*On fail goto*//*Label 340*/ 10826, // Rule ID 673 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_32>><<P:Predicate_atomic_load_add_32_release>>  =>  (AMOADD_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 673,
        GIR_Done,
      // Label 340: @10826
      GIM_Try, /*On fail goto*//*Label 341*/ 10860, // Rule ID 674 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_32>><<P:Predicate_atomic_load_add_32_acq_rel>>  =>  (AMOADD_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 674,
        GIR_Done,
      // Label 341: @10860
      GIM_Try, /*On fail goto*//*Label 342*/ 10894, // Rule ID 676 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_32>><<P:Predicate_atomic_load_add_32_acq_rel>>  =>  (AMOADD_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 676,
        GIR_Done,
      // Label 342: @10894
      GIM_Try, /*On fail goto*//*Label 343*/ 10928, // Rule ID 677 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_32>><<P:Predicate_atomic_load_add_32_seq_cst>>  =>  (AMOADD_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 677,
        GIR_Done,
      // Label 343: @10928
      GIM_Try, /*On fail goto*//*Label 344*/ 10962, // Rule ID 679 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_32>><<P:Predicate_atomic_load_add_32_seq_cst>>  =>  (AMOADD_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 679,
        GIR_Done,
      // Label 344: @10962
      GIM_Try, /*On fail goto*//*Label 345*/ 10996, // Rule ID 893 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_64>><<P:Predicate_atomic_load_add_64_monotonic>>  =>  (AMOADD_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 893,
        GIR_Done,
      // Label 345: @10996
      GIM_Try, /*On fail goto*//*Label 346*/ 11030, // Rule ID 895 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_64>><<P:Predicate_atomic_load_add_64_monotonic>>  =>  (AMOADD_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 895,
        GIR_Done,
      // Label 346: @11030
      GIM_Try, /*On fail goto*//*Label 347*/ 11064, // Rule ID 896 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_64>><<P:Predicate_atomic_load_add_64_acquire>>  =>  (AMOADD_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 896,
        GIR_Done,
      // Label 347: @11064
      GIM_Try, /*On fail goto*//*Label 348*/ 11098, // Rule ID 898 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_64>><<P:Predicate_atomic_load_add_64_acquire>>  =>  (AMOADD_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 898,
        GIR_Done,
      // Label 348: @11098
      GIM_Try, /*On fail goto*//*Label 349*/ 11132, // Rule ID 899 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_64>><<P:Predicate_atomic_load_add_64_release>>  =>  (AMOADD_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 899,
        GIR_Done,
      // Label 349: @11132
      GIM_Try, /*On fail goto*//*Label 350*/ 11166, // Rule ID 901 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_64>><<P:Predicate_atomic_load_add_64_release>>  =>  (AMOADD_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 901,
        GIR_Done,
      // Label 350: @11166
      GIM_Try, /*On fail goto*//*Label 351*/ 11200, // Rule ID 902 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_64>><<P:Predicate_atomic_load_add_64_acq_rel>>  =>  (AMOADD_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 902,
        GIR_Done,
      // Label 351: @11200
      GIM_Try, /*On fail goto*//*Label 352*/ 11234, // Rule ID 904 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_64>><<P:Predicate_atomic_load_add_64_acq_rel>>  =>  (AMOADD_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 904,
        GIR_Done,
      // Label 352: @11234
      GIM_Try, /*On fail goto*//*Label 353*/ 11268, // Rule ID 905 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_64>><<P:Predicate_atomic_load_add_64_seq_cst>>  =>  (AMOADD_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 905,
        GIR_Done,
      // Label 353: @11268
      GIM_Try, /*On fail goto*//*Label 354*/ 11302, // Rule ID 907 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_add_64>><<P:Predicate_atomic_load_add_64_seq_cst>>  =>  (AMOADD_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 907,
        GIR_Done,
      // Label 354: @11302
      GIM_Reject,
    // Label 334: @11303
    GIM_Reject,
    // Label 332: @11304
    GIM_Try, /*On fail goto*//*Label 355*/ 11651,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_Try, /*On fail goto*//*Label 356*/ 11344, // Rule ID 666 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_add_32>><<P:Predicate_atomic_load_add_32_monotonic>>  =>  (AMOADD_W:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 666,
        GIR_Done,
      // Label 356: @11344
      GIM_Try, /*On fail goto*//*Label 357*/ 11378, // Rule ID 669 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_add_32>><<P:Predicate_atomic_load_add_32_acquire>>  =>  (AMOADD_W_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 669,
        GIR_Done,
      // Label 357: @11378
      GIM_Try, /*On fail goto*//*Label 358*/ 11412, // Rule ID 672 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_add_32>><<P:Predicate_atomic_load_add_32_release>>  =>  (AMOADD_W_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 672,
        GIR_Done,
      // Label 358: @11412
      GIM_Try, /*On fail goto*//*Label 359*/ 11446, // Rule ID 675 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_add_32>><<P:Predicate_atomic_load_add_32_acq_rel>>  =>  (AMOADD_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 675,
        GIR_Done,
      // Label 359: @11446
      GIM_Try, /*On fail goto*//*Label 360*/ 11480, // Rule ID 678 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_add_32>><<P:Predicate_atomic_load_add_32_seq_cst>>  =>  (AMOADD_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 678,
        GIR_Done,
      // Label 360: @11480
      GIM_Try, /*On fail goto*//*Label 361*/ 11514, // Rule ID 894 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_add_64>><<P:Predicate_atomic_load_add_64_monotonic>>  =>  (AMOADD_D:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 894,
        GIR_Done,
      // Label 361: @11514
      GIM_Try, /*On fail goto*//*Label 362*/ 11548, // Rule ID 897 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_add_64>><<P:Predicate_atomic_load_add_64_acquire>>  =>  (AMOADD_D_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 897,
        GIR_Done,
      // Label 362: @11548
      GIM_Try, /*On fail goto*//*Label 363*/ 11582, // Rule ID 900 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_add_64>><<P:Predicate_atomic_load_add_64_release>>  =>  (AMOADD_D_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 900,
        GIR_Done,
      // Label 363: @11582
      GIM_Try, /*On fail goto*//*Label 364*/ 11616, // Rule ID 903 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_add_64>><<P:Predicate_atomic_load_add_64_acq_rel>>  =>  (AMOADD_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 903,
        GIR_Done,
      // Label 364: @11616
      GIM_Try, /*On fail goto*//*Label 365*/ 11650, // Rule ID 906 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_add:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_add_64>><<P:Predicate_atomic_load_add_64_seq_cst>>  =>  (AMOADD_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 906,
        GIR_Done,
      // Label 365: @11650
      GIM_Reject,
    // Label 355: @11651
    GIM_Reject,
    // Label 333: @11652
    GIM_Reject,
    // Label 18: @11653
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 368*/ 13807,
    /*GILLT_s32*//*Label 366*/ 11661,
    /*GILLT_s64*//*Label 367*/ 13089,
    // Label 366: @11661
    GIM_Try, /*On fail goto*//*Label 369*/ 13088,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_Try, /*On fail goto*//*Label 370*/ 11738, // Rule ID 785 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_32>><<P:Predicate_atomic_load_sub_32_monotonic>>  =>  (AMOADD_W:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_W,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 785,
        GIR_Done,
      // Label 370: @11738
      GIM_Try, /*On fail goto*//*Label 371*/ 11809, // Rule ID 787 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_32>><<P:Predicate_atomic_load_sub_32_monotonic>>  =>  (AMOADD_W:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_W,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 787,
        GIR_Done,
      // Label 371: @11809
      GIM_Try, /*On fail goto*//*Label 372*/ 11880, // Rule ID 788 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_32>><<P:Predicate_atomic_load_sub_32_acquire>>  =>  (AMOADD_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 788,
        GIR_Done,
      // Label 372: @11880
      GIM_Try, /*On fail goto*//*Label 373*/ 11951, // Rule ID 790 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_32>><<P:Predicate_atomic_load_sub_32_acquire>>  =>  (AMOADD_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 790,
        GIR_Done,
      // Label 373: @11951
      GIM_Try, /*On fail goto*//*Label 374*/ 12022, // Rule ID 791 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_32>><<P:Predicate_atomic_load_sub_32_release>>  =>  (AMOADD_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_W_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 791,
        GIR_Done,
      // Label 374: @12022
      GIM_Try, /*On fail goto*//*Label 375*/ 12093, // Rule ID 793 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_32>><<P:Predicate_atomic_load_sub_32_release>>  =>  (AMOADD_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_W_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 793,
        GIR_Done,
      // Label 375: @12093
      GIM_Try, /*On fail goto*//*Label 376*/ 12164, // Rule ID 794 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_32>><<P:Predicate_atomic_load_sub_32_acq_rel>>  =>  (AMOADD_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 794,
        GIR_Done,
      // Label 376: @12164
      GIM_Try, /*On fail goto*//*Label 377*/ 12235, // Rule ID 796 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_32>><<P:Predicate_atomic_load_sub_32_acq_rel>>  =>  (AMOADD_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 796,
        GIR_Done,
      // Label 377: @12235
      GIM_Try, /*On fail goto*//*Label 378*/ 12306, // Rule ID 797 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_32>><<P:Predicate_atomic_load_sub_32_seq_cst>>  =>  (AMOADD_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 797,
        GIR_Done,
      // Label 378: @12306
      GIM_Try, /*On fail goto*//*Label 379*/ 12377, // Rule ID 799 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_32>><<P:Predicate_atomic_load_sub_32_seq_cst>>  =>  (AMOADD_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 799,
        GIR_Done,
      // Label 379: @12377
      GIM_Try, /*On fail goto*//*Label 380*/ 12448, // Rule ID 1013 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_64>><<P:Predicate_atomic_load_sub_64_monotonic>>  =>  (AMOADD_D:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1013,
        GIR_Done,
      // Label 380: @12448
      GIM_Try, /*On fail goto*//*Label 381*/ 12519, // Rule ID 1015 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_64>><<P:Predicate_atomic_load_sub_64_monotonic>>  =>  (AMOADD_D:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1015,
        GIR_Done,
      // Label 381: @12519
      GIM_Try, /*On fail goto*//*Label 382*/ 12590, // Rule ID 1016 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_64>><<P:Predicate_atomic_load_sub_64_acquire>>  =>  (AMOADD_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1016,
        GIR_Done,
      // Label 382: @12590
      GIM_Try, /*On fail goto*//*Label 383*/ 12661, // Rule ID 1018 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_64>><<P:Predicate_atomic_load_sub_64_acquire>>  =>  (AMOADD_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1018,
        GIR_Done,
      // Label 383: @12661
      GIM_Try, /*On fail goto*//*Label 384*/ 12732, // Rule ID 1019 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_64>><<P:Predicate_atomic_load_sub_64_release>>  =>  (AMOADD_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_D_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1019,
        GIR_Done,
      // Label 384: @12732
      GIM_Try, /*On fail goto*//*Label 385*/ 12803, // Rule ID 1021 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_64>><<P:Predicate_atomic_load_sub_64_release>>  =>  (AMOADD_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_D_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1021,
        GIR_Done,
      // Label 385: @12803
      GIM_Try, /*On fail goto*//*Label 386*/ 12874, // Rule ID 1022 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_64>><<P:Predicate_atomic_load_sub_64_acq_rel>>  =>  (AMOADD_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1022,
        GIR_Done,
      // Label 386: @12874
      GIM_Try, /*On fail goto*//*Label 387*/ 12945, // Rule ID 1024 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_64>><<P:Predicate_atomic_load_sub_64_acq_rel>>  =>  (AMOADD_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1024,
        GIR_Done,
      // Label 387: @12945
      GIM_Try, /*On fail goto*//*Label 388*/ 13016, // Rule ID 1025 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_64>><<P:Predicate_atomic_load_sub_64_seq_cst>>  =>  (AMOADD_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1025,
        GIR_Done,
      // Label 388: @13016
      GIM_Try, /*On fail goto*//*Label 389*/ 13087, // Rule ID 1027 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i32] } GPR:{ *:[i32] }:$addr, GPR:{ *:[i32] }:$incr)<<P:Predicate_atomic_load_sub_64>><<P:Predicate_atomic_load_sub_64_seq_cst>>  =>  (AMOADD_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$addr, (SUB:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1027,
        GIR_Done,
      // Label 389: @13087
      GIM_Reject,
    // Label 369: @13088
    GIM_Reject,
    // Label 367: @13089
    GIM_Try, /*On fail goto*//*Label 390*/ 13806,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_Try, /*On fail goto*//*Label 391*/ 13166, // Rule ID 786 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i64] } GPR:{ *:[i64] }:$addr, GPR:{ *:[i64] }:$incr)<<P:Predicate_atomic_load_sub_32>><<P:Predicate_atomic_load_sub_32_monotonic>>  =>  (AMOADD_W:{ *:[i64] } GPR:{ *:[i64] }:$addr, (SUB:{ *:[i64] } X0:{ *:[i64] }, GPR:{ *:[i64] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_W,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 786,
        GIR_Done,
      // Label 391: @13166
      GIM_Try, /*On fail goto*//*Label 392*/ 13237, // Rule ID 789 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i64] } GPR:{ *:[i64] }:$addr, GPR:{ *:[i64] }:$incr)<<P:Predicate_atomic_load_sub_32>><<P:Predicate_atomic_load_sub_32_acquire>>  =>  (AMOADD_W_AQ:{ *:[i64] } GPR:{ *:[i64] }:$addr, (SUB:{ *:[i64] } X0:{ *:[i64] }, GPR:{ *:[i64] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 789,
        GIR_Done,
      // Label 392: @13237
      GIM_Try, /*On fail goto*//*Label 393*/ 13308, // Rule ID 792 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i64] } GPR:{ *:[i64] }:$addr, GPR:{ *:[i64] }:$incr)<<P:Predicate_atomic_load_sub_32>><<P:Predicate_atomic_load_sub_32_release>>  =>  (AMOADD_W_RL:{ *:[i64] } GPR:{ *:[i64] }:$addr, (SUB:{ *:[i64] } X0:{ *:[i64] }, GPR:{ *:[i64] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_W_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 792,
        GIR_Done,
      // Label 393: @13308
      GIM_Try, /*On fail goto*//*Label 394*/ 13379, // Rule ID 795 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i64] } GPR:{ *:[i64] }:$addr, GPR:{ *:[i64] }:$incr)<<P:Predicate_atomic_load_sub_32>><<P:Predicate_atomic_load_sub_32_acq_rel>>  =>  (AMOADD_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$addr, (SUB:{ *:[i64] } X0:{ *:[i64] }, GPR:{ *:[i64] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 795,
        GIR_Done,
      // Label 394: @13379
      GIM_Try, /*On fail goto*//*Label 395*/ 13450, // Rule ID 798 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i64] } GPR:{ *:[i64] }:$addr, GPR:{ *:[i64] }:$incr)<<P:Predicate_atomic_load_sub_32>><<P:Predicate_atomic_load_sub_32_seq_cst>>  =>  (AMOADD_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$addr, (SUB:{ *:[i64] } X0:{ *:[i64] }, GPR:{ *:[i64] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_W_AQ_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 798,
        GIR_Done,
      // Label 395: @13450
      GIM_Try, /*On fail goto*//*Label 396*/ 13521, // Rule ID 1014 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i64] } GPR:{ *:[i64] }:$addr, GPR:{ *:[i64] }:$incr)<<P:Predicate_atomic_load_sub_64>><<P:Predicate_atomic_load_sub_64_monotonic>>  =>  (AMOADD_D:{ *:[i64] } GPR:{ *:[i64] }:$addr, (SUB:{ *:[i64] } X0:{ *:[i64] }, GPR:{ *:[i64] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1014,
        GIR_Done,
      // Label 396: @13521
      GIM_Try, /*On fail goto*//*Label 397*/ 13592, // Rule ID 1017 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i64] } GPR:{ *:[i64] }:$addr, GPR:{ *:[i64] }:$incr)<<P:Predicate_atomic_load_sub_64>><<P:Predicate_atomic_load_sub_64_acquire>>  =>  (AMOADD_D_AQ:{ *:[i64] } GPR:{ *:[i64] }:$addr, (SUB:{ *:[i64] } X0:{ *:[i64] }, GPR:{ *:[i64] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1017,
        GIR_Done,
      // Label 397: @13592
      GIM_Try, /*On fail goto*//*Label 398*/ 13663, // Rule ID 1020 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i64] } GPR:{ *:[i64] }:$addr, GPR:{ *:[i64] }:$incr)<<P:Predicate_atomic_load_sub_64>><<P:Predicate_atomic_load_sub_64_release>>  =>  (AMOADD_D_RL:{ *:[i64] } GPR:{ *:[i64] }:$addr, (SUB:{ *:[i64] } X0:{ *:[i64] }, GPR:{ *:[i64] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_D_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1020,
        GIR_Done,
      // Label 398: @13663
      GIM_Try, /*On fail goto*//*Label 399*/ 13734, // Rule ID 1023 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i64] } GPR:{ *:[i64] }:$addr, GPR:{ *:[i64] }:$incr)<<P:Predicate_atomic_load_sub_64>><<P:Predicate_atomic_load_sub_64_acq_rel>>  =>  (AMOADD_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$addr, (SUB:{ *:[i64] } X0:{ *:[i64] }, GPR:{ *:[i64] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1023,
        GIR_Done,
      // Label 399: @13734
      GIM_Try, /*On fail goto*//*Label 400*/ 13805, // Rule ID 1026 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] addr
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_sub:{ *:[i64] } GPR:{ *:[i64] }:$addr, GPR:{ *:[i64] }:$incr)<<P:Predicate_atomic_load_sub_64>><<P:Predicate_atomic_load_sub_64_seq_cst>>  =>  (AMOADD_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$addr, (SUB:{ *:[i64] } X0:{ *:[i64] }, GPR:{ *:[i64] }:$incr))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SUB,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_AddRegister, /*InsnID*/1, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // incr
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AMOADD_D_AQ_RL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // addr
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_MergeMemOperands, /*InsnID*/0, /*MergeInsnID's*/0, GIU_MergeMemOperands_EndOfList,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1026,
        GIR_Done,
      // Label 400: @13805
      GIM_Reject,
    // Label 390: @13806
    GIM_Reject,
    // Label 368: @13807
    GIM_Reject,
    // Label 19: @13808
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 403*/ 14852,
    /*GILLT_s32*//*Label 401*/ 13816,
    /*GILLT_s64*//*Label 402*/ 14504,
    // Label 401: @13816
    GIM_Try, /*On fail goto*//*Label 404*/ 14503,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_Try, /*On fail goto*//*Label 405*/ 13856, // Rule ID 680 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_32>><<P:Predicate_atomic_load_and_32_monotonic>>  =>  (AMOAND_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 680,
        GIR_Done,
      // Label 405: @13856
      GIM_Try, /*On fail goto*//*Label 406*/ 13890, // Rule ID 682 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_32>><<P:Predicate_atomic_load_and_32_monotonic>>  =>  (AMOAND_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 682,
        GIR_Done,
      // Label 406: @13890
      GIM_Try, /*On fail goto*//*Label 407*/ 13924, // Rule ID 683 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_32>><<P:Predicate_atomic_load_and_32_acquire>>  =>  (AMOAND_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 683,
        GIR_Done,
      // Label 407: @13924
      GIM_Try, /*On fail goto*//*Label 408*/ 13958, // Rule ID 685 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_32>><<P:Predicate_atomic_load_and_32_acquire>>  =>  (AMOAND_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 685,
        GIR_Done,
      // Label 408: @13958
      GIM_Try, /*On fail goto*//*Label 409*/ 13992, // Rule ID 686 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_32>><<P:Predicate_atomic_load_and_32_release>>  =>  (AMOAND_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 686,
        GIR_Done,
      // Label 409: @13992
      GIM_Try, /*On fail goto*//*Label 410*/ 14026, // Rule ID 688 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_32>><<P:Predicate_atomic_load_and_32_release>>  =>  (AMOAND_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 688,
        GIR_Done,
      // Label 410: @14026
      GIM_Try, /*On fail goto*//*Label 411*/ 14060, // Rule ID 689 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_32>><<P:Predicate_atomic_load_and_32_acq_rel>>  =>  (AMOAND_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 689,
        GIR_Done,
      // Label 411: @14060
      GIM_Try, /*On fail goto*//*Label 412*/ 14094, // Rule ID 691 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_32>><<P:Predicate_atomic_load_and_32_acq_rel>>  =>  (AMOAND_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 691,
        GIR_Done,
      // Label 412: @14094
      GIM_Try, /*On fail goto*//*Label 413*/ 14128, // Rule ID 692 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_32>><<P:Predicate_atomic_load_and_32_seq_cst>>  =>  (AMOAND_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 692,
        GIR_Done,
      // Label 413: @14128
      GIM_Try, /*On fail goto*//*Label 414*/ 14162, // Rule ID 694 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_32>><<P:Predicate_atomic_load_and_32_seq_cst>>  =>  (AMOAND_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 694,
        GIR_Done,
      // Label 414: @14162
      GIM_Try, /*On fail goto*//*Label 415*/ 14196, // Rule ID 908 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_64>><<P:Predicate_atomic_load_and_64_monotonic>>  =>  (AMOAND_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 908,
        GIR_Done,
      // Label 415: @14196
      GIM_Try, /*On fail goto*//*Label 416*/ 14230, // Rule ID 910 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_64>><<P:Predicate_atomic_load_and_64_monotonic>>  =>  (AMOAND_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 910,
        GIR_Done,
      // Label 416: @14230
      GIM_Try, /*On fail goto*//*Label 417*/ 14264, // Rule ID 911 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_64>><<P:Predicate_atomic_load_and_64_acquire>>  =>  (AMOAND_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 911,
        GIR_Done,
      // Label 417: @14264
      GIM_Try, /*On fail goto*//*Label 418*/ 14298, // Rule ID 913 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_64>><<P:Predicate_atomic_load_and_64_acquire>>  =>  (AMOAND_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 913,
        GIR_Done,
      // Label 418: @14298
      GIM_Try, /*On fail goto*//*Label 419*/ 14332, // Rule ID 914 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_64>><<P:Predicate_atomic_load_and_64_release>>  =>  (AMOAND_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 914,
        GIR_Done,
      // Label 419: @14332
      GIM_Try, /*On fail goto*//*Label 420*/ 14366, // Rule ID 916 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_64>><<P:Predicate_atomic_load_and_64_release>>  =>  (AMOAND_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 916,
        GIR_Done,
      // Label 420: @14366
      GIM_Try, /*On fail goto*//*Label 421*/ 14400, // Rule ID 917 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_64>><<P:Predicate_atomic_load_and_64_acq_rel>>  =>  (AMOAND_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 917,
        GIR_Done,
      // Label 421: @14400
      GIM_Try, /*On fail goto*//*Label 422*/ 14434, // Rule ID 919 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_64>><<P:Predicate_atomic_load_and_64_acq_rel>>  =>  (AMOAND_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 919,
        GIR_Done,
      // Label 422: @14434
      GIM_Try, /*On fail goto*//*Label 423*/ 14468, // Rule ID 920 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_64>><<P:Predicate_atomic_load_and_64_seq_cst>>  =>  (AMOAND_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 920,
        GIR_Done,
      // Label 423: @14468
      GIM_Try, /*On fail goto*//*Label 424*/ 14502, // Rule ID 922 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_and_64>><<P:Predicate_atomic_load_and_64_seq_cst>>  =>  (AMOAND_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 922,
        GIR_Done,
      // Label 424: @14502
      GIM_Reject,
    // Label 404: @14503
    GIM_Reject,
    // Label 402: @14504
    GIM_Try, /*On fail goto*//*Label 425*/ 14851,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_Try, /*On fail goto*//*Label 426*/ 14544, // Rule ID 681 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_and_32>><<P:Predicate_atomic_load_and_32_monotonic>>  =>  (AMOAND_W:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 681,
        GIR_Done,
      // Label 426: @14544
      GIM_Try, /*On fail goto*//*Label 427*/ 14578, // Rule ID 684 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_and_32>><<P:Predicate_atomic_load_and_32_acquire>>  =>  (AMOAND_W_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 684,
        GIR_Done,
      // Label 427: @14578
      GIM_Try, /*On fail goto*//*Label 428*/ 14612, // Rule ID 687 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_and_32>><<P:Predicate_atomic_load_and_32_release>>  =>  (AMOAND_W_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 687,
        GIR_Done,
      // Label 428: @14612
      GIM_Try, /*On fail goto*//*Label 429*/ 14646, // Rule ID 690 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_and_32>><<P:Predicate_atomic_load_and_32_acq_rel>>  =>  (AMOAND_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 690,
        GIR_Done,
      // Label 429: @14646
      GIM_Try, /*On fail goto*//*Label 430*/ 14680, // Rule ID 693 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_and_32>><<P:Predicate_atomic_load_and_32_seq_cst>>  =>  (AMOAND_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 693,
        GIR_Done,
      // Label 430: @14680
      GIM_Try, /*On fail goto*//*Label 431*/ 14714, // Rule ID 909 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_and_64>><<P:Predicate_atomic_load_and_64_monotonic>>  =>  (AMOAND_D:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 909,
        GIR_Done,
      // Label 431: @14714
      GIM_Try, /*On fail goto*//*Label 432*/ 14748, // Rule ID 912 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_and_64>><<P:Predicate_atomic_load_and_64_acquire>>  =>  (AMOAND_D_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 912,
        GIR_Done,
      // Label 432: @14748
      GIM_Try, /*On fail goto*//*Label 433*/ 14782, // Rule ID 915 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_and_64>><<P:Predicate_atomic_load_and_64_release>>  =>  (AMOAND_D_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 915,
        GIR_Done,
      // Label 433: @14782
      GIM_Try, /*On fail goto*//*Label 434*/ 14816, // Rule ID 918 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_and_64>><<P:Predicate_atomic_load_and_64_acq_rel>>  =>  (AMOAND_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 918,
        GIR_Done,
      // Label 434: @14816
      GIM_Try, /*On fail goto*//*Label 435*/ 14850, // Rule ID 921 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_and:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_and_64>><<P:Predicate_atomic_load_and_64_seq_cst>>  =>  (AMOAND_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOAND_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 921,
        GIR_Done,
      // Label 435: @14850
      GIM_Reject,
    // Label 425: @14851
    GIM_Reject,
    // Label 403: @14852
    GIM_Reject,
    // Label 20: @14853
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 438*/ 15897,
    /*GILLT_s32*//*Label 436*/ 14861,
    /*GILLT_s64*//*Label 437*/ 15549,
    // Label 436: @14861
    GIM_Try, /*On fail goto*//*Label 439*/ 15548,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_Try, /*On fail goto*//*Label 440*/ 14901, // Rule ID 695 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_32>><<P:Predicate_atomic_load_or_32_monotonic>>  =>  (AMOOR_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 695,
        GIR_Done,
      // Label 440: @14901
      GIM_Try, /*On fail goto*//*Label 441*/ 14935, // Rule ID 697 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_32>><<P:Predicate_atomic_load_or_32_monotonic>>  =>  (AMOOR_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 697,
        GIR_Done,
      // Label 441: @14935
      GIM_Try, /*On fail goto*//*Label 442*/ 14969, // Rule ID 698 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_32>><<P:Predicate_atomic_load_or_32_acquire>>  =>  (AMOOR_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 698,
        GIR_Done,
      // Label 442: @14969
      GIM_Try, /*On fail goto*//*Label 443*/ 15003, // Rule ID 700 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_32>><<P:Predicate_atomic_load_or_32_acquire>>  =>  (AMOOR_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 700,
        GIR_Done,
      // Label 443: @15003
      GIM_Try, /*On fail goto*//*Label 444*/ 15037, // Rule ID 701 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_32>><<P:Predicate_atomic_load_or_32_release>>  =>  (AMOOR_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 701,
        GIR_Done,
      // Label 444: @15037
      GIM_Try, /*On fail goto*//*Label 445*/ 15071, // Rule ID 703 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_32>><<P:Predicate_atomic_load_or_32_release>>  =>  (AMOOR_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 703,
        GIR_Done,
      // Label 445: @15071
      GIM_Try, /*On fail goto*//*Label 446*/ 15105, // Rule ID 704 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_32>><<P:Predicate_atomic_load_or_32_acq_rel>>  =>  (AMOOR_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 704,
        GIR_Done,
      // Label 446: @15105
      GIM_Try, /*On fail goto*//*Label 447*/ 15139, // Rule ID 706 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_32>><<P:Predicate_atomic_load_or_32_acq_rel>>  =>  (AMOOR_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 706,
        GIR_Done,
      // Label 447: @15139
      GIM_Try, /*On fail goto*//*Label 448*/ 15173, // Rule ID 707 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_32>><<P:Predicate_atomic_load_or_32_seq_cst>>  =>  (AMOOR_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 707,
        GIR_Done,
      // Label 448: @15173
      GIM_Try, /*On fail goto*//*Label 449*/ 15207, // Rule ID 709 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_32>><<P:Predicate_atomic_load_or_32_seq_cst>>  =>  (AMOOR_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 709,
        GIR_Done,
      // Label 449: @15207
      GIM_Try, /*On fail goto*//*Label 450*/ 15241, // Rule ID 923 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_64>><<P:Predicate_atomic_load_or_64_monotonic>>  =>  (AMOOR_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 923,
        GIR_Done,
      // Label 450: @15241
      GIM_Try, /*On fail goto*//*Label 451*/ 15275, // Rule ID 925 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_64>><<P:Predicate_atomic_load_or_64_monotonic>>  =>  (AMOOR_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 925,
        GIR_Done,
      // Label 451: @15275
      GIM_Try, /*On fail goto*//*Label 452*/ 15309, // Rule ID 926 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_64>><<P:Predicate_atomic_load_or_64_acquire>>  =>  (AMOOR_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 926,
        GIR_Done,
      // Label 452: @15309
      GIM_Try, /*On fail goto*//*Label 453*/ 15343, // Rule ID 928 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_64>><<P:Predicate_atomic_load_or_64_acquire>>  =>  (AMOOR_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 928,
        GIR_Done,
      // Label 453: @15343
      GIM_Try, /*On fail goto*//*Label 454*/ 15377, // Rule ID 929 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_64>><<P:Predicate_atomic_load_or_64_release>>  =>  (AMOOR_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 929,
        GIR_Done,
      // Label 454: @15377
      GIM_Try, /*On fail goto*//*Label 455*/ 15411, // Rule ID 931 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_64>><<P:Predicate_atomic_load_or_64_release>>  =>  (AMOOR_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 931,
        GIR_Done,
      // Label 455: @15411
      GIM_Try, /*On fail goto*//*Label 456*/ 15445, // Rule ID 932 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_64>><<P:Predicate_atomic_load_or_64_acq_rel>>  =>  (AMOOR_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 932,
        GIR_Done,
      // Label 456: @15445
      GIM_Try, /*On fail goto*//*Label 457*/ 15479, // Rule ID 934 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_64>><<P:Predicate_atomic_load_or_64_acq_rel>>  =>  (AMOOR_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 934,
        GIR_Done,
      // Label 457: @15479
      GIM_Try, /*On fail goto*//*Label 458*/ 15513, // Rule ID 935 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_64>><<P:Predicate_atomic_load_or_64_seq_cst>>  =>  (AMOOR_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 935,
        GIR_Done,
      // Label 458: @15513
      GIM_Try, /*On fail goto*//*Label 459*/ 15547, // Rule ID 937 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_or_64>><<P:Predicate_atomic_load_or_64_seq_cst>>  =>  (AMOOR_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 937,
        GIR_Done,
      // Label 459: @15547
      GIM_Reject,
    // Label 439: @15548
    GIM_Reject,
    // Label 437: @15549
    GIM_Try, /*On fail goto*//*Label 460*/ 15896,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_Try, /*On fail goto*//*Label 461*/ 15589, // Rule ID 696 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_or_32>><<P:Predicate_atomic_load_or_32_monotonic>>  =>  (AMOOR_W:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 696,
        GIR_Done,
      // Label 461: @15589
      GIM_Try, /*On fail goto*//*Label 462*/ 15623, // Rule ID 699 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_or_32>><<P:Predicate_atomic_load_or_32_acquire>>  =>  (AMOOR_W_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 699,
        GIR_Done,
      // Label 462: @15623
      GIM_Try, /*On fail goto*//*Label 463*/ 15657, // Rule ID 702 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_or_32>><<P:Predicate_atomic_load_or_32_release>>  =>  (AMOOR_W_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 702,
        GIR_Done,
      // Label 463: @15657
      GIM_Try, /*On fail goto*//*Label 464*/ 15691, // Rule ID 705 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_or_32>><<P:Predicate_atomic_load_or_32_acq_rel>>  =>  (AMOOR_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 705,
        GIR_Done,
      // Label 464: @15691
      GIM_Try, /*On fail goto*//*Label 465*/ 15725, // Rule ID 708 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_or_32>><<P:Predicate_atomic_load_or_32_seq_cst>>  =>  (AMOOR_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 708,
        GIR_Done,
      // Label 465: @15725
      GIM_Try, /*On fail goto*//*Label 466*/ 15759, // Rule ID 924 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_or_64>><<P:Predicate_atomic_load_or_64_monotonic>>  =>  (AMOOR_D:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 924,
        GIR_Done,
      // Label 466: @15759
      GIM_Try, /*On fail goto*//*Label 467*/ 15793, // Rule ID 927 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_or_64>><<P:Predicate_atomic_load_or_64_acquire>>  =>  (AMOOR_D_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 927,
        GIR_Done,
      // Label 467: @15793
      GIM_Try, /*On fail goto*//*Label 468*/ 15827, // Rule ID 930 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_or_64>><<P:Predicate_atomic_load_or_64_release>>  =>  (AMOOR_D_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 930,
        GIR_Done,
      // Label 468: @15827
      GIM_Try, /*On fail goto*//*Label 469*/ 15861, // Rule ID 933 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_or_64>><<P:Predicate_atomic_load_or_64_acq_rel>>  =>  (AMOOR_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 933,
        GIR_Done,
      // Label 469: @15861
      GIM_Try, /*On fail goto*//*Label 470*/ 15895, // Rule ID 936 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_or:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_or_64>><<P:Predicate_atomic_load_or_64_seq_cst>>  =>  (AMOOR_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOOR_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 936,
        GIR_Done,
      // Label 470: @15895
      GIM_Reject,
    // Label 460: @15896
    GIM_Reject,
    // Label 438: @15897
    GIM_Reject,
    // Label 21: @15898
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 473*/ 16942,
    /*GILLT_s32*//*Label 471*/ 15906,
    /*GILLT_s64*//*Label 472*/ 16594,
    // Label 471: @15906
    GIM_Try, /*On fail goto*//*Label 474*/ 16593,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_Try, /*On fail goto*//*Label 475*/ 15946, // Rule ID 710 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_32>><<P:Predicate_atomic_load_xor_32_monotonic>>  =>  (AMOXOR_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 710,
        GIR_Done,
      // Label 475: @15946
      GIM_Try, /*On fail goto*//*Label 476*/ 15980, // Rule ID 712 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_32>><<P:Predicate_atomic_load_xor_32_monotonic>>  =>  (AMOXOR_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 712,
        GIR_Done,
      // Label 476: @15980
      GIM_Try, /*On fail goto*//*Label 477*/ 16014, // Rule ID 713 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_32>><<P:Predicate_atomic_load_xor_32_acquire>>  =>  (AMOXOR_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 713,
        GIR_Done,
      // Label 477: @16014
      GIM_Try, /*On fail goto*//*Label 478*/ 16048, // Rule ID 715 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_32>><<P:Predicate_atomic_load_xor_32_acquire>>  =>  (AMOXOR_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 715,
        GIR_Done,
      // Label 478: @16048
      GIM_Try, /*On fail goto*//*Label 479*/ 16082, // Rule ID 716 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_32>><<P:Predicate_atomic_load_xor_32_release>>  =>  (AMOXOR_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 716,
        GIR_Done,
      // Label 479: @16082
      GIM_Try, /*On fail goto*//*Label 480*/ 16116, // Rule ID 718 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_32>><<P:Predicate_atomic_load_xor_32_release>>  =>  (AMOXOR_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 718,
        GIR_Done,
      // Label 480: @16116
      GIM_Try, /*On fail goto*//*Label 481*/ 16150, // Rule ID 719 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_32>><<P:Predicate_atomic_load_xor_32_acq_rel>>  =>  (AMOXOR_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 719,
        GIR_Done,
      // Label 481: @16150
      GIM_Try, /*On fail goto*//*Label 482*/ 16184, // Rule ID 721 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_32>><<P:Predicate_atomic_load_xor_32_acq_rel>>  =>  (AMOXOR_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 721,
        GIR_Done,
      // Label 482: @16184
      GIM_Try, /*On fail goto*//*Label 483*/ 16218, // Rule ID 722 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_32>><<P:Predicate_atomic_load_xor_32_seq_cst>>  =>  (AMOXOR_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 722,
        GIR_Done,
      // Label 483: @16218
      GIM_Try, /*On fail goto*//*Label 484*/ 16252, // Rule ID 724 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_32>><<P:Predicate_atomic_load_xor_32_seq_cst>>  =>  (AMOXOR_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 724,
        GIR_Done,
      // Label 484: @16252
      GIM_Try, /*On fail goto*//*Label 485*/ 16286, // Rule ID 938 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_64>><<P:Predicate_atomic_load_xor_64_monotonic>>  =>  (AMOXOR_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 938,
        GIR_Done,
      // Label 485: @16286
      GIM_Try, /*On fail goto*//*Label 486*/ 16320, // Rule ID 940 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_64>><<P:Predicate_atomic_load_xor_64_monotonic>>  =>  (AMOXOR_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 940,
        GIR_Done,
      // Label 486: @16320
      GIM_Try, /*On fail goto*//*Label 487*/ 16354, // Rule ID 941 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_64>><<P:Predicate_atomic_load_xor_64_acquire>>  =>  (AMOXOR_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 941,
        GIR_Done,
      // Label 487: @16354
      GIM_Try, /*On fail goto*//*Label 488*/ 16388, // Rule ID 943 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_64>><<P:Predicate_atomic_load_xor_64_acquire>>  =>  (AMOXOR_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 943,
        GIR_Done,
      // Label 488: @16388
      GIM_Try, /*On fail goto*//*Label 489*/ 16422, // Rule ID 944 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_64>><<P:Predicate_atomic_load_xor_64_release>>  =>  (AMOXOR_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 944,
        GIR_Done,
      // Label 489: @16422
      GIM_Try, /*On fail goto*//*Label 490*/ 16456, // Rule ID 946 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_64>><<P:Predicate_atomic_load_xor_64_release>>  =>  (AMOXOR_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 946,
        GIR_Done,
      // Label 490: @16456
      GIM_Try, /*On fail goto*//*Label 491*/ 16490, // Rule ID 947 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_64>><<P:Predicate_atomic_load_xor_64_acq_rel>>  =>  (AMOXOR_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 947,
        GIR_Done,
      // Label 491: @16490
      GIM_Try, /*On fail goto*//*Label 492*/ 16524, // Rule ID 949 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_64>><<P:Predicate_atomic_load_xor_64_acq_rel>>  =>  (AMOXOR_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 949,
        GIR_Done,
      // Label 492: @16524
      GIM_Try, /*On fail goto*//*Label 493*/ 16558, // Rule ID 950 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_64>><<P:Predicate_atomic_load_xor_64_seq_cst>>  =>  (AMOXOR_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 950,
        GIR_Done,
      // Label 493: @16558
      GIM_Try, /*On fail goto*//*Label 494*/ 16592, // Rule ID 952 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_xor_64>><<P:Predicate_atomic_load_xor_64_seq_cst>>  =>  (AMOXOR_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 952,
        GIR_Done,
      // Label 494: @16592
      GIM_Reject,
    // Label 474: @16593
    GIM_Reject,
    // Label 472: @16594
    GIM_Try, /*On fail goto*//*Label 495*/ 16941,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_Try, /*On fail goto*//*Label 496*/ 16634, // Rule ID 711 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_xor_32>><<P:Predicate_atomic_load_xor_32_monotonic>>  =>  (AMOXOR_W:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 711,
        GIR_Done,
      // Label 496: @16634
      GIM_Try, /*On fail goto*//*Label 497*/ 16668, // Rule ID 714 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_xor_32>><<P:Predicate_atomic_load_xor_32_acquire>>  =>  (AMOXOR_W_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 714,
        GIR_Done,
      // Label 497: @16668
      GIM_Try, /*On fail goto*//*Label 498*/ 16702, // Rule ID 717 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_xor_32>><<P:Predicate_atomic_load_xor_32_release>>  =>  (AMOXOR_W_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 717,
        GIR_Done,
      // Label 498: @16702
      GIM_Try, /*On fail goto*//*Label 499*/ 16736, // Rule ID 720 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_xor_32>><<P:Predicate_atomic_load_xor_32_acq_rel>>  =>  (AMOXOR_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 720,
        GIR_Done,
      // Label 499: @16736
      GIM_Try, /*On fail goto*//*Label 500*/ 16770, // Rule ID 723 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_xor_32>><<P:Predicate_atomic_load_xor_32_seq_cst>>  =>  (AMOXOR_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 723,
        GIR_Done,
      // Label 500: @16770
      GIM_Try, /*On fail goto*//*Label 501*/ 16804, // Rule ID 939 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_xor_64>><<P:Predicate_atomic_load_xor_64_monotonic>>  =>  (AMOXOR_D:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 939,
        GIR_Done,
      // Label 501: @16804
      GIM_Try, /*On fail goto*//*Label 502*/ 16838, // Rule ID 942 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_xor_64>><<P:Predicate_atomic_load_xor_64_acquire>>  =>  (AMOXOR_D_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 942,
        GIR_Done,
      // Label 502: @16838
      GIM_Try, /*On fail goto*//*Label 503*/ 16872, // Rule ID 945 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_xor_64>><<P:Predicate_atomic_load_xor_64_release>>  =>  (AMOXOR_D_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 945,
        GIR_Done,
      // Label 503: @16872
      GIM_Try, /*On fail goto*//*Label 504*/ 16906, // Rule ID 948 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_xor_64>><<P:Predicate_atomic_load_xor_64_acq_rel>>  =>  (AMOXOR_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 948,
        GIR_Done,
      // Label 504: @16906
      GIM_Try, /*On fail goto*//*Label 505*/ 16940, // Rule ID 951 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_xor:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_xor_64>><<P:Predicate_atomic_load_xor_64_seq_cst>>  =>  (AMOXOR_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOXOR_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 951,
        GIR_Done,
      // Label 505: @16940
      GIM_Reject,
    // Label 495: @16941
    GIM_Reject,
    // Label 473: @16942
    GIM_Reject,
    // Label 22: @16943
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 508*/ 17987,
    /*GILLT_s32*//*Label 506*/ 16951,
    /*GILLT_s64*//*Label 507*/ 17639,
    // Label 506: @16951
    GIM_Try, /*On fail goto*//*Label 509*/ 17638,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_Try, /*On fail goto*//*Label 510*/ 16991, // Rule ID 725 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_32>><<P:Predicate_atomic_load_max_32_monotonic>>  =>  (AMOMAX_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 725,
        GIR_Done,
      // Label 510: @16991
      GIM_Try, /*On fail goto*//*Label 511*/ 17025, // Rule ID 727 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_32>><<P:Predicate_atomic_load_max_32_monotonic>>  =>  (AMOMAX_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 727,
        GIR_Done,
      // Label 511: @17025
      GIM_Try, /*On fail goto*//*Label 512*/ 17059, // Rule ID 728 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_32>><<P:Predicate_atomic_load_max_32_acquire>>  =>  (AMOMAX_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 728,
        GIR_Done,
      // Label 512: @17059
      GIM_Try, /*On fail goto*//*Label 513*/ 17093, // Rule ID 730 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_32>><<P:Predicate_atomic_load_max_32_acquire>>  =>  (AMOMAX_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 730,
        GIR_Done,
      // Label 513: @17093
      GIM_Try, /*On fail goto*//*Label 514*/ 17127, // Rule ID 731 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_32>><<P:Predicate_atomic_load_max_32_release>>  =>  (AMOMAX_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 731,
        GIR_Done,
      // Label 514: @17127
      GIM_Try, /*On fail goto*//*Label 515*/ 17161, // Rule ID 733 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_32>><<P:Predicate_atomic_load_max_32_release>>  =>  (AMOMAX_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 733,
        GIR_Done,
      // Label 515: @17161
      GIM_Try, /*On fail goto*//*Label 516*/ 17195, // Rule ID 734 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_32>><<P:Predicate_atomic_load_max_32_acq_rel>>  =>  (AMOMAX_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 734,
        GIR_Done,
      // Label 516: @17195
      GIM_Try, /*On fail goto*//*Label 517*/ 17229, // Rule ID 736 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_32>><<P:Predicate_atomic_load_max_32_acq_rel>>  =>  (AMOMAX_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 736,
        GIR_Done,
      // Label 517: @17229
      GIM_Try, /*On fail goto*//*Label 518*/ 17263, // Rule ID 737 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_32>><<P:Predicate_atomic_load_max_32_seq_cst>>  =>  (AMOMAX_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 737,
        GIR_Done,
      // Label 518: @17263
      GIM_Try, /*On fail goto*//*Label 519*/ 17297, // Rule ID 739 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_32>><<P:Predicate_atomic_load_max_32_seq_cst>>  =>  (AMOMAX_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 739,
        GIR_Done,
      // Label 519: @17297
      GIM_Try, /*On fail goto*//*Label 520*/ 17331, // Rule ID 953 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_64>><<P:Predicate_atomic_load_max_64_monotonic>>  =>  (AMOMAX_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 953,
        GIR_Done,
      // Label 520: @17331
      GIM_Try, /*On fail goto*//*Label 521*/ 17365, // Rule ID 955 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_64>><<P:Predicate_atomic_load_max_64_monotonic>>  =>  (AMOMAX_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 955,
        GIR_Done,
      // Label 521: @17365
      GIM_Try, /*On fail goto*//*Label 522*/ 17399, // Rule ID 956 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_64>><<P:Predicate_atomic_load_max_64_acquire>>  =>  (AMOMAX_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 956,
        GIR_Done,
      // Label 522: @17399
      GIM_Try, /*On fail goto*//*Label 523*/ 17433, // Rule ID 958 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_64>><<P:Predicate_atomic_load_max_64_acquire>>  =>  (AMOMAX_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 958,
        GIR_Done,
      // Label 523: @17433
      GIM_Try, /*On fail goto*//*Label 524*/ 17467, // Rule ID 959 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_64>><<P:Predicate_atomic_load_max_64_release>>  =>  (AMOMAX_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 959,
        GIR_Done,
      // Label 524: @17467
      GIM_Try, /*On fail goto*//*Label 525*/ 17501, // Rule ID 961 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_64>><<P:Predicate_atomic_load_max_64_release>>  =>  (AMOMAX_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 961,
        GIR_Done,
      // Label 525: @17501
      GIM_Try, /*On fail goto*//*Label 526*/ 17535, // Rule ID 962 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_64>><<P:Predicate_atomic_load_max_64_acq_rel>>  =>  (AMOMAX_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 962,
        GIR_Done,
      // Label 526: @17535
      GIM_Try, /*On fail goto*//*Label 527*/ 17569, // Rule ID 964 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_64>><<P:Predicate_atomic_load_max_64_acq_rel>>  =>  (AMOMAX_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 964,
        GIR_Done,
      // Label 527: @17569
      GIM_Try, /*On fail goto*//*Label 528*/ 17603, // Rule ID 965 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_64>><<P:Predicate_atomic_load_max_64_seq_cst>>  =>  (AMOMAX_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 965,
        GIR_Done,
      // Label 528: @17603
      GIM_Try, /*On fail goto*//*Label 529*/ 17637, // Rule ID 967 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_max_64>><<P:Predicate_atomic_load_max_64_seq_cst>>  =>  (AMOMAX_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 967,
        GIR_Done,
      // Label 529: @17637
      GIM_Reject,
    // Label 509: @17638
    GIM_Reject,
    // Label 507: @17639
    GIM_Try, /*On fail goto*//*Label 530*/ 17986,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_Try, /*On fail goto*//*Label 531*/ 17679, // Rule ID 726 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_max_32>><<P:Predicate_atomic_load_max_32_monotonic>>  =>  (AMOMAX_W:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 726,
        GIR_Done,
      // Label 531: @17679
      GIM_Try, /*On fail goto*//*Label 532*/ 17713, // Rule ID 729 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_max_32>><<P:Predicate_atomic_load_max_32_acquire>>  =>  (AMOMAX_W_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 729,
        GIR_Done,
      // Label 532: @17713
      GIM_Try, /*On fail goto*//*Label 533*/ 17747, // Rule ID 732 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_max_32>><<P:Predicate_atomic_load_max_32_release>>  =>  (AMOMAX_W_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 732,
        GIR_Done,
      // Label 533: @17747
      GIM_Try, /*On fail goto*//*Label 534*/ 17781, // Rule ID 735 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_max_32>><<P:Predicate_atomic_load_max_32_acq_rel>>  =>  (AMOMAX_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 735,
        GIR_Done,
      // Label 534: @17781
      GIM_Try, /*On fail goto*//*Label 535*/ 17815, // Rule ID 738 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_max_32>><<P:Predicate_atomic_load_max_32_seq_cst>>  =>  (AMOMAX_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 738,
        GIR_Done,
      // Label 535: @17815
      GIM_Try, /*On fail goto*//*Label 536*/ 17849, // Rule ID 954 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_max_64>><<P:Predicate_atomic_load_max_64_monotonic>>  =>  (AMOMAX_D:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 954,
        GIR_Done,
      // Label 536: @17849
      GIM_Try, /*On fail goto*//*Label 537*/ 17883, // Rule ID 957 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_max_64>><<P:Predicate_atomic_load_max_64_acquire>>  =>  (AMOMAX_D_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 957,
        GIR_Done,
      // Label 537: @17883
      GIM_Try, /*On fail goto*//*Label 538*/ 17917, // Rule ID 960 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_max_64>><<P:Predicate_atomic_load_max_64_release>>  =>  (AMOMAX_D_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 960,
        GIR_Done,
      // Label 538: @17917
      GIM_Try, /*On fail goto*//*Label 539*/ 17951, // Rule ID 963 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_max_64>><<P:Predicate_atomic_load_max_64_acq_rel>>  =>  (AMOMAX_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 963,
        GIR_Done,
      // Label 539: @17951
      GIM_Try, /*On fail goto*//*Label 540*/ 17985, // Rule ID 966 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_max:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_max_64>><<P:Predicate_atomic_load_max_64_seq_cst>>  =>  (AMOMAX_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAX_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 966,
        GIR_Done,
      // Label 540: @17985
      GIM_Reject,
    // Label 530: @17986
    GIM_Reject,
    // Label 508: @17987
    GIM_Reject,
    // Label 23: @17988
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 543*/ 19032,
    /*GILLT_s32*//*Label 541*/ 17996,
    /*GILLT_s64*//*Label 542*/ 18684,
    // Label 541: @17996
    GIM_Try, /*On fail goto*//*Label 544*/ 18683,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_Try, /*On fail goto*//*Label 545*/ 18036, // Rule ID 740 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_32>><<P:Predicate_atomic_load_min_32_monotonic>>  =>  (AMOMIN_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 740,
        GIR_Done,
      // Label 545: @18036
      GIM_Try, /*On fail goto*//*Label 546*/ 18070, // Rule ID 742 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_32>><<P:Predicate_atomic_load_min_32_monotonic>>  =>  (AMOMIN_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 742,
        GIR_Done,
      // Label 546: @18070
      GIM_Try, /*On fail goto*//*Label 547*/ 18104, // Rule ID 743 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_32>><<P:Predicate_atomic_load_min_32_acquire>>  =>  (AMOMIN_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 743,
        GIR_Done,
      // Label 547: @18104
      GIM_Try, /*On fail goto*//*Label 548*/ 18138, // Rule ID 745 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_32>><<P:Predicate_atomic_load_min_32_acquire>>  =>  (AMOMIN_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 745,
        GIR_Done,
      // Label 548: @18138
      GIM_Try, /*On fail goto*//*Label 549*/ 18172, // Rule ID 746 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_32>><<P:Predicate_atomic_load_min_32_release>>  =>  (AMOMIN_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 746,
        GIR_Done,
      // Label 549: @18172
      GIM_Try, /*On fail goto*//*Label 550*/ 18206, // Rule ID 748 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_32>><<P:Predicate_atomic_load_min_32_release>>  =>  (AMOMIN_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 748,
        GIR_Done,
      // Label 550: @18206
      GIM_Try, /*On fail goto*//*Label 551*/ 18240, // Rule ID 749 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_32>><<P:Predicate_atomic_load_min_32_acq_rel>>  =>  (AMOMIN_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 749,
        GIR_Done,
      // Label 551: @18240
      GIM_Try, /*On fail goto*//*Label 552*/ 18274, // Rule ID 751 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_32>><<P:Predicate_atomic_load_min_32_acq_rel>>  =>  (AMOMIN_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 751,
        GIR_Done,
      // Label 552: @18274
      GIM_Try, /*On fail goto*//*Label 553*/ 18308, // Rule ID 752 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_32>><<P:Predicate_atomic_load_min_32_seq_cst>>  =>  (AMOMIN_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 752,
        GIR_Done,
      // Label 553: @18308
      GIM_Try, /*On fail goto*//*Label 554*/ 18342, // Rule ID 754 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_32>><<P:Predicate_atomic_load_min_32_seq_cst>>  =>  (AMOMIN_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 754,
        GIR_Done,
      // Label 554: @18342
      GIM_Try, /*On fail goto*//*Label 555*/ 18376, // Rule ID 968 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_64>><<P:Predicate_atomic_load_min_64_monotonic>>  =>  (AMOMIN_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 968,
        GIR_Done,
      // Label 555: @18376
      GIM_Try, /*On fail goto*//*Label 556*/ 18410, // Rule ID 970 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_64>><<P:Predicate_atomic_load_min_64_monotonic>>  =>  (AMOMIN_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 970,
        GIR_Done,
      // Label 556: @18410
      GIM_Try, /*On fail goto*//*Label 557*/ 18444, // Rule ID 971 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_64>><<P:Predicate_atomic_load_min_64_acquire>>  =>  (AMOMIN_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 971,
        GIR_Done,
      // Label 557: @18444
      GIM_Try, /*On fail goto*//*Label 558*/ 18478, // Rule ID 973 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_64>><<P:Predicate_atomic_load_min_64_acquire>>  =>  (AMOMIN_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 973,
        GIR_Done,
      // Label 558: @18478
      GIM_Try, /*On fail goto*//*Label 559*/ 18512, // Rule ID 974 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_64>><<P:Predicate_atomic_load_min_64_release>>  =>  (AMOMIN_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 974,
        GIR_Done,
      // Label 559: @18512
      GIM_Try, /*On fail goto*//*Label 560*/ 18546, // Rule ID 976 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_64>><<P:Predicate_atomic_load_min_64_release>>  =>  (AMOMIN_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 976,
        GIR_Done,
      // Label 560: @18546
      GIM_Try, /*On fail goto*//*Label 561*/ 18580, // Rule ID 977 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_64>><<P:Predicate_atomic_load_min_64_acq_rel>>  =>  (AMOMIN_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 977,
        GIR_Done,
      // Label 561: @18580
      GIM_Try, /*On fail goto*//*Label 562*/ 18614, // Rule ID 979 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_64>><<P:Predicate_atomic_load_min_64_acq_rel>>  =>  (AMOMIN_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 979,
        GIR_Done,
      // Label 562: @18614
      GIM_Try, /*On fail goto*//*Label 563*/ 18648, // Rule ID 980 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_64>><<P:Predicate_atomic_load_min_64_seq_cst>>  =>  (AMOMIN_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 980,
        GIR_Done,
      // Label 563: @18648
      GIM_Try, /*On fail goto*//*Label 564*/ 18682, // Rule ID 982 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_min_64>><<P:Predicate_atomic_load_min_64_seq_cst>>  =>  (AMOMIN_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 982,
        GIR_Done,
      // Label 564: @18682
      GIM_Reject,
    // Label 544: @18683
    GIM_Reject,
    // Label 542: @18684
    GIM_Try, /*On fail goto*//*Label 565*/ 19031,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_Try, /*On fail goto*//*Label 566*/ 18724, // Rule ID 741 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_min_32>><<P:Predicate_atomic_load_min_32_monotonic>>  =>  (AMOMIN_W:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 741,
        GIR_Done,
      // Label 566: @18724
      GIM_Try, /*On fail goto*//*Label 567*/ 18758, // Rule ID 744 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_min_32>><<P:Predicate_atomic_load_min_32_acquire>>  =>  (AMOMIN_W_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 744,
        GIR_Done,
      // Label 567: @18758
      GIM_Try, /*On fail goto*//*Label 568*/ 18792, // Rule ID 747 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_min_32>><<P:Predicate_atomic_load_min_32_release>>  =>  (AMOMIN_W_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 747,
        GIR_Done,
      // Label 568: @18792
      GIM_Try, /*On fail goto*//*Label 569*/ 18826, // Rule ID 750 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_min_32>><<P:Predicate_atomic_load_min_32_acq_rel>>  =>  (AMOMIN_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 750,
        GIR_Done,
      // Label 569: @18826
      GIM_Try, /*On fail goto*//*Label 570*/ 18860, // Rule ID 753 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_min_32>><<P:Predicate_atomic_load_min_32_seq_cst>>  =>  (AMOMIN_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 753,
        GIR_Done,
      // Label 570: @18860
      GIM_Try, /*On fail goto*//*Label 571*/ 18894, // Rule ID 969 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_min_64>><<P:Predicate_atomic_load_min_64_monotonic>>  =>  (AMOMIN_D:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 969,
        GIR_Done,
      // Label 571: @18894
      GIM_Try, /*On fail goto*//*Label 572*/ 18928, // Rule ID 972 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_min_64>><<P:Predicate_atomic_load_min_64_acquire>>  =>  (AMOMIN_D_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 972,
        GIR_Done,
      // Label 572: @18928
      GIM_Try, /*On fail goto*//*Label 573*/ 18962, // Rule ID 975 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_min_64>><<P:Predicate_atomic_load_min_64_release>>  =>  (AMOMIN_D_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 975,
        GIR_Done,
      // Label 573: @18962
      GIM_Try, /*On fail goto*//*Label 574*/ 18996, // Rule ID 978 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_min_64>><<P:Predicate_atomic_load_min_64_acq_rel>>  =>  (AMOMIN_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 978,
        GIR_Done,
      // Label 574: @18996
      GIM_Try, /*On fail goto*//*Label 575*/ 19030, // Rule ID 981 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_min:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_min_64>><<P:Predicate_atomic_load_min_64_seq_cst>>  =>  (AMOMIN_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMIN_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 981,
        GIR_Done,
      // Label 575: @19030
      GIM_Reject,
    // Label 565: @19031
    GIM_Reject,
    // Label 543: @19032
    GIM_Reject,
    // Label 24: @19033
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 578*/ 20077,
    /*GILLT_s32*//*Label 576*/ 19041,
    /*GILLT_s64*//*Label 577*/ 19729,
    // Label 576: @19041
    GIM_Try, /*On fail goto*//*Label 579*/ 19728,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_Try, /*On fail goto*//*Label 580*/ 19081, // Rule ID 755 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_32>><<P:Predicate_atomic_load_umax_32_monotonic>>  =>  (AMOMAXU_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 755,
        GIR_Done,
      // Label 580: @19081
      GIM_Try, /*On fail goto*//*Label 581*/ 19115, // Rule ID 757 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_32>><<P:Predicate_atomic_load_umax_32_monotonic>>  =>  (AMOMAXU_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 757,
        GIR_Done,
      // Label 581: @19115
      GIM_Try, /*On fail goto*//*Label 582*/ 19149, // Rule ID 758 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_32>><<P:Predicate_atomic_load_umax_32_acquire>>  =>  (AMOMAXU_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 758,
        GIR_Done,
      // Label 582: @19149
      GIM_Try, /*On fail goto*//*Label 583*/ 19183, // Rule ID 760 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_32>><<P:Predicate_atomic_load_umax_32_acquire>>  =>  (AMOMAXU_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 760,
        GIR_Done,
      // Label 583: @19183
      GIM_Try, /*On fail goto*//*Label 584*/ 19217, // Rule ID 761 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_32>><<P:Predicate_atomic_load_umax_32_release>>  =>  (AMOMAXU_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 761,
        GIR_Done,
      // Label 584: @19217
      GIM_Try, /*On fail goto*//*Label 585*/ 19251, // Rule ID 763 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_32>><<P:Predicate_atomic_load_umax_32_release>>  =>  (AMOMAXU_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 763,
        GIR_Done,
      // Label 585: @19251
      GIM_Try, /*On fail goto*//*Label 586*/ 19285, // Rule ID 764 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_32>><<P:Predicate_atomic_load_umax_32_acq_rel>>  =>  (AMOMAXU_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 764,
        GIR_Done,
      // Label 586: @19285
      GIM_Try, /*On fail goto*//*Label 587*/ 19319, // Rule ID 766 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_32>><<P:Predicate_atomic_load_umax_32_acq_rel>>  =>  (AMOMAXU_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 766,
        GIR_Done,
      // Label 587: @19319
      GIM_Try, /*On fail goto*//*Label 588*/ 19353, // Rule ID 767 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_32>><<P:Predicate_atomic_load_umax_32_seq_cst>>  =>  (AMOMAXU_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 767,
        GIR_Done,
      // Label 588: @19353
      GIM_Try, /*On fail goto*//*Label 589*/ 19387, // Rule ID 769 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_32>><<P:Predicate_atomic_load_umax_32_seq_cst>>  =>  (AMOMAXU_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 769,
        GIR_Done,
      // Label 589: @19387
      GIM_Try, /*On fail goto*//*Label 590*/ 19421, // Rule ID 983 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_64>><<P:Predicate_atomic_load_umax_64_monotonic>>  =>  (AMOMAXU_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 983,
        GIR_Done,
      // Label 590: @19421
      GIM_Try, /*On fail goto*//*Label 591*/ 19455, // Rule ID 985 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_64>><<P:Predicate_atomic_load_umax_64_monotonic>>  =>  (AMOMAXU_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 985,
        GIR_Done,
      // Label 591: @19455
      GIM_Try, /*On fail goto*//*Label 592*/ 19489, // Rule ID 986 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_64>><<P:Predicate_atomic_load_umax_64_acquire>>  =>  (AMOMAXU_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 986,
        GIR_Done,
      // Label 592: @19489
      GIM_Try, /*On fail goto*//*Label 593*/ 19523, // Rule ID 988 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_64>><<P:Predicate_atomic_load_umax_64_acquire>>  =>  (AMOMAXU_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 988,
        GIR_Done,
      // Label 593: @19523
      GIM_Try, /*On fail goto*//*Label 594*/ 19557, // Rule ID 989 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_64>><<P:Predicate_atomic_load_umax_64_release>>  =>  (AMOMAXU_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 989,
        GIR_Done,
      // Label 594: @19557
      GIM_Try, /*On fail goto*//*Label 595*/ 19591, // Rule ID 991 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_64>><<P:Predicate_atomic_load_umax_64_release>>  =>  (AMOMAXU_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 991,
        GIR_Done,
      // Label 595: @19591
      GIM_Try, /*On fail goto*//*Label 596*/ 19625, // Rule ID 992 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_64>><<P:Predicate_atomic_load_umax_64_acq_rel>>  =>  (AMOMAXU_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 992,
        GIR_Done,
      // Label 596: @19625
      GIM_Try, /*On fail goto*//*Label 597*/ 19659, // Rule ID 994 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_64>><<P:Predicate_atomic_load_umax_64_acq_rel>>  =>  (AMOMAXU_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 994,
        GIR_Done,
      // Label 597: @19659
      GIM_Try, /*On fail goto*//*Label 598*/ 19693, // Rule ID 995 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_64>><<P:Predicate_atomic_load_umax_64_seq_cst>>  =>  (AMOMAXU_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 995,
        GIR_Done,
      // Label 598: @19693
      GIM_Try, /*On fail goto*//*Label 599*/ 19727, // Rule ID 997 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umax_64>><<P:Predicate_atomic_load_umax_64_seq_cst>>  =>  (AMOMAXU_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 997,
        GIR_Done,
      // Label 599: @19727
      GIM_Reject,
    // Label 579: @19728
    GIM_Reject,
    // Label 577: @19729
    GIM_Try, /*On fail goto*//*Label 600*/ 20076,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_Try, /*On fail goto*//*Label 601*/ 19769, // Rule ID 756 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umax_32>><<P:Predicate_atomic_load_umax_32_monotonic>>  =>  (AMOMAXU_W:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 756,
        GIR_Done,
      // Label 601: @19769
      GIM_Try, /*On fail goto*//*Label 602*/ 19803, // Rule ID 759 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umax_32>><<P:Predicate_atomic_load_umax_32_acquire>>  =>  (AMOMAXU_W_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 759,
        GIR_Done,
      // Label 602: @19803
      GIM_Try, /*On fail goto*//*Label 603*/ 19837, // Rule ID 762 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umax_32>><<P:Predicate_atomic_load_umax_32_release>>  =>  (AMOMAXU_W_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 762,
        GIR_Done,
      // Label 603: @19837
      GIM_Try, /*On fail goto*//*Label 604*/ 19871, // Rule ID 765 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umax_32>><<P:Predicate_atomic_load_umax_32_acq_rel>>  =>  (AMOMAXU_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 765,
        GIR_Done,
      // Label 604: @19871
      GIM_Try, /*On fail goto*//*Label 605*/ 19905, // Rule ID 768 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umax_32>><<P:Predicate_atomic_load_umax_32_seq_cst>>  =>  (AMOMAXU_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 768,
        GIR_Done,
      // Label 605: @19905
      GIM_Try, /*On fail goto*//*Label 606*/ 19939, // Rule ID 984 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umax_64>><<P:Predicate_atomic_load_umax_64_monotonic>>  =>  (AMOMAXU_D:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 984,
        GIR_Done,
      // Label 606: @19939
      GIM_Try, /*On fail goto*//*Label 607*/ 19973, // Rule ID 987 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umax_64>><<P:Predicate_atomic_load_umax_64_acquire>>  =>  (AMOMAXU_D_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 987,
        GIR_Done,
      // Label 607: @19973
      GIM_Try, /*On fail goto*//*Label 608*/ 20007, // Rule ID 990 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umax_64>><<P:Predicate_atomic_load_umax_64_release>>  =>  (AMOMAXU_D_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 990,
        GIR_Done,
      // Label 608: @20007
      GIM_Try, /*On fail goto*//*Label 609*/ 20041, // Rule ID 993 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umax_64>><<P:Predicate_atomic_load_umax_64_acq_rel>>  =>  (AMOMAXU_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 993,
        GIR_Done,
      // Label 609: @20041
      GIM_Try, /*On fail goto*//*Label 610*/ 20075, // Rule ID 996 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umax:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umax_64>><<P:Predicate_atomic_load_umax_64_seq_cst>>  =>  (AMOMAXU_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMAXU_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 996,
        GIR_Done,
      // Label 610: @20075
      GIM_Reject,
    // Label 600: @20076
    GIM_Reject,
    // Label 578: @20077
    GIM_Reject,
    // Label 25: @20078
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 613*/ 21122,
    /*GILLT_s32*//*Label 611*/ 20086,
    /*GILLT_s64*//*Label 612*/ 20774,
    // Label 611: @20086
    GIM_Try, /*On fail goto*//*Label 614*/ 20773,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_Try, /*On fail goto*//*Label 615*/ 20126, // Rule ID 770 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_32>><<P:Predicate_atomic_load_umin_32_monotonic>>  =>  (AMOMINU_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 770,
        GIR_Done,
      // Label 615: @20126
      GIM_Try, /*On fail goto*//*Label 616*/ 20160, // Rule ID 772 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_32>><<P:Predicate_atomic_load_umin_32_monotonic>>  =>  (AMOMINU_W:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 772,
        GIR_Done,
      // Label 616: @20160
      GIM_Try, /*On fail goto*//*Label 617*/ 20194, // Rule ID 773 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_32>><<P:Predicate_atomic_load_umin_32_acquire>>  =>  (AMOMINU_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 773,
        GIR_Done,
      // Label 617: @20194
      GIM_Try, /*On fail goto*//*Label 618*/ 20228, // Rule ID 775 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_32>><<P:Predicate_atomic_load_umin_32_acquire>>  =>  (AMOMINU_W_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 775,
        GIR_Done,
      // Label 618: @20228
      GIM_Try, /*On fail goto*//*Label 619*/ 20262, // Rule ID 776 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_32>><<P:Predicate_atomic_load_umin_32_release>>  =>  (AMOMINU_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 776,
        GIR_Done,
      // Label 619: @20262
      GIM_Try, /*On fail goto*//*Label 620*/ 20296, // Rule ID 778 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_32>><<P:Predicate_atomic_load_umin_32_release>>  =>  (AMOMINU_W_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 778,
        GIR_Done,
      // Label 620: @20296
      GIM_Try, /*On fail goto*//*Label 621*/ 20330, // Rule ID 779 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_32>><<P:Predicate_atomic_load_umin_32_acq_rel>>  =>  (AMOMINU_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 779,
        GIR_Done,
      // Label 621: @20330
      GIM_Try, /*On fail goto*//*Label 622*/ 20364, // Rule ID 781 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_32>><<P:Predicate_atomic_load_umin_32_acq_rel>>  =>  (AMOMINU_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 781,
        GIR_Done,
      // Label 622: @20364
      GIM_Try, /*On fail goto*//*Label 623*/ 20398, // Rule ID 782 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_32>><<P:Predicate_atomic_load_umin_32_seq_cst>>  =>  (AMOMINU_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 782,
        GIR_Done,
      // Label 623: @20398
      GIM_Try, /*On fail goto*//*Label 624*/ 20432, // Rule ID 784 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_32>><<P:Predicate_atomic_load_umin_32_seq_cst>>  =>  (AMOMINU_W_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 784,
        GIR_Done,
      // Label 624: @20432
      GIM_Try, /*On fail goto*//*Label 625*/ 20466, // Rule ID 998 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_64>><<P:Predicate_atomic_load_umin_64_monotonic>>  =>  (AMOMINU_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 998,
        GIR_Done,
      // Label 625: @20466
      GIM_Try, /*On fail goto*//*Label 626*/ 20500, // Rule ID 1000 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_64>><<P:Predicate_atomic_load_umin_64_monotonic>>  =>  (AMOMINU_D:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1000,
        GIR_Done,
      // Label 626: @20500
      GIM_Try, /*On fail goto*//*Label 627*/ 20534, // Rule ID 1001 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_64>><<P:Predicate_atomic_load_umin_64_acquire>>  =>  (AMOMINU_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1001,
        GIR_Done,
      // Label 627: @20534
      GIM_Try, /*On fail goto*//*Label 628*/ 20568, // Rule ID 1003 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_64>><<P:Predicate_atomic_load_umin_64_acquire>>  =>  (AMOMINU_D_AQ:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1003,
        GIR_Done,
      // Label 628: @20568
      GIM_Try, /*On fail goto*//*Label 629*/ 20602, // Rule ID 1004 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_64>><<P:Predicate_atomic_load_umin_64_release>>  =>  (AMOMINU_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1004,
        GIR_Done,
      // Label 629: @20602
      GIM_Try, /*On fail goto*//*Label 630*/ 20636, // Rule ID 1006 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_64>><<P:Predicate_atomic_load_umin_64_release>>  =>  (AMOMINU_D_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1006,
        GIR_Done,
      // Label 630: @20636
      GIM_Try, /*On fail goto*//*Label 631*/ 20670, // Rule ID 1007 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_64>><<P:Predicate_atomic_load_umin_64_acq_rel>>  =>  (AMOMINU_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1007,
        GIR_Done,
      // Label 631: @20670
      GIM_Try, /*On fail goto*//*Label 632*/ 20704, // Rule ID 1009 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_64>><<P:Predicate_atomic_load_umin_64_acq_rel>>  =>  (AMOMINU_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1009,
        GIR_Done,
      // Label 632: @20704
      GIM_Try, /*On fail goto*//*Label 633*/ 20738, // Rule ID 1010 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_64>><<P:Predicate_atomic_load_umin_64_seq_cst>>  =>  (AMOMINU_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1010,
        GIR_Done,
      // Label 633: @20738
      GIM_Try, /*On fail goto*//*Label 634*/ 20772, // Rule ID 1012 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)<<P:Predicate_atomic_load_umin_64>><<P:Predicate_atomic_load_umin_64_seq_cst>>  =>  (AMOMINU_D_AQ_RL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1012,
        GIR_Done,
      // Label 634: @20772
      GIM_Reject,
    // Label 614: @20773
    GIM_Reject,
    // Label 612: @20774
    GIM_Try, /*On fail goto*//*Label 635*/ 21121,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_Try, /*On fail goto*//*Label 636*/ 20814, // Rule ID 771 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umin_32>><<P:Predicate_atomic_load_umin_32_monotonic>>  =>  (AMOMINU_W:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 771,
        GIR_Done,
      // Label 636: @20814
      GIM_Try, /*On fail goto*//*Label 637*/ 20848, // Rule ID 774 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umin_32>><<P:Predicate_atomic_load_umin_32_acquire>>  =>  (AMOMINU_W_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_W_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 774,
        GIR_Done,
      // Label 637: @20848
      GIM_Try, /*On fail goto*//*Label 638*/ 20882, // Rule ID 777 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umin_32>><<P:Predicate_atomic_load_umin_32_release>>  =>  (AMOMINU_W_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_W_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 777,
        GIR_Done,
      // Label 638: @20882
      GIM_Try, /*On fail goto*//*Label 639*/ 20916, // Rule ID 780 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umin_32>><<P:Predicate_atomic_load_umin_32_acq_rel>>  =>  (AMOMINU_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 780,
        GIR_Done,
      // Label 639: @20916
      GIM_Try, /*On fail goto*//*Label 640*/ 20950, // Rule ID 783 //
        GIM_CheckFeatures, GIFBS_HasStdExtA,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/4,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umin_32>><<P:Predicate_atomic_load_umin_32_seq_cst>>  =>  (AMOMINU_W_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_W_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 783,
        GIR_Done,
      // Label 640: @20950
      GIM_Try, /*On fail goto*//*Label 641*/ 20984, // Rule ID 999 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Monotonic,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umin_64>><<P:Predicate_atomic_load_umin_64_monotonic>>  =>  (AMOMINU_D:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_D,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 999,
        GIR_Done,
      // Label 641: @20984
      GIM_Try, /*On fail goto*//*Label 642*/ 21018, // Rule ID 1002 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Acquire,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umin_64>><<P:Predicate_atomic_load_umin_64_acquire>>  =>  (AMOMINU_D_AQ:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_D_AQ,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1002,
        GIR_Done,
      // Label 642: @21018
      GIM_Try, /*On fail goto*//*Label 643*/ 21052, // Rule ID 1005 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::Release,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umin_64>><<P:Predicate_atomic_load_umin_64_release>>  =>  (AMOMINU_D_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_D_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1005,
        GIR_Done,
      // Label 643: @21052
      GIM_Try, /*On fail goto*//*Label 644*/ 21086, // Rule ID 1008 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::AcquireRelease,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umin_64>><<P:Predicate_atomic_load_umin_64_acq_rel>>  =>  (AMOMINU_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1008,
        GIR_Done,
      // Label 644: @21086
      GIM_Try, /*On fail goto*//*Label 645*/ 21120, // Rule ID 1011 //
        GIM_CheckFeatures, GIFBS_HasStdExtA_IsRV64,
        GIM_CheckMemorySizeEqualTo, /*MI*/0, /*MMO*/0, /*Size*/8,
        GIM_CheckAtomicOrdering, /*MI*/0, /*Order*/(int64_t)AtomicOrdering::SequentiallyConsistent,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
        // MIs[0] rs1
        GIM_CheckPointerToAny, /*MI*/0, /*Op*/1, /*SizeInBits*/64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (atomic_load_umin:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)<<P:Predicate_atomic_load_umin_64>><<P:Predicate_atomic_load_umin_64_seq_cst>>  =>  (AMOMINU_D_AQ_RL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::AMOMINU_D_AQ_RL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1011,
        GIR_Done,
      // Label 645: @21120
      GIM_Reject,
    // Label 635: @21121
    GIM_Reject,
    // Label 613: @21122
    GIM_Reject,
    // Label 26: @21123
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 648*/ 21391,
    /*GILLT_s32*//*Label 646*/ 21131,
    /*GILLT_s64*//*Label 647*/ 21304,
    // Label 646: @21131
    GIM_Try, /*On fail goto*//*Label 649*/ 21154, // Rule ID 348 //
      GIM_CheckConstantInt, /*MI*/0, /*Op*/0, 4,
      // MIs[0] Operand 1
      GIM_CheckIsImm, /*MI*/0, /*Op*/1,
      // (atomic_fence 4:{ *:[i32] }, (timm:{ *:[i32] }))  =>  (FENCE 2:{ *:[i32] }, 3:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FENCE,
      GIR_AddImm, /*InsnID*/0, /*Imm*/2,
      GIR_AddImm, /*InsnID*/0, /*Imm*/3,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 348,
      GIR_Done,
    // Label 649: @21154
    GIM_Try, /*On fail goto*//*Label 650*/ 21177, // Rule ID 350 //
      GIM_CheckConstantInt, /*MI*/0, /*Op*/0, 4,
      // MIs[0] Operand 1
      GIM_CheckIsImm, /*MI*/0, /*Op*/1,
      // (atomic_fence 4:{ *:[i32] }, (timm:{ *:[i32] }))  =>  (FENCE 2:{ *:[i32] }, 3:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FENCE,
      GIR_AddImm, /*InsnID*/0, /*Imm*/2,
      GIR_AddImm, /*InsnID*/0, /*Imm*/3,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 350,
      GIR_Done,
    // Label 650: @21177
    GIM_Try, /*On fail goto*//*Label 651*/ 21200, // Rule ID 351 //
      GIM_CheckConstantInt, /*MI*/0, /*Op*/0, 5,
      // MIs[0] Operand 1
      GIM_CheckIsImm, /*MI*/0, /*Op*/1,
      // (atomic_fence 5:{ *:[i32] }, (timm:{ *:[i32] }))  =>  (FENCE 3:{ *:[i32] }, 1:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FENCE,
      GIR_AddImm, /*InsnID*/0, /*Imm*/3,
      GIR_AddImm, /*InsnID*/0, /*Imm*/1,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 351,
      GIR_Done,
    // Label 651: @21200
    GIM_Try, /*On fail goto*//*Label 652*/ 21223, // Rule ID 353 //
      GIM_CheckConstantInt, /*MI*/0, /*Op*/0, 5,
      // MIs[0] Operand 1
      GIM_CheckIsImm, /*MI*/0, /*Op*/1,
      // (atomic_fence 5:{ *:[i32] }, (timm:{ *:[i32] }))  =>  (FENCE 3:{ *:[i32] }, 1:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FENCE,
      GIR_AddImm, /*InsnID*/0, /*Imm*/3,
      GIR_AddImm, /*InsnID*/0, /*Imm*/1,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 353,
      GIR_Done,
    // Label 652: @21223
    GIM_Try, /*On fail goto*//*Label 653*/ 21240, // Rule ID 354 //
      GIM_CheckConstantInt, /*MI*/0, /*Op*/0, 6,
      // MIs[0] Operand 1
      GIM_CheckIsImm, /*MI*/0, /*Op*/1,
      // (atomic_fence 6:{ *:[i32] }, (timm:{ *:[i32] }))  =>  (FENCE_TSO)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FENCE_TSO,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 354,
      GIR_Done,
    // Label 653: @21240
    GIM_Try, /*On fail goto*//*Label 654*/ 21257, // Rule ID 356 //
      GIM_CheckConstantInt, /*MI*/0, /*Op*/0, 6,
      // MIs[0] Operand 1
      GIM_CheckIsImm, /*MI*/0, /*Op*/1,
      // (atomic_fence 6:{ *:[i32] }, (timm:{ *:[i32] }))  =>  (FENCE_TSO)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FENCE_TSO,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 356,
      GIR_Done,
    // Label 654: @21257
    GIM_Try, /*On fail goto*//*Label 655*/ 21280, // Rule ID 357 //
      GIM_CheckConstantInt, /*MI*/0, /*Op*/0, 7,
      // MIs[0] Operand 1
      GIM_CheckIsImm, /*MI*/0, /*Op*/1,
      // (atomic_fence 7:{ *:[i32] }, (timm:{ *:[i32] }))  =>  (FENCE 3:{ *:[i32] }, 3:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FENCE,
      GIR_AddImm, /*InsnID*/0, /*Imm*/3,
      GIR_AddImm, /*InsnID*/0, /*Imm*/3,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 357,
      GIR_Done,
    // Label 655: @21280
    GIM_Try, /*On fail goto*//*Label 656*/ 21303, // Rule ID 359 //
      GIM_CheckConstantInt, /*MI*/0, /*Op*/0, 7,
      // MIs[0] Operand 1
      GIM_CheckIsImm, /*MI*/0, /*Op*/1,
      // (atomic_fence 7:{ *:[i32] }, (timm:{ *:[i32] }))  =>  (FENCE 3:{ *:[i32] }, 3:{ *:[i32] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FENCE,
      GIR_AddImm, /*InsnID*/0, /*Imm*/3,
      GIR_AddImm, /*InsnID*/0, /*Imm*/3,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 359,
      GIR_Done,
    // Label 656: @21303
    GIM_Reject,
    // Label 647: @21304
    GIM_Try, /*On fail goto*//*Label 657*/ 21327, // Rule ID 349 //
      GIM_CheckConstantInt, /*MI*/0, /*Op*/0, 4,
      // MIs[0] Operand 1
      GIM_CheckIsImm, /*MI*/0, /*Op*/1,
      // (atomic_fence 4:{ *:[i64] }, (timm:{ *:[i64] }))  =>  (FENCE 2:{ *:[i64] }, 3:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FENCE,
      GIR_AddImm, /*InsnID*/0, /*Imm*/2,
      GIR_AddImm, /*InsnID*/0, /*Imm*/3,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 349,
      GIR_Done,
    // Label 657: @21327
    GIM_Try, /*On fail goto*//*Label 658*/ 21350, // Rule ID 352 //
      GIM_CheckConstantInt, /*MI*/0, /*Op*/0, 5,
      // MIs[0] Operand 1
      GIM_CheckIsImm, /*MI*/0, /*Op*/1,
      // (atomic_fence 5:{ *:[i64] }, (timm:{ *:[i64] }))  =>  (FENCE 3:{ *:[i64] }, 1:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FENCE,
      GIR_AddImm, /*InsnID*/0, /*Imm*/3,
      GIR_AddImm, /*InsnID*/0, /*Imm*/1,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 352,
      GIR_Done,
    // Label 658: @21350
    GIM_Try, /*On fail goto*//*Label 659*/ 21367, // Rule ID 355 //
      GIM_CheckConstantInt, /*MI*/0, /*Op*/0, 6,
      // MIs[0] Operand 1
      GIM_CheckIsImm, /*MI*/0, /*Op*/1,
      // (atomic_fence 6:{ *:[i64] }, (timm:{ *:[i64] }))  =>  (FENCE_TSO)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FENCE_TSO,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 355,
      GIR_Done,
    // Label 659: @21367
    GIM_Try, /*On fail goto*//*Label 660*/ 21390, // Rule ID 358 //
      GIM_CheckConstantInt, /*MI*/0, /*Op*/0, 7,
      // MIs[0] Operand 1
      GIM_CheckIsImm, /*MI*/0, /*Op*/1,
      // (atomic_fence 7:{ *:[i64] }, (timm:{ *:[i64] }))  =>  (FENCE 3:{ *:[i64] }, 3:{ *:[i64] })
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FENCE,
      GIR_AddImm, /*InsnID*/0, /*Imm*/3,
      GIR_AddImm, /*InsnID*/0, /*Imm*/3,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 358,
      GIR_Done,
    // Label 660: @21390
    GIM_Reject,
    // Label 648: @21391
    GIM_Reject,
    // Label 27: @21392
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 663*/ 21482,
    /*GILLT_s32*//*Label 661*/ 21400,
    /*GILLT_s64*//*Label 662*/ 21453,
    // Label 661: @21400
    GIM_Try, /*On fail goto*//*Label 664*/ 21452,
      GIM_CheckI64ImmPredicate, /*MI*/0, /*Predicate*/GIPFP_I64_Predicate_simm12,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 665*/ 21430, // Rule ID 23 //
        // MIs[0] Operand 1
        // No operand predicates
        // (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm  =>  (ADDI:{ *:[i32] } X0:{ *:[i32] }, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::ADDI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddRegister, /*InsnID*/0, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/0, // imm
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 23,
        GIR_Done,
      // Label 665: @21430
      GIM_Try, /*On fail goto*//*Label 666*/ 21451, // Rule ID 25 //
        // MIs[0] Operand 1
        // No operand predicates
        // (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm  =>  (ADDI:{ *:[i32] } X0:{ *:[i32] }, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::ADDI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddRegister, /*InsnID*/0, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/0, // imm
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 25,
        GIR_Done,
      // Label 666: @21451
      GIM_Reject,
    // Label 664: @21452
    GIM_Reject,
    // Label 662: @21453
    GIM_Try, /*On fail goto*//*Label 667*/ 21481, // Rule ID 24 //
      GIM_CheckI64ImmPredicate, /*MI*/0, /*Predicate*/GIPFP_I64_Predicate_simm12,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      // No operand predicates
      // (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm  =>  (ADDI:{ *:[i64] } X0:{ *:[i64] }, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm)
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::ADDI,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_AddRegister, /*InsnID*/0, RISCV::X0, /*AddRegisterRegFlags*/0,
      GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/0, // imm
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 24,
      GIR_Done,
    // Label 667: @21481
    GIM_Reject,
    // Label 663: @21482
    GIM_Reject,
    // Label 28: @21483
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 670*/ 21834,
    /*GILLT_s32*//*Label 668*/ 21491,
    /*GILLT_s64*//*Label 669*/ 21713,
    // Label 668: @21491
    GIM_Try, /*On fail goto*//*Label 671*/ 21712,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 672*/ 21564, // Rule ID 71 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_AND,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_immbottomxlenset,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (shl:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (and:{ *:[i32] } GPR:{ *:[i32] }:$rs2, (imm:{ *:[i32] })<<P:Predicate_immbottomxlenset>>))  =>  (SLL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs2
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 71,
        GIR_Done,
      // Label 672: @21564
      GIM_Try, /*On fail goto*//*Label 673*/ 21619, // Rule ID 73 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_AND,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_immbottomxlenset,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (shl:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (and:{ *:[i32] } GPR:{ *:[i32] }:$rs2, (imm:{ *:[i32] })<<P:Predicate_immbottomxlenset>>))  =>  (SLL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs2
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 73,
        GIR_Done,
      // Label 673: @21619
      GIM_Try, /*On fail goto*//*Label 674*/ 21652, // Rule ID 59 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_uimmlog2xlen,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (shl:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_uimmlog2xlen>>:$shamt)  =>  (SLLI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_uimmlog2xlen>>:$shamt)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLLI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // shamt
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 59,
        GIR_Done,
      // Label 674: @21652
      GIM_Try, /*On fail goto*//*Label 675*/ 21685, // Rule ID 61 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_uimmlog2xlen,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (shl:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_uimmlog2xlen>>:$shamt)  =>  (SLLI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_uimmlog2xlen>>:$shamt)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLLI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // shamt
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 61,
        GIR_Done,
      // Label 675: @21685
      GIM_Try, /*On fail goto*//*Label 676*/ 21698, // Rule ID 68 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (shl:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (SLL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::SLL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 68,
        GIR_Done,
      // Label 676: @21698
      GIM_Try, /*On fail goto*//*Label 677*/ 21711, // Rule ID 70 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (shl:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (SLL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::SLL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 70,
        GIR_Done,
      // Label 677: @21711
      GIM_Reject,
    // Label 671: @21712
    GIM_Reject,
    // Label 669: @21713
    GIM_Try, /*On fail goto*//*Label 678*/ 21833,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 679*/ 21786, // Rule ID 72 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_AND,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_immbottomxlenset,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (shl:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (and:{ *:[i64] } GPR:{ *:[i64] }:$rs2, (imm:{ *:[i64] })<<P:Predicate_immbottomxlenset>>))  =>  (SLL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs2
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 72,
        GIR_Done,
      // Label 679: @21786
      GIM_Try, /*On fail goto*//*Label 680*/ 21819, // Rule ID 60 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_uimmlog2xlen,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (shl:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_uimmlog2xlen>>:$shamt)  =>  (SLLI:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_uimmlog2xlen>>:$shamt)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLLI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // shamt
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 60,
        GIR_Done,
      // Label 680: @21819
      GIM_Try, /*On fail goto*//*Label 681*/ 21832, // Rule ID 69 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (shl:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)  =>  (SLL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::SLL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 69,
        GIR_Done,
      // Label 681: @21832
      GIM_Reject,
    // Label 678: @21833
    GIM_Reject,
    // Label 670: @21834
    GIM_Reject,
    // Label 29: @21835
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 684*/ 22186,
    /*GILLT_s32*//*Label 682*/ 21843,
    /*GILLT_s64*//*Label 683*/ 22065,
    // Label 682: @21843
    GIM_Try, /*On fail goto*//*Label 685*/ 22064,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 686*/ 21916, // Rule ID 77 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_AND,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_immbottomxlenset,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (srl:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (and:{ *:[i32] } GPR:{ *:[i32] }:$rs2, (imm:{ *:[i32] })<<P:Predicate_immbottomxlenset>>))  =>  (SRL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SRL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs2
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 77,
        GIR_Done,
      // Label 686: @21916
      GIM_Try, /*On fail goto*//*Label 687*/ 21971, // Rule ID 79 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_AND,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_immbottomxlenset,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (srl:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (and:{ *:[i32] } GPR:{ *:[i32] }:$rs2, (imm:{ *:[i32] })<<P:Predicate_immbottomxlenset>>))  =>  (SRL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SRL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs2
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 79,
        GIR_Done,
      // Label 687: @21971
      GIM_Try, /*On fail goto*//*Label 688*/ 22004, // Rule ID 62 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_uimmlog2xlen,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (srl:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_uimmlog2xlen>>:$shamt)  =>  (SRLI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_uimmlog2xlen>>:$shamt)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SRLI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // shamt
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 62,
        GIR_Done,
      // Label 688: @22004
      GIM_Try, /*On fail goto*//*Label 689*/ 22037, // Rule ID 64 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_uimmlog2xlen,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (srl:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_uimmlog2xlen>>:$shamt)  =>  (SRLI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_uimmlog2xlen>>:$shamt)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SRLI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // shamt
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 64,
        GIR_Done,
      // Label 689: @22037
      GIM_Try, /*On fail goto*//*Label 690*/ 22050, // Rule ID 74 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (srl:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (SRL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::SRL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 74,
        GIR_Done,
      // Label 690: @22050
      GIM_Try, /*On fail goto*//*Label 691*/ 22063, // Rule ID 76 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (srl:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (SRL:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::SRL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 76,
        GIR_Done,
      // Label 691: @22063
      GIM_Reject,
    // Label 685: @22064
    GIM_Reject,
    // Label 683: @22065
    GIM_Try, /*On fail goto*//*Label 692*/ 22185,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 693*/ 22138, // Rule ID 78 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_AND,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_immbottomxlenset,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (srl:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (and:{ *:[i64] } GPR:{ *:[i64] }:$rs2, (imm:{ *:[i64] })<<P:Predicate_immbottomxlenset>>))  =>  (SRL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SRL,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs2
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 78,
        GIR_Done,
      // Label 693: @22138
      GIM_Try, /*On fail goto*//*Label 694*/ 22171, // Rule ID 63 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_uimmlog2xlen,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (srl:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_uimmlog2xlen>>:$shamt)  =>  (SRLI:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_uimmlog2xlen>>:$shamt)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SRLI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // shamt
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 63,
        GIR_Done,
      // Label 694: @22171
      GIM_Try, /*On fail goto*//*Label 695*/ 22184, // Rule ID 75 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (srl:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)  =>  (SRL:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::SRL,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 75,
        GIR_Done,
      // Label 695: @22184
      GIM_Reject,
    // Label 692: @22185
    GIM_Reject,
    // Label 684: @22186
    GIM_Reject,
    // Label 30: @22187
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 698*/ 22538,
    /*GILLT_s32*//*Label 696*/ 22195,
    /*GILLT_s64*//*Label 697*/ 22417,
    // Label 696: @22195
    GIM_Try, /*On fail goto*//*Label 699*/ 22416,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 700*/ 22268, // Rule ID 83 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_AND,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_immbottomxlenset,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (sra:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (and:{ *:[i32] } GPR:{ *:[i32] }:$rs2, (imm:{ *:[i32] })<<P:Predicate_immbottomxlenset>>))  =>  (SRA:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SRA,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs2
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 83,
        GIR_Done,
      // Label 700: @22268
      GIM_Try, /*On fail goto*//*Label 701*/ 22323, // Rule ID 85 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_AND,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_immbottomxlenset,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (sra:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (and:{ *:[i32] } GPR:{ *:[i32] }:$rs2, (imm:{ *:[i32] })<<P:Predicate_immbottomxlenset>>))  =>  (SRA:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SRA,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs2
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 85,
        GIR_Done,
      // Label 701: @22323
      GIM_Try, /*On fail goto*//*Label 702*/ 22356, // Rule ID 65 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_uimmlog2xlen,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (sra:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_uimmlog2xlen>>:$shamt)  =>  (SRAI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_uimmlog2xlen>>:$shamt)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SRAI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // shamt
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 65,
        GIR_Done,
      // Label 702: @22356
      GIM_Try, /*On fail goto*//*Label 703*/ 22389, // Rule ID 67 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_uimmlog2xlen,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (sra:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_uimmlog2xlen>>:$shamt)  =>  (SRAI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_uimmlog2xlen>>:$shamt)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SRAI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // shamt
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 67,
        GIR_Done,
      // Label 703: @22389
      GIM_Try, /*On fail goto*//*Label 704*/ 22402, // Rule ID 80 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (sra:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (SRA:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::SRA,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 80,
        GIR_Done,
      // Label 704: @22402
      GIM_Try, /*On fail goto*//*Label 705*/ 22415, // Rule ID 82 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (sra:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (SRA:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::SRA,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 82,
        GIR_Done,
      // Label 705: @22415
      GIM_Reject,
    // Label 699: @22416
    GIM_Reject,
    // Label 697: @22417
    GIM_Try, /*On fail goto*//*Label 706*/ 22537,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 707*/ 22490, // Rule ID 84 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_AND,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckType, /*MI*/1, /*Op*/2, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/1, /*OpIdx*/2, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/2, /*Predicate*/GIPFP_I64_Predicate_immbottomxlenset,
        // MIs[2] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (sra:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (and:{ *:[i64] } GPR:{ *:[i64] }:$rs2, (imm:{ *:[i64] })<<P:Predicate_immbottomxlenset>>))  =>  (SRA:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SRA,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs2
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 84,
        GIR_Done,
      // Label 707: @22490
      GIM_Try, /*On fail goto*//*Label 708*/ 22523, // Rule ID 66 //
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/2, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_uimmlog2xlen,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (sra:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_uimmlog2xlen>>:$shamt)  =>  (SRAI:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_uimmlog2xlen>>:$shamt)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SRAI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // shamt
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 66,
        GIR_Done,
      // Label 708: @22523
      GIM_Try, /*On fail goto*//*Label 709*/ 22536, // Rule ID 81 //
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        // (sra:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)  =>  (SRA:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::SRA,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 81,
        GIR_Done,
      // Label 709: @22536
      GIM_Reject,
    // Label 706: @22537
    GIM_Reject,
    // Label 698: @22538
    GIM_Reject,
    // Label 31: @22539
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 712*/ 24754,
    /*GILLT_s32*//*Label 710*/ 22547,
    /*GILLT_s64*//*Label 711*/ 24013,
    // Label 710: @22547
    GIM_Try, /*On fail goto*//*Label 713*/ 24012,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/3, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 714*/ 22594, // Rule ID 102 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_EQ,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckConstantInt, /*MI*/0, /*Op*/3, 0,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] }, SETEQ:{ *:[Other] })  =>  (SLTIU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 1:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 102,
        GIR_Done,
      // Label 714: @22594
      GIM_Try, /*On fail goto*//*Label 715*/ 22627, // Rule ID 104 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_EQ,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckConstantInt, /*MI*/0, /*Op*/3, 0,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] }, SETEQ:{ *:[Other] })  =>  (SLTIU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 1:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 104,
        GIR_Done,
      // Label 715: @22627
      GIM_Try, /*On fail goto*//*Label 716*/ 22661, // Rule ID 111 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_NE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckConstantInt, /*MI*/0, /*Op*/3, 0,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] }, SETNE:{ *:[Other] })  =>  (SLTU:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$rs1)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddRegister, /*InsnID*/0, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 111,
        GIR_Done,
      // Label 716: @22661
      GIM_Try, /*On fail goto*//*Label 717*/ 22695, // Rule ID 113 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_NE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckConstantInt, /*MI*/0, /*Op*/3, 0,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, 0:{ *:[i32] }, SETNE:{ *:[Other] })  =>  (SLTU:{ *:[i32] } X0:{ *:[i32] }, GPR:{ *:[i32] }:$rs1)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddRegister, /*InsnID*/0, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 113,
        GIR_Done,
      // Label 717: @22695
      GIM_Try, /*On fail goto*//*Label 718*/ 22736, // Rule ID 93 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_SLT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12, SETLT:{ *:[Other] })  =>  (SLTI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 93,
        GIR_Done,
      // Label 718: @22736
      GIM_Try, /*On fail goto*//*Label 719*/ 22777, // Rule ID 95 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_SLT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12, SETLT:{ *:[Other] })  =>  (SLTI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 95,
        GIR_Done,
      // Label 719: @22777
      GIM_Try, /*On fail goto*//*Label 720*/ 22818, // Rule ID 99 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_ULT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12, SETULT:{ *:[Other] })  =>  (SLTIU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 99,
        GIR_Done,
      // Label 720: @22818
      GIM_Try, /*On fail goto*//*Label 721*/ 22859, // Rule ID 101 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_ULT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12, SETULT:{ *:[Other] })  =>  (SLTIU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 101,
        GIR_Done,
      // Label 721: @22859
      GIM_Try, /*On fail goto*//*Label 722*/ 22919, // Rule ID 108 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_EQ,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12, SETEQ:{ *:[Other] })  =>  (SLTIU:{ *:[i32] } (XORI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12), 1:{ *:[i32] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::XORI,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/1, /*OldInsnID*/1, // imm12
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 108,
        GIR_Done,
      // Label 722: @22919
      GIM_Try, /*On fail goto*//*Label 723*/ 22979, // Rule ID 110 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_EQ,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12, SETEQ:{ *:[Other] })  =>  (SLTIU:{ *:[i32] } (XORI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12), 1:{ *:[i32] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::XORI,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/1, /*OldInsnID*/1, // imm12
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 110,
        GIR_Done,
      // Label 723: @22979
      GIM_Try, /*On fail goto*//*Label 724*/ 23040, // Rule ID 117 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_NE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12, SETNE:{ *:[Other] })  =>  (SLTU:{ *:[i32] } X0:{ *:[i32] }, (XORI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::XORI,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/1, /*OldInsnID*/1, // imm12
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddRegister, /*InsnID*/0, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 117,
        GIR_Done,
      // Label 724: @23040
      GIM_Try, /*On fail goto*//*Label 725*/ 23101, // Rule ID 119 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_NE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12, SETNE:{ *:[Other] })  =>  (SLTU:{ *:[i32] } X0:{ *:[i32] }, (XORI:{ *:[i32] } GPR:{ *:[i32] }:$rs1, (imm:{ *:[i32] })<<P:Predicate_simm12>>:$imm12))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::XORI,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/1, /*OldInsnID*/1, // imm12
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddRegister, /*InsnID*/0, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 119,
        GIR_Done,
      // Label 725: @23101
      GIM_Try, /*On fail goto*//*Label 726*/ 23135, // Rule ID 90 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_SLT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETLT:{ *:[Other] })  =>  (SLT:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLT,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 90,
        GIR_Done,
      // Label 726: @23135
      GIM_Try, /*On fail goto*//*Label 727*/ 23169, // Rule ID 92 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_SLT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETLT:{ *:[Other] })  =>  (SLT:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLT,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 92,
        GIR_Done,
      // Label 727: @23169
      GIM_Try, /*On fail goto*//*Label 728*/ 23203, // Rule ID 96 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_ULT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETULT:{ *:[Other] })  =>  (SLTU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 96,
        GIR_Done,
      // Label 728: @23203
      GIM_Try, /*On fail goto*//*Label 729*/ 23237, // Rule ID 98 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_ULT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETULT:{ *:[Other] })  =>  (SLTU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 98,
        GIR_Done,
      // Label 729: @23237
      GIM_Try, /*On fail goto*//*Label 730*/ 23290, // Rule ID 105 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_EQ,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETEQ:{ *:[Other] })  =>  (SLTIU:{ *:[i32] } (XOR:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2), 1:{ *:[i32] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::XOR,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 105,
        GIR_Done,
      // Label 730: @23290
      GIM_Try, /*On fail goto*//*Label 731*/ 23343, // Rule ID 107 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_EQ,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETEQ:{ *:[Other] })  =>  (SLTIU:{ *:[i32] } (XOR:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2), 1:{ *:[i32] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::XOR,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 107,
        GIR_Done,
      // Label 731: @23343
      GIM_Try, /*On fail goto*//*Label 732*/ 23397, // Rule ID 114 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_NE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETNE:{ *:[Other] })  =>  (SLTU:{ *:[i32] } X0:{ *:[i32] }, (XOR:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::XOR,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddRegister, /*InsnID*/0, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 114,
        GIR_Done,
      // Label 732: @23397
      GIM_Try, /*On fail goto*//*Label 733*/ 23451, // Rule ID 116 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_NE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETNE:{ *:[Other] })  =>  (SLTU:{ *:[i32] } X0:{ *:[i32] }, (XOR:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::XOR,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddRegister, /*InsnID*/0, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 116,
        GIR_Done,
      // Label 733: @23451
      GIM_Try, /*On fail goto*//*Label 734*/ 23485, // Rule ID 120 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_UGT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETUGT:{ *:[Other] })  =>  (SLTU:{ *:[i32] } GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 120,
        GIR_Done,
      // Label 734: @23485
      GIM_Try, /*On fail goto*//*Label 735*/ 23519, // Rule ID 122 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_UGT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETUGT:{ *:[Other] })  =>  (SLTU:{ *:[i32] } GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 122,
        GIR_Done,
      // Label 735: @23519
      GIM_Try, /*On fail goto*//*Label 736*/ 23572, // Rule ID 123 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_UGE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETUGE:{ *:[Other] })  =>  (XORI:{ *:[i32] } (SLTU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2), 1:{ *:[i32] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SLTU,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::XORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 123,
        GIR_Done,
      // Label 736: @23572
      GIM_Try, /*On fail goto*//*Label 737*/ 23625, // Rule ID 125 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_UGE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETUGE:{ *:[Other] })  =>  (XORI:{ *:[i32] } (SLTU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2), 1:{ *:[i32] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SLTU,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::XORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 125,
        GIR_Done,
      // Label 737: @23625
      GIM_Try, /*On fail goto*//*Label 738*/ 23678, // Rule ID 126 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_ULE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETULE:{ *:[Other] })  =>  (XORI:{ *:[i32] } (SLTU:{ *:[i32] } GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1), 1:{ *:[i32] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SLTU,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::XORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 126,
        GIR_Done,
      // Label 738: @23678
      GIM_Try, /*On fail goto*//*Label 739*/ 23731, // Rule ID 128 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_ULE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETULE:{ *:[Other] })  =>  (XORI:{ *:[i32] } (SLTU:{ *:[i32] } GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1), 1:{ *:[i32] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SLTU,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::XORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 128,
        GIR_Done,
      // Label 739: @23731
      GIM_Try, /*On fail goto*//*Label 740*/ 23765, // Rule ID 129 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_SGT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETGT:{ *:[Other] })  =>  (SLT:{ *:[i32] } GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLT,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 129,
        GIR_Done,
      // Label 740: @23765
      GIM_Try, /*On fail goto*//*Label 741*/ 23799, // Rule ID 131 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_SGT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETGT:{ *:[Other] })  =>  (SLT:{ *:[i32] } GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLT,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 131,
        GIR_Done,
      // Label 741: @23799
      GIM_Try, /*On fail goto*//*Label 742*/ 23852, // Rule ID 132 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_SGE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETGE:{ *:[Other] })  =>  (XORI:{ *:[i32] } (SLT:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2), 1:{ *:[i32] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SLT,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::XORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 132,
        GIR_Done,
      // Label 742: @23852
      GIM_Try, /*On fail goto*//*Label 743*/ 23905, // Rule ID 134 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_SGE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETGE:{ *:[Other] })  =>  (XORI:{ *:[i32] } (SLT:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2), 1:{ *:[i32] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SLT,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::XORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 134,
        GIR_Done,
      // Label 743: @23905
      GIM_Try, /*On fail goto*//*Label 744*/ 23958, // Rule ID 135 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_SLE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETLE:{ *:[Other] })  =>  (XORI:{ *:[i32] } (SLT:{ *:[i32] } GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1), 1:{ *:[i32] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SLT,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::XORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 135,
        GIR_Done,
      // Label 744: @23958
      GIM_Try, /*On fail goto*//*Label 745*/ 24011, // Rule ID 137 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_SLE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2, SETLE:{ *:[Other] })  =>  (XORI:{ *:[i32] } (SLT:{ *:[i32] } GPR:{ *:[i32] }:$rs2, GPR:{ *:[i32] }:$rs1), 1:{ *:[i32] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SLT,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::XORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 137,
        GIR_Done,
      // Label 745: @24011
      GIM_Reject,
    // Label 713: @24012
    GIM_Reject,
    // Label 711: @24013
    GIM_Try, /*On fail goto*//*Label 746*/ 24753,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/3, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 747*/ 24060, // Rule ID 103 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_EQ,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckConstantInt, /*MI*/0, /*Op*/3, 0,
        // (setcc:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] }, SETEQ:{ *:[Other] })  =>  (SLTIU:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 1:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 103,
        GIR_Done,
      // Label 747: @24060
      GIM_Try, /*On fail goto*//*Label 748*/ 24094, // Rule ID 112 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_NE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckConstantInt, /*MI*/0, /*Op*/3, 0,
        // (setcc:{ *:[i64] } GPR:{ *:[i64] }:$rs1, 0:{ *:[i64] }, SETNE:{ *:[Other] })  =>  (SLTU:{ *:[i64] } X0:{ *:[i64] }, GPR:{ *:[i64] }:$rs1)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddRegister, /*InsnID*/0, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 112,
        GIR_Done,
      // Label 748: @24094
      GIM_Try, /*On fail goto*//*Label 749*/ 24135, // Rule ID 94 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_SLT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (setcc:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12, SETLT:{ *:[Other] })  =>  (SLTI:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 94,
        GIR_Done,
      // Label 749: @24135
      GIM_Try, /*On fail goto*//*Label 750*/ 24176, // Rule ID 100 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_ULT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (setcc:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12, SETULT:{ *:[Other] })  =>  (SLTIU:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/0, /*OldInsnID*/1, // imm12
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 100,
        GIR_Done,
      // Label 750: @24176
      GIM_Try, /*On fail goto*//*Label 751*/ 24236, // Rule ID 109 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_EQ,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (setcc:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12, SETEQ:{ *:[Other] })  =>  (SLTIU:{ *:[i64] } (XORI:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12), 1:{ *:[i64] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::XORI,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/1, /*OldInsnID*/1, // imm12
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 109,
        GIR_Done,
      // Label 751: @24236
      GIM_Try, /*On fail goto*//*Label 752*/ 24297, // Rule ID 118 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_NE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_CONSTANT,
        GIM_CheckI64ImmPredicate, /*MI*/1, /*Predicate*/GIPFP_I64_Predicate_simm12,
        // MIs[1] Operand 1
        // No operand predicates
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (setcc:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12, SETNE:{ *:[Other] })  =>  (SLTU:{ *:[i64] } X0:{ *:[i64] }, (XORI:{ *:[i64] } GPR:{ *:[i64] }:$rs1, (imm:{ *:[i64] })<<P:Predicate_simm12>>:$imm12))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::XORI,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_CopyConstantAsSImm, /*NewInsnID*/1, /*OldInsnID*/1, // imm12
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddRegister, /*InsnID*/0, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 118,
        GIR_Done,
      // Label 752: @24297
      GIM_Try, /*On fail goto*//*Label 753*/ 24331, // Rule ID 91 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_SLT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2, SETLT:{ *:[Other] })  =>  (SLT:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLT,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 91,
        GIR_Done,
      // Label 753: @24331
      GIM_Try, /*On fail goto*//*Label 754*/ 24365, // Rule ID 97 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_ULT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2, SETULT:{ *:[Other] })  =>  (SLTU:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 97,
        GIR_Done,
      // Label 754: @24365
      GIM_Try, /*On fail goto*//*Label 755*/ 24418, // Rule ID 106 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_EQ,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2, SETEQ:{ *:[Other] })  =>  (SLTIU:{ *:[i64] } (XOR:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2), 1:{ *:[i64] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::XOR,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 106,
        GIR_Done,
      // Label 755: @24418
      GIM_Try, /*On fail goto*//*Label 756*/ 24472, // Rule ID 115 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_NE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2, SETNE:{ *:[Other] })  =>  (SLTU:{ *:[i64] } X0:{ *:[i64] }, (XOR:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2))
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::XOR,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddRegister, /*InsnID*/0, RISCV::X0, /*AddRegisterRegFlags*/0,
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 115,
        GIR_Done,
      // Label 756: @24472
      GIM_Try, /*On fail goto*//*Label 757*/ 24506, // Rule ID 121 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_UGT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2, SETUGT:{ *:[Other] })  =>  (SLTU:{ *:[i64] } GPR:{ *:[i64] }:$rs2, GPR:{ *:[i64] }:$rs1)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 121,
        GIR_Done,
      // Label 757: @24506
      GIM_Try, /*On fail goto*//*Label 758*/ 24559, // Rule ID 124 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_UGE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2, SETUGE:{ *:[Other] })  =>  (XORI:{ *:[i64] } (SLTU:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2), 1:{ *:[i64] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SLTU,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::XORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 124,
        GIR_Done,
      // Label 758: @24559
      GIM_Try, /*On fail goto*//*Label 759*/ 24612, // Rule ID 127 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_ULE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2, SETULE:{ *:[Other] })  =>  (XORI:{ *:[i64] } (SLTU:{ *:[i64] } GPR:{ *:[i64] }:$rs2, GPR:{ *:[i64] }:$rs1), 1:{ *:[i64] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SLTU,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::XORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 127,
        GIR_Done,
      // Label 759: @24612
      GIM_Try, /*On fail goto*//*Label 760*/ 24646, // Rule ID 130 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_SGT,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2, SETGT:{ *:[Other] })  =>  (SLT:{ *:[i64] } GPR:{ *:[i64] }:$rs2, GPR:{ *:[i64] }:$rs1)
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLT,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 130,
        GIR_Done,
      // Label 760: @24646
      GIM_Try, /*On fail goto*//*Label 761*/ 24699, // Rule ID 133 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_SGE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2, SETGE:{ *:[Other] })  =>  (XORI:{ *:[i64] } (SLT:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2), 1:{ *:[i64] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SLT,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::XORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 133,
        GIR_Done,
      // Label 761: @24699
      GIM_Try, /*On fail goto*//*Label 762*/ 24752, // Rule ID 136 //
        // MIs[0] Operand 1
        GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::ICMP_SLE,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::GPRRegClassID,
        // (setcc:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2, SETLE:{ *:[Other] })  =>  (XORI:{ *:[i64] } (SLT:{ *:[i64] } GPR:{ *:[i64] }:$rs2, GPR:{ *:[i64] }:$rs1), 1:{ *:[i64] })
        GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
        GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::SLT,
        GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/3, // rs2
        GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
        GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::XORI,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 136,
        GIR_Done,
      // Label 762: @24752
      GIM_Reject,
    // Label 746: @24753
    GIM_Reject,
    // Label 712: @24754
    GIM_Reject,
    // Label 32: @24755
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 765*/ 25935,
    /*GILLT_s32*//*Label 763*/ 24763,
    /*GILLT_s64*//*Label 764*/ 25544,
    // Label 763: @24763
    GIM_Try, /*On fail goto*//*Label 766*/ 24851, // Rule ID 1124 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/3, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::FCMP_ORD,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR32RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR32RegClassID,
      // (setcc:{ *:[i32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, SETO:{ *:[Other] })  =>  (AND:{ *:[i32] } (FEQ_S:{ *:[i32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs1), (FEQ_S:{ *:[i32] } FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs2))
      GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
      GIR_MakeTempReg, /*TempRegID*/1, /*TypeID*/GILLT_s32,
      GIR_BuildMI, /*InsnID*/2, /*Opcode*/RISCV::FEQ_S,
      GIR_AddTempRegister, /*InsnID*/2, /*TempRegID*/1, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_ConstrainSelectedInstOperands, /*InsnID*/2,
      GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::FEQ_S,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AND,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/1, /*TempRegFlags*/0,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1124,
      GIR_Done,
    // Label 766: @24851
    GIM_Try, /*On fail goto*//*Label 767*/ 24939, // Rule ID 1126 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/3, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::FCMP_ORD,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR32RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR32RegClassID,
      // (setcc:{ *:[i32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, SETO:{ *:[Other] })  =>  (AND:{ *:[i32] } (FEQ_S:{ *:[i32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs1), (FEQ_S:{ *:[i32] } FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs2))
      GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
      GIR_MakeTempReg, /*TempRegID*/1, /*TypeID*/GILLT_s32,
      GIR_BuildMI, /*InsnID*/2, /*Opcode*/RISCV::FEQ_S,
      GIR_AddTempRegister, /*InsnID*/2, /*TempRegID*/1, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_ConstrainSelectedInstOperands, /*InsnID*/2,
      GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::FEQ_S,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AND,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/1, /*TempRegFlags*/0,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1126,
      GIR_Done,
    // Label 767: @24939
    GIM_Try, /*On fail goto*//*Label 768*/ 25046, // Rule ID 1127 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/3, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::FCMP_UNO,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR32RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR32RegClassID,
      // (setcc:{ *:[i32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, SETUO:{ *:[Other] })  =>  (SLTIU:{ *:[i32] } (AND:{ *:[i32] } (FEQ_S:{ *:[i32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs1), (FEQ_S:{ *:[i32] } FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs2)), 1:{ *:[i32] })
      GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
      GIR_MakeTempReg, /*TempRegID*/1, /*TypeID*/GILLT_s32,
      GIR_MakeTempReg, /*TempRegID*/2, /*TypeID*/GILLT_s32,
      GIR_BuildMI, /*InsnID*/3, /*Opcode*/RISCV::FEQ_S,
      GIR_AddTempRegister, /*InsnID*/3, /*TempRegID*/2, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/3, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_Copy, /*NewInsnID*/3, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_ConstrainSelectedInstOperands, /*InsnID*/3,
      GIR_BuildMI, /*InsnID*/2, /*Opcode*/RISCV::FEQ_S,
      GIR_AddTempRegister, /*InsnID*/2, /*TempRegID*/1, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_ConstrainSelectedInstOperands, /*InsnID*/2,
      GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::AND,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/1, /*TempRegFlags*/0,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/2, /*TempRegFlags*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
      GIR_AddImm, /*InsnID*/0, /*Imm*/1,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1127,
      GIR_Done,
    // Label 768: @25046
    GIM_Try, /*On fail goto*//*Label 769*/ 25153, // Rule ID 1129 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/3, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::FCMP_UNO,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR32RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR32RegClassID,
      // (setcc:{ *:[i32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, SETUO:{ *:[Other] })  =>  (SLTIU:{ *:[i32] } (AND:{ *:[i32] } (FEQ_S:{ *:[i32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs1), (FEQ_S:{ *:[i32] } FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs2)), 1:{ *:[i32] })
      GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
      GIR_MakeTempReg, /*TempRegID*/1, /*TypeID*/GILLT_s32,
      GIR_MakeTempReg, /*TempRegID*/2, /*TypeID*/GILLT_s32,
      GIR_BuildMI, /*InsnID*/3, /*Opcode*/RISCV::FEQ_S,
      GIR_AddTempRegister, /*InsnID*/3, /*TempRegID*/2, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/3, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_Copy, /*NewInsnID*/3, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_ConstrainSelectedInstOperands, /*InsnID*/3,
      GIR_BuildMI, /*InsnID*/2, /*Opcode*/RISCV::FEQ_S,
      GIR_AddTempRegister, /*InsnID*/2, /*TempRegID*/1, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_ConstrainSelectedInstOperands, /*InsnID*/2,
      GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::AND,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/1, /*TempRegFlags*/0,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/2, /*TempRegFlags*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
      GIR_AddImm, /*InsnID*/0, /*Imm*/1,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1129,
      GIR_Done,
    // Label 769: @25153
    GIM_Try, /*On fail goto*//*Label 770*/ 25241, // Rule ID 1267 //
      GIM_CheckFeatures, GIFBS_HasStdExtD,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/3, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::FCMP_ORD,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR64RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR64RegClassID,
      // (setcc:{ *:[i32] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, SETO:{ *:[Other] })  =>  (AND:{ *:[i32] } (FEQ_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs1), (FEQ_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs2))
      GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
      GIR_MakeTempReg, /*TempRegID*/1, /*TypeID*/GILLT_s32,
      GIR_BuildMI, /*InsnID*/2, /*Opcode*/RISCV::FEQ_D,
      GIR_AddTempRegister, /*InsnID*/2, /*TempRegID*/1, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_ConstrainSelectedInstOperands, /*InsnID*/2,
      GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::FEQ_D,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AND,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/1, /*TempRegFlags*/0,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1267,
      GIR_Done,
    // Label 770: @25241
    GIM_Try, /*On fail goto*//*Label 771*/ 25329, // Rule ID 1269 //
      GIM_CheckFeatures, GIFBS_HasStdExtD,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/3, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::FCMP_ORD,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR64RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR64RegClassID,
      // (setcc:{ *:[i32] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, SETO:{ *:[Other] })  =>  (AND:{ *:[i32] } (FEQ_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs1), (FEQ_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs2))
      GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
      GIR_MakeTempReg, /*TempRegID*/1, /*TypeID*/GILLT_s32,
      GIR_BuildMI, /*InsnID*/2, /*Opcode*/RISCV::FEQ_D,
      GIR_AddTempRegister, /*InsnID*/2, /*TempRegID*/1, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_ConstrainSelectedInstOperands, /*InsnID*/2,
      GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::FEQ_D,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AND,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/1, /*TempRegFlags*/0,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1269,
      GIR_Done,
    // Label 771: @25329
    GIM_Try, /*On fail goto*//*Label 772*/ 25436, // Rule ID 1270 //
      GIM_CheckFeatures, GIFBS_HasStdExtD,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/3, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::FCMP_UNO,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR64RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR64RegClassID,
      // (setcc:{ *:[i32] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, SETUO:{ *:[Other] })  =>  (SLTIU:{ *:[i32] } (AND:{ *:[i32] } (FEQ_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs1), (FEQ_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs2)), 1:{ *:[i32] })
      GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
      GIR_MakeTempReg, /*TempRegID*/1, /*TypeID*/GILLT_s32,
      GIR_MakeTempReg, /*TempRegID*/2, /*TypeID*/GILLT_s32,
      GIR_BuildMI, /*InsnID*/3, /*Opcode*/RISCV::FEQ_D,
      GIR_AddTempRegister, /*InsnID*/3, /*TempRegID*/2, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/3, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_Copy, /*NewInsnID*/3, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_ConstrainSelectedInstOperands, /*InsnID*/3,
      GIR_BuildMI, /*InsnID*/2, /*Opcode*/RISCV::FEQ_D,
      GIR_AddTempRegister, /*InsnID*/2, /*TempRegID*/1, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_ConstrainSelectedInstOperands, /*InsnID*/2,
      GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::AND,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/1, /*TempRegFlags*/0,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/2, /*TempRegFlags*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
      GIR_AddImm, /*InsnID*/0, /*Imm*/1,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1270,
      GIR_Done,
    // Label 772: @25436
    GIM_Try, /*On fail goto*//*Label 773*/ 25543, // Rule ID 1272 //
      GIM_CheckFeatures, GIFBS_HasStdExtD,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/3, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::FCMP_UNO,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR64RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR64RegClassID,
      // (setcc:{ *:[i32] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, SETUO:{ *:[Other] })  =>  (SLTIU:{ *:[i32] } (AND:{ *:[i32] } (FEQ_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs1), (FEQ_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs2)), 1:{ *:[i32] })
      GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s32,
      GIR_MakeTempReg, /*TempRegID*/1, /*TypeID*/GILLT_s32,
      GIR_MakeTempReg, /*TempRegID*/2, /*TypeID*/GILLT_s32,
      GIR_BuildMI, /*InsnID*/3, /*Opcode*/RISCV::FEQ_D,
      GIR_AddTempRegister, /*InsnID*/3, /*TempRegID*/2, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/3, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_Copy, /*NewInsnID*/3, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_ConstrainSelectedInstOperands, /*InsnID*/3,
      GIR_BuildMI, /*InsnID*/2, /*Opcode*/RISCV::FEQ_D,
      GIR_AddTempRegister, /*InsnID*/2, /*TempRegID*/1, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_ConstrainSelectedInstOperands, /*InsnID*/2,
      GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::AND,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/1, /*TempRegFlags*/0,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/2, /*TempRegFlags*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
      GIR_AddImm, /*InsnID*/0, /*Imm*/1,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1272,
      GIR_Done,
    // Label 773: @25543
    GIM_Reject,
    // Label 764: @25544
    GIM_Try, /*On fail goto*//*Label 774*/ 25632, // Rule ID 1125 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/3, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::FCMP_ORD,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR32RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR32RegClassID,
      // (setcc:{ *:[i64] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, SETO:{ *:[Other] })  =>  (AND:{ *:[i64] } (FEQ_S:{ *:[i64] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs1), (FEQ_S:{ *:[i64] } FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs2))
      GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
      GIR_MakeTempReg, /*TempRegID*/1, /*TypeID*/GILLT_s64,
      GIR_BuildMI, /*InsnID*/2, /*Opcode*/RISCV::FEQ_S,
      GIR_AddTempRegister, /*InsnID*/2, /*TempRegID*/1, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_ConstrainSelectedInstOperands, /*InsnID*/2,
      GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::FEQ_S,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AND,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/1, /*TempRegFlags*/0,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1125,
      GIR_Done,
    // Label 774: @25632
    GIM_Try, /*On fail goto*//*Label 775*/ 25739, // Rule ID 1128 //
      GIM_CheckFeatures, GIFBS_HasStdExtF,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/3, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::FCMP_UNO,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR32RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR32RegClassID,
      // (setcc:{ *:[i64] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, SETUO:{ *:[Other] })  =>  (SLTIU:{ *:[i64] } (AND:{ *:[i64] } (FEQ_S:{ *:[i64] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs1), (FEQ_S:{ *:[i64] } FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs2)), 1:{ *:[i64] })
      GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
      GIR_MakeTempReg, /*TempRegID*/1, /*TypeID*/GILLT_s64,
      GIR_MakeTempReg, /*TempRegID*/2, /*TypeID*/GILLT_s64,
      GIR_BuildMI, /*InsnID*/3, /*Opcode*/RISCV::FEQ_S,
      GIR_AddTempRegister, /*InsnID*/3, /*TempRegID*/2, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/3, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_Copy, /*NewInsnID*/3, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_ConstrainSelectedInstOperands, /*InsnID*/3,
      GIR_BuildMI, /*InsnID*/2, /*Opcode*/RISCV::FEQ_S,
      GIR_AddTempRegister, /*InsnID*/2, /*TempRegID*/1, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_ConstrainSelectedInstOperands, /*InsnID*/2,
      GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::AND,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/1, /*TempRegFlags*/0,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/2, /*TempRegFlags*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
      GIR_AddImm, /*InsnID*/0, /*Imm*/1,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1128,
      GIR_Done,
    // Label 775: @25739
    GIM_Try, /*On fail goto*//*Label 776*/ 25827, // Rule ID 1268 //
      GIM_CheckFeatures, GIFBS_HasStdExtD,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/3, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::FCMP_ORD,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR64RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR64RegClassID,
      // (setcc:{ *:[i64] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, SETO:{ *:[Other] })  =>  (AND:{ *:[i64] } (FEQ_D:{ *:[i64] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs1), (FEQ_D:{ *:[i64] } FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs2))
      GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
      GIR_MakeTempReg, /*TempRegID*/1, /*TypeID*/GILLT_s64,
      GIR_BuildMI, /*InsnID*/2, /*Opcode*/RISCV::FEQ_D,
      GIR_AddTempRegister, /*InsnID*/2, /*TempRegID*/1, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_ConstrainSelectedInstOperands, /*InsnID*/2,
      GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::FEQ_D,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_Copy, /*NewInsnID*/1, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::AND,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/1, /*TempRegFlags*/0,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1268,
      GIR_Done,
    // Label 776: @25827
    GIM_Try, /*On fail goto*//*Label 777*/ 25934, // Rule ID 1271 //
      GIM_CheckFeatures, GIFBS_HasStdExtD,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/3, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      // MIs[0] Operand 1
      GIM_CheckCmpPredicate, /*MI*/0, /*Op*/1, /*Predicate*/CmpInst::FCMP_UNO,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR64RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR64RegClassID,
      // (setcc:{ *:[i64] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, SETUO:{ *:[Other] })  =>  (SLTIU:{ *:[i64] } (AND:{ *:[i64] } (FEQ_D:{ *:[i64] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs1), (FEQ_D:{ *:[i64] } FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs2)), 1:{ *:[i64] })
      GIR_MakeTempReg, /*TempRegID*/0, /*TypeID*/GILLT_s64,
      GIR_MakeTempReg, /*TempRegID*/1, /*TypeID*/GILLT_s64,
      GIR_MakeTempReg, /*TempRegID*/2, /*TypeID*/GILLT_s64,
      GIR_BuildMI, /*InsnID*/3, /*Opcode*/RISCV::FEQ_D,
      GIR_AddTempRegister, /*InsnID*/3, /*TempRegID*/2, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/3, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_Copy, /*NewInsnID*/3, /*OldInsnID*/0, /*OpIdx*/3, // rs2
      GIR_ConstrainSelectedInstOperands, /*InsnID*/3,
      GIR_BuildMI, /*InsnID*/2, /*Opcode*/RISCV::FEQ_D,
      GIR_AddTempRegister, /*InsnID*/2, /*TempRegID*/1, /*TempRegFlags*/RegState::Define,
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_Copy, /*NewInsnID*/2, /*OldInsnID*/0, /*OpIdx*/2, // rs1
      GIR_ConstrainSelectedInstOperands, /*InsnID*/2,
      GIR_BuildMI, /*InsnID*/1, /*Opcode*/RISCV::AND,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/0, /*TempRegFlags*/RegState::Define,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/1, /*TempRegFlags*/0,
      GIR_AddTempRegister, /*InsnID*/1, /*TempRegID*/2, /*TempRegFlags*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/1,
      GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::SLTIU,
      GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
      GIR_AddTempRegister, /*InsnID*/0, /*TempRegID*/0, /*TempRegFlags*/0,
      GIR_AddImm, /*InsnID*/0, /*Imm*/1,
      GIR_EraseFromParent, /*InsnID*/0,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1271,
      GIR_Done,
    // Label 777: @25934
    GIM_Reject,
    // Label 765: @25935
    GIM_Reject,
    // Label 33: @25936
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 780*/ 26022,
    /*GILLT_s32*//*Label 778*/ 25944,
    /*GILLT_s64*//*Label 779*/ 25990,
    // Label 778: @25944
    GIM_Try, /*On fail goto*//*Label 781*/ 25989,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 782*/ 25977, // Rule ID 479 //
        GIM_CheckFeatures, GIFBS_HasStdExtM,
        // (mulhu:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (MULHU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::MULHU,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 479,
        GIR_Done,
      // Label 782: @25977
      GIM_Try, /*On fail goto*//*Label 783*/ 25988, // Rule ID 481 //
        GIM_CheckFeatures, GIFBS_HasStdExtM,
        // (mulhu:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (MULHU:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::MULHU,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 481,
        GIR_Done,
      // Label 783: @25988
      GIM_Reject,
    // Label 781: @25989
    GIM_Reject,
    // Label 779: @25990
    GIM_Try, /*On fail goto*//*Label 784*/ 26021, // Rule ID 480 //
      GIM_CheckFeatures, GIFBS_HasStdExtM,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
      // (mulhu:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)  =>  (MULHU:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::MULHU,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 480,
      GIR_Done,
    // Label 784: @26021
    GIM_Reject,
    // Label 780: @26022
    GIM_Reject,
    // Label 34: @26023
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 787*/ 26109,
    /*GILLT_s32*//*Label 785*/ 26031,
    /*GILLT_s64*//*Label 786*/ 26077,
    // Label 785: @26031
    GIM_Try, /*On fail goto*//*Label 788*/ 26076,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
      GIM_Try, /*On fail goto*//*Label 789*/ 26064, // Rule ID 476 //
        GIM_CheckFeatures, GIFBS_HasStdExtM,
        // (mulhs:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (MULH:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::MULH,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 476,
        GIR_Done,
      // Label 789: @26064
      GIM_Try, /*On fail goto*//*Label 790*/ 26075, // Rule ID 478 //
        GIM_CheckFeatures, GIFBS_HasStdExtM,
        // (mulhs:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)  =>  (MULH:{ *:[i32] } GPR:{ *:[i32] }:$rs1, GPR:{ *:[i32] }:$rs2)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::MULH,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 478,
        GIR_Done,
      // Label 790: @26075
      GIM_Reject,
    // Label 788: @26076
    GIM_Reject,
    // Label 786: @26077
    GIM_Try, /*On fail goto*//*Label 791*/ 26108, // Rule ID 477 //
      GIM_CheckFeatures, GIFBS_HasStdExtM,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::GPRRegClassID,
      // (mulhs:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)  =>  (MULH:{ *:[i64] } GPR:{ *:[i64] }:$rs1, GPR:{ *:[i64] }:$rs2)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::MULH,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 477,
      GIR_Done,
    // Label 791: @26108
    GIM_Reject,
    // Label 787: @26109
    GIM_Reject,
    // Label 35: @26110
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 794*/ 27244,
    /*GILLT_s32*//*Label 792*/ 26118,
    /*GILLT_s64*//*Label 793*/ 26681,
    // Label 792: @26118
    GIM_Try, /*On fail goto*//*Label 795*/ 26680,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/3, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
      GIM_Try, /*On fail goto*//*Label 796*/ 26205, // Rule ID 1101 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR32RegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/0, /*OpIdx*/3, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/2, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/2, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (fma:{ *:[f32] } (fneg:{ *:[f32] } FPR32:{ *:[f32] }:$rs1), FPR32:{ *:[f32] }:$rs2, (fneg:{ *:[f32] } FPR32:{ *:[f32] }:$rs3))  =>  (FNMADD_S:{ *:[f32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs3, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FNMADD_S,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/2, /*OpIdx*/1, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1101,
        GIR_Done,
      // Label 796: @26205
      GIM_Try, /*On fail goto*//*Label 797*/ 26274, // Rule ID 1102 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR32RegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/0, /*OpIdx*/3, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/2, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/2, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (fma:{ *:[f32] } (fneg:{ *:[f32] } FPR32:{ *:[f32] }:$rs1), FPR32:{ *:[f32] }:$rs2, (fneg:{ *:[f32] } FPR32:{ *:[f32] }:$rs3))  =>  (FNMADD_S:{ *:[f32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs3, 7:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FNMADD_S,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/2, /*OpIdx*/1, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1102,
        GIR_Done,
      // Label 797: @26274
      GIM_Try, /*On fail goto*//*Label 798*/ 26343, // Rule ID 1103 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR32RegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/0, /*OpIdx*/3, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/2, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/2, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (fma:{ *:[f32] } (fneg:{ *:[f32] } FPR32:{ *:[f32] }:$rs1), FPR32:{ *:[f32] }:$rs2, (fneg:{ *:[f32] } FPR32:{ *:[f32] }:$rs3))  =>  (FNMADD_S:{ *:[f32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs3, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FNMADD_S,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/2, /*OpIdx*/1, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1103,
        GIR_Done,
      // Label 798: @26343
      GIM_Try, /*On fail goto*//*Label 799*/ 26399, // Rule ID 1098 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (fma:{ *:[f32] } (fneg:{ *:[f32] } FPR32:{ *:[f32] }:$rs1), FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs3)  =>  (FNMSUB_S:{ *:[f32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs3, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FNMSUB_S,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1098,
        GIR_Done,
      // Label 799: @26399
      GIM_Try, /*On fail goto*//*Label 800*/ 26455, // Rule ID 1099 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (fma:{ *:[f32] } (fneg:{ *:[f32] } FPR32:{ *:[f32] }:$rs1), FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs3)  =>  (FNMSUB_S:{ *:[f32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs3, 7:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FNMSUB_S,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1099,
        GIR_Done,
      // Label 800: @26455
      GIM_Try, /*On fail goto*//*Label 801*/ 26511, // Rule ID 1100 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (fma:{ *:[f32] } (fneg:{ *:[f32] } FPR32:{ *:[f32] }:$rs1), FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs3)  =>  (FNMSUB_S:{ *:[f32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs3, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FNMSUB_S,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1100,
        GIR_Done,
      // Label 801: @26511
      GIM_Try, /*On fail goto*//*Label 802*/ 26567, // Rule ID 1095 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR32RegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (fma:{ *:[f32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, (fneg:{ *:[f32] } FPR32:{ *:[f32] }:$rs3))  =>  (FMSUB_S:{ *:[f32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs3, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FMSUB_S,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1095,
        GIR_Done,
      // Label 802: @26567
      GIM_Try, /*On fail goto*//*Label 803*/ 26623, // Rule ID 1096 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR32RegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (fma:{ *:[f32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, (fneg:{ *:[f32] } FPR32:{ *:[f32] }:$rs3))  =>  (FMSUB_S:{ *:[f32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs3, 7:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FMSUB_S,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1096,
        GIR_Done,
      // Label 803: @26623
      GIM_Try, /*On fail goto*//*Label 804*/ 26679, // Rule ID 1097 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR32RegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (fma:{ *:[f32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, (fneg:{ *:[f32] } FPR32:{ *:[f32] }:$rs3))  =>  (FMSUB_S:{ *:[f32] } FPR32:{ *:[f32] }:$rs1, FPR32:{ *:[f32] }:$rs2, FPR32:{ *:[f32] }:$rs3, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FMSUB_S,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1097,
        GIR_Done,
      // Label 804: @26679
      GIM_Reject,
    // Label 795: @26680
    GIM_Reject,
    // Label 793: @26681
    GIM_Try, /*On fail goto*//*Label 805*/ 27243,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/2, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/3, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
      GIM_Try, /*On fail goto*//*Label 806*/ 26768, // Rule ID 1244 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR64RegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/0, /*OpIdx*/3, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/2, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/2, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (fma:{ *:[f64] } (fneg:{ *:[f64] } FPR64:{ *:[f64] }:$rs1), FPR64:{ *:[f64] }:$rs2, (fneg:{ *:[f64] } FPR64:{ *:[f64] }:$rs3))  =>  (FNMADD_D:{ *:[f64] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs3, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FNMADD_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/2, /*OpIdx*/1, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1244,
        GIR_Done,
      // Label 806: @26768
      GIM_Try, /*On fail goto*//*Label 807*/ 26837, // Rule ID 1245 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR64RegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/0, /*OpIdx*/3, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/2, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/2, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (fma:{ *:[f64] } (fneg:{ *:[f64] } FPR64:{ *:[f64] }:$rs1), FPR64:{ *:[f64] }:$rs2, (fneg:{ *:[f64] } FPR64:{ *:[f64] }:$rs3))  =>  (FNMADD_D:{ *:[f64] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs3, 7:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FNMADD_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/2, /*OpIdx*/1, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1245,
        GIR_Done,
      // Label 807: @26837
      GIM_Try, /*On fail goto*//*Label 808*/ 26906, // Rule ID 1246 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR64RegClassID,
        GIM_RecordInsn, /*DefineMI*/2, /*MI*/0, /*OpIdx*/3, // MIs[2]
        GIM_CheckOpcode, /*MI*/2, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/2, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/2, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        GIM_CheckIsSafeToFold, /*InsnID*/2,
        // (fma:{ *:[f64] } (fneg:{ *:[f64] } FPR64:{ *:[f64] }:$rs1), FPR64:{ *:[f64] }:$rs2, (fneg:{ *:[f64] } FPR64:{ *:[f64] }:$rs3))  =>  (FNMADD_D:{ *:[f64] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs3, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FNMADD_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/2, /*OpIdx*/1, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1246,
        GIR_Done,
      // Label 808: @26906
      GIM_Try, /*On fail goto*//*Label 809*/ 26962, // Rule ID 1241 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (fma:{ *:[f64] } (fneg:{ *:[f64] } FPR64:{ *:[f64] }:$rs1), FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs3)  =>  (FNMSUB_D:{ *:[f64] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs3, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FNMSUB_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1241,
        GIR_Done,
      // Label 809: @26962
      GIM_Try, /*On fail goto*//*Label 810*/ 27018, // Rule ID 1242 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (fma:{ *:[f64] } (fneg:{ *:[f64] } FPR64:{ *:[f64] }:$rs1), FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs3)  =>  (FNMSUB_D:{ *:[f64] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs3, 7:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FNMSUB_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1242,
        GIR_Done,
      // Label 810: @27018
      GIM_Try, /*On fail goto*//*Label 811*/ 27074, // Rule ID 1243 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/1, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/3, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (fma:{ *:[f64] } (fneg:{ *:[f64] } FPR64:{ *:[f64] }:$rs1), FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs3)  =>  (FNMSUB_D:{ *:[f64] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs3, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FNMSUB_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/3, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1243,
        GIR_Done,
      // Label 811: @27074
      GIM_Try, /*On fail goto*//*Label 812*/ 27130, // Rule ID 1238 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR64RegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (fma:{ *:[f64] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, (fneg:{ *:[f64] } FPR64:{ *:[f64] }:$rs3))  =>  (FMSUB_D:{ *:[f64] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs3, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FMSUB_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1238,
        GIR_Done,
      // Label 812: @27130
      GIM_Try, /*On fail goto*//*Label 813*/ 27186, // Rule ID 1239 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR64RegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (fma:{ *:[f64] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, (fneg:{ *:[f64] } FPR64:{ *:[f64] }:$rs3))  =>  (FMSUB_D:{ *:[f64] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs3, 7:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FMSUB_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1239,
        GIR_Done,
      // Label 813: @27186
      GIM_Try, /*On fail goto*//*Label 814*/ 27242, // Rule ID 1240 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/2, /*RC*/RISCV::FPR64RegClassID,
        GIM_RecordInsn, /*DefineMI*/1, /*MI*/0, /*OpIdx*/3, // MIs[1]
        GIM_CheckOpcode, /*MI*/1, TargetOpcode::G_FNEG,
        GIM_CheckType, /*MI*/1, /*Op*/1, /*Type*/GILLT_s64,
        GIM_CheckRegBankForClass, /*MI*/1, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckIsSafeToFold, /*InsnID*/1,
        // (fma:{ *:[f64] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, (fneg:{ *:[f64] } FPR64:{ *:[f64] }:$rs3))  =>  (FMSUB_D:{ *:[f64] } FPR64:{ *:[f64] }:$rs1, FPR64:{ *:[f64] }:$rs2, FPR64:{ *:[f64] }:$rs3, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FMSUB_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/2, // rs2
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/1, /*OpIdx*/1, // rs3
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1240,
        GIR_Done,
      // Label 814: @27242
      GIM_Reject,
    // Label 805: @27243
    GIM_Reject,
    // Label 794: @27244
    GIM_Reject,
    // Label 36: @27245
    GIM_Try, /*On fail goto*//*Label 815*/ 27272, // Rule ID 1211 //
      GIM_CheckFeatures, GIFBS_HasStdExtD,
      GIM_CheckType, /*MI*/0, /*Op*/0, /*Type*/GILLT_s64,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
      // (fpextend:{ *:[f64] } FPR32:{ *:[f32] }:$rs1)  =>  (FCVT_D_S:{ *:[f64] } FPR32:{ *:[f32] }:$rs1)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FCVT_D_S,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 1211,
      GIR_Done,
    // Label 815: @27272
    GIM_Reject,
    // Label 37: @27273
    GIM_Try, /*On fail goto*//*Label 816*/ 27361,
      GIM_CheckType, /*MI*/0, /*Op*/0, /*Type*/GILLT_s32,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
      GIM_Try, /*On fail goto*//*Label 817*/ 27314, // Rule ID 1208 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        // (fpround:{ *:[f32] } FPR64:{ *:[f64] }:$rs1)  =>  (FCVT_S_D:{ *:[f32] } FPR64:{ *:[f64] }:$rs1, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_S_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1208,
        GIR_Done,
      // Label 817: @27314
      GIM_Try, /*On fail goto*//*Label 818*/ 27337, // Rule ID 1209 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        // (fpround:{ *:[f32] } FPR64:{ *:[f64] }:$rs1)  =>  (FCVT_S_D:{ *:[f32] } FPR64:{ *:[f64] }:$rs1, 7:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_S_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1209,
        GIR_Done,
      // Label 818: @27337
      GIM_Try, /*On fail goto*//*Label 819*/ 27360, // Rule ID 1210 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        // (fpround:{ *:[f32] } FPR64:{ *:[f64] }:$rs1)  =>  (FCVT_S_D:{ *:[f32] } FPR64:{ *:[f64] }:$rs1, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_S_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1210,
        GIR_Done,
      // Label 819: @27360
      GIM_Reject,
    // Label 816: @27361
    GIM_Reject,
    // Label 38: @27362
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 822*/ 27540,
    /*GILLT_s32*//*Label 820*/ 27370,
    /*GILLT_s64*//*Label 821*/ 27478,
    // Label 820: @27370
    GIM_Try, /*On fail goto*//*Label 823*/ 27477,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
      GIM_Try, /*On fail goto*//*Label 824*/ 27407, // Rule ID 1303 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV32,
        // (fp_to_sint:{ *:[i32] } FPR64:{ *:[f64] }:$rs1)  =>  (FCVT_W_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs1, 1:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_W_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1303,
        GIR_Done,
      // Label 824: @27407
      GIM_Try, /*On fail goto*//*Label 825*/ 27430, // Rule ID 1305 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV32,
        // (fp_to_sint:{ *:[i32] } FPR64:{ *:[f64] }:$rs1)  =>  (FCVT_W_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs1, 1:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_W_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1305,
        GIR_Done,
      // Label 825: @27430
      GIM_Try, /*On fail goto*//*Label 826*/ 27453, // Rule ID 1330 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
        // (fp_to_sint:{ *:[i32] } FPR64:{ *:[f64] }:$rs1)  =>  (FCVT_L_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs1, 1:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_L_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1330,
        GIR_Done,
      // Label 826: @27453
      GIM_Try, /*On fail goto*//*Label 827*/ 27476, // Rule ID 1332 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
        // (fp_to_sint:{ *:[i32] } FPR64:{ *:[f64] }:$rs1)  =>  (FCVT_L_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs1, 1:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_L_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1332,
        GIR_Done,
      // Label 827: @27476
      GIM_Reject,
    // Label 823: @27477
    GIM_Reject,
    // Label 821: @27478
    GIM_Try, /*On fail goto*//*Label 828*/ 27539,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
      GIM_Try, /*On fail goto*//*Label 829*/ 27515, // Rule ID 1304 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV32,
        // (fp_to_sint:{ *:[i64] } FPR64:{ *:[f64] }:$rs1)  =>  (FCVT_W_D:{ *:[i64] } FPR64:{ *:[f64] }:$rs1, 1:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_W_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1304,
        GIR_Done,
      // Label 829: @27515
      GIM_Try, /*On fail goto*//*Label 830*/ 27538, // Rule ID 1331 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
        // (fp_to_sint:{ *:[i64] } FPR64:{ *:[f64] }:$rs1)  =>  (FCVT_L_D:{ *:[i64] } FPR64:{ *:[f64] }:$rs1, 1:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_L_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1331,
        GIR_Done,
      // Label 830: @27538
      GIM_Reject,
    // Label 828: @27539
    GIM_Reject,
    // Label 822: @27540
    GIM_Reject,
    // Label 39: @27541
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 833*/ 27719,
    /*GILLT_s32*//*Label 831*/ 27549,
    /*GILLT_s64*//*Label 832*/ 27657,
    // Label 831: @27549
    GIM_Try, /*On fail goto*//*Label 834*/ 27656,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
      GIM_Try, /*On fail goto*//*Label 835*/ 27586, // Rule ID 1306 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV32,
        // (fp_to_uint:{ *:[i32] } FPR64:{ *:[f64] }:$rs1)  =>  (FCVT_WU_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs1, 1:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_WU_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1306,
        GIR_Done,
      // Label 835: @27586
      GIM_Try, /*On fail goto*//*Label 836*/ 27609, // Rule ID 1308 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV32,
        // (fp_to_uint:{ *:[i32] } FPR64:{ *:[f64] }:$rs1)  =>  (FCVT_WU_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs1, 1:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_WU_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1308,
        GIR_Done,
      // Label 836: @27609
      GIM_Try, /*On fail goto*//*Label 837*/ 27632, // Rule ID 1333 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
        // (fp_to_uint:{ *:[i32] } FPR64:{ *:[f64] }:$rs1)  =>  (FCVT_LU_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs1, 1:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_LU_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1333,
        GIR_Done,
      // Label 837: @27632
      GIM_Try, /*On fail goto*//*Label 838*/ 27655, // Rule ID 1335 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
        // (fp_to_uint:{ *:[i32] } FPR64:{ *:[f64] }:$rs1)  =>  (FCVT_LU_D:{ *:[i32] } FPR64:{ *:[f64] }:$rs1, 1:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_LU_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1335,
        GIR_Done,
      // Label 838: @27655
      GIM_Reject,
    // Label 834: @27656
    GIM_Reject,
    // Label 832: @27657
    GIM_Try, /*On fail goto*//*Label 839*/ 27718,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::GPRRegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
      GIM_Try, /*On fail goto*//*Label 840*/ 27694, // Rule ID 1307 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV32,
        // (fp_to_uint:{ *:[i64] } FPR64:{ *:[f64] }:$rs1)  =>  (FCVT_WU_D:{ *:[i64] } FPR64:{ *:[f64] }:$rs1, 1:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_WU_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1307,
        GIR_Done,
      // Label 840: @27694
      GIM_Try, /*On fail goto*//*Label 841*/ 27717, // Rule ID 1334 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
        // (fp_to_uint:{ *:[i64] } FPR64:{ *:[f64] }:$rs1)  =>  (FCVT_LU_D:{ *:[i64] } FPR64:{ *:[f64] }:$rs1, 1:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_LU_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/1,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1334,
        GIR_Done,
      // Label 841: @27717
      GIM_Reject,
    // Label 839: @27718
    GIM_Reject,
    // Label 833: @27719
    GIM_Reject,
    // Label 40: @27720
    GIM_Try, /*On fail goto*//*Label 842*/ 27896,
      GIM_CheckType, /*MI*/0, /*Op*/0, /*Type*/GILLT_s64,
      GIM_SwitchType, /*MI*/0, /*Op*/1, /*[*/0, 2, /*)*//*default:*//*Label 845*/ 27774,
      /*GILLT_s32*//*Label 843*/ 27734,
      /*GILLT_s64*//*Label 844*/ 27754,
      // Label 843: @27734
      GIM_Try, /*On fail goto*//*Label 846*/ 27753, // Rule ID 1309 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (sint_to_fp:{ *:[f64] } GPR:{ *:[i32] }:$rs1)  =>  (FCVT_D_W:{ *:[f64] } GPR:{ *:[i32] }:$rs1)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FCVT_D_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1309,
        GIR_Done,
      // Label 846: @27753
      GIM_Reject,
      // Label 844: @27754
      GIM_Try, /*On fail goto*//*Label 847*/ 27773, // Rule ID 1310 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (sint_to_fp:{ *:[f64] } GPR:{ *:[i64] }:$rs1)  =>  (FCVT_D_W:{ *:[f64] } GPR:{ *:[i64] }:$rs1)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FCVT_D_W,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1310,
        GIR_Done,
      // Label 847: @27773
      GIM_Reject,
      // Label 845: @27774
      GIM_SwitchType, /*MI*/0, /*Op*/1, /*[*/0, 2, /*)*//*default:*//*Label 850*/ 27860,
      /*GILLT_s32*//*Label 848*/ 27782,
      /*GILLT_s64*//*Label 849*/ 27828,
      // Label 848: @27782
      GIM_Try, /*On fail goto*//*Label 851*/ 27827,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_Try, /*On fail goto*//*Label 852*/ 27803, // Rule ID 1311 //
          GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV32,
          // (sint_to_fp:{ *:[f64] } GPR:{ *:[i32] }:$rs1)  =>  (FCVT_D_W:{ *:[f64] } GPR:{ *:[i32] }:$rs1)
          GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FCVT_D_W,
          GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
          // GIR_Coverage, 1311,
          GIR_Done,
        // Label 852: @27803
        GIM_Try, /*On fail goto*//*Label 853*/ 27826, // Rule ID 1336 //
          GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
          // (sint_to_fp:{ *:[f64] } GPR:{ *:[i32] }:$rs1)  =>  (FCVT_D_L:{ *:[f64] } GPR:{ *:[i32] }:$rs1, 7:{ *:[i32] })
          GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_D_L,
          GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
          GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
          GIR_AddImm, /*InsnID*/0, /*Imm*/7,
          GIR_EraseFromParent, /*InsnID*/0,
          GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
          // GIR_Coverage, 1336,
          GIR_Done,
        // Label 853: @27826
        GIM_Reject,
      // Label 851: @27827
      GIM_Reject,
      // Label 849: @27828
      GIM_Try, /*On fail goto*//*Label 854*/ 27859, // Rule ID 1337 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (sint_to_fp:{ *:[f64] } GPR:{ *:[i64] }:$rs1)  =>  (FCVT_D_L:{ *:[f64] } GPR:{ *:[i64] }:$rs1, 7:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_D_L,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1337,
        GIR_Done,
      // Label 854: @27859
      GIM_Reject,
      // Label 850: @27860
      GIM_Try, /*On fail goto*//*Label 855*/ 27895, // Rule ID 1338 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
        GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (sint_to_fp:{ *:[f64] } GPR:{ *:[i32] }:$rs1)  =>  (FCVT_D_L:{ *:[f64] } GPR:{ *:[i32] }:$rs1, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_D_L,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1338,
        GIR_Done,
      // Label 855: @27895
      GIM_Reject,
    // Label 842: @27896
    GIM_Reject,
    // Label 41: @27897
    GIM_Try, /*On fail goto*//*Label 856*/ 28073,
      GIM_CheckType, /*MI*/0, /*Op*/0, /*Type*/GILLT_s64,
      GIM_SwitchType, /*MI*/0, /*Op*/1, /*[*/0, 2, /*)*//*default:*//*Label 859*/ 27951,
      /*GILLT_s32*//*Label 857*/ 27911,
      /*GILLT_s64*//*Label 858*/ 27931,
      // Label 857: @27911
      GIM_Try, /*On fail goto*//*Label 860*/ 27930, // Rule ID 1312 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (uint_to_fp:{ *:[f64] } GPR:{ *:[i32] }:$rs1)  =>  (FCVT_D_WU:{ *:[f64] } GPR:{ *:[i32] }:$rs1)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FCVT_D_WU,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1312,
        GIR_Done,
      // Label 860: @27930
      GIM_Reject,
      // Label 858: @27931
      GIM_Try, /*On fail goto*//*Label 861*/ 27950, // Rule ID 1313 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (uint_to_fp:{ *:[f64] } GPR:{ *:[i64] }:$rs1)  =>  (FCVT_D_WU:{ *:[f64] } GPR:{ *:[i64] }:$rs1)
        GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FCVT_D_WU,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1313,
        GIR_Done,
      // Label 861: @27950
      GIM_Reject,
      // Label 859: @27951
      GIM_SwitchType, /*MI*/0, /*Op*/1, /*[*/0, 2, /*)*//*default:*//*Label 864*/ 28037,
      /*GILLT_s32*//*Label 862*/ 27959,
      /*GILLT_s64*//*Label 863*/ 28005,
      // Label 862: @27959
      GIM_Try, /*On fail goto*//*Label 865*/ 28004,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        GIM_Try, /*On fail goto*//*Label 866*/ 27980, // Rule ID 1314 //
          GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV32,
          // (uint_to_fp:{ *:[f64] } GPR:{ *:[i32] }:$rs1)  =>  (FCVT_D_WU:{ *:[f64] } GPR:{ *:[i32] }:$rs1)
          GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::FCVT_D_WU,
          GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
          // GIR_Coverage, 1314,
          GIR_Done,
        // Label 866: @27980
        GIM_Try, /*On fail goto*//*Label 867*/ 28003, // Rule ID 1339 //
          GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
          // (uint_to_fp:{ *:[f64] } GPR:{ *:[i32] }:$rs1)  =>  (FCVT_D_LU:{ *:[f64] } GPR:{ *:[i32] }:$rs1, 7:{ *:[i32] })
          GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_D_LU,
          GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
          GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
          GIR_AddImm, /*InsnID*/0, /*Imm*/7,
          GIR_EraseFromParent, /*InsnID*/0,
          GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
          // GIR_Coverage, 1339,
          GIR_Done,
        // Label 867: @28003
        GIM_Reject,
      // Label 865: @28004
      GIM_Reject,
      // Label 863: @28005
      GIM_Try, /*On fail goto*//*Label 868*/ 28036, // Rule ID 1340 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (uint_to_fp:{ *:[f64] } GPR:{ *:[i64] }:$rs1)  =>  (FCVT_D_LU:{ *:[f64] } GPR:{ *:[i64] }:$rs1, 7:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_D_LU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1340,
        GIR_Done,
      // Label 868: @28036
      GIM_Reject,
      // Label 864: @28037
      GIM_Try, /*On fail goto*//*Label 869*/ 28072, // Rule ID 1341 //
        GIM_CheckFeatures, GIFBS_HasStdExtD_IsRV64,
        GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
        GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::GPRRegClassID,
        // (uint_to_fp:{ *:[f64] } GPR:{ *:[i32] }:$rs1)  =>  (FCVT_D_LU:{ *:[f64] } GPR:{ *:[i32] }:$rs1, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FCVT_D_LU,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1341,
        GIR_Done,
      // Label 869: @28072
      GIM_Reject,
    // Label 856: @28073
    GIM_Reject,
    // Label 42: @28074
    GIM_Try, /*On fail goto*//*Label 870*/ 28086, // Rule ID 3 //
      // MIs[0] imm20
      GIM_CheckIsMBB, /*MI*/0, /*Op*/0,
      // (br (bb:{ *:[Other] }):$imm20)  =>  (PseudoBR (bb:{ *:[Other] }):$imm20)
      GIR_MutateOpcode, /*InsnID*/0, /*RecycleInsnID*/0, /*Opcode*/RISCV::PseudoBR,
      GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
      // GIR_Coverage, 3,
      GIR_Done,
    // Label 870: @28086
    GIM_Reject,
    // Label 43: @28087
    GIM_SwitchType, /*MI*/0, /*Op*/0, /*[*/0, 2, /*)*//*default:*//*Label 873*/ 28265,
    /*GILLT_s32*//*Label 871*/ 28095,
    /*GILLT_s64*//*Label 872*/ 28180,
    // Label 871: @28095
    GIM_Try, /*On fail goto*//*Label 874*/ 28179,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s32,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR32RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR32RegClassID,
      GIM_Try, /*On fail goto*//*Label 875*/ 28132, // Rule ID 1085 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        // (fsqrt:{ *:[f32] } FPR32:{ *:[f32] }:$rs1)  =>  (FSQRT_S:{ *:[f32] } FPR32:{ *:[f32] }:$rs1, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSQRT_S,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1085,
        GIR_Done,
      // Label 875: @28132
      GIM_Try, /*On fail goto*//*Label 876*/ 28155, // Rule ID 1086 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        // (fsqrt:{ *:[f32] } FPR32:{ *:[f32] }:$rs1)  =>  (FSQRT_S:{ *:[f32] } FPR32:{ *:[f32] }:$rs1, 7:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSQRT_S,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1086,
        GIR_Done,
      // Label 876: @28155
      GIM_Try, /*On fail goto*//*Label 877*/ 28178, // Rule ID 1087 //
        GIM_CheckFeatures, GIFBS_HasStdExtF,
        // (fsqrt:{ *:[f32] } FPR32:{ *:[f32] }:$rs1)  =>  (FSQRT_S:{ *:[f32] } FPR32:{ *:[f32] }:$rs1, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSQRT_S,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1087,
        GIR_Done,
      // Label 877: @28178
      GIM_Reject,
    // Label 874: @28179
    GIM_Reject,
    // Label 872: @28180
    GIM_Try, /*On fail goto*//*Label 878*/ 28264,
      GIM_CheckType, /*MI*/0, /*Op*/1, /*Type*/GILLT_s64,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/0, /*RC*/RISCV::FPR64RegClassID,
      GIM_CheckRegBankForClass, /*MI*/0, /*Op*/1, /*RC*/RISCV::FPR64RegClassID,
      GIM_Try, /*On fail goto*//*Label 879*/ 28217, // Rule ID 1224 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        // (fsqrt:{ *:[f64] } FPR64:{ *:[f64] }:$rs1)  =>  (FSQRT_D:{ *:[f64] } FPR64:{ *:[f64] }:$rs1, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSQRT_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1224,
        GIR_Done,
      // Label 879: @28217
      GIM_Try, /*On fail goto*//*Label 880*/ 28240, // Rule ID 1225 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        // (fsqrt:{ *:[f64] } FPR64:{ *:[f64] }:$rs1)  =>  (FSQRT_D:{ *:[f64] } FPR64:{ *:[f64] }:$rs1, 7:{ *:[i64] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSQRT_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1225,
        GIR_Done,
      // Label 880: @28240
      GIM_Try, /*On fail goto*//*Label 881*/ 28263, // Rule ID 1226 //
        GIM_CheckFeatures, GIFBS_HasStdExtD,
        // (fsqrt:{ *:[f64] } FPR64:{ *:[f64] }:$rs1)  =>  (FSQRT_D:{ *:[f64] } FPR64:{ *:[f64] }:$rs1, 7:{ *:[i32] })
        GIR_BuildMI, /*InsnID*/0, /*Opcode*/RISCV::FSQRT_D,
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/0, // rd
        GIR_Copy, /*NewInsnID*/0, /*OldInsnID*/0, /*OpIdx*/1, // rs1
        GIR_AddImm, /*InsnID*/0, /*Imm*/7,
        GIR_EraseFromParent, /*InsnID*/0,
        GIR_ConstrainSelectedInstOperands, /*InsnID*/0,
        // GIR_Coverage, 1226,
        GIR_Done,
      // Label 881: @28263
      GIM_Reject,
    // Label 878: @28264
    GIM_Reject,
    // Label 873: @28265
    GIM_Reject,
    // Label 44: @28266
    GIM_Reject,
    };
  return MatchTable0;
}
#endif // ifdef GET_GLOBALISEL_IMPL
#ifdef GET_GLOBALISEL_PREDICATES_DECL
PredicateBitset AvailableModuleFeatures;
mutable PredicateBitset AvailableFunctionFeatures;
PredicateBitset getAvailableFeatures() const {
  return AvailableModuleFeatures | AvailableFunctionFeatures;
}
PredicateBitset
computeAvailableModuleFeatures(const RISCVSubtarget *Subtarget) const;
PredicateBitset
computeAvailableFunctionFeatures(const RISCVSubtarget *Subtarget,
                                 const MachineFunction *MF) const;
void setupGeneratedPerFunctionState(MachineFunction &MF) override;
#endif // ifdef GET_GLOBALISEL_PREDICATES_DECL
#ifdef GET_GLOBALISEL_PREDICATES_INIT
AvailableModuleFeatures(computeAvailableModuleFeatures(&STI)),
AvailableFunctionFeatures()
#endif // ifdef GET_GLOBALISEL_PREDICATES_INIT
