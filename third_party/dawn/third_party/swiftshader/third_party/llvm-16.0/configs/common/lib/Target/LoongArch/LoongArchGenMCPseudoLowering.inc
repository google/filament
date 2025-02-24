/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Pseudo-instruction MC lowering Source Fragment                             *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

bool LoongArchAsmPrinter::
emitPseudoExpansionLowering(MCStreamer &OutStreamer,
                            const MachineInstr *MI) {
  switch (MI->getOpcode()) {
  default: return false;
  case LoongArch::PseudoAtomicStoreD: {
    MCInst TmpInst;
    MCOperand MCOp;
    TmpInst.setOpcode(LoongArch::AMSWAP_DB_D);
    // Operand: rd
    TmpInst.addOperand(MCOperand::createReg(LoongArch::R0));
    // Operand: rk
    lowerOperand(MI->getOperand(2), MCOp);
    TmpInst.addOperand(MCOp);
    // Operand: rj
    lowerOperand(MI->getOperand(1), MCOp);
    TmpInst.addOperand(MCOp);
    EmitToStreamer(OutStreamer, TmpInst);
    break;
  }
  case LoongArch::PseudoAtomicStoreW: {
    MCInst TmpInst;
    MCOperand MCOp;
    TmpInst.setOpcode(LoongArch::AMSWAP_DB_W);
    // Operand: rd
    TmpInst.addOperand(MCOperand::createReg(LoongArch::R0));
    // Operand: rk
    lowerOperand(MI->getOperand(2), MCOp);
    TmpInst.addOperand(MCOp);
    // Operand: rj
    lowerOperand(MI->getOperand(1), MCOp);
    TmpInst.addOperand(MCOp);
    EmitToStreamer(OutStreamer, TmpInst);
    break;
  }
  case LoongArch::PseudoBR: {
    MCInst TmpInst;
    MCOperand MCOp;
    TmpInst.setOpcode(LoongArch::B);
    // Operand: imm26
    lowerOperand(MI->getOperand(0), MCOp);
    TmpInst.addOperand(MCOp);
    EmitToStreamer(OutStreamer, TmpInst);
    break;
  }
  case LoongArch::PseudoBRIND: {
    MCInst TmpInst;
    MCOperand MCOp;
    TmpInst.setOpcode(LoongArch::JIRL);
    // Operand: rd
    TmpInst.addOperand(MCOperand::createReg(LoongArch::R0));
    // Operand: rj
    lowerOperand(MI->getOperand(0), MCOp);
    TmpInst.addOperand(MCOp);
    // Operand: imm16
    lowerOperand(MI->getOperand(1), MCOp);
    TmpInst.addOperand(MCOp);
    EmitToStreamer(OutStreamer, TmpInst);
    break;
  }
  case LoongArch::PseudoB_TAIL: {
    MCInst TmpInst;
    MCOperand MCOp;
    TmpInst.setOpcode(LoongArch::B);
    // Operand: imm26
    lowerOperand(MI->getOperand(0), MCOp);
    TmpInst.addOperand(MCOp);
    EmitToStreamer(OutStreamer, TmpInst);
    break;
  }
  case LoongArch::PseudoCALLIndirect: {
    MCInst TmpInst;
    MCOperand MCOp;
    TmpInst.setOpcode(LoongArch::JIRL);
    // Operand: rd
    TmpInst.addOperand(MCOperand::createReg(LoongArch::R1));
    // Operand: rj
    lowerOperand(MI->getOperand(0), MCOp);
    TmpInst.addOperand(MCOp);
    // Operand: imm16
    TmpInst.addOperand(MCOperand::createImm(0));
    EmitToStreamer(OutStreamer, TmpInst);
    break;
  }
  case LoongArch::PseudoJIRL_CALL: {
    MCInst TmpInst;
    MCOperand MCOp;
    TmpInst.setOpcode(LoongArch::JIRL);
    // Operand: rd
    TmpInst.addOperand(MCOperand::createReg(LoongArch::R1));
    // Operand: rj
    lowerOperand(MI->getOperand(0), MCOp);
    TmpInst.addOperand(MCOp);
    // Operand: imm16
    lowerOperand(MI->getOperand(1), MCOp);
    TmpInst.addOperand(MCOp);
    EmitToStreamer(OutStreamer, TmpInst);
    break;
  }
  case LoongArch::PseudoJIRL_TAIL: {
    MCInst TmpInst;
    MCOperand MCOp;
    TmpInst.setOpcode(LoongArch::JIRL);
    // Operand: rd
    TmpInst.addOperand(MCOperand::createReg(LoongArch::R0));
    // Operand: rj
    lowerOperand(MI->getOperand(0), MCOp);
    TmpInst.addOperand(MCOp);
    // Operand: imm16
    lowerOperand(MI->getOperand(1), MCOp);
    TmpInst.addOperand(MCOp);
    EmitToStreamer(OutStreamer, TmpInst);
    break;
  }
  case LoongArch::PseudoRET: {
    MCInst TmpInst;
    MCOperand MCOp;
    TmpInst.setOpcode(LoongArch::JIRL);
    // Operand: rd
    TmpInst.addOperand(MCOperand::createReg(LoongArch::R0));
    // Operand: rj
    TmpInst.addOperand(MCOperand::createReg(LoongArch::R1));
    // Operand: imm16
    TmpInst.addOperand(MCOperand::createImm(0));
    EmitToStreamer(OutStreamer, TmpInst);
    break;
  }
  case LoongArch::PseudoTAILIndirect: {
    MCInst TmpInst;
    MCOperand MCOp;
    TmpInst.setOpcode(LoongArch::JIRL);
    // Operand: rd
    TmpInst.addOperand(MCOperand::createReg(LoongArch::R0));
    // Operand: rj
    lowerOperand(MI->getOperand(0), MCOp);
    TmpInst.addOperand(MCOp);
    // Operand: imm16
    TmpInst.addOperand(MCOperand::createImm(0));
    EmitToStreamer(OutStreamer, TmpInst);
    break;
  }
  case LoongArch::PseudoUNIMP: {
    MCInst TmpInst;
    MCOperand MCOp;
    TmpInst.setOpcode(LoongArch::AMSWAP_W);
    // Operand: rd
    TmpInst.addOperand(MCOperand::createReg(LoongArch::R0));
    // Operand: rk
    TmpInst.addOperand(MCOperand::createReg(LoongArch::R1));
    // Operand: rj
    TmpInst.addOperand(MCOperand::createReg(LoongArch::R0));
    EmitToStreamer(OutStreamer, TmpInst);
    break;
  }
  }
  return true;
}

