/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Pseudo-instruction MC lowering Source Fragment                             *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

bool ARMAsmPrinter::
emitPseudoExpansionLowering(MCStreamer &OutStreamer,
                            const MachineInstr *MI) {
  switch (MI->getOpcode()) {
    default: return false;
    case ARM::B: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::Bcc);
      // Operand: target
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      TmpInst.addOperand(MCOperand::createImm(14));
      TmpInst.addOperand(MCOperand::createReg(0));
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::LDMIA_RET: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::LDMIA_UPD);
      // Operand: wb
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: Rn
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      lowerOperand(MI->getOperand(3), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: regs
      lowerOperand(MI->getOperand(4), MCOp);
      TmpInst.addOperand(MCOp);
      // variable_ops
      for (unsigned i = 5, e = MI->getNumOperands(); i != e; ++i)
        if (lowerOperand(MI->getOperand(i), MCOp))
          TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::MLAv5: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::MLA);
      // Operand: Rd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: Rn
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: Rm
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: Ra
      lowerOperand(MI->getOperand(3), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      lowerOperand(MI->getOperand(4), MCOp);
      TmpInst.addOperand(MCOp);
      lowerOperand(MI->getOperand(5), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: s
      lowerOperand(MI->getOperand(6), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::MOVPCRX: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::MOVr);
      // Operand: Rd
      TmpInst.addOperand(MCOperand::createReg(ARM::PC));
      // Operand: Rm
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      TmpInst.addOperand(MCOperand::createImm(14));
      TmpInst.addOperand(MCOperand::createReg(0));
      // Operand: s
      TmpInst.addOperand(MCOperand::createReg(0));
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::MULv5: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::MUL);
      // Operand: Rd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: Rn
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: Rm
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      lowerOperand(MI->getOperand(3), MCOp);
      TmpInst.addOperand(MCOp);
      lowerOperand(MI->getOperand(4), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: s
      lowerOperand(MI->getOperand(5), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::SMLALv5: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::SMLAL);
      // Operand: RdLo
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: RdHi
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: Rn
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: Rm
      lowerOperand(MI->getOperand(3), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: RLo
      lowerOperand(MI->getOperand(4), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: RHi
      lowerOperand(MI->getOperand(5), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      lowerOperand(MI->getOperand(6), MCOp);
      TmpInst.addOperand(MCOp);
      lowerOperand(MI->getOperand(7), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: s
      lowerOperand(MI->getOperand(8), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::SMULLv5: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::SMULL);
      // Operand: RdLo
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: RdHi
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: Rn
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: Rm
      lowerOperand(MI->getOperand(3), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      lowerOperand(MI->getOperand(4), MCOp);
      TmpInst.addOperand(MCOp);
      lowerOperand(MI->getOperand(5), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: s
      lowerOperand(MI->getOperand(6), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::TAILJMPd: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::Bcc);
      // Operand: target
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      TmpInst.addOperand(MCOperand::createImm(14));
      TmpInst.addOperand(MCOperand::createReg(0));
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::TAILJMPr: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::BX);
      // Operand: dst
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::TAILJMPr4: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::MOVr);
      // Operand: Rd
      TmpInst.addOperand(MCOperand::createReg(ARM::PC));
      // Operand: Rm
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      TmpInst.addOperand(MCOperand::createImm(14));
      TmpInst.addOperand(MCOperand::createReg(0));
      // Operand: s
      TmpInst.addOperand(MCOperand::createReg(0));
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::UMLALv5: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::UMLAL);
      // Operand: RdLo
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: RdHi
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: Rn
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: Rm
      lowerOperand(MI->getOperand(3), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: RLo
      lowerOperand(MI->getOperand(4), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: RHi
      lowerOperand(MI->getOperand(5), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      lowerOperand(MI->getOperand(6), MCOp);
      TmpInst.addOperand(MCOp);
      lowerOperand(MI->getOperand(7), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: s
      lowerOperand(MI->getOperand(8), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::UMULLv5: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::UMULL);
      // Operand: RdLo
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: RdHi
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: Rn
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: Rm
      lowerOperand(MI->getOperand(3), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      lowerOperand(MI->getOperand(4), MCOp);
      TmpInst.addOperand(MCOp);
      lowerOperand(MI->getOperand(5), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: s
      lowerOperand(MI->getOperand(6), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::VMOVD0: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::VMOVv2i32);
      // Operand: Vd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: SIMM
      TmpInst.addOperand(MCOperand::createImm(0));
      // Operand: p
      TmpInst.addOperand(MCOperand::createImm(14));
      TmpInst.addOperand(MCOperand::createReg(0));
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::VMOVQ0: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::VMOVv4i32);
      // Operand: Vd
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: SIMM
      TmpInst.addOperand(MCOperand::createImm(0));
      // Operand: p
      TmpInst.addOperand(MCOperand::createImm(14));
      TmpInst.addOperand(MCOperand::createReg(0));
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::t2LDMIA_RET: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::t2LDMIA_UPD);
      // Operand: wb
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: Rn
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      lowerOperand(MI->getOperand(3), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: regs
      lowerOperand(MI->getOperand(4), MCOp);
      TmpInst.addOperand(MCOp);
      // variable_ops
      for (unsigned i = 5, e = MI->getNumOperands(); i != e; ++i)
        if (lowerOperand(MI->getOperand(i), MCOp))
          TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::tBRIND: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::tMOVr);
      // Operand: Rd
      TmpInst.addOperand(MCOperand::createReg(ARM::PC));
      // Operand: Rm
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::tBX_RET: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::tBX);
      // Operand: Rm
      TmpInst.addOperand(MCOperand::createReg(ARM::LR));
      // Operand: p
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::tBX_RET_vararg: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::tBX);
      // Operand: Rm
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::tBfar: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::tBL);
      // Operand: p
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: func
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::tLDMIA_UPD: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::tLDMIA);
      // Operand: Rn
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      lowerOperand(MI->getOperand(3), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: regs
      lowerOperand(MI->getOperand(4), MCOp);
      TmpInst.addOperand(MCOp);
      // variable_ops
      for (unsigned i = 5, e = MI->getNumOperands(); i != e; ++i)
        if (lowerOperand(MI->getOperand(i), MCOp))
          TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::tPOP_RET: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::tPOP);
      // Operand: p
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: regs
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      // variable_ops
      for (unsigned i = 3, e = MI->getNumOperands(); i != e; ++i)
        if (lowerOperand(MI->getOperand(i), MCOp))
          TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::tTAILJMPd: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::t2B);
      // Operand: target
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::tTAILJMPdND: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::tB);
      // Operand: target
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      lowerOperand(MI->getOperand(1), MCOp);
      TmpInst.addOperand(MCOp);
      lowerOperand(MI->getOperand(2), MCOp);
      TmpInst.addOperand(MCOp);
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
    case ARM::tTAILJMPr: {
      MCInst TmpInst;
      MCOperand MCOp;
      TmpInst.setOpcode(ARM::tBX);
      // Operand: Rm
      lowerOperand(MI->getOperand(0), MCOp);
      TmpInst.addOperand(MCOp);
      // Operand: p
      TmpInst.addOperand(MCOperand::createImm(14));
      TmpInst.addOperand(MCOperand::createReg(0));
      EmitToStreamer(OutStreamer, TmpInst);
      break;
    }
  }
  return true;
}

