/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Compress instruction Source Fragment                                       *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/


#ifdef GEN_COMPRESS_INSTR
#undef GEN_COMPRESS_INSTR

static bool RISCVValidateMCOperandForCompress(const MCOperand &MCOp,
                  const MCSubtargetInfo &STI,
                  unsigned PredicateIndex) {
  switch (PredicateIndex) {
  default:
    llvm_unreachable("Unknown MCOperandPredicate kind");
    break;
  case 1: {
  // uimm10_lsb00nonzero
  
    int64_t Imm;
    if (!MCOp.evaluateAsConstantImm(Imm))
      return false;
    return isShiftedUInt<8, 2>(Imm) && (Imm != 0);
  
  }
  case 2: {
  // simm6nonzero
  
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return (Imm != 0) && isInt<6>(Imm);
    return MCOp.isBareSymbolRef();
  
  }
  case 3: {
  // simm6
  
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isInt<6>(Imm);
    return MCOp.isBareSymbolRef();
  
  }
  case 4: {
  // simm10_lsb0000nonzero
  
    int64_t Imm;
    if (!MCOp.evaluateAsConstantImm(Imm))
      return false;
    return isShiftedInt<6, 4>(Imm) && (Imm != 0);
  
  }
  case 5: {
  // simm9_lsb0
  
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isShiftedInt<8, 1>(Imm);
    return MCOp.isBareSymbolRef();

  
  }
  case 6: {
  // uimm8_lsb000
  
    int64_t Imm;
    if (!MCOp.evaluateAsConstantImm(Imm))
      return false;
    return isShiftedUInt<5, 3>(Imm);
  
  }
  case 7: {
  // uimm9_lsb000
  
    int64_t Imm;
    if (!MCOp.evaluateAsConstantImm(Imm))
      return false;
    return isShiftedUInt<6, 3>(Imm);
  
  }
  case 8: {
  // uimm7_lsb00
  
    int64_t Imm;
    if (!MCOp.evaluateAsConstantImm(Imm))
      return false;
    return isShiftedUInt<5, 2>(Imm);
  
  }
  case 9: {
  // uimm8_lsb00
  
    int64_t Imm;
    if (!MCOp.evaluateAsConstantImm(Imm))
      return false;
    return isShiftedUInt<6, 2>(Imm);
  
  }
  case 10: {
  // simm12_lsb0
  
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isShiftedInt<11, 1>(Imm);
    return MCOp.isBareSymbolRef();
  
  }
  case 11: {
  // c_lui_imm
  
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return (Imm != 0) && (isUInt<5>(Imm) ||
             (Imm >= 0xfffe0 && Imm <= 0xfffff));
    return MCOp.isBareSymbolRef();
  
  }
  case 12: {
  // uimmlog2xlennonzero
  
    int64_t Imm;
    if (!MCOp.evaluateAsConstantImm(Imm))
      return false;
    if (STI.getTargetTriple().isArch64Bit())
      return  isUInt<6>(Imm) && (Imm != 0);
    return isUInt<5>(Imm) && (Imm != 0);
  
  }
  }
}

static bool compressInst(MCInst &OutInst,
                         const MCInst &MI,
                         const MCSubtargetInfo &STI) {
  switch (MI.getOpcode()) {
    default: return false;
    case RISCV::ADD: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) &&
      (MI.getOperand(1).getReg() == RISCV::X0) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(2).getReg()))) {
      // c.mv	$rs1, $rs2
      OutInst.setOpcode(RISCV::C_MV);
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(2).isReg()) &&
      (MI.getOperand(2).getReg() == RISCV::X0) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.mv	$rs1, $rs2
      OutInst.setOpcode(RISCV::C_MV);
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(1));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(2).getReg()))) {
      // c.add	$rs1, $rs2
      OutInst.setOpcode(RISCV::C_ADD);
      // Operand: rs1_wb
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(2).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(2).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(2).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.add	$rs1, $rs2
      OutInst.setOpcode(RISCV::C_ADD);
      // Operand: rs1_wb
      OutInst.addOperand(MI.getOperand(2));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(2));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(1));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case ADD
    case RISCV::ADDI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 1)) {
      // c.addi4spn	$rd, $rs1, $imm
      OutInst.setOpcode(RISCV::C_ADDI4SPN);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (MI.getOperand(0).getReg() == RISCV::X0) &&
      (MI.getOperand(1).isReg()) &&
      (MI.getOperand(1).getReg() == RISCV::X0) &&
      (MI.getOperand(2).isImm()) &&
      (MI.getOperand(2).getImm() == 0)) {
      // c.nop	
      OutInst.setOpcode(RISCV::C_NOP);
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 2)) {
      // c.addi	$rd, $imm
      OutInst.setOpcode(RISCV::C_ADDI);
      // Operand: rd_wb
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rd
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) &&
      (MI.getOperand(1).getReg() == RISCV::X0) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(0).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 3)) {
      // c.li	$rd, $imm
      OutInst.setOpcode(RISCV::C_LI);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (MI.getOperand(0).getReg() == RISCV::X2) &&
      (MI.getOperand(1).isReg()) &&
      (MI.getOperand(1).getReg() == RISCV::X2) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 4)) {
      // c.addi16sp	$rd, $imm
      OutInst.setOpcode(RISCV::C_ADDI16SP);
      // Operand: rd_wb
      OutInst.addOperand(MCOperand::createReg(RISCV::X2));
      // Operand: rd
      OutInst.addOperand(MCOperand::createReg(RISCV::X2));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(2).isImm()) &&
      (MI.getOperand(2).getImm() == 0) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.mv	$rs1, $rs2
      OutInst.setOpcode(RISCV::C_MV);
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(1));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case ADDI
    case RISCV::ADDIW: {
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 3)) {
      // c.addiw	$rd, $imm
      OutInst.setOpcode(RISCV::C_ADDIW);
      // Operand: rd_wb
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rd
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) &&
      (MI.getOperand(1).getReg() == RISCV::X0) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(0).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 3)) {
      // c.li	$rd, $imm
      OutInst.setOpcode(RISCV::C_LI);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case ADDIW
    case RISCV::ADDW: {
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg()))) {
      // c.addw	$rd, $rs2
      OutInst.setOpcode(RISCV::C_ADDW);
      // Operand: rd_wb
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rd
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(2).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(2).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.addw	$rd, $rs2
      OutInst.setOpcode(RISCV::C_ADDW);
      // Operand: rd_wb
      OutInst.addOperand(MI.getOperand(2));
      // Operand: rd
      OutInst.addOperand(MI.getOperand(2));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(1));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case ADDW
    case RISCV::AND: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg()))) {
      // c.and	$rd, $rs2
      OutInst.setOpcode(RISCV::C_AND);
      // Operand: rd_wb
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rd
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(2).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(2).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.and	$rd, $rs2
      OutInst.setOpcode(RISCV::C_AND);
      // Operand: rd_wb
      OutInst.addOperand(MI.getOperand(2));
      // Operand: rd
      OutInst.addOperand(MI.getOperand(2));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(1));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case AND
    case RISCV::ANDI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 3)) {
      // c.andi	$rs1, $imm
      OutInst.setOpcode(RISCV::C_ANDI);
      // Operand: rs1_wb
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case ANDI
    case RISCV::BEQ: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) &&
      (MI.getOperand(1).getReg() == RISCV::X0) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(0).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 5)) {
      // c.beqz	$rs1, $imm
      OutInst.setOpcode(RISCV::C_BEQZ);
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case BEQ
    case RISCV::BNE: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) &&
      (MI.getOperand(1).getReg() == RISCV::X0) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(0).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 5)) {
      // c.bnez	$rs1, $imm
      OutInst.setOpcode(RISCV::C_BNEZ);
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case BNE
    case RISCV::EBREAK: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC])) {
      // c.ebreak	
      OutInst.setOpcode(RISCV::C_EBREAK);
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case EBREAK
    case RISCV::FLD: {
    if (STI.getFeatureBits()[RISCV::FeatureStdExtD] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcd] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR64CRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 6)) {
      // c.fld	$rd, ${imm}(${rs1})
      OutInst.setOpcode(RISCV::C_FLD);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if (STI.getFeatureBits()[RISCV::FeatureStdExtD] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcd] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR64RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 7)) {
      // c.fldsp	$rd, ${imm}(${rs1})
      OutInst.setOpcode(RISCV::C_FLDSP);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case FLD
    case RISCV::FLW: {
    if (STI.getFeatureBits()[RISCV::FeatureStdExtF] &&
      !STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcf] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR32CRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 8)) {
      // c.flw	$rd, ${imm}(${rs1})
      OutInst.setOpcode(RISCV::C_FLW);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if (STI.getFeatureBits()[RISCV::FeatureStdExtF] &&
      !STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcf] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR32RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 9)) {
      // c.flwsp	$rd, ${imm}(${rs1})
      OutInst.setOpcode(RISCV::C_FLWSP);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case FLW
    case RISCV::FSD: {
    if (STI.getFeatureBits()[RISCV::FeatureStdExtD] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcd] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR64CRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 6)) {
      // c.fsd	$rs2, ${imm}(${rs1})
      OutInst.setOpcode(RISCV::C_FSD);
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if (STI.getFeatureBits()[RISCV::FeatureStdExtD] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcd] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR64RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 7)) {
      // c.fsdsp	$rs2, ${imm}(${rs1})
      OutInst.setOpcode(RISCV::C_FSDSP);
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case FSD
    case RISCV::FSW: {
    if (STI.getFeatureBits()[RISCV::FeatureStdExtF] &&
      !STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcf] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR32CRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 8)) {
      // c.fsw	$rs2, ${imm}(${rs1})
      OutInst.setOpcode(RISCV::C_FSW);
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if (STI.getFeatureBits()[RISCV::FeatureStdExtF] &&
      !STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcf] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR32RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 9)) {
      // c.fswsp	$rs2, ${imm}(${rs1})
      OutInst.setOpcode(RISCV::C_FSWSP);
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case FSW
    case RISCV::JAL: {
    if (!STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (MI.getOperand(0).getReg() == RISCV::X1) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(1), STI, 10)) {
      // c.jal	$offset
      OutInst.setOpcode(RISCV::C_JAL);
      // Operand: offset
      OutInst.addOperand(MI.getOperand(1));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (MI.getOperand(0).getReg() == RISCV::X0) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(1), STI, 10)) {
      // c.j	$offset
      OutInst.setOpcode(RISCV::C_J);
      // Operand: offset
      OutInst.addOperand(MI.getOperand(1));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case JAL
    case RISCV::JALR: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (MI.getOperand(0).getReg() == RISCV::X0) &&
      (MI.getOperand(2).isImm()) &&
      (MI.getOperand(2).getImm() == 0) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.jr	$rs1
      OutInst.setOpcode(RISCV::C_JR);
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (MI.getOperand(0).getReg() == RISCV::X1) &&
      (MI.getOperand(2).isImm()) &&
      (MI.getOperand(2).getImm() == 0) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.jalr	$rs1
      OutInst.setOpcode(RISCV::C_JALR);
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case JALR
    case RISCV::LD: {
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 6)) {
      // c.ld	$rd, ${imm}(${rs1})
      OutInst.setOpcode(RISCV::C_LD);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 7)) {
      // c.ldsp	$rd, ${imm}(${rs1})
      OutInst.setOpcode(RISCV::C_LDSP);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case LD
    case RISCV::LUI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0X2RegClassID].contains(MI.getOperand(0).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(1), STI, 11)) {
      // c.lui	$rd, $imm
      OutInst.setOpcode(RISCV::C_LUI);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(1));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case LUI
    case RISCV::LW: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 8)) {
      // c.lw	$rd, ${imm}(${rs1})
      OutInst.setOpcode(RISCV::C_LW);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 9)) {
      // c.lwsp	$rd, ${imm}(${rs1})
      OutInst.setOpcode(RISCV::C_LWSP);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case LW
    case RISCV::OR: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg()))) {
      // c.or	$rd, $rs2
      OutInst.setOpcode(RISCV::C_OR);
      // Operand: rd_wb
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rd
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(2).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(2).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.or	$rd, $rs2
      OutInst.setOpcode(RISCV::C_OR);
      // Operand: rd_wb
      OutInst.addOperand(MI.getOperand(2));
      // Operand: rd
      OutInst.addOperand(MI.getOperand(2));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(1));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case OR
    case RISCV::SD: {
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 6)) {
      // c.sd	$rs2, ${imm}(${rs1})
      OutInst.setOpcode(RISCV::C_SD);
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 7)) {
      // c.sdsp	$rs2, ${imm}(${rs1})
      OutInst.setOpcode(RISCV::C_SDSP);
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case SD
    case RISCV::SLLI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 12)) {
      // c.slli	$rd, $imm
      OutInst.setOpcode(RISCV::C_SLLI);
      // Operand: rd_wb
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rd
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case SLLI
    case RISCV::SRAI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 12)) {
      // c.srai	$rs1, $imm
      OutInst.setOpcode(RISCV::C_SRAI);
      // Operand: rs1_wb
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case SRAI
    case RISCV::SRLI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 12)) {
      // c.srli	$rs1, $imm
      OutInst.setOpcode(RISCV::C_SRLI);
      // Operand: rs1_wb
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case SRLI
    case RISCV::SUB: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg()))) {
      // c.sub	$rd, $rs2
      OutInst.setOpcode(RISCV::C_SUB);
      // Operand: rd_wb
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rd
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case SUB
    case RISCV::SUBW: {
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg()))) {
      // c.subw	$rd, $rs2
      OutInst.setOpcode(RISCV::C_SUBW);
      // Operand: rd_wb
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rd
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case SUBW
    case RISCV::SW: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 8)) {
      // c.sw	$rs2, ${imm}(${rs1})
      OutInst.setOpcode(RISCV::C_SW);
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForCompress(MI.getOperand(2), STI, 9)) {
      // c.swsp	$rs2, ${imm}(${rs1})
      OutInst.setOpcode(RISCV::C_SWSP);
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case SW
    case RISCV::UNIMP: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC])) {
      // c.unimp	
      OutInst.setOpcode(RISCV::C_UNIMP);
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case UNIMP
    case RISCV::XOR: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg()))) {
      // c.xor	$rd, $rs2
      OutInst.setOpcode(RISCV::C_XOR);
      // Operand: rd_wb
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rd
      OutInst.addOperand(MI.getOperand(1));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(2).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(2).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.xor	$rd, $rs2
      OutInst.setOpcode(RISCV::C_XOR);
      // Operand: rd_wb
      OutInst.addOperand(MI.getOperand(2));
      // Operand: rd
      OutInst.addOperand(MI.getOperand(2));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(1));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if

    } // case XOR
  } // switch
  return false;
}

#endif //GEN_COMPRESS_INSTR

#ifdef GEN_UNCOMPRESS_INSTR
#undef GEN_UNCOMPRESS_INSTR

static bool RISCVValidateMCOperandForUncompress(const MCOperand &MCOp,
                  const MCSubtargetInfo &STI,
                  unsigned PredicateIndex) {
  switch (PredicateIndex) {
  default:
    llvm_unreachable("Unknown MCOperandPredicate kind");
    break;
  case 1: {
  // simm12
  
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isInt<12>(Imm);
    return MCOp.isBareSymbolRef();
  
  }
  case 2: {
  // simm13_lsb0
  
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isShiftedInt<12, 1>(Imm);
    return MCOp.isBareSymbolRef();
  
  }
  case 3: {
  // simm21_lsb0_jal
  
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isShiftedInt<20, 1>(Imm);
    return MCOp.isBareSymbolRef();
  
  }
  case 4: {
  // uimm20_lui
  
    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isUInt<20>(Imm);
    return MCOp.isBareSymbolRef();
  
  }
  case 5: {
  // uimmlog2xlen
  
    int64_t Imm;
    if (!MCOp.evaluateAsConstantImm(Imm))
      return false;
    if (STI.getTargetTriple().isArch64Bit())
      return isUInt<6>(Imm);
    return isUInt<5>(Imm);
  
  }
  }
}

static bool uncompressInst(MCInst &OutInst,
                           const MCInst &MI,
                           const MCSubtargetInfo &STI) {
  switch (MI.getOpcode()) {
    default: return false;
    case RISCV::C_ADD: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(2).getReg()))) {
      // add	$rd, $rs1, $rs2
      OutInst.setOpcode(RISCV::ADD);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_ADD
    case RISCV::C_ADDI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // addi	$rd, $rs1, $imm12
      OutInst.setOpcode(RISCV::ADDI);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_ADDI
    case RISCV::C_ADDI16SP: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (MI.getOperand(0).getReg() == RISCV::X2) &&
      (MI.getOperand(1).isReg()) &&
      (MI.getOperand(1).getReg() == RISCV::X2) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // addi	$rd, $rs1, $imm12
      OutInst.setOpcode(RISCV::ADDI);
      // Operand: rd
      OutInst.addOperand(MCOperand::createReg(RISCV::X2));
      // Operand: rs1
      OutInst.addOperand(MCOperand::createReg(RISCV::X2));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_ADDI16SP
    case RISCV::C_ADDI4SPN: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // addi	$rd, $rs1, $imm12
      OutInst.setOpcode(RISCV::ADDI);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_ADDI4SPN
    case RISCV::C_ADDIW: {
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // addiw	$rd, $rs1, $imm12
      OutInst.setOpcode(RISCV::ADDIW);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_ADDIW
    case RISCV::C_ADDW: {
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(2).getReg()))) {
      // addw	$rd, $rs1, $rs2
      OutInst.setOpcode(RISCV::ADDW);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_ADDW
    case RISCV::C_AND: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(2).getReg()))) {
      // and	$rd, $rs1, $rs2
      OutInst.setOpcode(RISCV::AND);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_AND
    case RISCV::C_ANDI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // andi	$rd, $rs1, $imm12
      OutInst.setOpcode(RISCV::ANDI);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_ANDI
    case RISCV::C_BEQZ: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(1), STI, 2)) {
      // beq	$rs1, $rs2, $imm12
      OutInst.setOpcode(RISCV::BEQ);
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs2
      OutInst.addOperand(MCOperand::createReg(RISCV::X0));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(1));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_BEQZ
    case RISCV::C_BNEZ: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(1), STI, 2)) {
      // bne	$rs1, $rs2, $imm12
      OutInst.setOpcode(RISCV::BNE);
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs2
      OutInst.addOperand(MCOperand::createReg(RISCV::X0));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(1));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_BNEZ
    case RISCV::C_EBREAK: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC])) {
      // ebreak	
      OutInst.setOpcode(RISCV::EBREAK);
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_EBREAK
    case RISCV::C_FLD: {
    if (STI.getFeatureBits()[RISCV::FeatureStdExtD] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcd] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR64RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // fld	$rd, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::FLD);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_FLD
    case RISCV::C_FLDSP: {
    if (STI.getFeatureBits()[RISCV::FeatureStdExtD] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcd] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR64RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // fld	$rd, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::FLD);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_FLDSP
    case RISCV::C_FLW: {
    if (STI.getFeatureBits()[RISCV::FeatureStdExtF] &&
      !STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcf] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR32RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // flw	$rd, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::FLW);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_FLW
    case RISCV::C_FLWSP: {
    if (STI.getFeatureBits()[RISCV::FeatureStdExtF] &&
      !STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcf] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR32RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // flw	$rd, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::FLW);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_FLWSP
    case RISCV::C_FSD: {
    if (STI.getFeatureBits()[RISCV::FeatureStdExtD] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcd] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR64RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // fsd	$rs2, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::FSD);
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_FSD
    case RISCV::C_FSDSP: {
    if (STI.getFeatureBits()[RISCV::FeatureStdExtD] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcd] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR64RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // fsd	$rs2, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::FSD);
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_FSDSP
    case RISCV::C_FSW: {
    if (STI.getFeatureBits()[RISCV::FeatureStdExtF] &&
      !STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcf] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR32RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // fsw	$rs2, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::FSW);
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_FSW
    case RISCV::C_FSWSP: {
    if (STI.getFeatureBits()[RISCV::FeatureStdExtF] &&
      !STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcf] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR32RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // fsw	$rs2, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::FSW);
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_FSWSP
    case RISCV::C_J: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(0), STI, 3)) {
      // jal	$rd, $imm20
      OutInst.setOpcode(RISCV::JAL);
      // Operand: rd
      OutInst.addOperand(MCOperand::createReg(RISCV::X0));
      // Operand: imm20
      OutInst.addOperand(MI.getOperand(0));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_J
    case RISCV::C_JAL: {
    if (!STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(0), STI, 3)) {
      // jal	$rd, $imm20
      OutInst.setOpcode(RISCV::JAL);
      // Operand: rd
      OutInst.addOperand(MCOperand::createReg(RISCV::X1));
      // Operand: imm20
      OutInst.addOperand(MI.getOperand(0));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_JAL
    case RISCV::C_JALR: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      RISCVValidateMCOperandForUncompress(MCOperand::createImm(0), STI, 1)) {
      // jalr	$rd, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::JALR);
      // Operand: rd
      OutInst.addOperand(MCOperand::createReg(RISCV::X1));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: imm12
      OutInst.addOperand(MCOperand::createImm(0));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_JALR
    case RISCV::C_JR: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      RISCVValidateMCOperandForUncompress(MCOperand::createImm(0), STI, 1)) {
      // jalr	$rd, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::JALR);
      // Operand: rd
      OutInst.addOperand(MCOperand::createReg(RISCV::X0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: imm12
      OutInst.addOperand(MCOperand::createImm(0));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_JR
    case RISCV::C_LD: {
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // ld	$rd, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::LD);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_LD
    case RISCV::C_LDSP: {
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // ld	$rd, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::LD);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_LDSP
    case RISCV::C_LI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(1), STI, 1)) {
      // addi	$rd, $rs1, $imm12
      OutInst.setOpcode(RISCV::ADDI);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MCOperand::createReg(RISCV::X0));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(1));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_LI
    case RISCV::C_LUI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(1), STI, 4)) {
      // lui	$rd, $imm20
      OutInst.setOpcode(RISCV::LUI);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: imm20
      OutInst.addOperand(MI.getOperand(1));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_LUI
    case RISCV::C_LW: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // lw	$rd, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::LW);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_LW
    case RISCV::C_LWSP: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // lw	$rd, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::LW);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_LWSP
    case RISCV::C_MV: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MCOperand::createImm(0), STI, 1)) {
      // addi	$rd, $rs1, $imm12
      OutInst.setOpcode(RISCV::ADDI);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MCOperand::createImm(0));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_MV
    case RISCV::C_NOP: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      RISCVValidateMCOperandForUncompress(MCOperand::createImm(0), STI, 1)) {
      // addi	$rd, $rs1, $imm12
      OutInst.setOpcode(RISCV::ADDI);
      // Operand: rd
      OutInst.addOperand(MCOperand::createReg(RISCV::X0));
      // Operand: rs1
      OutInst.addOperand(MCOperand::createReg(RISCV::X0));
      // Operand: imm12
      OutInst.addOperand(MCOperand::createImm(0));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_NOP
    case RISCV::C_OR: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(2).getReg()))) {
      // or	$rd, $rs1, $rs2
      OutInst.setOpcode(RISCV::OR);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_OR
    case RISCV::C_SD: {
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // sd	$rs2, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::SD);
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_SD
    case RISCV::C_SDSP: {
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // sd	$rs2, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::SD);
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_SDSP
    case RISCV::C_SLLI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 5)) {
      // slli	$rd, $rs1, $shamt
      OutInst.setOpcode(RISCV::SLLI);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: shamt
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_SLLI
    case RISCV::C_SRAI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 5)) {
      // srai	$rd, $rs1, $shamt
      OutInst.setOpcode(RISCV::SRAI);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: shamt
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_SRAI
    case RISCV::C_SRLI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 5)) {
      // srli	$rd, $rs1, $shamt
      OutInst.setOpcode(RISCV::SRLI);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: shamt
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_SRLI
    case RISCV::C_SUB: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(2).getReg()))) {
      // sub	$rd, $rs1, $rs2
      OutInst.setOpcode(RISCV::SUB);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_SUB
    case RISCV::C_SUBW: {
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(2).getReg()))) {
      // subw	$rd, $rs1, $rs2
      OutInst.setOpcode(RISCV::SUBW);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_SUBW
    case RISCV::C_SW: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // sw	$rs2, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::SW);
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_SW
    case RISCV::C_SWSP: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(1).getReg())) &&
      RISCVValidateMCOperandForUncompress(MI.getOperand(2), STI, 1)) {
      // sw	$rs2, ${imm12}(${rs1})
      OutInst.setOpcode(RISCV::SW);
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(1));
      // Operand: imm12
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_SWSP
    case RISCV::C_UNIMP: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC])) {
      // unimp	
      OutInst.setOpcode(RISCV::UNIMP);
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if
      break;
    } // case C_UNIMP
    case RISCV::C_XOR: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(2).getReg()))) {
      // xor	$rd, $rs1, $rs2
      OutInst.setOpcode(RISCV::XOR);
      // Operand: rd
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs1
      OutInst.addOperand(MI.getOperand(0));
      // Operand: rs2
      OutInst.addOperand(MI.getOperand(2));
      OutInst.setLoc(MI.getLoc());
      return true;
    } // if

    } // case C_XOR
  } // switch
  return false;
}

#endif //GEN_UNCOMPRESS_INSTR


#ifdef GEN_CHECK_COMPRESS_INSTR
#undef GEN_CHECK_COMPRESS_INSTR

static bool RISCVValidateMachineOperand(const MachineOperand &MO,
                  const RISCVSubtarget *Subtarget,
                  unsigned PredicateIndex) {
  int64_t Imm = MO.getImm();
  switch (PredicateIndex) {
  default:
    llvm_unreachable("Unknown ImmLeaf Predicate kind");
    break;
  case 1: {
  // simm6nonzero
  return (Imm != 0) && isInt<6>(Imm);
  }
  case 2: {
  // simm10_lsb0000nonzero
  return (Imm != 0) && isShiftedInt<6, 4>(Imm);
  }
  case 3: {
  // uimm10_lsb00nonzero
  return isShiftedUInt<8, 2>(Imm) && (Imm != 0);
  }
  case 4: {
  // simm6
  return isInt<6>(Imm);
  }
  case 5: {
  // simm9_lsb0
  return isShiftedInt<8, 1>(Imm);
  }
  case 6: {
  // uimm8_lsb000
  return isShiftedUInt<5, 3>(Imm);
  }
  case 7: {
  // uimm9_lsb000
  return isShiftedUInt<6, 3>(Imm);
  }
  case 8: {
  // uimm7_lsb00
  return isShiftedUInt<5, 2>(Imm);
  }
  case 9: {
  // uimm8_lsb00
  return isShiftedUInt<6, 2>(Imm);
  }
  case 10: {
  // simm12_lsb0
  return isShiftedInt<11, 1>(Imm);
  }
  case 11: {
  // c_lui_imm
  return (Imm != 0) &&
                                 (isUInt<5>(Imm) ||
                                  (Imm >= 0xfffe0 && Imm <= 0xfffff));
  }
  case 12: {
  // uimmlog2xlennonzero
  
  if (Subtarget->is64Bit())
    return isUInt<6>(Imm) && (Imm != 0);
  return isUInt<5>(Imm) && (Imm != 0);

  }
  }
}

static bool isCompressibleInst(const MachineInstr &MI,
                               const RISCVSubtarget &STI) {
  switch (MI.getOpcode()) {
    default: return false;
    case RISCV::ADD: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(2).getReg()))) {
      // c.add	$rs1, $rs2
      // Operand: rs1_wb
      // Operand: rs1
      // Operand: rs2
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(2).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(2).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(2).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.add	$rs1, $rs2
      // Operand: rs1_wb
      // Operand: rs1
      // Operand: rs2
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) &&
      (MI.getOperand(1).getReg() == RISCV::X0) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(2).getReg()))) {
      // c.mv	$rs1, $rs2
      // Operand: rs1
      // Operand: rs2
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(2).isReg()) &&
      (MI.getOperand(2).getReg() == RISCV::X0) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.mv	$rs1, $rs2
      // Operand: rs1
      // Operand: rs2
      return true;
    } // if
      break;
    } // case ADD
    case RISCV::ADDI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 1)) {
      // c.addi	$rd, $imm
      // Operand: rd_wb
      // Operand: rd
      // Operand: imm
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (MI.getOperand(0).getReg() == RISCV::X2) &&
      (MI.getOperand(1).isReg()) &&
      (MI.getOperand(1).getReg() == RISCV::X2) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 2)) {
      // c.addi16sp	$rd, $imm
      // Operand: rd_wb
      // Operand: rd
      // Operand: imm
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 3)) {
      // c.addi4spn	$rd, $rs1, $imm
      // Operand: rd
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) &&
      (MI.getOperand(1).getReg() == RISCV::X0) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(0).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 4)) {
      // c.li	$rd, $imm
      // Operand: rd
      // Operand: imm
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(2).isImm()) &&
      (MI.getOperand(2).getImm() == 0) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.mv	$rs1, $rs2
      // Operand: rs1
      // Operand: rs2
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (MI.getOperand(0).getReg() == RISCV::X0) &&
      (MI.getOperand(1).isReg()) &&
      (MI.getOperand(1).getReg() == RISCV::X0) &&
      (MI.getOperand(2).isImm()) &&
      (MI.getOperand(2).getImm() == 0)) {
      // c.nop	
      return true;
    } // if
      break;
    } // case ADDI
    case RISCV::ADDIW: {
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 4)) {
      // c.addiw	$rd, $imm
      // Operand: rd_wb
      // Operand: rd
      // Operand: imm
      return true;
    } // if
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) &&
      (MI.getOperand(1).getReg() == RISCV::X0) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(0).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 4)) {
      // c.li	$rd, $imm
      // Operand: rd
      // Operand: imm
      return true;
    } // if
      break;
    } // case ADDIW
    case RISCV::ADDW: {
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg()))) {
      // c.addw	$rd, $rs2
      // Operand: rd_wb
      // Operand: rd
      // Operand: rs2
      return true;
    } // if
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(2).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(2).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.addw	$rd, $rs2
      // Operand: rd_wb
      // Operand: rd
      // Operand: rs2
      return true;
    } // if
      break;
    } // case ADDW
    case RISCV::AND: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg()))) {
      // c.and	$rd, $rs2
      // Operand: rd_wb
      // Operand: rd
      // Operand: rs2
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(2).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(2).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.and	$rd, $rs2
      // Operand: rd_wb
      // Operand: rd
      // Operand: rs2
      return true;
    } // if
      break;
    } // case AND
    case RISCV::ANDI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 4)) {
      // c.andi	$rs1, $imm
      // Operand: rs1_wb
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
      break;
    } // case ANDI
    case RISCV::BEQ: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) &&
      (MI.getOperand(1).getReg() == RISCV::X0) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(0).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 5)) {
      // c.beqz	$rs1, $imm
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
      break;
    } // case BEQ
    case RISCV::BNE: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) &&
      (MI.getOperand(1).getReg() == RISCV::X0) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(0).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 5)) {
      // c.bnez	$rs1, $imm
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
      break;
    } // case BNE
    case RISCV::EBREAK: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC])) {
      // c.ebreak	
      return true;
    } // if
      break;
    } // case EBREAK
    case RISCV::FLD: {
    if (STI.getFeatureBits()[RISCV::FeatureStdExtD] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcd] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR64CRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 6)) {
      // c.fld	$rd, ${imm}(${rs1})
      // Operand: rd
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
    if (STI.getFeatureBits()[RISCV::FeatureStdExtD] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcd] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR64RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 7)) {
      // c.fldsp	$rd, ${imm}(${rs1})
      // Operand: rd
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
      break;
    } // case FLD
    case RISCV::FLW: {
    if (STI.getFeatureBits()[RISCV::FeatureStdExtF] &&
      !STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcf] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR32CRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 8)) {
      // c.flw	$rd, ${imm}(${rs1})
      // Operand: rd
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
    if (STI.getFeatureBits()[RISCV::FeatureStdExtF] &&
      !STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcf] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR32RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 9)) {
      // c.flwsp	$rd, ${imm}(${rs1})
      // Operand: rd
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
      break;
    } // case FLW
    case RISCV::FSD: {
    if (STI.getFeatureBits()[RISCV::FeatureStdExtD] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcd] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR64CRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 6)) {
      // c.fsd	$rs2, ${imm}(${rs1})
      // Operand: rs2
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
    if (STI.getFeatureBits()[RISCV::FeatureStdExtD] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcd] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR64RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 7)) {
      // c.fsdsp	$rs2, ${imm}(${rs1})
      // Operand: rs2
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
      break;
    } // case FSD
    case RISCV::FSW: {
    if (STI.getFeatureBits()[RISCV::FeatureStdExtF] &&
      !STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcf] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR32CRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 8)) {
      // c.fsw	$rs2, ${imm}(${rs1})
      // Operand: rs2
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
    if (STI.getFeatureBits()[RISCV::FeatureStdExtF] &&
      !STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZcf] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::FPR32RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 9)) {
      // c.fswsp	$rs2, ${imm}(${rs1})
      // Operand: rs2
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
      break;
    } // case FSW
    case RISCV::JAL: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (MI.getOperand(0).getReg() == RISCV::X0) &&
      MI.getOperand(1).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(1), &STI, 10)) {
      // c.j	$offset
      // Operand: offset
      return true;
    } // if
    if (!STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (MI.getOperand(0).getReg() == RISCV::X1) &&
      MI.getOperand(1).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(1), &STI, 10)) {
      // c.jal	$offset
      // Operand: offset
      return true;
    } // if
      break;
    } // case JAL
    case RISCV::JALR: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (MI.getOperand(0).getReg() == RISCV::X1) &&
      (MI.getOperand(2).isImm()) &&
      (MI.getOperand(2).getImm() == 0) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.jalr	$rs1
      // Operand: rs1
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (MI.getOperand(0).getReg() == RISCV::X0) &&
      (MI.getOperand(2).isImm()) &&
      (MI.getOperand(2).getImm() == 0) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.jr	$rs1
      // Operand: rs1
      return true;
    } // if
      break;
    } // case JALR
    case RISCV::LD: {
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 6)) {
      // c.ld	$rd, ${imm}(${rs1})
      // Operand: rd
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 7)) {
      // c.ldsp	$rd, ${imm}(${rs1})
      // Operand: rd
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
      break;
    } // case LD
    case RISCV::LUI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0X2RegClassID].contains(MI.getOperand(0).getReg())) &&
      MI.getOperand(1).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(1), &STI, 11)) {
      // c.lui	$rd, $imm
      // Operand: rd
      // Operand: imm
      return true;
    } // if
      break;
    } // case LUI
    case RISCV::LW: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 8)) {
      // c.lw	$rd, ${imm}(${rs1})
      // Operand: rd
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 9)) {
      // c.lwsp	$rd, ${imm}(${rs1})
      // Operand: rd
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
      break;
    } // case LW
    case RISCV::OR: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg()))) {
      // c.or	$rd, $rs2
      // Operand: rd_wb
      // Operand: rd
      // Operand: rs2
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(2).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(2).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.or	$rd, $rs2
      // Operand: rd_wb
      // Operand: rd
      // Operand: rs2
      return true;
    } // if
      break;
    } // case OR
    case RISCV::SD: {
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 6)) {
      // c.sd	$rs2, ${imm}(${rs1})
      // Operand: rs2
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 7)) {
      // c.sdsp	$rs2, ${imm}(${rs1})
      // Operand: rs2
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
      break;
    } // case SD
    case RISCV::SLLI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRNoX0RegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 12)) {
      // c.slli	$rd, $imm
      // Operand: rd_wb
      // Operand: rd
      // Operand: imm
      return true;
    } // if
      break;
    } // case SLLI
    case RISCV::SRAI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 12)) {
      // c.srai	$rs1, $imm
      // Operand: rs1_wb
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
      break;
    } // case SRAI
    case RISCV::SRLI: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 12)) {
      // c.srli	$rs1, $imm
      // Operand: rs1_wb
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
      break;
    } // case SRLI
    case RISCV::SUB: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg()))) {
      // c.sub	$rd, $rs2
      // Operand: rd_wb
      // Operand: rd
      // Operand: rs2
      return true;
    } // if
      break;
    } // case SUB
    case RISCV::SUBW: {
    if (STI.getFeatureBits()[RISCV::Feature64Bit] &&
      (STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg()))) {
      // c.subw	$rd, $rs2
      // Operand: rd_wb
      // Operand: rd
      // Operand: rs2
      return true;
    } // if
      break;
    } // case SUBW
    case RISCV::SW: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 8)) {
      // c.sw	$rs2, ${imm}(${rs1})
      // Operand: rs2
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(0).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRRegClassID].contains(MI.getOperand(0).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::SPRegClassID].contains(MI.getOperand(1).getReg())) &&
      MI.getOperand(2).isImm() &&
      RISCVValidateMachineOperand(MI.getOperand(2), &STI, 9)) {
      // c.swsp	$rs2, ${imm}(${rs1})
      // Operand: rs2
      // Operand: rs1
      // Operand: imm
      return true;
    } // if
      break;
    } // case SW
    case RISCV::UNIMP: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC])) {
      // c.unimp	
      return true;
    } // if
      break;
    } // case UNIMP
    case RISCV::XOR: {
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(1).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(1).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg())) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg()))) {
      // c.xor	$rd, $rs2
      // Operand: rd_wb
      // Operand: rd
      // Operand: rs2
      return true;
    } // if
    if ((STI.getFeatureBits()[RISCV::FeatureExtZca] || STI.getFeatureBits()[RISCV::FeatureStdExtC]) &&
      (MI.getOperand(2).isReg()) && (MI.getOperand(0).isReg()) &&
      (MI.getOperand(2).getReg() ==  MI.getOperand(0).getReg()) &&
      (MI.getOperand(2).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(2).getReg())) &&
      (MI.getOperand(1).isReg()) &&
      (RISCVMCRegisterClasses[RISCV::GPRCRegClassID].contains(MI.getOperand(1).getReg()))) {
      // c.xor	$rd, $rs2
      // Operand: rd_wb
      // Operand: rd
      // Operand: rs2
      return true;
    } // if

    } // case XOR
  } // switch
  return false;
}

#endif //GEN_CHECK_COMPRESS_INSTR

