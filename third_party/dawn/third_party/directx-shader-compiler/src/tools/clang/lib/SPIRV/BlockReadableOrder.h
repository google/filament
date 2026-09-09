//===--- BlockReadableOrder.h - Visit blocks in human readable order ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// The SPIR-V spec requires code blocks to appear in an order satisfying the
// dominator-tree direction (ie, dominator before the dominated).  This is,
// actually, easy to achieve: any pre-order CFG traversal algorithm will do it.
// Because such algorithms visit a block only after traversing some path to it
// from the root, they necessarily visit the block's immediate dominator first.
//
// But not every graph-traversal algorithm outputs blocks in an order that
// appears logical to human readers.  The problem is that unrelated branches may
// be interspersed with each other, and merge blocks may come before some of the
// branches being merged.
//
// A good, human-readable order of blocks may be achieved by performing
// depth-first search but delaying continue and merge nodes until after all
// their branches have been visited.  This is implemented below by the
// BlockReadableOrderVisitor.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_BLOCKREADABLEORDER_H
#define LLVM_CLANG_LIB_SPIRV_BLOCKREADABLEORDER_H

#include "clang/SPIRV/SpirvBasicBlock.h"
#include "llvm/ADT/DenseSet.h"

namespace clang {
namespace spirv {

/// \brief A basic block visitor traversing basic blocks in a human readable
/// order and calling a pre-set callback on each basic block.
class BlockReadableOrderVisitor {
public:
  explicit BlockReadableOrderVisitor(std::function<void(SpirvBasicBlock *)> cb)
      : callback(cb) {}

  /// \brief Recursively visits all blocks reachable from the given starting
  /// basic block in a depth-first manner and calls the callback passed-in
  /// during construction on each basic block.
  void visit(SpirvBasicBlock *block);

private:
  std::function<void(SpirvBasicBlock *)> callback;

  llvm::DenseSet<SpirvBasicBlock *> doneBlocks; ///< Blocks already visited
  llvm::DenseSet<SpirvBasicBlock *> todoBlocks; ///< Blocks to be visited later
};

} // end namespace spirv
} // end namespace clang

#endif
