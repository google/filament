//===--- RelaxedPrecisionVisitor.h - RelaxedPrecision Visitor ----*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_RELAXEDPRECISIONVISITOR_H
#define LLVM_CLANG_LIB_SPIRV_RELAXEDPRECISIONVISITOR_H

#include "clang/SPIRV/SpirvContext.h"
#include "clang/SPIRV/SpirvVisitor.h"

namespace clang {
namespace spirv {

class RelaxedPrecisionVisitor : public Visitor {
public:
  RelaxedPrecisionVisitor(SpirvContext &spvCtx, const SpirvCodeGenOptions &opts)
      : Visitor(opts, spvCtx) {}

  bool visit(SpirvFunction *, Phase) override;

  bool visit(SpirvVariable *) override;
  bool visit(SpirvFunctionParameter *) override;
  bool visit(SpirvAccessChain *) override;
  bool visit(SpirvAtomic *) override;
  bool visit(SpirvBitFieldExtract *) override;
  bool visit(SpirvBitFieldInsert *) override;
  bool visit(SpirvConstantBoolean *) override;
  bool visit(SpirvConstantInteger *) override;
  bool visit(SpirvConstantFloat *) override;
  bool visit(SpirvConstantComposite *) override;
  bool visit(SpirvCompositeConstruct *) override;
  bool visit(SpirvCompositeExtract *) override;
  bool visit(SpirvCompositeInsert *) override;
  bool visit(SpirvExtInst *) override;
  bool visit(SpirvFunctionCall *) override;
  bool visit(SpirvLoad *) override;
  bool visit(SpirvSelect *) override;
  bool visit(SpirvStore *) override;
  bool visit(SpirvSpecConstantBinaryOp *) override;
  bool visit(SpirvSpecConstantUnaryOp *) override;
  bool visit(SpirvBinaryOp *) override;
  bool visit(SpirvUnaryOp *) override;
  bool visit(SpirvVectorShuffle *) override;
  bool visit(SpirvImageOp *) override;

  using Visitor::visit;

  /// The "sink" visit function for all instructions.
  ///
  /// By default, all other visit instructions redirect to this visit function.
  /// So that you want override this visit function to handle all instructions,
  /// regardless of their polymorphism.
  bool visitInstruction(SpirvInstruction *instr) override { return true; }
};

} // end namespace spirv
} // end namespace clang

#endif // LLVM_CLANG_LIB_SPIRV_RELAXEDPRECISIONVISITOR_H
