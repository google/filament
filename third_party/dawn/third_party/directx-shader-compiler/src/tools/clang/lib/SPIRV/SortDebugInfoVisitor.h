//===--- SortDebugInfoVisitor.h - Debug instrs in Valid order ----*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_SORTDEBUGINFOVISITOR_H
#define LLVM_CLANG_LIB_SPIRV_SORTDEBUGINFOVISITOR_H

#include "clang/SPIRV/SpirvContext.h"
#include "clang/SPIRV/SpirvInstruction.h"
#include "clang/SPIRV/SpirvModule.h"
#include "clang/SPIRV/SpirvVisitor.h"

namespace clang {
namespace spirv {

class SpirvFunction;
class SpirvBasicBlock;

/// The class responsible to sort rich DebugInfo instructions in a valid order
/// without any invalid forward references.
///
/// Since NonSemantic.Shader.DebugInfo.100 has no valid forward references, the
/// result will have no forward references at all.
class SortDebugInfoVisitor : public Visitor {
public:
  SortDebugInfoVisitor(SpirvContext &spvCtx, const SpirvCodeGenOptions &opts)
      : Visitor(opts, spvCtx) {}

  // Sorts debug instructions in a post order to remove invalid forward
  // references. Note that the post order guarantees a successor node is not
  // visited before its predecessor and this property can be used to sort
  // instructions in a valid layout without any invalid forward reference.
  bool visit(SpirvModule *, Phase);

  // Visiting different SPIR-V constructs.
  bool visit(SpirvFunction *, Phase) { return true; }
  bool visit(SpirvBasicBlock *, Phase) { return true; }

  /// The "sink" visit function for all instructions.
  ///
  /// By default, all other visit instructions redirect to this visit function.
  /// So that you want override this visit function to handle all instructions,
  /// regardless of their polymorphism.
  bool visitInstruction(SpirvInstruction *) { return true; }

  using Visitor::visit;

private:
  // Invokes visitor for each operand of the debug instruction `di`. If
  // `visitor` returns false, it stops and returns.
  void whileEachOperandOfDebugInstruction(
      SpirvDebugInstruction *di,
      llvm::function_ref<bool(SpirvDebugInstruction *)> visitor);
};

} // end namespace spirv
} // end namespace clang

#endif // LLVM_CLANG_LIB_SPIRV_SORTDEBUGINFOVISITOR_H
