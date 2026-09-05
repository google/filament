//===--- NonUniformVisitor.cpp - NonUniform Visitor --------------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "NonUniformVisitor.h"

namespace clang {
namespace spirv {

bool NonUniformVisitor::visit(SpirvLoad *instr) {
  if (instr->getPointer()->isNonUniform())
    instr->setNonUniform();
  return true;
}

bool NonUniformVisitor::visit(SpirvAccessChain *instr) {
  bool isNonUniform = instr->isNonUniform() || instr->getBase()->isNonUniform();
  for (auto *index : instr->getIndexes())
    isNonUniform = isNonUniform || index->isNonUniform();
  instr->setNonUniform(isNonUniform);
  return true;
}

bool NonUniformVisitor::visit(SpirvUnaryOp *instr) {
  if (instr->getOperand()->isNonUniform())
    instr->setNonUniform();
  return true;
}

bool NonUniformVisitor::visit(SpirvBinaryOp *instr) {
  if (instr->getOperand1()->isNonUniform() ||
      instr->getOperand2()->isNonUniform())
    instr->setNonUniform();
  return true;
}

bool NonUniformVisitor::visit(SpirvSampledImage *instr) {
  if (instr->getImage()->isNonUniform() || instr->getSampler()->isNonUniform())
    instr->setNonUniform();
  return true;
}

bool NonUniformVisitor::visit(SpirvImageTexelPointer *instr) {
  if (instr->getImage()->isNonUniform())
    instr->setNonUniform();
  return true;
}

bool NonUniformVisitor::visit(SpirvAtomic *instr) {
  if (instr->getPointer()->isNonUniform())
    instr->setNonUniform();
  return true;
}

} // end namespace spirv
} // end namespace clang
