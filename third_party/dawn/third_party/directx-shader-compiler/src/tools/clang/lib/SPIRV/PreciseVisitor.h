//===--- PreciseVisitor.h ---- Precise Visitor -------------------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_PRECISEVISITOR_H
#define LLVM_CLANG_LIB_SPIRV_PRECISEVISITOR_H

#include "clang/SPIRV/SpirvVisitor.h"

namespace clang {
namespace spirv {

class PreciseVisitor : public Visitor {
public:
  PreciseVisitor(SpirvContext &spvCtx, const SpirvCodeGenOptions &opts)
      : Visitor(opts, spvCtx) {}

  bool visit(SpirvFunction *, Phase) override;

  bool visit(SpirvVariable *) override;
  bool visit(SpirvReturn *) override;
  bool visit(SpirvSelect *) override;
  bool visit(SpirvVectorShuffle *) override;
  bool visit(SpirvBitFieldExtract *) override;
  bool visit(SpirvBitFieldInsert *) override;
  bool visit(SpirvAtomic *) override;
  bool visit(SpirvCompositeConstruct *) override;
  bool visit(SpirvCompositeExtract *) override;
  bool visit(SpirvCompositeInsert *) override;
  bool visit(SpirvLoad *) override;
  bool visit(SpirvStore *) override;
  bool visit(SpirvBinaryOp *) override;
  bool visit(SpirvUnaryOp *) override;
  bool visit(SpirvGroupNonUniformOp *) override;
  bool visit(SpirvExtInst *) override;
  bool visit(SpirvFunctionCall *) override;

  using Visitor::visit;

  // TODO: Support propagation of 'precise' through OpSpecConstantOp and image
  // operations if necessary. Related instruction classes are:
  // SpirvSpecConstantBinaryOp, SpirvSpecConstantUnaryOp
  // SpirvImageOp, SpirvImageQuery, SpirvImageTexelPointer, SpirvSampledImage

private:
  bool curFnRetValPrecise; ///< Whether current function is 'precise'
};

} // end namespace spirv
} // end namespace clang

#endif // LLVM_CLANG_LIB_SPIRV_PRECISEVISITOR_H
