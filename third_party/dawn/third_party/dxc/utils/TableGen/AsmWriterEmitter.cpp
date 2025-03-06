//===- AsmWriterEmitter.cpp - Generate an assembly writer -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This tablegen backend is emits an assembly printer for the current target.
// Note that this is currently fairly skeletal, but will grow over time.
//
//===----------------------------------------------------------------------===//

#include "AsmWriterInst.h"
#include "CodeGenTarget.h"
#include "SequenceToOffsetTable.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/TableGenBackend.h"
#include <algorithm>
#include <cassert>
#include <map>
#include <vector>
using namespace llvm;

#define DEBUG_TYPE "asm-writer-emitter"

namespace {
class AsmWriterEmitter {
  RecordKeeper &Records;
  CodeGenTarget Target;
  std::map<const CodeGenInstruction*, AsmWriterInst*> CGIAWIMap;
  const std::vector<const CodeGenInstruction*> *NumberedInstructions;
  std::vector<AsmWriterInst> Instructions;
  std::vector<std::string> PrintMethods;
public:
  AsmWriterEmitter(RecordKeeper &R);

  void run(raw_ostream &o);

private:
  void EmitPrintInstruction(raw_ostream &o);
  void EmitGetRegisterName(raw_ostream &o);
  void EmitPrintAliasInstruction(raw_ostream &O);

  AsmWriterInst *getAsmWriterInstByID(unsigned ID) const {
    assert(ID < NumberedInstructions->size());
    std::map<const CodeGenInstruction*, AsmWriterInst*>::const_iterator I =
      CGIAWIMap.find(NumberedInstructions->at(ID));
    assert(I != CGIAWIMap.end() && "Didn't find inst!");
    return I->second;
  }
  void FindUniqueOperandCommands(std::vector<std::string> &UOC,
                                 std::vector<unsigned> &InstIdxs,
                                 std::vector<unsigned> &InstOpsUsed) const;
};
} // end anonymous namespace

static void PrintCases(std::vector<std::pair<std::string,
                       AsmWriterOperand> > &OpsToPrint, raw_ostream &O) {
  O << "    case " << OpsToPrint.back().first << ": ";
  AsmWriterOperand TheOp = OpsToPrint.back().second;
  OpsToPrint.pop_back();

  // Check to see if any other operands are identical in this list, and if so,
  // emit a case label for them.
  for (unsigned i = OpsToPrint.size(); i != 0; --i)
    if (OpsToPrint[i-1].second == TheOp) {
      O << "\n    case " << OpsToPrint[i-1].first << ": ";
      OpsToPrint.erase(OpsToPrint.begin()+i-1);
    }

  // Finally, emit the code.
  O << TheOp.getCode();
  O << "break;\n";
}


/// EmitInstructions - Emit the last instruction in the vector and any other
/// instructions that are suitably similar to it.
static void EmitInstructions(std::vector<AsmWriterInst> &Insts,
                             raw_ostream &O) {
  AsmWriterInst FirstInst = Insts.back();
  Insts.pop_back();

  std::vector<AsmWriterInst> SimilarInsts;
  unsigned DifferingOperand = ~0;
  for (unsigned i = Insts.size(); i != 0; --i) {
    unsigned DiffOp = Insts[i-1].MatchesAllButOneOp(FirstInst);
    if (DiffOp != ~1U) {
      if (DifferingOperand == ~0U)  // First match!
        DifferingOperand = DiffOp;

      // If this differs in the same operand as the rest of the instructions in
      // this class, move it to the SimilarInsts list.
      if (DifferingOperand == DiffOp || DiffOp == ~0U) {
        SimilarInsts.push_back(Insts[i-1]);
        Insts.erase(Insts.begin()+i-1);
      }
    }
  }

  O << "  case " << FirstInst.CGI->Namespace << "::"
    << FirstInst.CGI->TheDef->getName() << ":\n";
  for (unsigned i = 0, e = SimilarInsts.size(); i != e; ++i)
    O << "  case " << SimilarInsts[i].CGI->Namespace << "::"
      << SimilarInsts[i].CGI->TheDef->getName() << ":\n";
  for (unsigned i = 0, e = FirstInst.Operands.size(); i != e; ++i) {
    if (i != DifferingOperand) {
      // If the operand is the same for all instructions, just print it.
      O << "    " << FirstInst.Operands[i].getCode();
    } else {
      // If this is the operand that varies between all of the instructions,
      // emit a switch for just this operand now.
      O << "    switch (MI->getOpcode()) {\n";
      std::vector<std::pair<std::string, AsmWriterOperand> > OpsToPrint;
      OpsToPrint.push_back(std::make_pair(FirstInst.CGI->Namespace + "::" +
                                          FirstInst.CGI->TheDef->getName(),
                                          FirstInst.Operands[i]));

      for (unsigned si = 0, e = SimilarInsts.size(); si != e; ++si) {
        AsmWriterInst &AWI = SimilarInsts[si];
        OpsToPrint.push_back(std::make_pair(AWI.CGI->Namespace+"::"+
                                            AWI.CGI->TheDef->getName(),
                                            AWI.Operands[i]));
      }
      std::reverse(OpsToPrint.begin(), OpsToPrint.end());
      while (!OpsToPrint.empty())
        PrintCases(OpsToPrint, O);
      O << "    }";
    }
    O << "\n";
  }
  O << "    break;\n";
}

void AsmWriterEmitter::
FindUniqueOperandCommands(std::vector<std::string> &UniqueOperandCommands,
                          std::vector<unsigned> &InstIdxs,
                          std::vector<unsigned> &InstOpsUsed) const {
  InstIdxs.assign(NumberedInstructions->size(), ~0U);

  // This vector parallels UniqueOperandCommands, keeping track of which
  // instructions each case are used for.  It is a comma separated string of
  // enums.
  std::vector<std::string> InstrsForCase;
  InstrsForCase.resize(UniqueOperandCommands.size());
  InstOpsUsed.assign(UniqueOperandCommands.size(), 0);

  for (unsigned i = 0, e = NumberedInstructions->size(); i != e; ++i) {
    const AsmWriterInst *Inst = getAsmWriterInstByID(i);
    if (!Inst)
      continue; // PHI, INLINEASM, CFI_INSTRUCTION, etc.

    std::string Command;
    if (Inst->Operands.empty())
      continue;   // Instruction already done.

    Command = "    " + Inst->Operands[0].getCode() + "\n";

    // Check to see if we already have 'Command' in UniqueOperandCommands.
    // If not, add it.
    bool FoundIt = false;
    for (unsigned idx = 0, e = UniqueOperandCommands.size(); idx != e; ++idx)
      if (UniqueOperandCommands[idx] == Command) {
        InstIdxs[i] = idx;
        InstrsForCase[idx] += ", ";
        InstrsForCase[idx] += Inst->CGI->TheDef->getName();
        FoundIt = true;
        break;
      }
    if (!FoundIt) {
      InstIdxs[i] = UniqueOperandCommands.size();
      UniqueOperandCommands.push_back(Command);
      InstrsForCase.push_back(Inst->CGI->TheDef->getName());

      // This command matches one operand so far.
      InstOpsUsed.push_back(1);
    }
  }

  // For each entry of UniqueOperandCommands, there is a set of instructions
  // that uses it.  If the next command of all instructions in the set are
  // identical, fold it into the command.
  for (unsigned CommandIdx = 0, e = UniqueOperandCommands.size();
       CommandIdx != e; ++CommandIdx) {

    for (unsigned Op = 1; ; ++Op) {
      // Scan for the first instruction in the set.
      std::vector<unsigned>::iterator NIT =
        std::find(InstIdxs.begin(), InstIdxs.end(), CommandIdx);
      if (NIT == InstIdxs.end()) break;  // No commonality.

      // If this instruction has no more operands, we isn't anything to merge
      // into this command.
      const AsmWriterInst *FirstInst =
        getAsmWriterInstByID(NIT-InstIdxs.begin());
      if (!FirstInst || FirstInst->Operands.size() == Op)
        break;

      // Otherwise, scan to see if all of the other instructions in this command
      // set share the operand.
      bool AllSame = true;

      for (NIT = std::find(NIT+1, InstIdxs.end(), CommandIdx);
           NIT != InstIdxs.end();
           NIT = std::find(NIT+1, InstIdxs.end(), CommandIdx)) {
        // Okay, found another instruction in this command set.  If the operand
        // matches, we're ok, otherwise bail out.
        const AsmWriterInst *OtherInst =
          getAsmWriterInstByID(NIT-InstIdxs.begin());

        if (!OtherInst || OtherInst->Operands.size() == Op ||
            OtherInst->Operands[Op] != FirstInst->Operands[Op]) {
          AllSame = false;
          break;
        }
      }
      if (!AllSame) break;

      // Okay, everything in this command set has the same next operand.  Add it
      // to UniqueOperandCommands and remember that it was consumed.
      std::string Command = "    " + FirstInst->Operands[Op].getCode() + "\n";

      UniqueOperandCommands[CommandIdx] += Command;
      InstOpsUsed[CommandIdx]++;
    }
  }

  // Prepend some of the instructions each case is used for onto the case val.
  for (unsigned i = 0, e = InstrsForCase.size(); i != e; ++i) {
    std::string Instrs = InstrsForCase[i];
    if (Instrs.size() > 70) {
      Instrs.erase(Instrs.begin()+70, Instrs.end());
      Instrs += "...";
    }

    if (!Instrs.empty())
      UniqueOperandCommands[i] = "    // " + Instrs + "\n" +
        UniqueOperandCommands[i];
  }
}


static void UnescapeString(std::string &Str) {
  for (unsigned i = 0; i != Str.size(); ++i) {
    if (Str[i] == '\\' && i != Str.size()-1) {
      switch (Str[i+1]) {
      default: continue;  // Don't execute the code after the switch.
      case 'a': Str[i] = '\a'; break;
      case 'b': Str[i] = '\b'; break;
      case 'e': Str[i] = 27; break;
      case 'f': Str[i] = '\f'; break;
      case 'n': Str[i] = '\n'; break;
      case 'r': Str[i] = '\r'; break;
      case 't': Str[i] = '\t'; break;
      case 'v': Str[i] = '\v'; break;
      case '"': Str[i] = '\"'; break;
      case '\'': Str[i] = '\''; break;
      case '\\': Str[i] = '\\'; break;
      }
      // Nuke the second character.
      Str.erase(Str.begin()+i+1);
    }
  }
}

/// EmitPrintInstruction - Generate the code for the "printInstruction" method
/// implementation. Destroys all instances of AsmWriterInst information, by
/// clearing the Instructions vector.
void AsmWriterEmitter::EmitPrintInstruction(raw_ostream &O) {
  Record *AsmWriter = Target.getAsmWriter();
  std::string ClassName = AsmWriter->getValueAsString("AsmWriterClassName");
  unsigned PassSubtarget = AsmWriter->getValueAsInt("PassSubtarget");

  O <<
  "/// printInstruction - This method is automatically generated by tablegen\n"
  "/// from the instruction set description.\n"
    "void " << Target.getName() << ClassName
            << "::printInstruction(const MCInst *MI, "
            << (PassSubtarget ? "const MCSubtargetInfo &STI, " : "")
            << "raw_ostream &O) {\n";

  // Build an aggregate string, and build a table of offsets into it.
  SequenceToOffsetTable<std::string> StringTable;

  /// OpcodeInfo - This encodes the index of the string to use for the first
  /// chunk of the output as well as indices used for operand printing.
  /// To reduce the number of unhandled cases, we expand the size from 32-bit
  /// to 32+16 = 48-bit.
  std::vector<uint64_t> OpcodeInfo;

  // Add all strings to the string table upfront so it can generate an optimized
  // representation.
  for (unsigned i = 0, e = NumberedInstructions->size(); i != e; ++i) {
    AsmWriterInst *AWI = CGIAWIMap[NumberedInstructions->at(i)];
    if (AWI &&
        AWI->Operands[0].OperandType ==
                 AsmWriterOperand::isLiteralTextOperand &&
        !AWI->Operands[0].Str.empty()) {
      std::string Str = AWI->Operands[0].Str;
      UnescapeString(Str);
      StringTable.add(Str);
    }
  }

  StringTable.layout();

  unsigned MaxStringIdx = 0;
  for (unsigned i = 0, e = NumberedInstructions->size(); i != e; ++i) {
    AsmWriterInst *AWI = CGIAWIMap[NumberedInstructions->at(i)];
    unsigned Idx;
    if (!AWI) {
      // Something not handled by the asmwriter printer.
      Idx = ~0U;
    } else if (AWI->Operands[0].OperandType !=
                        AsmWriterOperand::isLiteralTextOperand ||
               AWI->Operands[0].Str.empty()) {
      // Something handled by the asmwriter printer, but with no leading string.
      Idx = StringTable.get("");
    } else {
      std::string Str = AWI->Operands[0].Str;
      UnescapeString(Str);
      Idx = StringTable.get(Str);
      MaxStringIdx = std::max(MaxStringIdx, Idx);

      // Nuke the string from the operand list.  It is now handled!
      AWI->Operands.erase(AWI->Operands.begin());
    }

    // Bias offset by one since we want 0 as a sentinel.
    OpcodeInfo.push_back(Idx+1);
  }

  // Figure out how many bits we used for the string index.
  unsigned AsmStrBits = Log2_32_Ceil(MaxStringIdx+2);

  // To reduce code size, we compactify common instructions into a few bits
  // in the opcode-indexed table.
  unsigned BitsLeft = 64-AsmStrBits;

  std::vector<std::vector<std::string>> TableDrivenOperandPrinters;

  while (1) {
    std::vector<std::string> UniqueOperandCommands;
    std::vector<unsigned> InstIdxs;
    std::vector<unsigned> NumInstOpsHandled;
    FindUniqueOperandCommands(UniqueOperandCommands, InstIdxs,
                              NumInstOpsHandled);

    // If we ran out of operands to print, we're done.
    if (UniqueOperandCommands.empty()) break;

    // Compute the number of bits we need to represent these cases, this is
    // ceil(log2(numentries)).
    unsigned NumBits = Log2_32_Ceil(UniqueOperandCommands.size());

    // If we don't have enough bits for this operand, don't include it.
    if (NumBits > BitsLeft) {
      DEBUG(errs() << "Not enough bits to densely encode " << NumBits
                   << " more bits\n");
      break;
    }

    // Otherwise, we can include this in the initial lookup table.  Add it in.
    for (unsigned i = 0, e = InstIdxs.size(); i != e; ++i)
      if (InstIdxs[i] != ~0U) {
        OpcodeInfo[i] |= (uint64_t)InstIdxs[i] << (64-BitsLeft);
      }
    BitsLeft -= NumBits;

    // Remove the info about this operand.
    for (unsigned i = 0, e = NumberedInstructions->size(); i != e; ++i) {
      if (AsmWriterInst *Inst = getAsmWriterInstByID(i))
        if (!Inst->Operands.empty()) {
          unsigned NumOps = NumInstOpsHandled[InstIdxs[i]];
          assert(NumOps <= Inst->Operands.size() &&
                 "Can't remove this many ops!");
          Inst->Operands.erase(Inst->Operands.begin(),
                               Inst->Operands.begin()+NumOps);
        }
    }

    // Remember the handlers for this set of operands.
    TableDrivenOperandPrinters.push_back(std::move(UniqueOperandCommands));
  }


  // We always emit at least one 32-bit table. A second table is emitted if
  // more bits are needed.
  O<<"  static const uint32_t OpInfo[] = {\n";
  for (unsigned i = 0, e = NumberedInstructions->size(); i != e; ++i) {
    O << "    " << (OpcodeInfo[i] & 0xffffffff) << "U,\t// "
      << NumberedInstructions->at(i)->TheDef->getName() << "\n";
  }
  // Add a dummy entry so the array init doesn't end with a comma.
  O << "    0U\n";
  O << "  };\n\n";

  if (BitsLeft < 32) {
    // Add a second OpInfo table only when it is necessary.
    // Adjust the type of the second table based on the number of bits needed.
    O << "  static const uint"
      << ((BitsLeft < 16) ? "32" : (BitsLeft < 24) ? "16" : "8")
      << "_t OpInfo2[] = {\n";
    for (unsigned i = 0, e = NumberedInstructions->size(); i != e; ++i) {
      O << "    " << (OpcodeInfo[i] >> 32) << "U,\t// "
        << NumberedInstructions->at(i)->TheDef->getName() << "\n";
    }
    // Add a dummy entry so the array init doesn't end with a comma.
    O << "    0U\n";
    O << "  };\n\n";
  }

  // Emit the string itself.
  O << "  static const char AsmStrs[] = {\n";
  StringTable.emit(O, printChar);
  O << "  };\n\n";

  O << "  O << \"\\t\";\n\n";

  O << "  // Emit the opcode for the instruction.\n";
  if (BitsLeft < 32) {
    // If we have two tables then we need to perform two lookups and combine
    // the results into a single 64-bit value.
    O << "  uint64_t Bits1 = OpInfo[MI->getOpcode()];\n"
      << "  uint64_t Bits2 = OpInfo2[MI->getOpcode()];\n"
      << "  uint64_t Bits = (Bits2 << 32) | Bits1;\n";
  } else {
    // If only one table is used we just need to perform a single lookup.
    O << "  uint32_t Bits = OpInfo[MI->getOpcode()];\n";
  }
  O << "  assert(Bits != 0 && \"Cannot print this instruction.\");\n"
    << "  O << AsmStrs+(Bits & " << (1 << AsmStrBits)-1 << ")-1;\n\n";

  // Output the table driven operand information.
  BitsLeft = 64-AsmStrBits;
  for (unsigned i = 0, e = TableDrivenOperandPrinters.size(); i != e; ++i) {
    std::vector<std::string> &Commands = TableDrivenOperandPrinters[i];

    // Compute the number of bits we need to represent these cases, this is
    // ceil(log2(numentries)).
    unsigned NumBits = Log2_32_Ceil(Commands.size());
    assert(NumBits <= BitsLeft && "consistency error");

    // Emit code to extract this field from Bits.
    O << "\n  // Fragment " << i << " encoded into " << NumBits
      << " bits for " << Commands.size() << " unique commands.\n";

    if (Commands.size() == 2) {
      // Emit two possibilitys with if/else.
      O << "  if ((Bits >> "
        << (64-BitsLeft) << ") & "
        << ((1 << NumBits)-1) << ") {\n"
        << Commands[1]
        << "  } else {\n"
        << Commands[0]
        << "  }\n\n";
    } else if (Commands.size() == 1) {
      // Emit a single possibility.
      O << Commands[0] << "\n\n";
    } else {
      O << "  switch ((Bits >> "
        << (64-BitsLeft) << ") & "
        << ((1 << NumBits)-1) << ") {\n"
        << "  default: llvm_unreachable(\"Invalid command number.\");\n";

      // Print out all the cases.
      for (unsigned i = 0, e = Commands.size(); i != e; ++i) {
        O << "  case " << i << ":\n";
        O << Commands[i];
        O << "    break;\n";
      }
      O << "  }\n\n";
    }
    BitsLeft -= NumBits;
  }

  // Okay, delete instructions with no operand info left.
  for (unsigned i = 0, e = Instructions.size(); i != e; ++i) {
    // Entire instruction has been emitted?
    AsmWriterInst &Inst = Instructions[i];
    if (Inst.Operands.empty()) {
      Instructions.erase(Instructions.begin()+i);
      --i; --e;
    }
  }


  // Because this is a vector, we want to emit from the end.  Reverse all of the
  // elements in the vector.
  std::reverse(Instructions.begin(), Instructions.end());


  // Now that we've emitted all of the operand info that fit into 32 bits, emit
  // information for those instructions that are left.  This is a less dense
  // encoding, but we expect the main 32-bit table to handle the majority of
  // instructions.
  if (!Instructions.empty()) {
    // Find the opcode # of inline asm.
    O << "  switch (MI->getOpcode()) {\n";
    while (!Instructions.empty())
      EmitInstructions(Instructions, O);

    O << "  }\n";
    O << "  return;\n";
  }

  O << "}\n";
}

static const char *getMinimalTypeForRange(uint64_t Range) {
  assert(Range < 0xFFFFFFFFULL && "Enum too large");
  if (Range > 0xFFFF)
    return "uint32_t";
  if (Range > 0xFF)
    return "uint16_t";
  return "uint8_t";
}

static void
emitRegisterNameString(raw_ostream &O, StringRef AltName,
                       const std::deque<CodeGenRegister> &Registers) {
  SequenceToOffsetTable<std::string> StringTable;
  SmallVector<std::string, 4> AsmNames(Registers.size());
  unsigned i = 0;
  for (const auto &Reg : Registers) {
    std::string &AsmName = AsmNames[i++];

    // "NoRegAltName" is special. We don't need to do a lookup for that,
    // as it's just a reference to the default register name.
    if (AltName == "" || AltName == "NoRegAltName") {
      AsmName = Reg.TheDef->getValueAsString("AsmName");
      if (AsmName.empty())
        AsmName = Reg.getName();
    } else {
      // Make sure the register has an alternate name for this index.
      std::vector<Record*> AltNameList =
        Reg.TheDef->getValueAsListOfDefs("RegAltNameIndices");
      unsigned Idx = 0, e;
      for (e = AltNameList.size();
           Idx < e && (AltNameList[Idx]->getName() != AltName);
           ++Idx)
        ;
      // If the register has an alternate name for this index, use it.
      // Otherwise, leave it empty as an error flag.
      if (Idx < e) {
        std::vector<std::string> AltNames =
          Reg.TheDef->getValueAsListOfStrings("AltNames");
        if (AltNames.size() <= Idx)
          PrintFatalError(Reg.TheDef->getLoc(),
                          "Register definition missing alt name for '" +
                          AltName + "'.");
        AsmName = AltNames[Idx];
      }
    }
    StringTable.add(AsmName);
  }

  StringTable.layout();
  O << "  static const char AsmStrs" << AltName << "[] = {\n";
  StringTable.emit(O, printChar);
  O << "  };\n\n";

  O << "  static const " << getMinimalTypeForRange(StringTable.size()-1)
    << " RegAsmOffset" << AltName << "[] = {";
  for (unsigned i = 0, e = Registers.size(); i != e; ++i) {
    if ((i % 14) == 0)
      O << "\n    ";
    O << StringTable.get(AsmNames[i]) << ", ";
  }
  O << "\n  };\n"
    << "\n";
}

void AsmWriterEmitter::EmitGetRegisterName(raw_ostream &O) {
  Record *AsmWriter = Target.getAsmWriter();
  std::string ClassName = AsmWriter->getValueAsString("AsmWriterClassName");
  const auto &Registers = Target.getRegBank().getRegisters();
  std::vector<Record*> AltNameIndices = Target.getRegAltNameIndices();
  bool hasAltNames = AltNameIndices.size() > 1;

  O <<
  "\n\n/// getRegisterName - This method is automatically generated by tblgen\n"
  "/// from the register set description.  This returns the assembler name\n"
  "/// for the specified register.\n"
  "const char *" << Target.getName() << ClassName << "::";
  if (hasAltNames)
    O << "\ngetRegisterName(unsigned RegNo, unsigned AltIdx) {\n";
  else
    O << "getRegisterName(unsigned RegNo) {\n";
  O << "  assert(RegNo && RegNo < " << (Registers.size()+1)
    << " && \"Invalid register number!\");\n"
    << "\n";

  if (hasAltNames) {
    for (unsigned i = 0, e = AltNameIndices.size(); i < e; ++i)
      emitRegisterNameString(O, AltNameIndices[i]->getName(), Registers);
  } else
    emitRegisterNameString(O, "", Registers);

  if (hasAltNames) {
    O << "  switch(AltIdx) {\n"
      << "  default: llvm_unreachable(\"Invalid register alt name index!\");\n";
    for (unsigned i = 0, e = AltNameIndices.size(); i < e; ++i) {
      std::string Namespace = AltNameIndices[1]->getValueAsString("Namespace");
      std::string AltName(AltNameIndices[i]->getName());
      O << "  case " << Namespace << "::" << AltName << ":\n"
        << "    assert(*(AsmStrs" << AltName << "+RegAsmOffset"
        << AltName << "[RegNo-1]) &&\n"
        << "           \"Invalid alt name index for register!\");\n"
        << "    return AsmStrs" << AltName << "+RegAsmOffset"
        << AltName << "[RegNo-1];\n";
    }
    O << "  }\n";
  } else {
    O << "  assert (*(AsmStrs+RegAsmOffset[RegNo-1]) &&\n"
      << "          \"Invalid alt name index for register!\");\n"
      << "  return AsmStrs+RegAsmOffset[RegNo-1];\n";
  }
  O << "}\n";
}

namespace {
// IAPrinter - Holds information about an InstAlias. Two InstAliases match if
// they both have the same conditionals. In which case, we cannot print out the
// alias for that pattern.
class IAPrinter {
  std::vector<std::string> Conds;
  std::map<StringRef, std::pair<int, int>> OpMap;
  SmallVector<Record*, 4> ReqFeatures;

  std::string Result;
  std::string AsmString;
public:
  IAPrinter(std::string R, std::string AS) : Result(R), AsmString(AS) {}

  void addCond(const std::string &C) { Conds.push_back(C); }

  void addOperand(StringRef Op, int OpIdx, int PrintMethodIdx = -1) {
    assert(OpIdx >= 0 && OpIdx < 0xFE && "Idx out of range");
    assert(PrintMethodIdx >= -1 && PrintMethodIdx < 0xFF &&
           "Idx out of range");
    OpMap[Op] = std::make_pair(OpIdx, PrintMethodIdx);
  }

  bool isOpMapped(StringRef Op) { return OpMap.find(Op) != OpMap.end(); }
  int getOpIndex(StringRef Op) { return OpMap[Op].first; }
  std::pair<int, int> &getOpData(StringRef Op) { return OpMap[Op]; }

  std::pair<StringRef, StringRef::iterator> parseName(StringRef::iterator Start,
                                                      StringRef::iterator End) {
    StringRef::iterator I = Start;
    StringRef::iterator Next;
    if (*I == '{') {
      // ${some_name}
      Start = ++I;
      while (I != End && *I != '}')
        ++I;
      Next = I;
      // eat the final '}'
      if (Next != End)
        ++Next;
    } else {
      // $name, just eat the usual suspects.
      while (I != End &&
             ((*I >= 'a' && *I <= 'z') || (*I >= 'A' && *I <= 'Z') ||
              (*I >= '0' && *I <= '9') || *I == '_'))
        ++I;
      Next = I;
    }

    return std::make_pair(StringRef(Start, I - Start), Next);
  }

  void print(raw_ostream &O) {
    if (Conds.empty() && ReqFeatures.empty()) {
      O.indent(6) << "return true;\n";
      return;
    }

    O << "if (";

    for (std::vector<std::string>::iterator
           I = Conds.begin(), E = Conds.end(); I != E; ++I) {
      if (I != Conds.begin()) {
        O << " &&\n";
        O.indent(8);
      }

      O << *I;
    }

    O << ") {\n";
    O.indent(6) << "// " << Result << "\n";

    // Directly mangle mapped operands into the string. Each operand is
    // identified by a '$' sign followed by a byte identifying the number of the
    // operand. We add one to the index to avoid zero bytes.
    StringRef ASM(AsmString);
    SmallString<128> OutString;
    raw_svector_ostream OS(OutString);
    for (StringRef::iterator I = ASM.begin(), E = ASM.end(); I != E;) {
      OS << *I;
      if (*I == '$') {
        StringRef Name;
        std::tie(Name, I) = parseName(++I, E);
        assert(isOpMapped(Name) && "Unmapped operand!");

        int OpIndex, PrintIndex;
        std::tie(OpIndex, PrintIndex) = getOpData(Name);
        if (PrintIndex == -1) {
          // Can use the default printOperand route.
          OS << format("\\x%02X", (unsigned char)OpIndex + 1);
        } else
          // 3 bytes if a PrintMethod is needed: 0xFF, the MCInst operand
          // number, and which of our pre-detected Methods to call.
          OS << format("\\xFF\\x%02X\\x%02X", OpIndex + 1, PrintIndex + 1);
      } else {
        ++I;
      }
    }
    OS.flush();

    // Emit the string.
    O.indent(6) << "AsmString = \"" << OutString << "\";\n";

    O.indent(6) << "break;\n";
    O.indent(4) << '}';
  }

  bool operator==(const IAPrinter &RHS) const {
    if (Conds.size() != RHS.Conds.size())
      return false;

    unsigned Idx = 0;
    for (const auto &str : Conds)
      if (str != RHS.Conds[Idx++])
        return false;

    return true;
  }
};

} // end anonymous namespace

static unsigned CountNumOperands(StringRef AsmString, unsigned Variant) {
  std::string FlatAsmString =
      CodeGenInstruction::FlattenAsmStringVariants(AsmString, Variant);
  AsmString = FlatAsmString;

  return AsmString.count(' ') + AsmString.count('\t');
}

namespace {
struct AliasPriorityComparator {
  typedef std::pair<CodeGenInstAlias *, int> ValueType;
  bool operator()(const ValueType &LHS, const ValueType &RHS) const {
    if (LHS.second ==  RHS.second) {
      // We don't actually care about the order, but for consistency it
      // shouldn't depend on pointer comparisons.
      return LHS.first->TheDef->getName() < RHS.first->TheDef->getName();
    }

    // Aliases with larger priorities should be considered first.
    return LHS.second > RHS.second;
  }
};
}


void AsmWriterEmitter::EmitPrintAliasInstruction(raw_ostream &O) {
  Record *AsmWriter = Target.getAsmWriter();

  O << "\n#ifdef PRINT_ALIAS_INSTR\n";
  O << "#undef PRINT_ALIAS_INSTR\n\n";

  //////////////////////////////
  // Gather information about aliases we need to print
  //////////////////////////////

  // Emit the method that prints the alias instruction.
  std::string ClassName = AsmWriter->getValueAsString("AsmWriterClassName");
  unsigned Variant = AsmWriter->getValueAsInt("Variant");
  unsigned PassSubtarget = AsmWriter->getValueAsInt("PassSubtarget");

  std::vector<Record*> AllInstAliases =
    Records.getAllDerivedDefinitions("InstAlias");

  // Create a map from the qualified name to a list of potential matches.
  typedef std::set<std::pair<CodeGenInstAlias*, int>, AliasPriorityComparator>
      AliasWithPriority;
  std::map<std::string, AliasWithPriority> AliasMap;
  for (std::vector<Record*>::iterator
         I = AllInstAliases.begin(), E = AllInstAliases.end(); I != E; ++I) {
    CodeGenInstAlias *Alias = new CodeGenInstAlias(*I, Variant, Target);
    const Record *R = *I;
    int Priority = R->getValueAsInt("EmitPriority");
    if (Priority < 1)
      continue; // Aliases with priority 0 are never emitted.

    const DagInit *DI = R->getValueAsDag("ResultInst");
    const DefInit *Op = cast<DefInit>(DI->getOperator());
    AliasMap[getQualifiedName(Op->getDef())].insert(std::make_pair(Alias,
                                                                   Priority));
  }

  // A map of which conditions need to be met for each instruction operand
  // before it can be matched to the mnemonic.
  std::map<std::string, std::vector<IAPrinter*> > IAPrinterMap;

  // A list of MCOperandPredicates for all operands in use, and the reverse map
  std::vector<const Record*> MCOpPredicates;
  DenseMap<const Record*, unsigned> MCOpPredicateMap;

  for (auto &Aliases : AliasMap) {
    for (auto &Alias : Aliases.second) {
      const CodeGenInstAlias *CGA = Alias.first;
      unsigned LastOpNo = CGA->ResultInstOperandIndex.size();
      unsigned NumResultOps =
        CountNumOperands(CGA->ResultInst->AsmString, Variant);

      // Don't emit the alias if it has more operands than what it's aliasing.
      if (NumResultOps < CountNumOperands(CGA->AsmString, Variant))
        continue;

      IAPrinter *IAP = new IAPrinter(CGA->Result->getAsString(),
                                     CGA->AsmString);

      unsigned NumMIOps = 0;
      for (auto &Operand : CGA->ResultOperands)
        NumMIOps += Operand.getMINumOperands();

      std::string Cond;
      Cond = std::string("MI->getNumOperands() == ") + llvm::utostr(NumMIOps);
      IAP->addCond(Cond);

      bool CantHandle = false;

      unsigned MIOpNum = 0;
      for (unsigned i = 0, e = LastOpNo; i != e; ++i) {
        std::string Op = "MI->getOperand(" + llvm::utostr(MIOpNum) + ")";

        const CodeGenInstAlias::ResultOperand &RO = CGA->ResultOperands[i];

        switch (RO.Kind) {
        case CodeGenInstAlias::ResultOperand::K_Record: {
          const Record *Rec = RO.getRecord();
          StringRef ROName = RO.getName();
          int PrintMethodIdx = -1;

          // These two may have a PrintMethod, which we want to record (if it's
          // the first time we've seen it) and provide an index for the aliasing
          // code to use.
          if (Rec->isSubClassOf("RegisterOperand") ||
              Rec->isSubClassOf("Operand")) {
            std::string PrintMethod = Rec->getValueAsString("PrintMethod");
            if (PrintMethod != "" && PrintMethod != "printOperand") {
              PrintMethodIdx = std::find(PrintMethods.begin(),
                                         PrintMethods.end(), PrintMethod) -
                               PrintMethods.begin();
              if (static_cast<unsigned>(PrintMethodIdx) == PrintMethods.size())
                PrintMethods.push_back(PrintMethod);
            }
          }

          if (Rec->isSubClassOf("RegisterOperand"))
            Rec = Rec->getValueAsDef("RegClass");
          if (Rec->isSubClassOf("RegisterClass")) {
            IAP->addCond(Op + ".isReg()");

            if (!IAP->isOpMapped(ROName)) {
              IAP->addOperand(ROName, MIOpNum, PrintMethodIdx);
              Record *R = CGA->ResultOperands[i].getRecord();
              if (R->isSubClassOf("RegisterOperand"))
                R = R->getValueAsDef("RegClass");
              Cond = std::string("MRI.getRegClass(") + Target.getName() + "::" +
                     R->getName() + "RegClassID)"
                                    ".contains(" + Op + ".getReg())";
            } else {
              Cond = Op + ".getReg() == MI->getOperand(" +
                llvm::utostr(IAP->getOpIndex(ROName)) + ").getReg()";
            }
          } else {
            // Assume all printable operands are desired for now. This can be
            // overridden in the InstAlias instantiation if necessary.
            IAP->addOperand(ROName, MIOpNum, PrintMethodIdx);

            // There might be an additional predicate on the MCOperand
            unsigned Entry = MCOpPredicateMap[Rec];
            if (!Entry) {
              if (!Rec->isValueUnset("MCOperandPredicate")) {
                MCOpPredicates.push_back(Rec);
                Entry = MCOpPredicates.size();
                MCOpPredicateMap[Rec] = Entry;
              } else
                break; // No conditions on this operand at all
            }
            Cond = Target.getName() + ClassName + "ValidateMCOperand(" +
                   Op + ", " + llvm::utostr(Entry) + ")";
          }
          // for all subcases of ResultOperand::K_Record:
          IAP->addCond(Cond);
          break;
        }
        case CodeGenInstAlias::ResultOperand::K_Imm: {
          // Just because the alias has an immediate result, doesn't mean the
          // MCInst will. An MCExpr could be present, for example.
          IAP->addCond(Op + ".isImm()");

          Cond = Op + ".getImm() == "
            + llvm::utostr(CGA->ResultOperands[i].getImm());
          IAP->addCond(Cond);
          break;
        }
        case CodeGenInstAlias::ResultOperand::K_Reg:
          // If this is zero_reg, something's playing tricks we're not
          // equipped to handle.
          if (!CGA->ResultOperands[i].getRegister()) {
            CantHandle = true;
            break;
          }

          Cond = Op + ".getReg() == " + Target.getName() +
            "::" + CGA->ResultOperands[i].getRegister()->getName();
          IAP->addCond(Cond);
          break;
        }

        if (!IAP) break;
        MIOpNum += RO.getMINumOperands();
      }

      if (CantHandle) continue;
      IAPrinterMap[Aliases.first].push_back(IAP);
    }
  }

  //////////////////////////////
  // Write out the printAliasInstr function
  //////////////////////////////

  std::string Header;
  raw_string_ostream HeaderO(Header);

  HeaderO << "bool " << Target.getName() << ClassName
          << "::printAliasInstr(const MCInst"
          << " *MI, " << (PassSubtarget ? "const MCSubtargetInfo &STI, " : "")
          << "raw_ostream &OS) {\n";

  std::string Cases;
  raw_string_ostream CasesO(Cases);

  for (std::map<std::string, std::vector<IAPrinter*> >::iterator
         I = IAPrinterMap.begin(), E = IAPrinterMap.end(); I != E; ++I) {
    std::vector<IAPrinter*> &IAPs = I->second;
    std::vector<IAPrinter*> UniqueIAPs;

    for (std::vector<IAPrinter*>::iterator
           II = IAPs.begin(), IE = IAPs.end(); II != IE; ++II) {
      IAPrinter *LHS = *II;
      bool IsDup = false;
      for (std::vector<IAPrinter*>::iterator
             III = IAPs.begin(), IIE = IAPs.end(); III != IIE; ++III) {
        IAPrinter *RHS = *III;
        if (LHS != RHS && *LHS == *RHS) {
          IsDup = true;
          break;
        }
      }

      if (!IsDup) UniqueIAPs.push_back(LHS);
    }

    if (UniqueIAPs.empty()) continue;

    CasesO.indent(2) << "case " << I->first << ":\n";

    for (std::vector<IAPrinter*>::iterator
           II = UniqueIAPs.begin(), IE = UniqueIAPs.end(); II != IE; ++II) {
      IAPrinter *IAP = *II;
      CasesO.indent(4);
      IAP->print(CasesO);
      CasesO << '\n';
    }

    CasesO.indent(4) << "return false;\n";
  }

  if (CasesO.str().empty()) {
    O << HeaderO.str();
    O << "  return false;\n";
    O << "}\n\n";
    O << "#endif // PRINT_ALIAS_INSTR\n";
    return;
  }

  if (!MCOpPredicates.empty())
    O << "static bool " << Target.getName() << ClassName
      << "ValidateMCOperand(\n"
      << "       const MCOperand &MCOp, unsigned PredicateIndex);\n";

  O << HeaderO.str();
  O.indent(2) << "const char *AsmString;\n";
  O.indent(2) << "switch (MI->getOpcode()) {\n";
  O.indent(2) << "default: return false;\n";
  O << CasesO.str();
  O.indent(2) << "}\n\n";

  // Code that prints the alias, replacing the operands with the ones from the
  // MCInst.
  O << "  unsigned I = 0;\n";
  O << "  while (AsmString[I] != ' ' && AsmString[I] != '\t' &&\n";
  O << "         AsmString[I] != '\\0')\n";
  O << "    ++I;\n";
  O << "  OS << '\\t' << StringRef(AsmString, I);\n";

  O << "  if (AsmString[I] != '\\0') {\n";
  O << "    OS << '\\t';\n";
  O << "    do {\n";
  O << "      if (AsmString[I] == '$') {\n";
  O << "        ++I;\n";
  O << "        if (AsmString[I] == (char)0xff) {\n";
  O << "          ++I;\n";
  O << "          int OpIdx = AsmString[I++] - 1;\n";
  O << "          int PrintMethodIdx = AsmString[I++] - 1;\n";
  O << "          printCustomAliasOperand(MI, OpIdx, PrintMethodIdx, ";
  O << (PassSubtarget ? "STI, " : "");
  O << "OS);\n";
  O << "        } else\n";
  O << "          printOperand(MI, unsigned(AsmString[I++]) - 1, ";
  O << (PassSubtarget ? "STI, " : "");
  O << "OS);\n";
  O << "      } else {\n";
  O << "        OS << AsmString[I++];\n";
  O << "      }\n";
  O << "    } while (AsmString[I] != '\\0');\n";
  O << "  }\n\n";

  O << "  return true;\n";
  O << "}\n\n";

  //////////////////////////////
  // Write out the printCustomAliasOperand function
  //////////////////////////////

  O << "void " << Target.getName() << ClassName << "::"
    << "printCustomAliasOperand(\n"
    << "         const MCInst *MI, unsigned OpIdx,\n"
    << "         unsigned PrintMethodIdx,\n"
    << (PassSubtarget ? "         const MCSubtargetInfo &STI,\n" : "")
    << "         raw_ostream &OS) {\n";
  if (PrintMethods.empty())
    O << "  llvm_unreachable(\"Unknown PrintMethod kind\");\n";
  else {
    O << "  switch (PrintMethodIdx) {\n"
      << "  default:\n"
      << "    llvm_unreachable(\"Unknown PrintMethod kind\");\n"
      << "    break;\n";

    for (unsigned i = 0; i < PrintMethods.size(); ++i) {
      O << "  case " << i << ":\n"
        << "    " << PrintMethods[i] << "(MI, OpIdx, "
        << (PassSubtarget ? "STI, " : "") << "OS);\n"
        << "    break;\n";
    }
    O << "  }\n";
  }
  O << "}\n\n";

  if (!MCOpPredicates.empty()) {
    O << "static bool " << Target.getName() << ClassName
      << "ValidateMCOperand(\n"
      << "       const MCOperand &MCOp, unsigned PredicateIndex) {\n"
      << "  switch (PredicateIndex) {\n"
      << "  default:\n"
      << "    llvm_unreachable(\"Unknown MCOperandPredicate kind\");\n"
      << "    break;\n";

    for (unsigned i = 0; i < MCOpPredicates.size(); ++i) {
      Init *MCOpPred = MCOpPredicates[i]->getValueInit("MCOperandPredicate");
      if (StringInit *SI = dyn_cast<StringInit>(MCOpPred)) {
        O << "  case " << i + 1 << ": {\n"
          << SI->getValue() << "\n"
          << "    }\n";
      } else
        llvm_unreachable("Unexpected MCOperandPredicate field!");
    }
    O << "  }\n"
      << "}\n\n";
  }

  O << "#endif // PRINT_ALIAS_INSTR\n";
}

AsmWriterEmitter::AsmWriterEmitter(RecordKeeper &R) : Records(R), Target(R) {
  Record *AsmWriter = Target.getAsmWriter();
  for (const CodeGenInstruction *I : Target.instructions())
    if (!I->AsmString.empty() && I->TheDef->getName() != "PHI")
      Instructions.emplace_back(*I, AsmWriter->getValueAsInt("Variant"),
                                AsmWriter->getValueAsInt("PassSubtarget"));

  // Get the instruction numbering.
  NumberedInstructions = &Target.getInstructionsByEnumValue();

  // Compute the CodeGenInstruction -> AsmWriterInst mapping.  Note that not
  // all machine instructions are necessarily being printed, so there may be
  // target instructions not in this map.
  for (unsigned i = 0, e = Instructions.size(); i != e; ++i)
    CGIAWIMap.insert(std::make_pair(Instructions[i].CGI, &Instructions[i]));
}

void AsmWriterEmitter::run(raw_ostream &O) {
  EmitPrintInstruction(O);
  EmitGetRegisterName(O);
  EmitPrintAliasInstruction(O);
}


namespace llvm {

void EmitAsmWriter(RecordKeeper &RK, raw_ostream &OS) {
  emitSourceFileHeader("Assembly Writer Source Fragment", OS);
  AsmWriterEmitter(RK).run(OS);
}

} // End llvm namespace
