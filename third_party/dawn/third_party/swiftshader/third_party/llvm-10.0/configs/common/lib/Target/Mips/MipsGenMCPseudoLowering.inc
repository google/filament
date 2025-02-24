/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Pseudo-instruction MC lowering Source Fragment                             *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

bool MipsAsmPrinter::
emitPseudoExpansionLowering(MCStreamer &OutStreamer,
                            const MachineInstr *MI) {
  switch (MI->getOpcode()) {
    default: return false;
    case Mips::AND_V_D_PSEUDO: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::AND_V);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::AND_V_H_PSEUDO: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::AND_V);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::AND_V_W_PSEUDO: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::AND_V);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::B: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::BEQ);
      // Operand: rs
      TmpInst.addOperand(MCOperand::createReg(Mips::ZERO));
      // Operand: rt
      TmpInst.addOperand(MCOperand::createReg(Mips::ZERO));
      // Operand: offset
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::BAL_BR: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::BGEZAL);
      // Operand: rs
      TmpInst.addOperand(MCOperand::createReg(Mips::ZERO));
      // Operand: offset
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::BAL_BR_MM: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::BGEZAL_MM);
      // Operand: rs
      TmpInst.addOperand(MCOperand::createReg(Mips::ZERO));
      // Operand: offset
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::BSEL_D_PSEUDO: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::BSEL_V);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wd_in
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(3), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::BSEL_FD_PSEUDO: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::BSEL_V);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wd_in
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(3), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::BSEL_FW_PSEUDO: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::BSEL_V);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wd_in
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(3), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::BSEL_H_PSEUDO: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::BSEL_V);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wd_in
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(3), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::BSEL_W_PSEUDO: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::BSEL_V);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wd_in
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(3), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::B_MM: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::BEQ_MM);
      // Operand: rs
      TmpInst.addOperand(MCOperand::createReg(Mips::ZERO));
      // Operand: rt
      TmpInst.addOperand(MCOperand::createReg(Mips::ZERO));
      // Operand: offset
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::FABS_D: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::FMAX_A_D);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::FABS_W: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::FMAX_A_W);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::JALR64Pseudo: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JALR);
      // Operand: rd
      TmpInst.addOperand(MCOperand::createReg(Mips::RA));
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::JALRHB64Pseudo: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JALR_HB64);
      // Operand: rd
      TmpInst.addOperand(MCOperand::createReg(Mips::RA_64));
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::JALRHBPseudo: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JALR_HB);
      // Operand: rd
      TmpInst.addOperand(MCOperand::createReg(Mips::RA));
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::JALRPseudo: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JALR);
      // Operand: rd
      TmpInst.addOperand(MCOperand::createReg(Mips::RA));
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::JAL_MMR6: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::BALC_MMR6);
      // Operand: offset
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::NOP: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::SLL);
      // Operand: rd
      TmpInst.addOperand(MCOperand::createReg(Mips::ZERO));
      // Operand: rt
      TmpInst.addOperand(MCOperand::createReg(Mips::ZERO));
      // Operand: shamt
      TmpInst.addOperand(MCOperand::createImm(0));
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::NOR_V_D_PSEUDO: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::NOR_V);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::NOR_V_H_PSEUDO: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::NOR_V);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::NOR_V_W_PSEUDO: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::NOR_V);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::OR_V_D_PSEUDO: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::OR_V);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::OR_V_H_PSEUDO: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::OR_V);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::OR_V_W_PSEUDO: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::OR_V);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoCMPU_EQ_QB: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::CMPU_EQ_QB);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoCMPU_LE_QB: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::CMPU_LE_QB);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoCMPU_LT_QB: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::CMPU_LT_QB);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoCMP_EQ_PH: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::CMP_EQ_PH);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoCMP_LE_PH: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::CMP_LE_PH);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoCMP_LT_PH: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::CMP_LT_PH);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoDMULT: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::DMULT);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoDMULTu: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::DMULTu);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoDSDIV: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::DSDIV);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoDUDIV: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::DUDIV);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoIndirectBranch: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JR);
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoIndirectBranch64: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JR64);
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoIndirectBranch64R6: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JALR64);
      // Operand: rd
      TmpInst.addOperand(MCOperand::createReg(Mips::ZERO_64));
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoIndirectBranchR6: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JALR);
      // Operand: rd
      TmpInst.addOperand(MCOperand::createReg(Mips::ZERO));
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoIndirectBranch_MM: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JR_MM);
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoIndirectBranch_MMR6: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JRC16_MMR6);
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoIndirectHazardBranch: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JR_HB);
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoIndirectHazardBranch64: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JR_HB64);
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoIndrectHazardBranch64R6: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JR_HB64_R6);
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoIndrectHazardBranchR6: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JR_HB_R6);
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoMADD: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::MADD);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoMADDU: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::MADDU);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoMADDU_MM: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::MADDU);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoMADD_MM: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::MADD);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoMSUB: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::MSUB);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoMSUBU: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::MSUBU);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoMSUBU_MM: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::MSUBU);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoMSUB_MM: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::MSUB);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoMULT: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::MULT);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoMULT_MM: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::MULT);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoMULTu: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::MULTu);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoMULTu_MM: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::MULTu);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoPICK_PH: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::PICK_PH);
      // Operand: rd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rs
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(3), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoPICK_QB: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::PICK_QB);
      // Operand: rd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rs
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(3), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoSDIV: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::SDIV);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::PseudoUDIV: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::UDIV);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::SDIV_MM_Pseudo: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::SDIV_MM);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::TAILCALL: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::J);
      // Operand: target
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::TAILCALL64R6REG: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JALR64);
      // Operand: rd
      TmpInst.addOperand(MCOperand::createReg(Mips::ZERO_64));
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::TAILCALLHB64R6REG: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JR_HB64_R6);
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::TAILCALLHBR6REG: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JR_HB_R6);
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::TAILCALLR6REG: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JALR);
      // Operand: rd
      TmpInst.addOperand(MCOperand::createReg(Mips::ZERO));
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::TAILCALLREG: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JR);
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::TAILCALLREG64: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JR64);
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::TAILCALLREGHB: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JR_HB);
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::TAILCALLREGHB64: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JR_HB64);
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::TAILCALLREG_MM: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JRC16_MM);
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::TAILCALLREG_MMR6: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::JRC16_MM);
      // Operand: rs
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::TAILCALL_MM: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::J_MM);
      // Operand: target
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::TAILCALL_MMR6: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::BC_MMR6);
      // Operand: offset
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::TRAP: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::BREAK);
      // Operand: code_1
      TmpInst.addOperand(MCOperand::createImm(0));
      // Operand: code_2
      TmpInst.addOperand(MCOperand::createImm(0));
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::TRAP_MM: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::BREAK_MM);
      // Operand: code_1
      TmpInst.addOperand(MCOperand::createImm(0));
      // Operand: code_2
      TmpInst.addOperand(MCOperand::createImm(0));
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::UDIV_MM_Pseudo: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::UDIV_MM);
      // Operand: rs
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: rt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::XOR_V_D_PSEUDO: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::XOR_V);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::XOR_V_H_PSEUDO: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::XOR_V);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case Mips::XOR_V_W_PSEUDO: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(Mips::XOR_V);
      // Operand: wd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: ws
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: wt
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
  }
  return true;
}

