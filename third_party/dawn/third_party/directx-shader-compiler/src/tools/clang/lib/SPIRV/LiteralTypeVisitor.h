//===--- LiteralTypeVisitor.h - Literal Type Visitor -------------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_LITERALTYPEVISITOR_H
#define LLVM_CLANG_LIB_SPIRV_LITERALTYPEVISITOR_H

#include "clang/SPIRV/SpirvContext.h"
#include "clang/SPIRV/SpirvVisitor.h"

namespace clang {
namespace spirv {

class LiteralTypeVisitor : public Visitor {
public:
  LiteralTypeVisitor(const ASTContext &ctx, SpirvContext &spvCtx,
                     const SpirvCodeGenOptions &opts)
      : Visitor(opts, spvCtx), astContext(ctx), curFnAstReturnType({}) {}

  bool visit(SpirvFunction *, Phase) override;

  bool visit(SpirvVariable *) override;
  bool visit(SpirvAtomic *) override;
  bool visit(SpirvUnaryOp *) override;
  bool visit(SpirvBinaryOp *) override;
  bool visit(SpirvBitFieldInsert *) override;
  bool visit(SpirvBitFieldExtract *) override;
  bool visit(SpirvSelect *) override;
  bool visit(SpirvVectorShuffle *) override;
  bool visit(SpirvGroupNonUniformOp *) override;
  bool visit(SpirvLoad *) override;
  bool visit(SpirvStore *) override;
  bool visit(SpirvConstantComposite *) override;
  bool visit(SpirvCompositeConstruct *) override;
  bool visit(SpirvCompositeExtract *) override;
  bool visit(SpirvAccessChain *) override;
  bool visit(SpirvExtInst *) override;
  bool visit(SpirvReturn *) override;
  bool visit(SpirvCompositeInsert *) override;
  bool visit(SpirvImageOp *) override;
  bool visit(SpirvSwitch *) override;

  using Visitor::visit;

  // Note: We currently don't do anything to deduce literal types for the
  // following instructions:
  //
  // SpirvImageQuery
  // SpirvImageTexelPointer
  // SpirvSpecConstantBinaryOp
  // SpirvSpecConstantUnaryOp

  /// The "sink" visit function for all instructions.
  ///
  /// By default, all other visit instructions redirect to this visit function.
  /// So that you want override this visit function to handle all instructions,
  /// regardless of their polymorphism.
  bool visitInstruction(SpirvInstruction *instr) override;

private:
  /// If the given instruction's return type is a literal type and the given
  /// 'newType' is not a literal type, and they are of the same kind (both
  /// integer or both float), updates the instruction's result type to newType.
  /// Does nothing otherwise.
  void tryToUpdateInstLitType(SpirvInstruction *, QualType newType);

  /// returns true if the given literal type can be deduced to the given
  /// newType. In order for that to be true,
  /// a) litType must be a literal type
  /// b) litType and newType must be either scalar or vectors of the same size
  /// c) they must have the same underlying type (both int or both float)
  bool canDeduceTypeFromLitType(QualType litType, QualType newType);

  bool updateTypeForCompositeMembers(
      QualType compositeType, llvm::ArrayRef<SpirvInstruction *> constituents);

  /// Returns true if the given constant integer instruction contains a value
  /// that cannot fit in 32 bits.
  bool isLiteralLargerThan32Bits(SpirvConstantInteger *);

private:
  const ASTContext &astContext;
  QualType curFnAstReturnType;
};

} // end namespace spirv
} // end namespace clang

#endif // LLVM_CLANG_LIB_SPIRV_LITERALTYPEVISITOR_H
