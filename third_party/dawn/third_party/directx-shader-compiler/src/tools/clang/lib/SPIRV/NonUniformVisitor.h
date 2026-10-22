//===--- NonUniformVisitor.h - NonUniform Visitor ----------------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_NONUNIFORMVISITOR_H
#define LLVM_CLANG_LIB_SPIRV_NONUNIFORMVISITOR_H

#include "clang/SPIRV/FeatureManager.h"
#include "clang/SPIRV/SpirvContext.h"
#include "clang/SPIRV/SpirvVisitor.h"

namespace clang {
namespace spirv {

class SpirvBuilder;

/// Propagates the NonUniform decoration. According to the Vulkan Spec:
///
/// If an instruction loads from or stores to a resource (including atomics and
/// image instructions) and the resource descriptor being accessed is not
/// dynamically uniform, then the operand corresponding to that resource (e.g.
/// the pointer or sampled image operand) must be decorated with NonUniformEXT.
///
class NonUniformVisitor : public Visitor {
public:
  NonUniformVisitor(SpirvContext &spvCtx, const SpirvCodeGenOptions &opts)
      : Visitor(opts, spvCtx) {}

  bool visit(SpirvLoad *) override;
  bool visit(SpirvAccessChain *) override;
  bool visit(SpirvUnaryOp *) override;
  bool visit(SpirvBinaryOp *) override;
  bool visit(SpirvSampledImage *) override;
  bool visit(SpirvImageTexelPointer *) override;
  bool visit(SpirvAtomic *) override;

  using Visitor::visit;

  /// The "sink" visit function for all instructions.
  ///
  /// By default, all other visit instructions redirect to this visit function.
  /// So that you want override this visit function to handle all instructions,
  /// regardless of their polymorphism.
  bool visitInstruction(SpirvInstruction *instr) override { return true; }

private:
};

} // end namespace spirv
} // end namespace clang

#endif // LLVM_CLANG_LIB_SPIRV_NONUNIFORMVISITOR_H
