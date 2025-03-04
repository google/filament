//===- X86RecognizableInstr.cpp - Disassembler instruction spec --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is part of the X86 Disassembler Emitter.
// It contains the implementation of a single recognizable instruction.
// Documentation for the disassembler emitter in general can be found in
//  X86DisasemblerEmitter.h.
//
//===----------------------------------------------------------------------===//

#include "X86RecognizableInstr.h"
#include "X86DisassemblerShared.h"
#include "X86ModRMFilters.h"
#include "llvm/Support/ErrorHandling.h"
#include <string>

using namespace llvm;

#define MRM_MAPPING     \
  MAP(C0, 32)           \
  MAP(C1, 33)           \
  MAP(C2, 34)           \
  MAP(C3, 35)           \
  MAP(C4, 36)           \
  MAP(C5, 37)           \
  MAP(C6, 38)           \
  MAP(C7, 39)           \
  MAP(C8, 40)           \
  MAP(C9, 41)           \
  MAP(CA, 42)           \
  MAP(CB, 43)           \
  MAP(CC, 44)           \
  MAP(CD, 45)           \
  MAP(CE, 46)           \
  MAP(CF, 47)           \
  MAP(D0, 48)           \
  MAP(D1, 49)           \
  MAP(D2, 50)           \
  MAP(D3, 51)           \
  MAP(D4, 52)           \
  MAP(D5, 53)           \
  MAP(D6, 54)           \
  MAP(D7, 55)           \
  MAP(D8, 56)           \
  MAP(D9, 57)           \
  MAP(DA, 58)           \
  MAP(DB, 59)           \
  MAP(DC, 60)           \
  MAP(DD, 61)           \
  MAP(DE, 62)           \
  MAP(DF, 63)           \
  MAP(E0, 64)           \
  MAP(E1, 65)           \
  MAP(E2, 66)           \
  MAP(E3, 67)           \
  MAP(E4, 68)           \
  MAP(E5, 69)           \
  MAP(E6, 70)           \
  MAP(E7, 71)           \
  MAP(E8, 72)           \
  MAP(E9, 73)           \
  MAP(EA, 74)           \
  MAP(EB, 75)           \
  MAP(EC, 76)           \
  MAP(ED, 77)           \
  MAP(EE, 78)           \
  MAP(EF, 79)           \
  MAP(F0, 80)           \
  MAP(F1, 81)           \
  MAP(F2, 82)           \
  MAP(F3, 83)           \
  MAP(F4, 84)           \
  MAP(F5, 85)           \
  MAP(F6, 86)           \
  MAP(F7, 87)           \
  MAP(F8, 88)           \
  MAP(F9, 89)           \
  MAP(FA, 90)           \
  MAP(FB, 91)           \
  MAP(FC, 92)           \
  MAP(FD, 93)           \
  MAP(FE, 94)           \
  MAP(FF, 95)

// A clone of X86 since we can't depend on something that is generated.
namespace X86Local {
  enum {
    Pseudo      = 0,
    RawFrm      = 1,
    AddRegFrm   = 2,
    MRMDestReg  = 3,
    MRMDestMem  = 4,
    MRMSrcReg   = 5,
    MRMSrcMem   = 6,
    RawFrmMemOffs = 7,
    RawFrmSrc   = 8,
    RawFrmDst   = 9,
    RawFrmDstSrc = 10,
    RawFrmImm8  = 11,
    RawFrmImm16 = 12,
    MRMXr = 14, MRMXm = 15,
    MRM0r = 16, MRM1r = 17, MRM2r = 18, MRM3r = 19,
    MRM4r = 20, MRM5r = 21, MRM6r = 22, MRM7r = 23,
    MRM0m = 24, MRM1m = 25, MRM2m = 26, MRM3m = 27,
    MRM4m = 28, MRM5m = 29, MRM6m = 30, MRM7m = 31,
#define MAP(from, to) MRM_##from = to,
    MRM_MAPPING
#undef MAP
    lastMRM
  };

  enum {
    OB = 0, TB = 1, T8 = 2, TA = 3, XOP8 = 4, XOP9 = 5, XOPA = 6
  };

  enum {
    PS = 1, PD = 2, XS = 3, XD = 4
  };

  enum {
    VEX = 1, XOP = 2, EVEX = 3
  };

  enum {
    OpSize16 = 1, OpSize32 = 2
  };

  enum {
    AdSize16 = 1, AdSize32 = 2, AdSize64 = 3
  };
}

using namespace X86Disassembler;

/// isRegFormat - Indicates whether a particular form requires the Mod field of
///   the ModR/M byte to be 0b11.
///
/// @param form - The form of the instruction.
/// @return     - true if the form implies that Mod must be 0b11, false
///               otherwise.
static bool isRegFormat(uint8_t form) {
  return (form == X86Local::MRMDestReg ||
          form == X86Local::MRMSrcReg  ||
          form == X86Local::MRMXr ||
          (form >= X86Local::MRM0r && form <= X86Local::MRM7r));
}

/// byteFromBitsInit - Extracts a value at most 8 bits in width from a BitsInit.
///   Useful for switch statements and the like.
///
/// @param init - A reference to the BitsInit to be decoded.
/// @return     - The field, with the first bit in the BitsInit as the lowest
///               order bit.
static uint8_t byteFromBitsInit(BitsInit &init) {
  int width = init.getNumBits();

  assert(width <= 8 && "Field is too large for uint8_t!");

  int     index;
  uint8_t mask = 0x01;

  uint8_t ret = 0;

  for (index = 0; index < width; index++) {
    if (static_cast<BitInit*>(init.getBit(index))->getValue())
      ret |= mask;

    mask <<= 1;
  }

  return ret;
}

/// byteFromRec - Extract a value at most 8 bits in with from a Record given the
///   name of the field.
///
/// @param rec  - The record from which to extract the value.
/// @param name - The name of the field in the record.
/// @return     - The field, as translated by byteFromBitsInit().
static uint8_t byteFromRec(const Record* rec, const std::string &name) {
  BitsInit* bits = rec->getValueAsBitsInit(name);
  return byteFromBitsInit(*bits);
}

RecognizableInstr::RecognizableInstr(DisassemblerTables &tables,
                                     const CodeGenInstruction &insn,
                                     InstrUID uid) {
  UID = uid;

  Rec = insn.TheDef;
  Name = Rec->getName();
  Spec = &tables.specForUID(UID);

  if (!Rec->isSubClassOf("X86Inst")) {
    ShouldBeEmitted = false;
    return;
  }

  OpPrefix = byteFromRec(Rec, "OpPrefixBits");
  OpMap    = byteFromRec(Rec, "OpMapBits");
  Opcode   = byteFromRec(Rec, "Opcode");
  Form     = byteFromRec(Rec, "FormBits");
  Encoding = byteFromRec(Rec, "OpEncBits");

  OpSize           = byteFromRec(Rec, "OpSizeBits");
  AdSize           = byteFromRec(Rec, "AdSizeBits");
  HasREX_WPrefix   = Rec->getValueAsBit("hasREX_WPrefix");
  HasVEX_4V        = Rec->getValueAsBit("hasVEX_4V");
  HasVEX_4VOp3     = Rec->getValueAsBit("hasVEX_4VOp3");
  HasVEX_WPrefix   = Rec->getValueAsBit("hasVEX_WPrefix");
  HasMemOp4Prefix  = Rec->getValueAsBit("hasMemOp4Prefix");
  IgnoresVEX_L     = Rec->getValueAsBit("ignoresVEX_L");
  HasEVEX_L2Prefix = Rec->getValueAsBit("hasEVEX_L2");
  HasEVEX_K        = Rec->getValueAsBit("hasEVEX_K");
  HasEVEX_KZ       = Rec->getValueAsBit("hasEVEX_Z");
  HasEVEX_B        = Rec->getValueAsBit("hasEVEX_B");
  IsCodeGenOnly    = Rec->getValueAsBit("isCodeGenOnly");
  ForceDisassemble = Rec->getValueAsBit("ForceDisassemble");
  CD8_Scale        = byteFromRec(Rec, "CD8_Scale");

  Name      = Rec->getName();
  AsmString = Rec->getValueAsString("AsmString");

  Operands = &insn.Operands.OperandList;

  HasVEX_LPrefix   = Rec->getValueAsBit("hasVEX_L");

  // Check for 64-bit inst which does not require REX
  Is32Bit = false;
  Is64Bit = false;
  // FIXME: Is there some better way to check for In64BitMode?
  std::vector<Record*> Predicates = Rec->getValueAsListOfDefs("Predicates");
  for (unsigned i = 0, e = Predicates.size(); i != e; ++i) {
    if (Predicates[i]->getName().find("Not64Bit") != Name.npos ||
	Predicates[i]->getName().find("In32Bit") != Name.npos) {
      Is32Bit = true;
      break;
    }
    if (Predicates[i]->getName().find("In64Bit") != Name.npos) {
      Is64Bit = true;
      break;
    }
  }

  if (Form == X86Local::Pseudo || (IsCodeGenOnly && !ForceDisassemble)) {
    ShouldBeEmitted = false;
    return;
  }

  // Special case since there is no attribute class for 64-bit and VEX
  if (Name == "VMASKMOVDQU64") {
    ShouldBeEmitted = false;
    return;
  }

  ShouldBeEmitted  = true;
}

void RecognizableInstr::processInstr(DisassemblerTables &tables,
                                     const CodeGenInstruction &insn,
                                     InstrUID uid)
{
  // Ignore "asm parser only" instructions.
  if (insn.TheDef->getValueAsBit("isAsmParserOnly"))
    return;

  RecognizableInstr recogInstr(tables, insn, uid);

  if (recogInstr.shouldBeEmitted()) {
    recogInstr.emitInstructionSpecifier();
    recogInstr.emitDecodePath(tables);
  }
}

#define EVEX_KB(n) (HasEVEX_KZ && HasEVEX_B ? n##_KZ_B : \
                    (HasEVEX_K && HasEVEX_B ? n##_K_B : \
                    (HasEVEX_KZ ? n##_KZ : \
                    (HasEVEX_K? n##_K : (HasEVEX_B ? n##_B : n)))))

InstructionContext RecognizableInstr::insnContext() const {
  InstructionContext insnContext;

  if (Encoding == X86Local::EVEX) {
    if (HasVEX_LPrefix && HasEVEX_L2Prefix) {
      errs() << "Don't support VEX.L if EVEX_L2 is enabled: " << Name << "\n";
      llvm_unreachable("Don't support VEX.L if EVEX_L2 is enabled");
    }
    // VEX_L & VEX_W
    if (HasVEX_LPrefix && HasVEX_WPrefix) {
      if (OpPrefix == X86Local::PD)
        insnContext = EVEX_KB(IC_EVEX_L_W_OPSIZE);
      else if (OpPrefix == X86Local::XS)
        insnContext = EVEX_KB(IC_EVEX_L_W_XS);
      else if (OpPrefix == X86Local::XD)
        insnContext = EVEX_KB(IC_EVEX_L_W_XD);
      else if (OpPrefix == X86Local::PS)
        insnContext = EVEX_KB(IC_EVEX_L_W);
      else {
        errs() << "Instruction does not use a prefix: " << Name << "\n";
        llvm_unreachable("Invalid prefix");
      }
    } else if (HasVEX_LPrefix) {
      // VEX_L
      if (OpPrefix == X86Local::PD)
        insnContext = EVEX_KB(IC_EVEX_L_OPSIZE);
      else if (OpPrefix == X86Local::XS)
        insnContext = EVEX_KB(IC_EVEX_L_XS);
      else if (OpPrefix == X86Local::XD)
        insnContext = EVEX_KB(IC_EVEX_L_XD);
      else if (OpPrefix == X86Local::PS)
        insnContext = EVEX_KB(IC_EVEX_L);
      else {
        errs() << "Instruction does not use a prefix: " << Name << "\n";
        llvm_unreachable("Invalid prefix");
      }
    }
    else if (HasEVEX_L2Prefix && HasVEX_WPrefix) {
      // EVEX_L2 & VEX_W
      if (OpPrefix == X86Local::PD)
        insnContext = EVEX_KB(IC_EVEX_L2_W_OPSIZE);
      else if (OpPrefix == X86Local::XS)
        insnContext = EVEX_KB(IC_EVEX_L2_W_XS);
      else if (OpPrefix == X86Local::XD)
        insnContext = EVEX_KB(IC_EVEX_L2_W_XD);
      else if (OpPrefix == X86Local::PS)
        insnContext = EVEX_KB(IC_EVEX_L2_W);
      else {
        errs() << "Instruction does not use a prefix: " << Name << "\n";
        llvm_unreachable("Invalid prefix");
      }
    } else if (HasEVEX_L2Prefix) {
      // EVEX_L2
      if (OpPrefix == X86Local::PD)
        insnContext = EVEX_KB(IC_EVEX_L2_OPSIZE);
      else if (OpPrefix == X86Local::XD)
        insnContext = EVEX_KB(IC_EVEX_L2_XD);
      else if (OpPrefix == X86Local::XS)
        insnContext = EVEX_KB(IC_EVEX_L2_XS);
      else if (OpPrefix == X86Local::PS)
        insnContext = EVEX_KB(IC_EVEX_L2);
      else {
        errs() << "Instruction does not use a prefix: " << Name << "\n";
        llvm_unreachable("Invalid prefix");
      }
    }
    else if (HasVEX_WPrefix) {
      // VEX_W
      if (OpPrefix == X86Local::PD)
        insnContext = EVEX_KB(IC_EVEX_W_OPSIZE);
      else if (OpPrefix == X86Local::XS)
        insnContext = EVEX_KB(IC_EVEX_W_XS);
      else if (OpPrefix == X86Local::XD)
        insnContext = EVEX_KB(IC_EVEX_W_XD);
      else if (OpPrefix == X86Local::PS)
        insnContext = EVEX_KB(IC_EVEX_W);
      else {
        errs() << "Instruction does not use a prefix: " << Name << "\n";
        llvm_unreachable("Invalid prefix");
      }
    }
    // No L, no W
    else if (OpPrefix == X86Local::PD)
      insnContext = EVEX_KB(IC_EVEX_OPSIZE);
    else if (OpPrefix == X86Local::XD)
      insnContext = EVEX_KB(IC_EVEX_XD);
    else if (OpPrefix == X86Local::XS)
      insnContext = EVEX_KB(IC_EVEX_XS);
    else
      insnContext = EVEX_KB(IC_EVEX);
    /// eof EVEX
  } else if (Encoding == X86Local::VEX || Encoding == X86Local::XOP) {
    if (HasVEX_LPrefix && HasVEX_WPrefix) {
      if (OpPrefix == X86Local::PD)
        insnContext = IC_VEX_L_W_OPSIZE;
      else if (OpPrefix == X86Local::XS)
        insnContext = IC_VEX_L_W_XS;
      else if (OpPrefix == X86Local::XD)
        insnContext = IC_VEX_L_W_XD;
      else if (OpPrefix == X86Local::PS)
        insnContext = IC_VEX_L_W;
      else {
        errs() << "Instruction does not use a prefix: " << Name << "\n";
        llvm_unreachable("Invalid prefix");
      }
    } else if (OpPrefix == X86Local::PD && HasVEX_LPrefix)
      insnContext = IC_VEX_L_OPSIZE;
    else if (OpPrefix == X86Local::PD && HasVEX_WPrefix)
      insnContext = IC_VEX_W_OPSIZE;
    else if (OpPrefix == X86Local::PD)
      insnContext = IC_VEX_OPSIZE;
    else if (HasVEX_LPrefix && OpPrefix == X86Local::XS)
      insnContext = IC_VEX_L_XS;
    else if (HasVEX_LPrefix && OpPrefix == X86Local::XD)
      insnContext = IC_VEX_L_XD;
    else if (HasVEX_WPrefix && OpPrefix == X86Local::XS)
      insnContext = IC_VEX_W_XS;
    else if (HasVEX_WPrefix && OpPrefix == X86Local::XD)
      insnContext = IC_VEX_W_XD;
    else if (HasVEX_WPrefix && OpPrefix == X86Local::PS)
      insnContext = IC_VEX_W;
    else if (HasVEX_LPrefix && OpPrefix == X86Local::PS)
      insnContext = IC_VEX_L;
    else if (OpPrefix == X86Local::XD)
      insnContext = IC_VEX_XD;
    else if (OpPrefix == X86Local::XS)
      insnContext = IC_VEX_XS;
    else if (OpPrefix == X86Local::PS)
      insnContext = IC_VEX;
    else {
      errs() << "Instruction does not use a prefix: " << Name << "\n";
      llvm_unreachable("Invalid prefix");
    }
  } else if (Is64Bit || HasREX_WPrefix || AdSize == X86Local::AdSize64) {
    if (HasREX_WPrefix && (OpSize == X86Local::OpSize16 || OpPrefix == X86Local::PD))
      insnContext = IC_64BIT_REXW_OPSIZE;
    else if (HasREX_WPrefix && AdSize == X86Local::AdSize32)
      insnContext = IC_64BIT_REXW_ADSIZE;
    else if (OpSize == X86Local::OpSize16 && OpPrefix == X86Local::XD)
      insnContext = IC_64BIT_XD_OPSIZE;
    else if (OpSize == X86Local::OpSize16 && OpPrefix == X86Local::XS)
      insnContext = IC_64BIT_XS_OPSIZE;
    else if (OpSize == X86Local::OpSize16 && AdSize == X86Local::AdSize32)
      insnContext = IC_64BIT_OPSIZE_ADSIZE;
    else if (OpSize == X86Local::OpSize16 || OpPrefix == X86Local::PD)
      insnContext = IC_64BIT_OPSIZE;
    else if (AdSize == X86Local::AdSize32)
      insnContext = IC_64BIT_ADSIZE;
    else if (HasREX_WPrefix && OpPrefix == X86Local::XS)
      insnContext = IC_64BIT_REXW_XS;
    else if (HasREX_WPrefix && OpPrefix == X86Local::XD)
      insnContext = IC_64BIT_REXW_XD;
    else if (OpPrefix == X86Local::XD)
      insnContext = IC_64BIT_XD;
    else if (OpPrefix == X86Local::XS)
      insnContext = IC_64BIT_XS;
    else if (HasREX_WPrefix)
      insnContext = IC_64BIT_REXW;
    else
      insnContext = IC_64BIT;
  } else {
    if (OpSize == X86Local::OpSize16 && OpPrefix == X86Local::XD)
      insnContext = IC_XD_OPSIZE;
    else if (OpSize == X86Local::OpSize16 && OpPrefix == X86Local::XS)
      insnContext = IC_XS_OPSIZE;
    else if (OpSize == X86Local::OpSize16 && AdSize == X86Local::AdSize16)
      insnContext = IC_OPSIZE_ADSIZE;
    else if (OpSize == X86Local::OpSize16 || OpPrefix == X86Local::PD)
      insnContext = IC_OPSIZE;
    else if (AdSize == X86Local::AdSize16)
      insnContext = IC_ADSIZE;
    else if (OpPrefix == X86Local::XD)
      insnContext = IC_XD;
    else if (OpPrefix == X86Local::XS)
      insnContext = IC_XS;
    else
      insnContext = IC;
  }

  return insnContext;
}

void RecognizableInstr::adjustOperandEncoding(OperandEncoding &encoding) {
  // The scaling factor for AVX512 compressed displacement encoding is an
  // instruction attribute.  Adjust the ModRM encoding type to include the
  // scale for compressed displacement.
  if (encoding != ENCODING_RM || CD8_Scale == 0)
    return;
  encoding = (OperandEncoding)(encoding + Log2_32(CD8_Scale));
  assert(encoding <= ENCODING_RM_CD64 && "Invalid CDisp scaling");
}

void RecognizableInstr::handleOperand(bool optional, unsigned &operandIndex,
                                      unsigned &physicalOperandIndex,
                                      unsigned &numPhysicalOperands,
                                      const unsigned *operandMapping,
                                      OperandEncoding (*encodingFromString)
                                        (const std::string&,
                                         uint8_t OpSize)) {
  if (optional) {
    if (physicalOperandIndex >= numPhysicalOperands)
      return;
  } else {
    assert(physicalOperandIndex < numPhysicalOperands);
  }

  while (operandMapping[operandIndex] != operandIndex) {
    Spec->operands[operandIndex].encoding = ENCODING_DUP;
    Spec->operands[operandIndex].type =
      (OperandType)(TYPE_DUP0 + operandMapping[operandIndex]);
    ++operandIndex;
  }

  const std::string &typeName = (*Operands)[operandIndex].Rec->getName();

  OperandEncoding encoding = encodingFromString(typeName, OpSize);
  // Adjust the encoding type for an operand based on the instruction.
  adjustOperandEncoding(encoding);
  Spec->operands[operandIndex].encoding = encoding;
  Spec->operands[operandIndex].type = typeFromString(typeName,
                                                     HasREX_WPrefix, OpSize);

  ++operandIndex;
  ++physicalOperandIndex;
}

void RecognizableInstr::emitInstructionSpecifier() {
  Spec->name       = Name;

  Spec->insnContext = insnContext();

  const std::vector<CGIOperandList::OperandInfo> &OperandList = *Operands;

  unsigned numOperands = OperandList.size();
  unsigned numPhysicalOperands = 0;

  // operandMapping maps from operands in OperandList to their originals.
  // If operandMapping[i] != i, then the entry is a duplicate.
  unsigned operandMapping[X86_MAX_OPERANDS];
  assert(numOperands <= X86_MAX_OPERANDS && "X86_MAX_OPERANDS is not large enough");

  for (unsigned operandIndex = 0; operandIndex < numOperands; ++operandIndex) {
    if (!OperandList[operandIndex].Constraints.empty()) {
      const CGIOperandList::ConstraintInfo &Constraint =
        OperandList[operandIndex].Constraints[0];
      if (Constraint.isTied()) {
        operandMapping[operandIndex] = operandIndex;
        operandMapping[Constraint.getTiedOperand()] = operandIndex;
      } else {
        ++numPhysicalOperands;
        operandMapping[operandIndex] = operandIndex;
      }
    } else {
      ++numPhysicalOperands;
      operandMapping[operandIndex] = operandIndex;
    }
  }

#define HANDLE_OPERAND(class)               \
  handleOperand(false,                      \
                operandIndex,               \
                physicalOperandIndex,       \
                numPhysicalOperands,        \
                operandMapping,             \
                class##EncodingFromString);

#define HANDLE_OPTIONAL(class)              \
  handleOperand(true,                       \
                operandIndex,               \
                physicalOperandIndex,       \
                numPhysicalOperands,        \
                operandMapping,             \
                class##EncodingFromString);

  // operandIndex should always be < numOperands
  unsigned operandIndex = 0;
  // physicalOperandIndex should always be < numPhysicalOperands
  unsigned physicalOperandIndex = 0;

  // Given the set of prefix bits, how many additional operands does the
  // instruction have?
  unsigned additionalOperands = 0;
  if (HasVEX_4V || HasVEX_4VOp3)
    ++additionalOperands;
  if (HasEVEX_K)
    ++additionalOperands;

  switch (Form) {
  default: llvm_unreachable("Unhandled form");
  case X86Local::RawFrmSrc:
    HANDLE_OPERAND(relocation);
    return;
  case X86Local::RawFrmDst:
    HANDLE_OPERAND(relocation);
    return;
  case X86Local::RawFrmDstSrc:
    HANDLE_OPERAND(relocation);
    HANDLE_OPERAND(relocation);
    return;
  case X86Local::RawFrm:
    // Operand 1 (optional) is an address or immediate.
    // Operand 2 (optional) is an immediate.
    assert(numPhysicalOperands <= 2 &&
           "Unexpected number of operands for RawFrm");
    HANDLE_OPTIONAL(relocation)
    HANDLE_OPTIONAL(immediate)
    break;
  case X86Local::RawFrmMemOffs:
    // Operand 1 is an address.
    HANDLE_OPERAND(relocation);
    break;
  case X86Local::AddRegFrm:
    // Operand 1 is added to the opcode.
    // Operand 2 (optional) is an address.
    assert(numPhysicalOperands >= 1 && numPhysicalOperands <= 2 &&
           "Unexpected number of operands for AddRegFrm");
    HANDLE_OPERAND(opcodeModifier)
    HANDLE_OPTIONAL(relocation)
    break;
  case X86Local::MRMDestReg:
    // Operand 1 is a register operand in the R/M field.
    // - In AVX512 there may be a mask operand here -
    // Operand 2 is a register operand in the Reg/Opcode field.
    // - In AVX, there is a register operand in the VEX.vvvv field here -
    // Operand 3 (optional) is an immediate.
    assert(numPhysicalOperands >= 2 + additionalOperands &&
           numPhysicalOperands <= 3 + additionalOperands &&
           "Unexpected number of operands for MRMDestRegFrm");

    HANDLE_OPERAND(rmRegister)
    if (HasEVEX_K)
      HANDLE_OPERAND(writemaskRegister)

    if (HasVEX_4V)
      // FIXME: In AVX, the register below becomes the one encoded
      // in ModRMVEX and the one above the one in the VEX.VVVV field
      HANDLE_OPERAND(vvvvRegister)

    HANDLE_OPERAND(roRegister)
    HANDLE_OPTIONAL(immediate)
    break;
  case X86Local::MRMDestMem:
    // Operand 1 is a memory operand (possibly SIB-extended)
    // Operand 2 is a register operand in the Reg/Opcode field.
    // - In AVX, there is a register operand in the VEX.vvvv field here -
    // Operand 3 (optional) is an immediate.
    assert(numPhysicalOperands >= 2 + additionalOperands &&
           numPhysicalOperands <= 3 + additionalOperands &&
           "Unexpected number of operands for MRMDestMemFrm with VEX_4V");

    HANDLE_OPERAND(memory)

    if (HasEVEX_K)
      HANDLE_OPERAND(writemaskRegister)

    if (HasVEX_4V)
      // FIXME: In AVX, the register below becomes the one encoded
      // in ModRMVEX and the one above the one in the VEX.VVVV field
      HANDLE_OPERAND(vvvvRegister)

    HANDLE_OPERAND(roRegister)
    HANDLE_OPTIONAL(immediate)
    break;
  case X86Local::MRMSrcReg:
    // Operand 1 is a register operand in the Reg/Opcode field.
    // Operand 2 is a register operand in the R/M field.
    // - In AVX, there is a register operand in the VEX.vvvv field here -
    // Operand 3 (optional) is an immediate.
    // Operand 4 (optional) is an immediate.

    assert(numPhysicalOperands >= 2 + additionalOperands &&
           numPhysicalOperands <= 4 + additionalOperands &&
           "Unexpected number of operands for MRMSrcRegFrm");

    HANDLE_OPERAND(roRegister)

    if (HasEVEX_K)
      HANDLE_OPERAND(writemaskRegister)

    if (HasVEX_4V)
      // FIXME: In AVX, the register below becomes the one encoded
      // in ModRMVEX and the one above the one in the VEX.VVVV field
      HANDLE_OPERAND(vvvvRegister)

    if (HasMemOp4Prefix)
      HANDLE_OPERAND(immediate)

    HANDLE_OPERAND(rmRegister)

    if (HasVEX_4VOp3)
      HANDLE_OPERAND(vvvvRegister)

    if (!HasMemOp4Prefix)
      HANDLE_OPTIONAL(immediate)
    HANDLE_OPTIONAL(immediate) // above might be a register in 7:4
    HANDLE_OPTIONAL(immediate)
    break;
  case X86Local::MRMSrcMem:
    // Operand 1 is a register operand in the Reg/Opcode field.
    // Operand 2 is a memory operand (possibly SIB-extended)
    // - In AVX, there is a register operand in the VEX.vvvv field here -
    // Operand 3 (optional) is an immediate.

    assert(numPhysicalOperands >= 2 + additionalOperands &&
           numPhysicalOperands <= 4 + additionalOperands &&
           "Unexpected number of operands for MRMSrcMemFrm");

    HANDLE_OPERAND(roRegister)

    if (HasEVEX_K)
      HANDLE_OPERAND(writemaskRegister)

    if (HasVEX_4V)
      // FIXME: In AVX, the register below becomes the one encoded
      // in ModRMVEX and the one above the one in the VEX.VVVV field
      HANDLE_OPERAND(vvvvRegister)

    if (HasMemOp4Prefix)
      HANDLE_OPERAND(immediate)

    HANDLE_OPERAND(memory)

    if (HasVEX_4VOp3)
      HANDLE_OPERAND(vvvvRegister)

    if (!HasMemOp4Prefix)
      HANDLE_OPTIONAL(immediate)
    HANDLE_OPTIONAL(immediate) // above might be a register in 7:4
    break;
  case X86Local::MRMXr:
  case X86Local::MRM0r:
  case X86Local::MRM1r:
  case X86Local::MRM2r:
  case X86Local::MRM3r:
  case X86Local::MRM4r:
  case X86Local::MRM5r:
  case X86Local::MRM6r:
  case X86Local::MRM7r:
    // Operand 1 is a register operand in the R/M field.
    // Operand 2 (optional) is an immediate or relocation.
    // Operand 3 (optional) is an immediate.
    assert(numPhysicalOperands >= 0 + additionalOperands &&
           numPhysicalOperands <= 3 + additionalOperands &&
           "Unexpected number of operands for MRMnr");

    if (HasVEX_4V)
      HANDLE_OPERAND(vvvvRegister)

    if (HasEVEX_K)
      HANDLE_OPERAND(writemaskRegister)
    HANDLE_OPTIONAL(rmRegister)
    HANDLE_OPTIONAL(relocation)
    HANDLE_OPTIONAL(immediate)
    break;
  case X86Local::MRMXm:
  case X86Local::MRM0m:
  case X86Local::MRM1m:
  case X86Local::MRM2m:
  case X86Local::MRM3m:
  case X86Local::MRM4m:
  case X86Local::MRM5m:
  case X86Local::MRM6m:
  case X86Local::MRM7m:
    // Operand 1 is a memory operand (possibly SIB-extended)
    // Operand 2 (optional) is an immediate or relocation.
    assert(numPhysicalOperands >= 1 + additionalOperands &&
           numPhysicalOperands <= 2 + additionalOperands &&
           "Unexpected number of operands for MRMnm");

    if (HasVEX_4V)
      HANDLE_OPERAND(vvvvRegister)
    if (HasEVEX_K)
      HANDLE_OPERAND(writemaskRegister)
    HANDLE_OPERAND(memory)
    HANDLE_OPTIONAL(relocation)
    break;
  case X86Local::RawFrmImm8:
    // operand 1 is a 16-bit immediate
    // operand 2 is an 8-bit immediate
    assert(numPhysicalOperands == 2 &&
           "Unexpected number of operands for X86Local::RawFrmImm8");
    HANDLE_OPERAND(immediate)
    HANDLE_OPERAND(immediate)
    break;
  case X86Local::RawFrmImm16:
    // operand 1 is a 16-bit immediate
    // operand 2 is a 16-bit immediate
    HANDLE_OPERAND(immediate)
    HANDLE_OPERAND(immediate)
    break;
  case X86Local::MRM_F8:
    if (Opcode == 0xc6) {
      assert(numPhysicalOperands == 1 &&
             "Unexpected number of operands for X86Local::MRM_F8");
      HANDLE_OPERAND(immediate)
    } else if (Opcode == 0xc7) {
      assert(numPhysicalOperands == 1 &&
             "Unexpected number of operands for X86Local::MRM_F8");
      HANDLE_OPERAND(relocation)
    }
    break;
  case X86Local::MRM_C0: case X86Local::MRM_C1: case X86Local::MRM_C2:
  case X86Local::MRM_C3: case X86Local::MRM_C4: case X86Local::MRM_C8:
  case X86Local::MRM_C9: case X86Local::MRM_CA: case X86Local::MRM_CB:
  case X86Local::MRM_CF: case X86Local::MRM_D0: case X86Local::MRM_D1:
  case X86Local::MRM_D4: case X86Local::MRM_D5: case X86Local::MRM_D6:
  case X86Local::MRM_D7: case X86Local::MRM_D8: case X86Local::MRM_D9:
  case X86Local::MRM_DA: case X86Local::MRM_DB: case X86Local::MRM_DC:
  case X86Local::MRM_DD: case X86Local::MRM_DE: case X86Local::MRM_DF:
  case X86Local::MRM_E0: case X86Local::MRM_E1: case X86Local::MRM_E2:
  case X86Local::MRM_E3: case X86Local::MRM_E4: case X86Local::MRM_E5:
  case X86Local::MRM_E8: case X86Local::MRM_E9: case X86Local::MRM_EA:
  case X86Local::MRM_EB: case X86Local::MRM_EC: case X86Local::MRM_ED:
  case X86Local::MRM_EE: case X86Local::MRM_F0: case X86Local::MRM_F1:
  case X86Local::MRM_F2: case X86Local::MRM_F3: case X86Local::MRM_F4:
  case X86Local::MRM_F5: case X86Local::MRM_F6: case X86Local::MRM_F7:
  case X86Local::MRM_F9: case X86Local::MRM_FA: case X86Local::MRM_FB:
  case X86Local::MRM_FC: case X86Local::MRM_FD: case X86Local::MRM_FE:
  case X86Local::MRM_FF:
    // Ignored.
    break;
  }

  #undef HANDLE_OPERAND
  #undef HANDLE_OPTIONAL
}

void RecognizableInstr::emitDecodePath(DisassemblerTables &tables) const {
  // Special cases where the LLVM tables are not complete

#define MAP(from, to)                     \
  case X86Local::MRM_##from:

  OpcodeType    opcodeType  = (OpcodeType)-1;

  ModRMFilter*  filter      = nullptr;
  uint8_t       opcodeToSet = 0;

  switch (OpMap) {
  default: llvm_unreachable("Invalid map!");
  case X86Local::OB:
  case X86Local::TB:
  case X86Local::T8:
  case X86Local::TA:
  case X86Local::XOP8:
  case X86Local::XOP9:
  case X86Local::XOPA:
    switch (OpMap) {
    default: llvm_unreachable("Unexpected map!");
    case X86Local::OB:   opcodeType = ONEBYTE;      break;
    case X86Local::TB:   opcodeType = TWOBYTE;      break;
    case X86Local::T8:   opcodeType = THREEBYTE_38; break;
    case X86Local::TA:   opcodeType = THREEBYTE_3A; break;
    case X86Local::XOP8: opcodeType = XOP8_MAP;     break;
    case X86Local::XOP9: opcodeType = XOP9_MAP;     break;
    case X86Local::XOPA: opcodeType = XOPA_MAP;     break;
    }

    switch (Form) {
    default:
      filter = new DumbFilter();
      break;
    case X86Local::MRMDestReg: case X86Local::MRMDestMem:
    case X86Local::MRMSrcReg:  case X86Local::MRMSrcMem:
    case X86Local::MRMXr:      case X86Local::MRMXm:
      filter = new ModFilter(isRegFormat(Form));
      break;
    case X86Local::MRM0r:      case X86Local::MRM1r:
    case X86Local::MRM2r:      case X86Local::MRM3r:
    case X86Local::MRM4r:      case X86Local::MRM5r:
    case X86Local::MRM6r:      case X86Local::MRM7r:
      filter = new ExtendedFilter(true, Form - X86Local::MRM0r);
      break;
    case X86Local::MRM0m:      case X86Local::MRM1m:
    case X86Local::MRM2m:      case X86Local::MRM3m:
    case X86Local::MRM4m:      case X86Local::MRM5m:
    case X86Local::MRM6m:      case X86Local::MRM7m:
      filter = new ExtendedFilter(false, Form - X86Local::MRM0m);
      break;
    MRM_MAPPING
      filter = new ExactFilter(0xC0 + Form - X86Local::MRM_C0);   \
      break;
    } // switch (Form)

    opcodeToSet = Opcode;
    break;
  } // switch (OpMap)

  unsigned AddressSize = 0;
  switch (AdSize) {
  case X86Local::AdSize16: AddressSize = 16; break;
  case X86Local::AdSize32: AddressSize = 32; break;
  case X86Local::AdSize64: AddressSize = 64; break;
  }

  assert(opcodeType != (OpcodeType)-1 &&
         "Opcode type not set");
  assert(filter && "Filter not set");

  if (Form == X86Local::AddRegFrm) {
    assert(((opcodeToSet & 7) == 0) &&
           "ADDREG_FRM opcode not aligned");

    uint8_t currentOpcode;

    for (currentOpcode = opcodeToSet;
         currentOpcode < opcodeToSet + 8;
         ++currentOpcode)
      tables.setTableFields(opcodeType,
                            insnContext(),
                            currentOpcode,
                            *filter,
                            UID, Is32Bit, IgnoresVEX_L, AddressSize);
  } else {
    tables.setTableFields(opcodeType,
                          insnContext(),
                          opcodeToSet,
                          *filter,
                          UID, Is32Bit, IgnoresVEX_L, AddressSize);
  }

  delete filter;

#undef MAP
}

#define TYPE(str, type) if (s == str) return type;
OperandType RecognizableInstr::typeFromString(const std::string &s,
                                              bool hasREX_WPrefix,
                                              uint8_t OpSize) {
  if(hasREX_WPrefix) {
    // For instructions with a REX_W prefix, a declared 32-bit register encoding
    // is special.
    TYPE("GR32",              TYPE_R32)
  }
  if(OpSize == X86Local::OpSize16) {
    // For OpSize16 instructions, a declared 16-bit register or
    // immediate encoding is special.
    TYPE("GR16",              TYPE_Rv)
    TYPE("i16imm",            TYPE_IMMv)
  } else if(OpSize == X86Local::OpSize32) {
    // For OpSize32 instructions, a declared 32-bit register or
    // immediate encoding is special.
    TYPE("GR32",              TYPE_Rv)
  }
  TYPE("i16mem",              TYPE_Mv)
  TYPE("i16imm",              TYPE_IMM16)
  TYPE("i16i8imm",            TYPE_IMMv)
  TYPE("GR16",                TYPE_R16)
  TYPE("i32mem",              TYPE_Mv)
  TYPE("i32imm",              TYPE_IMMv)
  TYPE("i32i8imm",            TYPE_IMM32)
  TYPE("GR32",                TYPE_R32)
  TYPE("GR32orGR64",          TYPE_R32)
  TYPE("i64mem",              TYPE_Mv)
  TYPE("i64i32imm",           TYPE_IMM64)
  TYPE("i64i8imm",            TYPE_IMM64)
  TYPE("GR64",                TYPE_R64)
  TYPE("i8mem",               TYPE_M8)
  TYPE("i8imm",               TYPE_IMM8)
  TYPE("u8imm",               TYPE_UIMM8)
  TYPE("i32u8imm",            TYPE_UIMM8)
  TYPE("GR8",                 TYPE_R8)
  TYPE("VR128",               TYPE_XMM128)
  TYPE("VR128X",              TYPE_XMM128)
  TYPE("f128mem",             TYPE_M128)
  TYPE("f256mem",             TYPE_M256)
  TYPE("f512mem",             TYPE_M512)
  TYPE("FR64",                TYPE_XMM64)
  TYPE("FR64X",               TYPE_XMM64)
  TYPE("f64mem",              TYPE_M64FP)
  TYPE("sdmem",               TYPE_M64FP)
  TYPE("FR32",                TYPE_XMM32)
  TYPE("FR32X",               TYPE_XMM32)
  TYPE("f32mem",              TYPE_M32FP)
  TYPE("ssmem",               TYPE_M32FP)
  TYPE("RST",                 TYPE_ST)
  TYPE("i128mem",             TYPE_M128)
  TYPE("i256mem",             TYPE_M256)
  TYPE("i512mem",             TYPE_M512)
  TYPE("i64i32imm_pcrel",     TYPE_REL64)
  TYPE("i16imm_pcrel",        TYPE_REL16)
  TYPE("i32imm_pcrel",        TYPE_REL32)
  TYPE("SSECC",               TYPE_IMM3)
  TYPE("XOPCC",               TYPE_IMM3)
  TYPE("AVXCC",               TYPE_IMM5)
  TYPE("AVX512ICC",           TYPE_AVX512ICC)
  TYPE("AVX512RC",            TYPE_IMM32)
  TYPE("brtarget32",          TYPE_RELv)
  TYPE("brtarget16",          TYPE_RELv)
  TYPE("brtarget8",           TYPE_REL8)
  TYPE("f80mem",              TYPE_M80FP)
  TYPE("lea64_32mem",         TYPE_LEA)
  TYPE("lea64mem",            TYPE_LEA)
  TYPE("VR64",                TYPE_MM64)
  TYPE("i64imm",              TYPE_IMMv)
  TYPE("anymem",              TYPE_M)
  TYPE("opaque32mem",         TYPE_M1616)
  TYPE("opaque48mem",         TYPE_M1632)
  TYPE("opaque80mem",         TYPE_M1664)
  TYPE("opaque512mem",        TYPE_M512)
  TYPE("SEGMENT_REG",         TYPE_SEGMENTREG)
  TYPE("DEBUG_REG",           TYPE_DEBUGREG)
  TYPE("CONTROL_REG",         TYPE_CONTROLREG)
  TYPE("srcidx8",             TYPE_SRCIDX8)
  TYPE("srcidx16",            TYPE_SRCIDX16)
  TYPE("srcidx32",            TYPE_SRCIDX32)
  TYPE("srcidx64",            TYPE_SRCIDX64)
  TYPE("dstidx8",             TYPE_DSTIDX8)
  TYPE("dstidx16",            TYPE_DSTIDX16)
  TYPE("dstidx32",            TYPE_DSTIDX32)
  TYPE("dstidx64",            TYPE_DSTIDX64)
  TYPE("offset16_8",          TYPE_MOFFS8)
  TYPE("offset16_16",         TYPE_MOFFS16)
  TYPE("offset16_32",         TYPE_MOFFS32)
  TYPE("offset32_8",          TYPE_MOFFS8)
  TYPE("offset32_16",         TYPE_MOFFS16)
  TYPE("offset32_32",         TYPE_MOFFS32)
  TYPE("offset32_64",         TYPE_MOFFS64)
  TYPE("offset64_8",          TYPE_MOFFS8)
  TYPE("offset64_16",         TYPE_MOFFS16)
  TYPE("offset64_32",         TYPE_MOFFS32)
  TYPE("offset64_64",         TYPE_MOFFS64)
  TYPE("VR256",               TYPE_XMM256)
  TYPE("VR256X",              TYPE_XMM256)
  TYPE("VR512",               TYPE_XMM512)
  TYPE("VK1",                 TYPE_VK1)
  TYPE("VK1WM",               TYPE_VK1)
  TYPE("VK2",                 TYPE_VK2)
  TYPE("VK2WM",               TYPE_VK2)
  TYPE("VK4",                 TYPE_VK4)
  TYPE("VK4WM",               TYPE_VK4)
  TYPE("VK8",                 TYPE_VK8)
  TYPE("VK8WM",               TYPE_VK8)
  TYPE("VK16",                TYPE_VK16)
  TYPE("VK16WM",              TYPE_VK16)
  TYPE("VK32",                TYPE_VK32)
  TYPE("VK32WM",              TYPE_VK32)
  TYPE("VK64",                TYPE_VK64)
  TYPE("VK64WM",              TYPE_VK64)
  TYPE("GR16_NOAX",           TYPE_Rv)
  TYPE("GR32_NOAX",           TYPE_Rv)
  TYPE("GR64_NOAX",           TYPE_R64)
  TYPE("vx32mem",             TYPE_M32)
  TYPE("vx32xmem",            TYPE_M32)
  TYPE("vy32mem",             TYPE_M32)
  TYPE("vy32xmem",            TYPE_M32)
  TYPE("vz32mem",             TYPE_M32)
  TYPE("vx64mem",             TYPE_M64)
  TYPE("vx64xmem",            TYPE_M64)
  TYPE("vy64mem",             TYPE_M64)
  TYPE("vy64xmem",            TYPE_M64)
  TYPE("vz64mem",             TYPE_M64)
  TYPE("BNDR",                TYPE_BNDR)
  errs() << "Unhandled type string " << s << "\n";
  llvm_unreachable("Unhandled type string");
}
#undef TYPE

#define ENCODING(str, encoding) if (s == str) return encoding;
OperandEncoding
RecognizableInstr::immediateEncodingFromString(const std::string &s,
                                               uint8_t OpSize) {
  if(OpSize != X86Local::OpSize16) {
    // For instructions without an OpSize prefix, a declared 16-bit register or
    // immediate encoding is special.
    ENCODING("i16imm",        ENCODING_IW)
  }
  ENCODING("i32i8imm",        ENCODING_IB)
  ENCODING("SSECC",           ENCODING_IB)
  ENCODING("XOPCC",           ENCODING_IB)
  ENCODING("AVXCC",           ENCODING_IB)
  ENCODING("AVX512ICC",       ENCODING_IB)
  ENCODING("AVX512RC",        ENCODING_IB)
  ENCODING("i16imm",          ENCODING_Iv)
  ENCODING("i16i8imm",        ENCODING_IB)
  ENCODING("i32imm",          ENCODING_Iv)
  ENCODING("i64i32imm",       ENCODING_ID)
  ENCODING("i64i8imm",        ENCODING_IB)
  ENCODING("i8imm",           ENCODING_IB)
  ENCODING("u8imm",           ENCODING_IB)
  ENCODING("i32u8imm",        ENCODING_IB)
  // This is not a typo.  Instructions like BLENDVPD put
  // register IDs in 8-bit immediates nowadays.
  ENCODING("FR32",            ENCODING_IB)
  ENCODING("FR64",            ENCODING_IB)
  ENCODING("VR128",           ENCODING_IB)
  ENCODING("VR256",           ENCODING_IB)
  ENCODING("FR32X",           ENCODING_IB)
  ENCODING("FR64X",           ENCODING_IB)
  ENCODING("VR128X",          ENCODING_IB)
  ENCODING("VR256X",          ENCODING_IB)
  ENCODING("VR512",           ENCODING_IB)
  errs() << "Unhandled immediate encoding " << s << "\n";
  llvm_unreachable("Unhandled immediate encoding");
}

OperandEncoding
RecognizableInstr::rmRegisterEncodingFromString(const std::string &s,
                                                uint8_t OpSize) {
  ENCODING("RST",             ENCODING_FP)
  ENCODING("GR16",            ENCODING_RM)
  ENCODING("GR32",            ENCODING_RM)
  ENCODING("GR32orGR64",      ENCODING_RM)
  ENCODING("GR64",            ENCODING_RM)
  ENCODING("GR8",             ENCODING_RM)
  ENCODING("VR128",           ENCODING_RM)
  ENCODING("VR128X",          ENCODING_RM)
  ENCODING("FR64",            ENCODING_RM)
  ENCODING("FR32",            ENCODING_RM)
  ENCODING("FR64X",           ENCODING_RM)
  ENCODING("FR32X",           ENCODING_RM)
  ENCODING("VR64",            ENCODING_RM)
  ENCODING("VR256",           ENCODING_RM)
  ENCODING("VR256X",          ENCODING_RM)
  ENCODING("VR512",           ENCODING_RM)
  ENCODING("VK1",             ENCODING_RM)
  ENCODING("VK2",             ENCODING_RM)
  ENCODING("VK4",             ENCODING_RM)
  ENCODING("VK8",             ENCODING_RM)
  ENCODING("VK16",            ENCODING_RM)
  ENCODING("VK32",            ENCODING_RM)
  ENCODING("VK64",            ENCODING_RM)
  ENCODING("BNDR",            ENCODING_RM)
  errs() << "Unhandled R/M register encoding " << s << "\n";
  llvm_unreachable("Unhandled R/M register encoding");
}

OperandEncoding
RecognizableInstr::roRegisterEncodingFromString(const std::string &s,
                                                uint8_t OpSize) {
  ENCODING("GR16",            ENCODING_REG)
  ENCODING("GR32",            ENCODING_REG)
  ENCODING("GR32orGR64",      ENCODING_REG)
  ENCODING("GR64",            ENCODING_REG)
  ENCODING("GR8",             ENCODING_REG)
  ENCODING("VR128",           ENCODING_REG)
  ENCODING("FR64",            ENCODING_REG)
  ENCODING("FR32",            ENCODING_REG)
  ENCODING("VR64",            ENCODING_REG)
  ENCODING("SEGMENT_REG",     ENCODING_REG)
  ENCODING("DEBUG_REG",       ENCODING_REG)
  ENCODING("CONTROL_REG",     ENCODING_REG)
  ENCODING("VR256",           ENCODING_REG)
  ENCODING("VR256X",          ENCODING_REG)
  ENCODING("VR128X",          ENCODING_REG)
  ENCODING("FR64X",           ENCODING_REG)
  ENCODING("FR32X",           ENCODING_REG)
  ENCODING("VR512",           ENCODING_REG)
  ENCODING("VK1",             ENCODING_REG)
  ENCODING("VK2",             ENCODING_REG)
  ENCODING("VK4",             ENCODING_REG)
  ENCODING("VK8",             ENCODING_REG)
  ENCODING("VK16",            ENCODING_REG)
  ENCODING("VK32",            ENCODING_REG)
  ENCODING("VK64",            ENCODING_REG)
  ENCODING("VK1WM",           ENCODING_REG)
  ENCODING("VK2WM",           ENCODING_REG)
  ENCODING("VK4WM",           ENCODING_REG)
  ENCODING("VK8WM",           ENCODING_REG)
  ENCODING("VK16WM",          ENCODING_REG)
  ENCODING("VK32WM",          ENCODING_REG)
  ENCODING("VK64WM",          ENCODING_REG)
  ENCODING("BNDR",            ENCODING_REG)
  errs() << "Unhandled reg/opcode register encoding " << s << "\n";
  llvm_unreachable("Unhandled reg/opcode register encoding");
}

OperandEncoding
RecognizableInstr::vvvvRegisterEncodingFromString(const std::string &s,
                                                  uint8_t OpSize) {
  ENCODING("GR32",            ENCODING_VVVV)
  ENCODING("GR64",            ENCODING_VVVV)
  ENCODING("FR32",            ENCODING_VVVV)
  ENCODING("FR64",            ENCODING_VVVV)
  ENCODING("VR128",           ENCODING_VVVV)
  ENCODING("VR256",           ENCODING_VVVV)
  ENCODING("FR32X",           ENCODING_VVVV)
  ENCODING("FR64X",           ENCODING_VVVV)
  ENCODING("VR128X",          ENCODING_VVVV)
  ENCODING("VR256X",          ENCODING_VVVV)
  ENCODING("VR512",           ENCODING_VVVV)
  ENCODING("VK1",             ENCODING_VVVV)
  ENCODING("VK2",             ENCODING_VVVV)
  ENCODING("VK4",             ENCODING_VVVV)
  ENCODING("VK8",             ENCODING_VVVV)
  ENCODING("VK16",            ENCODING_VVVV)
  ENCODING("VK32",            ENCODING_VVVV)
  ENCODING("VK64",            ENCODING_VVVV)
  errs() << "Unhandled VEX.vvvv register encoding " << s << "\n";
  llvm_unreachable("Unhandled VEX.vvvv register encoding");
}

OperandEncoding
RecognizableInstr::writemaskRegisterEncodingFromString(const std::string &s,
                                                       uint8_t OpSize) {
  ENCODING("VK1WM",           ENCODING_WRITEMASK)
  ENCODING("VK2WM",           ENCODING_WRITEMASK)
  ENCODING("VK4WM",           ENCODING_WRITEMASK)
  ENCODING("VK8WM",           ENCODING_WRITEMASK)
  ENCODING("VK16WM",          ENCODING_WRITEMASK)
  ENCODING("VK32WM",          ENCODING_WRITEMASK)
  ENCODING("VK64WM",          ENCODING_WRITEMASK)
  errs() << "Unhandled mask register encoding " << s << "\n";
  llvm_unreachable("Unhandled mask register encoding");
}

OperandEncoding
RecognizableInstr::memoryEncodingFromString(const std::string &s,
                                            uint8_t OpSize) {
  ENCODING("i16mem",          ENCODING_RM)
  ENCODING("i32mem",          ENCODING_RM)
  ENCODING("i64mem",          ENCODING_RM)
  ENCODING("i8mem",           ENCODING_RM)
  ENCODING("ssmem",           ENCODING_RM)
  ENCODING("sdmem",           ENCODING_RM)
  ENCODING("f128mem",         ENCODING_RM)
  ENCODING("f256mem",         ENCODING_RM)
  ENCODING("f512mem",         ENCODING_RM)
  ENCODING("f64mem",          ENCODING_RM)
  ENCODING("f32mem",          ENCODING_RM)
  ENCODING("i128mem",         ENCODING_RM)
  ENCODING("i256mem",         ENCODING_RM)
  ENCODING("i512mem",         ENCODING_RM)
  ENCODING("f80mem",          ENCODING_RM)
  ENCODING("lea64_32mem",     ENCODING_RM)
  ENCODING("lea64mem",        ENCODING_RM)
  ENCODING("anymem",          ENCODING_RM)
  ENCODING("opaque32mem",     ENCODING_RM)
  ENCODING("opaque48mem",     ENCODING_RM)
  ENCODING("opaque80mem",     ENCODING_RM)
  ENCODING("opaque512mem",    ENCODING_RM)
  ENCODING("vx32mem",         ENCODING_RM)
  ENCODING("vx32xmem",        ENCODING_RM)
  ENCODING("vy32mem",         ENCODING_RM)
  ENCODING("vy32xmem",        ENCODING_RM)
  ENCODING("vz32mem",         ENCODING_RM)
  ENCODING("vx64mem",         ENCODING_RM)
  ENCODING("vx64xmem",        ENCODING_RM)
  ENCODING("vy64mem",         ENCODING_RM)
  ENCODING("vy64xmem",        ENCODING_RM)
  ENCODING("vz64mem",         ENCODING_RM)
  errs() << "Unhandled memory encoding " << s << "\n";
  llvm_unreachable("Unhandled memory encoding");
}

OperandEncoding
RecognizableInstr::relocationEncodingFromString(const std::string &s,
                                                uint8_t OpSize) {
  if(OpSize != X86Local::OpSize16) {
    // For instructions without an OpSize prefix, a declared 16-bit register or
    // immediate encoding is special.
    ENCODING("i16imm",        ENCODING_IW)
  }
  ENCODING("i16imm",          ENCODING_Iv)
  ENCODING("i16i8imm",        ENCODING_IB)
  ENCODING("i32imm",          ENCODING_Iv)
  ENCODING("i32i8imm",        ENCODING_IB)
  ENCODING("i64i32imm",       ENCODING_ID)
  ENCODING("i64i8imm",        ENCODING_IB)
  ENCODING("i8imm",           ENCODING_IB)
  ENCODING("u8imm",           ENCODING_IB)
  ENCODING("i32u8imm",        ENCODING_IB)
  ENCODING("i64i32imm_pcrel", ENCODING_ID)
  ENCODING("i16imm_pcrel",    ENCODING_IW)
  ENCODING("i32imm_pcrel",    ENCODING_ID)
  ENCODING("brtarget32",      ENCODING_Iv)
  ENCODING("brtarget16",      ENCODING_Iv)
  ENCODING("brtarget8",       ENCODING_IB)
  ENCODING("i64imm",          ENCODING_IO)
  ENCODING("offset16_8",      ENCODING_Ia)
  ENCODING("offset16_16",     ENCODING_Ia)
  ENCODING("offset16_32",     ENCODING_Ia)
  ENCODING("offset32_8",      ENCODING_Ia)
  ENCODING("offset32_16",     ENCODING_Ia)
  ENCODING("offset32_32",     ENCODING_Ia)
  ENCODING("offset32_64",     ENCODING_Ia)
  ENCODING("offset64_8",      ENCODING_Ia)
  ENCODING("offset64_16",     ENCODING_Ia)
  ENCODING("offset64_32",     ENCODING_Ia)
  ENCODING("offset64_64",     ENCODING_Ia)
  ENCODING("srcidx8",         ENCODING_SI)
  ENCODING("srcidx16",        ENCODING_SI)
  ENCODING("srcidx32",        ENCODING_SI)
  ENCODING("srcidx64",        ENCODING_SI)
  ENCODING("dstidx8",         ENCODING_DI)
  ENCODING("dstidx16",        ENCODING_DI)
  ENCODING("dstidx32",        ENCODING_DI)
  ENCODING("dstidx64",        ENCODING_DI)
  errs() << "Unhandled relocation encoding " << s << "\n";
  llvm_unreachable("Unhandled relocation encoding");
}

OperandEncoding
RecognizableInstr::opcodeModifierEncodingFromString(const std::string &s,
                                                    uint8_t OpSize) {
  ENCODING("GR32",            ENCODING_Rv)
  ENCODING("GR64",            ENCODING_RO)
  ENCODING("GR16",            ENCODING_Rv)
  ENCODING("GR8",             ENCODING_RB)
  ENCODING("GR16_NOAX",       ENCODING_Rv)
  ENCODING("GR32_NOAX",       ENCODING_Rv)
  ENCODING("GR64_NOAX",       ENCODING_RO)
  errs() << "Unhandled opcode modifier encoding " << s << "\n";
  llvm_unreachable("Unhandled opcode modifier encoding");
}
#undef ENCODING
