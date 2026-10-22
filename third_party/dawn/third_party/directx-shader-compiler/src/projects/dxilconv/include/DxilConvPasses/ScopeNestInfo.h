///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ScopeNestInfo.h                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Implementation of ScopeNestInfo class and related transformation pass.    //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// Pass to read the scope nest annotations in a cfg and provide a high-level
// view of the scope nesting structure.
//
// The pass follows the same usage patter as the LLVM LoopInfo pass. We have
// a ScopeNestInfo class that contains the results of the scope info
// analysis. The ScopeNestInfoWrapperPass class is the pass implementation
// that runs the analysis and saves the results so it can be queried by
// a later pass.
//
// This pass requires the the -scopenestedcfg pass has been run prior to
// running this pass because we rely on the cfg annotations added by the
// scopenestedcfg pass.
//
// This pass is itself a thin wrapper around the ScopeNestIterator pass. The
// iterator does the heavy lifting and we just cache the results of the
// iteration here. We keep the iterator separate so that it can be easily
// run outside the llvm pass infrastructure.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "DxilConvPasses/ScopeNest.h"
#include "llvm/Pass.h"

namespace llvm {
class Function;
class PassRegistry;
class FunctionPass;

llvm::FunctionPass *createScopeNestInfoWrapperPass();
void initializeScopeNestInfoWrapperPassPass(llvm::PassRegistry &);

// Class to hold the results of the scope nest analysis.
//
// Provides an iterator to examine the sequence of ScopeNestElements.
// We could provide a higher-level view of the scope nesting if needed,
// but that would probably build on the stream of elements anyway.
//
// This class is modeled after llvm LoopInfo.
class ScopeNestInfo {
public:
  typedef std::vector<ScopeNestEvent>::const_iterator elements_iterator;
  typedef iterator_range<elements_iterator> elements_iterator_range;

  elements_iterator elements_begin() { return m_scopeElements.begin(); }
  elements_iterator elements_end() { return m_scopeElements.end(); }
  elements_iterator_range elements() {
    return elements_iterator_range(elements_begin(), elements_end());
  }

  void Analyze(Function &F);
  void print(raw_ostream &O) const;
  void releaseMemory();

private:
  std::vector<ScopeNestEvent> m_scopeElements;

  raw_ostream &indent(raw_ostream &O, int level, StringRef str) const;
};

// The legacy pass manager's analysis pass to read scope nest annotation
// information.
//
// This class is modeled after the llvm LoopInfoWrapperPass.
class ScopeNestInfoWrapperPass : public FunctionPass {
  ScopeNestInfo SI;

public:
  static char ID; // Pass identification, replacement for typeid

  ScopeNestInfoWrapperPass() : FunctionPass(ID) {
    initializeScopeNestInfoWrapperPassPass(*PassRegistry::getPassRegistry());
  }

  ScopeNestInfo &getScopeNestedInfo() { return SI; }
  const ScopeNestInfo &getScopeNestedInfo() const { return SI; }

  // Read the scope nest annotation information for a given function.
  bool runOnFunction(Function &F) override;

  void releaseMemory() override;

  void print(raw_ostream &O, const Module *M = nullptr) const override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};
} // namespace llvm
