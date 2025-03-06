//===- DisassemblerEmitter.cpp - Generate a disassembler ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "CodeGenTarget.h"
// #include "X86DisassemblerTables.h" // HLSL Change
// #include "X86RecognizableInstr.h"  // HLSL Change
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/TableGenBackend.h"

using namespace llvm;
// using namespace llvm::X86Disassembler; // HLSL Change

/// DisassemblerEmitter - Contains disassembler table emitters for various
/// architectures.

/// X86 Disassembler Emitter
///
/// *** IF YOU'RE HERE TO RESOLVE A "Primary decode conflict", LOOK DOWN NEAR
///     THE END OF THIS COMMENT!
///
/// The X86 disassembler emitter is part of the X86 Disassembler, which is
/// documented in lib/Target/X86/X86Disassembler.h.
///
/// The emitter produces the tables that the disassembler uses to translate
/// instructions.  The emitter generates the following tables:
///
/// - One table (CONTEXTS_SYM) that contains a mapping of attribute masks to
///   instruction contexts.  Although for each attribute there are cases where
///   that attribute determines decoding, in the majority of cases decoding is
///   the same whether or not an attribute is present.  For example, a 64-bit
///   instruction with an OPSIZE prefix and an XS prefix decodes the same way in
///   all cases as a 64-bit instruction with only OPSIZE set.  (The XS prefix
///   may have effects on its execution, but does not change the instruction
///   returned.)  This allows considerable space savings in other tables.
/// - Six tables (ONEBYTE_SYM, TWOBYTE_SYM, THREEBYTE38_SYM, THREEBYTE3A_SYM,
///   THREEBYTEA6_SYM, and THREEBYTEA7_SYM contain the hierarchy that the
///   decoder traverses while decoding an instruction.  At the lowest level of
///   this hierarchy are instruction UIDs, 16-bit integers that can be used to
///   uniquely identify the instruction and correspond exactly to its position
///   in the list of CodeGenInstructions for the target.
/// - One table (INSTRUCTIONS_SYM) contains information about the operands of
///   each instruction and how to decode them.
///
/// During table generation, there may be conflicts between instructions that
/// occupy the same space in the decode tables.  These conflicts are resolved as
/// follows in setTableFields() (X86DisassemblerTables.cpp)
///
/// - If the current context is the native context for one of the instructions
///   (that is, the attributes specified for it in the LLVM tables specify
///   precisely the current context), then it has priority.
/// - If the current context isn't native for either of the instructions, then
///   the higher-priority context wins (that is, the one that is more specific).
///   That hierarchy is determined by outranks() (X86DisassemblerTables.cpp)
/// - If the current context is native for both instructions, then the table
///   emitter reports a conflict and dies.
///
/// *** RESOLUTION FOR "Primary decode conflict"S
///
/// If two instructions collide, typically the solution is (in order of
/// likelihood):
///
/// (1) to filter out one of the instructions by editing filter()
///     (X86RecognizableInstr.cpp).  This is the most common resolution, but
///     check the Intel manuals first to make sure that (2) and (3) are not the
///     problem.
/// (2) to fix the tables (X86.td and its subsidiaries) so the opcodes are
///     accurate.  Sometimes they are not.
/// (3) to fix the tables to reflect the actual context (for example, required
///     prefixes), and possibly to add a new context by editing
///     lib/Target/X86/X86DisassemblerDecoderCommon.h.  This is unlikely to be
///     the cause.
///
/// DisassemblerEmitter.cpp contains the implementation for the emitter,
///   which simply pulls out instructions from the CodeGenTarget and pushes them
///   into X86DisassemblerTables.
/// X86DisassemblerTables.h contains the interface for the instruction tables,
///   which manage and emit the structures discussed above.
/// X86DisassemblerTables.cpp contains the implementation for the instruction
///   tables.
/// X86ModRMFilters.h contains filters that can be used to determine which
///   ModR/M values are valid for a particular instruction.  These are used to
///   populate ModRMDecisions.
/// X86RecognizableInstr.h contains the interface for a single instruction,
///   which knows how to translate itself from a CodeGenInstruction and provide
///   the information necessary for integration into the tables.
/// X86RecognizableInstr.cpp contains the implementation for a single
///   instruction.

namespace llvm {

extern void EmitFixedLenDecoder(RecordKeeper &RK, raw_ostream &OS,
                                std::string PredicateNamespace,
                                std::string GPrefix,
                                std::string GPostfix,
                                std::string ROK,
                                std::string RFail,
                                std::string L);

void EmitDisassembler(RecordKeeper &Records, raw_ostream &OS) {
  CodeGenTarget Target(Records);
  emitSourceFileHeader(" * " + Target.getName() + " Disassembler", OS);

#if 0 // HLSL Change
  // X86 uses a custom disassembler.
  if (Target.getName() == "X86") {
    DisassemblerTables Tables;

    const std::vector<const CodeGenInstruction*> &numberedInstructions =
      Target.getInstructionsByEnumValue();

    for (unsigned i = 0, e = numberedInstructions.size(); i != e; ++i)
      RecognizableInstr::processInstr(Tables, *numberedInstructions[i], i);

    if (Tables.hasConflicts()) {
      PrintError(Target.getTargetRecord()->getLoc(), "Primary decode conflict");
      return;
    }

    Tables.emit(OS);
    return;
  }

  // ARM and Thumb have a CHECK() macro to deal with DecodeStatuses.
  if (Target.getName() == "ARM" || Target.getName() == "Thumb" ||
      Target.getName() == "AArch64" || Target.getName() == "ARM64") {
    std::string PredicateNamespace = Target.getName();
    if (PredicateNamespace == "Thumb")
      PredicateNamespace = "ARM";

    EmitFixedLenDecoder(Records, OS, PredicateNamespace,
                        "if (!Check(S, ", ")) return MCDisassembler::Fail;",
                        "S", "MCDisassembler::Fail",
                        "  MCDisassembler::DecodeStatus S = "
                          "MCDisassembler::Success;\n(void)S;");
    return;
  }
#endif // HLSL Change

  EmitFixedLenDecoder(Records, OS, Target.getName(),
                      "if (", " == MCDisassembler::Fail)"
                       " return MCDisassembler::Fail;",
                      "MCDisassembler::Success", "MCDisassembler::Fail", "");
}

} // End llvm namespace
